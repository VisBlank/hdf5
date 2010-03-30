cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_CPP_EXAMPLES)

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
#  Add in the examples for the Packet Table codes
# --------------------------------------------------------------------
IF (HDF5_BUILD_EXAMPLES)
  ADD_EXECUTABLE (ptExampleFL ${HDF5_HL_CPP_EXAMPLES_SOURCE_DIR}/ptExampleFL.cpp)
  TARGET_LINK_LIBRARIES (
      ptExampleFL
      ${HDF5_HL_CPP_LIB_NAME}
      ${HDF5_HL_LIB_NAME}
      ${HDF5_CPP_LIB_NAME}
      ${HDF5_LIB_NAME}
  )

  ADD_EXECUTABLE (ptExampleVL ${HDF5_HL_CPP_EXAMPLES_SOURCE_DIR}/ptExampleVL.cpp)
  TARGET_LINK_LIBRARIES (
      ptExampleVL
      ${HDF5_HL_CPP_LIB_NAME}
      ${HDF5_HL_LIB_NAME}
      ${HDF5_CPP_LIB_NAME}
      ${HDF5_LIB_NAME}
  )
ENDIF (HDF5_BUILD_EXAMPLES)
