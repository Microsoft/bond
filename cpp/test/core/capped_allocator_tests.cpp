#include "precompiled.h"
#include "capped_allocator_tests_generated/allocator_test_reflection.h"
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/irange.hpp>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100)
#endif
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#ifdef _MSC_VER
#pragma warning (pop)
#endif

#include <vector>
#include <limits>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(CappedAllocatorTests)

using all_counter_types = boost::mpl::list<
    bond::single_threaded_counter<>,
    bond::multi_threaded_counter<>,
    bond::shared_counter<bond::single_threaded_counter<>>,
    bond::shared_counter<bond::multi_threaded_counter<>>>;

using thread_safe_counter_types = boost::mpl::list<
    bond::multi_threaded_counter<>,
    bond::shared_counter<bond::multi_threaded_counter<>>>;

template <typename T = void>
struct allocator_with_state : std::allocator<T>
{
    template <typename U>
    struct rebind
    {
        using other = allocator_with_state<U>;
    };

    template <typename U>
    allocator_with_state(std::shared_ptr<U> state)
        : state{ std::move(state) }
    {}

    template <typename U>
    allocator_with_state(const allocator_with_state<U>& other)
        : state{ other.state }
    {}

    std::shared_ptr<void> state;
};

template <typename T>
inline bool operator==(const allocator_with_state<T>& a1, const allocator_with_state<T>& a2)
{
    return a1.state == a2.state;
}

template <typename T>
inline bool operator!=(const allocator_with_state<T>& a1, const allocator_with_state<T>& a2)
{
    return !(a1 == a2);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CounterBasicTests, Counter, all_counter_types)
{
    Counter counter{ 300 };
    BOOST_CHECK_EQUAL(counter.max_value(), 300u);
    BOOST_CHECK_EQUAL(counter.value(), 0u);
    BOOST_CHECK(counter.try_add(100));
    BOOST_CHECK_EQUAL(counter.value(), 100u);
    counter.subtract(70);
    BOOST_CHECK_EQUAL(counter.value(), 30u);
    BOOST_CHECK(counter.try_add(270));
    BOOST_CHECK_EQUAL(counter.value(), 300u);
    counter.subtract(10);
    BOOST_CHECK_EQUAL(counter.value(), 290u);
    BOOST_CHECK(!counter.try_add(11));
    BOOST_CHECK(!counter.try_add(100));
    BOOST_CHECK(!counter.try_add((std::numeric_limits<std::size_t>::max)() - counter.value()));
    BOOST_CHECK_EQUAL(counter.value(), 290u);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CounterThreadSafetyTests, Counter, thread_safe_counter_types)
{
    auto test = [](Counter& counter, std::size_t thread_count, std::size_t iterations, std::size_t delta)
    {
        std::vector<boost::scoped_thread<>> threads;

        for (size_t t = 0; t < thread_count; ++t)
        {
            threads.emplace_back([&]
            {
                for (std::size_t i = 0; i < iterations; ++i)
                {
                    counter.try_add(delta);
                }
            });
        }
    };

    // BOOST_TEST_CONTEXT("Iterations without overflow")
    {
        Counter counter{ 4000 };
        test(counter, 4, 100, 10);
        BOOST_CHECK_EQUAL(counter.value(), 4000u);
    }

    // BOOST_TEST_CONTEXT("Iterations with overflow")
    {
        Counter counter{ 3000 };
        test(counter, 4, 100, 10);
        BOOST_CHECK_EQUAL(counter.value(), 3000u);
    }
}

BOOST_AUTO_TEST_CASE(SharedCounterAllocationTests)
{
    auto state = std::make_shared<int>();
    BOOST_CHECK(state.unique());
    {
        allocator_with_state<> alloc{ state };
        BOOST_CHECK_EQUAL(state.use_count(), 2);

        bond::shared_counter<> counter{ 1024, alloc };
        const auto initial_value = counter.value();
        BOOST_CHECK_NE(initial_value, 0u);
        const auto inital_use_count = state.use_count();
        BOOST_CHECK_GT(inital_use_count, 2);

        BOOST_CHECK_THROW(
            bond::shared_counter<>(counter.value() - 1, alloc),
            std::bad_alloc);
        
        auto copy1 = counter;
        BOOST_CHECK_EQUAL(copy1.value(), initial_value);

        auto copy2 = counter;
        BOOST_CHECK_EQUAL(copy2.value(), initial_value);

        BOOST_CHECK_EQUAL(state.use_count(), inital_use_count);

        BOOST_CHECK(counter.try_add(10));
        BOOST_CHECK(copy1.try_add(20));
        BOOST_CHECK(copy1.try_add(30));

        const auto expected_value = initial_value + 60;

        BOOST_CHECK_EQUAL(counter.value(), expected_value);
        BOOST_CHECK_EQUAL(copy1.value(), expected_value);
        BOOST_CHECK_EQUAL(copy2.value(), expected_value);
    }
    BOOST_CHECK(state.unique());
}

