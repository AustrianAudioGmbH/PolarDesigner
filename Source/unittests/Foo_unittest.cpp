#include <catch2/catch_test_macros.hpp>

TEST_CASE ("Foo test", "[foo]")
{
    REQUIRE (1 + 1 == 2);
}
