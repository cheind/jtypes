/**
This file is part of jtypes.

Copyright(C) 2016 Christoph Heindl
All rights reserved.

This software may be modified and distributed under the terms
of MIT license. See the LICENSE file for details.
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <jtypes/jtypes.hpp>
#include <jtypes/jtypes_io.hpp>

TEST_CASE("jtypes can be initialized from simple types")
{
    using jtypes::jtype;

    {
        jtype x;
        REQUIRE(x.is_undefined());
        REQUIRE(x.type() == jtype::vtype::undefined);
    }
    
    {
        jtype x(nullptr);
        REQUIRE(x.is_null());
        REQUIRE(x.type() == jtype::vtype::null);
    }
    
    {
        jtype x(true);
        REQUIRE(x.is_boolean());
        REQUIRE(x.as<bool>() == true);
    }
    
    {
        jtype x(false);
        REQUIRE(x.is_boolean());
        REQUIRE(x.type() == jtype::vtype::boolean);
        REQUIRE(x.as<bool>() == false);
    }
    
    {
        jtype x(0);
        REQUIRE(x.is_signed_number());
        REQUIRE(x.type() == jtype::vtype::signed_number);
        REQUIRE(x == jtype(0));
        REQUIRE(x.as<int>() == 0);
    }
    
    {
        jtype x(3);
        REQUIRE(x.is_signed_number());
        REQUIRE(x.type() == jtype::vtype::signed_number);
        REQUIRE(x.as<int>() == 3);
    }
    
    {
        jtype x(-3);
        REQUIRE(x.is_signed_number());
        REQUIRE(x.as<int>() == -3);
    }

    {
        jtype x(3u);
        REQUIRE(x.is_unsigned_number());
        REQUIRE(x.as<int>() == 3);
    }
    
    {
        jtype x(3.f);
        REQUIRE(x.is_real_number());
        REQUIRE(x.type() == jtype::vtype::real_number);
        REQUIRE(x.as<float>() == 3.f);
    }
    
    {
        jtype x(3.0);
        REQUIRE(x.is_real_number());
        REQUIRE(x.as<float>() == 3.f);
    }
    
    {
        jtype x('x');
        REQUIRE(x.is_string());
        REQUIRE(x.type() == jtype::vtype::string);
        REQUIRE(x.as<std::string>() == "x");
    }
    
    {
        jtype x("hello world");
        REQUIRE(x.is_string());
        REQUIRE(x.as<std::string>() == "hello world");
    }
    
    {
        jtype x(std::string("hello world"));
        REQUIRE(x.is_string());
        REQUIRE(x.as<std::string>() == "hello world");
    }
}

TEST_CASE("jtypes can be assigned from simple types")
{
    using jtypes::jtype;
    
    jtype x = nullptr;
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
    using jtypes::jtype;

    using sig = std::string(const std::string&);
    
    {
        jtype x(jtype::function<sig>(&free_func));
        REQUIRE(x.is_function());
        REQUIRE(x.type() == jtype::vtype::function);
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        functor f;
        jtype x(jtype::function<sig>(f));
        REQUIRE(x.is_function());
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        jtype x(jtype::function<sig>([](const std::string &str) { return str; }));
        REQUIRE(x.is_function());
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        using namespace std::placeholders;
        functor f;
        auto b = std::bind(&functor::operator(), f, _1);

        jtype x(jtype::function<sig>(b));
        REQUIRE(x.is_function());
        REQUIRE(x.invoke<sig>("hello world") == "hello world");
    }

    {
        using namespace std::placeholders;

        using msig = std::string(std::string&&);

        functor f;
        auto b = std::bind(&functor::moveable, f, _1);

        std::string input = "hello world";
        jtype x(jtype::function<msig>(b));
        REQUIRE(x.invoke<msig>(std::move(input)) == "hello world");
        REQUIRE(input.empty());
    }
}

TEST_CASE("jtypes allows extracting function objects")
{
    using jtypes::jtype;
    using sig = int(int, int);
   
    jtype x;    
    x = jtype::function<sig>([](int x, int y) { return x + y; });
    
    
    std::function<sig> f = x.as<sig>();
    REQUIRE(f(1, 2) == 3);
    
    using invalid_sig = int(void);
    REQUIRE_THROWS_AS(x.as<invalid_sig>(), jtypes::type_error);
}

TEST_CASE("jtypes can be assigned from callables")
{
    using jtypes::jtype;
    using sig = int(int, int);
    
    jtype x;
    x = jtype::function<sig>([](int x, int y) { return x + y; });
    REQUIRE(x.is_function());
    REQUIRE(x.type() == jtype::vtype::function);
    REQUIRE(x.invoke<sig>(1, 2) == 3);
}

TEST_CASE("jtypes can be initialized from arrays")
{
    using jtypes::jtype;

    {
        jtype x = jtype::array({ 1,2,3 });
        REQUIRE(x.is_array());
        REQUIRE(x.type() == jtype::vtype::array);
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == 2);
        REQUIRE(x[2] == 3);
    }

    {
        jtype x = jtype::array({ 1u,"hello world", true });
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1u);
        REQUIRE(x[1] == "hello world");
        REQUIRE(x[2] == true);
    }
    
    {
        std::vector<int> v = {1, 2, 3};
        jtype x = jtype::array(v.begin(), v.end());
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == 2);
        REQUIRE(x[2] == 3);
    }
}

TEST_CASE("jtypes can be assigned from arrays")
{
    using jtypes::jtype;

    {
        jtype x;
        x = jtype::array{1,2,3};
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == 2);
        REQUIRE(x[2] == 3);
    }

    {
        jtype x;
        x = jtype::array({ 1, true, "hello" });
        REQUIRE(x.is_array());
        REQUIRE(x[0] == 1);
        REQUIRE(x[1] == true);
        REQUIRE(x[2] == "hello");
    }
}

TEST_CASE("jtypes can be initialized from dictionaries")
{
    using jtypes::jtype;

    {
        jtype x = jtype::object({
            {"a", 10},
            {"b", "hello world"}
        });
        REQUIRE(x.is_object());
        REQUIRE(x.type() == jtype::vtype::object);
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
    }
    
    {
        jtype x = jtype::object({
            {"a", jtype(10)},
            {"b", jtype("hello world")},
            {"c", jtype(nullptr)}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
        REQUIRE(x["c"] == nullptr);
    }
    
    {
        jtype x = jtype::object({
            {"a", 10},
            {"b", jtype::object({{"c", "hello world"}})}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"]["c"] == "hello world");
    }
}

TEST_CASE("jtypes can be assigned from dictionaries")
{
    using jtypes::jtype;

    {
        jtype x;
        x = jtype::object({
            {"a", 10},
            {"b", "hello world"}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
    }
    
    {
        jtype x;
        x = jtype::object({
            {"a", jtype(10)},
            {"b", jtype("hello world")},
            {"c", jtype(nullptr)}
        });
        REQUIRE(x.is_object());
        REQUIRE(x["a"] == 10);
        REQUIRE(x["b"] == "hello world");
        REQUIRE(x["c"] == nullptr);
    }
}

TEST_CASE("jtypes should be convertible to primitive types")
{
    using jtypes::jtype;
    // boolean

    REQUIRE(!jtype());
    REQUIRE(!jtype(nullptr));
    
    REQUIRE(jtype(true));
    REQUIRE(!jtype(false));
    
    REQUIRE(jtype(1));
    REQUIRE(!jtype(0));
    REQUIRE(jtype(1.0));
    REQUIRE(!jtype(0.0));
    
    REQUIRE(jtype("abc"));
    REQUIRE(!jtype(""));

    using sig = bool(void);
    REQUIRE(jtype(jtype::function<sig>([]() {return true; })));
    REQUIRE(!jtype(jtype::function<sig>()));
    
    REQUIRE(jtype(jtype::array({1,2,3})));
    REQUIRE(jtype(jtype::object({{"a", 10}})));
    
    // ints and floats
    
    REQUIRE(int(jtype(1)) == 1);
    REQUIRE(std::uint32_t(jtype(1)) == 1u);
    REQUIRE(float(jtype(1)) == 1.f);
    
}

TEST_CASE("jtypes should handle coercion to integral types")
{
    using jtypes::jtype;

    REQUIRE_THROWS_AS(jtype().as<int>(), jtypes::type_error);
    REQUIRE_THROWS_AS(jtype(nullptr).as<int>(), jtypes::type_error);
    REQUIRE_THROWS_AS(jtype(jtype::array{1,2,3}).as<int>(), jtypes::type_error);
    REQUIRE_THROWS_AS(jtype(jtype::object({{"a",1}})).as<int>(), jtypes::type_error);
    
    REQUIRE(jtype(true).as<int>() == 1);
    REQUIRE(jtype(false).as<int>() == 0);
    
    REQUIRE(jtype(5).as<int>() == 5);
    REQUIRE(jtype(5u).as<int>() == 5);
    REQUIRE(jtype(5.5).as<int>() == 5);
    
    REQUIRE(jtype(5).as<long>() == 5);
    REQUIRE(jtype(5.5).as<char>() == (char)5);
    
    REQUIRE(jtype("5").as<long>() == 5);
    REQUIRE(jtype("5.5").as<char>() == (char)5);
    
}

TEST_CASE("jtypes should handle coercion to floating point types")
{
    using jtypes::jtype;

    REQUIRE_THROWS_AS(jtype().as<float>(), jtypes::type_error);
    REQUIRE_THROWS_AS(jtype(nullptr).as<float>(), jtypes::type_error);
    REQUIRE_THROWS_AS(jtype(jtype::array({1,2,3})).as<double>(), jtypes::type_error);
    REQUIRE_THROWS_AS(jtype(jtype::object({{"a",1}})).as<float>(), jtypes::type_error);
    
    REQUIRE(jtype(true).as<float>() == 1.f);
    REQUIRE(jtype(false).as<float>() == 0.f);
    
    REQUIRE(jtype(5).as<float>() == 5.f);
    REQUIRE(jtype(5u).as<float>() == 5.f);
    REQUIRE(jtype(5.5).as<float>() == 5.5f);
    REQUIRE(jtype(5).as<double>() == 5.0);
    
    REQUIRE(jtype("5").as<double>() == 5.0);
    REQUIRE(jtype("-5.5").as<double>() == -5.5);
}

TEST_CASE("jtypes should handle coercion to string")
{
    using jtypes::jtype;

    REQUIRE(jtype().as<std::string>() == "undefined");
    REQUIRE(jtype(nullptr).as<std::string>() == "null");
    REQUIRE(jtype(true).as<std::string>() == "true");
    REQUIRE(jtype(false).as<std::string>() == "false");;
    REQUIRE(jtype(-2).as<std::string>() == "-2");
    REQUIRE(jtype(jtype::array({1,2,3})).as<std::string>() == "1,2,3");
    REQUIRE(jtype(jtype::array({'a','b','c'})).as<std::string>() == "a,b,c");
    REQUIRE(jtype("hello world!").as<std::string>() == "hello world!");
    REQUIRE(jtype(jtype::object({{"a", "x"}})).as<std::string>() == "object");

    using sig = int(void);
    REQUIRE(jtype(jtype::function<sig>([]() {return -1;})).as<std::string>() == "function");
    
}

TEST_CASE("jtypes key and values should be iterable")
{
    using jtypes::jtype;

    jtype x = jtype::object({
        {"a", 2},
        {"b", "hello world"},
        {"c", true}
    });
    
    jtype keys = x.keys();
    jtype values = x.values();
    
    REQUIRE(keys == jtype::array({"a", "b", "c"}));
    REQUIRE(values == jtype::array({2, "hello world", true}));
    
    // holds also for arrays
    
    x = jtype::array({"a", "b", "c"});
    keys = x.keys();
    values = x.values();
    
    REQUIRE(keys == jtype::array({0, 1, 2}));
    REQUIRE(values == jtype::array({"a", "b", "c"}));
}



TEST_CASE("jtypes support at for nested object creation")
{
    using jtypes::jtype;

    jtype x = jtype::object();
    
    x.at("first.number") = jtype(3);
    x.at("first.string") = jtype("hello world");
    
    REQUIRE(x.is_object());
    REQUIRE(x["first"]["number"].as<int>() == 3);
    REQUIRE(x["first"]["string"].as<std::string>() == "hello world");
    
    // No value given, implicitly creates undefined object leaf
    x = jtype::object();
    x.at("a.b.c");
    REQUIRE(x["a"]["b"]["c"].is_undefined());
    
    // Create path should also support an array instead of string
    x = jtype::object();
    x.at(jtype::array({"a", "b", "c"})) = jtype::object();
    REQUIRE(x["a"]["b"]["c"].is_object());
    
    // As a side node it should also support non string types.
    x = jtype::object();
    x.at(jtype::array({1, 2, "c"})) = jtype::object();
    REQUIRE(x["1"]["2"]["c"].is_object());
    
    // In const context similar no path can be created, but existing ones can be queried.
    x = jtype::object();
    x.at("first.number.first") = jtype(3);
    x.at("first.string") = jtype("hello world");
    x.at("first.empty") = jtype::object();
    
    const jtype &xx = x;
    REQUIRE(xx.at("first.number.first") == 3);
    REQUIRE(xx.at("first.string") == "hello world");
    REQUIRE(xx.at("first.empty").is_object());
    REQUIRE(xx.at("first.empty.x").is_undefined());
    REQUIRE((xx.at("first.empty.x.y.k") | 5) == 5);
    
    
    jtype y = jtype::array({1,2,3});
    REQUIRE(y.at(0) == 1);
    REQUIRE(y.at(1) == 2);
    REQUIRE(y.size() == 3);
    REQUIRE(y.at(5).is_undefined());
    REQUIRE(y.size() == 6);
}

TEST_CASE("jtypes support clearing of structured elements") {
    
    using jtypes::jtype;
    

    jtype x = jtype::object({
        {"a", 1},
        {"b", jtype::object({
            {"c", 3}
        })}
    });
    
    REQUIRE(x.size() == 2);
    REQUIRE(x["b"].size() == 1);
    x["b"].clear();
    REQUIRE(x["b"].is_object());
    REQUIRE(x["b"].size() == 0);
    
}


TEST_CASE("jtypes should allow on the array creation")
{
    using jtypes::jtype;

    jtype x = jtype::array();
    
    x[0] = jtype("a");
    x[3] = "b";
    
    REQUIRE(x.is_array());
    REQUIRE(x[0].as<std::string>() == "a");
    REQUIRE(x[1].as<std::string>() == "undefined");
    REQUIRE(x[2].as<std::string>() == "undefined");
    REQUIRE(x[3].as<std::string>() == "b");
    
    // const ref
    const jtype &xx = x;
    REQUIRE(xx.is_array());
    REQUIRE(xx[0].as<std::string>() == "a");
    REQUIRE(xx[1].as<std::string>() == "undefined");
    REQUIRE(xx[2].as<std::string>() == "undefined");
    REQUIRE(xx[3].as<std::string>() == "b");

    x = jtype::array();
    x.push_back(10);
    x.push_back("hello world");
    x.push_back(true);

    REQUIRE(x[0] == 10);
    REQUIRE(x[1] == "hello world");
    REQUIRE(x[2] == true);
    
}

TEST_CASE("jtypes should support default values")
{
    using jtypes::jtype;

    jtype x = jtype::object({
        {"a", 2},
        {"b", "hello world"},
        {"c", true}
    });
    
    REQUIRE((x["a"] | "not-here") == jtype(2));
    REQUIRE((x["b"] | "not-here") == jtype("hello world"));
    REQUIRE((x["d"] | "not-here") == jtype("not-here"));
}


TEST_CASE("jtypes should support equality comparison")
{
    using jtypes::jtype;

    // Numbers 

    REQUIRE(jtype(3) == 3);
    REQUIRE(jtype(-3) == -3);
    REQUIRE(jtype(3) == 3u);
    REQUIRE(jtype(3) == 3.0);
    REQUIRE(jtype(-3) == -3.0);
    REQUIRE(jtype(3) == 3.f);

    REQUIRE(jtype(3u) == 3);
    REQUIRE(jtype(3u) == 3u);
    REQUIRE(jtype(3u) == 3.0);
    REQUIRE(jtype(3u) == 3.f);

    REQUIRE(jtype(3.0) == 3);
    REQUIRE(jtype(3.0) == 3u);
    REQUIRE(jtype(3.0) == 3.0);
    REQUIRE(jtype(-3.0) == -3.0);
    REQUIRE(jtype(3.0) == 3.f);
    REQUIRE(jtype(-3.0) == -3);

    REQUIRE(jtype(3) != 2);
    REQUIRE(jtype(3) != 2u);
    REQUIRE(jtype(-3) != 2u);
    REQUIRE(jtype(3) != 2.0);
    REQUIRE(jtype(3) != 2.f);

    REQUIRE(jtype(3u) != 2);
    REQUIRE(jtype(3u) != -2);
    REQUIRE(jtype(3u) != 2u);
    REQUIRE(jtype(3u) != 2.0);
    REQUIRE(jtype(3u) != 2.f);

    REQUIRE(jtype(3.0) != 2);
    REQUIRE(jtype(3.0) != 2u);
    REQUIRE(jtype(-3.0) != 2);
    REQUIRE(jtype(-3.0) != 2u);
    REQUIRE(jtype(3.0) != 2.0);
    REQUIRE(jtype(3.0) != 2.f);

    // Arrays

    REQUIRE(jtype(jtype::array({1u, 2u, 3u})) == jtype(jtype::array({1, 2, 3})));

    // Objects

    jtype o1 = jtype::object({ {"a", "hello world"}, {"b", jtype::object({{"c", 3}})} });
    jtype o2 = jtype::object({ { "a", "hello world" },{ "b", jtype::object({ { "c", 3 } }) } });
    jtype o3 = jtype::object({ { "a", "hello world" },{ "b", jtype::object({ { "c", 4 } }) } });

    REQUIRE(o1 == o2);
    REQUIRE(o1 != o3);

    jtype o4 = o3;
    REQUIRE(o4 == o3);
    o4["a"] = "abc";
    REQUIRE(o4 != o3);
    
    // Functions
    
    jtype f1 = jtype::function<bool(void)>([](){return true;});
    jtype f2 = jtype::function<bool(void)>([](){return true;});
    jtype f3 = f1;
    
    REQUIRE(f1 != f2);
    REQUIRE(f1 == f3);
}

TEST_CASE("jtypes should support less-than comparison")
{
    using jtypes::jtype;

    REQUIRE(jtype(3) <= 3);
    REQUIRE(jtype(3) <= 3u);
    REQUIRE(jtype(3) <= 3.0);
    REQUIRE(jtype(3) >= 3.0);
    
    REQUIRE(jtype(3) < 4);
    REQUIRE(jtype(-3) < 3u);
    REQUIRE(jtype(3) < 4.0);

    // Strict weak ordering

    // Irreflexivity
    REQUIRE((jtype(3) < jtype(3)) == false);
    REQUIRE((jtype(3.5) < jtype(3.5)) == false);

    // Antisymmetry
    REQUIRE((jtype(3) < jtype(3.5)) == true);
    REQUIRE((jtype(3.5) < jtype(3)) == false);

    //Transitivity
    REQUIRE((jtype(3) < jtype(3.5)) == true);
    REQUIRE((jtype(3.5) < jtype(4)) == true);
    REQUIRE((jtype(3) < jtype(4)) == true);

    std::less<jtype> less;
    REQUIRE(less(jtype(2u), jtype(3.5)));
    REQUIRE(!less(jtype(3.5), jtype(2u)));
}

TEST_CASE("jtypes should support JSON")
{
    using jtypes::jtype;

    jtype x = jtype::object({
        {"a", 2.1},
        {"b", "hello world"},
        {"c", true},
        {"d", jtype::object({{"e", nullptr}})},
        {"f", jtype::array({1,2,3,"hello"})}
    });
    
    REQUIRE(jtypes::to_json(x) == R"({"a":2.1,"b":"hello world","c":true,"d":{"e":null},"f":[1,2,3,"hello"]})");
    REQUIRE(jtypes::from_json(jtypes::to_json(x)) == x);
    
    // Overloaded output / input operators
    
    const std::string s = jtypes::to_json(x);
    
    std::istringstream iss(s);
    jtype y;
    
    REQUIRE_NOTHROW(iss >> y);
    REQUIRE(x == y);
    
    std::ostringstream oss;
    oss << y;
    REQUIRE(s == oss.str());
    
    // Invalid JSON
    REQUIRE_THROWS_AS(jtypes::from_json("dasda"), jtypes::syntax_error);
    
    
    // Undefined and function objects are stripped when converted to json.
    jtype u = jtype::object({
        {"a", jtype()},
        {"b", jtype::array({1,jtype(),2})},
        {"c", jtype::function<int(int)>()}
    });

    std::stringstream str;
    str << u;
    
    jtype parsed;
    str >> parsed;

    REQUIRE(parsed == jtype::object({
        {"b", jtype::array({1,2}) }
    }));

}

#include <iterator>

using namespace jtypes;

using mapbox::util::variant;



TEST_CASE("jtypes should support iterators")
{
    using jtypes::jtype;
    
    jtype x = jtype::object({
        {"a", 2.1},
        {"b", "hello world"},
        {"c", true},
        {"d", jtype::object({{"e", nullptr}})},
        {"f", jtype::array({2,1,3,"hello"})}
    });
    
    {
        // Object iteration
        auto i = x.begin();
        auto end = x.end();
        
        REQUIRE(std::distance(i, end) == 5);
        
        REQUIRE(*i == 2.1); REQUIRE(i.key() == "a"); ++i;
        REQUIRE(*i == "hello world"); REQUIRE(i.key() == "b"); ++i;
        REQUIRE(*i == true); REQUIRE(i.key() == "c"); ++i;
        REQUIRE(*i == jtype::object({{"e", nullptr}})); REQUIRE(i.key() == "d"); ++i;
        REQUIRE(*i == jtype::array({2,1,3,"hello"})); REQUIRE(i.key() == "f"); ++i;
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
        
        jtype received = jtype::array();
        for (auto v : x) {
            received.push_back(v);
        }
        
        REQUIRE(received == jtype::array({2.1, "hello world", true, jtype::object({{"e", nullptr}}), jtype::array({2,1,3,"hello"})}));
        
        
        const jtype &xx = x;
        received = jtype::array();
        for (auto v : xx["f"]) {
            received.push_back(v);
        }
        
        REQUIRE(received == jtype::array({2,1,3,"hello"}));
    }
    
    {
        // Manipulation
        
        jtype x = jtype::array({ 5, 10, 6, 20, 7 });
        
        jtype y = jtype::array();
        std::transform(x.begin(), x.end(), std::back_inserter(y), [](const jtype &v) {return (int)v + 1;});
        REQUIRE(y == jtype::array({ 6, 11, 7, 21, 8 }));
    
        
        auto last = std::partition(x.begin(), x.end(), [](const jtype &i) { return i < 10;});
        
        
        for (auto i = x.begin(); i != last; ++i) {
            REQUIRE(*i < 10);
        }
        
        for (auto i = last; i != x.end(); ++i) {
            REQUIRE(*i >= 10);
        }
    }
    
    {
        jtype x = 1;
        REQUIRE(x.begin() == x.end());
    }
}

TEST_CASE("jtypes should support merging")
{
    using jtypes::jtype;

    jtype opts = jtype::object({
        {"a", 2.1},
        {"b", "hello world"},
        {"c", true},
        {"d", jtype::object({{"e", nullptr}})},
        {"f", jtype::array({2,1,3,"hello"})}
    });
    
    opts.merge_from(jtype::object({
        {"a", 3},
        {"x", "wuff"},
        {"d", jtype::object({{"d", 1}})},
        {"f", jtype::array({10,11})}
    }));
    
    jtype expected = jtype::object({
        {"a", 3},
        {"b", "hello world"},
        {"c", true},
        {"d", jtype::object({{"e", nullptr}, {"d", 1}})},
        {"f", jtype::array({10,11})},
        {"x", "wuff"}
    });
    
    REQUIRE(opts == expected);
}

TEST_CASE("jtypes should support split")
{
    using jtypes::jtype;

    jtype s = "a.b.c";
    REQUIRE(s.split('.') == jtype::array({"a", "b", "c"}));
    
    s = "a..b.c.";
    REQUIRE(s.split('.') == jtype::array({"a", "b", "c"}));
}

TEST_CASE("jtypes undefined behaviour should mimic ECMAScript 5 behaviour")
{
    using jtypes::jtype;

    jtype x;
    REQUIRE_THROWS_AS(x["a"], jtypes::type_error);
    REQUIRE(x.begin() == x.end());

    x = jtype::object();
    REQUIRE(x["a"].is_undefined());
    REQUIRE_THROWS_AS(x["a"]["b"], jtypes::type_error);
}



