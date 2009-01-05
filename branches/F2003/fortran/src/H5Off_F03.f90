!****h* ROBODoc/H5O (F03)
!
! NAME
!  H5O_PROVISIONAL
!
! PURPOSE
!  This file contains Fortran 90 and Fortran 2003 interfaces for H5O functions.
!  It contains the same functions as H5Off_DEPRECIATE.f90 but includes the
!  Fortran 2003 functions and the interface listings. This file will be compiled
!  instead of H5Off_DEPRECIATE.f90 if Fortran 2003 functions are enabled.
!
! COPYRIGHT
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!   Copyright by The HDF Group.                                               *
!   Copyright by the Board of Trustees of the University of Illinois.         *
!   All rights reserved.                                                      *
!                                                                             *
!   This file is part of HDF5.  The full HDF5 copyright notice, including     *
!   terms governing use, modification, and redistribution, is contained in    *
!   the files COPYING and Copyright.html.  COPYING can be found at the root   *
!   of the source code distribution tree; Copyright.html can be found at the  *
!   root level of an installed copy of the electronic HDF5 document set and   *
!   is linked from the top-level documents page.  It can also be found at     *
!   http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
!   access to either file, you may request a copy from help@hdfgroup.org.     *
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!
! NOTES
!                         *** IMPORTANT ***
!  If you add a new H5P function you must add the function name to the
!  Windows dll file 'hdf5_fortrandll.def' in the fortran/src directory.
!  This is needed for Windows based operating systems.
!
!*****

MODULE H5O_PROVISIONAL

  USE H5GLOBAL

CONTAINS

!****s* H5O (F03)/h5ovisit_f
!
! NAME
!  h5ovisit_f
!
! PURPOSE
!  Recursively visits all objects starting from a specified object.
!
! INPUTS
!  group_id 	 - Identifier of the group at which the recursive iteration begins
!  index_type 	 - Type of index; valid values include:
!                    H5_INDEX_NAME_F
!                    H5_INDEX_CRT_ORDER_F
!  order 	 - Order in which index is traversed; valid values include:
!                    H5_ITER_DEC_F
!                    H5_ITER_INC_F
!                    H5_ITER_NATIVE_F
!  op 	         - Callback function passing data regarding the group to the calling application
!  op_data 	 - User-defined pointer to data required by the application for its processing of the group
!
! OUTPUTS
!  idx 	         - returns the return value of the first operator that returns a positive value, or 
!                  zero if all members were processed with no operator returning non-zero.
!  hdferr 	 - error code:
!                    0 on success and -1 on failure
! AUTHOR
!  M.S. Breitenfeld
!  November 19, 2008
!
! SOURCE
  SUBROUTINE h5ovisit_f(group_id, index_type, order, op, op_data, return_value, hdferr)
    USE ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: group_id
    INTEGER, INTENT(IN) :: index_type 
    INTEGER, INTENT(IN) :: order
!!$    INTEGER(HSIZE_T), INTENT(INOUT) :: idx  ! IN : Iteration position at which to start
!!$                                            ! OUT: Position at which an interrupted iteration may be restarted

    TYPE(C_FUNPTR):: op      ! Callback function passing data regarding the link to the calling application
    TYPE(C_PTR)   :: op_data ! User-defined pointer to data required by the application for its processing of the link

    INTEGER, INTENT(OUT) :: return_value ! Success:   The return value of the first operator that
 				         !            returns non-zero, or zero if all members were
 				         !            processed with no operator returning non-zero.

 		                         ! Failure:   Negative if something goes wrong within the
 				         !            library, or the negative value returned by one
 				         !            of the operators.

    INTEGER, INTENT(OUT) :: hdferr       ! Error code:
                                         ! 0 on success and -1 on failure
!*****
    INTERFACE
       INTEGER FUNCTION h5ovisit_c(group_id, index_type, order, op, op_data)
         USE ISO_C_BINDING
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5OVISIT_C'::h5ovisit_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: group_id
         INTEGER, INTENT(IN) :: index_type
         INTEGER, INTENT(IN) :: order
         TYPE(C_FUNPTR), VALUE :: op
         TYPE(C_PTR), VALUE :: op_data
       END FUNCTION h5ovisit_c
    END INTERFACE

    return_value = h5ovisit_c(group_id, index_type, order, op, op_data)

    IF(return_value.GE.0)THEN
       hdferr = 0
    ELSE
       hdferr = -1
    END IF

  END SUBROUTINE h5ovisit_f

END MODULE H5O_PROVISIONAL

