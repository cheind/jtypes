/**
    This file is part of jtypes.

    Copyright(C) 2016 Christoph Heindl
    All rights reserved.

    This software may be modified and distributed under the terms
    of MIT license. See the LICENSE file for details.
*/

#ifndef JTYPES_H
#define JTYPES_H

#if defined(WIN32) || defined(_WIN32)
#pragma warning(push)
#pragma warning(disable: 4996) 
#endif

#include "variant.hpp"

#if defined(WIN32) || defined(_WIN32)
#pragma warning(pop) 
#endif

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <functional>

#include <iostream>


namespace jtypes {

    using mapbox::util::variant;

    class var;
    
    namespace functions {
        struct function_holder;
    }
   
    typedef int64_t sint_t;
    typedef uint64_t uint_t;
    typedef double real_t;

    struct undefined_t {
        bool operator==(const undefined_t &rhs) const { return false; }
        bool operator<(const undefined_t &rhs) const { return false; }
    };
    typedef std::nullptr_t null_t;
    typedef bool bool_t;
    typedef variant<int64_t, uint64_t, double> number_t;
    typedef std::string string_t;
    typedef functions::function_holder function_t;
    typedef std::vector<var> array_t;
    typedef std::unordered_map<std::string, var> object_t;
    
    
    bool operator<(const object_t &o1, const object_t &o2) { return false; }
    bool operator==(const object_t &o1, const object_t &o2) { return false; }



    enum class type {
        undefined = 0,
        null,
        boolean,
        number,
        string,
        function,
        array,
        object
    };

    enum class number_type {
        signed_integral,
        unsigned_integral,
        real
    };
    
    class bad_access : public std::logic_error {
        
    public:
        explicit bad_access(const std::string& what_arg)
        : logic_error(what_arg) {
        }
        
        explicit bad_access(const char* what_arg)
        : logic_error(what_arg) {
        }
        
    };
    
    namespace details {
        template<typename T>
        using EnableIfSignedIntegralPolicy = typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>::type;
        
        template<typename T>
        using EnableIfUnsignedIntegralPolicy = typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value>::type;
        
        template<typename T>
        using EnableIfFloatingPointPolicy = typename std::enable_if<std::is_floating_point<T>::value>::type;
    }
    
    namespace functions {
        
        //plain function pointers
        template<typename... Args, typename ReturnType>
        auto fnc(ReturnType(*p)(Args...))
        -> std::function<ReturnType(Args...)> {
            return{ p };
        }
        
        //nonconst member function pointers
        template<typename... Args, typename ReturnType, typename ClassType>
        auto fnc(ReturnType(ClassType::*p)(Args...))
        -> std::function<ReturnType(Args...)> {
            return{ p };
        }
        
        //const member function pointers
        template<typename... Args, typename ReturnType, typename ClassType>
        auto fnc(ReturnType(ClassType::*p)(Args...) const)
        -> std::function<ReturnType(Args...)> {
            return{ p };
        }
        
        //qualified functionoids
        template<typename FirstArg, typename... Args, class T>
        auto fnc(T&& t)
        -> std::function<decltype(t(std::declval<FirstArg>(), std::declval<Args>()...))(FirstArg, Args...)> {
            return{ std::forward<T>(t) };
        }
        
        //unqualified functionoids try to deduce the signature of `T::operator()` and use that.
        template<class T>
        auto fnc(T&& t)
        -> decltype(::jtypes::functions::fnc(&std::remove_reference<T>::type::operator())) {
            return{ std::forward<T>(t) };
        }
        
        struct function_wrapper_base {
            virtual ~function_wrapper_base() = default;
            virtual bool empty() const = 0;
        };
        
        template<typename ReturnType, typename... Args>
        struct function_wrapper : function_wrapper_base {
            std::function<ReturnType(Args...)> f;
            
            function_wrapper(const std::function<ReturnType(Args...)> &f_)
            :f(f_) {
            }
            
