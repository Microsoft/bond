// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <bond/core/config.h>

#include "bond_utils.h"
#include "io_manager_tag.h"

#include <bond/core/bonded.h>
#include <bond/ext/grpc/io_manager.h>
#include <bond/ext/grpc/scheduler.h>
#include <bond/ext/grpc/unary_call_result.h>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100 4702)
#endif

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/status.h>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#include <boost/assert.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include <functional>
#include <future>
#include <memory>

namespace bond { namespace ext { namespace gRPC { namespace detail
{
    /// @brief Helper base class Bond gRPC++ clients.
    ///
    /// @note This class is for use by generated and helper code only.
    class client
    {
    public:
        client(
            std::shared_ptr<grpc::ChannelInterface> channel,
            std::shared_ptr<io_manager> ioManager,
            const Scheduler& scheduler)
            : _channel{ std::move(channel) },
              _ioManager{ std::move(ioManager) },
              _scheduler{ scheduler }
        {
            BOOST_ASSERT(_scheduler);
        }

        client(const client& other) = delete;
        client& operator=(const client& other) = delete;

        client(client&& other) = default;
        client& operator=(client&& other) = default;

    private:
        template <typename Request, typename Response>
        class unary_call_data;

    protected:
#if !defined(__GNUC__) || (__GNUC__ > 7) || (__GNUC__ == 7 && __GNUC_MINOR__ >= 2)
        using Method = grpc::internal::RpcMethod;
#else
        // Workaround for a bug in GCC < 7.2: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67054.
        struct Method : grpc::internal::RpcMethod
        {
            using grpc::internal::RpcMethod::RpcMethod;
            Method();
        };
#endif

        Method make_method(const char* name) const
        {
            return Method{ name, grpc::internal::RpcMethod::NORMAL_RPC, _channel };
        }

        template <typename Request = Void, typename Response = bonded<Void>>
        void dispatch(
            const grpc::internal::RpcMethod& method,
            std::shared_ptr<grpc::ClientContext> context,
            const std::function<void(unary_call_result<Response>)>& cb,
            const bonded<Request>& request = bonded<Request>{ Request{} })
        {
            new unary_call_data<Request, Response>{
                method,
                request,
                _ioManager->shared_cq(),
                _channel,
                context ? std::move(context) : std::make_shared<grpc::ClientContext>(),
                _scheduler,
                cb };
        }

        template <typename Response, typename Request = Void>
        std::future<unary_call_result<Response>> dispatch(
            const grpc::internal::RpcMethod& method,
            std::shared_ptr<grpc::ClientContext> context,
            const bonded<Request>& request = bonded<Request>{ Request{} })
        {
            auto callback = std::make_shared<std::packaged_task<unary_call_result<Response>(unary_call_result<Response>)>>(
                [](unary_call_result<Response> response)
                {
                    return response.status().ok()
                        ? response
                        : throw UnaryCallException{ response.status(), response.context() };
                });

            auto result = callback->get_future();

            std::function<void(unary_call_result<Response>)> copyableCallback =
                [callback](unary_call_result<Response> response)
                {
                    (*callback)(std::move(response));
                };

            dispatch(method, std::move(context), copyableCallback, request);

            return result;
        }

    private:
        std::shared_ptr<grpc::ChannelInterface> _channel;
        std::shared_ptr<io_manager> _ioManager;
        Scheduler _scheduler;
    };

    /// @brief Implementation class that hold the state associated with
    /// outgoing unary calls.
    template <typename Request, typename Response>
    class client::unary_call_data
        : public boost::intrusive_ref_counter<unary_call_data<Request, Response>>,
          io_manager_tag
    {
    public:
        template <typename Callback>
        unary_call_data(
            const grpc::internal::RpcMethod& method,
            const bonded<Request>& request,
            std::shared_ptr<grpc::CompletionQueue> cq,
            std::shared_ptr<grpc::ChannelInterface> channel,
            std::shared_ptr<grpc::ClientContext> context,
            const Scheduler& scheduler,
            Callback&& cb)
            : _cq(std::move(cq)),
              _channel(std::move(channel)),
              _context(std::move(context)),
              _responseReader(
                  ::grpc::internal::ClientAsyncResponseReaderFactory<bonded<Response>>::Create(
                      _channel.get(),
                      _cq.get(),
                      method,
                      _context.get(),
                      request,
                      /* start */ true)),
              _scheduler(scheduler),
              _response(),
              _status(),
              _cb(std::forward<Callback>(cb)),
              _self(this)
        {
            BOOST_ASSERT(_scheduler);

            auto self = _self; // Make sure `this` will outlive the below call.
            _responseReader->Finish(&_response, &_status, tag());
        }

    private:
        /// @brief Invoked after the response has been received.
        void invoke(bool ok) override
        {
            if (ok && _cb)
            {
                // TODO: Use lambda with move-capture when allowed to use C++14.
                _scheduler(std::bind(
                    [](decltype(_cb)& cb, unary_call_result<Response>& result) { cb(std::move(result)); },
                    std::move(_cb),
                    unary_call_result<Response>{ std::move(_response), _status, std::move(_context) }));
            }

            _self.reset();
        }

        /// The completion port to post IO operations to.
        std::shared_ptr<grpc::CompletionQueue> _cq;
        /// The channel to send the request on.
        std::shared_ptr<grpc::ChannelInterface> _channel;
        /// @brief The client context under which the request was executed.
        std::shared_ptr<grpc::ClientContext> _context;
        /// A response reader.
        std::unique_ptr<grpc::ClientAsyncResponseReader<bonded<Response>>> _responseReader;
        /// The scheduler in which to invoke the callback.
        Scheduler _scheduler;
        /// @brief The response received from the service.
        bonded<Response> _response;
        /// @brief The status of the request.
        grpc::Status _status;
        /// The user code to invoke when a response is received.
        std::function<void(unary_call_result<Response>)> _cb;
        /// A pointer to ourselves used to keep us alive while waiting to
        /// receive the response.
        boost::intrusive_ptr<unary_call_data> _self;
    };

} } } } // namespace bond::ext::gRPC::detail
