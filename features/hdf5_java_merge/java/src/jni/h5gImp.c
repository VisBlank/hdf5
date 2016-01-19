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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdf5.h"
#include "h5jni.h"
#include "h5gImp.h"
#include "h5util.h"

/* missing definitions from hdf5.h */
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#ifdef __cplusplus
    herr_t obj_info_all(hid_t g_id, const char *name, const H5L_info_t *linfo, void *op_data);
    herr_t obj_info_max(hid_t g_id, const char *name, const H5L_info_t *linfo, void *op_data);
    int H5Gget_obj_info_max(hid_t, char **, int *, int *, unsigned long *, long);
    int H5Gget_obj_info_full( hid_t loc_id, char **objname, int *otype, int *ltype, unsigned long *fno, unsigned long *objno, int indexType, int indexOrder);
#else
    static herr_t obj_info_all(hid_t g_id, const char *name, const H5L_info_t *linfo, void *op_data);
    static herr_t obj_info_max(hid_t g_id, const char *name, const H5L_info_t *linfo, void *op_data);
    static int H5Gget_obj_info_max(hid_t, char **, int *, int *, unsigned long *, long);
    static int H5Gget_obj_info_full( hid_t loc_id, char **objname, int *otype, int *ltype, unsigned long *fno, unsigned long *objno, int indexType, int indexOrder);
#endif

typedef struct info_all
{
    char **objname;
    int *otype;
    int *ltype;
    unsigned long *objno;
    unsigned long *fno;
    unsigned long idxnum;
    int count;
} info_all_t;

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gcreate
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gcreate
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong size_hint)
{
    hid_t       status;
    const char *gName;

    PIN_JAVA_STRING(name, gName, -1);

    status = H5Gcreate2((hid_t)loc_id, gName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, gName);
    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gopen
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gopen
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name)
{
    hid_t status;
    const char *gName;

    PIN_JAVA_STRING(name, gName, -1);

    status = H5Gopen2((hid_t)loc_id, gName, H5P_DEFAULT);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, gName);
    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gclose
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5__1H5Gclose
  (JNIEnv *env, jclass clss, jlong group_id)
{
    herr_t retVal =  H5Gclose((hid_t)group_id);

    if (retVal < 0) {
        h5libraryError(env);
    }

    return (jint)retVal;
}

