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

#include <jni.h>
/* Header for class hdf_hdf5lib_H5_H5G */

#ifndef _Included_hdf_hdf5lib_H5_H5G
#define _Included_hdf_hdf5lib_H5_H5G
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gcreate
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gcreate
  (JNIEnv*, jclass, jlong, jstring, jlong);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gopen
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gopen
  (JNIEnv*, jclass, jlong, jstring);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gclose
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5__1H5Gclose
  (JNIEnv*, jclass, jlong);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_obj_info_full
 * Signature: (JLjava/lang/String;[Ljava/lang/String;[I[I[J[JIII)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Gget_1obj_1info_1full
  (JNIEnv*, jclass, jlong, jstring, jobjectArray, jintArray, jintArray, jlongArray, jlongArray, jint, jint, jint);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_obj_info_max
 * Signature: (J[Ljava/lang/String;[I[I[JJI)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Gget_1obj_1info_1max
  (JNIEnv*, jclass, jlong, jobjectArray, jintArray, jintArray, jlongArray, jlong, jint);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gcreate2
 * Signature: (JLjava/lang/String;JJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gcreate2
  (JNIEnv*, jclass, jlong, jstring, jlong, jlong, jlong);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gcreate_anon
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gcreate_1anon
  (JNIEnv*, jclass, jlong, jlong, jlong);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gopen2
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gopen2
  (JNIEnv*, jclass, jlong, jstring, jlong);


/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_create_plist
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Gget_1create_1plist
(JNIEnv*, jclass, jlong);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_info
 * Signature: (J)Lhdf/hdf5lib/structs/H5G_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Gget_1info
  (JNIEnv*, jclass, jlong);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_info_by_name
 * Signature: (JLjava/lang/String;J)Lhdf/hdf5lib/structs/H5G_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Gget_1info_1by_1name
  (JNIEnv*, jclass, jlong, jstring, jlong);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_info_by_idx
 * Signature: (JLjava/lang/String;IIJJ)Lhdf/hdf5lib/structs/H5G_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Gget_1info_1by_1idx
  (JNIEnv*, jclass, jlong, jstring, jint, jint, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif
