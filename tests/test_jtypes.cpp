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
        REQUIRE(x.is_undefined());
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
        jtypes::var x('x');
        REQUIRE(x.is_string());
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

TEST_CASE("jtypes can be assigned from simple types")
{
    jtypes::var x;
    
    x = nullptr;
    REQUIRE(x.is_null());
    
    x = true;
    REQUIRE(x.is_boolean());
    
    x = 0;
    REQUIRE(x.is_signed_integral());
    
    x = 10;
    REQUIRE(x.is_signed_integral());
    
    x = -3;
    REQUIRE(x.is_signed_integral());
    
    x = 3u;
    REQUIRE(x.is_unsigned_integral());

    x = 3.f;
    REQUIRE(x.is_real());
    
    x = 'a';
    REQUIRE(x.is_string());
    
    x = "hello";
    REQUIRE(x.is_string());
    
    x = std::string("hello");
    REQUIRE(x.is_string());
}


int func(int x, int y, int z) { return x + y + z; }
int overloaded(char x, int y, int z) { return x + y + z; }
int overloaded(int x, int y, int z) { return x + y + z; }
struct functionoid {
    int operator()(int x, int y, int z) { return x + y + z; }
    int const_call(int x) const { return x;}
};
struct functionoid_overload {
    int operator()(int x, int y, int z) { return x + y + z; }
    int operator()(char x, int y, int z) { return x + y + z; }
};


TEST_CASE("jtypes can be initialized from callables") 
{
    using jtypes::functions::fnc;
    
    {
        jtypes::var x(fnc(func));
        REQUIRE(x.is_function());
    }

    {
        jtypes::var x(fnc<int, int, int>(overloaded));
        REQUIRE(x.is_function());
    }

    {
        jtypes::var x(fnc<char, int, int>(overloaded));
        REQUIRE(x.is_function());
    }

    {
        functionoid o;
        jtypes::var x(fnc(o));
        REQUIRE(x.is_function());
    }

    {
        using namespace std::placeholders;
        functionoid o;
        jtypes::var x(fnc<int>(std::bind(&functionoid::const_call, o, _1)));
        REQUIRE(x.is_function());
    }

    {
        jtypes::var x(fnc([](int x, int y) { return x + y; }));
        REQUIRE(x.is_function());
    }

    {
        functionoid f;
        std::function<int(int, int, int)> ff = f;
        jtypes::var x(fnc(ff));
        REQUIRE(x.is_function());
    }


    {
        jtypes::var x = fnc([](int a, int b) {return a + b; });
        REQUIRE(x.invoke<int>(3, 4) == 7);
    }
}

