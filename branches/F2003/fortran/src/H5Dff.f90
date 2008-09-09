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
! This file contains Fortran90 interfaces for H5D functions.
! 
MODULE H5D
  USE H5GLOBAL

  INTERFACE h5dextend_f
     MODULE PROCEDURE h5dset_extent_f
  END INTERFACE

  INTERFACE h5dread_vl_f
     MODULE PROCEDURE h5dread_vl_integer
     MODULE PROCEDURE h5dread_vl_real
     MODULE PROCEDURE h5dread_vl_string
  END INTERFACE

  INTERFACE h5dwrite_vl_f
     MODULE PROCEDURE h5dwrite_vl_integer
     MODULE PROCEDURE h5dwrite_vl_real
     MODULE PROCEDURE h5dwrite_vl_string
  END INTERFACE

CONTAINS
          
!----------------------------------------------------------------------
! Name:		h5dcreate_f 
!
! Purpose: 	Creates a dataset at the specified location 	
!
! Inputs:  
!		loc_id		- file or group identifier
!		name		- dataset name
!		type_id		- dataset datatype identifier
!		space_id	- dataset dataspace identifier
! Outputs:  
!		dset_id		- dataset identifier
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!            creation_prp - Dataset creation property list
!            lcpl_id      - Link creation property list
!            dapl_id      - Dataset access property list
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	
!                 - Explicit Fortran interfaces were added for 
!	           called C functions (it is needed for Windows
!		   port).  February 28, 2001 
!
!                 - Added version's 1.8 new optional parameters
!                  February, 2008
!
! Comment:		
!----------------------------------------------------------------------
  
  SUBROUTINE h5dcreate_f(loc_id, name, type_id, space_id, dset_id, & 
       hdferr, dcpl_id, lcpl_id, dapl_id)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name   ! Name of the dataset 
    INTEGER(HID_T), INTENT(IN) :: type_id  ! Datatype identifier 
    INTEGER(HID_T), INTENT(IN) :: space_id ! Dataspace identifier 
    INTEGER(HID_T), INTENT(OUT) :: dset_id ! Dataset identifier 
    INTEGER, INTENT(OUT) :: hdferr         ! Error code

    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: dcpl_id ! Dataset creation property list
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: lcpl_id ! Link creation property list
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: dapl_id ! Dataset access property list

    INTEGER(HID_T) :: lcpl_id_default
    INTEGER(HID_T) :: dcpl_id_default
    INTEGER(HID_T) :: dapl_id_default

    INTEGER :: namelen                     ! Name length

