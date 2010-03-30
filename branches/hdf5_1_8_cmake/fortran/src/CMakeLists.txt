cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_F90_SRC C CXX Fortran)

#-----------------------------------------------------------------------------
# Setup output Directories
#-----------------------------------------------------------------------------
IF (NOT HDF5_EXTERNALLY_CONFIGURED)
  SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
  )
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
  )
  SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
  )
  SET (CMAKE_Fortran_MODULE_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin/fortran CACHE PATH "Single Directory for all fortran modules."
  )
ENDIF (NOT HDF5_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Fortran support is best served using cmake 2.8.0
#-----------------------------------------------------------------------------
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

#-----------------------------------------------------------------------------
# List Source files
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_F90_BINARY_DIR})

#-----------------------------------------------------------------------------
# Setup the Fortran auto-detection utilities
# H5test_kind(_SIZEOF).f90 used to generate H5fortran_detect.f90
# H5fortran_detect.f90     used to generate H5fort_type_defines.h
# H5fort_type_defines.h    used to generate H5f90i_gen.h + H5fortran_types.f90
#-----------------------------------------------------------------------------
IF (FORTRAN_HAVE_SIZEOF)
  ADD_EXECUTABLE (H5test_FortranHavekind
      ${HDF5_F90_SRC_SOURCE_DIR}/H5test_kind_SIZEOF.f90
  )
ELSE (FORTRAN_HAVE_SIZEOF)
  ADD_EXECUTABLE (H5test_FortranHavekind
      ${HDF5_F90_SRC_SOURCE_DIR}/H5test_kind.f90
  )
ENDIF (FORTRAN_HAVE_SIZEOF)

SET (CMD ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}${CFG_INIT}/H5test_FortranHavekind${EXE_EXT})
ADD_CUSTOM_COMMAND (
    OUTPUT ${HDF5_F90_BINARY_DIR}/H5fortran_detect.f90
    COMMAND ${CMD}
    ARGS > ${HDF5_F90_BINARY_DIR}/H5fortran_detect.f90
    WORKING_DIRECTORY ${HDF5_F90_BINARY_DIR}
    DEPENDS H5test_FortranHavekind
)
#-----------------------------------------------------------------------------
ADD_EXECUTABLE (H5fortran_detect
    ${HDF5_F90_BINARY_DIR}/H5fortran_detect.f90
)

SET (CMD ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}${CFG_INIT}/H5fortran_detect${EXE_EXT})
ADD_CUSTOM_COMMAND (
    OUTPUT ${HDF5_F90_BINARY_DIR}/H5fort_type_defines.h
    COMMAND ${CMD}
    ARGS > ${HDF5_F90_BINARY_DIR}/H5fort_type_defines.h
    WORKING_DIRECTORY ${HDF5_F90_BINARY_DIR}
    DEPENDS H5fortran_detect
)

#-----------------------------------------------------------------------------
ADD_EXECUTABLE (H5match_types
    ${HDF5_F90_BINARY_DIR}/H5fort_type_defines.h
    ${HDF5_F90_SRC_SOURCE_DIR}/H5match_types.c
)

SET (CMD ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}${CFG_INIT}/H5match_types${EXE_EXT})
ADD_CUSTOM_COMMAND (
    OUTPUT ${HDF5_F90_BINARY_DIR}/H5f90i_gen.h
           ${HDF5_F90_BINARY_DIR}/H5fortran_types.f90
    COMMAND ${CMD}
    WORKING_DIRECTORY ${HDF5_F90_BINARY_DIR}
    DEPENDS H5match_types
)

#-----------------------------------------------------------------------------
# f90CStub lib
#-----------------------------------------------------------------------------
SET (f90CStub_C_SRCS
    # generated files
    ${HDF5_F90_BINARY_DIR}/H5f90i_gen.h

    # normal distribution
    ${HDF5_F90_SRC_SOURCE_DIR}/H5f90kit.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5_f.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Af.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Df.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Ef.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Ff.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Gf.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5If.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Lf.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Of.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Pf.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Rf.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Sf.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Tf.c
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Zf.c
)

IF (H5_HAVE_PARALLEL)
  SET (f90CStub_C_SRCS
      ${f90CStub_C_SRCS}
      ${HDF5_F90_SRC_SOURCE_DIR}/H5FDmpiof.c
  )
ENDIF (H5_HAVE_PARALLEL)

SET_SOURCE_FILES_PROPERTIES (${f90CStub_C_SRCS} PROPERTIES LANGUAGE C)

ADD_LIBRARY (${HDF5_F90_C_LIB_NAME} ${f90CStub_C_SRCS})
TARGET_LINK_LIBRARIES (${HDF5_F90_C_LIB_NAME} hdf5)

#-----------------------------------------------------------------------------
# Fortran Real Size
#-----------------------------------------------------------------------------
IF (FORTRAN_DEFAULT_REAL_NOT_DOUBLE)
  # default real is 4 bytes, so include double signatures
  SET (F_DBLE "Include")
ELSE (FORTRAN_DEFAULT_REAL_NOT_DOUBLE)
  # default real is 8 bytes, so exclude double signatures
  SET (F_DBLE "Exclude")
ENDIF (FORTRAN_DEFAULT_REAL_NOT_DOUBLE)

#-----------------------------------------------------------------------------
# Fortran Modules
#-----------------------------------------------------------------------------
IF (H5_HAVE_PARALLEL)
  SET (H5FC_NAME h5pfc)
ELSE (H5_HAVE_PARALLEL)
  SET (H5FC_NAME h5fc)
ENDIF (H5_HAVE_PARALLEL)

#-----------------------------------------------------------------------------
SET (f90_F_SRCS
    ${HDF5_F90_BINARY_DIR}/H5fortran_types.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5f90global.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5_ff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5_DBLE_Interface${F_DBLE}.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Aff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Dff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Eff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Fff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Gff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Iff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Lff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Off.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Pff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Rff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Sff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Tff.f90
    ${HDF5_F90_SRC_SOURCE_DIR}/H5Zff.f90
)
#-----------------------------------------------------------------------------
# Force dependencies between libs to ensure build order is ok
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
IF (H5_HAVE_PARALLEL)
  ADD_LIBRARY (H5FDMPIO STATIC ${HDF5_F90_SRC_SOURCE_DIR}/H5FDmpioff.f90)
  ADD_DEPENDENCIES (H5FDMPIO H5GLOBAL)
  SET (HDF5MODULE_SRC
      ${HDF5_F90_SRC_SOURCE_DIR}/HDF5mpio.f90
  )
  SET (HDF5_DEPENDS H5FDMPIO)
ELSE (H5_HAVE_PARALLEL)
  SET (HDF5MODULE_SRC
      ${HDF5_F90_SRC_SOURCE_DIR}/HDF5.f90
  )
  SET (HDF5_DEPENDS "")
ENDIF (H5_HAVE_PARALLEL)
#-----------------------------------------------------------------------------
ADD_LIBRARY (${HDF5_F90_LIB_NAME} STATIC ${HDF5MODULE_SRC} ${f90_F_SRCS})
SET_GLOBAL_VARIABLE (HDF5_LIBRARIES_TO_EXPORT
    "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_F90_LIB_NAME};${HDF5_F90_C_LIB_NAME}"
)
ADD_DEPENDENCIES (${HDF5_F90_LIB_NAME} ${HDF5_F90_C_LIB_NAME})
