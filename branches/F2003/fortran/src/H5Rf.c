/****h* H5Rf/H5Rf
 * PURPOSE
 *   This file contains C stubs for H5R Fortran APIs
 *
 * COPYRIGHT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 ******
*/

#include "H5f90.h"

/****if* H5Rf/h5rcreate_object_c
 * NAME
 *     h5rcreate_object_c
 * PURPOSE
 *     Call H5Rcreate to create a reference to an object
 * INPUTS
 *     loc_id - file or group identifier
 *       name - name of the dataset
 *    namelen - name length
 * OUTPUTS
 *     ref  - reference to the object
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  Elena Pourmal
 *              Wednesday, December 1, 1999
 * SOURCE
*/
int_f
nh5rcreate_object_c (haddr_t_f *ref, hid_t_f *loc_id, _fcd name, int_f *namelen)
/******/
{
     int ret_value = -1;
     hid_t c_loc_id;
     int ret_value_c;
     char *c_name;
     size_t c_namelen;
     hobj_ref_t ref_c;

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen);
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Rcreate function.
      */
     c_loc_id = *loc_id;
     ret_value_c = H5Rcreate(&ref_c, c_loc_id, c_name, H5R_OBJECT, -1);

     HDfree(c_name);
     if (ret_value_c >= 0)  {
         *ref=(haddr_t_f)ref_c;
         ret_value = 0;
     }

     return ret_value;
}

/****if* H5Rf/h5rcreate_region_c
 * NAME
 *        h5rcreate_region_c
 * PURPOSE
 *     Call H5Rcreate to create a reference to dataset region
 *              region
 * INPUTS
 *      loc_id - file or group identifier
 *              name - name of the dataset
 *              namelen - name length
 *              space_id - dataset space identifier
 * OUTPUTS
 *     ref  - reference to the dataset region
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  Elena Pourmal
 *              Wednesday, December 1, 1999
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rcreate_region_c (int_f *ref, hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *space_id)
/******/
{
     int ret_value = -1;
     hid_t c_loc_id;
     hid_t c_space_id;
     int ret_value_c;
     char *c_name;
     size_t c_namelen;
     hdset_reg_ref_t ref_c;

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen);
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Rcreate function.
      */
     c_loc_id = *loc_id;
     c_space_id = *space_id;
     ret_value_c = H5Rcreate(&ref_c, c_loc_id, c_name, H5R_DATASET_REGION, c_space_id);

     HDfree(c_name);
     if (ret_value_c >= 0) {
         HDmemcpy (ref, &ref_c, H5R_DSET_REG_REF_BUF_SIZE);
         ret_value = 0;
     }
     return ret_value;
}

/****if* H5Rf/h5rcreate_ptr_c
 * NAME
 *  h5rcreate_ptr_c
 * PURPOSE
 *  Call H5Rcreate to create a reference to dataset region
 * INPUTS
 *  loc_id    - file or group identifier
 *  name      - name of the dataset
 *  namelen   - name length
 *  space_id  - dataset space identifier
 * OUTPUTS
 *  ref       - reference to the dataset region
 * RETURNS
 *  0 on success, -1 on failure
 * AUTHOR
 *  M.S. Breitenfeld
 *  June 20, 2008
 *
 * SOURCE
*/
int_f
nh5rcreate_ptr_c (void *ref, hid_t_f *loc_id, _fcd name, int_f *namelen, int_f *ref_type, hid_t_f *space_id)
/******/
{
     int ret_value = -1;
     char *c_name;
     size_t c_namelen;

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen);
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Rcreate function.
      */
     if(H5Rcreate(ref, (hid_t)*loc_id, c_name, (H5R_type_t)*ref_type, (hid_t)*space_id) >= 0)
       ret_value = 0;

     HDfree(c_name);
     return ret_value;
}

