cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_H5STAT)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# Add the h5stat and test executables
# --------------------------------------------------------------------
ADD_EXECUTABLE (h5stat
    ${HDF5_TOOLS_H5STAT_SOURCE_DIR}/h5stat.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (h5stat PROPERTIES OUTPUT_NAME "h5stat")
    SET_TARGET_PROPERTIES (h5stat PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5stat  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES h5stat)

IF (BUILD_TESTING)
  IF (NOT BUILD_SHARED_LIBS)
    ADD_EXECUTABLE (h5stat_gentest ${HDF5_TOOLS_H5STAT_SOURCE_DIR}/h5stat_gentest.c)
    TARGET_LINK_LIBRARIES (h5stat_gentest ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
    #ADD_TEST (h5stat_gentest h5stat_gentest)

    SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
    #    h5stat_gentest
    )
  ENDIF (NOT BUILD_SHARED_LIBS)
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Rules for Installation of tools using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        h5stat
    RUNTIME DESTINATION
        bin/tools
)
