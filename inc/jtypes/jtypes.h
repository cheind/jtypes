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

    struct undefined_t {};
    typedef std::nullptr_t null_t;
    typedef bool bool_t;
    typedef variant<int64_t, uint64_t, double> number_t;
    typedef std::string string_t;
    typedef functions::function_holder function_t;
    typedef std::vector<var> array_t;
    typedef std::unordered_map<std::string, var> object_t;
    
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
        template<typename T, typename R = void>
        using EnableIfSignedIntegralPolicy = typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using EnableIfUnsignedIntegralPolicy = typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using EnableIfFloatingPointPolicy = typename std::enable_if<std::is_floating_point<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using EnableIfNumberType = typename std::enable_if<std::is_same<T, sint_t>::value || std::is_same<T, uint_t>::value || std::is_same<T, real_t>::value, R>::type;
        
        template<typename T, typename R = void>
        using DisableIfNumberType = typename std::enable_if<!std::is_same<T, sint_t>::value && !std::is_same<T, uint_t>::value && !std::is_same<T, real_t>::value, R>::type;
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
        
        var(char v);
        
        var(const char* v);
        
        var(const string_t &v);
        
        var(std::string &&v);
        
        template<typename... Args, typename ReturnType>
        var(const std::function<ReturnType(Args...)> &v);
        
        // Assignments
        
        var &operator=(const var &rhs) = default;
        var &operator=(std::nullptr_t);
        var &operator=(bool rhs);
        var &operator=(char rhs);
        var &operator=(const char* v);
        var &operator=(const string_t &rhs);
        
        template<typename I>
        details::EnableIfSignedIntegralPolicy<I, var&>
        operator=(I t);
        
        template<typename I>
        details::EnableIfUnsignedIntegralPolicy<I, var&>
        operator=(I t);
        
        template<typename I>
        details::EnableIfFloatingPointPolicy<I, var&>
        operator=(I t);

        template<typename... Args, typename ReturnType>
        var &operator=(const std::function<ReturnType(Args...)> &rhs);
        
        template<typename T>
        var& operator=(std::initializer_list<T> rhs);
        
        template<typename T>
        var& operator=(const std::vector<T> &rhs);
        
        var& operator=(const array_t &rhs);
        
        var& operator=(std::initializer_list< std::pair<string_t, var> > rhs);

        
        // Array initializers
        
        template<typename T>
        var(std::initializer_list<T> v);
        
        template<typename T>
        var(const std::vector<T> &v);
        
        var(const array_t &v);

        template<typename T>
        var(T begin, T end, typename std::iterator_traits<T>::iterator_category* = nullptr);        
        
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
        
        template<class T>
        T as(const var &opts) const;
        
        // Callable interface
        
        template<typename ReturnType, typename... Args>
        ReturnType invoke(Args&&... args) const;
        
        // Array / Object accessors
        
        var &operator[](const var &key);
        const var &operator[](const var &key) const;
        
        // This or default value.
        
        const var &operator|(const var &default_value) const;
        
        // Keys accessor
        var keys() const;
        var values() const;
        
        // Array accessors
        
        array_t::iterator begin();
        array_t::iterator end();
        
        array_t::const_iterator begin() const;
        array_t::const_iterator end() const;

        // Array inserters

        void push_back(const var &v);
        
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
        
        template<class Range>
        void initialize_array(const Range &r);

        template<class Iter>
        void initialize_array(Iter begin, Iter end);
        
        template<class Range>
        void initialize_object(const Range &r);
        
        static const var &undefined_var();
        
        oneof _value;
    };

    bool operator<(const object_t &lhs, const object_t &rhs) { return false; }
    bool operator==(const undefined_t &lhs, const undefined_t &rhs) { return true; }
    bool operator<(const undefined_t &lhs, const undefined_t &rhs) { return false; }


    
    namespace details {
        
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

        template<typename NumberType>
        struct coerce {
            
            var opts;
            
            NumberType operator()(const bool_t &v) const { return v ? NumberType(1) : NumberType(0); }
            
            
            template< typename T = NumberType >
            NumberType operator()(const string_t &v, EnableIfSignedIntegralPolicy<T> *unused=0) const {
                try {
                    return NumberType(std::stol(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to integral type.");
                }
            }
            
            template< typename T = NumberType >
            NumberType operator()(const string_t &v, EnableIfUnsignedIntegralPolicy<T> *unused=0) const {
                try {
                    return NumberType(std::stoul(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to integral type.");
                }
            }
            
            template< typename T = NumberType >
            NumberType operator()(const string_t &v, EnableIfFloatingPointPolicy<T> *unused=0) const {
                try {
                    return NumberType(std::stod(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to integral type.");
                }
            }
            
            template<class T>
            NumberType operator()(const T &t, EnableIfNumberType<T> *unused=0) const {
                return NumberType(t); // Static cast needs to be addressed.
            }
            
            NumberType operator()(const number_t &v) const {
                return apply_visitor(*this, v);
            }
            
            template<class T>
            NumberType operator()(const T &t, DisableIfNumberType<T> *unused=0) const {
                throw bad_access("Failed to coerce type to integral type.");
            }
        };

        template<>
        struct coerce<bool> {
            bool operator()(const undefined_t &v) const { return false; }
            bool operator()(const null_t &v) const { return false; }
            bool operator()(const bool_t &v) const { return v; }
            bool operator()(const string_t &v) const { return !v.empty(); }
            bool operator()(const function_t &v) const { return !v.empty(); }
            bool operator()(const array_t &v) const { return true; }
            bool operator()(const object_t &v) const { return true; }
            
            template<class T>
            bool operator()(const T &v, EnableIfNumberType<T> *unused=0) const { return v != T(0); };
            
            bool operator()(const number_t &v) const {
                return apply_visitor(*this, v);
            }
        };
        
        template<>
        struct coerce<string_t> {
            var opts;
           
            string_t operator()(const undefined_t &v) const { return "undefined"; }
            string_t operator()(const null_t &v) const { return "null"; }
            string_t operator()(const bool_t &v) const { return v ? "true" : "false"; }
            string_t operator()(const string_t &v) const { return v; }
            string_t operator()(const function_t &v) const { return "function"; }
            string_t operator()(const array_t &v) const {
                return join(v, ",", [](const var &vv) { return vv.as<std::string>(); });
            }

            string_t operator()(const object_t &v) const { return "object"; }
            
            template<class T>
            string_t operator()(const T &v, EnableIfNumberType<T> *unused=0) const { 
                return std::to_string(v); 
            };

            
            string_t operator()(const number_t &v) const {
                return apply_visitor(*this, v);
            }
        };

        struct equal_numbers {

            bool operator()(sint_t lhs, uint_t rhs) const {
                return (lhs < 0) ? false : (uint_t)lhs == rhs;
            }

            bool operator()(uint_t lhs, sint_t rhs) const {
                return (rhs < 0) ? false : (uint_t)rhs == lhs;
            }

            // sin_t == sin_t && uint_t == uint_t handled in generic method

            bool operator()(uint_t lhs, real_t rhs) const {
                return (rhs < 0) ? false : (real_t)lhs == rhs;
            }

            bool operator()(real_t lhs, uint_t rhs) const {
                return (lhs < 0) ? false : (real_t)rhs == lhs;
            }

            template<class T, class U>
            bool operator()(T rhs, U lhs) const {
                return rhs == lhs; 
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
    
    inline var::var(const char* v)
    : _value(std::string(v)) {
    }
    
    inline var::var(char v)
    : _value(std::string(&v, 1)) {
    }
    
    
    inline var::var(const string_t &v)
    : _value(v) {
    }
    
    inline var::var(std::string &&v)
    : _value(std::move(v)) {
    }

    
    template<typename... Args, typename ReturnType>
    inline var::var(const std::function<ReturnType(Args...)> &v)
    : _value(function_t(v)) {
    }

    template<typename T>
    inline var::var(std::initializer_list<T> v) {
        initialize_array(v);
    }
    
    template<typename T>
    inline var::var(const std::vector<T> &v) {
        initialize_array(v);
    }

    template<typename T>
    inline var::var(T begin, T end, typename std::iterator_traits<T>::iterator_category*) {
        initialize_array(begin, end);
    }
    
    inline var::var(const array_t &v)
    :_value(v)
    {}
    
    inline var::var(std::initializer_list< std::pair<string_t, var> > v) {
        initialize_object(v);
    }
    
    inline var &var::operator=(std::nullptr_t) {
        _value = nullptr;
        return *this;
    }
    
    inline var &var::operator=(bool rhs) {
        _value = rhs;
        return *this;
    }
    
    inline var &var::operator=(char rhs) {
        _value = string_t(&rhs, 1);
        return *this;
    }
    
    inline var &var::operator=(const char *rhs) {
        _value = string_t(rhs);
        return *this;
    }
    
    inline var &var::operator=(const string_t &rhs) {
        _value = rhs;
        return *this;
    }
    
    template<typename... Args, typename ReturnType>
    inline var &var::operator=(const std::function<ReturnType(Args...)> &rhs) {
        _value = function_t(rhs);
        return *this;
    }
    
    template<typename T>
    inline var &var::operator=(std::initializer_list<T> rhs) {
        initialize_array(rhs);
        return *this;
    }

    template<typename T>
    inline var &var::operator=(const std::vector<T> &rhs) {
        initialize_array(rhs);
        return *this;
    }
    
    inline var &var::operator=(const array_t &rhs) {
        _value = rhs;
        return *this;
    }
    
    inline var &var::operator=(std::initializer_list< std::pair<string_t, var> > rhs) {
        initialize_object(rhs);
        return *this;
    }
    
    template<typename I>
    inline details::EnableIfSignedIntegralPolicy<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<int64_t>(t));
        return *this;
    }
    
    template<typename I>
    inline details::EnableIfUnsignedIntegralPolicy<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<uint64_t>(t));
        return *this;
    }
    
    template<typename I>
    inline details::EnableIfFloatingPointPolicy<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<double>(t));
        return *this;
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
    
    inline const var &var::operator|(const var &default_value) const {
        return as<bool>() ? *this : default_value;
    }

    template<class T>
    inline T var::as() const {
        details::coerce<T> visitor;
        return apply_visitor(visitor, _value);
    }
    
    template<class T>
    inline T var::as(const var &opts) const {
        details::coerce<T> visitor = {opts};
        return apply_visitor(visitor, _value);
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
        if (is_undefined()) {
            return undefined_var();
        } else if (key.is_number() && is_array()) {
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

    inline void var::push_back(const var &v) {
        array_t &a = get_variant_or_convert<array_t>();
        a.push_back(v);
    }
    
    template<class T>
    inline T &var::get_variant_or_convert() {
        if (!_value.is<T>())
            _value = T();
        return _value.get<T>();
    }
    
    template<class Range>
    inline void var::initialize_array(const Range &r) {
        initialize_array(std::begin(r), std::end(r));
    }

    template<class Iter>
    inline void var::initialize_array(Iter begin, Iter end) {
        using value_type = typename std::decay< decltype(*begin) >::type;

        array_t a;
        for (; begin != end; ++begin) {
            if (std::is_same<value_type, var>::value) {
                a.push_back(*begin);
            } else {
                a.emplace_back(var(*begin));
            }
        }
        _value = std::move(a);
    }
    
    template<class Range>
    inline void var::initialize_object(const Range &r) {
        
        object_t o;
        for (auto && t : r) {
            o.insert(object_t::value_type(t.first, t.second));
        }
        _value = std::move(o);
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
        if (is_number() && rhs.is_number()) {
            return mapbox::util::apply_visitor(
                details::equal_numbers(),
                _value.get<number_t>(),
                rhs._value.get<number_t>());
        } else {
            return _value == rhs._value;
        }
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
