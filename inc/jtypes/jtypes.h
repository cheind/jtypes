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

namespace jtypes {

    using mapbox::util::variant;

    class object;
    class array;
    class value;
    class undefined {};
    
    typedef std::nullptr_t null;

    class object {
    public:
        std::unordered_map<std::string, value> m;
    };

    class array {
    public:
        std::vector<value> a;

        array() = default;

        template<class T>
        array(const std::initializer_list<T> &t);
        
        array(const std::initializer_list<value> &t);
        
        const value &operator[](size_t idx) const;
        value &operator[](size_t idx);
    };


    class value {
    public:
        typedef variant<null, undefined, int, std::string, object, array> var;
        var v;

        value()
        :v(undefined())
        {}
        
        const static value theUndefined;

        value(var && v_)
        :v(std::move(v_)) {}
    };

    const value value::theUndefined = value(undefined());

    template<class T>
    array::array(const std::initializer_list<T> &t) {
        for (auto && v : t) {
            a.push_back(value(v));
        }
    }
    
    array::array(const std::initializer_list<value> &t) {
        a = t;
    }

    const value &array::operator[](size_t idx) const {
        if (idx < a.size()){
            return value::theUndefined;
        } else {
            return a[idx];
        }
    }
    
    value &array::operator[](size_t idx) {
        if (idx < a.size()){
            a.resize(idx + 1);
        }
        return a[idx];
    }

    

}

#endif
