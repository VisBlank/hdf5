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

  CALL group_info(fapl2,total_error)

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
! * Programmer:  M.S. Breitenfeld
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

  INTEGER :: max_compact, min_dense

  INTEGER :: idx_type ! /* Type of index to operate on */
  INTEGER :: order    ! /* Order within in the index */
  LOGICAL, DIMENSION(1:2) :: use_index = (/.FALSE.,.TRUE./) ! /* Use index on creation order values */
  CHARACTER(LEN =9) :: filename = 'links0.h5' 
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
  INTEGER :: error

!!$    hid_t	file_id = (-1); 	/* File ID */
!!$    hid_t	group_id = (-1);	/* Group ID */
!!$    hid_t	soft_group_id = (-1);	/* Group ID for soft links */
!!$    hid_t       gcpl_id = (-1); 	/* Group creation property list ID */
!!$    H5_index_t idx_type;               /* Type of index to operate on */
!!$    H5_iter_order_t order;              /* Order within in the index */
!!$    hbool_t     use_index;              /* Use index on creation order values */
!!$    unsigned    max_compact;            /* Maximum # of links to store in group compactly */
!!$    unsigned    min_dense;              /* Minimum # of links to store in group "densely" */
!!$    H5G_info_t  grp_info;               /* Buffer for querying object's info */
!!$    char        filename[NAME_BUF_SIZE];/* File name */
!!$    char        objname[NAME_BUF_SIZE]; /* Object name */
!!$    char        objname2[NAME_BUF_SIZE]; /* Object name */
!!$    char        valname[NAME_BUF_SIZE]; /* Link value */
!!$    herr_t      ret;                    /* Generic return value */
!!$    unsigned    u, v;                   /* Local index variables */

  ! /* Create group creation property list */
  CALL H5Pcreate_f(H5P_GROUP_CREATE_F, gcpl_id, error )
  CALL check("H5Pcreate_f", error, total_error)

  ! /* Query the group creation properties */
  CALL H5Pget_link_phase_change_f(gcpl_id, max_compact, min_dense, error)
  CALL check("H5Pget_link_phase_change_f", error, total_error)

  ! /* Loop over operating on different indices on link fields */
  DO idx_type = H5_INDEX_NAME_F, H5_INDEX_CRT_ORDER_F
     ! /* Loop over operating in different orders */
     DO order = H5_ITER_INC_F,  H5_ITER_NATIVE_F
        ! /* Loop over using index for creation order value */
        DO i = 1, 2
           
           ! /* Print appropriate test message */
           IF(idx_type == H5_INDEX_CRT_ORDER_F)THEN
              IF(order == H5_ITER_INC_F)THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A83)')"query group info by creation order index in increasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A85)')"query group info by creation order index in increasing order w/o creation order index"
                 ENDIF
              ELSE IF (order == H5_ITER_DEC_F) THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A83)')"query group info by creation order index in decreasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A85)')"query group info by creation order index in decreasing order w/o creation order index"
                 ENDIF
              ELSE
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A79)')"query group info by creation order index in native order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A81)')"query group info by creation order index in native order w/o creation order index"
                 ENDIF
              ENDIF
           ELSE
              IF(order == H5_ITER_INC_F)THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A83)')"query group info by creation order index in increasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A85)')"query group info by creation order index in increasing order w/o creation order index"
                 ENDIF
              ELSE IF (order == H5_ITER_DEC_F) THEN
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A83)')"query group info by creation order index in decreasing order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A85)')"query group info by creation order index in decreasing order w/o creation order index"
                 ENDIF
              ELSE
                 IF(use_index(i))THEN
                    WRITE(*,'(5x,A79)')"query group info by creation order index in native order w/creation order index"
                 ELSE
                    WRITE(*,'(5x,A81)')"query group info by creation order index in native order w/o creation order index"
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

