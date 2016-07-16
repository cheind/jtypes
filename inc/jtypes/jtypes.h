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

#ifndef JTYPES_NO_JSON

#include "json.hpp"

#endif

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <functional>


namespace jtypes {

    using mapbox::util::variant;
    
#ifndef JTYPES_NO_JSON
    using json = nlohmann::json;
#endif

    class var;
    
    namespace details {
        class fnc_holder;
        
        template<typename Type, typename UnqualifiedType = typename std::remove_cv<Type>::type>
        class var_iterator;
    }
   
    typedef int64_t sint_t;
    typedef uint64_t uint_t;
    typedef double real_t;

    struct undefined_t {};
    typedef std::nullptr_t null_t;
    typedef bool bool_t;
    typedef variant<int64_t, uint64_t, double> number_t;
    typedef std::string string_t;
    typedef details::fnc_holder function_t;
    typedef std::vector<var> array_t;
    typedef std::map<std::string, var> object_t;
    
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
    
    class syntax_error : public std::logic_error {
        
    public:
        explicit syntax_error(const std::string& what_arg)
        : logic_error(what_arg) {
        }
        
        explicit syntax_error(const char* what_arg)
        : logic_error(what_arg) {
        }
        
    };
    
    namespace details {
        template<typename T, typename R = void>
        using EnableIfSignedIntegralType = typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using EnableIfUnsignedIntegralType = typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using EnableIfFloatingPointType = typename std::enable_if<std::is_floating_point<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using EnableIfNumberType = typename std::enable_if<std::is_same<T, sint_t>::value || std::is_same<T, uint_t>::value || std::is_same<T, real_t>::value, R>::type;
        
        template<typename T, typename R = void>
        using DisableIfNumberType = typename std::enable_if<!std::is_same<T, sint_t>::value && !std::is_same<T, uint_t>::value && !std::is_same<T, real_t>::value, R>::type;


        struct type_erasure_base {
            virtual ~type_erasure_base() = default;
        };

        struct fnc_erasure_base : type_erasure_base {
            virtual bool empty() const = 0;
        };

        template<typename Sig>
        struct fnc_wrapper : fnc_erasure_base {
            std::function<Sig> f;

            fnc_wrapper(std::function<Sig> && _f)
                :f(std::forward<std::function<Sig>>(_f)) {
            }

            bool empty() const { return !f; }
        };

        template<class>
        struct result_of_sig;

        template <typename R, typename... Args>
        struct result_of_sig<R(Args...)> { using type = R; };

        class fnc_holder {
        public:
            template<typename Sig>
            fnc_holder(std::function<Sig> && f)
                :ptr(new fnc_wrapper<Sig>(std::forward<std::function<Sig>>(f))) {
            }

            template<typename Sig>
            std::function<Sig> as() const {
                fnc_wrapper<Sig> * g = dynamic_cast<fnc_wrapper<Sig> *>(ptr.get());
                if (g == nullptr) {
                    throw bad_access("Could not convert stored signature to target function signature");
                } else {
                    return g->f;
                }
            }

            template<typename R, typename ...Args>
            R invoke(Args && ... args) const {
                std::function<R(Args...)> f = as<R(Args...)>();
                if (f) {
                    return f(std::forward<Args>(args)...);
                } else {
                    throw std::bad_cast();
                }
            }

            template<typename Sig, typename ...Args>
            typename result_of_sig<Sig>::type invoke_with_signature(Args && ... args) const {
                const auto &f = as<Sig>();
                if (f) {
                    return f(std::forward<Args>(args)...);
                } else {
                    throw bad_access("Failed to invoke function. Function signature was ok, but function is empty.");
                }
            }

            bool empty() const {
                return !ptr || ptr->empty();
            }

            bool operator==(const fnc_holder &rhs) const {
                return false;
            }

            bool operator<(const fnc_holder &rhs) const {
                return false;
            }

        private:
            std::shared_ptr<fnc_erasure_base> ptr;
        };
        
    }

    
    class var {
    public:
        
        typedef details::var_iterator<var> iterator;
        typedef details::var_iterator<var const> const_iterator;
        
        // Value initializers
        
        var();
        
        var(std::nullptr_t);
        
        var(bool v);
        
        template<typename I>
        var(I t, typename details::EnableIfSignedIntegralType<I>* unused = 0);
        
        template<typename I>
        var(I t, typename details::EnableIfUnsignedIntegralType<I>* unused = 0);
        
