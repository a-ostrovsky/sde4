#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace sde4::util {

constexpr std::size_t findFirstNotNameChar(std::string_view s,
                                           std::size_t pos) noexcept {
  constexpr auto isValid = [](char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-' || c == ':' ||
           c == '.';
  };
  const auto view = s.substr(pos);
  const auto it = std::ranges::find_if_not(view, isValid);
  return pos + static_cast<std::size_t>(it - view.begin());
}

constexpr std::size_t findFirstNotWhitespace(std::string_view s,
                                             std::size_t pos) noexcept {
  constexpr auto isWS = [](char c) noexcept {
    return c == ' ' || (c >= '\t' && c <= '\r');
  };
  const auto view = s.substr(pos);
  const auto it = std::ranges::find_if_not(view, isWS);
  return pos + static_cast<std::size_t>(it - view.begin());
}

} // namespace sde4::util
