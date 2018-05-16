// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
 *
 * Portions Copyright 2015-2016, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include <bond/core/config.h>

#include "detail/service.h"
#include "server.h"
#include "thread_pool.h"

#ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4100 4702)
#endif

#include <grpcpp/server_builder.h>

#ifdef _MSC_VER
    #pragma warning (pop)
#endif

#include <boost/assert.hpp>

#include <memory>
#include <vector>

namespace bond { namespace ext { namespace gRPC
{
    /// @brief A builder class for the creation and startup of \a
    /// bond::ext::gRPC::server instances.
    class server_builder final
    {
    public:
        /// Register a service. This call does not take ownership of the
        /// service. The service must exist for the lifetime of the \p
        /// server instance returned by \p BuildAndStart().
        ///
        /// Matches requests with any :authority
        server_builder& RegisterService(std::unique_ptr<detail::service> service)
        {
            BOOST_ASSERT(service);
            _builder.RegisterService(service->grpc_service());
            _services.push_back(std::move(service));
            return *this;
        }

        /// Register a service. This call does not take ownership of the
        /// service. The service must exist for the lifetime of the \p
        /// server instance returned by BuildAndStart().
        ///
        /// Only matches requests with :authority \p host
        server_builder& RegisterService(const grpc::string& host, std::unique_ptr<detail::service> service)
        {
            BOOST_ASSERT(service);
            _builder.RegisterService(host, service->grpc_service());
            _services.push_back(std::move(service));
            return *this;
        }

        /// Set max receive message size in bytes.
        server_builder& SetMaxReceiveMessageSize(int max_receive_message_size)
        {
            _builder.SetMaxReceiveMessageSize(max_receive_message_size);
            return *this;
        }

        /// Set max send message size in bytes.
        server_builder& SetMaxSendMessageSize(int max_send_message_size)
        {
            _builder.SetMaxSendMessageSize(max_send_message_size);
            return *this;
        }

        /// @brief Set the support status for compression algorithms.
        ///
        /// All algorithms are enabled by default.
        ///
        /// Incoming calls compressed with an unsupported algorithm will
        /// fail with GRPC_STATUS_UNIMPLEMENTED.
        server_builder& SetCompressionAlgorithmSupportStatus(
            grpc_compression_algorithm algorithm,
            bool enabled)
        {
            _builder.SetCompressionAlgorithmSupportStatus(algorithm, enabled);
            return *this;
        }

        /// The default compression level to use for all channel calls in
        /// the absence of a call-specific level.
        server_builder& SetDefaultCompressionLevel(grpc_compression_level level)
        {
            _builder.SetDefaultCompressionLevel(level);
            return *this;
        }

        /// The default compression algorithm to use for all channel calls
        /// in the absence of a call-specific level. Note that it overrides
        /// any compression level set by \p SetDefaultCompressionLevel.
        server_builder& SetDefaultCompressionAlgorithm(
            grpc_compression_algorithm algorithm)
        {
            _builder.SetDefaultCompressionAlgorithm(algorithm);
            return *this;
        }

        /// Set the attached buffer pool for this server.
        server_builder& SetResourceQuota(const grpc::ResourceQuota& resource_quota)
        {
            _builder.SetResourceQuota(resource_quota);
            return *this;
        }

        server_builder& SetScheduler(const Scheduler& scheduler)
        {
            _scheduler = scheduler;
            return *this;
        }

        /// Tries to bind this server to the given \p addr.
        ///
        /// It can be invoked multiple times.
        ///
        /// @param addr The address to try to bind to the server (eg,
        /// localhost:1234, 192.168.1.1:31416, [::1]:27182, etc.).
        /// @param creds The credentials associated with the server.
        /// @param[out] selected_port Upon success, updated to contain the
        /// port number. \p nullptr otherwise.
        server_builder& AddListeningPort(
            const grpc::string& addr,
            std::shared_ptr<grpc::ServerCredentials> creds,
            int* selected_port = nullptr)
        {
            _builder.AddListeningPort(addr, creds, selected_port);
            return *this;
        }

        /// Return a running server which is ready for processing calls.
        server BuildAndStart()
        {
            if (!_scheduler)
            {
                _scheduler = thread_pool{};
            }

            auto cq = _builder.AddCompletionQueue();
            auto grpcServer = _builder.BuildAndStart();

            for (auto& service : _services)
            {
                service->start(cq.get(), _scheduler);
            }

            std::unique_ptr<io_manager> ioManager{
                new io_manager{
                    std::thread::hardware_concurrency(),
                    /*delay=*/ false,
                    std::move(cq) } };

            return server{ std::move(grpcServer), std::move(_services), std::move(ioManager) };
        }

    private:
        grpc::ServerBuilder _builder;
        std::vector<std::unique_ptr<detail::service>> _services;
        Scheduler _scheduler;
    };

} } } // namespace bond::ext::gRPC