        template<typename I>
        var(I t, typename details::EnableIfFloatingPointType<I>* unused = 0);
        
        var(char v);
        
        var(const char* v);
        
        var(const string_t &v);
        
        var(std::string &&v);

        // Function initializers

        var(const function_t &v);
        var(function_t &&v);

        // Array initializers
        
        var(const array_t &v);
        var(array_t &&v);
        
        // Dictionary initializers

        var(const object_t &v);
        var(object_t &&v);

        // Assignments

        var &operator=(const var &rhs) = default;
        var &operator=(std::nullptr_t);
        var &operator=(bool rhs);
        var &operator=(char rhs);
        var &operator=(const char* v);
        var &operator=(const string_t &rhs);

        template<typename I>
        details::EnableIfSignedIntegralType<I, var&>
            operator=(I t);

        template<typename I>
        details::EnableIfUnsignedIntegralType<I, var&>
            operator=(I t);

        template<typename I>
        details::EnableIfFloatingPointType<I, var&>
            operator=(I t);

        var& operator=(const array_t &rhs);
        var& operator=(array_t &&rhs);

        var& operator=(const object_t &rhs);
        var& operator=(object_t &&rhs);

        var &operator=(const function_t &rhs);
        var &operator=(function_t &&rhs);
        
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
        bool_t is_structured() const;
        
        // Getters with coercion
        
        template<class T>
        explicit operator T() const;
        
        template<typename T>
        typename std::enable_if<!std::is_function<T>::value, T>::type as(const var &opts = undefined_var()) const;
        
        template<typename T>
        typename std::enable_if<std::is_function<T>::value, std::function<T> >::type as(const var &opts = undefined_var()) const;
        
        // Callable interface

        template<typename Sig, typename ...Args>
        typename details::result_of_sig<Sig>::type
        invoke(Args && ... args) const;
        
        // Array / Object accessors
        
        var &operator[](const var &key);
        const var &operator[](const var &key) const;
        
        // This or default value.
        
        const var &operator|(const var &default_value) const;
        
        // Keys accessor
        array_t keys() const;
        array_t values() const;
        
        // Iterator access
        iterator begin();
        iterator end();
        
        const_iterator begin() const;
        const_iterator end() const;
        
        // Array inserters

        void push_back(const var &v);
        
        // Comparison interface
        
        bool operator==(var const& rhs) const;
        bool operator!=(var const& rhs) const;
        bool operator<(var const& rhs) const;
        bool operator>(var const& rhs) const;
        bool operator<=(var const& rhs) const;
        bool operator>=(var const &rhs) const;
        
        static const var &undefined_var();
        
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
        
        template<typename T>
        T &get_variant_or_convert();
        
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
        
