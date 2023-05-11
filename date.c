#include <janet.h>
#include <time.h>

// polyfills
#if JANET_VERSION_MAJOR < 2 && JANET_VERSION_MINOR < 28
const char* janet_getcbytes(const Janet *argv, int32_t n) {
    JanetByteView view = janet_getbytes(argv, n);
    const char *cstr = (const char *)view.bytes;
    if (strlen(cstr) != (size_t) view.len) {
        janet_panic("bytes contain embedded 0s");
    }
    return cstr;
}

const char *janet_optcbytes(const Janet *argv, int32_t argc, int32_t n, const char *dflt) {
    if (n >= argc || janet_checktype(argv[n], JANET_NIL)) {
        return dflt;
    }
    return janet_getcbytes(argv, n);
}
#endif

#if !defined(JANET_NO_SOURCEMAPS) && !defined(JANET_NO_DOCSTRINGS)
#undef JANET_FN
#define JANET_FN(CNAME, USAGE, DOCSTRING) \
    static const int32_t CNAME##_sourceline_ = __LINE__; \
    static const char CNAME##_docstring_[] = USAGE "\n\n" DOCSTRING; \
    Janet CNAME (int32_t argc, Janet *argv)
#endif

// forward declarations
JANET_CFUN(jgmtime);
JANET_CFUN(jlocaltime);
JANET_CFUN(jmktime);
JANET_CFUN(jmktime_inplace);
JANET_CFUN(jstrftime);
JANET_CFUN(jtmdict);

// native date/time module that's strictly based on the C specification
// must build with c99, may opt-into c11+ features
// we make native time_t and struct tm into abstract types to get free gc and methods

static JanetMethod jtime_methods[] = {
	{"gmtime", jgmtime},
	{"localtime", jlocaltime},
	{NULL, NULL},
};

static JanetMethod jtm_methods[] = {
	{"mktime", jmktime},
	{"normalize", jmktime_inplace},
	{"strftime", jstrftime},
	{"todict", jtmdict},
	{NULL, NULL},
};

static int jtime_get(void *p, Janet key, Janet *out) {
	(void) p;
	if (!janet_checktype(key, JANET_KEYWORD)) {
		return 0;
	}
	return janet_getmethod(janet_unwrap_keyword(key), jtime_methods, out);
}

#define JDATE_KEYEQ(name) if(janet_keyeq(key, #name)) { *out = janet_wrap_integer(tm->tm_##name); return 1; }
static int jtm_get(void *p, Janet key, Janet *out) {
	struct tm *tm = (struct tm*)p;
	if (!janet_checktype(key, JANET_KEYWORD)) {
		return 0;
	}

	// is it a method?
	if(janet_getmethod(janet_unwrap_keyword(key), jtm_methods, out)) {
		return 1;
	}

	// is it a tm member?
	JDATE_KEYEQ(sec);
	JDATE_KEYEQ(min);
	JDATE_KEYEQ(hour);
	JDATE_KEYEQ(mday);
	JDATE_KEYEQ(mon);
	// year is defined as years since 1900
	if (janet_keyeq(key, "year")) {
		*out = janet_wrap_integer(tm->tm_year + 1900);
		return 1;
	}
	JDATE_KEYEQ(wday);
	JDATE_KEYEQ(yday);
	if (janet_keyeq(key, "isdst")) {
		if(tm->tm_isdst == 0) {
			*out = janet_wrap_false();
		} else if (tm->tm_isdst > 1) {
			*out = janet_wrap_true();
		} else {
			*out = janet_ckeywordv("detect");
		}
		return 1;
	}
	return 0;
}
#undef JDATE_KEYEQ

static /*inline*/ int compare_time_t(time_t lhs, time_t rhs) {
	// time_t is either arithmetic (we could lhs - rhs)
	// or real (we can't do that since we return an int)
	return lhs < rhs ? -1 : (lhs > rhs ? 1 : 0);
}

static int jtime_compare(void *lhs, void *rhs) {
	time_t lhv = (*(time_t*)lhs);
	time_t rhv = (*(time_t*)rhs);
	return compare_time_t(lhv, rhv);
}

static int jtm_compare(void *lhs, void *rhs) {
	struct tm lhc = *((struct tm*)lhs);
	struct tm rhc = *((struct tm*)rhs);
	time_t lhv = mktime(&lhc);
	time_t rhv = mktime(&rhc);
	return compare_time_t(lhv, rhv);
}

#define JSTRFTIME_CHUNK 64
// we expect buffer to be empty on init
static JanetBuffer* strftime_buffer(const char *format, const struct tm *tm, JanetBuffer *buffer) {
	if(!buffer) buffer = janet_buffer(0);
	size_t offset = buffer->count;
	size_t written = 0;
	do {
		janet_buffer_extra(buffer, JSTRFTIME_CHUNK);
		written = strftime((char*)buffer->data + offset, buffer->capacity - offset,
				format, tm);
	} while (!written);
	buffer->count = written + offset; // does not include \0, which we don't want anyway
	return buffer;
}
#undef JSTRFTIME_CHUNK

