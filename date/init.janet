(import date/native)

(defn time
  `Return an opaque datetime representation of the current moment.`
  []
  (native/time))

(defn time?
  `Check if x is a datetime object.`
  [x]
  (= :date/time (type x)))

(defn tm?
  `Check if x is a datetm object.`
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
  Update a given datetime object as a whole.

  If nil is passed as time, the function will call (date/time) for you.

  This version will give you the corresponding localtime datetm object to mutate
  as you see fit. This is dangerous and not perfectly portable.
  Use at your own peril.

  Don't forget to return the resulting object, as it will be normalized and
  transformed back into a datetime object for you.
  ```
  [time fun]
  (default time (native/time))
  (assert (time? time))
  (assert (callable? fun))
  (def newtm (fun (:localtime time)))
  (:mktime newtm)) # mktime! is slightly more efficient and we're discarding it

(defn update-time*
  `Variant of update-time that takes a dictionary.`
  [time ds]
  (assert (dictionary? ds))
  (update-time! time (fn [tm]
                       (eachp [k v] ds (put-date tm k v))
                       tm)))

(defn update-time
  ```
  Update a given datetime object.

  This takes a value for seconds, minutes, hours, day-of-month, month, and year.

  The value may either be callable, in which case it will be given the old value
  as an argument, and the return value will be used, or a value, in which case
  it will be used as-is. Note that the resulting values need not be in any given
  range, as they will be normalized.

  It will automatically re-determine if DST is applicable and give you back a
  modified datetime object.
  If you need additional control over the underlying localtime tm call, use
  `update-time*` or `update-time!`.

  You can pass `nil` as `time`, in which case the current time will be used.
  Combined with specifying all the fields allows you to construct datetime
  objects in localtime. Constructing UTC datetime objects is non-trivial when
  limited to what is provided by ISO C99.
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
                      :isdst :detect}))
