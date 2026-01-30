/**
 * @file ast_output.cpp
 * @brief Example: Output AST information for structs using xccmeta
 *
 * This example demonstrates how to use xccmeta to:
 * 1. Parse C++ source code containing structs/classes
 * 2. Filter structs with a specific tag (e.g., @reflect)
 * 3. Output detailed AST information about each struct and its members
 *
 * Usage: xccmeta-example-ast_output
 */

#include <iostream>
#include <string>
#include <xccmeta.hpp>

// Sample input source code with structs marked for reflection
constexpr const char* INPUT_SOURCE = R"(
/// @reflect
struct Person {
    std::string name;
    int age;
    double height;

    void introduce() const {
        std::cout << "Hi, I'm " << name << std::endl;
    }

    bool is_adult() const {
        return age >= 18;
    }
};

/// @reflect
class Employee : public Person {
private:
    double salary;
    std::string department;

public:
    Employee(std::string n, int a, double h, double s, std::string d)
        : Person{n, a, h}, salary(s), department(d) {}

    double get_salary() const { return salary; }
    void set_salary(double s) { salary = s; }

    const std::string& get_department() const { return department; }
    void set_department(const std::string& d) { department = d; }

    void promote(double raise) {
        salary += raise;
    }
};

/// @reflect
struct Point {
    double x, y, z;

    Point(double x = 0, double y = 0, double z = 0)
        : x(x), y(y), z(z) {}

    double distance_from_origin() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};
)";

void print_field_info(const xccmeta::node_ptr& field) {
  std::cout << "    Field: " << field->get_name()
            << " (" << field->get_type().get_spelling() << ")"
            << " [" << xccmeta::access_specifier_to_string(field->get_access()) << "]" << std::endl;
}

void print_method_info(const xccmeta::node_ptr& method) {
  std::cout << "    Method: " << method->get_display_name()
            << " -> " << method->get_return_type().get_spelling()
            << " [" << xccmeta::access_specifier_to_string(method->get_access()) << "]";

  if (method->is_const_method()) {
    std::cout << " [const]";
  }
  if (method->is_virtual()) {
    std::cout << " [virtual]";
  }
  if (method->is_static()) {
    std::cout << " [static]";
  }

  std::cout << std::endl;

  // Print parameters
  auto params = method->get_parameters();
  if (!params.empty()) {
    std::cout << "      Parameters:" << std::endl;
    for (const auto& param : params) {
      std::cout << "        " << param->get_name()
                << " (" << param->get_type().get_spelling() << ")" << std::endl;
    }
  }
}

void print_base_info(const xccmeta::node_ptr& base) {
  std::cout << "    Base: " << base->get_name();
  if (base->is_virtual_base()) {
    std::cout << " [virtual]";
  }
  std::cout << " [" << xccmeta::access_specifier_to_string(base->get_access()) << "]" << std::endl;
}

int main() {
  std::cout << "=== xccmeta AST Output Example ===" << std::endl;
  std::cout << std::endl;

  // Step 1: Set up compile arguments
  xccmeta::compile_args args;
  args.set_standard(xccmeta::language_standard::cxx20);

  // Step 2: Parse the input source code
  std::cout << "[1] Parsing source code..." << std::endl;
  xccmeta::parser parser;
  auto ast = parser.parse(INPUT_SOURCE, args);

  if (!ast) {
    std::cerr << "Error: Failed to parse input source code" << std::endl;
    return 1;
  }

  std::cout << "    Parsed successfully. AST root: " << ast->get_kind_name() << std::endl;

  // Step 3: Find structs/classes with @reflect tag
  std::cout << "[2] Finding structs/classes with @reflect tag..." << std::endl;

  auto reflected_records = ast->find_descendants([](const xccmeta::node_ptr& n) {
    return (n->get_kind() == xccmeta::node::kind::struct_decl ||
            n->get_kind() == xccmeta::node::kind::class_decl) &&
           n->has_tag("reflect");
  });

  std::cout << "    Found " << reflected_records.size() << " record(s) with @reflect tag" << std::endl;
  std::cout << std::endl;

  // Step 4: Output AST information for each struct/class
  std::cout << "[3] AST Information:" << std::endl;
  std::cout << std::string(50, '=') << std::endl;

  for (const auto& record : reflected_records) {
    std::cout << "Record: " << record->get_name() << std::endl;
    std::cout << "  Kind: " << record->get_kind_name() << std::endl;
    std::cout << "  Qualified Name: " << record->get_qualified_name() << std::endl;

    if (!record->get_comment().empty()) {
      std::cout << "  Comment: " << record->get_comment() << std::endl;
    }

    // Base classes
    auto bases = record->get_bases();
    if (!bases.empty()) {
      std::cout << "  Base Classes:" << std::endl;
      for (const auto& base : bases) {
        print_base_info(base);
      }
    }

    // Fields
    auto fields = record->get_fields();
    if (!fields.empty()) {
      std::cout << "  Fields:" << std::endl;
      for (const auto& field : fields) {
        print_field_info(field);
      }
    }

    // Methods
    auto methods = record->get_methods();
    if (!methods.empty()) {
      std::cout << "  Methods:" << std::endl;
      for (const auto& method : methods) {
        print_method_info(method);
      }
    }

    std::cout << std::string(50, '-') << std::endl;
  }

  std::cout << std::endl;
  std::cout << "=== AST Output Complete ===" << std::endl;

  return 0;
}