/****if* H5Rf/h5rdereference_region_c
 * NAME
 *        h5rdereference_region_c
 * PURPOSE
 *     Call H5Rdereference to dereference to dataset region
 * INPUTS
 *      dset_id - dataset identifier
 *              ref - reference to the dataset region
 * OUTPUTS
 *     obj_id - dereferenced dataset identifier
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  Elena Pourmal
 *              Wednesday, December 1, 1999
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rdereference_region_c (hid_t_f *dset_id, int_f *ref, hid_t_f *obj_id)
/******/
{
     int ret_value = -1;
     hid_t c_dset_id;
     hdset_reg_ref_t ref_c;
     hid_t c_obj_id;

     HDmemcpy (&ref_c, ref, H5R_DSET_REG_REF_BUF_SIZE);

     /*
      * Call H5Rdereference function.
      */
     c_dset_id = *dset_id;
     c_obj_id = H5Rdereference(c_dset_id, H5R_DATASET_REGION, &ref_c);
     if(c_obj_id < 0) return ret_value;
     *obj_id = (hid_t_f)c_obj_id;
     ret_value = 0;
     return ret_value;
}


/****if* H5Rf/h5rdereference_object_c
 * NAME
 *        h5rdereference_object_c
 * PURPOSE
 *     Call H5Rdereference to dereference an object
 * INPUTS
 *      dset_id - dataset identifier
 *              ref - reference to an object
 * OUTPUTS
 *     obj_id - dereferenced  object identifier
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  Elena Pourmal
 *              Wednesday, December 1, 1999
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rdereference_object_c (hid_t_f *dset_id, haddr_t_f *ref, hid_t_f *obj_id)
/******/
{
     int ret_value = -1;
     hid_t c_dset_id;
     hid_t c_obj_id;
     hobj_ref_t ref_c;

     ref_c=*ref;

     /*
      * Call H5Rdereference function.
      */
     c_dset_id = *dset_id;
     c_obj_id = H5Rdereference(c_dset_id, H5R_OBJECT, &ref_c);
     if(c_obj_id < 0) return ret_value;
     *obj_id = (hid_t_f)c_obj_id;
     ret_value = 0;
     return ret_value;
}

/****if* H5Rf/h5rdereference_ptr_c
 * NAME
 *        h5rdereference_ptr_c
 * PURPOSE
 *     Call H5Rdereference
 * INPUTS
 *      obj_id - Valid identifier for the file containing the
 *                       referenced object or any object in that file.
 *             ref_typ - The reference type of ref.
 *                 ref - Object reference
 * OUTPUTS
 *     ref_obj_id - Identifier of referenced object
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  M.S. Breitenfeld
 *              June 20, 2008
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rdereference_ptr_c (hid_t_f *obj_id, int_f *ref_type, void *ref, hid_t_f *ref_obj_id)
/******/
{
  int ret_value = -1;
  hid_t c_ref_obj_id;

  /*
   * Call H5Rdereference function.
   */
  c_ref_obj_id = H5Rdereference((hid_t)*obj_id, (H5R_type_t)*ref_type, ref);
  if(c_ref_obj_id < 0) return ret_value;
  *ref_obj_id = (hid_t_f)c_ref_obj_id;
  ret_value = 0;
  return ret_value;
}

/****if* H5Rf/h5rget_region_region_object_c
 * NAME
 *        h5rget_region_region_object_c
 * PURPOSE
 *     Call H5Rget_region to dereference dataspace region
 * INPUTS
 *      dset_id - dataset identifier
 *              ref - reference to the dataset region
 * OUTPUTS
 *     space_id - dereferenced  dataset dataspace identifier
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  Elena Pourmal
 *              Wednesday, December 1, 1999
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rget_region_region_c (hid_t_f *dset_id, int_f *ref, hid_t_f *space_id)
/******/
{
     int ret_value = -1;
     hid_t c_dset_id;
     hid_t c_space_id;
     hdset_reg_ref_t ref_c;

     HDmemcpy (&ref_c, ref, H5R_DSET_REG_REF_BUF_SIZE);

     /*
      * Call H5Rget_region function.
      */
     c_dset_id = *dset_id;
     c_space_id = H5Rget_region(c_dset_id, H5R_DATASET_REGION, &ref_c);
     if(c_space_id < 0) return ret_value;
     *space_id = (hid_t_f)c_space_id;
     ret_value = 0;
     return ret_value;
}

