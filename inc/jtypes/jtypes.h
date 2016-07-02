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
    
    
    typedef variant<int, std::string, object, array> my_variant;
    
    class object {
    public:
        std::unordered_map<std::string, my_variant> m;
    };
    
    class array {
    public:
        std::vector<my_variant> a;
        
        array() = default;
        
        template<class T>
        array(const std::initializer_list<T> &t) {
            for (auto && v : t) {
                a.push_back(my_variant(v));
            }
        }
        
        array(const std::initializer_list<my_variant> &t) {
            a = t;
        }
        
    };
    
}

#endif
