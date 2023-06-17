(import date/native)

(def time
  `Return an opaque date/time representation of the current moment.`
  native/time)

(defn time?
  `Check if x is a date/time object.`
  [x]
  (= :date/time (type x)))

(defn- tm
  [f &opt o]
  (def out (native/tm o))
  (f out)
  out)

(def utc
  `Generate a date/tm object from a compatible dictionary. Implying UTC.
  Non-specified fields will be initialized to their default values (usually zero).`
  (partial tm native/timegm!))

(def local
  `Generate a date/tm object from a compatible dictionary. Implying localtime.
  Non-specified fields will be initialized to their default values (usually zero).`
  (partial tm native/mktime!))

(defn tm?
  `Check if x is a date/tm object.`
  [x]
  (= :date/tm (type x)))

(defn- callable?
  [x]
  (or (function? x) (cfunction? x))) # callable abstracts are hard to detect

(defn- map-pairs
  [ds fun]
  (->> ds
       pairs
       (map fun)
       from-pairs))

(defn- put-date
  [ds key val]
  (assert (tm? ds))
  (assert val)
  (if (callable? val)
    (update ds key val)
    (put    ds key val)))

(defn update-time!
  ```
  Update a given date/time object as a whole.

  If nil is passed as time, the function will call (date/time) for you.

  This version will give you the corresponding timegm date/tm object to mutate
  as you see fit. This is dangerous and not perfectly portable.
  Use at your own peril.

  Don't forget to return the resulting object, as it will be normalized and
  transformed back into a date/time object for you.
  ```
  [time fun]
  (default time (native/time))
  (assert (time? time))
  (assert (callable? fun))
  (def newtm (fun (:gmtime time)))
  (:timegm! newtm)) # timegm! is slightly more efficient and we're discarding it

(defn update-time*
  `Variant of update-time that takes a dictionary without normalizing it.`
  [time ds]
  (assert (dictionary? ds))
  (update-time! time (fn [tm]
                       (eachp [k v] ds (put-date tm k v))
                       tm)))

(defn update-time
  ```
  Update a given date/time object.

  This takes a value for seconds, minutes, hours, day-of-month, month, and year.

  The value may either be callable, in which case it will be given the old value
  as an argument, and the return value will be used, or a value, in which case
  it will be used as-is. Note that the resulting values need not be in any given
  range, as they will be normalized.

  If you need additional control over the underlying gmtime tm call, use
  `update-time*` or `update-time!`.

  You can pass `nil` as `time`, in which case the current time will be used.
  Combined with specifying all the fields allows you to construct date/time
  objects in UTC.
  ```
  [time &keys {:sec  second
               :min  minute
               :hour hour
               :mday month-day
               :mon  month
               :year year}]
  (update-time* time {:sec   second
                      :min   minute
                      :hour  hour
                      :mday  month-day
                      :mon   month
                      :year  year
                      # UTC only
                      :isdst false}))

(defn- format-fn
  [time fmt f]
  (def tm (f time))
  (assert (tm? tm))
  (:strftime tm fmt))

(defn format
  ```
  Format a given `date/time` object according to the format.
  If `utc?` is truthy, interpret it as UTC, else as your local timezone.
  Format may either be a strftime-compatible string, or a preset keyword.
  The following presets are available:
  * :iso8601 (along with its -calendar, -ordinal, and -week variants)
  * :rfc3339
  * :html
  * :rfc5322 (and its aliases: :email, :rfc5321, :internet)
  * :default

  Note that the iso8601 and rc3339 implementations are not perfectly compliant.
  For details, see `src/tm.c#strftime_formats[]`.
  ```
  [time &opt fmt utc?]
  (format-fn time
             (or fmt :default)
             (if utc?
               native/gmtime
               native/localtime)))
