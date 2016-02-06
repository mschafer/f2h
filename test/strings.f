!     This file contains tests for subroutines that take strings as arguments

!     Takes a string S as input and sets L to LEN(S) and C to S(1:1)
      SUBROUTINE STRING_TEST(C, L, S)

      IMPLICIT NONE

      COMMON /com_string1/ cs1, cs2, r1
      CHARACTER *(7) cs1
      CHARACTER *(5) cs2(8,6)
      REAL*8 r1
      
      CHARACTER*(*) S
      CHARACTER C
      INTEGER*4 L

      L = LEN(S)
      C = S(1:1)

      END


!     Takes an array of 3 strings of unknown length
!     IC gets the first character of each string as a 1 byte integer
      SUBROUTINE STRING_ARRAY_TEST(IC, SA)

      IMPLICIT NONE

      COMMON /com_string1/ cs1, cs2
      CHARACTER *(7) cs1
      CHARACTER *(5) cs2(8,6)
      REAL*8 r1

      CHARACTER*(*) SA(3)
      INTEGER*1 IC(3)
      CHARACTER *(7) local_string
      CHARACTER *(5) local_string_array(8,6)
      INTEGER I

      DO 10 I = 1,3
         IC(I) = ICHAR(SA(I)(1:1))
 10   CONTINUE

      END
      
