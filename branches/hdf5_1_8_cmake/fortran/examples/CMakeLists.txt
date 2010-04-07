cmake_minimum_required (VERSION 2.8)
# --------------------------------------------------------------------
# Notes: When creating examples they should be prefixed
# with "f90_ex_". This allows for easier filtering of the examples.
# --------------------------------------------------------------------
PROJECT (HDF5_F90_EXAMPLES C CXX Fortran)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_SOURCE_DIR})
LINK_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_Fortran_MODULE_DIRECTORY})

#-----------------------------------------------------------------------------
# Define Sources
#-----------------------------------------------------------------------------
SET (examples
    dsetexample
    fileexample
    rwdsetexample
    attrexample
    groupexample
    grpsexample
    grpdsetexample
    hyperslab
    selectele
    grpit
    refobjexample
    refregexample
    mountexample
    compound
)

FOREACH (example ${examples})
  ADD_EXECUTABLE (f90_ex_${example} ${HDF5_F90_EXAMPLES_SOURCE_DIR}/${example}.f90)
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      SET_TARGET_PROPERTIES (f90_ex_${example} PROPERTIES OUTPUT_NAME "f90_ex_${example}")
      SET_TARGET_PROPERTIES (f90_ex_${example} PROPERTIES PREFIX "dll")
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET f90_ex_${example} 
        APPEND PROPERTY COMPILE_DEFINITIONS 
  	      HDF5F90_WINDOWS
    )
  ENDIF (WIN32)
  TARGET_LINK_LIBRARIES (f90_ex_${example}
      ${HDF5_F90_LIB_TARGET}
      ${HDF5_F90_C_LIB_TARGET}
      ${HDF5_LIB_TARGET}
  )
ENDFOREACH (example ${examples})
