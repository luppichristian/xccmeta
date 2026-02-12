---
layout: default
title: Base
---

# Base

## Overview

Provides the `XCCMETA_API` macro for shared library symbol visibility and common includes used across all modules.

## Why Use This?

You don't include this directly. It's the foundation header that all other modules depend on. It centralizes platform-specific DLL export/import logic and ensures consistent STL types across the library.

## Types

| Type | Description |
|------|-------------|
| `XCCMETA_API` | Macro for DLL export/import (empty for static builds) |

## Macros

### `XCCMETA_API`

Platform-specific symbol visibility macro.

**Behavior:**
- Static build (default): expands to nothing
- Shared build, building library (`XCCMETA_EXPORTS` defined): `__declspec(dllexport)` on Windows, `__attribute__((visibility("default")))` on GCC/Clang
- Shared build, consuming library: `__declspec(dllimport)` on Windows

**Notes:** Do not include this header directly. Use `#include <xccmeta.hpp>` or include specific module headers.

## Common Includes

This header pulls in the following STL headers for all modules:
- `<cstdint>`
- `<memory>`
- `<optional>`
- `<string>`
- `<vector>`
