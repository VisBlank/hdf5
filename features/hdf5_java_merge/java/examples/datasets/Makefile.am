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

# Mark this directory as part of the JNI API
JNI_API=yes

JAVAROOT = ../..

pkgpath = examples/datasets
CLASSPATH_ENV=CLASSPATH=.:$(JAVAROOT):$(top_srcdir)/java/lib/jhdf5.jar:$(top_srcdir)/java/lib/slf4j-api-1.7.5.jar:$(top_srcdir)/java/lib/slf4j-simple-1.7.5.jar:$$CLASSPATH

jarfile = jar$(PACKAGE_TARNAME)datasets.jar
TestAlldir = $(datadir)/java

AM_JAVACFLAGS = -g -deprecation

TESTPACKAGE = 

TestAll_JAVA = \
	H5Ex_D_Alloc.java \
    H5Ex_D_Checksum.java \
    H5Ex_D_Chunk.java \
    H5Ex_D_Compact.java \
    H5Ex_D_External.java \
    H5Ex_D_FillValue.java \
    H5Ex_D_Gzip.java \
    H5Ex_D_Hyperslab.java \
    H5Ex_D_ReadWrite.java \
    H5Ex_D_Shuffle.java \
    H5Ex_D_Szip.java \
    H5Ex_D_UnlimitedAdd.java \
    H5Ex_D_UnlimitedGzip.java \
    H5Ex_D_UnlimitedMod.java \
    H5Ex_D_Nbit.java \
    H5Ex_D_Transform.java \
    H5Ex_D_Sofloat.java \
    H5Ex_D_Soint.java


$(jarfile): classTestAll.stamp
	$(JAR) cf $(JARFLAGS) $@ $(JAVAROOT)/$(pkgpath)/*.class

TestAll_DATA = $(jarfile)

check_SCRIPTS = runExample.sh
TEST_SCRIPT = $(check_SCRIPTS)

CLEANFILES = classTestAll.stamp $(jarfile) $(TESTPACKAGE)/*.class runExample.sh

clean:
	rm -rf *.class
	rm -f $(jarfile)
	rm -f classTestAll.stamp

include $(top_srcdir)/config/conclude.am
