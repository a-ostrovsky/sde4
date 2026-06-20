#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace sde4::util {

class CharSet {
  std::array<unsigned char, 256> m_table{};

public:
  constexpr CharSet() = default;

  constexpr explicit CharSet(std::string_view chars) noexcept {
    for (char c : chars) {
      m_table[static_cast<unsigned char>(c)] = 1;
    }
  }

  constexpr std::size_t findFirstNotInSet(std::string_view s,
                                          std::size_t pos) const noexcept {
    while (pos < s.size() && contains(s[pos])) {
      ++pos;
    }
    return pos;
  }

private:
  constexpr bool contains(char c) const noexcept {
    return m_table[static_cast<unsigned char>(c)] != 0;
  }
};

namespace detail {
static constexpr CharSet WhitespaceChars{" \r\n\f\t\v"};
static constexpr util::CharSet ValidNameChars{"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                              "abcdefghijklmnopqrstuvwxyz"
                                              "0123456789"
                                              "_-:."};
} // namespace detail

constexpr std::size_t findFirstNotWhitespace(std::string_view s,
                                             std::size_t pos) noexcept {
  return detail::WhitespaceChars.findFirstNotInSet(s, pos);
}

constexpr std::size_t findFirstNotNameChar(std::string_view s,
                                           std::size_t pos) noexcept {
  return detail::ValidNameChars.findFirstNotInSet(s, pos);
}

} // namespace sde4::util
