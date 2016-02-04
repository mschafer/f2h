      SUBROUTINE VECTOR_TEST(S, V)

      implicit none
      COMMON /ARRAYS_COMMON1/ IC, VC, MC, ZC

      INTEGER*1 IC
      REAL*4 VC(4)
      REAL*8 MC(3,3)
      COMPLEX*16 ZC(4)
      
      REAL*4 S, V
      INTEGER I
      DIMENSION V(3)

      DO 10 I = 1,3
         V(I) = V(I) + S
 10   CONTINUE
      
      END
      
      
      SUBROUTINE VECTOR_TEST2(S, V, N)

      implicit none
      REAL*4 S, V
      INTEGER I, N
      DIMENSION V(*)

      DO 10 I = 1,N
         V(I) = V(I) + S
 10   CONTINUE
      
      END
      
      SUBROUTINE MATRIX_TEST(S, M, NCOLS)

      implicit none
      REAL*4 S, M
      INTEGER I, NCOLS
      DIMENSION M(4,*)

      DO 10 I = 1,NCOLS
         M(2,I) = M(2,I) + S
 10   CONTINUE
      
      END

      SUBROUTINE ARRAY_TEST1(S, V, M, X)

      implicit none
      REAL*4 S, V, M, X
      DIMENSION V(10), M(4,4), X(3,5,7)
      INTEGER ROW, COL

      END
      
