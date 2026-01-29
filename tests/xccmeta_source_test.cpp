#include "xccmeta_source.hpp"
#include <gtest/gtest.h>

#include <string>

namespace {

  // ============================================================================
  // source_location tests
  // ============================================================================

  // Test default constructor
  TEST(SourceLocationTest, DefaultConstructor) {
    xccmeta::source_location loc;
    EXPECT_TRUE(loc.file.empty());
    EXPECT_EQ(loc.line, 0);
    EXPECT_EQ(loc.column, 0);
    EXPECT_EQ(loc.offset, 0);
    EXPECT_FALSE(loc.is_valid());
  }

  // Test parameterized constructor with all args
  TEST(SourceLocationTest, ConstructorWithAllArgs) {
    xccmeta::source_location loc("test.cpp", 10, 5, 100);
    EXPECT_EQ(loc.file, "test.cpp");
    EXPECT_EQ(loc.line, 10);
    EXPECT_EQ(loc.column, 5);
    EXPECT_EQ(loc.offset, 100);
    EXPECT_TRUE(loc.is_valid());
  }

  // Test constructor with default column and offset
  TEST(SourceLocationTest, ConstructorWithDefaults) {
    xccmeta::source_location loc("test.cpp", 10);
    EXPECT_EQ(loc.file, "test.cpp");
    EXPECT_EQ(loc.line, 10);
    EXPECT_EQ(loc.column, 1);
    EXPECT_EQ(loc.offset, 0);
    EXPECT_TRUE(loc.is_valid());
  }

  // Test is_valid with empty file
  TEST(SourceLocationTest, IsValidEmptyFile) {
    xccmeta::source_location loc("", 10, 5);
    EXPECT_FALSE(loc.is_valid());
  }

  // Test is_valid with zero line
  TEST(SourceLocationTest, IsValidZeroLine) {
    xccmeta::source_location loc("test.cpp", 0, 5);
    EXPECT_FALSE(loc.is_valid());
  }

  // Test same_file with matching files
  TEST(SourceLocationTest, SameFileMatching) {
    xccmeta::source_location loc1("test.cpp", 10, 5);
    xccmeta::source_location loc2("test.cpp", 20, 1);
    EXPECT_TRUE(loc1.same_file(loc2));
  }

  // Test same_file with different files
  TEST(SourceLocationTest, SameFileDifferent) {
    xccmeta::source_location loc1("test.cpp", 10, 5);
    xccmeta::source_location loc2("other.cpp", 10, 5);
    EXPECT_FALSE(loc1.same_file(loc2));
  }

  // Test to_string format
  TEST(SourceLocationTest, ToString) {
    xccmeta::source_location loc("src/test.cpp", 42, 7);
    EXPECT_EQ(loc.to_string(), "src/test.cpp:42:7");
  }

  // Test to_string_short format
  TEST(SourceLocationTest, ToStringShort) {
    xccmeta::source_location loc("src/test.cpp", 42, 7);
    EXPECT_EQ(loc.to_string_short(), "src/test.cpp:42");
  }

  // Test equality operator
  TEST(SourceLocationTest, EqualityOperator) {
    xccmeta::source_location loc1("test.cpp", 10, 5, 100);
    xccmeta::source_location loc2("test.cpp", 10, 5, 100);
    EXPECT_TRUE(loc1 == loc2);
    EXPECT_FALSE(loc1 != loc2);
  }

  // Test inequality with different line
  TEST(SourceLocationTest, InequalityDifferentLine) {
    xccmeta::source_location loc1("test.cpp", 10, 5, 100);
    xccmeta::source_location loc2("test.cpp", 11, 5, 100);
    EXPECT_FALSE(loc1 == loc2);
    EXPECT_TRUE(loc1 != loc2);
  }

  // Test inequality with different file
  TEST(SourceLocationTest, InequalityDifferentFile) {
    xccmeta::source_location loc1("test.cpp", 10, 5, 100);
    xccmeta::source_location loc2("other.cpp", 10, 5, 100);
    EXPECT_FALSE(loc1 == loc2);
  }

  // Test less-than operator - same file, different lines
  TEST(SourceLocationTest, LessThanDifferentLines) {
    xccmeta::source_location loc1("test.cpp", 10, 5);
    xccmeta::source_location loc2("test.cpp", 20, 5);
    EXPECT_TRUE(loc1 < loc2);
    EXPECT_FALSE(loc2 < loc1);
  }

  // Test less-than operator - same line, different columns
  TEST(SourceLocationTest, LessThanDifferentColumns) {
    xccmeta::source_location loc1("test.cpp", 10, 5);
    xccmeta::source_location loc2("test.cpp", 10, 10);
    EXPECT_TRUE(loc1 < loc2);
    EXPECT_FALSE(loc2 < loc1);
  }

