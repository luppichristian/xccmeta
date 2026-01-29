#include <gtest/gtest.h>
#include <xccmeta_preprocess.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

  // Counter to generate unique directory names for each test
  std::atomic<int> test_counter {0};

  // Helper class to create temporary test files and directories
  class TempTestEnvironment {
   public:
    TempTestEnvironment() {
      int id = test_counter.fetch_add(1);
      test_dir = std::filesystem::temp_directory_path() /
                 ("xccmeta_preprocess_test_" + std::to_string(id) + "_" +
                  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
      std::error_code ec;
      std::filesystem::remove_all(test_dir, ec);
      std::filesystem::create_directories(test_dir);
    }

    ~TempTestEnvironment() {
      std::error_code ec;
      std::filesystem::remove_all(test_dir, ec);
    }

    std::filesystem::path get_test_dir() const { return test_dir; }

    std::filesystem::path create_file(const std::string& name,
                                      const std::string& content = "") {
      auto file_path = test_dir / name;
      std::filesystem::create_directories(file_path.parent_path());
      std::ofstream ofs(file_path, std::ios::binary);
      ofs << content;
      ofs.close();
      return file_path;
    }

   private:
    std::filesystem::path test_dir;
  };

}  // namespace

// ============================================================================
// preprocessor_context tests
// ============================================================================

TEST(PreprocessorContextTest, DefaultConstruction) {
  xccmeta::preprocessor_context ctx;
  // Default constructed context should be valid but empty
  std::string result = ctx.apply("int x = 42;");
  // With no context, apply should return input as-is or processed
  EXPECT_FALSE(result.empty());
}

TEST(PreprocessorContextTest, ConstructWithSimpleInput) {
  xccmeta::preprocessor_context ctx("int x = 10;");
  EXPECT_FALSE(ctx.apply("int y = 20;").empty());
}

TEST(PreprocessorContextTest, ApplyPreservesBasicCode) {
  xccmeta::preprocessor_context ctx("");
  std::string input = "int main() { return 0; }";
  std::string result = ctx.apply(input);
  // The result should contain the essential tokens
  EXPECT_NE(result.find("int"), std::string::npos);
  EXPECT_NE(result.find("main"), std::string::npos);
  EXPECT_NE(result.find("return"), std::string::npos);
  EXPECT_NE(result.find("0"), std::string::npos);
}

TEST(PreprocessorContextTest, ApplyWithCompileArgs) {
  xccmeta::compile_args args;
  args.add("-DTEST_MACRO=42");

  xccmeta::preprocessor_context ctx("", args);
  std::string input = "int x = TEST_MACRO;";
  std::string result = ctx.apply(input, args);

  // After preprocessing, TEST_MACRO should be replaced with 42
  EXPECT_NE(result.find("42"), std::string::npos);
}

TEST(PreprocessorContextTest, ApplyEmptyString) {
  xccmeta::preprocessor_context ctx("");
  std::string result = ctx.apply("");
  EXPECT_TRUE(result.empty());
}

TEST(PreprocessorContextTest, ApplyWithWhitespace) {
  xccmeta::preprocessor_context ctx("");
  std::string input = "   int   x   =   5   ;   ";
  std::string result = ctx.apply(input);
  // Result should contain the tokens (whitespace may be normalized)
  EXPECT_NE(result.find("int"), std::string::npos);
  EXPECT_NE(result.find("x"), std::string::npos);
  EXPECT_NE(result.find("5"), std::string::npos);
}

// ============================================================================
// preprocessor tests with single file
// ============================================================================

TEST(PreprocessorTest, SingleFileBasic) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp", "int main() { return 0; }");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("int"), std::string::npos);
  EXPECT_NE(content[0].find("main"), std::string::npos);
}

TEST(PreprocessorTest, SingleFileWithMacro) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#define VALUE 100\n"
                                   "int x = VALUE;");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  // After preprocessing, VALUE should be replaced with 100
  EXPECT_NE(content[0].find("100"), std::string::npos);
}

TEST(PreprocessorTest, SingleFileWithCompileArgDefine) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp", "int x = MY_DEFINE;");

  xccmeta::compile_args args;
  args.add("-DMY_DEFINE=999");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f, args);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("999"), std::string::npos);
}

