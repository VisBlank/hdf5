PROJECT(HDF5_HL)

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
# Shared Libs
#-----------------------------------------------------------------------------
IF (BUILD_SHARED_LIBS)
  SET (HL_BUILT_AS_DYNAMIC_LIB 1)         
ENDIF (BUILD_SHARED_LIBS)
 
#-----------------------------------------------------------------------------
# List Source files
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES( ${HDF5_HL_SOURCE_DIR}/src )

SET (HL_SRCS
  ${HDF5_HL_SOURCE_DIR}/src/H5DS.c 
  ${HDF5_HL_SOURCE_DIR}/src/H5IM.c 
  ${HDF5_HL_SOURCE_DIR}/src/H5LT.c 
  ${HDF5_HL_SOURCE_DIR}/src/H5LTanalyze.c
  ${HDF5_HL_SOURCE_DIR}/src/H5LTparse.c 
  ${HDF5_HL_SOURCE_DIR}/src/H5PT.c 
  ${HDF5_HL_SOURCE_DIR}/src/H5TB.c
)

SET (HL_HEADERS
  ${HDF5_HL_SOURCE_DIR}/src/H5DSpublic.h 
  ${HDF5_HL_SOURCE_DIR}/src/H5IMpublic.h 
  ${HDF5_HL_SOURCE_DIR}/src/H5LTparse.h 
  ${HDF5_HL_SOURCE_DIR}/src/H5LTpublic.h 
  ${HDF5_HL_SOURCE_DIR}/src/H5PTpublic.h 
  ${HDF5_HL_SOURCE_DIR}/src/H5TBpublic.h 
  ${HDF5_HL_SOURCE_DIR}/src/hdf5_hl.h 
)

ADD_LIBRARY(${HDF5_HL_LIB_NAME} ${LIB_TYPE} ${HL_SRCS} ${HL_HEADERS} )
SET_GLOBAL_VARIABLE(HDF5_LIBRARIES_TO_EXPORT "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_HL_LIB_NAME}")
TARGET_LINK_LIBRARIES(${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )
H5_SET_LIB_OPTIONS(${HDF5_HL_LIB_NAME} ${HDF5_HL_LIB_NAME} ${LIB_TYPE})

#-- Build the High level Tools
OPTION(HDF5_BUILD_HL_TOOLS "Build the High Level Tools" OFF)
IF (HDF5_BUILD_HL_TOOLS) 
    add_subdirectory( ${HDF5_HL_SOURCE_DIR}/tools ${HDF5_HL_BINARY_DIR}/tools)
ENDIF() 


#-- Add High Level Examples
OPTION(HDF5_BUILD_HL_EXAMPLES "Build the High Level Examples" OFF)
IF(HDF5_BUILD_HL_EXAMPLES)
  add_subdirectory( ${HDF5_HL_SOURCE_DIR}/examples ${HDF5_HL_BINARY_DIR}/examples)
ENDIF()

#-- Build the Unit testing if requested
IF(NOT HDF5_EXTERNALLY_CONFIGURED)
  if (BUILD_TESTING)
      add_subdirectory( ${HDF5_HL_SOURCE_DIR}/test ${HDF5_HL_BINARY_DIR}/test)
  endif()
ENDIF(NOT HDF5_EXTERNALLY_CONFIGURED)

#-- Build the High Level Fortran source codes
if(HDF5_BUILD_FORTRAN)
    add_subdirectory(fortran)
endif()

if(HDF5_BUILD_CPP_LIB)
    add_subdirectory(${HDF5_HL_SOURCE_DIR}/c++ ${HDF5_HL_BINARY_DIR}/c++)
endif()

#-----------------------------------------------------------------------------
# Rules for Installation of headers/etc using make Install target
#-----------------------------------------------------------------------------
INSTALL (
  FILES  
    ${HL_HEADERS}
  DESTINATION 
    include
)

#-----------------------------------------------------------------------------
# Add library to CMake Install : Installs lib and cmake config info
#-----------------------------------------------------------------------------
INSTALL(
  TARGETS 
    ${HDF5_HL_LIB_NAME}
  EXPORT  
    ${HDF5_EXPORTED_TARGETS}
  LIBRARY DESTINATION lib 
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
