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
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (h5dump PROPERTIES OUTPUT_NAME "h5dump")
    SET_TARGET_PROPERTIES (h5dump PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5dump  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES h5dump)

IF (BUILD_TESTING)
  IF (NOT BUILD_SHARED_LIBS)
    ADD_EXECUTABLE (h5dumpgentest ${HDF5_TOOLS_H5DUMP_SOURCE_DIR}/h5dumpgentest.c)
    TARGET_LINK_LIBRARIES (h5dumpgentest ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
    #ADD_TEST (h5dumpgentest h5dumpgentest)

    SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
    #    h5dumpgentest
    )
  ENDIF (NOT BUILD_SHARED_LIBS)
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
