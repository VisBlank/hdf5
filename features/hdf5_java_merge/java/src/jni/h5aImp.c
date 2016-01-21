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

#include "hdf5.h"
#include "h5util.h"
#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include "h5jni.h"
#include "h5aImp.h"

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

static herr_t H5A_iterate_cb(hid_t g_id, const char *name, const H5A_info_t *info, void *op_data);


#ifdef H5_HAVE_WIN32_API
  #define strtoll(S,R,N)     _strtoi64(S,R,N)
  #define strtoull(S,R,N)    _strtoui64(S,R,N)
  #define strtof(S,R)    atof(S)
#endif /* H5_HAVE_WIN32_API */

static herr_t H5AreadVL_str  (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf);
static herr_t H5AreadVL_num  (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf);
static herr_t H5AreadVL_comp (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf);

static herr_t H5AwriteVL_str  (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf);
static herr_t H5AwriteVL_num  (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf);
static herr_t H5AwriteVL_comp (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf);

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Acreate
 * Signature: (JLjava/lang/String;JJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Acreate
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong type_id,
          jlong space_id, jlong create_plist)
{
    hid_t       status;
    const char *aName;

    PIN_JAVA_STRING(name, aName, -1);

    status = H5Acreate2((hid_t)loc_id, aName, (hid_t)type_id,
        (hid_t)space_id, (hid_t)create_plist, (hid_t)H5P_DEFAULT);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, aName);

    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aopen_name
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1name
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name)
{
    hid_t       status;
    const char *aName;

    PIN_JAVA_STRING(name, aName, -1);

    status = H5Aopen_name((hid_t)loc_id, aName);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name,aName);

    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aopen_idx
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1idx
  (JNIEnv *env, jclass clss, jlong loc_id, jint idx)
{
    hid_t retVal = -1;
    retVal =  H5Aopen_idx((hid_t)loc_id, (unsigned int) idx);
    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jlong)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Awrite
 * Signature: (JJ[B)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Awrite
  (JNIEnv *env, jclass clss, jlong attr_id, jlong mem_type_id, jbyteArray buf)
{
    herr_t   status;
    jbyte   *byteP;
    jboolean isCopy;

    if (buf == NULL) {
        h5nullArgument( env,"H5Awrite:  buf is NULL");
        return -1;
    }

    byteP = ENVPTR->GetByteArrayElements(ENVPAR buf, &isCopy);

    if (byteP == NULL) {
        h5JNIFatalError(env,"H5Awrite: buf is not pinned");
        return -1;
    }
    status = H5Awrite((hid_t)attr_id, (hid_t)mem_type_id, byteP);

    ENVPTR->ReleaseByteArrayElements(ENVPAR buf, byteP, JNI_ABORT);

    if (status < 0) {
        h5libraryError(env);
    }
    return (jint)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5AwriteVL
 * Signature: (JJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5AwriteVL
  (JNIEnv *env, jclass clss, jlong attr_id, jlong mem_type_id, jobjectArray buf)
{
    herr_t status;

    if (buf == NULL) {
        h5nullArgument( env,"H5AwriteVL:  buf is NULL");
        return -1;
    }

    if (H5Tis_variable_str((hid_t)mem_type_id) > 0) {
        status = H5AwriteVL_str (env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }
    else if (H5Tget_class((hid_t)mem_type_id) == H5T_COMPOUND) {
        status = H5AwriteVL_comp (env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }
    else if (H5Tget_class((hid_t)mem_type_id) == H5T_ARRAY) {
        status = H5AwriteVL_comp (env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }
    else {
        status = H5AwriteVL_num (env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }

    return (jint)status;
}

static
herr_t H5AwriteVL_num (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf)
{
    herr_t  status = -1;
    int     n;
    hvl_t  *wdata = NULL;
    jint    i;
    unsigned char   tmp_uchar = 0;
    char            tmp_char = 0;
    unsigned short  tmp_ushort = 0;
    short           tmp_short = 0;
    unsigned int    tmp_uint = 0;
    int             tmp_int = 0;
    unsigned long   tmp_ulong = 0;
    long            tmp_long = 0;
    unsigned long long tmp_ullong = 0;
    long long       tmp_llong = 0;
    float           tmp_float = 0.0;
    double          tmp_double = 0.0;
    long double     tmp_ldouble = 0.0;
    H5T_class_t     tclass = H5Tget_class(tid);
    size_t          size = H5Tget_size(tid);
    H5T_sign_t      nsign = H5Tget_sign(tid);
    hid_t           basetid = -1;
    hid_t           basesid = -1;
    H5T_class_t     basetclass = -1;
    char           *temp;
    char           *token;

    if(tclass == H5T_VLEN) {
        basetid = H5Tget_super(tid);
        size = H5Tget_size(basetid);
        basetclass = H5Tget_class(basetid);
    }
    n = ENVPTR->GetArrayLength(ENVPAR (jarray)buf);

    wdata = (hvl_t *)HDcalloc((size_t)(n+1), sizeof(hvl_t));
    if (!wdata) {
        h5JNIFatalError(env, "H5AwriteVL_str:  cannot allocate buffer");
        return -1;
    }
    for (i = 0; i < n; i++) {
        int m;

        jstring obj = (jstring) ENVPTR->GetObjectArrayElement(ENVPAR (jobjectArray) buf, i);
        if (obj != 0) {
            jsize length = ENVPTR->GetStringUTFLength(ENVPAR obj);
            const char *utf8 = ENVPTR->GetStringUTFChars(ENVPAR obj, 0);
            temp = HDmalloc((size_t)length+1);
            HDstrncpy(temp, utf8, (size_t)length);
            temp[length] = '\0';
            token = HDstrtok(temp, ",");
            m = 1;
            while (1) {
                token = HDstrtok (NULL, ",");
                if (token == NULL)
                    break;
                m++;
            }
            wdata[i].p = HDmalloc((size_t)m * size);
            wdata[i].len = (size_t)m;

            HDstrncpy(temp, utf8, (size_t)length);
            temp[length] = '\0';
            switch (tclass) {
                case H5T_FLOAT:
                    if (sizeof(float) == size) {
                        m = 0;
                        tmp_float = strtof(HDstrtok(temp, ","), NULL);
                        ((float *)wdata[i].p)[m++] = tmp_float;

                        while (1) {
                            token = HDstrtok (NULL, ",");
                            if (token == NULL)
                                break;
                            if (token[0] == ' ')
                                token++;
                            tmp_float = strtof(token, NULL);
                            ((float *)wdata[i].p)[m++] = tmp_float;
                        }
                    }
                    else if (sizeof(double) == size) {
                        m = 0;
                        tmp_double = HDstrtod(HDstrtok(temp, ","), NULL);
                        ((double *)wdata[i].p)[m++] = tmp_double;

                        while (1) {
                            token = HDstrtok (NULL, ",");
                            if (token == NULL)
                                break;
                            if (token[0] == ' ')
                                token++;
                            tmp_double = HDstrtod(token, NULL);
                            ((double *)wdata[i].p)[m++] = tmp_double;
                        }
                    }
#if H5_SIZEOF_LONG_DOUBLE !=0
                    else if (sizeof(long double) == size) {
                        m = 0;
                        tmp_ldouble = strtold(HDstrtok(temp, ","), NULL);
                        ((long double *)wdata[i].p)[m++] = tmp_ldouble;

                        while (1) {
                            token = HDstrtok (NULL, ",");
                            if (token == NULL)
                                break;
                            if (token[0] == ' ')
                                token++;
                            tmp_ldouble = strtold(token, NULL);
                            ((long double *)wdata[i].p)[m++] = tmp_ldouble;
                        }
                   }
#endif
                    break;
                case H5T_INTEGER:
                    if (sizeof(char) == size) {
                        if(H5T_SGN_NONE == nsign) {
                            m = 0;
                            tmp_uchar = (unsigned char)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                            ((unsigned char *)wdata[i].p)[m++] = tmp_uchar;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_uchar = (unsigned char)HDstrtoul(token, NULL, 10);
                                ((unsigned char *)wdata[i].p)[m++] = tmp_uchar;
                            }
                        }
                        else {
                            m = 0;
                            tmp_char = (char)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                            ((char *)wdata[i].p)[m++] = tmp_char;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_char = (char)HDstrtoul(token, NULL, 10);
                                ((char *)wdata[i].p)[m++] = tmp_char;
                            }
                        }
                    }
                    else if (sizeof(int) == size) {
                        if(H5T_SGN_NONE == nsign) {
                            m = 0;
                            tmp_uint = (unsigned int)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                            ((unsigned int *)wdata[i].p)[m++] = tmp_uint;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_uint = (unsigned int)HDstrtoul(token, NULL, 10);
                                ((unsigned int *)wdata[i].p)[m++] = tmp_uint;
                            }
                        }
                        else {
                            m = 0;
                            tmp_int = (int)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                            ((int *)wdata[i].p)[m++] = tmp_int;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_int = (int)HDstrtoul(token, NULL, 10);
                                ((int *)wdata[i].p)[m++] = tmp_int;
                            }
                        }
                    }
                    else if (sizeof(short) == size) {
                        if(H5T_SGN_NONE == nsign) {
                            m = 0;
                            tmp_ushort = (unsigned short)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                            ((unsigned short *)wdata[i].p)[m++] = tmp_ushort;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_ushort = (unsigned short)HDstrtoul(token, NULL, 10);
                                ((unsigned short *)wdata[i].p)[m++] = tmp_ushort;
                            }
                        }
                        else {
                            m = 0;
                            tmp_short = (short)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                            ((short *)wdata[i].p)[m++] = tmp_short;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_short = (short)HDstrtoul(token, NULL, 10);
                                ((short *)wdata[i].p)[m++] = tmp_short;
                            }
                        }
                    }
                    else if (sizeof(long) == size) {
                        if(H5T_SGN_NONE == nsign) {
                            m = 0;
                            tmp_ulong = HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                            ((unsigned long *)wdata[i].p)[m++] = tmp_ulong;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_ulong = HDstrtoul(token, NULL, 10);
                                ((unsigned long *)wdata[i].p)[m++] = tmp_ulong;
                            }
                        }
                        else {
                            m = 0;
                            tmp_long = HDstrtol(HDstrtok(temp, ","), NULL, 10);
                            ((long *)wdata[i].p)[m++] = tmp_long;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_long = HDstrtol(token, NULL, 10);
                                ((long *)wdata[i].p)[m++] = tmp_long;
                            }
                        }
                    }
                    else if (sizeof(long long) == size) {
                        if(H5T_SGN_NONE == nsign) {
                            m = 0;
                            tmp_ullong = HDstrtoull(HDstrtok(temp, ","), NULL, 10);
                            ((unsigned long long *)wdata[i].p)[m++] = tmp_ullong;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_ullong = HDstrtoull(token, NULL, 10);
                                ((unsigned long long *)wdata[i].p)[m++] = tmp_ullong;
                            }
                        }
                        else {
                            m = 0;
                            tmp_llong = HDstrtoll(HDstrtok(temp, ","), NULL, 10);
                            ((long long *)wdata[i].p)[m++] = tmp_llong;

                            while (1) {
                                token = HDstrtok (NULL, ",");
                                if (token == NULL)
                                    break;
                                if (token[0] == ' ')
                                    token++;
                                tmp_llong = HDstrtoll(token, NULL, 10);
                                ((long long *)wdata[i].p)[m++] = tmp_llong;
                            }
                       }
                    }
                    break;
                case H5T_STRING:
                    {
                    }
                    break;
                case H5T_COMPOUND:
                    {
                    }
                    break;
                case H5T_ENUM:
                    {
                    }
                    break;
                case H5T_REFERENCE:
                    {
                    }
                    break;
                case H5T_ARRAY:
                    {
                    }
                    break;
                case H5T_VLEN:
                    {
                        switch (basetclass) {
                        case H5T_FLOAT:
                            if (sizeof(float) == size) {
                                m = 0;
                                tmp_float = strtof(HDstrtok(temp, ","), NULL);
                                ((float *)wdata[i].p)[m++] = tmp_float;

                                while (1) {
                                    token = HDstrtok (NULL, ",");
                                    if (token == NULL)
                                        break;
                                    if (token[0] == ' ')
                                        token++;
                                    tmp_float = strtof(token, NULL);
                                    ((float *)wdata[i].p)[m++] = tmp_float;
                                }
                            }
                            else if (sizeof(double) == size) {
                                m = 0;
                                tmp_double = HDstrtod(HDstrtok(temp, ","), NULL);
                                ((double *)wdata[i].p)[m++] = tmp_double;

                                while (1) {
                                    token = HDstrtok (NULL, ",");
                                    if (token == NULL)
                                        break;
                                    if (token[0] == ' ')
                                        token++;
                                    tmp_double = HDstrtod(token, NULL);
                                    ((double *)wdata[i].p)[m++] = tmp_double;
                                }
                            }
        #if H5_SIZEOF_LONG_DOUBLE !=0
                            else if (sizeof(long double) == size) {
                                m = 0;
                                tmp_ldouble = strtold(HDstrtok(temp, ","), NULL);
                                ((long double *)wdata[i].p)[m++] = tmp_ldouble;

                                while (1) {
                                    token = HDstrtok (NULL, ",");
                                    if (token == NULL)
                                        break;
                                    if (token[0] == ' ')
                                        token++;
                                    tmp_ldouble = strtold(token, NULL);
                                    ((long double *)wdata[i].p)[m++] = tmp_ldouble;
                                }
                           }
        #endif
                            break;
                        case H5T_INTEGER:
                            if (sizeof(char) == size) {
                                if(H5T_SGN_NONE == nsign) {
                                    m = 0;
                                    tmp_uchar = (unsigned char)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                                    ((unsigned char *)wdata[i].p)[m++] = tmp_uchar;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_uchar = (unsigned char)HDstrtoul(token, NULL, 10);
                                        ((unsigned char *)wdata[i].p)[m++] = tmp_uchar;
                                    }
                                }
                                else {
                                    m = 0;
                                    tmp_char = (char)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                                    ((char *)wdata[i].p)[m++] = tmp_char;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_char = (char)HDstrtoul(token, NULL, 10);
                                        ((char *)wdata[i].p)[m++] = tmp_char;
                                    }
                                }
                            }
                            else if (sizeof(int) == size) {
                                if(H5T_SGN_NONE == nsign) {
                                    m = 0;
                                    tmp_uint = (unsigned int)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                                    ((unsigned int *)wdata[i].p)[m++] = tmp_uint;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_uint = (unsigned int)HDstrtoul(token, NULL, 10);
                                        ((unsigned int *)wdata[i].p)[m++] = tmp_uint;
                                    }
                                }
                                else {
                                    m = 0;
                                    tmp_int = (int)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                                    ((int *)wdata[i].p)[m++] = tmp_int;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_int = (int)HDstrtoul(token, NULL, 10);
                                        ((int *)wdata[i].p)[m++] = tmp_int;
                                    }
                                }
                            }
                            else if (sizeof(short) == size) {
                                if(H5T_SGN_NONE == nsign) {
                                    m = 0;
                                    tmp_ushort = (unsigned short)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                                    ((unsigned short *)wdata[i].p)[m++] = tmp_ushort;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        tmp_ushort = (unsigned short)HDstrtoul(token, NULL, 10);
                                        ((unsigned short *)wdata[i].p)[m++] = tmp_ushort;
                                    }
                                }
                                else {
                                    m = 0;
                                    tmp_short = (short)HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                                    ((short *)wdata[i].p)[m++] = tmp_short;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        tmp_short = (short)HDstrtoul(token, NULL, 10);
                                        ((short *)wdata[i].p)[m++] = tmp_short;
                                    }
                                }
                            }
                            else if (sizeof(long) == size) {
                                if(H5T_SGN_NONE == nsign) {
                                    m = 0;
                                    tmp_ulong = HDstrtoul(HDstrtok(temp, ","), NULL, 10);
                                    ((unsigned long *)wdata[i].p)[m++] = tmp_ulong;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_ulong = HDstrtoul(token, NULL, 10);
                                        ((unsigned long *)wdata[i].p)[m++] = tmp_ulong;
                                    }
                                }
                                else {
                                    m = 0;
                                    tmp_long = HDstrtol(HDstrtok(temp, ","), NULL, 10);
                                    ((long *)wdata[i].p)[m++] = tmp_long;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_long = HDstrtol(token, NULL, 10);
                                        ((long *)wdata[i].p)[m++] = tmp_long;
                                    }
                                }
                            }
                            else if (sizeof(long long) == size) {
                                if(H5T_SGN_NONE == nsign) {
                                    m = 0;
                                    tmp_ullong = HDstrtoull(HDstrtok(temp, ","), NULL, 10);
                                    ((unsigned long long *)wdata[i].p)[m++] = tmp_ullong;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_ullong = HDstrtoull(token, NULL, 10);
                                        ((unsigned long long *)wdata[i].p)[m++] = tmp_ullong;
                                    }
                                }
                                else {
                                    m = 0;
                                    tmp_llong = HDstrtoll(HDstrtok(temp, ","), NULL, 10);
                                    ((long long *)wdata[i].p)[m++] = tmp_llong;

                                    while (1) {
                                        token = HDstrtok (NULL, ",");
                                        if (token == NULL)
                                            break;
                                        if (token[0] == ' ')
                                            token++;
                                        tmp_llong = HDstrtoll(token, NULL, 10);
                                        ((long long *)wdata[i].p)[m++] = tmp_llong;
                                    }
                               }
                            }
                            break;
                        }
                    }
                    break;
            } /* end switch */

        }
    }

    status = H5Awrite((hid_t)aid, (hid_t)tid, wdata);

    for (i = 0; i < n; i++) {
       if(wdata[i].p) {
        HDfree(wdata[i].p);
       }
    }
    HDfree(wdata);

    if (status < 0) {
        h5libraryError(env);
    }

    return status;
}

