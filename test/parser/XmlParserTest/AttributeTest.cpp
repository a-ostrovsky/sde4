#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse attribute") {
  const std::string xml = "<root attr='val'/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
  CHECK(result.m_rootNodes.at(0)->m_numChildren == 1);
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_content.getName() == "attr");
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_content.getValue() == "val");
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_type == NodeType::Attribute);
}

TEST_CASE("parse two attributes") {
  const std::string xml = "<root attr1='val1' attr2='val2'/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_content.getName() == "attr1");
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_content.getValue() == "val1");
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_nextSibling->m_content.getName() ==
        "attr2");
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_nextSibling->m_content.getValue() ==
        "val2");
}
