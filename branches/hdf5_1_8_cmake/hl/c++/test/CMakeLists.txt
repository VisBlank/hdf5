cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_CPP_TEST)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
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
      ${HDF5_HL_CPP_LIB_TARGET}
      ${HDF5_HL_LIB_TARGET}
      ${HDF5_CPP_LIB_TARGET}
      ${HDF5_LIB_TARGET}
  )
  ADD_TEST (hl_ptableTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hl_ptableTest)
ENDIF (BUILD_TESTING)
