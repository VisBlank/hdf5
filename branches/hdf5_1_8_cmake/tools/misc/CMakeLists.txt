cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_MISC)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)

# --------------------------------------------------------------------
# Add the misc and test executables
# --------------------------------------------------------------------
#-- Misc Executables
ADD_EXECUTABLE (h5debug ${HDF5_TOOLS_MISC_SOURCE_DIR}/h5debug.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (h5debug PROPERTIES OUTPUT_NAME "h5debug")
    SET_TARGET_PROPERTIES (h5debug PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5debug ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})

ADD_EXECUTABLE (h5repart ${HDF5_TOOLS_MISC_SOURCE_DIR}/h5repart.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (h5repart PROPERTIES OUTPUT_NAME "h5repart")
    SET_TARGET_PROPERTIES (h5repart PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5repart ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})

ADD_EXECUTABLE (h5mkgrp ${HDF5_TOOLS_MISC_SOURCE_DIR}/h5mkgrp.c)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_TARGET_PROPERTIES (h5mkgrp PROPERTIES OUTPUT_NAME "h5mkgrp")
    SET_TARGET_PROPERTIES (h5mkgrp PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5mkgrp  ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})

SET (H5_DEP_EXECUTABLES
    h5debug
    h5repart
    h5mkgrp
)

IF (BUILD_TESTING)
  IF (NOT BUILD_SHARED_LIBS)
    ADD_EXECUTABLE (h5repart_gentest ${HDF5_TOOLS_MISC_SOURCE_DIR}/h5repart_gentest.c)
    TARGET_LINK_LIBRARIES (h5repart_gentest ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
    #ADD_TEST (NAME h5repart_gentest COMMAND $<TARGET_FILE:h5repart_gentest>)
  ENDIF (NOT BUILD_SHARED_LIBS)

  ADD_EXECUTABLE (h5repart_test ${HDF5_TOOLS_MISC_SOURCE_DIR}/repart_test.c)
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (h5repart_test PROPERTIES OUTPUT_NAME "h5repart_test")
      SET_TARGET_PROPERTIES (h5repart_test PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (h5repart_test ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})

  ADD_TEST (NAME h5repart_test COMMAND $<TARGET_FILE:h5repart_test>)

  SET (H5_DEP_EXECUTABLES ${H5_DEP_EXECUTABLES}
     #   h5repart_gentest
        h5repart_test
  )
ENDIF (BUILD_TESTING)

#-----------------------------------------------------------------------------
# Rules for Installation of tools using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        h5debug h5repart
    RUNTIME DESTINATION
        bin/tools
)
