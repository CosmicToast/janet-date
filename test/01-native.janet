(import date/native)

# capture current time in all formats
(def time  (native/time))
(def gmt   (:gmtime time))
(def local (:localtime time))

# no crashes yet? good
(def gd (:todict gmt))
(def ld (:todict local))

# comparisons
# compare with +/- 1 (catch mutability, off-by-one) and +/- 120 (ensure mktime works)
(loop [n :in [1 120]
       :let [dec |(- $ n)
             inc |(+ $ n)
             mrg |(merge $ {:sec ($1 ($ :sec))})]]
  (assert (= time (:mktime local)))
  (assert (> time (native/dict->time (mrg ld dec))))
  (assert (< time (native/dict->time (mrg ld inc))))
  (assert (= gmt (native/dict->tm gd)))
  (assert (> gmt (native/dict->tm (mrg gd dec))))
  (assert (< gmt (native/dict->tm (mrg gd inc)))))

# try all of the built-in formats
(def non-empty? (comp not zero? length))
(loop [obj :in [gmt local]
       fmt :in [:iso8601 :locale :email :rfc5322 "%c"]]
  (assert (non-empty? (:strftime obj fmt))
          (string/format "format produced empty string: %v" fmt)))
# try string and describe
(loop [obj :in [['time time] ['gmt gmt] ['local local]]
       fun :in [string describe]
       :let [[sym tim] obj]]
  (assert (non-empty? (fun tim))
          (string/format "calling function %v on %v failed" fun sym)))

(var ran? false)
(eachp [k v] gmt
  (set ran? true)
  (assert (keyword? k)))
(assert ran? "failed to iterate over tm")

# test timezone detection
# ... except when dst is on locally, it's not worth it, do not look into this
(when (false? (ld :isdst))
  (def ld2 (merge gd {:min (+ (gd :min) (native/tzoffset))}))
  (def lm2 (native/dict->tm ld2))
  (:normalize lm2)
  (assert (= lm2 local)))