BOOST_AUTO_TEST_CASE(AllocatorCounterReferenceTest)
{
    // BOOST_TEST_CONTEXT("Reference type holding a reference")
    {
        bond::single_threaded_counter<> counter{ 0 };
        bond::capped_allocator<std::allocator<char>, decltype(counter)&> alloc{ counter };
        BOOST_CHECK_EQUAL(&counter, &alloc.get_counter());
    }

    // BOOST_TEST_CONTEXT("Reference type holding a reference using std::ref")
    {
        bond::single_threaded_counter<> counter{ 0 };
        bond::capped_allocator<std::allocator<char>, decltype(counter)&> alloc{ std::ref(counter) };
        BOOST_CHECK_EQUAL(&counter, &alloc.get_counter());
    }

    // BOOST_TEST_CONTEXT("Value type holding a value")
    {
        bond::shared_counter<> counter{ 0 };
        bond::capped_allocator<std::allocator<char>, decltype(counter)> alloc{ counter };
        BOOST_CHECK_NE(&counter, &alloc.get_counter());
    }

    // BOOST_TEST_CONTEXT("Value type holding a reference")
    {
        bond::shared_counter<> counter{ 0 };
        bond::capped_allocator<std::allocator<char>, decltype(counter)> alloc{ std::ref(counter) };
        BOOST_CHECK_EQUAL(&counter, &alloc.get_counter());
    }
}

BOOST_AUTO_TEST_CASE(AllocatorSubtractOnDeallocateTest)
{
    bond::single_threaded_counter<> counter{ 10 };
    bond::capped_allocator<std::allocator<char>, decltype(counter)&> alloc{ counter, {}, false };

    auto test = [](decltype(alloc)& alloc)
    {
        const auto initial_count = alloc.get_counter().value();
        const auto ptr = alloc.allocate(1);
        BOOST_CHECK_EQUAL(alloc.get_counter().value(), initial_count + 1);
        alloc.deallocate(ptr, 1);
        BOOST_CHECK_EQUAL(alloc.get_counter().value(), initial_count + 1);
    };

    // BOOST_TEST_CONTEXT("No subtraction on deallocation")
    {
        test(alloc);
    }

    // BOOST_TEST_CONTEXT("No subtraction on deallocation for a copy")
    {
        auto copy = alloc;
        test(copy);
    }

    // BOOST_TEST_CONTEXT("No subtraction on deallocation for select_on_container_copy_construction result")
    {
        auto selected_alloc = alloc.select_on_container_copy_construction();
        test(selected_alloc);
    }
}

BOOST_AUTO_TEST_CASE(AllocatorCounterInBytesTest)
{
    bond::single_threaded_counter<> counter{ 10 };
    bond::capped_allocator<std::allocator<char[10]>, decltype(counter)&> alloc{ counter };
    BOOST_CHECK_EQUAL(counter.value(), 0u);
    const auto ptr = alloc.allocate(1);
    BOOST_CHECK_NE(ptr, static_cast<void*>(nullptr));
    BOOST_CHECK_EQUAL(counter.value(), 10u);
    alloc.deallocate(ptr, 1);
    BOOST_CHECK_EQUAL(counter.value(), 0u);
}

BOOST_AUTO_TEST_CASE(AllocatorAwareCounterTest)
{
    static std::vector<int> allocate_call_args;

    struct counter_mock
    {
        using value_type = int;

        using allocator_type = std::allocator<void>;

        explicit counter_mock(int)
        {}

        counter_mock(int x, const std::allocator<void>&)
        {
            allocate_call_args.push_back(x);
        }
    };


    // BOOST_TEST_CONTEXT("Counter passed by value")
    {
        bond::capped_allocator<std::allocator<void>, counter_mock> alloc{ counter_mock{ 10 } };
        BOOST_CHECK(allocate_call_args.empty());
    }

    // BOOST_TEST_CONTEXT("Counter passed by reference")
    {
        counter_mock counter{ 20 };
        bond::capped_allocator<std::allocator<void>, counter_mock> alloc{ std::ref(counter) };
        BOOST_CHECK(allocate_call_args.empty());
    }

    // BOOST_TEST_CONTEXT("Counter emplacement")
    {
        bond::capped_allocator<std::allocator<void>, counter_mock> alloc{ 30 };
        BOOST_REQUIRE_EQUAL(allocate_call_args.size(), 1u);
        BOOST_CHECK_EQUAL(allocate_call_args.front(), 30);
    }
}

