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
