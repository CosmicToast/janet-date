(import ../date)

(def now (date/time))
(assert (date/time? now))

(def gmt (:gmtime now))
(def loc (:localtime now))
(assert (date/tm? gmt))
(assert (date/tm? loc))
(assert (= (:mktime loc) (:timegm gmt)))

(def before (date/update-time now :sec -1 :min dec))
(def after (date/update-time now :sec 61 :min inc))
(assert (> after now before))

# default format
(assert (date/format now))
# utc default format
(assert (date/format now :default true))
# format string
(assert (date/format now :%c))

# ensure tm + gmtime are spec-compliant
(def u (date/utc {:year 1970}))
(def l (date/local {:year 1970}))
(assert (= u
           (:gmtime (:timegm u))))
(assert (= l
           (:localtime (:mktime l))))
