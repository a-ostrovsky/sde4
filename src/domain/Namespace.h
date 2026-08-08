#pragma once

#include "DataModel.h"
#include <string_view>

namespace sde4::domain {

constexpr std::string_view namespaceUri(const TreeNode& node);

namespace detail {

constexpr std::string_view kXmlNamespace{
    "http://www.w3.org/XML/1998/namespace"};
constexpr std::string_view kXmlnsNamespace{"http://www.w3.org/2000/xmlns/"};
constexpr std::string_view kXmlnsName{"xmlns"};
constexpr std::string_view kXmlnsDeclarationPrefix{"xmlns:"};

constexpr bool isXmlnsDeclarationName(std::string_view name) {
  return name.starts_with(kXmlnsName) &&
         (name.size() == kXmlnsName.size() || name[kXmlnsName.size()] == ':');
}

constexpr bool isNamespaceDeclarationFor(std::string_view attrName,
                                         std::string_view pfx) {
  if (!attrName.starts_with(kXmlnsName)) {
    return false;
  }
  const auto rest = attrName.substr(kXmlnsName.size());
  if (rest.empty()) {
    return pfx.empty();
  }
  return rest[0] == ':' && rest.substr(1) == pfx;
}

// The part of a qualified name before the first ':'; empty for unprefixed
// names.
constexpr std::string_view prefix(const TreeNode& node) {
  const auto name = node.m_content.getName();
  const auto colon = name.find(':');
  return colon == std::string_view::npos ? std::string_view{}
                                         : name.substr(0, colon);
}

} // namespace detail

constexpr std::string_view namespaceUri(const TreeNode& node) {
  if (node.m_type != NodeType::Element && node.m_type != NodeType::Attribute) {
    // Only namespaces for element and attribute nodes.
    return {};
  }

  const auto name = node.m_content.getName();

  if (node.m_type == NodeType::Attribute &&
      detail::isXmlnsDeclarationName(name)) {
    return detail::kXmlnsNamespace;
  }

  const auto pfx = detail::prefix(node);
  if (pfx == "xml") {
    return detail::kXmlNamespace;
  }

  if (node.m_type == NodeType::Attribute && pfx.empty()) {
    return {};
  }

  const auto* element =
      node.m_type == NodeType::Element ? &node : node.m_parent;
  while (element) {
    for (const auto* child = element->m_firstChild; child;
         child = child->m_nextSibling) {
      if (child->m_type == NodeType::Attribute &&
          detail::isNamespaceDeclarationFor(child->m_content.getName(), pfx)) {
        return child->m_content.getValue();
      }
    }
    element = element->m_parent;
  }

  return {};
}

} // namespace sde4::domain
