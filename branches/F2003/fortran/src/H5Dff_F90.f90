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
MODULE H5D_F03
  USE H5GLOBAL

! NOTES: (1) The maximum rank of an array allowed in Fortran is 7, therefore 
!            we only provide an interface for arrays up to and including rank 7.
!       
!        (2) Unfortunately we are using a generic interface and one of the factors
!            used in determining the proper routine to select is that of the array 
!            rank being passed, therefore we can not create just one subroutine for
!            each array type (integer, real, etc...) of various ranks and then use a 
!            rank 1 array of assumed size in the just one subroutine, 
!                          (i.e. integer, dimension(*) :: ... )
!                          (i.e. real   , dimension(*) :: ... ) etc...

  INTERFACE h5dwrite_f

     MODULE PROCEDURE h5dwrite_reference_obj
     MODULE PROCEDURE h5dwrite_reference_dsetreg
     MODULE PROCEDURE h5dwrite_integer_scalar
     MODULE PROCEDURE h5dwrite_integer_1 
     MODULE PROCEDURE h5dwrite_integer_2 
     MODULE PROCEDURE h5dwrite_integer_3 
     MODULE PROCEDURE h5dwrite_integer_4 
     MODULE PROCEDURE h5dwrite_integer_5 
     MODULE PROCEDURE h5dwrite_integer_6 
     MODULE PROCEDURE h5dwrite_integer_7 
     MODULE PROCEDURE h5dwrite_char_scalar
     MODULE PROCEDURE h5dwrite_char_1 
     MODULE PROCEDURE h5dwrite_char_2 
     MODULE PROCEDURE h5dwrite_char_3 
     MODULE PROCEDURE h5dwrite_char_4 
     MODULE PROCEDURE h5dwrite_char_5 
     MODULE PROCEDURE h5dwrite_char_6 
     MODULE PROCEDURE h5dwrite_char_7 
     MODULE PROCEDURE h5dwrite_real_scalar
     MODULE PROCEDURE h5dwrite_real_1
     MODULE PROCEDURE h5dwrite_real_2
     MODULE PROCEDURE h5dwrite_real_3
     MODULE PROCEDURE h5dwrite_real_4
     MODULE PROCEDURE h5dwrite_real_5
     MODULE PROCEDURE h5dwrite_real_6
     MODULE PROCEDURE h5dwrite_real_7
     ! Comment if on Crays
     MODULE PROCEDURE h5dwrite_double_scalar
     MODULE PROCEDURE h5dwrite_double_1
     MODULE PROCEDURE h5dwrite_double_2
     MODULE PROCEDURE h5dwrite_double_3
     MODULE PROCEDURE h5dwrite_double_4
     MODULE PROCEDURE h5dwrite_double_5
     MODULE PROCEDURE h5dwrite_double_6
     MODULE PROCEDURE h5dwrite_double_7
     ! End comment if on Crays

  END INTERFACE

  INTERFACE h5dread_f

     MODULE PROCEDURE h5dread_reference_obj
     MODULE PROCEDURE h5dread_reference_dsetreg
     MODULE PROCEDURE h5dread_integer_scalar
     MODULE PROCEDURE h5dread_integer_1 
     MODULE PROCEDURE h5dread_integer_2 
     MODULE PROCEDURE h5dread_integer_3 
     MODULE PROCEDURE h5dread_integer_4 
     MODULE PROCEDURE h5dread_integer_5 
     MODULE PROCEDURE h5dread_integer_6 
     MODULE PROCEDURE h5dread_integer_7 
     MODULE PROCEDURE h5dread_char_scalar
     MODULE PROCEDURE h5dread_char_1 
     MODULE PROCEDURE h5dread_char_2 
     MODULE PROCEDURE h5dread_char_3 
     MODULE PROCEDURE h5dread_char_4 
     MODULE PROCEDURE h5dread_char_5 
     MODULE PROCEDURE h5dread_char_6 
     MODULE PROCEDURE h5dread_char_7 
     MODULE PROCEDURE h5dread_real_scalar
     MODULE PROCEDURE h5dread_real_1
     MODULE PROCEDURE h5dread_real_2
     MODULE PROCEDURE h5dread_real_3
     MODULE PROCEDURE h5dread_real_4
     MODULE PROCEDURE h5dread_real_5
     MODULE PROCEDURE h5dread_real_6
     MODULE PROCEDURE h5dread_real_7
     ! Comment if on Crays
     MODULE PROCEDURE h5dread_double_scalar
     MODULE PROCEDURE h5dread_double_1
     MODULE PROCEDURE h5dread_double_2
     MODULE PROCEDURE h5dread_double_3
     MODULE PROCEDURE h5dread_double_4
     MODULE PROCEDURE h5dread_double_5
     MODULE PROCEDURE h5dread_double_6
     MODULE PROCEDURE h5dread_double_7
     ! End comment if on Crays

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

  INTERFACE h5dfill_f
     MODULE PROCEDURE h5dfill_integer
     MODULE PROCEDURE h5dfill_real
     MODULE PROCEDURE h5dfill_double
     MODULE PROCEDURE h5dfill_char
  END INTERFACE

