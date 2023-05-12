# Time is a mess
ISO C99 is fairly restrictive, while time in the wild is *wild*.

> The mktime function converts the broken-down time, expressed as local time, in the
> structure pointed to by timeptr into a calendar time value with the same encoding as
> that of the values returned by the time function. The original values of the tm_wday
> and tm_yday components of the structure are ignored, and the original values of the
> other components are not restricted to the ranges indicated above. On successful
> completion, the values of the tm_wday and tm_yday components of the structure are
> set appropriately, and the other components are set to represent the specified calendar
> time, but with their values forced to the ranges indicated above; the final value of
> tm_mday is not set until tm_mon and tm_year are determined.

This means that `mktime` must operate on `localtime` output.
The implication (which is accurate) implies that `gmtime(t) == gmtime(mktime(localtime(t)))`.

As such, the source of truth is `time_t`, which is UTC-only.
When we want to modify a `time_t`, we want to use localtime to perform the modification.
