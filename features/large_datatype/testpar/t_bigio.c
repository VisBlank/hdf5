
#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants definitions */
#define MAX_ERR_REPORT  10      /* Maximum number of errors reported */

/* Define some handy debugging shorthands, routines, ... */
/* debugging tools */

/* Print message mesg if verbose level is at least medium and
 * mesg is not an empty string.
 */
#define MESG(mesg)                                                     \
    if (VERBOSE_MED && *mesg != '\0')                                  \
	printf("%s\n", mesg)

/* 
 * VRFY: Verify if the condition val is true.
 * If it is true, then call MESG to print mesg, depending on the verbose
 * level.
 * If val is not true, it prints error messages and if the verbose
 * level is lower than medium, it calls MPI_Abort to abort the program.
 * If verbose level is at least medium, it will not abort.
 * This will allow program to continue and can be used for debugging.
 * (The "do {...} while(0)" is to group all the statements as one unit.)
 */
#define VRFY(val, mesg) do {                                            \
    if (val) {                                                          \
	MESG(mesg);                                                     \
    } else {                                                            \
        printf("Proc %d: ", mpi_rank);                                  \
        printf("*** Parallel ERROR ***\n");                             \
        printf("    VRFY (%s) failed at line %4d in %s\n",              \
               mesg, (int)__LINE__, __FILE__);                          \
        ++nerrors;                                                      \
        fflush(stdout);                                                 \
        if (!VERBOSE_MED) {                                             \
            printf("aborting MPI processes\n");                         \
            MPI_Abort(MPI_COMM_WORLD, 1);                               \
        }                                                               \
    }                                                                   \
} while(0)

/*
 * Checking for information purpose.
 * If val is false, print mesg; else nothing.
 * Either case, no error setting.
 */
#define INFO(val, mesg) do {                                            \
    if (val) {                                                          \
	MESG(mesg);                                                 \
    } else {                                                            \
        printf("Proc %d: ", mpi_rank);                                  \
        printf("*** PHDF5 REMARK (not an error) ***\n");                \
        printf("        Condition (%s) failed at line %4d in %s\n",     \
               mesg, (int)__LINE__, __FILE__);                          \
        fflush(stdout);                                                 \
    }                                                                   \
} while(0)

#define MPI_BANNER(mesg) do {                                           \
    if (VERBOSE_MED || MAINPROCESS){                                    \
	printf("--------------------------------\n");                   \
	printf("Proc %d: ", mpi_rank);                                  \
	printf("*** %s\n", mesg);                                       \
	printf("--------------------------------\n");                   \
    }                                                                   \
} while(0)

#define MAINPROCESS     (!mpi_rank) /* define process 0 as main process */


/* Constants definitions */
#define RANK		2

/* Hyperslab layout styles */
#define BYROW           1       /* divide into slabs of rows */
#define BYCOL           2       /* divide into blocks of columns */
#define ZROW            3       /* same as BYCOL except process 0 gets 0 rows */
#define ZCOL            4       /* same as BYCOL except process 0 gets 0 columns */
#define VERBOSE_MED 0
#define IN_ORDER     1
#define OUT_OF_ORDER 2

#define DATASET1 "DSET1"
#define DATASET2 "DSET2"
#define DATASET3 "DSET3"
#define DATASET4 "DSET4"
#define DATASET5 "DSET5"
#define DXFER_COLLECTIVE_IO 0x1  /* Collective IO*/
#define DXFER_INDEPENDENT_IO 0x2 /* Independent IO collectively */

/* Dataset data type.  Int's can be easily octo dumped. */
typedef hsize_t DATATYPE;

int dxfer_coll_type = DXFER_COLLECTIVE_IO;
size_t bigcount = 536870916;
char filename[20] = "bigio_test.h5";
int nerrors = 0;
int mpi_size, mpi_rank;


/*
 * Setup the coordinates for point selection.
 */
