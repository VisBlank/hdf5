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

JAVAROOT = ..

jarfile = jar$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).jar
jhdf5dir = $(datadir)/java

pkgpath = hdf/hdf5lib
CLASSPATH_ENV=CLASSPATH=.:$(JAVAROOT):$(top_srcdir)/java/lib/slf4j-api-1.7.5.jar:$$CLASSPATH

AM_JAVACFLAGS = -g -deprecation

HDFPACKAGE = hdf5lib/

jhdf5_JAVA =  \
	${HDFPACKAGE}callbacks/Callbacks.java \
	${HDFPACKAGE}callbacks/H5L_iterate_cb.java \
	${HDFPACKAGE}callbacks/H5L_iterate_t.java \
	${HDFPACKAGE}callbacks/H5O_iterate_cb.java \
	${HDFPACKAGE}callbacks/H5O_iterate_t.java \
	${HDFPACKAGE}callbacks/H5D_iterate_cb.java \
	${HDFPACKAGE}callbacks/H5D_iterate_t.java \
	${HDFPACKAGE}exceptions/HDF5AtomException.java  \
	${HDFPACKAGE}exceptions/HDF5AttributeException.java  \
	${HDFPACKAGE}exceptions/HDF5BtreeException.java  \
	${HDFPACKAGE}exceptions/HDF5DataFiltersException.java  \
	${HDFPACKAGE}exceptions/HDF5DataStorageException.java  \
	${HDFPACKAGE}exceptions/HDF5DatasetInterfaceException.java  \
	${HDFPACKAGE}exceptions/HDF5DataspaceInterfaceException.java  \
	${HDFPACKAGE}exceptions/HDF5DatatypeInterfaceException.java  \
	${HDFPACKAGE}exceptions/HDF5Exception.java  \
	${HDFPACKAGE}exceptions/HDF5ExternalFileListException.java  \
	${HDFPACKAGE}exceptions/HDF5FileInterfaceException.java  \
	${HDFPACKAGE}exceptions/HDF5FunctionArgumentException.java  \
	${HDFPACKAGE}exceptions/HDF5FunctionEntryExitException.java  \
	${HDFPACKAGE}exceptions/HDF5HeapException.java  \
	${HDFPACKAGE}exceptions/HDF5InternalErrorException.java  \
	${HDFPACKAGE}exceptions/HDF5JavaException.java  \
	${HDFPACKAGE}exceptions/HDF5LibraryException.java  \
	${HDFPACKAGE}exceptions/HDF5LowLevelIOException.java  \
	${HDFPACKAGE}exceptions/HDF5MetaDataCacheException.java  \
	${HDFPACKAGE}exceptions/HDF5ObjectHeaderException.java  \
	${HDFPACKAGE}exceptions/HDF5PropertyListInterfaceException.java  \
	${HDFPACKAGE}exceptions/HDF5ReferenceException.java  \
	${HDFPACKAGE}exceptions/HDF5ResourceUnavailableException.java  \
	${HDFPACKAGE}exceptions/HDF5SymbolTableException.java \
	${HDFPACKAGE}structs/H5_ih_info_t.java \
	${HDFPACKAGE}structs/H5A_info_t.java \
	${HDFPACKAGE}structs/H5G_info_t.java \
	${HDFPACKAGE}structs/H5L_info_t.java \
	${HDFPACKAGE}structs/H5O_info_t.java \
	${HDFPACKAGE}structs/H5O_hdr_info_t.java \
	${HDFPACKAGE}structs/H5AC_cache_config_t.java \
	${HDFPACKAGE}H5.java  \
	${HDFPACKAGE}HDF5Constants.java  \
	${HDFPACKAGE}HDF5GroupInfo.java  \
	${HDFPACKAGE}HDFArray.java  \
	${HDFPACKAGE}HDFNativeData.java

$(jarfile): classjhdf5.stamp
	$(JAR) cf $(JARFLAGS) $@ $(JAVAROOT)/$(pkgpath)

jhdf5_DATA = $(jarfile)

CLEANFILES = classjhdf5.stamp $(jarfile) $(HDFPACKAGE)/callbacks/*.class $(HDFPACKAGE)/exceptions/*.class $(HDFPACKAGE)/structs/*.class $(HDFPACKAGE)/*.class

clean:
	rm -rf ${HDFPACKAGE}*.class
	rm -f $(jarfile)
	rm -f classjhdf5.stamp


# Clean examples when check-clean is invoked
check-clean :: ;

#include $(top_srcdir)/config/conclude.am
