#pragma once
#include <janet.h>

#if JANET_VERSION_MAJOR < 2 && JANET_VERSION_MINOR < 28
#define POLYFILL_CBYTES
const char* janet_getcbytes(const Janet *argv, int32_t n);
const char *janet_optcbytes(const Janet *argv, int32_t argc, int32_t n, const char *dflt);

#if !defined(JANET_NO_SOURCEMAPS) && !defined(JANET_NO_DOCSTRINGS)
#undef JANET_FN
#define JANET_FN(CNAME, USAGE, DOCSTRING) \
    static const int32_t CNAME##_sourceline_ = __LINE__; \
    static const char CNAME##_docstring_[] = USAGE "\n\n" DOCSTRING; \
    Janet CNAME (int32_t argc, Janet *argv)
#elif !defined(JANET_NO_SOURCEMAPS) && defined(JANET_NO_DOCSTRINGS)
#undef JANET_FN
#define JANET_FN(CNAME, USAGE, DOCSTRING) \
    static const int32_t CNAME##_sourceline_ = __LINE__; \
    Janet CNAME (int32_t argc, Janet *argv)
#endif // !defined(JANET_NO_SOURCEMAPS)

#endif // JANET_VERSION_MAJOR < 2 && JANET_VERSION_MINOR < 28
