cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_FORTRAN_TESTS C CXX Fortran)

#-----------------------------------------------------------------------------
# Add Tests
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_SOURCE_DIR})
LINK_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_Fortran_MODULE_DIRECTORY})

#-- Adding test for hl_f90_tstlite
ADD_EXECUTABLE (hl_f90_tstlite tstlite.f90)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
      IF (MSVC)
        SET_TARGET_PROPERTIES (hl_f90_tstlite
            PROPERTIES
                COMPILE_FLAGS "/dll"
                LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        ) 
      ENDIF (MSVC)
    SET_TARGET_PROPERTIES (hl_f90_tstlite PROPERTIES OUTPUT_NAME "hl_f90_tstlite")
    SET_TARGET_PROPERTIES (hl_f90_tstlite PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (hl_f90_tstlite ${HDF5_HL_F90_LIB_TARGET} ${HDF5_F90_LIB_TARGET})
  IF (WIN32)
    SET_TARGET_PROPERTIES (hl_f90_tstlite PROPERTIES LINKER_LANGUAGE Fortran)
  ENDIF (WIN32)

ADD_TEST (NAME hl_f90_tstlite COMMAND $<TARGET_FILE:hl_f90_tstlite>)

#-- Adding test for hl_f90_tstimage
ADD_EXECUTABLE (hl_f90_tstimage tstimage.f90)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
      IF (MSVC)
        SET_TARGET_PROPERTIES (hl_f90_tstimage
            PROPERTIES
                COMPILE_FLAGS "/dll"
                LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        ) 
      ENDIF (MSVC)
    SET_TARGET_PROPERTIES (hl_f90_tstimage PROPERTIES OUTPUT_NAME "hl_f90_tstimage")
    SET_TARGET_PROPERTIES (hl_f90_tstimage PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (hl_f90_tstimage  ${HDF5_HL_F90_LIB_TARGET})
  IF (WIN32)
    SET_TARGET_PROPERTIES (hl_f90_tstimage PROPERTIES LINKER_LANGUAGE Fortran)
  ENDIF (WIN32)

ADD_TEST (NAME hl_f90_tstimage COMMAND $<TARGET_FILE:hl_f90_tstimage>)

#-- Adding test for hl_f90_tsttable
ADD_EXECUTABLE (hl_f90_tsttable tsttable.f90)
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
      IF (MSVC)
        SET_TARGET_PROPERTIES (hl_f90_tsttable
            PROPERTIES
                COMPILE_FLAGS "/dll"
                LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        ) 
      ENDIF (MSVC)
    SET_TARGET_PROPERTIES (hl_f90_tsttable PROPERTIES OUTPUT_NAME "hl_f90_tsttable")
    SET_TARGET_PROPERTIES (hl_f90_tsttable PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (hl_f90_tsttable ${HDF5_HL_F90_LIB_TARGET})
  IF (WIN32)
    SET_TARGET_PROPERTIES (hl_f90_tsttable PROPERTIES LINKER_LANGUAGE Fortran)
  ENDIF (WIN32)

ADD_TEST (NAME hl_f90_tsttable COMMAND $<TARGET_FILE:hl_f90_tsttable>)
