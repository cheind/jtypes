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

            template<typename ReturnType, typename... Args>
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

        };
    }

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

        template<typename ToType, typename EnableIfType = void>
        struct coerce_number {
        };

        template<>
        struct coerce_number<bool, void> {
            bool value = false;

            template<class T>
            void operator()(T v) { value = (v != T(0)); }
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

    class var {
    public:

        // Value initializers

        var()
            : _value(undefined_t()) {
        }

        var(std::nullptr_t)
            : _value(nullptr) {
        }

        var(bool v)
            : _value(v) {
        }


        template<typename I>
        var(I t, typename std::enable_if<std::is_integral<I>::value && std::is_signed<I>::value>::type* unused = 0)
            : _value(number_t(static_cast<int64_t>(t))) {
        }

        template<typename I>
        var(I t, typename std::enable_if<std::is_integral<I>::value && std::is_unsigned<I>::value>::type* unused = 0)
            : _value(number_t(static_cast<uint64_t>(t))) {
        }

        template<typename I>
        var(I t, typename std::enable_if<std::is_floating_point<I>::value>::type* unused = 0)
            : _value(number_t(static_cast<double>(t))) {
        }

        var(double v)
            : _value(number_t(v)) {
        }

        var(const char* v)
            : _value(std::string(v)) {
        }

        var(const std::string &v)
            : _value(v) {
        }

        var(std::string &&v)
            : _value(std::move(v)) {
        }

        template<typename... Args, typename ReturnType>
        var(std::function<ReturnType(Args...)> v)
            : _value(function_t(v)) {
        }


        // Array initializers

        template<typename T>
        var(std::initializer_list<T> v) {
            array_t a;
            for (auto && t : v) {
                a.emplace_back(var(t));
            }
            _value = std::move(a);
        }

        template<typename T>
        var(const std::vector<T> &v) {
            array_t a;
            for (auto && t : v) {
                a.emplace_back(var(t));
            }
            _value = std::move(a);
        }

        // Dictionary initializers

        var(std::initializer_list< std::pair<string_t, var> > v) {
            object_t o;
            for (auto &&t : v) {
                o.insert(object_t::value_type(t.first, t.second));
            }
            _value = std::move(o);
        }

        // Type queries
        type get_type() const { return (type)_value.which(); }


        bool_t is_undefined() const { return _value.which() == (int)type::undefined; }
        bool_t is_null() const { return _value.which() == (int)type::null; }
        bool_t is_boolean() const { return _value.which() == (int)type::boolean; }
        bool_t is_number() const { return _value.which() == (int)type::number; }
        bool_t is_string() const { return _value.which() == (int)type::string; }
        bool_t is_function() const { return _value.which() == (int)type::function; }
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
            details::coerce<T> visitor;
            apply_visitor(visitor, _value);
            return visitor.value;
        }

        // Callable interface
        template<typename ReturnType, typename... Args>
        ReturnType invoke(Args&&... args) const
        {
            if (is_function()) {
                const function_t &f = _value.get<function_t>();
                if (!f.empty()) {
                    return f.invoke<ReturnType>(std::forward<Args>(args)...);
                }
            }

            throw bad_access("Not a function or not callable.");
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

        static const var &undefined_var() {
            static var u;
            return u;
        }

        oneof _value;
    };


}

#endif
