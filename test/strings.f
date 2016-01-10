!     This file contains tests for subroutines that take strings as arguments

!     Takes a string S as input and sets L to LEN(S) and C to S(1:1)
      SUBROUTINE STRING_TEST(C, L, S)

      CHARACTER*(*) S
      CHARACTER C
      INTEGER*4 L

      L = LEN(S)
      C = S(1:1)

      END

!     Takes an array of 3 strings of unknown length
!     IC gets the first character of each string as a 1 byte integer
      SUBROUTINE STRING_ARRAY_TEST(IC, SA)

      CHARACTER*(*) SA(3)
      INTEGER*1 IC(LEN(SA))

      DO 10 I = 1,3
         IC(I) = ICHAR(SA(I)(1:1))
 10   CONTINUE

      END
      
