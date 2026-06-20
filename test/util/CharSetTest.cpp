#include "../../src/util/CharSet.h"
#include <doctest.h>
#include <string_view>

using namespace sde4::util;

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
