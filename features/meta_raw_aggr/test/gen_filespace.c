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

#include "hdf5.h"
#include <assert.h>

#define NELMTS(X)    	(sizeof(X)/sizeof(X[0]))	/* # of elements */

const char *FILENAMES[] = {
    "aggr_nopersist.h5",	/* H5F_FILE_SPACE_AGGR + not persisting free-space */
    "aggr_persist.h5",		/* H5F_FILE_SPACE_AGGR + persisting free-space */
    "paged_nopersist.h5",	/* H5F_FILE_SPACE_PAGE + not persisting free-space */
    "paged_persist.h5",		/* H5F_FILE_SPACE_PAGE + persisting free-space */
    "none_nopersist.h5",	/* H5F_FILE_SPACE_NONE + not persisting free-space */
    "none_persist.h5"		/* H5F_FILE_SPACE_NONE + persisting free-space */
};

#define DATASET		"dset"
#define NUM_ELMTS	100
#define FALSE		0
#define TRUE		1
#define INC_ENUM(TYPE,VAR) (VAR)=((TYPE)((VAR)+1))

/*
 * Compile and run this program in the trunk to generate
 * HDF5 files with combinations of 3 file space strategies
 * and persist/not persist free-space.
 * The library creates the file space info message with "mark if unknown"
 * in these files.
 *
 * Move these files to 1.8 branch for compatibility testing:
 * test_filespace_compatible() in test/tfile.c will use these files.
 *
 * Copy these files from the 1.8 branch back to the trunk for 
 * compatibility testing via test_filespace_round_compatible() in test/tfile.c.
 *
 */
static void gen_file(void)
{
    hid_t   	fid = -1;		/* File ID */
    hid_t       dataset = -1;		/* Dataset ID */
    hid_t       space = -1;		/* Dataspace ID */
    hid_t   	fcpl;			/* File creation property list */
    hsize_t     dim[1];			/* Dimension sizes */
    int         data[NUM_ELMTS];	/* Buffer for data */
    unsigned    i, j;			/* Local index variables */
    H5F_fspace_strategy_t fs_strategy;		/* File space handling strategy */
    hbool_t    	fs_persist;

    j = 0;
    for(fs_strategy = H5F_FSPACE_STRATEGY_AGGR; fs_strategy < H5F_FSPACE_STRATEGY_NTYPES; INC_ENUM(H5F_fspace_strategy_t, fs_strategy)) {
	for(fs_persist = FALSE; fs_persist <= TRUE; fs_persist++) {
	    /* Get a copy of the default file creation property */
	    fcpl = H5Pcreate(H5P_FILE_CREATE);

	    H5Pset_file_space_strategy(fcpl, fs_strategy, fs_persist, (hsize_t)1);

	    /* Create the file with the file space info */
	    fid = H5Fcreate(FILENAMES[j], H5F_ACC_TRUNC, fcpl, H5P_DEFAULT);

	    /* Create the dataset */
	    dim[0] = NUM_ELMTS;
	    space = H5Screate_simple(1, dim, NULL);
	    dataset = H5Dcreate2(fid, DATASET, H5T_NATIVE_INT, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	    for(i = 0; i < NUM_ELMTS; i++)
		data[i] = i;

	    /* Write the dataset */
	    H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

	    /* Closing */
	    H5Dclose(dataset);
	    H5Sclose(space);
	    H5Fclose(fid);
	    H5Pclose(fcpl);
	    ++j;
	}
    }
    assert(j == NELMTS(FILENAMES));
}

int main(void)
{
    gen_file();

    return 0;
}
