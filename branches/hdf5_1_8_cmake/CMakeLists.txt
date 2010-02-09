PROJECT(HDF5_F90 C CXX Fortran)

#-----------------------------------------------------------------------------
# Setup output Directories 
#-----------------------------------------------------------------------------
IF(NOT HDF5_EXTERNALLY_CONFIGURED)
  SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
  )
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
  )
  SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
  )
ENDIF(NOT HDF5_EXTERNALLY_CONFIGURED)

#-----------------------------------------------------------------------------
# Fortran support is best served using cmake 2.8.0
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8)

#-----------------------------------------------------------------------------
# Detect name mangling convention used between Fortran and C
#-----------------------------------------------------------------------------
include(FortranCInterface)
FortranCInterface_HEADER(${HDF5_F90_BINARY_DIR}/FCMangle.h
                         MACRO_NAMESPACE "H5_FC_"
                         SYMBOL_NAMESPACE "H5_FC_"
                         SYMBOLS mysub mymod:my_sub)

FILE(STRINGS ${HDF5_F90_BINARY_DIR}/FCMangle.h CONTENTS REGEX "H5_FC_GLOBAL\\(.*,.*\\) +(.*)") 
STRING(REGEX MATCH "H5_FC_GLOBAL\\(.*,.*\\) +(.*)" RESULT ${CONTENTS})
SET(H5_FC_FUNC "H5_FC_FUNC(name,NAME) ${CMAKE_MATCH_1}" PARENT_SCOPE)

FILE(STRINGS ${HDF5_F90_BINARY_DIR}/FCMangle.h CONTENTS REGEX "H5_FC_GLOBAL_\\(.*,.*\\) +(.*)") 
STRING(REGEX MATCH "H5_FC_GLOBAL_\\(.*,.*\\) +(.*)" RESULT ${CONTENTS})
SET(H5_FC_FUNC_ "H5_FC_FUNC_(name,NAME) ${CMAKE_MATCH_1}" PARENT_SCOPE)

#-----------------------------------------------------------------------------
# The provided CMake Fortran macros don't provide a general check function
# so this one is used for a sizeof test.
#-----------------------------------------------------------------------------
macro(CHECK_FORTRAN_FEATURE FUNCTION CODE VARIABLE)
  if(NOT DEFINED ${VARIABLE})
    message(STATUS "Testing Fortran ${FUNCTION}")
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_FUNCTION_EXISTS_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    else(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_FUNCTION_EXISTS_ADD_LIBRARIES)
    endif(CMAKE_REQUIRED_LIBRARIES)
    FILE(WRITE
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testFortranCompiler.f
      "${CODE}"
    )
    try_compile(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testFortranCompiler.f
      CMAKE_FLAGS "${CHECK_FUNCTION_EXISTS_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT
    )