        template<typename Range>
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
            NumberType operator()(const string_t &v, EnableIfSignedIntegralType<T> *unused=0) const {
                try {
                    return NumberType(std::stol(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to integral type.");
                }
            }
            
            template< typename T = NumberType >
            NumberType operator()(const string_t &v, EnableIfUnsignedIntegralType<T> *unused=0) const {
                try {
                    return NumberType(std::stoul(v));
                } catch (std::exception) {
                    throw bad_access("Failed to coerce type from string to integral type.");
                }
            }
            
            template< typename T = NumberType >
            NumberType operator()(const string_t &v, EnableIfFloatingPointType<T> *unused=0) const {
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
            var opts;

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
        
        struct less_numbers {
            
            bool operator()(sint_t lhs, uint_t rhs) const {
                return (lhs < 0) ? true : (uint_t)lhs < rhs;
            }
            
            bool operator()(uint_t lhs, sint_t rhs) const {
                return (rhs < 0) ? false : lhs < (uint_t)rhs;
            }
            
            bool operator()(uint_t lhs, real_t rhs) const {
                return (rhs < 0.0) ? false : (real_t)lhs < rhs;
            }
            
            bool operator()(real_t lhs, uint_t rhs) const {
                return (lhs < 0) ? true : lhs < (real_t)rhs;
            }
            
            template<class T, class U>
            bool operator()(T rhs, U lhs) const {
                return rhs < lhs;
            }
            
        };
        
        template<class Iter>
        inline var create_array(Iter begin, Iter end) {
            using value_type = typename std::decay< decltype(*begin) >::type;
            
            array_t a;
            for (; begin != end; ++begin) {
                if (std::is_same<value_type, var>::value) {
                    a.push_back(*begin);
                } else {
                    a.emplace_back(var(*begin));
                }
            }
            return var(std::move(a));
        }
        
        template<class Range>
        inline var create_array(const Range &r) {
            return create_array(std::begin(r), std::end(r));
        }
        
        template<typename Range>
        inline var create_object(const Range &r) {
            object_t o;
            for (auto && t : r) {
                o.insert(object_t::value_type(t.first, t.second));
            }
            return var(std::move(o));
        }
        
        template<typename Type, typename UnqualifiedType>
        class var_iterator : public std::iterator<std::forward_iterator_tag, UnqualifiedType, std::ptrdiff_t, Type*, Type&> {
        public:
            using array_iterator = typename std::conditional<std::is_const<Type>::value, array_t::const_iterator, array_t::iterator>::type;
            using object_iterator = typename std::conditional<std::is_const<Type>::value, object_t::const_iterator, object_t::iterator>::type;
            
            
            explicit var_iterator(const array_iterator &i)
            :_iter(i)
            {}
            
            explicit var_iterator(const object_iterator &i)
            :_iter(i)
            {}
            
            void swap(var_iterator& other) noexcept
            {
                using std::swap;
                swap(_iter, other._iter);
            }
            
            var_iterator& operator++ () // Pre-increment
            {
                if (_iter.which() == 0) ++_iter.template get<array_iterator>();
                else ++_iter.template get<object_iterator>();
                return *this;
            }
            
            var_iterator operator++ (int) // Post-increment
            {
                var_iterator tmp(*this);
                if (_iter.which() == 0) ++_iter.template get<array_iterator>();
                else ++_iter.template get<object_iterator>();
                return tmp;
            }
            
            template<class OtherType>
            bool operator == (const var_iterator<OtherType>& rhs) const
            {
                return _iter == rhs._iter;
            }
            
            template<class OtherType>
            bool operator != (const var_iterator<OtherType>& rhs) const
            {
                return !(*this == rhs);
            }
            
            var key() const {
                if (_iter.which() == 0) {
                    throw bad_access("Cannot access key of array iterator");
                } else {
                    return _iter.template get<object_iterator>()->first;
                }
            }
            
            Type& value() const {
                if (_iter.which() == 0) {
                    return *_iter.template get<array_iterator>();
                } else {
                    return _iter.template get<object_iterator>()->second;
                }
            }
            
            Type& operator* () const
            {
                if (_iter.which() == 0) {
                    return *_iter.template get<array_iterator>();
                } else {
                    return _iter.template get<object_iterator>()->second;
                }
            }
            
            Type* operator-> () const
            {
                if (_iter.which() == 0) {
                    return &(*_iter.template get<array_iterator>());
                } else {
                    return &(_iter.template get<object_iterator>()->second);
                }
            }
            
            operator var_iterator<const UnqualifiedType>() const
            {
                return var_iterator<const UnqualifiedType>(_iter);
            }
            
        private:
            
            var_iterator(const variant<array_iterator, object_iterator> &other)
            :_iter(other)
            {}
            
            variant<array_iterator, object_iterator> _iter;
        };
        
#ifndef JTYPES_NO_JSON
        
        inline json to_json(const var &v, bool *should_discard = 0) {
            if (should_discard) *should_discard = false;
            
            switch(v.get_type()) {
                case type::array: {
                    bool discard = false;
                    json j = json::array();
                    for (auto && vv : v) {
                        json jj = to_json(vv, &discard);
                        if (!discard) j.push_back(jj);
                    }
                    return j;
                }
                case type::boolean:
                    return v.as<bool>();
                case type::number:
                    if (v.is_signed_integral()) return v.as<sint_t>();
                    else if (v.is_unsigned_integral()) return v.as<uint_t>();
                    else return v.as<real_t>();
                case type::string:
                    return v.as<std::string>();
                case type::object: {
                    bool discard = false;
                    json j = json::object();
                    for (auto && k : v.keys()) {
                        json jj = to_json(v[k], &discard);
                        if (!discard) j[k.as<std::string>()] = jj;
                    }
                    return j;
                }
                case type::function:
                case type::undefined: {
                    if (should_discard) *should_discard = true;
                    return json();
                }
                case type::null: {
                    return nullptr;
                }
                    
            }
        }
        
        inline var from_json(const json &j) {
            switch (j.type()) {
                case json::value_t::array: {
                    var v;
                    for (auto iter = j.begin(); iter != j.end(); ++iter) {
                        v.push_back(from_json(*iter));
                    }
                    return v;
                }
                case json::value_t::boolean:
                    return j.get<bool>();
                case json::value_t::null:
                    return nullptr;
                case json::value_t::number_float:
                    return j.get<real_t>();
                case json::value_t::number_integer:
                    return j.get<sint_t>();
                case json::value_t::number_unsigned:
                    return j.get<uint_t>();
                case json::value_t::object: {
                    var v;
                    for (auto iter = j.begin(); iter != j.end(); ++iter) {
                        v[iter.key()] = from_json(iter.value());
                    }
                    return v;
                }
                case json::value_t::string:
                    return j.get<std::string>();
                case json::value_t::discarded:
                    return var();
            }
        }
#endif
        
    }
    
#ifndef JTYPES_NO_JSON
    
    inline std::string to_json(const var &v) {
        return details::to_json(v).dump();
    }
    
    inline std::string to_json(const var &v, int intend) {
        return details::to_json(v).dump(intend);
    }
    
    inline var from_json(const std::string &str) {
        try {
            json j = json::parse(str);
            return details::from_json(j);
        } catch (std::exception &e) {
            throw syntax_error(e.what());
        }
    }
    
    inline var from_json(std::istream &is) {
        try {
            json j = json::parse(is);
            return details::from_json(j);
        } catch (std::exception &e) {
            throw syntax_error(e.what());
        }
    }
    
#endif
    
    inline var arr(std::initializer_list<var> v) {
        return details::create_array(v);
    }
    
    template<typename T>
    inline var arr(T begin, T end, typename std::iterator_traits<T>::iterator_category * = nullptr) {
        return details::create_array(begin, end);
    }
    
    inline var obj(std::initializer_list<std::pair<const string_t, var>> v) {
        return details::create_object(v);
    }
    
    template<typename Sig>
    inline var fnc(std::function<Sig> && f = std::function<Sig>()) {
        return var(function_t(std::forward<std::function<Sig>>(f)));
    }
    
    std::ostream &operator<<(std::ostream &os, const var &v) {
#ifndef JTYPES_NO_JSON
        // use std::setw to format with intendation.
        os << details::to_json(v);
#else 
        os << "var";
#endif
        return os;
    }
    
    std::istream &operator>>(std::istream &is, var &v) {
#ifndef JTYPES_NO_JSON
        v = from_json(is);
#else
        throw syntax_error("no json parser available.");
#endif
        return is;
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
    inline var::var(I t, typename details::EnableIfSignedIntegralType<I>* unused)
    : _value(number_t(static_cast<int64_t>(t))) {
    }
    
    template<typename I>
    inline var::var(I t, typename details::EnableIfUnsignedIntegralType<I>* unused)
    : _value(number_t(static_cast<uint64_t>(t))) {
    }
    
    template<typename I>
    inline var::var(I t, typename details::EnableIfFloatingPointType<I>* unused)
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

    inline var::var(const function_t & v) 
        :_value(v)
    {}

    inline var::var(function_t && v) 
        :_value(std::move(v))
    {
    }
   
    inline var::var(const array_t &v)
    :_value(v)
    {}

    inline var::var(array_t &&v)
        : _value(std::move(v)) 
    {}

    inline var::var(const object_t & v) 
        :_value(v)
    {}

    inline var::var(object_t && v) 
        :_value(std::move(v))
    {}
   
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
    
    inline var &var::operator=(const array_t &rhs) {
        _value = rhs;
        return *this;
    }

    inline var & var::operator=(array_t && rhs) {
        _value = std::move(rhs);
        return *this;
    }

    inline var & var::operator=(const object_t & rhs) {
        _value = rhs;
        return *this;
    }

    inline var & var::operator=(object_t && rhs) {
        _value = std::move(rhs);
        return *this;
    }

    inline var & var::operator=(const function_t & rhs) {
        _value = rhs;
        return *this;
    }

    inline var & var::operator=(function_t && rhs) {
        _value = std::move(rhs);
        return *this;
    }
    
    
    template<typename I>
    inline details::EnableIfSignedIntegralType<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<int64_t>(t));
        return *this;
    }
    
    template<typename I>
    inline details::EnableIfUnsignedIntegralType<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<uint64_t>(t));
        return *this;
    }
    
