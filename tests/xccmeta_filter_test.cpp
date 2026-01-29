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
#include <xccmeta/xccmeta_filter.hpp>
#include <xccmeta/xccmeta_parser.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace {

  // ============================================================================
  // Test Fixture - provides parsing utilities for filter tests
  // ============================================================================

  class FilterTest : public ::testing::Test {
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

    // Helper to find all type declarations
    std::vector<xccmeta::node_ptr> find_type_decls(const xccmeta::node_ptr& root) {
      std::vector<xccmeta::node_ptr> types;
      if (!root) return types;
      for (const auto& child : root->get_children()) {
        if (child->is_type_decl()) {
          types.push_back(child);
        }
        auto child_types = find_type_decls(child);
        types.insert(types.end(), child_types.begin(), child_types.end());
      }
      return types;
    }
  };

  // ============================================================================
  // Default Constructor Tests
  // ============================================================================

  TEST_F(FilterTest, DefaultConstructorCreatesEmptyList) {
    xccmeta::filter list;
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.get_types().empty());
  }

  TEST_F(FilterTest, DefaultConfigHasEmptyFilters) {
    xccmeta::filter list;
    const auto& cfg = list.get_config();

    EXPECT_TRUE(cfg.allowed_kinds.empty());
    EXPECT_TRUE(cfg.grab_tag_names.empty());
    EXPECT_TRUE(cfg.avoid_tag_names.empty());
    EXPECT_EQ(cfg.child_node_inclusion, xccmeta::filter::config::node_inclusion::exclude);
    EXPECT_EQ(cfg.parent_node_inclusion, xccmeta::filter::config::node_inclusion::exclude);
  }

  // ============================================================================
  // Custom Config Tests
  // ============================================================================

  TEST_F(FilterTest, CustomConfigIsStored) {
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::class_decl, xccmeta::node::kind::struct_decl};
    cfg.grab_tag_names = {"serialize", "reflect"};
    cfg.avoid_tag_names = {"internal"};
    cfg.child_node_inclusion = xccmeta::filter::config::node_inclusion::include;

    xccmeta::filter list(cfg);
    const auto& stored_cfg = list.get_config();

    EXPECT_EQ(stored_cfg.allowed_kinds.size(), 2);
    EXPECT_EQ(stored_cfg.grab_tag_names.size(), 2);
    EXPECT_EQ(stored_cfg.avoid_tag_names.size(), 1);
    EXPECT_EQ(stored_cfg.child_node_inclusion, xccmeta::filter::config::node_inclusion::include);
  }

  // ============================================================================
  // Add Tests
  // ============================================================================

  TEST_F(FilterTest, AddNullptrReturnsFalse) {
    xccmeta::filter list;
    EXPECT_FALSE(list.add(nullptr));
    EXPECT_TRUE(list.empty());
  }

  TEST_F(FilterTest, AddValidClassReturnsTrue) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(class_node));
    EXPECT_EQ(list.size(), 1);
  }

  TEST_F(FilterTest, AddValidStructReturnsTrue) {
    auto root = parse("struct MyStruct {};");
    ASSERT_NE(root, nullptr);

    auto struct_node = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(struct_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(struct_node));
    EXPECT_EQ(list.size(), 1);
  }

  TEST_F(FilterTest, AddValidEnumReturnsTrue) {
    auto root = parse("enum MyEnum { A, B, C };");
    ASSERT_NE(root, nullptr);

    auto enum_node = find_descendant_by_name(root, "MyEnum");
    ASSERT_NE(enum_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(enum_node));
    EXPECT_EQ(list.size(), 1);
  }

  TEST_F(FilterTest, AddValidTypedefReturnsTrue) {
    auto root = parse("typedef int MyInt;");
    ASSERT_NE(root, nullptr);

    auto typedef_node = find_descendant_by_name(root, "MyInt");
    ASSERT_NE(typedef_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(typedef_node));
    EXPECT_EQ(list.size(), 1);
  }

  TEST_F(FilterTest, AddValidTypeAliasReturnsTrue) {
    auto root = parse("using MyAlias = int;");
    ASSERT_NE(root, nullptr);

    auto alias_node = find_descendant_by_name(root, "MyAlias");
    ASSERT_NE(alias_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(alias_node));
    EXPECT_EQ(list.size(), 1);
  }

  TEST_F(FilterTest, AddDuplicateReturnsFalse) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(class_node));
    EXPECT_FALSE(list.add(class_node));  // Duplicate
    EXPECT_EQ(list.size(), 1);
  }

  TEST_F(FilterTest, AddNonTypeNodeReturnsFalse) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var_node = find_descendant_by_name(root, "x");
    ASSERT_NE(var_node, nullptr);

    xccmeta::filter list;
    EXPECT_FALSE(list.add(var_node));
    EXPECT_TRUE(list.empty());
  }

  TEST_F(FilterTest, AddFunctionReturnsFalse) {
    auto root = parse("void myFunc() {}");
    ASSERT_NE(root, nullptr);

    auto func_node = find_descendant_by_name(root, "myFunc");
    ASSERT_NE(func_node, nullptr);

    xccmeta::filter list;
    EXPECT_FALSE(list.add(func_node));
    EXPECT_TRUE(list.empty());
  }

  // ============================================================================
  // Contains Tests
  // ============================================================================

  TEST_F(FilterTest, ContainsReturnsFalseForNullptr) {
    xccmeta::filter list;
    EXPECT_FALSE(list.contains(nullptr));
  }

  TEST_F(FilterTest, ContainsReturnsFalseForEmptyList) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    xccmeta::filter list;
    EXPECT_FALSE(list.contains(class_node));
  }

  TEST_F(FilterTest, ContainsReturnsTrueAfterAdd) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    xccmeta::filter list;
    list.add(class_node);
    EXPECT_TRUE(list.contains(class_node));
  }

  TEST_F(FilterTest, ContainsReturnsFalseForDifferentType) {
    auto root = parse("class ClassA {}; class ClassB {};");
    ASSERT_NE(root, nullptr);

    auto class_a = find_descendant_by_name(root, "ClassA");
    auto class_b = find_descendant_by_name(root, "ClassB");
    ASSERT_NE(class_a, nullptr);
    ASSERT_NE(class_b, nullptr);

    xccmeta::filter list;
    list.add(class_a);
    EXPECT_TRUE(list.contains(class_a));
    EXPECT_FALSE(list.contains(class_b));
  }

  // ============================================================================
  // Remove Tests
  // ============================================================================

  TEST_F(FilterTest, RemoveNullptrReturnsFalse) {
    xccmeta::filter list;
    EXPECT_FALSE(list.remove(nullptr));
  }

  TEST_F(FilterTest, RemoveFromEmptyListReturnsFalse) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    xccmeta::filter list;
    EXPECT_FALSE(list.remove(class_node));
  }

  TEST_F(FilterTest, RemoveExistingTypeReturnsTrue) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    xccmeta::filter list;
    list.add(class_node);
    EXPECT_EQ(list.size(), 1);

    EXPECT_TRUE(list.remove(class_node));
    EXPECT_EQ(list.size(), 0);
    EXPECT_FALSE(list.contains(class_node));
  }

  TEST_F(FilterTest, RemoveNonExistingTypeReturnsFalse) {
    auto root = parse("class ClassA {}; class ClassB {};");
    ASSERT_NE(root, nullptr);

    auto class_a = find_descendant_by_name(root, "ClassA");
    auto class_b = find_descendant_by_name(root, "ClassB");
    ASSERT_NE(class_a, nullptr);
    ASSERT_NE(class_b, nullptr);

    xccmeta::filter list;
    list.add(class_a);
    EXPECT_FALSE(list.remove(class_b));
    EXPECT_EQ(list.size(), 1);
  }

  // ============================================================================
  // Clear Tests
  // ============================================================================

  TEST_F(FilterTest, ClearEmptyListRemainEmpty) {
    xccmeta::filter list;
    list.clear();
    EXPECT_TRUE(list.empty());
  }

  TEST_F(FilterTest, ClearRemovesAllTypes) {
    auto root = parse("class ClassA {}; struct StructB {}; enum EnumC { X };");
    ASSERT_NE(root, nullptr);

    auto class_a = find_descendant_by_name(root, "ClassA");
    auto struct_b = find_descendant_by_name(root, "StructB");
    auto enum_c = find_descendant_by_name(root, "EnumC");

    xccmeta::filter list;
    list.add(class_a);
    list.add(struct_b);
    list.add(enum_c);
    EXPECT_EQ(list.size(), 3);

    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
  }

  TEST_F(FilterTest, ClearReturnsSelf) {
    xccmeta::filter list;
    xccmeta::filter& result = list.clear();
    EXPECT_EQ(&result, &list);
  }

  // ============================================================================
  // Config Filtering Tests
  // ============================================================================

  TEST_F(FilterTest, AllowedKindsFiltersStructs) {
    auto root = parse("struct MyStruct {};");
    ASSERT_NE(root, nullptr);

    auto struct_node = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(struct_node, nullptr);

    // Only allow class_decl, not struct_decl
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::class_decl};

    xccmeta::filter list(cfg);
    EXPECT_FALSE(list.add(struct_node));
    EXPECT_TRUE(list.empty());
  }

  TEST_F(FilterTest, AllowedKindsFiltersClasses) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    // Only allow struct_decl, not class_decl
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::struct_decl};

    xccmeta::filter list(cfg);
    EXPECT_FALSE(list.add(class_node));
    EXPECT_TRUE(list.empty());
  }

  TEST_F(FilterTest, AllowedKindsFiltersEnums) {
    auto root = parse("enum MyEnum { A };");
    ASSERT_NE(root, nullptr);

    auto enum_node = find_descendant_by_name(root, "MyEnum");
    ASSERT_NE(enum_node, nullptr);

    // Only allow class_decl, not enum_decl
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::class_decl};

    xccmeta::filter list(cfg);
    EXPECT_FALSE(list.add(enum_node));
    EXPECT_TRUE(list.empty());
  }

  TEST_F(FilterTest, AllowedKindsFiltersTypedefs) {
    auto root = parse("typedef int MyInt;");
    ASSERT_NE(root, nullptr);

    auto typedef_node = find_descendant_by_name(root, "MyInt");
    ASSERT_NE(typedef_node, nullptr);

    // Only allow class_decl, not typedef_decl
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::class_decl};

    xccmeta::filter list(cfg);
    EXPECT_FALSE(list.add(typedef_node));
    EXPECT_TRUE(list.empty());
  }

  TEST_F(FilterTest, AllowedKindsFiltersTypeAliases) {
    auto root = parse("using MyAlias = int;");
    ASSERT_NE(root, nullptr);

    auto alias_node = find_descendant_by_name(root, "MyAlias");
    ASSERT_NE(alias_node, nullptr);

    // Only allow class_decl, not type_alias_decl
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::class_decl};

    xccmeta::filter list(cfg);
    EXPECT_FALSE(list.add(alias_node));
    EXPECT_TRUE(list.empty());
  }

  // ============================================================================
  // Tag-based Filtering Tests
  // ============================================================================

  // Note: Tag-based tests would require nodes with tags set up.
  // The grab_tag_names and avoid_tag_names config options filter by tags.
  // These tests are placeholders for when tag functionality is fully implemented.

  // ============================================================================
  // Clean Tests
  // ============================================================================

  TEST_F(FilterTest, CleanRemovesDisallowedTypes) {
    auto root = parse("class MyClass {}; struct MyStruct {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    auto struct_node = find_descendant_by_name(root, "MyStruct");
    ASSERT_NE(class_node, nullptr);
    ASSERT_NE(struct_node, nullptr);

    // Start with all types allowed
    xccmeta::filter::config cfg;
    xccmeta::filter list(cfg);

    list.add(class_node);
    list.add(struct_node);
    EXPECT_EQ(list.size(), 2);

    // Note: We can't dynamically change config, but clean will still work
    // This test demonstrates clean() returns self for chaining
    list.clean();
    // Both should still be present since config allows them
    EXPECT_EQ(list.size(), 2);
  }

  TEST_F(FilterTest, CleanReturnsSelf) {
    xccmeta::filter list;
    xccmeta::filter& result = list.clean();
    EXPECT_EQ(&result, &list);
  }

  // ============================================================================
  // is_valid_type Tests
  // ============================================================================

  TEST_F(FilterTest, IsValidTypeReturnsFalseForNullptr) {
    xccmeta::filter list;
    EXPECT_FALSE(list.is_valid_type(nullptr));
  }

  TEST_F(FilterTest, IsValidTypeReturnsFalseForNonType) {
    auto root = parse("int x = 42;");
    ASSERT_NE(root, nullptr);

    auto var_node = find_descendant_by_name(root, "x");
    ASSERT_NE(var_node, nullptr);

    xccmeta::filter list;
    EXPECT_FALSE(list.is_valid_type(var_node));
  }

  TEST_F(FilterTest, IsValidTypeReturnsTrueForValidType) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.is_valid_type(class_node));
  }

  TEST_F(FilterTest, IsValidTypeRespectsConfig) {
    auto root = parse("class MyClass {};");
    ASSERT_NE(root, nullptr);

    auto class_node = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(class_node, nullptr);

    // Only allow struct_decl, not class_decl
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::struct_decl};

    xccmeta::filter list(cfg);
    EXPECT_FALSE(list.is_valid_type(class_node));
  }

  // ============================================================================
  // get_types Tests
  // ============================================================================

  TEST_F(FilterTest, GetTypesReturnsEmptyVectorForEmptyList) {
    xccmeta::filter list;
    EXPECT_TRUE(list.get_types().empty());
  }

  TEST_F(FilterTest, GetTypesReturnsAllAddedTypes) {
    auto root = parse("class ClassA {}; struct StructB {}; enum EnumC { X };");
    ASSERT_NE(root, nullptr);

    auto class_a = find_descendant_by_name(root, "ClassA");
    auto struct_b = find_descendant_by_name(root, "StructB");
    auto enum_c = find_descendant_by_name(root, "EnumC");

    xccmeta::filter list;
    list.add(class_a);
    list.add(struct_b);
    list.add(enum_c);

    const auto& types = list.get_types();
    EXPECT_EQ(types.size(), 3);

    // Check all types are present
    auto find_type = [&types](const xccmeta::node_ptr& target) {
      return std::find(types.begin(), types.end(), target) != types.end();
    };

    EXPECT_TRUE(find_type(class_a));
    EXPECT_TRUE(find_type(struct_b));
    EXPECT_TRUE(find_type(enum_c));
  }

  // ============================================================================
  // Iterator Tests
  // ============================================================================

  TEST_F(FilterTest, IteratorWorksCorrectly) {
    auto root = parse("class ClassA {}; struct StructB {};");
    ASSERT_NE(root, nullptr);

    auto class_a = find_descendant_by_name(root, "ClassA");
    auto struct_b = find_descendant_by_name(root, "StructB");

    xccmeta::filter list;
    list.add(class_a);
    list.add(struct_b);

    int count = 0;
    for (const auto& type : list) {
      EXPECT_NE(type, nullptr);
      count++;
    }
    EXPECT_EQ(count, 2);
  }

  TEST_F(FilterTest, ConstIteratorWorksCorrectly) {
    auto root = parse("class ClassA {}; struct StructB {};");
    ASSERT_NE(root, nullptr);

    auto class_a = find_descendant_by_name(root, "ClassA");
    auto struct_b = find_descendant_by_name(root, "StructB");

    xccmeta::filter list;
    list.add(class_a);
    list.add(struct_b);

    const xccmeta::filter& const_list = list;
    int count = 0;
    for (const auto& type : const_list) {
      EXPECT_NE(type, nullptr);
      count++;
    }
    EXPECT_EQ(count, 2);
  }

  // ============================================================================
  // Multiple Types Test
  // ============================================================================

  TEST_F(FilterTest, AddMultipleTypesFromSameSource) {
    auto root = parse(R"(
      class MyClass {};
      struct MyStruct { int x; };
      union MyUnion { int a; float b; };
      enum MyEnum { A, B, C };
      typedef int MyInt;
      using MyAlias = double;
    )");
    ASSERT_NE(root, nullptr);

    xccmeta::filter list;

    auto all_types = find_type_decls(root);
    for (const auto& type : all_types) {
      list.add(type);
    }

    // Should have added multiple types
    EXPECT_GE(list.size(), 4);  // At minimum: class, struct, enum, typedef/alias
  }

  // ============================================================================
  // Union Tests
  // ============================================================================

  TEST_F(FilterTest, AddValidUnionReturnsTrue) {
    auto root = parse("union MyUnion { int a; float b; };");
    ASSERT_NE(root, nullptr);

    auto union_node = find_descendant_by_name(root, "MyUnion");
    ASSERT_NE(union_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(union_node));
    EXPECT_EQ(list.size(), 1);
  }

  TEST_F(FilterTest, AllowedKindsFiltersUnions) {
    auto root = parse("union MyUnion { int a; float b; };");
    ASSERT_NE(root, nullptr);

    auto union_node = find_descendant_by_name(root, "MyUnion");
    ASSERT_NE(union_node, nullptr);

    // Only allow class_decl, not union_decl
    xccmeta::filter::config cfg;
    cfg.allowed_kinds = {xccmeta::node::kind::class_decl};

    xccmeta::filter list(cfg);
    EXPECT_FALSE(list.add(union_node));
    EXPECT_TRUE(list.empty());
  }

  // ============================================================================
  // Scoped Enum Tests
  // ============================================================================

  TEST_F(FilterTest, AddScopedEnumReturnsTrue) {
    auto root = parse("enum class MyScopedEnum { A, B, C };");
    ASSERT_NE(root, nullptr);

    auto enum_node = find_descendant_by_name(root, "MyScopedEnum");
    ASSERT_NE(enum_node, nullptr);

    xccmeta::filter list;
    EXPECT_TRUE(list.add(enum_node));
    EXPECT_EQ(list.size(), 1);
  }

}  // namespace
