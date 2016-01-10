!     this file contains functions to test f2h handling of return types
!     https://en.wikibooks.org/wiki/Fortran/Fortran_procedures_and_functions
!     
!     test function that returns a double
      REAL*8 FUNCTION TIMES2(A)
      REAL*8 A

      TIMES2 = A*2.0
      RETURN
      END

!     test function that returns a double complex
      COMPLEX*16 FUNCTION Z_TIMES2(A)
      COMPLEX*16 A

      Z_TIMES2 = A*2.0
      RETURN
      END


!     need to test alternate return mechanism with intent
!     and returning arrays and strings
