/* Tool to generate files with all possible pieces of HDF5 files */
/* File Format Components:
    (see: http://www.hdfgroup.org/HDF5/doc/H5.format.html)
    Level 0:
        0A - Superblock
            0A0 - v0 Superblock
            0A1 - v1 Superblock
            0A2 - v2 Superblock
        0B - Driver info
        0C - Superblock Extension
    Level 1:
        1A - B-trees
            1A1 - v1 B-tree node
            1A2 - v2 B-trees
                1A2a - v2 B-tree header
                1A2b - v2 B-tree internal node
                1A2c - v2 B-tree leaf node
        1B - Group symbol table
        1C - Group symbol table entry
        1D - Local heap
        1E - Global heap
        1F - Fractal heap
            1F1 - Header
            1F2 - Indirect block
            1F3 - Direct block
        1G - Free space manager
            1G1 - FS Header
            1G2 - FS Section info
        1H - SOHM Table
            1H1 - Shared Object Header Message Table
            1H2 - Shared Message Record List
    Level 2:
        2A - Object Header
            2A1 - Prefix
                2A1a - v1 Prefix
                2A1b - v2 Prefix
            2A2 - Messages
                2A2a - NIL
                2A2b - Dataspace
                2A2c - Link info
                2A2d - Datatype
                2A2e - Fill value (old)
                2A2f - Fill value (new)
                2A2g - Link Message
                2A2h - External File storage
                2A2i - Layout
                2A2j - Bogus
                2A2k - Group info
                2A2l - Filter pipeline
                2A2m - Attribute
                2A2n - Object comment
                2A2o - Modification time (old)
                2A2p - Shared message table
                2A2q - Object header continuation
                2A2r - Symbol table
                2A2s - Modification time (new)
                2A2t - B-tree 'K' Value
                2A2u - Driver info
                2A2v - Attribute info
                2A2w - Object ref count
                2A2x - Free space info
        2B - Data Object Data Storage
*/

/*
    Test Coverage:

        |0A0|0A1|0A2|0B|0C|1A1|1A2a|1A2b|1A2c|1B|1C|1D|1E|1F1|1F2|1F3|1G1|1G2|1H1|1H2|2A1a|2A1b|2A2a|2A2b|2A2c|2A2d|2A2e|2A2f|2A2g|2A2h|2A2i|2A2j|2A2k|2A2l|2A2m|2A2n|2A2o|2A2p|2A2q|2A2r|2A2s|2A2t|2A2u|2A2v|2A2w|2A2x|2B|
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
file_sb0|XXX|   |   |  |  |XXX|    |    |    |  |  |XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
file_sb0|XXX|   |   |XX|  |XXX|    |    |    |  |  |XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
w/driver|XXX|   |   |XX|  |XXX|    |    |    |  |  |XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
file_sb1|   |XXX|   |  |  |XXX|    |    |    |  |  |XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
file_sb1|   |XXX|   |XX|  |XXX|    |    |    |  |  |XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
w/driver|   |XXX|   |XX|  |XXX|    |    |    |  |  |XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
file_sb2|   |   |XXX|  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |    |XXXX|XXXX|    |XXXX|    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
file_sb2|   |   |XXX|  |XX|   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |    |XXXX|XXXX|    |XXXX|    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |  |
w/driver|   |   |XXX|  |XX|   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |    |XXXX|XXXX|    |XXXX|    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ns_grp_0|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|    |XXXX|    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ns_grp_c|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|    |XXXX|    |    |    |XXXX|    |    |    |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ns_grp_d|   |   |   |  |  |   |XXXX|XXXX|XXXX|  |  |  |  |XXX|XXX|XXX|XXX|XXX|   |   |XXXX|    |XXXX|    |XXXX|    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
os_grp_0|   |   |   |  |  |XXX|    |    |    |  |  |XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
os_grp_n|   |   |   |  |  |XXX|    |    |    |XX|XX|XX|  |   |   |   |   |   |   |   |XXXX|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_ctg_i|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_ctg_i|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |XX|
 w/data |   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |XX|
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_chk_i|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_chk_i|   |   |   |  |  |XXX|    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |XX|
 w/data |   |   |   |  |  |XXX|    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |XX|
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_cpt_i|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_cpt_i|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |  |
 w/data |   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_ctg_v|   |   |   |  |  |   |    |    |    |  |  |  |  |   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
ds_ctg_v|   |   |   |  |  |   |    |    |    |  |  |  |XX|   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |XX|
 w/data |   |   |   |  |  |   |    |    |    |  |  |  |XX|   |   |   |   |   |   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |    |XXXX|    |    |    |    |    |XX|
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
sh_msg_l|   |   |XXX|  |XX|XXX|    |    |    |XX|XX|XX|  |XXX|   |XXX|XXX|XXX|XXX|XXX|XXXX|XXXX|XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |XXXX|    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
sh_msg_l|   |   |XXX|  |XX|XXX|    |    |    |XX|XX|XX|  |XXX|   |XXX|XXX|XXX|XXX|XXX|XXXX|XXXX|XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |XXXX|XXXX|XXXX|    |    |XXXX|    |    |    |  |
w/driver|   |   |XXX|  |XX|XXX|    |    |    |XX|XX|XX|  |XXX|   |XXX|XXX|XXX|XXX|XXX|XXXX|XXXX|XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |XXXX|XXXX|XXXX|    |    |XXXX|    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
sh_msg_b|   |   |XXX|  |XX|XXX|XXXX|    |XXXX|XX|XX|XX|  |XXX|   |XXX|XXX|XXX|XXX|   |XXXX|XXXX|XXXX|XXXX|    |XXXX|XXXX|XXXX|    |    |XXXX|    |    |    |    |    |    |XXXX|    |XXXX|    |    |    |    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
sh_msg_b|   |   |XXX|  |XX|XXX|XXXX|    |XXXX|XX|XX|XX|  |XXX|   |XXX|XXX|XXX|XXX|   |XXXX|XXXX|XXXX|XXXX|    |XXXX|XXXX|XXXX|    |    |XXXX|    |    |    |    |    |    |XXXX|XXXX|XXXX|    |    |XXXX|    |    |    |  |
w/driver|   |   |XXX|  |XX|XXX|XXXX|    |XXXX|XX|XX|XX|  |XXX|   |XXX|XXX|XXX|XXX|   |XXXX|XXXX|XXXX|XXXX|    |XXXX|XXXX|XXXX|    |    |XXXX|    |    |    |    |    |    |XXXX|XXXX|XXXX|    |    |XXXX|    |    |    |  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+
fs_mgr  |   |   |XXX|  |XX|XXX|    |    |    |  |  |XX|  |   |   |   |XXX|XXX|   |   |XXXX|    |XXXX|XXXX|    |XXXX|    |XXXX|    |    |XXXX|    |    |    |    |    |    |    |    |XXXX|XXXX|    |XXXX|    |    |XXXX|  |
--------+---+---+---+--+--+---+----+----+----+--+--+--+--+---+---+---+---+---+---+---+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+--+

Errata:
 - Does not cover all types of dataset elements
    (i.e. needs support for compound datatypes, arrays, etc)
 - Does not cover all object header messages
 - Does not cover all object header message flags
    (Covers some, but they should be documented in the table above)
 - Does not cover object headers w/>1 hard link
 - Does not cover external file storage for datasets
 - Does not cover attributes
 - Does not cover creation order indices for groups
 - Does not cover creation order tracking/indices for attributes

*/


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "hdf5.h"

