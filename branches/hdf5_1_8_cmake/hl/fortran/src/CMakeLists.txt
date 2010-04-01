cmake_minimum_required (VERSION 2.8)
PROJECT(HDF5_HL_F90_SRC C CXX Fortran)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_F90_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES (${HDF5_F90_BINARY_DIR})
INCLUDE_DIRECTORIES (${HDF5_F90_SRC_BINARY_DIR})

LINK_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_FORTRAN_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

#-----------------------------------------------------------------------------
# List Source files
#-----------------------------------------------------------------------------
#MESSAGE ("Fortran source is in ${HDF5_F90_SOURCE_DIR}/src ")


SET (HDF5_HL_F90_F_SRCS
    ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5TBff.f90
    ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5LTff.f90
    ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5IMff.f90
)

SET (HDF5_HL_F90_C_SRCS
    ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5LTfc.c
    ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5IMfc.c
    ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5IMcc.c
    ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5TBfc.c
)

SET (HDF5_HL_F90_HEADERS ${HDF5_HL_F90_SRC_SOURCE_DIR}/H5LTf90proto.h)

INCLUDE_DIRECTORIES (${HDF5_F90_SRC_BINARY_DIR})
INCLUDE_DIRECTORIES (${HDF5_F90_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES (${HDF5_HL_F90_SRC_SOURCE_DIR})
INCLUDE_DIRECTORIES (${HDF5_HL_SOURCE_DIR}/src)

ADD_LIBRARY (${HDF5_HL_F90_C_LIB_TARGET}
    ${LIB_TYPE}
    ${HDF5_HL_F90_C_SRCS} ${HDF5_HL_F90_HEADERS}
)
SET_GLOBAL_VARIABLE (HDF5_LIBRARIES_TO_EXPORT
    "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_HL_F90_C_LIB_TARGET}"
)
TARGET_LINK_LIBRARIES (${HDF5_HL_F90_C_LIB_TARGET}
    ${HDF5_HL_LIB_TARGET}
    ${HDF5_F90_C_LIB_TARGET}
)
H5_SET_LIB_OPTIONS (${HDF5_HL_F90_C_LIB_TARGET} ${HDF5_HL_F90_C_LIB_NAME} ${LIB_TYPE})

ADD_LIBRARY (${HDF5_HL_F90_LIB_TARGET} STATIC ${HDF5_HL_F90_F_SRCS})
SET_GLOBAL_VARIABLE (HDF5_LIBRARIES_TO_EXPORT
    "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_HL_F90_LIB_TARGET}"
)
ADD_DEPENDENCIES (${HDF5_HL_F90_LIB_TARGET} ${HDF5_HL_F90_C_LIB_TARGET})

#-----------------------------------------------------------------------------
# Add Target(s) to CMake Install for import into other projects
#-----------------------------------------------------------------------------
IF (HDF5_EXPORTED_TARGETS)
  INSTALL (
      TARGETS
          ${HDF5_HL_F90_C_LIB_TARGET}
          ${HDF5_HL_F90_LIB_TARGET}
      EXPORT
          ${HDF5_EXPORTED_TARGETS}
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      RUNTIME DESTINATION bin
  )
ENDIF (HDF5_EXPORTED_TARGETS)
