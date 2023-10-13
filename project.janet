(declare-project
  :name "jmp"
  :description ```A wrapper around the GNU multiple precision arithmetic library```
  :version "0.0.1")

(def cflags '[])
(def lflags '["-lgmp"])

(declare-native
  :name "jmp"
  :source @["c/mpz.c"]
  :cflags [;default-cflags ;cflags]
  :lflags [;default-lflags ;lflags]
  )
