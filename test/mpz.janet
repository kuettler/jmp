(use jmp)

(assert (compare= (mpz 1) 1))
(assert (compare= (mpz (int/u64 3)) 3))

(def str-value "222222222222222222222222222222222222222222222222222222222222222220")
(assert (= (string (mpz str-value)) str-value))

# compare
(assert (= (mpz (int/s64 4)) (mpz 4)))
(assert (< (mpz (int/s64 3)) (mpz 4)))
(assert (> (mpz (int/s64 4)) (mpz 3)))

(assert (compare= (mpz (int/s64 4)) (int/s64 4)))
(assert (compare< (mpz (int/s64 3)) (int/s64 4)))
(assert (compare> (mpz (int/s64 4)) (int/s64 3)))

(assert (compare= (+ (mpz 1) (int/s64 2) (int/u64 3) (mpz 4) 5) 15))
#(print (+ (mpz 1) (int/s64 2) (int/u64 3) (mpz 4) 5 "aa" 3.4))

(assert (compare= (- (mpz 5) (mpz 2)) 3))
(assert (compare= (- (mpz 5) (int/u64 2)) 3))
(assert (compare= (- (mpz 5) (int/s64 2)) 3))
(assert (compare= (- (mpz 5) (int/s64 -2)) 7))
(assert (compare= (- (mpz 5) 2) 3))
(assert (compare= (- 4 (mpz 2)) 2))

(assert (compare= (* (mpz 5) (mpz 2)) 10))
(assert (compare= (* (mpz 5) (int/u64 2)) 10))
(assert (compare= (* (mpz 5) (int/s64 2)) 10))
(assert (compare= (* (mpz 5) (int/s64 -2)) -10))
(assert (compare= (* (mpz 5) 2) 10))
(assert (compare= (* 4 (mpz 2)) 8))

(assert (compare= (/ (mpz 6) (mpz 2)) 3))
(assert (compare= (/ (mpz 12) (mpz 2) (mpz 2)) 3))
(assert (compare= (/ (mpz 24) (mpz 2) (mpz 2)) 6))

(assert (compare= (/ (mpz 7) (mpz 2)) 3))
(assert (compare= (/ (mpz 5) (mpz 2)) 2))
(assert (compare= (/ (mpz 13) (mpz 2) (mpz 2)) 3))
(assert (compare= (/ (mpz 25) (mpz 2) (mpz 2)) 6))

(assert (compare= (/ (mpz 30) 6) 5))
(assert (compare= (/ (mpz 30) (int/u64 6)) 5))
(assert (compare= (/ (mpz 30) (int/s64 6)) 5))
(assert (compare= (/ (mpz 30) (int/s64 -6)) -5))
(assert (compare= (/ (mpz 30) (mpz -6)) -5))

(assert (compare= (/ 30 (mpz 6)) 5))
(assert (compare= (/ -30 (mpz 6)) -5))
(assert (compare= (/ 30 (mpz -6)) -5))

(assert (compare= (div (mpz 30) 6) 5))
(assert (compare= (div (mpz 30) (int/u64 6)) 5))
(assert (compare= (div (mpz 30) (int/s64 6)) 5))
(assert (compare= (div (mpz 30) (int/s64 -6)) -5))
(assert (compare= (div (mpz 30) (mpz -6)) -5))

(assert (compare= (div 30 (mpz 6)) 5))
(assert (compare= (div -30 (mpz 6)) -5))
(assert (compare= (div 30 (mpz -6)) -5))

(assert (compare= (div (mpz 7) (mpz 2)) 3))
(assert (compare= (div (mpz 5) (mpz 2)) 2))
(assert (compare= (div (mpz 13) (mpz 2) (mpz 2)) 3))
(assert (compare= (div (mpz 25) (mpz 2) (mpz 2)) 6))

(assert (compare= (% 30 (mpz 6)) 0))
(assert (compare= (% -30 (mpz 6)) 0))
(assert (compare= (% 30 (mpz -6)) 0))

(assert (compare= (% (mpz 7) (mpz 2)) 1))
(assert (compare= (% (mpz 5) (mpz 2)) 1))
(assert (compare= (% (mpz 13) (mpz 2) (mpz 2)) 1))
(assert (compare= (% (mpz 25) (mpz 2) (mpz 2)) 1))

(assert (compare= (% (mpz 33) (mpz 12) (mpz 4)) 1))
(assert (compare= (% (mpz -33) (mpz 12) (mpz 4)) 3))

(assert (compare= (% (mpz 3) 4) 3))
(assert (compare= (% (mpz 3) -4) 3))

(assert (compare= (% (mpz 3) (int/s64 4)) 3))
(assert (compare= (% (mpz 3) (int/s64 -4)) 3))

(assert (compare= (band (mpz 22) (mpz 7)) 6))
(assert (compare= (bor (mpz 2) (mpz 4) (mpz 16)) 22))
(assert (compare= (bxor (mpz 2) (mpz 4) (mpz 16)) 22))
(assert (compare= (bxor (mpz 2) (mpz 6) (mpz 16)) 20))

(assert (compare= (bnot (mpz 0)) -1))
(assert (compare= (bnot (mpz 10)) -11))

(def bitset (mpz 0))
(setbit bitset 8)
(assert (= (tstbit bitset 8) 1))
(clrbit bitset 8)
(assert (zero? (tstbit bitset 8)))
(combit bitset 8)
(assert (= (tstbit bitset 8) 1))

(def value 1234567890000)
(assert (= (import-str (export-str (mpz value))) (mpz value)))
