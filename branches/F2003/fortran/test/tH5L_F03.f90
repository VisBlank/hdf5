!****h* root/fortran/test/tF2003.f90
!
! NAME
!  tF2003.f90
!
! FUNCTION
!  Test FORTRAN HDF5 APIs which are dependent on FORTRAN 2003
!  features. Tests H5L, H5P, H5T APIs. 
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
! USES
!  liter_cb_mod, test_genprop_cls_cb1_mod
!
! CONTAINS SUBROUTINES
!  test_iter_group, test_create, test_genprop_class_callback,
!  test_array_compound_atomic, test_array_compound_array,
!  test_array_bkg 
!
!*****

! *****************************************
! ***        H 5 L   T E S T S
! *****************************************
MODULE liter_cb_mod

  USE HDF5
  USE ISO_C_BINDING
  IMPLICIT NONE
    
  TYPE iter_enum 
     INTEGER RET_ZERO
     INTEGER RET_TWO
     INTEGER RET_CHANGE
     INTEGER RET_CHANGE2
  END TYPE iter_enum

  !/* Custom group iteration callback data */
  TYPE, bind(c) ::  iter_info
     CHARACTER(LEN=1), DIMENSION(1:10) :: name ! /* The name of the object */
     INTEGER(c_int) :: TYPE    ! /* The TYPE of the object */
     INTEGER(c_int) :: command !/* The TYPE of RETURN value */
  END TYPE iter_info

  TYPE, bind(c) :: union_t
     INTEGER(haddr_t) :: address
     INTEGER(size_t) :: val_size
  END TYPE union_t

  TYPE, bind(c) :: H5L_info_t
     INTEGER(c_int) :: TYPE ! H5L_type_t     type
!       LOGICAL(c_bool) :: corder_valid ! hbool_t        corder_valid
     INTEGER(c_int64_t) :: corder ! int64_t        corder;
     INTEGER(c_int) :: cset ! H5T_cset_t     cset;
     TYPE(union_t) :: u
  END TYPE H5L_info_t

CONTAINS

!/****************************************************************
!**
!**  liter_cb(): Custom link iteration callback routine.
!**
!****************************************************************/

  INTEGER FUNCTION liter_cb(group, name, link_info, op_data) bind(C)

    USE HDF5
    USE ISO_C_BINDING
    IMPLICIT NONE

    INTEGER(HID_T), VALUE :: group
    CHARACTER(LEN=1), DIMENSION(1:10) :: name


    TYPE (H5L_info_t) :: link_info

    TYPE(iter_info) :: op_data

    INTEGER, SAVE :: count
    INTEGER, SAVE :: count2 

!!$    
!!$    iter_info *info = (iter_info *)op_data;
!!$    static int count = 0;
!!$    static int count2 = 0;

    op_data%name(1:10) = name(1:10)

    SELECT CASE (op_data%command)

    CASE(0)
       liter_cb = 0
    CASE(2)
       liter_cb = 2
    CASE(3)
       count = count + 1
       IF(count.GT.10) THEN
          liter_cb = 1
       ELSE
          liter_cb = 0
       ENDIF
    CASE(4)
       count2 = count2 + 1
       IF(count2.GT.10) THEN
          liter_cb = 1
       ELSE
          liter_cb = 0
       ENDIF
    END SELECT

  END FUNCTION liter_cb
END MODULE liter_cb_mod
