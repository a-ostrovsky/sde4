#pragma once

#include "../util/XmlStringDecoder.h"
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace sde4::domain {

enum class NodeType : std::uint8_t {
  Element,
  Attribute,
  Comment,
  ProcessingInstruction,
  Doctype,
  Declaration,
  Cdata,
  Text,
};

struct XmlError {
  std::string m_message{};
  std::size_t m_position{};
};

class TreeNodeContent {
  const char* m_nameStart{};
  const char* m_valueStart{};
  std::uint32_t m_nameLength{};
  std::uint32_t m_valueLength{};

public:
  TreeNodeContent() = default;
  TreeNodeContent(std::string_view name, std::string_view value)
      : m_nameStart(name.data()), m_valueStart(value.data()),
        m_nameLength(static_cast<std::uint32_t>(name.size())),
        m_valueLength(static_cast<std::uint32_t>(value.size())) {
    assert(name.size() < std::numeric_limits<std::uint32_t>::max());
    assert(value.size() < std::numeric_limits<std::uint32_t>::max());
  }

  std::string_view getName() const {
    return std::string_view(m_nameStart, m_nameLength);
  }
  std::string_view getValue() const {
    return std::string_view(m_valueStart, m_valueLength);
  }

  void setName(std::string_view s) {
    assert(s.size() < std::numeric_limits<std::uint32_t>::max());
    m_nameStart = s.data();
    m_nameLength = static_cast<std::uint32_t>(s.size());
  }
  void setValue(std::string_view s) {
    assert(s.size() < std::numeric_limits<std::uint32_t>::max());
    m_valueStart = s.data();
    m_valueLength = static_cast<std::uint32_t>(s.size());
  }

  // Resolves entity and character references per W3C XML §4.4/§4.6.
  std::string decodeValue() const {
    return util::XmlStringDecoder::decode(getValue());
  }
};

struct TreeNode;

struct TreeNode {
  TreeNodeContent m_content{};
  TreeNode* m_parent{};
  TreeNode* m_firstChild{};
  TreeNode* m_lastChild{};
  TreeNode* m_nextSibling{};
  std::uint32_t m_numChildren{};
  NodeType m_type{};
  bool m_isDeleted{}; // Whether this node is soft-deleted
};

static_assert(sizeof(TreeNode) <= 64,
              "TreeNode should still fit in an average cache line");

} // namespace sde4::domain
