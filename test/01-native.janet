(use date/native)

(defn platform [& ps] (some (partial = (os/which)) ps))

# you generally start with this
# `time` returns the time in UTC
(def now (time))

# you can convert a `time` object into a `tm` object which represents a date
# using :localtime or :gmtime, where gmtime interprets the time as UTC
(def loc (:localtime now))

# you can modify `tm` objects like an associative array
# this means that put, update, keys, values, etc all work
# let's make another `tm` object, but remove one second from it
(def loc* (:localtime now))
(update loc* :sec dec)

# if the second count was at 0, it would now be at -1, which is out of range
# that's not a problem though, you can actually already perform comparisons
(assert (< loc* loc))

# :localtime, :localtime!, :gmtime, :gmtime!, :mktime, and :mktime!
# all "renormalize" the object
# :mktime returns a `time` object
# :mktime! does that, and normalizes the `tm` object in place by mutating it
(assert (= (:mktime loc) now))
(assert (< (:mktime loc*) now))

# :gmtime(!) is equivalent to (:gmtime (:mktime(!) tm)), ditto for localtime
(def loc* (:localtime! loc*))
(assert (< loc* loc))

# macos has a bug in its libc that makes it misinterpret gmtime outputs
# as localtime under certain conditions, so the gmtime tests are skipped
# TODO: UTC tests
(unless (platform :macos))

# TODO: building a UTC datetime
