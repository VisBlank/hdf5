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
SUBROUTINE test_h5o(cleanup, total_error)
  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  LOGICAL, INTENT(IN)  :: cleanup
  INTEGER, INTENT(OUT) :: total_error

  ! /* Output message about test being performed */
  WRITE(*,*) "Testing Objects"

!!$  test_h5o_open();		/* Test generic OPEN FUNCTION */
!!$  test_h5o_open_by_addr();	/* Test opening objects by address */
!!$  test_h5o_close();		/* Test generic CLOSE FUNCTION */
!!$  test_h5o_refcount();        /* Test incrementing and decrementing reference count */
!!$  test_h5o_plist();           /* Test object creation properties */
  CALL test_h5o_link(total_error) ! /* Test object link routine */

END SUBROUTINE test_h5o

!/****************************************************************
!**
!**  test_h5o_link: Test creating link to object
!**
!****************************************************************/

SUBROUTINE test_h5o_link(total_error)

  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  INTEGER, INTENT(OUT) :: total_error

  INTEGER(HID_T) :: file_id
  INTEGER(HID_T) :: group_id
  INTEGER(HID_T) :: space_id
  INTEGER(HID_T) :: dset_id
  INTEGER(HID_T) :: type_id
  INTEGER(HID_T) :: fapl_id
  INTEGER(HID_T) :: lcpl_id
  CHARACTER(LEN=8), PARAMETER :: TEST_FILENAME = 'TestFile'
  INTEGER, PARAMETER :: TEST6_DIM1 = 2, TEST6_DIM2 = 5
  INTEGER(HSIZE_T), DIMENSION(1:2), PARAMETER :: dims = (/TEST6_DIM1,TEST6_DIM2/)
  INTEGER, DIMENSION(1:TEST6_DIM1,1:TEST6_DIM2) :: wdata, rdata

  INTEGER, PARAMETER :: TRUE = 1, FALSE = 0

  LOGICAL :: committed ! /* Whether the named datatype is committed */

  INTEGER :: i, n, j
  INTEGER ::  error  ! /* Value returned from API calls */

  ! /* Initialize the raw data */
  DO i = 1, TEST6_DIM1
     DO j = 1, TEST6_DIM2
        wdata(i,j) = i*j
     ENDDO
  ENDDO
  
  ! /* Create the dataspace */
  CALL h5screate_simple_f(2, dims, space_id, error)
  CALL check("h5screate_simple_f",error,total_error)

  ! /* Create LCPL with intermediate group creation flag set */
  CALL H5Pcreate_f(H5P_LINK_CREATE_F, lcpl_id, error)
  CALL check("h5Pcreate_f",error,total_error)

  CALL H5Pset_create_intermediate_group_f(lcpl_id, TRUE, error)
  CALL check("H5Pset_create_intermediate_group_f",error,total_error)


  ! /* Loop over using new group format */
  ! for(new_format = FALSE; new_format <= TRUE; new_format++) {

  !/* Make a FAPL that uses the "use the latest version of the format" bounds */
  CALL H5Pcreate_f(H5P_FILE_ACCESS_F,fapl_id,error)
  CALL check("h5Pcreate_f",error,total_error)
  
  ! /* Set the "use the latest version of the format" bounds for creating objects in the file */
  
  CALL H5Pset_libver_bounds_f(fapl_id, H5F_LIBVER_LATEST_F, H5F_LIBVER_LATEST_F, error)
  CALL check("H5Pset_libver_bounds_f",error, total_error)
  
!!$ ret = H5Pset_libver_bounds(fapl_id, (new_format ? H5F_LIBVER_LATEST : H5F_LIBVER_EARLIEST), H5F_LIBVER_LATEST);
  
  ! /* Create a new HDF5 file */
  CALL H5Fcreate_f(TEST_FILENAME, H5F_ACC_TRUNC_F, file_id, error, H5P_DEFAULT_F, fapl_id)
  CALL check("H5Fcreate_f", error, total_error)

  ! /* Close the FAPL */
  CALL h5pclose_f(fapl_id, error)
  CALL check("h5pclose_f",error,total_error)
  
  ! /* Create and commit a datatype with no name */
  CALL H5Tcopy_f( H5T_NATIVE_INTEGER, type_id, error)
  CALL check("H5Tcopy",error,total_error)
  
  CALL H5Tcommit_anon_f(file_id, type_id, error) ! using no optional parameters
  CALL check("H5Tcommit_anon",error,total_error)

  CALL H5Tcommitted_f(type_id, committed, error)
  CALL check("H5Tcommitted_f",error,total_error)
  CALL verifyLogical("H5Tcommitted_f", committed, .TRUE., total_error)

  ! /* Create a dataset with no name using the committed datatype*/
  CALL H5Dcreate_anon_f(file_id, type_id, space_id, dset_id, error ) ! using no optional parameters
  CALL check("H5Dcreate_anon_f",error,total_error)


  ! /* Verify that we can write to and read from the dataset */
  
  ! /* Write the data to the dataset */

  CALL h5dwrite_f(dset_id, H5T_NATIVE_INTEGER, wdata, dims, error, &
         mem_space_id=H5S_ALL_F, file_space_id=H5S_ALL_F, xfer_prp = H5P_DEFAULT_F)
  CALL check("h5dwrite_f", error, total_error)

  ! /* Read the data back */
  CALL h5dread_f(dset_id, H5T_NATIVE_INTEGER, rdata, dims, error, &
       mem_space_id=H5S_ALL_F, file_space_id=H5S_ALL_F, xfer_prp = H5P_DEFAULT_F)
  CALL check("h5dread_f", error, total_error)
        
  ! /* Verify the data */
  DO i = 1, TEST6_DIM1
     DO j = 1, TEST6_DIM2
        CALL VERIFY("H5Dread",wdata,rdata,total_error)
        wdata(i,j) = i*j
     ENDDO
  ENDDO


  
!!$    ! /* Create a group with no name*/
!!$    group_id = H5Gcreate_anon(file_id, H5P_DEFAULT, H5P_DEFAULT);
!!$    CHECK(group_id, FAIL, "H5Gcreate_anon");
!!$    
!!$    ! /* Link nameless datatype into nameless group */
!!$    ret = H5Olink(type_id, group_id, "datatype", H5P_DEFAULT, H5P_DEFAULT);
!!$    CHECK(ret, FAIL, "H5Olink");
!!$
!!$    ! /* Link nameless dataset into nameless group with intermediate group */
!!$    ret = H5Olink(dset_id, group_id, "inter_group/dataset", lcpl_id, H5P_DEFAULT);
!!$    CHECK(ret, FAIL, "H5Olink");

    ! /* Close IDs for dataset and datatype */
!!$        ret = H5Dclose(dset_id);
!!$        CHECK(ret, FAIL, "H5Dclose");
!!$        ret = H5Tclose(type_id);
!!$        CHECK(ret, FAIL, "H5Tclose");

!!$        /* Re-open datatype using new link */
!!$        type_id = H5Topen2(group_id, "datatype", H5P_DEFAULT);
!!$        CHECK(type_id, FAIL, "H5Topen2");
!!$
!!$        /* Link nameless group to root group and close the group ID*/
!!$        ret = H5Olink(group_id, file_id, "/group", H5P_DEFAULT, H5P_DEFAULT);
!!$        CHECK(ret, FAIL, "H5Olink");
!!$        ret = H5Gclose(group_id);
!!$        CHECK(ret, FAIL, "H5Gclose");
!!$
!!$        /* Open dataset through root group and verify its data */
!!$        dset_id = H5Dopen2(file_id, "/group/inter_group/dataset", H5P_DEFAULT);
!!$        CHECK(dset_id, FAIL, "H5Dopen2");
!!$
!!$        /* Read data from dataset */
!!$        ret = H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
!!$        CHECK(ret, FAIL, "H5Dread");
!!$        for(i = 0; i < TEST6_DIM1; i++)
!!$            for(j = 0; j < TEST6_DIM2; j++)
!!$                VERIFY(wdata[i][j], rdata[i][j], "H5Dread");
!!$            
  ! /* Close open IDs */

  CALL h5dclose_f(dset_id, error)
  CALL check("h5dclose_f",error,total_error)
  CALL h5tclose_f(type_id, error)
  CALL check("h5tclose_f",error,total_error)

  CALL h5fclose_f(file_id, error)
  CALL check("h5fclose_f",error,total_error)

  ! /* Close remaining IDs */
  CALL h5sclose_f(space_id, error)
  CALL check("h5sclose_f",error,total_error)
  CALL h5pclose_f(lcpl_id,error)
  CALL check("h5pclose_f", error, total_error)

END SUBROUTINE test_h5o_link

!/*-------------------------------------------------------------------------
! * Function:    delete_by_idx
! *
! * Purpose:     Create a group with creation order indices and test deleting
! *              links by index.
! *
! * Return:      Total error
! *
! * C Programmer:  Quincey Koziol
! *                Tuesday, November 14, 2006
! * Adapted to FORTRAN: M.S. Breitenfeld
! *                     March 3, 2008
! *
! *-------------------------------------------------------------------------
! */
SUBROUTINE delete_by_idx(fapl, total_error)

  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  INTEGER, INTENT(OUT) :: total_error
  INTEGER(HID_T), INTENT(IN) :: fapl

  INTEGER(HID_T) :: file_id  ! /* File ID */
  INTEGER(HID_T) :: group_id ! /* Group ID */
  INTEGER(HID_T) :: gcpl_id  ! /* Group creation property list ID */

  INTEGER :: idx_type        ! /* Type of index to operate on */
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./) 
                             ! /* Use index on creation order values */
  INTEGER :: max_compact     ! /* Maximum # of links to store in group compactly */ 
  INTEGER :: min_dense       ! /* Minimum # of links to store in group "densely" */

  CHARACTER(LEN=7) :: objname   ! /* Object name */
  CHARACTER(LEN=8) :: filename = 'file0.h5' ! /* File name */
  CHARACTER(LEN=7) :: tmpname   ! /* Temporary link name */
  CHARACTER(LEN=12), PARAMETER :: CORDER_GROUP_NAME = "corder_group"

  LOGICAL :: f_corder_valid ! Indicates whether the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! Indicates the size, in the number of characters, of the attribute

  INTEGER :: u ! /* Local index variable */
  INTEGER :: Input1, i
  INTEGER(HID_T) :: group_id2

  INTEGER :: iorder ! /* Order within in the index */
  CHARACTER(LEN=2) :: chr2
  INTEGER :: error
  !
  !
  !
  CHARACTER(LEN=6)  :: filename1 
  CHARACTER(LEN=6)  :: filename2
  CHARACTER(LEN=80) :: fix_filename1
  CHARACTER(LEN=80) :: fix_filename2
  INTEGER(SIZE_T) :: size_tmp

  DO i = 1, 80
     fix_filename1(i:i) = " "
     fix_filename2(i:i) = " "
  ENDDO

  ! /* Loop over operating on different indices on link fields */
  DO idx_type = H5_INDEX_NAME_F, H5_INDEX_CRT_ORDER_F
     ! /* Loop over operating in different orders */
     DO iorder = H5_ITER_INC_F,  H5_ITER_DEC_F
        ! /* Loop over using index for creation order value */
        DO i = 1, 2
           ! /* Print appropriate test message */
           IF(idx_type == H5_INDEX_CRT_ORDER_F)THEN
              IF(iorder == H5_ITER_INC_F)THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"deleting links by creation order index in increasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"deleting links by creation order index in increasing order w/o creation order index"
                 ENDIF
              ELSE
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"deleting links by creation order index in decreasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"deleting links by creation order index in decreasing order w/o creation order index"
                 ENDIF
              ENDIF
           ELSE
              IF(iorder == H5_ITER_INC_F)THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"deleting links by name index in increasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"deleting links by name index in increasing order w/o creation order index"
                 ENDIF
              ELSE
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"deleting links by name index in decreasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"deleting links by name index in decreasing order w/o creation order index"
                 ENDIF
              ENDIF
           ENDIF