#define FAMILY_SIZE (1024 * 1024)
#define DSET_DIMS (1024 * 1024)
#define DSET_SMALL_DIMS (64 * 1024)
#define DSET_CHUNK_DIMS 1024
#define DSET_COMPACT_DIMS 4096

const char *FILENAME[3][2] =
{ {"super0.h5", "super0-%05d.h5"},
  {"super1.h5", "super1-%05d.h5"},
  {"super2.h5", "super2-%05d.h5"} };

const char *SOHM_FILENAME[2][2] =
{ {"sohm-l.h5", "sohm-l-%05d.h5"},
  {"sohm-b.h5", "sohm-b-%05d.h5"} };

const char *FSM_FILENAME[2] =
{"fsm.h5", "fsm-%05d.h5"};


/* Creates file with v0 superblock, optionally w/driver info */
/* Covers: 0A0, 2A1a, 1A1, 1D, 2A2r */
/* When set_driver is TRUE, also covers: 0C */
int file_superblock0(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fapl;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates file with v1 superblock, optionally w/driver info */
/* Covers: 0A1, 2A1a, 1A1, 1D, 2A2r */
/* When set_driver is TRUE, also covers: 0C */
int file_superblock1(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fcpl;
    hid_t fapl;
    herr_t ret;

    fcpl = H5Pcreate(H5P_FILE_CREATE);
    assert(fcpl > 0);
    ret = H5Pset_istore_k(fcpl, 64);
    assert(ret >= 0);

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl);
    assert(fid > 0);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates file with v2 superblock, optionally w/driver info */
/* Covers: 0A2, 2A1b, 2A2a, 2A2c, 2A2k */
/* When set_driver is TRUE, also covers: 0C, 2A2u */
int file_superblock2(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fcpl;
    hid_t fapl;
    herr_t ret;

    fcpl = H5Pcreate(H5P_FILE_CREATE);
    assert(fcpl > 0);
    ret = H5Pset_link_phase_change(fcpl, 16, 14);
    assert(ret >= 0);

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);
    ret = H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    assert(ret >= 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl);
    assert(fid > 0);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates empty "new style" group */
/* Covers: 2A1a, 2A2a, 2A2c, 2A2k */
int ns_grp_0(const char *filename, hbool_t set_driver, const char *group_name)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gcpl = H5Pcreate(H5P_GROUP_CREATE);
    assert(gcpl > 0);
    ret = H5Pset_link_creation_order(gcpl, H5P_CRT_ORDER_TRACKED);
    assert(ret >= 0);

    gid = H5Gcreate2(fid, group_name, H5P_DEFAULT, gcpl, H5P_DEFAULT);
    assert(gid > 0);

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies empty "new style" group */
int vrfy_ns_grp_0(const char *filename, hbool_t set_driver, const char *group_name)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    H5G_info_t grp_info;
    unsigned crt_order_flags = 0;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gid = H5Gopen2(fid, group_name, H5P_DEFAULT);
    assert(gid > 0);

    gcpl = H5Gget_create_plist(gid);
    assert(gcpl > 0);
    ret = H5Pget_link_creation_order(gcpl, &crt_order_flags);
    assert(ret >= 0);
    assert(H5P_CRT_ORDER_TRACKED == crt_order_flags);

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    memset(&grp_info, 0, sizeof(grp_info));
    ret = H5Gget_info(gid, &grp_info);
    assert(ret >= 0);
    assert(H5G_STORAGE_TYPE_COMPACT == grp_info.storage_type);
    assert(0 == grp_info.nlinks);
    assert(0 == grp_info.max_corder);
    assert(false == grp_info.mounted);

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates compact "new style" group, with 'nlinks' soft/hard/external links in it */
/* Covers: 2A1a, 2A2a, 2A2c, 2A2g, 2A2k */
int ns_grp_c(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    unsigned max_compact;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gcpl = H5Pcreate(H5P_GROUP_CREATE);
    assert(gcpl > 0);
    ret = H5Pset_link_creation_order(gcpl, H5P_CRT_ORDER_TRACKED);
    assert(ret >= 0);

    gid = H5Gcreate2(fid, group_name, H5P_DEFAULT, gcpl, H5P_DEFAULT);
    assert(gid > 0);

    max_compact = 0;
    ret = H5Pget_link_phase_change(gcpl, &max_compact, NULL);
    assert(ret >= 0);
    assert(nlinks > 0);
    assert(nlinks < max_compact);
    for(u = 0; u < nlinks; u++) {
        char linkname[16];

        sprintf(linkname, "%u", u);
        if(0 == (u % 3)) {
            ret = H5Lcreate_soft(group_name, gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end if */
        else if(1 == (u % 3)) {
            ret = H5Lcreate_hard(fid, "/", gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end else-if */
        else {
            assert(2 == (u % 3));
            ret = H5Lcreate_external("external.h5", "/ext", gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end else */
    } /* end for */

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies compact "new style" group */
int vrfy_ns_grp_c(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    H5G_info_t grp_info;
    unsigned crt_order_flags = 0;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gid = H5Gopen2(fid, group_name, H5P_DEFAULT);
    assert(gid > 0);

    gcpl = H5Gget_create_plist(gid);
    assert(gcpl > 0);
    ret = H5Pget_link_creation_order(gcpl, &crt_order_flags);
    assert(ret >= 0);
    assert(H5P_CRT_ORDER_TRACKED == crt_order_flags);

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    memset(&grp_info, 0, sizeof(grp_info));
    ret = H5Gget_info(gid, &grp_info);
    assert(ret >= 0);
    assert(H5G_STORAGE_TYPE_COMPACT == grp_info.storage_type);
    assert(nlinks == grp_info.nlinks);
    assert(nlinks == grp_info.max_corder);
    assert(false == grp_info.mounted);

    for(u = 0; u < nlinks; u++) {
        H5L_info_t lnk_info;
        char linkname[16];
        htri_t link_exists;

        sprintf(linkname, "%u", u);
        link_exists = H5Lexists(gid, linkname, H5P_DEFAULT);
        assert(link_exists >= 0);

        memset(&lnk_info, 0, sizeof(grp_info));
        ret = H5Lget_info(gid, linkname, &lnk_info, H5P_DEFAULT);
        assert(ret >= 0);
        assert(true == lnk_info.corder_valid);
        assert(u == lnk_info.corder);
        assert(H5T_CSET_ASCII == lnk_info.cset);
        if(0 == (u % 3)) {
            char *slinkval;

            assert(H5L_TYPE_SOFT == lnk_info.type);
            assert((strlen(group_name) + 1) == lnk_info.u.val_size);

            slinkval = malloc(lnk_info.u.val_size);
            assert(slinkval);

            ret = H5Lget_val(gid, linkname, slinkval, lnk_info.u.val_size, H5P_DEFAULT);
            assert(ret >= 0);
            assert(0 == strcmp(slinkval, group_name));

            free(slinkval);
        } /* end if */
        else if(1 == (u % 3)) {
            H5O_info_t root_oinfo;

            assert(H5L_TYPE_HARD == lnk_info.type);
            memset(&root_oinfo, 0, sizeof(root_oinfo));
            ret = H5Oget_info(fid, &root_oinfo);
            assert(ret >= 0);
            assert(root_oinfo.addr == lnk_info.u.address);
        } /* end else-if */
        else {
            void *elinkval;
            const char *file = NULL;
            const char *path = NULL;

            assert(2 == (u % 3));
            assert(H5L_TYPE_EXTERNAL == lnk_info.type);

            elinkval = malloc(lnk_info.u.val_size);
            assert(elinkval);

            ret = H5Lget_val(gid, linkname, elinkval, lnk_info.u.val_size, H5P_DEFAULT);
            assert(ret >= 0);
            ret = H5Lunpack_elink_val(elinkval, lnk_info.u.val_size, NULL, &file, &path);
            assert(ret >= 0);
            assert(0 == strcmp(file, "external.h5"));
            assert(0 == strcmp(path, "/ext"));

            free(elinkval);
        } /* end else */
    } /* end for */

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates dense "new style" group, with 'nlinks' (soft/hard/external) in it */
/* Covers: 1A2a, 1A2b, 1A2c, 1F1, 1F2, 1F3, 1G1, 1G2, 2A1a, 2A2a, 2A2c, 2A2k */
int ns_grp_d(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    unsigned max_compact;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gcpl = H5Pcreate(H5P_GROUP_CREATE);
    assert(gcpl > 0);
    ret = H5Pset_link_creation_order(gcpl, H5P_CRT_ORDER_TRACKED);
    assert(ret >= 0);

    gid = H5Gcreate2(fid, group_name, H5P_DEFAULT, gcpl, H5P_DEFAULT);
    assert(gid > 0);

    max_compact = 0;
    ret = H5Pget_link_phase_change(gcpl, &max_compact, NULL);
    assert(ret >= 0);
    assert(nlinks > max_compact);
    for(u = 0; u < nlinks; u++) {
        char linkname[16];

        sprintf(linkname, "%u", u);
        if(0 == (u % 3)) {
            ret = H5Lcreate_soft(group_name, gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end if */
        else if(1 == (u % 3)) {
            ret = H5Lcreate_hard(fid, "/", gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end else-if */
        else {
            assert(2 == (u % 3));
            ret = H5Lcreate_external("external.h5", "/ext", gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end else */
    } /* end for */

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies dense "new style" group */
int vrfy_ns_grp_d(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    H5G_info_t grp_info;
    unsigned crt_order_flags = 0;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gid = H5Gopen2(fid, group_name, H5P_DEFAULT);
    assert(gid > 0);

    gcpl = H5Gget_create_plist(gid);
    assert(gcpl > 0);
    ret = H5Pget_link_creation_order(gcpl, &crt_order_flags);
    assert(ret >= 0);
    assert(H5P_CRT_ORDER_TRACKED == crt_order_flags);

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    memset(&grp_info, 0, sizeof(grp_info));
    ret = H5Gget_info(gid, &grp_info);
    assert(ret >= 0);
    assert(H5G_STORAGE_TYPE_DENSE == grp_info.storage_type);
    assert(nlinks == grp_info.nlinks);
    assert(nlinks == grp_info.max_corder);
    assert(false == grp_info.mounted);

    for(u = 0; u < nlinks; u++) {
        H5L_info_t lnk_info;
        char linkname[16];
        htri_t link_exists;

        sprintf(linkname, "%u", u);
        link_exists = H5Lexists(gid, linkname, H5P_DEFAULT);
        assert(link_exists >= 0);

        memset(&lnk_info, 0, sizeof(grp_info));
        ret = H5Lget_info(gid, linkname, &lnk_info, H5P_DEFAULT);
        assert(ret >= 0);
        assert(true == lnk_info.corder_valid);
        assert(u == lnk_info.corder);
        assert(H5T_CSET_ASCII == lnk_info.cset);
        if(0 == (u % 3)) {
            char *slinkval;

            assert(H5L_TYPE_SOFT == lnk_info.type);
            assert((strlen(group_name) + 1) == lnk_info.u.val_size);

            slinkval = malloc(lnk_info.u.val_size);
            assert(slinkval);

            ret = H5Lget_val(gid, linkname, slinkval, lnk_info.u.val_size, H5P_DEFAULT);
            assert(ret >= 0);
            assert(0 == strcmp(slinkval, group_name));

            free(slinkval);
        } /* end if */
        else if(1 == (u % 3)) {
            H5O_info_t root_oinfo;

            assert(H5L_TYPE_HARD == lnk_info.type);
            memset(&root_oinfo, 0, sizeof(root_oinfo));
            ret = H5Oget_info(fid, &root_oinfo);
            assert(ret >= 0);
            assert(root_oinfo.addr == lnk_info.u.address);
        } /* end else-if */
        else {
            void *elinkval;
            const char *file = NULL;
            const char *path = NULL;

            assert(2 == (u % 3));
            assert(H5L_TYPE_EXTERNAL == lnk_info.type);

            elinkval = malloc(lnk_info.u.val_size);
            assert(elinkval);

            ret = H5Lget_val(gid, linkname, elinkval, lnk_info.u.val_size, H5P_DEFAULT);
            assert(ret >= 0);
            ret = H5Lunpack_elink_val(elinkval, lnk_info.u.val_size, NULL, &file, &path);
            assert(ret >= 0);
            assert(0 == strcmp(file, "external.h5"));
            assert(0 == strcmp(path, "/ext"));

            free(elinkval);
        } /* end else */
    } /* end for */

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates empty "old style" group */
/* Covers: 1A1, 1D, 2A1a, 2A2r */
int os_grp_0(const char *filename, hbool_t set_driver, const char *group_name)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gid = H5Gcreate2(fid, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(gid > 0);

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies empty "old style" group */
int vrfy_os_grp_0(const char *filename, hbool_t set_driver, const char *group_name)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    H5G_info_t grp_info;
    unsigned crt_order_flags = 0;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gid = H5Gopen2(fid, group_name, H5P_DEFAULT);
    assert(gid > 0);

    gcpl = H5Gget_create_plist(gid);
    assert(gcpl > 0);
    ret = H5Pget_link_creation_order(gcpl, &crt_order_flags);
    assert(ret >= 0);
    assert(0 == crt_order_flags);

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    memset(&grp_info, 0, sizeof(grp_info));
    ret = H5Gget_info(gid, &grp_info);
    assert(ret >= 0);
    assert(H5G_STORAGE_TYPE_SYMBOL_TABLE == grp_info.storage_type);
    assert(0 == grp_info.nlinks);
    assert(0 == grp_info.max_corder);
    assert(false == grp_info.mounted);

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates "old style" group, with 'nlinks' soft/hard links in it */
/* Covers: 1A1, 1D, 2A1a, 2A2r */
int os_grp_n(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gid = H5Gcreate2(fid, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(gid > 0);

    assert(nlinks > 0);
    for(u = 0; u < nlinks; u++) {
        char linkname[16];

        sprintf(linkname, "%u", u);
        if(0 == (u % 2)) {
            ret = H5Lcreate_soft(group_name, gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end if */
        else {
            assert(1 == (u % 2));
            ret = H5Lcreate_hard(fid, "/", gid, linkname, H5P_DEFAULT, H5P_DEFAULT);
            assert(ret >= 0);
        } /* end else */
    } /* end for */

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies "old style" group */
int vrfy_os_grp_n(const char *filename, hbool_t set_driver, const char *group_name,
    unsigned nlinks)
{
    hid_t fid;
    hid_t fapl;
    hid_t gid;
    hid_t gcpl;
    H5G_info_t grp_info;
    unsigned crt_order_flags = 0;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    gid = H5Gopen2(fid, group_name, H5P_DEFAULT);
    assert(gid > 0);

    gcpl = H5Gget_create_plist(gid);
    assert(gcpl > 0);
    ret = H5Pget_link_creation_order(gcpl, &crt_order_flags);
    assert(ret >= 0);
    assert(0 == crt_order_flags);

    ret = H5Pclose(gcpl);
    assert(ret >= 0);

    memset(&grp_info, 0, sizeof(grp_info));
    ret = H5Gget_info(gid, &grp_info);
    assert(ret >= 0);
    assert(H5G_STORAGE_TYPE_SYMBOL_TABLE == grp_info.storage_type);
    assert(nlinks == grp_info.nlinks);
    assert(0 == grp_info.max_corder);
    assert(false == grp_info.mounted);

    for(u = 0; u < nlinks; u++) {
        H5L_info_t lnk_info;
        char linkname[16];
        htri_t link_exists;

        sprintf(linkname, "%u", u);
        link_exists = H5Lexists(gid, linkname, H5P_DEFAULT);
        assert(link_exists >= 0);

        memset(&lnk_info, 0, sizeof(grp_info));
        ret = H5Lget_info(gid, linkname, &lnk_info, H5P_DEFAULT);
        assert(ret >= 0);
        assert(false == lnk_info.corder_valid);
        assert(H5T_CSET_ASCII == lnk_info.cset);
        if(0 == (u % 2)) {
            char *slinkval;

            assert(H5L_TYPE_SOFT == lnk_info.type);
            assert((strlen(group_name) + 1) == lnk_info.u.val_size);

            slinkval = malloc(lnk_info.u.val_size);
            assert(slinkval);

            ret = H5Lget_val(gid, linkname, slinkval, lnk_info.u.val_size, H5P_DEFAULT);
            assert(ret >= 0);
            assert(0 == strcmp(slinkval, group_name));

            free(slinkval);
        } /* end if */
        else {
            H5O_info_t root_oinfo;

            assert(1 == (u % 2));
            assert(H5L_TYPE_HARD == lnk_info.type);
            memset(&root_oinfo, 0, sizeof(root_oinfo));
            ret = H5Oget_info(fid, &root_oinfo);
            assert(ret >= 0);
            assert(root_oinfo.addr == lnk_info.u.address);
        } /* end else */
    } /* end for */

    ret = H5Gclose(gid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates contiguous dataset w/int datatype */
/* Covers: 2A1a, 2A2a, 2A2b, 2A2d, 2A2f, 2A2i, 2A2s */
/* When write_data is TRUE, also covers: 2B */
int ds_ctg_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    hsize_t dims[1] = {DSET_DIMS};
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    dsid = H5Dcreate2(fid, dset_name, H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dsid > 0);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    if(write_data) {
        int *wdata;
        unsigned u;

        wdata = malloc(sizeof(int) * DSET_DIMS);
        assert(wdata);

        for(u = 0; u < DSET_DIMS; u++)
            wdata[u] = u;

        ret = H5Dwrite(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
        assert(ret >= 0);

        free(wdata);
    } /* end if */

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies contiguous datasets w/int datatypes */
int vrfy_ds_ctg_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    hid_t tid;
    hid_t dcpl;
    H5D_space_status_t allocation;
    H5D_layout_t layout;
    int ndims;
    hsize_t dims[1], max_dims[1];
    htri_t type_equal;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    dsid = H5Dopen2(fid, dset_name, H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_DIMS == dims[0]);
    assert(DSET_DIMS == max_dims[0]);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    type_equal = H5Tequal(tid, H5T_NATIVE_INT);
    assert(1 == type_equal);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dget_space_status(dsid, &allocation);
    assert(ret >= 0);
    assert((write_data && allocation == H5D_SPACE_STATUS_ALLOCATED) ||
        (!write_data && allocation == H5D_SPACE_STATUS_NOT_ALLOCATED));

    dcpl = H5Dget_create_plist(dsid);
    assert(dcpl > 0);

    layout = H5Pget_layout(dcpl);
    assert(H5D_CONTIGUOUS == layout);

    ret = H5Pclose(dcpl);
    assert(ret >= 0);

    if(write_data) {
        int *rdata;
        unsigned u;

        rdata = malloc(sizeof(int) * DSET_DIMS);
        assert(rdata);

        ret = H5Dread(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
        assert(ret >= 0);

        for(u = 0; u < DSET_DIMS; u++)
            assert(u == rdata[u]);

        free(rdata);
    } /* end if */


    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates chunked dataset w/int datatype */
/* Covers: 2A1a, 2A2a, 2A2b, 2A2d, 2A2f, 2A2i, 2A2s */
/* When write_data is TRUE, also covers: 1A1, 2B */
int ds_chk_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t dcpl;
    hid_t sid;
    hsize_t dims[1] = {DSET_DIMS};
    hsize_t chunk_dims[1] = {DSET_CHUNK_DIMS};
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    dcpl = H5Pcreate(H5P_DATASET_CREATE);
    assert(dcpl > 0);
    ret = H5Pset_chunk(dcpl, 1, chunk_dims);
    assert(ret >= 0);

    dsid = H5Dcreate2(fid, dset_name, H5T_NATIVE_INT, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT);
    assert(dsid > 0);

    ret = H5Pclose(dcpl);
    assert(ret >= 0);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    if(write_data) {
        int *wdata;
        unsigned u;

        wdata = malloc(sizeof(int) * DSET_DIMS);
        assert(wdata);

        for(u = 0; u < DSET_DIMS; u++)
            wdata[u] = u;

        ret = H5Dwrite(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
        assert(ret >= 0);

        free(wdata);
    } /* end if */

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies chunked datasets w/int datatypes */
int vrfy_ds_chk_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    hid_t tid;
    hid_t dcpl;
    H5D_space_status_t allocation;
    H5D_layout_t layout;
    int ndims;
    hsize_t dims[1], max_dims[1], chunk_dims[1];
    htri_t type_equal;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    dsid = H5Dopen2(fid, dset_name, H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_DIMS == dims[0]);
    assert(DSET_DIMS == max_dims[0]);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    type_equal = H5Tequal(tid, H5T_NATIVE_INT);
    assert(1 == type_equal);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dget_space_status(dsid, &allocation);
    assert(ret >= 0);
    assert((write_data && allocation == H5D_SPACE_STATUS_ALLOCATED) ||
        (!write_data && allocation == H5D_SPACE_STATUS_NOT_ALLOCATED));

    dcpl = H5Dget_create_plist(dsid);
    assert(dcpl > 0);

    layout = H5Pget_layout(dcpl);
    assert(H5D_CHUNKED == layout);
    ret = H5Pget_chunk(dcpl, 1, chunk_dims);
    assert(ret >= 0);
    assert(DSET_CHUNK_DIMS == chunk_dims[0]);

    ret = H5Pclose(dcpl);
    assert(ret >= 0);

    if(write_data) {
        int *rdata;
        unsigned u;

        rdata = malloc(sizeof(int) * DSET_DIMS);
        assert(rdata);

        ret = H5Dread(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
        assert(ret >= 0);

        for(u = 0; u < DSET_DIMS; u++)
            assert(u == rdata[u]);

        free(rdata);
    } /* end if */


    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates compact dataset w/int datatype */
/* Covers: 2A1a, 2A2a, 2A2b, 2A2d, 2A2f, 2A2i, 2A2s */
/* When write_data is TRUE, also covers: <nothing different> */
int ds_cpt_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t dcpl;
    hid_t sid;
    hsize_t dims[1] = {DSET_COMPACT_DIMS};
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    dcpl = H5Pcreate(H5P_DATASET_CREATE);
    assert(dcpl > 0);
    ret = H5Pset_layout(dcpl, H5D_COMPACT);
    assert(ret >= 0);

    dsid = H5Dcreate2(fid, dset_name, H5T_NATIVE_INT, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT);
    assert(dsid > 0);

    ret = H5Pclose(dcpl);
    assert(ret >= 0);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    if(write_data) {
        int *wdata;
        unsigned u;

        wdata = malloc(sizeof(int) * DSET_COMPACT_DIMS);
        assert(wdata);

        for(u = 0; u < DSET_COMPACT_DIMS; u++)
            wdata[u] = u;

        ret = H5Dwrite(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
        assert(ret >= 0);

        free(wdata);
    } /* end if */

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies compact datasets w/int datatypes */
int vrfy_ds_cpt_i(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    hid_t tid;
    hid_t dcpl;
    H5D_space_status_t allocation;
    H5D_layout_t layout;
    int ndims;
    hsize_t dims[1], max_dims[1];
    htri_t type_equal;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    dsid = H5Dopen2(fid, dset_name, H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_COMPACT_DIMS == dims[0]);
    assert(DSET_COMPACT_DIMS == max_dims[0]);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    type_equal = H5Tequal(tid, H5T_NATIVE_INT);
    assert(1 == type_equal);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dget_space_status(dsid, &allocation);
    assert(ret >= 0);
    assert(H5D_SPACE_STATUS_ALLOCATED == allocation);

    dcpl = H5Dget_create_plist(dsid);
    assert(dcpl > 0);

    layout = H5Pget_layout(dcpl);
    assert(H5D_COMPACT == layout);

    ret = H5Pclose(dcpl);
    assert(ret >= 0);

    if(write_data) {
        int *rdata;
        unsigned u;

        rdata = malloc(sizeof(int) * DSET_COMPACT_DIMS);
        assert(rdata);

        ret = H5Dread(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
        assert(ret >= 0);

        for(u = 0; u < DSET_COMPACT_DIMS; u++)
            assert(u == rdata[u]);

        free(rdata);
    } /* end if */


    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates contiguous dataset w/variable-length datatype */
/* Covers: 2A1a, 2A2a, 2A2b, 2A2d, 2A2f, 2A2i, 2A2s */
/* When write_data is TRUE, also covers: 1E, 2B */
int ds_ctg_v(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    hid_t tid;
    hsize_t dims[1] = {DSET_SMALL_DIMS};
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    tid = H5Tvlen_create(H5T_NATIVE_INT);
    assert(tid > 0);

    dsid = H5Dcreate2(fid, dset_name, tid, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dsid > 0);

    if(write_data) {
        hvl_t *wdata;
        unsigned u;

        wdata = malloc(sizeof(hvl_t) * DSET_SMALL_DIMS);
        assert(wdata);

        for(u = 0; u < DSET_SMALL_DIMS; u++) {
            int *tdata;
            unsigned len;
            unsigned v;

            len = (u % 10) + 1;
            tdata = (int *)malloc(sizeof(int) * len);
            assert(tdata);

            for(v = 0; v < len; v++)
                tdata[v] = u + v;

           wdata[u].len = len;
           wdata[u].p = tdata;
        } /* end for */

        ret = H5Dwrite(dsid, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
        assert(ret >= 0);

        ret = H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, wdata);
        assert(ret >= 0);

        free(wdata);
    } /* end if */

    ret = H5Sclose(sid);
    assert(ret >= 0);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies contiguous datasets w/variable-length datatypes */
int vrfy_ds_ctg_v(const char *filename, hbool_t set_driver, const char *dset_name,
    hbool_t write_data)
{
    hid_t fid;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    hid_t tid;
    hid_t tmp_tid;
    hid_t dcpl;
    H5D_space_status_t allocation;
    H5D_layout_t layout;
    int ndims;
    hsize_t dims[1], max_dims[1];
    htri_t type_equal;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    dsid = H5Dopen2(fid, dset_name, H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_SMALL_DIMS == dims[0]);
    assert(DSET_SMALL_DIMS == max_dims[0]);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    tmp_tid = H5Tvlen_create(H5T_NATIVE_INT);
    assert(tmp_tid > 0);

    type_equal = H5Tequal(tid, tmp_tid);
    assert(1 == type_equal);

    ret = H5Tclose(tmp_tid);
    assert(ret >= 0);

    ret = H5Dget_space_status(dsid, &allocation);
    assert(ret >= 0);
    assert((write_data && allocation == H5D_SPACE_STATUS_ALLOCATED) ||
        (!write_data && allocation == H5D_SPACE_STATUS_NOT_ALLOCATED));

    dcpl = H5Dget_create_plist(dsid);
    assert(dcpl > 0);

    layout = H5Pget_layout(dcpl);
    assert(H5D_CONTIGUOUS == layout);

    ret = H5Pclose(dcpl);
    assert(ret >= 0);

    if(write_data) {
        hvl_t *rdata;
        unsigned u;

        rdata = malloc(sizeof(hvl_t) * DSET_SMALL_DIMS);
        assert(rdata);

        ret = H5Dread(dsid, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
        assert(ret >= 0);

        for(u = 0; u < DSET_SMALL_DIMS; u++) {
            unsigned len;
            unsigned v;

            len = rdata[u].len;
            for(v = 0; v < len; v++) {
                int *tdata = rdata[u].p;

                assert(tdata);
                assert((u + v) == tdata[v]);
            } /* end for */
        } /* end for */

        ret = H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, rdata);
        assert(ret >= 0);

        free(rdata);
    } /* end if */

    ret = H5Sclose(sid);
    assert(ret >= 0);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates file with shared object header messages (list form) */
/* Covers: 0A2, 0C, 1A1, 1B, 1C, 1D, 1F1, 1F3, 1G1, 1G2, 1H1, 1H2, 2A1a, 2A1b, 2A2a, 2A2b, 2A2d, 2A2f, 2A2i, 2A2p, 2A2r, */
/* When set_driver is TRUE, also covers: 2A2q, 2A2u */
int sh_msg_l(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fcpl;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    unsigned u;
    hsize_t dims[1] = {DSET_SMALL_DIMS};
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fcpl = H5Pcreate(H5P_FILE_CREATE);
    assert(fcpl > 0);

    ret = H5Pset_shared_mesg_nindexes(fcpl, 1);
    assert(ret >= 0);
    ret = H5Pset_shared_mesg_index(fcpl, 0, H5O_SHMESG_ALL_FLAG, 8);
    assert(ret >= 0);
    ret = H5Pset_shared_mesg_phase_change(fcpl, 8, 6);
    assert(ret >= 0);

    fid = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl);
    assert(fid > 0);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    for(u = 0; u < 2; u++) {
        char dsetname[16];

        sprintf(dsetname, "%u", u);
        dsid = H5Dcreate2(fid, dsetname, H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        assert(dsid > 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    ret = H5Sclose(sid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies file w/shared object header messages (list form) */
int vrfy_sh_msg_l(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fapl;
    hid_t fcpl;
    unsigned nindexes = 0;
    unsigned mesg_type_flags = 0;
    unsigned min_mesg_size = 0;
    unsigned max_list = 0;
    unsigned min_btree = 0;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    fcpl = H5Fget_create_plist(fid);
    assert(fcpl > 0);

    ret = H5Pget_shared_mesg_nindexes(fcpl, &nindexes);
    assert(ret >= 0);
    assert(1 == nindexes);
    ret = H5Pget_shared_mesg_index(fcpl, 0, &mesg_type_flags, &min_mesg_size);
    assert(ret >= 0);
    assert(H5O_SHMESG_ALL_FLAG == mesg_type_flags);
    assert(8 == min_mesg_size);
    ret = H5Pget_shared_mesg_phase_change(fcpl, &max_list, &min_btree);
    assert(ret >= 0);
    assert(8 == max_list);
    assert(6 == min_btree);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    for(u = 0; u < 2; u++) {
        hid_t dsid;
        hid_t sid;
        hid_t tid;
        int ndims;
        hsize_t dims[1], max_dims[1];
        htri_t type_equal;
        char dsetname[16];

        sprintf(dsetname, "%u", u);
        dsid = H5Dopen2(fid, dsetname, H5P_DEFAULT);
        assert(dsid > 0);

        sid = H5Dget_space(dsid);
        assert(sid > 0);

        ndims = H5Sget_simple_extent_ndims(sid);
        assert(1 == ndims);

        ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
        assert(ret >= 0);
        assert(DSET_SMALL_DIMS == dims[0]);
        assert(DSET_SMALL_DIMS == max_dims[0]);

        ret = H5Sclose(sid);
        assert(ret >= 0);

        tid = H5Dget_type(dsid);
        assert(tid > 0);

        type_equal = H5Tequal(tid, H5T_NATIVE_INT);
        assert(1 == type_equal);

        ret = H5Tclose(tid);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates file with shared object header messages (B-tree form) */
/* Covers: 0A2, 0C, 1A1, 1A2a, 1A2c, 1B, 1C, 1D, 1F1, 1F3, 1G1, 1G2, 1H1, 2A1a, 2A1b, 2A2a, 2A2b, 2A2d, 2A2f, 2A2i, 2A2p, 2A2r, */
/* When set_driver is TRUE, also covers: 2A2q, 2A2u */
int sh_msg_b(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fcpl;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    unsigned u;
    hsize_t dims[1];
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fcpl = H5Pcreate(H5P_FILE_CREATE);
    assert(fcpl > 0);

    ret = H5Pset_shared_mesg_nindexes(fcpl, 1);
    assert(ret >= 0);
    ret = H5Pset_shared_mesg_index(fcpl, 0, H5O_SHMESG_ALL_FLAG, 8);
    assert(ret >= 0);
    ret = H5Pset_shared_mesg_phase_change(fcpl, 8, 6);
    assert(ret >= 0);

    fid = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl);
    assert(fid > 0);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    dims[0] = 10;
    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    for(u = 0; u < 2; u++) {
        char dsetname[16];

        sprintf(dsetname, "%u", u);
        dsid = H5Dcreate2(fid, dsetname, H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        assert(dsid > 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    ret = H5Sclose(sid);
    assert(ret >= 0);

    dims[0] = 20;
    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    for(u = 0; u < 2; u++) {
        hid_t dcpl;
        char dsetname[16];
        int fill = 1;

        dcpl = H5Pcreate(H5P_DATASET_CREATE);
        assert(dcpl > 0);
        ret = H5Pset_fill_value(dcpl, H5T_NATIVE_INT, &fill);
        assert(ret >= 0);

        sprintf(dsetname, "1%u", u);
        dsid = H5Dcreate2(fid, dsetname, H5T_NATIVE_INT, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT);
        assert(dsid > 0);

        ret = H5Pclose(dcpl);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    ret = H5Sclose(sid);
    assert(ret >= 0);

    dims[0] = 30;
    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    for(u = 0; u < 2; u++) {
        hid_t dcpl;
        char dsetname[16];
        int fill = 2;

        dcpl = H5Pcreate(H5P_DATASET_CREATE);
        assert(dcpl > 0);
        ret = H5Pset_fill_value(dcpl, H5T_NATIVE_INT, &fill);
        assert(ret >= 0);

        sprintf(dsetname, "2%u", u);
        dsid = H5Dcreate2(fid, dsetname, H5T_NATIVE_INT, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT);
        assert(dsid > 0);

        ret = H5Pclose(dcpl);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    ret = H5Sclose(sid);
    assert(ret >= 0);

    dims[0] = 40;
    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    for(u = 0; u < 2; u++) {
        hid_t dcpl;
        char dsetname[16];
        int fill = 3;

        dcpl = H5Pcreate(H5P_DATASET_CREATE);
        assert(dcpl > 0);
        ret = H5Pset_fill_value(dcpl, H5T_NATIVE_INT, &fill);
        assert(ret >= 0);

        sprintf(dsetname, "3%u", u);
        dsid = H5Dcreate2(fid, dsetname, H5T_NATIVE_INT, sid, H5P_DEFAULT, dcpl, H5P_DEFAULT);
        assert(dsid > 0);

        ret = H5Pclose(dcpl);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    ret = H5Sclose(sid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies file w/shared object header messages (B-tree form) */
int vrfy_sh_msg_b(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fapl;
    hid_t fcpl;
    unsigned nindexes = 0;
    unsigned mesg_type_flags = 0;
    unsigned min_mesg_size = 0;
    unsigned max_list = 0;
    unsigned min_btree = 0;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    fcpl = H5Fget_create_plist(fid);
    assert(fcpl > 0);

    ret = H5Pget_shared_mesg_nindexes(fcpl, &nindexes);
    assert(ret >= 0);
    assert(1 == nindexes);
    ret = H5Pget_shared_mesg_index(fcpl, 0, &mesg_type_flags, &min_mesg_size);
    assert(ret >= 0);
    assert(H5O_SHMESG_ALL_FLAG == mesg_type_flags);
    assert(8 == min_mesg_size);
    ret = H5Pget_shared_mesg_phase_change(fcpl, &max_list, &min_btree);
    assert(ret >= 0);
    assert(8 == max_list);
    assert(6 == min_btree);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    for(u = 0; u < 2; u++) {
        hid_t dsid;
        hid_t sid;
        hid_t tid;
        int ndims;
        hsize_t dims[1], max_dims[1];
        htri_t type_equal;
        char dsetname[16];

        sprintf(dsetname, "%u", u);
        dsid = H5Dopen2(fid, dsetname, H5P_DEFAULT);
        assert(dsid > 0);

        sid = H5Dget_space(dsid);
        assert(sid > 0);

        ndims = H5Sget_simple_extent_ndims(sid);
        assert(1 == ndims);

        ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
        assert(ret >= 0);
        assert(10 == dims[0]);
        assert(10 == max_dims[0]);

        ret = H5Sclose(sid);
        assert(ret >= 0);

        tid = H5Dget_type(dsid);
        assert(tid > 0);

        type_equal = H5Tequal(tid, H5T_NATIVE_INT);
        assert(1 == type_equal);

        ret = H5Tclose(tid);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    for(u = 0; u < 2; u++) {
        hid_t dsid;
        hid_t sid;
        hid_t tid;
        int ndims;
        hsize_t dims[1], max_dims[1];
        htri_t type_equal;
        char dsetname[16];

        sprintf(dsetname, "1%u", u);
        dsid = H5Dopen2(fid, dsetname, H5P_DEFAULT);
        assert(dsid > 0);

        sid = H5Dget_space(dsid);
        assert(sid > 0);

        ndims = H5Sget_simple_extent_ndims(sid);
        assert(1 == ndims);

        ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
        assert(ret >= 0);
        assert(20 == dims[0]);
        assert(20 == max_dims[0]);

        ret = H5Sclose(sid);
        assert(ret >= 0);

        tid = H5Dget_type(dsid);
        assert(tid > 0);

        type_equal = H5Tequal(tid, H5T_NATIVE_INT);
        assert(1 == type_equal);

        ret = H5Tclose(tid);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    for(u = 0; u < 2; u++) {
        hid_t dsid;
        hid_t sid;
        hid_t tid;
        int ndims;
        hsize_t dims[1], max_dims[1];
        htri_t type_equal;
        char dsetname[16];

        sprintf(dsetname, "2%u", u);
        dsid = H5Dopen2(fid, dsetname, H5P_DEFAULT);
        assert(dsid > 0);

        sid = H5Dget_space(dsid);
        assert(sid > 0);

        ndims = H5Sget_simple_extent_ndims(sid);
        assert(1 == ndims);

        ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
        assert(ret >= 0);
        assert(30 == dims[0]);
        assert(30 == max_dims[0]);

        ret = H5Sclose(sid);
        assert(ret >= 0);

        tid = H5Dget_type(dsid);
        assert(tid > 0);

        type_equal = H5Tequal(tid, H5T_NATIVE_INT);
        assert(1 == type_equal);

        ret = H5Tclose(tid);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    for(u = 0; u < 2; u++) {
        hid_t dsid;
        hid_t sid;
        hid_t tid;
        int ndims;
        hsize_t dims[1], max_dims[1];
        htri_t type_equal;
        char dsetname[16];

        sprintf(dsetname, "3%u", u);
        dsid = H5Dopen2(fid, dsetname, H5P_DEFAULT);
        assert(dsid > 0);

        sid = H5Dget_space(dsid);
        assert(sid > 0);

        ndims = H5Sget_simple_extent_ndims(sid);
        assert(1 == ndims);

        ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
        assert(ret >= 0);
        assert(40 == dims[0]);
        assert(40 == max_dims[0]);

        ret = H5Sclose(sid);
        assert(ret >= 0);

        tid = H5Dget_type(dsid);
        assert(tid > 0);

        type_equal = H5Tequal(tid, H5T_NATIVE_INT);
        assert(1 == type_equal);

        ret = H5Tclose(tid);
        assert(ret >= 0);

        ret = H5Dclose(dsid);
        assert(ret >= 0);
    } /* end for */

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Creates file with free space manager */
/* Covers: 0A2, 0C, 1A1, 1D, 1G1, 1G2, 2A1a, 2A2a, 2A2b, 2A2d, 2A2f, 2A2i, 2A2q, 2A2r, 2A2s, 2A2x */
/* When set_driver is TRUE, also covers: 2A2u (and no 2A2q) */
int fs_mgr(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fcpl;
    hid_t fapl;
    hid_t dsid;
    hid_t sid;
    hsize_t dims[1] = {DSET_SMALL_DIMS};
    int *wdata;
    unsigned u;
    herr_t ret;

    wdata = malloc(sizeof(int) * DSET_SMALL_DIMS);
    assert(wdata);

    for(u = 0; u < DSET_SMALL_DIMS; u++)
        wdata[u] = u;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fcpl = H5Pcreate(H5P_FILE_CREATE);
    assert(fcpl > 0);

    ret = H5Pset_file_space(fcpl, H5F_FILE_SPACE_ALL_PERSIST, 1);
    assert(ret >= 0);

    fid = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl);
    assert(fid > 0);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    sid = H5Screate_simple(1, dims, NULL);
    assert(sid > 0);

    dsid = H5Dcreate2(fid, "/A", H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dsid > 0);

    ret = H5Dwrite(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
    assert(ret >= 0);

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);


    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    dsid = H5Dcreate2(fid, "/C", H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dsid > 0);

    ret = H5Dwrite(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
    assert(ret >= 0);

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);


    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    dsid = H5Dcreate2(fid, "/E", H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dsid > 0);

    ret = H5Dwrite(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
    assert(ret >= 0);

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);


    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    dsid = H5Dcreate2(fid, "/G", H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dsid > 0);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    ret = H5Dwrite(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
    assert(ret >= 0);

    free(wdata);

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

/* Verifies file w/file free space manager */
int vrfy_fs_mgr(const char *filename, hbool_t set_driver)
{
    hid_t fid;
    hid_t fapl;
    hid_t fcpl;
    hid_t dsid;
    hid_t sid;
    hid_t tid;
    H5F_file_space_type_t strategy;
    hsize_t threshold;
    int *rdata;
    int ndims;
    hsize_t dims[1], max_dims[1];
    htri_t type_equal;
    unsigned u;
    herr_t ret;

    fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl > 0);

    /* Use a non-default file driver, if requested */
    if(set_driver) {
        ret = H5Pset_fapl_family(fapl, FAMILY_SIZE, H5P_DEFAULT);
        assert(ret >= 0);
    } /* end if */

    fid = H5Fopen(filename, H5F_ACC_RDWR, fapl);
    assert(fid > 0);

    ret = H5Pclose(fapl);
    assert(ret >= 0);

    fcpl = H5Fget_create_plist(fid);
    assert(fcpl > 0);

    ret = H5Pget_file_space(fcpl, &strategy, &threshold);
    assert(ret >= 0);
    assert(H5F_FILE_SPACE_ALL_PERSIST == strategy);
    assert(1 == threshold);

    ret = H5Pclose(fcpl);
    assert(ret >= 0);

    rdata = malloc(sizeof(int) * DSET_SMALL_DIMS);
    assert(rdata);

    dsid = H5Dopen2(fid, "/A", H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_SMALL_DIMS == dims[0]);
    assert(DSET_SMALL_DIMS == max_dims[0]);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    type_equal = H5Tequal(tid, H5T_NATIVE_INT);
    assert(1 == type_equal);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dread(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
    assert(ret >= 0);

    for(u = 0; u < DSET_SMALL_DIMS; u++)
        assert(u == rdata[u]);

    ret = H5Dclose(dsid);
    assert(ret >= 0);


    dsid = H5Dopen2(fid, "/C", H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_SMALL_DIMS == dims[0]);
    assert(DSET_SMALL_DIMS == max_dims[0]);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    type_equal = H5Tequal(tid, H5T_NATIVE_INT);
    assert(1 == type_equal);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dread(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
    assert(ret >= 0);

    for(u = 0; u < DSET_SMALL_DIMS; u++)
        assert(u == rdata[u]);

    ret = H5Dclose(dsid);
    assert(ret >= 0);


    dsid = H5Dopen2(fid, "/E", H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_SMALL_DIMS == dims[0]);
    assert(DSET_SMALL_DIMS == max_dims[0]);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    type_equal = H5Tequal(tid, H5T_NATIVE_INT);
    assert(1 == type_equal);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dread(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
    assert(ret >= 0);

    for(u = 0; u < DSET_SMALL_DIMS; u++)
        assert(u == rdata[u]);

    ret = H5Dclose(dsid);
    assert(ret >= 0);


    dsid = H5Dopen2(fid, "/G", H5P_DEFAULT);
    assert(dsid > 0);

    sid = H5Dget_space(dsid);
    assert(sid > 0);

    ndims = H5Sget_simple_extent_ndims(sid);
    assert(1 == ndims);

    ret = H5Sget_simple_extent_dims(sid, dims, max_dims);
    assert(ret >= 0);
    assert(DSET_SMALL_DIMS == dims[0]);
    assert(DSET_SMALL_DIMS == max_dims[0]);

    ret = H5Sclose(sid);
    assert(ret >= 0);

    tid = H5Dget_type(dsid);
    assert(tid > 0);

    type_equal = H5Tequal(tid, H5T_NATIVE_INT);
    assert(1 == type_equal);

    ret = H5Tclose(tid);
    assert(ret >= 0);

    ret = H5Dread(dsid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
    assert(ret >= 0);

    for(u = 0; u < DSET_SMALL_DIMS; u++)
        assert(u == rdata[u]);

    free(rdata);

    ret = H5Dclose(dsid);
    assert(ret >= 0);

    ret = H5Fclose(fid);
    assert(ret >= 0);

    return(0);
}

#if 0
int main(int argc, const char *argv[])
{
    unsigned u, v;              /* Local index variables */

    printf("Generating Files\n");

    /* Create files with different superblock versions & driver info */
    for(u = 0; u < 3; u++)
        for(v = 0; v < 2; v++) {
            /* Create appropriate file */
            if(0 == u)
                file_superblock0(FILENAME[u][v], v);
            else if(1 == u)
                file_superblock1(FILENAME[u][v], v);
            else 
                file_superblock2(FILENAME[u][v], v);

            /* Add & verify an empty "new style" group to all files */
            ns_grp_0(FILENAME[u][v], v, "/A");
            vrfy_ns_grp_0(FILENAME[u][v], v, "/A");

            /* Add & verify a compact "new style" group (3 link messages) to all files */
            ns_grp_c(FILENAME[u][v], v, "/B", 3);
            vrfy_ns_grp_c(FILENAME[u][v], v, "/B", 3);

            /* Add & verify a dense "new style" group (w/300 links, in v2 B-tree & fractal heap) to all files */
            ns_grp_d(FILENAME[u][v], v, "/C", 300);
            vrfy_ns_grp_d(FILENAME[u][v], v, "/C", 300);

            /* Add & verify an empty "old style" group to all files */
            os_grp_0(FILENAME[u][v], v, "/D");
            vrfy_os_grp_0(FILENAME[u][v], v, "/D");

            /* Add & verify an "old style" group (w/300 links, in v1 B-tree & local heap) to all files */
            os_grp_n(FILENAME[u][v], v, "/E", 300);
            vrfy_os_grp_n(FILENAME[u][v], v, "/E", 300);

            /* Add & verify a contiguous dataset w/integer datatype (but no data) to all files */
            ds_ctg_i(FILENAME[u][v], v, "/F", false);
            vrfy_ds_ctg_i(FILENAME[u][v], v, "/F", false);

            /* Add & verify a contiguous dataset w/integer datatype (and data) to all files */
            ds_ctg_i(FILENAME[u][v], v, "/G", true);
            vrfy_ds_ctg_i(FILENAME[u][v], v, "/G", true);

            /* Add & verify a chunked dataset w/integer datatype (but no data) to all files */
            ds_chk_i(FILENAME[u][v], v, "/H", false);
            vrfy_ds_chk_i(FILENAME[u][v], v, "/H", false);

            /* Add & verify a chunked dataset w/integer datatype (and data) to all files */
            ds_chk_i(FILENAME[u][v], v, "/I", true);
            vrfy_ds_chk_i(FILENAME[u][v], v, "/I", true);

            /* Add & verify a compact dataset w/integer datatype (but no data) to all files */
            ds_cpt_i(FILENAME[u][v], v, "/J", false);
            vrfy_ds_cpt_i(FILENAME[u][v], v, "/J", false);

            /* Add & verify a compact dataset w/integer datatype (and data) to all files */
            ds_cpt_i(FILENAME[u][v], v, "/K", true);
            vrfy_ds_cpt_i(FILENAME[u][v], v, "/K", true);

            /* Add & verify a contiguous dataset w/variable-length datatype (but no data) to all files */
            ds_ctg_v(FILENAME[u][v], v, "/L", false);
            vrfy_ds_ctg_v(FILENAME[u][v], v, "/L", false);

            /* Add & verify a contiguous dataset w/variable-length datatype (and data) to all files */
            ds_ctg_v(FILENAME[u][v], v, "/M", true);
            vrfy_ds_ctg_v(FILENAME[u][v], v, "/M", true);

            /* I don't think adding chunked & compact datasets w/variable-length
             *  datatypes adds enough value to create those generators -QAK
             */
        } /* end for */


    /* Create files with shared object header messages (list form) */
    for(v = 0; v < 2; v++) {
        sh_msg_l(SOHM_FILENAME[0][v], v);
        vrfy_sh_msg_l(SOHM_FILENAME[0][v], v);
    } /* end for */

    /* Create files with shared object header messages (B-tree form) */
    for(v = 0; v < 2; v++) {
        sh_msg_b(SOHM_FILENAME[1][v], v);
        vrfy_sh_msg_b(SOHM_FILENAME[1][v], v);
    } /* end for */


    /* Create files with free space manager */
    for(v = 0; v < 2; v++) {
        fs_mgr(FSM_FILENAME[v], v);
        vrfy_fs_mgr(FSM_FILENAME[v], v);
    } /* end for */

    printf("Finished\n");
    return(0);
}
#endif
