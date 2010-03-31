cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_F90_EXAMPLES C CXX Fortran)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_FORTRAN_SOURCE_DIR})
LINK_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_FORTRAN_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

SET (examples
    exlite
)

FOREACH (example ${examples})
  ADD_EXECUTABLE (hl_f90_ex_${example} ${HDF5_HL_EXAMPLES_SOURCE_DIR}/${example}.f90)
  TARGET_LINK_LIBRARIES (hl_f90_ex_${example} ${HDF5_HL_F90_LIB_TARGET})
ENDFOREACH (example ${examples})
