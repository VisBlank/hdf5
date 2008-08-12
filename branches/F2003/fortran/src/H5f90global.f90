! COPYRIGHT
!
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
! NAME
!    H5GLOBAL
!
! DESCRIPTION
!    Definitions for reference datatypes.
!
! USES
!    H5FORTRAN_TYPES - This module is generated at run time. See 
!                      H5fortran_types.f90 for further informaton.
!
! NOTES
!    If you change the values of the variables declared as PARAMETER, 
!    the corresponding values must be changed in the H5f90.h file
!
!
    MODULE H5GLOBAL
      USE H5FORTRAN_TYPES
! 
!
! Definitions for reference datatypes.
! If you change the value of these parameters, do not forget to change corresponding
! values in the H5f90.h file. 
      INTEGER, PARAMETER :: REF_REG_BUF_LEN = 3 

!      TYPE, bind(C) :: hobj_ref_t_f
      TYPE :: hobj_ref_t_f
         INTEGER(HADDR_T) ref
      END TYPE hobj_ref_t_f

!      TYPE, bind(C) :: hdset_reg_ref_t_f
      TYPE :: hdset_reg_ref_t_f
         INTEGER ref(REF_REG_BUF_LEN) 
      END TYPE hdset_reg_ref_t_f

      INTEGER, PARAMETER :: PREDEF_TYPES_LEN = 10 ! Do not forget to change this
                                                  ! value when new predefined
                                                  ! datatypes are added
      ! Do not forget to change the following line when new predefined 
      ! floating data types are added
      INTEGER, PARAMETER :: FLOATING_TYPES_LEN = 4

      ! Do not forget to change the following line when new predefined 
      ! integer data types are added
      INTEGER, PARAMETER :: INTEGER_TYPES_LEN = 17

      INTEGER(HID_T) H5T_NATIVE_INTEGER_1, &
           H5T_NATIVE_INTEGER_2, &
           H5T_NATIVE_INTEGER_4, &
           H5T_NATIVE_INTEGER_8, &
           H5T_NATIVE_INTEGER, &
           H5T_NATIVE_REAL, &
           H5T_NATIVE_DOUBLE, &
           H5T_NATIVE_CHARACTER , &
           H5T_STD_REF_OBJ,      &
           H5T_STD_REF_DSETREG, &
           H5T_IEEE_F32BE,  &
           H5T_IEEE_F32LE,  &
           H5T_IEEE_F64BE,  &
           H5T_IEEE_F64LE,  &
           H5T_STD_I8BE,    &
           H5T_STD_I8LE,    &
           H5T_STD_I16BE,   &
           H5T_STD_I16LE,   &
           H5T_STD_I32BE,   &
           H5T_STD_I32LE,   &
           H5T_STD_I64BE,   &
           H5T_STD_I64LE,   &
           H5T_STD_U8BE,    &
           H5T_STD_U8LE,    &
           H5T_STD_U16BE,   &
           H5T_STD_U16LE,   &
           H5T_STD_U32BE,   &
           H5T_STD_U32LE,   &
           H5T_STD_U64BE,   &
           H5T_STD_U64LE,   &
           H5T_STRING


      INTEGER(HID_T), DIMENSION(PREDEF_TYPES_LEN) :: predef_types
      EQUIVALENCE (predef_types(1), H5T_NATIVE_INTEGER)
      EQUIVALENCE (predef_types(2), H5T_NATIVE_REAL)
      EQUIVALENCE (predef_types(3), H5T_NATIVE_DOUBLE)
      EQUIVALENCE (predef_types(4), H5T_NATIVE_CHARACTER)
      EQUIVALENCE (predef_types(5), H5T_STD_REF_OBJ)
      EQUIVALENCE (predef_types(6), H5T_STD_REF_DSETREG)
      EQUIVALENCE (predef_types(7), H5T_NATIVE_INTEGER_1)
      EQUIVALENCE (predef_types(8), H5T_NATIVE_INTEGER_2)
      EQUIVALENCE (predef_types(9), H5T_NATIVE_INTEGER_4)
      EQUIVALENCE (predef_types(10), H5T_NATIVE_INTEGER_8)
      
      
      INTEGER(HID_T), DIMENSION(FLOATING_TYPES_LEN) :: floating_types
      EQUIVALENCE (floating_types(1), H5T_IEEE_F32BE )
      EQUIVALENCE (floating_types(2), H5T_IEEE_F32LE)
      EQUIVALENCE (floating_types(3), H5T_IEEE_F64BE)
      EQUIVALENCE (floating_types(4), H5T_IEEE_F64LE)
      
      INTEGER(HID_T), DIMENSION(INTEGER_TYPES_LEN) :: integer_types
      EQUIVALENCE (integer_types(1), H5T_STD_I8BE )
      EQUIVALENCE (integer_types(2), H5T_STD_I8LE)
      EQUIVALENCE (integer_types(3), H5T_STD_I16BE)
      EQUIVALENCE (integer_types(4), H5T_STD_I16LE)
      EQUIVALENCE (integer_types(5), H5T_STD_I32BE)
      EQUIVALENCE (integer_types(6), H5T_STD_I32LE)
      EQUIVALENCE (integer_types(7), H5T_STD_I64BE)
      EQUIVALENCE (integer_types(8), H5T_STD_I64LE)
      EQUIVALENCE (integer_types(9), H5T_STD_U8BE)
      EQUIVALENCE (integer_types(10), H5T_STD_U8LE)
      EQUIVALENCE (integer_types(11), H5T_STD_U16BE)
      EQUIVALENCE (integer_types(12), H5T_STD_U16LE)
      EQUIVALENCE (integer_types(13), H5T_STD_U32BE)
      EQUIVALENCE (integer_types(14), H5T_STD_U32LE)
      EQUIVALENCE (integer_types(15), H5T_STD_U64BE)
      EQUIVALENCE (integer_types(16), H5T_STD_U64LE)
      EQUIVALENCE (integer_types(17), H5T_STRING)


