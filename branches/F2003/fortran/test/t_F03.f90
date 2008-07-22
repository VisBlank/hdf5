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

MODULE liter_cb_mod

  IMPLICIT NONE
    
  TYPE iter_enum 
     INTEGER RET_ZERO
     INTEGER RET_TWO
     INTEGER RET_CHANGE
     INTEGER RET_CHANGE2
  END TYPE iter_enum

  !/* Custom group iteration callback data */
  TYPE iter_info
     CHARACTER(LEN=10) :: name ! /* The name of the object */
     INTEGER :: TYPE         ! /* The TYPE of the object */
     INTEGER :: command !/* The TYPE of RETURN value */
  END TYPE iter_info

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

    INTEGER(HID_T) group
    CHARACTER(LEN=10) :: name
    INTEGER :: link_info

    TYPE(iter_info) :: op_data

    INTEGER, SAVE :: count
    INTEGER, SAVE :: count2 

!!$    
!!$    iter_info *info = (iter_info *)op_data;
!!$    static int count = 0;
!!$    static int count2 = 0;

    op_data%name = name


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
    INTEGER i                  !/* counting variable */
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

    CALL H5Literate_f(file, H5_INDEX_NAME_F, H5_ITER_INC_F, idx, f1, f2, ret_value, error);
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

    DO 
       f1 = C_FUNLOC(liter_cb)
       f2 = C_LOC(info)

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
       
 !/* Verify that the correct name is retrieved */
       CALL verifystring("H5Literate_f", info%name, lnames(INT(idx)), total_error)

    END DO

    CALL VERIFY("H5Literate_f", error,-1, total_error)

    IF(i .NE. (NDATASETS + 2)) THEN
       CALL VERIFY("H5Literate_f", i, INT(NDATASETS + 2), total_error)
       PRINT*,"ERROR: Group iteration function didn't perform multiple iterations correctly"
    ENDIF

    !/* Test all objects in group, when callback changes return value */
    !/* This also tests the "restarting" ability, because the index changes */

    info%command = 3
    idx = 0
    i = 0

    DO 
       f1 = C_FUNLOC(liter_cb)
       f2 = C_LOC(info)

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

       !/* Verify that the correct name is retrieved */
       CALL verifystring("H5Literate_f", info%name, lnames(INT(idx)), total_error)
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
       REAL(C_FLOAT) :: a
       INTEGER(C_INT) :: x
       REAL(C_DOUBLE) :: y
       CHARACTER(LEN=1) :: z
    END TYPE comp_datatype

    TYPE(comp_datatype), TARGET :: rd_c, fill_ctype

    INTEGER(SIZE_T) :: type_sizei  ! Size of the integer datatype 
    INTEGER(SIZE_T) :: type_sizer  ! Size of the real datatype 
    INTEGER(SIZE_T) :: type_sized  ! Size of the double datatype 
    INTEGER(SIZE_T) :: type_sizec  ! Size of the double datatype
    INTEGER(SIZE_T) :: sizeof_compound ! total size of compound
    INTEGER :: error

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
    CALL h5tget_size_f(H5T_NATIVE_INTEGER, type_sizei, error)
    CALL check("h5tget_size_f", error, total_error)
    
    CALL h5tget_size_f(H5T_NATIVE_REAL, type_sizer, error)
    CALL check("h5tget_size_f", error, total_error)
    
    CALL h5tget_size_f(H5T_NATIVE_DOUBLE, type_sized, error)
    CALL check("h5tget_size_f", error, total_error)

    CALL h5tget_size_f(H5T_NATIVE_CHARACTER, type_sizec, error)
    CALL check("h5tget_size_f", error, total_error)
    
    sizeof_compound =  INT(type_sizei + type_sizer + type_sized + type_sizec, size_t)

    CALL h5tcreate_f(H5T_COMPOUND_F, sizeof_compound, comp_type_id, error)
    CALL check("h5tcreate_f", error, total_error)

    CALL h5tinsert_f(comp_type_id, "a", INT(0,SIZE_T), H5T_NATIVE_REAL, error)
    CALL check("h5tinsert_f", error, total_error)
    CALL h5tinsert_f(comp_type_id, "x", type_sizer, H5T_NATIVE_INTEGER, error)
    CALL check("h5tinsert_f", error, total_error)
    CALL h5tinsert_f(comp_type_id, "y", type_sizei+type_sizer, H5T_NATIVE_DOUBLE, error)
    CALL check("h5tinsert_f", error, total_error)
    CALL h5tinsert_f(comp_type_id, "z", type_sizei+type_sizer+type_sized, H5T_NATIVE_CHARACTER, error)
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

       PRINT*,"***ERROR: Got wrong fill value"
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

    IMPLICIT NONE

  CONTAINS

    INTEGER FUNCTION test_genprop_cls_cb1_f(list_id, create_data ) bind(C)

      USE HDF5
      USE ISO_C_BINDING
      IMPLICIT NONE
      
      INTEGER(HID_T), INTENT(IN), VALUE :: list_id
      
      TYPE cop_cb_struct_ ! /* Struct for iterations */
         INTEGER :: count
         INTEGER(HID_T) :: id
      END TYPE cop_cb_struct_

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
       INTEGER(c_int) :: i
       REAL(c_float) :: f
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


    !/* Create a compound datatype to refer to */
    CALL h5tget_size_f(H5T_NATIVE_INTEGER, type_sizei, error)
    CALL check("h5tget_size_f", error, total_error)

    CALL h5tget_size_f(H5T_NATIVE_REAL, type_sizer, error)
    CALL check("h5tget_size_f", error, total_error)

    sizeof_compound =  INT(type_sizei + type_sizer, size_t)

    CALL h5tcreate_f(H5T_COMPOUND_F, sizeof_compound, tid2, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Insert integer field */  
    offset = 0
    CALL h5tinsert_f(tid2, "i", offset, H5T_NATIVE_INTEGER, error)
    CALL check("h5tinsert_f", error, total_error)
     
    !/* Insert float field */

    offset = offset + type_sizei
    CALL h5tinsert_f(tid2, "f", offset, H5T_NATIVE_REAL, error)
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

    f_ptr = C_LOC(wdata)
    CALL h5dwrite_f(dataset, tid1, f_ptr, rdims, error )
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
    CALL VERIFY("H5Tget_member_offset_f",INT(off),type_sizei, total_error)    

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

    f_ptr = C_LOC(rdata)
    CALL H5Dread_f(dataset, tid1, f_ptr, rdims1, error)
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

!/****************************************************************
!**
!**  test_array_compound_array(): Test basic array datatype code.
!**      Tests 1-D array of compound datatypes (with array fields)
!**
!****************************************************************/

  SUBROUTINE test_array_compound_array(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE
    
    INTEGER, INTENT(INOUT) :: total_error

    ! /* 1-D array datatype */
    INTEGER, PARAMETER :: ARRAY1_RANK= 1
    INTEGER, PARAMETER :: ARRAY1_DIM1= 4
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
    INTEGER(HSIZE_T), DIMENSION(1) :: tdims1=(/ARRAY2_DIM1/)

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
    ! First calculate total size by calculating sizes of each member
    !
    CALL h5tcopy_f(H5T_NATIVE_CHARACTER, dt5_id, error)
    CALL check("h5tcopy_f", error, total_error)
    sizechar = 2
    CALL h5tset_size_f(dt5_id, sizechar, error)
    CALL check("h5tset_size_f", error, total_error)
    CALL h5tget_size_f(dt5_id, type_sizec, error)
    CALL check("h5tget_size_f", error, total_error)

    CALL h5tget_size_f(H5T_NATIVE_INTEGER, type_sizei, error)
    CALL check("h5tget_size_f", error, total_error)

    CALL h5tget_size_f(H5T_NATIVE_REAL, type_sizer, error)
    CALL check("h5tget_size_f", error, total_error)

    sizeof_compound =  INT(type_sizei + type_sizer*ARRAY2_DIM1 + type_sizec*ARRAY2_DIM1, size_t)

    CALL h5tcreate_f(H5T_COMPOUND_F, sizeof_compound, tid2, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Insert integer field */  
    offset = 0
    CALL h5tinsert_f(tid2, "i", offset, H5T_NATIVE_INTEGER, error)
    CALL check("h5tinsert_f", error, total_error)


    !/* Create an array of floats datatype */
    CALL h5tarray_create_f(H5T_NATIVE_REAL, ARRAY1_RANK, tdims1, tid3, error)
    CALL check("h5tarray_create_f", error, total_error)

    !/* Insert float array field */

    offset = offset + type_sizei
    CALL h5tinsert_f(tid2, "f", offset, tid3, error)
    CALL check("h5tinsert_f", error, total_error)

    !/* Create an array of character datatype */
    CALL h5tarray_create_f(H5T_NATIVE_CHARACTER, ARRAY1_RANK, tdims1, tid4, error)
    CALL check("h5tarray_create_f", error, total_error)

    !/* Insert character array field */

    offset = offset + type_sizer*ARRAY2_DIM1
    CALL h5tinsert_f(tid2, "c", offset, tid4, error)
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

    ALLOCATE(rdims(1:2)) ! dummy not needed

    f_ptr = C_LOC(wdata)
    CALL h5dwrite_f(dataset, tid1, f_ptr, rdims, error )
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
    CALL VERIFY("H5Tget_member_offset_f",INT(off),type_sizei, total_error) 

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
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims1(i)), total_error)
    ENDDO

    ! /* Check the 3rd field's name */
    CALL H5Tget_member_name_f(tid2, 2, mname, namelen,error)
    CALL check("H5Tget_member_name_f", error, total_error)
    CALL verifystring("H5Tget_member_name_f",mname(1:namelen),"c", total_error)

    ! /* Check the 3rd field's offset */
    CALL H5Tget_member_offset_f(tid2, 2, off, error)
    CALL check("H5Tget_member_offset_f", error, total_error)
    CALL VERIFY("H5Tget_member_offset_f",INT(off),type_sizei+type_sizer*ARRAY2_DIM1, total_error) 

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
       CALL VERIFY("h5tget_array_dims_f", INT(rdims1(i)), INT(tdims1(i)), total_error)
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

    CALL H5Tequal_f(tid3, H5T_NATIVE_CHARACTER, flag, error)
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
    f_ptr = C_LOC(rdata)
    CALL H5Dread_f(dataset, tid1, f_ptr, rdims1, error)
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

!/****************************************************************
!**
!**  test_array_bkg(): Test basic array datatype code.
!**      Tests reading compound datatype with array fields and
!**          writing partial fields.
!**
!****************************************************************/

  SUBROUTINE test_array_bkg(total_error)
    
    USE HDF5 
    USE ISO_C_BINDING
    IMPLICIT NONE
    
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
       REAL*4, DIMENSION(1:ALEN) :: b
       REAL*8, DIMENSION(1:ALEN) :: c
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
       REAL*4, DIMENSION(1:ALEN) :: b
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

    CALL h5tget_size_f(H5T_NATIVE_REAL, type_sizer, error)
    CALL check("h5tget_size_f", error, total_error)

    CALL h5tget_size_f(H5T_NATIVE_DOUBLE, type_sized, error)
    CALL check("h5tget_size_f", error, total_error)

    dtsinfo%offset(1)   = 0
    
    dtsinfo%offset(2)   = type_sizei*ALEN

    dtsinfo%offset(3)   = dtsinfo%offset(2) + type_sizer*ALEN

    !/* Initialize the data type IDs */
    !/* ---------------------------- */
    dtsinfo%datatype(1) = H5T_NATIVE_INTEGER;
    dtsinfo%datatype(2) = H5T_NATIVE_REAL;
    dtsinfo%datatype(3) = H5T_NATIVE_DOUBLE;


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

    sizeof_compound =  INT( (type_sizei + type_sizer + type_sized)*ALEN, size_t)

    CALL h5tcreate_f(H5T_COMPOUND_F, sizeof_compound, type, error)
    CALL check("h5tcreate_f", error, total_error)

    !/* Add  members to the compound data type */
    !/* -------------------------------------- */

    DO i = 1, dtsinfo%nsubfields
       CALL h5tarray_create_f(dtsinfo%datatype(i), ndims(i), dima, array_dt, error)
       CALL check("h5tarray_create_f", error, total_error)

       CALL h5tinsert_f(type, dtsinfo%name(i), dtsinfo%offset(i), array_dt, error)
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

    f_ptr = C_LOC(cf)

    CALL h5dwrite_f(dataset, type, f_ptr, rdims, error )
    CALL check("h5dwrite_f", error, total_error)


    ALLOCATE(rdims1(1:2)) ! dummy not needed
    f_ptr = C_LOC(cfr)
    CALL H5Dread_f(dataset, type, f_ptr, rdims1, error)
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

    CALL h5tcreate_f(H5T_COMPOUND_F, sizeof_compound, type, error)
    CALL check("h5tcreate_f", error, total_error)

    CALL h5tarray_create_f(H5T_NATIVE_REAL, 1, dima, array_dt, error)
    CALL check("h5tarray_create_f", error, total_error)

    CALL h5tinsert_f(TYPE, "Two", 0, array_dt, error)
    CALL check("h5tinsert_f", error, total_error)

    !/* Initialize the data to overwrite */
    !/* -------------------------------- */
    DO i = 1, LENGTH
       DO j = 1, ALEN
          fld(i)%b(j) = 1.313
          cf(i)%b(j) = fld(i)%b(j)
       ENDDO
    ENDDO

    f_ptr = C_LOC(fld)

    CALL h5dwrite_f(dataset, TYPE, f_ptr, rdims, error )
    CALL check("h5dwrite_f", error, total_error)


    ! /* Read just the field changed */
    
    f_ptr = C_LOC(fldr)
    CALL H5Dread_f(dataset, TYPE, f_ptr, rdims1, error)
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

    f_ptr = C_LOC(cfr)
    CALL H5Dread_f(dataset, TYPE, f_ptr, rdims1, error)
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

    f_ptr = C_LOC(cfr)
    CALL H5Dread_f(dataset, TYPE, f_ptr, rdims1, error)
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

