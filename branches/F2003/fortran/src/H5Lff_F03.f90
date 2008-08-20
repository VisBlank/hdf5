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
MODULE H5L_PROVISIONAL

  USE H5GLOBAL

CONTAINS

!----------------------------------------------------------------------
! Name:		h5literate_f 
!
! Purpose: 	Iterates through links in a group.
!
! Inputs:
!     group_id - Identifier specifying subject group
!   index_type - Type of index which determines the order
!        order - Order within index
!          idx - Iteration position at which to start
!           op - Callback function passing data regarding the link to the calling application
!      op_data - User-defined pointer to data required by the application for its processing of the link
! 
! Outputs: 
!          idx - Position at which an interrupted iteration may be restarted
!       hdferr - error code:
!			Success:  0
!			Failure: -1   
! Optional parameters:
!
! Programmer:	M.S. Breitenfeld
!		July 8, 2008
!
! Modifications:
!
! Comment:		
!----------------------------------------------------------------------
  SUBROUTINE h5literate_f(group_id, index_type, order, idx, op, op_data, return_value, hdferr) 
!
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5literate_f
!DEC$endif
!
    USE ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: group_id  ! Identifier specifying subject group
    INTEGER, INTENT(IN) :: index_type       ! Type of index which determines the order:
                                                ! H5_INDEX_NAME_F - Alpha-numeric index on name
                                                ! H5_INDEX_CRT_ORDER_F - Index on creation order
    INTEGER, INTENT(IN) :: order            ! Order within index:
                                                ! H5_ITER_INC_F - Increasing order
                                                ! H5_ITER_DEC_F - Decreasing order
                                                ! H5_ITER_NATIVE_F - Fastest available order
    INTEGER(HSIZE_T), INTENT(INOUT) :: idx  ! IN : Iteration position at which to start
                                            ! OUT: Position at which an interrupted iteration may be restarted

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
    INTERFACE
       INTEGER FUNCTION h5literate_c(group_id, index_type, order, idx, op, op_data)
         USE ISO_C_BINDING
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5LITERATE_C'::h5literate_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: group_id 
         INTEGER, INTENT(IN) :: index_type
         INTEGER, INTENT(IN) :: order
         INTEGER(HSIZE_T), INTENT(INOUT) :: idx
         TYPE(C_FUNPTR), VALUE :: op
         TYPE(C_PTR), VALUE :: op_data
       END FUNCTION h5literate_c
    END INTERFACE

    return_value = h5literate_c(group_id, index_type, order, idx, op, op_data)

    IF(return_value.GE.0)THEN
       hdferr = 0
    ELSE
       hdferr = -1
    END IF

  END SUBROUTINE h5literate_f

!----------------------------------------------------------------------
! Name:		h5literate_by_name_f 
!
! Purpose: 	Iterates through links in a group.
!
! Inputs:
!     loc_id   - File or group identifier specifying location of subject group
!   group_name - Name of subject group 
!   index_type - Type of index which determines the order
!        order - Order within index
!          idx - Iteration position at which to start
!           op - Callback function passing data regarding the link to the calling application
!      op_data - User-defined pointer to data required by the application for its processing of the link
! 
! Outputs: 
!          idx - Position at which an interrupted iteration may be restarted
!       hdferr - error code:
!			Success:  0
!			Failure: -1   
! Optional parameters:
!      lapl_id - Link access property list
!
! Programmer:	M.S. Breitenfeld
!		Augest 18, 2008
!
! Modifications:
!
! Comment:		
!----------------------------------------------------------------------
  SUBROUTINE h5literate_by_name_f(loc_id, group_name, index_type, order, idx, op, op_data, return_value, hdferr, lapl_id) 
!
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5literate_by_name_f
!DEC$endif
!
    USE ISO_C_BINDING
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id  ! Identifier specifying subject group
    CHARACTER(LEN=*) :: group_name ! Name of subject group 
    INTEGER, INTENT(IN) :: index_type       ! Type of index which determines the order:
                                                ! H5_INDEX_NAME_F - Alpha-numeric index on name
                                                ! H5_INDEX_CRT_ORDER_F - Index on creation order
    INTEGER, INTENT(IN) :: order            ! Order within index:
                                                ! H5_ITER_INC_F - Increasing order
                                                ! H5_ITER_DEC_F - Decreasing order
                                                ! H5_ITER_NATIVE_F - Fastest available order
    INTEGER(HSIZE_T), INTENT(INOUT) :: idx  ! IN : Iteration position at which to start
                                            ! OUT: Position at which an interrupted iteration may be restarted

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

    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: lapl_id ! Link access property list

    INTEGER(HID_T) :: lapl_id_default
    INTEGER(SIZE_T) :: namelen

    INTERFACE
       INTEGER FUNCTION h5literate_by_name_c(loc_id, name, namelen, index_type, order, idx, op, op_data, lapl_id_default)
         USE ISO_C_BINDING
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5LITERATE_BY_NAME_C'::h5literate_by_name_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: loc_id
         CHARACTER(LEN=*) :: name
         INTEGER(SIZE_T) :: namelen
         INTEGER, INTENT(IN) :: index_type
         INTEGER, INTENT(IN) :: order
         INTEGER(HSIZE_T), INTENT(INOUT) :: idx
         TYPE(C_FUNPTR), VALUE :: op
         TYPE(C_PTR), VALUE :: op_data
         INTEGER(HID_T) :: lapl_id_default
       END FUNCTION h5literate_by_name_c
    END INTERFACE

    namelen  = LEN(group_name)

    lapl_id_default = H5P_DEFAULT_F
    IF(PRESENT(lapl_id)) lapl_id_default = lapl_id
    
    return_value = h5literate_by_name_c(loc_id, group_name, namelen, index_type, order, idx, op, op_data,lapl_id_default)

    IF(return_value.GE.0)THEN
       hdferr = 0
    ELSE
       hdferr = -1
    END IF

  END SUBROUTINE h5literate_by_name_f

END MODULE H5L_PROVISIONAL
