/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

/*
 * This file contains public declarations for the H5S module.
 */
#ifndef _H5Rpublic_H
#define _H5Rpublic_H

/* Public headers needed by this file */
#include <H5public.h>
#include <H5Ipublic.h>

/*
 * Reference types allowed.
 */
typedef enum {
    H5R_BADTYPE     =   (-1),   /* invalid Reference Type                     */
    H5R_OBJECT,                 /* Object reference                           */
    H5R_DATASET_REGION,         /* Dataset Region Reference                   */
    H5R_INTERNAL,               /* Internal Reference                         */
    H5R_MAXTYPE                 /* highest type in group (Invalid as true type)*/
} H5R_type_t;

/* Reference structure for user's code */
typedef struct {
    H5R_type_t  type;           /* Type of reference information in union */
    unsigned long objno[2];     /* OID of object referenced */
    struct H5F_t *file;         /* Pointer to the file the reference is in */
    union {     /* union to hold structs */
        struct {
        } obj;                  /* Object reference structure */
        struct {
        } dreg;                 /* Dataset Region reference structure */
        struct {
        } intrl;                /* Internal reference structure */
    }u;
} href_t;

/* Publicly visible datastructures */

#ifdef __cplusplus
extern "C" {
#endif

/* Functions in H5R.c */
herr_t H5Rcreate(href_t *ref, hid_t loc_id, const char *name,
        H5R_type_t ref_type, hid_t space_id);
hid_t H5Rdereference(href_t *ref);
hid_t H5Rget_space(href_t *ref);
H5R_type_t H5Rget_type(href_t *ref);

#ifdef __cplusplus
}
#endif

#endif  /* _H5Rpublic_H */
