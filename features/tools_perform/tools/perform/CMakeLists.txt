cmake_minimum_required (VERSION 2.8.10)
PROJECT (HDF5_TOOLS_PERF)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${HDF5_TOOLS_SRC_DIR}/lib)

# --------------------------------------------------------------------
# Add the misc executables
# --------------------------------------------------------------------
#-- Misc Executables
    ADD_EXECUTABLE (h5perf_gentest ${HDF5_TOOLS_PERF_SOURCE_DIR}/h5perf_gentest.c)
    TARGET_NAMING (h5perf_gentest ${LIB_TYPE})
    TARGET_LINK_LIBRARIES (h5perf_gentest ${HDF5_LIB_TARGET})
    SET_TARGET_PROPERTIES (h5perf_gentest PROPERTIES FOLDER generator/tools)

##############################################################################
##############################################################################
###           T E S T I N G                                                ###
##############################################################################
##############################################################################

##############################################################################
##############################################################################
###           T H E   T E S T S  M A C R O S                               ###
##############################################################################
##############################################################################

##############################################################################
##############################################################################
###           T H E   T E S T S                                            ###
##############################################################################
##############################################################################

  # Remove any output file left over from previous test run
  ADD_TEST (
      NAME h5perf-clear-objects
      COMMAND    ${CMAKE_COMMAND}
          -E remove 
          test_subgroups.h5
          test_attributes.h5
          test_datasets.h5
          test_compounds.h5
          test_subgroups.h5.out
          test_attributes.h5.out
          test_datasets.h5.out
          test_compounds.h5.out
  )

#  generate the test files
  ADD_TEST (NAME h5perf_gentest1 COMMAND $<TARGET_FILE:h5perf_gentest> -f test_subgroups.h5 -g 50000)
  SET_TESTS_PROPERTIES (h5perf_gentest1 PROPERTIES DEPENDS h5perf-clear-objects)
  ADD_TEST (NAME h5perf_gentest2 COMMAND $<TARGET_FILE:h5perf_gentest> -f test_attributes.h5 -a 10000)
  SET_TESTS_PROPERTIES (h5perf_gentest2 PROPERTIES DEPENDS h5perf-clear-objects)
  ADD_TEST (NAME h5perf_gentest3 COMMAND $<TARGET_FILE:h5perf_gentest> -f test_datasets.h5 -d 10000)
  SET_TESTS_PROPERTIES (h5perf_gentest3 PROPERTIES DEPENDS h5perf-clear-objects)
  ADD_TEST (NAME h5perf_gentest4 COMMAND $<TARGET_FILE:h5perf_gentest> -f test_compounds.h5 -r 10000)
  SET_TESTS_PROPERTIES (h5perf_gentest4 PROPERTIES DEPENDS h5perf-clear-objects)

# use h5dump --header on each test file
  ADD_TEST (NAME h5perf_h5dumph1 COMMAND $<TARGET_FILE:h5dump> --header test_subgroups.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dumph1 PROPERTIES DEPENDS h5perf_gentest1)
  ADD_TEST (NAME h5perf_h5dumph2 COMMAND $<TARGET_FILE:h5dump> --header test_attributes.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dumph2 PROPERTIES DEPENDS h5perf_gentest2)
  ADD_TEST (NAME h5perf_h5dumph3 COMMAND $<TARGET_FILE:h5dump> --header test_datasets.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dumph3 PROPERTIES DEPENDS h5perf_gentest3)
  ADD_TEST (NAME h5perf_h5dumph4 COMMAND $<TARGET_FILE:h5dump> --header test_compounds.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dumph4 PROPERTIES DEPENDS h5perf_gentest4)

# use h5dump on each test file
  ADD_TEST (NAME h5perf_h5dump1 COMMAND $<TARGET_FILE:h5dump> test_subgroups.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dump1 PROPERTIES DEPENDS h5perf_h5dumph1)
  ADD_TEST (NAME h5perf_h5dump2 COMMAND $<TARGET_FILE:h5dump> test_attributes.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dump2 PROPERTIES DEPENDS h5perf_h5dumph2)
  ADD_TEST (NAME h5perf_h5dump3 COMMAND $<TARGET_FILE:h5dump> test_datasets.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dump3 PROPERTIES DEPENDS h5perf_h5dumph3)
  ADD_TEST (NAME h5perf_h5dump4 COMMAND $<TARGET_FILE:h5dump> test_compounds.h5)
  SET_TESTS_PROPERTIES (h5perf_h5dump4 PROPERTIES DEPENDS h5perf_h5dumph4)

