cmake_minimum_required (VERSION 2.8)
PROJECT(HDF5_HL_F90 C CXX Fortran)

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

INCLUDE_DIRECTORIES (${HDF5_F90_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES (${HDF5_F90_BINARY_DIR})
INCLUDE_DIRECTORIES (${HDF5_F90_SRC_BINARY_DIR})

LINK_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_FORTRAN_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

#-----------------------------------------------------------------------------
# List Source files
#-----------------------------------------------------------------------------
ADD_SUBDIRECTORY (${HDF5_HL_F90_SOURCE_DIR}/src ${HDF5_HL_F90_BINARY_DIR}/src)

#-----------------------------------------------------------------------------
# Build the HL Fortran Examples
#-----------------------------------------------------------------------------
IF (HDF5_BUILD_FORTRAN_EXAMPLES)
  ADD_SUBDIRECTORY (${HDF5_HL_F90_SOURCE_DIR}/examples ${HDF5_HL_F90_BINARY_DIR}/examples)
ENDIF (HDF5_BUILD_FORTRAN_EXAMPLES)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
IF (BUILD_TESTING)
#  ADD_SUBDIRECTORY (${HDF5_HL_F90_SOURCE_DIR}/test ${HDF5_HL_F90_BINARY_DIR}/test)
ENDIF (BUILD_TESTING)