!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5dcreate_c(loc_id, name, namelen, type_id, &
            space_id, lcpl_id_default, dcpl_id_default, dapl_id_default, dset_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DCREATE_C'::h5dcreate_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: loc_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER :: namelen
         INTEGER(HID_T), INTENT(IN) :: type_id
         INTEGER(HID_T), INTENT(IN) :: space_id

         INTEGER(HID_T) :: lcpl_id_default 
         INTEGER(HID_T) :: dcpl_id_default
         INTEGER(HID_T) :: dapl_id_default

         INTEGER(HID_T), INTENT(OUT) :: dset_id
       END FUNCTION h5dcreate_c
    END INTERFACE
    
    lcpl_id_default = H5P_DEFAULT_F
    dcpl_id_default = H5P_DEFAULT_F
    dapl_id_default = H5P_DEFAULT_F

    IF(PRESENT(lcpl_id)) lcpl_id_default = lcpl_id
    IF(PRESENT(dcpl_id)) dcpl_id_default = dcpl_id
    IF(PRESENT(dapl_id)) dapl_id_default = dapl_id

    namelen = LEN(name)
    hdferr = h5dcreate_c(loc_id, name, namelen, type_id, space_id, & 
         lcpl_id_default, dcpl_id_default, dapl_id_default, dset_id)
 
  END SUBROUTINE h5dcreate_f
          
!----------------------------------------------------------------------
! Name:		h5dopen_f 
!
! Purpose: 	Opens an existing dataset.  	
!
! Inputs:  
!		loc_id		- file or group identifier
!		name		- dataset name
! Outputs:  
!		dset_id		- dataset identifier
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!	       dapl_id	        - Dataset access property list		
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications:  -Explicit Fortran interfaces were added for 
!		   called C functions (it is needed for Windows
!		   port).  February 28, 2001 
!
!                 -Added 1.8 (optional) parameter dapl_id
!                  February, 2008, M.S. Breitenfeld
!
! Comment:		
!----------------------------------------------------------------------

  SUBROUTINE h5dopen_f(loc_id, name, dset_id, hdferr, dapl_id)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
    CHARACTER(LEN=*), INTENT(IN) :: name   ! Name of the dataset 
    INTEGER(HID_T), INTENT(OUT) :: dset_id ! Dataset identifier
    INTEGER, INTENT(OUT) :: hdferr         ! Error code 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: dapl_id ! Dataset access property list
    INTEGER :: namelen                     ! Name length
    
    INTEGER(HID_T) :: dapl_id_default
    
!            INTEGER, EXTERNAL :: h5dopen_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5dopen_c(loc_id, name, namelen, dapl_id_default, dset_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DOPEN_C'::h5dopen_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: name
         INTEGER(HID_T), INTENT(IN) :: loc_id
         CHARACTER(LEN=*), INTENT(IN) :: name
         INTEGER :: namelen
         INTEGER(HID_T), INTENT(IN) :: dapl_id_default
         INTEGER(HID_T), INTENT(OUT) :: dset_id
       END FUNCTION h5dopen_c
    END INTERFACE
    
    dapl_id_default = H5P_DEFAULT_F
    IF(PRESENT(dapl_id)) dapl_id_default = dapl_id
    
    namelen = LEN(name)
    hdferr = h5dopen_c(loc_id, name, namelen, dapl_id_default, dset_id) 
    
  END SUBROUTINE h5dopen_f
          
!----------------------------------------------------------------------
! Name:		h5dclose_f 
!
! Purpose: 	Closes a dataset.  	
!
! Inputs:  
!		dset_id		- dataset identifier
! Outputs:  
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE			
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  February 28, 2001 
!
! Comment:		
!----------------------------------------------------------------------

  SUBROUTINE h5dclose_f(dset_id, hdferr)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id ! Dataset identifier
    INTEGER, INTENT(OUT) :: hdferr        ! Error code
    
!            INTEGER, EXTERNAL :: h5dclose_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5dclose_c(dset_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DCLOSE_C'::h5dclose_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
       END FUNCTION h5dclose_c
    END INTERFACE
    
    hdferr = h5dclose_c(dset_id)
    
  END SUBROUTINE h5dclose_f

!----------------------------------------------------------------------
! Name:		h5dget_type_f 
!
! Purpose:	Returns an identifier for a copy of the datatype for a 
!		dataset.   	
!
! Inputs:  
!		dataset_id	- dataset identifier
! Outputs:  
!		datatype_id	- dataspace identifier
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE			
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  February 28, 2001 
!
! Comment:		
!----------------------------------------------------------------------

  SUBROUTINE h5dget_type_f(dataset_id, datatype_id, hdferr)
    IMPLICIT NONE 
    INTEGER(HID_T), INTENT(IN) :: dataset_id      ! Dataset identifier
    INTEGER(HID_T), INTENT(OUT) :: datatype_id    ! Datatype identifier
    INTEGER, INTENT(OUT) :: hdferr                ! Error code 
!            INTEGER, EXTERNAL :: h5dget_type_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5dget_type_c (dataset_id, datatype_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DGET_TYPE_C'::h5dget_type_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dataset_id
         INTEGER(HID_T), INTENT(OUT) :: datatype_id
       END FUNCTION h5dget_type_c
    END INTERFACE
    
    hdferr = h5dget_type_c (dataset_id, datatype_id)
  END SUBROUTINE h5dget_type_f

!----------------------------------------------------------------------
! Name:		h5dset_extent (instead of obsolete name: h5dextend_f) 
!
! Purpose:	Extends a dataset with unlimited dimension.	
!
! Inputs:  
!		dataset_id	- dataset identifier
!		size		- array containing the new magnitude of 
!				  each dimension
! Outputs:  
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE			
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  February 28, 2001 
!
!                       Changed name from the now obsolete h5dextend_f
!                       to h5dset_extent_f. Provided interface to old name
!                       for backward compatability. -MSB- March 14, 2008
!
! Comment:		
!----------------------------------------------------------------------


  SUBROUTINE h5dset_extent_f(dataset_id, size, hdferr)
    IMPLICIT NONE 
    INTEGER(HID_T), INTENT(IN) :: dataset_id      ! Dataset identifier
    INTEGER(HSIZE_T), DIMENSION(*), INTENT(IN)  :: size
    ! Array containing 
    ! dimensions' sizes 
    INTEGER, INTENT(OUT) :: hdferr                ! Error code 
    
    !
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dset_extent_c(dataset_id, size)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DSET_EXTENT_C'::h5dset_extent_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dataset_id
         INTEGER(HSIZE_T), DIMENSION(*), INTENT(IN)  :: size
       END FUNCTION h5dset_extent_c
    END INTERFACE
    
    hdferr = H5Dset_extent_c(dataset_id, size)
  END SUBROUTINE h5dset_extent_f


!----------------------------------------------------------------------
! Name:		h5dget_create_plist_f 
!
! Purpose:	Returns an identifier for a copy of the dataset creation 
!		property list for a dataset. 	
!
! Inputs:  
!		dataset_id	- dataset identifier
! Outputs:  
!		plist_id	- creation property list identifier
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE			
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  February 28, 2001 
!
! Comment:		
!----------------------------------------------------------------------

  SUBROUTINE h5dget_create_plist_f(dataset_id, plist_id, hdferr)
    IMPLICIT NONE 
    INTEGER(HID_T), INTENT(IN) :: dataset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(OUT) :: plist_id    ! Dataset creation
                                               ! property list identifier
    INTEGER, INTENT(OUT) :: hdferr             ! Error code 

!            INTEGER, EXTERNAL :: h5dget_create_plist_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5dget_create_plist_c(dataset_id, plist_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DGET_CREATE_PLIST_C'::h5dget_create_plist_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dataset_id
         INTEGER(HID_T), INTENT(OUT) :: plist_id
       END FUNCTION h5dget_create_plist_c
    END INTERFACE
    
    hdferr = h5dget_create_plist_c(dataset_id, plist_id)
  END SUBROUTINE h5dget_create_plist_f

!----------------------------------------------------------------------
! Name:		h5dget_storage_size_f 
!
! Purpose:	Returns the amount of storage requires by a dataset	
!
! Inputs:  
!		dataset_id	- dataset identifier
! Outputs:  
!		size		- datastorage size
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE			
!
! Programmer:	Elena Pourmal
!		October 15, 2002	
!
! Modifications: 	
!
! Comment:		
!----------------------------------------------------------------------


  SUBROUTINE h5dget_storage_size_f(dataset_id, size, hdferr)
    IMPLICIT NONE 
    INTEGER(HID_T), INTENT(IN) :: dataset_id      ! Dataset identifier
    INTEGER(HSIZE_T),  INTENT(OUT)  :: size
    ! Amount of storage 
    ! allocated for dataset
    INTEGER, INTENT(OUT) :: hdferr                ! Error code 
    
    INTERFACE
       INTEGER FUNCTION h5dget_storage_size_c(dataset_id, size)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DGET_STORAGE_SIZE_C'::h5dget_storage_size_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dataset_id
         INTEGER(HSIZE_T), INTENT(OUT)  :: size
       END FUNCTION h5dget_storage_size_c
    END INTERFACE
    
    hdferr = h5dget_storage_size_c(dataset_id, size)
  END SUBROUTINE h5dget_storage_size_f

!----------------------------------------------------------------------
! Name:		h5dvlen_get_max_len_f 
!
! Purpose:	Returns maximum length of the VL array elements
!
! Inputs:  
!		dataset_id	- dataset identifier
!		type_id		- datatype identifier
!		space_id	- dataspace identifier
! Outputs:  
!		size		- buffer size
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE			
!
! Programmer:	Elena Pourmal
!		October 15, 2002	
!
! Modifications: 	
!
! Comment:		
!----------------------------------------------------------------------

  SUBROUTINE h5dvlen_get_max_len_f(dataset_id, type_id, space_id, len,  hdferr)
    IMPLICIT NONE 
    INTEGER(HID_T), INTENT(IN) :: dataset_id      ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: type_id         ! Datatype identifier
    INTEGER(HID_T), INTENT(IN) :: space_id        ! Dataspace identifier
    INTEGER(SIZE_T),  INTENT(OUT)  :: len         ! Maximum length of the element
    INTEGER, INTENT(OUT) :: hdferr                ! Error code 
    
    INTERFACE
       INTEGER FUNCTION h5dvlen_get_max_len_c(dataset_id, type_id, space_id, len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DVLEN_GET_MAX_LEN_C'::h5dvlen_get_max_len_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dataset_id
         INTEGER(HID_T), INTENT(IN) :: type_id 
         INTEGER(HID_T), INTENT(IN) :: space_id     
         INTEGER(SIZE_T), INTENT(OUT)  :: len 
       END FUNCTION h5dvlen_get_max_len_c
    END INTERFACE
    
    hdferr = h5dvlen_get_max_len_c(dataset_id, type_id,  space_id, len)
  END SUBROUTINE h5dvlen_get_max_len_f
  
  !----------------------------------------------------------------------
  ! Name:		h5dget_space_status_f
  !
  ! Purpose:      Returns the status of data space allocation. 
  !
  ! Inputs:  
  !		dset_id		- dataset identifier
  ! Outputs:  
  !               flag            - status; may have one of the following values:
  !				  H5D_SPACE_STS_ERROR_F
  !				  H5D_SPACE_STS_NOT_ALLOCATED_F
  !				  H5D_SPACE_STS_PART_ALLOCATED_F
  !				  H5D_SPACE_STS_ALLOCATED_F
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  !
  ! Programmer:	Elena Pourmal
  !		March 12, 2003
  !
  !----------------------------------------------------------------------

  SUBROUTINE h5dget_space_status_f(dset_id, flag, hdferr)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id  ! Dataspace identifier
    INTEGER, INTENT(IN)        :: flag     ! Memory buffer to fill in
    INTEGER, INTENT(OUT)       :: hdferr   ! Error code 

    !            INTEGER, EXTERNAL :: h5dget_space_status_c
    ! MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dget_space_status_c(dset_id, flag)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DGET_SPACE_STATUS_C'::h5dget_space_status_c  
         !DEC$ ENDIF
         INTEGER(HID_T) :: dset_id
         INTEGER        :: flag
       END FUNCTION h5dget_space_status_c
    END INTERFACE

    hdferr = h5dget_space_status_c(dset_id, flag)
  END SUBROUTINE h5dget_space_status_f

  !----------------------------------------------------------------------
  ! Name:		h5dcreate_anon_f 
  !
  ! Purpose: 	Creates a dataset in a file without linking it into the file structure 
  !
  ! Inputs:  
  !		loc_id		- Identifier of the file or group within which to create the dataset.
  !		type_id		- Identifier of the datatype to use when creating the dataset.
  !		space_id	- Identifier of the dataspace to use when creating the dataset.
  ! Outputs:  
  !		dset_id		- dataset identifier
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  ! Optional parameters:
  !               dcpl_id       - Dataset creation property list identifier.
  !               dapl_id  	- Dataset access property list identifier.
  !
  ! Programmer:   M.S. Breitenfeld
  !		  February 11, 2008
  !
  ! Modifications:
  !
  ! Comment:		
  !----------------------------------------------------------------------

  SUBROUTINE h5dcreate_anon_f(loc_id, type_id, space_id, dset_id, hdferr, dcpl_id, dapl_id)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier. 
    INTEGER(HID_T), INTENT(IN) :: type_id  ! Datatype identifier. 
    INTEGER(HID_T), INTENT(IN) :: space_id ! Dataspace identifier.
    INTEGER(HID_T), INTENT(OUT) :: dset_id ! Dataset identifier. 
    INTEGER, INTENT(OUT) :: hdferr         ! Error code.
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: dcpl_id  ! Dataset creation property list identifier.
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: dapl_id  ! Dataset access property list identifier.

    INTEGER(HID_T) :: dcpl_id_default
    INTEGER(HID_T) :: dapl_id_default

    !
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dcreate_anon_c(loc_id, type_id, space_id, dcpl_id_default, dapl_id_default, dset_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DCREATE_ANON_C'::h5dcreate_anon_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: loc_id 
         INTEGER(HID_T), INTENT(IN) :: type_id
         INTEGER(HID_T), INTENT(IN) :: space_id
         INTEGER(HID_T) :: dcpl_id_default
         INTEGER(HID_T) :: dapl_id_default
         INTEGER(HID_T), INTENT(OUT) :: dset_id
       END FUNCTION h5dcreate_anon_c
    END INTERFACE

    dcpl_id_default = H5P_DEFAULT_F
    dapl_id_default = H5P_DEFAULT_F

    IF(PRESENT(dcpl_id)) dcpl_id_default = dcpl_id
    IF(PRESENT(dapl_id)) dapl_id_default = dapl_id

    hdferr = h5dcreate_anon_c(loc_id, type_id, space_id, dcpl_id_default, dapl_id_default, dset_id)

  END SUBROUTINE h5dcreate_anon_f


  SUBROUTINE h5dwrite_vl_integer(dset_id, mem_type_id, buf, dims, len, &
       hdferr, &
       mem_space_id, file_space_id, xfer_prp)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! MAX len x num_elem
    INTEGER(SIZE_T), INTENT(IN), DIMENSION(*) :: len ! Array to store 
                                                     ! the length of each
                                                     ! element
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2)), TARGET :: buf   ! Data buffer
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: mem_space_id 
    ! Memory dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: file_space_id 
    ! File dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: xfer_prp 
    ! Transfer property list identifier 
    INTEGER(HID_T) :: xfer_prp_default 
    INTEGER(HID_T) :: mem_space_id_default
    INTEGER(HID_T) :: file_space_id_default

    INTERFACE
       INTEGER FUNCTION h5dwrite_vl_integer_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims, len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_VL_INTEGER_C'::h5dwrite_vl_integer_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER(SIZE_T), INTENT(IN), DIMENSION(*) :: len 
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dwrite_vl_integer_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F
    
    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id


    hdferr = h5dwrite_vl_integer_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims, len)

  END SUBROUTINE h5dwrite_vl_integer

  SUBROUTINE h5dread_vl_integer(dset_id, mem_type_id, buf, dims, len, &
       hdferr, &
       mem_space_id, file_space_id, xfer_prp)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! MAX len x num_elem
    INTEGER(SIZE_T), INTENT(INOUT), DIMENSION(*) :: len ! Array to store 
                                                        ! the length of each
                                                        ! element
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)), TARGET :: buf   ! Data buffer
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 
    ! -1 if failed, 0 otherwise
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: mem_space_id 
    ! Memory dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: file_space_id 
    ! File dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: xfer_prp 
    ! Transfer property list identifier 
    INTEGER(HID_T) :: xfer_prp_default 
    INTEGER(HID_T)  :: mem_space_id_default
    INTEGER(HID_T) :: file_space_id_default 
    INTEGER(HID_T) :: tmp
    INTEGER :: error
    
    INTERFACE
       INTEGER FUNCTION h5dread_vl_integer_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims, len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_VL_INTEGER_C'::h5dread_vl_integer_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER(SIZE_T), INTENT(INOUT), DIMENSION(*) :: len 
         INTEGER, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dread_vl_integer_c
    END INTERFACE

    CALL h5dget_space_f(dset_id, tmp, error) 
    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = tmp
    file_space_id_default = tmp
    
    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_vl_integer_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims, len)
           
  END SUBROUTINE h5dread_vl_integer

  SUBROUTINE h5dwrite_vl_real(dset_id, mem_type_id, buf, dims, len, &
       hdferr, &
       mem_space_id, file_space_id, xfer_prp)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! MAX len x num_elem
    INTEGER(SIZE_T), INTENT(IN), DIMENSION(*) :: len ! Array to store 
    ! the length of each
    ! element
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2)) :: buf   ! Data buffer
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: mem_space_id 
    ! Memory dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: file_space_id 
    ! File dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: xfer_prp 
    ! Transfer property list identifier 
    
    INTEGER(HID_T) :: xfer_prp_default 
    INTEGER(HID_T)  :: mem_space_id_default
    INTEGER(HID_T) :: file_space_id_default 
    
    INTERFACE
       INTEGER FUNCTION h5dwrite_vl_real_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims, len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_VL_REAL_C'::h5dwrite_vl_real_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER(SIZE_T), INTENT(IN), DIMENSION(*) :: len 
         REAL, INTENT(IN), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dwrite_vl_real_c
    END INTERFACE

    
    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F
    
    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 
    
    hdferr = h5dwrite_vl_real_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims, len)
    
  END SUBROUTINE h5dwrite_vl_real

  SUBROUTINE h5dread_vl_real(dset_id, mem_type_id, buf, dims, len, &
       hdferr, &
       mem_space_id, file_space_id, xfer_prp)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! MAX len x num_elem
    INTEGER(SIZE_T), INTENT(INOUT), DIMENSION(*) :: len ! Array to store 
    ! the length of each
                                                              ! element
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)) :: buf   ! Data buffer
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 
    ! -1 if failed, 0 otherwise
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: mem_space_id 
    ! Memory dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: file_space_id 
    ! File dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: xfer_prp 
    ! Transfer property list identifier 
    
    INTEGER(HID_T) :: xfer_prp_default 
    INTEGER(HID_T)  :: mem_space_id_default
    INTEGER(HID_T) :: file_space_id_default 
    INTEGER(HID_T) :: tmp
    INTEGER :: error
    
    INTERFACE
       INTEGER FUNCTION h5dread_vl_real_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims, len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_VL_REAL_C'::h5dread_vl_real_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER(SIZE_T), INTENT(INOUT), DIMENSION(*) :: len 
         REAL, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dread_vl_real_c
    END INTERFACE

    CALL h5dget_space_f(dset_id, tmp, error) 
    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = tmp
    file_space_id_default = tmp
    
    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 
    
    hdferr = h5dread_vl_real_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims, len)
    
  END SUBROUTINE h5dread_vl_real
  
  SUBROUTINE h5dwrite_vl_string(dset_id, mem_type_id, buf, dims, str_len, &
       hdferr, &
       mem_space_id, file_space_id, xfer_prp)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! number of strings
    INTEGER(SIZE_T), INTENT(IN), DIMENSION(*) :: str_len ! Array to store 
    ! the length of each
    ! element
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(2)) :: buf           ! Data buffer
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: mem_space_id 
    ! Memory dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: file_space_id 
    ! File dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: xfer_prp 
    ! Transfer property list identifier 
    
    INTEGER(HID_T) :: xfer_prp_default 
    INTEGER(HID_T)  :: mem_space_id_default
    INTEGER(HID_T) :: file_space_id_default 
