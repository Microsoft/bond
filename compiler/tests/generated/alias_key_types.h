
#pragma once

#include <bond/core/bond_version.h>

#if BOND_VERSION < 0x0700
#error This file was generated by a newer version of the Bond compiler and is incompatible with your version of the Bond library.
#endif

#if BOND_MIN_CODEGEN_VERSION > 0x0a10
#error This file was generated by an older version of the Bond compiler and is incompatible with your version of the Bond library.
#endif

#include <bond/core/config.h>
#include <bond/core/containers.h>



namespace test
{
    
    struct foo
    {
        std::map<std::string, int32_t> m;
        std::set<int32_t> s;
        
        foo()
        {
        }

        
#ifndef BOND_NO_CXX11_DEFAULTED_FUNCTIONS
        // Compiler generated copy ctor OK
        foo(const foo&) = default;
#endif
        
#if !defined(BOND_NO_CXX11_DEFAULTED_MOVE_CTOR)
        foo(foo&&) = default;
#elif !defined(BOND_NO_CXX11_RVALUE_REFERENCES)
        foo(foo&& other)
          : m(std::move(other.m)),
            s(std::move(other.s))
        {
        }
#endif
        
        
#ifndef BOND_NO_CXX11_DEFAULTED_FUNCTIONS
        // Compiler generated operator= OK
        foo& operator=(const foo&) = default;
        foo& operator=(foo&&) = default;
#endif

        bool operator==(const foo& other) const
        {
            return true
                && (m == other.m)
                && (s == other.s);
        }

        bool operator!=(const foo& other) const
        {
            return !(*this == other);
        }

        void swap(foo& other)
        {
            using std::swap;
            swap(m, other.m);
            swap(s, other.s);
        }

        struct Schema;

    protected:
        void InitMetadata(const char*, const char*)
        {
        }
    };

    inline void swap(::test::foo& left, ::test::foo& right)
    {
        left.swap(right);
    }
} // namespace test

