/* $Id$ */

/*
 * Parallel tests for file operations
 */

#include <testphdf5.h>

/*
 * test file access by communicator besides COMM_WORLD.
 * Split COMM_WORLD into two, one (even_comm) contains the original
 * processes of even ranks.  The other (odd_comm) contains the original
 * processes of odd ranks.  Processes in even_comm creates a file, then
 * cloose it, using even_comm.  Processes in old_comm just do a barrier
 * using odd_comm.  Then they all do a barrier using COMM_WORLD.
 * If the file creation and cloose does not do correct collective action
 * according to the communicator argument, the processes will freeze up
 * sooner or later due to barrier mixed up.
 */
void
test_split_comm_access(char *filename[])
{
    int mpi_size, mpi_rank;
    MPI_Comm comm;
    MPI_Info info = MPI_INFO_NULL;
    int color, mrc;
    int newrank, newprocs;
    hid_t fid;			/* file IDs */
    hid_t acc_tpl;		/* File access properties */
    herr_t ret;			/* generic return value */

    if (verbose)
	printf("Split Communicator access test on file %s %s\n",
	    filename[0], filename[1]);

    /* set up MPI parameters */
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);
    color = mpi_rank%2;
    mrc = MPI_Comm_split (MPI_COMM_WORLD, color, mpi_rank, &comm);
    VRFY((mrc==MPI_SUCCESS), "");
    MPI_Comm_size(comm,&newprocs);
    MPI_Comm_rank(comm,&newrank);

    if (color){
	/* odd-rank processes */
	mrc = MPI_Barrier(comm);
	VRFY((mrc==MPI_SUCCESS), "");
    }else{
	/* even-rank processes */
	int sub_mpi_rank;	/* rank in the sub-comm */
	MPI_Comm_rank(comm,&sub_mpi_rank);

	/* setup file access template */
	acc_tpl = H5Pcreate (H5P_FILE_ACCESS);
	VRFY((acc_tpl != FAIL), "");
	
	/* set Parallel access with communicator */
	ret = H5Pset_mpi(acc_tpl, comm, info);     
	VRFY((ret != FAIL), "");

	/* create the file collectively */
	fid=H5Fcreate(filename[color],H5F_ACC_TRUNC,H5P_DEFAULT,acc_tpl);
	VRFY((fid != FAIL), "H5Fcreate succeeded");

	/* Release file-access template */
	ret=H5Pclose(acc_tpl);
	VRFY((ret != FAIL), "");

	/* close the file */
	ret=H5Fclose(fid);
	VRFY((ret != FAIL), "");

	/* detele the test file */
	if (sub_mpi_rank == 0){
	    mrc = MPI_File_delete(filename[color], info);
	    VRFY((mrc==MPI_SUCCESS), "");
	}
    }
}



/*
 * MPIO independent overlapping writes.
 *
 * First n-1 processes open 1 file
 * Each of the n-1 process write a byte to the file in round-robin
 * fashion.  E.g. process 0 writes byte (0, n-1, n-1+n-1, ...),
 * process 1 write bytes (1, n, n+n-1, ...)
 * Last process (n-1) just waits
 * First n-1 processes finish writing and close the file.
 * Last process opens the same file and verifies the data.
 */

#define MPIO_TEST_WRITE_SIZE 512*1024     /* 1/2 MB */

void
test_mpio_overlap_writes(char *filename[])
{
    int mpi_size, mpi_rank;
    MPI_Comm comm;
    MPI_Info info = MPI_INFO_NULL;
    int color, mrc;
    MPI_File	fh;
    int newrank, newprocs;
    hid_t fid;			/* file IDs */
    hid_t acc_tpl;		/* File access properties */
    herr_t ret;			/* generic return value */
    int i;
    char  byte;
    MPI_Offset  mpi_off;
    MPI_Status  mpi_stat;


    if (verbose)
	printf("MPIO independent overlapping writes test on file %s\n",
	    filename[0]);

    /* set up MPI parameters */
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);

    /* Need at least 2 processes */
    VRFY((mpi_size >= 2), "Has at least 2 processes");

    /* splits processes 0 to n-2 into one comm. and the last one into another */
    color = ((mpi_rank < (mpi_size - 1)) ? 0 : 1);
    mrc = MPI_Comm_split (MPI_COMM_WORLD, color, mpi_rank, &comm);
    VRFY((mrc==MPI_SUCCESS), "Comm_split succeeded");

    if (color==0){
	/* First n-1 processes (color==0) open a file and write it */
	mrc = MPI_File_open(comm, filename[0], MPI_MODE_CREATE|MPI_MODE_RDWR,
		info, &fh);
	VRFY((mrc==MPI_SUCCESS), "");

	for (mpi_off=mpi_rank; mpi_off < MPIO_TEST_WRITE_SIZE;
	    mpi_off += (mpi_size - 1)){
		byte = mpi_off & 0x7f;
		mrc = MPI_File_write_at(fh, mpi_off, &byte, 1, MPI_BYTE, &mpi_stat);
		VRFY((mrc==MPI_SUCCESS), "");
	}

	/* close file and free the communicator */
	mrc = MPI_File_close(&fh);
	VRFY((mrc==MPI_SUCCESS), "MPI_FILE_CLOSE");
	mrc = MPI_Comm_free(&comm);
	VRFY((mrc==MPI_SUCCESS), "MPI_Comm_free");
    
	/* sync with the other waiting processes */
	mrc = MPI_Barrier(MPI_COMM_WORLD);
	VRFY((mrc==MPI_SUCCESS), "Sync after writes");
    }else{
	/* other processes waits till writes are done,
	 * then open file to verify data.
	 */
	mrc = MPI_Barrier(MPI_COMM_WORLD);
	VRFY((mrc==MPI_SUCCESS), "Sync after writes");

	mrc = MPI_File_open(comm, filename[0], MPI_MODE_RDONLY,
		info, &fh);
	VRFY((mrc==MPI_SUCCESS), "");

	for (mpi_off=0; mpi_off < MPIO_TEST_WRITE_SIZE; mpi_off++){
	    mrc = MPI_File_read_at(fh, mpi_off, &byte, 1, MPI_BYTE, &mpi_stat);
	    VRFY((mrc==MPI_SUCCESS), "");
	    if (byte!=(mpi_off & 0x7f))
		printf("proc %d: found data error, expect %d, got %d\n",
		    mpi_rank, mpi_off & 0x7f, byte);
	}

	/* close file and free the communicator */
	mrc = MPI_File_close(&fh);
	VRFY((mrc==MPI_SUCCESS), "MPI_FILE_CLOSE");
	mrc = MPI_Comm_free(&comm);
	VRFY((mrc==MPI_SUCCESS), "MPI_Comm_free");
    }

    /*
     * one more sync to ensure all processes have done reading
     * before ending this test.
     */
    mrc = MPI_Barrier(MPI_COMM_WORLD);
    VRFY((mrc==MPI_SUCCESS), "Sync before leaving test");
}

