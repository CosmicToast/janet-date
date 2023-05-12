#include "date.h"

JANET_MODULE_ENTRY(JanetTable *env) {
	janet_cfuns_ext(env, "date/native", jd_time_cfuns);
	janet_cfuns_ext(env, "date/native", jd_tm_cfuns);
}
