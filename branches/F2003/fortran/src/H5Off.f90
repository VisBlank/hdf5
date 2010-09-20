!****h* ROBODoc/H5O
!
! NAME
!  MODULE H5O
!
! FILE
!  fortran/src/H5Off.f90
!
! PURPOSE
!  This file contains Fortran interfaces for H5O functions. It includes
!  all the functions that are independent on whether the Fortran 2003 functions
!  are enabled or disabled.
!
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
! NOTES
!                         *** IMPORTANT ***
!  If you add a new H5O function you must add the function name to the
!  Windows dll file 'hdf5_fortrandll.def' in the fortran/src directory.
!  This is needed for Windows based operating systems.
!
!*****

MODULE H5O

  USE H5GLOBAL

CONTAINS

!****s* H5O/h5olink_f
!
! NAME
!  h5olink_f
!
! PURPOSE
!  Creates a hard link to an object in an HDF5 file.
! INPUTS
!  object_id 	 - Object to be linked.
!  new_loc_id 	 - File or group identifier specifying location at which object is to be linked.
!  new_link_name - Name of link to be created, relative to new_loc_id.
! OUTPUTS
!  hdferr:       - error code
!                   Success:  0
!                   Failure: -1
! OPTIONAL PARAMETERS
!  lcpl_id 	 - Link creation property list identifier.
!  lapl_id 	 - Link access property list identifier.
! AUTHOR
!  M. Scot Breitenfeld
!  April 21, 2008
!
! SOURCE
  SUBROUTINE h5olink_f(object_id, new_loc_id, new_link_name, hdferr, lcpl_id, lapl_id)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: object_id  ! Object to be linked
    INTEGER(HID_T), INTENT(IN) :: new_loc_id ! File or group identifier specifying
                                             ! location at which object is to be linked.
    CHARACTER(LEN=*), INTENT(IN) :: new_link_name ! Name of link to be created, relative to new_loc_id.
    INTEGER, INTENT(OUT) :: hdferr        ! Error code
                                          !   Success:  0
                                          !   Failure: -1
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: lcpl_id ! Link creation property list identifier.
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: lapl_id ! Link creation property list identifier.
!*****
    INTEGER(HID_T) :: lapl_id_default
    INTEGER(HID_T) :: lcpl_id_default

    INTEGER(SIZE_T) :: new_link_namelen

    INTERFACE
       INTEGER FUNCTION h5olink_c(object_id, new_loc_id, new_link_name, new_link_namelen, &
            lcpl_id_default, lapl_id_default)
         USE H5GLOBAL
         !DEC$IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ATTRIBUTES C,reference,decorate,alias:'H5OLINK_C'::h5olink_c
         !DEC$ENDIF
         !DEC$ATTRIBUTES reference :: new_link_name
         INTEGER(HID_T), INTENT(IN) :: object_id
         INTEGER(HID_T), INTENT(IN) :: new_loc_id
         CHARACTER(LEN=*), INTENT(IN) :: new_link_name
         INTEGER(SIZE_T) :: new_link_namelen
         INTEGER(HID_T) :: lapl_id_default
         INTEGER(HID_T) :: lcpl_id_default
       END FUNCTION h5olink_c
    END INTERFACE

    new_link_namelen = LEN(new_link_name)

    lapl_id_default = H5P_DEFAULT_F
    IF(PRESENT(lapl_id)) lapl_id_default = lapl_id
    lcpl_id_default = H5P_DEFAULT_F
    IF(PRESENT(lcpl_id)) lcpl_id_default = lcpl_id

    hdferr = h5olink_c(object_id, new_loc_id, new_link_name, new_link_namelen, &
         lcpl_id_default, lapl_id_default)

  END SUBROUTINE h5olink_f

!****s* H5O/h5oopen_f
!
! NAME
!  h5oopen_f
! PURPOSE
!  Opens an object in an HDF5 file by location identifier and path name.
!
! INPUTS
!  loc_id 	 - File or group identifier
!  name 	 - Path to the object, relative to loc_id.
! OUTPUTS
!  obj_id 	 - Object identifier for the opened object
!  hdferr:       - error code
!                   Success:  0
!                   Failure: -1
! OPTIONAL PARAMETERS
!  lapl_id 	 - Access property list identifier for the link pointing to the object
!
! AUTHOR
!  M. Scot Breitenfeld
!  April 18, 2008
! SOURCE
  SUBROUTINE h5oopen_f(loc_id, name, obj_id, hdferr, lapl_id)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id  ! File or group identifier
    CHARACTER(LEN=*), INTENT(IN) :: name  ! Path to the object, relative to loc_id
    INTEGER(HID_T), INTENT(OUT) :: obj_id ! Object identifier for the opened object
    INTEGER, INTENT(OUT) :: hdferr        ! Error code
                                          !   Success:  0
                                          !   Failure: -1
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: lapl_id  ! Attribute access property list
!*****
    INTEGER(HID_T) :: lapl_id_default
    INTEGER(SIZE_T) :: namelen

    INTERFACE
       INTEGER FUNCTION h5oopen_c(loc_id, name, namelen, lapl_id_default, obj_id)
         USE H5GLOBAL
         !DEC$IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ATTRIBUTES C,reference,decorate,alias:'H5OOPEN_C'::h5oopen_c
         !DEC$ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: loc_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER(HID_T) :: lapl_id_default
         INTEGER(SIZE_T) :: namelen
         INTEGER(HID_T), INTENT(OUT) :: obj_id
       END FUNCTION h5oopen_c
    END INTERFACE

    namelen = LEN(name)

    lapl_id_default = H5P_DEFAULT_F
    IF(PRESENT(lapl_id)) lapl_id_default = lapl_id

    hdferr = h5oopen_c(loc_id, name, namelen, lapl_id_default, obj_id)

  END SUBROUTINE h5oopen_f
