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

/* Programmer:  John Mainzer
 *              9/4/15
 *
 *              This file contains declarations of all functions defined
 *		in genall5.c
 */

int file_superblock0(const char *filename, hbool_t set_driver);
int file_superblock1(const char *filename, hbool_t set_driver);
int file_superblock2(const char *filename, hbool_t set_driver);

int ns_grp_0(const char *filename, hbool_t set_driver, const char *group_name);
int vrfy_ns_grp_0(const char *filename, hbool_t set_driver, 
    const char *group_name);

int ns_grp_c(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks);
int vrfy_ns_grp_c(const char *filename, hbool_t set_driver, 
    const char *group_name, unsigned nlinks);

int ns_grp_d(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks);
int vrfy_ns_grp_d(const char *filename, hbool_t set_driver, 
    const char *group_name, unsigned nlinks);

int os_grp_0(const char *filename, hbool_t set_driver, const char *group_name);
int vrfy_os_grp_0(const char *filename, hbool_t set_driver, 
    const char *group_name);

int os_grp_n(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks);
int vrfy_os_grp_n(const char *filename, hbool_t set_driver, 
    const char *group_name, unsigned nlinks);

int ds_ctg_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data);
int vrfy_ds_ctg_i(const char *filename, hbool_t set_driver, 
    const char *dset_name, hbool_t write_data);

int ds_chk_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data);
int vrfy_ds_chk_i(const char *filename, hbool_t set_driver, 
    const char *dset_name, hbool_t write_data);

int ds_cpt_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data);
int vrfy_ds_cpt_i(const char *filename, hbool_t set_driver, 
    const char *dset_name, hbool_t write_data);

int ds_ctg_v(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data);
int vrfy_ds_ctg_v(const char *filename, hbool_t set_driver, 
    const char *dset_name, hbool_t write_data);

int sh_msg_l(const char *filename, hbool_t set_driver);
int vrfy_sh_msg_l(const char *filename, hbool_t set_driver);

int sh_msg_b(const char *filename, hbool_t set_driver);
int vrfy_sh_msg_b(const char *filename, hbool_t set_driver);

int fs_mgr(const char *filename, hbool_t set_driver);
int vrfy_fs_mgr(const char *filename, hbool_t set_driver);