void point_set(hsize_t start[],
               hsize_t count[],
               hsize_t stride[],
               hsize_t block[],
               size_t num_points, 
               hsize_t coords[],
               int order)
{
    hsize_t i,j, k = 0, m ,n, s1 ,s2;

    if(OUT_OF_ORDER == order)
        k = (num_points * RANK) - 1;
    else if(IN_ORDER == order)
        k = 0;

    s1 = start[0];
    s2 = start[1];

    for(i = 0 ; i < count[0]; i++)
        for(j = 0 ; j < count[1]; j++)
            for(m = 0 ; m < block[0]; m++)
                for(n = 0 ; n < block[1]; n++)
                    if(OUT_OF_ORDER == order) {
                        coords[k--] = s2 + (stride[1] * j) + n;
                        coords[k--] = s1 + (stride[0] * i) + m;
                    }
                    else if(IN_ORDER == order) {
                        coords[k++] = s1 + stride[0] * i + m;
                        coords[k++] = s2 + stride[1] * j + n;
                    }
}

/*
 * Fill the dataset with trivial data for testing.
 * Assume dimension rank is 2 and data is stored contiguous.
 */
static void
dataset_fill(hsize_t start[], hsize_t block[], DATATYPE * dataset)
{
    DATATYPE *dataptr = dataset;
    hsize_t i, j;

    /* put some trivial data in the data_array */
    for (i=0; i < block[0]; i++){
	for (j=0; j < block[1]; j++){
	    *dataptr = (DATATYPE)((i+start[0])*100 + (j+start[1]+1));
	    dataptr++;
	}
    }
}


/*
 * Print the content of the dataset.
 */
static void
dataset_print(hsize_t start[], hsize_t block[], DATATYPE * dataset)
{
    DATATYPE *dataptr = dataset;
    hsize_t i, j;

    /* print the column heading */
    printf("%-8s", "Cols:");
    for (j=0; j < block[1]; j++){
	printf("%3lu ", (unsigned long)(start[1]+j));
    }
    printf("\n");

    /* print the slab data */
    for (i=0; i < block[0]; i++){
	printf("Row %2lu: ", (unsigned long)(i+start[0]));
	for (j=0; j < block[1]; j++){
	    printf("%03d ", *dataptr++);
	}
	printf("\n");
    }
}


/*
 * Print the content of the dataset.
 */
int
dataset_vrfy(hsize_t start[], hsize_t count[], hsize_t stride[], hsize_t block[], DATATYPE *dataset, DATATYPE *original)
{
    hsize_t i, j;
    int vrfyerrs;

    /* print it if VERBOSE_MED */
    if(VERBOSE_MED) {
	printf("dataset_vrfy dumping:::\n");
	printf("start(%lu, %lu), count(%lu, %lu), stride(%lu, %lu), block(%lu, %lu)\n",
	    (unsigned long)start[0], (unsigned long)start[1], (unsigned long)count[0], (unsigned long)count[1],
	    (unsigned long)stride[0], (unsigned long)stride[1], (unsigned long)block[0], (unsigned long)block[1]);
	printf("original values:\n");
	dataset_print(start, block, original);
	printf("compared values:\n");
	dataset_print(start, block, dataset);
    }

    vrfyerrs = 0;
    for (i=0; i < block[0]; i++){
	for (j=0; j < block[1]; j++){
	    if(*dataset != *original){
		if(vrfyerrs++ < MAX_ERR_REPORT || VERBOSE_MED){
		    printf("Dataset Verify failed at [%lu][%lu](row %lu, col %lu): expect %d, got %d\n",
			(unsigned long)i, (unsigned long)j,
			(unsigned long)(i+start[0]), (unsigned long)(j+start[1]),
			*(original), *(dataset));
		}
		dataset++;
		original++;
	    }
	}
    }
    if(vrfyerrs > MAX_ERR_REPORT && !VERBOSE_MED)
	printf("[more errors ...]\n");
    if(vrfyerrs)
	printf("%d errors found in dataset_vrfy\n", vrfyerrs);
    return(vrfyerrs);
}