            bool empty() const override {
                return !(bool)f;
            }
        };
        
        struct function_holder {
            std::shared_ptr<function_wrapper_base> ptr;
            
            template<typename ReturnType, typename... Args>
            function_holder(const std::function<ReturnType(Args...)> &f)
            :ptr(new function_wrapper<ReturnType, Args...>(f)) {
            }
            
            template<typename ReturnType, typename... Args>
            ReturnType invoke(Args&&... args) const {
                function_wrapper<ReturnType, typename std::decay<Args>::type...> * g =
                dynamic_cast< function_wrapper<ReturnType, typename std::decay<Args>::type...> *>(ptr.get());
                
                if (g == nullptr) {
                    throw bad_access("Failed function to signature requested by parameters.");;
                }
                
                return g->f(std::forward<Args>(args)...);
            }
            
            bool empty() const {
                return !ptr || ptr->empty();
            }
            
            bool operator==(const function_holder &rhs) const {
                return false;
            }
            
            bool operator<(const function_holder &rhs) const {
                return false;
            }
            
        };
    }

    
    class var {
    public:
        
        // Value initializers
        
        var();
        
        
        var(std::nullptr_t);
        
        var(bool v);
        
        
        template<typename I>
        var(I t, typename details::EnableIfSignedIntegralPolicy<I>* unused = 0);
        
        template<typename I>
        var(I t, typename details::EnableIfUnsignedIntegralPolicy<I>* unused = 0);
        
        template<typename I>
        var(I t, typename details::EnableIfFloatingPointPolicy<I>* unused = 0);
        
        var(double v);
        
        var(char);
        
        var(const char* v);
        
        var(const string_t &v);
        
        var(std::string &&v);
        
        template<typename... Args, typename ReturnType>
        var(std::function<ReturnType(Args...)> v);
        
        
        // Array initializers
        
        template<typename T>
        var(std::initializer_list<T> v);
        
        template<typename T>
        var(const std::vector<T> &v);
        
        var(const array_t &v);
        
        // Dictionary initializers
        
        var(std::initializer_list< std::pair<string_t, var> > v);
        
        // Type queries
        type get_type() const;
        bool_t is_undefined() const;
        bool_t is_null() const;
        bool_t is_boolean() const;
        bool_t is_number() const;
        bool_t is_string() const;
        bool_t is_function() const;
        bool_t is_array() const;
        bool_t is_object() const;
        bool_t is_signed_integral() const;
        bool_t is_unsigned_integral() const;
        bool_t is_real() const;
        
        // Getters with coercion
        
        explicit operator bool() const;
        
        template<class T>
        T as() const;
        
        // Callable interface
        
        template<typename ReturnType, typename... Args>
        ReturnType invoke(Args&&... args) const;
        
        // Array / Object accessors
        
        var &operator[](const var &key);
        const var &operator[](const var &key) const;
        
        // Keys accessor
        var keys() const;
        var values() const;
        
        // Array accessors
        
        array_t::iterator begin();
        array_t::iterator end();
        
        array_t::const_iterator begin() const;
        array_t::const_iterator end() const;
        
        // Comparison interface
        
        bool operator==(var const& rhs) const;
        bool operator!=(var const& rhs) const;
        bool operator<(var const& rhs) const;
        bool operator>(var const& rhs) const;
        bool operator<=(var const& rhs) const;
        bool operator>=(var const &rhs) const;
        
        
    private:
        typedef variant<
        undefined_t,
        null_t,
        bool_t,
        number_t,
        string_t,
        function_t,
        array_t,
        object_t
        > oneof;
        
        template<class T>
        T &get_variant_or_convert();
        
        
        static const var &undefined_var();
        
