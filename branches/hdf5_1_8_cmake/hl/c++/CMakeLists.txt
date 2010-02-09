# SET project's name
PROJECT( HDF5_HL_CPP )
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

SET (HDF5_HL_CPP_SRCS ${HDF5_HL_CPP_SOURCE_DIR}/src/H5PacketTable.cpp)
SET (HDF5_HL_CPP_HDRS ${HDF5_HL_CPP_SOURCE_DIR}/src/H5PacketTable.h)
INCLUDE_DIRECTORIES(${HDF5_HL_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${HDF5_HL_CPP_SOURCE_DIR}/src)

add_library( ${HDF5_HL_CPP_LIB_NAME} ${HDF5_HL_CPP_SRCS} )
SET_GLOBAL_VARIABLE(HDF5_LIBRARIES_TO_EXPORT "${HDF5_LIBRARIES_TO_EXPORT};${HDF5_HL_CPP_LIB_NAME}")
TARGET_LINK_LIBRARIES(${HDF5_HL_CPP_LIB_NAME}
                     ${HDF5_HL_LIB_NAME}
                     ${HDF5_CPP_LIB_NAME} 
                     ${HDF5_LIB_NAME} )
H5_SET_LIB_OPTIONS(${HDF5_HL_CPP_LIB_NAME} ${HDF5_HL_CPP_LIB_NAME} ${LIB_TYPE})

# --------------------------------------------------------------------
#  Add in the examples for the Packet Table codes
# --------------------------------------------------------------------
IF (HDF5_BUILD_HL_EXAMPLES)
    ADD_EXECUTABLE(ptExampleFL ${HDF5_HL_CPP_SOURCE_DIR}/examples/ptExampleFL.cpp)
    TARGET_LINK_LIBRARIES(ptExampleFL 
                           ${HDF5_HL_CPP_LIB_NAME}
                         ${HDF5_HL_LIB_NAME}
                         ${HDF5_CPP_LIB_NAME} 
                         ${HDF5_LIB_NAME}  )
                            
    ADD_EXECUTABLE(ptExampleVL ${HDF5_HL_CPP_SOURCE_DIR}/examples/ptExampleVL.cpp)
    TARGET_LINK_LIBRARIES(ptExampleVL 
                          ${HDF5_HL_CPP_LIB_NAME}
                         ${HDF5_HL_LIB_NAME}
                         ${HDF5_CPP_LIB_NAME} 
                         ${HDF5_LIB_NAME}  )
ENDIF()

# --------------------------------------------------------------------
# Add in the unit tests for the packet table c++ wrapper
# --------------------------------------------------------------------

IF (BUILD_TESTING)
    INCLUDE_DIRECTORIES(${HDF5_TEST_SOURCE_DIR} )
    INCLUDE_DIRECTORIES(${HDF5_HL_SOURCE_DIR}/test)
    INCLUDE_DIRECTORIES(${HDF5_CPP_SOURCE_DIR}/src)

    ADD_EXECUTABLE(hl_ptableTest ${HDF5_HL_CPP_SOURCE_DIR}/test/ptableTest.cpp)
    TARGET_LINK_LIBRARIES(hl_ptableTest 
                           ${HDF5_HL_CPP_LIB_NAME}
                         ${HDF5_HL_LIB_NAME}
                         ${HDF5_CPP_LIB_NAME} 
                         ${HDF5_LIB_NAME}  )
    ADD_TEST(hl_ptableTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hl_ptableTest)
ENDIF()

#-----------------------------------------------------------------------------
# Rules for Installation of headers/etc using make Install target
#-----------------------------------------------------------------------------
INSTALL(
  FILES 
    ${HDF5_HL_CPP_HDRS}
  DESTINATION 
    include/hl/cpp
)

#-----------------------------------------------------------------------------
# Add library to a (global) CMake 'Install target' for external use 
#-----------------------------------------------------------------------------
INSTALL(
  TARGETS 
    ${HDF5_HL_CPP_LIB_NAME}
  EXPORT 
    ${HDF5_EXPORTED_TARGETS}
  LIBRARY DESTINATION lib 
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
