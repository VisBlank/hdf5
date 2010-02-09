# --------------------------------------------------------------------
# Notes: When creating unit test executables they should be prefixed 
# with "cpp_". This allows for easier filtering of the test suite when
# using ctest. An example would be
#  ctest -R cpp_
# which would only run the C++ based unit tests.
# --------------------------------------------------------------------

project (HDF5_CPP_TEST)

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

SET (CPP_TEST_SRCS
    ${HDF5_CPP_TEST_SOURCE_DIR}/testhdf5.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/tattr.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/tcompound.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/tfile.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/tfilter.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/th5s.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/tlinks.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/trefer.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/ttypes.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/tvlstr.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/h5cpputil.cpp
)


INCLUDE_DIRECTORIES( ${HDF5_TEST_SOURCE_DIR} )

ADD_EXECUTABLE(cpp_testhdf5 ${CPP_TEST_SRCS} )
TARGET_LINK_LIBRARIES(cpp_testhdf5 
                        ${HDF5_CPP_LIB_NAME} 
                        ${HDF5_LIB_NAME}
                        ${HDF5_TEST_LIB_NAME} )
ADD_TEST(cpp_testhdf5 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cpp_testhdf5)
  
set(CPP_DSET_TEST_SRCS 
    ${HDF5_CPP_TEST_SOURCE_DIR}/dsets.cpp
    ${HDF5_CPP_TEST_SOURCE_DIR}/h5cpputil.cpp)
  
add_executable(cpp_dset ${CPP_DSET_TEST_SRCS})
target_link_libraries(cpp_dset 
                    ${HDF5_CPP_LIB_NAME} 
                    ${HDF5_LIB_NAME}
                    ${HDF5_TEST_LIB_NAME} )
ADD_TEST(cpp_dset ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cpp_dset)
  
  
  