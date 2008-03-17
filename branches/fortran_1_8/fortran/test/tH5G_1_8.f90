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
SUBROUTINE group_test(cleanup, total_error)
  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  LOGICAL, INTENT(IN)  :: cleanup
  INTEGER, INTENT(OUT) :: total_error

  INTEGER(HID_T) :: fapl, fapl2, my_fapl ! /* File access property lists */
  
  INTEGER :: error

  CALL H5Pcreate_f(H5P_FILE_ACCESS_F, fapl, error)
  CALL check("H5Pcreate_f",error, total_error)

  ! /* Copy the file access property list */
  CALL H5Pcopy_f(fapl, fapl2, error)
  CALL check("H5Pcopy_f",error, total_error)

  ! /* Set the "use the latest version of the format" bounds for creating objects in the file */
  CALL H5Pset_libver_bounds_f(fapl2, H5F_LIBVER_LATEST_F, H5F_LIBVER_LATEST_F, error)
  CALL check("H5Pset_libver_bounds_f",error, total_error)

  ! /* Check for FAPL to USE */
  my_fapl = fapl2

  
  CALL mklinks(fapl2, total_error)
  CALL group_info(fapl2,total_error)
!  CALL ud_hard_links(fapl2,total_error)
  CALL timestamps(fapl2, total_error)
  CALL test_move_preserves(fapl2, total_error)
  CALL delete_by_idx(fapl2, total_error)
  CALL test_lcpl(fapl, total_error)

  CALL objcopy(fapl, total_error)

END SUBROUTINE group_test

!/*-------------------------------------------------------------------------
! * Function:    group_info
! *
! * Purpose:     Create a group with creation order indices and test querying
! *              group info.
! *
! * Return:      Success:        0
! *              Failure:        -1
! *
! * Programmer:  Adapted from C test routines by
! *              M.S. Breitenfeld
! *              February 18, 2008
! *
! *-------------------------------------------------------------------------
! */

