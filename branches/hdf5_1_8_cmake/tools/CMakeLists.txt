cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS)

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

INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# If testing was NOT enabled, then we need to build the tools library
# --------------------------------------------------------------------
IF (NOT BUILD_TESTING)
  ADD_SUBDIRECTORY (${HDF5_TOOLS_SOURCE_DIR}/lib)
ENDIF (NOT BUILD_TESTING)

#-- Add the h5diff and test executables
ADD_EXECUTABLE (h5diff
    ${HDF5_TOOLS_SOURCE_DIR}/h5diff/h5diff_common.c
    ${HDF5_TOOLS_SOURCE_DIR}/h5diff/h5diff_main.c)
TARGET_LINK_LIBRARIES (h5diff  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})

#-- Add the h5dump and test executables
ADD_EXECUTABLE (h5dump ${HDF5_TOOLS_SOURCE_DIR}/h5dump/h5dump.c)
TARGET_LINK_LIBRARIES (h5dump  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})

#-- Add the h5import and test executables
ADD_EXECUTABLE (h5import ${HDF5_TOOLS_SOURCE_DIR}/h5import/h5import.c)
TARGET_LINK_LIBRARIES (h5import  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})

#-- Add the h5dump and test executables
ADD_EXECUTABLE (h5jam ${HDF5_TOOLS_SOURCE_DIR}/h5jam/h5jam.c)
TARGET_LINK_LIBRARIES (h5jam  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})
ADD_EXECUTABLE (getub ${HDF5_TOOLS_SOURCE_DIR}/h5jam/getub.c)
TARGET_LINK_LIBRARIES (getub  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})
ADD_EXECUTABLE (tellub ${HDF5_TOOLS_SOURCE_DIR}/h5jam/tellub.c)
TARGET_LINK_LIBRARIES (tellub  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})
ADD_EXECUTABLE (h5unjam ${HDF5_TOOLS_SOURCE_DIR}/h5jam/h5unjam.c)
TARGET_LINK_LIBRARIES (h5unjam  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})

#-- Add the h5ls executable
ADD_EXECUTABLE (h5ls ${HDF5_TOOLS_SOURCE_DIR}/h5ls/h5ls.c)
TARGET_LINK_LIBRARIES (h5ls  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})


#-- Misc Executables
ADD_EXECUTABLE (h5debug ${HDF5_TOOLS_SOURCE_DIR}/misc/h5debug.c)
TARGET_LINK_LIBRARIES (h5debug ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
ADD_EXECUTABLE (h5repart ${HDF5_TOOLS_SOURCE_DIR}/misc/h5repart.c)
TARGET_LINK_LIBRARIES (h5repart ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})

#-- h5Repack executables
SET (REPACK_COMMON_SRCS
    ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_copy.c
    ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_filters.c
  #  ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_list.c
    ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_opttable.c
    ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_parse.c
    ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_refs.c
    ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_verify.c
    ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack.c
)
INCLUDE_DIRECTORIES (${HDF5_TEST_SOURCE_DIR})
ADD_EXECUTABLE (h5repack ${REPACK_COMMON_SRCS} ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repack_main.c)
TARGET_LINK_LIBRARIES (h5repack  ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})

SET (H5_DEP_EXECUTABLES
    h5diff
    h5dump
    h5import
    h5jam
    getub
    tellub
    h5unjam
    h5ls
    h5debug
    h5repart
    h5repack
)


IF (BUILD_TESTING)

    ADD_EXECUTABLE (h5diffgentest ${HDF5_TOOLS_SOURCE_DIR}/h5diff/h5diffgentest.c)
    TARGET_LINK_LIBRARIES (h5diffgentest ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
    #ADD_TEST (h5diffgentest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/h5diffgentest)

    ADD_EXECUTABLE (h5dumpgentest ${HDF5_TOOLS_SOURCE_DIR}/h5dump/h5dumpgentest.c)
    TARGET_LINK_LIBRARIES (h5dumpgentest ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
    #ADD_TEST (h5dumpgentest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/h5dumpgentest)

    ADD_EXECUTABLE (h5jamgentest ${HDF5_TOOLS_SOURCE_DIR}/h5jam/h5jamgentest.c)
    TARGET_LINK_LIBRARIES (h5jamgentest ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
    #ADD_TEST (h5jamgentest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/h5jamgentest)

    ADD_EXECUTABLE (h5importtest ${HDF5_TOOLS_SOURCE_DIR}/h5import/h5importtest.c)
    TARGET_LINK_LIBRARIES (h5importtest ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
    ADD_TEST (h5importtest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/h5importtest)

    ADD_EXECUTABLE (h5repacktest ${REPACK_COMMON_SRCS} ${HDF5_TOOLS_SOURCE_DIR}/h5repack/h5repacktst.c)
    TARGET_LINK_LIBRARIES (h5repacktest  ${HDF5_TOOLS_LIB_NAME} ${HDF5_TEST_LIB_NAME})
    ADD_TEST (h5repacktest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/h5repacktest)

    ADD_EXECUTABLE (testh5repack_detect_szip ${HDF5_TOOLS_SOURCE_DIR}/h5repack/testh5repack_detect_szip.c)
    TARGET_LINK_LIBRARIES (testh5repack_detect_szip ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME} ${HDF5_TEST_LIB_NAME})
    ADD_TEST (testh5repack_detect_szip ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testh5repack_detect_szip)

    SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
     #   h5diffgentest
        binread
     #   h5dumpgentest
     #   h5jamgentest
        h5importtest
        h5repacktest
        testh5repack_detect_szip
    )

ENDIF (BUILD_TESTING)

#--- Set up the installation Code for Make Install
INSTALL (
    TARGETS
        h5debug h5diff h5dump h5import
        h5ls h5repack h5repart
        h5jam h5unjam
    RUNTIME DESTINATION bin
)