  // Test less-than operator - different files
  TEST(SourceLocationTest, LessThanDifferentFiles) {
    xccmeta::source_location loc1("a.cpp", 100, 100);
    xccmeta::source_location loc2("b.cpp", 1, 1);
    EXPECT_TRUE(loc1 < loc2);  // "a.cpp" < "b.cpp" lexicographically
  }

  // ============================================================================
  // source_range tests
  // ============================================================================

  // Test default-initialized range
  TEST(SourceRangeTest, DefaultInitialized) {
    xccmeta::source_range range;
    EXPECT_FALSE(range.is_valid());
  }

  // Test from() with two locations
  TEST(SourceRangeTest, FromTwoLocations) {
    xccmeta::source_location start("test.cpp", 10, 1, 100);
    xccmeta::source_location end("test.cpp", 15, 20, 200);
    auto range = xccmeta::source_range::from(start, end);
    EXPECT_EQ(range.start, start);
    EXPECT_EQ(range.end, end);
    EXPECT_TRUE(range.is_valid());
  }

  // Test from() with single location
  TEST(SourceRangeTest, FromSingleLocation) {
    xccmeta::source_location loc("test.cpp", 10, 5, 100);
    auto range = xccmeta::source_range::from(loc);
    EXPECT_EQ(range.start, loc);
    EXPECT_EQ(range.end, loc);
    EXPECT_TRUE(range.is_valid());
    EXPECT_TRUE(range.is_empty());
  }

  // Test is_valid with invalid start
  TEST(SourceRangeTest, IsValidInvalidStart) {
    xccmeta::source_location start("", 10, 1);  // invalid - empty file
    xccmeta::source_location end("test.cpp", 15, 20);
    auto range = xccmeta::source_range::from(start, end);
    EXPECT_FALSE(range.is_valid());
  }

