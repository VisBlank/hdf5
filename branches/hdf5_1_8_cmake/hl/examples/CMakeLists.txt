# set project's name
PROJECT( HDF5_HL_EXAMPLES )
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


#-- Adding Example program ex_image1
ADD_EXECUTABLE(ex_image1 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_image1.c)
TARGET_LINK_LIBRARIES(ex_image1 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_image2
ADD_EXECUTABLE(ex_image2 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_image2.c)
TARGET_LINK_LIBRARIES(ex_image2 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_lite1
ADD_EXECUTABLE(ex_lite1 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_lite1.c)
TARGET_LINK_LIBRARIES(ex_lite1 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_lite2
ADD_EXECUTABLE(ex_lite2 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_lite2.c)
TARGET_LINK_LIBRARIES(ex_lite2 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_lite3
ADD_EXECUTABLE(ex_lite3 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_lite3.c)
TARGET_LINK_LIBRARIES(ex_lite3 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_01
ADD_EXECUTABLE(ex_table_01 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_01.c)
TARGET_LINK_LIBRARIES(ex_table_01 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_02
ADD_EXECUTABLE(ex_table_02 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_02.c)
TARGET_LINK_LIBRARIES(ex_table_02 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_03
ADD_EXECUTABLE(ex_table_03 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_03.c)
TARGET_LINK_LIBRARIES(ex_table_03 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_04
ADD_EXECUTABLE(ex_table_04 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_04.c)
TARGET_LINK_LIBRARIES(ex_table_04 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_05
ADD_EXECUTABLE(ex_table_05 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_05.c)
TARGET_LINK_LIBRARIES(ex_table_05 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_06
ADD_EXECUTABLE(ex_table_06 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_06.c)
TARGET_LINK_LIBRARIES(ex_table_06 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_07
ADD_EXECUTABLE(ex_table_07 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_07.c)
TARGET_LINK_LIBRARIES(ex_table_07 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_08
ADD_EXECUTABLE(ex_table_08 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_08.c)
TARGET_LINK_LIBRARIES(ex_table_08 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_09
ADD_EXECUTABLE(ex_table_09 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_09.c)
TARGET_LINK_LIBRARIES(ex_table_09 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_10
ADD_EXECUTABLE(ex_table_10 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_10.c)
TARGET_LINK_LIBRARIES(ex_table_10 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_11
ADD_EXECUTABLE(ex_table_11 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_11.c)
TARGET_LINK_LIBRARIES(ex_table_11 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

#-- Adding Example program ex_table_12
ADD_EXECUTABLE(ex_table_12 ${HDF5_HL_EXAMPLES_SOURCE_DIR}/ex_table_12.c)
TARGET_LINK_LIBRARIES(ex_table_12 ${HDF5_HL_LIB_NAME} ${HDF5_LIB_NAME} )

