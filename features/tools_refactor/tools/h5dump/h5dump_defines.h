/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef H5DUMP_DEFINES_H__
#define H5DUMP_DEFINES_H__

#define H5DUMP_MAX_RANK     H5S_MAX_RANK

#define ATTRIBUTE_DATA  0
#define DATASET_DATA    1
#define ENUM_DATA       2
#define COL             3

/* Strings for output */
#define ATTRIBUTE       "ATTRIBUTE"
#define BLOCK           "BLOCK"
#define SUPER_BLOCK     "SUPER_BLOCK"
#define COMPRESSION     "COMPRESSION"
#define CONCATENATOR    "//"
#define COMPLEX         "COMPLEX"
#define COUNT           "COUNT"
#define CSET            "CSET"
#define CTYPE           "CTYPE"
#define DATA            "DATA"
#define DATASPACE       "DATASPACE"
#define EXTERNAL        "EXTERNAL"
#define FILENO          "FILENO"
#define HARDLINK        "HARDLINK"
#define NLINK           "NLINK"
#define OBJID           "OBJECTID"
#define OBJNO           "OBJNO"
#define S_SCALAR        "SCALAR"
#define S_SIMPLE        "SIMPLE"
#define S_NULL          "NULL"
#define SOFTLINK        "SOFTLINK"
#define EXTLINK         "EXTERNAL_LINK"
#define UDLINK          "USERDEFINED_LINK"
#define START           "START"
#define STRIDE          "STRIDE"
#define STRSIZE         "STRSIZE"
#define STRPAD          "STRPAD"
#define SUBSET          "SUBSET"
#define FILTERS         "FILTERS"
#define DEFLATE         "COMPRESSION DEFLATE"
#define DEFLATE_LEVEL   "LEVEL"
#define SHUFFLE         "PREPROCESSING SHUFFLE"
#define FLETCHER32      "CHECKSUM FLETCHER32"
#define SZIP            "COMPRESSION SZIP"
#define NBIT            "COMPRESSION NBIT"
#define SCALEOFFSET            "COMPRESSION SCALEOFFSET"
#define SCALEOFFSET_MINBIT            "MIN BITS"
#define STORAGE_LAYOUT  "STORAGE_LAYOUT"
#define CONTIGUOUS      "CONTIGUOUS"
#define COMPACT         "COMPACT"
#define CHUNKED         "CHUNKED"
#define EXTERNAL_FILE   "EXTERNAL_FILE"
#define FILLVALUE       "FILLVALUE"
#define FILE_CONTENTS   "FILE_CONTENTS"
#define PACKED_BITS     "PACKED_BITS"
#define PACKED_OFFSET   "OFFSET"
#define PACKED_LENGTH   "LENGTH"

#define BEGIN           "{"
#define END             "}"

/* Macros for displaying objects */
#define begin_obj(obj,name,begin)                               \
    do {              \
        if (name)                                               \
            HDfprintf(stdout, "%s \"%s\" %s", (obj), (name), (begin));   \
        else                                                    \
            HDfprintf(stdout, "%s %s", (obj), (begin));      \
    } while(0);

#define end_obj(obj,end)                                        \
    do {              \
        if(HDstrlen(end)) {                                     \
            HDfprintf(stdout, "%s", end);                                  \
            if(HDstrlen(obj))                                   \
                HDfprintf(stdout, " ");                                    \
        }                                                       \
        if(HDstrlen(obj))                                       \
            HDfprintf(stdout, "%s", obj);                                  \
    } while(0);


/* 3 private values: can't be set, but can be read.
   Note: these are defined in H5Zprivate, they are
   duplicated here.
 */
#define H5_SZIP_LSB_OPTION_MASK         8
#define H5_SZIP_MSB_OPTION_MASK         16
#define H5_SZIP_RAW_OPTION_MASK         128

#endif  /* !H5DUMP_DEFINES_H__ */