!            CHARACTER, DIMENSION(dims(1)*dims(2)) :: tmp_buf

    INTERFACE
       INTEGER FUNCTION h5dwrite_vl_string_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            !                                          xfer_prp_default, tmp_buf, dims, str_len)
            xfer_prp_default, buf, dims, str_len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_VL_STRING_C'::h5dwrite_vl_string_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims
         INTEGER(SIZE_T), INTENT(IN), DIMENSION(*) :: str_len 
         !              CHARACTER, INTENT(IN), &
         !              DIMENSION(dims(1)*dims(2)) :: tmp_buf
         CHARACTER(LEN=*), DIMENSION(dims(2)) :: buf
       END FUNCTION h5dwrite_vl_string_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 
!            do i = 1, dims(2)
!               do j = 1, dims(1)
!               tmp_buf((i-1)*dims(1) +j) = buf(i)(j:j)
!               enddo
!            enddo 
!              write(*,*) (tmp_buf(j:j), j=1,dims(1)*dims(2))
!              write(*,*) str_len(1), str_len(2), str_len(3), str_len(4)

    hdferr = h5dwrite_vl_string_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims, str_len)
           
  END SUBROUTINE h5dwrite_vl_string

  SUBROUTINE h5dread_vl_string(dset_id, mem_type_id, buf, dims, str_len, &
                                         hdferr, &
                                         mem_space_id, file_space_id, xfer_prp)
    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! number of strings
    INTEGER(SIZE_T), INTENT(OUT), DIMENSION(*) :: str_len ! Array to store 
    ! the length of each
    ! element
    CHARACTER(LEN=*), INTENT(OUT), &
         DIMENSION(dims(2)) :: buf           ! Data buffer
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: mem_space_id 
    ! Memory dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: file_space_id 
    ! File dataspace identfier 
    INTEGER(HID_T), OPTIONAL, INTENT(IN) :: xfer_prp 
    ! Transfer property list identifier 
    
    INTEGER(HID_T) :: xfer_prp_default 
    INTEGER(HID_T)  :: mem_space_id_default
    INTEGER(HID_T) :: file_space_id_default 
    !            CHARACTER, DIMENSION(dims(1)*dims(2)) :: tmp_buf
    !            integer i, j

    INTERFACE
       INTEGER FUNCTION h5dread_vl_string_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            !                                          xfer_prp_default, tmp_buf, dims, str_len)
            xfer_prp_default, buf, dims, str_len)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_VL_STRING_C'::h5dread_vl_string_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims
         INTEGER(SIZE_T), INTENT(OUT), DIMENSION(*) :: str_len 
         !              CHARACTER, INTENT(IN), &
         !              DIMENSION(dims(1)*dims(2)) :: tmp_buf
         CHARACTER(LEN=*), DIMENSION(dims(2)) :: buf
       END FUNCTION h5dread_vl_string_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 
    !            do i = 1, dims(2)
    !               do j = 1, dims(1)
    !               tmp_buf((i-1)*dims(1) +j) = buf(i)(j:j)
    !               enddo
    !            enddo 
    !              write(*,*) (tmp_buf(j:j), j=1,dims(1)*dims(2))
    !              write(*,*) str_len(1), str_len(2), str_len(3), str_len(4)

    hdferr = h5dread_vl_string_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims, str_len)
    RETURN 
  END SUBROUTINE h5dread_vl_string

