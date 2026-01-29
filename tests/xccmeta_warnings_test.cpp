#include <gtest/gtest.h>
#include <xccmeta/xccmeta_warnings.hpp>

#include <string>

namespace {

  // ============================================================================
  // Default Constructor Tests
  // ============================================================================

  TEST(CompileWarningsTest, DefaultConstructor) {
    xccmeta::compile_warnings w;
    EXPECT_EQ(w.build(), "");
  }

  // ============================================================================
  // Single Warning Tests
  // ============================================================================

  TEST(CompileWarningsTest, PushSingleWarning) {
    xccmeta::compile_warnings w;
    w.push("This is a warning");

    std::string result = w.build();
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("This is a warning"), std::string::npos);
  }

  TEST(CompileWarningsTest, SingleWarningContainsMSVCPragma) {
    xccmeta::compile_warnings w;
    w.push("Test warning");

    std::string result = w.build();
    EXPECT_NE(result.find("#ifdef _MSC_VER"), std::string::npos);
    EXPECT_NE(result.find("#pragma message"), std::string::npos);
  }

  TEST(CompileWarningsTest, SingleWarningContainsGCCWarning) {
    xccmeta::compile_warnings w;
    w.push("Test warning");

    std::string result = w.build();
    EXPECT_NE(result.find("#else"), std::string::npos);
    EXPECT_NE(result.find("#warning"), std::string::npos);
  }

  TEST(CompileWarningsTest, SingleWarningContainsEndif) {
    xccmeta::compile_warnings w;
    w.push("Test warning");

    std::string result = w.build();
    EXPECT_NE(result.find("#endif"), std::string::npos);
  }

  // ============================================================================
  // Multiple Warnings Tests
  // ============================================================================

  TEST(CompileWarningsTest, PushMultipleWarnings) {
    xccmeta::compile_warnings w;
    w.push("First warning");
    w.push("Second warning");
    w.push("Third warning");

    std::string result = w.build();
    EXPECT_NE(result.find("First warning"), std::string::npos);
    EXPECT_NE(result.find("Second warning"), std::string::npos);
    EXPECT_NE(result.find("Third warning"), std::string::npos);
  }

  TEST(CompileWarningsTest, MultipleWarningsHaveMultiplePragmas) {
    xccmeta::compile_warnings w;
    w.push("Warning 1");
    w.push("Warning 2");

    std::string result = w.build();

    // Count occurrences of #pragma message
    size_t pragma_count = 0;
    size_t pos = 0;
    while ((pos = result.find("#pragma message", pos)) != std::string::npos) {
      ++pragma_count;
      ++pos;
    }
    EXPECT_EQ(pragma_count, 2);
  }

  TEST(CompileWarningsTest, MultipleWarningsHaveMultipleGCCWarnings) {
    xccmeta::compile_warnings w;
    w.push("Warning 1");
    w.push("Warning 2");
    w.push("Warning 3");

    std::string result = w.build();

    // Count occurrences of #warning
    size_t warning_count = 0;
    size_t pos = 0;
    while ((pos = result.find("#warning", pos)) != std::string::npos) {
      ++warning_count;
      ++pos;
    }
    EXPECT_EQ(warning_count, 3);
  }

  // ============================================================================
  // Chaining Tests
  // ============================================================================

  TEST(CompileWarningsTest, PushReturnsReference) {
    xccmeta::compile_warnings w;
    xccmeta::compile_warnings& ref = w.push("Test");
    EXPECT_EQ(&ref, &w);
  }

  TEST(CompileWarningsTest, ChainingMultiplePushes) {
    xccmeta::compile_warnings w;
    w.push("First").push("Second").push("Third");

    std::string result = w.build();
    EXPECT_NE(result.find("First"), std::string::npos);
    EXPECT_NE(result.find("Second"), std::string::npos);
    EXPECT_NE(result.find("Third"), std::string::npos);
  }

  // ============================================================================
  // Empty and Edge Case Tests
  // ============================================================================

  TEST(CompileWarningsTest, EmptyWarningMessage) {
    xccmeta::compile_warnings w;
    w.push("");

    std::string result = w.build();
    // Should still produce output with the preprocessor directives
    EXPECT_NE(result.find("#ifdef _MSC_VER"), std::string::npos);
  }

  TEST(CompileWarningsTest, WarningWithSpecialCharacters) {
    xccmeta::compile_warnings w;
    w.push("Warning: value < 0 && value > 100");

    std::string result = w.build();
    EXPECT_NE(result.find("value < 0 && value > 100"), std::string::npos);
  }

  TEST(CompileWarningsTest, WarningWithNewlines) {
    xccmeta::compile_warnings w;
    w.push("Line1\nLine2");

    std::string result = w.build();
    EXPECT_NE(result.find("Line1\nLine2"), std::string::npos);
  }

  TEST(CompileWarningsTest, WarningWithQuotes) {
    xccmeta::compile_warnings w;
    w.push("Use \"proper\" quotes");

    std::string result = w.build();
    EXPECT_NE(result.find("Use \"proper\" quotes"), std::string::npos);
  }

  // ============================================================================
  // Output Format Tests
  // ============================================================================

  TEST(CompileWarningsTest, MSVCFormatCorrect) {
    xccmeta::compile_warnings w;
    w.push("MyWarning");

    std::string result = w.build();
    EXPECT_NE(result.find("#pragma message(\"Warning: MyWarning\")"), std::string::npos);
  }

  TEST(CompileWarningsTest, GCCFormatCorrect) {
    xccmeta::compile_warnings w;
    w.push("MyWarning");

    std::string result = w.build();
    EXPECT_NE(result.find("#warning \"MyWarning\""), std::string::npos);
  }

  TEST(CompileWarningsTest, StructureOrderCorrect) {
    xccmeta::compile_warnings w;
    w.push("Test");

    std::string result = w.build();

    // Check that #ifdef comes before #else and #else comes before #endif
    size_t ifdef_pos = result.find("#ifdef _MSC_VER");
    size_t else_pos = result.find("#else");
    size_t endif_pos = result.find("#endif");

    EXPECT_LT(ifdef_pos, else_pos);
    EXPECT_LT(else_pos, endif_pos);
  }

  TEST(CompileWarningsTest, MSVCBlockBeforeElse) {
    xccmeta::compile_warnings w;
    w.push("TestMessage");

    std::string result = w.build();

    size_t pragma_pos = result.find("#pragma message");
    size_t else_pos = result.find("#else");

    // #pragma message should appear between #ifdef and #else
    EXPECT_LT(pragma_pos, else_pos);
  }

  TEST(CompileWarningsTest, GCCBlockAfterElse) {
    xccmeta::compile_warnings w;
    w.push("TestMessage");

    std::string result = w.build();

    size_t warning_pos = result.find("#warning");
    size_t else_pos = result.find("#else");

    // #warning should appear after #else
    EXPECT_GT(warning_pos, else_pos);
  }

  // ============================================================================
  // Build Idempotency Tests
  // ============================================================================

  TEST(CompileWarningsTest, BuildCanBeCalledMultipleTimes) {
    xccmeta::compile_warnings w;
    w.push("Test warning");

    std::string result1 = w.build();
    std::string result2 = w.build();

    EXPECT_EQ(result1, result2);
  }

  TEST(CompileWarningsTest, BuildDoesNotClearWarnings) {
    xccmeta::compile_warnings w;
    w.push("Initial warning");

    w.build();  // First build
    w.push("Additional warning");

    std::string result = w.build();
    EXPECT_NE(result.find("Initial warning"), std::string::npos);
    EXPECT_NE(result.find("Additional warning"), std::string::npos);
  }

  // ============================================================================
  // Realistic Usage Tests
  // ============================================================================

  TEST(CompileWarningsTest, DeprecationWarning) {
    xccmeta::compile_warnings w;
    w.push("Function 'old_api' is deprecated, use 'new_api' instead");

    std::string result = w.build();
    EXPECT_NE(result.find("deprecated"), std::string::npos);
    EXPECT_NE(result.find("old_api"), std::string::npos);
    EXPECT_NE(result.find("new_api"), std::string::npos);
  }

  TEST(CompileWarningsTest, TypeMismatchWarning) {
    xccmeta::compile_warnings w;
    w.push("Type mismatch: expected 'int', got 'float'");

    std::string result = w.build();
    EXPECT_NE(result.find("Type mismatch"), std::string::npos);
  }

  TEST(CompileWarningsTest, MultipleContextualWarnings) {
    xccmeta::compile_warnings w;
    w.push("Missing documentation for class 'Foo'")
        .push("Missing documentation for method 'Foo::bar'")
        .push("Missing documentation for parameter 'x'");

    std::string result = w.build();

    // Verify all warnings are present
    EXPECT_NE(result.find("class 'Foo'"), std::string::npos);
    EXPECT_NE(result.find("method 'Foo::bar'"), std::string::npos);
    EXPECT_NE(result.find("parameter 'x'"), std::string::npos);
  }

}  // namespace
