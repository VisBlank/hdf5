/**/
#include "h5test.h"

#ifdef H5_HAVE_AIO_H
#include <aio.h>
#endif

#define TEST_BUFFER_SIZE 100

static const char *FILENAME[] = {
    "aiotest",
    NULL
};

/**/
int
main(int argc, char *argv[])
{
    char        filename[1024], buffer[TEST_BUFFER_SIZE];
    int         retval=1, ntasks=1, self=0;
    hid_t       fapl=-1, file=-1, file_space=-1, mem_space=-1, dset=-1;
    hsize_t     dset_size, buffer_size=TEST_BUFFER_SIZE;
    hssize_t    file_offset;

#if defined(USE_ASYNC_IO) && defined(H5_HAVE_AIO_H)
    struct aiocb aio;
    memset(&aio, 0, sizeof aio);
#endif

#ifdef H5_HAVE_PARALLEL
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &self);
#endif
    
    /* Create the file */
    h5_reset();
    fapl = h5_fileaccess();
    if (H5Pset_fapl_mpiposix(fapl, MPI_COMM_WORLD)<0)
        goto error;

    h5_fixname(FILENAME[0], fapl, filename, sizeof filename);
    if ((file=H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl))<0)
        goto error;

    /* Create the dataset and selections for task-rank order writes to that dataset */
    dset_size = ntasks * buffer_size;
    if ((file_space=H5Screate_simple(1, &dset_size, NULL))<0)
        goto error;
    if ((dset=H5Dcreate(file, "dset", H5T_NATIVE_CHAR, file_space, H5P_DEFAULT))<0)
        goto error;
    if ((mem_space=H5Screate_simple(1, &buffer_size, NULL))<0)
        goto error;
    file_offset = self * buffer_size;
    if (H5Sselect_hyperslab(file_space, H5S_SELECT_SET, &file_offset, NULL, &buffer_size, NULL)<0)
        goto error;
    
    /* Arrange for H5Dwrite to be asynchronous if possible */
#if defined(USE_ASYNC_IO) && defined(H5_HAVE_PARALLEL) && defined(H5_HAVE_AIO_H)
    H5FDmpiposix_async_notify(buffer, &aio);
#endif
    
    /* Write the data */
    memset(buffer, 0xAA, sizeof buffer);
    if (H5Dwrite(dset, H5T_NATIVE_CHAR, mem_space, file_space, H5P_DEFAULT, buffer)<0)
        goto error;

    /* See if the operation was asynchronous and wait for it to complete */
#if defined(USE_ASYNC_IO) && defined(H5_HAVE_PARALLEL) && defined(H5_HAVE_AIO_H)
    if (aio.aio_buf) {
        printf("%d: write was asynchronous. Waiting for completion...\n", self);
        do {
#ifdef H5_HAVE_STRUCT_AIOCB_AIO_FILDES
            const struct aiocb *const reqp = &aio;
            int status = aio_suspend(&reqp, 1, NULL); /*POSIX*/
#else
            struct aiocb *reqp = &aio;
            int status = aio_suspend(1, &reqp); /*AIX*/
#endif
            assert(status>=0);
        } while (EINPROGRESS==aio_error(&aio));
        if (aio_error(&aio))
            fprintf(stderr, "%d: aio_error() = %d, \"%s\"\n",
                    self, aio_error(&aio), strerror(aio_error(&aio)));
        aio_return(&aio);
        memset(&aio, 0, sizeof aio);
        printf("%d: write completed.\n", self);
    } else {
        printf("%d: write was not asynchronous (but could/should have been).\n", self);
    }
#else
    printf("%d: write was not asynchronous (async I/O is not supported).\n", self);
#endif
    retval = 0;

    /* Cleanup */
 error:
    if (dset>=0)
        H5Dclose(dset);
    if (file_space>=0)
        H5Sclose(file_space);
    if (mem_space>=0)
        H5Sclose(mem_space);
    if (file>=0)
        H5Fclose(file);
    if (fapl>=0)
        h5_cleanup(FILENAME, fapl);

    H5close();
#ifdef H5_HAVE_PARALLEL
    MPI_Finalize();
#endif
    
    return retval;
}
