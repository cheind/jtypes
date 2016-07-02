/**
This file is part of cpp-restify.

Copyright(C) 2016 Christoph Heindl
All rights reserved.

This software may be modified and distributed under the terms
of MIT license. See the LICENSE file for details.
*/

#include "catch.hpp"

#include <restify/helpers.h>
#include <iostream>

TEST_CASE("helpers-json-cast")
{
    using namespace restify;
    
    Json::Value strInt = "123";
    REQUIRE(json_cast<std::string>(strInt) == "123");
    REQUIRE(json_cast<int>(strInt) == 123);
    REQUIRE(json_cast<float>(strInt) == 123.f);
    
    Json::Value strBool0 = "true";
    REQUIRE(json_cast<bool>(strBool0) == true);
    
    Json::Value arr;
    arr.append("1");
    arr.append("2");
    std::string arrayStr = json_cast<std::string>(arr);
    std::istringstream arrayIss(arrayStr);
    Json::Value arrayParsed;
    REQUIRE_NOTHROW(arrayIss >> arrayParsed);
    REQUIRE(arr == arrayParsed);

    Json::Value obj = restify::json()("input", 3);
    std::string objStr = json_cast<std::string>(obj);
    std::istringstream objIss(objStr);
    Json::Value objParsed;
    REQUIRE_NOTHROW(objIss >> objParsed);
    REQUIRE(obj == objParsed);
    
    Json::Value strNotInt = "23kdnd";
    REQUIRE_THROWS_AS(json_cast<int>(strNotInt), Error);
    REQUIRE_THROWS_AS(json_cast<float>(strNotInt), Error);
    REQUIRE_THROWS_AS(json_cast<bool>(strNotInt), Error);
}


TEST_CASE("helpers-json-merge") {

    const char *a = R"(
    {
        "message": "hello world",
        "body": {
            "width": 3,
            "height": 5
        },
        "count": 3
    })";


    const char *b = R"(
    {
        "message": "hugo",
        "body": {
            "height": 5.3,
            "depth": 2
        }
    })";


    using restify::json;

    Json::Value merged = json(a).mergeFrom(json(b), restify::JsonMergeFlags::MergeEverything);
    REQUIRE(merged["message"] == "hugo");
    REQUIRE(merged["body"]["width"] == 3);
    REQUIRE(merged["body"]["height"] == 5.3);
    REQUIRE(merged["body"]["depth"] == 2);
    REQUIRE(merged["count"] == 3);

    merged = json(a).mergeFrom(json(b), restify::JsonMergeFlags::MergeExistingElements);
    REQUIRE(merged["message"] == "hugo");
    REQUIRE(merged["body"]["width"] == 3);
    REQUIRE(merged["body"]["height"] == 5.3);
    REQUIRE(merged["body"]["depth"].isNull());
    REQUIRE(merged["count"] == 3);

    merged = json(a).mergeFrom(json(b), restify::JsonMergeFlags::MergeExistingElementsWithSameType);
    REQUIRE(merged["message"] == "hugo");
    REQUIRE(merged["body"]["width"] == 3);
    REQUIRE(merged["body"]["height"] == 5); // not same type
    REQUIRE(merged["body"]["depth"].isNull());
    REQUIRE(merged["count"] == 3);

    merged = json(a).mergeFrom(json(b), restify::JsonMergeFlags::MergeNewElements);
    REQUIRE(merged["message"] == "hello world");
    REQUIRE(merged["body"]["width"] == 3);
    REQUIRE(merged["body"]["height"] == 5);
    REQUIRE(merged["body"]["depth"] == 2);
    REQUIRE(merged["count"] == 3);



}

TEST_CASE("helpers-split-string") {
    
    
    REQUIRE(restify::splitString(" a=2& b = 3.1&c = 1;2;3 ", '&', true, false) ==
            (std::vector<std::string> {"a=2", "b = 3.1", "c = 1;2;3"}));
    

    REQUIRE(restify::splitString(" a=2& b = 3.1&c = 1;2;3\r\n ", '&', true, true) ==
        (std::vector<std::string> {"a=2", "b = 3.1", "c = 1;2;3"}));
    
}

TEST_CASE("helpers-json-builder") {
    
    restify::JsonBuilder jbp;

    Json::Value v = jbp
        .set("body.message", "Hello World")
        .set("body.count", 3)
        .set("body.elements.[0]", "a")
        .set("body.elements.[1]", "b")
        .set("body.elements.[2]", "c")
        .set("body.elements", "[3]", "d")
        .toJson();

   
    REQUIRE(v["body"].isObject());
    REQUIRE(v["body"]["message"].asString() == "Hello World");
    REQUIRE(v["body"]["elements"].isArray());
    REQUIRE(v["body"]["elements"].size() == 4);
    REQUIRE(v["body"]["elements"][0] == "a");
    REQUIRE(v["body"]["elements"][1] == "b");
    REQUIRE(v["body"]["elements"][2] == "c");
    REQUIRE(v["body"]["elements"][3] == "d");
    REQUIRE(v["body"]["count"].asInt() == 3);
}

TEST_CASE("helpers-json-builder-alt") {

    Json::Value v = restify::json()
        ("body.message", "Hello World")
        ("body.count", 3)
        ("body.elements.[0]", "a")
        ("body.elements.[1]", "b")
        ("body.elements.[2]", "c");


    REQUIRE(v["body"].isObject());
    REQUIRE(v["body"]["message"].asString() == "Hello World");
    REQUIRE(v["body"]["elements"].isArray());
    REQUIRE(v["body"]["elements"].size() == 3);
    REQUIRE(v["body"]["elements"][0] == "a");
    REQUIRE(v["body"]["elements"][1] == "b");
    REQUIRE(v["body"]["elements"][2] == "c");
    REQUIRE(v["body"]["count"].asInt() == 3);
}

TEST_CASE("helpers-json-from-string") {

    const char *jsonstr = R"(
    {
        "message": "hello world",
        "count": 3
    })";

    Json::Value v =
        restify::json(jsonstr) // Parse from stream
        ("isParamSet", true);  // Continue modifying

    REQUIRE(v["message"] == "hello world");
    REQUIRE(v["count"] == 3);
    REQUIRE(v["isParamSet"] == true);

    // Invalid string
    const char *notjsonstr = R"(
    {
        "message" "hello world"
        "count": 3
    })";

    REQUIRE_THROWS_AS(restify::json(notjsonstr), restify::Error);

}

TEST_CASE("helpers-json-builder-adapt") {

    Json::Value val;
    val["body"]["message"] = "Hello World";

    Json::Value v = restify::json(val)
        ("body.count", 3)
        ("body.elements.[0]", "a")
        ("body.elements.[1]", "b")
        ("body.elements.[2]", "c");


    REQUIRE(v["body"].isObject());
    REQUIRE(v["body"]["message"].asString() == "Hello World");
    REQUIRE(v["body"]["elements"].isArray());
    REQUIRE(v["body"]["elements"].size() == 3);
    REQUIRE(v["body"]["elements"][0] == "a");
    REQUIRE(v["body"]["elements"][1] == "b");
    REQUIRE(v["body"]["elements"][2] == "c");
    REQUIRE(v["body"]["count"].asInt() == 3);

}
