#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("pretty-print whitespace becomes Text nodes") {
  const std::string xml = "<a>\n  <b/>\n</a>";
  const auto result = XmlParser::parse(xml);
  const auto* a = result.m_rootNodes.at(0);
  REQUIRE(a->m_numChildren == 3);
  const auto* c0 = a->m_firstChild;
  CHECK(c0->m_type == NodeType::Text);
  CHECK(c0->m_content.getValue() == "\n  ");
  const auto* c1 = c0->m_nextSibling;
  CHECK(c1->m_type == NodeType::Element);
  CHECK(c1->m_content.getName() == "b");
  const auto* c2 = c1->m_nextSibling;
  CHECK(c2->m_type == NodeType::Text);
  CHECK(c2->m_content.getValue() == "\n");
  CHECK(c2->m_nextSibling == nullptr);
  CHECK(a->m_lastChild == c2);
}

TEST_CASE("mixed content preserves all text runs") {
  const std::string xml = "<p>Hello <b>world</b> again</p>";
  const auto result = XmlParser::parse(xml);
  const auto* p = result.m_rootNodes.at(0);
  REQUIRE(p->m_numChildren == 3);
  const auto* c0 = p->m_firstChild;
  CHECK(c0->m_type == NodeType::Text);
  CHECK(c0->m_content.getValue() == "Hello ");
  const auto* c1 = c0->m_nextSibling;
  CHECK(c1->m_type == NodeType::Element);
  CHECK(c1->m_content.getName() == "b");
  REQUIRE(c1->m_numChildren == 1);
  CHECK(c1->m_firstChild->m_type == NodeType::Text);
  CHECK(c1->m_firstChild->m_content.getValue() == "world");
  const auto* c2 = c1->m_nextSibling;
  CHECK(c2->m_type == NodeType::Text);
  CHECK(c2->m_content.getValue() == " again");
  CHECK(c2->m_nextSibling == nullptr);
  CHECK(p->m_lastChild == c2);
}