        oneof _value;
    };

    
    namespace details {
        template <typename _Tp> struct is_type : public std::false_type {};
        template <>          struct is_type<undefined_t> : public std::true_type {};
        template <>          struct is_type<null_t> : public std::true_type {};
        template <>          struct is_type<bool_t> : public std::true_type {};
        template <>          struct is_type<string_t> : public std::true_type {};
        template <>          struct is_type<function_t> : public std::true_type {};
        template <>          struct is_type<array_t> : public std::true_type {};
        template <>          struct is_type<object_t> : public std::true_type {};
        template <>          struct is_type<number_t> : public std::true_type {};


        template <typename _Tp> struct enum_of {};
        template <>          struct enum_of<undefined_t> : public std::integral_constant<type, type::undefined> {};
        template <>          struct enum_of<null_t> : public std::integral_constant<type, type::null> {};
        template <>          struct enum_of<bool_t> : public std::integral_constant<type, type::boolean> {};
        template <>          struct enum_of<string_t> : public std::integral_constant<type, type::string> {};
        template <>          struct enum_of<function_t> : public std::integral_constant<type, type::function> {};
        template <>          struct enum_of<array_t> : public std::integral_constant<type, type::array> {};
        template <>          struct enum_of<object_t> : public std::integral_constant<type, type::object> {};
        template <>          struct enum_of<number_t> : public std::integral_constant<type, type::number> {};
        
        template<typename Range, typename Transform>
        inline std::string
        join(const Range &input, const std::string& separator, Transform trans)
        {
            auto it = std::begin(input);
            auto it_end = std::end(input);
            
            std::ostringstream result;
            
            if(it != it_end)
            {
                result << trans(*it);
                ++it;
            }
            
            for(;it != it_end; ++it)
            {
                result << separator << trans(*it);
            }
            
            return result.str();
        }
        
        template< typename Range>
        inline std::string
        join(const Range &input, const std::string& separator)
        {
            typedef decltype(std::begin(input)) range_value;
            return join(input, separator, [](const range_value &v) {return v;});
        }
        
        template<typename ToType, typename EnableIfType = void>
        struct coerce_number {
        };

        template<>
        struct coerce_number<bool, void> {
            bool value = false;

            template<class T>
            void operator()(T v) { value = (v != T(0)); }
        };
        
        template<>
        struct coerce_number<std::string, void> {
            std::string value;
            
            template<class T>
            void operator()(T v) { value = std::to_string(v); }
        };

        template<typename ToType>
        struct coerce_number<ToType, void> {
            ToType value = ToType(0);

            template<class T>
            void operator()(T v) { value = ToType(v); }
        };


        template<typename ToType, typename EnableIfType = void>
        struct coerce {
        };

        template<>
        struct coerce<bool, void> {
            bool value = false;

            void operator()(const undefined_t &v) { value = false; }
            void operator()(const null_t &v) { value = false; }
            void operator()(const bool_t &v) { value = v; }
            void operator()(const string_t &v) { value = !v.empty(); }
            void operator()(const function_t &v) { value = !v.empty(); }
            void operator()(const array_t &v) { value = true; }
            void operator()(const object_t &v) { value = true; }
            void operator()(const number_t &v) {
                coerce_number<bool> cn;
                apply_visitor(cn, v);
                value = cn.value;
            }
        };
        
        template<>
        struct coerce<std::string, void> {
            std::string value;
            
            void operator()(const undefined_t &v) { value = "undefined"; }
            void operator()(const null_t &v) { value = "null"; }
            void operator()(const bool_t &v) { value = v ? "true" : "false"; }
            void operator()(const string_t &v) { value = v; }
            void operator()(const function_t &v) { value = "function"; }
            void operator()(const array_t &v) {
                std::ostringstream oss;
                oss << '[';
                oss << join(v, ",", [](const var &vv) { return vv.as<std::string>();});
                oss << ']';
                value = oss.str();
            }
            void operator()(const object_t &v) {
                value = "";
            }
            
            void operator()(const number_t &v) {
                coerce_number<std::string> cn;
                apply_visitor(cn, v);
                value = cn.value;
            }
        };

        template<typename ToType>
        struct coerce<ToType, typename std::enable_if<std::is_integral<ToType>::value>::type > {
            ToType value = ToType(0);

