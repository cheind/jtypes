/**
    This file is part of jtypes.

    Copyright(C) 2016 Christoph Heindl
    All rights reserved.

    This software may be modified and distributed under the terms
    of MIT license. See the LICENSE file for details.
*/

#ifndef JTYPES_H
#define JTYPES_H

#include "variant.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <stdexcept>

#include <iostream>


namespace jtypes {

    using mapbox::util::variant;
    
    class var;
    
    typedef std::nullptr_t null_t;
    typedef bool bool_t;
    typedef std::string string_t;
    typedef std::vector<var> array_t;
    typedef std::unordered_map<std::string, var> object_t;
    typedef int64_t sint_t;
    typedef uint64_t uint_t;
    typedef double real_t;
    typedef variant<int64_t, uint64_t, double> number_t;
    
    enum class type {
        null = 0,
        boolean,
        number,
        string,
        array,
        object
    };
    
    enum class number_type {
        signed_integral,
        unsigned_integral,
        real
    };
    
    class bad_access : public std::logic_error
    {
        
    public:
        explicit bad_access(const std::string& what_arg)
        : logic_error(what_arg) {}
        
        explicit bad_access(const char* what_arg)
        : logic_error(what_arg) {}
        
    };
    
    namespace detail {
        template <typename _Tp> struct is_jtypes_type  : public std::false_type {};
        template <>          struct is_jtypes_type<null_t>  : public std::true_type {};
        template <>          struct is_jtypes_type<bool_t>  : public std::true_type {};
        template <>          struct is_jtypes_type<string_t>  : public std::true_type {};
        template <>          struct is_jtypes_type<array_t>  : public std::true_type {};
        template <>          struct is_jtypes_type<object_t>  : public std::true_type {};
        template <>          struct is_jtypes_type<number_t>  : public std::true_type {};
        
        
        template <typename _Tp> struct jtype_to_enum {};
        template <>          struct jtype_to_enum<null_t> : public std::integral_constant<type, type::null> {};
        template <>          struct jtype_to_enum<bool_t> : public std::integral_constant<type, type::boolean> {};
        template <>          struct jtype_to_enum<string_t> : public std::integral_constant<type, type::string> {};
        template <>          struct jtype_to_enum<array_t> : public std::integral_constant<type, type::array> {};
        template <>          struct jtype_to_enum<object_t> : public std::integral_constant<type, type::object> {};
        template <>          struct jtype_to_enum<number_t> : public std::integral_constant<type, type::number> {};
        
        template<typename ToType, typename EnableIfType = void>
        struct coerce_number
        {};
        
        template<>
        struct coerce_number<bool, void>
        {
            bool value = false;
            
            template<class T>
            void operator()(T v) { value = (v != T(0)); }
        };
        
        template<typename ToType>
        struct coerce_number<ToType, void>
        {
            ToType value = ToType(0);
            
            template<class T>
            void operator()(T v) { value = ToType(v); }
        };
        
        
        template<typename ToType, typename EnableIfType = void>
        struct coerce
        {};
        
        template<>
        struct coerce<bool, void>
        {
            bool value = false;
            
            void operator()(const null_t &v)  { value = false; }
            void operator()(const bool_t &v)  { value = v; }
            void operator()(const string_t &v)  { value = !v.empty(); }
            void operator()(const array_t &v)  { value = true; }
            void operator()(const object_t &v)  { value = true; }
            void operator()(const number_t &v)  {
                coerce_number<bool> cn;
                apply_visitor(cn, v);
                value = cn.value;
            }
        };
        
        template<typename ToType>
        struct coerce<ToType, typename std::enable_if<std::is_integral<ToType>::value>::type >
        {
            ToType value = ToType(0);
            
