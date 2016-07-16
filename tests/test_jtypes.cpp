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
        REQUIRE(x.type() == jtypes::vtype::undefined);
    }
    
    {
        jtypes::var x(nullptr);
        REQUIRE(x.is_null());
        REQUIRE(x.type() == jtypes::vtype::null);
    }
    
    {
        jtypes::var x(true);
        REQUIRE(x.is_boolean());
        REQUIRE(x.as<bool>() == true);
    }
    
    {
        jtypes::var x(false);
        REQUIRE(x.is_boolean());
        REQUIRE(x.type() == jtypes::vtype::boolean);
        REQUIRE(x.as<bool>() == false);
    }
    
    {
        jtypes::var x(0);
        REQUIRE(x.is_signed_number());
        REQUIRE(x.type() == jtypes::vtype::signed_number);
        REQUIRE(x == jtypes::var(0));
        REQUIRE(x.as<int>() == 0);
    }
    
    {
        jtypes::var x(3);
        REQUIRE(x.is_signed_number());
        REQUIRE(x.type() == jtypes::vtype::signed_number);
        REQUIRE(x.as<int>() == 3);
    }
    
    {
        jtypes::var x(-3);
        REQUIRE(x.is_signed_number());
        REQUIRE(x.as<int>() == -3);
    }

    {
        jtypes::var x(3u);
        REQUIRE(x.is_unsigned_number());
        REQUIRE(x.as<int>() == 3);
    }
    
    {
        jtypes::var x(3.f);
        REQUIRE(x.is_real_number());
        REQUIRE(x.type() == jtypes::vtype::real_number);
        REQUIRE(x.as<float>() == 3.f);
    }
    
    {
        jtypes::var x(3.0);
        REQUIRE(x.is_real_number());
        REQUIRE(x.as<float>() == 3.f);
    }
    
    {
        jtypes::var x('x');
        REQUIRE(x.is_string());
        REQUIRE(x.type() == jtypes::vtype::string);
        REQUIRE(x.as<std::string>() == "x");
    }
    
    {
        jtypes::var x("hello world");
        REQUIRE(x.is_string());
        REQUIRE(x.as<std::string>() == "hello world");
    }
    
    {
        jtypes::var x(std::string("hello world"));
        REQUIRE(x.is_string());
        REQUIRE(x.as<std::string>() == "hello world");
    }
}

TEST_CASE("jtypes can be assigned from simple types")
{
    jtypes::var x;
    
    x = nullptr;
    REQUIRE(x.is_null());
    
    x = true;
    REQUIRE(x.is_boolean());
    REQUIRE(x.as<bool>() == true);
    REQUIRE(x == true);
    
    x = 0;
    REQUIRE(x.is_signed_number());
    REQUIRE(x == 0);
    
    x = 10;
    REQUIRE(x.is_signed_number());
    REQUIRE(x == 10);
    
    x = -3;
    REQUIRE(x.is_signed_number());
    REQUIRE(x == -3);
    
    x = 3u;
    REQUIRE(x.is_unsigned_number());
    REQUIRE(x == 3u);

    x = 3.f;
    REQUIRE(x.is_real_number());
    REQUIRE(x == 3.0);
    
    x = 'a';
    REQUIRE(x.is_string());
    REQUIRE(x == "a");
    
    x = "hello";
    REQUIRE(x.is_string());
    REQUIRE(x == "hello");
    
    x = std::string("hello");
    REQUIRE(x.is_string());
    REQUIRE(x == "hello");
    REQUIRE(x != "hdaello");
}

std::string free_func(const std::string &str) {
    return str;
}

struct functor {
    std::string operator()(const std::string &str) const {
        return str;
    }

    std::string moveable(std::string &&str) const {
        return std::move(str);
    }
};

