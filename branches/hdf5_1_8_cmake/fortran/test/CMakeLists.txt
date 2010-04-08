cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_FORTRAN_TESTS C CXX Fortran)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_BINARY_DIR} ${HDF5_F90_SOURCE_DIR}/src)
LINK_DIRECTORIES (
    ${HDF5_SRC_DIR}
    ${HDF5_F90_BINARY_DIR}
    ${HDF5_F90_SOURCE_DIR}/src
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ${CMAKE_Fortran_MODULE_DIRECTORY}
)

#-----------------------------------------------------------------------------
# Add Test Lib
#-----------------------------------------------------------------------------
ADD_LIBRARY (${HDF5_F90_C_TEST_LIB_TARGET} ${LIB_TYPE} t.c)
# Must explicity add a dependency here to force the generation of headers
# which are created for ${HDF5_F90_C_LIB_TARGET}
ADD_DEPENDENCIES(${HDF5_F90_C_TEST_LIB_TARGET} ${HDF5_F90_C_LIB_TARGET} )
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET ${HDF5_F90_C_TEST_LIB_TARGET}
	    APPEND PROPERTY COMPILE_DEFINITIONS
		    HDF5FORTTEST_CSTUB_DLL_EXPORTS
			_HDF5TESTUSEDLL_
	)
  ENDIF (WIN32)
ELSE (BUILD_SHARED_LIBS)
  SET_TARGET_PROPERTIES (${HDF5_F90_C_TEST_LIB_TARGET} PROPERTIES OUTPUT_NAME "${HDF5_F90_C_TEST_LIB_NAME}")
  SET_TARGET_PROPERTIES (${HDF5_F90_C_TEST_LIB_TARGET} PROPERTIES PREFIX "lib")
ENDIF ( BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (${HDF5_F90_C_TEST_LIB_TARGET}
  ${HDF5_F90_C_LIB_TARGET}
  ${HDF5_TEST_LIB_TARGET}
  ${HDF5_F90_LIB_TARGET}
)
H5_SET_LIB_OPTIONS (${HDF5_F90_C_TEST_LIB_TARGET} ${HDF5_F90_C_TEST_LIB_NAME} ${LIB_TYPE})

ADD_LIBRARY (${HDF5_F90_TEST_LIB_TARGET} ${LIB_TYPE} tf.f90)
# Must explicity add a dependency here to force the generation of headers
# which are created for ${HDF5_F90_C_LIB_TARGET}
ADD_DEPENDENCIES(${HDF5_F90_TEST_LIB_TARGET} ${HDF5_F90_C_LIB_TARGET} )
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET ${HDF5_F90_TEST_LIB_TARGET}
	    APPEND PROPERTY COMPILE_DEFINITIONS
		    BUILD_HDF5_DLL
	)
  ENDIF (WIN32)
ELSE (BUILD_SHARED_LIBS)
  SET_TARGET_PROPERTIES (${HDF5_F90_TEST_LIB_TARGET} PROPERTIES OUTPUT_NAME "${HDF5_F90_TEST_LIB_NAME}")
  SET_TARGET_PROPERTIES (${HDF5_F90_TEST_LIB_TARGET} PROPERTIES PREFIX "lib")
ENDIF (BUILD_SHARED_LIBS)
IF (WIN32)
  SET_PROPERTY (TARGET ${HDF5_F90_TEST_LIB_TARGET}
      APPEND PROPERTY COMPILE_DEFINITIONS
  	    HDF5F90_WINDOWS
  )
ENDIF (WIN32)
TARGET_LINK_LIBRARIES (${HDF5_F90_TEST_LIB_TARGET} ${HDF5_F90_C_TEST_LIB_TARGET})
H5_SET_LIB_OPTIONS (${HDF5_F90_TEST_LIB_TARGET} ${HDF5_F90_TEST_LIB_NAME} ${LIB_TYPE})

#-----------------------------------------------------------------------------
# Add Tests
#-----------------------------------------------------------------------------