!           PRINT*,'lkjf',IOR(H5P_CRT_ORDER_TRACKED_F, Input1)

           CALL H5Pset_attr_creation_order_f(gcpl_id, IOR(H5P_CRT_ORDER_TRACKED_F, Input1), error)
           CALL check("H5Pset_attr_creation_order_f", error, total_error)

           ! /* Create group WITH creation order tracking on */

           CALL H5Gcreate_f(file_id, CORDER_GROUP_NAME, H5P_DEFAULT_F, gcpl_id, H5P_DEFAULT_F, group_id, error)
           CALL check("H5Gcreate_f", error, total_error)

           ! /* Create group with creation order tracking on for soft links */
           CALL H5Gcreate_f(file_id, CORDER_SOFT_GROUP_NAME, H5P_DEFAULT_F, gcpl_id, H5P_DEFAULT_F, soft_group_id, error)
           CALL check("H5Gcreate_f", error, total_error)

           ! /* Check for out of bound query by index on empty group, should fail */
           CALL H5Gget_info_by_idx_f(group_id, ".", H5_INDEX_NAME_F, order, INT(0,HSIZE_T), H5P_DEFAULT_F, &
                storage_type, nlinks, max_corder, error)
           CALL VERIFY("H5Gget_info_by_idx", error, -1, total_error)

           ! /* Create several links, up to limit of compact form */
           DO u = 0, max_compact-1

              ! /* Make name for link */
              WRITE(chr2,'(I2.2)') u
              objname = 'fill '//chr2

              ! /* Create hard link, with group object */
              CALL H5Gcreate_f(group_id, objname, H5P_DEFAULT_F, gcpl_id, H5P_DEFAULT_F, group_id2, error )
              CALL check("H5Gcreate_f", error, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_f(group_id2, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_f", error, total_error)

              ! /* Check (new/empty) group's information */
              CALL VERIFY("H5Gget_info_f", INT(storage_type), INT(H5G_STORAGE_TYPE_COMPACT_F), total_error)
              CALL VERIFY("H5Gget_info_f", INT(max_corder), 0, total_error)
              CALL VERIFY("H5Gget_info_f", INT(nlinks), 0, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id, objname, H5P_DEFAULT_F, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name", error, total_error)

              ! /* Check (new/empty) group's information */
              CALL VERIFY("H5Gget_info_f", INT(storage_type), INT(H5G_STORAGE_TYPE_COMPACT_F), total_error)
              CALL VERIFY("H5Gget_info_f", INT(max_corder), 0, total_error)
              CALL VERIFY("H5Gget_info_f", INT(nlinks), 0, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id2, ".", H5P_DEFAULT_F, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name", error, total_error)

              ! /* Check (new/empty) group's information */
              CALL VERIFY("H5Gget_info_f", INT(storage_type), INT(H5G_STORAGE_TYPE_COMPACT_F), total_error)
              CALL VERIFY("H5Gget_info_f", INT(max_corder), 0, total_error)
              CALL VERIFY("H5Gget_info_f", INT(nlinks), 0, total_error)

              ! /* Create objects in new group created */
              DO v = 0, u
                 PRINT*,v
                 ! /* Make name for link */
                 WRITE(chr2,'(I2.2)') v
                 objname2 = 'fill '//chr2

                 

                 ! /* Create hard link, with group object */
                 CALL H5Gcreate_f(group_id2, objname2, H5P_DEFAULT_F, H5P_DEFAULT_F, H5P_DEFAULT_F, group_id3, error )
                 CALL check("H5Gcreate_f", error, total_error)

                 ! /* Close group created */
                 CALL H5Gclose_f(group_id3, error)
                 CALL check("H5Gclose_f", error, total_error)
                 
              ENDDO

              ! /* Retrieve group's information */
              CALL H5Gget_info_f(group_id2, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_f", error, total_error)


              ! /* Check (new) group's information */
              PRINT*,storage_type, H5G_STORAGE_TYPE_COMPACT_F
              PRINT*,max_corder, u+1
              PRINT*,nlinks, u+1
              CALL VERIFY("H5Gget_info_f1", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f2", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_f3", nlinks, u+1, total_error)
              stop

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id, objname, H5P_DEFAULT_F, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name_f", error, total_error)

              ! /* Check (new) group's information */
              CALL VERIFY("H5Gget_info_by_name_f1", INT(storage_type), INT(H5G_STORAGE_TYPE_COMPACT_F), total_error)
              CALL VERIFY("H5Gget_info_by_name_f2", INT(max_corder), u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f3", INT(nlinks), u+1, total_error)

              ! /* Retrieve group's information */
              CALL H5Gget_info_by_name_f(group_id2, ".", H5P_DEFAULT_F, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name_f", error, total_error)

              ! /* Check (new) group's information */
              CALL VERIFY("H5Gget_info_by_name_f", INT(storage_type), INT(H5G_STORAGE_TYPE_COMPACT_F), total_error)
              CALL VERIFY("H5Gget_info_by_name_f", INT(max_corder), u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", INT(nlinks), u+1, total_error)

              ! /* Retrieve group's information */
              IF(order.NE.H5_ITER_NATIVE_F)THEN
                 IF(order.EQ.H5_ITER_INC_F) THEN
                    CALL H5Gget_info_by_idx_f(group_id, ".", idx_type, order, INT(u,HSIZE_T), H5P_DEFAULT_F, &
                         storage_type, nlinks, max_corder, error)
                    CALL check("H5Gget_info_by_idx_f", error, total_error)
                 ELSE
                    CALL H5Gget_info_by_idx_f(group_id, ".", idx_type, order, INT(0,HSIZE_T), H5P_DEFAULT_F, &
                         storage_type, nlinks, max_corder, error)
                    CALL check("H5Gget_info_by_idx_f", error, total_error)
                 ENDIF
              ! /* Check (new) group's information */
                 CALL VERIFY("H5Gget_info_by_idx_f", INT(storage_type), (H5G_STORAGE_TYPE_COMPACT_F), total_error)
                 CALL VERIFY("H5Gget_info_by_idx_f", INT(max_corder), u+1, total_error)
                 CALL VERIFY("H5Gget_info_by_idx_f", INT(nlinks), u+1, total_error)
              ENDIF
              ! /* Close group created */
              CALL H5Gclose_f(group_id2, error)
              CALL check("H5Gclose_f", error, total_error)
              
              ! /* Retrieve main group's information */
              CALL H5Gget_info_f(group_id, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_f", error, total_error)

              ! /* Check main group's information */
              CALL VERIFY("H5Gget_info_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_f", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_f", nlinks, u+1, total_error)
              
              ! /* Retrieve main group's information, by name */
              CALL H5Gget_info_by_name_f(file_id, CORDER_GROUP_NAME, H5P_DEFAULT_F, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name_f", error, total_error)
              
              ! /* Check main group's information */
              CALL VERIFY("H5Gget_info_by_name_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", nlinks, u+1, total_error)

              ! /* Retrieve main group's information, by name */
              CALL H5Gget_info_by_name_f(group_id, ".", H5P_DEFAULT_F, storage_type, nlinks, max_corder, error)
              CALL check("H5Gget_info_by_name_f", error, total_error)

              ! /* Check main group's information */
              CALL VERIFY("H5Gget_info_by_name_f", storage_type, H5G_STORAGE_TYPE_COMPACT_F, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", max_corder, u+1, total_error)
              CALL VERIFY("H5Gget_info_by_name_f", nlinks, u+1, total_error)


              ! /* Create soft link in another group, to objects in main group */
              valname = CORDER_GROUP_NAME//objname

              CALL H5Lcreate_soft_f(valname, soft_group_id, objname, H5P_DEFAULT_F, H5P_DEFAULT_F, error)
              
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

