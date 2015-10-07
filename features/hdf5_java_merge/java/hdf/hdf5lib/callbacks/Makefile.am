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

JAVAC           = @JAVAC@
JAR         	= @JAR@

CLASSPATH=@CLASSPATH@:$(top_srcdir)/java/lib/slf4j-api-1.7.5.jar

JAVA_SRCS = \
Callbacks.java \
H5L_iterate_cb.java \
H5L_iterate_t.java \
H5O_iterate_cb.java \
H5O_iterate_t.java \
H5D_iterate_cb.java \
H5D_iterate_t.java

# Mark this directory as part of the JNI API
JNI_API=yes

CLASS_DIR=$(TOP)/lib
CLP="$(CLASSPATH):$(CLASS_DIR):. "
JAR_DIR=$(CLASS_DIR)
OUT_DIR=$(CLASS_DIR)/$(DIR)

JFLAGS= -g -deprecation -d $(CLASS_DIR)	-classpath $(CLP)
JARFLAGS = cfm

JC	= $(JAVAC) $(JFLAGS)

# .class.java rule, add file to list
$(OUT_DIR)/%.class : %.java
	$(JC) $@

include $(top_srcdir)/config/conclude.am
