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
  H5_NAMING (hl_f90_ex_${example})
  IF (BUILD_SHARED_LIBS)
    IF (WIN32)
      IF (MSVC)
        SET_TARGET_PROPERTIES (hl_f90_ex_${example}
            PROPERTIES
                COMPILE_FLAGS "/dll"
                LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        ) 
      ENDIF (MSVC)
    ENDIF (WIN32)
  ENDIF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES (hl_f90_ex_${example} 
      ${HDF5_F90_LIB_TARGET}
      ${HDF5_HL_F90_LIB_TARGET}
  )
  SET_TARGET_PROPERTIES (hl_f90_ex_${example} PROPERTIES LINKER_LANGUAGE Fortran)
ENDFOREACH (example ${examples})
