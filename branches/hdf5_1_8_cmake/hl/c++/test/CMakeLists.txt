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
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (hl_ptableTest PROPERTIES OUTPUT_NAME "hl_ptableTest")
      SET_TARGET_PROPERTIES (hl_ptableTest PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (
      hl_ptableTest
      ${HDF5_LIB_TARGET}
      ${HDF5_TEST_LIB_TARGET}
      ${HDF5_CPP_LIB_TARGET}
      ${HDF5_HL_LIB_TARGET}
      ${HDF5_HL_CPP_LIB_TARGET}
  )
  GET_TARGET_PROPERTY (hl_ptableTestpath hl_ptableTest LOCATION)
  GET_FILENAME_COMPONENT (hl_ptableTestexe ${hl_ptableTestpath} NAME)
  ADD_TEST (hl_ptableTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${hl_ptableTestexe})
ENDIF (BUILD_TESTING)