#-- Adding test for testhdf5_fortran
ADD_EXECUTABLE (testhdf5_fortran
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
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET testhdf5_fortran
        APPEND PROPERTY COMPILE_DEFINITIONS
            HDF5CPP_USEDLL
    )
    SET_TARGET_PROPERTIES (testhdf5_fortran PROPERTIES OUTPUT_NAME "testhdf5_fortran")
    SET_TARGET_PROPERTIES (testhdf5_fortran PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (testhdf5_fortran ${HDF5_F90_TEST_LIB_TARGET})
IF (MSVC)
  TARGET_LINK_LIBRARIES (testhdf5_fortran "ws2_32.lib")
ENDIF (MSVC)
GET_TARGET_PROPERTY (testhdf5_fortranpath testhdf5_fortran LOCATION)
GET_FILENAME_COMPONENT (testhdf5_fortranexe ${testhdf5_fortranpath} NAME)
ADD_TEST (testhdf5_fortran ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${testhdf5_fortranexe})

#-- Adding test for testhdf5_fortran_1_8
ADD_EXECUTABLE (testhdf5_fortran_1_8
    fortranlib_test_1_8.f90
    tH5F.f90
    tH5O.f90
    tH5A_1_8.f90
    tH5G_1_8.f90
)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET testhdf5_fortran_1_8
        APPEND PROPERTY COMPILE_DEFINITIONS
            HDF5CPP_USEDLL
    )
    SET_TARGET_PROPERTIES (testhdf5_fortran_1_8 PROPERTIES OUTPUT_NAME "testhdf5_fortran_1_8")
    SET_TARGET_PROPERTIES (testhdf5_fortran_1_8 PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (testhdf5_fortran_1_8 ${HDF5_F90_TEST_LIB_TARGET})
IF (MSVC)
  TARGET_LINK_LIBRARIES (testhdf5_fortran_1_8 "ws2_32.lib")
ENDIF (MSVC)
GET_TARGET_PROPERTY (testhdf5_fortran_1_8path testhdf5_fortran_1_8 LOCATION)
GET_FILENAME_COMPONENT (testhdf5_fortran_1_8exe ${testhdf5_fortran_1_8path} NAME)
ADD_TEST (testhdf5_fortran_1_8 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${testhdf5_fortran_1_8exe})

#-- Adding test for fflush1
ADD_EXECUTABLE (fflush1 fflush1.f90)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET fflush1
        APPEND PROPERTY COMPILE_DEFINITIONS
            HDF5CPP_USEDLL
    )
    SET_TARGET_PROPERTIES (fflush1 PROPERTIES OUTPUT_NAME "fflush1")
    SET_TARGET_PROPERTIES (fflush1 PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (fflush1 ${HDF5_F90_TEST_LIB_TARGET})
IF (MSVC)
  TARGET_LINK_LIBRARIES (fflush1 "ws2_32.lib")
ENDIF (MSVC)
GET_TARGET_PROPERTY (fflush1path fflush1 LOCATION)
GET_FILENAME_COMPONENT (fflush1exe ${fflush1path} NAME)
ADD_TEST (fflush1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${fflush1exe})

#-- Adding test for fflush2
ADD_EXECUTABLE (fflush2 fflush2.f90)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET fflush2
        APPEND PROPERTY COMPILE_DEFINITIONS
            HDF5CPP_USEDLL
    )
    SET_TARGET_PROPERTIES (fflush2 PROPERTIES OUTPUT_NAME "fflush2")
    SET_TARGET_PROPERTIES (fflush2 PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (fflush2 ${HDF5_F90_TEST_LIB_TARGET})
IF (MSVC)
  TARGET_LINK_LIBRARIES (fflush2 "ws2_32.lib")
ENDIF (MSVC)
GET_TARGET_PROPERTY (fflush2path fflush2 LOCATION)
GET_FILENAME_COMPONENT (fflush2exe ${fflush2path} NAME)
ADD_TEST (fflush2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${fflush2exe})
