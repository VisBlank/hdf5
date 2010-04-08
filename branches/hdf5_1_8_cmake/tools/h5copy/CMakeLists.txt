cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_H5COPY)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# Add the h5copy and test executables
# --------------------------------------------------------------------
ADD_EXECUTABLE (h5copy ${HDF5_TOOLS_H5COPY_SOURCE_DIR}/h5copy.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET h5copy
        APPEND PROPERTY COMPILE_DEFINITIONS
            _HDF5USEDLL_
			_HDF5TOOLSUSEDLL_
    )
    SET_TARGET_PROPERTIES (h5copy PROPERTIES OUTPUT_NAME "h5copy")
    SET_TARGET_PROPERTIES (h5copy PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5copy  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES h5copy)

IF (BUILD_TESTING)
  IF (NOT BUILD_SHARED_LIBS)
    ADD_EXECUTABLE (h5copygentest ${HDF5_TOOLS_H5COPY_SOURCE_DIR}/h5copygentest.c)
    TARGET_LINK_LIBRARIES (h5copygentest ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
    #ADD_TEST (h5copygentest h5copygentest)

    SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
        #h5copygentest
    )
  ENDIF (NOT BUILD_SHARED_LIBS)
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Rules for Installation of tools using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        h5copy
    RUNTIME DESTINATION
        bin/tools
)