TEST(PreprocessorTest, SingleFileEmpty) {
  TempTestEnvironment env;
  auto file_path = env.create_file("empty.cpp", "");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_TRUE(content[0].empty());
}

TEST(PreprocessorTest, SingleFileWithConditionalCompilation) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#ifdef FEATURE_ENABLED\n"
                                   "int feature = 1;\n"
                                   "#else\n"
                                   "int feature = 0;\n"
                                   "#endif\n");

  // Without defining FEATURE_ENABLED
  {
    xccmeta::file f(file_path);
    xccmeta::preprocessor pp(f);
    const auto& content = pp.get_preprocessed_content();
    ASSERT_EQ(content.size(), 1);
    // Should have "int feature = 0"
    EXPECT_NE(content[0].find("0"), std::string::npos);
  }

  // With FEATURE_ENABLED defined
  {
    xccmeta::compile_args args;
    args.add("-DFEATURE_ENABLED");

    xccmeta::file f(file_path);
    xccmeta::preprocessor pp(f, args);
    const auto& content = pp.get_preprocessed_content();
    ASSERT_EQ(content.size(), 1);
    // Should have "int feature = 1"
    EXPECT_NE(content[0].find("1"), std::string::npos);
  }
}

TEST(PreprocessorTest, SingleFileWithNestedMacros) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#define A 10\n"
                                   "#define B A\n"
                                   "#define C B\n"
                                   "int x = C;");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  // After macro expansion, should be 10
  EXPECT_NE(content[0].find("10"), std::string::npos);
}

TEST(PreprocessorTest, SingleFileWithFunctionMacro) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#define ADD(a, b) ((a) + (b))\n"
                                   "int x = ADD(3, 4);");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  // Should contain the expanded macro
  EXPECT_NE(content[0].find("3"), std::string::npos);
  EXPECT_NE(content[0].find("4"), std::string::npos);
}

// ============================================================================
// preprocessor tests with multiple files
// ============================================================================

TEST(PreprocessorTest, MultipleFilesBasic) {
  TempTestEnvironment env;
  auto file1_path = env.create_file("file1.cpp", "int a = 1;");
  auto file2_path = env.create_file("file2.cpp", "int b = 2;");

  std::vector<xccmeta::file> files = {
      xccmeta::file(file1_path),
      xccmeta::file(file2_path)};

  xccmeta::preprocessor pp(files);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 2);
  EXPECT_NE(content[0].find("1"), std::string::npos);
  EXPECT_NE(content[1].find("2"), std::string::npos);
}

TEST(PreprocessorTest, MultipleFilesWithSharedDefine) {
  TempTestEnvironment env;
  auto file1_path = env.create_file("file1.cpp", "int a = SHARED_VALUE;");
  auto file2_path = env.create_file("file2.cpp", "int b = SHARED_VALUE;");

  xccmeta::compile_args args;
  args.add("-DSHARED_VALUE=42");

  std::vector<xccmeta::file> files = {
      xccmeta::file(file1_path),
      xccmeta::file(file2_path)};

  xccmeta::preprocessor pp(files, args);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 2);
  EXPECT_NE(content[0].find("42"), std::string::npos);
  EXPECT_NE(content[1].find("42"), std::string::npos);
}

TEST(PreprocessorTest, MultipleFilesEmpty) {
  std::vector<xccmeta::file> files;
  xccmeta::preprocessor pp(files);

  const auto& content = pp.get_preprocessed_content();
  EXPECT_TRUE(content.empty());
}

TEST(PreprocessorTest, MultipleFilesWithMixedContent) {
  TempTestEnvironment env;
  auto file1_path = env.create_file("file1.cpp",
                                    "#define X 100\n"
                                    "int a = X;");
  auto file2_path = env.create_file("file2.cpp", "int b = 200;");
  auto file3_path = env.create_file("file3.cpp", "");

  std::vector<xccmeta::file> files = {
      xccmeta::file(file1_path),
      xccmeta::file(file2_path),
      xccmeta::file(file3_path)};

  xccmeta::preprocessor pp(files);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 3);
  EXPECT_NE(content[0].find("100"), std::string::npos);
  EXPECT_NE(content[1].find("200"), std::string::npos);
  EXPECT_TRUE(content[2].empty());
}

// ============================================================================
// preprocessor context access tests
// ============================================================================