/*
 * Example of using the parallel HDF5 library to create two datasets
 * in one HDF5 file with collective parallel access support.
 * The Datasets are of sizes (number-of-mpi-processes x dim0) x dim1.
 * Each process controls only a slab of size dim0 x dim1 within each
 * dataset. [Note: not so yet.  Datasets are of sizes dim0xdim1 and
 * each process controls a hyperslab within.]
 */

void
dataset_big_write(void)
{

    hid_t xfer_plist;		/* Dataset transfer properties list */
    hid_t sid;   		/* Dataspace ID */
    hid_t file_dataspace;	/* File dataspace ID */
    hid_t mem_dataspace;	/* memory dataspace ID */
    hid_t dataset;
    hid_t datatype;		/* Datatype ID */
    hsize_t dims[RANK];   	/* dataset dim sizes */
    hsize_t start[RANK];			/* for hyperslab setting */
    hsize_t count[RANK], stride[RANK];		/* for hyperslab setting */
    hsize_t block[RANK];			/* for hyperslab setting */
    hsize_t *coords = NULL;
    int i;
    herr_t ret;         	/* Generic return value */
    hid_t fid;                  /* HDF5 file ID */
    hid_t acc_tpl;		/* File access templates */
    hsize_t h;
    size_t num_points;
    DATATYPE * wdata;


    /* allocate memory for data buffer */
    wdata = (DATATYPE *)malloc(bigcount*sizeof(DATATYPE));
    VRFY((wdata != NULL), "wdata malloc succeeded");

    /* setup file access template */
    acc_tpl = H5Pcreate (H5P_FILE_ACCESS);
    VRFY((acc_tpl >= 0), "H5P_FILE_ACCESS");
    H5Pset_fapl_mpio(acc_tpl, MPI_COMM_WORLD, MPI_INFO_NULL);

    /* create the file collectively */
    fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, acc_tpl);
    VRFY((fid >= 0), "H5Fcreate succeeded");

    /* Release file-access template */
    ret = H5Pclose(acc_tpl);
    VRFY((ret >= 0), "");


    /* Each process takes a slabs of rows. */
    printf("\nTesting Dataset1 write by ROW\n");
    /* Create a large dataset */
    dims[0] = bigcount;
    dims[1] = mpi_size;
    sid = H5Screate_simple (RANK, dims, NULL);
    VRFY((sid >= 0), "H5Screate_simple succeeded");
    dataset = H5Dcreate2(fid, DATASET1, H5T_NATIVE_LLONG, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dcreate2 succeeded");
    H5Sclose(sid);

    block[0] = dims[0]/mpi_size;
    block[1] = dims[1];
    stride[0] = block[0];
    stride[1] = block[1];
    count[0] = 1;
    count[1] = 1;
    start[0] = mpi_rank*block[0];
    start[1] = 0;

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, block, NULL);
    VRFY((mem_dataspace >= 0), "");

    /* fill the local slab with some trivial data */
    dataset_fill(start, block, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
	dataset_print(start, block, wdata);
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "H5Pcreate xfer succeeded");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pset_dxpl_mpio succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* write data collectively */
    MESG("writeAll by Row");
    { 
        int i,j,k =0;
        for (i=0; i < block[0]; i++){
            for (j=0; j < block[1]; j++){
                if(k < 10) {
                    printf("%lld ", wdata[k]);
                    k++;
                }
            }
        }
        printf("\n");
    }

    ret = H5Dwrite(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                   xfer_plist, wdata);
    VRFY((ret >= 0), "H5Dwrite dataset1 succeeded");

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);

    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");



    /* Each process takes a slabs of cols. */
    printf("\nTesting Dataset2 write by COL\n");
    /* Create a large dataset */
    dims[0] = bigcount;
    dims[1] = mpi_size;
    sid = H5Screate_simple (RANK, dims, NULL);
    VRFY((sid >= 0), "H5Screate_simple succeeded");
    dataset = H5Dcreate2(fid, DATASET2, H5T_NATIVE_LLONG, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dcreate2 succeeded");
    H5Sclose(sid);

    block[0] = dims[0];
    block[1] = dims[1]/mpi_size;
    stride[0] = block[0];
    stride[1] = block[1];
    count[0] = 1;
    count[1] = 1;
    start[0] = 0;
    start[1] = mpi_rank*block[1];

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, block, NULL);
    VRFY((mem_dataspace >= 0), "");

    /* fill the local slab with some trivial data */
    dataset_fill(start, block, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
	dataset_print(start, block, wdata);
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "H5Pcreate xfer succeeded");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pset_dxpl_mpio succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* write data collectively */
    MESG("writeAll by Col");
    { 
        int i,j,k =0;
        for (i=0; i < block[0]; i++){
            for (j=0; j < block[1]; j++){
                if(k < 10) {
                    printf("%lld ", wdata[k]);
                    k++;
                }
            }
        }
        printf("\n");
    }

    ret = H5Dwrite(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                   xfer_plist, wdata);
    VRFY((ret >= 0), "H5Dwrite dataset1 succeeded");

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);

    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");



    /* ALL selection */
    printf("\nTesting Dataset3 write select ALL proc 0, NONE others\n");
    /* Create a large dataset */
    dims[0] = bigcount;
    dims[1] = 1;
    sid = H5Screate_simple (RANK, dims, NULL);
    VRFY((sid >= 0), "H5Screate_simple succeeded");
    dataset = H5Dcreate2(fid, DATASET3, H5T_NATIVE_LLONG, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dcreate2 succeeded");
    H5Sclose(sid);

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    if(MAINPROCESS) {
        ret = H5Sselect_all(file_dataspace);
        VRFY((ret >= 0), "H5Sset_all succeeded");
    }
    else {
        ret = H5Sselect_none(file_dataspace);
        VRFY((ret >= 0), "H5Sset_none succeeded");
    }

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, dims, NULL);
    VRFY((mem_dataspace >= 0), "");
    if(!MAINPROCESS) {
        ret = H5Sselect_none(mem_dataspace);
        VRFY((ret >= 0), "H5Sset_none succeeded");
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "H5Pcreate xfer succeeded");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pset_dxpl_mpio succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* fill the local slab with some trivial data */
    dataset_fill(start, dims, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
    }

    /* write data collectively */
    MESG("writeAll by process 0");
    { 
        int i,j,k =0;
        for (i=0; i < block[0]; i++){
            for (j=0; j < block[1]; j++){
                if(k < 10) {
                    printf("%lld ", wdata[k]);
                    k++;
                }
            }
        }
        printf("\n");
    }

    ret = H5Dwrite(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                   xfer_plist, wdata);
    VRFY((ret >= 0), "H5Dwrite dataset1 succeeded");

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);

    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");

