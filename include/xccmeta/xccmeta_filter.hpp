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

#pragma once

#include "xccmeta_node.hpp"

namespace xccmeta {

  // This is a list of unique nodes (type nodes), with no duplicates allowed.
  class XCCMETA_API filter {
   public:
    struct config {
      enum node_inclusion {
        exclude,             // Only include the specified node
        include,             // Include direct children of the specified node
        include_recursively  // Include all descendant nodes
      };

      // Allowed kinds of types to include
      std::vector<node::kind> allowed_kinds;

      // If a node has a tag name in this list, it will be included
      // If empty, no inclusion filtering is done
      std::vector<std::string> grab_tag_names;

      // If a node has a tag name in this list, it will be excluded
      std::vector<std::string> avoid_tag_names;

      // Whether to include nodes into the list even if they dont have tags
      // These still dont't bypass ALLOWED_KINDS filtering but they do bypass tag name filtering
      node_inclusion child_node_inclusion = node_inclusion::exclude;
      node_inclusion parent_node_inclusion = node_inclusion::exclude;
    };

    filter(const config& cfg = {});

    // Go over all types and remove any that don't meet the criteria
    filter& clean();

    // Check if a type is already in the list (by USR)
    bool contains(const node_ptr& type) const;

    // Add a type to the list (returns true if added, false if already exists or invalid)
    bool add(const node_ptr& type);

    // Remove a type from the list (returns true if removed, false if not found)
    bool remove(const node_ptr& type);

    // Clear all types from the list
    filter& clear();

    // Get the number of types in the list
    std::size_t size() const;

    // Check if the list is empty
    bool empty() const;

    // Get all types in the list
    const std::vector<node_ptr>& get_types() const;

    // Get the current configuration
    const config& get_config() const;

    // Check if a node is a valid type node for this list's configuration
    bool is_valid_type(const node_ptr& type) const;

    // Check if type matches the config criteria
    bool matches_config(const node_ptr& type) const;

    // Iterator support
    auto begin() { return types_.begin(); }
    auto end() { return types_.end(); }
    auto begin() const { return types_.begin(); }
    auto end() const { return types_.end(); }
    auto cbegin() const { return types_.cbegin(); }
    auto cend() const { return types_.cend(); }

   private:
    // Find a type by USR, returns iterator
    std::vector<node_ptr>::iterator find_by_usr(const std::string& usr);
    std::vector<node_ptr>::const_iterator find_by_usr(const std::string& usr) const;

    std::vector<node_ptr> types_;
    config config_;
  };

}  // namespace xccmeta