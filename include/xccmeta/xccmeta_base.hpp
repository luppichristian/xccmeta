#pragma once

// #############################################################################
// XCCMETA_API macro for shared library symbol visibility
// #############################################################################

#if defined(XCCMETA_SHARED)
#  if defined(_WIN32) || defined(_WIN64)
#    if defined(XCCMETA_EXPORTS)
#      define XCCMETA_API __declspec(dllexport)
#    else
#      define XCCMETA_API __declspec(dllimport)
#    endif
#  elif defined(__GNUC__) && __GNUC__ >= 4
#    define XCCMETA_API __attribute__((visibility("default")))
#  else
#    define XCCMETA_API
#  endif
#else
#  define XCCMETA_API
#endif

// #############################################################################
// Libraries
// #############################################################################

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
