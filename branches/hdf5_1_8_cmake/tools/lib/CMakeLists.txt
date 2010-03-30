cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_TOOLS_LIB)

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

#INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib)
#INCLUDE_DIRECTORIES (${HDF5_PROJECT_DIR}/test)
IF (WIN32)
  IF (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS (-D_HDF5USEDLL_)
  ENDIF (BUILD_SHARED_LIBS)
ENDIF (WIN32)

SET (H5_TOOLS_LIB_SRCS
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5diff.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5diff_array.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5diff_attr.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5diff_dset.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5diff_util.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_filters.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_ref.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_str.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_type.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_utils.c
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5trav.c
)

SET (H5_TOOLS_LIB_HDRS
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5trav.h
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools.h
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_utils.h
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_str.h
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_ref.h
    ${HDF5_TOOLS_SOURCE_DIR}/lib/h5diff.h
)


#-- Always build a static library for linking the ${HDF5_LIB_NAME} tools together
ADD_LIBRARY (${HDF5_TOOLS_LIB_NAME}-static STATIC ${H5_TOOLS_LIB_SRCS} ${H5_TOOLS_LIB_HDRS})
SET_GLOBAL_VARIABLE( HDF5_LIBRARIES_TO_EXPORT "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_TOOLS_LIB_NAME}-static")
SET_TARGET_PROPERTIES (${HDF5_TOOLS_LIB_NAME}-static PROPERTIES OUTPUT_NAME "HDF5_TEST_LIB_NAME")
SET_TARGET_PROPERTIES (${HDF5_TOOLS_LIB_NAME}-static PROPERTIES PREFIX "lib")
H5_SET_LIB_OPTIONS (
    ${HDF5_TOOLS_LIB_NAME}-static ${HDF5_TOOLS_LIB_NAME}
    STATIC
    HDF5_TOOLS_LIB_NAME_RELEASE 
	HDF5_TOOLS_LIB_NAME_DEBUG
)
