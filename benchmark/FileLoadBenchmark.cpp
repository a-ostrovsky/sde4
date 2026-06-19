#include "../src/parser/private/FileReader.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <string_view>

namespace sde4::benchmark {

void loadFileAndClobber(const std::filesystem::path& path) {
  auto content = sde4::parser::FileReader::loadFileIntoMemory(path);
  asm volatile("" : : "r"(content->data()), "r"(content->size()) : "memory");
}

void createXmlFile(const std::filesystem::path& path, std::size_t targetBytes) {
  static constexpr std::array roots = {
      std::string_view{"data"},    std::string_view{"config"},
      std::string_view{"catalog"}, std::string_view{"items"},
      std::string_view{"profile"},
  };
  static constexpr std::array tags = {
      std::string_view{"item"},    std::string_view{"entry"},
      std::string_view{"field"},   std::string_view{"param"},
      std::string_view{"record"},  std::string_view{"setting"},
      std::string_view{"prop"},    std::string_view{"value"},
      std::string_view{"section"}, std::string_view{"block"},
  };
  static constexpr std::array attrs = {
      std::string_view{"id"},    std::string_view{"name"},
      std::string_view{"type"},  std::string_view{"key"},
      std::string_view{"class"}, std::string_view{"ref"},
      std::string_view{"lang"},  std::string_view{"state"},
  };
  static constexpr std::array words = {
      std::string_view{"alpha"}, std::string_view{"beta"},
      std::string_view{"gamma"}, std::string_view{"delta"},
      std::string_view{"value"}, std::string_view{"test"},
      std::string_view{"main"},  std::string_view{"sample"},
  };

  std::mt19937 rng{std::random_device{}()};
  auto rnd = [&](int lo, int hi) {
    return std::uniform_int_distribution{lo, hi}(rng);
  };

  std::string xml;
  xml.reserve(targetBytes + 256);

  auto root = roots[rnd(0, static_cast<int>(roots.size()) - 1)];
  xml += '<';
  xml += root;
  xml += ">\n";

  while (xml.size() + 48 < targetBytes) {
    switch (rnd(0, 5)) {
    case 0: {
      auto t = tags[rnd(0, static_cast<int>(tags.size()) - 1)];
      xml += "  <";
      xml += t;
      for (int a = rnd(1, 3); a > 0; --a) {
        xml += ' ';
        xml += attrs[rnd(0, static_cast<int>(attrs.size()) - 1)];
        xml += "=\"";
        xml += words[rnd(0, static_cast<int>(words.size()) - 1)];
        xml += '"';
      }
      xml += '>';
      xml += words[rnd(0, static_cast<int>(words.size()) - 1)];
      xml.append(rnd(0, 32), ' ');
      xml += "</";
      xml += t;
      xml += ">\n";
      break;
    }
    case 1: {
      auto t = tags[rnd(0, static_cast<int>(tags.size()) - 1)];
      auto t2 = tags[rnd(0, static_cast<int>(tags.size()) - 1)];
      xml += "  <";
      xml += t;
      xml += ">\n    <";
      xml += t2;
      xml += '>';
      xml += words[rnd(0, static_cast<int>(words.size()) - 1)];
      xml += "</";
      xml += t2;
      xml += ">\n  </";
      xml += t;
      xml += ">\n";
      break;
    }
    case 2:
      xml += "  <!-- ";
      xml += words[rnd(0, static_cast<int>(words.size()) - 1)];
      xml += " -->\n";
      break;
    case 3: {
      xml += "  <";
      xml += tags[rnd(0, static_cast<int>(tags.size()) - 1)];
      for (int a = rnd(1, 2); a > 0; --a) {
        xml += ' ';
        xml += attrs[rnd(0, static_cast<int>(attrs.size()) - 1)];
        xml += "=\"";
        xml += words[rnd(0, static_cast<int>(words.size()) - 1)];
        xml += '"';
      }
      xml += "/>\n";
      break;
    }
    case 4:
      xml += "  <text>";
      xml.append(rnd(16, 128), 'a' + rnd(0, 25));
      xml += "</text>\n";
      break;
    case 5: {
      auto t = tags[rnd(0, static_cast<int>(tags.size()) - 1)];
      xml += "  <";
      xml += t;
      xml += ">\n    ";
      xml += words[rnd(0, static_cast<int>(words.size()) - 1)];
      xml.append(rnd(0, 16), '.');
      xml += "\n  </";
      xml += t;
      xml += ">\n";
      break;
    }
    }
  }

  xml += "</";
  xml += root;
  xml += ">\n";

  constexpr auto closeFile = [](FILE* f) noexcept { std::fclose(f); };
  std::unique_ptr<FILE, decltype(closeFile)> f{
      fopen(path.string().c_str(), "wb"),
  };
  if (!f)
    return;
  fwrite(xml.data(), 1, xml.size(), f.get());
}

} // namespace sde4::benchmark

int main(int argc, char* argv[]) {
  if (argc >= 4 && !std::strcmp(argv[1], "create")) {
    sde4::benchmark::createXmlFile(argv[2], std::stoul(argv[3]));
    return 0;
  }
  if (argc >= 3 && !std::strcmp(argv[1], "read")) {
    sde4::benchmark::loadFileAndClobber(argv[2]);
    return 0;
  }
  std::fprintf(stderr,
               "usage: benchmark read <file>  |  benchmark create <file> "
               "<bytes>\n");
  return 1;
}