#if 0
    /* Point selection */
    printf("\nTesting Dataset4 write point selection\n");
    /* Create a large dataset */
    dims[0] = bigcount;
    dims[1] = mpi_size * 4;
    sid = H5Screate_simple (RANK, dims, NULL);
    VRFY((sid >= 0), "H5Screate_simple succeeded");
    dataset = H5Dcreate2(fid, DATASET4, H5T_NATIVE_LLONG, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dcreate2 succeeded");
    H5Sclose(sid);

    block[0] = dims[0]/2;
    block[1] = 2;
    stride[0] = dims[0]/2;
    stride[1] = 2;
    count[0] = 1;
    count[1] = 1;
    start[0] = 0;
    start[1] = dims[1]/4 * mpi_rank;

    dataset_fill(start, block, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
	dataset_print(start, block, wdata);
    }

    num_points = bigcount;

    coords = (hsize_t *)malloc(num_points * RANK * sizeof(hsize_t));
    VRFY((coords != NULL), "coords malloc succeeded");

    point_set (start, count, stride, block, num_points, coords, IN_ORDER);
    /* create a file dataspace */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    ret = H5Sselect_elements(file_dataspace, H5S_SELECT_SET, num_points, coords);
    VRFY((ret >= 0), "H5Sselect_elements succeeded");

    if(coords) free(coords);

    /* create a memory dataspace */
    mem_dataspace = H5Screate_simple (1, &bigcount, NULL);
    VRFY((mem_dataspace >= 0), "");

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "H5Pcreate xfer succeeded");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pset_dxpl_mpio succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    ret = H5Dwrite(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                   xfer_plist, wdata);
    VRFY((ret >= 0), "H5Dwrite dataset1 succeeded");

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);

    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");



    /* Irregular selection */
    /* Need larger memory for data buffer */
    free(wdata);
    wdata = (DATATYPE *)malloc(bigcount*4*sizeof(DATATYPE));
    VRFY((wdata != NULL), "wdata malloc succeeded");

    printf("\nTesting Dataset5 write irregular selection\n");
    /* Create a large dataset */
    dims[0] = bigcount;
    dims[1] = mpi_size * 4;
    sid = H5Screate_simple (RANK, dims, NULL);
    VRFY((sid >= 0), "H5Screate_simple succeeded");
    dataset = H5Dcreate2(fid, DATASET5, H5T_NATIVE_LLONG, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dcreate2 succeeded");
    H5Sclose(sid);

    /* first select 1 col in this procs splice */
    block[0] = dims[0];
    block[1] = 1;
    stride[0] = block[0];
    stride[1] = block[1];
    count[0] = 1;
    count[1] = 1;
    start[0] = 0;
    start[1] = mpi_rank * 4;

    /* create a file dataspace */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    /* create a memory dataspace */
    mem_dataspace = H5Screate_simple (RANK, dims, NULL);
    VRFY((mem_dataspace >= 0), "");

    ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");
    ret = H5Sselect_hyperslab(mem_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");

    /* select every other row in the process splice and OR it with
       the col selection to create an irregular selection */
    for(h=0 ; h<dims[0] ; h+=2) {
        block[0] = 1;
        block[1] = 4;
        stride[0] = block[0];
        stride[1] = block[1];
        count[0] = 1;
        count[1] = 1;
        start[0] = h;
        start[1] = mpi_rank * 4;

        ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_OR, start, stride, count, block);
        VRFY((ret >= 0), "H5Sset_hyperslab succeeded");
        ret = H5Sselect_hyperslab(mem_dataspace, H5S_SELECT_OR, start, stride, count, block);
        VRFY((ret >= 0), "H5Sset_hyperslab succeeded");
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "H5Pcreate xfer succeeded");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pset_dxpl_mpio succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* fill the local slab with some trivial data */
    dataset_fill(start, dims, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
    }

    ret = H5Dwrite(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                   xfer_plist, wdata);
    VRFY((ret >= 0), "H5Dwrite dataset1 succeeded");

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);

    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");
