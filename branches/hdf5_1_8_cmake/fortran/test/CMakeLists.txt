cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_FORTRAN_TESTS C CXX Fortran)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_BINARY_DIR} ${HDF5_F90_SOURCE_DIR}/src)
LINK_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_BINARY_DIR} ${HDF5_F90_SOURCE_DIR}/src ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_Fortran_MODULE_DIRECTORY})

#-----------------------------------------------------------------------------
# Add Test Lib
#-----------------------------------------------------------------------------
ADD_LIBRARY (${HDF5_F90_C_TEST_LIB_TARGET} t.c)
# Must explicity add a dependency here to force the generation of headers
# which are created for ${HDF5_F90_C_LIB_TARGET}
ADD_DEPENDENCIES(${HDF5_F90_C_TEST_LIB_TARGET} ${HDF5_F90_C_LIB_TARGET} )
TARGET_LINK_LIBRARIES (${HDF5_F90_C_TEST_LIB_TARGET} 
  ${HDF5_F90_C_LIB_TARGET}
  ${HDF5_TEST_LIB_TARGET} 
  ${HDF5_F90_LIB_TARGET} 
)

ADD_LIBRARY (${HDF5_F90_TEST_LIB_TARGET} STATIC tf.f90)
# Must explicity add a dependency here to force the generation of headers
# which are created for ${HDF5_F90_C_LIB_TARGET}
ADD_DEPENDENCIES(${HDF5_F90_TEST_LIB_TARGET} ${HDF5_F90_C_LIB_TARGET} )
TARGET_LINK_LIBRARIES (${HDF5_F90_TEST_LIB_TARGET} ${HDF5_F90_C_TEST_LIB_TARGET})

#-----------------------------------------------------------------------------
# Add Tests
#-----------------------------------------------------------------------------

#-- Adding test for fortranlib_test
ADD_EXECUTABLE (fortranlib_test
    fortranlib_test.f90
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
TARGET_LINK_LIBRARIES (fortranlib_test ${HDF5_F90_TEST_LIB_TARGET})
ADD_TEST (fortranlib_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fortranlib_test)

#-- Adding test for fortranlib_test_1_8
ADD_EXECUTABLE (fortranlib_test_1_8
    fortranlib_test_1_8.f90
    tH5F.f90
    tH5O.f90
    tH5A_1_8.f90
    tH5G_1_8.f90
)
TARGET_LINK_LIBRARIES (fortranlib_test_1_8 ${HDF5_F90_TEST_LIB_TARGET})
ADD_TEST (fortranlib_test_1_8 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fortranlib_test_1_8)

#-- Adding test for fflush1
ADD_EXECUTABLE (fflush1 fflush1.f90)
TARGET_LINK_LIBRARIES (fflush1 ${HDF5_F90_TEST_LIB_TARGET})
ADD_TEST (fflush1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fflush1)

#-- Adding test for fflush2
ADD_EXECUTABLE (fflush2 fflush2.f90)
TARGET_LINK_LIBRARIES (fflush2 ${HDF5_F90_TEST_LIB_TARGET})
ADD_TEST (fflush2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fflush2)