#    MESSAGE( "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ")
#    MESSAGE( "Test result ${OUTPUT}")
#    MESSAGE( "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ")

    if(${VARIABLE})
      set(${VARIABLE} 1 CACHE INTERNAL "Have Fortran function ${FUNCTION}")
      message(STATUS "Testing Fortran ${FUNCTION} - OK")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Determining if the Fortran ${FUNCTION} exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    else(${VARIABLE})
      message(STATUS "Testing Fortran ${FUNCTION} - Fail")
      set(${VARIABLE} "" CACHE INTERNAL "Have Fortran function ${FUNCTION}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining if the Fortran ${FUNCTION} exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    endif(${VARIABLE})
  endif(NOT DEFINED ${VARIABLE})
endmacro(CHECK_FORTRAN_FEATURE)

#-----------------------------------------------------------------------------
# Configure Checks which require Fortran compilation must go in here
# not in the main ConfigureChecks.cmake files, because if the user has
# no Fortran compiler, problems arise.
#
# Be careful with leading spaces here, do not remove them.
#-----------------------------------------------------------------------------
CHECK_FORTRAN_FEATURE(sizeof 
  "
       PROGRAM main
       i = sizeof(x)       
       END PROGRAM
  " 
  FORTRAN_HAVE_SIZEOF
)  

CHECK_FORTRAN_FEATURE(RealIsNotDouble 
  "
       MODULE type_mod
         INTERFACE h5t	
           MODULE PROCEDURE h5t_real
           MODULE PROCEDURE h5t_dble
         END INTERFACE
       CONTAINS
         SUBROUTINE h5t_real(r)
           REAL :: r
         END SUBROUTINE h5t_real
         SUBROUTINE h5t_dble(d)
           DOUBLE PRECISION :: d
         END SUBROUTINE h5t_dble
       END MODULE type_mod
       PROGRAM main
         USE type_mod
         REAL :: r
         DOUBLE PRECISION :: d
         CALL h5t(r)
         CALL h5t(d)
       END PROGRAM main
  " 
  FORTRAN_DEFAULT_REAL_NOT_DOUBLE
)  

#-----------------------------------------------------------------------------
# Traverse source subdirectory
#-----------------------------------------------------------------------------
#ADD_SUBDIRECTORY(src)
SET(FORTRAN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

#-----------------------------------------------------------------------------
# When building utility executables we reuse these variables
#-----------------------------------------------------------------------------
#inherit EXE_EXT     from parent HDF5 cmake project
#inherit MAKE_SYSTEM from parent HDF5 cmake project
#inherit CFG_INIT    from parent HDF5 cmake project

#-----------------------------------------------------------------------------
# Auto-generated files will be placed and included from here
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES(${HDF5_F90_BINARY_DIR})

#-----------------------------------------------------------------------------
# Setup the Fortran auto-detection utilities
# H5test_kind(_SIZEOF).f90 used to generate H5fortran_detect.f90
# H5fortran_detect.f90     used to generate H5fort_type_defines.h
# H5fort_type_defines.h    used to generate H5f90i_gen.h + H5fortran_types.f90
#-----------------------------------------------------------------------------
IF (FORTRAN_HAVE_SIZEOF)
  ADD_EXECUTABLE(H5test_FortranHavekind
    ${FORTRAN_SRC_DIR}/H5test_kind_SIZEOF.f90
  )
ELSE (FORTRAN_HAVE_SIZEOF)
  ADD_EXECUTABLE(H5test_FortranHavekind 
    ${FORTRAN_SRC_DIR}/H5test_kind.f90
  )
ENDIF (FORTRAN_HAVE_SIZEOF)

SET (CMD ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}${CFG_INIT}/H5test_FortranHavekind${EXE_EXT})
ADD_CUSTOM_COMMAND(
  OUTPUT ${HDF5_F90_BINARY_DIR}/H5fortran_detect.f90
  COMMAND ${CMD} 
  ARGS > ${HDF5_F90_BINARY_DIR}/H5fortran_detect.f90
  WORKING_DIRECTORY ${HDF5_F90_BINARY_DIR}
  DEPENDS H5test_FortranHavekind
)
#-----------------------------------------------------------------------------
ADD_EXECUTABLE(H5fortran_detect 
  ${HDF5_F90_BINARY_DIR}/H5fortran_detect.f90
)

SET (CMD ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}${CFG_INIT}/H5fortran_detect${EXE_EXT})
ADD_CUSTOM_COMMAND(
  OUTPUT ${HDF5_F90_BINARY_DIR}/H5fort_type_defines.h
  COMMAND ${CMD} 
  ARGS > ${HDF5_F90_BINARY_DIR}/H5fort_type_defines.h
  WORKING_DIRECTORY ${HDF5_F90_BINARY_DIR}
  DEPENDS H5fortran_detect
)

#-----------------------------------------------------------------------------
ADD_EXECUTABLE(H5match_types 
  ${HDF5_F90_BINARY_DIR}/H5fort_type_defines.h
  ${FORTRAN_SRC_DIR}/H5match_types.c
)