!
!****s* H5O/h5oclose_f
!
! NAME
!  h5oclose_f
!
! PURPOSE
!  Closes an object in an HDF5 file.
!
! INPUTS
!  object_id     - Object identifier
! OUTPUTS
!  hdferr 	 - Returns 0 if successful and -1 if fails
!
! AUTHOR
!  M. Scot Breitenfeld
!  December 17, 2008
!
! SOURCE
  SUBROUTINE h5oclose_f(object_id, hdferr)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN)  :: object_id
    INTEGER,        INTENT(OUT) :: hdferr
!*****
    INTERFACE
       INTEGER FUNCTION h5oclose_c(object_id)
         USE H5GLOBAL
         !DEC$IF DEFINED(HDF5F90_WINDOWS)
         !DEC$OBJECTIBUTES C,reference,decorate,alias:'H5OCLOSE_C'::h5oclose_c
         !DEC$ENDIF
         INTEGER(HID_T), INTENT(IN) :: object_id
       END FUNCTION h5oclose_c
    END INTERFACE

    hdferr = h5oclose_c(object_id)
  END SUBROUTINE h5oclose_f

!
!****s* H5O/h5open_by_addr_f
! NAME		
!  h5oopen_by_addr_f 
!
! PURPOSE
!  Opens an object using its address within an HDF5 file. 
!
! INPUTS  
!    loc_id - File or group identifier
!    addr   - Object’s address in the file
! OUTPUTS: 
!    obj_id - Object identifier for the opened object
!    hdferr - Returns 0 if successful and -1 if fails
!
! AUTHOR	
!  M. Scot Breitenfeld
!  September 14, 2009
! 
! SOURCE
  SUBROUTINE h5oopen_by_addr_f(loc_id, addr, obj_id, hdferr)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id  ! File or group identifier
    INTEGER(HADDR_T), INTENT(IN) :: addr  ! Object’s address in the file
    INTEGER(HID_T), INTENT(OUT) :: obj_id ! Object identifier for the opened object
    INTEGER, INTENT(OUT) :: hdferr        ! Error code:
                                          ! 0 on success and -1 on failure
!*****
    INTERFACE
       INTEGER FUNCTION h5oopen_by_addr_c(loc_id, addr, obj_id)
         USE H5GLOBAL
         !DEC$IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ATTRIBUTES C,reference,decorate,alias:'H5OOPEN_BY_ADDR_C'::h5oopen_by_addr_c
         !DEC$ENDIF
         INTEGER(HID_T), INTENT(IN) :: loc_id
         INTEGER(HADDR_T), INTENT(IN) :: addr
         INTEGER(HID_T), INTENT(OUT) :: obj_id
       END FUNCTION h5oopen_by_addr_c
    END INTERFACE

    hdferr = h5oopen_by_addr_c(loc_id, addr, obj_id)

  END SUBROUTINE h5oopen_by_addr_f

!!$
!!$
!!$TYPE space_t
!!$   INTEGER(hsize_t) :: total ! Total space for storing object header in file
!!$   INTEGER(hsize_t) :: meta  ! Space within header for object header metadata information
!!$   INTEGER(hsize_t) :: mesg  ! Space within header for actual message information
!!$   INTEGER(hsize_t) :: free  ! Free space within object header
!!$END TYPE space_t
!!$
!!$TYPE mesg_t
!!$   INTEGER(c_int64_t) :: present ! Flags to indicate presence of message type in header 
!!$   INTEGER(c_int64_t) :: shared ! Flags to indicate message type is shared in header
!!$ ! uint64_t present
!!$ ! uint64_t shared
!!$END TYPE mesg_t
!!$
!!$TYPE hdr_t
!!$   INTEGER(c_int) :: version ! Version number of header format in file
!!$   ! unsigned version                          */
!!$   INTEGER(c_int) :: nmesgs ! Number of object header messages
!!$   ! unsigned nmesgs 
!!$   INTEGER(c_int) :: nchunks ! Number of object header chunks 
!!$   ! unsigned nchunks
!!$   INTEGER(c_int) :: flags ! Object header status flags
!!$   ! unsigned flags
!!$   TYPE(space_t) :: space
!!$   TYPE(mesg_t) :: mesg
!!$END TYPE hdr_t
!!$
!!$! Extra metadata storage for obj & attributes
!!$TYPE H5_ih_info_t
!!$    INTEGER(hsize_t) :: index_size ! btree and/or list
!!$    INTEGER(hsize_t) :: heap_size
!!$ END TYPE H5_ih_info_t
!!$
!!$TYPE meta_size_t
!!$   TYPE(H5_ih_info_t) :: obj  ! v1/v2 B-tree & local/fractal heap for groups, B-tree for chunked datasets
!!$   TYPE(H5_ih_info_t) :: attr ! v2 B-tree & heap for attributes
!!$ENDTYPE meta_size_t
!!$
!!$
!!$TYPE H5O_info_t
!!$   INTEGER(HADDR_T) ::  fileno ! File number that object is located in
!!$   ! unsigned long       fileno;
!!$   INTEGER(HADDR_T) :: addr  ! Object address in file  
!!$   INTEGER :: type ! Basic object type (group, dataset, etc.) 
!!$   ! H5O_type_t          type
!!$   INTEGER(c_int) :: rc  ! Reference count of object
!!$   ! unsigned rc
!!$    time_t              atime;          /* Access time                        */
!!$    time_t              mtime;          /* Modification time                  */
!!$    time_t              ctime;          /* Change time                        */
!!$    time_t              btime;          /* Birth time                         */
!!$   INTEGER(hsize_t) :: num_attrs ! # of attributes attached to object
!!$
!!$   TYPE(hdr_t) :: hdr
!!$
!!$   TYPE(meta_size_t) :: meta_size
!!$END TYPE H5O_info_t


