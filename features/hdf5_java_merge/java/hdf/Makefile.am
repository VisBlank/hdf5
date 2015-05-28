#/****************************************************************************
#* Copyright by The HDF Group.                                               *
#* Copyright by the Board of Trustees of the University of Illinois.         *
#* All rights reserved.                                                      *
#*                                                                           *
#* This file is part of HDF Java Products. The full HDF Java copyright       *
#* notice, including terms governing use, modification, and redistribution,  *
#* is contained in the file, java/COPYING. You can also access it online at  *
#* http://www.hdfgroup.org/products/licenses.html.  If you do not have       *
#* access to the file, you may request a copy from help@hdfgroup.org.        *
#****************************************************************************/
## Makefile.am
## Run automake to generate a Makefile.in from this file.
#
# HDF5 Java native interface (JNI) Library Makefile(.in)

include $(top_srcdir)/config/commence.am

## Only recurse into subdirectories if the Java (JNI) interface is enabled.
if BUILD_JNI_CONDITIONAL
   SUBDIRS=hdf5lib
endif
DIST_SUBDIRS = hdf5lib

install-examples uninstall-examples: ;

installcheck-local: ;

# Clean examples when check-clean is invoked
check-clean :: ;

include $(top_srcdir)/config/conclude.am