BOOST_AUTO_TEST_CASE(AllocatorExceptionSafetyTest)
{
    bond::capped_allocator<std::allocator<char>, bond::shared_counter<>> alloc{ 100 };
    const auto count = alloc.get_counter().value();
    BOOST_CHECK_NE(count, 0u);

    BOOST_CHECK_THROW(alloc.allocate(alloc.get_counter().max_value() - count + 1), std::bad_alloc);
    BOOST_CHECK_EQUAL(alloc.get_counter().value(), count);
}

BOOST_AUTO_TEST_CASE(AllocatorComparisonTest)
{
    auto state = std::make_shared<int>();

    bond::capped_allocator<allocator_with_state<>> a1(1024, state);
    BOOST_CHECK_EQUAL(a1.get_allocator().state, state);

    bond::capped_allocator<allocator_with_state<>> a2(1024, state);
    BOOST_CHECK_EQUAL(a2.get_allocator().state, state);

    BOOST_CHECK_NE(&a1.get_counter(), &a2.get_counter());
    BOOST_CHECK((a1 == a2));

    bond::capped_allocator<allocator_with_state<>> a3(1024, std::make_shared<int>());
    BOOST_CHECK_NE(a1.get_allocator().state, a3.get_allocator().state);
    BOOST_CHECK_NE(a2.get_allocator().state, a3.get_allocator().state);
    BOOST_CHECK((a1 != a3));
    BOOST_CHECK((a2 != a3));
}

using all_protocols = boost::mpl::list<
    bond::SimpleBinaryReader<bond::InputBuffer>,
    bond::CompactBinaryReader<bond::InputBuffer>,
    bond::FastBinaryReader<bond::InputBuffer> >;

BOOST_AUTO_TEST_CASE_TEMPLATE(BondStructDeserializationTest, Reader, all_protocols)
{
    capped_allocator_tests::test_allocator<std::allocator<char>> alloc{ 1024 * 1024 };

    capped_allocator_tests::Struct from{ alloc };

    InitRandom(from);

    // BOOST_TEST_CONTEXT("Serialization with overflow")
    {
        using Writer = typename bond::get_protocol_writer<Reader, bond::OutputMemoryStream<decltype(alloc)>>::type;

        decltype(alloc) new_alloc{ 1024 };
        typename Writer::Buffer output{ new_alloc };
        Writer writer{ output };
        BOOST_CHECK_THROW(bond::Serialize(from, writer), std::bad_alloc);
    }

    bond::blob buffer;

    // BOOST_TEST_CONTEXT("Serialization without overflow")
    {
        using Writer = typename bond::get_protocol_writer<Reader, bond::OutputBuffer>::type;

        typename Writer::Buffer output;
        Writer writer{ output };
        BOOST_CHECK_NO_THROW(bond::Serialize(from, writer));

        buffer = output.GetBuffer();
    }

    // BOOST_TEST_CONTEXT("Compile-time schema deserialize without overflow")
    {
        Reader reader{ buffer };

        decltype(alloc) new_alloc{ 1024 * 1024 };
        decltype(from) to{ new_alloc };
        BOOST_CHECK_NO_THROW(bond::Deserialize(reader, to));
        BOOST_CHECK((from == to));
    }

    // BOOST_TEST_CONTEXT("Compile-time schema deserialize with overflow")
    {
        Reader reader{ buffer };

        decltype(alloc) new_alloc{ 4 * 1024 };
        decltype(from) to{ new_alloc };
        BOOST_CHECK_THROW(bond::Deserialize(reader, to), std::bad_alloc);
    }

    // BOOST_TEST_CONTEXT("Runtime schema deserialize without overflow")
    {
        Reader reader{ buffer };
        bond::bonded<void> bonded{ reader, bond::GetRuntimeSchema<decltype(from)>() };

        decltype(alloc) new_alloc{ 1024 * 1024 };
        decltype(from) to{ new_alloc };
        BOOST_CHECK_NO_THROW(bonded.Deserialize(to));
        BOOST_CHECK((from == to));
    }

    // BOOST_TEST_CONTEXT("Runtime schema deserialize with overflow")
    {
        Reader reader{ buffer };
        bond::bonded<void> bonded{ reader, bond::GetRuntimeSchema<decltype(from)>() };

        decltype(alloc) new_alloc{ 4 * 1024 };
        decltype(from) to{ new_alloc };
        BOOST_CHECK_THROW(bonded.Deserialize(to), std::bad_alloc);
    }
}

BOOST_AUTO_TEST_SUITE_END()

bool init_unit_test()
{
    return true;
}
