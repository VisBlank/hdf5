PROJECT(HDF5_FORTRAN_TESTS C CXX Fortran)

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
# Add Test Lib
#-----------------------------------------------------------------------------
ADD_LIBRARY(hdf5_f90Ctest
  t.c
)
TARGET_LINK_LIBRARIES(hdf5_f90Ctest ${HDF5_F90_C_LIB_NAME} ${HDF5_TEST_LIB_NAME} )

ADD_LIBRARY(h5test_fortran STATIC
  tf.f90
)
TARGET_LINK_LIBRARIES(h5test_fortran hdf5_f90Ctest )
add_dependencies(h5test_fortran ${HDF5_F90_LIB_NAME})

#-----------------------------------------------------------------------------
# Add Tests
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES(${HDF5_SRC_DIR} ${HDF5_FORTRAN_SOURCE_DIR})
LINK_DIRECTORIES(${HDF5_SRC_DIR} ${HDF5_FORTRAN_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

SET (TEST_MODULE_LIST 
  ${HDF5_F90_LIB_NAME} H5GLOBAL H5F H5E H5G H5I H5L H5S H5D H5A H5T H5O
  H5P H5R H5Z H5_DBLE_INTERFACE H5LIB
)

#-- Adding test for fortranlib_test
ADD_EXECUTABLE(fortranlib_test fortranlib_test.f90
  tH5A.f90 
  tH5D.f90 
  tH5E.f90 
  tH5F.f90 
  tH5G.f90 
  tH5I.f90 
  tH5P.f90 
  tH5R.f90 
  tH5S.f90 
  tH5Sselect.f90 
  tH5T.f90 
  tH5VL.f90 
  tH5Z.f90 
)
TARGET_LINK_LIBRARIES(fortranlib_test h5test_fortran ${TEST_MODULE_LIST} )
ADD_TEST(fortranlib_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fortranlib_test)

#-- Adding test for fortranlib_test_1_8
ADD_EXECUTABLE(fortranlib_test_1_8 
  fortranlib_test_1_8.f90
  tH5F.f90 
  tH5O.f90 
  tH5A_1_8.f90 
  tH5G_1_8.f90 
)
TARGET_LINK_LIBRARIES(fortranlib_test_1_8 h5test_fortran ${TEST_MODULE_LIST} )
ADD_TEST(fortranlib_test_1_8 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fortranlib_test_1_8)

#-- Adding test for fflush1
ADD_EXECUTABLE(fflush1 fflush1.f90)
TARGET_LINK_LIBRARIES(fflush1 h5test_fortran ${TEST_MODULE_LIST} )
ADD_TEST(fflush1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fflush1)

#-- Adding test for fflush2
ADD_EXECUTABLE(fflush2 fflush2.f90)
TARGET_LINK_LIBRARIES(fflush2 h5test_fortran ${TEST_MODULE_LIST} )
ADD_TEST(fflush2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fflush2)



