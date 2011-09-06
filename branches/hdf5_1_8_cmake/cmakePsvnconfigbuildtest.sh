cd pbuild
cmake -C ../config/cmake/cacheinit.cmake -G "Unix Makefiles" -DSYSTEM_NAME_EXT=Fedora_13_x64_PAR -DHDF5_BUILD_CPP_LIB:BOOL=OFF -DHDF5_ENABLE_PARALLEL:BOOL=ON -DHDF5_ALLOW_EXTERNAL_SUPPORT:STRING=SVN ..
ctest -D Experimental -A ../config/cmake/cacheinit.cmake
cd ..
