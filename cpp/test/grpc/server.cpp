// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "services_grpc.h"

#include <bond/ext/grpc/server.h>

#include <boost/static_assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/debug.hpp>

BOOST_AUTO_TEST_SUITE(ServerTests)

class Service1 : public unit_test::Service1::Service
{
public:
    using Service1::Service::Service;

private:
    void Tick(bond::ext::gRPC::unary_call<bond::Void, bond::Void>) override
    {}
};

class Service2 : public unit_test::Service2::Service
{
public:
    using Service2::Service::Service;

private:
    void Tick(bond::ext::gRPC::unary_call<bond::Void, bond::Void>) override
    {}
};

auto scheduler = [](const std::function<void()>& f) { f(); };

const std::string server_address = "127.0.0.1:50051";

BOOST_AUTO_TEST_CASE(SingleServiceStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    BOOST_CHECK_NO_THROW(
        bond::ext::gRPC::server::Start(
            builder,
            std::unique_ptr<Service1>{ new Service1{ scheduler } }));
}

BOOST_AUTO_TEST_CASE(ServicePackStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    BOOST_CHECK_NO_THROW(
        bond::ext::gRPC::server::Start(
            builder,
            std::unique_ptr<Service1>{ new Service1{ scheduler } },
            std::unique_ptr<Service2>{ new Service2{ scheduler } }));
}

BOOST_AUTO_TEST_CASE(ServiceContainerStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    std::vector<std::unique_ptr<Service1>> v;
    v.emplace_back(new Service1{ scheduler });
    v.emplace_back(new Service1{ scheduler });

    BOOST_CHECK_THROW(
        bond::ext::gRPC::server::Start(builder, std::move(v)),
        bond::ext::gRPC::ServerBuildException);
}

BOOST_AUTO_TEST_CASE(AbstractServiceContainerStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    std::vector<std::unique_ptr<bond::ext::gRPC::abstract_service>> v;
    v.emplace_back(new Service1{ scheduler });
    v.emplace_back(new Service2{ scheduler });

    BOOST_CHECK_NO_THROW(bond::ext::gRPC::server::Start(builder, std::move(v)));
}

BOOST_AUTO_TEST_CASE(AbstractServicePackStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    BOOST_CHECK_NO_THROW(
        bond::ext::gRPC::server::Start(
            builder,
            std::unique_ptr<bond::ext::gRPC::abstract_service>{ new Service1{ scheduler } },
            std::unique_ptr<bond::ext::gRPC::abstract_service>{ new Service2{ scheduler } }));
}

const std::string n1 = "s1";
const std::string n2 = "s2";

BOOST_AUTO_TEST_CASE(SingleNamedServiceStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    BOOST_CHECK_NO_THROW(
        bond::ext::gRPC::server::Start(
            builder,
            std::make_pair(n1, std::unique_ptr<Service1>{ new Service1{ scheduler } })));
}

BOOST_AUTO_TEST_CASE(NamedServicePackStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    BOOST_CHECK_NO_THROW(
        bond::ext::gRPC::server::Start(
            builder,
            std::make_pair(n1, std::unique_ptr<Service1>{ new Service1{ scheduler } }),
            std::make_pair(n2, std::unique_ptr<Service2>{ new Service2{ scheduler } })));
}

BOOST_AUTO_TEST_CASE(NamedServiceContainerStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    std::vector<bond::ext::gRPC::named_service<Service1>> v;
    v.emplace_back(n1, new Service1{ scheduler });
    v.emplace_back(n2, new Service1{ scheduler });

    BOOST_CHECK_NO_THROW(bond::ext::gRPC::server::Start(builder, std::move(v)));
}

BOOST_AUTO_TEST_CASE(SameNameServiceContainerStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    std::vector<bond::ext::gRPC::named_service<Service1>> v;
    v.emplace_back(n1, new Service1{ scheduler });
    v.emplace_back(n1, new Service1{ scheduler });

    BOOST_CHECK_THROW(
        bond::ext::gRPC::server::Start(builder, std::move(v)),
        bond::ext::gRPC::ServerBuildException);
}

BOOST_AUTO_TEST_CASE(AbstractNamedServiceContainerStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    std::vector<bond::ext::gRPC::named_service<bond::ext::gRPC::abstract_service>> v;
    v.emplace_back(n1, new Service1{ scheduler });
    v.emplace_back(n2, new Service2{ scheduler });

    BOOST_CHECK_NO_THROW(bond::ext::gRPC::server::Start(builder, std::move(v)));
}

BOOST_AUTO_TEST_CASE(AbstractNamedServicePackStartTest)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    BOOST_CHECK_NO_THROW(
        bond::ext::gRPC::server::Start(
            builder,
            std::make_pair(n1, std::unique_ptr<bond::ext::gRPC::abstract_service>{ new Service1{ scheduler } }),
            std::make_pair(n2, std::unique_ptr<bond::ext::gRPC::abstract_service>{ new Service2{ scheduler } })));
}

BOOST_AUTO_TEST_SUITE_END()

bool init_unit_test()
{
    // grpc allocates a bunch of stuff on-demand caused the leak tracker to
    // report leaks. Disable it for this test.
    boost::debug::detect_memory_leaks(false);

    return true;
}
