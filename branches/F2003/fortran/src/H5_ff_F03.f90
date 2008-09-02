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
!
! This file contains helper functions for Fortran 2003 features. 
! Only compiled when Fortran 2003 is specified as the compiler.
!
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
!	       offset - offset  
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
!
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5offsetof
!DEC$endif
!
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
