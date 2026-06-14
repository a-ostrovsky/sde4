#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse attribute") {
  const std::string xml = "<root attr='val'/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_name == "root");
  CHECK(result.m_rootNodes.at(0)->m_children.size() == 1);
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_name == "attr");
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_value == "val");
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_type ==
        NodeType::Attribute);
}

TEST_CASE("parse two attributes") {
  const std::string xml = "<root attr1='val1' attr2='val2'/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_name == "attr1");
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_value == "val1");
  CHECK(result.m_rootNodes.at(0)->m_children.at(1)->m_name == "attr2");
  CHECK(result.m_rootNodes.at(0)->m_children.at(1)->m_value == "val2");
}