SUBROUTINE group_info(fapl, total_error)

  USE HDF5 ! This module contains all necessary modules 
  
  IMPLICIT NONE
  INTEGER, INTENT(OUT) :: total_error
  INTEGER(HID_T), INTENT(IN) :: fapl

  INTEGER(HID_T) :: gcpl_id ! /* Group creation property list ID */

  INTEGER :: max_compact ! /* Maximum # of links to store in group compactly */ 
  INTEGER :: min_dense ! /* Minimum # of links to store in group "densely" */

  INTEGER :: idx_type ! /* Type of index to operate on */
  INTEGER :: order, iorder   ! /* Order within in the index */
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./) ! /* Use index on creation order values */
  CHARACTER(LEN =9) :: filename = 'links0.h5' ! /* File name */
  INTEGER :: Input1
  INTEGER(HID_T) :: group_id ! /* Group ID */
  INTEGER(HID_T) :: soft_group_id ! /* Group ID for soft links */

  INTEGER :: i ! /* Local index variables */
  INTEGER :: storage_type ! Type of storage for links in group:
                                          ! H5G_STORAGE_TYPE_COMPACT: Compact storage
                                          ! H5G_STORAGE_TYPE_DENSE: Indexed storage
                                          ! H5G_STORAGE_TYPE_SYMBOL_TABLE: Symbol tables, the original HDF5 structure
  INTEGER :: nlinks ! Number of links in group
  INTEGER :: max_corder ! Current maximum creation order value for group 

  INTEGER :: u,v  ! /* Local index variables */
  CHARACTER(LEN=2) :: chr2
  INTEGER(HID_T) :: group_id2, group_id3 ! /* Group IDs */
  CHARACTER(LEN=7) :: objname ! /* Object name */
  CHARACTER(LEN=7) :: objname2 ! /* Object name */
  CHARACTER(LEN=19) :: valname !  /* Link value */
  CHARACTER(LEN=12), PARAMETER :: CORDER_GROUP_NAME = "corder_group"
  CHARACTER(LEN=17), PARAMETER :: CORDER_SOFT_GROUP_NAME =  "corder_soft_group"
  INTEGER(HID_T) :: file_id ! /* File ID */
  INTEGER :: error ! /* Generic return value */

  ! /* Create group creation property list */
  CALL H5Pcreate_f(H5P_GROUP_CREATE_F, gcpl_id, error )
  CALL check("H5Pcreate_f", error, total_error)

  ! /* Query the group creation properties */
  CALL H5Pget_link_phase_change_f(gcpl_id, max_compact, min_dense, error)
  CALL check("H5Pget_link_phase_change_f", error, total_error)

  ! /* Loop over operating on different indices on link fields */
  DO idx_type = H5_INDEX_NAME_F, H5_INDEX_CRT_ORDER_F
     ! /* Loop over operating in different orders */
     DO iorder = H5_ITER_INC_F,  H5_ITER_NATIVE_F
        ! /* Loop over using index for creation order value */
        DO i = 1, 2
           ! /* Print appropriate test message */
           IF(idx_type == H5_INDEX_CRT_ORDER_F)THEN
              IF(iorder == H5_ITER_INC_F)THEN
                 order = H5_ITER_INC_F
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"query group info by creation order index in increasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"query group info by creation order index in increasing order w/o creation order index"
                 ENDIF
              ELSE IF (iorder == H5_ITER_DEC_F) THEN
                 order = H5_ITER_DEC_F
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"query group info by creation order index in decreasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"query group info by creation order index in decreasing order w/o creation order index"
                 ENDIF
              ELSE
                 order = H5_ITER_NATIVE_F
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"query group info by creation order index in native order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"query group info by creation order index in native order w/o creation order index"
                 ENDIF
              ENDIF
           ELSE
              IF(iorder == H5_ITER_INC_F)THEN
                 order = H5_ITER_INC_F
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"query group info by creation order index in increasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"query group info by creation order index in increasing order w/o creation order index"
                 ENDIF
              ELSE IF (iorder == H5_ITER_DEC_F) THEN
                 order = H5_ITER_DEC_F
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"query group info by creation order index in decreasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"query group info by creation order index in decreasing order w/o creation order index"
                 ENDIF
              ELSE
                 order = H5_ITER_NATIVE_F
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A)')"query group info by creation order index in native order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A)')"query group info by creation order index in native order w/o creation order index"
                 ENDIF
              ENDIF
           END IF

           ! /* Create file */
           CALL H5Fcreate_f(filename, H5F_ACC_TRUNC_F, file_id, error, H5P_DEFAULT_F, fapl)
           CALL check("H5Fcreate_f", error, total_error)

           ! /* Set creation order tracking & indexing on group */
           IF(use_index(i))THEN
              Input1 = H5P_CRT_ORDER_INDEXED_F
           ELSE
              Input1 = 0
           ENDIF
           CALL H5Pset_link_creation_order_f(gcpl_id, IOR(H5P_CRT_ORDER_TRACKED_F, Input1), error)
           CALL check("H5Pset_link_creation_order_f", error, total_error)

           ! /* Create group with creation order tracking on */
           CALL H5Gcreate_f(file_id, CORDER_GROUP_NAME, group_id, error, gcpl_id=gcpl_id)
           CALL check("H5Gcreate_f", error, total_error)

           ! /* Create group with creation order tracking on for soft links */
           CALL H5Gcreate_f(file_id, CORDER_SOFT_GROUP_NAME, soft_group_id, error, &
                OBJECT_NAMELEN_DEFAULT_F, H5P_DEFAULT_F, gcpl_id)
           CALL check("H5Gcreate_f", error, total_error)

           ! /* Check for out of bound query by index on empty group, should fail */
           CALL H5Gget_info_by_idx_f(group_id, ".", H5_INDEX_NAME_F, order, INT(0,HSIZE_T), &
                storage_type, nlinks, max_corder, error)
           CALL VERIFY("H5Gget_info_by_idx", error, -1, total_error)

           ! /* Create several links, up to limit of compact form */
           DO u = 0, max_compact-1

              ! /* Make name for link */
              WRITE(chr2,'(I2.2)') u
              objname = 'fill '//chr2

              ! /* Create hard link, with group object */
              CALL H5Gcreate_f(group_id, objname, group_id2, error, OBJECT_NAMELEN_DEFAULT_F, H5P_DEFAULT_F, gcpl_id)
              CALL check("H5Gcreate_f", error, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_f(group_id2, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_f", error, total_error)

              ! /* Check (new/empty) group's information */
              CALL VERIFY("H5Gget_info_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f", max_corder, 0, total_error)
              CALL VERIFY("H5Gget_info_f", nlinks, 0, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id, objname, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name", error, total_error)

              ! /* Check (new/empty) group's information */
              CALL VERIFY("H5Gget_info_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f", max_corder, 0, total_error)
              CALL VERIFY("H5Gget_info_f", nlinks, 0, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id2, ".", storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name", error, total_error)

              ! /* Check (new/empty) group's information */
              CALL VERIFY("H5Gget_info_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f", max_corder, 0, total_error)
              CALL VERIFY("H5Gget_info_f", nlinks, 0, total_error)

              ! /* Create objects in new group created */
              DO v = 0, u
                 ! /* Make name for link */
                 WRITE(chr2,'(I2.2)') v
                 objname2 = 'fill '//chr2

                 ! /* Create hard link, with group object */
                 CALL H5Gcreate_f(group_id2, objname2, group_id3, error )
                 CALL check("H5Gcreate_f", error, total_error)

                 ! /* Close group created */
                 CALL H5Gclose_f(group_id3, error)
                 CALL check("H5Gclose_f", error, total_error)
              ENDDO

              ! /* Retrieve group's information */
              CALL H5Gget_info_f(group_id2, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_f", error, total_error)

              ! /* Check (new) group's information */
              CALL VERIFY("H5Gget_info_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_f", nlinks, u+1, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id, objname, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name_f", error, total_error)

              ! /* Check (new) group's information */
              CALL VERIFY("H5Gget_info_by_name_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_by_name_f",max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", nlinks, u+1, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id2, ".", storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name_f", error, total_error)

              ! /* Check (new) group's information */
              CALL VERIFY("H5Gget_info_by_name_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_by_name_f2", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", nlinks, u+1, total_error)

              ! /* Retrieve group's information */
              IF(order.NE.H5_ITER_NATIVE_F)THEN
                 IF(order.EQ.H5_ITER_INC_F) THEN
                    CALL H5Gget_info_by_idx_f(group_id, ".", idx_type, order, INT(u,HSIZE_T), &
                         storage_type, nlinks, max_corder, error,lapl_id=H5P_DEFAULT_F)
                    CALL check("H5Gget_info_by_idx_f", error, total_error)
                 ELSE
                    CALL H5Gget_info_by_idx_f(group_id, ".", idx_type, order, INT(0,HSIZE_T), &
                         storage_type, nlinks, max_corder, error)
                    CALL check("H5Gget_info_by_idx_f", error, total_error)
                 ENDIF
              ! /* Check (new) group's information */
                 CALL VERIFY("H5Gget_info_by_idx_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
                 CALL VERIFY("H5Gget_info_by_idx_f33", max_corder, u+1, total_error)
                 CALL VERIFY("H5Gget_info_by_idx_f", nlinks, u+1, total_error)
              ENDIF
              ! /* Close group created */
              CALL H5Gclose_f(group_id2, error)
              CALL check("H5Gclose_f", error, total_error)
              
              ! /* Retrieve main group's information */
              CALL H5Gget_info_f(group_id, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_f", error, total_error)

              ! /* Check main group's information */
              CALL VERIFY("H5Gget_info_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f2", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_f", nlinks, u+1, total_error)
              
              ! /* Retrieve main group's information, by name */
              CALL H5Gget_info_by_name_f(file_id, CORDER_GROUP_NAME, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name_f", error, total_error)
              
              ! /* Check main group's information */
              CALL VERIFY("H5Gget_info_by_name_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", nlinks, u+1, total_error)

              ! /* Retrieve main group's information, by name */
              CALL H5Gget_info_by_name_f(group_id, ".", storage_type, nlinks, max_corder, error, H5P_DEFAULT_F)
              CALL check("H5Gget_info_by_name_f", error, total_error)

              ! /* Check main group's information */
              CALL VERIFY("H5Gget_info_by_name_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", nlinks, u+1, total_error)

              ! /* Create soft link in another group, to objects in main group */
              valname = CORDER_GROUP_NAME//objname

              CALL H5Lcreate_soft_f(valname, soft_group_id, objname, error, H5P_DEFAULT_F, H5P_DEFAULT_F)
              
              ! /* Retrieve soft link group's information, by name */
              CALL H5Gget_info_f(soft_group_id, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_f", error, total_error)

              ! /* Check soft link group's information */
              CALL VERIFY("H5Gget_info_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_f", nlinks, u+1, total_error)
           ENDDO

           ! /* Verify state of group (compact) */
           ! if(H5G_has_links_test(group_id, NULL) != TRUE) TEST_ERROR

           !/* Check for out of bound query by index */
           !     H5E_BEGIN_TRY {
           !         ret = H5Gget_info_by_idx(group_id, ".", H5_INDEX_NAME, order, (hsize_t)u, &grp_info, H5P_DEFAULT);
           !     } H5E_END_TRY;
           !     if(ret >= 0) TEST_ERROR

           ! /* Create more links, to push group into dense form */
!!$                for(; u < (max_compact * 2); u++) {
!!$                    hid_t group_id2, group_id3;	        /* Group IDs */
!!$
!!$                    /* Make name for link */
!!$                    sprintf(objname, "filler %02u", u);
!!$
!!$                    /* Create hard link, with group object */
!!$                    if((group_id2 = H5Gcreate2(group_id, objname, H5P_DEFAULT, gcpl_id, H5P_DEFAULT)) < 0) TEST_ERROR
!!$
!!$
!!$                    /* Retrieve group's information */
!!$                    if(H5Gget_info(group_id2, &grp_info) < 0) TEST_ERROR
!!$
!!$                    /* Check (new/empty) group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_COMPACT) TEST_ERROR
!!$                    if(grp_info.max_corder != 0) TEST_ERROR
!!$                    if(grp_info.nlinks != 0) TEST_ERROR
!!$
!!$                    /* Retrieve group's information, by name */
!!$                    if(H5Gget_info_by_name(group_id, objname, &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Check (new/empty) group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_COMPACT) TEST_ERROR
!!$                    if(grp_info.max_corder != 0) TEST_ERROR
!!$                    if(grp_info.nlinks != 0) TEST_ERROR
!!$
!!$                    /* Retrieve group's information, by name */
!!$                    if(H5Gget_info_by_name(group_id2, ".", &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Check (new/empty) group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_COMPACT) TEST_ERROR
!!$                    if(grp_info.max_corder != 0) TEST_ERROR
!!$                    if(grp_info.nlinks != 0) TEST_ERROR
!!$
!!$
!!$                    /* Create objects in new group created */
!!$                    for(v = 0; v <= u; v++) {
!!$                        /* Make name for link */
!!$                        sprintf(objname2, "filler %02u", v);
!!$
!!$                        /* Create hard link, with group object */
!!$                        if((group_id3 = H5Gcreate2(group_id2, objname2, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) TEST_ERROR
!!$
!!$                        /* Close group created */
!!$                        if(H5Gclose(group_id3) < 0) TEST_ERROR
!!$                    } /* end for */
!!$
!!$
!!$                    /* Retrieve group's information */
!!$                    if(H5Gget_info(group_id2, &grp_info) < 0) TEST_ERROR
!!$
!!$                    /* Check (new) group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                    if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                    if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$
!!$                    /* Retrieve group's information, by name */
!!$                    if(H5Gget_info_by_name(group_id, objname, &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Check (new) group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                    if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                    if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$
!!$                    /* Retrieve group's information, by name */
!!$                    if(H5Gget_info_by_name(group_id2, ".", &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Check (new) group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                    if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                    if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$
!!$
!!$                    /* Retrieve group's information */
!!$                    if(order != H5_ITER_NATIVE) {
!!$                        if(order == H5_ITER_INC) {
!!$                            if(H5Gget_info_by_idx(group_id, ".", idx_type, order, (hsize_t)u, &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$                        } /* end if */
!!$                        else {
!!$                            if(H5Gget_info_by_idx(group_id, ".", idx_type, order, (hsize_t)0, &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$                        } /* end else */
!!$
!!$                        /* Check (new) group's information */
!!$                        if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                        if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                        if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$                    } /* end if */
!!$
!!$                    /* Close group created */
!!$                    if(H5Gclose(group_id2) < 0) TEST_ERROR
!!$
!!$
!!$                    /* Retrieve main group's information */
!!$                    if(H5Gget_info(group_id, &grp_info) < 0) TEST_ERROR
!!$
!!$                    /* Check main group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                    if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                    if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$
!!$                    /* Retrieve main group's information, by name */
!!$                    if(H5Gget_info_by_name(file_id, CORDER_GROUP_NAME, &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Check main group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                    if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                    if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$
!!$                    /* Retrieve main group's information, by name */
!!$                    if(H5Gget_info_by_name(group_id, ".", &grp_info, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Check main group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                    if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                    if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$
!!$
!!$                    /* Create soft link in another group, to objects in main group */
!!$                    sprintf(valname, "/%s/%s", CORDER_GROUP_NAME, objname);
!!$                    if(H5Lcreate_soft(valname, soft_group_id, objname, H5P_DEFAULT, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$                    /* Retrieve soft link group's information, by name */
!!$                    if(H5Gget_info(soft_group_id, &grp_info) < 0) TEST_ERROR
!!$
!!$                    /* Check soft link group's information */
!!$                    if(grp_info.storage_type != H5G_STORAGE_TYPE_DENSE) TEST_ERROR
!!$                    if(grp_info.max_corder != (int64_t)(u + 1)) TEST_ERROR
!!$                    if(grp_info.nlinks != (hsize_t)(u + 1)) TEST_ERROR
!!$                } /* end for */
!!$
!!$                /* Verify state of group (dense) */
!!$                if(H5G_is_new_dense_test(group_id) != TRUE) TEST_ERROR
!!$
!!$                /* Check for out of bound query by index */
!!$                H5E_BEGIN_TRY {
!!$                    ret = H5Gget_info_by_idx(group_id, ".", H5_INDEX_NAME, order, (hsize_t)u, &grp_info, H5P_DEFAULT);
!!$                } H5E_END_TRY;
!!$                if(ret >= 0) TEST_ERROR


           ! /* Close the groups */

              CALL H5Gclose_f(group_id, error)
              CALL check("H5Gclose_f", error, total_error)
              CALL H5Gclose_f(soft_group_id, error)
              CALL check("H5Gclose_f", error, total_error)
           
              ! /* Close the file */
              CALL H5Fclose_f(file_id, error)
              CALL check("H5Fclose_f", error, total_error)
           ENDDO
        ENDDO
     ENDDO

     ! /* Free resources */
     CALL H5Pclose_f(gcpl_id, error)
     CALL check("H5Pclose_f", error, total_error)

   END SUBROUTINE group_info

!/*-------------------------------------------------------------------------
! * Function:    timestamps
! *
! * Purpose:     Verify that disabling tracking timestamps for an object
! *              works correctly
! *
! *
! * Programmer:  M.S. Breitenfeld
! *              February 20, 2008
! *
! *-------------------------------------------------------------------------
! */

   SUBROUTINE timestamps(fapl, total_error)

     USE HDF5 ! This module contains all necessary modules 
     
     IMPLICIT NONE
     INTEGER, INTENT(OUT) :: total_error
     INTEGER(HID_T), INTENT(IN) :: fapl

     INTEGER(HID_T) :: file_id !/* File ID */
     INTEGER(HID_T) :: group_id !/* Group ID */
     INTEGER(HID_T) :: group_id2 !/* Group ID */
     INTEGER(HID_T) :: gcpl_id !/* Group creation property list ID */
     INTEGER(HID_T) :: gcpl_id2 !/* Group creation property list ID */
     CHARACTER(LEN =9) :: filename = 'links9.h5' ! /* File name */
     ! /* Timestamp macros */
     CHARACTER(LEN=10), PARAMETER :: TIMESTAMP_GROUP_1="timestamp1"
     CHARACTER(LEN=10), PARAMETER :: TIMESTAMP_GROUP_2="timestamp2"
     LOGICAL :: track_times

     INTEGER :: error

     ! /* Print test message */
     WRITE(*,*) "timestamps on objects"

     ! /* Create group creation property list */
     CALL H5Pcreate_f(H5P_GROUP_CREATE_F, gcpl_id, error )
     CALL check("H5Pcreate_f", error, total_error)

     ! /* Query the object timestamp setting */
     CALL H5Pget_obj_track_times_f(gcpl_id, track_times, error)
     CALL check("H5Pget_obj_track_times_f", error, total_error)

     !/* Check default timestamp information */
     CALL VerifyLogical("H5Pget_obj_track_times",track_times,.TRUE.,total_error)

     ! /* Set a non-default object timestamp setting */
     CALL H5Pset_obj_track_times_f(gcpl_id, .FALSE., error)
     CALL check("H5Pset_obj_track_times_f", error, total_error)

     ! /* Query the object timestamp setting */
     CALL H5Pget_obj_track_times_f(gcpl_id, track_times, error)
     CALL check("H5Pget_obj_track_times_f", error, total_error)

     ! /* Check default timestamp information */
     CALL VerifyLogical("H5Pget_obj_track_times",track_times,.FALSE.,total_error)

     ! /* Create file */
     !h5_fixname(FILENAME[0], fapl, filename, sizeof filename);
     
     CALL H5Fcreate_f(filename, H5F_ACC_TRUNC_F, file_id, error, H5P_DEFAULT_F, fapl)
     CALL check("h5fcreate_f",error,total_error)

    ! /* Create group with non-default object timestamp setting */
     CALL h5gcreate_f(file_id, TIMESTAMP_GROUP_1, group_id, error, &
          OBJECT_NAMELEN_DEFAULT_F, H5P_DEFAULT_F, gcpl_id, H5P_DEFAULT_F)
     CALL check("h5fcreate_f",error,total_error)

    ! /* Close the group creation property list */
     CALL H5Pclose_f(gcpl_id, error)
     CALL check("H5Pclose_f", error, total_error)

    ! /* Create group with default object timestamp setting */
     CALL h5gcreate_f(file_id, TIMESTAMP_GROUP_2, group_id2, error, &
          OBJECT_NAMELEN_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F)
     CALL check("h5fcreate_f",error,total_error)

    ! /* Retrieve the new groups' creation properties */
     CALL H5Gget_create_plist_f(group_id, gcpl_id, error)
     CALL check("H5Gget_create_plist", error, total_error)
     CALL H5Gget_create_plist_f(group_id2, gcpl_id2, error)
     CALL check("H5Gget_create_plist", error, total_error)

    ! /* Query & verify the object timestamp settings */
     CALL H5Pget_obj_track_times_f(gcpl_id, track_times, error)
     CALL check("H5Pget_obj_track_times_f", error, total_error)
     CALL VerifyLogical("H5Pget_obj_track_times1",track_times,.FALSE.,total_error)
     CALL H5Pget_obj_track_times_f(gcpl_id2, track_times, error)
     CALL check("H5Pget_obj_track_times_f", error, total_error)
     CALL VerifyLogical("H5Pget_obj_track_times2",track_times,.TRUE.,total_error)
     
!    /* Query the object information for each group */
!    if(H5Oget_info(group_id, &oinfo) < 0) TEST_ERROR
!    if(H5Oget_info(group_id2, &oinfo2) < 0) TEST_ERROR

!!$    /* Sanity check object information for each group */
!!$    if(oinfo.atime != 0) TEST_ERROR
!!$    if(oinfo.mtime != 0) TEST_ERROR
!!$    if(oinfo.ctime != 0) TEST_ERROR
!!$    if(oinfo.btime != 0) TEST_ERROR
!!$    if(oinfo.atime == oinfo2.atime) TEST_ERROR
!!$    if(oinfo.mtime == oinfo2.mtime) TEST_ERROR
!!$    if(oinfo.ctime == oinfo2.ctime) TEST_ERROR
!!$    if(oinfo.btime == oinfo2.btime) TEST_ERROR
!!$    if((oinfo.hdr.flags & H5O_HDR_STORE_TIMES) != 0) TEST_ERROR
!!$    if((oinfo2.hdr.flags & H5O_HDR_STORE_TIMES) == 0) TEST_ERROR
!!$    if(oinfo.hdr.space.total >= oinfo2.hdr.space.total) TEST_ERROR
!!$    if(oinfo.hdr.space.meta >= oinfo2.hdr.space.meta) TEST_ERROR

     ! /* Close the property lists */
     CALL H5Pclose_f(gcpl_id, error)
     CALL check("H5Pclose_f", error, total_error)
     CALL H5Pclose_f(gcpl_id2, error)
     CALL check("H5Pclose_f", error, total_error)

     ! /* Close the groups */
     CALL H5Gclose_f(group_id, error)
     CALL check("H5Gclose_f", error, total_error)
     CALL H5Gclose_f(group_id2, error)
     CALL check("H5Gclose_f", error, total_error)

     !/* Close the file */
     CALL H5Fclose_f(file_id, error)
     CALL check("H5Fclose_f", error, total_error)

     !/* Re-open the file */

     CALL h5fopen_f(FileName, H5F_ACC_RDONLY_F, file_id, error, H5P_DEFAULT_F)
     CALL check("h5fopen_f",error,total_error)

     !/* Open groups */
     CALL H5Gopen_f(file_id, TIMESTAMP_GROUP_1, group_id, error) ! with no optional param.
     CALL check("H5Gopen_f", error, total_error)
     CALL H5Gopen_f(file_id, TIMESTAMP_GROUP_2, group_id2, error, H5P_DEFAULT_F) ! with optional param.
     CALL check("H5Gopen_f", error, total_error)

    ! /* Retrieve the new groups' creation properties */
     CALL H5Gget_create_plist_f(group_id, gcpl_id, error)
     CALL check("H5Gget_create_plist", error, total_error)
     CALL H5Gget_create_plist_f(group_id2, gcpl_id2, error)
     CALL check("H5Gget_create_plist", error, total_error)

    ! /* Query & verify the object timestamp settings */

     CALL H5Pget_obj_track_times_f(gcpl_id, track_times, error)
     CALL check("H5Pget_obj_track_times_f", error, total_error)
     CALL VerifyLogical("H5Pget_obj_track_times1",track_times,.FALSE.,total_error)
     CALL H5Pget_obj_track_times_f(gcpl_id2, track_times, error)
     CALL check("H5Pget_obj_track_times_f", error, total_error)
     CALL VerifyLogical("H5Pget_obj_track_times2",track_times,.TRUE.,total_error)
!!$
!!$    /* Query the object information for each group */
!!$    if(H5Oget_info(group_id, &oinfo) < 0) TEST_ERROR
!!$    if(H5Oget_info(group_id2, &oinfo2) < 0) TEST_ERROR
!!$
!!$    /* Sanity check object information for each group */
!!$    if(oinfo.atime != 0) TEST_ERROR
!!$    if(oinfo.mtime != 0) TEST_ERROR
!!$    if(oinfo.ctime != 0) TEST_ERROR
!!$    if(oinfo.btime != 0) TEST_ERROR
!!$    if(oinfo.atime == oinfo2.atime) TEST_ERROR
!!$    if(oinfo.mtime == oinfo2.mtime) TEST_ERROR
!!$    if(oinfo.ctime == oinfo2.ctime) TEST_ERROR
!!$    if(oinfo.btime == oinfo2.btime) TEST_ERROR
!!$    if((oinfo.hdr.flags & H5O_HDR_STORE_TIMES) != 0) TEST_ERROR
!!$    if((oinfo2.hdr.flags & H5O_HDR_STORE_TIMES) == 0) TEST_ERROR
!!$    if(oinfo.hdr.space.total >= oinfo2.hdr.space.total) TEST_ERROR
!!$    if(oinfo.hdr.space.meta >= oinfo2.hdr.space.meta) TEST_ERROR

     ! /* Close the property lists */
     CALL H5Pclose_f(gcpl_id, error)
     CALL check("H5Pclose_f", error, total_error)
     CALL H5Pclose_f(gcpl_id2, error)
     CALL check("H5Pclose_f", error, total_error)

     ! /* Close the groups */
     CALL H5Gclose_f(group_id, error)
     CALL check("H5Gclose_f", error, total_error)
     CALL H5Gclose_f(group_id2, error)
     CALL check("H5Gclose_f", error, total_error)

     !/* Close the file */
     CALL H5Fclose_f(file_id, error)
     CALL check("H5Fclose_f", error, total_error)

   END SUBROUTINE timestamps

!/*-------------------------------------------------------------------------
! * Function:	mklinks
! *
! * Purpose:	Build a file with assorted links.
! *
! *
! * Programmer:	Adapted from C test by:
! *             M.S. Breitenfeld
! *
! * Modifications:
! *
! *-------------------------------------------------------------------------
! */

   SUBROUTINE mklinks(fapl, total_error)

     USE HDF5 ! This module contains all necessary modules 
     
     IMPLICIT NONE
     INTEGER, INTENT(OUT) :: total_error
     INTEGER(HID_T), INTENT(IN) :: fapl

     INTEGER(HID_T) :: file, scalar, grp, d1
     CHARACTER(LEN=12), PARAMETER :: filename ='TestLinks.h5'
     INTEGER(HSIZE_T), DIMENSION(1) :: adims2 = (/1/) ! Attribute dimension
     INTEGER ::   arank = 1                      ! Attribure rank
     INTEGER :: error

     WRITE(*,*) "link creation (w/new group format)"

     ! /* Create a file */
     CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, file, error, H5P_DEFAULT_F, fapl)
     CALL check("h5fcreate_f",error,total_error)
     CALL h5screate_simple_f(arank, adims2, scalar, error)
     CALL check("h5screate_simple_f",error,total_error)

     !/* Create a group */
     CALL H5Gcreate_f(file, "grp1", grp, error) 
     CALL check("H5Gcreate_f", error, total_error)
     CALL H5Gclose_f(grp, error)
     CALL check("h5gclose_f",error,total_error)

     !/* Create a dataset */
     CALL h5dcreate_f(file, "d1", H5T_NATIVE_INTEGER, scalar, d1, error)
     CALL check("h5dcreate_f",error,total_error)
     CALL h5dclose_f(d1, error)
     CALL check("h5dclose_f",error,total_error)

     !/* Create a hard link */
     CALL H5Lcreate_hard_f(file, "d1", INT(H5L_SAME_LOC_F,HID_T), "grp1/hard", error)
     CALL check("H5Lcreate_hard_f", error, total_error)
     
     !/* Create a symbolic link */
     CALL H5Lcreate_soft_f("/d1", file, "grp1/soft",error)
     CALL check("H5Lcreate_soft_f", error, total_error)

    !/* Create a symbolic link to something that doesn't exist */

     CALL H5Lcreate_soft_f("foobar", file, "grp1/dangle",error)

    !/* Create a recursive symbolic link */
     CALL H5Lcreate_soft_f("/grp1/recursive", file, "/grp1/recursive",error)

    !/* Close */
     CALL h5sclose_f(scalar, error)
     CALL check("h5sclose_f",error,total_error)
     CALL h5fclose_f(file, error)
     CALL check("h5fclose_f",error,total_error)

  END SUBROUTINE mklinks

!/*-------------------------------------------------------------------------
! * Function:    test_move_preserves
! *
! * Purpose:     Tests that moving and renaming links preserves their
! *              properties.
! *
! * Programmer:  M.S. Breitenfeld
! *              March 3, 2008
! *
! * Modifications:
! *
! *-------------------------------------------------------------------------
! */

  SUBROUTINE test_move_preserves(fapl_id, total_error)

    USE HDF5 ! This module contains all necessary modules 
    
    IMPLICIT NONE
    INTEGER, INTENT(OUT) :: total_error
    INTEGER(HID_T), INTENT(IN) :: fapl_id

    INTEGER(HID_T):: file_id
    INTEGER(HID_T):: group_id
    INTEGER(HID_T):: fcpl_id ! /* Group creation property list ID */
    INTEGER(HID_T):: lcpl_id
    INTEGER(HID_T):: lcpl2_id
    !H5O_info_t oinfo;
    !H5L_info_t linfo;
    INTEGER :: old_cset
    INTEGER :: old_corder
    !H5T_cset_t old_cset;
    !int64_t old_corder;         /* Creation order value of link */
    !time_t old_modification_time;
    !time_t curr_time;
    !unsigned crt_order_flags;   /* Status of creation order info for GCPL */
    !char filename[1024];

    INTEGER :: crt_order_flags ! /* Status of creation order info for GCPL */
    CHARACTER(LEN=9), PARAMETER :: filename = 'testmp.h5'

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

    INTEGER :: error

    WRITE(*,*) "moving and copying links preserves their properties (w/new group format)"

    !/* Create a file creation property list with creation order stored for links
    ! * in the root group
    ! */

    CALL H5Pcreate_f(H5P_FILE_CREATE_F, fcpl_id, error)
    CALL check("H5Pcreate_f",error, total_error)

    CALL H5Pget_link_creation_order_f(fcpl_id, crt_order_flags, error)
    CALL check("H5Pget_link_creation_order_f",error, total_error)
    CALL VERIFY("H5Pget_link_creation_order_f",crt_order_flags,0, total_error)
    
    CALL H5Pset_link_creation_order_f(fcpl_id, H5P_CRT_ORDER_TRACKED_F, error)
    CALL check("H5Pset_link_creation_order_f", error, total_error)
 
    CALL H5Pget_link_creation_order_f(fcpl_id, crt_order_flags, error)
    CALL check("H5Pget_link_creation_order_f",error, total_error)
    CALL VERIFY("H5Pget_link_creation_order_f",crt_order_flags, H5P_CRT_ORDER_TRACKED_F, total_error)

    !/* Create file */
    !/* (with creation order tracking for the root group) */
    
    CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, file_id, error, fcpl_id, fapl_id)
    CALL check("h5fcreate_f",error,total_error)

    !/* Create a link creation property list with the UTF-8 character encoding */
    CALL H5Pcreate_f(H5P_LINK_CREATE_F, lcpl_id, error)
    CALL check("H5Pcreate_f",error, total_error)

    CALL H5Pset_char_encoding_f(lcpl_id, H5T_CSET_UTF8_F, error)
    CALL check("H5Pset_char_encoding_f",error, total_error)

    !/* Create a group with that lcpl */
    CALL H5Gcreate_f(file_id, "group", group_id, error,lcpl_id=lcpl_id, gcpl_id=H5P_DEFAULT_F, gapl_id=H5P_DEFAULT_F) 
    CALL check("H5Gcreate_f", error, total_error)
    CALL H5Gclose_f(group_id, error)

    ! /* Get the group's link's information */
    CALL H5Lget_info_f(file_id, "group", &
         cset, corder, f_corder_valid, link_type, address, val_size, &
         error, H5P_DEFAULT_F)
    CALL check("H5Lget_info_f",error,total_error)

!    if(H5Oget_info_by_name(file_id, "group", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR

    old_cset = cset
    CALL VERIFY("H5Lget_info_f",old_cset,H5T_CSET_UTF8_F,total_error)
    CALL VerifyLogical("H5Lget_info_f",f_corder_valid,.TRUE.,total_error)
    old_corder = corder;
    CALL VERIFY("H5Lget_info_f",old_corder,0,total_error)

!    old_modification_time = oinfo.mtime;

!    /* If this test happens too quickly, the times will all be the same.  Make sure the time changes. */
!    curr_time = HDtime(NULL);
!    while(HDtime(NULL) <= curr_time)
!        ;

!    /* Close the file and reopen it */
    CALL H5Fclose_f(file_id, error)
    CALL check("H5Fclose_f", error, total_error)
    
!!$    if((file_id = H5Fopen(filename, H5F_ACC_RDWR, fapl_id)) < 0) TEST_ERROR
!!$
!!$    /* Get the link's character set & modification time .  They should be unchanged */
!!$    if(H5Lget_info(file_id, "group", &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(H5Oget_info_by_name(file_id, "group", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_modification_time != oinfo.mtime) TEST_ERROR
!!$    if(old_cset != linfo.cset) TEST_ERROR
!!$    if(linfo.corder_valid != TRUE) TEST_ERROR
!!$    if(old_corder != linfo.corder) TEST_ERROR
!!$
!!$    /* Create a new link to the group.  It should have a different creation order value but the same modification time */
!!$    if(H5Lcreate_hard(file_id, "group", file_id, "group2", H5P_DEFAULT, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(H5Oget_info_by_name(file_id, "group2", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_modification_time != oinfo.mtime) TEST_ERROR
!!$    if(H5Lget_info(file_id, "group2", &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_corder == linfo.corder) TEST_ERROR
!!$    if(linfo.corder_valid != TRUE) TEST_ERROR
!!$    if(linfo.corder != 1) TEST_ERROR
!!$    if(linfo.cset != H5T_CSET_ASCII) TEST_ERROR
!!$
!!$    /* Copy the first link to a UTF-8 name.
!!$     *  Its creation order value should be different, but modification time
!!$     * should not change.
!!$     */
!!$    if(H5Lcopy(file_id, "group", file_id, "group_copied", lcpl_id, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(H5Oget_info_by_name(file_id, "group_copied", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_modification_time != oinfo.mtime) TEST_ERROR
!!$    if(H5Lget_info(file_id, "group_copied", &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder_valid != TRUE) TEST_ERROR
!!$    if(linfo.corder != 2) TEST_ERROR
!!$
!!$    /* Check that its character encoding is UTF-8 */
!!$    if(linfo.cset != H5T_CSET_UTF8) TEST_ERROR
!!$
!!$    /* Move the link with the default property list. */
!!$    if(H5Lmove(file_id, "group_copied", file_id, "group_copied2", H5P_DEFAULT, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(H5Oget_info_by_name(file_id, "group_copied2", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_modification_time != oinfo.mtime) TEST_ERROR
!!$    if(H5Lget_info(file_id, "group_copied2", &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder_valid != TRUE) TEST_ERROR
!!$    if(linfo.corder != 3) TEST_ERROR
!!$
!!$    /* Check that its character encoding is not UTF-8 */
!!$    if(linfo.cset == H5T_CSET_UTF8) TEST_ERROR
!!$
!!$    /* Check that the original link is unchanged */
!!$    if(H5Oget_info_by_name(file_id, "group", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_modification_time != oinfo.mtime) TEST_ERROR
!!$    if(H5Lget_info(file_id, "group", &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder_valid != TRUE) TEST_ERROR
!!$    if(old_corder != linfo.corder) TEST_ERROR
!!$    if(linfo.cset != H5T_CSET_UTF8) TEST_ERROR
!!$
!!$    /* Move the first link to a UTF-8 name.
!!$     *  Its creation order value will change, but modification time should not
!!$     *  change. */
!!$    if(H5Lmove(file_id, "group", file_id, "group_moved", lcpl_id, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(H5Oget_info_by_name(file_id, "group_moved", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_modification_time != oinfo.mtime) TEST_ERROR
!!$    if(H5Lget_info(file_id, "group_moved", &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder_valid != TRUE) TEST_ERROR
!!$    if(linfo.corder != 4) TEST_ERROR
!!$
!!$    /* Check that its character encoding is UTF-8 */
!!$    if(linfo.cset != H5T_CSET_UTF8) TEST_ERROR
!!$
!!$    /* Move the link again using the default property list. */
!!$    if(H5Lmove(file_id, "group_moved", file_id, "group_moved_again", H5P_DEFAULT, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(H5Oget_info_by_name(file_id, "group_moved_again", &oinfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(old_modification_time != oinfo.mtime) TEST_ERROR
!!$    if(H5Lget_info(file_id, "group_moved_again", &linfo, H5P_DEFAULT) < 0) TEST_ERROR
!!$    if(linfo.corder_valid != TRUE) TEST_ERROR
!!$    if(linfo.corder != 5) TEST_ERROR
!!$
!!$    /* Check that its character encoding is not UTF-8 */
!!$    if(linfo.cset == H5T_CSET_UTF8) TEST_ERROR

    ! /* Close open IDs */
     CALL H5Pclose_f(fcpl_id, error)
     CALL check("H5Pclose_f", error, total_error)
     CALL H5Pclose_f(lcpl_id, error)
     CALL check("H5Pclose_f", error, total_error)
    
    ! if(H5Fclose(file_id) < 0) TEST_ERROR

   END SUBROUTINE test_move_preserves

!!$!/*-------------------------------------------------------------------------
!!$! * Function:    ud_hard_links
!!$! *
!!$! * Purpose:     Check that the functionality of hard links can be duplicated
!!$! *              with user-defined links.
!!$! *
!!$! *
!!$! * Programmer:  M.S. Breitenfeld
!!$! *              February, 2008
!!$! *
!!$! *-------------------------------------------------------------------------
!!$! */
!!$!
!!$!/* Callback functions for UD hard links. */
!!$!/* UD_hard_create increments the object's reference count */
!!$
!!$  SUBROUTINE ud_hard_links(fapl, total_error)
!!$    
!!$    USE HDF5 ! This module contains all necessary modules 
!!$     
!!$    IMPLICIT NONE
!!$    INTEGER, INTENT(OUT) :: total_error
!!$    INTEGER(HID_T), INTENT(IN) :: fapl
!!$
!!$    INTEGER(HID_T) :: fid ! /* File ID */
!!$    INTEGER(HID_T) :: gid ! /* Group IDs */
!!$
!!$    CHARACTER(LEN=10) :: objname = 'objname.h5' ! /* Object name */
!!$    CHARACTER(LEN=10), PARAMETER :: filename = 'filname.h5'
!!$    
!!$    INTEGER(HSIZE_T) :: name_len ! /* Size of an empty file */
!!$
!!$    INTEGER, PARAMETER :: UD_HARD_TYPE=201
!!$    LOGICAL :: registered
!!$
!!$!/* Link information */
!!$
!!$!    ssize_t     name_len;                       /* Length of object name */
!!$!    h5_stat_size_t empty_size;                  /* Size of an empty file */
!!$
!!$
!!$    WRITE(*,*) "user-defined hard link (w/new group format)"
!!$
!!$    ! /* Set up filename and create file*/
!!$
!!$    CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, H5P_DEFAULT_F, fapl)
!!$    CALL check("h5fcreate_f",error,total_error)
!!$    
!!$    ! /* Close file */
!!$    CALL h5fclose_f(fid, error)
!!$    CALL check("h5fclose_f",error,total_error)
!!$    
!!$    ! if((empty_size = h5_get_file_size(filename))<0) TEST_ERROR
!!$
!!$    ! /* Create file */
!!$    CALL h5fcreate_f(FileName, H5F_ACC_TRUNC_F, fid, error, H5P_DEFAULT_F, fapl)
!!$    CALL check("h5fcreate_f",error,total_error)
!!$
!!$    ! /* Check that external links are registered and UD hard links are not */
!!$
!!$    CALL H5Lis_registered(H5L_TYPE_EXTERNAL, registered, error)
!!$    CALL VerifyLogical("H5Lis_registered", registered, .TRUE., total_error)
!!$
!!$    CALL H5Lis_registered(UD_HARD_TYPE, registered, error)
!!$    CALL VerifyLogical("H5Lis_registered", registered, .FALSE., total_error)
!!$
!!$    !/* Register "user-defined hard links" with the library */
!!$!    if(H5Lregister(UD_hard_class) < 0) TEST_ERROR
!!$
!!$    /* Check that UD hard links are now registered */
!!$    if(H5Lis_registered(H5L_TYPE_EXTERNAL) != TRUE) TEST_ERROR
!!$    if(H5Lis_registered(UD_HARD_TYPE) != TRUE) TEST_ERROR
!!$
!!$    /* Create a group for the UD hard link to point to */
!!$    if((gid = H5Gcreate2(fid, "group", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) TEST_ERROR
!!$
!!$    /* Get address for the group to give to the hard link */
!!$    if(H5Lget_info(fid, "group", &li, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$    if(H5Gclose(gid) < 0) TEST_ERROR
!!$
!!$
!!$    /* Create a user-defined "hard link" to the group using the address we got
!!$     * from H5Lget_info */
!!$    if(H5Lcreate_ud(fid, "ud_link", UD_HARD_TYPE, &(li.u.address), sizeof(haddr_t), H5P_DEFAULT, H5P_DEFAULT) < 0) TEST_ERROR
!!$
!!$    /* Close and re-open file to ensure that data is written to disk */
!!$    if(H5Fclose(fid) < 0) TEST_ERROR
!!$    if((fid = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) TEST_ERROR
!!$
!!$    /* Open group through UD link */
!!$    if((gid = H5Gopen2(fid, "ud_link", H5P_DEFAULT)) < 0) FAIL_STACK_ERROR
!!$
!!$    /* Check name */
!!$    if((name_len = H5Iget_name( gid, objname, (size_t)NAME_BUF_SIZE )) < 0) TEST_ERROR
!!$    if(HDstrcmp(objname, "/group")) TEST_ERROR
!!$
!!$    /* Create object in group */
!!$    if((gid2 = H5Gcreate2(gid, "new_group", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) TEST_ERROR
!!$
!!$    /* Close groups*/
!!$    if(H5Gclose(gid2) < 0) TEST_ERROR
!!$    if(H5Gclose(gid) < 0) TEST_ERROR
!!$
!!$    /* Re-open group without using ud link to check that it was created properly */
!!$    if((gid = H5Gopen2(fid, "group/new_group", H5P_DEFAULT)) < 0) FAIL_STACK_ERROR
!!$
!!$    /* Check name */
!!$    if((name_len = H5Iget_name( gid, objname, (size_t)NAME_BUF_SIZE )) < 0) TEST_ERROR
!!$    if(HDstrcmp(objname, "/group/new_group")) TEST_ERROR
!!$
!!$    /* Close opened object */
!!$    if(H5Gclose(gid) < 0) FAIL_STACK_ERROR
!!$
!!$    /* Check that H5Lget_objinfo works on the hard link */
!!$    if(H5Lget_info(fid, "ud_link", &li, H5P_DEFAULT) < 0) FAIL_STACK_ERROR
!!$    /* UD hard links have no query function, thus return a "link length" of 0 */
!!$    if(li.u.val_size != 0) TEST_ERROR
!!$    if(UD_HARD_TYPE != li.type) {
!!$	H5_FAILED();
!!$	puts("    Unexpected link class - should have been a UD hard link");
!!$	goto error;
!!$    } /* end if */
!!$
!!$    /* Unlink the group pointed to by the UD link.  It shouldn't be
!!$     * deleted because of the UD link. */
!!$    if(H5Ldelete(fid, "/group", H5P_DEFAULT) < 0) FAIL_STACK_ERROR
!!$
!!$    /* Ensure we can open the group through the UD link */
!!$    if((gid = H5Gopen2(fid, "ud_link", H5P_DEFAULT)) < 0) FAIL_STACK_ERROR
!!$
!!$    /* Unlink the group contained within it. */
!!$    if(H5Ldelete(gid, "new_group", H5P_DEFAULT) < 0) FAIL_STACK_ERROR
!!$    if(H5Gclose(gid) < 0) FAIL_STACK_ERROR
!!$
!!$    /* Now delete the UD link.  This should cause the group to be
!!$     * deleted, too. */
!!$    if(H5Ldelete(fid, "ud_link", H5P_DEFAULT) < 0) FAIL_STACK_ERROR
!!$
!!$    /* Close file */
!!$    if(H5Fclose(fid) < 0) FAIL_STACK_ERROR
!!$
!!$    /* The file should be empty again. */
!!$    if(empty_size != h5_get_file_size(filename)) TEST_ERROR
!!$
!!$    if(H5Lunregister(UD_HARD_TYPE) < 0) FAIL_STACK_ERROR
!!$
!!$    PASSED();
!!$    return 0;
!!$
!!$ error:
!!$    H5E_BEGIN_TRY {
!!$    	H5Gclose(gid2);
!!$    	H5Gclose(gid);
!!$    	H5Fclose(fid);
!!$    } H5E_END_TRY;
!!$    return -1;
!!$} /* end ud_hard_links() */