SET (CMD ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}${CFG_INIT}/H5match_types${EXE_EXT})
ADD_CUSTOM_COMMAND(
  OUTPUT ${HDF5_F90_BINARY_DIR}/H5f90i_gen.h
         ${HDF5_F90_BINARY_DIR}/H5fortran_types.f90
  COMMAND ${CMD}
  WORKING_DIRECTORY ${HDF5_F90_BINARY_DIR}
  DEPENDS H5match_types
)

#-----------------------------------------------------------------------------
# f90CStub lib
#-----------------------------------------------------------------------------
SET(f90CStub_C_SRCS
  # generated files
  ${HDF5_F90_BINARY_DIR}/H5f90i_gen.h
  
  # normal distribution
  ${FORTRAN_SRC_DIR}/H5f90kit.c 
  ${FORTRAN_SRC_DIR}/H5_f.c 
  ${FORTRAN_SRC_DIR}/H5Af.c 
  ${FORTRAN_SRC_DIR}/H5Df.c
  ${FORTRAN_SRC_DIR}/H5Ef.c
  ${FORTRAN_SRC_DIR}/H5Ff.c 
  ${FORTRAN_SRC_DIR}/H5Gf.c
  ${FORTRAN_SRC_DIR}/H5If.c 
  ${FORTRAN_SRC_DIR}/H5Lf.c 
  ${FORTRAN_SRC_DIR}/H5Of.c 
  ${FORTRAN_SRC_DIR}/H5Pf.c 
  ${FORTRAN_SRC_DIR}/H5Rf.c 
  ${FORTRAN_SRC_DIR}/H5Sf.c
  ${FORTRAN_SRC_DIR}/H5Tf.c 
  ${FORTRAN_SRC_DIR}/H5Zf.c 
)

IF (H5_HAVE_PARALLEL)
  SET (f90CStub_C_SRCS 
    ${f90CStub_C_SRCS} 
    ${FORTRAN_SRC_DIR}/H5FDmpiof.c 
  )
ENDIF (H5_HAVE_PARALLEL)

SET_SOURCE_FILES_PROPERTIES( ${f90CStub_C_SRCS} PROPERTIES LANGUAGE C) 

ADD_LIBRARY(${HDF5_F90_C_LIB_NAME} ${f90CStub_C_SRCS} )
TARGET_LINK_LIBRARIES(${HDF5_F90_C_LIB_NAME} hdf5)

#-----------------------------------------------------------------------------
# Fortran Real Size
#-----------------------------------------------------------------------------
IF (FORTRAN_DEFAULT_REAL_NOT_DOUBLE)
  # default real is 4 bytes, so include double signatures
  SET (F_DBLE "Include")
ELSE (FORTRAN_DEFAULT_REAL_NOT_DOUBLE)
  # default real is 8 bytes, so exclude double signatures
  SET (F_DBLE "Exclude")
ENDIF (FORTRAN_DEFAULT_REAL_NOT_DOUBLE)

#-----------------------------------------------------------------------------
# Fortran Modules
#-----------------------------------------------------------------------------
IF (H5_HAVE_PARALLEL)
  SET (H5FC_NAME h5pfc)
ELSE (H5_HAVE_PARALLEL)
  SET (H5FC_NAME h5fc)
ENDIF (H5_HAVE_PARALLEL)

