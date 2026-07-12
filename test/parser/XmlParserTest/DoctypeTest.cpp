#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse simple doctype") {
  const std::string xml = "<!DOCTYPE root><root/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.size() == 2);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Doctype);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "root");
}

TEST_CASE("parse doctype with internal subset") {
  const std::string xml = "<!DOCTYPE root [ <!ELEMENT root EMPTY> ]><root/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Doctype);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
  CHECK(result.m_rootNodes.at(0)->m_content.getValue() == "[ <!ELEMENT root EMPTY> ]");
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "root");
  CHECK(result.m_errors.empty());
}

TEST_CASE("parse doctype with public identifier") {
  const std::string xml =
      "<!DOCTYPE root PUBLIC \"-//W3C//DTD XHTML 1.0//EN\" "
      "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"><root/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Doctype);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "root");
  CHECK(result.m_errors.empty());
}

TEST_CASE("parse malformed doctype missing closing") {
  const std::string xml = "<!DOCTYPE root";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Doctype);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
  CHECK_FALSE(result.m_errors.empty());
}
