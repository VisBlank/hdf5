/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF Java Products. The full HDF Java copyright       *
 * notice, including terms governing use, modification, and redistribution,  *
 * is contained in the file, COPYING.  COPYING can be found at the root of   *
 * the source code distribution tree. You can also access it online  at      *
 * http://www.hdfgroup.org/products/licenses.html.  If you do not have       *
 * access to the file, you may request a copy from help@hdfgroup.org.        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 *  For details of the HDF libraries, see the HDF Documentation at:
 *    http://hdfdfgroup.org/HDF5/doc/
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "hdf5.h"
#include <stdlib.h>
#include <jni.h>
#include "h5jni.h"

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Iget_type
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Iget_1type
  (JNIEnv *env, jclass clss, jlong obj_id)
{
    H5I_type_t retVal = H5I_BADID;
    retVal =  H5Iget_type((hid_t)obj_id);
    if (retVal == H5I_BADID) {
        h5libraryError(env);
    }
    return (jint)retVal;
}


/**********************************************************************
 *                                                                    *
 *          New functions release 1.6.2 versus release 1.6.1          *
 *                                                                    *
 **********************************************************************/

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Iget_name
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Iget_1name
  (JNIEnv *env, jclass clss, jlong obj_id, jobjectArray name, jlong buf_size)
{
    char *aName;
    jstring str;
    hssize_t size;
    long bs;

    bs = (long)buf_size;
    if (bs <= 0) {
        h5badArgument( env, "H5Iget_name:  buf_size <= 0");
        return -1;
    }
    aName = (char*)malloc(sizeof(char)*bs);
    if (aName == NULL) {
        h5outOfMemory( env, "H5Iget_name:  malloc failed");
        return -1;
    }

    size = H5Iget_name((hid_t)obj_id, aName, (size_t)buf_size);
    if (size < 0) {
        free(aName);
        h5libraryError(env);
        return -1;
        /*  exception, returns immediately */
    }
    /* successful return -- save the string; */
    str = ENVPTR->NewStringUTF(ENVPAR aName);
    ENVPTR->SetObjectArrayElement(ENVPAR name,0,str);

    free(aName);
    return (jlong)size;
}


/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Iget_ref
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Iget_1ref
  (JNIEnv *env, jclass clss, jlong obj_id)
{
    int retVal = -1;
    retVal = H5Iget_ref( (hid_t)obj_id);
    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jint)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Iinc_ref
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Iinc_1ref
  (JNIEnv *env, jclass clss, jlong obj_id)
{
    int retVal = -1;
    retVal = H5Iinc_ref( (hid_t)obj_id);
    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jint)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Idec_1ref
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Idec_1ref
  (JNIEnv *env, jclass clss, jlong obj_id)
{
    int retVal = -1;
    retVal = H5Idec_ref( (hid_t)obj_id);
    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jint)retVal;
}



/**********************************************************************
 *                                                                    *
 *          New functions release 1.6.3 versus release 1.6.2          *
 *                                                                    *
 **********************************************************************/

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Iget_file_id
 * Signature: (J)J
 */

JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Iget_1file_1id
  (JNIEnv *env, jclass clss, jlong obj_id)
{
    hid_t file_id = 0;

    file_id = H5Iget_file_id ((hid_t) obj_id);

    if (file_id < 0) {
        h5libraryError(env);
    }

    return (jlong) file_id;
}

/**********************************************************************
 *                                                                    *
 *          New functions release 1.8.0                               *
 *                                                                    *
 **********************************************************************/

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Iget_type_ref
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Iget_1type_1ref
  (JNIEnv *env, jclass clss, jint type)
{
  int retVal;

  retVal = H5Iget_type_ref((H5I_type_t)type);

  
  if (retVal <0){
    h5libraryError(env);
  }

  return (jint)retVal;

}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Inmembers
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Inmembers
  (JNIEnv *env, jclass clss, jint type)
{
   herr_t retVal;
   hsize_t num_members;

   retVal = H5Inmembers((H5I_type_t)type, &num_members);

   if (retVal <0){
    h5libraryError(env);
  }

   return (jint)num_members;

}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Iis_valid
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Iis_1valid
(JNIEnv *env, jclass clss, jlong obj_id)
{
    htri_t   bval = 0;

    bval = H5Iis_valid((hid_t)obj_id);

    if (bval > 0) {
        return JNI_TRUE;
    }
    else if (bval == 0) {
        return JNI_FALSE;
    }
    else {
        h5libraryError(env);
        return JNI_FALSE;
    }
}
/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Itype_exists
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Itype_1xists
(JNIEnv *env, jclass clss, jint type)
{
    htri_t   bval = 0;

    bval = H5Itype_exists((H5I_type_t)type);

    if (bval > 0) {
        return JNI_TRUE;
    }
    else if (bval == 0) {
        return JNI_FALSE;
    }
    else {
        h5libraryError(env);
        return JNI_FALSE;
    }
}

#ifdef __cplusplus
}
#endif