#endif

    H5Fclose(fid);
    free(wdata);
}

/*
 * Example of using the parallel HDF5 library to read two datasets
 * in one HDF5 file with collective parallel access support.
 * The Datasets are of sizes (number-of-mpi-processes x dim0) x dim1.
 * Each process controls only a slab of size dim0 x dim1 within each
 * dataset. [Note: not so yet.  Datasets are of sizes dim0xdim1 and
 * each process controls a hyperslab within.]
 */

void
dataset_big_read(void)
{
    hid_t fid;                  /* HDF5 file ID */
    hid_t acc_tpl;		/* File access templates */
    hid_t xfer_plist;		/* Dataset transfer properties list */
    hid_t file_dataspace;	/* File dataspace ID */
    hid_t mem_dataspace;	/* memory dataspace ID */
    hid_t dataset;
    DATATYPE *rdata = NULL;	/* data buffer */
    DATATYPE *wdata = NULL; 	/* expected data buffer */
    hsize_t dims[RANK];   	/* dataset dim sizes */
    hsize_t start[RANK];			/* for hyperslab setting */
    hsize_t count[RANK], stride[RANK];		/* for hyperslab setting */
    hsize_t block[RANK];			/* for hyperslab setting */
    int i,j,k;
    hsize_t h;
    size_t num_points;
    hsize_t *coords = NULL;
    herr_t ret;         	/* Generic return value */

    /* allocate memory for data buffer */
    rdata = (DATATYPE *)malloc(bigcount*sizeof(DATATYPE));
    VRFY((rdata != NULL), "rdata malloc succeeded");
    wdata = (DATATYPE *)malloc(bigcount*sizeof(DATATYPE));
    VRFY((wdata != NULL), "wdata malloc succeeded");

    memset(rdata, 0, bigcount*sizeof(DATATYPE));

    /* setup file access template */
    acc_tpl = H5Pcreate (H5P_FILE_ACCESS);
    VRFY((acc_tpl >= 0), "H5P_FILE_ACCESS");
    H5Pset_fapl_mpio(acc_tpl, MPI_COMM_WORLD, MPI_INFO_NULL);

    /* open the file collectively */
    fid=H5Fopen(filename,H5F_ACC_RDONLY,acc_tpl);
    VRFY((fid >= 0), "H5Fopen succeeded");

    /* Release file-access template */
    ret = H5Pclose(acc_tpl);
    VRFY((ret >= 0), "");


    printf("\nRead Testing Dataset1 by COL\n");
    dataset = H5Dopen2(fid, DATASET1, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dopen2 succeeded");

    dims[0] = bigcount;
    dims[1] = mpi_size;
    /* Each process takes a slabs of cols. */
    block[0] = dims[0];
    block[1] = dims[1]/mpi_size;
    stride[0] = block[0];
    stride[1] = block[1];
    count[0] = 1;
    count[1] = 1;
    start[0] = 0;
    start[1] = mpi_rank*block[1];

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, block, NULL);
    VRFY((mem_dataspace >= 0), "");

    /* fill dataset with test data */
    dataset_fill(start, block, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pcreate xfer succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* read data collectively */
    ret = H5Dread(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                  xfer_plist, rdata);
    VRFY((ret >= 0), "H5Dread dataset1 succeeded");

    { 
        int i,j,k =0;
        for (i=0; i < block[0]; i++){
            for (j=0; j < block[1]; j++){
                if(k < 10) {
                    printf("%lld ", rdata[k]);
                    k++;
                }
            }
        }
        printf("\n");
    }

    /* verify the read data with original expected data */
    ret = dataset_vrfy(start, count, stride, block, rdata, wdata);
    if(ret) {fprintf(stderr, "verify failed\n"); exit(1);}

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);
    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");


    printf("\nRead Testing Dataset2 by ROW\n");
    memset(rdata, 0, bigcount*sizeof(DATATYPE));
    dataset = H5Dopen2(fid, DATASET2, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dopen2 succeeded");

    dims[0] = bigcount;
    dims[1] = mpi_size;
    /* Each process takes a slabs of rows. */
    block[0] = dims[0]/mpi_size;
    block[1] = dims[1];
    stride[0] = block[0];
    stride[1] = block[1];
    count[0] = 1;
    count[1] = 1;
    start[0] = mpi_rank*block[0];
    start[1] = 0;

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, block, NULL);
    VRFY((mem_dataspace >= 0), "");

    /* fill dataset with test data */
    dataset_fill(start, block, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pcreate xfer succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* read data collectively */
    ret = H5Dread(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                  xfer_plist, rdata);
    VRFY((ret >= 0), "H5Dread dataset2 succeeded");

    { 
        int i,j,k =0;
        for (i=0; i < block[0]; i++){
            for (j=0; j < block[1]; j++){
                if(k < 10) {
                    printf("%lld ", rdata[k]);
                    k++;
                }
            }
        }
        printf("\n");
    }

    /* verify the read data with original expected data */
    ret = dataset_vrfy(start, count, stride, block, rdata, wdata);
    if(ret) {fprintf(stderr, "verify failed\n"); exit(1);}

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);
    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");


    printf("\nRead Testing Dataset3 read select ALL proc 0, NONE others\n");
    memset(rdata, 0, bigcount*sizeof(DATATYPE));
    dataset = H5Dopen2(fid, DATASET3, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dopen2 succeeded");

    dims[0] = bigcount;
    dims[1] = 1;

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    if(MAINPROCESS) {
        ret = H5Sselect_all(file_dataspace);
        VRFY((ret >= 0), "H5Sset_all succeeded");
    }
    else {
        ret = H5Sselect_none(file_dataspace);
        VRFY((ret >= 0), "H5Sset_none succeeded");
    }

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, dims, NULL);
    VRFY((mem_dataspace >= 0), "");
    if(!MAINPROCESS) {
        ret = H5Sselect_none(mem_dataspace);
        VRFY((ret >= 0), "H5Sset_none succeeded");
    }

    /* fill dataset with test data */
    dataset_fill(start, dims, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pcreate xfer succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* read data collectively */
    ret = H5Dread(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                  xfer_plist, rdata);
    VRFY((ret >= 0), "H5Dread dataset3 succeeded");

    { 
        int i,j,k =0;
        for (i=0; i < block[0]; i++){
            for (j=0; j < block[1]; j++){
                if(k < 10) {
                    printf("%lld ", rdata[k]);
                    k++;
                }
            }
        }
        printf("\n");
    }

    if(MAINPROCESS) {
        /* verify the read data with original expected data */
        ret = dataset_vrfy(start, count, stride, block, rdata, wdata);
        if(ret) {fprintf(stderr, "verify failed\n"); exit(1);}
    }

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);
    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");

#if 0
    printf("\nRead Testing Dataset4 with Point selection\n");
    dataset = H5Dopen2(fid, DATASET4, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dopen2 succeeded");

    dims[0] = bigcount;
    dims[1] = mpi_size * 4;

    block[0] = dims[0]/2;
    block[1] = 2;
    stride[0] = dims[0]/2;
    stride[1] = 2;
    count[0] = 1;
    count[1] = 1;
    start[0] = 0;
    start[1] = dims[1]/4 * mpi_rank;

    dataset_fill(start, block, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
	dataset_print(start, block, wdata);
    }

    num_points = bigcount;

    coords = (hsize_t *)malloc(num_points * RANK * sizeof(hsize_t));
    VRFY((coords != NULL), "coords malloc succeeded");

    point_set (start, count, stride, block, num_points, coords, IN_ORDER);
    /* create a file dataspace */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    ret = H5Sselect_elements(file_dataspace, H5S_SELECT_SET, num_points, coords);
    VRFY((ret >= 0), "H5Sselect_elements succeeded");

    if(coords) free(coords);

    /* create a memory dataspace */
    mem_dataspace = H5Screate_simple (1, &bigcount, NULL);
    VRFY((mem_dataspace >= 0), "");

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pcreate xfer succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* read data collectively */
    ret = H5Dread(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                  xfer_plist, rdata);
    VRFY((ret >= 0), "H5Dread dataset1 succeeded");

    ret = dataset_vrfy(start, count, stride, block, rdata, wdata);
    if(ret) {fprintf(stderr, "verify failed\n"); exit(1);}

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);
    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");

    printf("\nRead Testing Dataset5 with Irregular selection\n");
    /* Need larger memory for data buffer */
    free(wdata);
    wdata = (DATATYPE *)malloc(bigcount*4*sizeof(DATATYPE));
    VRFY((wdata != NULL), "wdata malloc succeeded");

    free(rdata);
    rdata = (DATATYPE *)malloc(bigcount*4*sizeof(DATATYPE));
    VRFY((rdata != NULL), "rdata malloc succeeded");

    dataset = H5Dopen2(fid, DATASET5, H5P_DEFAULT);
    VRFY((dataset >= 0), "H5Dopen2 succeeded");

    dims[0] = bigcount;
    dims[1] = mpi_size * 4;

    /* first select 1 col in this proc splice */
    block[0] = dims[0];
    block[1] = 1;
    stride[0] = block[0];
    stride[1] = block[1];
    count[0] = 1;
    count[1] = 1;
    start[0] = 0;
    start[1] = mpi_rank * 4;

    /* create a file dataspace */
    file_dataspace = H5Dget_space (dataset);
    VRFY((file_dataspace >= 0), "H5Dget_space succeeded");
    /* create a memory dataspace */
    mem_dataspace = H5Screate_simple (RANK, dims, NULL);
    VRFY((mem_dataspace >= 0), "");

    ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");
    ret = H5Sselect_hyperslab(mem_dataspace, H5S_SELECT_SET, start, stride, count, block);
    VRFY((ret >= 0), "H5Sset_hyperslab succeeded");

    /* select every other row in the process splice and OR it with
       the col selection to create an irregular selection */
    for(h=0 ; h<dims[0] ; h+=2) {
        block[0] = 1;
        block[1] = 4;
        stride[0] = block[0];
        stride[1] = block[1];
        count[0] = 1;
        count[1] = 1;
        start[0] = h;
        start[1] = mpi_rank * 4;

        ret = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_OR, start, stride, count, block);
        VRFY((ret >= 0), "H5Sset_hyperslab succeeded");
        ret = H5Sselect_hyperslab(mem_dataspace, H5S_SELECT_OR, start, stride, count, block);
        VRFY((ret >= 0), "H5Sset_hyperslab succeeded");
    }

    /* fill dataset with test data */
    dataset_fill(start, block, wdata);
    MESG("data_array initialized");
    if(VERBOSE_MED){
	MESG("data_array created");
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "");
    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pcreate xfer succeeded");
    if(dxfer_coll_type == DXFER_INDEPENDENT_IO) {
        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0),"set independent IO collectively succeeded");
    }

    /* read data collectively */
    ret = H5Dread(dataset, H5T_NATIVE_LLONG, mem_dataspace, file_dataspace,
                  xfer_plist, rdata);
    VRFY((ret >= 0), "H5Dread dataset1 succeeded");

    /* verify the read data with original expected data */
    block[0] = dims[0];
    block[1] = 1;
    stride[0] = block[0];
    stride[1] = block[1];
    count[0] = 1;
    count[1] = 1;
    start[0] = 0;
    start[1] = mpi_rank * 4;
    ret = dataset_vrfy(start, count, stride, block, rdata, wdata);
    if(ret) {fprintf(stderr, "verify failed\n"); exit(1);}

    for(h=0 ; h<dims[0] ; h+=2) {
        block[0] = 1;
        block[1] = 4;
        stride[0] = block[0];
        stride[1] = block[1];
        count[0] = 1;
        count[1] = 1;
        start[0] = h;
        start[1] = mpi_rank * 4;
        ret = dataset_vrfy(start, count, stride, block, rdata, wdata);
        if(ret) {fprintf(stderr, "verify failed\n"); exit(1);}
    }

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);
    ret = H5Dclose(dataset);
    VRFY((ret >= 0), "H5Dclose1 succeeded");
#endif

    H5Fclose(fid);

    /* release data buffers */
    if(rdata) free(rdata);
    if(wdata) free(wdata);
} /* dataset_large_readAll */

int main(int argc, char **argv) 
{

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);

    dataset_big_write();
    MPI_Barrier(MPI_COMM_WORLD);

    dataset_big_read();
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}

/*  LocalWords:  stdout
 */