!           CALL h5_fixname_f(filename1, fix_filename1, H5P_DEFAULT_F, error)
!           IF(error .NE. 0) STOP

           ! /* Create file */
           CALL H5Fcreate_f(filename, H5F_ACC_TRUNC_F, file_id, error, access_prp=fapl)
           CALL check("delete_by_idx.H5Fcreate_f", error, total_error)
           
           ! /* Create group creation property list */
           CALL H5Pcreate_f(H5P_GROUP_CREATE_F, gcpl_id, error )
           CALL check("delete_by_idx.H5Pcreate_f", error, total_error)

           ! /* Set creation order tracking & indexing on group */
           IF(use_index(i))THEN
              Input1 = H5P_CRT_ORDER_INDEXED_F
           ELSE
              Input1 = 0
           ENDIF

           CALL H5Pset_link_creation_order_f(gcpl_id, IOR(H5P_CRT_ORDER_TRACKED_F, Input1), error)
           CALL check("delete_by_idx.H5Pset_link_creation_order_f", error, total_error)

           ! /* Create group with creation order tracking on */
           CALL H5Gcreate_f(file_id, CORDER_GROUP_NAME, group_id, error, gcpl_id=gcpl_id)
           CALL check("delete_by_idx.H5Gcreate_f", error, total_error)

           ! /* Query the group creation properties */
           CALL H5Pget_link_phase_change_f(gcpl_id, max_compact, min_dense, error)
           CALL check("delete_by_idx.H5Pget_link_phase_change_f", error, total_error)


           ! /* Delete links from one end */

           ! /* Check for deletion on empty group */
           CALL H5Ldelete_by_idx_f(group_id, ".", idx_type, iorder, INT(0,HSIZE_T), error)
           CALL VERIFY("delete_by_idx.H5Ldelete_by_idx_f", error, -1, total_error) ! test should fail (error = -1)
           ! /* Create several links, up to limit of compact form */
           DO u = 0, max_compact-1
              ! /* Make name for link */
              WRITE(chr2,'(I2.2)') u
              objname = 'fill '//chr2

              ! /* Create hard link, with group object */
              CALL H5Gcreate_f(group_id, objname, group_id2, error)
              CALL check("delete_by_idx.H5Gcreate_f", error, total_error)
              CALL H5Gclose_f(group_id2, error)
              CALL check("delete_by_idx.H5Gclose_f", error, total_error)

              ! /* Verify link information for new link */
              CALL link_info_by_idx_check(group_id, objname, u, &
                   .TRUE., use_index, total_error)
           ENDDO

           ! /* Verify state of group (compact) */
           ! IF(H5G_has_links_test(group_id, NULL) != TRUE) TEST_ERROR

           ! /* Check for out of bound deletion */

           CALL H5Ldelete_by_idx_f(group_id, ".", idx_type, iorder, INT(u,HSIZE_T), error)
           CALL VERIFY("delete_by_idx.H5Ldelete_by_idx_f", error, -1, total_error) ! test should fail (error = -1)


           ! /* Delete links from compact group */

           DO u = 0, (max_compact - 1) -1
              ! /* Delete first link in appropriate order */
              CALL H5Ldelete_by_idx_f(group_id, ".", idx_type, iorder, INT(0,HSIZE_T), error)
              CALL check("delete_by_idx.H5Ldelete_by_idx_f", error, total_error)
              ! /* Verify the link information for first link in appropriate order */
              ! HDmemset(&linfo, 0, sizeof(linfo));

              CALL H5Lget_info_by_idx_f(group_id, ".", idx_type, iorder, INT(0,HSIZE_T), &
                   f_corder_valid, corder, cset, data_size, error)

              IF(iorder.EQ.H5_ITER_INC_F)THEN
                 CALL VERIFY("delete_by_idx.H5Lget_info_by_idx_f", corder, u+1, total_error)
              ELSE
                 CALL VERIFY("delete_by_idx.H5Lget_info_by_idx_f", corder, (max_compact - (u + 2)), total_error)
              ENDIF

              ! /* Verify the name for first link in appropriate order */
              ! HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$              size_tmp = 20
