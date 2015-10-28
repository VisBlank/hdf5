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

JAVAROOT = ..

pkgpath = test
CLASSPATH_ENV=CLASSPATH=.:$(JAVAROOT):$(top_srcdir)/java/lib/hdf5_java.jar:$(top_srcdir)/java/lib/junit.jar:$(top_srcdir)/java/lib/hamcrest-core.jar:$(top_srcdir)/java/lib/slf4j-api-1.7.5.jar:$(top_srcdir)/java/lib/ext/slf4j-simple-1.7.5.jar:$$CLASSPATH

jarfile = jar$(PACKAGE_TARNAME)test.jar
TestAlldir = $(datadir)/java

AM_JAVACFLAGS = $(H5_JAVACFLAGS) -deprecation

TESTPACKAGE = 

TestAll_JAVA = \
	${TESTPACKAGE}TestH5.java \
    ${TESTPACKAGE}TestH5E.java \
    ${TESTPACKAGE}TestH5Edefault.java \
    ${TESTPACKAGE}TestH5Eregister.java \
    ${TESTPACKAGE}TestH5Fparams.java \
    ${TESTPACKAGE}TestH5Fbasic.java \
    ${TESTPACKAGE}TestH5F.java \
    ${TESTPACKAGE}TestH5Gbasic.java \
    ${TESTPACKAGE}TestH5G.java \
    ${TESTPACKAGE}TestH5Giterate.java \
    ${TESTPACKAGE}TestH5Sbasic.java \
    ${TESTPACKAGE}TestH5S.java \
    ${TESTPACKAGE}TestH5Tparams.java \
    ${TESTPACKAGE}TestH5Tbasic.java \
    ${TESTPACKAGE}TestH5T.java \
    ${TESTPACKAGE}TestH5Dparams.java \
    ${TESTPACKAGE}TestH5D.java \
    ${TESTPACKAGE}TestH5Dplist.java \
    ${TESTPACKAGE}TestH5Lparams.java \
    ${TESTPACKAGE}TestH5Lbasic.java \
    ${TESTPACKAGE}TestH5Lcreate.java \
    ${TESTPACKAGE}TestH5R.java \
    ${TESTPACKAGE}TestH5P.java \
    ${TESTPACKAGE}TestH5PData.java \
    ${TESTPACKAGE}TestH5Pfapl.java \
    ${TESTPACKAGE}TestH5A.java \
    ${TESTPACKAGE}TestH5Oparams.java \
    ${TESTPACKAGE}TestH5Obasic.java \
    ${TESTPACKAGE}TestH5Ocreate.java \
    ${TESTPACKAGE}TestH5Ocopy.java \
    ${TESTPACKAGE}TestH5Z.java \
    ${TESTPACKAGE}TestAll.java

$(jarfile): classTestAll.stamp
	$(JAR) cf $(JARFLAGS) $@ $(JAVAROOT)/$(pkgpath)/*.class

TestAll_DATA = $(jarfile)

check_SCRIPTS = junit.sh
TEST_SCRIPT = $(check_SCRIPTS)

CLEANFILES = classTestAll.stamp $(jarfile) $(TESTPACKAGE)/*.class junit.sh

#JAVA_JUNIT  = $(JAVA_SRCS)
#noinst_JAVA = @JAVA_JUNIT@
#EXTRA_JAVA  = $(JAVA_JUNIT)
TESTS_JUNIT = AllJunitTests
#EXTRA_TEST  = $(TESTS_JUNIT)

AllJunitTests :
	echo "#! /bin/sh" > $@
	echo "exec @JUNIT@ test.TestAll" >> $@
	chmod +x $@

clean:
	rm -rf ${TESTPACKAGE}*.class
	rm -f $(jarfile)
	rm -f classTestAll.stamp

include $(top_srcdir)/config/conclude.am
