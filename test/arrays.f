      SUBROUTINE ARRAY_TEST1(X)

      REAL*4 X
      DIMENSION X(2,3)
      INTEGER ROW, COL

      DO 10 ROW = 1,2
         DO 20 COL = 1,3
            X(ROW, COL) = COL + (ROW-1)*3.
 20      CONTINUE
 10   CONTINUE

      END
      