/****if* H5Rf/h5rget_object_type_obj_c
 * NAME
 *        h5rget_object_type_obj_c
 * PURPOSE
 *     Call H5Rget_object_type to retrieve the type of the object reference points
 *              to
 * INPUTS
 *      dset_id - dataset identifier
 *              ref - reference to the dataset region
 * OUTPUTS
 *     obj_type - type of dereferenced object
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  Elena Pourmal
 *              Wednesday, December 1, 1999
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rget_object_type_obj_c (hid_t_f *dset_id, haddr_t_f *ref, int_f *obj_type)
/******/
{
     H5O_type_t c_obj_type;
     hobj_ref_t ref_c;
     int_f ret_value = -1;

     ref_c = *ref;

     /*
      * Call H5Rget_object_type function.
      */
     if(H5Rget_obj_type2((hid_t)*dset_id, H5R_OBJECT, &ref_c, &c_obj_type) < 0)
         return ret_value;

     *obj_type = (int_f)c_obj_type;

     ret_value = 0;

     return ret_value;
}

/****if* H5Rf/h5rget_name_object_c
 * NAME
 *        h5rget_name_object_c
 * PURPOSE
 *     Call H5Rget_name for an object
 * INPUTS
 *
 *       loc_id - Identifier for the dataset containing the reference or for the group that dataset is in.
 *          ref - An object or dataset region reference.
 *
 * OUTPUTS
 *     name - A name associated with the referenced object or dataset region.
 *              size - The size of the name buffer.
 *
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  M.S. Breitenfeld
 *              March 31, 2008
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rget_name_object_c (hid_t_f *loc_id, haddr_t_f *ref, _fcd name, size_t_f *name_len, size_t_f *size_default)
/******/
{
     hobj_ref_t ref_c;
     int_f ret_value = -1;
     ssize_t c_size;
     size_t c_bufsize;
     char *c_buf= NULL;  /* Buffer to hold C string */

     ref_c = *ref;

     c_bufsize = (size_t)*name_len+1;
     /*
      * Allocate buffer to hold name of an attribute
      */
     if ((c_buf = HDmalloc(c_bufsize)) == NULL)
       return ret_value;

     /*
      * Call H5Rget_name function.
      */
     if((c_size=H5Rget_name((hid_t)*loc_id, H5R_OBJECT, &ref_c, c_buf, c_bufsize)) < 0)
         return ret_value;
     /*
      * Convert C name to FORTRAN and place it in the given buffer
      */
     HD5packFstring(c_buf, _fcdtocp(name), c_bufsize-1);

     *size_default = (size_t_f)c_size;
     ret_value = 0;
     if(c_buf) HDfree(c_buf);

     return ret_value;
}

/****if* H5Rf/h5rget_name_region_c
 * NAME
 *        h5rget_name_region_c
 * PURPOSE
 *     Call H5Rget_name for a dataset region
 * INPUTS
 *
 *       loc_id - Identifier for the dataset containing the reference or for the group that dataset is in.
 *          ref - An object or dataset region reference.
 *
 * OUTPUTS
 *     name - A name associated with the referenced object or dataset region.
 *              size - The size of the name buffer.
 *
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  M.S. Breitenfeld
 *              March 31, 2008
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rget_name_region_c (hid_t_f *loc_id, int_f *ref, _fcd name, size_t_f *name_len, size_t_f *size_default)
/******/
{
     hdset_reg_ref_t ref_c;
     int_f ret_value = -1;
     ssize_t c_size;
     size_t c_bufsize;
     char *c_buf= NULL;  /* Buffer to hold C string */

     HDmemcpy (&ref_c, ref, H5R_DSET_REG_REF_BUF_SIZE);

     c_bufsize = (size_t)*name_len+1;
     /*
      * Allocate buffer to hold name of an attribute
      */
     if ((c_buf = HDmalloc(c_bufsize)) == NULL)
       return ret_value;

     /*
      * Call H5Rget_name function.
      */
     if((c_size=H5Rget_name((hid_t)*loc_id, H5R_DATASET_REGION, &ref_c, c_buf, c_bufsize)) < 0)
         return ret_value;
     /*
      * Convert C name to FORTRAN and place it in the given buffer
      */
     HD5packFstring(c_buf, _fcdtocp(name), c_bufsize-1);

     *size_default = (size_t_f)c_size;
     ret_value = 0;
     if(c_buf) HDfree(c_buf);

     return ret_value;
}

