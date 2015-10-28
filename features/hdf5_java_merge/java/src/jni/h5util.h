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

#ifndef H5UTIL_H__
#define H5UTIL_H__

#ifndef SUCCEED
#define SUCCEED     0
#endif

#ifndef FAIL
#define FAIL        (-1)
#endif

typedef struct h5str_t {
    char    *s;
    size_t   max;  /* the allocated size of the string */
} h5str_t;

void    h5str_new (h5str_t *str, size_t len);
void    h5str_free (h5str_t *str);
void    h5str_resize (h5str_t *str, size_t new_len);
char*   h5str_append (h5str_t *str, const char* cstr);
int     h5str_sprintf(h5str_t *str, hid_t container, hid_t tid, void *buf, int expand_data);
void    h5str_array_free(char **strs, size_t len);
int     h5str_dump_simple_dset(FILE *stream, hid_t dset, int binary_order);
int     h5str_dump_region_blocks_data(h5str_t *str, hid_t region, hid_t region_obj);
int     h5str_dump_region_points_data(h5str_t *str, hid_t region, hid_t region_obj);

#endif  /* H5UTIL_H__ */
