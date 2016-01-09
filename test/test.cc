#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/ZHfstOspeller.h"

TEST_CASE("ZHfstOspeller functions", "[ZHfstOspeller]") {
    hfst_ol::ZHfstOspeller sp;

    SECTION("Suggest with no spellers should be empty") {
	auto vec = sp.suggest("test");
	REQUIRE(vec.size() == 0);
    }
}
