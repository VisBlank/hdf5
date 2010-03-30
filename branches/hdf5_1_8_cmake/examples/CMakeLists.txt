cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_EXAMPLES)

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

SET (examples
    h5_write
    h5_read
    h5_extend_write
    h5_chunk_read
    h5_compound
    h5_group
    h5_select
    h5_attribute
    h5_mount
    h5_reference
    h5_drivers
    h5_ref2reg
    h5_extlink
    h5_elink_unix2win
    h5_shared_mesg
)

FOREACH (example ${examples})
  ADD_EXECUTABLE (${example} ${HDF5_EXAMPLES_SOURCE_DIR}/${example}.c)
  TARGET_LINK_LIBRARIES (${example} ${HDF5_LIB_NAME})
ENDFOREACH (example ${examples})

IF (H5_HAVE_PARALLEL)
  ADD_EXECUTABLE (ph5example ${HDF5_EXAMPLES_SOURCE_DIR}/ph5example.c)
  TARGET_LINK_LIBRARIES (ph5example ${HDF5_LIB_NAME})
ENDIF (H5_HAVE_PARALLEL)
