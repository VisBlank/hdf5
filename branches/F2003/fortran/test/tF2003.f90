!****h* root/fortran/test/tF2003.f90
!
! NAME
!  tF2003.f90
!
! FUNCTION
!  Test FORTRAN HDF5 APIs which are dependent on the FORTRAN 2003
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


! *****************************************
! ***        H 5 E   T E S T S
! *****************************************

MODULE test_my_hdf5_error_handler


  IMPLICIT NONE

CONTAINS

!/****************************************************************
!**
!**  my_hdf5_error_handler: Custom error callback routine.
!**
!****************************************************************/

    INTEGER FUNCTION my_hdf5_error_handler(estack_id, data_inout) bind(C)

    ! This error function handle works with only version 2 error stack

    USE HDF5
    USE ISO_C_BINDING
    IMPLICIT NONE

    ! estack_id is always passed from C as: H5E_DEFAULT
    INTEGER(HID_T) :: estack_id 
    ! data that was registered with H5Eset_auto_f
!    INTEGER, DIMENSION(1:2) :: data_inout
    INTEGER :: data_inout

    PRINT*, " "
    PRINT*, " Subtest: H5Eset_auto_f custom error message with callback, WITH DATA"
    PRINT*, "         -This message should be written to standard out-  "
    PRINT*, "          Data Values Passed In =", data_inout
    PRINT*, " "

    data_inout = 10*data_inout

    my_hdf5_error_handler = 1 ! this is not used by the C routine

  END FUNCTION my_hdf5_error_handler

  INTEGER FUNCTION my_hdf5_error_handler_nodata(estack_id, data_inout) bind(C)

    ! This error function handle works with only version 2 error stack

    USE HDF5
    USE ISO_C_BINDING
    IMPLICIT NONE

    ! estack_id is always passed from C as: H5E_DEFAULT
    INTEGER(HID_T) :: estack_id 
    ! data that was registered with H5Eset_auto_f
    TYPE(C_PTR) :: data_inout
    
    PRINT*, " "
    PRINT*, " Subtest: H5Eset_auto_f custom error message with callback, NO DATA"
    PRINT*, "         -This message should be written to standard out-  "
    PRINT*, " "

    my_hdf5_error_handler_nodata = 1 ! this is not used by the C routine

  END FUNCTION my_hdf5_error_handler_nodata
  
END MODULE test_my_hdf5_error_handler
          
          
!!$       
!!$
!!$    
!!$
!!$    switch(info->command) {
!!$        case RET_ZERO:
!!$            return(0);
!!$
!!$        case RET_TWO:
!!$            return(2);
!!$
!!$        case RET_CHANGE:
!!$            count++;
!!$            return(count > 10 ? 1 : 0);
!!$
!!$        case RET_CHANGE2:
!!$            count2++;
!!$            return(count2 > 10 ? 1 : 0);
!!$
!!$        default:
!!$            printf("invalid iteration command");
!!$            return(-1);
!!$    } /* end switch */
!!$} /* end liter_cb() */


!/****************************************************************
!**
!**  test_iter_group(): Test group iteration functionality
!**
!****************************************************************/
SUBROUTINE test_iter_group(total_error)

    USE HDF5 
    USE ISO_C_BINDING
    USE liter_cb_mod
    IMPLICIT NONE
    
    INTEGER, INTENT(INOUT) :: total_error
    INTEGER(HID_T) :: fapl
    INTEGER(HID_T) :: file ! /* File ID */
    INTEGER(hid_t) :: dataset          !/* Dataset ID */
    INTEGER(hid_t) :: datatype         !/* Common datatype ID */
    INTEGER(hid_t) :: filespace        !/* Common dataspace ID */
    INTEGER(hid_t) :: root_group,grp   !/* Root group ID */
    INTEGER i,j                  !/* counting variable */
    INTEGER(hsize_t) idx            !/* Index in the group */
    CHARACTER(LEN=11) :: DATAFILE = "titerate.h5"
    INTEGER, PARAMETER :: ndatasets = 50
    CHARACTER(LEN=10) :: name ! /* temporary name buffer */
    CHARACTER(LEN=10), DIMENSION(1:ndatasets+2) :: lnames ! /* Names of the links created */
!!$    char dataset_name[NAMELEN];  /* dataset name */

    TYPE(iter_info), TARGET :: info

!!$    iter_info info;         /* Custom iteration information */
!!$    H5G_info_t ginfo;       /* Buffer for querying object's info */
!!$    herr_t ret;		    /* Generic return value */

    INTEGER :: error
    INTEGER :: ret_value
    TYPE(C_PTR) :: f_ptr
    TYPE(C_FUNPTR) :: f1
    TYPE(C_PTR) :: f2
    CHARACTER(LEN=2) :: ichr2
    CHARACTER(LEN=10) :: ichr10

    !/* Get the default FAPL */
    CALL H5Pcreate_f(H5P_FILE_ACCESS_F, fapl, error)
    CALL check("h5pcreate_f", error, total_error)

    !/* Set the "use the latest version of the format" bounds for creating objects in the file */
    CALL H5Pset_libver_bounds_f(fapl, H5F_LIBVER_LATEST_F, H5F_LIBVER_LATEST_F, error)
    CALL check("H5Pset_libver_bounds_f",error, total_error)

    !/* Create the test file with the datasets */
    CALL h5fcreate_f(DATAFILE, H5F_ACC_TRUNC_F, file, error, H5P_DEFAULT_F, fapl)
    CALL check("h5fcreate_f", error, total_error)

    !/* Test iterating over empty group */
!    info.command = RET_ZERO;
    
!    info%command%RET_ZERO = 0
    idx = 0
    info%command = 0
    f1 = C_FUNLOC(liter_cb)
    f2 = C_LOC(info)


    CALL H5Literate_f(file, H5_INDEX_NAME_F, H5_ITER_INC_F, idx, f1, f2, ret_value, error)
    CALL check("H5Literate_f", error, total_error)

    CALL H5Tcopy_f(H5T_NATIVE_INTEGER, datatype, error)
    CALL check("H5Tcopy_f", error, total_error)

    CALL H5Screate_f(H5S_SCALAR_F, filespace, error)
    CALL check("H5Screate_f", error, total_error)

    DO i = 1, ndatasets
       WRITE(ichr2, '(I2.2)') i

       name = 'Dataset '//ichr2

       CALL h5dcreate_f(file, name, datatype, filespace, dataset, error)
       CALL check("H5dcreate_f", error, total_error)

       lnames(i) = name

       CALL h5dclose_f(dataset,error)
       CALL check("H5dclose_f", error, total_error)

    ENDDO

    ! /* Create a group and named datatype under root group for testing */

    CALL H5Gcreate_f(file, "grp0000000", grp, error)
    CALL check("H5Gcreate_f", error, total_error)

    lnames(ndatasets+2) = "grp0000000" 

!!$
!!$    lnames[NDATASETS] = HDstrdup("grp");
!!$    CHECK(lnames[NDATASETS], NULL, "strdup");
!!$
    
    CALL H5Tcommit_f(file, "dtype00000", datatype, error)
    CALL check("H5Tcommit_f", error, total_error)

    lnames(ndatasets+1) = "dtype00000" 

    ! /* Close everything up */

    CALL H5Tclose_f(datatype, error)
    CALL check("H5Tclose_f", error, total_error)

    CALL H5Gclose_f(grp, error)
    CALL check("H5Gclose_f", error, total_error)
    
    CALL H5Sclose_f(filespace, error)
    CALL check("H5Sclose_f", error, total_error)

    CALL H5Fclose_f(file, error)
    CALL check("H5Fclose_f", error, total_error)

    ! /* Iterate through the datasets in the root group in various ways */
    CALL H5Fopen_f(DATAFILE, H5F_ACC_RDONLY_F, file, error, access_prp=fapl)
    CALL check("h5fopen_f", error, total_error)

    !/* Test all objects in group, when callback always returns 0 */
    info%command = 0
    idx = 0
    CALL H5Literate_f(file, H5_INDEX_NAME_F, H5_ITER_INC_F, idx, f1, f2, ret_value, error)
    IF(ret_value.GT.0)THEN
       PRINT*,"ERROR: Group iteration function didn't return zero correctly!"
       CALL verify("H5Literate_f", error, -1, total_error)
    ENDIF

    !  /* Test all objects in group, when callback always returns 1 */
    !  /* This also tests the "restarting" ability, because the index changes */
    
    info%command = 2
    idx = 0
    i = 0
    f1 = C_FUNLOC(liter_cb)
    f2 = C_LOC(info)
    DO 
       CALL H5Literate_f(file, H5_INDEX_NAME_F, H5_ITER_INC_F, idx, f1, f2, ret_value, error)
       IF(error.LT.0) EXIT
       ! /* Verify return value from iterator gets propagated correctly */
       CALL VERIFY("H5Literate", ret_value, 2, total_error)
       ! /* Increment the number of times "2" is returned */
       i = i + 1
       !/* Verify that the index is the correct value */
       CALL VERIFY("H5Literate", INT(idx), INT(i), total_error)
       IF(idx .GT.ndatasets+2)THEN
          PRINT*,"ERROR: Group iteration function walked too far!"
       ENDIF
       
 !/* Verify the correct name is retrieved */
       DO j = 1, 10
          ichr10(j:j) = info%name(j)(1:1)
       ENDDO
       CALL verifystring("H5Literate_f", ichr10, lnames(INT(idx)), total_error)
       IF(i.EQ.52)EXIT ! prints out error message otherwise (for gcc/gfortran/g95) not intel (why) -FIX- scot
    END DO
    
    ! put check if did not walk far enough -scot FIX

    IF(i .NE. (NDATASETS + 2)) THEN
       CALL VERIFY("H5Literate_f", i, INT(NDATASETS + 2), total_error)
       PRINT*,"ERROR: Group iteration function didn't perform multiple iterations correctly"
    ENDIF

    !/* Test all objects in group, when callback changes return value */
    !/* This also tests the "restarting" ability, because the index changes */

    info%command = 3
    idx = 0
    i = 0
 
    f1 = C_FUNLOC(liter_cb)
    f2 = C_LOC(info)
    DO

       CALL H5Literate_f(file, H5_INDEX_NAME_F, H5_ITER_INC_F, idx, f1, f2, ret_value, error)
       IF(error.LT.0) EXIT
       CALL VERIFY("H5Literate_f", ret_value, 1, total_error)

       !/* Increment the number of times "1" is returned */
       i = i + 1

       !/* Verify that the index is the correct value */
       CALL VERIFY("H5Literate_f", INT(idx), INT(i+10), total_error)

       IF(idx .GT.ndatasets+2)THEN
          PRINT*,"Group iteration function walked too far!"
       ENDIF

       DO j = 1, 10
          ichr10(j:j) = info%name(j)(1:1)
       ENDDO
       !/* Verify that the correct name is retrieved */
       CALL verifystring("H5Literate_f", ichr10, lnames(INT(idx)), total_error)
       IF(i.EQ.42)EXIT ! prints out error message otherwise (for gcc/gfortran/g95) not intel (why) -FIX- scot
    ENDDO

    IF(i .NE. 42 .OR. idx .NE. 52)THEN
       PRINT*,"ERROR: Group iteration function didn't perform multiple iterations correctly!"
       CALL check("H5Literate_f",-1,total_error)
    ENDIF

    CALL H5Fclose_f(file, error)
    CALL check("H5Fclose_f", error, total_error)
       
  END SUBROUTINE test_iter_group