    template<typename I>
    inline details::EnableIfFloatingPointType<I, var&>
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
    inline bool_t var::is_structured() const { return is_object() || is_array(); }
    
    inline bool_t var::is_signed_integral() const { return is_number() && _value.get<number_t>().which() == (int)number_type::signed_integral; }
    inline bool_t var::is_unsigned_integral() const { return is_number() && _value.get<number_t>().which() == (int)number_type::unsigned_integral; }
    inline bool_t var::is_real() const { return is_number() && _value.get<number_t>().which() == (int)number_type::real; }
    
    
    template<class T>
    inline var::operator T() const {
        return as<T>();
    }
    
    inline const var &var::operator|(const var &default_value) const {
        return as<bool>() ? *this : default_value;
    }
    
    template<typename T>
    inline typename std::enable_if<!std::is_function<T>::value, T>::type var::as(const var &opts) const
    {
        details::coerce<T> visitor = {opts};
        return apply_visitor(visitor, _value);
    }
    
    template<typename T>
    inline typename std::enable_if<std::is_function<T>::value, std::function<T> >::type var::as(const var &opts) const
    {
        if (is_function()) {
            const function_t &f = _value.get<function_t>();
            return f.as<T>();
        }
        
        throw bad_access("Not a function or callable.");
    }
    