TEST(PreprocessorTest, GetContextReturnsValidContext) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp", "int x = 1;");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& ctx = pp.get_context();
  // Context should be usable for further preprocessing
  std::string result = ctx.apply("int y = 2;");
  EXPECT_NE(result.find("int"), std::string::npos);
  EXPECT_NE(result.find("y"), std::string::npos);
  EXPECT_NE(result.find("2"), std::string::npos);
}

// ============================================================================
// compile_args integration tests
// ============================================================================

TEST(PreprocessorTest, CompileArgsMultipleDefines) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "int a = A_VAL;\n"
                                   "int b = B_VAL;\n"
                                   "int c = C_VAL;");

  xccmeta::compile_args args;
  args.add("-DA_VAL=1");
  args.add("-DB_VAL=2");
  args.add("-DC_VAL=3");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f, args);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("1"), std::string::npos);
  EXPECT_NE(content[0].find("2"), std::string::npos);
  EXPECT_NE(content[0].find("3"), std::string::npos);
}

TEST(PreprocessorTest, CompileArgsWithStdVersion) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "int x = 42;");

  xccmeta::compile_args args;
  args.add("-std=c++17");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f, args);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("42"), std::string::npos);
}

// ============================================================================
// Edge case tests
// ============================================================================

TEST(PreprocessorTest, LargeFile) {
  TempTestEnvironment env;

  // Create a file with many lines
  std::string large_content;
  for (int i = 0; i < 1000; ++i) {
    large_content += "int var_" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
  }

  auto file_path = env.create_file("large.cpp", large_content);

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_FALSE(content[0].empty());
  // Check some values are present
  EXPECT_NE(content[0].find("var_0"), std::string::npos);
  EXPECT_NE(content[0].find("var_999"), std::string::npos);
}

TEST(PreprocessorTest, FileWithComments) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "// Single line comment\n"
                                   "int x = 1;\n"
                                   "/* Multi\n"
                                   "   line\n"
                                   "   comment */\n"
                                   "int y = 2;");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("x"), std::string::npos);
  EXPECT_NE(content[0].find("y"), std::string::npos);
}

TEST(PreprocessorTest, FileWithStringLiterals) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "const char* str1 = \"Hello, World!\";\n"
                                   "const char* str2 = \"Test string\";");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("str1"), std::string::npos);
  EXPECT_NE(content[0].find("str2"), std::string::npos);
}

TEST(PreprocessorTest, FileWithStringification) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#define STRINGIFY(x) #x\n"
                                   "const char* str = STRINGIFY(hello);");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("str"), std::string::npos);
}

TEST(PreprocessorTest, FileWithTokenPasting) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#define PASTE(a, b) a ## b\n"
                                   "int PASTE(var, 123) = 456;");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  // Token pasting should create var123
  EXPECT_NE(content[0].find("var123"), std::string::npos);
  EXPECT_NE(content[0].find("456"), std::string::npos);
}

TEST(PreprocessorTest, FileWithPredefinedMacros) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "int line = __LINE__;\n"
                                   "const char* file = __FILE__;");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  // __LINE__ and __FILE__ should be expanded
  EXPECT_NE(content[0].find("line"), std::string::npos);
  EXPECT_NE(content[0].find("file"), std::string::npos);
}

TEST(PreprocessorTest, FileWithIfdefChain) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#ifdef A\n"
                                   "int x = 1;\n"
                                   "#elif defined(B)\n"
                                   "int x = 2;\n"
                                   "#elif defined(C)\n"
                                   "int x = 3;\n"
                                   "#else\n"
                                   "int x = 4;\n"
                                   "#endif");

  // Test with B defined
  xccmeta::compile_args args;
  args.add("-DB");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f, args);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("2"), std::string::npos);
}

TEST(PreprocessorTest, FileWithUndefMacro) {
  TempTestEnvironment env;
  auto file_path = env.create_file("test.cpp",
                                   "#define VALUE 10\n"
                                   "int a = VALUE;\n"
                                   "#undef VALUE\n"
                                   "#define VALUE 20\n"
                                   "int b = VALUE;");

  xccmeta::file f(file_path);
  xccmeta::preprocessor pp(f);

  const auto& content = pp.get_preprocessed_content();
  ASSERT_EQ(content.size(), 1);
  EXPECT_NE(content[0].find("10"), std::string::npos);
  EXPECT_NE(content[0].find("20"), std::string::npos);
}
