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
    jtypes::value var;
    
    var.v = jtypes::object();
    var.v.get<jtypes::object>().m["x"] = jtypes::value(3);
    var.v.get<jtypes::object>().m["a"] = jtypes::value(jtypes::array({1,2,3}));
    
    REQUIRE(var.v.get<jtypes::object>().m["a"].v.which() == 3);
    
}
