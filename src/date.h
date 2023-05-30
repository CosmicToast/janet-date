#pragma once
#include <janet.h>
#include <time.h>
#include "polyfill.h"

// util.c
JanetBuffer *strftime_buffer(const char *format, const struct tm *tm, JanetBuffer *buffer);

// time.c
extern const JanetRegExt jd_time_cfuns[];
time_t *jd_gettime(Janet *argv, int32_t n);
time_t *jd_maketime(void);
JANET_CFUN(jd_gmtime);
JANET_CFUN(jd_localtime);
JANET_CFUN(jd_time);

// tm.c
extern const JanetRegExt jd_tm_cfuns[];
struct tm *jd_gettm(Janet *argv, int32_t n);
struct tm *jd_opttm(Janet *argv, int32_t argc, int32_t n);
struct tm *jd_maketm(void);
JANET_CFUN(jd_mktime);
JANET_CFUN(jd_mktime_inplace);
JANET_CFUN(jd_strftime);
