#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse CDATA inside element") {
  const std::string xml = "<root><![CDATA[data]]></root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_numChildren == 1);
  const auto* cdataNode = result.m_rootNodes.at(0)->m_firstChild;
  CHECK(cdataNode->m_type == NodeType::Cdata);
  CHECK(cdataNode->m_content.getValue() == "data");
  CHECK(cdataNode->m_content.getName().empty());
}

TEST_CASE("parse CDATA alongside elements") {
  const std::string xml = "<![CDATA[c1]]><root/><![CDATA[c2]]>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.size() == 3);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Cdata);
  CHECK(result.m_rootNodes.at(0)->m_content.getValue() == "c1");
  CHECK(result.m_rootNodes.at(1)->m_type == NodeType::Element);
  CHECK(result.m_rootNodes.at(2)->m_type == NodeType::Cdata);
  CHECK(result.m_rootNodes.at(2)->m_content.getValue() == "c2");
}

TEST_CASE("parse malformed CDATA missing closing") {
  const std::string xml = "<root><![CDATA[unclosed</root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_numChildren == 1);
  const auto* cdataNode = result.m_rootNodes.at(0)->m_firstChild;
  CHECK(cdataNode->m_type == NodeType::Cdata);
  CHECK(cdataNode->m_content.getValue() == "unclosed</root>");
  CHECK_FALSE(result.m_errors.empty());
}
