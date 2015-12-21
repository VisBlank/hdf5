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

/*
 *  For details of the HDF libraries, see the HDF Documentation at:
 *    http://hdfdfgroup.org/HDF5/doc/
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include <stdlib.h>
#include "hdf5.h"
#include "h5jni.h"
#include "h5oImp.h"

#ifdef __cplusplus
#define CBENVPTR (cbenv)
#define CBENVPAR
#define JVMPTR (jvm)
#define JVMPAR
#define JVMPAR2
#else
#define CBENVPTR (*cbenv)
#define CBENVPAR cbenv,
#define JVMPTR (*jvm)
#define JVMPAR jvm
#define JVMPAR2 jvm,
#endif

static herr_t H5O_iterate_cb(hid_t g_id, const char *name, const H5O_info_t *info, void *op_data);

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    _H5Oopen
     * Signature: (JLjava/lang/String;J)J
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Oopen
      (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong access_plist_id)
    {
        hid_t    status;
        const char *oName;
        jboolean isCopy;

        if (name == NULL) {
            h5nullArgument( env, "H5Oopen:  name is NULL");
            return -1;
        }

        oName = ENVPTR->GetStringUTFChars(ENVPAR name,&isCopy);

        if (oName == NULL) {
            h5JNIFatalError( env, "H5Oopen:  object name not pinned");
            return -1;
        }

        status = H5Oopen((hid_t)loc_id, oName, (hid_t)access_plist_id );

        ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);
        if (status < 0) {
            h5libraryError(env);
        }
        return (jlong)status;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    _H5Oclose
     * Signature: (J)I
     */
    JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5__1H5Oclose
      (JNIEnv *env, jclass clss, jlong object_id)
    {
        herr_t retVal =  H5Oclose((hid_t)object_id) ;

        if (retVal < 0) {
            h5libraryError(env);
        }

        return (jint)retVal;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Ocopy
     * Signature: (JLjava/lang/String;JLjava/lang/String;JJ)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Ocopy
      (JNIEnv *env, jclass clss, jlong cur_loc_id, jstring cur_name, jlong dst_loc_id, jstring dst_name, jlong create_id, jlong access_id)
    {
        const char *lCurName;
        const char *lDstName;
        jboolean isCopy;
        herr_t   status = -1;

        if (cur_name == NULL) {
            h5nullArgument(env, "H5Ocopy:  cur_name is NULL");
            return;
        }

        lCurName = ENVPTR->GetStringUTFChars(ENVPAR cur_name, &isCopy);
        if (lCurName == NULL) {
            h5JNIFatalError(env, "H5Ocopy:  cur_name not pinned");
            return;
        }

        if (dst_name == NULL) {
            ENVPTR->ReleaseStringUTFChars(ENVPAR cur_name, lCurName);
            h5nullArgument(env, "H5Ocopy:  dst_name is NULL");
            return;
        }

        lDstName = ENVPTR->GetStringUTFChars(ENVPAR dst_name, &isCopy);
        if (lDstName == NULL) {
            ENVPTR->ReleaseStringUTFChars(ENVPAR cur_name, lCurName);
            h5JNIFatalError(env, "H5Ocopy:  dst_name not pinned");
            return;
        }

        status = H5Ocopy((hid_t)cur_loc_id, lCurName, (hid_t)dst_loc_id, lDstName, (hid_t)create_id, (hid_t)access_id);

        ENVPTR->ReleaseStringUTFChars(ENVPAR cur_name, lCurName);
        ENVPTR->ReleaseStringUTFChars(ENVPAR dst_name, lDstName);

        if (status < 0) {
           h5libraryError(env);
           return;
        }

        return;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oget_info
     * Signature: (J)Lhdf/hdf5lib/structs/H5O_info_t;
     */
    JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Oget_1info
    (JNIEnv *env, jclass clss, jlong loc_id)
    {
        herr_t      status;
        H5O_info_t  infobuf;
        jclass      cls;
        jmethodID   constructor;
        jvalue      args[12];
        jobject     hdrinfobuf;
        jobject     ihinfobuf1;
        jobject     ihinfobuf2;
        jobject     ret_info_t = NULL;

        status = H5Oget_info((hid_t)loc_id, (H5O_info_t*)&infobuf);

        if (status < 0) {
           h5libraryError(env);
           return NULL;
        }

        // get a reference to the H5_hdr_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5O_hdr_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5O_hdr_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(IIIIJJJJJJ)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5O_hdr_info_t failed\n");
           return NULL;
        }
        args[0].i = (jint)infobuf.hdr.version;
        args[1].i = (jint)infobuf.hdr.nmesgs;
        args[2].i = (jint)infobuf.hdr.nchunks;
        args[3].i = (jint)infobuf.hdr.flags;
        args[4].j = (jlong)infobuf.hdr.space.total;
        args[5].j = (jlong)infobuf.hdr.space.meta;
        args[6].j = (jlong)infobuf.hdr.space.mesg;
        args[7].j = (jlong)infobuf.hdr.space.free;
        args[8].j = (jlong)infobuf.hdr.mesg.present;
        args[9].j = (jlong)infobuf.hdr.mesg.shared;
        hdrinfobuf = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        // get a reference to the H5_ih_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5_ih_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5_ih_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(JJ)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5_ih_info_t failed\n");
           return NULL;
        }
        args[0].j = (jlong)infobuf.meta_size.obj.index_size;
        args[1].j = (jlong)infobuf.meta_size.obj.heap_size;
        ihinfobuf1 = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);
        args[0].j = (jlong)infobuf.meta_size.attr.index_size;
        args[1].j = (jlong)infobuf.meta_size.attr.heap_size;
        ihinfobuf2 = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        // get a reference to the H5O_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5O_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5O_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(JJIIJJJJJLhdf/hdf5lib/structs/H5O_hdr_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5O_info_t failed\n");
           return NULL;
        }
        args[0].j = (jlong)infobuf.fileno;
        args[1].j = (jlong)infobuf.addr;
        args[2].i = infobuf.type;
        args[3].i = (jint)infobuf.rc;
        args[4].j = (jlong)infobuf.num_attrs;
        args[5].j = infobuf.atime;
        args[6].j = infobuf.mtime;
        args[7].j = infobuf.ctime;
        args[8].j = infobuf.btime;
        args[9].l = hdrinfobuf;
        args[10].l = ihinfobuf1;
        args[11].l = ihinfobuf2;
        ret_info_t = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        return ret_info_t;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oget_info_by_name
     * Signature: (JLjava/lang/String;J)Lhdf/hdf5lib/structs/H5O_info_t;
     */
    JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Oget_1info_1by_1name
    (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong access_id)
    {
        const char *lName;
        herr_t      status;
        H5O_info_t  infobuf;
        jboolean    isCopy;
        jclass      cls;
        jmethodID   constructor;
        jvalue      args[12];
        jobject     hdrinfobuf;
        jobject     ihinfobuf1;
        jobject     ihinfobuf2;
        jobject     ret_info_t = NULL;

        if (name == NULL) {
            h5nullArgument(env, "H5Oget_info_by_name:  name is NULL");
            return NULL;
        }

        lName = ENVPTR->GetStringUTFChars(ENVPAR name, &isCopy);
        if (lName == NULL) {
            h5JNIFatalError(env, "H5Oget_info_by_name:  name not pinned");
            return NULL;
        }

        status = H5Oget_info_by_name((hid_t)loc_id, lName, (H5O_info_t*)&infobuf, (hid_t)access_id);

        ENVPTR->ReleaseStringUTFChars(ENVPAR name, lName);

        if (status < 0) {
           h5libraryError(env);
           return NULL;
        }

        // get a reference to the H5_hdr_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5O_hdr_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5O_hdr_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(IIIIJJJJJJ)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5O_hdr_info_t failed\n");
           return NULL;
        }
        args[0].i = (jint)infobuf.hdr.version;
        args[1].i = (jint)infobuf.hdr.nmesgs;
        args[2].i = (jint)infobuf.hdr.nchunks;
        args[3].i = (jint)infobuf.hdr.flags;
        args[4].j = (jlong)infobuf.hdr.space.total;
        args[5].j = (jlong)infobuf.hdr.space.meta;
        args[6].j = (jlong)infobuf.hdr.space.mesg;
        args[7].j = (jlong)infobuf.hdr.space.free;
        args[8].j = (jlong)infobuf.hdr.mesg.present;
        args[9].j = (jlong)infobuf.hdr.mesg.shared;
        hdrinfobuf = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        // get a reference to the H5_ih_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5_ih_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5_ih_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(JJ)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5_ih_info_t failed\n");
           return NULL;
        }
        args[0].j = (jlong)infobuf.meta_size.obj.index_size;
        args[1].j = (jlong)infobuf.meta_size.obj.heap_size;
        ihinfobuf1 = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);
        args[0].j = (jlong)infobuf.meta_size.attr.index_size;
        args[1].j = (jlong)infobuf.meta_size.attr.heap_size;
        ihinfobuf2 = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        // get a reference to the H5O_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5O_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5O_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(JJIIJJJJJLhdf/hdf5lib/structs/H5O_hdr_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5O_info_t failed\n");
           return NULL;
        }
        args[0].j = (jlong)infobuf.fileno;
        args[1].j = (jlong)infobuf.addr;
        args[2].i = infobuf.type;
        args[3].i = (jint)infobuf.rc;
        args[4].j = (jlong)infobuf.num_attrs;
        args[5].j = infobuf.atime;
        args[6].j = infobuf.mtime;
        args[7].j = infobuf.ctime;
        args[8].j = infobuf.btime;
        args[9].l = hdrinfobuf;
        args[10].l = ihinfobuf1;
        args[11].l = ihinfobuf2;
        ret_info_t = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        return ret_info_t;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oget_info_by_idx
     * Signature: (JLjava/lang/String;IIJJ)Lhdf/hdf5lib/structs/H5O_info_t;
     */
    JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Oget_1info_1by_1idx
    (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jint index_field, jint order, jlong link_n, jlong access_id)
    {
        const char *lName;
        herr_t      status;
        H5O_info_t  infobuf;
        jboolean    isCopy;
        jclass      cls;
        jmethodID   constructor;
        jvalue      args[12];
        jobject     hdrinfobuf;
        jobject     ihinfobuf1;
        jobject     ihinfobuf2;
        jobject     ret_info_t = NULL;

        if (name == NULL) {
            h5nullArgument(env, "H5Oget_info_by_idx:  name is NULL");
            return NULL;
        }

        lName = ENVPTR->GetStringUTFChars(ENVPAR name, &isCopy);
        if (lName == NULL) {
            h5JNIFatalError(env, "H5Oget_info_by_idx:  name not pinned");
            return NULL;
        }

        status = H5Oget_info_by_idx((hid_t)loc_id, lName, (H5_index_t)index_field, (H5_iter_order_t)order, (hsize_t)link_n, (H5O_info_t*)&infobuf, (hid_t)access_id);

        ENVPTR->ReleaseStringUTFChars(ENVPAR name, lName);

        if (status < 0) {
           h5libraryError(env);
           return NULL;
        }

        // get a reference to the H5_hdr_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5O_hdr_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5O_hdr_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(IIIIJJJJJJ)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5O_hdr_info_t failed\n");
           return NULL;
        }
        args[0].i = (jint)infobuf.hdr.version;
        args[1].i = (jint)infobuf.hdr.nmesgs;
        args[2].i = (jint)infobuf.hdr.nchunks;
        args[3].i = (jint)infobuf.hdr.flags;
        args[4].j = (jlong)infobuf.hdr.space.total;
        args[5].j = (jlong)infobuf.hdr.space.meta;
        args[6].j = (jlong)infobuf.hdr.space.mesg;
        args[7].j = (jlong)infobuf.hdr.space.free;
        args[8].j = (jlong)infobuf.hdr.mesg.present;
        args[9].j = (jlong)infobuf.hdr.mesg.shared;
        hdrinfobuf = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        // get a reference to the H5_ih_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5_ih_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5_ih_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(JJ)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5_ih_info_t failed\n");
           return NULL;
        }
        args[0].j = (jlong)infobuf.meta_size.obj.index_size;
        args[1].j = (jlong)infobuf.meta_size.obj.heap_size;
        ihinfobuf1 = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);
        args[0].j = (jlong)infobuf.meta_size.attr.index_size;
        args[1].j = (jlong)infobuf.meta_size.attr.heap_size;
        ihinfobuf2 = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        // get a reference to the H5O_info_t class
        cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5O_info_t");
        if (cls == 0) {
           h5JNIFatalError( env, "JNI error: GetObjectClass H5O_info_t failed\n");
           return NULL;
        }
        // get a reference to the constructor; the name is <init>
        constructor = ENVPTR->GetMethodID(ENVPAR cls, "<init>", "(JJIIJJJJJLhdf/hdf5lib/structs/H5O_hdr_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;)V");
        if (constructor == 0) {
            h5JNIFatalError( env, "JNI error: GetMethodID H5O_info_t failed\n");
           return NULL;
        }
        args[0].j = (jlong)infobuf.fileno;
        args[1].j = (jlong)infobuf.addr;
        args[2].i = infobuf.type;
        args[3].i = (jint)infobuf.rc;
        args[4].j = (jlong)infobuf.num_attrs;
        args[5].j = infobuf.atime;
        args[6].j = infobuf.mtime;
        args[7].j = infobuf.ctime;
        args[8].j = infobuf.btime;
        args[9].l = hdrinfobuf;
        args[10].l = ihinfobuf1;
        args[11].l = ihinfobuf2;
        ret_info_t = ENVPTR->NewObjectA(ENVPAR cls, constructor, args);

        return ret_info_t;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Olink
     * Signature: (JJLjava/lang/String;JJ)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Olink
      (JNIEnv *env, jclass clss, jlong cur_loc_id, jlong dst_loc_id, jstring dst_name, jlong create_id, jlong access_id)
    {
        const char *lDstName;
        jboolean isCopy;
        herr_t   status = -1;

        if (dst_name == NULL) {
            h5nullArgument(env, "H5Ocreate_hard:  dst_name is NULL");
            return;
        }

        lDstName = ENVPTR->GetStringUTFChars(ENVPAR dst_name, &isCopy);
        if (lDstName == NULL) {
            h5JNIFatalError( env, "H5Ocreate_hard:  dst_name not pinned");
            return;
        }

        status = H5Olink((hid_t)cur_loc_id, (hid_t)dst_loc_id, lDstName, (hid_t)create_id, (hid_t)access_id);

        ENVPTR->ReleaseStringUTFChars(ENVPAR dst_name, lDstName);

        if (status < 0) {
           h5libraryError(env);
           return;
        }

        return;
    }

    static
    herr_t H5O_iterate_cb(hid_t g_id, const char *name, const H5O_info_t *info, void *op_data) {
        JNIEnv    *cbenv;
        jint       status;
        jclass     cls;
        jmethodID  mid;
        jstring    str;
        jmethodID  constructor;
        jvalue     args[12];
        jobject    hdrinfobuf;
        jobject    ihinfobuf1;
        jobject    ihinfobuf2;
        jobject    cb_info_t = NULL;

        if(JVMPTR->AttachCurrentThread(JVMPAR2 (void**)&cbenv, NULL) != 0) {
            /* printf("JNI H5O_iterate_cb error: AttachCurrentThread failed\n"); */
            JVMPTR->DetachCurrentThread(JVMPAR);
            return -1;
        }
        cls = CBENVPTR->GetObjectClass(CBENVPAR visit_callback);
        if (cls == 0) {
            /* printf("JNI H5O_iterate_cb error: GetObjectClass failed\n"); */
           JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        mid = CBENVPTR->GetMethodID(CBENVPAR cls, "callback", "(JLjava/lang/String;Lhdf/hdf5lib/structs/H5O_info_t;Lhdf/hdf5lib/callbacks/H5O_iterate_t;)I");
        if (mid == 0) {
            /* printf("JNI H5O_iterate_cb error: GetMethodID failed\n"); */
            JVMPTR->DetachCurrentThread(JVMPAR);
            return -1;
        }
        str = CBENVPTR->NewStringUTF(CBENVPAR name);

        // get a reference to the H5_hdr_info_t class
        cls = CBENVPTR->FindClass(CBENVPAR "hdf/hdf5lib/structs/H5O_hdr_info_t");
        if (cls == 0) {
            JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        // get a reference to the constructor; the name is <init>
        constructor = CBENVPTR->GetMethodID(CBENVPAR cls, "<init>", "(IIIIJJJJJJ)V");
        if (constructor == 0) {
            JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        args[0].i = (jint)info->hdr.version;
        args[1].i = (jint)info->hdr.nmesgs;
        args[2].i = (jint)info->hdr.nchunks;
        args[3].i = (jint)info->hdr.flags;
        args[4].j = (jlong)info->hdr.space.total;
        args[5].j = (jlong)info->hdr.space.meta;
        args[6].j = (jlong)info->hdr.space.mesg;
        args[7].j = (jlong)info->hdr.space.free;
        args[8].j = (jlong)info->hdr.mesg.present;
        args[9].j = (jlong)info->hdr.mesg.shared;
        hdrinfobuf = CBENVPTR->NewObjectA(CBENVPAR cls, constructor, args);

        // get a reference to the H5_ih_info_t class
        cls = CBENVPTR->FindClass(CBENVPAR "hdf/hdf5lib/structs/H5_ih_info_t");
        if (cls == 0) {
            JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        // get a reference to the constructor; the name is <init>
        constructor = CBENVPTR->GetMethodID(CBENVPAR cls, "<init>", "(JJ)V");
        if (constructor == 0) {
            JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        args[0].j = (jlong)info->meta_size.obj.index_size;
        args[1].j = (jlong)info->meta_size.obj.heap_size;
        ihinfobuf1 = CBENVPTR->NewObjectA(CBENVPAR cls, constructor, args);
        args[0].j = (jlong)info->meta_size.attr.index_size;
        args[1].j = (jlong)info->meta_size.attr.heap_size;
        ihinfobuf2 = CBENVPTR->NewObjectA(CBENVPAR cls, constructor, args);

        // get a reference to the H5O_info_t class
        cls = CBENVPTR->FindClass(CBENVPAR "hdf/hdf5lib/structs/H5O_info_t");
        if (cls == 0) {
            JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        // get a reference to the constructor; the name is <init>
        constructor = CBENVPTR->GetMethodID(CBENVPAR cls, "<init>", "(JJIIJJJJJLhdf/hdf5lib/structs/H5O_hdr_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;Lhdf/hdf5lib/structs/H5_ih_info_t;)V");
        if (constructor == 0) {
            JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        args[0].j = (jlong)info->fileno;
        args[1].j = (jlong)info->addr;
        args[2].i = info->type;
        args[3].i = (jint)info->rc;
        args[4].j = (jlong)info->num_attrs;
        args[5].j = info->atime;
        args[6].j = info->mtime;
        args[7].j = info->ctime;
        args[8].j = info->btime;
        args[9].l = hdrinfobuf;
        args[10].l = ihinfobuf1;
        args[11].l = ihinfobuf2;
        cb_info_t = CBENVPTR->NewObjectA(CBENVPAR cls, constructor, args);

        status = CBENVPTR->CallIntMethod(CBENVPAR visit_callback, mid, g_id, str, cb_info_t, op_data);

        JVMPTR->DetachCurrentThread(JVMPAR);
        return status;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Ovisit
     * Signature: (JIILjava/lang/Object;Ljava/lang/Object;)I
     */
    JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Ovisit
      (JNIEnv *env, jclass clss, jlong grp_id, jint idx_type, jint order,
              jobject callback_op, jobject op_data)
    {
        herr_t        status = -1;

        ENVPTR->GetJavaVM(ENVPAR &jvm);
        visit_callback = callback_op;

        if (op_data == NULL) {
            h5nullArgument(env, "H5Ovisit:  op_data is NULL");
            return -1;
        }
        if (callback_op == NULL) {
            h5nullArgument(env, "H5Ovisit:  callback_op is NULL");
            return -1;
        }

        status = H5Ovisit((hid_t)grp_id, (H5_index_t)idx_type, (H5_iter_order_t)order, (H5O_iterate_t)H5O_iterate_cb, (void*)op_data);

        if (status < 0) {
           h5libraryError(env);
           return status;
        }

        return status;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Ovisit_by_name
     * Signature: (JLjava/lang/String;IILjava/lang/Object;Ljava/lang/Object;J)I
     */
    JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Ovisit_1by_1name
      (JNIEnv *env, jclass clss, jlong grp_id, jstring name, jint idx_type, jint order,
              jobject callback_op, jobject op_data, jlong access_id)
    {
        jboolean      isCopy;
        const char   *lName;
        herr_t        status = -1;

        ENVPTR->GetJavaVM(ENVPAR &jvm);
        visit_callback = callback_op;

        if (name == NULL) {
            h5nullArgument(env, "H5Ovisit_by_name:  name is NULL");
            return -1;
        }

        lName = ENVPTR->GetStringUTFChars(ENVPAR name, &isCopy);
        if (lName == NULL) {
            h5JNIFatalError(env, "H5Ovisit_by_name:  name not pinned");
            return -1;
        }

        if (op_data == NULL) {
            h5nullArgument(env, "H5Ovisit_by_name:  op_data is NULL");
            return -1;
        }
        if (callback_op == NULL) {
            h5nullArgument(env, "H5Ovisit_by_name:  callback_op is NULL");
            return -1;
        }

        status = H5Ovisit_by_name((hid_t)grp_id, lName, (H5_index_t)idx_type, (H5_iter_order_t)order, (H5O_iterate_t)H5O_iterate_cb, (void*)op_data, (hid_t)access_id);

        ENVPTR->ReleaseStringUTFChars(ENVPAR name, lName);

        if (status < 0) {
           h5libraryError(env);
           return status;
        }

        return status;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oset_comment
     * Signature: (JLjava/lang/String;)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Oset_1comment
      (JNIEnv *env, jclass clss, jlong loc_id, jstring comment)
    {
        herr_t      status;
        const char *oComment;
        jboolean    isCopy;

        if (comment == NULL) {
            oComment = NULL;
        }
        else {
            oComment = ENVPTR->GetStringUTFChars(ENVPAR comment, &isCopy);
            if (oComment == NULL) {
                h5JNIFatalError( env, "H5Oset_comment:  comment not pinned");
                return;
            }
        }

        status = H5Oset_comment((hid_t)loc_id, oComment);

        ENVPTR->ReleaseStringUTFChars(ENVPAR comment, oComment);

        if (status < 0) {
            h5libraryError(env);
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oset_comment_by_name
     * Signature: (JLjava/lang/String;Ljava/lang/String;J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Oset_1comment_1by_1name
      (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jstring comment, jlong access_id)
    {
        herr_t      status;
        const char *oName;
        const char *oComment;
        jboolean    isCopy;

        if (name == NULL) {
            h5nullArgument( env, "H5Oset_comment_by_name:  name is NULL");
            return;
        }
        oName = ENVPTR->GetStringUTFChars(ENVPAR name, &isCopy);
        if (oName == NULL) {
            h5JNIFatalError( env, "H5Oset_comment_by_name:  name not pinned");
            return;
        }
        if (comment == NULL) {
            oComment = NULL;
        }
        else {
            oComment = ENVPTR->GetStringUTFChars(ENVPAR comment,&isCopy);
            if (oComment == NULL) {
                ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);
                h5JNIFatalError( env, "H5Oset_comment_by_name:  comment not pinned");
                return;
            }
        }

        status = H5Oset_comment_by_name((hid_t)loc_id, oName, oComment, (hid_t)access_id);

        ENVPTR->ReleaseStringUTFChars(ENVPAR comment, oComment);
        ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);

        if (status < 0) {
            h5libraryError(env);
        }
        return;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oget_comment
     * Signature: (J)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Oget_1comment
      (JNIEnv *env, jclass clss, jlong loc_id)
    {
        char   *oComment;
        ssize_t  buf_size;
        ssize_t status;
        jstring str;

        /* get the length of the comment */
        buf_size = H5Oget_comment((hid_t)loc_id, NULL, 0);
        if (buf_size < 0) {
            h5badArgument( env, "H5Oget_comment:  buf_size < 0");
            return NULL;
        }
        if (buf_size == 0) {
            return NULL;
        }

        buf_size++; /* add extra space for the null terminator */
        oComment = (char *)malloc(sizeof(char) * (size_t)buf_size);
        if (oComment == NULL) {
            /* exception -- out of memory */
            h5outOfMemory( env, "H5Oget_comment:  malloc failed");
            return NULL;
        }

        status = H5Oget_comment((hid_t)loc_id, (char*)oComment, (size_t)buf_size);

        if (status >= 0) {
            /*  may throw OutOfMemoryError */
            str = ENVPTR->NewStringUTF(ENVPAR oComment);
            free(oComment);
            if (str == NULL) {
                h5JNIFatalError( env, "H5Oget_comment:  return string not allocated");
                return NULL;
            }
        }
        else {
            free(oComment);
            h5libraryError(env);
            return NULL;
        }

        return (jstring)str;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oget_comment_by_name
     * Signature: (JLjava/lang/String;J)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Oget_1comment_1by_1name
      (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong access_id)
    {
        char    *oComment;
        const char *oName;
        ssize_t   buf_size;
        ssize_t  status;
        jstring  str;
        jboolean isCopy;

        if (name == NULL) {
            h5nullArgument( env, "H5Oget_comment_by_name:  name is NULL");
            return NULL;
        }
        oName = ENVPTR->GetStringUTFChars(ENVPAR name, &isCopy);
        if (oName == NULL) {
            h5JNIFatalError( env, "H5Oget_comment_by_name:  name not pinned");
            return NULL;
        }

        /* get the length of the comment */
        buf_size = H5Oget_comment_by_name((hid_t)loc_id, oName, NULL, 0, (hid_t)access_id);
        if (buf_size < 0) {
            ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);
            h5badArgument( env, "H5Oget_comment_by_name:  buf_size < 0");
            return NULL;
        }
        if (buf_size == 0) {
            ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);
            return NULL;
        }

        buf_size++; /* add extra space for the null terminator */
        oComment = (char *)malloc(sizeof(char) * (size_t)buf_size);
        if (oComment == NULL) {
            ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);
            h5outOfMemory( env, "H5Oget_comment_by_name:  malloc failed");
            return NULL;
        }

        status = H5Oget_comment_by_name((hid_t)loc_id, oName, (char*)oComment, (size_t)buf_size, (hid_t)access_id);
        ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);

        if (status >= 0) {
            /*  may throw OutOfMemoryError */
            str = ENVPTR->NewStringUTF(ENVPAR oComment);
            free(oComment);
            if (str == NULL) {
                h5JNIFatalError( env, "H5Oget_comment_by_name:  return string not allocated");
                return NULL;
            }
        }
        else {
            free(oComment);
            h5libraryError(env);
            return NULL;
        }

        return (jstring)str;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oexists_by_name
     * Signature: (JLjava/lang/String;J)Z
     */
    JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Oexists_1by_1name
      (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong access_id)
    {
        htri_t bval;
        const char *oName;
        jboolean isCopy;

        if (name == NULL) {
            h5nullArgument( env, "H5Oexists_by_name:  name is NULL");
            return NULL;
        }
        oName = ENVPTR->GetStringUTFChars(ENVPAR name, &isCopy);
        if (oName == NULL) {
            h5JNIFatalError( env, "H5Oexists_by_name:  name not pinned");
            return NULL;
        }

        bval = H5Oexists_by_name((hid_t)loc_id, oName, (hid_t)access_id);

        ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);

        if (bval > 0) {
            return JNI_TRUE;
        }
        else if (bval == 0) {
            return JNI_FALSE;
        }
        else {
            /* raise exception -- return value is irrelevant */
            h5libraryError(env);
            return JNI_FALSE;
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Odecr_refcount
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Odecr_1refcount
    (JNIEnv *env, jclass clss, jlong object_id)
    {
      herr_t retVal =  H5Odecr_refcount((hid_t)object_id) ;

      if (retVal < 0) {
          h5libraryError(env);
      }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Oincr_refcount
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Oincr_1refcount
    (JNIEnv *env, jclass clss, jlong object_id)
    {
      herr_t retVal =  H5Oincr_refcount((hid_t)object_id) ;

      if (retVal < 0) {
          h5libraryError(env);
      }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    _H5Oopen_by_addr
     * Signature: (JJ)J;
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Oopen_1by_1addr
      (JNIEnv *env, jclass clss, jlong loc_id, jlong addr)
    {
        hid_t    status;

        status = H5Oopen_by_addr((hid_t)loc_id, (haddr_t)addr );

        if (status < 0) {
            h5libraryError(env);
        }
        return (jlong)status;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    _H5Oopen_by_idx
     * Signature: (JLjava/lang/String;IIJJ)J;
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Oopen_1by_1idx
      (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jint index_field, jint order, jlong link_n, jlong lapl_id)
    {
        hid_t    status;
        const char *oName;
        jboolean isCopy;

        if (name == NULL) {
            h5nullArgument( env, "H5Oopen:  name is NULL");
            return -1;
        }

        oName = ENVPTR->GetStringUTFChars(ENVPAR name,&isCopy);

        if (oName == NULL) {
            h5JNIFatalError( env, "H5Oopen:  object name not pinned");
            return -1;
        }

        status = H5Oopen_by_idx((hid_t)loc_id, oName, (H5_index_t)index_field, (H5_iter_order_t)order, (hsize_t)link_n, (hid_t)lapl_id );

        ENVPTR->ReleaseStringUTFChars(ENVPAR name, oName);
        if (status < 0) {
            h5libraryError(env);
        }
        return (jlong)status;
    }


#ifdef __cplusplus
}
#endif
