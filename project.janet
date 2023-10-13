(declare-project
  :name "jmp"
  :description ```A wrapper around the GNU multiple precision arithmetic library ```
  :version "0.0.0")

(declare-source
  :prefix "jmp"
  :source ["jmp/init.janet"])

(declare-native
  :name "jmp-native"
  :source @["c/module.c"])