(define (map f lista)
    (if
        (null? lista)
        lista
        (cons (f (car lista)) (map f (cdr lista)))
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; merging two sorted lists X and Y according to cmp
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (merge X Y cmp)
    (cond
        ((null? X) Y)
        ((null? Y) X)
        (else
            (if
                (cmp (car X) (car Y))
                (cons (car X) (merge (cdr X) Y cmp))
                (cons (car Y) (merge X (cdr Y) cmp))
            )
        )
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; generic merge_sort using cmp
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (split_at p L)
    (if (= 0 p)
        (list (list) L)
        (let
            (
                (T (split_at (- p 1) (cdr L)))
            )
            (list
                (cons (car L) (car T))
                (car (cdr T))
            )    
        )
    )
)

(define (split L)
    (split_at (quotient (length L) 2) L)
)

(define (merge_sort L cmp)
    (if
        (< (length L) 2)
        L
        (let*
            (
                (AB (split L))
                (A (merge_sort (car AB) cmp))
                (B (merge_sort (car (cdr AB)) cmp))
            )
            (merge A B cmp)
        )
    )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; block merge sort 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (split_into_blocks B cmp)
    (if
        (null? (cdr B))
        (list B)
        (let*
            (
                (T (split_into_blocks (cdr B) cmp))
            )
            (if
                (cmp (car B) (car (car T)))
                (cons
                    (cons (car B) (car T)) (cdr T)
                )
                (cons
                    (list (car B)) T
                )
            )
        )
    )
)

(define (insert X blocks)
    (if
        (or (null? blocks) (<= (length X) (length (car blocks))))
        (cons X blocks)
        (cons (car blocks) (insert X (cdr blocks)))
    )
)
    
(define (merge_blocks blocks cmp)
    (if
        (null? (cdr blocks))
        (car blocks)
        (let
            (
                (X (car blocks))
                (Y (car (cdr blocks)))
                (R (cdr (cdr blocks)))
            )
            (merge_blocks (insert (merge X Y cmp) R) cmp)
        )
    )
)

(define (block_merge_sort A key cmp)
	(define (elem_key x) (list (key x) x))
	(define (cmp_key x y) (cmp (car x) (car y)))
    (define (cmp_blocks x y) (<= (length x) (length y)))
    (if
        (or (null? A) (null? (cdr A)))
        A
        (let*
            (
                (B (map elem_key A))
                (blocks (split_into_blocks B cmp_key))
                (s_blocks (merge_sort blocks cmp_blocks))
                (C (merge_blocks blocks cmp_key))
            )
            (map cadr C)
        )
    )
)

; (block_merge_sort (list 3 0 1 2 6 5 4) (lambda (x) x) <)










