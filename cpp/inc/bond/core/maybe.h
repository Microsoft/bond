// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <bond/core/config.h>
#include <bond/core/traits.h>

#include <boost/none.hpp>
#include <boost/optional/optional.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace bond
{


BOND_NORETURN void NothingException();

namespace detail
{

/// Internal base class with shared implementation between the two maybe
/// variants. Consult the documentation for bond::maybe<T> for its public
/// interface.
template <typename T>
class maybe_common
{
public:
    /// @brief The type of the value that may be inside the maybe.
    using value_type = T;

    /// @brief Check if this object contains nothing.
    /// @return true if this holds nothing; otherwise false.
    bool is_nothing() const BOND_NOEXCEPT
    {
        return !static_cast<bool>(_value);
    }

    /// @brief Check if this object contains a value.
    /// @return true if this object hold a value; otherwise false.
    /// @since 8.0.0
    explicit operator bool() const BOND_NOEXCEPT
    {
        return !is_nothing();
    }

    /// @brief Set to nothing.
    void set_nothing() BOND_NOEXCEPT
    {
        // asigning boost::none is noexcept, but assigning { } is not
        _value = boost::none;
    }

    /// @brief Construct a value in place.
    ///
    /// @since 8.0.0
    template<typename... Args>
    T& emplace(Args&&... args)
    {
        _value.emplace(std::forward<Args>(args)...);
        return *_value;
    }

    /// @brief Get a reference to the value.
    /// @throw NothingException if the object contains nothing
    T& value()
    {
        if (is_nothing())
        {
            NothingException();
        }

        return *_value;
    }

    /// @brief Get a constant reference to the value.
    /// @throw NothingException if the object contains nothing
    const T& value() const
    {
        if (is_nothing())
        {
            NothingException();
        }

        return *_value;
    }

    /// @brief Get a reference to the value.
    ///
    /// Will never throw, but has undefined behavior if the object contains
    /// nothing.
    ///
    /// @since 8.0.0
    T& value(const std::nothrow_t&) BOND_NOEXCEPT
    {
        BOOST_ASSERT(!is_nothing());
        return *_value;
    }

    /// @brief Get a constant reference to the value.
    ///
    /// Will never throw, but has undefined behavior if the object contains
    /// nothing.
    ///
    /// @since 8.0.0
    const T& value(const std::nothrow_t&) const BOND_NOEXCEPT
    {
        BOOST_ASSERT(!is_nothing());
        return *_value;
    }

    /// @brief Compares two maybes for value equality.
    ///
    /// @return true if both maybes hold nothing; returns false if one maybe
    /// holds nothing and the other holds a values; otherwise, calls
    /// operator== with the two values.
    ///
    /// @since 8.0.0
    friend bool operator==(const maybe_common& lhs, const maybe_common& rhs)
    {
        return lhs._value == rhs._value;
    }

    /// @brief Compares two maybes for value inequality.
    ///
    /// See operator==(const maybe_common&,const maybe_common&) for details
    /// about how maybes holding nothing are handled.
    ///
    /// @since 8.0.0
    friend bool operator!=(const maybe_common& lhs, const maybe_common& rhs)
    {
        return !(lhs == rhs);
    }

    /// @brief Compares a maybe and a value for equality.
    ///
    /// @return false if the maybe holds nothing; otherwise, calls
    /// operator== with the maybe's value and the provided value.
    ///
    /// @since 8.0.0
    friend bool operator==(const maybe_common& lhs, const T& rhs)
    {
        return lhs._value == rhs;
    }

    /// @brief Compares a maybe and a value for inequality.
    ///
    /// See operator==(const maybe_common&,const T&) for details about how
    /// maybes holding nothing are handled.
    ///
    /// @since 8.0.0
    friend bool operator!=(const maybe_common& lhs, const T& rhs)
    {
        return !(lhs == rhs);
    }

    /// @brief Compares a value and a maybe for equality.
    ///
    /// @return false if the maybe holds nothing; otherwise, calls
    /// operator== with the provided value and maybe's value.
    ///
    /// @since 8.0.0
    friend bool operator==(const T& lhs, const maybe_common& rhs)
    {
        return lhs == rhs._value;
    }

    /// @brief Compares and a value and a maybe for inequality.
    ///
    /// See operator==(const T&,const maybe_common&) for details about how
    /// maybes holding nothing are handled.
    ///
    /// @since 8.0.0
    friend bool operator!=(const T& lhs, const maybe_common& rhs)
    {
        return !(lhs == rhs);
    }

protected:
    maybe_common() = default;

    #if defined(_MSC_VER) && _MSC_VER < 1900
    // Using = default with MSVC 2013 on this function causes the compiler
    // to make extra copies, which makes this type fail when it's holding a
    // move-only type.
    //
    // Manually implementing this function works just fine, however.
    maybe_common(const maybe_common& that)
        : _value(that._value)
    { }
    #else
    maybe_common(const maybe_common&) = default;
    #endif

    template <typename... Args>
    explicit maybe_common(const T& value, Args&&... args)
        : _value()
    {
        _value.emplace(value, std::forward<Args>(args)...);
    }

    template <typename... Args>
    explicit maybe_common(T&& value, Args&&... args)
        : _value()
    {
        _value.emplace(std::move(value), std::forward<Args>(args)...);
    }

    maybe_common(maybe_common&& that) BOND_NOEXCEPT_IF(std::is_nothrow_move_constructible<boost::optional<T>>::value)
        : _value(std::move(that._value))
    {
        // unlike std::optional/boost::optional, moved-from bond::maybe
        // instances are guaranteed to be nothing.
        //
        // asigning boost::none is noexcept, but assigning { } is not
        that._value = boost::none;
    }

    #if defined(_MSC_VER) && _MSC_VER < 1900
    // Using = default with MSVC 2013 on this function causes the compiler
    // to make extra copies, which makes this type fail when it's holding a
    // move-only type.
    //
    // Manually implementing this function works just fine, however.
    maybe_common& operator=(const maybe_common& that)
    {
        _value = that._value;
        return *this;
    }

    // MSVC 2013 cannot = default rvalue asignment operators
    maybe_common& operator=(maybe_common&& that) BOND_NOEXCEPT_IF(BOND_NOEXCEPT_EXPR(_value = std::move(that._value)))
    {
        _value = std::move(that._value);

        // unlike std::optional/boost::optional, moved-from bond::maybe
        // instances are guaranteed to be nothing.
        //
        // asigning boost::none is noexcept, but assigning { } is not
        that._value = boost::none;

        return *this;
    }
    #else
    maybe_common& operator=(const maybe_common&) = default;
    maybe_common& operator=(maybe_common&&) = default;
    #endif

    maybe_common& assign(const T& value)
    {
        emplace(value);
        return *this;
    }

    maybe_common& assign(T&& value)
    {
        emplace(std::move(value));
        return *this;
    }

    boost::optional<T> _value;
};

} // namespace detail

template <typename T, typename Enabled = void>
class maybe;

/// @brief Type used for fields with default value of 'nothing'
///
/// This specialization is used for instance of T without allocators.
///
/// See [User's Manual](../../manual/bond_cpp.html#default-value-of-nothing)
template <typename T>
class maybe<T, typename boost::disable_if<detail::has_allocator<T>>::type>
    : public detail::maybe_common<T>
{
public:
    /// @brief Create a maybe that holds nothing.
    maybe() = default;

    #if defined(_MSC_VER) && _MSC_VER < 1900
    // Using = default with MSVC 2013 on this function causes the compiler
    // to make extra copies, which makes this type fail when it's holding a
    // move-only type.
    //
    // Manually implementing this function works just fine, however.
    maybe(const maybe& that)
        : detail::maybe_common<T>(that)
    { }

    // MSVC 2013 cannot = default ctors
    maybe(maybe&& that) BOND_NOEXCEPT_IF(std::is_nothrow_move_constructible<typename detail::maybe_common<T>>::value)
        : detail::maybe_common<T>(std::move(that))
    { }
    #else
    maybe(const maybe&) = default;
    maybe(maybe&&) = default;
    #endif

    /// @brief Create a maybe that holds a value by copying the value.
    explicit
    maybe(const T& value)
        : detail::maybe_common<T>(value)
    { }

    /// @brief Create a maybe that holds a value by moving from the value.
    ///
    /// @since 8.0.0
    explicit
    maybe(T&& value)
        : detail::maybe_common<T>(std::move(value))
    { }

    #if defined(_MSC_VER) && _MSC_VER < 1900
    // Using = default with MSVC 2013 on this function causes the compiler
    // to make extra copies, which makes this type fail when it's holding a
    // move-only type.
    //
    // Manually implementing this function works just fine, however.
    maybe& operator=(const maybe& that)
    {
        detail::maybe_common<T>::operator=(that);
        return *this;
    }

    // MSVC 2013 cannot = default rvalue asignment operators
    maybe& operator=(maybe&& that) BOND_NOEXCEPT_IF(BOND_NOEXCEPT_EXPR(detail::maybe_common<T>::operator=(std::move(that))))
    {
        detail::maybe_common<T>::operator=(std::move(that));
        return *this;
    }
    #else
    maybe& operator=(const maybe&) = default;
    maybe& operator=(maybe&&) = default;
    #endif

    /// @brief Assign by copying the value.
    maybe& operator=(const T& value)
    {
        this->assign(value);
        return *this;
    }

    /// @brief Move-assign from value.
    /// @since 8.0.0
    maybe& operator=(T&& value)
    {
        this->assign(std::move(value));
        return *this;
    }

    /// @brief Set the maybe to hold a value, if needed.
    ///
    /// If this instance contains nothing, construct a default instance of
    /// T; otherwise, preserve the existing value.
    ///
    /// @return A reference to the value.
    T& set_value()
    {
        if (this->is_nothing())
        {
            this->emplace();
        }

        return *this->_value;
    }

    /// @brief Swap this object with that object
    void swap(maybe& that)
    {
        using std::swap;
        swap(this->_value, that._value);
    }
};

/// @brief Type used for fields with default value of 'nothing'
///
/// This specialization is used for instance of T with allocators.
///
/// See [User's Manual](../../manual/bond_cpp.html#default-value-of-nothing)
template <typename T>
class maybe<T, typename boost::enable_if<detail::has_allocator<T>>::type>
    : public detail::maybe_common<T>,
      private detail::allocator_type<T>::type
{
public:
    using allocator_type = typename detail::allocator_type<T>::type;

    #if defined(_MSC_VER) && _MSC_VER < 1900
    // = default fails on MSVC 2013 when the allocator is not default
    // constructible.
    maybe() { }

    // Using = default with MSVC 2013 on this function causes the compiler
    // to make extra copies, which makes this type fail when it's holding a
    // move-only type.
    //
    // Manually implementing this function works just fine, however.
    maybe(const maybe& that)
        : detail::maybe_common<T>(that),
          allocator_type(that)
        { }

    // MSVC 2013 cannot = default rvalue ctors
    maybe(maybe&& that) BOND_NOEXCEPT_IF(
           std::is_nothrow_move_constructible<typename detail::maybe_common<T>>::value
        && std::is_nothrow_move_constructible<allocator_type>::value)
        : detail::maybe_common<T>(std::move(that.base())),
          allocator_type(std::move(that.allocator()))
    { }
    #else
    /// @brief Create a  maybe that holds nothing.
    maybe() = default;
    maybe(const maybe&) = default;
    maybe(maybe&&) = default;
    #endif

    maybe(const maybe& that, const allocator_type& alloc)
        : detail::maybe_common<T>(),
          allocator_type(alloc)
    {
        if (!that.is_nothing())
        {
            this->emplace(*that._value, alloc);
        }
    }

    maybe(maybe&& that, const allocator_type& alloc)
        : detail::maybe_common<T>(),
          allocator_type(alloc)
    {
        if (!that.is_nothing())
        {
            this->emplace(std::move(*that._value), alloc);
        }

        // unlike std::optional/boost::optional, moved-from bond::maybe
        // instances are guaranteed to be nothing.
        //
        // asigning boost::none is noexcept, but assigning { } is not
        that._value = boost::none;
    }

    /// @brief Construct a maybe that holds nothing, but remember the
    /// allocator so that it can be used to construct a T if needed.
    ///
    /// @since 8.0.0
    explicit maybe(const allocator_type& alloc)
        : detail::maybe_common<T>(),
          allocator_type(alloc)
    { }

    /// @brief Create a maybe that holds a copy of the value.
    explicit maybe(const T& value)
        : detail::maybe_common<T>(value),
          allocator_type()
    { }

    /// @since 8.0.0
    maybe(const T& value, const allocator_type& alloc)
        : detail::maybe_common<T>(),
          allocator_type(alloc)
    {
        this->emplace(value, alloc);
    }

    /// @brief Create a non-empty maybe by moving from the value.
    ///
    /// @since 8.0.0
    maybe(T&& value)
        : detail::maybe_common<T>(std::move(value)),
          allocator_type()
    { }

    /// @since 8.0.0
    maybe(T&& value, const allocator_type& alloc)
        : detail::maybe_common<T>(),
          allocator_type(alloc)
    {
        this->emplace(std::move(value), alloc);
    }

    #if defined(_MSC_VER) && _MSC_VER < 1900
    // Using = default with MSVC 2013 on this function causes the compiler
    // to make extra copies, which makes this type fail when it's holding a
    // move-only type.
    //
    // Manually implementing this function works just fine, however.
    maybe& operator=(const maybe& that)
    {
        detail::maybe_common<T>::operator=(that);
        allocator_type::operator=(that);
        return *this;
    }

    // MSVC 2013 cannot = default rvalue ctors
    maybe& operator=(maybe&& that) BOND_NOEXCEPT_IF(
           BOND_NOEXCEPT_EXPR(detail::maybe_common<T>::operator=(std::move(that.base())))
        && BOND_NOEXCEPT_EXPR(allocator_type::operator=(std::move(that.allocator()))))
    {
        detail::maybe_common<T>::operator=(std::move(that.base()));
        allocator_type::operator=(std::move(that.allocator()));
        return *this;
    }
    #else
    maybe& operator=(const maybe&) = default;
    maybe& operator=(maybe&&) = default;
    #endif

    /// @brief Assign by copying the value.
    maybe& operator=(const T& value)
    {
        this->assign(value);
        return *this;
    }

    /// @brief Move-assign from value.
    /// @since 8.0.0
    maybe& operator=(T&& value)
    {
        this->assign(std::move(value));
        return *this;
    }

    // We need to get rid of any operator== that may come from the allocator
    // so the friend free functions from maybe_common don't have any
    // competition.
    bool operator==(const allocator_type&) = delete;

    /// @brief Set to non-empty, if needed.
    ///
    /// If this object contains nothing, construct an instance of T, passing
    /// it the saved allocator; otherwise, preserve the existing value.
    ///
    /// @return A reference to the value.
    T& set_value()
    {
        if (this->is_nothing())
        {
            this->emplace(allocator());
        }

        return *this->_value;
    }

    /// @brief Swap this object with that object
    void swap(maybe& that)
    {
        using std::swap;
        swap(this->_value, that._value);
        swap(allocator(), that.allocator());
    }

    /// @brief Get the allocator that this maybe uses.
    /// @since 8.0.0
    allocator_type get_allocator() const BOND_NOEXCEPT
    {
        return allocator();
    }

private:
    detail::maybe_common<T>& base() BOND_NOEXCEPT { return *this; }
    const detail::maybe_common<T>& base() const BOND_NOEXCEPT { return *this; }
    allocator_type& allocator() BOND_NOEXCEPT { return *this; }
    const allocator_type& allocator() const BOND_NOEXCEPT { return *this; }
};

template<typename T>
inline void swap(maybe<T>& x, maybe<T>& y)
{
    x.swap(y);
}

} // namespace bond
