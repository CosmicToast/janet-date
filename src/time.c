#include "date.h"
#include "janet.h"

// wrappers around time_t

static JanetMethod jd_time_methods[] = {
	{"gmtime",    jd_gmtime},
	{"localtime", jd_localtime},
	{"todict",    jd_dict_time},
	{NULL, NULL},
};

static int jd_time_compare(void *lhs, void *rhs) {
	time_t lhv = (*(time_t*)lhs);
	time_t rhv = (*(time_t*)rhs);
	return difftime(lhv, rhv);
}

static int jd_time_get(void *p, Janet key, Janet *out) {
	(void) p;
	if (!janet_checktype(key, JANET_KEYWORD)) {
		return 0;
	}
	return janet_getmethod(janet_unwrap_keyword(key), jd_time_methods, out);
}

// time_t is always a UTC-representation
static void jd_time_tostring(void *p, JanetBuffer *buffer) {
	strftime_buffer("%F %T.000 UTC", localtime(p), buffer);
}

static const JanetAbstractType jd_time_t = {
	"time",
	NULL,
	NULL,
	jd_time_get,
	NULL,
	NULL,
	NULL,
	jd_time_tostring,
	jd_time_compare,
	JANET_ATEND_COMPARE
};

time_t *jd_gettime(Janet *argv, int32_t n) {
	return (time_t*)janet_getabstract(argv, n, &jd_time_t);
}

time_t *jd_maketime(void) {
	return janet_abstract(&jd_time_t, sizeof(time_t));
}

JANET_FN(jd_dict_time,
		"(dict->time {...})",
		"") {
	janet_fixarity(argc, 1);
	JanetDictView dict = janet_getdictionary(argv, 0);
	struct tm *tm = jd_tm_from_dict(dict);
	return janet_wrap_abstract(tm);
}

JANET_FN(jd_gmtime,
		"(gmtime (time))",
		"") {
	janet_fixarity(argc, 1);
	time_t *time   = jd_gettime(argv, 0);
	struct tm *tm  = gmtime(time);
	struct tm *out = jd_maketm();
	*out = *tm;
	return janet_wrap_abstract(out);
}

JANET_FN(jd_localtime,
		"(localtime (time))",
		"WARNING: do not use this unless it's for final display.") {
	janet_fixarity(argc, 1);
	time_t *time   = jd_gettime(argv, 0);
	struct tm *tm  = localtime(time);
	struct tm *out = jd_maketm();
	*out = *tm;
	return janet_wrap_abstract(out);
}

JANET_FN(jd_time,
		"(time)",
		"") {
	(void) argv;
	janet_fixarity(argc, 0);
	time_t *out = jd_maketime();
	time(out);
	return janet_wrap_abstract(out);
}

const JanetRegExt jd_time_cfuns[] = {
	JANET_REG("dict->time", jd_dict_time),
	JANET_REG("gmtime",     jd_gmtime),
	JANET_REG("localtime",  jd_localtime),
	JANET_REG("time",       jd_time),
	JANET_REG_END
};
