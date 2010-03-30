## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
SET (CTEST_PROJECT_NAME "HDF5-1.8")
SET (CTEST_NIGHTLY_START_TIME "00:00:00 EST")

SET (CTEST_DROP_METHOD "http")
SET (CTEST_DROP_SITE "cdash.hdfgroup.org")
SET (CTEST_DROP_LOCATION "/submit.php?project=HDF5-1.8")
SET (CTEST_DROP_SITE_CDASH TRUE)
