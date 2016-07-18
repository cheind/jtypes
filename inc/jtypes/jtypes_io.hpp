/**
    This file is part of jtypes.

    Copyright(C) 2016 Christoph Heindl
    All rights reserved.

    This software may be modified and distributed under the terms
    of MIT license. See the LICENSE file for details.
*/

#ifndef JTYPES_IO_H
#define JTYPES_IO_H

#if defined(WIN32) || defined(_WIN32)
#pragma warning(push)
#pragma warning(disable: 4996) 
#endif

#include "json.hpp"

#if defined(WIN32) || defined(_WIN32)
#pragma warning(pop) 
#endif

#include "jtypes.hpp"

namespace jtypes {

    using json = nlohmann::json;

    namespace details {

        inline json to_json(const jtype &v, bool *should_discard = 0) {
            if (should_discard) *should_discard = false;
            
            switch(v.type()) {
                case jtype::vtype::array: {
                    bool discard = false;
                    json j = json::array();
                    for (auto && vv : v) {
                        json jj = to_json(vv, &discard);
                        if (!discard) j.push_back(jj);
                    }
                    return j;
                }
                case jtype::vtype::boolean:
                    return v.as<bool>();
                case jtype::vtype::signed_number:
                    return v.as<std::int64_t>();
                case jtype::vtype::unsigned_number:
                    return v.as<std::uint64_t>();
                case jtype::vtype::real_number:
                    return v.as<double>();
                case jtype::vtype::string:
                    return v.as<std::string>();
                case jtype::vtype::object: {
                    bool discard = false;
                    json j = json::object();
                    for (auto && k : v.keys()) {
                        json jj = to_json(v[k], &discard);
                        if (!discard) j[k.as<std::string>()] = jj;
                    }
                    return j;
                }
                case jtype::vtype::function:
                case jtype::vtype::undefined: {
                    if (should_discard) *should_discard = true;
                    return json();
                }
                case jtype::vtype::null: {
                    return nullptr;
                }                    
            }
            throw type_error("to_json() unexpected type.");
        }
        
        inline jtype from_json(const json &j) {
            switch (j.type()) {
                case json::value_t::array: {
                    jtype v = jtype::array_t();
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
                    jtype v = jtype::object_t();
                    for (auto iter = j.begin(); iter != j.end(); ++iter) {
                        v[iter.key()] = from_json(iter.value());
                    }
                    return v;
                }
                case json::value_t::string:
                    return j.get<std::string>();
                case json::value_t::discarded:
                    return jtype();
            }

            throw type_error("from_json() unexpected type.");
        }
    }
    
    inline std::string to_json(const jtype &v) {
        return details::to_json(v).dump();
    }
    
    inline std::string to_json(const jtype &v, int intend) {
        return details::to_json(v).dump(intend);
    }
    
    inline jtype from_json(const std::string &str) {
        try {
            json j = json::parse(str);
            return details::from_json(j);
        } catch (std::exception &e) {
            throw syntax_error(e.what());
        }
    }
    
    inline jtype from_json(std::istream &is) {
        try {
            json j = json::parse(is);
            return details::from_json(j);
        } catch (std::exception &e) {
            throw syntax_error(e.what());
        }
    }    
    
    inline std::ostream &operator<<(std::ostream &os, const jtype &v) {
        // use std::setw to format with intendation.
        os << details::to_json(v);
        return os;
    }
    
    inline std::istream &operator>>(std::istream &is, jtype &v) {
        v = from_json(is);
        return is;
    }
}

#endif