!      COMMON /PREDEFINED_TYPES/ H5T_NATIVE_INTEGER, &
!                                H5T_NATIVE_REAL, &
!                                H5T_NATIVE_DOUBLE, &
!                                H5T_NATIVE_CHARACTER, &
!                                H5T_STD_REF_OBJ, &
!                                H5T_STD_REF_DSETREG
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /PREDEFINED_TYPES/
!DEC$endif
      COMMON /PREDEFINED_TYPES/  predef_types

!      COMMON /FLOATING_TYPES/ H5T_IEEE_F32BE,  &
!                              H5T_IEEE_F32LE,  &
!                              H5T_IEEE_F64BE,  &
!                              H5T_IEEE_F64LE
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /FLOATING_TYPES/
!DEC$endif
      COMMON /FLOATING_TYPES/ floating_types 
!
!      COMMON /INTEGER_TYPES/ H5T_STD_I8BE,  &
!                             H5T_STD_I8LE,    &
!                             H5T_STD_I16BE,   &
!                             H5T_STD_I16LE,   &
!                             H5T_STD_I32BE,   &
!                             H5T_STD_I32LE,   &
!                             H5T_STD_I64BE,   &
!                             H5T_STD_I64LE,   &
!                             H5T_STD_U8BE,    &
!                             H5T_STD_U8LE,    &
!                             H5T_STD_U16BE,   &
!                             H5T_STD_U16LE,   &
!                             H5T_STD_U32BE,   &
!                             H5T_STD_U32LE,   &
!                             H5T_STD_U64BE,   &
!                             H5T_STD_U64LE
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /INTEGER_TYPES/
!DEC$endif
      COMMON /INTEGER_TYPES/ integer_types
