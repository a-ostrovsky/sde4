#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace sde4::domain {

enum class NodeType : std::uint8_t {
  Element,
  Attribute,
  Comment,
  ProcessingInstruction,
  Doctype,
  // TODO: For later
  // eCDATA, eDECLARATION
};

struct XmlError {
  std::string m_message{};
  std::size_t m_position{};
};

class TreeNodeContent {
private:
  const char* m_nameStart{};
  const char* m_valueStart{};
  std::uint32_t m_nameLength{};
  std::uint32_t m_valueLength{};

public:
  TreeNodeContent() = default;
  TreeNodeContent(std::string_view name, std::string_view value)
      : m_nameStart(name.data()), m_valueStart(value.data()),
        m_nameLength(name.size()), m_valueLength(value.size()) {}

  std::string_view getName() const {
    return std::string_view(m_nameStart, m_nameLength);
  }
  std::string_view getValue() const {
    return std::string_view(m_valueStart, m_valueLength);
  }

  void setName(std::string_view s) {
    m_nameStart = s.data();
    m_nameLength = static_cast<std::uint32_t>(s.size());
  }
  void setValue(std::string_view s) {
    m_valueStart = s.data();
    m_valueLength = static_cast<std::uint32_t>(s.size());
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
};

} // namespace sde4::domain
