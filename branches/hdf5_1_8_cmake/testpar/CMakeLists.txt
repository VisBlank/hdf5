cmake_minimum_required (VERSION 2.8)
PROJECT (H5_TEST_PAR)

#-----------------------------------------------------------------------------
# Define Tests
#-----------------------------------------------------------------------------

SET (testphdf5_SRCS
    ${HDF5_TEST_PAR_SOURCE_DIR}/testphdf5.c
    ${HDF5_TEST_PAR_SOURCE_DIR}/t_dset.c t_file.c
    ${HDF5_TEST_PAR_SOURCE_DIR}/t_mdset.c t_ph5basic.c
    ${HDF5_TEST_PAR_SOURCE_DIR}/t_coll_chunk.c
    ${HDF5_TEST_PAR_SOURCE_DIR}/t_span_tree.c
    ${HDF5_TEST_PAR_SOURCE_DIR}/t_chunk_alloc.c
    ${HDF5_TEST_PAR_SOURCE_DIR}/t_filter_read.c
)

#-- Adding test for testhdf5
ADD_EXECUTABLE (testphdf5 ${testphdf5_SRCS})
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (testphdf5 PROPERTIES OUTPUT_NAME "testphdf5")
    SET_TARGET_PROPERTIES (testphdf5 PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (testphdf5 ${HDF5_TEST_LIB_TARGET} ${HDF5_LIB_TARGET})

ADD_TEST (NAME testphdf5 COMMAND $<TARGET_FILE:testphdf5>)

MACRO (ADD_H5P_TEST file)
  ADD_EXECUTABLE (${file} ${HDF5_TEST_PAR_SOURCE_DIR}/${file}.c)
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (${file} PROPERTIES OUTPUT_NAME "${file}")
      SET_TARGET_PROPERTIES (${file} PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (${file} ${HDF5_TEST_LIB_TARGET} ${HDF5_LIB_TARGET})

  ADD_TEST (NAME ${file} COMMAND $<TARGET_FILE:${file}>)
ENDMACRO (ADD_H5P_TEST file)

SET (H5P_TESTS
    t_mpi
    t_posix_compliant
    t_cache
    t_pflush1
    t_pflush2
)

FOREACH (testp ${H5P_TESTS})
    #MESSAGE (STATUS "Adding test for ${testp}")
    ADD_H5P_TEST(${testp})
ENDFOREACH (testp ${H5P_TESTS})

