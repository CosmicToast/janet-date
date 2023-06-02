#pragma once
#include "compat.h"
#include "polyfill.h"

// technically included above, here for posterity
#include <janet.h>
#include <time.h>

// util.c
JanetBuffer *strftime_buffer(const char *format, const struct tm *tm, JanetBuffer *buffer);

// time.c
void jd_time_register(JanetTable*, const char*);
time_t *jd_gettime(Janet *argv, int32_t n);
time_t *jd_maketime(void);
JANET_CFUN(jd_gmtime);
JANET_CFUN(jd_localtime);
JANET_CFUN(jd_time);

// tm.c
void jd_tm_register(JanetTable*, const char*);
struct tm *jd_gettm(Janet *argv, int32_t n);
struct tm *jd_opttm(Janet *argv, int32_t argc, int32_t n);
struct tm *jd_maketm(void);
JANET_CFUN(jd_mktime);
JANET_CFUN(jd_mktime_inplace);
JANET_CFUN(jd_timegm);
JANET_CFUN(jd_timegm_inplace);
JANET_CFUN(jd_strftime);
