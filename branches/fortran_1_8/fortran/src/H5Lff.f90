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
! 
! Outputs: 
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!         lapl_id  - Link access property list identifier
!
! Programmer:	M.S. Breitenfeld
!		January, 2008
!
! Modifications:
!
! Comment:		
!----------------------------------------------------------------------
  SUBROUTINE h5ldelete_f(loc_id, name, hdferr, lapl_id) 
!
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5ldelete_f
!DEC$endif
!
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id  ! Identifier of the file or group containing the object
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Name of the link to delete
    INTEGER, INTENT(OUT) :: hdferr        ! Error code: 
                                          ! 0 on success and -1 on failure
    INTEGER(SIZE_T) :: namelen
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: lapl_id ! Link access property list identifier
    INTEGER(HID_T) :: lapl_id_default
    

!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5ldelete_c(loc_id, name, namelen, lapl_id_default)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5LDELETE_C'::h5ldelete_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: loc_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER(HID_T) :: lapl_id_default
         INTEGER(SIZE_T) :: namelen
       END FUNCTION h5ldelete_c
    END INTERFACE

    namelen = LEN(name)

    lapl_id_default = H5P_DEFAULT_F
    IF(PRESENT(lapl_id)) lapl_id_default = lapl_id

    hdferr = h5ldelete_c(loc_id, name, namelen, lapl_id_default)

  END SUBROUTINE h5ldelete_f 

!----------------------------------------------------------------------
! Name:		H5Lcreate_soft_f 
!
! Purpose: 	Creates a soft link to an object.
!
! Inputs:
!       target_path - Path to the target object, which is not required to exist.
!       link_loc_id - The file or group identifier for the new link.
!       link_name   - The name of the new link.
! 
! Outputs: 
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!       lcpl_id     - Link creation property list identifier.
!       lapl_id     - Link access property list identifier.
!
! Programmer:	M.S. Breitenfeld
!		February 20, 2008
!
! Modifications:
!
! Comment:		
!----------------------------------------------------------------------
  SUBROUTINE h5lcreate_soft_f(target_path, link_loc_id, link_name, hdferr, lcpl_id, lapl_id) 
!
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5lcreate_soft_f
!DEC$endif
!
    IMPLICIT NONE
    CHARACTER(LEN=*), INTENT(IN) :: target_path     ! Path to the target object, which is not required to exist.
    INTEGER(HID_T), INTENT(IN) ::   link_loc_id     ! The file or group identifier for the new link.
    CHARACTER(LEN=*), INTENT(IN) :: link_name       ! The name of the new link.
    INTEGER, INTENT(OUT) :: hdferr        ! Error code: 
                                          ! 0 on success and -1 on failure
    INTEGER(HID_T), OPTIONAL, INTENT(IN) ::   lcpl_id         ! Link creation property list identifier.
    INTEGER(HID_T), OPTIONAL, INTENT(IN) ::   lapl_id         ! Link access property list identifier.

    INTEGER(HID_T) :: lcpl_id_default 
    INTEGER(HID_T) :: lapl_id_default     
    INTEGER(SIZE_T) :: target_path_len
    INTEGER(SIZE_T) :: link_name_len

!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5lcreate_soft_c(target_path, target_path_len, &
            link_loc_id, &
            link_name,link_name_len, &
            lcpl_id_default, lapl_id_default )
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5LCREATE_SOFT_C'::h5lcreate_soft_c
         !DEC$ ENDIF
         CHARACTER(LEN=*), INTENT(IN) :: target_path
         INTEGER(SIZE_T) :: target_path_len
         INTEGER(HID_T), INTENT(IN) ::   link_loc_id
         CHARACTER(LEN=*), INTENT(IN) :: link_name
         INTEGER(SIZE_T) :: link_name_len
         INTEGER(HID_T) :: lcpl_id_default
         INTEGER(HID_T) :: lapl_id_default
       END FUNCTION h5lcreate_soft_c
    END INTERFACE

    target_path_len = LEN(target_path)
    link_name_len = LEN(link_name)

    lcpl_id_default = H5P_DEFAULT_F
    IF(PRESENT(lcpl_id)) lcpl_id_default = lcpl_id
    lapl_id_default = H5P_DEFAULT_F
    IF(PRESENT(lapl_id)) lapl_id_default = lapl_id

    hdferr = h5lcreate_soft_c(target_path, target_path_len,&
         link_loc_id, &
         link_name, link_name_len, &
         lcpl_id_default, lapl_id_default )

  END SUBROUTINE h5lcreate_soft_f

END MODULE H5L
