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

TEST_CASE("Basic speller", "[speller_basic.zhfst]") {
    hfst_ol::ZHfstOspeller sp;
    INFO("Path: " << sp.read_zhfst("speller_basic.zhfst"));

    SECTION("Test strings") {
        REQUIRE(sp.spell("olut") == true);
        REQUIRE(sp.spell("vesi") == false);
        REQUIRE(sp.spell("sivolutesi") == false);
        REQUIRE(sp.spell("olu") == false);
        REQUIRE(sp.spell("ßþ”×\\") == false);
        REQUIRE(sp.spell("") == false);
    }
}

TEST_CASE("Speller analyse", "[speller_analyser.zhfst]") {
    hfst_ol::ZHfstOspeller sp;
    INFO("Path: " << sp.read_zhfst("speller_analyser.zhfst"));

    SECTION("Test strings") {
        REQUIRE(sp.analyse("olut").size() == 1);
        REQUIRE(sp.analyse("vesi").size() == 0);
        REQUIRE(sp.analyse("sivolutesi").size() == 0);
        REQUIRE(sp.analyse("olu").size() == 0);
        REQUIRE(sp.analyse("ßþ”×\\").size() == 0);
        REQUIRE(sp.analyse("").size() == 0);
    }
}

TEST_CASE("Bad error model", "[bad_errormodel.zhfst]") {

}
