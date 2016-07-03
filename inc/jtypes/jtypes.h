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
    
    class object {
    public:
        std::unordered_map<std::string, value> m;
    };
    
    class array {
    public:
        std::vector<value> a;
        
        array() = default;
        
        template<class T>
        array(const std::initializer_list<T> &t) {
            for (auto && v : t) {
                a.push_back(value(v));
            }
        }
        
        array(const std::initializer_list<value> &t) {
            a = t;
        }
        
    };
    
    
    class value {
    public:
        typedef variant<int, std::string, object, array> var;
        var v;
        
        value() = default;
        
        value(var && v_)
        :v(std::move(v_)) {}
    };
    

    
    
    

    

    
}

#endif
