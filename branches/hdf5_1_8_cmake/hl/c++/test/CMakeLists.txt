cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_CPP_TEST)

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

INCLUDE_DIRECTORIES (${HDF5_HL_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES (${HDF5_HL_CPP_SOURCE_DIR}/src)

# --------------------------------------------------------------------
# Add in the unit tests for the packet table c++ wrapper
# --------------------------------------------------------------------

IF (BUILD_TESTING)
  INCLUDE_DIRECTORIES (${HDF5_TEST_SOURCE_DIR})
  INCLUDE_DIRECTORIES (${HDF5_HL_SOURCE_DIR}/test)
  INCLUDE_DIRECTORIES (${HDF5_CPP_SOURCE_DIR}/src)

  ADD_EXECUTABLE (hl_ptableTest ${HDF5_HL_CPP_TEST_SOURCE_DIR}/ptableTest.cpp)
  TARGET_LINK_LIBRARIES (
      hl_ptableTest
      ${HDF5_HL_CPP_LIB_NAME}
      ${HDF5_HL_LIB_NAME}
      ${HDF5_CPP_LIB_NAME}
      ${HDF5_LIB_NAME}
  )
  ADD_TEST (hl_ptableTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hl_ptableTest)
ENDIF (BUILD_TESTING)
