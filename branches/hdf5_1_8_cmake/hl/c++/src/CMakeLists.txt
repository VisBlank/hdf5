cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_CPP_SRC)

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

SET (HDF5_HL_CPP_SRCS ${HDF5_HL_CPP_SRC_SOURCE_DIR}/H5PacketTable.cpp)
SET (HDF5_HL_CPP_HDRS ${HDF5_HL_CPP_SRC_SOURCE_DIR}/H5PacketTable.h)
INCLUDE_DIRECTORIES (${HDF5_HL_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES (${HDF5_HL_CPP_SRC_SOURCE_DIR})

ADD_LIBRARY ( ${HDF5_HL_CPP_LIB_NAME} ${HDF5_HL_CPP_SRCS})
SET_GLOBAL_VARIABLE (HDF5_LIBRARIES_TO_EXPORT "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_HL_CPP_LIB_NAME}")
TARGET_LINK_LIBRARIES (
    ${HDF5_HL_CPP_LIB_NAME}
    ${HDF5_HL_LIB_NAME}
    ${HDF5_CPP_LIB_NAME}
    ${HDF5_LIB_NAME}
)
H5_SET_LIB_OPTIONS (${HDF5_HL_CPP_LIB_NAME} ${HDF5_HL_CPP_LIB_NAME} ${LIB_TYPE})

#-----------------------------------------------------------------------------
# Rules for Installation of headers/etc using make Install target
#-----------------------------------------------------------------------------
INSTALL (
    FILES
        ${HDF5_HL_CPP_HDRS}
    DESTINATION
        include/hl/cpp
)

#-----------------------------------------------------------------------------
# Add library to a (global) CMake 'Install target' for external use
#-----------------------------------------------------------------------------
INSTALL (
    TARGETS
        ${HDF5_HL_CPP_LIB_NAME}
    EXPORT
        ${HDF5_EXPORTED_TARGETS}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
