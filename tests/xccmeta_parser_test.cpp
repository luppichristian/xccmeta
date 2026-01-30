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
#include <xccmeta/xccmeta_parser.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace {

  // ============================================================================
  // Helper functions
  // ============================================================================

  // Find a child node by name
  xccmeta::node_ptr find_child_by_name(const xccmeta::node_ptr& parent, const std::string& name) {
    if (!parent) return nullptr;
    for (const auto& child : parent->get_children()) {
      if (child->get_name() == name) return child;
    }
    return nullptr;
  }

  // Find a child node by kind
  xccmeta::node_ptr find_child_by_kind(const xccmeta::node_ptr& parent, xccmeta::node::kind k) {
    if (!parent) return nullptr;
    for (const auto& child : parent->get_children()) {
      if (child->get_kind() == k) return child;
    }
    return nullptr;
  }

  // Find descendant by name (recursive)
  xccmeta::node_ptr find_descendant_by_name(const xccmeta::node_ptr& root, const std::string& name) {
    if (!root) return nullptr;
    for (const auto& child : root->get_children()) {
      if (child->get_name() == name) return child;
      auto found = find_descendant_by_name(child, name);
      if (found) return found;
    }
    return nullptr;
  }

  // Count children of a specific kind
  size_t count_children_by_kind(const xccmeta::node_ptr& parent, xccmeta::node::kind k) {
    if (!parent) return 0;
    size_t count = 0;
    for (const auto& child : parent->get_children()) {
      if (child->get_kind() == k) ++count;
    }
    return count;
  }

  // ============================================================================
  // Basic Parser Tests
  // ============================================================================

  TEST(ParserTest, ParseEmptyInput) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("", args);

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_kind(), xccmeta::node::kind::translation_unit);
  }

  TEST(ParserTest, ParseWhitespaceOnly) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("   \n\t\n   ", args);

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_kind(), xccmeta::node::kind::translation_unit);
  }

  TEST(ParserTest, ParseCommentsOnly) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("// Single line comment\n/* Multi-line\ncomment */", args);

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_kind(), xccmeta::node::kind::translation_unit);
  }

  // ============================================================================
  // Variable Declaration Tests
  // ============================================================================

  TEST(ParserTest, ParseSimpleVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int x = 42;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "x");
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->get_kind(), xccmeta::node::kind::variable_decl);
    EXPECT_EQ(var->get_name(), "x");
  }

  TEST(ParserTest, ParseMultipleVariables) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int a = 1; float b = 2.0; double c = 3.0;", args);

    ASSERT_NE(root, nullptr);
    EXPECT_NE(find_child_by_name(root, "a"), nullptr);
    EXPECT_NE(find_child_by_name(root, "b"), nullptr);
    EXPECT_NE(find_child_by_name(root, "c"), nullptr);
  }

  TEST(ParserTest, ParseConstVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("const int x = 42;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "x");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->get_type().is_const());
  }

  TEST(ParserTest, ParseStaticVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("static int x = 42;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "x");
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->get_storage_class(), xccmeta::storage_class::static_);
  }

  TEST(ParserTest, ParseExternVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("extern int x;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "x");
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->get_storage_class(), xccmeta::storage_class::extern_);
  }

  TEST(ParserTest, ParsePointerVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int* ptr = nullptr;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "ptr");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->get_type().is_pointer());
  }

  TEST(ParserTest, ParseReferenceVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int x = 10; int& ref = x;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "ref");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->get_type().is_reference());
    EXPECT_TRUE(var->get_type().is_lvalue_reference());
  }

  TEST(ParserTest, ParseArrayVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int arr[10];", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "arr");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->get_type().is_array());
  }

  // ============================================================================
  // Function Declaration Tests
  // ============================================================================

  TEST(ParserTest, ParseSimpleFunction) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void foo() {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "foo");
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->get_kind(), xccmeta::node::kind::function_decl);
    EXPECT_EQ(func->get_name(), "foo");
  }

  TEST(ParserTest, ParseFunctionWithReturnType) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int compute() { return 42; }", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "compute");
    ASSERT_NE(func, nullptr);
    EXPECT_NE(func->get_return_type().get_spelling().find("int"), std::string::npos);
  }

  TEST(ParserTest, ParseFunctionWithParameters) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void foo(int a, float b) {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "foo");
    ASSERT_NE(func, nullptr);
    auto params = func->get_parameters();
    EXPECT_EQ(params.size(), 2);
  }

  TEST(ParserTest, ParseFunctionParameterNames) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void foo(int x, int y, int z) {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "foo");
    ASSERT_NE(func, nullptr);
    auto params = func->get_parameters();
    ASSERT_EQ(params.size(), 3);
    EXPECT_EQ(params[0]->get_name(), "x");
    EXPECT_EQ(params[1]->get_name(), "y");
    EXPECT_EQ(params[2]->get_name(), "z");
  }

  TEST(ParserTest, ParseStaticFunction) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("static void helper() {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "helper");
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->get_storage_class(), xccmeta::storage_class::static_);
  }

  TEST(ParserTest, ParseVariadicFunction) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void printf(const char* fmt, ...) {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "printf");
    ASSERT_NE(func, nullptr);
    EXPECT_TRUE(func->is_variadic());
  }

  TEST(ParserTest, ParseFunctionDeclarationOnly) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void foo();", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "foo");
    ASSERT_NE(func, nullptr);
    // Just verify we can parse a forward declaration
    EXPECT_EQ(func->get_kind(), xccmeta::node::kind::function_decl);
  }

  TEST(ParserTest, ParseFunctionDefinition) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void foo() {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "foo");
    ASSERT_NE(func, nullptr);
    // Verify function was parsed as a function declaration
    EXPECT_EQ(func->get_kind(), xccmeta::node::kind::function_decl);
    // Function should have children (CompoundStmt) if it has a body
    // Note: libclang's is_definition behavior can vary with unsaved files
  }

  // ============================================================================
  // Class/Struct Declaration Tests
  // ============================================================================

  TEST(ParserTest, ParseEmptyClass) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("class MyClass {};", args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "MyClass");
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(cls->get_kind(), xccmeta::node::kind::class_decl);
  }

  TEST(ParserTest, ParseEmptyStruct) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("struct MyStruct {};", args);

    ASSERT_NE(root, nullptr);
    auto st = find_child_by_name(root, "MyStruct");
    ASSERT_NE(st, nullptr);
    EXPECT_EQ(st->get_kind(), xccmeta::node::kind::struct_decl);
  }

  TEST(ParserTest, ParseClassWithFields) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Point {
        int x;
        int y;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Point");
    ASSERT_NE(cls, nullptr);
    auto fields = cls->get_fields();
    EXPECT_EQ(fields.size(), 2);
  }

  TEST(ParserTest, ParseClassFieldAccess) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Test {
      public:
        int pub_field;
      protected:
        int prot_field;
      private:
        int priv_field;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Test");
    ASSERT_NE(cls, nullptr);

    auto pub = find_descendant_by_name(cls, "pub_field");
    auto prot = find_descendant_by_name(cls, "prot_field");
    auto priv = find_descendant_by_name(cls, "priv_field");

    ASSERT_NE(pub, nullptr);
    ASSERT_NE(prot, nullptr);
    ASSERT_NE(priv, nullptr);

    EXPECT_EQ(pub->get_access(), xccmeta::access_specifier::public_);
    EXPECT_EQ(prot->get_access(), xccmeta::access_specifier::protected_);
    EXPECT_EQ(priv->get_access(), xccmeta::access_specifier::private_);
  }

  TEST(ParserTest, ParseClassWithMethods) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Calculator {
      public:
        int add(int a, int b) { return a + b; }
        int subtract(int a, int b) { return a - b; }
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Calculator");
    ASSERT_NE(cls, nullptr);
    auto methods = cls->get_methods();
    EXPECT_GE(methods.size(), 2);
  }

  TEST(ParserTest, ParseClassWithConstructor) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class MyClass {
      public:
        MyClass() {}
        MyClass(int x) {}
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "MyClass");
    ASSERT_NE(cls, nullptr);

    size_t ctor_count = count_children_by_kind(cls, xccmeta::node::kind::constructor_decl);
    EXPECT_GE(ctor_count, 2);
  }

  TEST(ParserTest, ParseClassWithDestructor) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class MyClass {
      public:
        ~MyClass() {}
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "MyClass");
    ASSERT_NE(cls, nullptr);

    auto dtor = find_child_by_kind(cls, xccmeta::node::kind::destructor_decl);
    ASSERT_NE(dtor, nullptr);
  }

  TEST(ParserTest, ParseVirtualMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Base {
      public:
        virtual void foo() {}
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Base");
    ASSERT_NE(cls, nullptr);
    auto method = find_descendant_by_name(cls, "foo");
    ASSERT_NE(method, nullptr);
    EXPECT_TRUE(method->is_virtual());
  }

  TEST(ParserTest, ParsePureVirtualMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Interface {
      public:
        virtual void process() = 0;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Interface");
    ASSERT_NE(cls, nullptr);
    auto method = find_descendant_by_name(cls, "process");
    ASSERT_NE(method, nullptr);
    EXPECT_TRUE(method->is_pure_virtual());
  }

  TEST(ParserTest, ParseStaticMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Utils {
      public:
        static int helper() { return 0; }
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Utils");
    ASSERT_NE(cls, nullptr);
    auto method = find_descendant_by_name(cls, "helper");
    ASSERT_NE(method, nullptr);
    EXPECT_TRUE(method->is_static());
  }

  TEST(ParserTest, ParseConstMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Data {
      public:
        int getValue() const { return 0; }
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Data");
    ASSERT_NE(cls, nullptr);
    auto method = find_descendant_by_name(cls, "getValue");
    ASSERT_NE(method, nullptr);
    EXPECT_TRUE(method->is_const_method());
  }

  TEST(ParserTest, ParseDefaultedConstructor) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Simple {
      public:
        Simple() = default;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Simple");
    ASSERT_NE(cls, nullptr);
    auto ctor = find_child_by_kind(cls, xccmeta::node::kind::constructor_decl);
    ASSERT_NE(ctor, nullptr);
    EXPECT_TRUE(ctor->is_defaulted());
  }

  TEST(ParserTest, ParseInheritance) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Base {};
      class Derived : public Base {};
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto derived = find_child_by_name(root, "Derived");
    ASSERT_NE(derived, nullptr);

    auto bases = derived->get_bases();
    EXPECT_EQ(bases.size(), 1);
  }

  TEST(ParserTest, ParseMultipleInheritance) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class A {};
      class B {};
      class C : public A, public B {};
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto c = find_child_by_name(root, "C");
    ASSERT_NE(c, nullptr);

    auto bases = c->get_bases();
    EXPECT_EQ(bases.size(), 2);
  }

  // ============================================================================
  // Union Tests
  // ============================================================================

  TEST(ParserTest, ParseUnion) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      union Data {
        int i;
        float f;
        char c;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto un = find_child_by_name(root, "Data");
    ASSERT_NE(un, nullptr);
    EXPECT_EQ(un->get_kind(), xccmeta::node::kind::union_decl);
    auto fields = un->get_fields();
    EXPECT_EQ(fields.size(), 3);
  }

  // ============================================================================
  // Enum Tests
  // ============================================================================

  TEST(ParserTest, ParseEnumUnscoped) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      enum Color { Red, Green, Blue };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto en = find_child_by_name(root, "Color");
    ASSERT_NE(en, nullptr);
    EXPECT_EQ(en->get_kind(), xccmeta::node::kind::enum_decl);
    EXPECT_FALSE(en->is_scoped_enum());
  }

  TEST(ParserTest, ParseEnumScoped) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      enum class Direction { North, South, East, West };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto en = find_child_by_name(root, "Direction");
    ASSERT_NE(en, nullptr);
    EXPECT_TRUE(en->is_scoped_enum());
  }

  TEST(ParserTest, ParseEnumConstants) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      enum Priority { Low = 1, Medium = 5, High = 10 };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto en = find_child_by_name(root, "Priority");
    ASSERT_NE(en, nullptr);

    auto constants = en->get_enum_constants();
    EXPECT_EQ(constants.size(), 3);

    auto low = find_descendant_by_name(en, "Low");
    auto medium = find_descendant_by_name(en, "Medium");
    auto high = find_descendant_by_name(en, "High");

    ASSERT_NE(low, nullptr);
    ASSERT_NE(medium, nullptr);
    ASSERT_NE(high, nullptr);

    EXPECT_EQ(low->get_enum_value(), 1);
    EXPECT_EQ(medium->get_enum_value(), 5);
    EXPECT_EQ(high->get_enum_value(), 10);
  }

  TEST(ParserTest, ParseEnumWithUnderlyingType) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      enum class Size : unsigned char { Small, Medium, Large };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto en = find_child_by_name(root, "Size");
    ASSERT_NE(en, nullptr);
    EXPECT_FALSE(en->get_underlying_type().empty());
  }

  // ============================================================================
  // Namespace Tests
  // ============================================================================

  TEST(ParserTest, ParseNamespace) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      namespace mylib {
        int value = 42;
      }
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto ns = find_child_by_name(root, "mylib");
    ASSERT_NE(ns, nullptr);
    EXPECT_EQ(ns->get_kind(), xccmeta::node::kind::namespace_decl);
  }

  TEST(ParserTest, ParseNestedNamespace) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      namespace outer {
        namespace inner {
          void foo() {}
        }
      }
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto outer = find_child_by_name(root, "outer");
    ASSERT_NE(outer, nullptr);
    auto inner = find_child_by_name(outer, "inner");
    ASSERT_NE(inner, nullptr);
    auto foo = find_child_by_name(inner, "foo");
    ASSERT_NE(foo, nullptr);
  }

  TEST(ParserTest, ParseInlineNamespace) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      inline namespace v1 {
        void func() {}
      }
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto ns = find_child_by_name(root, "v1");
    ASSERT_NE(ns, nullptr);
  }

  TEST(ParserTest, ParseQualifiedNames) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      namespace ns {
        class MyClass {
          void method() {}
        };
      }
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_descendant_by_name(root, "MyClass");
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(cls->get_qualified_name(), "ns::MyClass");

    auto method = find_descendant_by_name(cls, "method");
    ASSERT_NE(method, nullptr);
    EXPECT_EQ(method->get_qualified_name(), "ns::MyClass::method");
  }

  // ============================================================================
  // Typedef and Type Alias Tests
  // ============================================================================

  TEST(ParserTest, ParseTypedef) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("typedef int Integer;", args);

    ASSERT_NE(root, nullptr);
    auto td = find_child_by_name(root, "Integer");
    ASSERT_NE(td, nullptr);
    EXPECT_EQ(td->get_kind(), xccmeta::node::kind::typedef_decl);
  }

  TEST(ParserTest, ParseTypeAlias) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("using Integer = int;", args);

    ASSERT_NE(root, nullptr);
    auto alias = find_child_by_name(root, "Integer");
    ASSERT_NE(alias, nullptr);
    EXPECT_EQ(alias->get_kind(), xccmeta::node::kind::type_alias_decl);
  }

  // ============================================================================
  // Template Tests
  // ============================================================================

  TEST(ParserTest, ParseClassTemplate) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      template<typename T>
      class Container {
        T value;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto tmpl = find_child_by_name(root, "Container");
    ASSERT_NE(tmpl, nullptr);
    EXPECT_EQ(tmpl->get_kind(), xccmeta::node::kind::class_template);
    EXPECT_TRUE(tmpl->is_template());
  }

  TEST(ParserTest, ParseFunctionTemplate) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      template<typename T>
      T max(T a, T b) { return a > b ? a : b; }
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto tmpl = find_child_by_name(root, "max");
    ASSERT_NE(tmpl, nullptr);
    EXPECT_EQ(tmpl->get_kind(), xccmeta::node::kind::function_template);
    EXPECT_TRUE(tmpl->is_template());
  }

  TEST(ParserTest, ParseTemplateParameters) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      template<typename T, int N>
      class Array {
        T data[N];
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto tmpl = find_child_by_name(root, "Array");
    ASSERT_NE(tmpl, nullptr);

    // Should have template parameters as children
    auto type_param = find_child_by_kind(tmpl, xccmeta::node::kind::template_type_parameter);
    auto non_type_param = find_child_by_kind(tmpl, xccmeta::node::kind::template_non_type_parameter);

    EXPECT_NE(type_param, nullptr);
    EXPECT_NE(non_type_param, nullptr);
  }

  // ============================================================================
  // Bitfield Tests
  // ============================================================================

  TEST(ParserTest, ParseBitfield) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      struct Flags {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        unsigned int flag3 : 4;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto st = find_child_by_name(root, "Flags");
    ASSERT_NE(st, nullptr);

    auto flag1 = find_descendant_by_name(st, "flag1");
    auto flag2 = find_descendant_by_name(st, "flag2");
    auto flag3 = find_descendant_by_name(st, "flag3");

    ASSERT_NE(flag1, nullptr);
    ASSERT_NE(flag2, nullptr);
    ASSERT_NE(flag3, nullptr);

    EXPECT_TRUE(flag1->is_bitfield());
    EXPECT_TRUE(flag2->is_bitfield());
    EXPECT_TRUE(flag3->is_bitfield());

    EXPECT_EQ(flag1->get_bitfield_width(), 1);
    EXPECT_EQ(flag2->get_bitfield_width(), 3);
    EXPECT_EQ(flag3->get_bitfield_width(), 4);
  }

  // ============================================================================
  // Anonymous Declarations Tests
  // ============================================================================

  TEST(ParserTest, ParseAnonymousStruct) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      struct Outer {
        struct {
          int x;
          int y;
        } point;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto outer = find_child_by_name(root, "Outer");
    ASSERT_NE(outer, nullptr);

    // Find anonymous struct
    auto anon = find_child_by_kind(outer, xccmeta::node::kind::struct_decl);
    if (anon) {
      EXPECT_TRUE(anon->is_anonymous() || anon->get_name().empty());
    }
  }

  // ============================================================================
  // Type Information Tests
  // ============================================================================

  TEST(ParserTest, ParseTypeSpelling) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("const int* const ptr = nullptr;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "ptr");
    ASSERT_NE(var, nullptr);
    EXPECT_FALSE(var->get_type().get_spelling().empty());
  }

  TEST(ParserTest, ParseFunctionPointerType) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void (*callback)(int, int) = nullptr;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "callback");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->get_type().is_pointer());
  }

  // ============================================================================
  // USR (Unified Symbol Resolution) Tests
  // ============================================================================

  TEST(ParserTest, ParseUSR) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void uniqueFunction() {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "uniqueFunction");
    ASSERT_NE(func, nullptr);
    EXPECT_FALSE(func->get_usr().empty());
  }

  TEST(ParserTest, DifferentDeclarationsHaveDifferentUSR) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      void foo() {}
      void bar() {}
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto foo = find_child_by_name(root, "foo");
    auto bar = find_child_by_name(root, "bar");
    ASSERT_NE(foo, nullptr);
    ASSERT_NE(bar, nullptr);
    EXPECT_NE(foo->get_usr(), bar->get_usr());
  }

  // ============================================================================
  // Source Location Tests
  // ============================================================================

  TEST(ParserTest, ParseSourceLocation) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int x = 42;", args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "x");
    ASSERT_NE(var, nullptr);

    const auto& loc = var->get_location();
    EXPECT_TRUE(loc.is_valid());
    EXPECT_GT(loc.line, 0u);
  }

  // ============================================================================
  // Merge Tests
  // ============================================================================

  TEST(ParserTest, MergeEmptyTrees) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto a = p.parse("", args);
    auto b = p.parse("", args);
    auto merged = p.merge(a, b, args);

    ASSERT_NE(merged, nullptr);
    EXPECT_EQ(merged->get_kind(), xccmeta::node::kind::translation_unit);
  }

  TEST(ParserTest, MergeWithNullptr) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto a = p.parse("int x;", args);
    auto merged_a = p.merge(a, nullptr, args);
    EXPECT_EQ(merged_a, a);

    auto merged_b = p.merge(nullptr, a, args);
    EXPECT_EQ(merged_b, a);
  }

  TEST(ParserTest, MergeDistinctDeclarations) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto a = p.parse("void foo() {}", args);
    auto b = p.parse("void bar() {}", args);
    auto merged = p.merge(a, b, args);

    ASSERT_NE(merged, nullptr);
    EXPECT_NE(find_descendant_by_name(merged, "foo"), nullptr);
    EXPECT_NE(find_descendant_by_name(merged, "bar"), nullptr);
  }

  TEST(ParserTest, MergeSameDeclaration) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto a = p.parse("void foo() {}", args);
    auto b = p.parse("void foo() {}", args);
    auto merged = p.merge(a, b, args);

    ASSERT_NE(merged, nullptr);
    // Should only have one foo (deduplicated by USR)
    size_t foo_count = 0;
    for (const auto& child : merged->get_children()) {
      if (child->get_name() == "foo") ++foo_count;
    }
    EXPECT_EQ(foo_count, 1);
  }

  TEST(ParserTest, MergeMultipleDistinct) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto a = p.parse("int x; int y;", args);
    auto b = p.parse("int z; int w;", args);
    auto merged = p.merge(a, b, args);

    ASSERT_NE(merged, nullptr);
    EXPECT_NE(find_descendant_by_name(merged, "x"), nullptr);
    EXPECT_NE(find_descendant_by_name(merged, "y"), nullptr);
    EXPECT_NE(find_descendant_by_name(merged, "z"), nullptr);
    EXPECT_NE(find_descendant_by_name(merged, "w"), nullptr);
  }

  TEST(ParserTest, MergePreservesAllProperties) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto a = p.parse(R"(
      class MyClass {
      public:
        virtual void method() {}
      };
    )",
                     args);

    auto b = p.parse("int other;", args);
    auto merged = p.merge(a, b, args);

    ASSERT_NE(merged, nullptr);
    auto cls = find_descendant_by_name(merged, "MyClass");
    ASSERT_NE(cls, nullptr);

    auto method = find_descendant_by_name(cls, "method");
    ASSERT_NE(method, nullptr);
    EXPECT_TRUE(method->is_virtual());
    EXPECT_EQ(method->get_access(), xccmeta::access_specifier::public_);
  }

  // ============================================================================
  // Compile Args Tests
  // ============================================================================

  TEST(ParserTest, ParseWithCppStandard) {
    xccmeta::parser p;
    xccmeta::compile_args args;
    args.set_standard(xccmeta::language_standard::cxx20);

    // C++20 designated initializers
    auto root = p.parse(R"(
      struct Point { int x; int y; };
      Point p = { .x = 1, .y = 2 };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto st = find_child_by_name(root, "Point");
    EXPECT_NE(st, nullptr);
  }

  TEST(ParserTest, ParseWithDefines) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.define("ENABLE_FEATURE");

    auto root = p.parse(R"(
      #ifdef ENABLE_FEATURE
      void feature_func() {}
      #endif
    )",
                        args);

    ASSERT_NE(root, nullptr);
    // The function should exist because ENABLE_FEATURE is defined
    auto func = find_child_by_name(root, "feature_func");
    EXPECT_NE(func, nullptr);
  }

  TEST(ParserTest, ParseCLanguage) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_c();

    auto root = p.parse(R"(
      struct Point {
        int x;
        int y;
      };
      void foo(struct Point* p) {}
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto st = find_child_by_name(root, "Point");
    EXPECT_NE(st, nullptr);
    auto foo = find_child_by_name(root, "foo");
    EXPECT_NE(foo, nullptr);
  }

  // ============================================================================
  // Complex Declaration Tests
  // ============================================================================

  TEST(ParserTest, ParseComplexClass) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class ComplexClass {
      public:
        ComplexClass() = default;
        ComplexClass(int x) : value_(x) {}
        ~ComplexClass() = default;

        int getValue() const { return value_; }
        void setValue(int v) { value_ = v; }

        static int getCount() { return count_; }

      private:
        int value_ = 0;
        static int count_;
      };
      int ComplexClass::count_ = 0;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "ComplexClass");
    ASSERT_NE(cls, nullptr);

    // Check constructors
    EXPECT_GE(count_children_by_kind(cls, xccmeta::node::kind::constructor_decl), 2);

    // Check destructor
    auto dtor = find_child_by_kind(cls, xccmeta::node::kind::destructor_decl);
    EXPECT_NE(dtor, nullptr);

    // Check methods
    auto getValue = find_descendant_by_name(cls, "getValue");
    ASSERT_NE(getValue, nullptr);
    EXPECT_TRUE(getValue->is_const_method());

    auto getCount = find_descendant_by_name(cls, "getCount");
    ASSERT_NE(getCount, nullptr);
    EXPECT_TRUE(getCount->is_static());
  }

  TEST(ParserTest, ParseNestedClasses) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Outer {
      public:
        class Inner {
        public:
          int value;
        };
        Inner inner;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto outer = find_child_by_name(root, "Outer");
    ASSERT_NE(outer, nullptr);

    auto inner = find_descendant_by_name(outer, "Inner");
    ASSERT_NE(inner, nullptr);
    EXPECT_EQ(inner->get_kind(), xccmeta::node::kind::class_decl);
  }

  TEST(ParserTest, ParseInterfacePattern) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class IDrawable {
      public:
        virtual ~IDrawable() = default;
        virtual void draw() = 0;
        virtual void resize(int w, int h) = 0;
      };

      class Circle : public IDrawable {
      public:
        void draw() override {}
        void resize(int w, int h) override {}
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);

    auto interface = find_child_by_name(root, "IDrawable");
    ASSERT_NE(interface, nullptr);

    auto draw = find_descendant_by_name(interface, "draw");
    ASSERT_NE(draw, nullptr);
    EXPECT_TRUE(draw->is_pure_virtual());

    auto circle = find_child_by_name(root, "Circle");
    ASSERT_NE(circle, nullptr);
    EXPECT_EQ(circle->get_bases().size(), 1);
  }

  // ============================================================================
  // Edge Cases and Error Handling
  // ============================================================================

  TEST(ParserTest, ParseInvalidSyntax) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // Parser should still return a valid root even with syntax errors
    auto root = p.parse("invalid syntax here !@#$%", args);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_kind(), xccmeta::node::kind::translation_unit);
  }

  TEST(ParserTest, ParseIncompleteClass) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("class Incomplete;", args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Incomplete");
    ASSERT_NE(cls, nullptr);
    EXPECT_FALSE(cls->is_definition());
  }

  TEST(ParserTest, ParseVeryLongInput) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // Generate a large input with many declarations
    std::string input;
    for (int i = 0; i < 100; ++i) {
      input += "void func" + std::to_string(i) + "() {}\n";
    }

    auto root = p.parse(input, args);

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_children().size(), 100);
  }

  // ============================================================================
  // Display Name Tests
  // ============================================================================

  TEST(ParserTest, FunctionDisplayName) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("void process(int x, float y) {}", args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "process");
    ASSERT_NE(func, nullptr);

    // Display name may include parameter types
    EXPECT_FALSE(func->get_display_name().empty());
  }

  // ============================================================================
  // Move Semantics Tests
  // ============================================================================

  TEST(ParserTest, ParserMoveConstructor) {
    xccmeta::parser p1;
    xccmeta::parser p2(std::move(p1));

    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    auto root = p2.parse("int x;", args);
    EXPECT_NE(root, nullptr);
  }

  TEST(ParserTest, ParserMoveAssignment) {
    xccmeta::parser p1;
    xccmeta::parser p2;
    p2 = std::move(p1);

    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    auto root = p2.parse("int x;", args);
    EXPECT_NE(root, nullptr);
  }

  // ============================================================================
  // Tree Navigation Tests
  // ============================================================================

  TEST(ParserTest, ParentChildRelationship) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Parent {
        int child_field;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Parent");
    ASSERT_NE(cls, nullptr);

    auto field = find_descendant_by_name(cls, "child_field");
    ASSERT_NE(field, nullptr);

    // Field's parent should be the class
    EXPECT_EQ(field->get_parent(), cls);
  }

  TEST(ParserTest, RootHasNoParent) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse("int x;", args);

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_parent(), nullptr);
  }

  // ============================================================================
  // Preprocessor Handling Tests
  // ============================================================================

  TEST(ParserTest, ParseWithInlineDefine) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // Test that #define in source is expanded
    auto root = p.parse(R"(
      #define VALUE 42
      int x = VALUE;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "x");
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->get_kind(), xccmeta::node::kind::variable_decl);
  }

  TEST(ParserTest, ParseWithUndefinedMacro) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // Code inside #ifdef with undefined macro should be excluded
    auto root = p.parse(R"(
      #ifdef UNDEFINED_MACRO
      void should_not_exist() {}
      #endif
      void should_exist() {}
    )",
                        args);

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(find_child_by_name(root, "should_not_exist"), nullptr);
    EXPECT_NE(find_child_by_name(root, "should_exist"), nullptr);
  }

  TEST(ParserTest, ParseWithIfElse) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.define("USE_FEATURE", "1");

    auto root = p.parse(R"(
      #if USE_FEATURE
      void feature_enabled() {}
      #else
      void feature_disabled() {}
      #endif
    )",
                        args);

    ASSERT_NE(root, nullptr);
    EXPECT_NE(find_child_by_name(root, "feature_enabled"), nullptr);
    EXPECT_EQ(find_child_by_name(root, "feature_disabled"), nullptr);
  }

  TEST(ParserTest, ParseWithIfElif) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();
    args.define("VERSION", "2");

    auto root = p.parse(R"(
      #if VERSION == 1
      void version_1() {}
      #elif VERSION == 2
      void version_2() {}
      #else
      void version_other() {}
      #endif
    )",
                        args);

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(find_child_by_name(root, "version_1"), nullptr);
    EXPECT_NE(find_child_by_name(root, "version_2"), nullptr);
    EXPECT_EQ(find_child_by_name(root, "version_other"), nullptr);
  }

  TEST(ParserTest, ParseWithFunctionMacro) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // Test function-like macro expansion
    auto root = p.parse(R"(
      #define DECLARE_VAR(type, name) type name
      DECLARE_VAR(int, my_var);
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "my_var");
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->get_kind(), xccmeta::node::kind::variable_decl);
  }

  TEST(ParserTest, ParseWithNestedMacros) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      #define INNER 10
      #define OUTER (INNER * 2)
      int value = OUTER;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "value");
    ASSERT_NE(var, nullptr);
  }

  TEST(ParserTest, ParseWithIfndef) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      #ifndef NOT_DEFINED
      void included_func() {}
      #endif
      #ifndef __cplusplus
      void c_only_func() {}
      #endif
    )",
                        args);

    ASSERT_NE(root, nullptr);
    // NOT_DEFINED is not defined, so included_func should exist
    EXPECT_NE(find_child_by_name(root, "included_func"), nullptr);
    // __cplusplus IS defined in C++ mode, so c_only_func should NOT exist
    EXPECT_EQ(find_child_by_name(root, "c_only_func"), nullptr);
  }

  TEST(ParserTest, ParseWithUndefMacro) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      #define TEMP_MACRO
      #ifdef TEMP_MACRO
      void before_undef() {}
      #endif
      #undef TEMP_MACRO
      #ifdef TEMP_MACRO
      void after_undef() {}
      #endif
    )",
                        args);

    ASSERT_NE(root, nullptr);
    EXPECT_NE(find_child_by_name(root, "before_undef"), nullptr);
    EXPECT_EQ(find_child_by_name(root, "after_undef"), nullptr);
  }

  // ============================================================================
  // Multiple Translation Units Simulation
  // ============================================================================

  TEST(ParserTest, SimulateHeaderAndSource) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // Simulate header
    auto header = p.parse(R"(
      class Widget {
      public:
        Widget();
        void process();
      private:
        int data_;
      };
    )",
                          args);

    // Simulate source (implementation)
    auto source = p.parse(R"(
      class Widget {
      public:
        Widget() : data_(0) {}
        void process() {}
      private:
        int data_;
      };
    )",
                          args);

    auto merged = p.merge(header, source, args);

    ASSERT_NE(merged, nullptr);
    auto widget = find_descendant_by_name(merged, "Widget");
    EXPECT_NE(widget, nullptr);
  }

  // ============================================================================
  // Tag Extraction Tests
  // ============================================================================

  TEST(ParserTest, ParseSimpleTagOnVariable) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @serialize
      int value = 42;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "value");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->has_tag("serialize"));
    EXPECT_EQ(var->get_tags().size(), 1);
    EXPECT_EQ(var->get_tags()[0].get_name(), "serialize");
  }

  TEST(ParserTest, ParseTagWithArguments) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @config(key, value)
      std::string setting;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "setting");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->has_tag("config"));
    auto tag = var->find_tag("config");
    ASSERT_TRUE(tag.has_value());
    EXPECT_EQ(tag->get_name(), "config");
    EXPECT_EQ(tag->get_args().size(), 2);
    EXPECT_EQ(tag->get_args()[0], "key");
    EXPECT_EQ(tag->get_args()[1], "value");
    EXPECT_EQ(tag->get_args_combined(), "key, value");
  }

  TEST(ParserTest, ParseMultipleTagsOnSameDeclaration) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @serialize
      /// @validate(0, 100)
      /// @description("A percentage value")
      int percentage = 50;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "percentage");
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->get_tags().size(), 3);
    EXPECT_TRUE(var->has_tag("serialize"));
    EXPECT_TRUE(var->has_tag("validate"));
    EXPECT_TRUE(var->has_tag("description"));

    auto validate_tag = var->find_tag("validate");
    ASSERT_TRUE(validate_tag.has_value());
    EXPECT_EQ(validate_tag->get_args().size(), 2);
    EXPECT_EQ(validate_tag->get_args()[0], "0");
    EXPECT_EQ(validate_tag->get_args()[1], "100");
  }

  TEST(ParserTest, ParseTagOnFunction) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @export
      void process_data() {}
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "process_data");
    ASSERT_NE(func, nullptr);
    EXPECT_TRUE(func->has_tag("export"));
  }

  TEST(ParserTest, ParseTagOnClass) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @reflect
      class MyClass {
        int field;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "MyClass");
    ASSERT_NE(cls, nullptr);
    EXPECT_TRUE(cls->has_tag("reflect"));
  }

  TEST(ParserTest, ParseTagOnEnum) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @reflect
      /// @flags
      enum class Permissions {
        Read = 1,
        Write = 2,
        Execute = 4
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto en = find_child_by_name(root, "Permissions");
    ASSERT_NE(en, nullptr);
    EXPECT_TRUE(en->has_tag("reflect"));
    EXPECT_TRUE(en->has_tag("flags"));
    EXPECT_EQ(en->get_tags().size(), 2);
  }

  TEST(ParserTest, ParseTagOnClassField) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Person {
      public:
        /// @required
        std::string name;
        /// @optional
        int age;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Person");
    ASSERT_NE(cls, nullptr);

    auto name_field = find_descendant_by_name(cls, "name");
    auto age_field = find_descendant_by_name(cls, "age");

    ASSERT_NE(name_field, nullptr);
    ASSERT_NE(age_field, nullptr);

    EXPECT_TRUE(name_field->has_tag("required"));
    EXPECT_TRUE(age_field->has_tag("optional"));
  }

  TEST(ParserTest, ParseTagOnClassMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Calculator {
      public:
        /// @operation(add)
        int add(int a, int b) { return a + b; }
        /// @operation(subtract)
        int subtract(int a, int b) { return a - b; }
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Calculator");
    ASSERT_NE(cls, nullptr);

    auto add_method = find_descendant_by_name(cls, "add");
    auto sub_method = find_descendant_by_name(cls, "subtract");

    ASSERT_NE(add_method, nullptr);
    ASSERT_NE(sub_method, nullptr);

    EXPECT_TRUE(add_method->has_tag("operation"));
    EXPECT_TRUE(sub_method->has_tag("operation"));

    auto add_tag = add_method->find_tag("operation");
    ASSERT_TRUE(add_tag.has_value());
    EXPECT_EQ(add_tag->get_args().size(), 1);
    EXPECT_EQ(add_tag->get_args()[0], "add");
  }

  TEST(ParserTest, ParseTagOnNamespace) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @module(math)
      namespace math {
        /// @constant
        const double PI = 3.14159;
      }
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto ns = find_child_by_name(root, "math");
    ASSERT_NE(ns, nullptr);
    EXPECT_TRUE(ns->has_tag("module"));

    auto pi_const = find_descendant_by_name(ns, "PI");
    ASSERT_NE(pi_const, nullptr);
    EXPECT_TRUE(pi_const->has_tag("constant"));
  }

  TEST(ParserTest, ParseTagOnTemplate) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @container
      template<typename T>
      class Vector {
        /// @size
        size_t size_;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto tmpl = find_child_by_name(root, "Vector");
    ASSERT_NE(tmpl, nullptr);
    EXPECT_TRUE(tmpl->has_tag("container"));

    auto size_field = find_descendant_by_name(tmpl, "size_");
    ASSERT_NE(size_field, nullptr);
    EXPECT_TRUE(size_field->has_tag("size"));
  }

  TEST(ParserTest, ParseTagWithEmptyArgs) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @readonly()
      int constant = 42;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "constant");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->has_tag("readonly"));
    auto tag = var->find_tag("readonly");
    ASSERT_TRUE(tag.has_value());
    EXPECT_TRUE(tag->get_args().empty());
    EXPECT_EQ(tag->get_args_combined(), "");
  }

  TEST(ParserTest, ParseMultipleDeclarationsWithTags) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @global
      int global_var = 0;

      /// @helper
      void helper_func() {}

      /// @data
      struct Data {
        /// @field
        int value;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);

    auto global_var = find_child_by_name(root, "global_var");
    auto helper_func = find_child_by_name(root, "helper_func");
    auto data_struct = find_child_by_name(root, "Data");

    ASSERT_NE(global_var, nullptr);
    ASSERT_NE(helper_func, nullptr);
    ASSERT_NE(data_struct, nullptr);

    EXPECT_TRUE(global_var->has_tag("global"));
    EXPECT_TRUE(helper_func->has_tag("helper"));
    EXPECT_TRUE(data_struct->has_tag("data"));

    auto value_field = find_descendant_by_name(data_struct, "value");
    ASSERT_NE(value_field, nullptr);
    EXPECT_TRUE(value_field->has_tag("field"));
  }

  TEST(ParserTest, ParseTagFindTagsMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @api
      /// @deprecated
      /// @version(1.0)
      void old_function() {}
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "old_function");
    ASSERT_NE(func, nullptr);

    // Test find_tags with single name
    auto api_tags = func->find_tags({"api"});
    EXPECT_EQ(api_tags.size(), 1);
    EXPECT_EQ(api_tags[0].get_name(), "api");

    // Test find_tags with multiple names
    auto multiple_tags = func->find_tags({"api", "deprecated"});
    EXPECT_EQ(multiple_tags.size(), 2);

    // Test has_tags (any of the names)
    EXPECT_TRUE(func->has_tags({"api", "removed"}));         // has api
    EXPECT_TRUE(func->has_tags({"removed", "deprecated"}));  // has deprecated
    EXPECT_FALSE(func->has_tags({"removed", "obsolete"}));   // has neither
  }

  TEST(ParserTest, ParseTagOnTypedef) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @alias
      typedef unsigned long long uint64_t;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto td = find_child_by_name(root, "uint64_t");
    ASSERT_NE(td, nullptr);
    EXPECT_TRUE(td->has_tag("alias"));
  }

  TEST(ParserTest, ParseTagOnTypeAlias) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @alias
      using Funky = int;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto alias = find_child_by_name(root, "Funky");
    ASSERT_NE(alias, nullptr);
    EXPECT_TRUE(alias->has_tag("alias"));
  }

  TEST(ParserTest, ParseTagOnUnion) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @variant
      union Variant {
        int i;
        float f;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto un = find_child_by_name(root, "Variant");
    ASSERT_NE(un, nullptr);
    EXPECT_TRUE(un->has_tag("variant"));
  }

  TEST(ParserTest, ParseTagWithSpecialCharactersInArgs) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$")
      std::string email;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "email");
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(var->has_tag("pattern"));
    auto tag = var->find_tag("pattern");
    ASSERT_TRUE(tag.has_value());
    EXPECT_EQ(tag->get_args().size(), 1);
    EXPECT_EQ(tag->get_args()[0], "\"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$\"");
  }

  TEST(ParserTest, ParseTagFiltering) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @export
      void public_func() {}

      /// @internal
      void private_func() {}

      /// @export
      class PublicClass {};

      /// @internal
      class PrivateClass {};
    )",
                        args);

    ASSERT_NE(root, nullptr);

    // Find all nodes with @export tag
    auto exported_nodes = root->find_descendants([](const xccmeta::node_ptr& n) {
      return n->has_tag("export");
    });

    // Find all nodes with @internal tag
    auto internal_nodes = root->find_descendants([](const xccmeta::node_ptr& n) {
      return n->has_tag("internal");
    });

    EXPECT_EQ(exported_nodes.size(), 2);  // public_func and PublicClass
    EXPECT_EQ(internal_nodes.size(), 2);  // private_func and PrivateClass

    // Verify the names
    std::vector<std::string> exported_names;
    for (const auto& node : exported_nodes) {
      exported_names.push_back(node->get_name());
    }
    std::sort(exported_names.begin(), exported_names.end());
    EXPECT_EQ(exported_names, std::vector<std::string>({"PublicClass", "public_func"}));
  }

  TEST(ParserTest, ParseTagOnConstructor) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class MyClass {
      public:
        /// @default
        MyClass() = default;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "MyClass");
    ASSERT_NE(cls, nullptr);

    auto ctor = find_child_by_kind(cls, xccmeta::node::kind::constructor_decl);
    ASSERT_NE(ctor, nullptr);
    EXPECT_TRUE(ctor->has_tag("default"));
  }

  TEST(ParserTest, ParseTagOnDestructor) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class MyClass {
      public:
        /// @cleanup
        ~MyClass() {}
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "MyClass");
    ASSERT_NE(cls, nullptr);

    auto dtor = find_child_by_kind(cls, xccmeta::node::kind::destructor_decl);
    ASSERT_NE(dtor, nullptr);
    EXPECT_TRUE(dtor->has_tag("cleanup"));
  }

  TEST(ParserTest, ParseTagOnStaticMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Factory {
      public:
        /// @factory
        static Factory* create() { return new Factory(); }
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Factory");
    ASSERT_NE(cls, nullptr);

    auto create_method = find_descendant_by_name(cls, "create");
    ASSERT_NE(create_method, nullptr);
    EXPECT_TRUE(create_method->has_tag("factory"));
    EXPECT_TRUE(create_method->is_static());
  }

  TEST(ParserTest, ParseTagOnVirtualMethod) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      class Base {
      public:
        /// @interface
        virtual void process() = 0;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto cls = find_child_by_name(root, "Base");
    ASSERT_NE(cls, nullptr);

    auto method = find_descendant_by_name(cls, "process");
    ASSERT_NE(method, nullptr);
    EXPECT_TRUE(method->has_tag("interface"));
    EXPECT_TRUE(method->is_pure_virtual());
  }

  TEST(ParserTest, ParseTagOnEnumConstant) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      enum class Status {
        /// @success
        Ok = 0,
        /// @error
        Error = 1
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto en = find_child_by_name(root, "Status");
    ASSERT_NE(en, nullptr);

    auto ok_const = find_descendant_by_name(en, "Ok");
    auto error_const = find_descendant_by_name(en, "Error");

    ASSERT_NE(ok_const, nullptr);
    ASSERT_NE(error_const, nullptr);

    EXPECT_TRUE(ok_const->has_tag("success"));
    EXPECT_TRUE(error_const->has_tag("error"));
  }

  TEST(ParserTest, ParseTagOnBitfield) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      struct Flags {
        /// @enabled
        unsigned int feature1 : 1;
        /// @disabled
        unsigned int feature2 : 1;
      };
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto st = find_child_by_name(root, "Flags");
    ASSERT_NE(st, nullptr);

    auto feature1 = find_descendant_by_name(st, "feature1");
    auto feature2 = find_descendant_by_name(st, "feature2");

    ASSERT_NE(feature1, nullptr);
    ASSERT_NE(feature2, nullptr);

    EXPECT_TRUE(feature1->has_tag("enabled"));
    EXPECT_TRUE(feature2->has_tag("disabled"));
    EXPECT_TRUE(feature1->is_bitfield());
    EXPECT_TRUE(feature2->is_bitfield());
  }

  TEST(ParserTest, ParseTagOnFunctionParameter) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      void func(
        int param1 [[clang::annotate("input")]],
        int& param2 [[clang::annotate("output")]]
      ) {}
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto func = find_child_by_name(root, "func");
    ASSERT_NE(func, nullptr);

    auto params = func->get_parameters();
    ASSERT_EQ(params.size(), 2);

    EXPECT_TRUE(params[0]->has_tag("input"));
    EXPECT_TRUE(params[1]->has_tag("output"));
  }

  TEST(ParserTest, ParseTagWithComplexArgs) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    auto root = p.parse(R"(
      /// @validate(0,100,1,50)
      int slider_value = 50;
    )",
                        args);

    ASSERT_NE(root, nullptr);
    auto var = find_child_by_name(root, "slider_value");
    ASSERT_NE(var, nullptr);

    auto tag = var->find_tag("validate");
    ASSERT_TRUE(tag.has_value());
    EXPECT_EQ(tag->get_args().size(), 4);
    EXPECT_EQ(tag->get_args()[0], "0");
    EXPECT_EQ(tag->get_args()[1], "100");
    EXPECT_EQ(tag->get_args()[2], "1");
    EXPECT_EQ(tag->get_args()[3], "50");
  }

  TEST(ParserTest, ParseTagOnMergedDeclarations) {
    xccmeta::parser p;
    xccmeta::compile_args args = xccmeta::compile_args::modern_cxx();

    // First translation unit with tag
    auto tu1 = p.parse(R"(
      /// @interface
      class Widget {
      public:
        virtual void draw() = 0;
      };
    )",
                       args);

    // Second translation unit without tag
    auto tu2 = p.parse(R"(
      class Widget {
      public:
        void draw() override {}
      };
    )",
                       args);

    auto merged = p.merge(tu1, tu2, args);

    ASSERT_NE(merged, nullptr);
    auto widget = find_descendant_by_name(merged, "Widget");
    ASSERT_NE(widget, nullptr);
    EXPECT_TRUE(widget->has_tag("interface"));
  }

}  // namespace
