PROJECT(HDF5_HL_F90 C CXX Fortran)

#-----------------------------------------------------------------------------
# List Source files
#-----------------------------------------------------------------------------
MESSAGE( "Fortran source is in ${HDF5_F90_SOURCE_DIR}/src ")
INCLUDE_DIRECTORIES( ${HDF5_F90_SOURCE_DIR}/src )
INCLUDE_DIRECTORIES( ${HDF5_HL_F90_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${HDF5_F90_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${HDF5_HL_F90_SOURCE_DIR}/src )

ADD_LIBRARY(H5TB STATIC ${HDF5_HL_F90_SOURCE_DIR}/src/H5TBff.f90 )
ADD_LIBRARY(H5LT STATIC ${HDF5_HL_F90_SOURCE_DIR}/src/H5LTff.f90 )
ADD_LIBRARY(H5IM STATIC ${HDF5_HL_F90_SOURCE_DIR}/src/H5IMff.f90 )

SET (HDF5_HL_F90_C_SRCS
  ${HDF5_HL_F90_SOURCE_DIR}/src/H5LTfc.c  
  ${HDF5_HL_F90_SOURCE_DIR}/src/H5IMfc.c 
  ${HDF5_HL_F90_SOURCE_DIR}/src/H5IMcc.c 
  ${HDF5_HL_F90_SOURCE_DIR}/src/H5TBfc.c 
)

add_dependencies(H5TB H5GLOBAL)
add_dependencies(H5LT H5GLOBAL)
add_dependencies(H5IM H5GLOBAL)

SET (HDF5_HL_F90_HEADERS
  ${HDF5_HL_F90_SOURCE_DIR}/src/H5LTf90proto.h 
)

ADD_LIBRARY(${HDF5_HL_F90_C_LIB_NAME} 
  ${LIB_TYPE} 
  ${HDF5_HL_F90_C_SRCS} ${HDF5_HL_F90_HEADERS} 
)
SET_GLOBAL_VARIABLE(HDF5_LIBRARIES_TO_EXPORT 
  "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_HL_F90_C_LIB_NAME}"
)
TARGET_LINK_LIBRARIES(${HDF5_HL_F90_C_LIB_NAME} 
  ${HDF5_HL_LIB_NAME} 
  ${HDF5_F90_C_LIB_NAME}
)
H5_SET_LIB_OPTIONS(${HDF5_HL_F90_C_LIB_NAME} 
  ${HDF5_HL_F90_C_LIB_NAME} ${LIB_TYPE}
)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

IF (BUILD_TESTING)
  ADD_SUBDIRECTORY(${HDF5_HL_F90_SOURCE_DIR}/test ${HDF5_HL_F90_BINARY_DIR}/test)
ENDIF (BUILD_TESTING)
