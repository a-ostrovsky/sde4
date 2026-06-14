#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
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

TEST_CASE("parse malformed comment missing closing") {
  const std::string xml = "<root><!--unclosed</root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_children.size() == 1);
  const auto* commentNode = result.m_rootNodes.at(0)->m_children.at(0);
  CHECK(commentNode->m_type == NodeType::Comment);
  CHECK(commentNode->m_value == "unclosed</root>");
  CHECK_FALSE(result.m_errors.empty());
}