!!$              CALL H5Lget_name_by_idx_f(group_id, ".", idx_type, order, INT(0,HSIZE_T), size_tmp, tmpname, error)
!!$              CALL check("delete_by_idx.H5Lget_name_by_idx_f", error, total_error)
!!$
!!$              IF(order .EQ. H5_ITER_INC_F)THEN
!!$                 WRITE(chr2,'(I2.2)') u + 1
!!$              ELSE
!!$                 WRITE(chr2,'(I2.2)') (max_compact - (u + 2))
!!$              ENDIF
!!$              objname = 'fill '//chr2
!!$              PRINT*,objname, tmpname
!!$              CALL verifyString("delete_by_idx.H5Lget_name_by_idx_f", objname, tmpname,  total_error)
           ENDDO
!!$
!!$                /* Delete last link */
!!$                if(H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                /* Verify state of group (empty) */
!!$                if(H5G_has_links_test(group_id, NULL) == TRUE) TEST_ERROR
!!$
!!$                /* Create more links, to push group into dense form */
!!$                for(u = 0; u < (max_compact * 2); u++) {
!!$                    hid_t group_id2;	        /* Group ID */
!!$
!!$                    /* Make name for link */
!!$                    sprintf(objname, "filler %02u", u);
!!$
!!$                    /* Create hard link, with group object */
!!$                    if((group_id2 = H5Gcreate2(group_id, objname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) TEST_ERROR
!!$                    if(H5Gclose(group_id2) < 0) TEST_ERROR
!!$
!!$                    /* Verify state of group (dense) */
!!$                    if(u >= max_compact)
!!$                        if(H5G_is_new_dense_test(group_id) != TRUE) TEST_ERROR
!!$
!!$                    /* Verify link information for new link */
!!$                    if(link_info_by_idx_check(group_id, objname, (hsize_t)u, TRUE, use_index) < 0) TEST_ERROR
!!$                } /* end for */
!!$
!!$                /* Check for out of bound deletion again */
!!$                H5E_BEGIN_TRY {
!!$                    ret = H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)u, H5P_DEFAULT);
!!$                } H5E_END_TRY;
!!$                if(ret >= 0) TEST_ERROR
!!$
!!$                /* Delete links from dense group, in appropriate order */
!!$                for(u = 0; u < ((max_compact * 2) - 1); u++) {
!!$                    /* Delete first link in appropriate order */
!!$                    if(H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Verify the link information for first link in appropriate order */
!!$                    HDmemset(&linfo, 0, sizeof(linfo));
!!$                    if(H5Lget_info_by_idx(group_id, ".", idx_type, order, (hsize_t)0, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$                    if(order == H5_ITER_INC) {
!!$                        if(linfo.corder != (u + 1)) TEST_ERROR
!!$                    } /* end if */
!!$                    else {
!!$                        if(linfo.corder != ((max_compact * 2) - (u + 2))) TEST_ERROR
!!$                    } /* end else */
!!$
!!$                    /* Verify the name for first link in appropriate order */
!!$                    HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$                    if(H5Lget_name_by_idx(group_id, ".", idx_type, order, (hsize_t)0, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$                    if(order == H5_ITER_INC)
!!$                        sprintf(objname, "filler %02u", (u + 1));
!!$                    else
!!$                        sprintf(objname, "filler %02u", ((max_compact * 2) - (u + 2)));
!!$                    if(HDstrcmp(objname, tmpname)) TEST_ERROR
!!$                } /* end for */
!!$
!!$                /* Delete last link */
!!$                if(H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                /* Verify state of group (empty) */
!!$                if(H5G_has_links_test(group_id, NULL) == TRUE) TEST_ERROR
!!$                if(H5G_is_new_dense_test(group_id) == TRUE) TEST_ERROR
!!$
!!$                /* Check for deletion on empty group again */
!!$                H5E_BEGIN_TRY {
!!$                    ret = H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT);
!!$                } H5E_END_TRY;
!!$                if(ret >= 0) TEST_ERROR
!!$
!!$
!!$                /* Delete links in middle */
!!$
!!$
!!$                /* Create more links, to push group into dense form */
!!$                for(u = 0; u < (max_compact * 2); u++) {
!!$                    hid_t group_id2;	        /* Group ID */
!!$
!!$                    /* Make name for link */
!!$                    sprintf(objname, "filler %02u", u);
!!$
!!$                    /* Create hard link, with group object */
!!$                    if((group_id2 = H5Gcreate2(group_id, objname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) TEST_ERROR
!!$                    if(H5Gclose(group_id2) < 0) TEST_ERROR
!!$
!!$                    /* Verify state of group (dense) */
!!$                    if(u >= max_compact)
!!$                        if(H5G_is_new_dense_test(group_id) != TRUE) TEST_ERROR
!!$
!!$                    /* Verify link information for new link */
!!$                    if(link_info_by_idx_check(group_id, objname, (hsize_t)u, TRUE, use_index) < 0) TEST_ERROR
!!$                } /* end for */
!!$
!!$                /* Delete every other link from dense group, in appropriate order */
!!$                for(u = 0; u < max_compact; u++) {
!!$                    /* Delete link */
!!$                    if(H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)u, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Verify the link information for current link in appropriate order */
!!$                    HDmemset(&linfo, 0, sizeof(linfo));
!!$                    if(H5Lget_info_by_idx(group_id, ".", idx_type, order, (hsize_t)u, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$                    if(order == H5_ITER_INC) {
!!$                        if(linfo.corder != ((u * 2) + 1)) TEST_ERROR
!!$                    } /* end if */
!!$                    else {
!!$                        if(linfo.corder != ((max_compact * 2) - ((u * 2) + 2))) TEST_ERROR
!!$                    } /* end else */
!!$
!!$                    /* Verify the name for current link in appropriate order */
!!$                    HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$                    if(H5Lget_name_by_idx(group_id, ".", idx_type, order, (hsize_t)u, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$                    if(order == H5_ITER_INC)
!!$                        sprintf(objname, "filler %02u", ((u * 2) + 1));
!!$                    else
!!$                        sprintf(objname, "filler %02u", ((max_compact * 2) - ((u * 2) + 2)));
!!$                    if(HDstrcmp(objname, tmpname)) TEST_ERROR
!!$                } /* end for */
!!$
!!$                /* Delete remaining links from dense group, in appropriate order */
!!$                for(u = 0; u < (max_compact - 1); u++) {
!!$                    /* Delete link */
!!$                    if(H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Verify the link information for first link in appropriate order */
!!$                    HDmemset(&linfo, 0, sizeof(linfo));
!!$                    if(H5Lget_info_by_idx(group_id, ".", idx_type, order, (hsize_t)0, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$                    if(order == H5_ITER_INC) {
!!$                        if(linfo.corder != ((u * 2) + 3)) TEST_ERROR
!!$                    } /* end if */
!!$                    else {
!!$                        if(linfo.corder != ((max_compact * 2) - ((u * 2) + 4))) TEST_ERROR
!!$                    } /* end else */
!!$
!!$                    /* Verify the name for first link in appropriate order */
!!$                    HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$                    if(H5Lget_name_by_idx(group_id, ".", idx_type, order, (hsize_t)0, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$                    if(order == H5_ITER_INC)
!!$                        sprintf(objname, "filler %02u", ((u * 2) + 3));
!!$                    else
!!$                        sprintf(objname, "filler %02u", ((max_compact * 2) - ((u * 2) + 4)));
!!$                    if(HDstrcmp(objname, tmpname)) TEST_ERROR
!!$                } /* end for */
!!$
!!$                /* Delete last link */
!!$                if(H5Ldelete_by_idx(group_id, ".", idx_type, order, (hsize_t)0, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                /* Verify state of group (empty) */
!!$                if(H5G_has_links_test(group_id, NULL) == TRUE) TEST_ERROR
!!$                if(H5G_is_new_dense_test(group_id) == TRUE) TEST_ERROR
!!$
!!$
!!$
           ! /* Close the group */
           CALL H5Gclose_f(group_id, error)
           CALL check("delete_by_idx.H5Gclose_f", error, total_error)

           !/* Close the group creation property list */
           CALL H5Pclose_f(gcpl_id, error)
           CALL check("delete_by_idx.H5Gclose_f", error, total_error)

           !/* Close the file */
           CALL H5Fclose_f(file_id, error)
           CALL check("delete_by_idx.H5Gclose_f", error, total_error)

        ENDDO
     ENDDO
  ENDDO
!!$
!!$    return 0;
!!$
!!$error:
!!$    H5E_BEGIN_TRY {
!!$        H5Pclose(gcpl_id);
!!$        H5Gclose(group_id);
!!$        H5Fclose(file_id);
!!$    } H5E_END_TRY;
!!$    return -1;
!!$} /* end delete_by_idx() */

END SUBROUTINE delete_by_idx

!/*-------------------------------------------------------------------------
! * Function:    link_info_by_idx_check
! *
! * Purpose:     Support routine for link_info_by_idx, to verify the link
! *              info is correct for a link
! *
! * Note:	This routine assumes that the links have been inserted in the
! *              group in alphabetical order.
! *
! * Return:      Success:        0
! *              Failure:        -1
! *
! * Programmer:  Quincey Koziol
! *              Tuesday, November  7, 2006
! *
! *-------------------------------------------------------------------------
! */
SUBROUTINE link_info_by_idx_check(group_id, linkname, n, &
    hard_link, use_index, total_error)

  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  INTEGER, INTENT(INOUT) :: total_error
  INTEGER(HID_T), INTENT(IN) :: group_id
  CHARACTER(LEN=*), INTENT(IN) :: linkname
  INTEGER, INTENT(IN) :: n
  LOGICAL, INTENT(IN) :: hard_link
  LOGICAL, INTENT(IN) :: use_index

  LOGICAL :: f_corder_valid ! Indicates whether the creation order data is valid for this attribute 
  INTEGER :: corder ! Is a positive integer containing the creation order of the attribute
  INTEGER :: cset ! Indicates the character set used for the attribute’s name
  INTEGER(HSIZE_T) :: data_size   ! Indicates the size, in the number of characters, of the attribute

  CHARACTER(LEN=7) :: tmpname     !/* Temporary link name */
  CHARACTER(LEN=3) :: tmpname_small !/* to small temporary link name */
  CHARACTER(LEN=10) :: tmpname_big !/* to big temporary link name */

  CHARACTER(LEN=7) :: valname     !/* Link value name */
  CHARACTER(LEN=7) :: tmpval      !/* Temporary link value */
  CHARACTER(LEN=2) :: chr2
  INTEGER(SIZE_T) :: size_tmp
  INTEGER :: error

  ! /* Make link value for increasing/native order queries */

  WRITE(chr2,'(I2.2)') n
  valname = 'valn.'//chr2

  ! /* Verify the link information for first link, in increasing creation order */
  !  HDmemset(&linfo, 0, sizeof(linfo));
  CALL H5Lget_info_by_idx_f(group_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(0,HSIZE_T), &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("H5Lget_info_by_idx_f", error, total_error)
  CALL VERIFY("H5Lget_info_by_idx_f", corder, 0, total_error)

  ! /* Verify the link information for new link, in increasing creation order */
  ! HDmemset(&linfo, 0, sizeof(linfo));
  CALL H5Lget_info_by_idx_f(group_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(n,HSIZE_T), &
       f_corder_valid, corder, cset, data_size, error)
  CALL check("H5Lget_info_by_idx_f", error, total_error)
  CALL VERIFY("H5Lget_info_by_idx_f", corder, n, total_error)

  ! /* Verify value for new soft link, in increasing creation order */
!!$  IF(hard_link)THEN
!!$     ! HDmemset(tmpval, 0, (size_t)NAME_BUF_SIZE);
!!$     
!!$     CALL H5Lget_val_by_idx_f(group_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, n, tmpval, INT(7,SIZE_T),error)
!!$     CALL check("H5Lget_val_by_idx",error,total_error)
!!$
!!$!     IF(HDstrcmp(valname, tmpval)) TEST_ERROR
!!$  ENDIF

  ! /* Verify the name for new link, in increasing creation order */
  !  HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);

  ! The actual size of tmpname should be 7

  ! Try with a size set to zero
  size_tmp = INT(LEN(tmpname_small),SIZE_T)
  CALL H5Lget_name_by_idx_f(group_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(n,HSIZE_T), size_tmp, tmpname_small, error)
  CALL check("link_info_by_idx_check.H5Lget_name_by_idx_f", error, total_error)
  CALL verifyString("link_info_by_idx_check.H5Lget_name_by_idx_f", &
       linkname(1:LEN(tmpname_small)), tmpname_small(1:LEN(tmpname_small)),  total_error)
  CALL VERIFY("link_info_by_idx_check.H5Lget_name_by_idx_f", size_tmp, 7, total_error)

  ! try it with the correct size
  size_tmp = INT(LEN(tmpname),SIZE_T)
  CALL H5Lget_name_by_idx_f(group_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(n,HSIZE_T), size_tmp, tmpname, error)
  CALL check("link_info_by_idx_check.H5Lget_name_by_idx_f", error, total_error)
  CALL verifyString("link_info_by_idx_check.H5Lget_name_by_idx_f", &
       linkname(1:LEN(tmpname)), tmpname(1:LEN(tmpname)),  total_error)
  CALL VERIFY("link_info_by_idx_check.H5Lget_name_by_idx_f", size_tmp, 7, total_error)

  size_tmp = INT(LEN(tmpname_big),SIZE_T)
  CALL H5Lget_name_by_idx_f(group_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(n,HSIZE_T), size_tmp, tmpname_big, error)
  CALL check("link_info_by_idx_check.H5Lget_name_by_idx_f", error, total_error)
  CALL verifyString("link_info_by_idx_check.H5Lget_name_by_idx_f", &
       linkname(1:7), tmpname_big(1:7),  total_error)
  CALL VERIFY("link_info_by_idx_check.H5Lget_name_by_idx_f", size_tmp, 7, total_error)

  ! Try with a buffer set to small

!!$  size_tmp = INT(4,SIZE_T)
!!$  CALL H5Lget_name_by_idx_f(group_id, ".", H5_INDEX_CRT_ORDER_F, H5_ITER_INC_F, INT(n,HSIZE_T), size_tmp, tmpname, error)
!!$  CALL check("H5Lget_name_by_idx_f", error, total_error)
!!$  CALL verifyString("H5Lget_name_by_idx_f", linkname, tmpname,  total_error)


!!$
!!$    if(H5Lget_name_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, n, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(HDstrcmp(linkname, tmpname)) TEST_ERROR

!!$    /* Don't test "native" order if there is no creation order index, since
!!$     *  there's not a good way to easily predict the link's order in the name
!!$     *  index.
!!$     */
!!$    if(use_index) {
!!$        /* Verify the link information for first link, in native creation order (which is increasing) */
!!$        HDmemset(&linfo, 0, sizeof(linfo));
!!$        if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, (hsize_t)0, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$        if(linfo.corder != 0) TEST_ERROR
!!$
!!$        /* Verify the link information for new link, in native creation order (which is increasing) */
!!$        HDmemset(&linfo, 0, sizeof(linfo));
!!$        if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, n, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$        if(linfo.corder != (int64_t)n) TEST_ERROR
!!$
!!$        /* Verify value for new soft link, in native creation order (which is increasing) */
!!$        if(!hard_link) {
!!$            HDmemset(tmpval, 0, (size_t)NAME_BUF_SIZE);
!!$            if(H5Lget_val_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, n, tmpval, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$            if(HDstrcmp(valname, tmpval)) TEST_ERROR
!!$        } /* end if */
!!$
!!$        /* Verify the name for new link, in native creation order (which is increasing) */
!!$        HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$        if(H5Lget_name_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, n, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$        if(HDstrcmp(linkname, tmpname)) TEST_ERROR
!!$    } /* end if */
!!$
!!$    /* Verify the link information for first link, in decreasing creation order */
!!$    HDmemset(&linfo, 0, sizeof(linfo));
!!$    if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, n, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder != 0) TEST_ERROR
!!$
!!$    /* Verify the link information for new link, in decreasing creation order */
!!$    HDmemset(&linfo, 0, sizeof(linfo));
!!$    if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, (hsize_t)0, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder != (int64_t)n) TEST_ERROR
!!$
!!$    /* Verify value for new soft link, in decreasing creation order */
!!$    if(!hard_link) {
!!$        HDmemset(tmpval, 0, (size_t)NAME_BUF_SIZE);
!!$        if(H5Lget_val_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, (hsize_t)0, tmpval, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$        if(HDstrcmp(valname, tmpval)) TEST_ERROR
!!$    } /* end if */
!!$
!!$    /* Verify the name for new link, in decreasing creation order */
!!$    HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$    if(H5Lget_name_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, (hsize_t)0, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(HDstrcmp(linkname, tmpname)) TEST_ERROR
!!$
!!$
!!$    /* Verify the link information for first link, in increasing link name order */
!!$    HDmemset(&linfo, 0, sizeof(linfo));
!!$    if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, (hsize_t)0, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder != 0) TEST_ERROR
!!$
!!$    /* Verify the link information for new link, in increasing link name order */
!!$    HDmemset(&linfo, 0, sizeof(linfo));
!!$    if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, n, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder != (int64_t)n) TEST_ERROR
!!$
!!$    /* Verify value for new soft link, in increasing link name order */
!!$    if(!hard_link) {
!!$        HDmemset(tmpval, 0, (size_t)NAME_BUF_SIZE);
!!$        if(H5Lget_val_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, n, tmpval, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$        if(HDstrcmp(valname, tmpval)) TEST_ERROR
!!$    } /* end if */
!!$
!!$    /* Verify the name for new link, in increasing link name order */
!!$    HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$    if(H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, n, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(HDstrcmp(linkname, tmpname)) TEST_ERROR
!!$
!!$    /* Don't test "native" order queries on link name order, since there's not
!!$     *  a good way to easily predict the order of the links in the name index.
!!$     */
!!$
!!$    /* Verify the link information for first link, in decreasing link name order */
!!$    HDmemset(&linfo, 0, sizeof(linfo));
!!$    if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_DEC, n, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder != 0) TEST_ERROR
!!$
!!$    /* Verify the link information for new link, in decreasing link name order */
!!$    HDmemset(&linfo, 0, sizeof(linfo));
!!$    if(H5Lget_info_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_DEC, (hsize_t)0, &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder != (int64_t)n) TEST_ERROR
!!$
!!$    /* Verify value for new soft link, in decreasing link name order */
!!$    if(!hard_link) {
!!$        HDmemset(tmpval, 0, (size_t)NAME_BUF_SIZE);
!!$        if(H5Lget_val_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_DEC, (hsize_t)0, tmpval, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$        if(HDstrcmp(valname, tmpval)) TEST_ERROR
!!$    } /* end if */
!!$
!!$    /* Verify the name for new link, in decreasing link name order */
!!$    HDmemset(tmpname, 0, (size_t)NAME_BUF_SIZE);
!!$    if(H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_DEC, (hsize_t)0, tmpname, (size_t)NAME_BUF_SIZE, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(HDstrcmp(linkname, tmpname)) TEST_ERROR
!!$
!!$    /* Success */
!!$    return(0);
!!$
!!$error:
!!$    /* Failure */
!!$    return(-1);
!!$} /* end link_info_by_idx_check() */

  END SUBROUTINE link_info_by_idx_check


!/*-------------------------------------------------------------------------
! * Function:    test_lcpl
! *
! * Purpose:     Tests Link Creation Property Lists
! *
! * Return:      Success:        0
! *              Failure:        number of errors
! *
! * Programmer:  M.S. Breitenfeld
! *              Modified C routine
! *              March 12, 2008
! *
! * Modifications:
! *
! *-------------------------------------------------------------------------
! */


  SUBROUTINE test_lcpl(fapl, total_error)

  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  INTEGER, INTENT(INOUT) :: total_error
  INTEGER(HID_T), INTENT(IN) :: fapl
    
  INTEGER(HID_T) :: file_id
  INTEGER(HID_T) :: group_id
  INTEGER(HID_T) :: space_id, data_space
  INTEGER(HID_T) :: dset_id
  INTEGER(HID_T) :: type_id
  INTEGER(HID_T) :: lcpl_id
  
  INTEGER :: cset ! Indicates the character set used for the link’s name. 
  INTEGER :: corder ! Specifies the link’s creation order position.
  LOGICAL :: f_corder_valid ! Indicates whether the value in corder is valid.
  INTEGER :: link_type ! Specifies the link class:
     	                              !  H5L_LINK_HARD_F      - Hard link
     	                              !  H5L_LINK_SOFT_F      - Soft link
     	                              !  H5L_LINK_EXTERNAL_F  - External link
     	                              !  H5L_LINK_ERROR _F    - Error
  INTEGER :: address  ! If the link is a hard link, address specifies the file address that the link points to
  INTEGER(HSIZE_T) :: val_size ! If the link is a symbolic link, val_size will be the length of the link value
  INTEGER(HSIZE_T) :: data_size   ! Indicates the size, in the number of characters, of the attribute

  CHARACTER(LEN=1024) :: filename = 'tempfile.h5'
  INTEGER, PARAMETER :: TEST6_DIM1 = 8, TEST6_DIM2 = 7
  INTEGER(HSIZE_T), DIMENSION(1:2), PARAMETER :: dims = (/TEST6_DIM1,TEST6_DIM2/)

  INTEGER :: encoding
  INTEGER :: error
  LOGICAL :: Lexists
  INTEGER(HSIZE_T), DIMENSION(1:2), PARAMETER :: extend_dim = (/TEST6_DIM1-2,TEST6_DIM2-3/)
  INTEGER(HSIZE_T), DIMENSION(1:2) :: dimsout, maxdimsout ! dimensions

  INTEGER :: i

  WRITE(*,*) "link creation property lists (w/new group format)"

  
  !/* Actually, intermediate group creation is tested elsewhere (tmisc).
  ! * Here we only need to test the character encoding property */

  !/* Create file */
  !  h5_fixname(FILENAME[0], fapl, filename, sizeof filename);
  
  CALL H5Fcreate_f(filename, H5F_ACC_TRUNC_F, file_id, error, H5P_DEFAULT_F, fapl)
  CALL check("test_lcpl.H5Fcreate_f", error, total_error)


  ! /* Create and link a group with the default LCPL */
  
  CALL H5Gcreate_f(file_id, "/group", group_id, error)
  CALL check("test_lcpl.H5Gcreate_f", error, total_error)
  

  ! /* Check that its character encoding is the default */
  
  CALL H5Lget_info_f(file_id, "group", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error, H5P_DEFAULT_F)

!/* File-wide default character encoding can not yet be set via the file
! * creation property list and is always ASCII. */
!#define H5F_DEFAULT_CSET H5T_CSET_ASCII  -- FROM H5Fprivate.h --

  CALL VERIFY("test_lcpl.H5Lget_info_f",cset, H5T_CSET_ASCII_F,total_error)

  ! /* Create and commit a datatype with the default LCPL */
  CALL h5tcopy_f(H5T_NATIVE_INTEGER, type_id, error)
  CALL check("test_lcpl.h5tcopy_f",error,total_error)
  CALL h5tcommit_f(file_id, "/type", type_id, error)
  CALL check("test_lcpl.h5tcommit_f", error, total_error)
  CALL h5tclose_f(type_id, error)
  CALL check("test_lcpl.h5tclose_f", error, total_error)
  

  ! /* Check that its character encoding is the default */
  CALL H5Lget_info_f(file_id, "type", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.h5tclose_f", error, total_error)

!/* File-wide default character encoding can not yet be set via the file
! * creation property list and is always ASCII. */
!#define H5F_DEFAULT_CSET H5T_CSET_ASCII  -- FROM H5Fprivate.h --

  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_ASCII_F,total_error)

  !/* Create a dataspace */
  CALL h5screate_simple_f(2, dims, space_id, error)
  CALL check("test_lcpl.h5screate_simple_f",error,total_error)

  ! /* Create a dataset using the default LCPL */
  CALL h5dcreate_f(file_id, "/dataset", H5T_NATIVE_INTEGER, space_id, dset_id, error)
  CALL check("test_lcpl.h5dcreate_f", error, total_error)
  CALL h5dclose_f(dset_id, error)
  CALL check("test_lcpl.h5dclose_f", error, total_error)

  ! Reopen

  CALL H5Dopen_f(file_id, "/dataset", dset_id, error)
  CALL check("test_lcpl.h5dopen_f", error, total_error)

  !  /* Extend the  dataset */
  CALL H5Dset_extent_f(dset_id, extend_dim, error)
  CALL check("test_lcpl.H5Dset_extent_f", error, total_error)
  !  /* Verify the dataspaces */
        !
          !Get dataset's dataspace handle.
          !
  CALL h5dget_space_f(dset_id, data_space, error)
  CALL check("h5dget_space_f",error,total_error)

  CALL h5sget_simple_extent_dims_f(data_space, dimsout, maxdimsout, error)
  CALL check("test_lcpl.h5sget_simple_extent_dims_f",error, total_error)
  
  DO i = 1, 2
     CALL VERIFY("H5Sget_simple_extent_dims", dimsout(i), extend_dim(i), total_error)
     CALL VERIFY("H5Sget_simple_extent_dims", maxdimsout(i), dims(i), total_error)
  ENDDO

  ! /* close data set */

  CALL h5dclose_f(dset_id, error)
  CALL check("test_lcpl.h5dclose_f", error, total_error) 

  ! /* Check that its character encoding is the default */
  CALL H5Lget_info_f(file_id, "dataset", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)

!/* File-wide default character encoding can not yet be set via the file
! * creation property list and is always ASCII. */
!#define H5F_DEFAULT_CSET H5T_CSET_ASCII  -- FROM H5Fprivate.h --

  CALL verify("test_lcpl.h5tclose_f",cset, H5T_CSET_ASCII_F,total_error)

  !/* Create a link creation property list with the UTF-8 character encoding */
  CALL H5Pcreate_f(H5P_LINK_CREATE_F,lcpl_id,error)
  CALL check("test_lcpl.h5Pcreate_f",error,total_error)
  CALL H5Pset_char_encoding_f(lcpl_id, H5T_CSET_UTF8_F, error)
  CALL check("test_lcpl.H5Pset_char_encoding_f",error, total_error)

  ! /* Create and link a group with the new LCPL */
  CALL H5Gcreate_f(file_id, "/group2", group_id, error,lcpl_id=lcpl_id)
  CALL check("test_lcpl.test_lcpl.H5Gcreate_f", error, total_error)
  CALL H5Gclose_f(group_id, error)
  CALL check("test_lcpl.test_lcpl.H5Gclose_f", error, total_error)


  !/* Check that its character encoding is UTF-8 */
  CALL H5Lget_info_f(file_id, "group2", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_UTF8_F,total_error)


  ! /* Create and commit a datatype with the new LCPL */

  CALL h5tcopy_f(H5T_NATIVE_INTEGER, type_id, error)
  CALL check("test_lcpl.h5tcopy_f",error,total_error)
  CALL h5tcommit_f(file_id, "/type2", type_id, error, lcpl_id=lcpl_id)
  CALL check("test_lcpl.h5tcommit_f", error, total_error)
  CALL h5tclose_f(type_id, error)
  CALL check("test_lcpl.h5tclose_f", error, total_error)


  !/* Check that its character encoding is UTF-8 */
  CALL H5Lget_info_f(file_id, "type2", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_UTF8_F,total_error)

  ! /* Create a dataset using the new LCPL */
  CALL h5dcreate_f(file_id, "/dataset2", H5T_NATIVE_INTEGER, space_id, dset_id, error,lcpl_id=lcpl_id)
  CALL check("test_lcpl.h5dcreate_f", error, total_error)

  CALL h5dclose_f(dset_id, error)
  CALL check("test_lcpl.h5dclose_f", error, total_error)

  CALL H5Pget_char_encoding_f(lcpl_id, encoding, error)
  CALL check("test_lcpl.H5Pget_char_encoding_f", error, total_error)
  CALL VERIFY("test_lcpl.H5Pget_char_encoding_f", encoding, H5T_CSET_UTF8_F, total_error) 

  ! /* Check that its character encoding is UTF-8 */
  CALL H5Lget_info_f(file_id, "dataset2", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f2",cset, H5T_CSET_UTF8_F,total_error)

  ! /* Create a new link to the dataset with a different character encoding. */
  CALL H5Pclose_f(lcpl_id, error)
  CALL check("test_lcpl.H5Pclose_f", error, total_error)

  CALL H5Pcreate_f(H5P_LINK_CREATE_F,lcpl_id,error)
  CALL check("test_lcpl.h5Pcreate_f",error,total_error)
  CALL H5Pset_char_encoding_f(lcpl_id, H5T_CSET_ASCII_F, error)
  CALL check("test_lcpl.H5Pset_char_encoding_f",error, total_error)
  CALL H5Lcreate_hard_f(file_id, "/dataset2", file_id, "/dataset2_link", error, lcpl_id)
  CALL check("test_lcpl.H5Lcreate_hard_f",error, total_error)

  CALL H5Lexists_f(file_id,"/dataset2_link",Lexists, error)
  CALL check("test_lcpl.H5Lexists",error, total_error)
  CALL verifylogical("test_lcpl.H5Lexists", Lexists,.TRUE.,total_error)

  ! /* Check that its character encoding is ASCII */
  CALL H5Lget_info_f(file_id, "/dataset2_link", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_ASCII_F,total_error) 

  ! /* Check that the first link's encoding hasn't changed */

  CALL H5Lget_info_f(file_id, "/dataset2", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f3",cset, H5T_CSET_UTF8_F,total_error)


  !/* Make sure that LCPLs work properly for other API calls: */
  !/* H5Lcreate_soft */
  
  CALL H5Pset_char_encoding_f(lcpl_id, H5T_CSET_UTF8_F, error)
  CALL check("test_lcpl.H5Pset_char_encoding_f",error, total_error)
  CALL H5Lcreate_soft_f("test_lcpl.dataset2", file_id, "slink_to_dset2",error,lcpl_id)

  CALL H5Lget_info_f(file_id, "slink_to_dset2", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_UTF8_F,total_error)


  ! /* H5Lmove */
  CALL H5Pset_char_encoding_f(lcpl_id, H5T_CSET_ASCII_F, error)
  CALL check("test_lcpl.H5Pset_char_encoding_f",error, total_error)

  CALL H5Lmove_f(file_id, "slink_to_dset2", file_id, "moved_slink", error, lcpl_id, H5P_DEFAULT_F)
  CALL check("test_lcpl.H5Lmove_f",error, total_error)

  CALL H5Lget_info_f(file_id, "moved_slink", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_ASCII_F,total_error)


  ! /* H5Lcopy */
  
  CALL H5Pset_char_encoding_f(lcpl_id, H5T_CSET_UTF8_F, error)
  CALL check("test_lcpl.H5Pset_char_encoding_f",error, total_error)

  CALL H5Lcopy_f(file_id, "moved_slink", file_id, "copied_slink", error, lcpl_id)
  
  CALL H5Lget_info_f(file_id, "copied_slink", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_UTF8_F,total_error)


  ! /* H5Lcreate_external */

  CALL H5Lcreate_external_f("test_lcpl.filename", "path", file_id, "extlink", error, lcpl_id)
  CALL check("test_lcpl.H5Lcreate_external_f", error, total_error)

  CALL H5Lget_info_f(file_id, "extlink", &
       cset, corder, f_corder_valid, link_type, address, val_size, &
       error)
  CALL check("test_lcpl.H5Lget_info_f", error, total_error)
  CALL verify("test_lcpl.H5Lget_info_f",cset, H5T_CSET_UTF8_F,total_error)


  ! /* Close open IDs */

  CALL H5Pclose_f(lcpl_id, error)
  CALL check("test_lcpl.H5Pclose_f", error, total_error)
  CALL H5Sclose_f(space_id, error)
  CALL check("test_lcpl.h5Sclose_f",error,total_error)
  CALL H5Fclose_f(file_id, error)
  CALL check("test_lcpl.H5Fclose_f", error, total_error)

END SUBROUTINE test_lcpl


SUBROUTINE objcopy(fapl, total_error)

  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  INTEGER, INTENT(INOUT) :: total_error
  INTEGER(HID_T), INTENT(IN) :: fapl

  INTEGER(HID_T) :: fapl2, pid

  INTEGER :: flag, cpy_flags

  INTEGER :: error

  flag = H5O_COPY_SHALLOW_HIERARCHY_F

!/* Copy the file access property list */
  CALL H5Pcopy_f(fapl, fapl2, error)
  CALL check("H5Pcopy_f", error, total_error)

!/* Set the "use the latest version of the format" bounds for creating objects in the file */
  CALL H5Pset_libver_bounds_f(fapl2, H5F_LIBVER_LATEST_F, H5F_LIBVER_LATEST_F, error)
  
  ! /* create property to pass copy options */
  CALL h5pcreate_f(H5P_OBJECT_COPY_F, pid, error)
  CALL check("h5pcreate_f",error, total_error)

  ! /* set options for object copy */
  CALL H5Pset_copy_object_f(pid, flag, error) 
  CALL check("H5Pset_copy_object_f",error, total_error)

  ! /* Verify object copy flags */
  CALL H5Pget_copy_object_f(pid, cpy_flags, error)
  CALL check("H5Pget_copy_object_f",error, total_error)
  CALL VERIFY("H5Pget_copy_object_f", cpy_flags, flag, total_error)

!!$
!!$  CALL test_copy_option(fcpl_src, fcpl_dst, my_fapl, H5O_COPY_WITHOUT_ATTR_FLAG, 
!!$                       FALSE, "H5Ocopy(): without attributes");



  CALL lapl_nlinks(fapl2, total_error)

END SUBROUTINE objcopy

