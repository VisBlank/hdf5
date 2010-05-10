cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_H5DUMP)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# Add the h5dump and test executables
# --------------------------------------------------------------------
ADD_EXECUTABLE (h5dump ${HDF5_TOOLS_H5DUMP_SOURCE_DIR}/h5dump.c)
H5_NAMING (h5dump)
TARGET_LINK_LIBRARIES (h5dump  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES h5dump)

IF (BUILD_TESTING)
  IF (NOT BUILD_SHARED_LIBS)
    ADD_EXECUTABLE (h5dumpgentest ${HDF5_TOOLS_H5DUMP_SOURCE_DIR}/h5dumpgentest.c)
    H5_NAMING (h5dumpgentest)
    TARGET_LINK_LIBRARIES (h5dumpgentest ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
    
    #ADD_TEST (NAME h5dumpgentest COMMAND $<TARGET_FILE:h5dumpgentest>)

    SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
    #    h5dumpgentest
    )
  ENDIF (NOT BUILD_SHARED_LIBS)
  
    #-- Copy all the HDF5 files from the test directory into the source directory
  SET (HDF5_REFERENCE_FILES
      tgroup-1.ddl
      tgroup-2.ddl
  )
  SET (HDF5_REFERENCE_TEST_FILES
      tgroup.h5
  )

  FOREACH (ddl_file ${HDF5_REFERENCE_FILES})
    SET (ddldest "${PROJECT_BINARY_DIR}/${ddl_file}")
    #MESSAGE (STATUS " Translating ${ddl_file}")
    ADD_CUSTOM_COMMAND (
        TARGET     h5dump
        POST_BUILD
        COMMAND    ${XLATE_UTILITY}
        ARGS       ${HDF5_TOOLS_SOURCE_DIR}/testfiles/${ddl_file} ${ddldest} -l3
    )
  ENDFOREACH (ddl_file ${HDF5_REFERENCE_FILES})

  FOREACH (h5_file ${HDF5_REFERENCE_TEST_FILES})
    SET (dest "${PROJECT_BINARY_DIR}/${h5_file}")
    #MESSAGE (STATUS " Copying ${h5_file}")
    ADD_CUSTOM_COMMAND (
        TARGET     h5dump
        POST_BUILD
        COMMAND    ${CMAKE_COMMAND}
        ARGS       -E copy_if_different ${HDF5_TOOLS_SOURCE_DIR}/testfiles/${h5_file} ${dest}
    )
  ENDFOREACH (h5_file ${HDF5_REFERENCE_TEST_FILES})

  MACRO (ADD_H5_TEST resultfile resultcode)
    ADD_TEST (
        NAME H5DUMP-${resultfile}
        COMMAND "${CMAKE_COMMAND}"
            -D "TEST_PROGRAM=$<TARGET_FILE:h5dump>"
            -D "TEST_ARGS:STRING=${ARGN}"
            -D "TEST_FOLDER=${PROJECT_BINARY_DIR}"
            -D "TEST_OUTPUT=${resultfile}.out"
            -D "TEST_EXPECT=${resultcode}"
            -D "TEST_REFERENCE=${resultfile}.ddl"
            -P "${HDF5_RESOURCES_DIR}/runTest.cmake"
    )
  ENDMACRO (ADD_H5_TEST file)


  # test for displaying groups
  ADD_H5_TEST (tgroup-1 0 tgroup.h5)
  # test for displaying the selected groups
  ADD_H5_TEST (tgroup-2 1 --group=/g2 --group / -g /y tgroup.h5)
   
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Rules for Installation of tools using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        h5dump
    RUNTIME DESTINATION
        bin/tools
)
