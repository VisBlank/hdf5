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
HDF5AtomException.java  \
HDF5AttributeException.java  \
HDF5BtreeException.java  \
HDF5DataFiltersException.java  \
HDF5DataStorageException.java  \
HDF5DatasetInterfaceException.java  \
HDF5DataspaceInterfaceException.java  \
HDF5DatatypeInterfaceException.java  \
HDF5Exception.java  \
HDF5ExternalFileListException.java  \
HDF5FileInterfaceException.java  \
HDF5FunctionArgumentException.java  \
HDF5FunctionEntryExitException.java  \
HDF5HeapException.java  \
HDF5InternalErrorException.java  \
HDF5JavaException.java  \
HDF5LibraryException.java  \
HDF5LowLevelIOException.java  \
HDF5MetaDataCacheException.java  \
HDF5ObjectHeaderException.java  \
HDF5PropertyListInterfaceException.java  \
HDF5ReferenceException.java  \
HDF5ResourceUnavailableException.java  \
HDF5SymbolTableException.java

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
