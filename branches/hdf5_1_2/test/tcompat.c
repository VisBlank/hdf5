/****************************************************************************
 * NCSA HDF								                                    *
 * Software Development Group						                        *
 * National Center for Supercomputing Applications			                *
 * University of Illinois at Urbana-Champaign				                *
 * 605 E. Springfield, Champaign IL 61820				                    *
 *									                                        *
 * For conditions of distribution and use, see the accompanying		        *
 * hdf/COPYING file.							                            *
 *									                                        *
 ****************************************************************************/

#ifdef RCSID
static char		RcsId[] = "$Revision$";
#endif

/* $Id$ */

/***********************************************************
*
* Test program:	 tcompat
*
* Test the Datatype compatibility functionality
*
*************************************************************/

#include <testhdf5.h>

#include <hdf5.h>

#define TESTFILE   "tarrnew.h5"

/****************************************************************
**
**  test_compat(): Main datatype compatibility testing routine.
** 
****************************************************************/
void 
test_compat(void)
{
    char testfile[512]="";          /* Character buffer for corrected test file name */
    char *srcdir = getenv("srcdir");    /* Pointer to the directory the source code is located within */
    hid_t		fid1;		/* HDF5 File IDs		*/
    hid_t		dataset;	/* Dataset ID			*/
    herr_t		ret;		/* Generic return value		*/

    /* Output message about test being performed */
    MESSAGE(5, ("Testing Datatypes Compatibility\n"));

    /*
     * Try reading a file that has been prepared that has datasets with 
     * compound datatypes which use an newer version (version 2) of the
     * datatype object header message for describing the datatype.
     *
     * If this test fails and the datatype object header message version has
     *  changed, follow the instructions in gen_new_array.c (in the 1.3+ code
     *  branch) for regenerating the tarrnew.h5 file.
     */
    /* Generate the correct name for the test file, by prepending the source path */
    if (srcdir && ((strlen(srcdir) + strlen(TESTFILE) + 1) < sizeof(testfile))) {
        strcpy(testfile, srcdir);
        strcat(testfile, "/");
    }
    strcat(testfile, TESTFILE);

    /* Open the testfile */
    fid1 = H5Fopen(testfile, H5F_ACC_RDONLY, H5P_DEFAULT);
    CHECK_I(fid1, "H5Fopen");

    /* Only try to proceed if the file is around */
    if (fid1 >= 0){
        /* Try to open the first dataset (with compound datatype of array fields), should fail */
        dataset = H5Dopen(fid1, "Dataset1");
        VERIFY(dataset, FAIL, "H5Dopen");

        /* Try to open the second dataset (with array datatype), should fail */
        dataset = H5Dopen(fid1, "Dataset2");
        VERIFY(dataset, FAIL, "H5Dopen");

        /* Close the file */
        ret = H5Fclose(fid1);
        CHECK_I(ret, "H5Fclose");
    }
    else
        printf("***cannot open the pre-created array datatype test file (%s)\n",testfile);

}   /* test_compat() */


/*-------------------------------------------------------------------------
 * Function:	cleanup_compat
 *
 * Purpose:	Cleanup temporary test files
 *
 * Return:	none
 *
 * Programmer:	Quincey Koziol
 *              June 8, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
cleanup_compat(void)
{
}

