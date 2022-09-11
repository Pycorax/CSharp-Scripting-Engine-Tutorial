#pragma once

// Select the correct export system based on the compiler
#   if defined _WIN32 || defined __CYGWIN__ || defined _MSC_VER
#       define API_EXPORT __declspec(dllexport)
#       define API_IMPORT __declspec(dllimport)
#   elif defined __GNUC__ && __GNUC__ >= 4
#       define API_EXPORT __attribute__((visibility("default")))
#       define API_IMPORT __attribute__((visibility("default")))
#   else /* Unsupported compiler */
#       define API_EXPORT
#       define API_IMPORT
#   endif
    // Define the correct
#   ifndef DLL_API
#      if defined DLL_API_EXPORT
#          define DLL_API API_EXPORT
#      else
#          define DLL_API API_IMPORT
#      endif
#   endif