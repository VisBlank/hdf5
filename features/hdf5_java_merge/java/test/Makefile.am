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
include $(top_srcdir)/config/lt_vers.am

JAVAC           = @JAVAC@
JAR         	= @JAR@

CLASSPATH=@CLASSPATH@:$(top_srcdir)/java/lib/jhdf5.jar:$(top_srcdir)/java/lib/junit.jar:$(top_srcdir)/java/lib/hamcrest-core.jar:$(top_srcdir)/java/lib/slf4j-api-1.7.5.jar:$(top_srcdir)/java/lib/ext/slf4j-nop-1.7.5.jar

JAVA_SRCS = \
	TestH5.java \
    TestH5E.java \
    TestH5Edefault.java \
    TestH5Eregister.java \
    TestH5Fparams.java \
    TestH5Fbasic.java \
    TestH5F.java \
    TestH5Gbasic.java \
    TestH5G.java \
    TestH5Giterate.java \
    TestH5Sbasic.java \
    TestH5S.java \
    TestH5Tparams.java \
    TestH5Tbasic.java \
    TestH5T.java \
    TestH5Dparams.java \
    TestH5D.java \
    TestH5Dplist.java \
    TestH5Lparams.java \
    TestH5Lbasic.java \
    TestH5Lcreate.java \
    TestH5R.java \
    TestH5P.java \
    TestH5PData.java \
    TestH5Pfapl.java \
    TestH5A.java \
    TestH5Oparams.java \
    TestH5Obasic.java \
    TestH5Ocreate.java \
    TestH5Ocopy.java \
    TestH5Z.java \
    TestAll.java


CLASS_DIR=$(TOP)/lib
CLP="$(CLASSPATH):$(CLASS_DIR):. "
JAR_DIR=$(CLASS_DIR)
OUT_DIR=$(CLASS_DIR)/$(DIR)

JFLAGS= -g -deprecation -d $(CLASS_DIR)	-classpath $(CLP)
JARFLAGS = cfm

JC	= $(JAVAC) $(JFLAGS)

JAVA_JUNIT  = $(JAVA_SRCS)
noinst_JAVA = @JAVA_JUNIT@
EXTRA_JAVA  = $(JAVA_JUNIT)
TESTS_JUNIT = AllJunitTests
EXTRA_TEST  = $(TESTS_JUNIT)

AllJunitTests :
	echo "#! /bin/sh" > $@
	echo "exec @JUNIT@ test.TestAll" >> $@
	chmod +x $@

# .class.java rule, add file to list
$(OUT_DIR)/%.class : %.java
	$(JC) $@

include $(top_srcdir)/config/conclude.am
