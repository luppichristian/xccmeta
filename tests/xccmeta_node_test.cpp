#include "xccmeta_node.hpp"
#include <gtest/gtest.h>
#include "xccmeta_parser.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace {

  // ============================================================================
  // Test Fixture - creates a tree of nodes with tags for testing
  // ============================================================================

  class NodeTagTest : public ::testing::Test {
   protected:
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // Helper to parse code and get the root node
    xccmeta::node_ptr parse(const std::string& code) {
      return p.parse(code, args);
    }

    // Helper to find a descendant by name
    xccmeta::node_ptr find_descendant_by_name(const xccmeta::node_ptr& root, const std::string& name) {
      if (!root) return nullptr;
      for (const auto& child : root->get_children()) {
        if (child->get_name() == name) return child;
        auto found = find_descendant_by_name(child, name);
        if (found) return found;
      }
      return nullptr;
    }
  };

  // ============================================================================
  // Basic tag tests using parsed C++ code with attributes
  // ============================================================================

  // Note: Since libclang may not expose all C++ attributes directly, we test
  // the tag helper functions using nodes that may or may not have tags.
  // The focus is on ensuring the helper functions work correctly.

  // ============================================================================
  // has_tag / has_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, HasTagReturnsFalseWhenNoTags) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var = find_descendant_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    EXPECT_FALSE(var->has_tag("xccmeta::serialize"));
    EXPECT_FALSE(var->has_tag("any_tag"));
  }

  TEST_F(NodeTagTest, HasTagsReturnsFalseWhenNoTags) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var = find_descendant_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    EXPECT_FALSE(var->has_tags({"xccmeta::serialize", "xccmeta::readonly"}));
    EXPECT_FALSE(var->has_tags({}));  // Empty list should return false
  }

  // ============================================================================
  // find_tag / find_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, FindTagReturnsNulloptWhenNoTags) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var = find_descendant_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    auto result = var->find_tag("xccmeta::serialize");
    EXPECT_FALSE(result.has_value());
  }

  TEST_F(NodeTagTest, FindTagsReturnsEmptyWhenNoTags) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var = find_descendant_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    auto results = var->find_tags({"xccmeta::serialize", "xccmeta::readonly"});
    EXPECT_TRUE(results.empty());
  }

  // ============================================================================
  // get_children_by_tag / get_children_by_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, GetChildrenByTagReturnsEmptyWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
        int field3;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    auto children = my_struct->get_children_by_tag("xccmeta::serialize");
    EXPECT_TRUE(children.empty());
  }

  TEST_F(NodeTagTest, GetChildrenByTagsReturnsEmptyWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
        int field3;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    auto children = my_struct->get_children_by_tags({"xccmeta::serialize", "xccmeta::readonly"});
    EXPECT_TRUE(children.empty());
  }

  // ============================================================================
  // get_children_without_tag / get_children_without_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, GetChildrenWithoutTagReturnsAllWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
        int field3;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    // All children should be returned since none have the tag
    auto children = my_struct->get_children_without_tag("xccmeta::serialize");
    EXPECT_EQ(children.size(), my_struct->get_children().size());
  }

  TEST_F(NodeTagTest, GetChildrenWithoutTagsReturnsAllWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
        int field3;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    // All children should be returned since none have any of the tags
    auto children = my_struct->get_children_without_tags({"xccmeta::serialize", "xccmeta::readonly"});
    EXPECT_EQ(children.size(), my_struct->get_children().size());
  }

  // ============================================================================
  // find_child_with_tag / find_child_with_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, FindChildWithTagReturnsNullptrWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    auto child = my_struct->find_child_with_tag("xccmeta::serialize");
    EXPECT_EQ(child, nullptr);
  }

  TEST_F(NodeTagTest, FindChildWithTagsReturnsNullptrWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    auto child = my_struct->find_child_with_tags({"xccmeta::serialize", "xccmeta::readonly"});
    EXPECT_EQ(child, nullptr);
  }

  // ============================================================================
  // find_child_without_tag / find_child_without_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, FindChildWithoutTagReturnsFirstChildWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);
    ASSERT_FALSE(my_struct->get_children().empty());

    // Should return the first child since none have the tag
    auto child = my_struct->find_child_without_tag("xccmeta::serialize");
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child, my_struct->get_children().front());
  }

  TEST_F(NodeTagTest, FindChildWithoutTagsReturnsFirstChildWhenNoMatchingTags) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);
    ASSERT_FALSE(my_struct->get_children().empty());

    // Should return the first child since none have any of the tags
    auto child = my_struct->find_child_without_tags({"xccmeta::serialize", "xccmeta::readonly"});
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child, my_struct->get_children().front());
  }

  TEST_F(NodeTagTest, FindChildWithoutTagReturnsNullptrWhenNoChildren) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var = find_descendant_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    // Variable has no children, should return nullptr
    auto child = var->find_child_without_tag("xccmeta::serialize");
    EXPECT_EQ(child, nullptr);
  }

  // ============================================================================
  // get_parent_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, GetParentTagsReturnsEmptyWhenNoParent) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    // Root has no parent
    auto parent_tags = root->get_parent_tags();
    EXPECT_TRUE(parent_tags.empty());
  }

  TEST_F(NodeTagTest, GetParentTagsReturnsEmptyWhenParentsHaveNoTags) {
    auto root = parse(R"(
      namespace ns {
        struct MyStruct {
          int field;
        };
      }
    )");
    ASSERT_NE(root, nullptr);

    auto field = find_descendant_by_name(root, "field");
    ASSERT_NE(field, nullptr);

    // No tags on parents
    auto parent_tags = field->get_parent_tags();
    EXPECT_TRUE(parent_tags.empty());
  }

  // ============================================================================
  // get_all_tags tests
  // ============================================================================

  TEST_F(NodeTagTest, GetAllTagsReturnsEmptyWhenNoTags) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var = find_descendant_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    auto all_tags = var->get_all_tags();
    EXPECT_TRUE(all_tags.empty());
  }

  TEST_F(NodeTagTest, GetAllTagsReturnsEmptyForRoot) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto all_tags = root->get_all_tags();
    EXPECT_TRUE(all_tags.empty());
  }

  // ============================================================================
  // Consistency tests - ensure all functions behave consistently
  // ============================================================================

  TEST_F(NodeTagTest, TagFunctionsAreConsistent) {
    auto root = parse(R"(
      struct Parent {
        struct Child {
          int value;
        };
      };
    )");
    ASSERT_NE(root, nullptr);

    auto parent = find_descendant_by_name(root, "Parent");
    auto child = find_descendant_by_name(root, "Child");
    auto value = find_descendant_by_name(root, "value");

    ASSERT_NE(parent, nullptr);
    ASSERT_NE(child, nullptr);
    ASSERT_NE(value, nullptr);

    // All should have empty tags
    EXPECT_TRUE(parent->get_tags().empty());
    EXPECT_TRUE(child->get_tags().empty());
    EXPECT_TRUE(value->get_tags().empty());

    // All should have empty parent tags
    EXPECT_TRUE(parent->get_parent_tags().empty());
    EXPECT_TRUE(child->get_parent_tags().empty());
    EXPECT_TRUE(value->get_parent_tags().empty());

    // has_tag should be false for all
    EXPECT_FALSE(parent->has_tag("any"));
    EXPECT_FALSE(child->has_tag("any"));
    EXPECT_FALSE(value->has_tag("any"));

    // has_tags should be false for all
    EXPECT_FALSE(parent->has_tags({"tag1", "tag2"}));
    EXPECT_FALSE(child->has_tags({"tag1", "tag2"}));
    EXPECT_FALSE(value->has_tags({"tag1", "tag2"}));
  }

  // ============================================================================
  // Edge cases
  // ============================================================================

  TEST_F(NodeTagTest, TagFunctionsHandleEmptyStringTag) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var = find_descendant_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    // Empty string tag should also return false/empty
    EXPECT_FALSE(var->has_tag(""));
    EXPECT_FALSE(var->has_tags({""}));
    EXPECT_FALSE(var->find_tag("").has_value());
    EXPECT_TRUE(var->find_tags({""}).empty());
  }

  TEST_F(NodeTagTest, GetChildrenWithoutEmptyTagList) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    // Empty tag list - all children should be returned (they don't have "any" of the empty set of tags)
    auto children = my_struct->get_children_without_tags({});
    EXPECT_EQ(children.size(), my_struct->get_children().size());
  }

  TEST_F(NodeTagTest, GetChildrenByEmptyTagList) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
        int field2;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    // Empty tag list - no children should match
    auto children = my_struct->get_children_by_tags({});
    EXPECT_TRUE(children.empty());
  }

  TEST_F(NodeTagTest, FindChildWithEmptyTagList) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);

    // Empty tag list - should return nullptr (no tag to match)
    auto child = my_struct->find_child_with_tags({});
    EXPECT_EQ(child, nullptr);
  }

  TEST_F(NodeTagTest, FindChildWithoutEmptyTagList) {
    auto root = parse(R"(
      struct MyStruct {
        int field1;
      };
    )");
    ASSERT_NE(root, nullptr);

    auto my_struct = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(my_struct, nullptr);
    ASSERT_FALSE(my_struct->get_children().empty());

    // Empty tag list - should return first child (no tag to exclude)
    auto child = my_struct->find_child_without_tags({});
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child, my_struct->get_children().front());
  }

}  // namespace
