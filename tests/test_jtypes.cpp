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

TEST_CASE("jtypes-should-work")
{
    jtypes::my_variant var;
    
    var = jtypes::object();
    var.get<jtypes::object>().m["x"] = 3;
    var.get<jtypes::object>().m["a"] = jtypes::array({1,2,3});
    
    REQUIRE(var.get<jtypes::object>().m["a"].which() == 3);
    
}
