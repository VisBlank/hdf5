/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Programmer:	Robb Matzke <matzke@llnl.gov>
 *		Friday, October 10, 1997
 *
 * Purpose:	Hyperslab operations are rather complex, so this file
 *		attempts to test them extensively so we can be relatively
 *		sure they really work.	We only test 1d, 2d, and 3d cases
 *		because testing general dimensionalities would require us to
 *		rewrite much of the hyperslab stuff.
 */
#include "h5test.h"
#include "H5private.h"
#include "H5Eprivate.h"
#include "H5Vprivate.h"

#define TEST_SMALL	0x0001
#define TEST_MEDIUM	0x0002

#define VARIABLE_SRC	0
#define VARIABLE_DST	1
#define VARIABLE_BOTH	2

#define ARRAY_FILL_SIZE 4
#define ARRAY_OFFSET_NDIMS 3


/*-------------------------------------------------------------------------
 * Function:	init_full
 *
 * Purpose:	Initialize full array.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *		Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static unsigned
init_full(uint8_t *array, size_t nx, size_t ny, size_t nz)
{
    size_t		    i, j, k;
    uint8_t		    acc = 128;
    unsigned		    total = 0;

    for (i=0; i<nx; i++) {
	for (j=0; j<ny; j++) {
	    for (k=0; k<nz; k++) {
		total += acc;
		*array++ = acc++;
	    }
	}
    }
    return total;
}

/*-------------------------------------------------------------------------
 * Function:	print_array
 *
 * Purpose:	Prints the values in an array
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *		Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
print_array(uint8_t *array, size_t nx, size_t ny, size_t nz)
{
    size_t	i, j, k;

    for (i=0; i<nx; i++) {
	if (nz>1) {
	    printf("i=%lu:\n", (unsigned long)i);
	} else {
	    printf("%03lu:", (unsigned long)i);
	}

	for (j=0; j<ny; j++) {
	    if (nz>1)
		printf("%03lu:", (unsigned long)j);
	    for (k=0; k<nz; k++) {
		printf(" %3d", *array++);
	    }
	    if (nz>1)
		printf("\n");
	}
	printf("\n");
    }
}

/*-------------------------------------------------------------------------
 * Function:	print_ref
 *
 * Purpose:	Prints the reference value
 *
 * Return:	Success:	0
 *
 *		Failure:	
 *
 * Programmer:	Robb Matzke
 *		Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
print_ref(size_t nx, size_t ny, size_t nz)
{
    uint8_t		   *array;

    array = HDcalloc(nx*ny*nz,sizeof(uint8_t));

    printf("Reference array:\n");
    init_full(array, nx, ny, nz);
    print_array(array, nx, ny, nz);
}

/*-------------------------------------------------------------------------
 * Function:	test_fill
 *
 * Purpose:	Tests the H5V_hyper_fill() function.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Saturday, October 11, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_fill(size_t nx, size_t ny, size_t nz,
	  size_t di, size_t dj, size_t dk,
	  size_t ddx, size_t ddy, size_t ddz)
{
    uint8_t	*dst = NULL;		/*destination array		*/
    hsize_t	hs_size[3];	    	/*hyperslab size		*/
    hsize_t	dst_size[3];	    	/*destination total size	*/
    hssize_t	dst_offset[3];	    	/*offset of hyperslab in dest   */
    unsigned	ref_value;  		/*reference value		*/
    unsigned	acc;	    		/*accumulator		    	*/
    size_t	i, j, k, dx, dy, dz;	/*counters		   	*/
    size_t	u, v, w;
    unsigned		ndims;			/*hyperslab dimensionality	*/
    char	dim[64], s[256];    	/*temp string		    	*/
    unsigned	fill_value;	    	/*fill value		    	*/

    /*
     * Dimensionality.
     */
    if (0 == nz) {
	if (0 == ny) {
	    ndims = 1;
	    ny = nz = 1;
	    sprintf(dim, "%lu", (unsigned long) nx);
	} else {
	    ndims = 2;
	    nz = 1;
	    sprintf(dim, "%lux%lu", (unsigned long) nx, (unsigned long) ny);
	}
    } else {
	ndims = 3;
	sprintf(dim, "%lux%lux%lu",
		(unsigned long) nx, (unsigned long) ny, (unsigned long) nz);
    }
    sprintf(s, "Testing hyperslab fill %-11s variable hyperslab", dim);
    printf("%-70s", s);
    fflush(stdout);

    /* Allocate array */
    dst = HDcalloc(1,nx*ny*nz);
    init_full(dst, nx, ny, nz);

    for (i = 0; i < nx; i += di) {
	for (j = 0; j < ny; j += dj) {
	    for (k = 0; k < nz; k += dk) {
		for (dx = 1; dx <= nx - i; dx += ddx) {
		    for (dy = 1; dy <= ny - j; dy += ddy) {
			for (dz = 1; dz <= nz - k; dz += ddz) {

			    /* Describe the hyperslab */
			    dst_size[0] = nx;
			    dst_size[1] = ny;
			    dst_size[2] = nz;
			    dst_offset[0] = (hssize_t)i;
			    dst_offset[1] = (hssize_t)j;
			    dst_offset[2] = (hssize_t)k;
			    hs_size[0] = dx;
			    hs_size[1] = dy;
			    hs_size[2] = dz;

			    for (fill_value=0;
				 fill_value<256;
				 fill_value+=64) {
				/*
				 * Initialize the full array, then subtract the
				 * original * fill values and add the new ones.
				 */
				ref_value = init_full(dst, nx, ny, nz);
				for (u=(size_t)dst_offset[0];
				     u<dst_offset[0]+dx;
				     u++) {
				    for (v = (size_t)dst_offset[1];
					 v < dst_offset[1] + dy;
					 v++) {
					for (w = (size_t)dst_offset[2];
					     w < dst_offset[2] + dz;
					     w++) {
					    ref_value -= dst[u*ny*nz+v*nz+w];
					}
				    }
				}
				ref_value += fill_value * dx * dy * dz;

				/* Fill the hyperslab with some value */
				H5V_hyper_fill(ndims, hs_size, dst_size,
					       dst_offset, dst, fill_value);

				/*
				 * Sum the array and compare it to the
				 * reference value.
				 */
				acc = 0;
				for (u = 0; u < nx; u++) {
				    for (v = 0; v < ny; v++) {
					for (w = 0; w < nz; w++) {
					    acc += dst[u*ny*nz + v*nz + w];
					}
				    }
				}

				if (acc != ref_value) {
				    puts("*FAILED*");
				    if (!isatty(1)) {
					/*
					 * Print debugging info unless output
					 * is going directly to a terminal.
					 */
					AT();
					printf("   acc != ref_value\n");
					printf("   i=%lu, j=%lu, k=%lu, "
					       "dx=%lu, dy=%lu, dz=%lu, "
					       "fill=%d\n",
					       (unsigned long)i,
					       (unsigned long)j,
					       (unsigned long)k,
					       (unsigned long)dx,
					       (unsigned long)dy,
					       (unsigned long)dz,
					       fill_value);
					print_ref(nx, ny, nz);
					printf("\n   Result is:\n");
					print_array(dst, nx, ny, nz);
				    }
				    goto error;
				}
			    }
			}
		    }
		}
	    }
	}
    }
    puts(" PASSED");
    HDfree(dst);
    return SUCCEED;

  error:
    HDfree(dst);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	test_copy
 *
 * Purpose:	Tests H5V_hyper_copy().
 *
 *		The NX, NY, and NZ arguments are the size for the source and
 *		destination arrays.  You may pass zero for NZ or for NY and
 *		NZ to test the 2-d and 1-d cases respectively.
 *
 *		A hyperslab is copied from/to (depending on MODE) various
 *		places in SRC and DST beginning at 0,0,0 and increasing
 *		location by DI,DJ,DK in the x, y, and z directions.
 *
 *		For each hyperslab location, various sizes of hyperslabs are
 *		tried beginning with 1x1x1 and increasing the size in each
 *		dimension by DDX,DDY,DDZ.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_copy(int mode,
	  size_t nx, size_t ny, size_t nz,
	  size_t di, size_t dj, size_t dk,
	  size_t ddx, size_t ddy, size_t ddz)
{
    uint8_t	*src = NULL;		/*source array			*/
    uint8_t	*dst = NULL;		/*destination array		*/
    hsize_t	hs_size[3];		/*hyperslab size		*/
    hsize_t	dst_size[3];		/*destination total size	*/
    hsize_t	src_size[3];		/*source total size		*/
    hssize_t	dst_offset[3];		/*offset of hyperslab in dest	*/
    hssize_t	src_offset[3];		/*offset of hyperslab in source */
    unsigned	ref_value;		/*reference value		*/
    unsigned	acc;			/*accumulator			*/
    hsize_t	i, j, k, dx, dy, dz;	/*counters		     	*/
    hsize_t	u, v, w;
    unsigned		ndims;			/*hyperslab dimensionality	*/
    char	dim[64], s[256];	/*temp string			*/
    const char	*sub;

    /*
     * Dimensionality.
     */
    if (0 == nz) {
	if (0 == ny) {
	    ndims = 1;
	    ny = nz = 1;
	    sprintf(dim, "%lu", (unsigned long) nx);
	} else {
	    ndims = 2;
	    nz = 1;
	    sprintf(dim, "%lux%lu", (unsigned long) nx, (unsigned long) ny);
	}
    } else {
	ndims = 3;
	sprintf(dim, "%lux%lux%lu",
		(unsigned long) nx, (unsigned long) ny, (unsigned long) nz);
    }

    switch (mode) {
    case VARIABLE_SRC:
	/*
	 * The hyperslab "travels" through the source array but the
	 * destination hyperslab is always at the origin of the destination
	 * array.
	 */
	sub = "variable source";
	break;
    case VARIABLE_DST:
	/*
	 * We always read a hyperslab from the origin of the source and copy it
	 * to a hyperslab at various locations in the destination.
	 */
	sub = "variable destination";
	break;
    case VARIABLE_BOTH:
	/*
	 * We read the hyperslab from various locations in the source and copy
	 * it to the same location in the destination.
	 */
	sub = "sync source & dest  ";
	break;
    default:
	abort();
    }

    sprintf(s, "Testing hyperslab copy %-11s %s", dim, sub);
    printf("%-70s", s);
    fflush(stdout);

    /*
     * Allocate arrays
     */
    src = HDcalloc(1,nx*ny*nz);
    dst = HDcalloc(1,nx*ny*nz);
    init_full(src, nx, ny, nz);

    for (i=0; i<nx; i+=di) {
	for (j=0; j<ny; j+=dj) {
	    for (k=0; k<nz; k+=dk) {
		for (dx=1; dx<=nx-i; dx+=ddx) {
		    for (dy=1; dy<=ny-j; dy+=ddy) {
			for (dz=1; dz<=nz-k; dz+=ddz) {

			    /*
			     * Describe the source and destination hyperslabs
			     * and the arrays to which they belong.
			     */
			    hs_size[0] = dx;
			    hs_size[1] = dy;
			    hs_size[2] = dz;
			    dst_size[0] = src_size[0] = nx;
			    dst_size[1] = src_size[1] = ny;
			    dst_size[2] = src_size[2] = nz;
			    switch (mode) {
			    case VARIABLE_SRC:
				dst_offset[0] = 0;
				dst_offset[1] = 0;
				dst_offset[2] = 0;
				src_offset[0] = (hssize_t)i;
				src_offset[1] = (hssize_t)j;
				src_offset[2] = (hssize_t)k;
				break;
			    case VARIABLE_DST:
				dst_offset[0] = (hssize_t)i;
				dst_offset[1] = (hssize_t)j;
				dst_offset[2] = (hssize_t)k;
				src_offset[0] = 0;
				src_offset[1] = 0;
				src_offset[2] = 0;
				break;
			    case VARIABLE_BOTH:
				dst_offset[0] = (hssize_t)i;
				dst_offset[1] = (hssize_t)j;
				dst_offset[2] = (hssize_t)k;
				src_offset[0] = (hssize_t)i;
				src_offset[1] = (hssize_t)j;
				src_offset[2] = (hssize_t)k;
				break;
			    default:
				abort();
			    }

			    /*
			     * Sum the main array directly to get a reference
			     * value to compare against later.
			     */
			    ref_value = 0;
			    for (u=src_offset[0]; u<src_offset[0]+dx; u++) {
				for (v=src_offset[1];
				     v<src_offset[1]+dy;
				     v++) {
				    for (w=src_offset[2];
					 w<src_offset[2]+dz;
					 w++) {
					ref_value += src[u*ny*nz + v*nz + w];
				    }
				}
			    }

			    /*
			     * Set all loc values to 1 so we can detect writing
			     * outside the hyperslab.
			     */
			    for (u=0; u<nx; u++) {
				for (v=0; v<ny; v++) {
				    for (w=0; w<nz; w++) {
					dst[u*ny*nz + v*nz + w] = 1;
				    }
				}
			    }

			    /*
			     * Copy a hyperslab from the global array to the
			     * local array.
			     */
			    H5V_hyper_copy(ndims, hs_size,
					   dst_size, dst_offset, dst,
					   src_size, src_offset, src);

			    /*
			     * Sum the destination hyperslab.  It should be
			     * the same as the reference value.
			     */
			    acc = 0;
			    for (u=dst_offset[0]; u<dst_offset[0]+dx; u++) {
				for (v=dst_offset[1];
				     v<dst_offset[1]+dy;
				     v++) {
				    for (w=dst_offset[2];
					 w<dst_offset[2]+dz;
					 w++) {
					acc += dst[u*ny*nz + v*nz + w];
				    }
				}
			    }
			    if (acc != ref_value) {
				puts("*FAILED*");
				if (!isatty(1)) {
				    /*
				     * Print debugging info unless output is
				     * going directly to a terminal.
				     */
				    AT();
				    printf("   acc != ref_value\n");
				    printf("   i=%lu, j=%lu, k=%lu, "
					   "dx=%lu, dy=%lu, dz=%lu\n",
					   (unsigned long)i,
					   (unsigned long)j,
					   (unsigned long)k,
					   (unsigned long)dx,
					   (unsigned long)dy,
					   (unsigned long)dz);
				    print_ref(nx, ny, nz);
				    printf("\n	 Destination array is:\n");
				    print_array(dst, nx, ny, nz);
				}
				goto error;
			    }
			    /*
			     * Sum the entire array. It should be a fixed
			     * amount larger than the reference value since
			     * we added the border of 1's to the hyperslab.
			     */
			    acc = 0;
			    for (u=0; u<nx; u++) {
				for (v=0; v<ny; v++) {
				    for (w=0; w<nz; w++) {
					acc += dst[u*ny*nz + v*nz + w];
				    }
				}
			    }

			    /*
			     * The following casts are to work around an
			     * optimization bug in the Mongoose 7.20 Irix64
			     * compiler.
			     */
			    if (acc+(unsigned)dx*(unsigned)dy*(unsigned)dz !=
				ref_value + nx*ny*nz) {
				puts("*FAILED*");
				if (!isatty(1)) {
				    /*
				     * Print debugging info unless output is
				     * going directly to a terminal.
				     */
				    AT();
				    printf("   acc != ref_value + nx*ny*nz - "
					   "dx*dy*dz\n");
				    printf("   i=%lu, j=%lu, k=%lu, "
					   "dx=%lu, dy=%lu, dz=%lu\n",
					   (unsigned long)i,
					   (unsigned long)j,
					   (unsigned long)k,
					   (unsigned long)dx,
					   (unsigned long)dy,
					   (unsigned long)dz);
				    print_ref(nx, ny, nz);
				    printf("\n	 Destination array is:\n");
				    print_array(dst, nx, ny, nz);
				}
				goto error;
			    }
			}
		    }
		}
	    }
	}
    }
    puts(" PASSED");
    HDfree(src);
    HDfree(dst);
    return SUCCEED;

  error:
    HDfree(src);
    HDfree(dst);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	test_multifill
 *
 * Purpose:	Tests the H5V_stride_copy() function by using it to fill a
 *		hyperslab by replicating a multi-byte sequence.	 This might
 *		be useful to initialize an array of structs with a default
 *		struct value, or to initialize an array of floating-point
 *		values with a default bit-pattern.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Saturday, October 11, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_multifill(size_t nx)
{
    hsize_t		    i, j;
    hsize_t		    size;
    hssize_t		    src_stride;
    hssize_t		    dst_stride;
    char		    s[64];
	
    struct a_struct {
	int			left;
	double			mid;
	int			right;
    } fill		   , *src = NULL, *dst = NULL;

    printf("%-70s", "Testing multi-byte fill value");
    fflush(stdout);

    /* Initialize the source and destination */
    src = HDmalloc(nx * sizeof(*src));
    dst = HDmalloc(nx * sizeof(*dst));
    for (i = 0; i < nx; i++) {
	src[i].left = 1111111;
	src[i].mid = 12345.6789;
	src[i].right = 2222222;
	dst[i].left = 3333333;
	dst[i].mid = 98765.4321;
	dst[i].right = 4444444;
    }

    /*
     * Describe the fill value.	 The zero stride says to read the same thing
     * over and over again.
     */
    fill.left = 55555555;
    fill.mid = 3.1415927;
    fill.right = 66666666;
    src_stride = 0;

    /*
     * The destination stride says to fill in one value per array element
     */
    dst_stride = sizeof(fill);

    /*
     * Copy the fill value into each element
     */
    size = nx;
    H5V_stride_copy(1, (hsize_t)sizeof(double), &size,
		    &dst_stride, &(dst[0].mid), &src_stride, &(fill.mid));

    /*
     * Check
     */
    s[0] = '\0';
    for (i = 0; i < nx; i++) {
	if (dst[i].left != 3333333) {
	    sprintf(s, "bad dst[%lu].left", (unsigned long)i);
	} else if (dst[i].mid != fill.mid) {
	    sprintf(s, "bad dst[%lu].mid", (unsigned long)i);
	} else if (dst[i].right != 4444444) {
	    sprintf(s, "bad dst[%lu].right", (unsigned long)i);
	}
	if (s[0]) {
	    puts("*FAILED*");
	    if (!isatty(1)) {
		AT();
		printf("   fill={%d,%g,%d}\n   ",
		       fill.left, fill.mid, fill.right);
		for (j = 0; j < sizeof(fill); j++) {
		    printf(" %02x", ((uint8_t *) &fill)[j]);
		}
		printf("\n   dst[%lu]={%d,%g,%d}\n   ",
		       (unsigned long)i,
		       dst[i].left, dst[i].mid, dst[i].right);
		for (j = 0; j < sizeof(dst[i]); j++) {
		    printf(" %02x", ((uint8_t *) (dst + i))[j]);
		}
		printf("\n");
	    }
	    goto error;
	}
    }
	
    puts(" PASSED");
    HDfree(src);
    HDfree(dst);
    return SUCCEED;

  error:
    HDfree(src);
    HDfree(dst);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	test_endian
 *
 * Purpose:	Tests the H5V_stride_copy() function by using it to copy an
 *		array of integers and swap the byte ordering from little
 *		endian to big endian or vice versa depending on the hardware.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Saturday, October 11, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_endian(size_t nx)
{
    uint8_t	*src = NULL;		/*source array			*/
    uint8_t	*dst = NULL;		/*destination array		*/
    hssize_t	src_stride[2];		/*source strides		*/
    hssize_t	dst_stride[2];		/*destination strides		*/
    hsize_t	size[2];		/*size vector			*/
    hsize_t	i, j;

    printf("%-70s", "Testing endian conversion by stride");
    fflush(stdout);

    /* Initialize arrays */
    src = HDmalloc(nx * 4);
    init_full(src, nx, 4, 1);
    dst = HDcalloc(nx , 4);

    /* Initialize strides */
    src_stride[0] = 0;
    src_stride[1] = 1;
    dst_stride[0] = 8;
    dst_stride[1] = -1;
    size[0] = nx;
    size[1] = 4;

    /* Copy the array */
    H5V_stride_copy(2, (hsize_t)1, size, dst_stride, dst + 3, src_stride, src);

    /* Compare */
    for (i = 0; i < nx; i++) {
	for (j = 0; j < 4; j++) {
	    if (src[i * 4 + j] != dst[i * 4 + 3 - j]) {
		puts("*FAILED*");
		if (!isatty(1)) {
		    /*
		     * Print debugging info unless output is going directly
		     * to a terminal.
		     */
		    AT();
		    printf("   i=%lu, j=%lu\n",
			   (unsigned long)i, (unsigned long)j);
		    printf("   Source array is:\n");
		    print_array(src, nx, 4, 1);
		    printf("\n	 Result is:\n");
		    print_array(dst, nx, 4, 1);
		}
		goto error;
	    }
	}
    }

    puts(" PASSED");
    HDfree(src);
    HDfree(dst);
    return SUCCEED;

  error:
    HDfree(src);
    HDfree(dst);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	test_transpose
 *
 * Purpose:	Copy a 2d array from here to there and transpose the elements
 *		as it's copied.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Saturday, October 11, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_transpose(size_t nx, size_t ny)
{
    int	*src = NULL;
    int	*dst = NULL;
    hsize_t	i, j;
    hssize_t	src_stride[2], dst_stride[2];
    hsize_t	size[2];
    char	s[256];

    sprintf(s, "Testing 2d transpose by stride %4lux%-lud",
	    (unsigned long) nx, (unsigned long) ny);
    printf("%-70s", s);
    fflush(stdout);

    /* Initialize */
    src = HDmalloc(nx * ny * sizeof(*src));
    for (i = 0; i < nx; i++) {
	for (j = 0; j < ny; j++) {
	    src[i * ny + j] = (int)(i * ny + j);
	}
    }
    dst = HDcalloc(nx*ny,sizeof(*dst));

    /* Build stride info */
    size[0] = nx;
    size[1] = ny;
    src_stride[0] = 0;
    src_stride[1] = sizeof(*src);
    dst_stride[0] = (ssize_t)((1 - nx * ny) * sizeof(*src));
    dst_stride[1] = (ssize_t)(nx * sizeof(*src));

    /* Copy and transpose */
    if (nx == ny) {
	H5V_stride_copy(2, (hsize_t)sizeof(*src), size,
			dst_stride, dst,
			src_stride, src);
    } else {
	H5V_stride_copy(2, (hsize_t)sizeof(*src), size,
			dst_stride, dst,
			src_stride, src);
    }

    /* Check */
    for (i = 0; i < nx; i++) {
	for (j = 0; j < ny; j++) {
	    if (src[i * ny + j] != dst[j * nx + i]) {
		puts("*FAILED*");
		if (!isatty(1)) {
		    AT();
		    printf("   diff at i=%lu, j=%lu\n",
			   (unsigned long)i, (unsigned long)j);
		    printf("   Source is:\n");
		    for (i = 0; i < nx; i++) {
			printf("%3lu:", (unsigned long)i);
			for (j = 0; j < ny; j++) {
			    printf(" %6d", src[i * ny + j]);
			}
			printf("\n");
		    }
		    printf("\n	 Destination is:\n");
		    for (i = 0; i < ny; i++) {
			printf("%3lu:", (unsigned long)i);
			for (j = 0; j < nx; j++) {
			    printf(" %6d", dst[i * nx + j]);
			}
			printf("\n");
		    }
		}
		goto error;
	    }
	}
    }

    puts(" PASSED");
    HDfree(src);
    HDfree(dst);
    return SUCCEED;

  error:
    HDfree(src);
    HDfree(dst);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	test_sub_super
 *
 * Purpose:	Tests H5V_stride_copy() to reduce the resolution of an image
 *		by copying half the pixels in the X and Y directions.  Then
 *		we use the small image and duplicate every pixel to result in
 *		a 2x2 square.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, October 13, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_sub_super(size_t nx, size_t ny)
{
    uint8_t	*full = NULL;	/*original image		*/
    uint8_t	*half = NULL;	/*image at 1/2 resolution	*/
    uint8_t	*twice = NULL;	/*2x2 pixels			*/
    hssize_t	src_stride[4];	/*source stride info		*/
    hssize_t	dst_stride[4];	/*destination stride info	*/
    hsize_t	size[4];	/*number of sample points	*/
    hsize_t	i, j;
    char	s[256];

    sprintf(s, "Testing image sampling %4lux%-4lu to %4lux%-4lu ",
	    (unsigned long) (2 * nx), (unsigned long) (2 * ny),
	    (unsigned long) nx, (unsigned long) ny);
    printf("%-70s", s);
    fflush(stdout);

    /* Initialize */
    full = HDmalloc(4 * nx * ny);
    init_full(full, 2 * nx, 2 * ny, 1);
    half = HDcalloc(1,nx*ny);
    twice = HDcalloc(4,nx*ny);

    /* Setup */
    size[0] = nx;
    size[1] = ny;
    src_stride[0] = (ssize_t)(2 * ny);
    src_stride[1] = 2;
    dst_stride[0] = 0;
    dst_stride[1] = 1;

    /* Copy */
    H5V_stride_copy(2, (hsize_t)sizeof(uint8_t), size,
		    dst_stride, half, src_stride, full);

    /* Check */
    for (i = 0; i < nx; i++) {
	for (j = 0; j < ny; j++) {
	    if (full[4 * i * ny + 2 * j] != half[i * ny + j]) {
		puts("*FAILED*");
		if (!isatty(1)) {
		    AT();
		    printf("   full[%lu][%lu] != half[%lu][%lu]\n",
			   (unsigned long)i*2,
			   (unsigned long)j*2,
			   (unsigned long)i,
			   (unsigned long)j);
		    printf("   full is:\n");
		    print_array(full, 2 * nx, 2 * ny, 1);
		    printf("\n	 half is:\n");
		    print_array(half, nx, ny, 1);
		}
		goto error;
	    }
	}
    }
    puts(" PASSED");

    /*
     * Test replicating pixels to produce an image twice as large in each
     * dimension.
     */
    sprintf(s, "Testing image sampling %4lux%-4lu to %4lux%-4lu ",
	    (unsigned long) nx, (unsigned long) ny,
	    (unsigned long) (2 * nx), (unsigned long) (2 * ny));
    printf("%-70s", s);
    fflush(stdout);

    /* Setup stride */
    size[0] = nx;
    size[1] = ny;
    size[2] = 2;
    size[3] = 2;
    src_stride[0] = 0;
    src_stride[1] = 1;
    src_stride[2] = 0;
    src_stride[3] = 0;
    dst_stride[0] = (ssize_t)(2 * ny);
    dst_stride[1] = (ssize_t)(2 * sizeof(uint8_t) - 4 * ny);
    dst_stride[2] = (ssize_t)(2 * ny - 2 * sizeof(uint8_t));
    dst_stride[3] = sizeof(uint8_t);

    /* Copy */
    H5V_stride_copy(4, (hsize_t)sizeof(uint8_t), size,
		    dst_stride, twice, src_stride, half);

    /* Check */
    s[0] = '\0';
    for (i = 0; i < nx; i++) {
	for (j = 0; j < ny; j++) {
	    if (half[i*ny+j] != twice[4*i*ny + 2*j]) {
		sprintf(s, "half[%lu][%lu] != twice[%lu][%lu]",
			(unsigned long)i,
			(unsigned long)j,
			(unsigned long)i*2,
			(unsigned long)j*2);
	    } else if (half[i*ny + j] != twice[4*i*ny + 2*j + 1]) {
		sprintf(s, "half[%lu][%lu] != twice[%lu][%lu]",
			(unsigned long)i,
			(unsigned long)j,
			(unsigned long)i*2,
			(unsigned long)j*2+1);
	    } else if (half[i*ny + j] != twice[(2*i +1)*2*ny + 2*j]) {
		sprintf(s, "half[%lu][%lu] != twice[%lu][%lu]",
			(unsigned long)i,
			(unsigned long)j,
			(unsigned long)i*2+1,
			(unsigned long)j*2);
	    } else if (half[i*ny + j] != twice[(2*i+1)*2*ny + 2*j+1]) {
		sprintf(s, "half[%lu][%lu] != twice[%lu][%lu]",
			(unsigned long)i,
			(unsigned long)j,
			(unsigned long)i*2+1,
			(unsigned long)j*2+1);
	    }
	    if (s[0]) {
		puts("*FAILED*");
		if (!isatty(1)) {
		    AT();
		    printf("   %s\n   Half is:\n", s);
		    print_array(half, nx, ny, 1);
		    printf("\n	 Twice is:\n");
		    print_array(twice, 2 * nx, 2 * ny, 1);
		}
		goto error;
	    }
	}
    }
    puts(" PASSED");

    HDfree(full);
    HDfree(half);
    HDfree(twice);
    return SUCCEED;

  error:
    HDfree(full);
    HDfree(half);
    HDfree(twice);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	test_array_fill
 *
 * Purpose:	Tests H5V_array_fill routine by copying a multibyte value
 *              (an array of ints, in our case) into all the elements of an
 *              array.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *		Monday, April 21, 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_array_fill(size_t lo, size_t hi)
{
    int         *dst;           /* Destination                  */
    int         src[ARRAY_FILL_SIZE]; /* Source to duplicate    */
    size_t	u, v, w;        /* Local index variables        */
    char	s[256];

    sprintf(s, "array filling %4lu-%-4lu elements", (unsigned long)lo,(unsigned long)hi);
    TESTING(s);

    /* Initialize */
    dst = HDcalloc(sizeof(int),ARRAY_FILL_SIZE * hi);

    /* Setup */
    for(u=0; u<ARRAY_FILL_SIZE; u++)
        src[u]=(char)u;

    /* Fill */
    for(w=lo; w<=hi; w++) {
        H5V_array_fill(dst,src,sizeof(src),w);

        /* Check */
        for(u=0; u<w; u++)
            for(v=0; v<ARRAY_FILL_SIZE; v++)
                if(dst[(u*ARRAY_FILL_SIZE)+v]!=src[v]) TEST_ERROR;

        HDmemset(dst,0,sizeof(int)*ARRAY_FILL_SIZE*w);
    } /* end for */
    PASSED();

    HDfree(dst);
    return SUCCEED;

  error:
    HDfree(dst);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	test_array_offset_n_calc
 *
 * Purpose:	Tests H5V_array_offset and H5V_array_calc routines by comparing
 *              computed array offsets against calculated ones and then going
 *              back to the coordinates from the offset and checking those.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *		Monday, April 21, 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_array_offset_n_calc(size_t n, size_t x, size_t y, size_t z)
{
    hsize_t     *a, *temp_a;    /* Array for stored calculated offsets */
    hsize_t     off;            /* Offset in array */
    size_t	u, v, w;        /* Local index variables        */
    hsize_t     dims[ARRAY_OFFSET_NDIMS];        /* X, Y & X coordinates of array to check */
    hssize_t    coords[ARRAY_OFFSET_NDIMS];      /* X, Y & X coordinates to check offset of */
    hssize_t    new_coords[ARRAY_OFFSET_NDIMS];  /* X, Y & X coordinates of offset */
    char	s[256];

    sprintf(s, "array offset %4lux%4lux%4lu elements", (unsigned long)z,(unsigned long)y,(unsigned long)x);
    TESTING(s);

    /* Initialize */
    a = HDmalloc(sizeof(hsize_t) * x * y *z);
    dims[0]=z;
    dims[1]=y;
    dims[2]=x;

    /* Setup */
    for(u=0, temp_a=a, off=0; u<z; u++)
        for(v=0; v<y; v++)
            for(w=0; w<x; w++)
                *temp_a++ = off++;

    /* Check offsets */
    for(u=0; u<n; u++) {
        /* Get random coordinate */
        coords[0] = (hssize_t)(HDrandom() % z);
        coords[1] = (hssize_t)(HDrandom() % y);
        coords[2] = (hssize_t)(HDrandom() % x);

        /* Get offset of coordinate */
        off=H5V_array_offset(ARRAY_OFFSET_NDIMS,dims,coords);

        /* Check offset of coordinate */
        if(a[off]!=off) TEST_ERROR;

        /* Get coordinates of offset */
        if(H5V_array_calc(off,ARRAY_OFFSET_NDIMS,dims,new_coords)<0) TEST_ERROR;

        /* Check computed coordinates */
        for(v=0; v<ARRAY_OFFSET_NDIMS; v++)
            if(coords[v]!=new_coords[v]) {
                HDfprintf(stderr,"coords[%u]=%Hu, new_coords[%u]=%Hu\n",(unsigned)v,coords[v],(unsigned)v,new_coords[v]);
                TEST_ERROR;
            }
    } /* end for */

    PASSED();

    HDfree(a);
    return SUCCEED;

  error:
    HDfree(a);
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	Test various hyperslab operations.  Give the words
 *		`small' and/or `medium' on the command line or only `small'
 *		is assumed.
 *
 * Return:	Success:	exit(0)
 *
 *		Failure:	exit(non-zero)
 *
 * Programmer:	Robb Matzke
 *		Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    herr_t		    status;
    int			    nerrors = 0;
    unsigned		    size_of_test;

    /* Parse arguments or assume `small' & `medium' */
    if (1 == argc) {
	size_of_test = TEST_SMALL | TEST_MEDIUM;
    } else {
	int			i;
	for (i = 1, size_of_test = 0; i < argc; i++) {
	    if (!strcmp(argv[i], "small")) {
		size_of_test |= TEST_SMALL;
	    } else if (!strcmp(argv[i], "medium")) {
		size_of_test |= TEST_MEDIUM;
	    } else {
		printf("unrecognized argument: %s\n", argv[i]);
		exit(1);
	    }
	}
    }
    printf("Test sizes: ");
    if (size_of_test & TEST_SMALL)
	printf(" SMALL");
    if (size_of_test & TEST_MEDIUM)
	printf(" MEDIUM");
    printf("\n");

    /* Set the random # seed */
    HDsrandom((unsigned long)HDtime(NULL));

    /*
     * Open the library explicitly for thread-safe builds, so per-thread
     * things are initialized correctly.
     */
#ifdef H5_HAVE_THREADSAFE
    H5open();
#endif  /* H5_HAVE_THREADSAFE */

    /*
     *------------------------------ 
     * TEST HYPERSLAB FILL OPERATION
     *------------------------------ 
     */
    if (size_of_test & TEST_SMALL) {
	status = test_fill(11, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_fill(11, 10, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_fill(3, 5, 5, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_fill(113, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_fill(15, 11, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_fill(5, 7, 7, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
   /*------------------------------
    * TEST HYPERSLAB COPY OPERATION
    *------------------------------ 
    */

    /* exhaustive, one-dimensional test */
    if (size_of_test & TEST_SMALL) {
	status = test_copy(VARIABLE_SRC, 11, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_DST, 11, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_BOTH, 11, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_copy(VARIABLE_SRC, 179, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_DST, 179, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_BOTH, 179, 0, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
    /* exhaustive, two-dimensional test */
    if (size_of_test & TEST_SMALL) {
	status = test_copy(VARIABLE_SRC, 11, 10, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_DST, 11, 10, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_BOTH, 11, 10, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_copy(VARIABLE_SRC, 13, 19, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_DST, 13, 19, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_BOTH, 13, 19, 0, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
    /* sparse, two-dimensional test */
    if (size_of_test & TEST_MEDIUM) {
	status = test_copy(VARIABLE_SRC, 73, 67, 0, 7, 11, 1, 13, 11, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_DST, 73, 67, 0, 7, 11, 1, 13, 11, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_BOTH, 73, 67, 0, 7, 11, 1, 13, 11, 1);
	nerrors += status < 0 ? 1 : 0;
    }
    /* exhaustive, three-dimensional test */
    if (size_of_test & TEST_SMALL) {
	status = test_copy(VARIABLE_SRC, 3, 5, 5, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_DST, 3, 5, 5, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_BOTH, 3, 5, 5, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_copy(VARIABLE_SRC, 7, 9, 5, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_DST, 7, 9, 5, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
	status = test_copy(VARIABLE_BOTH, 7, 9, 5, 1, 1, 1, 1, 1, 1);
	nerrors += status < 0 ? 1 : 0;
    }
   /*---------------------
    * TEST MULTI-BYTE FILL
    *--------------------- 
    */

    if (size_of_test & TEST_SMALL) {
	status = test_multifill(10);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_multifill(500000);
	nerrors += status < 0 ? 1 : 0;
    }
   /*---------------------------
    * TEST TRANSLATION OPERATORS
    *---------------------------
    */

    if (size_of_test & TEST_SMALL) {
	status = test_endian(10);
	nerrors += status < 0 ? 1 : 0;
	status = test_transpose(9, 9);
	nerrors += status < 0 ? 1 : 0;
	status = test_transpose(3, 11);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_endian(800000);
	nerrors += status < 0 ? 1 : 0;
	status = test_transpose(1200, 1200);
	nerrors += status < 0 ? 1 : 0;
	status = test_transpose(800, 1800);
	nerrors += status < 0 ? 1 : 0;
    }
   /*-------------------------
    * TEST SAMPLING OPERATIONS
    *------------------------- 
    */

    if (size_of_test & TEST_SMALL) {
	status = test_sub_super(5, 10);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_sub_super(480, 640);
	nerrors += status < 0 ? 1 : 0;
    }
   /*-------------------------
    * TEST ARRAY FILL OPERATIONS
    *------------------------- 
    */

    if (size_of_test & TEST_SMALL) {
	status = test_array_fill(1,9);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_array_fill(9,257);
	nerrors += status < 0 ? 1 : 0;
    }
   /*-------------------------
    * TEST ARRAY OFFSET OPERATIONS
    *------------------------- 
    */

    if (size_of_test & TEST_SMALL) {
	status = test_array_offset_n_calc(20,7,11,13);
	nerrors += status < 0 ? 1 : 0;
    }
    if (size_of_test & TEST_MEDIUM) {
	status = test_array_offset_n_calc(500,71,193,347);
	nerrors += status < 0 ? 1 : 0;
    }

    /*--- END OF TESTS ---*/

    if (nerrors) {
	printf("***** %d HYPERSLAB TEST%s FAILED! *****\n",
	       nerrors, 1 == nerrors ? "" : "S");
	if (isatty(1)) {
	    printf("(Redirect output to a pager or a file to see "
		   "debug output)\n");
	}
	exit(1);
    }
    printf("All hyperslab tests passed.\n");

#ifdef H5_HAVE_THREADSAFE
    H5close();
#endif  /* H5_HAVE_THREADSAFE */
    return 0;
}
