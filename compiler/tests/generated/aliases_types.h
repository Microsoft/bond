
#pragma once

#include <bond/core/bond_version.h>

#if BOND_VERSION < 0x0700
#error This file was generated by a newer version of the Bond compiler and is incompatible with your version of the Bond library.
#endif

#if BOND_MIN_CODEGEN_VERSION > 0x0b00
#error This file was generated by an older version of the Bond compiler and is incompatible with your version of the Bond library.
#endif

#include <bond/core/config.h>
#include <bond/core/containers.h>



namespace tests
{
    
    template <typename T>
    struct Foo
    {
        std::vector<std::vector<T> > aa;
        
        Foo()
        {
        }

        
        // Compiler generated copy ctor OK
        Foo(const Foo&) = default;
        
#if defined(_MSC_VER) && (_MSC_VER < 1900)  // Versions of MSVC prior to 1900 do not support = default for move ctors
        Foo(Foo&& other)
          : aa(std::move(other.aa))
        {
        }
#else
        Foo(Foo&&) = default;
#endif
        
        
#if defined(_MSC_VER) && (_MSC_VER < 1900)  // Versions of MSVC prior to 1900 do not support = default for move ctors
        Foo& operator=(Foo other)
        {
            other.swap(*this);
            return *this;
        }
#else
        // Compiler generated operator= OK
        Foo& operator=(const Foo&) = default;
        Foo& operator=(Foo&&) = default;
#endif

        bool operator==(const Foo& other) const
        {
            return true
                && (aa == other.aa);
        }

        bool operator!=(const Foo& other) const
        {
            return !(*this == other);
        }

        void swap(Foo& other)
        {
            using std::swap;
            swap(aa, other.aa);
        }

        struct Schema;

    protected:
        void InitMetadata(const char*, const char*)
        {
        }
    };

    template <typename T>
    inline void swap(::tests::Foo<T>& left, ::tests::Foo<T>& right)
    {
        left.swap(right);
    }

    
    namespace _bond_enumerators
    {
    namespace EnumToWrap
    {
        enum EnumToWrap
        {
            anEnumValue
        };
        
        inline const char* GetTypeName(enum EnumToWrap)
        {
            return "EnumToWrap";
        }

        inline const char* GetTypeName(enum EnumToWrap, const ::bond::qualified_name_tag&)
        {
            return "tests.EnumToWrap";
        }

        template <typename Map = std::map<enum EnumToWrap, std::string> >
        inline const Map& GetValueToNameMap(enum EnumToWrap)
        {
            static const Map _value_to_name_EnumToWrap
                {
                    { anEnumValue, "anEnumValue" }
                };
            return _value_to_name_EnumToWrap;
        }

        template <typename Map = std::map<std::string, enum EnumToWrap> >
        inline const Map& GetNameToValueMap(enum EnumToWrap)
        {
            static const Map _name_to_value_EnumToWrap
                {
                    { "anEnumValue", anEnumValue }
                };
            return _name_to_value_EnumToWrap;
        }

        const std::string& ToString(enum EnumToWrap value);

        void FromString(const std::string& name, enum EnumToWrap& value);

        bool ToEnum(enum EnumToWrap& value, const std::string& name);

        bool FromEnum(std::string& name, enum EnumToWrap value);

    } // namespace EnumToWrap
    } // namespace _bond_enumerators

    using namespace _bond_enumerators::EnumToWrap;
    

    
    struct WrappingAnEnum
    {
        ::tests::EnumToWrap aWrappedEnum;
        
        WrappingAnEnum()
          : aWrappedEnum(::tests::_bond_enumerators::EnumToWrap::anEnumValue)
        {
        }

        
        // Compiler generated copy ctor OK
        WrappingAnEnum(const WrappingAnEnum&) = default;
        
#if defined(_MSC_VER) && (_MSC_VER < 1900)  // Versions of MSVC prior to 1900 do not support = default for move ctors
        WrappingAnEnum(WrappingAnEnum&& other)
          : aWrappedEnum(std::move(other.aWrappedEnum))
        {
        }
#else
        WrappingAnEnum(WrappingAnEnum&&) = default;
#endif
        
        
#if defined(_MSC_VER) && (_MSC_VER < 1900)  // Versions of MSVC prior to 1900 do not support = default for move ctors
        WrappingAnEnum& operator=(WrappingAnEnum other)
        {
            other.swap(*this);
            return *this;
        }
#else
        // Compiler generated operator= OK
        WrappingAnEnum& operator=(const WrappingAnEnum&) = default;
        WrappingAnEnum& operator=(WrappingAnEnum&&) = default;
#endif

        bool operator==(const WrappingAnEnum& other) const
        {
            return true
                && (aWrappedEnum == other.aWrappedEnum);
        }

        bool operator!=(const WrappingAnEnum& other) const
        {
            return !(*this == other);
        }

        void swap(WrappingAnEnum& other)
        {
            using std::swap;
            swap(aWrappedEnum, other.aWrappedEnum);
        }

        struct Schema;

    protected:
        void InitMetadata(const char*, const char*)
        {
        }
    };

    inline void swap(::tests::WrappingAnEnum& left, ::tests::WrappingAnEnum& right)
    {
        left.swap(right);
    }
} // namespace tests

