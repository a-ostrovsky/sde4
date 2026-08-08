#include "../../src/util/XmlStringDecoder.h"
#include <doctest.h>

using namespace sde4::util;

TEST_CASE("decode passes through plain text") {
  CHECK(XmlStringDecoder::decode("hello world") == "hello world");
}

TEST_CASE("decode resolves predefined entities") {
  CHECK(XmlStringDecoder::decode("a&amp;b") == "a&b");
  CHECK(XmlStringDecoder::decode("&lt;tag&gt;") == "<tag>");
  CHECK(XmlStringDecoder::decode("&apos;q&apos;") == "'q'");
  CHECK(XmlStringDecoder::decode("&quot;q&quot;") == "\"q\"");
}

TEST_CASE("decode resolves numeric character references") {
  CHECK(XmlStringDecoder::decode("&#38;") == "&");
  CHECK(XmlStringDecoder::decode("&#x26;") == "&");
  CHECK(XmlStringDecoder::decode("&#X26;") == "&");
  CHECK(XmlStringDecoder::decode("&#228;") == "\xC3\xA4");
  CHECK(XmlStringDecoder::decode("&#x1F600;") == "\xF0\x9F\x98\x80");
}

TEST_CASE("decode passes through unknown entities verbatim") {
  CHECK(XmlStringDecoder::decode("&auml;") == "&auml;");
  CHECK(XmlStringDecoder::decode("x&unknown;y") == "x&unknown;y");
}

TEST_CASE("decode passes through malformed references verbatim") {
  CHECK(XmlStringDecoder::decode("&amp") == "&amp");
  CHECK(XmlStringDecoder::decode("&;") == "&;");
  CHECK(XmlStringDecoder::decode("&#xZZ;") == "&#xZZ;");
  CHECK(XmlStringDecoder::decode("&#x110000;") == "&#x110000;");
  CHECK(XmlStringDecoder::decode("&#xD800;") == "&#xD800;");
  CHECK(XmlStringDecoder::decode("&#0;") == "&#0;");
}

TEST_CASE("decode handles mixed content") {
  CHECK(XmlStringDecoder::decode("a&amp;b&#38;c") == "a&b&c");
}
