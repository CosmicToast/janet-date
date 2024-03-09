# Date
Complete ISO C99 date and time support for Janet.

It contains the following components:
* `date/native`: a low level C interface to ISO C99 facilities (plus `timegm`).
* `date`: a safer janet wrapper around `date/native`.
Feel free to use either of these.

## Installation
`jpm install https://github.com/cosmictoast/janet-date.git`

You can add it to your `project.janet` dependency list like so:
```janet
(declare-project
  :name "my-project"
  :dependencies
  [{:repo "https://github.com/cosmictoast/janet-date.git"
    :tag "v1.0.2"}]
  # ...
```

## Caveats
On most platforms (excluding windows), the native time.h impementation is
replaced by the [IANA's tzcode](https://data.iana.org/time-zones/tz-link.html).
It is shipped with this repository as a submodule.
However, if your code has different behavior than native libc code,
this is likely why.

This is often desirable, since some platforms (such as macos) have incorrect
datetime handling in the libc.

In addition, this library ships the non-standard `timegm` function.
It is provided natively in windows and in IANA's tzcode, but not on many other
platforms.

## Examples
Note that these examples exclusively show off the janet bindings,
for native API, please see the test suite and `src/` directory.

```janet
# there are two types of objects, date/time and date/tm
# date/time represents a specific moment in time, free of context
# you can create one that represents the current moment using date/time
(def now (date/time))

# the other object type is date/tm
# date/tm represents a specific calendar time
# this includes timezone information, year, day of year, and so on
# you can create a date/tm object yourself, or from a date/time object
(def calendar-now-utc    (:gmtime now))
(def calendar-now-local  (:localtime now))
# other fields are initialized to their default values, usually zero
(def calendar-unix-epoch (date/utc {:year 1970}))
# you can also use date/local to construct a date/time object
# from a local calendar time

# you can get a date/time object from a date/tm object
# however, since date/time does not have the concept of timezones,
# you have to specify whether the date/tm object is in UTC or localtime
(def now-from-utc   (:timegm calendar-now-utc))   # timegm is for GMT (UTC)
(def now-from-local (:mktime calendar-now-local)) # mktime is for localtime

# you can compare date/time objects
(assert (= now-from-utc now-from-local))
(assert (= now-from-utc now))

# you can compare date/tm objects of the same type (local or UTC)
# however, such comparisons are not very robust

# you can print either date/time or date/tm objects in various formats
(print (date/format now))            # default format
(print (date/format now "%c"))       # equivalent
(print (date/format now :html))      # standard WG formatting
(print (date/format now :html true)) # print as UTC instead of localtime

(print (:strftime calendar-now-utc :html)) # takes same format argument
# the :strftime message is lower level, you *must* specify a format
# (:strftime calendar-now-utc) # error: arity mismatch

# you may mutate date/tm objects
# when you call mktime or gmtime, you are normalizing the object
# you can also normalize it in-place
(put calendar-now-utc :sec 120) # out of range
(:timegm! calendar-now-utc)
# now in range
(assert (> (:timegm calendar-now-utc) now))

# however, it is recommended to work with date/time objects
# you can mutate those using the update-time function suite
(def in-a-bit
 (update-time now
  # you can set values directly
  :hour 30
  # or you can pass an x -> x function like to `update`
  :sec inc))
(assert (> in-a-bit now))

# update-time is normalized to strip unsafe keys and takes keywords
# you may also use the less safe update-time* to pass a raw dict
(def in-a-bit* (update-time* now {:hour 30 :sec inc}))
(assert (= in-a-bit in-a-bit*))

# finally, there's a for-internal-use update-time!
# it takes a function that will apply transformations
# to the intermediate date/tm object
# it's not recommended for end-users

# finally, you can pass nil to the update functions, in which case
# (date/time) will be used, letting you skip a step
```
