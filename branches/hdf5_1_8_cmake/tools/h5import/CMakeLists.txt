cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_H5IMPORT)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# Add the h5import and test executables
# --------------------------------------------------------------------
ADD_EXECUTABLE (h5import ${HDF5_TOOLS_H5IMPORT_SOURCE_DIR}/h5import.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (h5import PROPERTIES OUTPUT_NAME "h5import")
    SET_TARGET_PROPERTIES (h5import PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5import  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES h5import)

IF (BUILD_TESTING)
  ADD_EXECUTABLE (h5importtest ${HDF5_TOOLS_H5IMPORT_SOURCE_DIR}/h5importtest.c)
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (h5importtest PROPERTIES OUTPUT_NAME "h5importtest")
      SET_TARGET_PROPERTIES (h5importtest PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (h5importtest ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
  GET_TARGET_PROPERTY (h5importtestpath h5importtest LOCATION)
  GET_FILENAME_COMPONENT (h5importtestexe ${h5importtestpath} NAME)
  ADD_TEST (h5importtest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${h5importtestexe})

  SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
        h5importtest
  )
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Rules for Installation of tools using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        h5import
    RUNTIME DESTINATION
        bin/tools
)
