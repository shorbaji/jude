(define double (lambda (x) (+ x x)))
(define twice
  (lambda (fn)
    (lambda (x)
      (fn (fn x)))))

(define quad
  (twice double))

(define octo
  (twice quad))

(double 4)

(quad 4)

(define fact (lambda (x)
	       (if (eq x 0)
		   1
		   (* x (fact (- x 1))))))