TEST_CASE("jtypes can be initialized from callables") 
{
    using jtypes::fnc;

    using sig = std::string(const std::string&);
    
    {
        jtypes::var x(fnc<sig>(&free_func));
        REQUIRE(x.is_function());
        REQUIRE(x.type() == jtypes::vtype::function);
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        functor f;
        jtypes::var x(fnc<sig>(f));
        REQUIRE(x.is_function());
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        jtypes::var x(fnc<sig>([](const std::string &str) { return str; }));
        REQUIRE(x.is_function());
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        using namespace std::placeholders;
        functor f;
        auto b = std::bind(&functor::operator(), f, _1);

        jtypes::var x(fnc<sig>(b));
        REQUIRE(x.is_function());
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        using namespace std::placeholders;
        
        using msig = std::string(std::string&&);

        functor f;
        auto b = std::bind(&functor::moveable, f, _1);

        std::string input = "hello world";
        jtypes::var x(fnc<msig>(b));
        REQUIRE(x.invoke<msig>(std::move(input)) == "hello world");
        REQUIRE(input.empty());
    }

   
}

TEST_CASE("jtypes allows extracting function objects")
{
    
    using jtypes::fnc;
    using sig = int(int, int);
   
    jtypes::var x;    
    x = fnc<sig>([](int x, int y) { return x + y; });
    
    
    std::function<sig> f = x.as<sig>();
    REQUIRE(f(1, 2) == 3);
    
    using invalid_sig = int(void);
    REQUIRE_THROWS_AS(x.as<invalid_sig>(), jtypes::bad_access);
}

TEST_CASE("jtypes can be assigned from callables")
{
    
    using jtypes::fnc;
    using sig = int(int, int);
    
    jtypes::var x;
    x = fnc<sig>([](int x, int y) { return x + y; });
    REQUIRE(x.is_function());
    REQUIRE(x.type() == jtypes::vtype::function);
    REQUIRE(x.invoke<sig>(1, 2) == 3);
}

TEST_CASE("jtypes can be initialized from arrays")
{
    using jtypes::arr;
    using jtypes::obj;

    {
        jtypes::var x = arr({ 1,2,3 });
        REQUIRE(x.is_array());
        REQUIRE(x.type() == jtypes::vtype::array);
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == 2);
        REQUIRE(x[2] == 3);
    }

    {
        jtypes::var x = arr({ 1u,"hello world", true });
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1u);
        REQUIRE(x[1] == "hello world");
        REQUIRE(x[2] == true);
    }
    
    {
        std::vector<int> v = {1, 2, 3};
        jtypes::var x = arr(v.begin(), v.end());
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == 2);
        REQUIRE(x[2] == 3);
    }
}

TEST_CASE("jtypes can be assigned from arrays")
{
    using jtypes::arr;
    using jtypes::obj;

    {
        jtypes::var x;
        x = arr({1,2,3});
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == 2);
        REQUIRE(x[2] == 3);
    }

    {
        jtypes::var x;
        x = arr({ 1, true, "hello" });
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == true);
        REQUIRE(x[2] == "hello");
    }
}

