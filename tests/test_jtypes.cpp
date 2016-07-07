/**
This file is part of jtypes.

Copyright(C) 2016 Christoph Heindl
All rights reserved.

This software may be modified and distributed under the terms
of MIT license. See the LICENSE file for details.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <jtypes/jtypes.h>

TEST_CASE("jtypes can be initialized from simple types")
{
    {
        jtypes::var x;
        REQUIRE(x.is_null());
    }
    
    {
        jtypes::var x(nullptr);
        REQUIRE(x.is_null());
    }
    
    {
        jtypes::var x(true);
        REQUIRE(x.is_boolean());
    }
    
    {
        jtypes::var x(false);
        REQUIRE(x.is_boolean());
    }
    
    {
        jtypes::var x(0);
        REQUIRE(x.is_signed_integral());
    }
    
    {
        jtypes::var x(3);
        REQUIRE(x.is_signed_integral());
    }
    
    {
        jtypes::var x(-3);
        REQUIRE(x.is_signed_integral());
    }

    {
        jtypes::var x(3u);
        REQUIRE(x.is_unsigned_integral());
    }
    
    {
        jtypes::var x(3.f);
        REQUIRE(x.is_real());
    }
    
    {
        jtypes::var x(3.0);
        REQUIRE(x.is_real());
    }
    
    {
        jtypes::var x("hello world");
        REQUIRE(x.is_string());
    }
    
    {
        jtypes::var x(std::string("hello world"));
        REQUIRE(x.is_string());
    }
}

TEST_CASE("jtypes can be initialized from arrays")
{
    {
        jtypes::var x({1,2,3});
        REQUIRE(x.is_array());
    }
    
    {
        jtypes::var x({jtypes::var(1),jtypes::var(2),jtypes::var(3)});
        REQUIRE(x.is_array());
    }
    
    {
        std::vector<int> v = {1, 2, 3};
        jtypes::var x(v);
        REQUIRE(x.is_array());
    }
    
    {
        jtypes::array_t v = {1, 2, 3};
        jtypes::var x(v);
        REQUIRE(x.is_array());
    }
}

TEST_CASE("jtypes can be initialized from dictionaries")
{
    {
        jtypes::var x = {
            {"a", 10},
            {"b", "hello world"}
        };
        REQUIRE(x.is_object());
    }
    
    {
        jtypes::var x = {
            {"a", jtypes::var(10)},
            {"b", jtypes::var("hello world")}
        };
        REQUIRE(x.is_object());
    }
    
    {
        jtypes::var x = {
            {"a", 10},
            {"b", {{"c", "hello world"}}}
        };
        REQUIRE(x.is_object());
    }
}

TEST_CASE("jtypes can fetch values without coercion")
{
    
    jtypes::var x = {
        {"a", 10},
        {"b", "hello world"}
    };

}

TEST_CASE("jtypes should be convertible to bool")
{
    REQUIRE(!jtypes::var());
    REQUIRE(!jtypes::var(nullptr));
    
    REQUIRE(jtypes::var(true));
    REQUIRE(!jtypes::var(false));
    
    REQUIRE(jtypes::var(1));
    REQUIRE(!jtypes::var(0));
    REQUIRE(jtypes::var(1.0));
    REQUIRE(!jtypes::var(0.0));
    
    REQUIRE(jtypes::var("abc"));
    REQUIRE(!jtypes::var(""));
    
    REQUIRE(jtypes::var({1,2,3}));
    REQUIRE(jtypes::var({{"a", 10}}));
}

TEST_CASE("jtypes should handle coercion to integral types")
{
    
    REQUIRE_THROWS_AS(jtypes::var().as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var({1,2,3}).as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var({{"a",1}}).as<int>(), jtypes::bad_access);
    
    REQUIRE(jtypes::var(true).as<int>() == 1);
    REQUIRE(jtypes::var(false).as<int>() == 0);
    
    REQUIRE(jtypes::var(5).as<int>() == 5);
    REQUIRE(jtypes::var(5u).as<int>() == 5);
    REQUIRE(jtypes::var(5.5).as<int>() == 5);
    
    REQUIRE(jtypes::var(5).as<long>() == 5);
    REQUIRE(jtypes::var(5.5).as<char>() == (char)5);
}