TEST_CASE("jtypes can be assigned from callables")
{
    using jtypes::functions::fnc;
    
    jtypes::var x;
    
    x = fnc([](int x, int y) { return x + y; });
    REQUIRE(x.is_function());
    
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

TEST_CASE("jtypes can be assigned from arrays")
{
    {
        jtypes::var x;
        x = {1,2,3};
        REQUIRE(x.is_array());
    }
    
    {
        jtypes::var x;
        x = {jtypes::var(1),jtypes::var(2),jtypes::var(3)};
        REQUIRE(x.is_array());
    }
    
    {
        std::vector<int> v = {1, 2, 3};
        jtypes::var x;
        x = v;
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
            {"b", jtypes::var("hello world")},
            {"c", jtypes::var(nullptr)}
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

TEST_CASE("jtypes can be assigned from dictionaries")
{
    {
        jtypes::var x;
        x = {
            {"a", 10},
            {"b", "hello world"}
        };
        REQUIRE(x.is_object());
    }
    
    {
        jtypes::var x;
        x = {
            {"a", jtypes::var(10)},
            {"b", jtypes::var("hello world")},
            {"c", jtypes::var(nullptr)}
        };
        REQUIRE(x.is_object());
    }
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

    REQUIRE(jtypes::var(jtypes::functions::fnc([]() {return true; })));
    
    REQUIRE(jtypes::var({1,2,3}));
    REQUIRE(jtypes::var({{"a", 10}}));
}

TEST_CASE("jtypes should handle coercion to integral types")
{
    
    REQUIRE_THROWS_AS(jtypes::var().as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(nullptr).as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var({1,2,3}).as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var({{"a",1}}).as<int>(), jtypes::bad_access);
    
    REQUIRE(jtypes::var(true).as<int>() == 1);
    REQUIRE(jtypes::var(false).as<int>() == 0);
    
    REQUIRE(jtypes::var(5).as<int>() == 5);
    REQUIRE(jtypes::var(5u).as<int>() == 5);
    REQUIRE(jtypes::var(5.5).as<int>() == 5);
    
    REQUIRE(jtypes::var(5).as<long>() == 5);
    REQUIRE(jtypes::var(5.5).as<char>() == (char)5);
    
    REQUIRE(jtypes::var("5").as<long>() == 5);
    REQUIRE(jtypes::var("5.5").as<char>() == (char)5);
    
}

TEST_CASE("jtypes should handle coercion to floating point types")
{
    
    REQUIRE_THROWS_AS(jtypes::var().as<float>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(nullptr).as<float>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var({1,2,3}).as<double>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var({{"a",1}}).as<float>(), jtypes::bad_access);
    
    REQUIRE(jtypes::var(true).as<float>() == 1.f);
    REQUIRE(jtypes::var(false).as<float>() == 0.f);
    
    REQUIRE(jtypes::var(5).as<float>() == 5.f);
    REQUIRE(jtypes::var(5u).as<float>() == 5.f);
    REQUIRE(jtypes::var(5.5).as<float>() == 5.5f);
    REQUIRE(jtypes::var(5).as<double>() == 5.0);
    
    REQUIRE(jtypes::var("5").as<double>() == 5.0);
    REQUIRE(jtypes::var("-5.5").as<double>() == -5.5);
}

TEST_CASE("jtypes should handle coercion to string")
{
    using jtypes::functions::fnc;

    REQUIRE(jtypes::var().as<std::string>() == "undefined");
    REQUIRE(jtypes::var(nullptr).as<std::string>() == "null");
    REQUIRE(jtypes::var(true).as<std::string>() == "true");
    REQUIRE(jtypes::var(false).as<std::string>() == "false");;
    REQUIRE(jtypes::var(-2).as<std::string>() == "-2");
    REQUIRE(jtypes::var({1,2,3}).as<std::string>() == "1,2,3");
    REQUIRE(jtypes::var({'a','b','c'}).as<std::string>() == "a,b,c");
    REQUIRE(jtypes::var("hello world!").as<std::string>() == "hello world!");
    REQUIRE(jtypes::var({{"a", "x"}}).as<std::string>() == "object");
    REQUIRE(jtypes::var(fnc([]() {return -1;})).as<std::string>() == "function");
    
}

TEST_CASE("jtypes key and values should be iterable")
{
    jtypes::var x = {
        {"a", 2},
        {"b", "hello world"},
        {"c", true}
    };
    
    jtypes::var keys = x.keys();
    jtypes::var values = x.values();
    
    REQUIRE(keys == jtypes::var({"a", "b", "c"}));
    REQUIRE(values == jtypes::var({2, "hello world", true}));
    
    // holds also for arrays
    
    x = jtypes::var({"a", "b", "c"});
    keys = x.keys();
    values = x.values();
    
    REQUIRE(keys == jtypes::var({"0", "1", "2"}));
    REQUIRE(values == jtypes::var({"a", "b", "c"}));
}


TEST_CASE("jtypes should allow nested object creation")
{
    jtypes::var x; // undefined
    
    x["first"]["number"] = jtypes::var(3);
    x["first"]["string"] = jtypes::var("hello world");
    
    REQUIRE(x.is_object());
    REQUIRE(x["first"]["number"].as<int>() == 3);
    
    // const
    
    const jtypes::var &xx = x;
    REQUIRE(xx.is_object());
    REQUIRE(xx["first"]["number"].as<int>() == 3);
}

TEST_CASE("jtypes should allow on the fly array creation")
{
    jtypes::var x; // undefined
    
    x[0] = jtypes::var("a");
    x[3] = "b";
    
    REQUIRE(x.is_array());
    REQUIRE(x[0].as<std::string>() == "a");
    REQUIRE(x[1].as<std::string>() == "undefined");
    REQUIRE(x[2].as<std::string>() == "undefined");
    REQUIRE(x[3].as<std::string>() == "b");
    
    // const ref
    const jtypes::var &xx = x;
    REQUIRE(xx.is_array());
    REQUIRE(xx[0].as<std::string>() == "a");
    REQUIRE(xx[1].as<std::string>() == "undefined");
    REQUIRE(xx[2].as<std::string>() == "undefined");
    REQUIRE(xx[3].as<std::string>() == "b");
    
}

TEST_CASE("jtypes should support default values")
{
    jtypes::var x = {
        {"a", 2},
        {"b", "hello world"},
        {"c", true}
    };
    
    REQUIRE((x["a"] | "not-here") == jtypes::var(2));
    REQUIRE((x["b"] | "not-here") == jtypes::var("hello world"));
    REQUIRE((x["d"] | "not-here") == jtypes::var("not-here"));
}