!
! Fortran flags
!
!
! H5F flags (DO NOT FORGET TO UPDATE WHEN NEW FLAGS ARE ADDEDD !)
!
! H5F flags declaration
!
      INTEGER, PARAMETER :: H5F_FLAGS_LEN = 19
      INTEGER H5F_flags(H5F_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5F_FLAGS/
!DEC$endif
      COMMON /H5F_FLAGS/ H5F_flags

      INTEGER :: H5F_ACC_RDWR_F 
      INTEGER :: H5F_ACC_RDONLY_F
      INTEGER :: H5F_ACC_TRUNC_F
      INTEGER :: H5F_ACC_EXCL_F
      INTEGER :: H5F_ACC_DEBUG_F
      INTEGER :: H5F_SCOPE_LOCAL_F
      INTEGER :: H5F_SCOPE_GLOBAL_F
      INTEGER :: H5F_CLOSE_DEFAULT_F
      INTEGER :: H5F_CLOSE_WEAK_F
      INTEGER :: H5F_CLOSE_SEMI_F
      INTEGER :: H5F_CLOSE_STRONG_F
      INTEGER :: H5F_OBJ_FILE_F
      INTEGER :: H5F_OBJ_DATASET_F
      INTEGER :: H5F_OBJ_GROUP_F
      INTEGER :: H5F_OBJ_DATATYPE_F
      INTEGER :: H5F_OBJ_ALL_F
      INTEGER :: H5F_LIBVER_EARLIEST_F
      INTEGER :: H5F_LIBVER_LATEST_F

      EQUIVALENCE(H5F_flags(1), H5F_ACC_RDWR_F)
      EQUIVALENCE(H5F_flags(2), H5F_ACC_RDONLY_F)
      EQUIVALENCE(H5F_flags(3), H5F_ACC_TRUNC_F)
      EQUIVALENCE(H5F_flags(4), H5F_ACC_EXCL_F)
      EQUIVALENCE(H5F_flags(5), H5F_ACC_DEBUG_F)
      EQUIVALENCE(H5F_flags(6), H5F_SCOPE_LOCAL_F)
      EQUIVALENCE(H5F_flags(7), H5F_SCOPE_GLOBAL_F)
      EQUIVALENCE(H5F_flags(8), H5F_CLOSE_DEFAULT_F)
      EQUIVALENCE(H5F_flags(9), H5F_CLOSE_WEAK_F)
      EQUIVALENCE(H5F_flags(10), H5F_CLOSE_SEMI_F)
      EQUIVALENCE(H5F_flags(11), H5F_CLOSE_STRONG_F)
      EQUIVALENCE(H5F_flags(12), H5F_OBJ_FILE_F)
      EQUIVALENCE(H5F_flags(13), H5F_OBJ_DATASET_F)
      EQUIVALENCE(H5F_flags(14), H5F_OBJ_GROUP_F)
      EQUIVALENCE(H5F_flags(15), H5F_OBJ_DATATYPE_F)
      EQUIVALENCE(H5F_flags(16), H5F_OBJ_ALL_F)
      EQUIVALENCE(H5F_flags(17), H5F_LIBVER_EARLIEST_F)
      EQUIVALENCE(H5F_flags(18), H5F_LIBVER_LATEST_F)
!
! H5generic flags declaration
!
      INTEGER, PARAMETER :: H5generic_FLAGS_LEN = 9
      INTEGER H5generic_flags(H5generic_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5generic_FLAGS/
!DEC$endif
      COMMON /H5generic_FLAGS/ H5generic_flags

      INTEGER :: H5_INDEX_UNKNOWN_F
      INTEGER :: H5_INDEX_NAME_F
      INTEGER :: H5_INDEX_CRT_ORDER_F
      INTEGER :: H5_INDEX_N_F
      INTEGER :: H5_ITER_UNKNOWN_F
      INTEGER :: H5_ITER_INC_F
      INTEGER :: H5_ITER_DEC_F
      INTEGER :: H5_ITER_NATIVE_F 
      INTEGER :: H5_ITER_N_F

      EQUIVALENCE(H5generic_flags(1), H5_INDEX_UNKNOWN_F)
      EQUIVALENCE(H5generic_flags(2), H5_INDEX_NAME_F)
      EQUIVALENCE(H5generic_flags(3), H5_INDEX_CRT_ORDER_F)
      EQUIVALENCE(H5generic_flags(4), H5_INDEX_N_F)
      EQUIVALENCE(H5generic_flags(5), H5_ITER_UNKNOWN_F)
      EQUIVALENCE(H5generic_flags(6), H5_ITER_INC_F)     
      EQUIVALENCE(H5generic_flags(7), H5_ITER_DEC_F)
      EQUIVALENCE(H5generic_flags(8), H5_ITER_NATIVE_F)
      EQUIVALENCE(H5generic_flags(9), H5_ITER_N_F)


!
! H5G flags declaration
!
      INTEGER, PARAMETER :: H5G_FLAGS_LEN = 12
      INTEGER H5G_flags(H5G_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5G_FLAGS/
!DEC$endif
      COMMON /H5G_FLAGS/ H5G_flags

      INTEGER :: H5G_UNKNOWN_F
      INTEGER :: H5G_GROUP_F
      INTEGER :: H5G_DATASET_F
      INTEGER :: H5G_TYPE_F
! XXX: Fix problems with H5G_LINK_F! - QAK
      INTEGER :: H5G_LINK_F
      INTEGER :: H5G_LINK_ERROR_F
      INTEGER :: H5G_LINK_HARD_F
      INTEGER :: H5G_LINK_SOFT_F
      INTEGER :: H5G_STORAGE_TYPE_UNKNOWN_F
      INTEGER :: H5G_STORAGE_TYPE_SYMBOL_TABLE_F
      INTEGER :: H5G_STORAGE_TYPE_COMPACT_F
      INTEGER :: H5G_STORAGE_TYPE_DENSE_F

      EQUIVALENCE(H5G_flags(1), H5G_UNKNOWN_F)
      EQUIVALENCE(H5G_flags(2), H5G_GROUP_F)
      EQUIVALENCE(H5G_flags(3), H5G_DATASET_F)
      EQUIVALENCE(H5G_flags(4), H5G_TYPE_F) 
! XXX: Fix problems with H5G_LINK_F! - QAK ! these are really H5L values -MSB-
      EQUIVALENCE(H5G_flags(5), H5G_LINK_F)
      EQUIVALENCE(H5G_flags(6), H5G_LINK_ERROR_F)
      EQUIVALENCE(H5G_flags(7), H5G_LINK_HARD_F)
      EQUIVALENCE(H5G_flags(8), H5G_LINK_SOFT_F)
! XXX

      EQUIVALENCE(H5G_flags(9), H5G_STORAGE_TYPE_UNKNOWN_F ) 
      EQUIVALENCE(H5G_flags(10), H5G_STORAGE_TYPE_SYMBOL_TABLE_F) 
      EQUIVALENCE(H5G_flags(11), H5G_STORAGE_TYPE_COMPACT_F)  
      EQUIVALENCE(H5G_flags(12), H5G_STORAGE_TYPE_DENSE_F) 
!
! H5D flags declaration
!

      INTEGER, PARAMETER :: H5D_FLAGS_LEN = 19 
      INTEGER H5D_flags(H5D_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5D_FLAGS/
!DEC$endif
      COMMON /H5D_FLAGS/ H5D_flags

      INTEGER :: H5D_COMPACT_F  
      INTEGER :: H5D_CONTIGUOUS_F
      INTEGER :: H5D_CHUNKED_F

      INTEGER :: H5D_ALLOC_TIME_ERROR_F
      INTEGER :: H5D_ALLOC_TIME_DEFAULT_F
      INTEGER :: H5D_ALLOC_TIME_EARLY_F
      INTEGER :: H5D_ALLOC_TIME_LATE_F
      INTEGER :: H5D_ALLOC_TIME_INCR_F

      INTEGER :: H5D_SPACE_STS_ERROR_F
      INTEGER :: H5D_SPACE_STS_NOT_ALLOCATED_F
      INTEGER :: H5D_SPACE_STS_PART_ALLOCATED_F
      INTEGER :: H5D_SPACE_STS_ALLOCATED_F

      INTEGER :: H5D_FILL_TIME_ERROR_F
      INTEGER :: H5D_FILL_TIME_ALLOC_F
      INTEGER :: H5D_FILL_TIME_NEVER_F

      INTEGER :: H5D_FILL_VALUE_ERROR_F
      INTEGER :: H5D_FILL_VALUE_UNDEFINED_F
      INTEGER :: H5D_FILL_VALUE_DEFAULT_F
      INTEGER :: H5D_FILL_VALUE_USER_DEFINED_F

      EQUIVALENCE(H5D_flags(1), H5D_COMPACT_F)
      EQUIVALENCE(H5D_flags(2), H5D_CONTIGUOUS_F)
      EQUIVALENCE(H5D_flags(3), H5D_CHUNKED_F)

      EQUIVALENCE(H5D_flags(4), H5D_ALLOC_TIME_ERROR_F)
      EQUIVALENCE(H5D_flags(5), H5D_ALLOC_TIME_DEFAULT_F)
      EQUIVALENCE(H5D_flags(6), H5D_ALLOC_TIME_EARLY_F)
      EQUIVALENCE(H5D_flags(7), H5D_ALLOC_TIME_LATE_F)
      EQUIVALENCE(H5D_flags(8), H5D_ALLOC_TIME_INCR_F)

      EQUIVALENCE(H5D_flags(9), H5D_SPACE_STS_ERROR_F)
      EQUIVALENCE(H5D_flags(10), H5D_SPACE_STS_NOT_ALLOCATED_F)
      EQUIVALENCE(H5D_flags(11), H5D_SPACE_STS_PART_ALLOCATED_F)
      EQUIVALENCE(H5D_flags(12), H5D_SPACE_STS_ALLOCATED_F)

      EQUIVALENCE(H5D_flags(13), H5D_FILL_TIME_ERROR_F)
      EQUIVALENCE(H5D_flags(14), H5D_FILL_TIME_ALLOC_F)
      EQUIVALENCE(H5D_flags(15), H5D_FILL_TIME_NEVER_F)

      EQUIVALENCE(H5D_flags(16), H5D_FILL_VALUE_ERROR_F)
      EQUIVALENCE(H5D_flags(17), H5D_FILL_VALUE_UNDEFINED_F)
      EQUIVALENCE(H5D_flags(18), H5D_FILL_VALUE_DEFAULT_F)
      EQUIVALENCE(H5D_flags(19), H5D_FILL_VALUE_USER_DEFINED_F)

!
! H5FD flags declaration
!
      INTEGER, PARAMETER :: H5FD_FLAGS_LEN = 11
      INTEGER H5FD_flags(H5FD_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5FD_FLAGS/
!DEC$endif
      COMMON /H5FD_FLAGS/ H5FD_flags
      
      INTEGER :: H5FD_MPIO_INDEPENDENT_F 
      INTEGER :: H5FD_MPIO_COLLECTIVE_F
      INTEGER :: H5FD_MEM_NOLIST_F
      INTEGER :: H5FD_MEM_DEFAULT_F
      INTEGER :: H5FD_MEM_SUPER_F
      INTEGER :: H5FD_MEM_BTREE_F
      INTEGER :: H5FD_MEM_DRAW_F
      INTEGER :: H5FD_MEM_GHEAP_F
      INTEGER :: H5FD_MEM_LHEAP_F
      INTEGER :: H5FD_MEM_OHDR_F
      INTEGER :: H5FD_MEM_NTYPES_F
 
      EQUIVALENCE(H5FD_flags(1), H5FD_MPIO_INDEPENDENT_F)
      EQUIVALENCE(H5FD_flags(2), H5FD_MPIO_COLLECTIVE_F)
      EQUIVALENCE(H5FD_flags(3), H5FD_MEM_NOLIST_F)
      EQUIVALENCE(H5FD_flags(4), H5FD_MEM_DEFAULT_F)
      EQUIVALENCE(H5FD_flags(5), H5FD_MEM_SUPER_F)
      EQUIVALENCE(H5FD_flags(6), H5FD_MEM_BTREE_F)
      EQUIVALENCE(H5FD_flags(7), H5FD_MEM_DRAW_F)
      EQUIVALENCE(H5FD_flags(8), H5FD_MEM_GHEAP_F)
      EQUIVALENCE(H5FD_flags(9), H5FD_MEM_LHEAP_F)
      EQUIVALENCE(H5FD_flags(10), H5FD_MEM_OHDR_F)
      EQUIVALENCE(H5FD_flags(11), H5FD_MEM_NTYPES_F)
!
! H5FD file drivers flags declaration
!
      INTEGER, PARAMETER :: H5FD_HID_FLAGS_LEN = 7
      INTEGER(HID_T) H5FD_hid_flags(H5FD_HID_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5FD_HID_FLAGS/
!DEC$endif
      COMMON /H5FD_HID_FLAGS/ H5FD_hid_flags
      
      INTEGER(HID_T) :: H5FD_CORE_F
      INTEGER(HID_T) :: H5FD_FAMILY_F
      INTEGER(HID_T) :: H5FD_LOG_F
      INTEGER(HID_T) :: H5FD_MPIO_F
      INTEGER(HID_T) :: H5FD_MULTI_F
      INTEGER(HID_T) :: H5FD_SEC2_F
      INTEGER(HID_T) :: H5FD_STDIO_F

      EQUIVALENCE(H5FD_hid_flags(1), H5FD_CORE_F)
      EQUIVALENCE(H5FD_hid_flags(2), H5FD_FAMILY_F)
      EQUIVALENCE(H5FD_hid_flags(3), H5FD_LOG_F)
      EQUIVALENCE(H5FD_hid_flags(4), H5FD_MPIO_F)
      EQUIVALENCE(H5FD_hid_flags(5), H5FD_MULTI_F)
      EQUIVALENCE(H5FD_hid_flags(6), H5FD_SEC2_F)
      EQUIVALENCE(H5FD_hid_flags(7), H5FD_STDIO_F)

!
! H5I flags declaration
!
      INTEGER, PARAMETER :: H5I_FLAGS_LEN = 7
      INTEGER H5I_flags(H5I_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5I_FLAGS/
!DEC$endif
      COMMON /H5I_FLAGS/ H5I_flags

      INTEGER ::  H5I_FILE_F
      INTEGER ::  H5I_GROUP_F
      INTEGER ::  H5I_DATATYPE_F
      INTEGER ::  H5I_DATASPACE_F
      INTEGER ::  H5I_DATASET_F
      INTEGER ::  H5I_ATTR_F
      INTEGER ::  H5I_BADID_F

      EQUIVALENCE(H5I_flags(1), H5I_FILE_F)
      EQUIVALENCE(H5I_flags(2), H5I_GROUP_F)
      EQUIVALENCE(H5I_flags(3), H5I_DATATYPE_F)
      EQUIVALENCE(H5I_flags(4), H5I_DATASPACE_F)
      EQUIVALENCE(H5I_flags(5), H5I_DATASET_F)
      EQUIVALENCE(H5I_flags(6), H5I_ATTR_F)
      EQUIVALENCE(H5I_flags(7), H5I_BADID_F)
!
! H5L flags declaration
!
      INTEGER, PARAMETER :: H5L_FLAGS_LEN = 6
      INTEGER :: H5L_flags(H5L_FLAGS_LEN)

      INTEGER :: H5L_LINK_F
      INTEGER :: H5L_LINK_ERROR_F
      INTEGER :: H5L_LINK_HARD_F
      INTEGER :: H5L_LINK_SOFT_F
      INTEGER :: H5L_SAME_LOC_F
      INTEGER :: H5L_LINK_CLASS_T_VERS_F

!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5L_FLAGS/
!DEC$endif
      COMMON /H5L_FLAGS/ H5L_flags

      EQUIVALENCE(H5L_flags(1), H5L_LINK_F)
      EQUIVALENCE(H5L_flags(2), H5L_LINK_ERROR_F)
      EQUIVALENCE(H5L_flags(3), H5L_LINK_HARD_F)
      EQUIVALENCE(H5L_flags(4), H5L_LINK_SOFT_F)
      EQUIVALENCE(H5L_flags(5), H5L_SAME_LOC_F)
      EQUIVALENCE(H5L_flags(6), H5L_LINK_CLASS_T_VERS_F)
!
! H5O flags declaration
!
      INTEGER, PARAMETER :: H5O_FLAGS_LEN = 22
      INTEGER :: H5o_flags(H5O_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5O_FLAGS/
!DEC$endif
      COMMON /H5O_FLAGS/ H5O_flags

      INTEGER :: H5O_COPY_SHALLOW_HIERARCHY_F ! * THESE VARIABLES DO
      INTEGER :: H5O_COPY_EXPAND_SOFT_LINK_F ! NOT MATCH THE C VARIABLE
      INTEGER :: H5O_COPY_EXPAND_EXT_LINK_F !  IN ORDER
      INTEGER :: H5O_COPY_EXPAND_REFERENCE_F ! TO STAY UNDER THE
      INTEGER :: H5O_COPY_WITHOUT_ATTR_FLAG_F 
      INTEGER :: H5O_COPY_PRESERVE_NULL_FLAG_F
      INTEGER :: H5O_COPY_ALL_F
      INTEGER :: H5O_SHMESG_NONE_FLAG_F
      INTEGER :: H5O_SHMESG_SDSPACE_FLAG_F
      INTEGER :: H5O_SHMESG_DTYPE_FLAG_F
      INTEGER :: H5O_SHMESG_FILL_FLAG_F
      INTEGER :: H5O_SHMESG_PLINE_FLAG_F
      INTEGER :: H5O_SHMESG_ATTR_FLAG_F
      INTEGER :: H5O_SHMESG_ALL_FLAG_F
      INTEGER :: H5O_HDR_CHUNK0_SIZE_F
      INTEGER :: H5O_HDR_ATTR_CRT_ORDER_TRACK_F ! 32 CHARACTER
      INTEGER :: H5O_HDR_ATTR_CRT_ORDER_INDEX_F ! VARIABLE
      INTEGER :: H5O_HDR_ATTR_STORE_PHASE_CHA_F !  LENGTH *
      INTEGER :: H5O_HDR_STORE_TIMES_F
      INTEGER :: H5O_HDR_ALL_FLAGS_F
      INTEGER :: H5O_SHMESG_MAX_NINDEXES_F
      INTEGER :: H5O_SHMESG_MAX_LIST_SIZE_F

      EQUIVALENCE(h5o_flags(1) , H5O_COPY_SHALLOW_HIERARCHY_F)
      EQUIVALENCE(h5o_flags(2) , H5O_COPY_EXPAND_SOFT_LINK_F)
      EQUIVALENCE(h5o_flags(3) , H5O_COPY_EXPAND_EXT_LINK_F)
      EQUIVALENCE(h5o_flags(4) , H5O_COPY_EXPAND_REFERENCE_F)
      EQUIVALENCE(h5o_flags(5) , H5O_COPY_WITHOUT_ATTR_FLAG_F)
      EQUIVALENCE(h5o_flags(6) , H5O_COPY_PRESERVE_NULL_FLAG_F)
      EQUIVALENCE(h5o_flags(7) , H5O_COPY_ALL_F)
      EQUIVALENCE(h5o_flags(8) , H5O_SHMESG_NONE_FLAG_F)
      EQUIVALENCE(h5o_flags(9) , H5O_SHMESG_SDSPACE_FLAG_F)
      EQUIVALENCE(h5o_flags(10) , H5O_SHMESG_DTYPE_FLAG_F)
      EQUIVALENCE(h5o_flags(11) , H5O_SHMESG_FILL_FLAG_F)
      EQUIVALENCE(h5o_flags(12) , H5O_SHMESG_PLINE_FLAG_F)
      EQUIVALENCE(h5o_flags(13) , H5O_SHMESG_ATTR_FLAG_F)
      EQUIVALENCE(h5o_flags(14) , H5O_SHMESG_ALL_FLAG_F)
      EQUIVALENCE(h5o_flags(15) , H5O_HDR_CHUNK0_SIZE_F)
      EQUIVALENCE(h5o_flags(16) , H5O_HDR_ATTR_CRT_ORDER_TRACK_F)
      EQUIVALENCE(h5o_flags(17) , H5O_HDR_ATTR_CRT_ORDER_INDEX_F)
      EQUIVALENCE(h5o_flags(18) , H5O_HDR_ATTR_STORE_PHASE_CHA_F)
      EQUIVALENCE(h5o_flags(19) , H5O_HDR_STORE_TIMES_F)
      EQUIVALENCE(h5o_flags(20) , H5O_HDR_ALL_FLAGS_F) 
      EQUIVALENCE(h5o_flags(21) , H5O_SHMESG_MAX_NINDEXES_F)
      EQUIVALENCE(h5o_flags(22) , H5O_SHMESG_MAX_LIST_SIZE_F)

!
! H5P flags declaration
!
      INTEGER, PARAMETER :: H5P_FLAGS_LEN = 18 
      INTEGER(HID_T) H5P_flags(H5P_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5P_FLAGS/
!DEC$endif
      COMMON /H5P_FLAGS/ H5P_flags

      INTEGER(HID_T) :: H5P_FILE_CREATE_F 
      INTEGER(HID_T) :: H5P_FILE_ACCESS_F 
      INTEGER(HID_T) :: H5P_DATASET_CREATE_F
      INTEGER(HID_T) :: H5P_DATASET_XFER_F 
      INTEGER(HID_T) :: H5P_FILE_MOUNT_F 
      INTEGER(HID_T) :: H5P_DEFAULT_F 
      INTEGER(HID_T) :: H5P_ROOT_F  
      INTEGER(HID_T) :: H5P_OBJECT_CREATE_F
      INTEGER(HID_T) :: H5P_DATASET_ACCESS_F
      INTEGER(HID_T) :: H5P_GROUP_CREATE_F
      INTEGER(HID_T) :: H5P_GROUP_ACCESS_F
      INTEGER(HID_T) :: H5P_DATATYPE_CREATE_F
      INTEGER(HID_T) :: H5P_DATATYPE_ACCESS_F
      INTEGER(HID_T) :: H5P_STRING_CREATE_F
      INTEGER(HID_T) :: H5P_ATTRIBUTE_CREATE_F
      INTEGER(HID_T) :: H5P_OBJECT_COPY_F
      INTEGER(HID_T) :: H5P_LINK_CREATE_F
      INTEGER(HID_T) :: H5P_LINK_ACCESS_F

      EQUIVALENCE(H5P_flags(1), H5P_FILE_CREATE_F)
      EQUIVALENCE(H5P_flags(2), H5P_FILE_ACCESS_F)
      EQUIVALENCE(H5P_flags(3), H5P_DATASET_CREATE_F)
      EQUIVALENCE(H5P_flags(4), H5P_DATASET_XFER_F)
      EQUIVALENCE(H5P_flags(5), H5P_FILE_MOUNT_F)
      EQUIVALENCE(H5P_flags(6), H5P_DEFAULT_F)
      EQUIVALENCE(H5P_flags(7), H5P_ROOT_F)
      EQUIVALENCE(H5P_flags(8), H5P_OBJECT_CREATE_F)
      EQUIVALENCE(H5P_flags(9), H5P_DATASET_ACCESS_F)
      EQUIVALENCE(H5P_flags(10), H5P_GROUP_CREATE_F)
      EQUIVALENCE(H5P_flags(11), H5P_GROUP_ACCESS_F)
      EQUIVALENCE(H5P_flags(12), H5P_DATATYPE_CREATE_F)
      EQUIVALENCE(H5P_flags(13), H5P_DATATYPE_ACCESS_F)
      EQUIVALENCE(H5P_flags(14), H5P_STRING_CREATE_F)
      EQUIVALENCE(H5P_flags(15), H5P_ATTRIBUTE_CREATE_F)
      EQUIVALENCE(H5P_flags(16), H5P_OBJECT_COPY_F)
      EQUIVALENCE(H5P_flags(17), H5P_LINK_CREATE_F)
      EQUIVALENCE(H5P_flags(18), H5P_LINK_ACCESS_F)

!
! H5P intgere flags declaration
!
      INTEGER, PARAMETER :: H5P_FLAGS_INT_LEN = 2 
      INTEGER  H5P_flags_int(H5P_FLAGS_INT_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5P_FLAGS_INT/
!DEC$endif
      COMMON /H5P_FLAGS_INT/ H5P_flags_int
      INTEGER :: H5P_CRT_ORDER_INDEXED_F 
      INTEGER :: H5P_CRT_ORDER_TRACKED_F 
      EQUIVALENCE(H5P_flags_int(1), H5P_CRT_ORDER_INDEXED_F)
      EQUIVALENCE(H5P_flags_int(2), H5P_CRT_ORDER_TRACKED_F)
!
! H5R flags declaration
!
      INTEGER, PARAMETER :: H5R_FLAGS_LEN = 2
      INTEGER H5R_flags(H5R_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5R_FLAGS/
!DEC$endif
      COMMON /H5R_FLAGS/ H5R_flags
      
      INTEGER :: H5R_OBJECT_F
      INTEGER :: H5R_DATASET_REGION_F

      EQUIVALENCE(H5R_flags(1), H5R_OBJECT_F)
      EQUIVALENCE(H5R_flags(2), H5R_DATASET_REGION_F)

!
! H5S flags declaration
!
      INTEGER, PARAMETER :: H5S_FLAGS_LEN = 20 
      INTEGER H5S_flags(H5S_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5S_FLAGS/
!DEC$endif
      COMMON /H5S_FLAGS/ H5S_flags

      INTEGER :: H5S_SCALAR_F 
      INTEGER :: H5S_SIMPLE_F 
      INTEGER :: H5S_NULL_F 

      INTEGER :: H5S_UNLIMITED_F
      INTEGER :: H5S_ALL_F

      INTEGER :: H5S_SELECT_NOOP_F
      INTEGER :: H5S_SELECT_SET_F
      INTEGER :: H5S_SELECT_OR_F
      INTEGER :: H5S_SELECT_AND_F 
      INTEGER :: H5S_SELECT_XOR_F 
      INTEGER :: H5S_SELECT_NOTB_F 
      INTEGER :: H5S_SELECT_NOTA_F 
      INTEGER :: H5S_SELECT_APPEND_F 
      INTEGER :: H5S_SELECT_PREPEND_F 
      INTEGER :: H5S_SELECT_INVALID_F 


      INTEGER :: H5S_SEL_ERROR_F
      INTEGER :: H5S_SEL_NONE_F
      INTEGER :: H5S_SEL_POINTS_F
      INTEGER :: H5S_SEL_HYPERSLABS_F
      INTEGER :: H5S_SEL_ALL_F

      EQUIVALENCE(H5S_flags(1), H5S_SCALAR_F)
      EQUIVALENCE(H5S_flags(2), H5S_SIMPLE_F)
      EQUIVALENCE(H5S_flags(3), H5S_NULL_F)
      EQUIVALENCE(H5S_flags(4), H5S_SELECT_SET_F)
      EQUIVALENCE(H5S_flags(5), H5S_SELECT_OR_F)
      EQUIVALENCE(H5S_flags(6), H5S_UNLIMITED_F)
      EQUIVALENCE(H5S_flags(7), H5S_ALL_F)

      EQUIVALENCE(H5S_flags(8), H5S_SELECT_NOOP_F)
      EQUIVALENCE(H5S_flags(9), H5S_SELECT_AND_F) 
      EQUIVALENCE(H5S_flags(10), H5S_SELECT_XOR_F)
      EQUIVALENCE(H5S_flags(11), H5S_SELECT_NOTB_F)
      EQUIVALENCE(H5S_flags(12), H5S_SELECT_NOTA_F)
      EQUIVALENCE(H5S_flags(13), H5S_SELECT_APPEND_F) 
      EQUIVALENCE(H5S_flags(14), H5S_SELECT_PREPEND_F) 
      EQUIVALENCE(H5S_flags(15), H5S_SELECT_INVALID_F)


      EQUIVALENCE(H5S_flags(16), H5S_SEL_ERROR_F)
      EQUIVALENCE(H5S_flags(17), H5S_SEL_NONE_F)
      EQUIVALENCE(H5S_flags(18), H5S_SEL_POINTS_F)
      EQUIVALENCE(H5S_flags(19), H5S_SEL_HYPERSLABS_F)
      EQUIVALENCE(H5S_flags(20), H5S_SEL_ALL_F)


!
! H5T flags declaration
!
      INTEGER, PARAMETER :: H5T_FLAGS_LEN = 31
      INTEGER H5T_flags(H5T_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5T_FLAGS/
!DEC$endif
      COMMON /H5T_FLAGS/ H5T_flags

      INTEGER ::  H5T_NO_CLASS_F 
      INTEGER ::  H5T_INTEGER_F 
      INTEGER ::  H5T_FLOAT_F  
      INTEGER ::  H5T_TIME_F 
      INTEGER ::  H5T_STRING_F 
      INTEGER ::  H5T_BITFIELD_F
      INTEGER ::  H5T_OPAQUE_F 
      INTEGER ::  H5T_COMPOUND_F 
      INTEGER ::  H5T_REFERENCE_F
      INTEGER ::  H5T_ENUM_F 
      INTEGER ::  H5T_VLEN_F
      INTEGER ::  H5T_ARRAY_F
      INTEGER ::  H5T_ORDER_LE_F 
      INTEGER ::  H5T_ORDER_BE_F
      INTEGER ::  H5T_ORDER_VAX_F
      INTEGER ::  H5T_PAD_ZERO_F
      INTEGER ::  H5T_PAD_ONE_F
      INTEGER ::  H5T_PAD_BACKGROUND_F
      INTEGER ::  H5T_PAD_ERROR_F    
      INTEGER ::  H5T_SGN_NONE_F   
      INTEGER ::  H5T_SGN_2_F     
      INTEGER ::  H5T_SGN_ERROR_F
      INTEGER ::  H5T_NORM_IMPLIED_F
      INTEGER ::  H5T_NORM_MSBSET_F
      INTEGER ::  H5T_NORM_NONE_F 
      INTEGER ::  H5T_CSET_ASCII_F
      INTEGER ::  H5T_CSET_UTF8_F
      INTEGER ::  H5T_STR_NULLTERM_F 
      INTEGER ::  H5T_STR_NULLPAD_F 
      INTEGER ::  H5T_STR_SPACEPAD_F
      INTEGER ::  H5T_STR_ERROR_F
       
      EQUIVALENCE(H5T_flags(1), H5T_NO_CLASS_F)
      EQUIVALENCE(H5T_flags(2), H5T_INTEGER_F)
      EQUIVALENCE(H5T_flags(3), H5T_FLOAT_F)
      EQUIVALENCE(H5T_flags(4), H5T_TIME_F)
      EQUIVALENCE(H5T_flags(5), H5T_STRING_F)
      EQUIVALENCE(H5T_flags(6), H5T_BITFIELD_F)
      EQUIVALENCE(H5T_flags(7), H5T_OPAQUE_F)
      EQUIVALENCE(H5T_flags(8), H5T_COMPOUND_F)
      EQUIVALENCE(H5T_flags(9), H5T_REFERENCE_F)
      EQUIVALENCE(H5T_flags(10), H5T_ENUM_F)
      EQUIVALENCE(H5T_flags(11), H5T_ORDER_LE_F)
      EQUIVALENCE(H5T_flags(12), H5T_ORDER_BE_F)
      EQUIVALENCE(H5T_flags(13), H5T_ORDER_VAX_F)
      EQUIVALENCE(H5T_flags(14), H5T_PAD_ZERO_F)
      EQUIVALENCE(H5T_flags(15), H5T_PAD_ONE_F)
      EQUIVALENCE(H5T_flags(16), H5T_PAD_BACKGROUND_F)
      EQUIVALENCE(H5T_flags(17), H5T_PAD_ERROR_F)
      EQUIVALENCE(H5T_flags(18), H5T_SGN_NONE_F)
      EQUIVALENCE(H5T_flags(19), H5T_SGN_2_F)
      EQUIVALENCE(H5T_flags(20), H5T_SGN_ERROR_F)
      EQUIVALENCE(H5T_flags(21), H5T_NORM_IMPLIED_F)
      EQUIVALENCE(H5T_flags(22), H5T_NORM_MSBSET_F)
      EQUIVALENCE(H5T_flags(23), H5T_NORM_NONE_F)
      EQUIVALENCE(H5T_flags(24), H5T_CSET_ASCII_F)
      EQUIVALENCE(H5T_flags(25), H5T_CSET_UTF8_F)
      EQUIVALENCE(H5T_flags(26), H5T_STR_NULLTERM_F)
      EQUIVALENCE(H5T_flags(27), H5T_STR_NULLPAD_F)
      EQUIVALENCE(H5T_flags(28), H5T_STR_SPACEPAD_F)
      EQUIVALENCE(H5T_flags(29), H5T_STR_ERROR_F)
      EQUIVALENCE(H5T_flags(30), H5T_VLEN_F)
      EQUIVALENCE(H5T_flags(31), H5T_ARRAY_F)

!
! H5Z flags declaration
!
      INTEGER, PARAMETER :: H5Z_FLAGS_LEN = 14
      INTEGER H5Z_flags(H5Z_FLAGS_LEN)
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5Z_FLAGS/
!DEC$endif
      COMMON /H5Z_FLAGS/ H5Z_flags

      INTEGER :: H5Z_FILTER_ERROR_F 
      INTEGER :: H5Z_FILTER_NONE_F 
      INTEGER :: H5Z_FILTER_ALL_F
      INTEGER :: H5Z_FILTER_DEFLATE_F 
      INTEGER :: H5Z_FILTER_SHUFFLE_F 
      INTEGER :: H5Z_FILTER_FLETCHER32_F 
      INTEGER :: H5Z_FILTER_SZIP_F 
      INTEGER :: H5Z_ERROR_EDC_F
      INTEGER :: H5Z_DISABLE_EDC_F
      INTEGER :: H5Z_ENABLE_EDC_F
      INTEGER :: H5Z_NO_EDC_F
      INTEGER :: H5Z_FLAG_OPTIONAL_F
      INTEGER :: H5Z_FILTER_ENCODE_ENABLED_F
      INTEGER :: H5Z_FILTER_DECODE_ENABLED_F

      EQUIVALENCE(H5Z_flags(1), H5Z_FILTER_ERROR_F)
      EQUIVALENCE(H5Z_flags(2), H5Z_FILTER_NONE_F)
      EQUIVALENCE(H5Z_flags(3), H5Z_FILTER_DEFLATE_F)
      EQUIVALENCE(H5Z_flags(4), H5Z_FILTER_SHUFFLE_F)
      EQUIVALENCE(H5Z_flags(5), H5Z_FILTER_FLETCHER32_F)
      EQUIVALENCE(H5Z_flags(6), H5Z_ERROR_EDC_F)
      EQUIVALENCE(H5Z_flags(7), H5Z_DISABLE_EDC_F)
      EQUIVALENCE(H5Z_flags(8), H5Z_ENABLE_EDC_F)
      EQUIVALENCE(H5Z_flags(9), H5Z_NO_EDC_F)
      EQUIVALENCE(H5Z_flags(10), H5Z_FILTER_SZIP_F)
      EQUIVALENCE(H5Z_flags(11), H5Z_FLAG_OPTIONAL_F)
      EQUIVALENCE(H5Z_flags(12), H5Z_FILTER_ENCODE_ENABLED_F)
      EQUIVALENCE(H5Z_flags(13), H5Z_FILTER_DECODE_ENABLED_F)
      EQUIVALENCE(H5Z_flags(14), H5Z_FILTER_ALL_F)


!
! H5 Library flags declaration
!
     INTEGER, PARAMETER :: H5LIB_FLAGS_LEN =  2
     INTEGER :: H5LIB_flags(H5LIB_FLAGS_LEN) 
!DEC$if defined(BUILD_HDF5_DLL)
!DEC$ ATTRIBUTES DLLEXPORT :: /H5LIB_FLAGS/
!DEC$endif
     COMMON /H5LIB_FLAGS/ H5LIB_flags
     INTEGER :: H5_SZIP_EC_OM_F
     INTEGER :: H5_SZIP_NN_OM_F
!
     EQUIVALENCE(H5LIB_flags(1), H5_SZIP_EC_OM_F)
     EQUIVALENCE(H5LIB_flags(2), H5_SZIP_NN_OM_F)


! General H5 flags declarations
!
!!$      INTEGER, PARAMETER :: H5_FLAGS_LEN = 2
!!$      INTEGER H5_flags(H5_FLAGS_LEN)
!!$!DEC$if defined(BUILD_HDF5_DLL)
!!$!DEC$ ATTRIBUTES DLLEXPORT :: /H5_FLAGS/
!!$!DEC$endif
!!$      COMMON /H5_FLAGS/ H5_flags
!!$
!!$      INTEGER :: _F
!!$      INTEGER :: H5F_SCOPE_LOCAL_F
!!$
!!$      EQUIVALENCE(H5F_flags(1), H5F_SCOPE_GLOBAL_F)
!!$      EQUIVALENCE(H5F_flags(2), H5F_SCOPE_LOCAL_F)

   END MODULE H5GLOBAL
      
