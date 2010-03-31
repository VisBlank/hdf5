cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_FORTRAN_TESTS C CXX Fortran)

#-----------------------------------------------------------------------------
# Add Tests
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_SOURCE_DIR})
LINK_DIRECTORIES (${HDF5_SRC_DIR} ${HDF5_F90_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_Fortran_MODULE_DIRECTORY})

#-- Adding test for hl_f90_tstlite
ADD_EXECUTABLE (hl_f90_tstlite tstlite.f90)
TARGET_LINK_LIBRARIES (hl_f90_tstlite ${HDF5_HL_F90_LIB_TARGET} ${HDF5_F90_LIB_TARGET})
ADD_TEST (hl_f90_tstlite ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hl_f90_tstlite)

#-- Adding test for hl_f90_tstimage
ADD_EXECUTABLE (hl_f90_tstimage tstimage.f90)
TARGET_LINK_LIBRARIES (hl_f90_tstimage  ${HDF5_HL_F90_LIB_TARGET})
ADD_TEST (hl_f90_tstimage ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hl_f90_tstimage)

#-- Adding test for hl_f90_tsttable
ADD_EXECUTABLE (hl_f90_tsttable tsttable.f90)
TARGET_LINK_LIBRARIES (hl_f90_tsttable ${HDF5_HL_F90_LIB_TARGET})
ADD_TEST (hl_f90_tsttable ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hl_f90_tsttable)
