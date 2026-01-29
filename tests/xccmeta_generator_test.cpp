/*
MIT License

Copyright (c) 2026 Christian Luppi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gtest/gtest.h>
#include <xccmeta/xccmeta_generator.hpp>
#include <xccmeta/xccmeta_parser.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

  // ============================================================================
  // Test Fixture
  // ============================================================================

  class GeneratorTest : public ::testing::Test {
   protected:
    std::string test_output_file;

    void SetUp() override {
      // Create a unique temporary file path for each test
      test_output_file = std::filesystem::temp_directory_path().string() + "/xccmeta_generator_test_" +
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".txt";
    }

    void TearDown() override {
      // Clean up the test file
      if (std::filesystem::exists(test_output_file)) {
        std::filesystem::remove(test_output_file);
      }
    }

    // Helper to read file contents
    std::string read_file_contents(const std::string& path) {
      std::ifstream file(path);
      if (!file.is_open()) return "";
      std::stringstream buffer;
      buffer << file.rdbuf();
      return buffer.str();
    }
  };

  // ============================================================================
  // Constructor Tests
  // ============================================================================

  TEST_F(GeneratorTest, ConstructorCreatesFile) {
    {
      xccmeta::generator gen(test_output_file);
      gen.done();
    }
    EXPECT_TRUE(std::filesystem::exists(test_output_file));
  }

  // ============================================================================
  // out() Tests
  // ============================================================================

  TEST_F(GeneratorTest, OutWritesDataWithNewline) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("Hello World");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_EQ(contents, "Hello World\n");
  }

  TEST_F(GeneratorTest, OutChaining) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("Line 1").out("Line 2").out("Line 3");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_EQ(contents, "Line 1\nLine 2\nLine 3\n");
  }

  TEST_F(GeneratorTest, OutEmptyString) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_EQ(contents, "\n");
  }

  // ============================================================================
  // separator() Tests
  // ============================================================================

  TEST_F(GeneratorTest, SeparatorWritesLine) {
    {
      xccmeta::generator gen(test_output_file);
      gen.separator();
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_EQ(contents, "// ============================================================================\n");
  }

  TEST_F(GeneratorTest, SeparatorChaining) {
    {
      xccmeta::generator gen(test_output_file);
      gen.separator().separator();
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    std::string expected =
        "// ============================================================================\n"
        "// ============================================================================\n";
    EXPECT_EQ(contents, expected);
  }

  // ============================================================================
  // named_separator() Tests
  // ============================================================================

  TEST_F(GeneratorTest, NamedSeparatorWritesNamedSection) {
    {
      xccmeta::generator gen(test_output_file);
      gen.named_separator("Test Section");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("// === Test Section") != std::string::npos);
  }

  TEST_F(GeneratorTest, NamedSeparatorWithEmptyName) {
    {
      xccmeta::generator gen(test_output_file);
      gen.named_separator("");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("// ===") != std::string::npos);
  }

  // ============================================================================
  // warn() Tests
  // ============================================================================

  TEST_F(GeneratorTest, WarnWithMessage) {
    {
      xccmeta::generator gen(test_output_file);
      gen.warn("Test warning message");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("Test warning message") != std::string::npos);
    EXPECT_TRUE(contents.find("Warnings") != std::string::npos);
  }

  TEST_F(GeneratorTest, WarnWithSourceLocation) {
    {
      xccmeta::generator gen(test_output_file);
      xccmeta::source_location loc;
      loc.file = "test.cpp";
      loc.line = 42;
      loc.column = 10;
      gen.warn("Warning at location", loc);
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("Warning at location") != std::string::npos);
  }

  TEST_F(GeneratorTest, WarnWithNullNode) {
    {
      xccmeta::generator gen(test_output_file);
      gen.warn("Null node warning", nullptr);
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("Null node warning") != std::string::npos);
  }

  TEST_F(GeneratorTest, WarnWithValidNode) {
    xccmeta::parser p;
    auto args = xccmeta::compile_args::modern_cxx();
    auto root = p.parse("class TestClass {};", args);
    ASSERT_NE(root, nullptr);

    // Find the class node
    xccmeta::node_ptr class_node = nullptr;
    for (const auto& child : root->get_children()) {
      if (child->get_name() == "TestClass") {
        class_node = child;
        break;
      }
    }
    ASSERT_NE(class_node, nullptr);

    {
      xccmeta::generator gen(test_output_file);
      gen.warn("Warning with node", class_node);
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("Warning with node") != std::string::npos);
  }

  TEST_F(GeneratorTest, MultipleWarnings) {
    {
      xccmeta::generator gen(test_output_file);
      gen.warn("First warning");
      gen.warn("Second warning");
      gen.warn("Third warning");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("First warning") != std::string::npos);
    EXPECT_TRUE(contents.find("Second warning") != std::string::npos);
    EXPECT_TRUE(contents.find("Third warning") != std::string::npos);
  }

  // ============================================================================
  // done() Tests
  // ============================================================================

  TEST_F(GeneratorTest, DoneReturnsTrueOnSuccess) {
    xccmeta::generator gen(test_output_file);
    gen.out("Test content");
    EXPECT_TRUE(gen.done());
  }

  TEST_F(GeneratorTest, DoneClosesFile) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("Test content");
      gen.done();
    }
    // File should be readable after done() is called
    std::string contents = read_file_contents(test_output_file);
    EXPECT_EQ(contents, "Test content\n");
  }

  TEST_F(GeneratorTest, NoWarningsNoWarningSection) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("Just content");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("Warnings") == std::string::npos);
  }

  // ============================================================================
  // Combined Usage Tests
  // ============================================================================

  TEST_F(GeneratorTest, FullGenerationWorkflow) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("#pragma once");
      gen.out("");
      gen.named_separator("Generated Code");
      gen.out("// This is auto-generated code");
      gen.out("class GeneratedClass {};");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("#pragma once") != std::string::npos);
    EXPECT_TRUE(contents.find("Generated Code") != std::string::npos);
    EXPECT_TRUE(contents.find("GeneratedClass") != std::string::npos);
  }

  TEST_F(GeneratorTest, GenerationWithWarnings) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("// Generated file");
      gen.out("class MyClass {};");
      gen.warn("Deprecated usage detected");
      gen.done();
    }
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("// Generated file") != std::string::npos);
    EXPECT_TRUE(contents.find("MyClass") != std::string::npos);
    EXPECT_TRUE(contents.find("Deprecated usage detected") != std::string::npos);
  }

  // ============================================================================
  // Destructor Tests
  // ============================================================================

  TEST_F(GeneratorTest, DestructorCallsDone) {
    {
      xccmeta::generator gen(test_output_file);
      gen.out("Content before destructor");
      // Let destructor be called
    }
    // File should exist and have content
    EXPECT_TRUE(std::filesystem::exists(test_output_file));
    std::string contents = read_file_contents(test_output_file);
    EXPECT_TRUE(contents.find("Content before destructor") != std::string::npos);
  }

}  // namespace
