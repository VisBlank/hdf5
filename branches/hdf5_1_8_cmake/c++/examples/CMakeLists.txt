project (HDF5_CPP_EXAMPLES)

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


ADD_EXECUTABLE( cpp_chunks ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/chunks.cpp)
TARGET_LINK_LIBRARIES( cpp_chunks ${HDF5_CPP_LIB_NAME} ${HDF5_LIB_NAME} )

ADD_EXECUTABLE( cpp_compound ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/compound.cpp)
TARGET_LINK_LIBRARIES( cpp_compound ${HDF5_CPP_LIB_NAME} ${HDF5_LIB_NAME} )

ADD_EXECUTABLE( cpp_create ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/create.cpp)
TARGET_LINK_LIBRARIES( cpp_create ${HDF5_CPP_LIB_NAME} ${HDF5_LIB_NAME} )

ADD_EXECUTABLE( cpp_extend_ds ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/extend_ds.cpp)
TARGET_LINK_LIBRARIES( cpp_extend_ds ${HDF5_CPP_LIB_NAME} ${HDF5_LIB_NAME} )

ADD_EXECUTABLE( cpp_h5group ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/h5group.cpp)
TARGET_LINK_LIBRARIES( cpp_h5group ${HDF5_CPP_LIB_NAME} ${HDF5_LIB_NAME} )

ADD_EXECUTABLE( cpp_readdata ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/readdata.cpp)
TARGET_LINK_LIBRARIES( cpp_readdata ${HDF5_CPP_LIB_NAME} ${HDF5_LIB_NAME} )

ADD_EXECUTABLE( cpp_writedata ${HDF5_CPP_EXAMPLES_SOURCE_DIR}/writedata.cpp)
TARGET_LINK_LIBRARIES( cpp_writedata ${HDF5_CPP_LIB_NAME} ${HDF5_LIB_NAME} )

