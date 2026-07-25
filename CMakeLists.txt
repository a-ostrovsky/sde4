cmake_minimum_required(VERSION 4.2)
project(sde4 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
add_compile_options(/std:c++latest)
else()
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sde4
  src/main.cpp
  src/parser/private/FileReader.cpp
  src/parser/private/XmlFileParser.cpp
  src/parser/private/XmlParser.cpp
)

add_executable(sde4_benchmark
  benchmark/FileLoadBenchmark.cpp
  src/parser/private/FileReader.cpp
  src/parser/private/XmlParser.cpp
)

target_compile_options(sde4_benchmark PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/std:c++latest>
)

# --- doctest -----------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(doctest
  URL https://raw.githubusercontent.com/doctest/doctest/v2.5.2/doctest/doctest.h
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(doctest)

# --- tests -------------------------------------------------------------------
enable_testing()
add_executable(sde4_tests
  test/main.cpp
  test/parser/XmlParserTest/ElementTest.cpp
  test/parser/XmlParserTest/AttributeTest.cpp
  test/parser/XmlParserTest/CdataTest.cpp
  test/parser/XmlParserTest/CommentTest.cpp
  test/parser/XmlParserTest/ProcessingInstructionTest.cpp
  test/parser/XmlParserTest/DoctypeTest.cpp
  test/parser/XmlParserTest/DeclarationTest.cpp
  test/util/VectorListTest.cpp
  test/util/CharSetTest.cpp
  src/parser/private/XmlParser.cpp
)
target_include_directories(sde4_tests PRIVATE "${doctest_SOURCE_DIR}")
add_test(NAME sde4_tests COMMAND sde4_tests)
