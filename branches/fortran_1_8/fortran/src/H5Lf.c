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

/* This files contains C stubs for H5L Fortran APIs */

#include "H5f90.h"
#include "H5Eprivate.h"

/*----------------------------------------------------------------------------
 * Name:        h5ldelete_c
 * Purpose:     Call h5ldelete_c
 * Inputs:      
 *    loc_id  - Identifier of the file or group containing the object
 *    name    - Name of the link to delete
 *    lapl_id - Link access property list identifier
 *    namelen - length of name
 * Returns:     0 on success, -1 on failure
 * Programmer:  M.S. Breitenfeld
 *              January, 2008
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f
nh5ldelete_c ( hid_t_f *loc_id, _fcd name, size_t_f *namelen, hid_t_f *lapl_id )
{
  char *c_name = NULL;
  int ret_value = 0;

  /*
   * Convert FORTRAN name to C name
   */
  if((c_name = HD5f2cstring(name, (size_t)*namelen)) == NULL)
    HGOTO_DONE(FAIL);

  /*
   * Call H5Adelete function.
   */
  if( H5Ldelete( (hid_t)*loc_id, c_name, (hid_t)*lapl_id  ) < 0)
    HGOTO_DONE(FAIL);

done:
    if(c_name)
        HDfree(c_name);

    return ret_value;
}