static void jtm_tostring(void *p, JanetBuffer *buffer) {
	// ISO 8601
	strftime_buffer("%F %T.000", (struct tm*)p, buffer);
}

static void jtime_tostring(void *p, JanetBuffer *buffer) {
	// ctime is deprecated but lets us know the intended approach is localtime()
	jtm_tostring(localtime(p), buffer);
}

static const char* keys[] = {
	"sec", "min", "hour", "mday", "mon", "year", "wday", "yday", NULL
};
static Janet jtm_next(void *p, Janet key) {
	(void) p;
	const char **ptr = keys;
	while(*ptr) {
		if (janet_keyeq(key, *ptr)) {
			return *(++ptr) ? janet_ckeywordv(*ptr) : janet_wrap_nil();
		}
		ptr++;
	}
	return janet_ckeywordv(keys[0]);
}

// TODO: hash
static const JanetAbstractType jtime_type = {
	"time",
	NULL,
	NULL,
	jtime_get,
	NULL,
	NULL,
	NULL,
	jtime_tostring,
	jtime_compare,
	JANET_ATEND_COMPARE
};

static const JanetAbstractType jtm_type = {
	"tm",
	NULL,
	NULL,
	jtm_get,
	NULL,
	NULL,
	NULL,
	jtm_tostring,
	jtm_compare,
	NULL,
	jtm_next,
	JANET_ATEND_NEXT
};

static time_t *janet_getjtime(Janet *argv, int32_t n) {
	return (time_t*)janet_getabstract(argv, n, &jtime_type);
}

static struct tm *janet_getjtm(Janet *argv, int32_t n) {
	return (struct tm*)janet_getabstract(argv, n, &jtm_type);
}

JANET_FN(jgmtime,
		"",
		"") {
	janet_fixarity(argc, 1);
	time_t *time = janet_getjtime(argv, 0);
	struct tm *tm = janet_abstract(&jtm_type, sizeof(struct tm));
	struct tm *in = gmtime(time);
	*tm = *in;
	return janet_wrap_abstract(tm);
}

JANET_FN(jlocaltime,
		"",
		"") {
	janet_fixarity(argc, 1);
	time_t *time = janet_getjtime(argv, 0);
	struct tm *tm = janet_abstract(&jtm_type, sizeof(struct tm));
	struct tm *in = localtime(time);
	*tm = *in;
	return janet_wrap_abstract(tm);
}

// does not mutate input
JANET_FN(jmktime,
		"",
		"") {
	janet_fixarity(argc, 1);
	struct tm *tm = janet_getjtm(argv, 0);
	struct tm new = *tm;
	time_t *time = janet_abstract(&jtime_type, sizeof(time_t));
	*time = mktime(&new);
	return janet_wrap_abstract(time);
}

// mutates input
JANET_FN(jmktime_inplace,
		"",
		"") {
	janet_fixarity(argc, 1);
	struct tm *tm = janet_getjtm(argv, 0);
	time_t *time = janet_abstract(&jtime_type, sizeof(time_t));
	*time = mktime(tm);
	return janet_wrap_abstract(time);
}

JANET_FN(jtime,
		"",
		"") {
	janet_fixarity(argc, 0);
	time_t *jtime = (time_t*)janet_abstract(&jtime_type, sizeof(time_t));
	time(jtime);
	return janet_wrap_abstract(jtime);
}

struct strftime_format {
	const char* keyword;
	const char* format;
};
const static struct strftime_format builtin_formats[] = {
	{"iso8601", "%F %T.000"},
	{"locale",  "%c"},
	// WARN: will not work as you expect if it came from gmtime
	{"rfc5322", "%a, %d %b %Y %T %z"},
	// variant of rfc5322 that is compatible with gmtime and only gmtime
	{"email",   "%d %b %Y %T -0000"},
	{NULL, NULL}
};

JANET_FN(jstrftime,
		"",
		"") {
	janet_fixarity(argc, 2);
	// we reverse the order of the function for tm to be the first arg for the method
	struct tm *tm = janet_getjtm(argv, 0);

	// determine format
	const char *format = NULL;
	// is it a preset?
	if (janet_checktype(argv[1], JANET_KEYWORD)) {
		const struct strftime_format *ptr = builtin_formats;
		while(ptr->keyword) {
			if (janet_keyeq(argv[1], ptr->keyword)) {
				format = ptr->format;
				break;
			}
			ptr++;
		}
	}
	// either not a preset or not found
	if (!format) format = janet_getcbytes(argv, 1);

	return janet_wrap_buffer(strftime_buffer(format, tm, NULL));
}