    template<typename Sig, typename... Args>
    inline typename details::result_of_sig<Sig>::type var::invoke(Args&&... args) const
    {
        if (is_function()) {
            const function_t &f = _value.get<function_t>();
            return f.invoke_with_signature<Sig>(std::forward<Args>(args)...);
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
    
    template<typename T>
    inline T &var::get_variant_or_convert() {
        if (!_value.is<T>())
            _value = T();
        return _value.get<T>();
    }
        
    
    inline const var &var::undefined_var() {
        static var u;
        return u;
    }

    inline array_t var::keys() const {
        array_t r;
        
        if (is_object()) {
            const object_t &o = _value.get<object_t>();
            for (auto p : o) {
                r.push_back(var(p.first));
            }
        } else if (is_array()) {
            const array_t &a = _value.get<array_t>();
            for (size_t i = 0; i < a.size(); ++i) {
                r.push_back(var(std::to_string(i)));
            }
        }
        
        return r;
        
    }
    
    inline array_t var::values() const {
        array_t r;
        
        if (is_object()) {
            for (auto k : keys()) {
                r.push_back((*this)[k]);
            }
        } else if (is_array()) {
            r = _value.get<array_t>();
        }
        return r;
    }
    
    inline var::iterator var::begin() {
        if (!is_structured())
            object_t &o = get_variant_or_convert<object_t>();
        
        if (is_object()) {
            return iterator(_value.get<object_t>().begin());
        } else {
            return iterator(_value.get<array_t>().begin());
        }
    }
    
    inline var::iterator var::end() {
        if (!is_structured())
            object_t &o = get_variant_or_convert<object_t>();
        
        if (is_object()) {
            return iterator(_value.get<object_t>().end());
        } else {
            return iterator(_value.get<array_t>().end());
        }
    }
    
    inline var::const_iterator var::begin() const {
        if (!is_structured())
            throw bad_access("iterator interface requires structured type.");
        
        if (is_object()) {
            return const_iterator(_value.get<object_t>().begin());
        } else {
            return const_iterator(_value.get<array_t>().begin());
        }
    }
    
    inline var::const_iterator var::end() const {
        if (!is_structured())
            throw bad_access("iterator interface requires structured type.");
        
        if (is_object()) {
            return const_iterator(_value.get<object_t>().end());
        } else {
            return const_iterator(_value.get<array_t>().end());
        }
    }
    

    inline bool var::operator==(var const& rhs) const {
        if (is_number() && rhs.is_number()) {
            return mapbox::util::apply_visitor(details::equal_numbers(), _value.get<number_t>(), rhs._value.get<number_t>());
        } else {
            return _value == rhs._value;
        }
    }
    
    inline bool var::operator!=(var const& rhs) const {
        return !(*this == rhs);
    }
    
    inline bool var::operator<(var const& rhs) const {
        if (is_number() && rhs.is_number()) {
            return mapbox::util::apply_visitor(details::less_numbers(), _value.get<number_t>(), rhs._value.get<number_t>());
        } else {
            return _value < rhs._value;
        }
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