!
!!$!****s* H5O/h5oget_info_by_name_f
!
! NAME
!  h5oget_info_by_name_f
!
! PURPOSE
!  Retrieves the metadata for an object, identifying the object by location and relative name.
!
! INPUTS
!  loc_id 	  - File or group identifier specifying location of group in which object 
!                   is located.
!  name 	  - Name of group, relative to loc_id
!
! OUTPUTS  NOTE: In C it is defined as a structure: H5O_info_t
!    **** NEED TO MAKE THIS DERIVED DATATYPE ****
!  hdferr 	  - Returns 0 if successful and -1 if fails
! OPTIONAL PARAMETERS
!  lapl_id 	  - Link access property list
!
! AUTHOR
!  M. Scot Breitenfeld
!  December 1, 2008
!
! SOURCE
!!$  SUBROUTINE h5oget_info_by_name_f(loc_id, name, &
!!$       f_corder_valid, corder, cset, data_size, hdferr, lapl_id)
!!$    IMPLICIT NONE
!!$    INTEGER(HID_T)  , INTENT(IN)            :: loc_id
!!$    CHARACTER(LEN=*), INTENT(IN)            :: name
!!$    LOGICAL         , INTENT(OUT)           :: f_corder_valid
!!$    INTEGER         , INTENT(OUT)           :: corder
!!$    INTEGER         , INTENT(OUT)           :: cset
!!$    INTEGER(HSIZE_T), INTENT(OUT)           :: data_size
!!$    INTEGER         , INTENT(OUT)           :: hdferr
!!$    INTEGER(HID_T)  , INTENT(IN) , OPTIONAL :: lapl_id
!!$!*****
!!$    INTEGER         :: corder_valid
!!$    INTEGER(SIZE_T) :: namelen
!!$    INTEGER(HID_T)  :: lapl_id_default
!!$    
!!$    INTERFACE
!!$       INTEGER FUNCTION h5oget_info_by_name_c(loc_id, name, namelen, lapl_id_default, &
!!$            corder_valid, corder, cset, data_size)
!!$         USE H5GLOBAL
!!$         !DEC$IF DEFINED(HDF5F90_WINDOWS)
!!$         !DEC$ATTRIBUTES C,reference,decorate,alias:'H5OGET_INFO_BY_NAME_C'::h5oget_info_by_name_c
!!$         !DEC$ENDIF
!!$         INTEGER(HID_T)  , INTENT(IN)  :: loc_id
!!$         CHARACTER(LEN=*), INTENT(IN)  :: name
!!$         INTEGER(SIZE_T) , INTENT(IN)  :: namelen
!!$         INTEGER(HID_T)  , INTENT(IN)  :: lapl_id_default
!!$         INTEGER         , INTENT(OUT) :: corder_valid
!!$         INTEGER         , INTENT(OUT) :: corder
!!$         INTEGER         , INTENT(OUT) :: cset
!!$         INTEGER(HSIZE_T), INTENT(OUT) :: data_size
!!$
!!$       END FUNCTION h5oget_info_by_name_c
!!$    END INTERFACE
!!$
!!$    namelen = LEN(name)
!!$
!!$    lapl_id_default = H5P_DEFAULT_F
!!$    IF(PRESENT(lapl_id)) lapl_id_default = lapl_id
!!$
!!$    hdferr = H5Oget_info_by_name_c(loc_id, name, namelen, lapl_id_default, &
!!$         corder_valid, corder, cset, data_size)
!!$
!!$    f_corder_valid =.FALSE.
!!$    IF (corder_valid .EQ. 1) f_corder_valid =.TRUE.
!!$
!!$  END SUBROUTINE H5Oget_info_by_name_f


END MODULE H5O

