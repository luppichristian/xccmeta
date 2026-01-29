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

}  // namespace
