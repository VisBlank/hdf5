cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_H5LS)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# If testing was NOT enabled, then we need to build the tools library
# --------------------------------------------------------------------
#-- Add the h5ls executable
ADD_EXECUTABLE (h5ls ${HDF5_TOOLS_H5LS_SOURCE_DIR}/h5ls.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET h5ls
        APPEND PROPERTY COMPILE_DEFINITIONS
            _HDF5USEDLL_
			_HDF5TOOLSUSEDLL_
    )
    SET_TARGET_PROPERTIES (h5ls PROPERTIES OUTPUT_NAME "h5ls")
    SET_TARGET_PROPERTIES (h5ls PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5ls  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES
    h5ls
)

IF (BUILD_TESTING)
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Rules for Installation of tools using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        h5ls
    RUNTIME DESTINATION
        bin/tools
)