!----------------------------------------------------------------------
! Name:		h5dget_space_f 
!
! Purpose:	Returns an identifier for a copy of the dataspace for a 
!		dataset.   	
!
! Inputs:  
!		dataset_id	- dataset identifier
! Outputs:  
!		dataspace_id	- dataspace identifier
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE			
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  February 28, 2001
!
! Comment:		
!----------------------------------------------------------------------
  SUBROUTINE h5dget_space_f(dataset_id, dataspace_id, hdferr)
    IMPLICIT NONE 
    INTEGER(HID_T), INTENT(IN) :: dataset_id      ! Dataset identifier
    INTEGER(HID_T), INTENT(OUT) :: dataspace_id   ! Dataspace identifier
    INTEGER, INTENT(OUT) :: hdferr                ! Error code 
    
!            INTEGER, EXTERNAL :: h5dget_space_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
    INTERFACE
       INTEGER FUNCTION h5dget_space_c(dataset_id, dataspace_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DGET_SPACE_C'::h5dget_space_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dataset_id
         INTEGER(HID_T), INTENT(OUT) :: dataspace_id
       END FUNCTION h5dget_space_c
    END INTERFACE
    
    hdferr = h5dget_space_c(dataset_id, dataspace_id)
  END SUBROUTINE h5dget_space_f

END MODULE H5D


