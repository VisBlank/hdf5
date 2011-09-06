cd build
cmake -C ../config/cmake/cacheinit.cmake -G "Unix Makefiles" -DSYSTEM_NAME_EXT=Fedora_13_x64 -DHDF5_ALLOW_EXTERNAL_SUPPORT:STRING=SVN ..
ctest -D Experimental -A ../config/cmake/cacheinit.cmake
cd ..