! *****************************************
! ***        H 5 P   T E S T S
! *****************************************

!/*-------------------------------------------------------------------------
! * Function:	test_create
! *
! * Purpose:	Tests H5Pset_fill_value_f and H5Pget_fill_value_f
! *
! * Return:	Success:	0
! *
! *		Failure:	number of errors
! *
! * Programmer:	M.S. Breitenfeld
! *             June 24, 2008
! *
! * Modifications:
! *
! *-------------------------------------------------------------------------
! */
  SUBROUTINE test_create(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE

    INTEGER, INTENT(INOUT) :: total_error
    INTEGER(HID_T) :: fapl

    INTEGER(hid_t) :: file=-1, space=-1, dcpl=-1, comp_type_id=-1
    INTEGER(hid_t) :: dset1=-1, dset2=-1, dset3=-1, dset4=-1, dset5=-1, &
         dset6=-1, dset7=-1, dset8=-1, dset9=-1
    INTEGER(hsize_t), DIMENSION(1:5), PARAMETER :: cur_size = (/2, 8, 8, 4, 2/)
    INTEGER(hsize_t), DIMENSION(1:5), PARAMETER :: ch_size= (/1, 1, 1, 4, 1/)
    CHARACTER(LEN=14) :: filename ='test_create.h5'

    ! /* compound datatype operations */
    TYPE, BIND(C) :: comp_datatype
       REAL :: a
       INTEGER :: x
       DOUBLE PRECISION :: y
       CHARACTER(LEN=1) :: z
    END TYPE comp_datatype

    TYPE(comp_datatype), TARGET :: rd_c, fill_ctype

    INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
    INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype 
    INTEGER(SIZE_T) :: type_sized  ! Size of the double datatype 
    INTEGER(SIZE_T) :: type_sizec  ! Size of the double datatype
    INTEGER(SIZE_T) :: sizeof_compound ! total size of compound
    INTEGER :: error
    INTEGER(SIZE_T) :: h5off
    TYPE(C_PTR) :: f_ptr

    !/*
    ! * Create a file.
    ! */
    CALL h5fcreate_f(filename,H5F_ACC_TRUNC_F,file,error)
    CALL check("h5fcreate_f", error, total_error)   
    
    CALL h5screate_simple_f(5, cur_size, space, error, cur_size)
    CALL check("h5screate_simple_f", error, total_error)
    
    CALL H5Pcreate_f(H5P_DATASET_CREATE_F, dcpl, error)
    CALL check("H5Pcreate_f", error, total_error)
    
    CALL h5pset_chunk_f(dcpl, 5, ch_size, error)
    CALL check("h5pset_chunk_f",error, total_error)

  ! /* Create a compound datatype */

    CALL h5tcreate_f(H5T_COMPOUND_F, INT(SIZEOF(fill_ctype),size_t), comp_type_id, error)
    CALL check("h5tcreate_f", error, total_error)
    h5off = H5OFFSETOF(C_LOC(fill_ctype), C_LOC(fill_ctype%a))
    CALL h5tinsert_f(comp_type_id, "a", h5off , H5T_NATIVE_REAL, error)
    CALL check("h5tinsert_f", error, total_error)
    CALL h5tinsert_f(comp_type_id, "x", H5OFFSETOF(C_LOC(fill_ctype), C_LOC(fill_ctype%x)), H5T_NATIVE_INTEGER, error)
    CALL check("h5tinsert_f", error, total_error)
    CALL h5tinsert_f(comp_type_id, "y", H5OFFSETOF(C_LOC(fill_ctype), C_LOC(fill_ctype%y)), H5T_NATIVE_DOUBLE, error)
    CALL check("h5tinsert_f", error, total_error)
    CALL h5tinsert_f(comp_type_id, "z", H5OFFSETOF(C_LOC(fill_ctype), C_LOC(fill_ctype%z)), H5T_NATIVE_CHARACTER, error)
    CALL check("h5tinsert_f", error, total_error)


    CALL H5Pset_alloc_time_f(dcpl, H5D_ALLOC_TIME_LATE_F,error)
    CALL check("H5Pset_alloc_time_f",error, total_error)

    CALL H5Pset_fill_time_f(dcpl, H5D_FILL_TIME_ALLOC_F, error)
    CALL check("H5Pset_fill_time_f",error, total_error)
  
  ! /* Compound datatype test */

    f_ptr = C_LOC(fill_ctype)

    CALL H5Pget_fill_value_f(dcpl, comp_type_id, f_ptr, error)
    CALL check("H5Pget_fill_value_f",error, total_error)

    fill_ctype%y = 4444.
    fill_ctype%z = 'S'
    fill_ctype%a = 5555.
    fill_ctype%x = 55

    f_ptr = C_LOC(fill_ctype)

    CALL H5Pset_fill_value_f(dcpl, comp_type_id, f_ptr, error)
    CALL check("H5Pget_fill_value_f",error, total_error)
    
    CALL h5dcreate_f(file,"dset9", comp_type_id, space, dset9, error, dcpl_id=dcpl)
    CALL check("h5dcreate_f", error, total_error)
 
    CALL h5dclose_f(dset9, error)
    CALL check("h5dclose_f", error, total_error)

    CALL h5fclose_f(file,error)
    CALL check("h5fclose_f", error, total_error)

    ! /* Open the file and get the dataset fill value from each dataset */
    CALL H5Pcreate_f(H5P_FILE_ACCESS_F, fapl, error)
    CALL check("H5Pcreate_f",error, total_error)

    CALL H5Pset_libver_bounds_f(fapl, H5F_LIBVER_LATEST_F, H5F_LIBVER_LATEST_F, error)
    CALL check("H5Pset_libver_bounds_f",error, total_error)

    CALL h5fopen_f (FILENAME, H5F_ACC_RDONLY_F, file, error, fapl)
    CALL check("h5fopen_f", error, total_error)

    !/* Compound datatype test */
    CALL h5dopen_f(file, "dset9", dset9, error)
    CALL check("h5dopen_f", error, total_error)

    CALL H5Dget_create_plist_f(dset9, dcpl, error)
    CALL check("H5Dget_create_plist_f", error, total_error)
    
    f_ptr = C_LOC(rd_c)

    CALL H5Pget_fill_value_f(dcpl, comp_type_id, f_ptr, error)
    CALL check("H5Pget_fill_value_f", error, total_error)

    IF( rd_c%a .NE. fill_ctype%a .OR. &
        rd_c%y .NE. fill_ctype%y .OR. &
        rd_c%x .NE. fill_ctype%x .OR. &
        rd_c%z .NE. fill_ctype%z )THEN

       PRINT*,"***ERROR: Returned wrong fill value"
       total_error = total_error + 1

    ENDIF

    CALL h5dclose_f(dset9, error)
    CALL check("h5dclose_f", error, total_error)

    CALL H5Pclose_f(dcpl, error)
    CALL check("H5Pclose_f", error, total_error)

    CALL h5fclose_f(file,error)
    CALL check("h5fclose_f", error, total_error)

  END SUBROUTINE test_create

  MODULE test_genprop_cls_cb1_mod

! Callback subroutine for test_genprop_class_callback
! and the function H5Pcreate_class_f.

    USE HDF5
    USE ISO_C_BINDING
    IMPLICIT NONE

    TYPE, bind(C) :: cop_cb_struct_ ! /* Struct for iterations */
       INTEGER :: count
       INTEGER(HID_T) :: id
    END TYPE cop_cb_struct_

  CONTAINS

    INTEGER FUNCTION test_genprop_cls_cb1_f(list_id, create_data ) bind(C)

      USE HDF5
      USE ISO_C_BINDING
      IMPLICIT NONE
      
      INTEGER(HID_T), INTENT(IN), VALUE :: list_id
      
      TYPE(cop_cb_struct_) :: create_data
      
      create_data%count = create_data%count + 1
      create_data%id = list_id
      
      test_genprop_cls_cb1_f = 0
      
    END FUNCTION test_genprop_cls_cb1_f

  END MODULE test_genprop_cls_cb1_mod

  SUBROUTINE test_genprop_class_callback(total_error)

!/****************************************************************
!**
!**  test_genprop_class_callback(): Test basic generic property list code.
!**      Tests callbacks for property lists in a generic class.
!**
!**  FORTRAN TESTS:
!**      Tests function H5Pcreate_class_f with callback.
!**
!****************************************************************/

    USE HDF5
    USE ISO_C_BINDING
    USE test_genprop_cls_cb1_mod
    IMPLICIT NONE

    INTEGER, INTENT(INOUT) :: total_error
    
    INTEGER(hid_t) :: cid1 !/* Generic Property class ID */
    INTEGER(hid_t) :: lid1 !/* Generic Property list ID */
    INTEGER(hid_t) :: lid2 !/* 2nd Generic Property list ID */
    INTEGER(size_t) :: nprops !/* Number of properties in class */

    TYPE cb_struct
       INTEGER :: count
       INTEGER(hid_t) :: id
    END TYPE cb_struct

    TYPE(cb_struct), TARGET :: crt_cb_struct, cls_cb_struct

    CHARACTER(LEN=7) :: CLASS1_NAME = "Class 1"
    TYPE(C_FUNPTR) :: f1, f3, f5
    TYPE(C_PTR) :: f2, f4, f6

    CHARACTER(LEN=10) :: PROP1_NAME = "Property 1"
    INTEGER(SIZE_T) :: PROP1_SIZE = 10
    CHARACTER(LEN=10) :: PROP2_NAME = "Property 2"
    INTEGER(SIZE_T) :: PROP2_SIZE = 10
    CHARACTER(LEN=10) :: PROP3_NAME = "Property 3"
    INTEGER(SIZE_T) :: PROP3_SIZE = 10
    CHARACTER(LEN=10) :: PROP4_NAME = "Property 4"
    INTEGER(SIZE_T) :: PROP4_SIZE = 10
    INTEGER :: PROP1_DEF_VALUE = 10
    INTEGER :: PROP2_DEF_VALUE = 10
    INTEGER :: PROP3_DEF_VALUE = 10
    INTEGER :: PROP4_DEF_VALUE = 10

    INTEGER :: error ! /* Generic RETURN value	*/

    f1 = C_FUNLOC(test_genprop_cls_cb1_f)
    f5 = C_FUNLOC(test_genprop_cls_cb1_f)

    f2 = C_LOC(crt_cb_struct)
    f6 = C_LOC(cls_cb_struct)

    !/* Create a new generic class, derived from the root of the class hierarchy */
    CALL H5Pcreate_class_f(H5P_ROOT_F,CLASS1_NAME, cid1, error, f1, f2, c_null_funptr, c_null_ptr, f5, f6)
    CALL check("H5Pcreate_class_f", error, total_error)

    !/* Insert first property into class (with no callbacks) */
    CALL H5Pregister_f(cid1, PROP1_NAME, PROP1_SIZE, PROP1_DEF_VALUE, error)
    CALL check("H5Pregister_f", error, total_error)
    !/* Insert second property into class (with no callbacks) */
    CALL H5Pregister_f(cid1, PROP2_NAME, PROP2_SIZE, PROP2_DEF_VALUE, error)
    CALL check("H5Pregister_f", error, total_error)
    !/* Insert third property into class (with no callbacks) */
    CALL H5Pregister_f(cid1, PROP3_NAME, PROP3_SIZE, PROP3_DEF_VALUE, error)
    CALL check("H5Pregister_f", error, total_error)

    !/* Insert fourth property into class (with no callbacks) */
    CALL H5Pregister_f(cid1, PROP4_NAME, PROP4_SIZE, PROP4_DEF_VALUE, error)
    CALL check("H5Pregister_f", error, total_error)

    ! /* Check the number of properties in class */
    CALL H5Pget_nprops_f(cid1, nprops, error)
    CALL check("H5Pget_nprops_f", error, total_error)
    CALL VERIFY("H5Pget_nprops_f", INT(nprops), 4, total_error)

    ! /* Initialize class callback structs */

    crt_cb_struct%count = 0
    crt_cb_struct%id    = -1
    cls_cb_struct%count = 0
    cls_cb_struct%id    = -1

    !/* Create a property list from the class */
    CALL H5Pcreate_f(cid1, lid1, error)
    CALL check("H5Pcreate", error, total_error)

    !/* Verify that the creation callback occurred */
    CALL VERIFY("H5Pcreate", INT(crt_cb_struct%count), 1, total_error)
    CALL VERIFY("H5Pcreate", INT(crt_cb_struct%id), INT(lid1), total_error)

    ! /* Check the number of properties in list */
    CALL H5Pget_nprops_f(lid1,nprops, error)
    CALL check("H5Pget_nprops_f", error, total_error)
    CALL VERIFY("H5Pget_nprops_f", INT(nprops), 4, total_error)

    ! /* Create another property list from the class */
    CALL H5Pcreate_f(cid1, lid2, error)
    CALL check("H5Pcreate", error, total_error)

    ! /* Verify that the creation callback occurred */
    CALL VERIFY("H5Pcreate", INT(crt_cb_struct%count), 2, total_error)
    CALL VERIFY("H5Pcreate", INT(crt_cb_struct%id), INT(lid2), total_error)

    ! /* Check the number of properties in list */
    CALL H5Pget_nprops_f(lid2,nprops, error)
    CALL check("H5Pget_nprops_f", error, total_error)
    CALL VERIFY("H5Pget_nprops_f", INT(nprops), 4, total_error)

    ! /* Close first list */
    CALL H5Pclose_f(lid1, error);
    CALL check("h5pclose", error, total_error)

    !/* Verify that the close callback occurred */
    CALL VERIFY("H5Pcreate", INT(cls_cb_struct%count), 1, total_error)
    CALL VERIFY("H5Pcreate", INT(cls_cb_struct%id), INT(lid1), total_error)

    !/* Close second list */
    CALL H5Pclose_f(lid2, error);
    CALL check("h5pclose", error, total_error)

    !/* Verify that the close callback occurred */
    CALL VERIFY("H5Pcreate", INT(cls_cb_struct%count), 2, total_error)
    CALL VERIFY("H5Pcreate", INT(cls_cb_struct%id), INT(lid2), total_error)

    !/* Close class */
    CALL H5Pclose_class_f(cid1, error)
    CALL check("H5Pclose_class_f", error, total_error)

  END SUBROUTINE test_genprop_class_callback




! *****************************************
! ***        H 5 T   T E S T S
! *****************************************

!/****************************************************************
!**
!**  test_array_compound_atomic(): Test basic array datatype code.
!**  Tests 1-D array of compound datatypes (with no array fields)
!**
!****************************************************************/
!
  SUBROUTINE test_array_compound_atomic(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE
    
    INTEGER, INTENT(INOUT) :: total_error
    ! /* 1-D dataset WITH fixed dimensions */
    CHARACTER(LEN=6), PARAMETER :: SPACE1_NAME = "Space1"
    INTEGER, PARAMETER :: SPACE1_RANK = 1
    INTEGER, PARAMETER :: SPACE1_DIM1 = 4
    ! /* 1-D array datatype */
    INTEGER, PARAMETER :: ARRAY1_RANK= 1
    INTEGER, PARAMETER :: ARRAY1_DIM1= 4
    CHARACTER(LEN=10), PARAMETER :: FILENAME = "tarray1.h5"

    TYPE s1_t
       INTEGER :: i
       REAL :: f
    END TYPE s1_t
    TYPE(s1_t), DIMENSION(:,:), ALLOCATABLE, TARGET :: wdata ! /* Information to write */
    TYPE(s1_t), DIMENSION(:,:), ALLOCATABLE, TARGET :: rdata ! /* Information read in */
    INTEGER(hid_t)	:: fid1	!/* HDF5 File IDs		*/
    INTEGER(hid_t) :: dataset	!/* Dataset ID			*/
    INTEGER(hid_t)	:: sid1       !/* Dataspace ID			*/
    INTEGER(hid_t)	:: tid1       !/* Array Datatype ID			*/
    INTEGER(hid_t)	:: tid2       !/* Compound Datatype ID			*/
    
    INTEGER(HSIZE_T), DIMENSION(1) :: sdims1 = (/SPACE1_DIM1/)
    INTEGER(HSIZE_T), DIMENSION(1) :: tdims1=(/ARRAY1_DIM1/)
    INTEGER :: ndims ! /* Array rank for reading */
    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims1 !/* Array dimensions for reading */
    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims !/* Array dimensions for reading */
    INTEGER :: nmemb !/* Number of compound members */
    CHARACTER(LEN=20) :: mname !/* Name of compound field */
    INTEGER(size_t) :: off   !/* Offset of compound field */
    INTEGER(hid_t) :: mtid   !/* Datatype ID for field */
    INTEGER :: i,j      ! /* counting variables */ 
    INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
    INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype
    
    INTEGER(SIZE_T) :: sizeof_compound ! total size of compound
    INTEGER :: error    ! /* Generic RETURN value */
    INTEGER(SIZE_T)     ::   offset     ! Member's offset
    INTEGER :: namelen
    LOGICAL :: flag

    TYPE(C_PTR) :: f_ptr ! Needed to pass the pointer, for g95 compiler to work

    ALLOCATE( wdata(1:SPACE1_DIM1,1:ARRAY1_DIM1) )
    ALLOCATE( rdata(1:SPACE1_DIM1,1:ARRAY1_DIM1) )
      
    !/* Initialize array data to write */
    DO i = 1, SPACE1_DIM1
       DO j = 1, ARRAY1_DIM1
          wdata(i,j)%i = i * 10 + j
          wdata(i,j)%f = i * 2.5 + j
       ENDDO
    ENDDO

    !/* Create file */
    CALL h5fcreate_f(FILENAME,H5F_ACC_TRUNC_F,fid1,error)
    CALL check("h5fcreate_f", error, total_error)    

    !/* Create dataspace for datasets */
    CALL h5screate_simple_f(SPACE1_RANK, sdims1, sid1, error)
    CALL check("h5screate_simple_f", error, total_error)

    CALL h5tcreate_f(H5T_COMPOUND_F, H5OFFSETOF(C_LOC(wdata(1,1)), C_LOC(wdata(2,1))), tid2, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Insert integer field */
    CALL h5tinsert_f(tid2, "i", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%i)), H5T_NATIVE_INTEGER, error)
    CALL check("h5tinsert_f", error, total_error)
     
    !/* Insert float field */

    CALL h5tinsert_f(tid2, "f", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), H5T_NATIVE_REAL, error)
    CALL check("h5tinsert_f", error, total_error)

    ! /* Create an array datatype to refer to */
    CALL h5tarray_create_f(tid2, ARRAY1_RANK, tdims1, tid1, error)
    CALL check("h5tarray_create_f", error, total_error)

    !/* Close compound datatype */
    CALL h5tclose_f(tid2,error)
    CALL check("h5tclose_f", error, total_error)


    !/* Create a dataset */
    CALL h5dcreate_f(fid1,"Dataset1",tid1, sid1, dataset,error)
    CALL check("h5dcreate_f", error, total_error)

    !/* Write dataset to disk */

    ALLOCATE(rdims(1:2)) ! dummy not needed

    f_ptr = C_LOC(wdata(1,1))
    CALL h5dwrite_f(dataset, tid1, f_ptr, error )
    CALL check("h5dwrite_f", error, total_error)
    !/* Close Dataset */ 
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    !/* Close datatype */
    CALL h5tclose_f(tid1,error)
    CALL check("h5tclose_f", error, total_error)

    !/* Close disk dataspace */
    CALL h5sclose_f(sid1,error)
    CALL check("h5sclose_f", error, total_error)

    !/* Close file */
    CALL h5fclose_f(fid1,error)
    CALL check("h5fclose_f", error, total_error)

    !/* Re-open file */
    CALL h5fopen_f (FILENAME, H5F_ACC_RDONLY_F, fid1, error)
    CALL check("h5fopen_f", error, total_error)

    !/* Open the dataset */ 
    CALL h5dopen_f(fid1, "Dataset1", dataset, error)
    CALL check("h5dopen_f", error, total_error)

    !/* Get the datatype */    
    CALL h5dget_type_f(dataset, tid1, error)
    CALL check("h5dget_type_f", error, total_error)

    !/* Check the array rank */
    CALL h5tget_array_ndims_f(tid1, ndims, error)
    CALL check("h5tget_array_ndims_f", error, total_error)
    CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)

    !/* Get the array dimensions */
    ALLOCATE(rdims1(1:ndims))
    CALL h5tget_array_dims_f(tid1, rdims1, error)
    CALL check("h5tget_array_dims_f", error, total_error)


    !/* Check the array dimensions */
    DO i = 1, ndims
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims1(i)), total_error)
    ENDDO

    !/* Get the compound datatype */
    CALL h5tget_super_f(tid1, tid2, error)
    CALL check("h5tget_super_f", error, total_error)

    !/* Check the number of members */
    CALL h5tget_nmembers_f(tid2, nmemb, error)
    CALL check("h5tget_nmembers_f", error, total_error)
    CALL VERIFY("h5tget_nmembers_f", nmemb, 2, total_error)

    !/* Check the 1st field's name */
    CALL H5Tget_member_name_f(tid2, 0, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"i", total_error)

    ! /* Check the 1st field's offset */
    CALL H5Tget_member_offset_f(tid2, 0, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),0, total_error)    

    !/* Check the 1st field's datatype */
    CALL H5Tget_member_type_f(tid2, 0, mtid, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    CALL H5Tequal_f(mtid, H5T_NATIVE_INTEGER, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    CALL h5tclose_f(mtid,error)
    CALL check("h5tclose_f", error, total_error)

    !/* Check the 2nd field's name */
    CALL H5Tget_member_name_f(tid2, 1, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"f", total_error)

    ! /* Check the 2nd field's offset */
    CALL H5Tget_member_offset_f(tid2, 1, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), total_error)    
    
    !/* Check the 2nd field's datatype */
    CALL H5Tget_member_type_f(tid2, 1, mtid, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    CALL H5Tequal_f(mtid, H5T_NATIVE_REAL, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    CALL h5tclose_f(mtid,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close Compound Datatype */
    CALL h5tclose_f(tid2, error)
    CALL check("h5tclose_f", error, total_error)

    !/* Read dataset from disk */

    f_ptr = C_LOC(rdata(1,1))
    CALL H5Dread_f(dataset, tid1, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    !/* Compare data read in */
    DO i = 1, SPACE1_DIM1
       DO j = 1, ARRAY1_DIM1
          IF(wdata(i,j)%i.NE.rdata(i,j)%i)THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF(wdata(i,j)%f.NE.rdata(i,j)%f)THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO

    !/* Close Datatype */
    CALL h5tclose_f(tid1,error)
    CALL check("h5tclose_f", error, total_error)

    !/* Close Dataset */
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    !/* Close file */
    CALL h5fclose_f(fid1,error)
    CALL check("h5fclose_f", error, total_error)

  END SUBROUTINE test_array_compound_atomic
!!$
!!$!/****************************************************************
!!$!**
!!$!**  test_array_compound_array(): Test basic array datatype code.
!!$!**      Tests 1-D array of compound datatypes (with array fields)
!!$!**
!!$!****************************************************************/
!!$
  SUBROUTINE test_array_compound_array(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE
    
    INTEGER, INTENT(INOUT) :: total_error

    ! /* 1-D array datatype */
    INTEGER, PARAMETER :: ARRAY1_RANK= 1
    INTEGER, PARAMETER :: ARRAY1_DIM1= 3
    INTEGER, PARAMETER :: ARRAY2_DIM1= 5

    INTEGER, PARAMETER :: SPACE1_RANK = 1
    INTEGER, PARAMETER :: SPACE1_DIM1 = 4
    CHARACTER(LEN=10), PARAMETER :: FILENAME = "tarray2.h5"

    TYPE st_t_struct ! /* Typedef for compound datatype */
       INTEGER :: i
       REAL, DIMENSION(1:ARRAY2_DIM1) :: f
       CHARACTER(LEN=2), DIMENSION(1:ARRAY2_DIM1) :: c
    END TYPE st_t_struct
    ! /* Information to write */
    TYPE(st_t_struct), DIMENSION(1:SPACE1_DIM1,1:ARRAY1_DIM1), TARGET :: wdata
    ! /* Information read in */
    TYPE(st_t_struct), DIMENSION(1:SPACE1_DIM1,1:ARRAY1_DIM1), TARGET :: rdata


    INTEGER(hid_t) :: fid1		! /* HDF5 File IDs		*/
    INTEGER(hid_t) :: dataset	! /* Dataset ID			*/
    integer(hid_t) :: sid1      ! /* Dataspace ID			*/
    integer(hid_t) :: tid1      ! /* Array Datatype ID	*/
    integer(hid_t) :: tid2      ! /* Compound Datatype ID	*/
    integer(hid_t) :: tid3      ! /* Nested Array Datatype ID	*/
    integer(hid_t) :: tid4      ! /* Nested Array Datatype ID	*/
    INTEGER(HID_T) :: dt5_id      ! Memory datatype identifier 

    INTEGER(HSIZE_T), DIMENSION(1) :: sdims1 = (/SPACE1_DIM1/)
    INTEGER(HSIZE_T), DIMENSION(1) :: tdims1=(/ARRAY1_DIM1/)
    INTEGER(HSIZE_T), DIMENSION(1) :: tdims2=(/ARRAY2_DIM1/)

    INTEGER  ndims      !/* Array rank for reading */

    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims1 !/* Array dimensions for reading */
    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims !/* Array dimensions for reading */

    INTEGER :: nmemb !/* Number of compound members */
    CHARACTER(LEN=20) :: mname !/* Name of compound field */
    INTEGER(size_t) :: off   !/* Offset of compound field */
    INTEGER(size_t) :: offset   !/* Offset of compound field */
    INTEGER(hid_t) :: mtid   !/* Datatype ID for field */ 
    INTEGER(hid_t) :: mtid2   !/* Datatype ID for field */ 
    INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
    INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype 
    INTEGER(SIZE_T) :: type_sizec  ! Size of the character datatype
    INTEGER(SIZE_T) :: sizeof_compound ! total size of compound

    INTEGER :: mclass     ! /* Datatype class for field */
    INTEGER :: i,j,k      !/* counting variables */

    INTEGER :: error
    CHARACTER(LEN=2) :: ichr2
    INTEGER(SIZE_T) :: sizechar
    INTEGER :: namelen
    LOGICAL :: flag 
    INTEGER(HID_T) :: atype_id       !String Attribute Datatype identifier
    INTEGER(SIZE_T) :: attrlen    ! Length of the attribute string 

    TYPE(c_ptr) :: f_ptr

    ! /* Initialize array data to write */
    DO i = 1, SPACE1_DIM1
       DO j = 1, array1_DIM1
          wdata(i,j)%i = i*10+j
          DO k = 1, ARRAY2_DIM1
             wdata(i,j)%f(k) = 10*i+j+.5
             WRITE(ichr2,'(I2.2)') k
             wdata(i,j)%c(k) = ichr2
          ENDDO
       ENDDO
    ENDDO

    ! /* Create file */
    CALL h5fcreate_f(FILENAME,H5F_ACC_TRUNC_F,fid1,error)
    CALL check("h5fcreate_f", error, total_error)   


    ! /* Create dataspace for datasets */
    CALL h5screate_simple_f(SPACE1_RANK, sdims1, sid1, error)
    CALL check("h5screate_simple_f", error, total_error)

    ! /* Create a compound datatype to refer to */
    !
    CALL h5tcreate_f(H5T_COMPOUND_F,  H5OFFSETOF(C_LOC(wdata(1,1)), C_LOC(wdata(2,1))), tid2, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Insert integer field */
    CALL h5tinsert_f(tid2, "i", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%i)), H5T_NATIVE_INTEGER, error)
    CALL check("h5tinsert_f", error, total_error)

    !/* Create an array of floats datatype */
    CALL h5tarray_create_f(H5T_NATIVE_REAL, ARRAY1_RANK, tdims2, tid3, error)
    CALL check("h5tarray_create_f", error, total_error)
    !/* Insert float array field */

    CALL h5tinsert_f(tid2, "f", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), tid3, error)
    CALL check("h5tinsert_f", error, total_error)

    !
    ! Create datatype for the String attribute.
    !
    CALL h5tcopy_f(H5T_NATIVE_CHARACTER, atype_id, error)
    CALL check("h5tcopy_f",error,total_error)
 
    attrlen = LEN(wdata(1,1)%c(1)) 
    CALL h5tset_size_f(atype_id, attrlen, error)
    CALL check("h5tset_size_f",error,total_error)    

    !/* Create an array of character datatype */
    CALL h5tarray_create_f(atype_id, ARRAY1_RANK, tdims2, tid4, error)
    CALL check("h5tarray_create_f", error, total_error)

    !/* Insert character array field */
    CALL h5tinsert_f(tid2, "c", H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%c(1))), tid4, error)
    CALL check("h5tinsert2_f", error, total_error)

    ! /* Close array of floats field datatype */
    CALL h5tclose_f(tid3,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5tclose_f(tid4,error)
    CALL check("h5tclose_f", error, total_error)

    !/* Create an array datatype to refer to */
    CALL h5tarray_create_f(tid2, ARRAY1_RANK, tdims1, tid1, error)
    CALL check("h5tarray_create_f", error, total_error)

    !/* Close compound datatype */
    CALL h5tclose_f(tid2,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Create a dataset */
    CALL h5dcreate_f(fid1,"Dataset1",tid1, sid1, dataset,error)
    CALL check("h5dcreate_f", error, total_error)


    !/* Write dataset to disk */
    f_ptr = C_LOC(wdata(1,1))
    CALL h5dwrite_f(dataset, tid1, f_ptr, error )
    CALL check("h5dwrite_f", error, total_error)

    !/* Close Dataset */
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    ! /* Close datatype */
    CALL h5tclose_f(tid1,error)
    CALL check("h5tclose_f", error, total_error)

    !/* Close disk dataspace */
    CALL h5sclose_f(sid1,error)
    CALL check("h5sclose_f", error, total_error)

    !/* Close file */
    CALL h5fclose_f(fid1,error)
    CALL check("h5fclose_f", error, total_error)

    ! /* Re-open file */
    CALL h5fopen_f (FILENAME, H5F_ACC_RDONLY_F, fid1, error)
    CALL check("h5fopen_f", error, total_error)

    !/* Open the dataset */

    CALL h5dopen_f(fid1, "Dataset1", dataset, error)
    CALL check("h5dopen_f", error, total_error)
    
    ! /* Get the datatype */
    CALL h5dget_type_f(dataset, tid1, error)
    CALL check("h5dget_type_f", error, total_error)

    ! /* Check the array rank */
    CALL h5tget_array_ndims_f(tid1, ndims, error)
    CALL check("h5tget_array_ndims_f", error, total_error)
    CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)


    !/* Get the array dimensions */
    ALLOCATE(rdims1(1:ndims))
    CALL h5tget_array_dims_f(tid1, rdims1, error)
    CALL check("h5tget_array_dims_f", error, total_error)

    ! /* Check the array dimensions */
    DO i = 1, ndims
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims1(i)), total_error)
    ENDDO

    ! /* Get the compound datatype */
    CALL h5tget_super_f(tid1, tid2, error)
    CALL check("h5tget_super_f", error, total_error)

    ! /* Check the number of members */
    CALL h5tget_nmembers_f(tid2, nmemb, error)
    CALL check("h5tget_nmembers_f", error, total_error)
    CALL VERIFY("h5tget_nmembers_f", nmemb, 3, total_error)

    ! /* Check the 1st field's name */
    CALL H5Tget_member_name_f(tid2, 0, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"i", total_error)

    ! /* Check the 1st field's offset */

    CALL H5Tget_member_offset_f(tid2, 0, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),0, total_error) 

    ! /* Check the 1st field's datatype */
    CALL H5Tget_member_type_f(tid2, 0, mtid, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    CALL H5Tequal_f(mtid, H5T_NATIVE_INTEGER, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    CALL h5tclose_f(mtid,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Check the 2nd field's name */
    CALL H5Tget_member_name_f(tid2, 1, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"f", total_error)

    ! /* Check the 2nd field's offset */
    CALL H5Tget_member_offset_f(tid2, 1, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%f)), total_error) 

    ! /* Check the 2nd field's datatype */
    CALL H5Tget_member_type_f(tid2, 1, mtid, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    ! /* Get the 2nd field's class */
    CALL H5Tget_class_f(mtid, mclass, error)
    CALL check("H5Tget_class_f", error, total_error)
    CALL VERIFY("H5Tget_class_f",mclass, H5T_ARRAY_F, total_error)

    ! /* Check the array rank */
    CALL h5tget_array_ndims_f(mtid, ndims, error)
    CALL check("h5tget_array_ndims_f", error, total_error)
    CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)

    ! /* Get the array dimensions */
    CALL h5tget_array_dims_f(mtid, rdims1, error)
    CALL check("h5tget_array_dims_f", error, total_error)

    ! /* Check the array dimensions */
    DO i = 1, ndims
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims2(i)), total_error)
    ENDDO

    ! /* Check the 3rd field's name */
    CALL H5Tget_member_name_f(tid2, 2, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"c", total_error)

    ! /* Check the 3rd field's offset */
    CALL H5Tget_member_offset_f(tid2, 2, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),&
         H5OFFSETOF(C_LOC(wdata(1,1)),C_LOC(wdata(1,1)%c(1))), total_error) 

    ! /* Check the 3rd field's datatype */
    CALL H5Tget_member_type_f(tid2, 2, mtid2, error)
    CALL check("H5Tget_member_type_f", error, total_error)

    ! /* Get the 3rd field's class */
    CALL H5Tget_class_f(mtid2, mclass, error)
    CALL check("H5Tget_class_f", error, total_error)
    CALL VERIFY("H5Tget_class_f",mclass, H5T_ARRAY_F, total_error)

    ! /* Check the array rank */
    CALL h5tget_array_ndims_f(mtid2, ndims, error)
    CALL check("h5tget_array_ndims_f", error, total_error)
    CALL VERIFY("h5tget_array_ndims_f",ndims, ARRAY1_RANK, total_error)

    ! /* Get the array dimensions */
    CALL h5tget_array_dims_f(mtid2, rdims1, error)
    CALL check("h5tget_array_dims_f", error, total_error)

    ! /* Check the array dimensions */
    DO i = 1, ndims
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims2(i)), total_error)
    ENDDO

    ! /* Check the nested array's datatype */
    CALL H5Tget_super_f(mtid, tid3, error)
    CALL check("H5Tget_super_f", error, total_error)

    CALL H5Tequal_f(tid3, H5T_NATIVE_REAL, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    ! /* Check the nested array's datatype */
    CALL H5Tget_super_f(mtid2, tid3, error)
    CALL check("H5Tget_super_f", error, total_error)

    CALL H5Tequal_f(tid3, atype_id, flag, error)
    CALL check("H5Tequal_f", error, total_error) 
    CALL VerifyLogical("H5Tequal_f", flag, .TRUE., total_error)

    ! /* Close the array's base type datatype */
    CALL h5tclose_f(tid3, error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close the member datatype */
    CALL h5tclose_f(mtid,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close the member datatype */
    CALL h5tclose_f(mtid2,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close Compound Datatype */
    CALL h5tclose_f(tid2,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* READ dataset from disk */
    
    f_ptr = c_null_ptr
    f_ptr = C_LOC(rdata(1,1))
    CALL H5Dread_f(dataset, tid1, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    ! /* Compare data read in */
    DO i = 1, SPACE1_DIM1
       DO j = 1, ARRAY1_DIM1
          IF(wdata(i,j)%i.NE.rdata(i,j)%i)THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          DO k = 1, ARRAY2_DIM1
             IF(wdata(i,j)%f(k).NE.rdata(i,j)%f(k))THEN
                PRINT*, 'ERROR: Wrong real array data is read back by H5Dread_f '
                total_error = total_error + 1
             ENDIF
             IF(wdata(i,j)%c(k).NE.rdata(i,j)%c(k))THEN
                PRINT*, 'ERROR: Wrong character array data is read back by H5Dread_f '
                total_error = total_error + 1
             ENDIF
          ENDDO
       ENDDO
    ENDDO

    ! /* Close Datatype */
    CALL h5tclose_f(tid1,error)
    CALL check("h5tclose_f", error, total_error)

    ! /* Close Dataset */
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    ! /* Close file */
    CALL h5fclose_f(fid1,error)
    CALL check("h5fclose_f", error, total_error)
  END SUBROUTINE test_array_compound_array
!!$
!!$!/****************************************************************
!!$!**
!!$!**  test_array_bkg(): Test basic array datatype code.
!!$!**      Tests reading compound datatype with array fields and
!!$!**          writing partial fields.
!!$!**
!!$!****************************************************************/
!!$
  SUBROUTINE test_array_bkg(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE

    INTEGER, PARAMETER :: r_k4 = SELECTED_REAL_KIND(5)
    INTEGER, PARAMETER :: r_k8 = SELECTED_REAL_KIND(10)

    INTEGER, INTENT(INOUT) :: total_error

    INTEGER, PARAMETER :: LENGTH = 5
    INTEGER, PARAMETER :: ALEN = 10
    INTEGER, PARAMETER :: RANK = 1
    INTEGER, PARAMETER :: NMAX = 100
    CHARACTER(LEN=17), PARAMETER :: FIELDNAME = "ArrayofStructures"

    INTEGER(hid_t) :: fid, array_dt
    INTEGER(hid_t) :: space
    INTEGER(hid_t) :: type
    INTEGER(hid_t) :: dataset

    INTEGER(hsize_t), DIMENSION(1:1) :: dim =(/LENGTH/)
    INTEGER(hsize_t), DIMENSION(1:1) :: dima =(/ALEN/)

    INTEGER :: i, j
    INTEGER, DIMENSION(1:3) :: ndims = (/1,1,1/)

    TYPE CmpField_struct
       INTEGER, DIMENSION(1:ALEN) :: a
       REAL(KIND=r_k4), DIMENSION(1:ALEN) :: b
       REAL(KIND=r_k8), DIMENSION(1:ALEN) :: c
    ENDTYPE CmpField_struct

    TYPE(CmpField_struct), DIMENSION(1:LENGTH), TARGET :: cf
    TYPE(CmpField_struct), DIMENSION(1:LENGTH), TARGET :: cfr
    
    TYPE CmpDTSinfo_struct
       INTEGER :: nsubfields
       CHARACTER(LEN=5), DIMENSION(1:nmax) :: name
       INTEGER(size_t), DIMENSION(1:nmax) :: offset
       INTEGER(hid_t), DIMENSION(1:nmax) :: datatype
    END TYPE CmpDTSinfo_struct

    TYPE(CmpDTSinfo_struct) :: dtsinfo

    TYPE fld_t_struct
       REAL(KIND=r_k4), DIMENSION(1:ALEN) :: b
    END TYPE fld_t_struct
 
    INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
    INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype 
    INTEGER(SIZE_T) :: type_sized  ! Size of the double datatype
    INTEGER(SIZE_T) :: sizeof_compound ! total size of compound

    TYPE(fld_t_struct), DIMENSION(1:LENGTH), TARGET :: fld
    TYPE(fld_t_struct), DIMENSION(1:LENGTH), TARGET :: fldr

    CHARACTER(LEN=10), PARAMETER :: FILENAME = "tarray3.h5"

    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims1 !/* Array dimensions for reading */
    INTEGER(HSIZE_T), ALLOCATABLE, DIMENSION(:) :: rdims !/* Array dimensions for reading */

    INTEGER :: error
    TYPE(c_ptr) :: f_ptr
    
    TYPE(c_funptr) :: func

!    /* Initialize the data */
!    /* ------------------- */

    DO i = 1, LENGTH
       DO j = 1, ALEN
          cf(i)%a(j) = 100*(i+1) + j
          cf(i)%b(j) = (100.*(i+1) + 0.01*j)
          cf(i)%c(j) = 100.*(i+1) + 0.02*j
       ENDDO
    ENDDO

    !/* Set the number of data members */
    !/* ------------------------------ */

    dtsinfo%nsubfields = 3

    !/* Initialize the offsets  */
    !/* ----------------------- */
    CALL h5tget_size_f(H5T_NATIVE_INTEGER, type_sizei, error)
    CALL check("h5tget_size_f", error, total_error)
    IF(sizeof(cf(1)%b(1)).EQ.4)THEN
       CALL h5tget_size_f(H5T_NATIVE_REAL_4, type_sizer, error)
       CALL check("h5tget_size_f", error, total_error)
    ELSE IF(sizeof(cf(1)%b(1)).EQ.8)THEN
       CALL h5tget_size_f(H5T_NATIVE_REAL_8, type_sizer, error)
       CALL check("h5tget_size_f", error, total_error)
    ENDIF

    CALL h5tget_size_f(H5T_NATIVE_DOUBLE, type_sized, error)
    CALL check("h5tget_size_f", error, total_error)

    dtsinfo%offset(1)   = H5OFFSETOF(C_LOC(cf(1)),C_LOC(cf(1)%a(1)))
    dtsinfo%offset(2)   = H5OFFSETOF(C_LOC(cf(1)),C_LOC(cf(1)%b(1))) 
    dtsinfo%offset(3)   = H5OFFSETOF(C_LOC(cf(1)),C_LOC(cf(1)%c(1)))


    !/* Initialize the data type IDs */
    !/* ---------------------------- */
    dtsinfo%datatype(1) = H5T_NATIVE_INTEGER;
    dtsinfo%datatype(2) = H5T_NATIVE_REAL_4;
    dtsinfo%datatype(3) = H5T_NATIVE_REAL_8;


    !/* Initialize the names of data members */
    !/* ------------------------------------ */
     
    dtsinfo%name(1) = "One  "
    dtsinfo%name(2) = "Two  "
    dtsinfo%name(3) = "Three"
       
    !/* Create file */
    !/* ----------- */
    CALL h5fcreate_f(FILENAME,H5F_ACC_TRUNC_F,fid,error)
    CALL check("h5fcreate_f", error, total_error)   


    !/* Create data space */
    !/* ----------------- */
    CALL h5screate_simple_f(RANK, dim, space, error)
    CALL check("h5screate_simple_f", error, total_error)


    !/* Create the memory data type */
    !/* --------------------------- */

    CALL h5tcreate_f(H5T_COMPOUND_F, H5OFFSETOF(C_LOC(cf(1)), C_LOC(cf(2))), type, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Add  members to the compound data type */
    !/* -------------------------------------- */

    DO i = 1, dtsinfo%nsubfields
       CALL h5tarray_create_f(dtsinfo%datatype(i), ndims(i), dima, array_dt, error)
       CALL check("h5tarray_create_f", error, total_error)
       CALL H5Tinsert_f(type, dtsinfo%name(i), dtsinfo%offset(i), array_dt, error)
       CALL check("h5tinsert_f", error, total_error)

       CALL h5tclose_f(array_dt,error)
       CALL check("h5tclose_f", error, total_error)
    ENDDO

    !/* Create the dataset */
    !/* ------------------ *//
    CALL h5dcreate_f(fid,FIELDNAME,type, space, dataset,error)
    CALL check("h5dcreate_f", error, total_error)

    !/* Write data to the dataset */
    !/* ------------------------- */

    ALLOCATE(rdims(1:2)) ! dummy not needed

    f_ptr = C_LOC(cf(1))

    CALL h5dwrite_f(dataset, type, f_ptr, error )
    CALL check("h5dwrite_f", error, total_error)


    ALLOCATE(rdims1(1:2)) ! dummy not needed
    f_ptr = C_LOC(cfr(1))
    CALL H5Dread_f(dataset, type, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    !/* Verify correct data */
    !/* ------------------- */
    DO i = 1, LENGTH
       DO j = 1, ALEN
           IF( cf(i)%a(j) .NE. cfr(i)%a(j) )THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%b(j) .NE. cfr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%c(j) .NE. cfr(i)%c(j) )THEN
             PRINT*, 'ERROR: Wrong double data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO


    !/* Release IDs */
    !/* ----------- */
    CALL h5tclose_f(type,error)
    CALL check("h5tclose_f", error, total_error)
    CALL h5sclose_f(space,error)
    CALL check("h5sclose_f", error, total_error)
    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)
    CALL h5fclose_f(fid,error)
    CALL check("h5fclose_f", error, total_error)

    !/******************************/
    !/* Reopen the file and update */
    !/******************************/

    CALL h5fopen_f (FILENAME, H5F_ACC_RDWR_F, fid, error)
    CALL check("h5fopen_f", error, total_error)

    CALL h5dopen_f(fid, FIELDNAME, dataset, error)
    CALL check("h5dopen_f", error, total_error)

    sizeof_compound =  INT( type_sizer*ALEN, size_t)

    CALL h5tcreate_f(H5T_COMPOUND_F, sizeof_compound , type, error)
    CALL check("h5tcreate_f", error, total_error)

    CALL h5tarray_create_f(H5T_NATIVE_REAL_4, 1, dima, array_dt, error)
    CALL check("h5tarray_create_f", error, total_error)

    CALL h5tinsert_f(TYPE, "Two", 0_size_t, array_dt, error)
    CALL check("h5tinsert_f", error, total_error)

    !/* Initialize the data to overwrite */
    !/* -------------------------------- */
    DO i = 1, LENGTH
       DO j = 1, ALEN
          fld(i)%b(j) = 1.313
          cf(i)%b(j) = fld(i)%b(j)
       ENDDO
    ENDDO

    f_ptr = C_LOC(fld(1))

    CALL h5dwrite_f(dataset, TYPE, f_ptr, error )
    CALL check("h5dwrite_f", error, total_error)


    ! /* Read just the field changed */
    
    f_ptr = C_LOC(fldr(1))
    CALL H5Dread_f(dataset, TYPE, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    DO i = 1, LENGTH
       DO j = 1, ALEN
          IF( fld(i)%b(j) .NE. fldr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO
    CALL h5tclose_f(TYPE,error)
    CALL check("h5tclose_f", error, total_error)
    CALL h5tclose_f(array_dt,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5dget_type_f(dataset, type, error)
    CALL check("h5dget_type_f", error, total_error)


    ! /* Read the entire dataset again */

    f_ptr = C_LOC(cfr(1))
    CALL H5Dread_f(dataset, TYPE, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)


    !/* Verify correct data */
    !/* ------------------- */

    DO i = 1, LENGTH
       DO j = 1, ALEN
           IF( cf(i)%a(j) .NE. cfr(i)%a(j) )THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%b(j) .NE. cfr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%c(j) .NE. cfr(i)%c(j) )THEN
             PRINT*, 'ERROR: Wrong double data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO

    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    CALL h5tclose_f(type,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5fclose_f(fid,error)
    CALL check("h5fclose_f", error, total_error)

!/****************************************************/
!/* Reopen the file and print out all the data again */
!/****************************************************/

    CALL h5fopen_f (FILENAME, H5F_ACC_RDWR_F, fid, error)
    CALL check("h5fopen_f", error, total_error)


    CALL h5dopen_f(fid, FIELDNAME, dataset, error)
    CALL check("h5dopen_f", error, total_error)


    CALL h5dget_type_f(dataset, type, error)
    CALL check("h5dget_type_f", error, total_error)


    !/* Reset the data to read in */
    !/* ------------------------- */

    DO i = 1, LENGTH
       cfr(i)%a(:) = 0
       cfr(i)%b(:) = 0
       cfr(i)%c(:) = 0
    ENDDO

    f_ptr = C_LOC(cfr(1))
    CALL H5Dread_f(dataset, TYPE, f_ptr, error)
    CALL check("H5Dread_f", error, total_error)

    !/* Verify correct data */
    !/* ------------------- */

    DO i = 1, LENGTH
       DO j = 1, ALEN
           IF( cf(i)%a(j) .NE. cfr(i)%a(j) )THEN
             PRINT*, 'ERROR: Wrong integer data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%b(j) .NE. cfr(i)%b(j) )THEN
             PRINT*, 'ERROR: Wrong real data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
          IF( cf(i)%c(j) .NE. cfr(i)%c(j) )THEN
             PRINT*, 'ERROR: Wrong double data is read back by H5Dread_f '
             total_error = total_error + 1
          ENDIF
       ENDDO
    ENDDO

    CALL h5dclose_f(dataset, error)
    CALL check("h5dclose_f", error, total_error)

    CALL h5tclose_f(type,error)
    CALL check("h5tclose_f", error, total_error)

    CALL h5fclose_f(fid,error)
    CALL check("h5fclose_f", error, total_error)

  END SUBROUTINE test_array_bkg

  SUBROUTINE test_error(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    USE test_my_hdf5_error_handler

    IMPLICIT NONE

    INTEGER(hid_t), PARAMETER :: FAKE_ID = -1
    INTEGER :: total_error
    INTEGER(hid_t) :: file
    INTEGER(hid_t) :: dataset, space
    INTEGER(hid_t)  :: estack_id
    INTEGER(hsize_t), DIMENSION(1:2) :: dims
    CHARACTER(LEN=10) :: FUNC_test_error = "test_error"
    TYPE(C_FUNPTR) :: old_func
    TYPE(C_PTR) :: old_data, null_data
    INTEGER :: error
    TYPE(C_FUNPTR) :: op
    INTEGER, DIMENSION(1:100,1:200), TARGET :: ipoints2
!!    INTEGER, DIMENSION(1:2), TARGET :: my_hdf5_error_handler_data
    INTEGER, DIMENSION(:), POINTER :: ptr_data
    INTEGER, TARGET :: my_hdf5_error_handler_data
    TYPE(C_PTR) :: f_ptr
    TYPE(C_FUNPTR) :: func

    TYPE(C_PTR), TARGET :: f_ptr1
    TYPE(C_FUNPTR), TARGET :: func1

    INTEGER, DIMENSION(1:1) :: array_shape
    LOGICAL :: is_associated

!    my_hdf5_error_handler_data(1:2) =(/1,2/)
    my_hdf5_error_handler_data = 99
    CALL h5fcreate_f("terror.h5", H5F_ACC_TRUNC_F, file, error)
    CALL check("h5fcreate_f", error, total_error)

    ! Create the data space
    dims(1) = 10
    dims(2) = 20
    CALL H5Screate_simple_f(2, dims, space, error)
    CALL check("h5screate_simple_f", error, total_error)

    ! ** SET THE CUSTOMIZED PRINTING OF ERROR STACK **

    ! set the customized error handling routine
    func = c_funloc(my_hdf5_error_handler)
    
    ! set the data sent to the customized routine
    f_ptr = c_loc(my_hdf5_error_handler_data)

    ! turn on automatic printing, and use a custom error routine with input data
    CALL H5Eset_auto_f(1, error, H5E_DEFAULT_F, func, f_ptr)

    ! Create the erring dataset
    CALL h5dcreate_f(FAKE_ID,"a_dataset",H5T_NATIVE_INTEGER, space, dataset, error)
    CALL VERIFY("h5dcreate_f", error, -1, total_error)

!!$    CALL VERIFY("H5Eset_auto_f",my_hdf5_error_handler_data(1),10, total_error)
!!$    CALL VERIFY("H5Eset_auto_f",my_hdf5_error_handler_data(2),20, total_error)

    ! Test enabling and disabling default printing
    
    CALL H5Eget_auto_f(H5E_DEFAULT_F, func1, f_ptr1, error)
    CALL VERIFY("H5Eget_auto_f", error, 0, total_error)

!    PRINT*,c_associated(f_ptr1)

    ALLOCATE(ptr_data(1:2))
    ptr_data = 0
    array_shape(1) = 2
    CALL C_F_POINTER(f_ptr1, ptr_data, array_shape)

!    ptr_data => f_ptr1(1)

!    PRINT*,ptr_data(1)

!!$    if(old_data != NULL)
!!$	TEST_ERROR;
!!$#ifdef H5_USE_16_API
!!$    if (old_func != (H5E_auto_t)H5Eprint)
!!$	TEST_ERROR;
!!$#else /* H5_USE_16_API */
!!$    if (old_func != (H5E_auto2_t)H5Eprint2)
!!$	TEST_ERROR;
!!$#endif /* H5_USE_16_API */


    ! set the customized error handling routine
    func = c_funloc(my_hdf5_error_handler_nodata)
    ! set the data sent to the customized routine as null
    f_ptr = C_NULL_PTR
    ! turn on automatic printing, and use a custom error routine with no input data
    CALL H5Eset_auto_f(1, error, H5E_DEFAULT_F, func, f_ptr)

    CALL h5dcreate_f(FAKE_ID,"a_dataset",H5T_NATIVE_INTEGER, space, dataset, error)
    CALL VERIFY("h5dcreate_f", error, -1, total_error)


    ! turn on automatic printing with h5eprint_f which prints an error stack in the default manner.

!    func = c_funloc(h5eprint_f)
!    CALL H5Eset_auto_f(0, error, H5E_DEFAULT_F, func, C_NULL_PTR)

    CALL H5Eset_auto_f(0, error)
    CALL h5dcreate_f(FAKE_ID,"a_dataset",H5T_NATIVE_INTEGER, space, dataset, error)

    CALL H5Eset_auto_f(1, error)
    CALL h5dcreate_f(FAKE_ID,"a_dataset",H5T_NATIVE_INTEGER, space, dataset, error)

  END SUBROUTINE test_error


  SUBROUTINE test_h5kind_to_type(total_error)

    USE ISO_C_BINDING

    USE HDF5 ! This module contains all necessary modules

    IMPLICIT NONE
    
    INTEGER, INTENT(INOUT) :: total_error
    
    INTEGER, PARAMETER :: int_kind_1 = SELECTED_INT_KIND(Fortran_INTEGER_1)  !should map to INTEGER*1 on most modern processors
    INTEGER, PARAMETER :: int_kind_4 = SELECTED_INT_KIND(Fortran_INTEGER_2)  !should map to INTEGER*2 on most modern processors
    INTEGER, PARAMETER :: int_kind_8 = SELECTED_INT_KIND(Fortran_INTEGER_4)  !should map to INTEGER*4 on most modern processors
    INTEGER, PARAMETER :: int_kind_16 = SELECTED_INT_KIND(Fortran_INTEGER_8) !should map to INTEGER*8 on most modern processors
    
    INTEGER, PARAMETER :: real_kind_7 = SELECTED_REAL_KIND(Fortran_REAL_4) !should map to REAL*4 on most modern processors
    INTEGER, PARAMETER :: real_kind_15 = SELECTED_REAL_KIND(Fortran_REAL_8) !should map to REAL*8 on most modern processors
    
    CHARACTER(LEN=8), PARAMETER :: filename = "dsetf.h5" ! File name
    CHARACTER(LEN=5), PARAMETER :: dsetname1 = "dset1"     ! Dataset name
    CHARACTER(LEN=5), PARAMETER :: dsetname2 = "dset2"     ! Dataset name
    CHARACTER(LEN=5), PARAMETER :: dsetname4 = "dset4"     ! Dataset name
    CHARACTER(LEN=5), PARAMETER :: dsetname8 = "dset8"     ! Dataset name
    CHARACTER(LEN=6), PARAMETER :: dsetnamer = "dsetr"     ! Dataset name
    CHARACTER(LEN=6), PARAMETER :: dsetnamer4 = "dsetr4"     ! Dataset name
    CHARACTER(LEN=6), PARAMETER :: dsetnamer8 = "dsetr8"     ! Dataset name
    
    INTEGER(HID_T) :: file_id       ! File identifier 
    INTEGER(HID_T) :: dset_id1      ! Dataset identifier  
    INTEGER(HID_T) :: dset_id4      ! Dataset identifier   
    INTEGER(HID_T) :: dset_id8      ! Dataset identifier  
    INTEGER(HID_T) :: dset_id16     ! Dataset identifier     
    INTEGER(HID_T) :: dset_idr       ! Dataset identifier 
    INTEGER(HID_T) :: dset_idr4      ! Dataset identifier   
    INTEGER(HID_T) :: dset_idr8      ! Dataset identifier 
    
    INTEGER :: error ! Error flag
    INTEGER :: i, j
    
! Data buffers:

    INTEGER, DIMENSION(1:4) :: dset_data

    INTEGER(int_kind_1), DIMENSION(1:4), TARGET :: dset_data_i1, data_out_i1
    INTEGER(int_kind_4), DIMENSION(1:4), TARGET :: dset_data_i4, data_out_i4
    INTEGER(int_kind_8), DIMENSION(1:4), TARGET :: dset_data_i8, data_out_i8
    INTEGER(int_kind_16), DIMENSION(1:4), TARGET :: dset_data_i16, data_out_i16

    REAL, DIMENSION(1:4), TARGET :: dset_data_r, data_out_r
    REAL(real_kind_7), DIMENSION(1:4), TARGET :: dset_data_r7, data_out_r7
    REAL(real_kind_15), DIMENSION(1:4), TARGET :: dset_data_r15, data_out_r15
    
    INTEGER(HSIZE_T), DIMENSION(1:1) :: data_dims = (/4/) 
    INTEGER(HID_T) :: dspace_id     ! Dataspace identifier
    
    TYPE(C_PTR) :: f_ptr
    INTEGER(hid_t) :: datatype         !/* Common datatype ID */

    !
    ! Initialize the dset_data array.
    !
    DO i = 1, 4
       dset_data_i1(i)  = i
       dset_data_i4(i)  = i
       dset_data_i8(i)  = i
       dset_data_i16(i) = i

       dset_data_r(i) = (i)*100.
       dset_data_r7(i) = (i)*100.
       dset_data_r15(i) = (i)*1000.
       
    END DO

    CALL h5fcreate_f(filename, H5F_ACC_TRUNC_F, file_id, error)
    CALL check("h5fcreate_f",error, total_error)
  !
  ! Create dataspaces for datasets
  !
    CALL h5screate_simple_f(1, data_dims , dspace_id, error)
    CALL check("h5screate_simple_f",error, total_error)
  !
  ! Create the dataset.
  !
    CALL H5Dcreate_f(file_id, dsetname1, h5kind_to_type(int_kind_1,H5_INTEGER_KIND),  dspace_id, dset_id1, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetname2, h5kind_to_type(int_kind_4,H5_INTEGER_KIND),  dspace_id, dset_id4, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetname4, h5kind_to_type(int_kind_8,H5_INTEGER_KIND),  dspace_id, dset_id8, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetname8, h5kind_to_type(int_kind_16,H5_INTEGER_KIND), dspace_id, dset_id16, error)
    CALL check("H5Dcreate_f",error, total_error)
    
    CALL H5Dcreate_f(file_id, dsetnamer, H5T_NATIVE_REAL, dspace_id, dset_idr, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetnamer4, h5kind_to_type(real_kind_7,H5_REAL_KIND),  dspace_id, dset_idr4, error)
    CALL check("H5Dcreate_f",error, total_error)
    CALL H5Dcreate_f(file_id, dsetnamer8, h5kind_to_type(real_kind_15,H5_REAL_KIND), dspace_id, dset_idr8, error)
    CALL check("H5Dcreate_f",error, total_error)

  !
  ! Write the dataset.
  !
    f_ptr = C_LOC(dset_data_i1(1))
    CALL h5dwrite_f(dset_id1, h5kind_to_type(int_kind_1,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_i4(1))
    CALL h5dwrite_f(dset_id4, h5kind_to_type(int_kind_4,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_i8(1))
    CALL h5dwrite_f(dset_id8, h5kind_to_type(int_kind_8,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_i16(1))
    CALL h5dwrite_f(dset_id16, h5kind_to_type(int_kind_16,H5_INTEGER_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_r(1))
    CALL h5dwrite_f(dset_idr, H5T_NATIVE_REAL, f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_r7(1))
    CALL h5dwrite_f(dset_idr4, h5kind_to_type(real_kind_7,H5_REAL_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
    f_ptr = C_LOC(dset_data_r15(1))
    CALL h5dwrite_f(dset_idr8, h5kind_to_type(real_kind_15,H5_REAL_KIND), f_ptr, error)
    CALL check("H5Dwrite_f",error, total_error)
  !
  ! Close the file
  !
    CALL h5fclose_f(file_id, error)
    CALL check("h5fclose_f",error, total_error)

  ! Open the file

    CALL h5fopen_f(filename, H5F_ACC_RDONLY_F, file_id, error)
    CALL check("h5fopen_f",error, total_error)
  !
  ! Read the dataset.
  !
  ! Read data back into an integer size that is larger then the original size used for 
  ! writing the data
    f_ptr = C_LOC(data_out_i1)
    CALL h5dread_f(dset_id1, h5kind_to_type(int_kind_1,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_i4)
    CALL h5dread_f(dset_id4, h5kind_to_type(int_kind_4,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_i8)
    CALL h5dread_f(dset_id8, h5kind_to_type(int_kind_8,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_i16)
    CALL h5dread_f(dset_id16, h5kind_to_type(int_kind_16,H5_INTEGER_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_r)
    CALL h5dread_f(dset_idr, H5T_NATIVE_REAL, f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_r7)
    CALL h5dread_f(dset_idr4, h5kind_to_type(real_kind_7,H5_REAL_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    f_ptr = C_LOC(data_out_r15)
    CALL h5dread_f(dset_idr8, h5kind_to_type(real_kind_15,H5_REAL_KIND), f_ptr,  error)
    CALL check("h5dread_f",error, total_error)
    
    DO i = 1, 4
       
       CALL verify_Fortran_INTEGER_4("h5kind_to_type1",INT(dset_data_i1(i),int_kind_8),INT(data_out_i1(i),int_kind_8),total_error)
       CALL verify_Fortran_INTEGER_4("h5kind_to_type2",INT(dset_data_i4(i),int_kind_8),INT(data_out_i4(i),int_kind_8),total_error)
       CALL verify_Fortran_INTEGER_4("h5kind_to_type3",INT(dset_data_i8(i),int_kind_8),INT(data_out_i8(i),int_kind_8),total_error)
       CALL verify_Fortran_INTEGER_4("h5kind_to_type4",INT(dset_data_i16(i),int_kind_8),INT(data_out_i16(i),int_kind_8),total_error)

       CALL verify_real_kind_7("h5kind_to_type5",REAL(dset_data_r(i),real_kind_7),REAL(data_out_r(i),real_kind_7),total_error)
       CALL verify_real_kind_7("h5kind_to_type6",REAL(dset_data_r7(i),real_kind_7),REAL(data_out_r7(i),real_kind_7),total_error)
       CALL verify_real_kind_7("h5kind_to_type7",REAL(dset_data_r15(i),real_kind_7),REAL(data_out_r15(i),real_kind_7),total_error)

    END DO

  !
  ! Close the dataset.
  !
    CALL h5dclose_f(dset_id1, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_id4, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_id8, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_id16, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_idr4, error)
    CALL check("h5dclose_f",error, total_error)
    CALL h5dclose_f(dset_idr8, error)
    CALL check("h5dclose_f",error, total_error)
  !
  ! Close the file.
  !
    CALL h5fclose_f(file_id, error)
    CALL check("h5fclose_f",error, total_error)

END SUBROUTINE test_h5kind_to_type


