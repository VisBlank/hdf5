cmake_minimum_required (VERSION 2.8)
# --------------------------------------------------------------------
# Notes: When creating examples they should be prefixed
# with "cpp_ex_". This allows for easier filtering of the examples.
# --------------------------------------------------------------------

PROJECT (HDF5_CPP_EXAMPLES)

#-----------------------------------------------------------------------------
# Define examples
#-----------------------------------------------------------------------------

SET (examples
    create
    readdata
    writedata
    compound
    extend_ds
    chunks
    h5group
)

FOREACH (example ${examples})
  ADD_EXECUTABLE (cpp_ex_${example} ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/${example}.cpp)
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (cpp_ex_${example} PROPERTIES OUTPUT_NAME "cpp_ex_${example}")
      SET_TARGET_PROPERTIES (cpp_ex_${example} PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (cpp_ex_${example} ${HDF5_CPP_LIB_TARGET} ${HDF5_LIB_TARGET})
ENDFOREACH (example ${examples})
