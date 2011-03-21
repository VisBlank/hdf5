HDF5 Windows external libraries README

Background:

Building HDF5 from source requires two libraries that are not destributed
with the HDF5 source package: szip and zlib.  Szip is a somewhat
obscure compression library that works well will floating point data but
has non-open licensing in some circumstances (read the szip docs for more
info).

This document will walk you through setting up to build these libraries.

NOTE:  It is *highly* advisable that you build zlib and szip from source
when you build HDF5.  This is to be absolutely sure that all your libraries
are using the same C runtime (CRT).  Using different C runtimes (even
debug vs. release!) can cause hard-to-debug memory/resource issues.  See
the CRT document in the HDF5 solution directory for more information.


********
* Zlib *
********

Though the zlib is widely available for Windows, the existing binary packages
either use the WINAPI calling convention (we need CDECL) and/or do not
supply all the necessary libraries (32- and 64-bit, static and dynamic,
debug and release).

There are two project files: one to build the static library and one to build
the dynamic library.  Either can be switched between debug/release and 32/64
bit.  The Visual Studio configuration manager should do this automatically
when you pick your HDF5 platform and configuration.

Since the zlib source code is not included in the HDF5 distribution, you'll
have to go get it.

	http://www.zlib.net/

Just download the source tarball and extract the contents into the zlib_src
directory.  This should give you folders like ...external\zlib\src\amiga\, etc.
If you do this correctly, all the projects should find what they need
without any further work on your part.  If they can't find the files, it's
probably because you have an extra directory from the extraction.  e.g.
...external\zlib\src\zlib-1.2.5\amiga\, etc.

The output files include the bitness in the name, which I'm sure will
drive people nuts but it helps me to avoid boneheaded linking errors.

Notes:

- Only the library gets built - no testing, etc.
- Everything compiled as C code
- Warning 4996 (deprecated C functions) turned off
- No Itanium support
- No def file ordinals
- No assembly language is used (maybe in the future)



********
* SZip *
********

Download the source to this library from our website

http://www.hdfgroup.org/ftp/lib-external/szip/2.1/src/

Sadly, this is only available as gzipped tarball right now, so you'll
need 7zip or similar software to unzip it.  Extract the contents into the
szip src folder and then dig the SZconfig.h file out of the szipproj.zip
file in the windows folder.  Copy it in with the rest of the SZip source
code.

As with the zlib projects, there are independent static and dynamic
library projects.  The output files are named similarly.

There is no def file for the szip dll, btw.  Szip uses __declspec(dllexport)
in the API definition.

Notes:

- Only the library gets built - no testing, etc.
- Everything compiled as C code
- Warning 4996 (deprecated C functions) turned off
- You will get a LOT of conversion warnings (4244, etc.) when you build szip
