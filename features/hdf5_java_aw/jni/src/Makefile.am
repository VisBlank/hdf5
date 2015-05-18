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
#

include $(top_srcdir)/config/commence.am
include $(top_srcdir)/config/lt_vers.am

# Include src directory and JNI flags
AM_CFLAGS+=-I$(top_srcdir)/src $(JNIFLAGS)

# This is our main target
lib_LTLIBRARIES=libjhdf5.la

# Add libtool numbers to the HDF5 Java (JNI) library (from config/lt_vers.am)
libjhdf5_la_LDFLAGS= -version-info $(LT_VERS_INTERFACE):$(LT_VERS_REVISION):$(LT_VERS_AGE) $(AM_LDFLAGS)

# Source files for the library
libjhdf5_la_SOURCES=exceptionImp.c h5Constants.c nativeData.c h5util.c h5Imp.c \
	h5aImp.c h5dImp.c h5eImp.c h5fImp.c h5gImp.c h5iImp.c h5lImp.c h5oImp.c    \
	h5pImp.c h5rImp.c h5sImp.c h5tImp.c h5zImp.c

# HDF5 Java (JNI) library depends on HDF5 Library.
libjhdf5_la_LIBADD=$(LIBHDF5)

# Mark this directory as part of the JNI API
JNI_API=yes

include $(top_srcdir)/config/conclude.am
