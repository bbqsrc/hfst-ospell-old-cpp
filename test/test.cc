#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/ZHfstOspeller.h"

TEST_CASE("ZHfstOspeller", "[ZHfst]") {
    REQUIRE(new hfst_ol::ZHfstOspeller() != NULL);
}
