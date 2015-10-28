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
JAVA_API=yes

JAVAROOT = ../..

pkgpath = examples/datatypes
CLASSPATH_ENV=CLASSPATH=.:$(JAVAROOT):$(top_srcdir)/java/lib/hdf5_java.jar:$(top_srcdir)/java/lib/slf4j-api-1.7.5.jar:$(top_srcdir)/java/lib/ext/slf4j-simple-1.7.5.jar:$$CLASSPATH

jarfile = jar$(PACKAGE_TARNAME)datatypes.jar
TestAlldir = $(datadir)/java

AM_JAVACFLAGS = $(H5_JAVACFLAGS) -deprecation

TESTPACKAGE = 

TestAll_JAVA = \
    H5Ex_T_Array.java \
    H5Ex_T_ArrayAttribute.java \
    H5Ex_T_Bit.java \
    H5Ex_T_BitAttribute.java \
    H5Ex_T_Commit.java \
    H5Ex_T_Compound.java \
    H5Ex_T_CompoundAttribute.java \
    H5Ex_T_Float.java \
    H5Ex_T_FloatAttribute.java \
    H5Ex_T_Integer.java \
    H5Ex_T_IntegerAttribute.java \
    H5Ex_T_ObjectReference.java \
    H5Ex_T_ObjectReferenceAttribute.java \
    H5Ex_T_Opaque.java \
    H5Ex_T_OpaqueAttribute.java \
    H5Ex_T_String.java \
    H5Ex_T_StringAttribute.java \
    H5Ex_T_VLString.java


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
