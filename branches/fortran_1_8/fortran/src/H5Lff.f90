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
! This file contains Fortran90 interfaces for H5L functions.
!
MODULE H5L

  USE H5GLOBAL

CONTAINS

!----------------------------------------------------------------------
! Name:		h5ldelete_f 
!
! Purpose: 	Removes a link from a group.
!
! Inputs:  
!         loc_id   - Identifier of the file or group containing the object
!         name     - Name of the link to delete
!         lapl_id  - Link access property list identifier
! 
! Outputs: 
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	M.S. Breitenfeld
!		January, 2008
!
! Modifications:
!
! Comment:		
!----------------------------------------------------------------------
  SUBROUTINE h5ldelete_f(loc_id, name, lapl_id, hdferr) 
!
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5ldelete_f
!DEC$endif
!
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id  ! Identifier of the file or group containing the object
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of the link to delete
    INTEGER(HID_T), INTENT(IN) :: lapl_id ! Link access property list identifier
    INTEGER, INTENT(OUT) :: hdferr        ! Error code: 
                                          ! 0 on success and -1 on failure
    INTEGER(SIZE_T) :: namelen

!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5ldelete_c(loc_id, name, namelen, lapl_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5LDELETE_C'::h5ldelete_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: loc_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER(HID_T), INTENT(IN) :: lapl_id
         INTEGER(SIZE_T) :: namelen
       END FUNCTION h5ldelete_c
    END INTERFACE

    namelen = LEN(name)

    hdferr = h5ldelete_c(loc_id, name, namelen, lapl_id)

  END SUBROUTINE h5ldelete_f 

END MODULE H5L