TEST_CASE("jtypes can be initialized from dictionaries")
{
    using jtypes::arr;
    using jtypes::obj;

    {
        jtypes::var x = obj({
            {"a", 10},
            {"b", "hello world"}
        });
        REQUIRE(x.is_object());
        REQUIRE(x.type() == jtypes::vtype::object);
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
    }
    
    {
        jtypes::var x = obj({
            {"a", jtypes::var(10)},
            {"b", jtypes::var("hello world")},
            {"c", jtypes::var(nullptr)}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
        REQUIRE(x["c"] == nullptr);
    }
    
    {
        jtypes::var x = obj({
            {"a", 10},
            {"b", obj({{"c", "hello world"}})}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"]["c"] == "hello world");
    }
}

TEST_CASE("jtypes can be assigned from dictionaries")
{
    using jtypes::arr;
    using jtypes::obj;

    {
        jtypes::var x;
        x = obj({
            {"a", 10},
            {"b", "hello world"}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
    }
    
    {
        jtypes::var x;
        x = obj({
            {"a", jtypes::var(10)},
            {"b", jtypes::var("hello world")},
            {"c", jtypes::var(nullptr)}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
        REQUIRE(x["c"] == nullptr);
    }
}

TEST_CASE("jtypes should be convertible to primitive types")
{
    using jtypes::arr;
    using jtypes::obj;
    using jtypes::fnc;
    
    // boolean

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

    using sig = bool(void);
    REQUIRE(jtypes::var(fnc<sig>([]() {return true; })));
    REQUIRE(!jtypes::var(fnc<sig>()));
    
    REQUIRE(jtypes::var(arr({1,2,3})));
    REQUIRE(jtypes::var(obj({{"a", 10}})));
    
    // ints and floats
    
    REQUIRE(int(jtypes::var(1)) == 1);
    REQUIRE(uint(jtypes::var(1)) == 1u);
    REQUIRE(float(jtypes::var(1)) == 1.f);
    
}

TEST_CASE("jtypes should handle coercion to integral types")
{
    using jtypes::arr;
    using jtypes::obj;

    REQUIRE_THROWS_AS(jtypes::var().as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(nullptr).as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(arr({1,2,3})).as<int>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(obj({{"a",1}})).as<int>(), jtypes::bad_access);
    
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
    using jtypes::arr;
    using jtypes::obj;

    REQUIRE_THROWS_AS(jtypes::var().as<float>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(nullptr).as<float>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(arr({1,2,3})).as<double>(), jtypes::bad_access);
    REQUIRE_THROWS_AS(jtypes::var(obj({{"a",1}})).as<float>(), jtypes::bad_access);
    
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
    using jtypes::fnc;
    using jtypes::arr;
    using jtypes::obj;

    REQUIRE(jtypes::var().as<std::string>() == "undefined");
    REQUIRE(jtypes::var(nullptr).as<std::string>() == "null");
    REQUIRE(jtypes::var(true).as<std::string>() == "true");
    REQUIRE(jtypes::var(false).as<std::string>() == "false");;
    REQUIRE(jtypes::var(-2).as<std::string>() == "-2");
    REQUIRE(jtypes::var(arr({1,2,3})).as<std::string>() == "1,2,3");
    REQUIRE(jtypes::var(arr({'a','b','c'})).as<std::string>() == "a,b,c");
    REQUIRE(jtypes::var("hello world!").as<std::string>() == "hello world!");
    REQUIRE(jtypes::var(obj({{"a", "x"}})).as<std::string>() == "object");

    using sig = int(void);
    REQUIRE(jtypes::var(fnc<sig>([]() {return -1;})).as<std::string>() == "function");
    
}

TEST_CASE("jtypes key and values should be iterable")
{
    using jtypes::arr;
    using jtypes::obj;

    jtypes::var x = obj({
        {"a", 2},
        {"b", "hello world"},
        {"c", true}
    });
    
    jtypes::var keys = x.keys();
    jtypes::var values = x.values();
    
    REQUIRE(keys == arr({"a", "b", "c"}));
    REQUIRE(values == arr({2, "hello world", true}));
    
    // holds also for arrays
    
    x = arr({"a", "b", "c"});
    keys = x.keys();
    values = x.values();
    
    REQUIRE(keys == arr({0, 1, 2}));
    REQUIRE(values == arr({"a", "b", "c"}));
}



TEST_CASE("jtypes should allow nested object creation")
{
    jtypes::var x;
    
    jtypes::create_path(x, "first.number", jtypes::var(3));
    jtypes::create_path(x, "first.string", jtypes::var("hello world"));
    
    REQUIRE(x.is_object());
    REQUIRE(x["first"]["number"].as<int>() == 3);
    REQUIRE(x["first"]["string"].as<std::string>() == "hello world");
    
    // const
    
    const jtypes::var &xx = x;
    REQUIRE(xx.is_object());
    REQUIRE(xx["first"]["number"].as<int>() == 3);
}


TEST_CASE("jtypes should allow on the array creation")
{
    jtypes::var x = jtypes::arr();
    
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

    x = jtypes::arr();
    x.push_back(10);
    x.push_back("hello world");
    x.push_back(true);

    REQUIRE(x[0] == 10);
    REQUIRE(x[1] == "hello world");
    REQUIRE(x[2] == true);
    
}

TEST_CASE("jtypes should support default values")
{
    using jtypes::arr;
    using jtypes::obj;

    jtypes::var x = obj({
        {"a", 2},
        {"b", "hello world"},
        {"c", true}
    });
    
    REQUIRE((x["a"] | "not-here") == jtypes::var(2));
    REQUIRE((x["b"] | "not-here") == jtypes::var("hello world"));
    REQUIRE((x["d"] | "not-here") == jtypes::var("not-here"));
}


TEST_CASE("jtypes should support equality comparison")
{
    using jtypes::arr;
    using jtypes::obj;
    using jtypes::fnc;

    // Numbers 

    REQUIRE(jtypes::var(3) == 3);
    REQUIRE(jtypes::var(-3) == -3);
    REQUIRE(jtypes::var(3) == 3u);
    REQUIRE(jtypes::var(3) == 3.0);
    REQUIRE(jtypes::var(-3) == -3.0);
    REQUIRE(jtypes::var(3) == 3.f);

    REQUIRE(jtypes::var(3u) == 3);
    REQUIRE(jtypes::var(3u) == 3u);
    REQUIRE(jtypes::var(3u) == 3.0);
    REQUIRE(jtypes::var(3u) == 3.f);

    REQUIRE(jtypes::var(3.0) == 3);
    REQUIRE(jtypes::var(3.0) == 3u);
    REQUIRE(jtypes::var(3.0) == 3.0);
    REQUIRE(jtypes::var(-3.0) == -3.0);
    REQUIRE(jtypes::var(3.0) == 3.f);
    REQUIRE(jtypes::var(-3.0) == -3);

    REQUIRE(jtypes::var(3) != 2);
    REQUIRE(jtypes::var(3) != 2u);
    REQUIRE(jtypes::var(-3) != 2u);
    REQUIRE(jtypes::var(3) != 2.0);
    REQUIRE(jtypes::var(3) != 2.f);

    REQUIRE(jtypes::var(3u) != 2);
    REQUIRE(jtypes::var(3u) != -2);
    REQUIRE(jtypes::var(3u) != 2u);
    REQUIRE(jtypes::var(3u) != 2.0);
    REQUIRE(jtypes::var(3u) != 2.f);

    REQUIRE(jtypes::var(3.0) != 2);
    REQUIRE(jtypes::var(3.0) != 2u);
    REQUIRE(jtypes::var(-3.0) != 2);
    REQUIRE(jtypes::var(-3.0) != 2u);
    REQUIRE(jtypes::var(3.0) != 2.0);
    REQUIRE(jtypes::var(3.0) != 2.f);

    // Arrays

    REQUIRE(jtypes::var(arr({1u, 2u, 3u})) == jtypes::var(arr({1, 2, 3})));

    // Objects

    jtypes::var o1 = obj({ {"a", "hello world"}, {"b", obj({{"c", 3}})} });
    jtypes::var o2 = obj({ { "a", "hello world" },{ "b", obj({ { "c", 3 } }) } });
    jtypes::var o3 = obj({ { "a", "hello world" },{ "b", obj({ { "c", 4 } }) } });

    REQUIRE(o1 == o2);
    REQUIRE(o1 != o3);

    jtypes::var o4 = o3;
    REQUIRE(o4 == o3);
    o4["a"] = "abc";
    REQUIRE(o4 != o3);
    
    // Functions
    
    jtypes::var f1 = fnc<bool(void)>([](){return true;});
    jtypes::var f2 = fnc<bool(void)>([](){return true;});
    jtypes::var f3 = f1;
    
    REQUIRE(f1 != f2);
    REQUIRE(f1 == f3);
}

TEST_CASE("jtypes should support less-than comparison")
{
    using jtypes::arr;
    using jtypes::obj;

    REQUIRE(jtypes::var(3) <= 3);
    REQUIRE(jtypes::var(3) <= 3u);
    REQUIRE(jtypes::var(3) <= 3.0);
    REQUIRE(jtypes::var(3) >= 3.0);
    
    REQUIRE(jtypes::var(3) < 4);
    REQUIRE(jtypes::var(-3) < 3u);
    REQUIRE(jtypes::var(3) < 4.0);

    // Strict weak ordering

    // Irreflexivity
    REQUIRE((jtypes::var(3) < jtypes::var(3)) == false);
    REQUIRE((jtypes::var(3.5) < jtypes::var(3.5)) == false);

    // Antisymmetry
    REQUIRE((jtypes::var(3) < jtypes::var(3.5)) == true);
    REQUIRE((jtypes::var(3.5) < jtypes::var(3)) == false);

    //Transitivity
    REQUIRE((jtypes::var(3) < jtypes::var(3.5)) == true);
    REQUIRE((jtypes::var(3.5) < jtypes::var(4)) == true);
    REQUIRE((jtypes::var(3) < jtypes::var(4)) == true);

    std::less<jtypes::var> less;
    REQUIRE(less(jtypes::var(2u), jtypes::var(3.5)));
    REQUIRE(!less(jtypes::var(3.5), jtypes::var(2u)));
}

TEST_CASE("jtypes should support JSON")
{
    jtypes::var x = jtypes::obj({
        {"a", 2.1},
        {"b", "hello world"},
        {"c", true},
        {"d", jtypes::obj({{"e", nullptr}})},
        {"f", jtypes::arr({1,2,3,"hello"})}
    });
    
    REQUIRE(jtypes::to_json(x) == R"({"a":2.1,"b":"hello world","c":true,"d":{"e":null},"f":[1,2,3,"hello"]})");
    REQUIRE(jtypes::from_json(jtypes::to_json(x)) == x);
    
    // Overloaded output / input operators
    
    const std::string s = jtypes::to_json(x);
    
    std::istringstream iss(s);
    jtypes::var y;
    
    REQUIRE_NOTHROW(iss >> y);
    REQUIRE(x == y);
    
    std::ostringstream oss;
    oss << y;
    REQUIRE(s == oss.str());
    
    // Invalid JSON
    REQUIRE_THROWS_AS(jtypes::from_json("dasda"), jtypes::syntax_error);
    
    
    // Undefined and function objects are stripped when converted to json.
    jtypes::var u = jtypes::obj({
        {"a", jtypes::var()},
        {"b", jtypes::arr({1,jtypes::var(),2})},
        {"c", jtypes::fnc<int(int)>()}
    });

    std::stringstream str;
    str << u;
    
    jtypes::var parsed;
    str >> parsed;

    REQUIRE(parsed == jtypes::obj({
        {"b", jtypes::arr({1,2}) }
    }));

}

#include <iterator>

using namespace jtypes;

using mapbox::util::variant;



TEST_CASE("jtypes should support iterators")
{
    
    
    jtypes::var x = jtypes::obj({
        {"a", 2.1},
        {"b", "hello world"},
        {"c", true},
        {"d", jtypes::obj({{"e", nullptr}})},
        {"f", jtypes::arr({2,1,3,"hello"})}
    });
    
    {
        // Object iteration
        auto i = x.begin();
        auto end = x.end();
        
        REQUIRE(std::distance(i, end) == 5);
        
        REQUIRE(*i == 2.1); REQUIRE(i.key() == "a"); ++i;
        REQUIRE(*i == "hello world"); REQUIRE(i.key() == "b"); ++i;
        REQUIRE(*i == true); REQUIRE(i.key() == "c"); ++i;
        REQUIRE(*i == jtypes::obj({{"e", nullptr}})); REQUIRE(i.key() == "d"); ++i;
        REQUIRE(*i == jtypes::arr({2,1,3,"hello"})); REQUIRE(i.key() == "f"); ++i;
        REQUIRE(i == end);
    }
    
    {
        // Array iteration
        
        auto i = x["f"].begin();
        auto end = x["f"].end();
    
        REQUIRE(*i == 2); REQUIRE(i.key() == 0); ++i;
        REQUIRE(*i == 1); REQUIRE(i.key() == 1); ++i;
        REQUIRE(*i == 3); REQUIRE(i.key() == 2); ++i;
        REQUIRE(*i == "hello"); REQUIRE(i.key() == 3); ++i;
        REQUIRE(i == end);
    }
    
    {
        // Range iteration
        
        jtypes::var received = jtypes::arr();
        for (auto v : x) {
            received.push_back(v);
        }
        
        REQUIRE(received == jtypes::arr({2.1, "hello world", true, jtypes::obj({{"e", nullptr}}), jtypes::arr({2,1,3,"hello"})}));
        
        
        const jtypes::var &xx = x;
        received = jtypes::arr();
        for (auto v : xx["f"]) {
            received.push_back(v);
        }
        
        REQUIRE(received == jtypes::arr({2,1,3,"hello"}));
    }
    
    {
        // Manipulation
        
        jtypes::var x = arr({ 5, 10, 6, 20, 7 });
        
        jtypes::var y = jtypes::arr();
        std::transform(x.begin(), x.end(), std::back_inserter(y), [](const var &v) {return (int)v + 1;});
        REQUIRE(y == arr({ 6, 11, 7, 21, 8 }));
    
        
        auto last = std::partition(x.begin(), x.end(), [](const var &i) { return i < 10;});
        
        
        for (auto i = x.begin(); i != last; ++i) {
            REQUIRE(*i < 10);
        }
        
        for (auto i = last; i != x.end(); ++i) {
            REQUIRE(*i >= 10);
        }
    }
    
    {
        jtypes::var x = 1;
        REQUIRE_THROWS_AS(x.begin(), jtypes::type_error);
        REQUIRE_THROWS_AS(x.end(), jtypes::type_error);
    }
}

TEST_CASE("jtypes should support merging")
{
    jtypes::var opts = jtypes::obj({
        {"a", 2.1},
        {"b", "hello world"},
        {"c", true},
        {"d", jtypes::obj({{"e", nullptr}})},
        {"f", jtypes::arr({2,1,3,"hello"})}
    }).merge_from(jtypes::obj({
        {"a", 3},
        {"x", "wuff"},
        {"d", jtypes::obj({{"d", 1}})},
        {"f", jtypes::arr({10,11})}
    }));
    
    jtypes::var expected = jtypes::obj({
        {"a", 3},
        {"b", "hello world"},
        {"c", true},
        {"d", jtypes::obj({{"e", nullptr}, {"d", 1}})},
        {"f", jtypes::arr({10,11})},
        {"x", "wuff"}
    });
    
    REQUIRE(opts == expected);
}

TEST_CASE("jtypes should support split")
{
    jtypes::var s = "a.b.c";
    REQUIRE(s.split('.') == jtypes::arr({"a", "b", "c"}));
}

TEST_CASE("jtypes behaviour should mimic ECMAScript 5 behaviour")
{
    jtypes::var x;
    REQUIRE(x.is_undefined());
    
    
    REQUIRE_THROWS_AS(x.invoke<void(void)>(), jtypes::type_error);
}



