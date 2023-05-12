(use date/native)

# you always start with this
# `time` returns the time in UTC
(def now (time))

# if you want to modify the time, you should use localtime, like so
(def lt (:localtime now))
(update lt :sec |(+ $ 2000))
(update lt :year |(- $ 10))

(def gt (:gmtime now))
(put gt :sec 13)

(pp now)
(pp lt)
(pp gt)
