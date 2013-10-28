cmake_minimum_required (VERSION 2.8.10)
PROJECT (HDF5_OBJC)

#-----------------------------------------------------------------------------
# Apply Definitions to compiler in this directory and below
#-----------------------------------------------------------------------------
ADD_DEFINITIONS (${HDF5_EXTRA_C_FLAGS})

#-----------------------------------------------------------------------------
# Shared/Static Libs
# Are there shared/static restrictions in Objective-C? - DER
#-----------------------------------------------------------------------------
IF (BUILD_SHARED_LIBS)
  SET (OBJC_BUILT_AS_DYNAMIC_LIB 1)
ELSE (BUILD_SHARED_LIBS)
  SET (OBJC_BUILT_AS_STATIC_LIB 1)
ENDIF (BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
# Generate configure file
# FIXME - DER
#-----------------------------------------------------------------------------
CONFIGURE_FILE (${HDF5_RESOURCES_DIR}/H5cxx_config.h.in
                ${HDF5_BINARY_DIR}/H5cxx_pubconf.h
)

#-----------------------------------------------------------------------------
# Setup Include directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_OBJC_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES (${HDF5_BINARY_DIR})

#-----------------------------------------------------------------------------
# Parallel/MPI, prevent spurious cpp/cxx warnings
# AS FAR AS I CAN TELL, WE NEED TO PREVENT BUILDING THE OBJECTIVE-C
# LIBRARIES WHEN PARALLEL IS TURNED ON. - DER
#-----------------------------------------------------------------------------
IF (H5_HAVE_PARALLEL)
  ADD_DEFINITIONS ("-DMPICH_SKIP_MPICXX")
  ADD_DEFINITIONS ("-DMPICH_IGNORE_CXX_SEEK")
ENDIF (H5_HAVE_PARALLEL)

# Is this a parellel thing only? - DER
ADD_SUBDIRECTORY (${HDF5_OBJC_SOURCE_DIR}/src ${HDF5_OBJC_BINARY_DIR}/src)

#-----------------------------------------------------------------------------
# Build the Objective-C examples
#-----------------------------------------------------------------------------
IF (HDF5_BUILD_EXAMPLES)
  ADD_SUBDIRECTORY (${HDF5_OBJC_SOURCE_DIR}/examples ${HDF5_OBJC_BINARY_DIR}/examples)
ENDIF (HDF5_BUILD_EXAMPLES)

#-----------------------------------------------------------------------------
# Build the Objective-C unit tests
#-----------------------------------------------------------------------------
IF (BUILD_TESTING)
  ADD_SUBDIRECTORY (${HDF5_OBJC_SOURCE_DIR}/test ${HDF5_OBJC_BINARY_DIR}/test)
ENDIF (BUILD_TESTING)
