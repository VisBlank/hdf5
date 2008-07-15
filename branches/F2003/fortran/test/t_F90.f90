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

! *****************************************
! ***        H 5 L   T E S T S
! *****************************************

!/*-------------------------------------------------------------------------
! * Function:	test_iter_group
! *
! * Purpose:	Dummy function in replace of Fortran 2003 routines
! *
! * Return:	Success:	0
! *		Failure:	number of errors
! *
! * Programmer:	M.S. Breitenfeld
! *             July 15, 2008
! *
! *-------------------------------------------------------------------------
! */

SUBROUTINE test_iter_group(total_error)
  
  IMPLICIT NONE
  INTEGER, INTENT(INOUT) :: total_error

  RETURN

END SUBROUTINE test_iter_group


! *****************************************
! ***        H 5 P   T E S T S
! *****************************************

!/*-------------------------------------------------------------------------
! * Function:	test_getset_vl
! *
! * Purpose:	Dummy function in replace of Fortran 2003 routines
! *
! * Return:	Success:	0
! *		Failure:	number of errors
! *
! * Programmer:	M.S. Breitenfeld
! *             June 18, 2008
! *
! *-------------------------------------------------------------------------
! */

SUBROUTINE test_getset_vl(cleanup, total_error)
  
  IMPLICIT NONE
  LOGICAL, INTENT(IN)  :: cleanup
  INTEGER, INTENT(INOUT) :: total_error

  RETURN

END SUBROUTINE test_getset_vl

!/*-------------------------------------------------------------------------
! * Function:	test_getset
! *
! * Purpose:	Dummy function in replace of Fortran 2003 routines
! *
! * Return:	Success:	0
! *
! *		Failure:	number of errors
! *
! * Programmer:	M.S. Breitenfeld
! *             June 16, 2008
! *
! * Modifications:
! *
! *-------------------------------------------------------------------------
! */

SUBROUTINE test_getset(cleanup, total_error)

  IMPLICIT NONE
  LOGICAL, INTENT(IN)  :: cleanup
  INTEGER, INTENT(INOUT) :: total_error

  RETURN

END SUBROUTINE test_getset

SUBROUTINE test_genprop_class_callback(total_error)

  IMPLICIT NONE
  INTEGER, INTENT(INOUT) :: total_error

  RETURN

END SUBROUTINE test_genprop_class_callback


! *****************************************
! ***        H 5 T   T E S T S
! *****************************************

SUBROUTINE test_array_compound_atomic(total_error)
    
  IMPLICIT NONE
    
  INTEGER, INTENT(INOUT) :: total_error

END SUBROUTINE test_array_compound_atomic


SUBROUTINE test_array_compound_array(total_error)
  
  IMPLICIT NONE
  
  INTEGER, INTENT(INOUT) :: total_error
  
END SUBROUTINE test_array_compound_array


SUBROUTINE test_array_bkg(total_error)
  
  IMPLICIT NONE
  
  INTEGER, INTENT(INOUT) :: total_error
  
END SUBROUTINE test_array_bkg

SUBROUTINE test_create(total_error)
  
  IMPLICIT NONE
  
  INTEGER, INTENT(INOUT) :: total_error
  
END SUBROUTINE test_create
