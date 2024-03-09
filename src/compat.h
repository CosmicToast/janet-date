#pragma once

// GNU-ish systems require this to expose tm_gmtoff and tm_zone
// this is important because of how we detect their existence
#define _GNU_SOURCE 1

#include <time.h>

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
