#include "date.h"
#include "janet.h"

// wrappers around struct tm

static JanetMethod jd_tm_methods[] = {
	{"mktime",    jd_mktime},
	{"mktime!",   jd_mktime_inplace},
	{"normalize", jd_mktime_inplace},
	{"strftime",  jd_strftime},
	{"todict",    jd_tm_dict},
	{NULL, NULL},
};

static int jd_tm_compare(void *lhs, void *rhs) {
	struct tm lhp = (*(struct tm*)lhs);
	struct tm rhp = (*(struct tm*)rhs);
	time_t lhv = mktime(&lhp);
	time_t rhv = mktime(&rhp);
	return difftime(lhv, rhv);
}

static int jd_tm_get(void *p, Janet key, Janet *out) {
	if (!janet_checktype(key, JANET_KEYWORD)) {
		return 0;
	}

	// is it a method?
	if(janet_getmethod(janet_unwrap_keyword(key), jd_tm_methods, out)) {
		return 1;
	}

	// piggyback off jd_tm_to_table
	JanetTable *tb = jd_tm_to_table(p);
	*out = janet_table_rawget(tb, key);

	return janet_checktype(*out, JANET_NIL);
}

static const char* jd_tm_keys[] = {
	"sec", "min", "hour", "mday", "mon", "year", "wday", "yday", NULL,
};
static Janet jd_tm_next(void *p, Janet key) {
	(void) p;
	const char **ptr = jd_tm_keys;
	while (*ptr) {
		if (janet_keyeq(key, *ptr)) {
			return *(++ptr) ? janet_ckeywordv(*ptr) : janet_wrap_nil();
		}
		ptr++;
	}
	return janet_ckeywordv(jd_tm_keys[0]);
}

// struct tm can represent non-UTC
// it does not keep TZ information so we can't display it without potentially lying
static void jd_tm_tostring(void *p, JanetBuffer *buffer) {
	strftime_buffer("%F %T.000", p, buffer);
}

static const JanetAbstractType jd_tm_t = {
	"tm",
	NULL,
	NULL,
	jd_tm_get,
	NULL,
	NULL,
	NULL,
	jd_tm_tostring,
	jd_tm_compare,
	NULL,
	jd_tm_next,
	JANET_ATEND_NEXT
};

struct tm *jd_gettm(Janet *argv, int32_t n) {
	return (struct tm*)janet_getabstract(argv, n, &jd_tm_t);
}

struct tm *jd_maketm(void) {
	return janet_abstract(&jd_tm_t, sizeof(struct tm));
}

JANET_FN(jd_dict_tm,
		"",
		"") {
	janet_fixarity(argc, 1);
	JanetDictView dict = janet_getdictionary(argv, 0);
	return janet_wrap_abstract(jd_tm_from_dict(dict));
}

JANET_FN(jd_mktime,
		"",
		"") {
	janet_fixarity(argc, 1);
	struct tm *tm = jd_gettm(argv, 0);
	struct tm *nw = jd_maketm();
	*nw = *tm;
	time_t *time = jd_maketime();
	*time = mktime(nw);
	return janet_wrap_abstract(time);
}

JANET_FN(jd_mktime_inplace,
		"",
		"") {
	janet_fixarity(argc, 1);
	struct tm *tm = jd_gettm(argv, 0);
	time_t *time = jd_maketime();
	*time = mktime(tm);
	return janet_wrap_abstract(time);
}

JANET_FN(jd_tm_dict,
		"",
		"") {
	janet_fixarity(argc, 1);
	struct tm *tm = jd_gettm(argv, 0);
	return janet_wrap_table(jd_tm_to_table(tm));
}

// strftime
struct strftime_format {
	const char *keyword;
	const char *format;
};
const static struct strftime_format strftime_formats[] = {
	{NULL, NULL},
};
JANET_FN(jd_strftime,
		"",
		"") {
	janet_fixarity(argc, 2);
	// tm is first for pseudo-OO
	struct tm *tm = jd_gettm(argv, 0);

	// determine format
	const char *format = NULL;

	// is it a preset?
	if (janet_checktype(argv[1], JANET_KEYWORD)) {
		const struct strftime_format *ptr = strftime_formats;
		while (ptr->keyword) {
			if (janet_keyeq(argv[1], ptr->keyword)) {
				format = ptr->format;
				break;
			}
			ptr++;
		}
	}

	// preset not found
	if (!format) format = janet_getcbytes(argv, 1);
	return janet_wrap_buffer(strftime_buffer(format, tm, NULL));
}

const JanetRegExt jd_tm_cfuns[] = {
	JANET_REG("dict->tm", jd_dict_tm),
	JANET_REG("mktime",   jd_mktime),
	JANET_REG("mktime!",  jd_mktime_inplace),
	JANET_REG("strftime", jd_strftime),
	JANET_REG("tm->dict", jd_tm_dict),
	JANET_REG_END
};
