!****h* fortran/src/H5_ff_F03.f90
!
! NAME
!   H5LIB
!
! FUNCTION
!   This file contains helper functions for Fortran 2003 features and is
!   only compiled when Fortran 2003 features are enabled, otherwise
!   the file H5_ff_DEPRECIATED.f90 is compiled.
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
!                          *** IMPORTANT ***
!   If you add a new function you must add the function name to the 
!   Windows dll file 'hdf5_fortrandll.def' in the fortran/src directory.
!   This is needed for Windows based operating systems.
!*****

MODULE H5LIB_PROVISIONAL

CONTAINS

!----------------------------------------------------------------------
! Name:		h5offsetof 
!
! Purpose:	Computes the offset in memory 
!
! Inputs:       start - starting pointer address
!                 end - ending pointer address
! Outputs:  
!	        offset - offset  
! Optional parameters:
!				NONE			
!
! Programmer: M.S. Breitenfeld
!             Augest 25, 2008
!
!
! Comment: 		
!----------------------------------------------------------------------
  INTEGER(SIZE_T) FUNCTION h5offsetof(start,END) RESULT(offset)
    USE, INTRINSIC :: ISO_C_BINDING
    USE H5GLOBAL
    IMPLICIT NONE

    TYPE(C_PTR), VALUE, INTENT(IN) :: start, end
    INTEGER(C_INTPTR_T) :: int_address_start, int_address_end

    int_address_start = TRANSFER(start, int_address_start)
    int_address_end   = TRANSFER(end  , int_address_end  )

    offset = int_address_end - int_address_start

  END FUNCTION h5offsetof

END MODULE H5LIB_PROVISIONAL
