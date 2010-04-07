cmake_minimum_required (VERSION 2.8)
PROJECT (HDF5_PERFORM )

IF (NOT (WIN32 AND BUILD_SHARED_LIBS))

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TEST_SOURCE_DIR})
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SOURCE_DIR}/lib )

#-----------------------------------------------------------------------------
# Add Tests
#-----------------------------------------------------------------------------

#-- Adding test for h5perf_serial
SET (h5perf_serial_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/sio_timer.c
    ${HDF5_PERFORM_SOURCE_DIR}/sio_perf.c
    ${HDF5_PERFORM_SOURCE_DIR}/sio_engine.c
)
ADD_EXECUTABLE (h5perf_serial ${h5perf_serial_SRCS})
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET h5perf_serial 
        APPEND PROPERTY COMPILE_DEFINITIONS 
            _HDF5USEDLL_ 
			_HDF5TESTUSEDLL_
    )
    SET_TARGET_PROPERTIES (h5perf_serial PROPERTIES OUTPUT_NAME "h5perf_serial")
    SET_TARGET_PROPERTIES (h5perf_serial PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (h5perf_serial ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
ADD_TEST (h5perf_serial ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/h5perf_serial)

#-- Adding test for chunk
SET (chunk_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/chunk.c
)
ADD_EXECUTABLE(chunk ${chunk_SRCS})
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET chunk 
        APPEND PROPERTY COMPILE_DEFINITIONS 
            _HDF5USEDLL_ 
			_HDF5TESTUSEDLL_
    )
    SET_TARGET_PROPERTIES (chunk PROPERTIES OUTPUT_NAME "chunk")
    SET_TARGET_PROPERTIES (chunk PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES(chunk ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
ADD_TEST(chunk ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/chunk)


#-- Adding test for iopipe
SET (iopipe_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/iopipe.c
)
ADD_EXECUTABLE (iopipe ${iopipe_SRCS})
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET iopipe 
        APPEND PROPERTY COMPILE_DEFINITIONS 
            _HDF5USEDLL_ 
			_HDF5TESTUSEDLL_
    )
    SET_TARGET_PROPERTIES (iopipe PROPERTIES OUTPUT_NAME "iopipe")
    SET_TARGET_PROPERTIES (iopipe PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (iopipe ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
ADD_TEST (iopipe ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/iopipe)

#-- Adding test for overhead
SET (overhead_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/overhead.c
)
ADD_EXECUTABLE (overhead ${overhead_SRCS})
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET overhead 
        APPEND PROPERTY COMPILE_DEFINITIONS 
            _HDF5USEDLL_ 
			_HDF5TESTUSEDLL_
    )
    SET_TARGET_PROPERTIES (overhead PROPERTIES OUTPUT_NAME "overhead")
    SET_TARGET_PROPERTIES (overhead PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (overhead ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET})
ADD_TEST (overhead ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/overhead)

#-- Adding test for perf_meta
SET (perf_meta_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/perf_meta.c
)
ADD_EXECUTABLE (perf_meta ${perf_meta_SRCS})
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET perf_meta 
        APPEND PROPERTY COMPILE_DEFINITIONS 
            _HDF5USEDLL_ 
			_HDF5TESTUSEDLL_
    )
    SET_TARGET_PROPERTIES (perf_meta PROPERTIES OUTPUT_NAME "perf_meta")
    SET_TARGET_PROPERTIES (perf_meta PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (perf_meta ${HDF5_LIB_TARGET} ${HDF5_TOOLS_LIB_TARGET} ${HDF5_TEST_LIB_TARGET})
ADD_TEST (perf_meta ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perf_meta)

#-- Adding test for zip_perf
SET (zip_perf_SRCS
    ${HDF5_PERFORM_SOURCE_DIR}/zip_perf.c
)
ADD_EXECUTABLE (zip_perf ${zip_perf_SRCS})
IF (BUILD_SHARED_LIBS)
  IF (WIN32)
    SET_PROPERTY (TARGET zip_perf 
        APPEND PROPERTY COMPILE_DEFINITIONS 
            _HDF5USEDLL_ 
			_HDF5TESTUSEDLL_
    )
    SET_TARGET_PROPERTIES (zip_perf PROPERTIES OUTPUT_NAME "zip_perf")
    SET_TARGET_PROPERTIES (zip_perf PROPERTIES PREFIX "dll")
  ENDIF (WIN32)
ENDIF (BUILD_SHARED_LIBS)
TARGET_LINK_LIBRARIES (zip_perf ${HDF5_TOOLS_LIB_TARGET} ${HDF5_LIB_TARGET})
ADD_TEST (zip_perf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zip_perf)

ENDIF (NOT (WIN32 AND BUILD_SHARED_LIBS))