            void operator()(const bool_t &v) { value = v ? ToType(1) : ToType(0); }
            void operator()(const string_t &v) {
                try {
                    value = ToType(std::stol(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to integral type.");
                }
            }

            void operator()(const number_t &v) {
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
        struct coerce<ToType, typename std::enable_if<std::is_floating_point<ToType>::value>::type > {
            ToType value = ToType(0);

            void operator()(const bool_t &v) { value = v ? ToType(1) : ToType(0); }
            void operator()(const string_t &v) {
                try {
                    value = ToType(std::stod(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to floating point type.");
                }
            }

            void operator()(const number_t &v) {
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

    // Implementation
    
    
    inline var::var()
    : _value(undefined_t()) {
    }
    
    inline var::var(std::nullptr_t)
    : _value(nullptr) {
    }
    
    inline var::var(bool v)
    : _value(v) {
    }
    
    template<typename I>
    inline var::var(I t, typename details::EnableIfSignedIntegralPolicy<I>* unused)
    : _value(number_t(static_cast<int64_t>(t))) {
    }
    
    template<typename I>
    inline var::var(I t, typename details::EnableIfUnsignedIntegralPolicy<I>* unused)
    : _value(number_t(static_cast<uint64_t>(t))) {
    }
    
    template<typename I>
    inline var::var(I t, typename details::EnableIfFloatingPointPolicy<I>* unused)
    : _value(number_t(static_cast<double>(t))) {
    }
    
    inline var::var(double v)
    : _value(number_t(v)) {
    }
    
    inline var::var(const char* v)
    : _value(std::string(v)) {
    }
    
    inline var::var(char v)
    : _value(std::string(&v, 1)) {
    }
    
    
    inline var::var(const std::string &v)
    : _value(v) {
    }
    
    inline var::var(std::string &&v)
    : _value(std::move(v)) {
    }

    
    template<typename... Args, typename ReturnType>
    inline var::var(std::function<ReturnType(Args...)> v)
    : _value(function_t(v)) {
    }

    template<typename T>
    inline var::var(std::initializer_list<T> v) {
        array_t a;
        for (auto && t : v) {
            a.emplace_back(var(t));
        }
        _value = std::move(a);
    }
    
    template<typename T>
    inline var::var(const std::vector<T> &v) {
        array_t a;
        for (auto && t : v) {
            a.emplace_back(var(t));
        }
        _value = std::move(a);
    }
    
    inline var::var(const array_t &v)
    :_value(v)
    {}
    
    inline var::var(std::initializer_list< std::pair<string_t, var> > v) {
        object_t o;
        for (auto &&t : v) {
            o.insert(object_t::value_type(t.first, t.second));
        }
        _value = std::move(o);
    }
    
    inline type var::get_type() const { return (type)_value.which(); }
    
    inline bool_t var::is_undefined() const { return _value.which() == (int)type::undefined; }
    inline bool_t var::is_null() const { return _value.which() == (int)type::null; }
    inline bool_t var::is_boolean() const { return _value.which() == (int)type::boolean; }
    inline bool_t var::is_number() const { return _value.which() == (int)type::number; }
    inline bool_t var::is_string() const { return _value.which() == (int)type::string; }
    inline bool_t var::is_function() const { return _value.which() == (int)type::function; }
    inline bool_t var::is_array() const { return _value.which() == (int)type::array; }
    inline bool_t var::is_object() const { return _value.which() == (int)type::object; }
    
    inline bool_t var::is_signed_integral() const { return is_number() && _value.get<number_t>().which() == (int)number_type::signed_integral; }
    inline bool_t var::is_unsigned_integral() const { return is_number() && _value.get<number_t>().which() == (int)number_type::unsigned_integral; }
    inline bool_t var::is_real() const { return is_number() && _value.get<number_t>().which() == (int)number_type::real; }
    
    inline var::operator bool() const {
        return as<bool>();
    }

    template<class T>
    inline T var::as() const {
        details::coerce<T> visitor;
        apply_visitor(visitor, _value);
        return visitor.value;
    }
    
    template<typename ReturnType, typename... Args>
    inline ReturnType var::invoke(Args&&... args) const
    {
        if (is_function()) {
            const function_t &f = _value.get<function_t>();
            if (!f.empty()) {
                return f.invoke<ReturnType>(std::forward<Args>(args)...);
            }
        }
        
        throw bad_access("Not a function or not callable.");
    }
    
    // Object / Array accessors
    inline var &var::operator[](const var &key) {
        
        if (key.is_number()) {
            
            array_t &a = get_variant_or_convert<array_t>();
            size_t idx = key.as<size_t>();
            if (a.size() < idx + 1) {
                a.resize(idx + 1);
            }
            return a[idx];
        } else if (key.is_string()) {
            // Implicit conversion to object
            object_t &o = get_variant_or_convert<object_t>();
            
            auto iter = o.insert(object_t::value_type(key.as<std::string>(), var()));
            return iter.first->second;
        } else {
            throw bad_access("Key is not number or string.");
        }
        
    }
    
    inline const var &var::operator[](const var &key) const {
        if (key.is_number() && is_array()) {
            const array_t &a = _value.get<array_t>();
            size_t idx = key.as<size_t>();
            if (idx < a.size()) {
                return a[idx];
            } else {
                return undefined_var();
            }
        } else if (key.is_string() && is_object()) {
            const object_t &o = _value.get<object_t>();
            
            auto iter = o.find(key.as<std::string>());
            
            if (iter != o.end()) {
                return iter->second;
            } else {
                return undefined_var();
            }
        } else {
            throw bad_access("Key is not number or string.");
        }
    }
    
    template<class T>
    inline T &var::get_variant_or_convert() {
        if (!_value.is<T>())
            _value = T();
        return _value.get<T>();
    }
    
    
    inline const var &var::undefined_var() {
        static var u;
        return u;
    }

    inline var var::keys() const {
        array_t r;
        
        if (is_object()) {
            const object_t &o = _value.get<object_t>();
            for (auto p : o) {
                r.push_back(var(p.first));
            }
            std::sort(r.begin(), r.end());
        } else if (is_array()) {
            const array_t &a = _value.get<array_t>();
            for (size_t i = 0; i < a.size(); ++i) {
                r.push_back(var(std::to_string(i)));
            }
        }
        
        return var(r);
        
    }
    
    inline var var::values() const {
        array_t r;
        
        if (is_object()) {
            for (auto k : keys()) {
                r.push_back((*this)[k]);
            }
        } else if (is_array()) {
            r = _value.get<array_t>();
        }
        return var(r);
    }
    
    inline array_t::iterator var::begin() {
        if (!is_array())
            throw bad_access("Not an array.");
        return _value.get<array_t>().begin();
    }
    
    inline array_t::iterator var::end() {
        if (!is_array())
            throw bad_access("Not an array.");
        return _value.get<array_t>().end();
    }
    
    inline array_t::const_iterator var::begin() const {
        if (!is_array())
            throw bad_access("Not an array.");
        return _value.get<array_t>().begin();
    }
    
    inline array_t::const_iterator var::end() const {
        if (!is_array())
            throw bad_access("Not an array.");
        return _value.get<array_t>().end();
    }
    
    inline bool var::operator==(var const& rhs) const {
        return _value == rhs._value;
    }
    
    inline bool var::operator!=(var const& rhs) const {
        return !(*this == rhs);
        
    }
    
    inline bool var::operator<(var const& rhs) const {
        return _value < rhs._value;
    }
    
    inline bool var::operator>(var const& rhs) const {
        return rhs < *this;
    }
    
    inline bool var::operator<=(var const& rhs) const {
        return !(*this > rhs);
    }
    
    inline bool var::operator>=(var const &rhs) const {
        return !(*this < rhs);
    }
}

#endif
