cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_PERFORM )

#-----------------------------------------------------------------------------
# Setup output Directories
#-----------------------------------------------------------------------------
IF (NOT HDF5_EXTERNALLY_CONFIGURED)
  SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
  )
  SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
  )
  SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
  )
  SET (CMAKE_Fortran_MODULE_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin/fortran CACHE PATH "Single Directory for all fortran modules."
  )
ENDIF (NOT HDF5_EXTERNALLY_CONFIGURED)

INCLUDE_DIRECTORIES (${HDF5_TEST_SOURCE_DIR})
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib )

#-- Adding test for h5perf_serial
SET (h5perf_serial_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/sio_timer.c
    ${HDF5_PERFORM_SOURCE_DIR}/sio_perf.c
    ${HDF5_PERFORM_SOURCE_DIR}/sio_engine.c
)
ADD_EXECUTABLE (h5perf_serial ${h5perf_serial_SRCS})
TARGET_LINK_LIBRARIES (h5perf_serial ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
ADD_TEST (h5perf_serial ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/h5perf_serial)


#-- Adding test for chunk
SET (chunk_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/chunk.c
)
ADD_EXECUTABLE(chunk ${chunk_SRCS})
TARGET_LINK_LIBRARIES(chunk ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
ADD_TEST(chunk ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/chunk)


#-- Adding test for iopipe
SET (iopipe_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/iopipe.c
)
ADD_EXECUTABLE (iopipe ${iopipe_SRCS})
TARGET_LINK_LIBRARIES (iopipe ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
ADD_TEST (iopipe ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/iopipe)

#-- Adding test for overhead
SET (overhead_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/overhead.c
)
ADD_EXECUTABLE (overhead ${overhead_SRCS})
TARGET_LINK_LIBRARIES (overhead ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME})
ADD_TEST (overhead ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/overhead)

#-- Adding test for perf_meta
SET (perf_meta_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/perf_meta.c
)
ADD_EXECUTABLE (perf_meta ${perf_meta_SRCS})
TARGET_LINK_LIBRARIES (perf_meta ${HDF5_LIB_NAME} ${HDF5_TOOLS_LIB_NAME} ${HDF5_TEST_LIB_NAME})
ADD_TEST (perf_meta ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perf_meta)


#-- Adding test for zip_perf
SET (zip_perf_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/zip_perf.c
)
ADD_EXECUTABLE (zip_perf ${zip_perf_SRCS})
TARGET_LINK_LIBRARIES (zip_perf ${HDF5_TOOLS_LIB_NAME} ${HDF5_LIB_NAME})
ADD_TEST (zip_perf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zip_perf)


