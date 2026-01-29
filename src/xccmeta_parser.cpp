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

#include "xccmeta/xccmeta_parser.hpp"
#include "libclang_include.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace xccmeta {

  // ============================================================================
  // Internal parser implementation class (friend of node and type_info)
  // ============================================================================

  class parser_impl {
   public:
    // Convert CXString to std::string and dispose
    static std::string cx_string_to_std(CXString cx_str) {
      const char* c_str = clang_getCString(cx_str);
      std::string result = c_str ? c_str : "";
      clang_disposeString(cx_str);
      return result;
    }

    // Convert CXCursorKind to node::kind
    static node::kind cursor_kind_to_node_kind(CXCursorKind ck) {
      switch (ck) {
        case CXCursor_TranslationUnit:           return node::kind::translation_unit;
        case CXCursor_Namespace:                 return node::kind::namespace_decl;
        case CXCursor_NamespaceAlias:            return node::kind::namespace_alias;
        case CXCursor_UsingDirective:            return node::kind::using_directive;
        case CXCursor_UsingDeclaration:          return node::kind::using_declaration;
        case CXCursor_ClassDecl:                 return node::kind::class_decl;
        case CXCursor_StructDecl:                return node::kind::struct_decl;
        case CXCursor_UnionDecl:                 return node::kind::union_decl;
        case CXCursor_EnumDecl:                  return node::kind::enum_decl;
        case CXCursor_EnumConstantDecl:          return node::kind::enum_constant_decl;
        case CXCursor_TypedefDecl:               return node::kind::typedef_decl;
        case CXCursor_TypeAliasDecl:             return node::kind::type_alias_decl;
        case CXCursor_FieldDecl:                 return node::kind::field_decl;
        case CXCursor_CXXMethod:                 return node::kind::method_decl;
        case CXCursor_Constructor:               return node::kind::constructor_decl;
        case CXCursor_Destructor:                return node::kind::destructor_decl;
        case CXCursor_ConversionFunction:        return node::kind::conversion_decl;
        case CXCursor_FunctionDecl:              return node::kind::function_decl;
        case CXCursor_FunctionTemplate:          return node::kind::function_template;
        case CXCursor_ParmDecl:                  return node::kind::parameter_decl;
        case CXCursor_VarDecl:                   return node::kind::variable_decl;
        case CXCursor_ClassTemplate:             return node::kind::class_template;
        case CXCursor_TemplateTypeParameter:     return node::kind::template_type_parameter;
        case CXCursor_NonTypeTemplateParameter:  return node::kind::template_non_type_parameter;
        case CXCursor_TemplateTemplateParameter: return node::kind::template_template_parameter;
        case CXCursor_FriendDecl:                return node::kind::friend_decl;
        case CXCursor_CXXBaseSpecifier:          return node::kind::base_specifier;
        case CXCursor_LinkageSpec:               return node::kind::linkage_spec;
        case CXCursor_StaticAssert:              return node::kind::static_assert_decl;
        default:                                 return node::kind::unknown;
      }
    }

    // Convert CX_CXXAccessSpecifier to access_specifier
    static access_specifier cx_access_to_access(CX_CXXAccessSpecifier acc) {
      switch (acc) {
        case CX_CXXPublic:    return access_specifier::public_;
        case CX_CXXProtected: return access_specifier::protected_;
        case CX_CXXPrivate:   return access_specifier::private_;
        default:              return access_specifier::invalid;
      }
    }

    // Convert CX_StorageClass to storage_class
    static storage_class cx_storage_to_storage(CX_StorageClass sc) {
      switch (sc) {
        case CX_SC_Extern:   return storage_class::extern_;
        case CX_SC_Static:   return storage_class::static_;
        case CX_SC_Register: return storage_class::register_;
        case CX_SC_Auto:     return storage_class::auto_;
        default:             return storage_class::none;
      }
    }

    // Populate type_info from CXType
    static void populate_type_info(type_info& ti, CXType cx_type) {
      ti.set_spelling(cx_string_to_std(clang_getTypeSpelling(cx_type)));
      ti.set_canonical(cx_string_to_std(clang_getTypeSpelling(clang_getCanonicalType(cx_type))));

      ti.set_const(clang_isConstQualifiedType(cx_type) != 0);
      ti.set_volatile(clang_isVolatileQualifiedType(cx_type) != 0);
      ti.set_restrict(clang_isRestrictQualifiedType(cx_type) != 0);

      ti.set_pointer(cx_type.kind == CXType_Pointer);
      ti.set_lvalue_reference(cx_type.kind == CXType_LValueReference);
      ti.set_rvalue_reference(cx_type.kind == CXType_RValueReference);
      ti.set_reference(ti.is_lvalue_reference() || ti.is_rvalue_reference());
      ti.set_array(cx_type.kind == CXType_ConstantArray || cx_type.kind == CXType_IncompleteArray ||
                   cx_type.kind == CXType_VariableArray || cx_type.kind == CXType_DependentSizedArray);

      // Check for function pointer
      if (cx_type.kind == CXType_Pointer) {
        CXType pointee = clang_getPointeeType(cx_type);
        ti.set_function_pointer(pointee.kind == CXType_FunctionProto || pointee.kind == CXType_FunctionNoProto);
      }

      // Pointee type for pointers/references
      if (ti.is_pointer() || ti.is_reference()) {
        CXType pointee = clang_getPointeeType(cx_type);
        ti.set_pointee_type(cx_string_to_std(clang_getTypeSpelling(pointee)));
      }

      // Array info
      if (ti.is_array()) {
        CXType elem_type = clang_getArrayElementType(cx_type);
        ti.set_array_element_type(cx_string_to_std(clang_getTypeSpelling(elem_type)));
        long long arr_size = clang_getArraySize(cx_type);
        ti.set_array_size(arr_size >= 0 ? arr_size : -1);
      }

      // Size and alignment
      long long size = clang_Type_getSizeOf(cx_type);
      ti.set_size_bytes(size >= 0 ? size : -1);
      long long align = clang_Type_getAlignOf(cx_type);
      ti.set_alignment(align >= 0 ? align : -1);
    }

    // Populate source_location from CXSourceLocation
    static source_location cx_location_to_source(CXSourceLocation cx_loc) {
      CXFile file;
      unsigned line, column, offset;
      clang_getSpellingLocation(cx_loc, &file, &line, &column, &offset);

      std::string filename;
      if (file) {
        filename = cx_string_to_std(clang_getFileName(file));
      }

      return source_location(filename, line, column, offset);
    }

    // Populate source_range from CXSourceRange
    static source_range cx_range_to_source(CXSourceRange cx_range) {
      return source_range::from(
          cx_location_to_source(clang_getRangeStart(cx_range)),
          cx_location_to_source(clang_getRangeEnd(cx_range)));
    }

    // Populate a node from a cursor
    static void populate_node_from_cursor(node_ptr n, CXCursor cursor) {
      // Names
      n->set_name(cx_string_to_std(clang_getCursorSpelling(cursor)));
      n->set_display_name(cx_string_to_std(clang_getCursorDisplayName(cursor)));
      n->set_usr(cx_string_to_std(clang_getCursorUSR(cursor)));

      // Mangled name (if available)
      CXString mangled = clang_Cursor_getMangling(cursor);
      const char* mangled_str = clang_getCString(mangled);
      if (mangled_str && mangled_str[0] != '\0') {
        n->set_mangled_name(mangled_str);
      }
      clang_disposeString(mangled);

      // Build qualified name by traversing semantic parents
      std::string qualified;
      CXCursor sem_parent = clang_getCursorSemanticParent(cursor);
      std::vector<std::string> parts;
      while (!clang_Cursor_isNull(sem_parent) &&
             clang_getCursorKind(sem_parent) != CXCursor_TranslationUnit) {
        std::string part = cx_string_to_std(clang_getCursorSpelling(sem_parent));
        if (!part.empty()) {
          parts.push_back(part);
        }
        sem_parent = clang_getCursorSemanticParent(sem_parent);
      }
      for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
        qualified += *it + "::";
      }
      qualified += n->get_name();
      n->set_qualified_name(qualified);

      // Location and extent
      n->set_location(cx_location_to_source(clang_getCursorLocation(cursor)));
      n->set_extent(cx_range_to_source(clang_getCursorExtent(cursor)));

      // Type info
      CXType cx_type = clang_getCursorType(cursor);
      if (cx_type.kind != CXType_Invalid) {
        populate_type_info(n->get_type_mutable(), cx_type);
      }

      // Return type for functions/methods
      CXType result_type = clang_getCursorResultType(cursor);
      if (result_type.kind != CXType_Invalid) {
        populate_type_info(n->get_return_type_mutable(), result_type);
      }

      // Access specifier
      n->set_access(cx_access_to_access(clang_getCXXAccessSpecifier(cursor)));

      // Storage class
      n->set_storage_class(cx_storage_to_storage(clang_Cursor_getStorageClass(cursor)));

      // Definition vs declaration
      n->set_definition(clang_isCursorDefinition(cursor) != 0);

      // Method/function properties
      CXCursorKind kind = clang_getCursorKind(cursor);
      if (kind == CXCursor_CXXMethod || kind == CXCursor_FunctionDecl ||
          kind == CXCursor_Constructor || kind == CXCursor_Destructor) {
        n->set_virtual(clang_CXXMethod_isVirtual(cursor) != 0);
        n->set_pure_virtual(clang_CXXMethod_isPureVirtual(cursor) != 0);
        n->set_static(clang_CXXMethod_isStatic(cursor) != 0);
        n->set_const_method(clang_CXXMethod_isConst(cursor) != 0);
        n->set_variadic(clang_Cursor_isVariadic(cursor) != 0);

        // Default/deleted (only available for constructors/destructors/methods)
        n->set_defaulted(clang_CXXMethod_isDefaulted(cursor) != 0);
      }

      // Constructor explicit
      if (kind == CXCursor_Constructor) {
        n->set_explicit(clang_CXXConstructor_isConvertingConstructor(cursor) == 0 &&
                        clang_CXXConstructor_isCopyConstructor(cursor) == 0 &&
                        clang_CXXConstructor_isMoveConstructor(cursor) == 0 &&
                        clang_CXXConstructor_isDefaultConstructor(cursor) == 0);
      }

      // Enum constant value
      if (kind == CXCursor_EnumConstantDecl) {
        n->set_enum_value(clang_getEnumConstantDeclValue(cursor));
      }

      // Enum underlying type
      if (kind == CXCursor_EnumDecl) {
        CXType underlying = clang_getEnumDeclIntegerType(cursor);
        if (underlying.kind != CXType_Invalid) {
          n->set_underlying_type(cx_string_to_std(clang_getTypeSpelling(underlying)));
        }
        // Check for scoped enum (enum class)
        n->set_scoped_enum(clang_EnumDecl_isScoped(cursor) != 0);
      }

      // Template info
      CXCursor specialized = clang_getSpecializedCursorTemplate(cursor);
      if (!clang_Cursor_isNull(specialized)) {
        n->set_template_specialization(true);
      }
      if (kind == CXCursor_ClassTemplate || kind == CXCursor_FunctionTemplate) {
        n->set_template(true);
      }

      // Base class info
      if (kind == CXCursor_CXXBaseSpecifier) {
        n->set_virtual_base(clang_isVirtualBase(cursor) != 0);
      }

      // Bitfield info
      if (kind == CXCursor_FieldDecl) {
        if (clang_Cursor_isBitField(cursor)) {
          n->set_bitfield(true);
          n->set_bitfield_width(clang_getFieldDeclBitWidth(cursor));
        }
      }

      // Anonymous struct/union/enum
      if (kind == CXCursor_StructDecl || kind == CXCursor_UnionDecl ||
          kind == CXCursor_EnumDecl || kind == CXCursor_ClassDecl) {
        n->set_anonymous(clang_Cursor_isAnonymous(cursor) != 0);
      }

      // Documentation comment
      CXString raw_comment = clang_Cursor_getRawCommentText(cursor);
      const char* raw_comment_str = clang_getCString(raw_comment);
      if (raw_comment_str && raw_comment_str[0] != '\0') {
        n->set_comment(raw_comment_str);
      }
      clang_disposeString(raw_comment);

      CXString brief_comment = clang_Cursor_getBriefCommentText(cursor);
      const char* brief_comment_str = clang_getCString(brief_comment);
      if (brief_comment_str && brief_comment_str[0] != '\0') {
        n->set_brief_comment(brief_comment_str);
      }
      clang_disposeString(brief_comment);
    }

    // Check if cursor should be processed
    static bool should_process_cursor(CXCursor cursor) {
      CXCursorKind kind = clang_getCursorKind(cursor);

      switch (kind) {
        case CXCursor_TranslationUnit:
        case CXCursor_Namespace:
        case CXCursor_NamespaceAlias:
        case CXCursor_UsingDirective:
        case CXCursor_UsingDeclaration:
        case CXCursor_ClassDecl:
        case CXCursor_StructDecl:
        case CXCursor_UnionDecl:
        case CXCursor_EnumDecl:
        case CXCursor_EnumConstantDecl:
        case CXCursor_TypedefDecl:
        case CXCursor_TypeAliasDecl:
        case CXCursor_FieldDecl:
        case CXCursor_CXXMethod:
        case CXCursor_Constructor:
        case CXCursor_Destructor:
        case CXCursor_ConversionFunction:
        case CXCursor_FunctionDecl:
        case CXCursor_FunctionTemplate:
        case CXCursor_ParmDecl:
        case CXCursor_VarDecl:
        case CXCursor_ClassTemplate:
        case CXCursor_TemplateTypeParameter:
        case CXCursor_NonTypeTemplateParameter:
        case CXCursor_TemplateTemplateParameter:
        case CXCursor_FriendDecl:
        case CXCursor_CXXBaseSpecifier:
        case CXCursor_LinkageSpec:
        case CXCursor_StaticAssert:
          return true;
        default:
          return false;
      }
    }

    // Deep-clone a node and its children
    static node_ptr clone_node(const node_ptr& src) {
      if (!src) return nullptr;

      node_ptr copy = node::create(src->get_kind());
      copy->set_usr(src->get_usr());
      copy->set_name(src->get_name());
      copy->set_qualified_name(src->get_qualified_name());
      copy->set_display_name(src->get_display_name());
      copy->set_mangled_name(src->get_mangled_name());
      copy->set_location(src->get_location());
      copy->set_extent(src->get_extent());
      copy->set_type(src->get_type());
      copy->set_return_type(src->get_return_type());
      copy->set_access(src->get_access());
      copy->set_storage_class(src->get_storage_class());
      copy->set_definition(src->is_definition());
      copy->set_virtual(src->is_virtual());
      copy->set_pure_virtual(src->is_pure_virtual());
      copy->set_override(src->is_override());
      copy->set_final(src->is_final());
      copy->set_static(src->is_static());
      copy->set_const_method(src->is_const_method());
      copy->set_inline(src->is_inline());
      copy->set_explicit(src->is_explicit());
      copy->set_constexpr(src->is_constexpr());
      copy->set_noexcept(src->is_noexcept());
      copy->set_deleted(src->is_deleted());
      copy->set_defaulted(src->is_defaulted());
      copy->set_anonymous(src->is_anonymous());
      copy->set_scoped_enum(src->is_scoped_enum());
      copy->set_template(src->is_template());
      copy->set_template_specialization(src->is_template_specialization());
      copy->set_variadic(src->is_variadic());
      copy->set_bitfield(src->is_bitfield());
      copy->set_bitfield_width(src->get_bitfield_width());
      copy->set_has_default_value(src->has_default_value());
      copy->set_default_value(src->get_default_value());
      copy->set_underlying_type(src->get_underlying_type());
      copy->set_enum_value(src->get_enum_value());
      copy->set_virtual_base(src->is_virtual_base());
      copy->set_comment(src->get_comment());
      copy->set_brief_comment(src->get_brief_comment());

      for (const auto& t : src->get_tags()) {
        copy->add_tag(t);
      }

      for (const auto& child : src->get_children()) {
        node_ptr child_copy = clone_node(child);
        if (child_copy) {
          copy->add_child(child_copy);
        }
      }

      return copy;
    }

    // Visitor context
    struct visitor_context {
      node_ptr current_parent;
      std::unordered_map<std::string, node_ptr> usr_to_node;
    };

    // Visitor callback
    static CXChildVisitResult visit_cursor(CXCursor cursor, CXCursor /* parent */, CXClientData client_data) {
      auto* ctx = static_cast<visitor_context*>(client_data);

      if (!should_process_cursor(cursor)) {
        return CXChildVisit_Recurse;
      }

      // Create a new node for this cursor
      node::kind nk = cursor_kind_to_node_kind(clang_getCursorKind(cursor));
      node_ptr new_node = node::create(nk);
      populate_node_from_cursor(new_node, cursor);

      // Add to parent
      ctx->current_parent->add_child(new_node);

      // Store in USR map for merging
      std::string usr = new_node->get_usr();
      if (!usr.empty()) {
        ctx->usr_to_node[usr] = new_node;
      }

      // Recurse into children with this node as the parent
      node_ptr old_parent = ctx->current_parent;
      ctx->current_parent = new_node;
      clang_visitChildren(cursor, visit_cursor, ctx);
      ctx->current_parent = old_parent;

      return CXChildVisit_Continue;
    }
  };

  // ============================================================================
  // Parser implementation
  // ============================================================================

  std::shared_ptr<node> parser::parse(const std::string& input, const compile_args& args) {
    // Create libclang index
    CXIndex index = clang_createIndex(0, 0);
    if (!index) {
      return node::create(node::kind::translation_unit);
    }

    // Convert compile args to C-style array
    const std::vector<std::string>& args_vec = args.get_args();
    std::vector<const char*> c_args;
    c_args.reserve(args_vec.size());
    for (const auto& arg : args_vec) {
      c_args.push_back(arg.c_str());
    }

    // Create an unsaved file for the input
    CXUnsavedFile unsaved_file;
    unsaved_file.Filename = "input.cpp";
    unsaved_file.Contents = input.c_str();
    unsaved_file.Length = static_cast<unsigned long>(input.size());

    // Parse the translation unit
    CXTranslationUnit tu = nullptr;
    CXErrorCode error = clang_parseTranslationUnit2(
        index,
        "input.cpp",
        c_args.data(),
        static_cast<int>(c_args.size()),
        &unsaved_file,
        1,
        CXTranslationUnit_DetailedPreprocessingRecord |
            CXTranslationUnit_SkipFunctionBodies |
            CXTranslationUnit_KeepGoing,
        &tu);

    if (error != CXError_Success || !tu) {
      clang_disposeIndex(index);
      return node::create(node::kind::translation_unit);
    }

    // Create root node
    node_ptr root = node::create(node::kind::translation_unit);
    root->set_name("input.cpp");

    // Set up visitor context
    parser_impl::visitor_context ctx;
    ctx.current_parent = root;

    // Get the cursor for the translation unit and visit
    CXCursor tu_cursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(tu_cursor, parser_impl::visit_cursor, &ctx);

    // Cleanup
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return root;
  }

  std::shared_ptr<node> parser::merge(std::shared_ptr<node> a, std::shared_ptr<node> b, const compile_args& /* args */) {
    if (!a) return b;
    if (!b) return a;

    // Create a new merged translation unit
    node_ptr merged = node::create(node::kind::translation_unit);
    merged->set_name("merged");

    // Build a map of USR -> node from 'a' for quick lookup
    std::unordered_map<std::string, node_ptr> usr_map;

    // Helper to collect all nodes with USRs from a tree
    std::function<void(const node_ptr&, std::unordered_map<std::string, node_ptr>&)> collect_usrs =
        [&collect_usrs](const node_ptr& n, std::unordered_map<std::string, node_ptr>& map) {
          if (!n) return;
          const std::string& usr = n->get_usr();
          if (!usr.empty()) {
            map[usr] = n;
          }
          for (const auto& child : n->get_children()) {
            collect_usrs(child, map);
          }
        };

    collect_usrs(a, usr_map);

    // Track which USRs from 'b' we've already added
    std::unordered_set<std::string> added_usrs;

    // Add all children from 'a' (cloned)
    for (const auto& child : a->get_children()) {
      node_ptr cloned = parser_impl::clone_node(child);
      merged->add_child(cloned);
      const std::string& usr = child->get_usr();
      if (!usr.empty()) {
        added_usrs.insert(usr);
      }
    }

    // Add children from 'b' that are not already in 'a' (by USR)
    for (const auto& b_child : b->get_children()) {
      if (!b_child) continue;

      const std::string& usr = b_child->get_usr();

      if (usr.empty()) {
        // No USR, just add it
        merged->add_child(parser_impl::clone_node(b_child));
        continue;
      }

      auto it = usr_map.find(usr);
      if (it == usr_map.end()) {
        // Not in 'a', add from 'b'
        merged->add_child(parser_impl::clone_node(b_child));
        added_usrs.insert(usr);
      }
      // If both have the same USR, keep 'a's version (already added)
    }

    return merged;
  }

}  // namespace xccmeta
