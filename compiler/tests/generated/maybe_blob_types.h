
#pragma once

#include <bond/core/bond_version.h>

#if BOND_VERSION < 0x0520
#error This file was generated by a newer version of the Bond compiler and is incompatible with your version of the Bond library.
#endif

#if BOND_MIN_CODEGEN_VERSION > 0x0a00
#error This file was generated by an older version of the Bond compiler and is incompatible with your version of the Bond library.
#endif

#include <bond/core/config.h>
#include <bond/core/containers.h>
#include <bond/core/blob.h>


namespace tests
{
    
    struct Foo
    {
        ::bond::maybe< ::bond::blob> b;
        
        template <typename = void> // Workaround to avoid compilation if not used
        Foo()
        {
        }

        
#ifndef BOND_NO_CXX11_DEFAULTED_FUNCTIONS
        // Compiler generated copy ctor OK
        Foo(const Foo&) = default;
#endif
        
#if !defined(BOND_NO_CXX11_DEFAULTED_MOVE_CTOR)
        Foo(Foo&&) = default;
#elif !defined(BOND_NO_CXX11_RVALUE_REFERENCES)
        Foo(Foo&& other)
          : b(std::move(other.b))
        {
        }
#endif
        
        
#ifndef BOND_NO_CXX11_DEFAULTED_FUNCTIONS
        // Compiler generated operator= OK
        Foo& operator=(const Foo&) = default;
#endif

        bool operator==(const Foo& other) const
        {
            return true
                && (b == other.b);
        }

        bool operator!=(const Foo& other) const
        {
            return !(*this == other);
        }

        void swap(Foo& other)
        {
            using std::swap;
            swap(b, other.b);
        }

        struct Schema;

    protected:
        void InitMetadata(const char*, const char*)
        {
        }
    };

    inline void swap(::tests::Foo& left, ::tests::Foo& right)
    {
        left.swap(right);
    }
} // namespace tests

