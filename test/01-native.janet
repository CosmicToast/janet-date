(use date/native)

# you generally start with this
# `time` returns the time in UTC
# note that it's an *opaque* type
# you should not try to interpret it except with comparisons
(def now (time))

# you can convert a `time` object into a `tm` object which represents a date
# using :localtime or :gmtime, where gmtime interprets the time as UTC
(def loc (:localtime now))
(def gmt (:gmtime    now))

# you can modify `tm` objects like an associative array
# this means that put, update, keys, values, etc all work
# let's make another `tm` object, but remove one second from it
(def loc* (:localtime now))
(update loc* :sec dec)

# if the second count was at 0, it would now be at -1, which is out of range
# that's not a problem though, you can actually already perform comparisons
# WARN: you can ONLY compare `tm` objects of the same type (localtime vs gmtime)
(assert (< loc* loc))

# :mktime, and :mktime! "renormalize" the object
# :mktime returns a `time` object
# :mktime! does that, and normalizes the `tm` object in place by mutating it
(assert (= (:mktime loc)  now))
(assert (< (:mktime loc*) now))

# you must not call :mktime or :mktime! on the output of a :gmtime
# the data *will* be wrong, as mktime (as per the spec) presumes :localtime
(assert (not= (:mktime gmt) now))

# as such, the correct approach is to keep things in `time` format as much as
# possible, only converting to :localtime or :gmtime for mutation and formatting
# this is roughly what the janet wrapper does
(defn- put-date
  [ds key val]
  (when val # allow null val for update-time
    (if (function? val)
      (update ds key val)
      (put ds key val))))
(defn update-time
  [time &keys {:sec seconds
               :min minutes
               :hour hours
               :mday month-day
               :mon month
               :year year}]
  (assert (= (type time) :date/time))
  (let [l (:localtime time)
        p (partial put-date l)]
    (p :sec seconds)
    (p :min minutes)
    (p :hour hours)
    (p :mday month-day)
    (p :mon month)
    (p :year year)
    (:mktime! l))) # mktime! is slightly more efficient and we're going to GC l anyway

# all supported keys can be given a function
(assert (< now (update-time now :sec inc)))
(assert (> now (update-time now :sec dec)))

# or a value to set them to
(assert (< now (update-time now :sec 61)))
(assert (> now (update-time now :sec -1)))