/****if* H5Rf/h5rget_name_ptr_c
 * NAME
 *        h5rget_name_ptr_c
 * PURPOSE
 *     Call H5Rget_name
 * INPUTS
 *
 *       loc_id - Identifier for the dataset containing the reference or for the group that dataset is in.
 *     ref_type - Type of reference.
 *          ref - An object or dataset region reference.
 *
 * OUTPUTS
 *     name - A name associated with the referenced object or dataset region.
 *              size - The size of the name buffer.
 *
 * RETURNS
 *     0 on success, -1 on failure
 * AUTHOR
 *  M.S. Breitenfeld
 *              June 20, 2008
 * HISTORY
 *
 * SOURCE
*/
int_f
nh5rget_name_ptr_c (hid_t_f *loc_id, int_f *ref_type, void *ref, _fcd name, size_t_f *name_len, size_t_f *size_default)
/******/
{
     int_f ret_value = -1;
     ssize_t c_size;
     size_t c_bufsize;
     char *c_buf= NULL;  /* Buffer to hold C string */

     c_bufsize = (size_t)*name_len+1;
     /*
      * Allocate buffer to hold name of an attribute
      */
     if ((c_buf = HDmalloc(c_bufsize)) == NULL)
       return ret_value;

     /*
      * Call H5Rget_name function.
      */
     if((c_size=H5Rget_name((hid_t)*loc_id, (H5R_type_t)*ref_type, ref, c_buf, c_bufsize)) < 0){
       if(c_buf) HDfree(c_buf);
       return ret_value;
     }
     /*
      * Convert C name to FORTRAN and place it in the given buffer
      */
     HD5packFstring(c_buf, _fcdtocp(name), c_bufsize-1);

     *size_default = (size_t_f)c_size;
     ret_value = 0;
     if(c_buf) HDfree(c_buf);

     return ret_value;
}

/****if* H5Rf/h5rget_obj_type_c
 * NAME
 *   h5rget_obj_type_c
 * PURPOSE
 *   Call H5Rget_obj_type
 * INPUTS
 *   loc_id   - Identifier for the dataset containing the reference or
 *              for the group that dataset is in.
 *   ref_type - Type of reference to query.
 *   ref      - Reference to query.
 *
 * OUTPUTS
 *   obj_type - Type of referenced object. 
 *                H5G_UNKNOWN_F (-1)
 *                H5G_LINK_F      0
 *                H5G_GROUP_F     1
 *                H5G_DATASET_F   2
 *                H5G_TYPE_F      3
 * RETURNS
 *   0 on success, -1 on failure
 * AUTHOR
 *   M.S. Breitenfeld
 *   December 17, 2008
 *
 * SOURCE
*/
int_f
nh5rget_obj_type_c (hid_t_f *loc_id, int_f *ref_type, void *ref, int_f *obj_type)
/******/
{
  int_f ret_value = -1;
  H5O_type_t obj_type_c;

  /*
   * Call H5Rget_obj_type function.
   */
  if((H5Rget_obj_type((hid_t)*loc_id, (H5R_type_t)*ref_type, ref, &obj_type_c)) < 0)
    return ret_value;

  *obj_type = (int_f)obj_type_c;

  ret_value = 0;
  return ret_value;
}
