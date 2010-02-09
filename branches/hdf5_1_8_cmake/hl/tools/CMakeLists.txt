# set project's name
PROJECT( HDF5_HL_TOOLS )
cmake_minimum_required(VERSION 2.6)

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

SET ( GIF2H5_SRCS 
      ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/gif.h
      ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/decompress.c
      ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/gif2hdf.c
      ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/gif2mem.c
      ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/gifread.c
      ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/writehdf.c
      ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_utils.c )
  
  #-- Add gif2hdf5 program
include_directories(${HDF5_TOOLS_SOURCE_DIR}/lib)
include_directories(${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5)
ADD_EXECUTABLE(gif2h5 ${GIF2H5_SRCS})
TARGET_LINK_LIBRARIES(gif2h5 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME} )
  
#-- Add h52gif program
set (hdf2gif_SRCS
  ${HDF5_TOOLS_SOURCE_DIR}/lib/h5tools_utils.c
  ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/hdf2gif.c 
  ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/hdfgifwr.c
)
ADD_EXECUTABLE(hdf2gif ${hdf2gif_SRCS})
TARGET_LINK_LIBRARIES(hdf2gif ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME} )
  
#--- Set up the installation Code for Make Install
INSTALL(TARGETS gif2h5 hdf2gif
  RUNTIME DESTINATION bin
)
  
# --------------------------------------------------------------------
# This executable can generate the actual test files - Currently not 
#  used in the CMake Build system as we rely on the test files that are
#  shipped with HDF5 source archives
# --------------------------------------------------------------------
IF(BUILD_TESTING)
    ADD_EXECUTABLE(hl_h52gifgentest ${HDF5_HL_TOOLS_SOURCE_DIR}/gif2h5/h52gifgentst.c)
    TARGET_LINK_LIBRARIES(hl_h52gifgentest ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )
   # ADD_TEST(hl_h52gifgentest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hl_h52gifgentest)
ENDIF(BUILD_TESTING)

