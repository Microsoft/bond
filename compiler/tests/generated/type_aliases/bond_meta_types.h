
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



namespace deprecated
{
namespace bondmeta
{
    
    struct HasMetaFields
    {
        using allocator_type = arena;

        std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<allocator_type>::template rebind_alloc<char> > full_name;
        std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<allocator_type>::template rebind_alloc<char> > name;
        
        struct _bond_vc12_ctor_workaround_ {};
        template <int = 0> // Workaround to avoid compilation if not used
        HasMetaFields(_bond_vc12_ctor_workaround_ = {})
        {
            InitMetadata("HasMetaFields", "deprecated.bondmeta.HasMetaFields");
        }

        HasMetaFields(const HasMetaFields& other)
          : full_name(other.full_name.get_allocator()),
            name(other.name.get_allocator())
        {
            InitMetadata("HasMetaFields", "deprecated.bondmeta.HasMetaFields");
        }
        
        HasMetaFields(HasMetaFields&& other)
          : full_name(std::move(other.full_name)),
            name(std::move(other.name))
        {
            InitMetadata("HasMetaFields", "deprecated.bondmeta.HasMetaFields");
        }
        
        explicit
        HasMetaFields(const allocator_type& allocator)
          : full_name(allocator),
            name(allocator)
        {
            InitMetadata("HasMetaFields", "deprecated.bondmeta.HasMetaFields");
        }
        
        HasMetaFields& operator=(HasMetaFields other)
        {
            other.swap(*this);
            return *this;
        }

        bool operator==(const HasMetaFields&) const
        {
            return true
                /* skip bond_meta::full_name field 'full_name' */
                /* skip bond_meta::name field 'name' */;
        }

        bool operator!=(const HasMetaFields& other) const
        {
            return !(*this == other);
        }

        void swap(HasMetaFields&)
        {
            using std::swap;
            /* skip bond_meta::full_name field 'full_name' */
            /* skip bond_meta::name field 'name' */
        }

        struct Schema;

    protected:
        void InitMetadata(const char*name0, const char*qual_name)
        {
            this->name = name0;
            this->full_name = qual_name;
        }
    };

    inline void swap(::deprecated::bondmeta::HasMetaFields& left, ::deprecated::bondmeta::HasMetaFields& right)
    {
        left.swap(right);
    }
} // namespace bondmeta
} // namespace deprecated