// common between dict->tm and dict->time
static Janet tm_from_vals(JanetDictView dict) {
	struct tm *tm = janet_abstract(&jtm_type, sizeof(struct tm));
	Janet jsec   = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("sec"));
	Janet jmin   = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("min"));
	Janet jhour  = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("hour"));
	Janet jmday  = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("mday"));
	Janet jmon   = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("mon"));
	Janet jyear  = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("year"));
	Janet jwday  = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("wday"));
	Janet jyday  = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("yday"));
	Janet jisdst = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("isdst"));

	tm->tm_sec  = janet_checktype(jsec,  JANET_NUMBER) ? janet_unwrap_integer(jsec)  : 0;
	tm->tm_min  = janet_checktype(jmin,  JANET_NUMBER) ? janet_unwrap_integer(jmin)  : 0;
	tm->tm_hour = janet_checktype(jhour, JANET_NUMBER) ? janet_unwrap_integer(jhour) : 0;
	tm->tm_mday = janet_checktype(jmday, JANET_NUMBER) ? janet_unwrap_integer(jmday) : 0;
	tm->tm_mon  = janet_checktype(jmon,  JANET_NUMBER) ? janet_unwrap_integer(jmon)  : 0;
	// year is defined as since 1900, so we normalize
	tm->tm_year = janet_checktype(jyear, JANET_NUMBER) ? janet_unwrap_integer(jyear) - 1900 : 0;
	tm->tm_wday = janet_checktype(jwday, JANET_NUMBER) ? janet_unwrap_integer(jwday) : 0;
	tm->tm_yday = janet_checktype(jyday, JANET_NUMBER) ? janet_unwrap_integer(jyday) : 0;
	tm->tm_isdst = janet_keyeq(jisdst, "detect") ? -1 : (janet_truthy(jisdst) ? 1 : 0);
	return janet_wrap_abstract(tm);
}

// convenience to make a time_t from a dictionary
JANET_FN(jnewtime,
		"",
		"") {
	janet_fixarity(argc, 1);
	JanetDictView dict = janet_getdictionary(argv, 0);
	Janet args[1] = { tm_from_vals(dict) };
	return jmktime(1, args);
}

// convenience to make a struct tm from a dictionary
JANET_FN(jnewtm,
		"",
		"") {
	janet_fixarity(argc, 1);
	JanetDictView dict = janet_getdictionary(argv, 0);
	return tm_from_vals(dict);
}

// convenience to make a dict out of a tm
JANET_FN(jtmdict,
		"",
		"") {
	janet_fixarity(argc, 1);
	struct tm *tm = janet_getjtm(argv, 0);
	JanetTable *out = janet_table(9);

	janet_table_put(out, janet_ckeywordv("sec"),  janet_wrap_integer(tm->tm_sec));
	janet_table_put(out, janet_ckeywordv("min"),  janet_wrap_integer(tm->tm_min));
	janet_table_put(out, janet_ckeywordv("hour"), janet_wrap_integer(tm->tm_hour));
	janet_table_put(out, janet_ckeywordv("mday"), janet_wrap_integer(tm->tm_mday));
	janet_table_put(out, janet_ckeywordv("mon"),  janet_wrap_integer(tm->tm_mon));
	// year is defined as since 1900, so we normalize
	janet_table_put(out, janet_ckeywordv("year"), janet_wrap_integer(tm->tm_year + 1900));
	janet_table_put(out, janet_ckeywordv("wday"), janet_wrap_integer(tm->tm_wday));
	janet_table_put(out, janet_ckeywordv("yday"), janet_wrap_integer(tm->tm_yday));
	if(tm->tm_isdst == 0) {
		janet_table_put(out, janet_ckeywordv("isdst"), janet_wrap_false());
	} else if (tm->tm_isdst > 1) {
		janet_table_put(out, janet_ckeywordv("isdst"), janet_wrap_true());
	} else {
		janet_table_put(out, janet_ckeywordv("isdst"), janet_ckeywordv("detect"));
	}
	return janet_wrap_table(out);
}

static const JanetRegExt cfuns[] = {
	JANET_REG("gmtime",     jgmtime),
	JANET_REG("localtime",  jlocaltime),
	JANET_REG("mktime",     jmktime),
	JANET_REG("mktime!",    jmktime_inplace),
	JANET_REG("time",       jtime),
	JANET_REG("strftime",   jstrftime),
	JANET_REG("tm->dict",   jtmdict),
	JANET_REG("dict->tm",   jnewtm),
	JANET_REG("dict->time", jnewtime),
	JANET_REG_END,
};

JANET_MODULE_ENTRY(JanetTable *env) {
	janet_cfuns_ext(env, "date/native", cfuns);
}
