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
#include "json.hpp"

#if defined(WIN32) || defined(_WIN32)
#pragma warning(pop) 
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
    using json = nlohmann::json;

    class var;
    
    namespace details {
        class fnc_holder;
     
        template<typename Type, typename UnqualifiedType = typename std::remove_cv<Type>::type>
        class var_iterator;
        
        struct undefined_t;
    }
    
    std::string to_json(const var &v);
    std::string to_json(const var &v, int intend);
    var from_json(const std::string &str);
    var from_json(std::istream &is);
    
    std::ostream &operator<<(std::ostream &os, const var &v);
    std::istream &operator>>(std::istream &is, var &v);
    
    class type_error : public std::logic_error {
        
    public:
        explicit type_error(const std::string& what_arg)
        : logic_error(what_arg) {
        }
        
        explicit type_error(const char* what_arg)
        : logic_error(what_arg) {
        }
        
    };
    
    class range_error : public std::logic_error {
        
    public:
        explicit range_error(const std::string& what_arg)
        : logic_error(what_arg) {
        }
        
        explicit range_error(const char* what_arg)
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
    
    namespace meta {
        template<typename T, typename R = void>
        using if_is_signed_integral = typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using if_is_unsigned_integral = typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using if_is_real = typename std::enable_if<std::is_floating_point<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using if_is_number_t = typename std::enable_if<std::is_same<T, std::int64_t>::value || std::is_same<T, std::uint64_t>::value || std::is_same<T, double>::value, R>::type;
        
        template<typename T, typename R = void>
        using if_not_is_number_t = typename std::enable_if<!std::is_same<T, std::int64_t>::value && !std::is_same<T, std::uint64_t>::value && !std::is_same<T, double>::value, R>::type;
        
        template<typename T, typename R = void>
        using if_is_function = typename std::enable_if<std::is_function<T>::value, R>::type;
        
        template<typename T, typename R = void>
        using if_not_is_function = typename std::enable_if<!std::is_function<T>::value, R>::type;
        
        template<class>
        struct result_of_sig;
        
        template <typename R, typename... Args>
        struct result_of_sig<R(Args...)> { using type = R; };
    }
    
    namespace details {

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
                    throw type_error("as() stored function signature not convertible to target signature");
                }
                
                return g->f;
            }

            template<typename R, typename ...Args>
            R invoke(Args && ... args) const {
                std::function<R(Args...)> f = as<R(Args...)>();
                if (!f) {
                    throw type_error("invoke() empty function called");
                }
                return f(std::forward<Args>(args)...);
            }

            template<typename Sig, typename ...Args>
            typename ::jtypes::meta::result_of_sig<Sig>::type invoke_with_signature(Args && ... args) const {
                const auto &f = as<Sig>();
                
                if (!f) {
                    throw type_error("invoke_with_signature() empty function called");
                }
                
                return f(std::forward<Args>(args)...);
            }

            bool empty() const {
                return !ptr || ptr->empty();
            }

        private:
            friend inline bool operator==(const fnc_holder &lhs, const fnc_holder &rhs);
            friend inline bool operator<(const fnc_holder &lhs, const fnc_holder &rhs);
            
            std::shared_ptr<fnc_erasure_base> ptr;
        };
        
        inline bool operator==(const fnc_holder &lhs, const fnc_holder &rhs) { return lhs.ptr.get() == rhs.ptr.get(); }
        inline bool operator<(const fnc_holder &lhs, const fnc_holder &rhs) { return false; }
        
        struct undefined_t {};
        inline bool operator==(const undefined_t &lhs, const undefined_t &rhs) { return true; }
        inline bool operator<(const undefined_t &lhs, const undefined_t &rhs) { return false; }
        
    }

    
    class var {
    public:
        enum class vtype {
            undefined = 0,
            null,
            boolean,
            signed_number,
            unsigned_number,
            real_number,
            string,
            function,
            array,
            object,
        };
        
        using undefined_t = details::undefined_t ;
        using null_t = std::nullptr_t ;
        using number_t = variant<std::int64_t, std::uint64_t, double>;
        using function_t = details::fnc_holder;
        using array_t = std::vector<var>;
        using object_t = std::map<std::string, var>;

        typedef details::var_iterator<var> iterator;
        typedef details::var_iterator<var const> const_iterator;
        typedef var value_type;


        using null = null_t;
        using undefined = undefined_t;
        using object = object_t;
        using array = array_t;
        
        // Value initializers
        
        var();
        
        var(std::nullptr_t);
        
        var(bool v);
        
        template<typename I>
        var(I t, typename meta::if_is_signed_integral<I>* unused = 0);
        
        template<typename I>
        var(I t, typename meta::if_is_unsigned_integral<I>* unused = 0);
        
        template<typename I>
        var(I t, typename meta::if_is_real<I>* unused = 0);
        
        var(char v);
        
        var(const char* v);
        
        var(const std::string &v);
        
        var(std::string &&v);

        var(const undefined_t &v);
        var(undefined_t &&v);

        // Function initializers
        template<class Sig, typename = meta::if_is_function<Sig> >
        static var function(std::function<Sig> && v = std::function<Sig>());

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
        var &operator=(const std::string &rhs);

        template<typename I>
        meta::if_is_signed_integral<I, var&>
            operator=(I t);

        template<typename I>
        meta::if_is_unsigned_integral<I, var&>
            operator=(I t);

        template<typename I>
        meta::if_is_real<I, var&>
            operator=(I t);

        var& operator=(const array_t &rhs);
        var& operator=(array_t &&rhs);

        var& operator=(const object_t &rhs);
        var& operator=(object_t &&rhs);

        var &operator=(const function_t &rhs);
        var &operator=(function_t &&rhs);

        var &operator=(const undefined_t &rhs);
        var &operator=(undefined_t &&rhs);
        
        // Type queries
        vtype type() const;
        bool is_undefined() const;
        bool is_null() const;
        bool is_boolean() const;
        bool is_number() const;
        bool is_signed_number() const;
        bool is_unsigned_number() const;
        bool is_real_number() const;
        bool is_string() const;
        bool is_function() const;
        bool is_array() const;
        bool is_object() const;
        bool is_structured() const;
        
        // Getters with coercion
        
        template<class T>
        explicit operator T() const;
        explicit operator bool() const;
        
        template<typename T>
        meta::if_not_is_function<T, T> as(const var &opts = undefined()) const;
        
        template<typename T>
        meta::if_is_function<T, std::function<T> > as(const var &opts = undefined()) const;
        
        // Callable interface

        template<typename Sig, typename ...Args>
        typename meta::result_of_sig<Sig>::type
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
        
        
        var size() const;
        
        // Helper methods
        
        var &merge_from(const var &other);
        
        var split(const var &separator) const;
        
        var &at(const var &path);
        const var &at(const var &path) const;
        
        void clear();

        const var& global_undefined() const;
        
    private:
        typedef variant<
        undefined_t,
        null_t,
        bool,
        number_t,
        std::string,
        function_t,
        array_t,
        object_t
        > oneof;
        
        oneof _value;
    };

    inline bool operator<(const var::object_t &lhs, const var::object_t &rhs) { return false; }

    
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
        
        inline std::vector<std::string> split(const std::string &src, char separator) {
        
            std::vector<std::string> elems;
            std::istringstream iss(src);
            std::string tok;
            
            while(std::getline(iss, tok, separator)) {
                if (!tok.empty())
                    elems.push_back(tok);
            }
            
            return elems;
        }

        template<typename NumberType>
        struct coerce {
            
            var opts;
            
            NumberType operator()(const bool &v) const { return v ? NumberType(1) : NumberType(0); }
            
            
            template< typename T = NumberType >
            NumberType operator()(const std::string &v, meta::if_is_signed_integral<T> *unused=0) const {
                try {
                    return NumberType(std::stol(v));
                } catch (std::exception) {
                    throw type_error("failed to coerce type from string to integral type");
                }
            }
            
            template< typename T = NumberType >
            NumberType operator()(const std::string &v, meta::if_is_unsigned_integral<T> *unused=0) const {
                try {
                    return NumberType(std::stoul(v));
                } catch (std::exception) {
                    throw type_error("failed to coerce type from string to integral type");
                }
            }
            
            template< typename T = NumberType >
            NumberType operator()(const std::string &v, meta::if_is_real<T> *unused=0) const {
                try {
                    return NumberType(std::stod(v));
                } catch (std::exception) {
                    throw type_error("failed to coerce type from string to integral type");
                }
            }
            
            template<class T>
            NumberType operator()(const T &t, meta::if_is_number_t<T> *unused=0) const {
                return NumberType(t); // Static cast needs to be addressed.
            }
            
            NumberType operator()(const var::number_t &v) const {
                return apply_visitor(*this, v);
            }
            
            template<class T>
            NumberType operator()(const T &t, meta::if_not_is_number_t<T> *unused=0) const {
                throw type_error("failed to coerce type to integral type");
            }
        };

        template<>
        struct coerce<bool> {
            var opts;

            bool operator()(const var::undefined_t &v) const { return false; }
            bool operator()(const var::null_t &v) const { return false; }
            bool operator()(const bool &v) const { return v; }
            bool operator()(const std::string &v) const { return !v.empty(); }
            bool operator()(const var::function_t &v) const { return !v.empty(); }
            bool operator()(const var::array_t &v) const { return true; }
            bool operator()(const var::object_t &v) const { return true; }
            
            template<class T>
            bool operator()(const T &v, meta::if_is_number_t<T> *unused=0) const { return v != T(0); };
            
            bool operator()(const var::number_t &v) const {
                return apply_visitor(*this, v);
            }
        };
        
        template<>
        struct coerce<std::string> {
            var opts;
           
            std::string operator()(const var::undefined_t &v) const { return "undefined"; }
            std::string operator()(const var::null_t &v) const { return "null"; }
            std::string operator()(const bool &v) const { return v ? "true" : "false"; }
            std::string operator()(const std::string &v) const { return v; }
            std::string operator()(const var::function_t &v) const { return "function"; }
            std::string operator()(const var::array_t &v) const {
                return join(v, ",", [](const var &vv) { return vv.as<std::string>(); });
            }

            std::string operator()(const var::object_t &v) const { return "object"; }
            
            template<class T>
            std::string operator()(const T &v, meta::if_is_number_t<T> *unused=0) const {
                return std::to_string(v); 
            };

            
            std::string operator()(const var::number_t &v) const {
                return apply_visitor(*this, v);
            }
        };

        struct equal_numbers {

            bool operator()(std::int64_t lhs, std::uint64_t rhs) const {
                return (lhs < 0) ? false : (std::uint64_t)lhs == rhs;
            }

            bool operator()(std::uint64_t lhs, std::int64_t rhs) const {
                return (rhs < 0) ? false : (std::uint64_t)rhs == lhs;
            }

            bool operator()(std::uint64_t lhs, double rhs) const {
                return (rhs < 0) ? false : (double)lhs == rhs;
            }

            bool operator()(double lhs, std::uint64_t rhs) const {
                return (lhs < 0) ? false : (double)rhs == lhs;
            }

            template<class T, class U>
            bool operator()(T rhs, U lhs) const {
                return rhs == lhs; 
            }

        };
        
        struct less_numbers {
            
            bool operator()(std::int64_t lhs, std::uint64_t rhs) const {
                return (lhs < 0) ? true : (std::uint64_t)lhs < rhs;
            }
            
            bool operator()(std::uint64_t lhs, std::int64_t rhs) const {
                return (rhs < 0) ? false : lhs < (std::uint64_t)rhs;
            }
            
            bool operator()(std::uint64_t lhs, double rhs) const {
                return (rhs < 0.0) ? false : (double)lhs < rhs;
            }
            
            bool operator()(double lhs, std::uint64_t rhs) const {
                return (lhs < 0) ? true : lhs < (double)rhs;
            }
            
            template<class T, class U>
            bool operator()(T rhs, U lhs) const {
                return rhs < lhs;
            }
            
        };
        
        template<class Iter>
        inline var create_array(Iter begin, Iter end) {
            using value_type = typename std::decay< decltype(*begin) >::type;
            
            var::array_t a;
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
            var::object_t o;
            for (auto && t : r) {
                o.insert(var::object_t::value_type(t.first, t.second));
            }
            return var(std::move(o));
        }
        
        template<typename Type, typename UnqualifiedType>
        class var_iterator : public std::iterator<std::forward_iterator_tag, UnqualifiedType, std::ptrdiff_t, Type*, Type&> {
        public:
            using array_iterator = typename std::conditional<std::is_const<Type>::value, var::array_t::const_iterator, var::array_t::iterator>::type;
            using object_iterator = typename std::conditional<std::is_const<Type>::value, var::object_t::const_iterator, var::object_t::iterator>::type;
            
            var_iterator()
            :_iter(mapbox::util::no_init())
            {}
            
            explicit var_iterator(const array_iterator &i, std::uint64_t offset)
            :_iter(index_array_iter_pair(offset, i))
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
                inc();
                return *this;
            }
            
            var_iterator operator++ (int) // Post-increment
            {
                var_iterator tmp(*this);
                inc();
                return tmp;
            }
            
            template<class OtherType>
            bool operator == (const var_iterator<OtherType>& rhs) const
            {
                if (_iter.valid() || rhs._iter.valid()) {
                    return _iter == rhs._iter;
                } else {
                    // Both are invalid
                    return true;
                }
                
            }
            
            template<class OtherType>
            bool operator != (const var_iterator<OtherType>& rhs) const
            {
                return !(*this == rhs);
            }
            
            var key() const {
                if (!_iter.valid())
                    throw type_error("key() requires a valid iterator");
                
                if (_iter.which() == 0) {
                    return _iter.template get<index_array_iter_pair>().first;
                } else {
                    return _iter.template get<object_iterator>()->first;
                }
            }
            
            Type& value() const {
                if (!_iter.valid())
                    throw type_error("value() requires a valid iterator");
                
                if (_iter.which() == 0) {
                    return *_iter.template get<index_array_iter_pair>().second;
                } else {
                    return _iter.template get<object_iterator>()->second;
                }
            }
            
            Type& operator* () const
            {
                if (!_iter.valid())
                    throw type_error("accessing iterator's value requires a valid iterator");
                
                if (_iter.which() == 0) {
                    return *_iter.template get<index_array_iter_pair>().second;
                } else {
                    return _iter.template get<object_iterator>()->second;
                }
            }
            
            Type* operator-> () const
            {
                if (!_iter.valid())
                    throw type_error("accessing iterator's value requires a valid iterator");
                
                if (_iter.which() == 0) {
                    return &(*_iter.template get<index_array_iter_pair>().second);
                } else {
                    return &(_iter.template get<object_iterator>()->second);
                }
            }
            
            operator var_iterator<const UnqualifiedType>() const
            {
                return var_iterator<const UnqualifiedType>(_iter);
            }
            
        private:
            
            void inc() {
                if (_iter.which() == 0) {
                    auto &r = _iter.template get<index_array_iter_pair>();
                    ++r.first;
                    ++r.second;
                } else  {
                    ++_iter.template get<object_iterator>();
                }
            }
            
            using index_array_iter_pair = std::pair<std::uint64_t, array_iterator>;
            
            var_iterator(const variant<index_array_iter_pair, object_iterator> &other)
            :_iter(other)
            {}
            
            variant<index_array_iter_pair, object_iterator> _iter;
        };
        
        inline bool merge(var &dst, const var &src) {
            if (!src.is_object())
                return false;
            
            if (!dst.is_object()) {
                dst = src;
                return true;
            }
            
            // For each entry in src
            for (auto && k : src.keys()) {
                if (dst[k].is_object() && src[k].is_object()) {
                    merge(dst[k], src[k]);
                } else {
                    dst[k] = src[k];
                }
            }
            
            return true;
        }
        
        inline json to_json(const var &v, bool *should_discard = 0) {
            if (should_discard) *should_discard = false;
            
            switch(v.type()) {
                case var::vtype::array: {
                    bool discard = false;
                    json j = json::array();
                    for (auto && vv : v) {
                        json jj = to_json(vv, &discard);
                        if (!discard) j.push_back(jj);
                    }
                    return j;
                }
                case var::vtype::boolean:
                    return v.as<bool>();
                case var::vtype::signed_number:
                    return v.as<std::int64_t>();
                case var::vtype::unsigned_number:
                    return v.as<std::uint64_t>();
                case var::vtype::real_number:
                    return v.as<double>();
                case var::vtype::string:
                    return v.as<std::string>();
                case var::vtype::object: {
                    bool discard = false;
                    json j = json::object();
                    for (auto && k : v.keys()) {
                        json jj = to_json(v[k], &discard);
                        if (!discard) j[k.as<std::string>()] = jj;
                    }
                    return j;
                }
                case var::vtype::function:
                case var::vtype::undefined: {
                    if (should_discard) *should_discard = true;
                    return json();
                }
                case var::vtype::null: {
                    return nullptr;
                }                    
            }
            throw type_error("to_json() unexpected type.");
        }
        
        inline var from_json(const json &j) {
            switch (j.type()) {
                case json::value_t::array: {
                    var v = var::array_t();
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
                    return j.get<double>();
                case json::value_t::number_integer:
                    return j.get<std::int64_t>();
                case json::value_t::number_unsigned:
                    return j.get<std::uint64_t>();
                case json::value_t::object: {
                    var v = var::object_t();
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

            throw type_error("from_json() unexpected type.");
        }
    }
    
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
    
    inline std::ostream &operator<<(std::ostream &os, const var &v) {
        // use std::setw to format with intendation.
        os << details::to_json(v);
        return os;
    }
    
    inline std::istream &operator>>(std::istream &is, var &v) {
        v = from_json(is);
        return is;
    }

    // Implementation of var
    
    
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
    inline var::var(I t, typename meta::if_is_signed_integral<I>* unused)
    : _value(number_t(static_cast<int64_t>(t))) {
    }
    
    template<typename I>
    inline var::var(I t, typename meta::if_is_unsigned_integral<I>* unused)
    : _value(number_t(static_cast<uint64_t>(t))) {
    }
    
    template<typename I>
    inline var::var(I t, typename meta::if_is_real<I>* unused)
    : _value(number_t(static_cast<double>(t))) {
    }

    template<class Sig, typename>
    inline var var::function(std::function<Sig>&& v) {
        return var(var::function_t(std::forward<std::function<Sig>>(v)));
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

    inline var::var(const undefined_t &v)
    : _value(v) 
    {}

    inline var::var(undefined_t &&v)
        : _value(std::move(v))             
    {}

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
        _value = std::string(&rhs, 1);
        return *this;
    }
    
    inline var &var::operator=(const char *rhs) {
        _value = std::string(rhs);
        return *this;
    }
    
    inline var &var::operator=(const std::string &rhs) {
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

    inline var & var::operator=(const undefined_t & rhs) {
        _value = rhs;
        return *this;
    }

    inline var & var::operator=(undefined_t && rhs) {
        _value = std::move(rhs);
        return *this;
    }
    
    
    template<typename I>
    inline meta::if_is_signed_integral<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<int64_t>(t));
        return *this;
    }
    
    template<typename I>
    inline meta::if_is_unsigned_integral<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<uint64_t>(t));
        return *this;
    }
    
    template<typename I>
    inline meta::if_is_real<I, var&>
    var::operator=(I t) {
        _value = number_t(static_cast<double>(t));
        return *this;
    }
    
    
    inline var::vtype var::type() const {
        if      (is_undefined()) return vtype::undefined;
        else if (is_null()) return vtype::null;
        else if (is_boolean()) return vtype::boolean;
        else if (is_signed_number()) return vtype::signed_number;
        else if (is_unsigned_number()) return vtype::unsigned_number;
        else if (is_real_number()) return vtype::real_number;
        else if (is_string()) return vtype::string;
        else if (is_function()) return vtype::function;
        else if (is_array()) return vtype::array;
        else if (is_object()) return vtype::object;
        
        throw range_error("type() unknown type");
    }
    
    inline bool var::is_undefined() const { return _value.is<undefined_t>(); }
    inline bool var::is_null() const { return _value.is<null_t>(); }
    inline bool var::is_boolean() const { return _value.is<bool>(); }
    inline bool var::is_number() const { return _value.is<number_t>(); }
    inline bool var::is_signed_number() const { return is_number() && _value.get<number_t>().is<std::int64_t>(); }
    inline bool var::is_unsigned_number() const { return is_number() && _value.get<number_t>().is<std::uint64_t>(); }
    inline bool var::is_real_number() const { return is_number() && _value.get<number_t>().is<double>(); }
    inline bool var::is_string() const { return _value.is<std::string>(); }
    inline bool var::is_function() const { return _value.is<function_t>(); }
    inline bool var::is_array() const { return _value.is<array_t>(); }
    inline bool var::is_object() const { return _value.is<object_t>(); }
    inline bool var::is_structured() const { return is_object() || is_array();}
    
    
    template<class T>
    inline var::operator T() const {
        return as<T>();
    }

    inline var::operator bool() const {
        return as<bool>();
    }
    
    inline const var &var::operator|(const var &default_value) const {
        return as<bool>() ? *this : default_value;
    }
    
    template<typename T>
    inline meta::if_not_is_function<T, T> var::as(const var &opts) const
    {
        details::coerce<T> visitor = {opts};
        return apply_visitor(visitor, _value);
    }
    
    template<typename T>
    inline meta::if_is_function<T, std::function<T> > var::as(const var &opts) const
    {
        if (!is_function())
            throw type_error("as() with function signature called but cannot be coerced to function");
        
        const function_t &f = _value.get<function_t>();
        return f.as<T>();
    }
    
    template<typename Sig, typename... Args>
    inline typename meta::result_of_sig<Sig>::type var::invoke(Args&&... args) const
    {
        if (!is_function())
            throw type_error("invoke() not a function");
        
        const function_t &f = _value.get<function_t>();
        return f.invoke_with_signature<Sig>(std::forward<Args>(args)...);
    }
    
    // Object / Array accessors
    inline var &var::operator[](const var &key) {
        if (!is_structured()) {
            throw type_error("operator[] requires a structured type");
        }
        
        if (key.is_number() && is_array()) {
            
            array_t &a = _value.get<array_t>();
            size_t idx = key.as<size_t>();
            if (a.size() < idx + 1) {
                a.resize(idx + 1);
            }
            return a[idx];
        } else if (key.is_string() && is_object()) {
            object_t &o = _value.get<object_t>();
            auto iter = o.insert(object_t::value_type(key.as<std::string>(), var()));
            return iter.first->second;
        } else {
            throw type_error("operator[] key type and structured var type do not match");
        }
        
    }
    
    inline const var &var::operator[](const var &key) const {
        
        if (!is_structured()) {
            throw type_error("operator[] requires a structured type.");
        }
        
        if (key.is_number() && is_array()) {
            const array_t &a = _value.get<array_t>();
            size_t idx = key.as<size_t>();
            if (idx < a.size()) {
                return a[idx];
            } else {
                return var::global_undefined();
            }
        } else if (key.is_string() && is_object()) {
            const object_t &o = _value.get<object_t>();
            
            auto iter = o.find(key.as<std::string>());
            
            if (iter != o.end()) {
                return iter->second;
            } else {
                return var::global_undefined();
            }
        } else {
            throw type_error("operator[] key type and structured var type do not match.");
        }
    }

    inline void var::push_back(const var &v) {
        if (!is_array())
            throw type_error("push_back() requires array type.");
        
        array_t &a = _value.get<array_t>();
        a.push_back(v);
    }
    

    inline var::array_t var::keys() const {
        array_t r;
        
        if (is_object()) {
            const object_t &o = _value.get<object_t>();
            for (auto p : o) {
                r.push_back(var(p.first));
            }
        } else if (is_array()) {
            const array_t &a = _value.get<array_t>();
            for (size_t i = 0; i < a.size(); ++i) {
                r.push_back(var(i));
            }
        }
        
        return r;
        
    }
    
    inline var::array_t var::values() const {
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
        
        if (!is_structured()) {
            return iterator();
        } else if (is_object()) {
            return iterator(_value.get<object_t>().begin());
        } else {
            return iterator(_value.get<array_t>().begin(), 0);
        }
    }
    
    inline var::iterator var::end() {
        if (!is_structured()) {
            return iterator();
        } else if (is_object()) {
            return iterator(_value.get<object_t>().end());
        } else {
            return iterator(_value.get<array_t>().end(), _value.get<array_t>().size());
        }
    }
    
    inline var::const_iterator var::begin() const {
        if (!is_structured()) {
            return const_iterator();
        } else if (is_object()) {
            return const_iterator(_value.get<object_t>().begin());
        } else {
            return const_iterator(_value.get<array_t>().begin(), 0);
        }
    }
    
    inline var::const_iterator var::end() const {
        if (!is_structured()) {
            return const_iterator();
        } else if (is_object()) {
            return const_iterator(_value.get<object_t>().end());
        } else {
            return const_iterator(_value.get<array_t>().end(), _value.get<array_t>().size());
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
    
    inline var var::size() const {
        if (!is_structured())
            throw type_error("size() requires a structured type");
        
        if (is_array()) {
            return _value.get<array_t>().size();
        } else {
            return _value.get<object_t>().size();
        }
    }
    
    inline var &var::merge_from(const var &other) {
        details::merge(*this, other);
        return *this;
    }
    
    inline var var::split(const var &delim) const {
        const std::string s_src = as<std::string>();
        const std::string s_delim = delim.as<std::string>();
        
        if (s_delim.size() != 1) {
            throw range_error("split() requires the deliminator to be a single character");
        }
        
        return details::create_array(details::split(s_src, s_delim.at(0)));
    }
    
    inline var &var::at(const var &path) {
        
        if (!is_structured())
            throw type_error("at() requires structured type");
        
        if (is_array()) {
            return (*this)[path];
        }
        
        var elems = path.is_array() ? path : var(path.as<std::string>()).split('.');
        
        if (elems.size() == 0)
            return *this;
        
        const int n = (int)elems.size();
        
        var *e = this;
        for (size_t i = 0; i < n-1; ++i) {
            const std::string pe = elems[i].as<std::string>();
            var &c = (*e)[pe];
            if (!c.is_object()) {
                c = var::object();
            }
            e = &c;
        }
        
        return (*e)[elems[n-1]];
    }
    
    
    inline const var &var::at(const var &path) const {
        if (!is_structured())
            throw type_error("at() requires structured type");
        
        if (is_array()) {
            return (*this)[path];
        }
        
        var elems = path.is_array() ? path : var(path.as<std::string>()).split('.');
        
        if (elems.size() == 0)
            return *this;
        
        const int n = (int)elems.size();
        
        const var *e = this;
        for (size_t i = 0; i < n; ++i) {
            const std::string pe = elems[i].as<std::string>();
            const var &c = (*e)[pe];
            if (c.is_undefined()) {
                return c;
            }
            e = &c;
        }
        
        return *e;
    }
    
    inline void var::clear() {
        if (!is_structured())
            throw type_error("at() requires structured type");
        
        if (is_object())
            _value.get<object_t>().clear();
        else if (is_array())
            _value.get<array_t>().clear();
        
    }

    inline const var & var::global_undefined() const {
        static var u = undefined();
        return u;
    }
}

#endif