/*
/////////////////////////////////////////////////////////////////////////////////
//
//
// Add these methods so that we don't need to call H5Gget_objtype_by_idx
// in a loop to get information for all the object in a group, which takes
// a lot of time to finish if the number of objects is more than 10,000
//
/////////////////////////////////////////////////////////////////////////////////
*/
/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_obj_info_full
 * Signature: (JLjava/lang/String;[Ljava/lang/String;[I[I[J[JIII)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Gget_1obj_1info_1full
  (JNIEnv *env, jclass clss, jlong loc_id, jstring group_name,
  jobjectArray objName, jintArray oType, jintArray lType, jlongArray fNo,
  jlongArray oRef, jint n, jint indx_type, jint indx_order)
{
    herr_t        ret_val = -1;
    const char   *gName = NULL;
    char        **oName = NULL;
    jboolean      isCopy;
    jstring       str;
    jint          *otarr;
    jint          *ltarr;
    jlong         *refP;
    jlong         *fnoP;
    unsigned long *refs=NULL;
    unsigned long *fnos=NULL;
    hid_t          gid = (hid_t)loc_id;
    int            i;
    int            indexType = indx_type;
    int            indexOrder = indx_order;

    if (group_name != NULL) {
        gName = ENVPTR->GetStringUTFChars(ENVPAR group_name,&isCopy);
        if (gName == NULL) {
            h5JNIFatalError(env, "H5Gget_obj_info_full:  name not pinned");
            return -1;
        }
        gid = H5Gopen2((hid_t)loc_id, gName, H5P_DEFAULT);

        ENVPTR->ReleaseStringUTFChars(ENVPAR group_name, gName);

        if(gid < 0) {
            h5JNIFatalError(env, "H5Gget_obj_info_full: could not get group identifier");
            return -1;
        }
    }

    if (oType == NULL) {
        h5nullArgument(env, "H5Gget_obj_info_full:  oType is NULL");
        return -1;
    }

    if (lType == NULL) {
        h5nullArgument(env, "H5Gget_obj_info_full:  lType is NULL");
        return -1;
    }

    if (oRef == NULL) {
        h5nullArgument(env, "H5Gget_obj_info_full:  oRef is NULL");
        return -1;
    }

    otarr = ENVPTR->GetIntArrayElements(ENVPAR oType,&isCopy);
    if (otarr == NULL) {
        h5JNIFatalError(env, "H5Gget_obj_info_full:  otype not pinned");
        return -1;
    }

    ltarr = ENVPTR->GetIntArrayElements(ENVPAR lType, &isCopy);
    if (ltarr == NULL) {
        ENVPTR->ReleaseIntArrayElements(ENVPAR oType,otarr, JNI_ABORT);
        h5JNIFatalError(env, "H5Gget_obj_info_full:  ltype not pinned");
        return -1;
    }

    refP = ENVPTR->GetLongArrayElements(ENVPAR oRef, &isCopy);
    fnoP = ENVPTR->GetLongArrayElements(ENVPAR fNo, &isCopy);
    if (refP == NULL) {
        ENVPTR->ReleaseIntArrayElements(ENVPAR lType,ltarr, JNI_ABORT);
        ENVPTR->ReleaseIntArrayElements(ENVPAR oType,otarr, JNI_ABORT);
        h5JNIFatalError(env, "H5Gget_obj_info_full:  type not pinned");
        return -1;
    }

    oName = (char **)HDcalloc((size_t)n, sizeof(*oName));
    if (!oName)
      goto error;

    refs = (unsigned long *)HDcalloc((size_t)n, sizeof(unsigned long));
    fnos = (unsigned long *)HDcalloc((size_t)n, sizeof(unsigned long));
    if (!refs || !fnos)
      goto error;

    ret_val = H5Gget_obj_info_full(gid, oName, (int *)otarr, (int *)ltarr, fnos, refs, indexType, indexOrder);

    if (ret_val < 0)
        goto error;

    if (refs) {
        for (i=0; i<n; i++) {
            refP[i] = (jlong)refs[i];
        }
    }

    if (fnos) {
        for (i=0; i<n; i++) {
            fnoP[i] = (jlong)fnos[i];
        }
    }

    if (oName) {
        for (i=0; i<n; i++) {
            if (*(oName+i)) {
                str = ENVPTR->NewStringUTF(ENVPAR *(oName+i));
                ENVPTR->SetObjectArrayElement(ENVPAR objName, i, (jobject)str);
            }
        } /* for (i=0; i<n; i++)*/
    }

    if (group_name != NULL)
        H5Gclose(gid);
    ENVPTR->ReleaseIntArrayElements(ENVPAR lType, ltarr, 0);
    ENVPTR->ReleaseIntArrayElements(ENVPAR oType, otarr, 0);
    ENVPTR->ReleaseLongArrayElements(ENVPAR oRef, refP, 0);
    ENVPTR->ReleaseLongArrayElements(ENVPAR fNo, fnoP, 0);
    if (oName)
        h5str_array_free(oName, (size_t)n);
    if (refs)
        HDfree(refs);
    if (fnos)
        HDfree(fnos);

    return ret_val;

error:
    if (group_name != NULL)
        H5Gclose(gid);
    ENVPTR->ReleaseIntArrayElements(ENVPAR lType, ltarr, JNI_ABORT);
    ENVPTR->ReleaseIntArrayElements(ENVPAR oType,otarr, JNI_ABORT);
    ENVPTR->ReleaseLongArrayElements(ENVPAR oRef,  refP, JNI_ABORT);
    ENVPTR->ReleaseLongArrayElements(ENVPAR fNo, fnoP, JNI_ABORT);
    if (oName)
        h5str_array_free(oName, (size_t)n);
    if (refs)
        HDfree(refs);
    if (fnos)
        HDfree(fnos);
    h5libraryError(env);

    return -1;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_obj_info_max
 * Signature: (J[Ljava/lang/String;[I[I[JJI)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Gget_1obj_1info_1max
  (JNIEnv *env, jclass clss, jlong loc_id, jobjectArray objName,
          jintArray oType, jintArray lType, jlongArray oRef,
          jlong maxnum, jint n)
{
    herr_t         ret_val = -1;
    char         **oName=NULL;
    jboolean       isCopy;
    jstring        str;
    jint          *otarr;
    jint          *ltarr;
    jlong         *refP;
    unsigned long *refs;
    int            i;

    if (oType == NULL) {
        h5nullArgument(env, "H5Gget_obj_info_max:  oType is NULL");
        return -1;
    }

    if (lType == NULL) {
        h5nullArgument(env, "H5Gget_obj_info_max:  lType is NULL");
        return -1;
    }

    if (oRef == NULL) {
        h5nullArgument(env, "H5Gget_obj_info_max:  oRef is NULL");
        return -1;
    }

    otarr = ENVPTR->GetIntArrayElements(ENVPAR oType, &isCopy);
    if (otarr == NULL) {
        h5JNIFatalError(env, "H5Gget_obj_info_max:  otype not pinned");
        return -1;
    }

    ltarr = ENVPTR->GetIntArrayElements(ENVPAR lType, &isCopy);
    if (ltarr == NULL) {
        ENVPTR->ReleaseIntArrayElements(ENVPAR oType, otarr, JNI_ABORT);
        h5JNIFatalError(env, "H5Gget_obj_info_max:  ltype not pinned");
        return -1;
    }

    refP = ENVPTR->GetLongArrayElements(ENVPAR oRef, &isCopy);
    if (refP == NULL) {
        ENVPTR->ReleaseIntArrayElements(ENVPAR oType, otarr, JNI_ABORT);
        ENVPTR->ReleaseIntArrayElements(ENVPAR lType, ltarr, JNI_ABORT);
        h5JNIFatalError(env, "H5Gget_obj_info_max:  type not pinned");
        return -1;
    }

    oName = (char **)HDcalloc((size_t)n, sizeof(*oName));
    refs = (unsigned long *)HDcalloc((size_t)n, sizeof(unsigned long));

    ret_val = H5Gget_obj_info_max((hid_t)loc_id, oName, (int*)otarr, (int*)ltarr, refs, maxnum );

    if (ret_val < 0) {
        ENVPTR->ReleaseIntArrayElements(ENVPAR lType, ltarr, JNI_ABORT);
        ENVPTR->ReleaseIntArrayElements(ENVPAR oType, otarr, JNI_ABORT);
        ENVPTR->ReleaseLongArrayElements(ENVPAR oRef, refP, JNI_ABORT);
        h5str_array_free(oName, (size_t)n);
        HDfree(refs);
        h5libraryError(env);
        return -1;
    }

    ENVPTR->ReleaseIntArrayElements(ENVPAR lType, ltarr, 0);
    ENVPTR->ReleaseIntArrayElements(ENVPAR oType, otarr, 0);

    if (refs) {
        for (i=0; i<n; i++) {
            refP[i] = (jlong) refs[i];
        }
    }
    HDfree(refs);
    ENVPTR->ReleaseLongArrayElements(ENVPAR oRef, refP, 0);

    if (oName) {
        for (i=0; i<n; i++) {
            if (*(oName+i)) {
                str = ENVPTR->NewStringUTF(ENVPAR *(oName+i));
                ENVPTR->SetObjectArrayElement(ENVPAR objName, i, (jobject)str);
            }
        } /* for (i=0; i<n; i++)*/
    }

    h5str_array_free(oName, (size_t)n);

    return ret_val;
}

int H5Gget_obj_info_full(hid_t loc_id, char **objname, int *otype, int *ltype, unsigned long *fno, unsigned long *objno, int indexType, int indexOrder)
{
    info_all_t info;
    info.objname = objname;
    info.otype = otype;
    info.ltype = ltype;
    info.idxnum = 0;
    info.fno = fno;
    info.objno = objno;
    info.count = 0;

    if(H5Literate(loc_id, (H5_index_t)indexType, (H5_iter_order_t)indexOrder, NULL, obj_info_all, (void *)&info) < 0) {
        /* iterate failed, try normal alphabetical order */
        if(H5Literate(loc_id, H5_INDEX_NAME, H5_ITER_INC, NULL, obj_info_all, (void *)&info) < 0)
            return -1;
    }

    return info.count;
}

int H5Gget_obj_info_max(hid_t loc_id, char **objname, int *otype, int *ltype, unsigned long *objno, long maxnum)
{
    info_all_t info;
    info.objname = objname;
    info.otype = otype;
    info.ltype = ltype;
    info.idxnum = (unsigned long)maxnum;
    info.objno = objno;
    info.count = 0;

    if(H5Lvisit(loc_id, H5_INDEX_NAME, H5_ITER_NATIVE, obj_info_max, (void *)&info) < 0)
        return -1;

    return info.count;
}

herr_t obj_info_all(hid_t loc_id, const char *name, const H5L_info_t *info, void *op_data)
{
    int         type = -1;
    hid_t       oid = -1;
    herr_t      retVal = -1;
    info_all_t *datainfo = (info_all_t*)op_data;
    H5O_info_t  object_info;

    retVal = H5Oget_info_by_name(loc_id, name, &object_info, H5P_DEFAULT);

    if (retVal < 0) {
        *(datainfo->otype+datainfo->count) = -1;
        *(datainfo->ltype+datainfo->count) = -1;
        *(datainfo->objname+datainfo->count) = (char *)HDmalloc(strlen(name)+1);
        HDstrcpy(*(datainfo->objname+datainfo->count), name);
        *(datainfo->objno+datainfo->count) = (unsigned long)-1;
    }
    else {
        *(datainfo->otype+datainfo->count) = object_info.type;
        *(datainfo->ltype+datainfo->count) = info->type;
        *(datainfo->objname+datainfo->count) = (char *)HDmalloc(HDstrlen(name)+1);
        HDstrcpy(*(datainfo->objname+datainfo->count), name);

    *(datainfo->fno+datainfo->count) = object_info.fileno;
    *(datainfo->objno+datainfo->count) = (unsigned long)object_info.addr;
    /*
        if(info->type==H5L_TYPE_HARD)
            *(datainfo->objno+datainfo->count) = (unsigned long)info->u.address;
        else
            *(datainfo->objno+datainfo->count) = info->u.val_size;
        */
    }

    datainfo->count++;

    return 0;
}

herr_t obj_info_max(hid_t loc_id, const char *name, const H5L_info_t *info, void *op_data)
{
    int         type = -1;
    herr_t      retVal = 0;
    info_all_t *datainfo = (info_all_t*)op_data;
    H5O_info_t  object_info;

    retVal = H5Oget_info(loc_id, &object_info);
    if (retVal < 0) {
        *(datainfo->otype+datainfo->count) = -1;
        *(datainfo->ltype+datainfo->count) = -1;
        *(datainfo->objname+datainfo->count) = NULL;
        *(datainfo->objno+datainfo->count) = (unsigned long)-1;
        return 1;
    }
    else {
        *(datainfo->otype+datainfo->count) = object_info.type;
        *(datainfo->ltype+datainfo->count) = info->type;
        /* this will be freed by h5str_array_free(oName, n)*/
        *(datainfo->objname+datainfo->count) = (char *)HDmalloc(HDstrlen(name)+1);
        strcpy(*(datainfo->objname+datainfo->count), name);
    if(info->type==H5L_TYPE_HARD)
            *(datainfo->objno+datainfo->count) = (unsigned long)info->u.address;
        else
            *(datainfo->objno+datainfo->count) = info->u.val_size;
    }
    datainfo->count++;
    if(datainfo->count < (int)datainfo->idxnum)
        return 0;
    else
        return 1;
}

/*
 * Create a java object of hdf.h5.structs.H5G_info_t
 * public class H5G_info_t {
 *   public H5G_STORAGE_TYPE  storage_type; // Type of storage for links in group
 *   public long     nlinks;       // Number of links in group
 *   public long     max_corder;   // Current max. creation order value for group
 *   public int      mounted;      // Whether group has a file mounted on it
 * }
 *
 */
jobject create_H5G_info_t(JNIEnv *env, H5G_info_t group_info)
{
    jclass   cls;
    jboolean jmounted;
    jint     storage_type;
    jobject  obj;
    jfieldID fid_storage_type, fid_nlinks, fid_max_corder, fid_mounted;

    cls = ENVPTR->FindClass(ENVPAR "hdf/hdf5lib/structs/H5G_info_t");
    if (cls == NULL) return NULL;

    obj = ENVPTR->AllocObject(ENVPAR cls);
    if (obj == NULL) return NULL;

    fid_storage_type = ENVPTR->GetFieldID(ENVPAR cls, "storage_type", "I");
    fid_nlinks = ENVPTR->GetFieldID(ENVPAR cls, "nlinks", "J");
    fid_max_corder = ENVPTR->GetFieldID(ENVPAR cls, "max_corder", "J");
    fid_mounted = ENVPTR->GetFieldID(ENVPAR cls, "mounted", "Z");

    if (fid_storage_type==NULL || fid_nlinks==NULL || fid_max_corder==NULL ||
            fid_mounted == NULL)
        return NULL;

    jmounted = (group_info.mounted==0) ? JNI_FALSE : JNI_TRUE;
    storage_type = (jint)group_info.storage_type;

    ENVPTR->SetIntField(ENVPAR obj, fid_storage_type, (jint)storage_type);
    ENVPTR->SetLongField(ENVPAR obj, fid_nlinks, (jlong)group_info.nlinks);
    ENVPTR->SetLongField(ENVPAR obj, fid_max_corder, (jlong)group_info.max_corder);
    ENVPTR->SetBooleanField(ENVPAR obj, fid_mounted, jmounted);

    return obj;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gcreate2
 * Signature: (JLjava/lang/String;JJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gcreate2
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name,
          jlong link_plist_id, jlong create_plist_id, jlong access_plist_id)
{
    hid_t       status;
    const char *gName;

    PIN_JAVA_STRING(name, gName, -1);

    status = H5Gcreate2((hid_t)loc_id, gName, (hid_t)link_plist_id, (hid_t)create_plist_id, (hid_t)access_plist_id );

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, gName);
    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gcreate_anon
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gcreate_1anon
  (JNIEnv *env, jclass cls, jlong loc_id, jlong gcpl_id, jlong gapl_id)
{
    hid_t ret_val;

    ret_val = H5Gcreate_anon((hid_t)loc_id, (hid_t)gcpl_id, (hid_t)gapl_id);

    if (ret_val < 0) {
        h5libraryError(env);
    }
    return (jlong)ret_val;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Gopen2
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Gopen2
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong access_plist_id)
{
    hid_t status;
    const char *gName;

    PIN_JAVA_STRING(name, gName, -1);

    status = H5Gopen2((hid_t)loc_id, gName, (hid_t)access_plist_id );

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, gName);
    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}


/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_create_plist
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Gget_1create_1plist
(JNIEnv *env, jclass cls, jlong loc_id)
{
  hid_t ret_val;

  ret_val = H5Gget_create_plist((hid_t)loc_id);

  if (ret_val < 0) {
      h5libraryError(env);
  }

  return (jlong)ret_val;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_info
 * Signature: (J)Lhdf/hdf5lib/structs/H5G_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Gget_1info
  (JNIEnv *env, jclass cls, jlong loc_id)
{
    H5G_info_t group_info;
    herr_t     ret_val = -1;

    ret_val = H5Gget_info((hid_t)loc_id, &group_info);

    if (ret_val < 0) {
        h5libraryError(env);
        return NULL;
    }

    return create_H5G_info_t(env, group_info);
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_info_by_name
 * Signature: (JLjava/lang/String;J)Lhdf/hdf5lib/structs/H5G_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Gget_1info_1by_1name
  (JNIEnv *env, jclass cls, jlong loc_id, jstring name, jlong lapl_id)
{
    herr_t      ret_val = -1;
    const char *gName;
    H5G_info_t  group_info;

    PIN_JAVA_STRING(name, gName, NULL);

    ret_val = H5Gget_info_by_name((hid_t)loc_id, gName, &group_info, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, gName);

    if (ret_val < 0) {
        h5libraryError(env);
        return NULL;
    }

    return create_H5G_info_t(env, group_info);
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Gget_info_by_idx
 * Signature: (JLjava/lang/String;IIJJ)Lhdf/hdf5lib/structs/H5G_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Gget_1info_1by_1idx
  (JNIEnv *env, jclass cls, jlong loc_id, jstring name, jint index_type,
          jint order, jlong n, jlong lapl_id)
{
    herr_t          ret_val = -1;
    const char     *gName;
    H5G_info_t      group_info;
    H5_index_t      cindex_type = (H5_index_t)index_type;
    H5_iter_order_t corder = (H5_iter_order_t)order;

    PIN_JAVA_STRING(name, gName, NULL);

    ret_val = H5Gget_info_by_idx((hid_t)loc_id, gName, cindex_type,
            corder, (hsize_t)n, &group_info, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, gName);

    if (ret_val < 0) {
        h5libraryError(env);
        return NULL;
    }

    return create_H5G_info_t(env, group_info);
}


#ifdef __cplusplus
}
#endif
