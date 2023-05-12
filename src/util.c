#include "date.h"

#define JD_STRFTIME_CHUNK 64
JanetBuffer *strftime_buffer(const char *format, const struct tm *tm, JanetBuffer *buffer) {
	if (!buffer) buffer = janet_buffer(0);
	size_t offset  = buffer->count;
	size_t written = 0;
	do {
		janet_buffer_extra(buffer, JD_STRFTIME_CHUNK);
		written = strftime((char*)buffer->data + offset, buffer->capacity - offset, format, tm);
	} while (!written);
	buffer->count = written + offset; // does not include \0, but we don't want it anyway
	return buffer;
}

static inline void tm_set_dict(JanetDictView dict, char *key, int *v) {
	Janet k = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv(key));
	*v = janet_checktype(k, JANET_NUMBER) ? janet_unwrap_integer(k) : 0;
}
struct tm *jd_tm_from_dict(JanetDictView dict) {
	struct tm *tm = jd_maketm();

	tm_set_dict(dict, "sec",  &tm->tm_sec);
	tm_set_dict(dict, "min",  &tm->tm_min);
	tm_set_dict(dict, "hour", &tm->tm_hour);
	tm_set_dict(dict, "mday", &tm->tm_mday);
	tm_set_dict(dict, "mon",  &tm->tm_mon);
	Janet year = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("year"));
	tm->tm_year = janet_checktype(year, JANET_NUMBER) ? janet_unwrap_integer(year) + 1900 : 1900;
	tm_set_dict(dict, "wday", &tm->tm_wday);
	tm_set_dict(dict, "yday", &tm->tm_yday);
	Janet isdst = janet_dictionary_get(dict.kvs, dict.cap, janet_ckeywordv("isdst"));
	tm->tm_isdst = janet_keyeq(isdst, "detect") ? -1 : (janet_truthy(isdst) ? 1 : 0);

	return tm;
}

#define PUTV(T, K, V) janet_table_put(T, janet_ckeywordv(K), V)
#define PUT(T, K, V) PUTV(T, K, janet_wrap_integer(V))
JanetTable *jd_tm_to_table(struct tm *tm) {
	JanetTable *out = janet_table(9);

	PUT(out, "sec",  tm->tm_sec);
	PUT(out, "min",  tm->tm_min);
	PUT(out, "hour", tm->tm_hour);
	PUT(out, "mday", tm->tm_mday);
	PUT(out, "mon",  tm->tm_mon);
	PUT(out, "year", tm->tm_year + 1900);
	PUT(out, "wday", tm->tm_wday);
	PUT(out, "yday", tm->tm_yday);
	if (tm->tm_isdst < 0) {
		PUTV(out, "isdst", janet_ckeywordv("detect"));
	} else {
		PUTV(out, "isdst", tm->tm_isdst ? janet_wrap_true() : janet_wrap_false());
	}

	return out;
}
