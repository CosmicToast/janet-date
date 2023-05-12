#pragma once
#include <janet.h>
#include <time.h>
#include "polyfill.h"

// util.c
JanetBuffer *strftime_buffer(const char *format, const struct tm *tm, JanetBuffer *buffer);
struct tm   *jd_tm_from_dict(JanetDictView dict);
JanetTable  *jd_tm_to_table(struct tm *tm);

// time.c
extern const JanetRegExt jd_time_cfuns[];
time_t *jd_gettime(Janet *argv, int32_t n);
time_t *jd_maketime(void);
JANET_CFUN(jd_dict_time);
JANET_CFUN(jd_gmtime);
JANET_CFUN(jd_localtime);
JANET_CFUN(jd_time);

// tm.c
extern const JanetRegExt jd_tm_cfuns[];
struct tm *jd_gettm(Janet *argv, int32_t n);
struct tm *jd_maketm(void);
JANET_CFUN(jd_dict_tm);
JANET_CFUN(jd_mktime);
JANET_CFUN(jd_mktime_inplace);
JANET_CFUN(jd_strftime);
JANET_CFUN(jd_tm_dict);
