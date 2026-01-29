#include "xccmeta_tags.hpp"
#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

  // Test default constructor
  TEST(TagTest, DefaultConstructor) {
    xccmeta::tag t;
    EXPECT_TRUE(t.get_name().empty());
    EXPECT_TRUE(t.get_args().empty());
    EXPECT_EQ(t.get_args_combined(), "");
    EXPECT_EQ(t.get_full(), "()");
  }

  // Test constructor with name only (no args)
  TEST(TagTest, ConstructorWithNameOnly) {
    xccmeta::tag t("xccmeta::serialize", {});
    EXPECT_EQ(t.get_name(), "xccmeta::serialize");
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test constructor with name and empty vector
  TEST(TagTest, ConstructorWithEmptyVector) {
    std::vector<std::string> empty_args;
    xccmeta::tag t("xccmeta::readonly", empty_args);
    EXPECT_EQ(t.get_name(), "xccmeta::readonly");
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test constructor with name and single argument
  TEST(TagTest, ConstructorWithSingleArg) {
    xccmeta::tag t("xccmeta::serialize", {"json"});
    EXPECT_EQ(t.get_name(), "xccmeta::serialize");
    ASSERT_EQ(t.get_args().size(), 1);
    EXPECT_EQ(t.get_args()[0], "json");
  }

  // Test constructor with name and two arguments
  TEST(TagTest, ConstructorWithTwoArgs) {
    xccmeta::tag t("xccmeta::config", {"key", "value"});
    EXPECT_EQ(t.get_name(), "xccmeta::config");
    ASSERT_EQ(t.get_args().size(), 2);
    EXPECT_EQ(t.get_args()[0], "key");
    EXPECT_EQ(t.get_args()[1], "value");
  }

  // Test constructor with name and multiple arguments
  TEST(TagTest, ConstructorWithMultipleArgs) {
    xccmeta::tag t("xccmeta::export", {"public", "versioned", "async"});
    EXPECT_EQ(t.get_name(), "xccmeta::export");
    ASSERT_EQ(t.get_args().size(), 3);
    EXPECT_EQ(t.get_args()[0], "public");
    EXPECT_EQ(t.get_args()[1], "versioned");
    EXPECT_EQ(t.get_args()[2], "async");
  }

  // Test constructor with many arguments
  TEST(TagTest, ConstructorWithManyArgs) {
    std::vector<std::string> many_args = {"a", "b", "c", "d", "e", "f"};
    xccmeta::tag t("xccmeta::multiarg", many_args);
    EXPECT_EQ(t.get_name(), "xccmeta::multiarg");
    ASSERT_EQ(t.get_args().size(), 6);
    for (size_t i = 0; i < many_args.size(); ++i) {
      EXPECT_EQ(t.get_args()[i], many_args[i]);
    }
  }

  // Test get_args_combined with no args
  TEST(TagTest, GetArgsCombinedEmpty) {
    xccmeta::tag t("xccmeta::readonly", {});
    EXPECT_EQ(t.get_args_combined(), "");
  }

  // Test get_args_combined with single arg
  TEST(TagTest, GetArgsCombinedSingle) {
    xccmeta::tag t("xccmeta::serialize", {"json"});
    EXPECT_EQ(t.get_args_combined(), "json");
  }

  // Test get_args_combined with two args
  TEST(TagTest, GetArgsCombinedTwo) {
    xccmeta::tag t("xccmeta::pair", {"first", "second"});
    EXPECT_EQ(t.get_args_combined(), "first, second");
  }

  // Test get_args_combined with multiple args
  TEST(TagTest, GetArgsCombinedMultiple) {
    xccmeta::tag t("xccmeta::export", {"public", "versioned", "async"});
    EXPECT_EQ(t.get_args_combined(), "public, versioned, async");
  }

  // Test get_full with no args
  TEST(TagTest, GetFullNoArgs) {
    xccmeta::tag t("xccmeta::readonly", {});
    EXPECT_EQ(t.get_full(), "xccmeta::readonly()");
  }

  // Test get_full with single arg
  TEST(TagTest, GetFullSingleArg) {
    xccmeta::tag t("xccmeta::serialize", {"json"});
    EXPECT_EQ(t.get_full(), "xccmeta::serialize(json)");
  }

  // Test get_full with multiple args
  TEST(TagTest, GetFullMultipleArgs) {
    xccmeta::tag t("xccmeta::export", {"public", "versioned"});
    EXPECT_EQ(t.get_full(), "xccmeta::export(public, versioned)");
  }

  // Test simple tag name without namespace
  TEST(TagTest, SimpleTagNameNoNamespace) {
    xccmeta::tag t("deprecated", {"use_new_api"});
    EXPECT_EQ(t.get_name(), "deprecated");
    ASSERT_EQ(t.get_args().size(), 1);
    EXPECT_EQ(t.get_args()[0], "use_new_api");
  }

  // Test tag with numeric string arguments
  TEST(TagTest, NumericStringArgs) {
    xccmeta::tag t("xccmeta::version", {"1", "2", "3"});
    EXPECT_EQ(t.get_name(), "xccmeta::version");
    ASSERT_EQ(t.get_args().size(), 3);
    EXPECT_EQ(t.get_args()[0], "1");
    EXPECT_EQ(t.get_args()[1], "2");
    EXPECT_EQ(t.get_args()[2], "3");
    EXPECT_EQ(t.get_args_combined(), "1, 2, 3");
  }

  // Test roundtrip: construct then get_full
  TEST(TagTest, RoundtripConstructAndGetFull) {
    std::vector<std::string> args = {"arg1", "arg2", "arg3"};
    xccmeta::tag t("xccmeta::test", args);
    EXPECT_EQ(t.get_full(), "xccmeta::test(arg1, arg2, arg3)");
  }

  // Test tag with empty name
  TEST(TagTest, EmptyName) {
    xccmeta::tag t("", {"arg"});
    EXPECT_TRUE(t.get_name().empty());
    ASSERT_EQ(t.get_args().size(), 1);
    EXPECT_EQ(t.get_args()[0], "arg");
  }

  // Test tag with whitespace in arguments
  TEST(TagTest, WhitespaceInArgs) {
    xccmeta::tag t("xccmeta::desc", {"hello world", "foo bar"});
    EXPECT_EQ(t.get_name(), "xccmeta::desc");
    ASSERT_EQ(t.get_args().size(), 2);
    EXPECT_EQ(t.get_args()[0], "hello world");
    EXPECT_EQ(t.get_args()[1], "foo bar");
    EXPECT_EQ(t.get_args_combined(), "hello world, foo bar");
  }

  // Test tag with special characters in arguments
  TEST(TagTest, SpecialCharsInArgs) {
    xccmeta::tag t("xccmeta::regex", {"[a-z]+", "\\d{3}"});
    EXPECT_EQ(t.get_name(), "xccmeta::regex");
    ASSERT_EQ(t.get_args().size(), 2);
    EXPECT_EQ(t.get_args()[0], "[a-z]+");
    EXPECT_EQ(t.get_args()[1], "\\d{3}");
  }

  // Test tag with quoted strings in arguments
  TEST(TagTest, QuotedStringsInArgs) {
    xccmeta::tag t("xccmeta::doc", {"\"This is quoted\"", "'single quoted'"});
    EXPECT_EQ(t.get_name(), "xccmeta::doc");
    ASSERT_EQ(t.get_args().size(), 2);
    EXPECT_EQ(t.get_args()[0], "\"This is quoted\"");
    EXPECT_EQ(t.get_args()[1], "'single quoted'");
  }

  // Test tag with nested namespace
  TEST(TagTest, NestedNamespace) {
    xccmeta::tag t("xccmeta::serialize::json", {"pretty", "compact"});
    EXPECT_EQ(t.get_name(), "xccmeta::serialize::json");
    ASSERT_EQ(t.get_args().size(), 2);
  }

  // Test tag with only namespace prefix
  TEST(TagTest, OnlyNamespacePrefix) {
    xccmeta::tag t("xccmeta::", {});
    EXPECT_EQ(t.get_name(), "xccmeta::");
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test get_name returns const reference
  TEST(TagTest, GetNameReturnsConstRef) {
    xccmeta::tag t("xccmeta::test", {"arg"});
    const std::string& name_ref = t.get_name();
    EXPECT_EQ(name_ref, "xccmeta::test");
  }

  // Test get_args returns const reference
  TEST(TagTest, GetArgsReturnsConstRef) {
    xccmeta::tag t("xccmeta::test", {"arg1", "arg2"});
    const std::vector<std::string>& args_ref = t.get_args();
    ASSERT_EQ(args_ref.size(), 2);
    EXPECT_EQ(args_ref[0], "arg1");
    EXPECT_EQ(args_ref[1], "arg2");
  }

  // Test copy construction
  TEST(TagTest, CopyConstruction) {
    xccmeta::tag original("xccmeta::test", {"a", "b", "c"});
    xccmeta::tag copy(original);

    EXPECT_EQ(copy.get_name(), original.get_name());
    EXPECT_EQ(copy.get_args(), original.get_args());
    EXPECT_EQ(copy.get_full(), original.get_full());
  }

  // Test copy assignment
  TEST(TagTest, CopyAssignment) {
    xccmeta::tag original("xccmeta::test", {"a", "b"});
    xccmeta::tag other("xccmeta::other", {"x"});

    other = original;

    EXPECT_EQ(other.get_name(), "xccmeta::test");
    EXPECT_EQ(other.get_args().size(), 2);
  }

  // Test move construction
  TEST(TagTest, MoveConstruction) {
    xccmeta::tag original("xccmeta::test", {"a", "b", "c"});
    std::string expected_full = original.get_full();

    xccmeta::tag moved(std::move(original));

    EXPECT_EQ(moved.get_full(), expected_full);
  }

  // Test with very long argument
  TEST(TagTest, VeryLongArgument) {
    std::string long_arg(1000, 'x');
    xccmeta::tag t("xccmeta::longarg", {long_arg});
    EXPECT_EQ(t.get_name(), "xccmeta::longarg");
    ASSERT_EQ(t.get_args().size(), 1);
    EXPECT_EQ(t.get_args()[0], long_arg);
  }

  // Test with Unicode characters (if supported)
  TEST(TagTest, UnicodeCharsInArgs) {
    xccmeta::tag t("xccmeta::i18n", {"héllo", "wörld", "日本語"});
    EXPECT_EQ(t.get_name(), "xccmeta::i18n");
    ASSERT_EQ(t.get_args().size(), 3);
    EXPECT_EQ(t.get_args()[0], "héllo");
    EXPECT_EQ(t.get_args()[1], "wörld");
    EXPECT_EQ(t.get_args()[2], "日本語");
  }

  // ==================== Tests for tag::parse() ====================

  // Test parse with no arguments (no parentheses)
  TEST(TagTest, ParseNoArgs) {
    auto t = xccmeta::tag::parse("xccmeta::readonly");
    EXPECT_EQ(t.get_name(), "xccmeta::readonly");
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test parse with empty parentheses
  TEST(TagTest, ParseEmptyParens) {
    auto t = xccmeta::tag::parse("xccmeta::init()");
    EXPECT_EQ(t.get_name(), "xccmeta::init");
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test parse with single argument
  TEST(TagTest, ParseSingleArg) {
    auto t = xccmeta::tag::parse("xccmeta::serialize(json)");
    EXPECT_EQ(t.get_name(), "xccmeta::serialize");
    ASSERT_EQ(t.get_args().size(), 1);
    EXPECT_EQ(t.get_args()[0], "json");
  }

  // Test parse with multiple arguments
  TEST(TagTest, ParseMultipleArgs) {
    auto t = xccmeta::tag::parse("xccmeta::export(public,versioned,async)");
    EXPECT_EQ(t.get_name(), "xccmeta::export");
    ASSERT_EQ(t.get_args().size(), 3);
    EXPECT_EQ(t.get_args()[0], "public");
    EXPECT_EQ(t.get_args()[1], "versioned");
    EXPECT_EQ(t.get_args()[2], "async");
  }

  // Test parse with spaces in arguments (spaces are preserved)
  TEST(TagTest, ParseWithSpaces) {
    auto t = xccmeta::tag::parse("xccmeta::config(option1, option2, option3)");
    EXPECT_EQ(t.get_name(), "xccmeta::config");
    ASSERT_EQ(t.get_args().size(), 3);
    EXPECT_EQ(t.get_args()[0], "option1");
    EXPECT_EQ(t.get_args()[1], " option2");
    EXPECT_EQ(t.get_args()[2], " option3");
  }

  // Test parse with numeric arguments
  TEST(TagTest, ParseNumericArgs) {
    auto t = xccmeta::tag::parse("xccmeta::version(1,2,3)");
    EXPECT_EQ(t.get_name(), "xccmeta::version");
    ASSERT_EQ(t.get_args().size(), 3);
    EXPECT_EQ(t.get_args()[0], "1");
    EXPECT_EQ(t.get_args()[1], "2");
    EXPECT_EQ(t.get_args()[2], "3");
  }

  // Test parse with quoted string argument
  TEST(TagTest, ParseQuotedStringArg) {
    auto t = xccmeta::tag::parse("xccmeta::description(\"This is a test\")");
    EXPECT_EQ(t.get_name(), "xccmeta::description");
    ASSERT_EQ(t.get_args().size(), 1);
    EXPECT_EQ(t.get_args()[0], "\"This is a test\"");
  }

  // Test parse with empty string
  TEST(TagTest, ParseEmptyString) {
    auto t = xccmeta::tag::parse("");
    EXPECT_TRUE(t.get_name().empty());
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test parse with only namespace
  TEST(TagTest, ParseNamespaceOnly) {
    auto t = xccmeta::tag::parse("xccmeta::");
    EXPECT_EQ(t.get_name(), "xccmeta::");
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test parse simple name without namespace
  TEST(TagTest, ParseSimpleName) {
    auto t = xccmeta::tag::parse("deprecated");
    EXPECT_EQ(t.get_name(), "deprecated");
    EXPECT_TRUE(t.get_args().empty());
  }

  // Test parse with nested parentheses (edge case)
  TEST(TagTest, ParseNestedParens) {
    auto t = xccmeta::tag::parse("xccmeta::func(a(b))");
    EXPECT_EQ(t.get_name(), "xccmeta::func");
    // Current implementation takes everything between first '(' and last ')'
    ASSERT_GE(t.get_args().size(), 1);
  }

  // Test parse result can be used with get_full
  TEST(TagTest, ParseAndGetFull) {
    auto t = xccmeta::tag::parse("xccmeta::test(a,b,c)");
    EXPECT_EQ(t.get_full(), "xccmeta::test(a, b, c)");
  }

}  // namespace