  // Test is_valid with invalid end
  TEST(SourceRangeTest, IsValidInvalidEnd) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 0, 20);  // invalid - zero line
    auto range = xccmeta::source_range::from(start, end);
    EXPECT_FALSE(range.is_valid());
  }

  // Test is_empty
  TEST(SourceRangeTest, IsEmpty) {
    xccmeta::source_location loc("test.cpp", 10, 5, 100);
    auto range = xccmeta::source_range::from(loc, loc);
    EXPECT_TRUE(range.is_empty());
  }

  // Test is_empty when not empty
  TEST(SourceRangeTest, IsNotEmpty) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 10, 5);
    auto range = xccmeta::source_range::from(start, end);
    EXPECT_FALSE(range.is_empty());
  }

  // Test contains location - inside range
  TEST(SourceRangeTest, ContainsLocationInside) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 20, 1);
    auto range = xccmeta::source_range::from(start, end);

    xccmeta::source_location inside("test.cpp", 15, 5);
    EXPECT_TRUE(range.contains(inside));
  }

  // Test contains location - at start
  TEST(SourceRangeTest, ContainsLocationAtStart) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 20, 1);
    auto range = xccmeta::source_range::from(start, end);
    EXPECT_TRUE(range.contains(start));
  }

  // Test contains location - at end
  TEST(SourceRangeTest, ContainsLocationAtEnd) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 20, 1);
    auto range = xccmeta::source_range::from(start, end);
    EXPECT_TRUE(range.contains(end));
  }

  // Test contains location - before range
  TEST(SourceRangeTest, ContainsLocationBefore) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 20, 1);
    auto range = xccmeta::source_range::from(start, end);

    xccmeta::source_location before("test.cpp", 5, 1);
    EXPECT_FALSE(range.contains(before));
  }

  // Test contains location - after range
  TEST(SourceRangeTest, ContainsLocationAfter) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 20, 1);
    auto range = xccmeta::source_range::from(start, end);

    xccmeta::source_location after("test.cpp", 25, 1);
    EXPECT_FALSE(range.contains(after));
  }

  // Test contains location - different file
  TEST(SourceRangeTest, ContainsLocationDifferentFile) {
    xccmeta::source_location start("test.cpp", 10, 1);
    xccmeta::source_location end("test.cpp", 20, 1);
    auto range = xccmeta::source_range::from(start, end);

    xccmeta::source_location other("other.cpp", 15, 1);
    EXPECT_FALSE(range.contains(other));
  }

  // Test contains range - fully inside
  TEST(SourceRangeTest, ContainsRangeFullyInside) {
    auto outer = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 30, 1));

    auto inner = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 15, 1),
        xccmeta::source_location("test.cpp", 25, 1));

    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(inner.contains(outer));
  }

  // Test contains range - partially overlapping
  TEST(SourceRangeTest, ContainsRangePartiallyOverlapping) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 15, 1),
        xccmeta::source_location("test.cpp", 25, 1));

    EXPECT_FALSE(range1.contains(range2));
    EXPECT_FALSE(range2.contains(range1));
  }

  // Test overlaps - overlapping ranges
  TEST(SourceRangeTest, OverlapsTrue) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 15, 1),
        xccmeta::source_location("test.cpp", 25, 1));

    EXPECT_TRUE(range1.overlaps(range2));
    EXPECT_TRUE(range2.overlaps(range1));
  }

  // Test overlaps - non-overlapping ranges
  TEST(SourceRangeTest, OverlapsFalse) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 15, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 20, 1),
        xccmeta::source_location("test.cpp", 25, 1));

    EXPECT_FALSE(range1.overlaps(range2));
    EXPECT_FALSE(range2.overlaps(range1));
  }

  // Test overlaps - adjacent ranges (touching but not overlapping)
  TEST(SourceRangeTest, OverlapsAdjacent) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 15, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 15, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    // Adjacent ranges should not overlap (end of one == start of other)
    EXPECT_FALSE(range1.overlaps(range2));
  }

  // Test overlaps - different files
  TEST(SourceRangeTest, OverlapsDifferentFiles) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("other.cpp", 10, 1),
        xccmeta::source_location("other.cpp", 20, 1));

    EXPECT_FALSE(range1.overlaps(range2));
  }

  // Test length
  TEST(SourceRangeTest, Length) {
    auto range = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1, 100),
        xccmeta::source_location("test.cpp", 15, 1, 250));
    EXPECT_EQ(range.length(), 150);
  }

  // Test length with zero-length range
  TEST(SourceRangeTest, LengthZero) {
    xccmeta::source_location loc("test.cpp", 10, 1, 100);
    auto range = xccmeta::source_range::from(loc);
    EXPECT_EQ(range.length(), 0);
  }

  // Test length with invalid range
  TEST(SourceRangeTest, LengthInvalid) {
    xccmeta::source_range range;
    EXPECT_EQ(range.length(), 0);
  }

  // Test to_string
  TEST(SourceRangeTest, ToString) {
    auto range = xccmeta::source_range::from(
        xccmeta::source_location("src/test.cpp", 10, 5),
        xccmeta::source_location("src/test.cpp", 20, 15));
    EXPECT_EQ(range.to_string(), "src/test.cpp:10:5-20:15");
  }

  // Test equality operator
  TEST(SourceRangeTest, EqualityOperator) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    EXPECT_TRUE(range1 == range2);
    EXPECT_FALSE(range1 != range2);
  }

  // Test inequality operator
  TEST(SourceRangeTest, InequalityOperator) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 25, 1));

    EXPECT_FALSE(range1 == range2);
    EXPECT_TRUE(range1 != range2);
  }

  // Test merge - non-overlapping ranges
  TEST(SourceRangeTest, MergeNonOverlapping) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1, 100),
        xccmeta::source_location("test.cpp", 15, 1, 150));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 20, 1, 200),
        xccmeta::source_location("test.cpp", 25, 1, 250));

    auto merged = xccmeta::source_range::merge(range1, range2);
    EXPECT_EQ(merged.start.line, 10);
    EXPECT_EQ(merged.end.line, 25);
  }

  // Test merge - overlapping ranges
  TEST(SourceRangeTest, MergeOverlapping) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 15, 1),
        xccmeta::source_location("test.cpp", 25, 1));

    auto merged = xccmeta::source_range::merge(range1, range2);
    EXPECT_EQ(merged.start.line, 10);
    EXPECT_EQ(merged.end.line, 25);
  }

  // Test merge - with invalid first range
  TEST(SourceRangeTest, MergeFirstInvalid) {
    xccmeta::source_range invalid;
    auto valid = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto merged = xccmeta::source_range::merge(invalid, valid);
    EXPECT_EQ(merged, valid);
  }

  // Test merge - with invalid second range
  TEST(SourceRangeTest, MergeSecondInvalid) {
    auto valid = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));
    xccmeta::source_range invalid;

    auto merged = xccmeta::source_range::merge(valid, invalid);
    EXPECT_EQ(merged, valid);
  }

  // Test merge - different files (should return first)
  TEST(SourceRangeTest, MergeDifferentFiles) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 20, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("other.cpp", 5, 1),
        xccmeta::source_location("other.cpp", 30, 1));

    auto merged = xccmeta::source_range::merge(range1, range2);
    EXPECT_EQ(merged, range1);
  }

  // Test merge - commutative for same file
  TEST(SourceRangeTest, MergeCommutative) {
    auto range1 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 10, 1),
        xccmeta::source_location("test.cpp", 15, 1));

    auto range2 = xccmeta::source_range::from(
        xccmeta::source_location("test.cpp", 20, 1),
        xccmeta::source_location("test.cpp", 25, 1));

    auto merged1 = xccmeta::source_range::merge(range1, range2);
    auto merged2 = xccmeta::source_range::merge(range2, range1);

    EXPECT_EQ(merged1.start, merged2.start);
    EXPECT_EQ(merged1.end, merged2.end);
  }

}  // namespace
