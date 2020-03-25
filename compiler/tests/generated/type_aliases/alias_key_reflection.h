
#pragma once

#include "alias_key_types.h"
#include <bond/core/reflection.h>

namespace test
{
    //
    // foo
    //
    struct foo::Schema
    {
        typedef ::bond::no_base base;

        static const ::bond::Metadata metadata;
        
        private: static const ::bond::Metadata s_m_metadata;
        private: static const ::bond::Metadata s_s_metadata;

        public: struct var
        {
            // m
            struct m : ::bond::reflection::FieldTemplate<
                0,
                ::bond::reflection::optional_field_modifier,
                foo,
                std::map< ::test::String, ::test::Int, std::less< ::test::String>, typename std::allocator_traits<arena>::template rebind_alloc<std::pair<const ::test::String, ::test::Int> > >,
                &foo::m,
                &s_m_metadata
            > {};

            struct m m;
        
            // s
            struct s : ::bond::reflection::FieldTemplate<
                1,
                ::bond::reflection::optional_field_modifier,
                foo,
                std::set< ::test::Int, std::less< ::test::Int>, typename std::allocator_traits<arena>::template rebind_alloc< ::test::Int> >,
                &foo::s,
                &s_s_metadata
            > {};

            struct s s;
        };

        private: typedef boost::mpl::list<> fields0;
        private: typedef boost::mpl::push_front<fields0, var::s>::type fields1;
        private: typedef boost::mpl::push_front<fields1, var::m>::type fields2;

        public: typedef fields2::type fields;
        
        
        static ::bond::Metadata GetMetadata()
        {
            return ::bond::reflection::MetadataInit("foo", "test.foo",
                ::bond::reflection::Attributes()
            );
        }
    };
    

    
} // namespace test