#-----------------------------------------------------------------------------
ADD_LIBRARY(H5FORTRAN_TYPES   STATIC ${HDF5_F90_BINARY_DIR}/H5fortran_types.f90 )
ADD_LIBRARY(H5GLOBAL          STATIC ${FORTRAN_SRC_DIR}/H5f90global.f90 )
ADD_LIBRARY(H5LIB             STATIC ${FORTRAN_SRC_DIR}/H5_ff.f90 )
ADD_LIBRARY(H5_DBLE_INTERFACE STATIC ${FORTRAN_SRC_DIR}/H5_DBLE_Interface${F_DBLE}.f90 )
ADD_LIBRARY(H5A               STATIC ${FORTRAN_SRC_DIR}/H5Aff.f90 )
ADD_LIBRARY(H5D               STATIC ${FORTRAN_SRC_DIR}/H5Dff.f90 )
ADD_LIBRARY(H5E               STATIC ${FORTRAN_SRC_DIR}/H5Eff.f90 )
ADD_LIBRARY(H5F               STATIC ${FORTRAN_SRC_DIR}/H5Fff.f90 )
ADD_LIBRARY(H5G               STATIC ${FORTRAN_SRC_DIR}/H5Gff.f90 )
ADD_LIBRARY(H5I               STATIC ${FORTRAN_SRC_DIR}/H5Iff.f90 )
ADD_LIBRARY(H5L               STATIC ${FORTRAN_SRC_DIR}/H5Lff.f90 )
ADD_LIBRARY(H5O               STATIC ${FORTRAN_SRC_DIR}/H5Off.f90 )
ADD_LIBRARY(H5P               STATIC ${FORTRAN_SRC_DIR}/H5Pff.f90 )
ADD_LIBRARY(H5R               STATIC ${FORTRAN_SRC_DIR}/H5Rff.f90 )
ADD_LIBRARY(H5S               STATIC ${FORTRAN_SRC_DIR}/H5Sff.f90 )
ADD_LIBRARY(H5T               STATIC ${FORTRAN_SRC_DIR}/H5Tff.f90 )
ADD_LIBRARY(H5Z               STATIC ${FORTRAN_SRC_DIR}/H5Zff.f90 )
#-----------------------------------------------------------------------------
# Force dependencies between libs to ensure build order is ok
#-----------------------------------------------------------------------------
add_dependencies(H5A H5GLOBAL)
add_dependencies(H5D H5GLOBAL)
add_dependencies(H5E H5GLOBAL)
add_dependencies(H5F H5GLOBAL)
add_dependencies(H5G H5GLOBAL)
add_dependencies(H5I H5GLOBAL)
add_dependencies(H5L H5GLOBAL)
add_dependencies(H5O H5GLOBAL)
add_dependencies(H5P H5GLOBAL)
add_dependencies(H5R H5GLOBAL)
add_dependencies(H5S H5GLOBAL)
add_dependencies(H5T H5GLOBAL)
add_dependencies(H5Z H5GLOBAL)
add_dependencies(H5GLOBAL          H5FORTRAN_TYPES)
add_dependencies(H5LIB             H5GLOBAL)
add_dependencies(H5_DBLE_INTERFACE H5GLOBAL)
add_dependencies(H5_DBLE_INTERFACE H5A H5D H5P)
#-----------------------------------------------------------------------------
IF (H5_HAVE_PARALLEL)
  ADD_LIBRARY(H5FDMPIO STATIC ${FORTRAN_SRC_DIR}/H5FDmpioff.f90 )
  add_dependencies(H5FDMPIO H5GLOBAL)
  SET (HDF5MODULE_SRC 
    ${FORTRAN_SRC_DIR}/HDF5mpio.f90
  )
  SET (HDF5_DEPENDS H5FDMPIO)
ELSE (H5_HAVE_PARALLEL)
  SET (HDF5MODULE_SRC 
    ${FORTRAN_SRC_DIR}/HDF5.f90
  )
  SET (HDF5_DEPENDS "")
ENDIF (H5_HAVE_PARALLEL)
#-----------------------------------------------------------------------------
ADD_LIBRARY(${HDF5_F90_LIB_NAME} STATIC ${HDF5MODULE_SRC} )
SET_GLOBAL_VARIABLE(HDF5_LIBRARIES_TO_EXPORT 
  "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_F90_LIB_NAME};${HDF5_F90_C_LIB_NAME}"
)
add_dependencies(${HDF5_F90_LIB_NAME}
  H5GLOBAL H5F H5E H5G H5I H5L H5S H5D H5A H5T H5O
  H5P H5R H5Z H5_DBLE_INTERFACE H5LIB ${HDF5_DEPENDS}
  ${HDF5_F90_C_LIB_NAME}
)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

IF (BUILD_TESTING)
  ADD_SUBDIRECTORY(${HDF5_F90_SOURCE_DIR}/test ${HDF5_F90_BINARY_DIR}/test)
ENDIF (BUILD_TESTING)