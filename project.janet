(declare-project
  :name "date"
  :description "C99 date library for Janet"
  :author "Chloe Kudryavtsev <toast@bunkerlabs.net>"
  :license "Unlicense"
  :repo "https://github.com/CosmicToast/janet-date.git"
  :url  "https://github.com/CosmicToast/janet-date.git")

(declare-source
  :source ["date"])

(def tz-source ["tz/localtime.c"
                "tz/asctime.c"
                "tz/difftime.c"
                "tz/strftime.c"])

(declare-native
  :name "date/native"
  :source ["src/main.c"
           "src/polyfill.c"
           "src/time.c"
           "src/tm.c"
           "src/util.c"

           ;(if (= :windows (os/which)) [] tz-source)])
