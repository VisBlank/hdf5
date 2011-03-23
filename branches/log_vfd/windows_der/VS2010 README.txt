In Visual Studio 2010, static linakage rules have changed a little so you'll
have to make a simple change to the HDF5 static library.

1) Open up the static HDF5 library (c_static) properties.

2) Go to "Common Properties"->"Framework and References".

3) For both szip_static and zlib_static, change "Link Library Dependencies"
   to true.


That should fix the missing symbol errors.