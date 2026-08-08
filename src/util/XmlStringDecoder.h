#pragma once

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>

namespace sde4::util {

// Resolves entity and character references per W3C XML
class XmlStringDecoder {
public:
  static constexpr std::string decode(std::string_view raw) {
    const auto ampPos = raw.find('&');
    if (ampPos == std::string_view::npos) {
      return std::string(raw);
    }
    std::string out;
    out.reserve(raw.size());
    out.append(raw.substr(0, ampPos));

    std::size_t pos = ampPos;
    while (pos < raw.size()) {
      const auto amp = raw.find('&', pos);
      if (amp == std::string_view::npos) {
        out.append(raw.substr(pos));
        break;
      }
      out.append(raw.substr(pos, amp - pos));
      const auto semi = raw.find(';', amp + 1);
      if (semi == std::string_view::npos) {
        out.append(raw.substr(amp));
        break;
      }
      const auto token = raw.substr(amp + 1, semi - amp - 1);
      if (token == "amp") {
        out += '&';
      } else if (token == "lt") {
        out += '<';
      } else if (token == "gt") {
        out += '>';
      } else if (token == "apos") {
        out += '\'';
      } else if (token == "quot") {
        out += '"';
      } else {
        bool resolved = false;
        if (token.starts_with('#')) {
          if (const auto cp = parseCharRef(token)) {
            appendUtf8(out, cp);
            resolved = true;
          }
        }
        if (!resolved) {
          out.append(raw.substr(amp, semi - amp + 1));
        }
      }
      pos = semi + 1;
    }
    return out;
  }

private:
  // Returns the referenced code point, or 0 when the reference is not valid
  static constexpr char32_t parseCharRef(std::string_view token) {
    token.remove_prefix(1);
    int base = 10;
    if (token.starts_with('x') || token.starts_with('X')) {
      base = 16;
      token.remove_prefix(1);
    }
    std::uint32_t value{};
    const auto [ptr, ec] =
        std::from_chars(token.data(), token.data() + token.size(), value, base);
    if (ec != std::errc{} || ptr != token.data() + token.size()) {
      return 0;
    }
    const auto cp = static_cast<char32_t>(value);
    // U+0000 is not an XML character; surrogates and code points past
    // U+10FFFF cannot be encoded as UTF-8.
    if (cp == 0 || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
      return 0;
    }
    return cp;
  }

  // UTF-8: ASCII is 1 byte; multi-byte sequences are a lead byte
  // (0b110/0b1110/0b11110 announcing 2/3/4 bytes) followed by
  // 0b10xxxxxx continuation bytes. Every byte carries 6 payload bits.
  static constexpr void appendUtf8(std::string& out, char32_t cp) {
    if (cp < 0x80) {
      // 0x80 = 0b10000000: below it the code point fits one byte as-is.
      out += static_cast<char>(cp);
    } else if (cp < 0x800) {
      // 2-byte sequence: 0b110xxxxx lead + one 0b10xxxxxx continuation.
      // 0b11000000 = 2-byte lead marker (110 on top, payload below);
      out += static_cast<char>(0b11000000 | (cp >> 6));
      // 0b10000000 = continuation marker (10 + 6 payload slots);
      // 0b00111111 = keep only the 6 payload bits;
      out += static_cast<char>(0b10000000 | (cp & 0b00111111));
    } else if (cp < 0x10000) {
      // 3-byte sequence: 0b1110xxxx lead + two 0b10xxxxxx continuations.
      // 0b11100000 = 3-byte lead marker; cp >> 12 pushes the 12 payload
      // bits of the two continuation bytes below (2 * 6).
      out += static_cast<char>(0b11100000 | (cp >> 12));
      // 0b10000000 = continuation marker; cp >> 6 drops the 6 payload
      // bits of the byte below; 0b00111111 keeps the next 6 (bits 6-11).
      out += static_cast<char>(0b10000000 | ((cp >> 6) & 0b00111111));
      // 0b10000000 = continuation marker;
      out += static_cast<char>(0b10000000 | (cp & 0b00111111));
    } else {
      // 4-byte sequence: 0b11110xxx lead + three 0b10xxxxxx continuations.
      // 0b11110000 = 4-byte lead marker; cp >> 18 drops the 18 payload
      // bits of the three continuation bytes below (3 * 6).
      out += static_cast<char>(0b11110000 | (cp >> 18));
      // 0b10000000 = continuation marker; cp >> 12 + 0b00111111 pick bits
      // 12-17.
      out += static_cast<char>(0b10000000 | ((cp >> 12) & 0b00111111));
      // 0b10000000 = continuation marker; cp >> 6 + 0b00111111 pick bits 6-11.
      out += static_cast<char>(0b10000000 | ((cp >> 6) & 0b00111111));
      // 0b10000000 = continuation marker; 0b00111111 keeps bits 0-5.
      out += static_cast<char>(0b10000000 | (cp & 0b00111111));
    }
  }
};

} // namespace sde4::util