# use h5ls on each test file
  ADD_TEST (NAME h5perf_h5ls1 COMMAND $<TARGET_FILE:h5ls> -w80  test_subgroups.h5)
  SET_TESTS_PROPERTIES (h5perf_h5ls1 PROPERTIES DEPENDS h5perf_h5dump1)
  ADD_TEST (NAME h5perf_h5ls2 COMMAND $<TARGET_FILE:h5ls> -w80  test_attributes.h5)
  SET_TESTS_PROPERTIES (h5perf_h5ls2 PROPERTIES DEPENDS h5perf_h5dump2)
  ADD_TEST (NAME h5perf_h5ls3 COMMAND $<TARGET_FILE:h5ls> -w80  test_datasets.h5)
  SET_TESTS_PROPERTIES (h5perf_h5ls3 PROPERTIES DEPENDS h5perf_h5dump3)
  ADD_TEST (NAME h5perf_h5ls4 COMMAND $<TARGET_FILE:h5ls> -w80  test_compounds.h5)
  SET_TESTS_PROPERTIES (h5perf_h5ls4 PROPERTIES DEPENDS h5perf_h5dump4)

# use h5diff on each test file
  ADD_TEST (NAME h5perf_h5diff1 COMMAND $<TARGET_FILE:h5diff> -r test_subgroups.h5 test_subgroups.h5)
  SET_TESTS_PROPERTIES (h5perf_h5diff1 PROPERTIES DEPENDS h5perf_h5ls1)
  ADD_TEST (NAME h5perf_h5diff2 COMMAND $<TARGET_FILE:h5diff> -r test_attributes.h5 test_attributes.h5)
  SET_TESTS_PROPERTIES (h5perf_h5diff2 PROPERTIES DEPENDS h5perf_h5ls2)
  ADD_TEST (NAME h5perf_h5diff3 COMMAND $<TARGET_FILE:h5diff> -r test_datasets.h5 test_datasets.h5)
  SET_TESTS_PROPERTIES (h5perf_h5diff3 PROPERTIES DEPENDS h5perf_h5ls3)
  ADD_TEST (NAME h5perf_h5diff4 COMMAND $<TARGET_FILE:h5diff> -r test_compounds.h5 test_compounds.h5)
  SET_TESTS_PROPERTIES (h5perf_h5diff4 PROPERTIES DEPENDS h5perf_h5ls4)

# use h5stat on each test file
  ADD_TEST (NAME h5perf_h5stat1 COMMAND $<TARGET_FILE:h5stat> -fFgGdDTAs test_subgroups.h5)
  SET_TESTS_PROPERTIES (h5perf_h5stat1 PROPERTIES DEPENDS h5perf_h5diff1)
  ADD_TEST (NAME h5perf_h5stat2 COMMAND $<TARGET_FILE:h5stat> -fFgGdDTAs test_attributes.h5)
  SET_TESTS_PROPERTIES (h5perf_h5stat2 PROPERTIES DEPENDS h5perf_h5diff2)
  ADD_TEST (NAME h5perf_h5stat3 COMMAND $<TARGET_FILE:h5stat> -fFgGdDTAs test_datasets.h5)
  SET_TESTS_PROPERTIES (h5perf_h5stat3 PROPERTIES DEPENDS h5perf_h5diff3)
  ADD_TEST (NAME h5perf_h5stat4 COMMAND $<TARGET_FILE:h5stat> -fFgGdDTAs test_compounds.h5)
  SET_TESTS_PROPERTIES (h5perf_h5stat4 PROPERTIES DEPENDS h5perf_h5diff4)

# use h5stat on each test file
  ADD_TEST (NAME h5perf_h5copy1 COMMAND $<TARGET_FILE:h5copy> -f allflags -i test_subgroups.h5 -o test_subgroups.h5.out -v -s / -d /COPY)
  SET_TESTS_PROPERTIES (h5perf_h5copy1 PROPERTIES DEPENDS h5perf_h5stat1)
  ADD_TEST (NAME h5perf_h5copy2 COMMAND $<TARGET_FILE:h5copy> -f allflags -i test_attributes.h5 -o test_attributes.h5.out -v -s / -d /COPY)
  SET_TESTS_PROPERTIES (h5perf_h5copy2 PROPERTIES DEPENDS h5perf_h5stat2)
  ADD_TEST (NAME h5perf_h5copy3 COMMAND $<TARGET_FILE:h5copy> -f allflags -i test_datasets.h5 -o test_datasets.h5.out -v -s / -d /COPY)
  SET_TESTS_PROPERTIES (h5perf_h5copy3 PROPERTIES DEPENDS h5perf_h5stat3)
  ADD_TEST (NAME h5perf_h5copy4 COMMAND $<TARGET_FILE:h5copy> -f allflags -i test_compounds.h5 -o test_compounds.h5.out -v -s / -d /COPY)
  SET_TESTS_PROPERTIES (h5perf_h5copy4 PROPERTIES DEPENDS h5perf_h5stat4)

#############################################################################
##############################################################################
###           I N S T A L L A T I O N                                      ###
##############################################################################
##############################################################################