static
herr_t H5AwriteVL_comp (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf)
{
    herr_t      status;

    h5unimplemented(env, "H5AwriteVL_comp:  not implemented");
    status = -1;

    return status;
}

static
herr_t H5AwriteVL_str (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf)
{
    herr_t  status = -1;
    char  **wdata;
    jsize   size;
    jint    i;

    size = ENVPTR->GetArrayLength(ENVPAR (jarray) buf);

    wdata = (char**)HDcalloc((size_t)size + 1, sizeof(char*));
    if (!wdata) {
        h5JNIFatalError(env, "H5AwriteVL_str:  cannot allocate buffer");
        return -1;
    }

    HDmemset(wdata, 0, (size_t)size * sizeof(char*));
    for (i = 0; i < size; ++i) {
        jstring obj = (jstring) ENVPTR->GetObjectArrayElement(ENVPAR (jobjectArray) buf, i);
        if (obj != 0) {
            jsize length = ENVPTR->GetStringUTFLength(ENVPAR obj);
            const char *utf8 = ENVPTR->GetStringUTFChars(ENVPAR obj, 0);

            if (utf8) {
                wdata[i] = (char*)HDmalloc((size_t)length + 1);
                if (wdata[i]) {
                    HDmemset(wdata[i], 0, ((size_t)length + 1));
                    HDstrncpy(wdata[i], utf8, (size_t)length);
                }
            }

            ENVPTR->ReleaseStringUTFChars(ENVPAR obj, utf8);
            ENVPTR->DeleteLocalRef(ENVPAR obj);
        }
    } /*for (i = 0; i < size; ++i) */

    status = H5Awrite((hid_t)aid, (hid_t)tid, wdata);

    for (i = 0; i < size; i++) {
       if(wdata[i]) {
        HDfree(wdata[i]);
       }
    }
    HDfree(wdata);

    if (status < 0) {
        h5libraryError(env);
    }

    return (jint)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aread
 * Signature: (JJ[B)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Aread
  (JNIEnv *env, jclass clss, jlong attr_id, jlong mem_type_id, jbyteArray buf)
{
    herr_t   status;
    jbyte   *byteP;
    jboolean isCopy;

    if (buf == NULL) {
        h5nullArgument( env,"H5Aread:  buf is NULL");
        return -1;
    }

    byteP = ENVPTR->GetByteArrayElements(ENVPAR buf, &isCopy);

    if (byteP == NULL) {
        h5JNIFatalError( env,"H5Aread: buf is not pinned");
        return -1;
    }

    status = H5Aread((hid_t)attr_id, (hid_t)mem_type_id, byteP);

    if (status < 0) {
        ENVPTR->ReleaseByteArrayElements(ENVPAR buf, byteP, JNI_ABORT);
        h5libraryError(env);
    }
    else  {
        ENVPTR->ReleaseByteArrayElements(ENVPAR buf, byteP, 0);
    }

    return (jint)status;

}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_space
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aget_1space
  (JNIEnv *env, jclass clss, jlong attr_id)
{
    hid_t retVal = -1;
    retVal = H5Aget_space((hid_t)attr_id);
    if (retVal < 0) {
        /* throw exception */
        h5libraryError(env);
    }
    return (jlong)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_type
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aget_1type
  (JNIEnv *env, jclass clss, jlong attr_id)
{
    hid_t retVal = -1;
    retVal = H5Aget_type((hid_t)attr_id);
    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jlong)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_name
 * Signature: (JJ[Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Aget_1name
  (JNIEnv *env, jclass clss, jlong attr_id, jlong buf_size, jobjectArray name)
{
    char    *aName;
    jstring  str;
    hssize_t size;
    ssize_t  bs;

    if (buf_size == 0 && name == NULL)
      return (jlong) H5Aget_name((hid_t)attr_id, 0, NULL);

    bs = (ssize_t)buf_size;
    if (bs <= 0) {
        h5badArgument(env, "H5Aget_name:  buf_size <= 0");
        return -1;
    }
    aName = (char*)HDmalloc(sizeof(char) * (size_t)bs);
    if (aName == NULL) {
        h5outOfMemory(env, "H5Aget_name:  malloc failed");
        return -1;
    }
    size = H5Aget_name((hid_t)attr_id, (size_t)buf_size, aName);
    if (size < 0) {
        HDfree(aName);
        h5libraryError(env);
        return -1;
        /*  exception, returns immediately */
    }
    /* successful return -- save the string; */

    str = ENVPTR->NewStringUTF(ENVPAR aName);

    if (str == NULL) {
        HDfree(aName);
        h5JNIFatalError(env,"H5Aget_name:  return string failed");
        return -1;
    }
    HDfree(aName);
    /*  Note: throws ArrayIndexOutOfBoundsException,
        ArrayStoreException */

    ENVPTR->SetObjectArrayElement(ENVPAR name, 0, str);

    return (jlong)size;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_num_attrs
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Aget_1num_1attrs
  (JNIEnv *env, jclass clss, jlong loc_id)
{
    int retVal = -1;
    retVal = H5Aget_num_attrs((hid_t)loc_id);
    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jint)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Adelete
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Adelete
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name)
{
    herr_t      status;
    const char *aName;

    PIN_JAVA_STRING(name, aName, -1);

    status = H5Adelete((hid_t)loc_id, aName);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, aName);

    if (status < 0) {
        h5libraryError(env);
    }
    return (jint)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aclose
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5__1H5Aclose
  (JNIEnv *env, jclass clss, jlong attr_id)
{
    herr_t retVal = 0;

    if (attr_id > 0)
        retVal = H5Aclose((hid_t)attr_id);

    if (retVal < 0) {
        h5libraryError(env);
    }

    return (jint)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5AreadVL
 * Signature: (JJ[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5AreadVL
  (JNIEnv *env, jclass clss, jlong attr_id, jlong mem_type_id, jobjectArray buf)
{
    htri_t isStr;

    if (buf == NULL) {
        h5nullArgument(env, "H5AreadVL:  buf is NULL");
        return -1;
    }

    isStr = H5Tis_variable_str((hid_t)mem_type_id);

    if (H5Tis_variable_str((hid_t)mem_type_id) > 0) {
        return (jint) H5AreadVL_str(env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }
    else if (H5Tget_class((hid_t)mem_type_id) == H5T_COMPOUND) {
        return (jint) H5AreadVL_comp(env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }
    else if (H5Tget_class((hid_t)mem_type_id) == H5T_ARRAY) {
        return (jint) H5AreadVL_comp(env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }
    else {
        return (jint) H5AreadVL_num(env, (hid_t)attr_id, (hid_t)mem_type_id, buf);
    }
}

static
herr_t H5AreadVL_num (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf)
{
    herr_t  status;
    int     i;
    int     n;
    size_t  max_len = 0;
    h5str_t h5str;
    jstring jstr;
    hvl_t  *rdata = NULL;
    size_t  size;
    hid_t   sid;
    hsize_t dims[H5S_MAX_RANK];

    n = ENVPTR->GetArrayLength(ENVPAR buf);
    rdata = (hvl_t *)HDcalloc((size_t)n+1, sizeof(hvl_t));
    if (rdata == NULL) {
        h5JNIFatalError(env, "H5AreadVL_num:  failed to allocate buff for read");
        return -1;
    }

    status = H5Aread(aid, tid, rdata);
    dims[0] = (hsize_t)n;
    sid = H5Screate_simple(1, dims, NULL);
    if (status < 0) {
        H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, rdata);
        H5Sclose(sid);
        HDfree(rdata);
        h5JNIFatalError(env, "H5AreadVL_num: failed to read data");
        return -1;
    }

    for (i = 0; i < n; i++) {
        if ((rdata +i)->len > max_len)
            max_len = (rdata + i)->len;
    }

    size = H5Tget_size(tid);
    HDmemset((void *)&h5str, (int)0, (size_t)sizeof(h5str_t));
    h5str_new(&h5str, 4 * size);

    if (h5str.s == NULL) {
        H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, rdata);
        H5Sclose(sid);
        HDfree(rdata);
        h5JNIFatalError(env, "H5AreadVL_num:  failed to allocate strng buf");
        return -1;
    }

    for (i = 0; i < n; i++) {
        h5str.s[0] = '\0';
        h5str_sprintf(&h5str, aid, tid, rdata + i, 0);
        jstr = ENVPTR->NewStringUTF(ENVPAR h5str.s);
        ENVPTR->SetObjectArrayElement(ENVPAR buf, i, jstr);
    }

    h5str_free(&h5str);
    H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, rdata);
    H5Sclose(sid);

    HDfree(rdata);

    return status;
}

static
herr_t H5AreadVL_comp (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf)
{
    herr_t      status;
    int         i;
    int         n;
    size_t      max_len = 0;
    h5str_t     h5str;
    jstring     jstr;
    char       *rdata;
    size_t      size;
    hid_t       p_type;

    p_type = H5Tget_native_type(tid, H5T_DIR_DEFAULT);
    size = (((H5Tget_size(tid))>(H5Tget_size(p_type))) ? (H5Tget_size(tid)) : (H5Tget_size(p_type)));
    H5Tclose(p_type);
    n = ENVPTR->GetArrayLength(ENVPAR buf);
    rdata = (char *)HDmalloc((size_t)n * size);

    if (rdata == NULL) {
        h5JNIFatalError(env, "H5AreadVL_comp:  failed to allocate buff for read");
        return -1;
    }

    status = H5Aread(aid, tid, rdata);

    if (status < 0) {
        HDfree(rdata);
        h5JNIFatalError(env, "H5AreadVL_comp: failed to read data");
        return -1;
    }

    HDmemset(&h5str, 0, sizeof(h5str_t));
    h5str_new(&h5str, 4 * size);

    if (h5str.s == NULL) {
        HDfree(rdata);
        h5JNIFatalError(env, "H5AreadVL_comp:  failed to allocate string buf");
        return -1;
    }

    for (i = 0; i < n; i++) {
        h5str.s[0] = '\0';
        h5str_sprintf(&h5str, aid, tid, rdata + ((size_t)i * size), 0);
        jstr = ENVPTR->NewStringUTF(ENVPAR h5str.s);
        ENVPTR->SetObjectArrayElement(ENVPAR buf, i, jstr);
    }

    h5str_free(&h5str);

    HDfree(rdata);

    return status;
}

static
herr_t H5AreadVL_str (JNIEnv *env, hid_t aid, hid_t tid, jobjectArray buf)
{
    herr_t  status=-1;
    jstring jstr;
    char  **strs;
    int     i, n;
    hid_t   sid;
    hsize_t dims[H5S_MAX_RANK];

    n = ENVPTR->GetArrayLength(ENVPAR buf);

    strs =(char **)HDmalloc((size_t)n * sizeof(char *));
    if (strs == NULL) {
        h5JNIFatalError( env, "H5AreadVL_str:  failed to allocate buff for read variable length strings");
        return -1;
    }

    status = H5Aread(aid, tid, strs);
    if (status < 0) {
        dims[0] = (hsize_t)n;
        sid = H5Screate_simple(1, dims, NULL);
        H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, strs);
        H5Sclose(sid);
        HDfree(strs);
        h5JNIFatalError(env, "H5AreadVL_str: failed to read variable length strings");
        return -1;
    }

    for (i=0; i<n; i++) {
        jstr = ENVPTR->NewStringUTF(ENVPAR strs[i]);
        ENVPTR->SetObjectArrayElement(ENVPAR buf, i, jstr);
        HDfree (strs[i]);
    }

    /*
    for repeatedly reading an attribute with a large number of strs (e.g., 1,000,000 strings,
    H5Dvlen_reclaim() may crash on Windows because the Java GC will not be able to collect
    free space in time. Instead, use "free(strs[i])" to free individual strings
    after it is done.
    H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, strs);
    */

    if (strs)
        HDfree(strs);

    return status;
}

/*
 * Copies the content of one dataset to another dataset
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Acopy
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Acopy
  (JNIEnv *env, jclass clss, jlong src_id, jlong dst_id)
{
    jbyte  *buf;
    herr_t  retVal = -1;
    hid_t   src_did = (hid_t)src_id;
    hid_t   dst_did = (hid_t)dst_id;
    hid_t   tid = -1;
    hid_t   sid = -1;
    hsize_t total_size = 0;


    sid = H5Aget_space(src_did);
    if (sid < 0) {
        h5libraryError(env);
        return -1;
    }

    tid = H5Aget_type(src_did);
    if (tid < 0) {
        H5Sclose(sid);
        h5libraryError(env);
        return -1;
    }

    total_size = (hsize_t)H5Sget_simple_extent_npoints(sid) * (hsize_t)H5Tget_size(tid);

    H5Sclose(sid);

    buf = (jbyte *)HDmalloc( (size_t)total_size * sizeof(jbyte));
    if (buf == NULL) {
    H5Tclose(tid);
        h5outOfMemory( env, "H5Acopy:  malloc failed");
        return -1;
    }

    retVal = H5Aread(src_did, tid, buf);
    H5Tclose(tid);

    if (retVal < 0) {
        HDfree(buf);
        h5libraryError(env);
        return (jint)retVal;
    }

    tid = H5Aget_type(dst_did);
    if (tid < 0) {
        HDfree(buf);
        h5libraryError(env);
        return -1;
    }
    retVal = H5Awrite(dst_did, tid, buf);
    H5Tclose(tid);
    HDfree(buf);

    if (retVal < 0) {
        h5libraryError(env);
    }

    return (jint)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Acreate2
 * Signature: (JLjava/lang/String;JJJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Acreate2
(JNIEnv *env, jclass clss, jlong loc_id, jstring name, jlong type_id,
        jlong space_id, jlong create_plist, jlong access_plist)
{
    hid_t       status;
    const char *aName;

    PIN_JAVA_STRING(name, aName, -1);

    status = H5Acreate2((hid_t)loc_id, aName, (hid_t)type_id,
        (hid_t)space_id, (hid_t)create_plist, (hid_t)access_plist );

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, aName);

    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}


/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Aopen
 * Signature: (JLjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen
  (JNIEnv *env, jclass clss, jlong obj_id, jstring name, jlong access_plist)

{
    hid_t       retVal;
    const char *aName;

    PIN_JAVA_STRING(name, aName, -1);

    retVal = H5Aopen((hid_t)obj_id, aName, (hid_t)access_plist);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, aName);

    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jlong)retVal;

}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Aopen_by_idx
 * Signature: (JLjava/lang/String;IIJJJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1by_1idx
  (JNIEnv *env, jclass clss, jlong loc_id, jstring name, jint idx_type, jint order, jlong n, jlong aapl_id, jlong lapl_id)
{
    hid_t       retVal;
    const char *aName;

    PIN_JAVA_STRING(name, aName, -1);

    retVal = H5Aopen_by_idx((hid_t)loc_id, aName, (H5_index_t)idx_type,
            (H5_iter_order_t)order, (hsize_t)n, (hid_t)aapl_id, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, aName);

    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jlong)retVal;
}

/*
* Class:     hdf_hdf5lib_H5
* Method:    _H5Acreate_by_name
* Signature: (JLjava/lang/String;Ljava/lang/String;JJJJJ)J
*/
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Acreate_1by_1name
(JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jstring attr_name, jlong type_id, jlong space_id, jlong acpl_id, jlong aapl_id, jlong lapl_id)
{
    hid_t       retVal;
    const char *aName;
    const char *attrName;

    PIN_JAVA_STRING_TWO(obj_name, aName, attr_name, attrName, -1);

    retVal = H5Acreate_by_name((hid_t)loc_id, aName, attrName, (hid_t)type_id,
            (hid_t)space_id, (hid_t)acpl_id, (hid_t)aapl_id, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR attr_name, attrName);

    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jlong)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aexists_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;J)Z
 */
JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Aexists_1by_1name
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jstring attr_name, jlong lapl_id)
{
    htri_t      retVal;
    const char *aName;
    const char *attrName;

    PIN_JAVA_STRING_TWO(obj_name, aName, attr_name, attrName, JNI_FALSE);

    retVal = H5Aexists_by_name((hid_t)loc_id, aName, attrName, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR attr_name, attrName);

    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jboolean)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Arename
 * Signature: (JLjava/lang/String;Ljava/lang/String)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Arename
  (JNIEnv *env, jclass clss, jlong loc_id, jstring old_attr_name, jstring new_attr_name)
{
    herr_t      retVal;
    const char *oName;
    const char *nName;

    PIN_JAVA_STRING_TWO(old_attr_name, oName, new_attr_name, nName, -1);

    retVal = H5Arename((hid_t)loc_id, oName, nName);

    ENVPTR->ReleaseStringUTFChars(ENVPAR old_attr_name, oName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR new_attr_name, nName);

    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jint)retVal;
}


/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Arename_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Arename_1by_1name
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jstring old_attr_name, jstring new_attr_name, jlong lapl_id)
{
    herr_t      retVal;
    const char *aName;
    const char *oName;
    const char *nName;

    PIN_JAVA_STRING_THREE(obj_name, aName, old_attr_name, oName, new_attr_name, nName, -1);

    retVal = H5Arename_by_name((hid_t)loc_id, aName, oName, nName, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR old_attr_name, oName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR new_attr_name, nName);

    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jint)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_name_by_idx
 * Signature: (JLjava/lang/String;IIJJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Aget_1name_1by_1idx
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jint idx_type, jint order, jlong n, jlong lapl_id)
{
    size_t   buf_size;
    char    *aValue;
    jlong    status_size;
    jstring  str = NULL;
    const char *aName;

    PIN_JAVA_STRING(obj_name, aName, NULL);

    /* get the length of the attribute name */
    status_size = H5Aget_name_by_idx((hid_t)loc_id, aName, (H5_index_t)idx_type,
            (H5_iter_order_t) order, (hsize_t) n, (char*)NULL, (size_t)0, (hid_t)lapl_id);

    if(status_size < 0) {
        ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);
        h5libraryError(env);
        return NULL;
    }
    buf_size = (size_t)status_size + 1;/* add extra space for the null terminator */

    aValue = (char*)HDmalloc(sizeof(char) * buf_size);
    if (aValue == NULL) {
        ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);
        h5outOfMemory(env, "H5Aget_name_by_idx:  malloc failed ");
        return NULL;
    }

    status_size = H5Aget_name_by_idx((hid_t)loc_id, aName, (H5_index_t)idx_type,
            (H5_iter_order_t) order, (hsize_t) n, (char*)aValue, (size_t)buf_size, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);

    if (status_size < 0) {
        HDfree(aValue);
        h5libraryError(env);
        return NULL;
    }
    /* may throw OutOfMemoryError */
    str = ENVPTR->NewStringUTF(ENVPAR aValue);
    if (str == NULL) {
        /* exception -- fatal JNI error */
        HDfree(aValue);
        h5JNIFatalError(env, "H5Aget_name_by_idx:  return string not created");
        return NULL;
    }

    HDfree(aValue);

    return str;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_storage_size
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Aget_1storage_1size
  (JNIEnv *env, jclass clss, jlong attr_id)
{
    hsize_t retVal = (hsize_t)-1;

    retVal = H5Aget_storage_size((hid_t)attr_id);
/* probably returns '0' if fails--don't do an exception
    if (retVal < 0) {
        h5libraryError(env);
    }
*/
    return (jlong)retVal;
}


/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_info
 * Signature: (J)Lhdf/hdf5lib/structs/H5A_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Aget_1info
  (JNIEnv *env, jclass clss, jlong attr_id)
{
    herr_t     status;
    H5A_info_t ainfo;
    jvalue     args[4];
    jobject    ret_obj = NULL;

    status = H5Aget_info((hid_t)attr_id, &ainfo);

    if (status < 0) {
       h5libraryError(env);
       return NULL;
    }

    args[0].z = ainfo.corder_valid;
    args[1].j = ainfo.corder;
    args[2].i = ainfo.cset;
    args[3].j = (jlong)ainfo.data_size;
    CALL_CONSTRUCTOR("hdf/hdf5lib/structs/H5A_info_t", "(ZJIJ)V", args);

    return ret_obj;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_info_by_idx
 * Signature: (JLjava/lang/String;IIJJ)Lhdf/hdf5lib/structs/H5A_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Aget_1info_1by_1idx
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jint idx_type, jint order, jlong n, jlong lapl_id)
{
    herr_t      status;
    H5A_info_t  ainfo;
    jvalue      args[4];
    jobject     ret_obj = NULL;
    const char *aName;

    PIN_JAVA_STRING(obj_name, aName, NULL);

    status = H5Aget_info_by_idx((hid_t)loc_id, aName, (H5_index_t)idx_type,
            (H5_iter_order_t)order, (hsize_t)n, &ainfo, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);

    if (status < 0) {
       h5libraryError(env);
       return NULL;
    }

    args[0].z = ainfo.corder_valid;
    args[1].j = ainfo.corder;
    args[2].i = ainfo.cset;
    args[3].j = (jlong)ainfo.data_size;
    CALL_CONSTRUCTOR("hdf/hdf5lib/structs/H5A_info_t", "(ZJIJ)V", args);

    return ret_obj;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_info_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;J)Lhdf/hdf5lib/structs/H5A_info_t;
 */
JNIEXPORT jobject JNICALL Java_hdf_hdf5lib_H5_H5Aget_1info_1by_1name
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jstring attr_name, jlong lapl_id)
{
    const char *aName;
    const char *attrName;
    herr_t      status;
    H5A_info_t  ainfo;
    jvalue      args[4];
    jobject     ret_obj = NULL;

    PIN_JAVA_STRING_TWO(obj_name, aName, attr_name, attrName, NULL);

    status = H5Aget_info_by_name((hid_t)loc_id, aName, attrName, &ainfo, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR attr_name, attrName);

    if (status < 0) {
       h5libraryError(env);
       return NULL;
    }

    args[0].z = ainfo.corder_valid;
    args[1].j = ainfo.corder;
    args[2].i = ainfo.cset;
    args[3].j = (jlong)ainfo.data_size;
    CALL_CONSTRUCTOR("hdf/hdf5lib/structs/H5A_info_t", "(ZJIJ)V", args);

    return ret_obj;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Adelete_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Adelete_1by_1name
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jstring attr_name, jlong lapl_id)
{
    herr_t      retVal;
    const char *aName;
    const char *attrName;

    PIN_JAVA_STRING_TWO(obj_name, aName, attr_name, attrName, -1);

    retVal = H5Adelete_by_name((hid_t)loc_id, aName, attrName, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR attr_name, attrName);

    if (retVal < 0) {
        h5libraryError(env);
    }
    return (jint)retVal;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aexists
 * Signature: (JLjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Aexists
  (JNIEnv *env, jclass clss, jlong obj_id, jstring attr_name)
{
    htri_t      bval = 0;
    const char *aName;

    PIN_JAVA_STRING(attr_name, aName, JNI_FALSE);

    bval = H5Aexists((hid_t)obj_id, aName);

    ENVPTR->ReleaseStringUTFChars(ENVPAR attr_name, aName);

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
 * Method:    H5Adelete_by_idx
 * Signature: (JLjava/lang/String;IIJJ)V
 */
JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Adelete_1by_1idx
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jint idx_type, jint order, jlong n, jlong lapl_id)
{
    herr_t      status;
    const char *aName;

    PIN_JAVA_STRING0(obj_name, aName);

    status = H5Adelete_by_idx((hid_t)loc_id, aName, (H5_index_t)idx_type, (H5_iter_order_t)order, (hsize_t)n, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, aName);

    if (status < 0) {
        h5libraryError(env);
        return;
    }
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    _H5Aopen_by_name
 * Signature: (JLjava/lang/String;Ljava/lang/String;JJ)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aopen_1by_1name
  (JNIEnv *env, jclass clss, jlong loc_id, jstring obj_name, jstring attr_name, jlong aapl_id, jlong lapl_id)

{
    hid_t       status;
    const char *aName;
    const char *oName;

    PIN_JAVA_STRING_TWO(obj_name, oName, attr_name, aName, -1);

    status = H5Aopen_by_name((hid_t)loc_id, oName, aName, (hid_t)aapl_id, (hid_t)lapl_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR obj_name, oName);
    ENVPTR->ReleaseStringUTFChars(ENVPAR attr_name, aName);

    if (status < 0) {
        h5libraryError(env);
    }
    return (jlong)status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aget_create_plist
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5__1H5Aget_1create_1plist
  (JNIEnv *env, jclass clss, jlong attr_id)
{
    hid_t retVal = -1;
    retVal = H5Aget_create_plist((hid_t)attr_id);
    if (retVal < 0) {
        /* throw exception */
        h5libraryError(env);
    }
    return (jlong)retVal;
}

static
herr_t H5A_iterate_cb(hid_t g_id, const char *name, const H5A_info_t *info, void *op_data) {
    JNIEnv    *cbenv;
    jint       status;
    jclass     cls;
    jmethodID  mid;
    jstring    str;
    jmethodID  constructor;
    jvalue     args[4];
    jobject    cb_info_t = NULL;

    if(JVMPTR->AttachCurrentThread(JVMPAR2 (void**)&cbenv, NULL) != 0) {
        /* printf("JNI H5A_iterate_cb error: AttachCurrentThread failed\n"); */
        JVMPTR->DetachCurrentThread(JVMPAR);
        return -1;
    }
    cls = CBENVPTR->GetObjectClass(CBENVPAR visit_callback);
    if (cls == 0) {
        /* printf("JNI H5A_iterate_cb error: GetObjectClass failed\n"); */
       JVMPTR->DetachCurrentThread(JVMPAR);
       return -1;
    }
    mid = CBENVPTR->GetMethodID(CBENVPAR cls, "callback", "(JLjava/lang/String;Lhdf/hdf5lib/structs/H5A_info_t;Lhdf/hdf5lib/callbacks/H5A_iterate_t;)I");
    if (mid == 0) {
        /* printf("JNI H5A_iterate_cb error: GetMethodID failed\n"); */
        JVMPTR->DetachCurrentThread(JVMPAR);
        return -1;
    }
    str = CBENVPTR->NewStringUTF(CBENVPAR name);

    args[0].z = info->corder_valid;
    args[1].j = info->corder;
    args[2].i = info->cset;
    args[3].j = (jlong)info->data_size;
    /* get a reference to your class if you don't have it already */
    cls = CBENVPTR->FindClass(CBENVPAR "hdf/hdf5lib/structs/H5A_info_t");
    if (cls == 0) {
        /* printf("JNI H5A_iterate_cb error: GetObjectClass info failed\n"); */
       JVMPTR->DetachCurrentThread(JVMPAR);
       return -1;
    }
    /* get a reference to the constructor; the name is <init> */
    constructor = CBENVPTR->GetMethodID(CBENVPAR cls, "<init>", "(ZJIJ)V");
    if (constructor == 0) {
        /* printf("JNI H5A_iterate_cb error: GetMethodID constructor failed\n"); */
        JVMPTR->DetachCurrentThread(JVMPAR);
        return -1;
    }
    cb_info_t = CBENVPTR->NewObjectA(CBENVPAR cls, constructor, args);

    status = CBENVPTR->CallIntMethod(CBENVPAR visit_callback, mid, g_id, str, cb_info_t, op_data);

    JVMPTR->DetachCurrentThread(JVMPAR);
    return status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aiterate
 * Signature: (JIIJLjava/lang/Object;Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Aiterate
  (JNIEnv *env, jclass clss, jlong grp_id, jint idx_type, jint order,
          jlong idx, jobject callback_op, jobject op_data)
{
    hsize_t       start_idx = (hsize_t)idx;
    herr_t        status = -1;

    ENVPTR->GetJavaVM(ENVPAR &jvm);
    visit_callback = callback_op;

    if (op_data == NULL) {
        h5nullArgument(env,  "H5Aiterate:  op_data is NULL");
        return -1;
    }
    if (callback_op == NULL) {
        h5nullArgument(env,  "H5Aiterate:  callback_op is NULL");
        return -1;
    }

    status = H5Aiterate2((hid_t)grp_id, (H5_index_t)idx_type, (H5_iter_order_t)order, (hsize_t*)&start_idx, (H5A_operator2_t)H5A_iterate_cb, (void*)op_data);

    if (status < 0) {
       h5libraryError(env);
       return status;
    }

    return status;
}

/*
 * Class:     hdf_hdf5lib_H5
 * Method:    H5Aiterate_by_name
 * Signature: (JLjava/lang/String;IIJLjava/lang/Object;Ljava/lang/Object;J)I
 */
JNIEXPORT jint JNICALL Java_hdf_hdf5lib_H5_H5Aiterate_1by_1name
  (JNIEnv *env, jclass clss, jlong grp_id, jstring name, jint idx_type, jint order,
          jlong idx, jobject callback_op, jobject op_data, jlong access_id)
{
    const char   *lName;
    hsize_t       start_idx = (hsize_t)idx;
    herr_t        status = -1;

    ENVPTR->GetJavaVM(ENVPAR &jvm);
    visit_callback = callback_op;

    PIN_JAVA_STRING(name, lName, -1);

    if (op_data == NULL) {
        h5nullArgument(env,  "H5Aiterate_by_name:  op_data is NULL");
        return -1;
    }
    if (callback_op == NULL) {
        h5nullArgument(env,  "H5Literate_by_name:  callback_op is NULL");
        return -1;
    }

    status = H5Aiterate_by_name((hid_t)grp_id, lName, (H5_index_t)idx_type, (H5_iter_order_t)order, (hsize_t*)&start_idx, (H5A_operator2_t)H5A_iterate_cb, (void*)op_data, (hid_t)access_id);

    ENVPTR->ReleaseStringUTFChars(ENVPAR name, lName);

    if (status < 0) {
       h5libraryError(env);
       return status;
    }

    return status;
}

#ifdef __cplusplus
}
#endif
