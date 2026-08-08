#include "../../src/domain/Namespace.h"
#include "../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <cstdlib>
#include <string>

using namespace sde4::parser;
using namespace sde4::domain;

namespace {

const TreeNode& findChild(const TreeNode* parent, NodeType type) {
  for (const TreeNode* child = parent->m_firstChild; child != nullptr;
       child = child->m_nextSibling) {
    if (child->m_type == type) {
      return *child;
    }
  }
  FAIL("no child of requested type found");
  std::abort();
}

const TreeNode& findAttribute(const TreeNode* parent, const char* name) {
  for (const TreeNode* child = parent->m_firstChild; child != nullptr;
       child = child->m_nextSibling) {
    if (child->m_type == NodeType::Attribute &&
        child->m_content.getName() == name) {
      return *child;
    }
  }
  FAIL("no attribute found");
  std::abort();
}

} // namespace

TEST_CASE("xmlns declaration and xml prefix resolve to spec namespaces") {
  const std::string xml = "<a xmlns=\"u\" xmlns:p=\"v\" xml:lang=\"en\"/>";
  const auto result = XmlParser::parse(xml);
  const auto* a = result.m_rootNodes.at(0);
  const auto& xmlns = findAttribute(a, "xmlns");
  const auto& xmlnsP = findAttribute(a, "xmlns:p");
  const auto& xmlLang = findAttribute(a, "xml:lang");
  CHECK(namespaceUri(xmlns) == "http://www.w3.org/2000/xmlns/");
  CHECK(namespaceUri(xmlnsP) == "http://www.w3.org/2000/xmlns/");
  CHECK(namespaceUri(xmlLang) == "http://www.w3.org/XML/1998/namespace");
}

TEST_CASE("element uses own declaration and inherits from ancestors") {
  const std::string xml = "<a xmlns=\"u\"><b/></a>";
  const auto result = XmlParser::parse(xml);
  const auto* a = result.m_rootNodes.at(0);
  const auto& b = findChild(a, NodeType::Element);
  CHECK(namespaceUri(*a) == "u");
  CHECK(namespaceUri(b) == "u");
  CHECK(a->m_parent == nullptr);
}

TEST_CASE("empty xmlns declaration shadows outer default namespace") {
  const std::string xml = "<a xmlns=\"u\"><b xmlns=\"\"/></a>";
  const auto result = XmlParser::parse(xml);
  const auto* a = result.m_rootNodes.at(0);
  const auto& b = findChild(a, NodeType::Element);
  CHECK(namespaceUri(*a) == "u");
  CHECK(namespaceUri(b) == "");
}

TEST_CASE("prefix matching is exact, not substring") {
  const std::string xml = "<a xmlns:svg=\"v\"><s:b/></a>";
  const auto result = XmlParser::parse(xml);
  const auto* a = result.m_rootNodes.at(0);
  const auto& b = findChild(a, NodeType::Element);
  CHECK(namespaceUri(b) == "");
}

TEST_CASE("prefixed element and attribute resolve via own element") {
  const std::string xml = "<p:a xmlns:p=\"u\" p:b=\"x\"/>";
  const auto result = XmlParser::parse(xml);
  const auto* a = result.m_rootNodes.at(0);
  const auto& attr = findAttribute(a, "p:b");
  CHECK(namespaceUri(*a) == "u");
  CHECK(namespaceUri(attr) == "u");
}
