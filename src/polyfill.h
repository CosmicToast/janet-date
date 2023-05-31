#pragma once
#include <janet.h>
#include <time.h>

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

// timegm
#ifdef _MSC_VER
#define timegm _mkgmtime
#else
// since tm is a pointer, it should be ABI-compatible even if the true type is mismatched
time_t timegm(struct tm *tm);
#endif

// ===

// public domain code from
// https://github.com/eggert/tz.git

/* Infer TM_ZONE on systems where this information is known, but suppress
   guessing if NO_TM_ZONE is defined.  Similarly for TM_GMTOFF.  */
#if (defined __GLIBC__ \
     || defined __tm_zone /* musl */ \
     || defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__ \
     || (defined __APPLE__ && defined __MACH__))
# if !defined TM_GMTOFF && !defined NO_TM_GMTOFF
#  define TM_GMTOFF tm_gmtoff
# endif
# if !defined TM_ZONE && !defined NO_TM_ZONE
#  define TM_ZONE tm_zone
# endif
#endif
