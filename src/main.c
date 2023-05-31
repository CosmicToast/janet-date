#include "date.h"

JANET_MODULE_ENTRY(JanetTable *env) {
	jd_time_register(env, "date/native");
	jd_tm_register(env, "date/native");
}