CONTAINS

  SUBROUTINE h5dread_integer_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_scalar
    !DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT) :: buf ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dread_integer_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_S_C'::h5dread_integer_s_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(OUT) :: buf
       END FUNCTION h5dread_integer_s_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_integer_scalar

  SUBROUTINE h5dread_integer_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT), DIMENSION(dims(1)) :: buf
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
    !
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_1_C'::h5dread_integer_1_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(OUT), DIMENSION(dims(1)) :: buf
       END FUNCTION h5dread_integer_1_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_integer_1

  SUBROUTINE h5dread_integer_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT), DIMENSION(dims(1),dims(2)) :: buf
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
    !
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_2_C'::h5dread_integer_2_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(OUT), DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dread_integer_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dread_integer_2

  SUBROUTINE h5dread_integer_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_3_C'::h5dread_integer_3_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(OUT), DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dread_integer_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dread_integer_3

  SUBROUTINE h5dread_integer_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT), DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
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

    ! 
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_4_C'::h5dread_integer_4_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(OUT), DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
       END FUNCTION h5dread_integer_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dread_integer_4

  SUBROUTINE h5dread_integer_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT), DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !   
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_5_C'::h5dread_integer_5_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dread_integer_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dread_integer_5

  SUBROUTINE h5dread_integer_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
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

    !
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_6_C'::h5dread_integer_6_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dread_integer_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dread_integer_6

  SUBROUTINE h5dread_integer_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_integer_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_integer_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_integer_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_INTEGER_7_C'::h5dread_integer_7_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dread_integer_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_integer_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dread_integer_7

  SUBROUTINE h5dread_char_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_scalar
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT) :: buf ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dreadc_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_S_C'::h5dreadc_s_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(OUT) :: buf
       END FUNCTION h5dreadc_s_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_scalar

  SUBROUTINE h5dread_char_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dreadc_1_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_1_C'::h5dreadc_1_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(INOUT), &
              DIMENSION(dims(1)) :: buf
       END FUNCTION h5dreadc_1_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_1

  SUBROUTINE h5dread_char_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dreadc_2_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_2_C'::h5dreadc_2_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(INOUT), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dreadc_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_2

  SUBROUTINE h5dread_char_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dreadc_3_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_3_C'::h5dreadc_3_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dreadc_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_3

  SUBROUTINE h5dread_char_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dreadc_4_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_4_C'::h5dreadc_4_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
       END FUNCTION h5dreadc_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_4

  SUBROUTINE h5dread_char_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dreadc_5_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_5_C'::h5dreadc_5_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dreadc_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_5

  SUBROUTINE h5dread_char_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dreadc_6_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_6_C'::h5dreadc_6_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dreadc_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_6

  SUBROUTINE h5dread_char_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_char_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dreadc_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dreadc_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREADC_7_C'::h5dreadc_7_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dreadc_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dreadc_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_char_7

  SUBROUTINE h5dread_real_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_scalar
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT) :: buf ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dread_real_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_S_C'::h5dread_real_s_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(OUT) :: buf
       END FUNCTION h5dread_real_s_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_scalar

  SUBROUTINE h5dread_real_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_real_1_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_1_C'::h5dread_real_1_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(INOUT), &
              DIMENSION(dims(1)) :: buf
       END FUNCTION h5dread_real_1_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_1

  SUBROUTINE h5dread_real_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_real_2_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_2_C'::h5dread_real_2_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dread_real_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_2

  SUBROUTINE h5dread_real_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_real_3_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_3_C'::h5dread_real_3_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(INOUT), & 
              DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dread_real_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_3

  SUBROUTINE h5dread_real_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3), dims(4)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_real_4_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_4_C'::h5dread_real_4_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3), dims(4)) :: buf
       END FUNCTION h5dread_real_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_4

  SUBROUTINE h5dread_real_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_real_5_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_5_C'::h5dread_real_5_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(INOUT), & 
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dread_real_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_5

  SUBROUTINE h5dread_real_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_real_6_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_6_C'::h5dread_real_6_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dread_real_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_6

  SUBROUTINE h5dread_real_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_real_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_real_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_real_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_REAL_7_C'::h5dread_real_7_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dread_real_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_real_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_real_7

  SUBROUTINE h5dread_double_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_scalar
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT) :: buf ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dread_double_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_S_C'::h5dread_double_s_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(OUT) :: buf
       END FUNCTION h5dread_double_s_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_scalar

  SUBROUTINE h5dread_double_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_double_1_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_1_C'::h5dread_double_1_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(INOUT), &
              DIMENSION(dims(1)) :: buf
       END FUNCTION h5dread_double_1_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_1

  SUBROUTINE h5dread_double_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_double_2_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_2_C'::h5dread_double_2_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(INOUT), & 
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dread_double_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_2

  SUBROUTINE h5dread_double_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_double_3_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_3_C'::h5dread_double_3_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dread_double_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_3

  SUBROUTINE h5dread_double_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
    ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dread_double_4_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_4_C'::h5dread_double_4_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
       END FUNCTION h5dread_double_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_4

  SUBROUTINE h5dread_double_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dread_double_5_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_5_C'::h5dread_double_5_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dread_double_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_5

  SUBROUTINE h5dread_double_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
    ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dread_double_6_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_6_C'::h5dread_double_6_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dread_double_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_6

  SUBROUTINE h5dread_double_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_double_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(INOUT), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
    ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dread_double_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dread_double_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DREAD_DOUBLE_7_C'::h5dread_double_7_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(INOUT), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dread_double_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dread_double_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dread_double_7


  SUBROUTINE h5dwrite_integer_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_scalar
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER, INTENT(IN) :: buf ! Data buffer
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_S_C'::h5dwrite_integer_s_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN) :: buf
       END FUNCTION h5dwrite_integer_s_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_integer_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_integer_scalar

  SUBROUTINE h5dwrite_integer_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_1_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_1_C'::h5dwrite_integer_1_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1)) :: buf
       END FUNCTION h5dwrite_integer_1_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_integer_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_integer_1

  SUBROUTINE h5dwrite_integer_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(IN), &
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_2_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_2_C'::h5dwrite_integer_2_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dwrite_integer_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 
    hdferr = h5dwrite_integer_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dwrite_integer_2

  SUBROUTINE h5dwrite_integer_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_3_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_3_C'::h5dwrite_integer_3_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dwrite_integer_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_integer_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dwrite_integer_3

  SUBROUTINE h5dwrite_integer_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_4_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_4_C'::h5dwrite_integer_4_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
       END FUNCTION h5dwrite_integer_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F
    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_integer_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dwrite_integer_4

  SUBROUTINE h5dwrite_integer_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_5_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_5_C'::h5dwrite_integer_5_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dwrite_integer_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F


    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_integer_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dwrite_integer_5

  SUBROUTINE h5dwrite_integer_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_6_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_6_C'::h5dwrite_integer_6_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dwrite_integer_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_integer_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dwrite_integer_6

  SUBROUTINE h5dwrite_integer_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_integer_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    INTEGER, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_integer_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_integer_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_INTEGER_7_C'::h5dwrite_integer_7_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         INTEGER, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dwrite_integer_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_integer_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, &
         buf, dims)

  END SUBROUTINE h5dwrite_integer_7


  SUBROUTINE h5dwrite_char_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_scalar
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN) :: buf ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dwritec_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_S_C'::h5dwritec_s_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN) :: buf
       END FUNCTION h5dwritec_s_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_scalar

  SUBROUTINE h5dwrite_char_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwritec_1_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_1_C'::h5dwritec_1_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN), &
              DIMENSION(dims(1)) :: buf
       END FUNCTION h5dwritec_1_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_1

  SUBROUTINE h5dwrite_char_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwritec_2_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_2_C'::h5dwritec_2_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dwritec_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_2

  SUBROUTINE h5dwrite_char_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwritec_3_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_3_C'::h5dwritec_3_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dwritec_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_3

  SUBROUTINE h5dwrite_char_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwritec_4_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_4_C'::h5dwritec_4_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
       END FUNCTION h5dwritec_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_4

  SUBROUTINE h5dwrite_char_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwritec_5_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_5_C'::h5dwritec_5_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dwritec_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_5

  SUBROUTINE h5dwrite_char_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwritec_6_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_6_C'::h5dwritec_6_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dwritec_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_6

  SUBROUTINE h5dwrite_char_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_char_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    CHARACTER(LEN=*), INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwritec_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwritec_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITEC_7_C'::h5dwritec_7_c
         !DEC$ ENDIF
         !DEC$ATTRIBUTES reference :: buf 
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         CHARACTER(LEN=*), INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dwritec_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwritec_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_char_7

  SUBROUTINE h5dwrite_real_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_scalar
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN) :: buf ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_S_C'::h5dwrite_real_s_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN) :: buf
       END FUNCTION h5dwrite_real_s_c
    END INTERFACE


    xfer_prp_default  = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F
    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_scalar

  SUBROUTINE h5dwrite_real_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN), &
         DIMENSION(dims(1)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_1_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_1_C'::h5dwrite_real_1_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN), &
              DIMENSION(dims(1)) :: buf
       END FUNCTION h5dwrite_real_1_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_1

  SUBROUTINE h5dwrite_real_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_2_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_2_C'::h5dwrite_real_2_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dwrite_real_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_2

  SUBROUTINE h5dwrite_real_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_3_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_3_C'::h5dwrite_real_3_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dwrite_real_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_3

  SUBROUTINE h5dwrite_real_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_4_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_4_C'::h5dwrite_real_4_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
       END FUNCTION h5dwrite_real_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_4

  SUBROUTINE h5dwrite_real_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_5_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_5_C'::h5dwrite_real_5_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dwrite_real_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_5

  SUBROUTINE h5dwrite_real_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_6_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_6_C'::h5dwrite_real_6_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dwrite_real_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_6

  SUBROUTINE h5dwrite_real_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_real_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    REAL, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_real_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_real_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_REAL_7_C'::h5dwrite_real_7_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         REAL, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dwrite_real_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_real_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_real_7


  SUBROUTINE h5dwrite_double_scalar(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_scalar
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN) :: buf ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_s_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_s_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_S_C'::h5dwrite_double_s_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN) :: buf
       END FUNCTION h5dwrite_double_s_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_s_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_scalar

  SUBROUTINE h5dwrite_double_1(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_1
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_1_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_1_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_1_C'::h5dwrite_double_1_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN), &
              DIMENSION(dims(1)) :: buf
       END FUNCTION h5dwrite_double_1_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_1_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_1

  SUBROUTINE h5dwrite_double_2(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_2
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_2_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_2_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_2_C'::h5dwrite_double_2_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN), &
              DIMENSION(dims(1),dims(2)) :: buf
       END FUNCTION h5dwrite_double_2_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_2_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_2

  SUBROUTINE h5dwrite_double_3(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_3
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_3_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_3_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_3_C'::h5dwrite_double_3_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3)) :: buf
       END FUNCTION h5dwrite_double_3_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_3_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_3

  SUBROUTINE h5dwrite_double_4(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_4
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_4_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_4_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_4_C'::h5dwrite_double_4_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4)) :: buf
       END FUNCTION h5dwrite_double_4_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_4_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_4

  SUBROUTINE h5dwrite_double_5(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_5
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_5_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_5_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_5_C'::h5dwrite_double_5_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5)) :: buf
       END FUNCTION h5dwrite_double_5_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_5_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_5

  SUBROUTINE h5dwrite_double_6(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_6
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
    ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_6_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_6_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_6_C'::h5dwrite_double_6_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6)) :: buf
       END FUNCTION h5dwrite_double_6_c
    END INTERFACE


    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_6_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_6

  SUBROUTINE h5dwrite_double_7(dset_id, mem_type_id, buf, dims, hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_double_7
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
    DOUBLE PRECISION, INTENT(IN), &
         DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
    ! Data buffer
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

    !            INTEGER, EXTERNAL :: h5dwrite_double_7_c
    !  MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dwrite_double_7_c(dset_id, mem_type_id, &
            mem_space_id_default, & 
            file_space_id_default, &
            xfer_prp_default, buf, dims)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DWRITE_DOUBLE_7_C'::h5dwrite_double_7_c
         !DEC$ ENDIF
         INTEGER(HID_T), INTENT(IN) :: dset_id
         INTEGER(HID_T), INTENT(IN) :: mem_type_id
         INTEGER(HID_T)  :: mem_space_id_default
         INTEGER(HID_T) :: file_space_id_default
         INTEGER(HID_T) :: xfer_prp_default
         INTEGER(HSIZE_T), INTENT(IN), DIMENSION(*) :: dims
         DOUBLE PRECISION, INTENT(IN), &
              DIMENSION(dims(1),dims(2),dims(3),dims(4),dims(5),dims(6),dims(7)) :: buf
       END FUNCTION h5dwrite_double_7_c
    END INTERFACE

    xfer_prp_default = H5P_DEFAULT_F
    mem_space_id_default = H5S_ALL_F
    file_space_id_default = H5S_ALL_F

    IF (PRESENT(xfer_prp)) xfer_prp_default = xfer_prp 
    IF (PRESENT(mem_space_id))  mem_space_id_default = mem_space_id 
    IF (PRESENT(file_space_id)) file_space_id_default = file_space_id 

    hdferr = h5dwrite_double_7_c(dset_id, mem_type_id, mem_space_id_default, &
         file_space_id_default, xfer_prp_default, buf, dims)

  END SUBROUTINE h5dwrite_double_7

  SUBROUTINE h5dwrite_vl_integer(dset_id, mem_type_id, buf, dims, len, &
       hdferr, &
       mem_space_id, file_space_id, xfer_prp)
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_vl_integer
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! MAX len x num_elem
    INTEGER(SIZE_T), INTENT(IN), DIMENSION(*) :: len ! Array to store 
                                                     ! the length of each
                                                     ! element
    INTEGER, INTENT(IN), &
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
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_vl_integer
!DEC$endif

    IMPLICIT NONE
    INTEGER(HID_T), INTENT(IN) :: dset_id   ! Dataset identifier
    INTEGER(HID_T), INTENT(IN) :: mem_type_id ! Memory datatype identifier
    INTEGER(HSIZE_T), INTENT(IN), DIMENSION(2) :: dims ! MAX len x num_elem
    INTEGER(SIZE_T), INTENT(INOUT), DIMENSION(*) :: len ! Array to store 
                                                        ! the length of each
                                                        ! element
    INTEGER, INTENT(INOUT), &
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
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_vl_real
!DEC$endif

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
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_vl_real
!DEC$endif

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
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dwrite_vl_string
!DEC$endif

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
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dread_vl_string
!DEC$endif

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

    if (present(xfer_prp)) xfer_prp_default = xfer_prp 
    if (present(mem_space_id))  mem_space_id_default = mem_space_id 
    if (present(file_space_id)) file_space_id_default = file_space_id 
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
!This definition is needed for Windows DLLs
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$attributes dllexport :: h5dget_space_f
!DEC$endif
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

  !----------------------------------------------------------------------
  ! Name:		h5dfill_integer
  !
  ! Purpose:      Fills dataspace elements with a fill value in a memory buffer.	
  !               Only INTEGER, CHARACTER, REAL and DOUBLE PRECISION datatypes 
  !               of the fillvalues and buffers are supported. Buffer and fillvalue
  !               are assumed to have the same datatype.
  !               Only one-dimesional buffers are supported.
  !
  ! Inputs:  
  !		fill_value	- fill value
  !		space_id	- memory space selection identifier
  !		buf		- data buffer iin memory ro apply selection to
  !				- of k-th dimension of the buf array
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  !
  ! Programmer:	Elena Pourmal
  !		March 12, 2003
  !
  !----------------------------------------------------------------------

  SUBROUTINE h5dfill_integer(fill_value, space_id, buf,  hdferr)
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5dfill_integer
    !DEC$endif

    IMPLICIT NONE
    INTEGER, INTENT(IN) :: fill_value  ! Fill value
    INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
    INTEGER, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 

    INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
    INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 

    !            INTEGER, EXTERNAL :: h5dfill_integer_c
    ! MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dfill_integer_c(fill_value, fill_type_id, space_id, &
            buf, mem_type_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DFILL_INTEGER_C'::h5dfill_integer_c  
         !DEC$ ENDIF
         INTEGER, INTENT(IN) :: fill_value  ! Fill value
         INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
         INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
         INTEGER, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
         INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 
       END FUNCTION h5dfill_integer_c
    END INTERFACE
    fill_type_id = H5T_NATIVE_INTEGER
    mem_type_id  = H5T_NATIVE_INTEGER

    hdferr = h5dfill_integer_c(fill_value, fill_type_id, space_id, & 
         buf, mem_type_id)

  END SUBROUTINE h5dfill_integer

  !----------------------------------------------------------------------
  ! Name:		h5dfill_real
  !
  ! Purpose:      Fills dataspace elements with a fill value in a memory buffer.	
  !               Only INTEGER, CHARACTER, REAL and DOUBLE PRECISION datatypes 
  !               of the fillvalues and buffers are supported. Buffer and fillvalue
  !               are assumed to have the same datatype.
  !               Only one-dimesional buffers are supported.
  !
  ! Inputs:  
  !		fill_value	- fill value
  !		space_id	- memory space selection identifier
  !		buf		- data buffer iin memory ro apply selection to
  !				- of k-th dimension of the buf array
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  !
  ! Programmer:	Elena Pourmal
  !		March 12, 2003
  !
  !----------------------------------------------------------------------

  SUBROUTINE h5dfill_real(fill_valuer, space_id, buf,  hdferr)
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5dfill_real
    !DEC$endif

    IMPLICIT NONE
    REAL, INTENT(IN) :: fill_valuer  ! Fill value
    INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
    REAL, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 

    INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
    INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 

    !            INTEGER, EXTERNAL :: h5dfill_real_c
    ! MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dfill_real_c(fill_valuer, fill_type_id, space_id, &
            buf, mem_type_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DFILL_REAL_C'::h5dfill_real_c  
         !DEC$ ENDIF
         REAL, INTENT(IN) :: fill_valuer  ! Fill value
         INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
         INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
         REAL, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
         INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 
       END FUNCTION h5dfill_real_c
    END INTERFACE
    fill_type_id = H5T_NATIVE_REAL
    mem_type_id  = H5T_NATIVE_REAL

    hdferr = h5dfill_real_c(fill_valuer, fill_type_id, space_id, & 
         buf, mem_type_id)
  END SUBROUTINE h5dfill_real

  !----------------------------------------------------------------------
  ! Name:		h5dfill_double
  !
  ! Purpose:      Fills dataspace elements with a fill value in a memory buffer.	
  !               Only INTEGER, CHARACTER, REAL and DOUBLE PRECISION datatypes 
  !               of the fillvalues and buffers are supported. Buffer and fillvalue
  !               are assumed to have the same datatype.
  !               Only one-dimesional buffers are supported.
  !
  ! Inputs:  
  !		fill_value	- fill value
  !		space_id	- memory space selection identifier
  !		buf		- data buffer iin memory ro apply selection to
  !				- of k-th dimension of the buf array
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  !
  ! Programmer:	Elena Pourmal
  !		March 12, 2003
  !
  !----------------------------------------------------------------------

  SUBROUTINE h5dfill_double(fill_value, space_id, buf,  hdferr)
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5dfill_double
    !DEC$endif

    IMPLICIT NONE
    DOUBLE PRECISION, INTENT(IN) :: fill_value  ! Fill value
    INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
    DOUBLE PRECISION, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 

    INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
    INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 

    !            INTEGER, EXTERNAL :: h5dfill_double_c
    ! MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dfill_double_c(fill_value, fill_type_id, space_id, &
            buf, mem_type_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DFILL_DOUBLE_C'::h5dfill_double_c  
         !DEC$ ENDIF
         DOUBLE PRECISION, INTENT(IN) :: fill_value  ! Fill value
         INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
         INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
         DOUBLE PRECISION, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
         INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 
       END FUNCTION h5dfill_double_c
    END INTERFACE
    fill_type_id = H5T_NATIVE_DOUBLE
    mem_type_id  = H5T_NATIVE_DOUBLE

    hdferr = h5dfill_double_c(fill_value, fill_type_id, space_id, & 
         buf, mem_type_id)

  END SUBROUTINE h5dfill_double

  !----------------------------------------------------------------------
  ! Name:		h5dfill_char
  !
  ! Purpose:      Fills dataspace elements with a fill value in a memory buffer.	
  !               Only INTEGER, CHARACTER, REAL and DOUBLE PRECISION datatypes 
  !               of the fillvalues and buffers are supported. Buffer and fillvalue
  !               are assumed to have the same datatype.
  !               Only one-dimesional buffers are supported.
  !
  ! Inputs:  
  !		fill_value	- fill value
  !		space_id	- memory space selection identifier
  !		buf		- data buffer iin memory ro apply selection to
  !				- of k-th dimension of the buf array
  ! Outputs:  
  !		hdferr:		- error code		
  !				 	Success:  0
  !				 	Failure: -1   
  !
  ! Programmer:	Elena Pourmal
  !		March 12, 2003
  !
  !----------------------------------------------------------------------

  SUBROUTINE h5dfill_char(fill_value, space_id, buf,  hdferr)
    !This definition is needed for Windows DLLs
    !DEC$if defined(BUILD_HDF5_DLL)
    !DEC$attributes dllexport :: h5dfill_char
    !DEC$endif

    IMPLICIT NONE
    CHARACTER, INTENT(IN) :: fill_value  ! Fill value
    INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
    CHARACTER, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
    INTEGER, INTENT(OUT) :: hdferr      ! Error code 

    INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
    INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 

    !            INTEGER, EXTERNAL :: h5dfillc_c
    ! MS FORTRAN needs explicit interface for C functions called here.
    !
    INTERFACE
       INTEGER FUNCTION h5dfillc_c(fill_value, fill_type_id, space_id, &
            buf, mem_type_id)
         USE H5GLOBAL
         !DEC$ IF DEFINED(HDF5F90_WINDOWS)
         !DEC$ ATTRIBUTES C,reference,decorate,alias:'H5DFILLC_C'::h5dfillc_c  
         !DEC$ ENDIF
         CHARACTER, INTENT(IN) :: fill_value  ! Fill value
         INTEGER(HID_T) :: fill_type_id ! Fill value datatype identifier
         INTEGER(HID_T), INTENT(IN) :: space_id ! Memory dataspace selection identifier
         CHARACTER, INTENT(IN), DIMENSION(*) :: buf ! Memory buffer to fill in
         INTEGER(HID_T) :: mem_type_id !  Buffer dadtype identifier 
       END FUNCTION h5dfillc_c
    END INTERFACE
    fill_type_id = H5T_NATIVE_CHARACTER
    mem_type_id  = H5T_NATIVE_CHARACTER

    hdferr = h5dfillc_c(fill_value, fill_type_id, space_id, & 
         buf, mem_type_id)

  END SUBROUTINE h5dfill_char


END MODULE H5D_F03
