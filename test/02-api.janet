(import ../date)

(def now (date/time))
(assert (date/time? now))

(def gmt (:gmtime now))
(def loc (:localtime now))
(assert (date/tm? gmt))
(assert (date/tm? loc))
(assert (= (:mktime loc) (:timegm gmt)))

(def before (date/update-time* now {:sec -1 :min dec}))
(def after  (date/update-time* now {:sec 61 :min inc}))
(assert (> after now before))
