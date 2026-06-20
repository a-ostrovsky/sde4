#include "../../src/util/CharSet.h"
#include <doctest.h>
#include <string_view>

using namespace sde4::util;

TEST_CASE("CharSet membership") {
  static constexpr CharSet digits{"0123456789"};
  CHECK(digits.findFirstNotInSet("abc12", 0) == 0); // 'a' not a digit
  CHECK(digits.findFirstNotInSet("abc12", 3) ==
        5); // after 'c' everything is a digit
  CHECK(digits.findFirstNotInSet("", 0) == 0);
}

TEST_CASE("findFirstNotNameChar") {
  CHECK(findFirstNotNameChar("elem", 0) == 4);     // all valid
  CHECK(findFirstNotNameChar("elem<ent", 0) == 4); // '<' invalid at index 4
  CHECK(findFirstNotNameChar(" elem", 0) == 0);    // space invalid at start
}

TEST_CASE("findFirstNotWhitespace") {
  CHECK(findFirstNotWhitespace("a b", 1) == 2);
  CHECK(findFirstNotWhitespace("a\tb", 1) == 2);
  CHECK(findFirstNotWhitespace("a\rb", 1) == 2);
  CHECK(findFirstNotWhitespace("a\nb", 1) == 2);
  CHECK(findFirstNotWhitespace("a\r\n\tb", 1) == 4);
  CHECK(findFirstNotWhitespace("a\fb", 1) == 2);
  CHECK(findFirstNotWhitespace("a\vb", 1) == 2);
}
