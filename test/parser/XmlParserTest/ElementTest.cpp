#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse root element") {
  const std::string xml = "<root></root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_allNodes.size() == 1);
  CHECK(result.m_rootNodes.size() == 1);
  CHECK(result.m_rootNodes[0]->m_content.getName() == "root");
  CHECK(result.m_rootNodes[0]->m_type == NodeType::Element);
}

TEST_CASE("parse root element with content") {
  const std::string xml = "<root>content</root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes[0]->m_content.getName() == "root");
  CHECK(result.m_rootNodes[0]->m_content.getValue() == "content");
}

TEST_CASE("parse self-closing element") {
  const std::string xml = "<root/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
}

TEST_CASE("parse element with spaces") {
  const std::string xml = "<\troot   />";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
}

TEST_CASE("parse nested element") {
  const std::string xml = "<root><nested></nested></root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_allNodes.size() == 2);
  CHECK(result.m_rootNodes.size() == 1);
  CHECK(result.m_rootNodes[0]->m_content.getName() == "root");
  CHECK(result.m_rootNodes[0]->m_numChildren == 1);
  CHECK(result.m_rootNodes[0]->m_firstChild->m_content.getName() == "nested");
}

// // Error handling

// TEST_CASE("parse root element with error") {
//   const std::string xml = "<root";
//   const auto result = XmlParser::parse(xml);
//   CHECK(result.m_allNodes.size() == 1);
//   CHECK(result.m_rootNodes.size() == 1);
//   CHECK(result.m_rootNodes[0]->m_name == "root");
//   CHECK(!result.m_errors.empty());
// }

// TEST_CASE("parse wrongly closed element") {
//   const std::string xml = "<root></other>";
//   const auto result = XmlParser::parse(xml);
//   CHECK(result.m_allNodes.size() == 1);
//   CHECK(!result.m_errors.empty());
// }
