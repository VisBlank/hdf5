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
     IF(i.EQ.52)EXIT ! prints out error message otherwise (for gcc/gfortran/g95) not intel (why) -FIXME- scot
  END DO

  ! put check if did not walk far enough -scot FIXME

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
