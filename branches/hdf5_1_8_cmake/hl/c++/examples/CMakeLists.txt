cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_HL_CPP_EXAMPLES)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_HL_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES (${HDF5_HL_CPP_SOURCE_DIR}/src)

# --------------------------------------------------------------------
#  Add in the examples for the Packet Table codes
# --------------------------------------------------------------------
IF (HDF5_BUILD_EXAMPLES)
  ADD_EXECUTABLE (ptExampleFL ${HDF5_HL_CPP_EXAMPLES_SOURCE_DIR}/ptExampleFL.cpp)
  TARGET_LINK_LIBRARIES (
      ptExampleFL
      ${HDF5_HL_CPP_LIB_TARGET}
      ${HDF5_HL_LIB_TARGET}
      ${HDF5_CPP_LIB_TARGET}
      ${HDF5_LIB_TARGET}
  )

  ADD_EXECUTABLE (ptExampleVL ${HDF5_HL_CPP_EXAMPLES_SOURCE_DIR}/ptExampleVL.cpp)
  TARGET_LINK_LIBRARIES (
      ptExampleVL
      ${HDF5_HL_CPP_LIB_TARGET}
      ${HDF5_HL_LIB_TARGET}
      ${HDF5_CPP_LIB_TARGET}
      ${HDF5_LIB_TARGET}
  )
ENDIF (HDF5_BUILD_EXAMPLES)