            void operator()(const bool_t &v)  { value = v ? ToType(1) : ToType(0); }
            void operator()(const string_t &v)  {
                try {
                    value = ToType(std::stol(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to integral type.");
                }
            }
            
            void operator()(const number_t &v)  {
                coerce_number<ToType> cn;
                apply_visitor(cn, v);
                value = cn.value;
            }
            
            template<class T>
            void operator()(const T &t) {
                throw bad_access("Failed to coerce type to integral type.");
            }
        };
        
        template<typename ToType>
        struct coerce<ToType, typename std::enable_if<std::is_floating_point<ToType>::value>::type >
        {
            ToType value = ToType(0);
            
            void operator()(const bool_t &v)  { value = v ? ToType(1) : ToType(0); }
            void operator()(const string_t &v)  {
                try {
                    value = ToType(std::stod(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to floating point type.");
                }
            }
            
            void operator()(const number_t &v)  {
                coerce_number<ToType> cn;
                apply_visitor(cn, v);
                value = cn.value;
            }
            
            template<class T>
            void operator()(const T &t) {
                throw bad_access("Failed to coerce type to integral type.");
            }
        };
    }
    

    
    class var
    {
    public:
        
        // Value initializers
        
        var()
        : _value(nullptr) {}
        
        var(std::nullptr_t)
        : _value(nullptr) {}
        
        var(bool v)
        : _value(v) {}
        
        
        template<typename I>
        var(I t, typename std::enable_if<std::is_integral<I>::value && std::is_signed<I>::value>::type* unused = 0)
        : _value(number_t(static_cast<int64_t>(t)))
        {}
        
        template<typename I>
        var(I t, typename std::enable_if<std::is_integral<I>::value && std::is_unsigned<I>::value>::type* unused = 0)
        : _value(number_t(static_cast<uint64_t>(t)))
        {}
        
        template<typename I>
        var(I t, typename std::enable_if<std::is_floating_point<I>::value>::type* unused = 0)
        : _value(number_t(static_cast<double>(t)))
        {}
        
        var(double v)
        : _value(number_t(v))
        {}
        
        var(const char* v)
        : _value(std::string(v))
        {}
        
        var(const std::string &v)
        : _value(v)
        {}
        
        var(std::string &&v)
        : _value(std::move(v))
        {}
        
        // Array initializers
        
        template<typename T>
        var(std::initializer_list<T> v)
        {
            array_t a;
            for (auto && t : v) {
                a.emplace_back(var(t));
            }
            _value = std::move(a);
        }
        
        template<typename T>
        var(const std::vector<T> &v)
        {
            array_t a;
            for (auto && t : v) {
                a.emplace_back(var(t));
            }
            _value = std::move(a);
        }
        
        // Dictionary initializers
        
        var(std::initializer_list< std::pair<string_t, var> > v)
        {
            object_t o;
            for (auto &&t : v) {
                o.insert(object_t::value_type(t.first, t.second));
            }
            _value = std::move(o);
        }
        
        // Type queries
        type get_type() const { return (type)_value.which(); }
        
        
        bool_t is_null() const { return _value.which() == (int)type::null; }
        bool_t is_boolean() const { return _value.which() == (int)type::boolean; }
        bool_t is_number() const { return _value.which() == (int)type::number; }
        bool_t is_string() const { return _value.which() == (int)type::string; }
        bool_t is_array() const { return _value.which() == (int)type::array; }
        bool_t is_object() const { return _value.which() == (int)type::object; }
        
        bool_t is_signed_integral() const { return is_number() && _value.get<number_t>().which() == (int)number_type::signed_integral; }
        bool_t is_unsigned_integral() const { return is_number() && _value.get<number_t>().which() == (int)number_type::unsigned_integral; }
        bool_t is_real() const { return is_number() && _value.get<number_t>().which() == (int)number_type::real; }
        
        // Getters with coercion
        
        explicit operator bool() const {
            return as<bool>();
        }
        
        template<class T>
        T as() const {
            detail::coerce<T> visitor;
            apply_visitor(visitor, _value);
            return visitor.value;
        }
        
        
        /*
        template<class T>
        const typename std::enable_if<detail::is_jtypes_type<T>::value, T>::type &
        get() const {
            if (_value.which() == (int)detail::jtype_to_enum<T>::value) {
                return _value.get<T>();
            } else {
                throw bad_access("The value held by var does not correspond to requested type in var::get().");
            }
            
        }*/
        
        
        
    private:
        typedef variant<null_t, bool_t, number_t, string_t, array_t, object_t> oneof;
        
        oneof _value;
    };
    

}

#endif
