#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse comment inside element") {
  const std::string xml = "<root><!--c--></root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_children.size() == 1);
  const auto* commentNode = result.m_rootNodes.at(0)->m_children.at(0);
  CHECK(commentNode->m_type == NodeType::Comment);
  CHECK(commentNode->m_value == "c");
}

TEST_CASE("parse comment alongside element") {
  const std::string xml = "<!--c1--><root/><!--c2-->";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.size() == 3);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Comment);
  CHECK(result.m_rootNodes.at(0)->m_value == "c1");
  CHECK(result.m_rootNodes.at(1)->m_type == NodeType::Element);
  CHECK(result.m_rootNodes.at(2)->m_type == NodeType::Comment);
  CHECK(result.m_rootNodes.at(2)->m_value == "c2");
}

TEST_CASE("parse malformed comment missing closing") {
  const std::string xml = "<root><!--unclosed</root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_children.size() == 1);
  const auto* commentNode = result.m_rootNodes.at(0)->m_children.at(0);
  CHECK(commentNode->m_type == NodeType::Comment);
  CHECK(commentNode->m_value == "unclosed</root>");
  CHECK_FALSE(result.m_errors.empty());
}
