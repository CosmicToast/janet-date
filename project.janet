(declare-project
  :name "date"
  :description "C99 date library for Janet"
  :author "Chloe Kudryavtsev <toast@bunkerlabs.net>"
  :license "Unlicense"
  :repo "https://github.com/CosmicToast/janet-date.git")

(declare-source
  :source ["date"])

(declare-native
  :name "date/native"
  :source ["date.c"])
