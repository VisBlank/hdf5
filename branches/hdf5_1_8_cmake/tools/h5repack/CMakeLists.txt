cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_H5REPACK)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# Add h5Repack executables and tests
# --------------------------------------------------------------------
SET (REPACK_COMMON_SRCS
    ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack_copy.c
    ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack_filters.c
    ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack_opttable.c
    ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack_parse.c
    ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack_refs.c
    ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack_verify.c
    ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack.c
)
INCLUDE_DIRECTORIES (${HDF5_TEST_SOURCE_DIR})
ADD_EXECUTABLE (h5repack ${REPACK_COMMON_SRCS} ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repack_main.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (h5repack PROPERTIES OUTPUT_NAME "h5repack")
    SET_TARGET_PROPERTIES (h5repack PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5repack  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES h5repack)

IF (BUILD_TESTING)
  ADD_EXECUTABLE (testh5repack_detect_szip ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/testh5repack_detect_szip.c)
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (testh5repack_detect_szip PROPERTIES OUTPUT_NAME "testh5repack_detect_szip")
      SET_TARGET_PROPERTIES (testh5repack_detect_szip PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (testh5repack_detect_szip ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET} ${HDF5_TEST_LIB_TARGET})
  GET_TARGET_PROPERTY (testh5repack_detect_szippath testh5repack_detect_szip LOCATION)
  GET_FILENAME_COMPONENT (testh5repack_detect_szipexe ${testh5repack_detect_szippath} NAME)
  ADD_TEST (testh5repack_detect_szip ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${testh5repack_detect_szipexe})
  SET (passRegex "yes")
  SET_TESTS_PROPERTIES (testh5repack_detect_szip PROPERTIES PASS_REGULAR_EXPRESSION "yes")

  ADD_EXECUTABLE (h5repacktest ${REPACK_COMMON_SRCS} ${HDF5_TOOLS_H5REPACK_SOURCE_DIR}/h5repacktst.c)
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (h5repacktest PROPERTIES OUTPUT_NAME "h5repacktest")
      SET_TARGET_PROPERTIES (h5repacktest PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (h5repacktest  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_TEST_LIB_TARGET})
  GET_TARGET_PROPERTY (h5repacktestpath h5repacktest LOCATION)
  GET_FILENAME_COMPONENT (h5repacktestexe ${h5repacktestpath} NAME)
  ADD_TEST (h5repacktest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${h5repacktestexe})

  SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
        h5repacktest
        testh5repack_detect_szip
  )
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Rules for Installation of tools using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        h5repack
    RUNTIME DESTINATION
        bin/tools
)
