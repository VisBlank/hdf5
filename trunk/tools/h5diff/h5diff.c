

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <assert.h>


#include "hdf5.h"
#include "h5trav.h"


/*

dset1 dset2 h5diff_test1.h5 h5diff_test2.h5
dset1 dset2 -l h5diff_test1.h5 h5diff_test2.h5
h5diff_test1.h5 h5diff_test2.h5
-r h5diff_test1.h5 h5diff_test2.h5
dset1 dset2 -n 2 h5diff_test1.h5 h5diff_test2.h5
dset3 dset4 -d 0.01 h5diff_test1.h5 h5diff_test2.h5
dset5 dset6 -p 0.05 h5diff_test1.h5 h5diff_test2.h5
dset5 dset7 h5diff_test1.h5 h5diff_test2.h5
dset8 dset9 h5diff_test2.h5 h5diff_test2.h5

*/


#define FFORMAT "%-15g %-15g %-15g\n"
#define IFORMAT "%-15d %-15d %-15d\n"
#define SPACES  "          "



#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


typedef struct options_t
{
	int    l_; /* list */
 int    r_; /* report only what objects differ */
 int    d_; /* delta difference */
 double d_delta; /* delta value */
 int    p_; /* relative error */
 double p_relative; /* relative error value */
 int    n_; /* count */
 int    n_number_count; /* value */
} options_t;

int do_test_files();

int diff_dataset( hid_t file1_id, hid_t file2_id, const char *obj1_name, 
                  const char *obj2_name, options_t options );
int array_diff( void *buf1, void *buf2, hsize_t tot_cnt, hid_t type_id, int rank,
                 hsize_t *dims, options_t options );
void print_pos( int curr_pos, int *acc, int *pos, int rank );
void print_class( H5T_class_t tclass, char *sclass );


const char *file1_name = NULL;
const char *file2_name = NULL;
char       *obj1_name  = NULL;
char       *obj2_name  = NULL;


/*-------------------------------------------------------------------------
 * Function: usage
 *
 * Purpose: print a usage message  
 *
 * Return: void
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October 8, 2002
 *
 * Comments:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

void usage( const char *progname )
{
#define USAGE   "\n\
  [OBJ1_NAME]       Name of an HDF5 object\n\
  [OBJ2_NAME]       Name of an HDF5 object\n\
  [-h ]             Print a basic help message (this message)\n\
  [-l ]             List contents of file\n\
  [-r ]             Print only what objects differ\n\
  [-n count]        Print difference up to count number for each variable\n\
  [-d delta]        Print difference when it is greater than limit delta\n\
  [-p relative]     Print differences which are within a relative error value\n\
  FILE1_NAME        File name of the first HDF5 file\n\
  FILE2_NAME        File name of the second HDF5 file\n"

 fprintf(stderr,
  "h5diff [OBJ1_NAME] [OBJ2_NAME] [-h] [-l] [-r] [-d] [-n count] [-d delta] [-p relativet] FILE1_NAME FILE2_NAME\n%s",
  USAGE);
 fprintf(stderr,"\n");
 fprintf(stderr,"Items in [ ] are optional \n");
 exit(EXIT_FAILURE);
}


/*-------------------------------------------------------------------------
 * Function: main
 *
 * Purpose: H5diff 
 *
 * Return: Success: 0, Failure: 1
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October 8, 2002
 *
 * Comments:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

int main(int argc, const char *argv[])
{
 
 const char *progname = argv[0];
 int        dset_only  = FALSE;
 int        have_group = FALSE;
 int        argno, i, j;
 const char *s = NULL;
 
 hid_t      file1_id, file2_id; 
 herr_t     status;
 int        nobjects1, nobjects2;
 info_t     *info1=NULL;
 info_t     *info2=NULL;
 int        obj1_found = 0;
 int        obj2_found = 0;
 options_t  options = {0,0,0,0,0,0,0,0};


 /* string compare */
 char       *pdest;
 int        result;
 int        len;

	void      *edata;
 hid_t     (*func)(void*);



 do_test_files();


/*-------------------------------------------------------------------------
 * parse command line options
 *-------------------------------------------------------------------------
 */
 
 if (argc < 2) {
  usage( progname );
  exit(EXIT_FAILURE);
 }
 
 /* last 2 items are the file names */
 for (argno=1; argno<argc ; argno++) 
 {
  
  /* get the single-letter switches */
  if ( '-'==argv[argno][0] )
  {
   
   for (s=argv[argno]+1; *s; s++) 
   {
    switch (*s) {
    case 'h': 
     usage(progname);
     exit(EXIT_SUCCESS);
    case 'l': 
     options.l_ = 1;
     break;
    case 'r': 
     options.r_ = 1;
     break;
    case 'd': 
     /* if it is not another option */
     if ( '-' !=argv[argno+1][0] )
     {
      options.d_      = 1;
      options.d_delta = atof(argv[argno+1]);
     }
     break;
    case 'p': 
     if ( '-' !=argv[argno+1][0] )
     {
      options.p_         = 1;
      options.p_relative  = atof(argv[argno+1]);
     }
     break;
    case 'n': 
     if ( '-' !=argv[argno+1][0] )
     {
      options.n_             = 1;
      options.n_number_count = atoi(argv[argno+1]);
     }
     break;
    } /*switch*/
   } /*for*/ 
  } /*if*/
  
  else
   
  {
   
   /* 2 last args are the file names, and it is not a -switch parameter */
   if ( argno < argc-2 && '-' !=argv[argno-1][0] )
   {
    if ( obj1_name == NULL )
     obj1_name = argv[argno];

    if ( obj2_name == NULL )
    {
     
     /* check if we have a second object name */
     if ( '-' !=argv[argno+1][0] )
      /* yes */
      obj2_name = argv[argno+1];
     else
      /* no */
      obj2_name = obj1_name;
    }
   }
   
  }
  
 }/*for*/
 

/*-------------------------------------------------------------------------
 * process the files
 *-------------------------------------------------------------------------
 */
 
 file1_name = argv[argc-2];
 file2_name = argv[argc-1];


	/* disable error reporting */
 H5Eget_auto(&func, &edata);
 H5Eset_auto(NULL, NULL);

	
	/* Open the files */
 if ((file1_id=H5Fopen(file1_name,H5F_ACC_RDONLY,H5P_DEFAULT))<0 )
	{
  printf("h5diff: %s: No such file or directory\n", file1_name );
  exit(EXIT_FAILURE);
	}

	if ((file2_id=H5Fopen(file2_name,H5F_ACC_RDONLY,H5P_DEFAULT))<0 )
	{
  printf("h5diff: %s: No such file or directory\n", file2_name );
  exit(EXIT_FAILURE);
	}

	/* enable error reporting */
 H5Eset_auto(func, edata);


/*-------------------------------------------------------------------------
 * get the number of objects in the files
 *-------------------------------------------------------------------------
 */

 nobjects1 = H5get_object_info( file1_id, NULL );
 nobjects2 = H5get_object_info( file2_id, NULL );

/*-------------------------------------------------------------------------
 * get the list of objects in the files
 *-------------------------------------------------------------------------
 */

 info1 = (info_t*) malloc( nobjects1 * sizeof(info_t));
 info2 = (info_t*) malloc( nobjects2 * sizeof(info_t));

 H5get_object_info( file1_id, info1 );
 H5get_object_info( file2_id, info2 );

 if ( options.l_ )
 {
  printf("File 1: # of entries = %d\n", nobjects1);
  for ( i = 0; i < nobjects1; i++)
  {
   switch ( info1[i].type )
   {
   case H5G_GROUP:
    printf("%s \t %s\n", info1[i].name, "group" );
    break;
   case H5G_DATASET:
    printf("%s \t %s\n", info1[i].name, "dataset" );
    break;
   case H5G_TYPE:
    printf("%s \t %s\n", info1[i].name, "datatype" );
    break;
   }
  }
  
  printf("File 2: # of entries = %d\n", nobjects2);
  for ( i = 0; i < nobjects2; i++)
  {
   switch ( info2[i].type )
   {
   case H5G_GROUP:
    printf("%s \t %s\n", info2[i].name, "group" );
    break;
   case H5G_DATASET:
    printf("%s \t %s\n", info2[i].name, "dataset" );
    break;
   case H5G_TYPE:
    printf("%s \t %s\n", info2[i].name, "datatype" );
    break;
   }
  }
  printf("\n");
 }
  



/*-------------------------------------------------------------------------
 * object name was supplied
 *-------------------------------------------------------------------------
 */
 
 /* object name was supplied, find obj1_name */
 if ( obj1_name )
 {
		
  for ( i = 0; i < nobjects1; i++)
  {
			
   pdest = strstr( info1[i].name, obj1_name );
   result = pdest - info1[i].name;
   len = strlen(obj1_name);
   
   /* found at position result */
   if( pdest != NULL && 
    /* check if it is not a substring */
    info1[i].name[result-1] == '/' &&
    /* check if it is the last or in the middle */
    (info1[i].name[result+len]=='/' || 
				info1[i].name[result+len]=='\0') )
   {
    printf( "%s found in file 1 <%s>\n\n", info1[i].name, file1_name);
				
				obj1_found = 1;
				
				/* go to second file and find obj2_name */
    for ( j = 0; j < nobjects2; j++)
    {
     
     pdest = strstr( info2[j].name, obj2_name );
     result = pdest - info2[j].name;
					
     len = strlen(obj2_name);
     
     /* found at position result */
     if( pdest != NULL && 
      /* check if it is not a substring */
      info2[j].name[result-1] == '/' &&
      /* check if it is the last or in the middle */
      (info2[j].name[result+len]=='/' || info2[j].name[result+len]=='\0') )
					{
						
						obj2_found = 1;
      /* objects are the same type */
      if ( info1[i].type == info2[j].type )
						{
							
							switch ( info1[i].type )
							{
								
							/*-------------------------------------------------------------------------
							* H5G_GROUP
							*-------------------------------------------------------------------------
								*/ 
								
							case H5G_GROUP:
								
								printf( "%s found in file 2 <%s>\n", info2[j].name, file2_name ); 
								
								break;
								
								/*-------------------------------------------------------------------------
								* H5G_DATASET
								*-------------------------------------------------------------------------
								*/
								
							case H5G_DATASET:

								printf( "%s found in file 2 <%s>\n", info2[j].name, file2_name ); 
								/* compare with the absolute name */
								diff_dataset(file1_id,file2_id,info1[i].name,info2[j].name,options);
								printf("\n");
								break;
								
								/*-------------------------------------------------------------------------
								* H5G_TYPE
								*-------------------------------------------------------------------------
								*/
								
							case H5G_TYPE:
								
								printf( "%s found in file 2 <%s>\n", info2[j].name, file2_name ); 
								
								break;
								
							} /* switch */
						}
						
     }
     
    } /* j */
				
				if ( obj2_found == 0 )
					printf( "%s was not found in file 2 <%s>\n\n", obj2_name, file2_name);
    
   }
   
  } /* i */
		
		if ( obj1_found == 0 )
			printf( "%s was not found in file 1 <%s>\n\n", obj1_name, file1_name);
		
		
 }
	
	
	
/*-------------------------------------------------------------------------
 * compare all datasets
 *-------------------------------------------------------------------------
 */
	
 else
  
 {
  for ( i = 0; i < nobjects1; i++)
  {
   obj1_name = info1[i].name;
   len = strlen(obj1_name);

			printf( "%s found in file 1 <%s>\n\n", info1[i].name, file1_name);
   
			obj2_found = 0;
   for ( j = 0; j < nobjects2; j++)
   {
    /* find an object in file2 with same name as in file 1 */
				
    pdest = strstr( info2[j].name, obj1_name );
    result = pdest - info2[j].name;
				
				obj2_name = info2[j].name;
				    
    /* found at position result */
    if( pdest != NULL && 
     /* check if it is not a substring */
     info2[j].name[result] == '/' &&
     /* check if it is the last or in the middle */
     (info2[j].name[result+len]=='/' || info2[j].name[result+len]=='\0') &&
     /* objects are the same type */
     info1[i].type == info2[j].type )
    {
     
					obj2_found = 1;
     switch ( info1[i].type )
     {
      
     /*-------------------------------------------------------------------------
     * H5G_GROUP
     *-------------------------------------------------------------------------
      */ 
      
     case H5G_GROUP:
      
						printf( "%s found in file 2 <%s>\n", info2[j].name, file2_name ); 
      
      break;
      
      /*-------------------------------------------------------------------------
      * H5G_DATASET
      *-------------------------------------------------------------------------
      */
      
     case H5G_DATASET:
						
						printf( "%s found in file 2 <%s>\n", info2[j].name, file2_name ); 
      /* compare with the absolute name */
      diff_dataset(file1_id,file2_id,info1[i].name,info2[j].name,options);
      printf("\n");
      break;
      
      /*-------------------------------------------------------------------------
      * H5G_TYPE
      *-------------------------------------------------------------------------
      */
      
     case H5G_TYPE:
						
						printf( "%s found in file 2 <%s>\n", info2[j].name, file2_name ); 
      
      break;
     } /* switch */
    } /* if */
	  } /* j */
				
			if ( obj2_found == 0 )
    printf( "%s is in file 1 <%s>, but not in file 2 <%s>\n\n", obj1_name, file1_name, file2_name);

  } /* i */
  
 }
	
 /* close */
 status = H5Fclose(file1_id);
 status = H5Fclose(file2_id);
	
 if ( info1 )
  free(info1);
 if ( info2 )
  free(info2);
	
 return 0;
	
}






/*-------------------------------------------------------------------------
 * Function: diff_dataset
 *
 * Purpose: 
 *
 * Return: Success: 0, Failure: -11
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October 8, 2002
 *
 * Comments:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */



int diff_dataset( hid_t file1_id, hid_t file2_id, const char *obj1_name,
                  const char *obj2_name, options_t options )
{

 hid_t   dset1_id, dset2_id; 
 hid_t   space1_id, space2_id; 
 hid_t   type1_id, type2_id;
 hid_t   rank1, rank2; 
 void    *buf1, *buf2;
 hsize_t tot_cnt, tot_cnt1, tot_cnt2;
 hsize_t dims1[32], dims2[32];
 int     i, j;
 herr_t  status;
	H5T_class_t tclass1;
	H5T_class_t tclass2;
	char    sclass1[20];
	char    sclass2[20];
	int     nfound;


/*-------------------------------------------------------------------------
 * open the handles
 *-------------------------------------------------------------------------
 */

 /* Open the datasets */
 if ( (dset1_id = H5Dopen(file1_id,obj1_name)) < 0 )
  return -1;

 if ( (dset2_id = H5Dopen(file2_id,obj2_name)) < 0 )
  return -1;

 /* Get the datatype */
 if ( (type1_id = H5Dget_type(dset1_id)) < 0 )
  goto out;

 /* Get the datatype */
 if ( (type2_id = H5Dget_type(dset2_id)) < 0 )
  goto out;

	 /* Get the dataspace handle */
 if ( (space1_id = H5Dget_space(dset1_id)) < 0 )
  return -1;

 /* Get rank */
 if ( (rank1 = H5Sget_simple_extent_ndims(space1_id)) < 0 )
  return -1;

 /* Get the dataspace handle */
 if ( (space2_id = H5Dget_space(dset2_id)) < 0 )
  return -1;

 /* Get rank */
 if ( (rank2 = H5Sget_simple_extent_ndims(space2_id)) < 0 )
  return -1;

 /* Get dimensions */
 if ( H5Sget_simple_extent_dims(space1_id,dims1,NULL) < 0 )
  goto out;

 /* Get dimensions */
 if ( H5Sget_simple_extent_dims(space2_id,dims2,NULL) < 0 )
  goto out;


/*-------------------------------------------------------------------------
 * check for the same class datatype
 *-------------------------------------------------------------------------
 */

	if ((tclass1=H5Tget_class(type1_id))<0) 
		goto out;

	if ((tclass2=H5Tget_class(type2_id))<0) 
		goto out;

	if ( tclass1 != tclass2 )
 {
		print_class( tclass1, sclass1 );
		print_class( tclass2, sclass2 );
		printf( "<%s> is of class %s and <%s> is of class %s\n\n", 
			obj1_name, sclass1, obj2_name, sclass2 );
	 goto out;
 }


/*-------------------------------------------------------------------------
 * check for non supported datatypes
 *-------------------------------------------------------------------------
 */


	switch (tclass1) 
	{
	case H5T_TIME:
		printf( "H5T_TIME comparison is not supported\n");			
		goto out;
	case H5T_STRING:
		printf( "H5T_STRING comparison is not supported\n");			
		goto out;
	case H5T_BITFIELD:
		printf( "H5T_BITFIELD comparison is not supported\n");				
		goto out;
	case H5T_OPAQUE:
		printf( "H5T_OPAQUE comparison is not supported\n");				
		goto out;
	case H5T_COMPOUND:
		printf( "H5T_COMPOUND comparison is not supported\n");						
		goto out;
	case H5T_REFERENCE:
		printf( "H5T_REFERENCE comparison is not supported\n");			
		goto out;
	case H5T_ENUM:
		printf( "H5T_ENUM comparison is not supported\n");	
	goto out;
	case H5T_VLEN:
		printf( "H5T_VLEN comparison is not supported\n");	
		goto out;
	case H5T_ARRAY:
		printf( "H5T_ARRAY comparison is not supported\n");	
		goto out;
	}

	


/*-------------------------------------------------------------------------
 * check for the same rank
 *-------------------------------------------------------------------------
 */
 
 if ( rank1 != rank2 )
 {
		printf( "<%s> is of rank %d and <%s> is of rank %d\n\n", 
			obj1_name, rank1, obj2_name, rank2 );
  goto out;
 }


/*-------------------------------------------------------------------------
 * check for the same dimensionality
 *-------------------------------------------------------------------------
 */

	for (i = 0; i < rank1; i++) 
 {
  if ( dims1[i] != dims2[i] )
		{
   printf( "<%s> has different dimensions than <%s>\n", obj1_name, obj2_name );
			printf( "<%s>: ", obj1_name );
			printf("[ " );  
   for (j = 0; j < rank1; j++) 
				printf("%d ", dims1[j]  );
   printf("]\n" );
			printf( "<%s>: ", obj2_name );
			printf("[ " );  
   for (j = 0; j < rank1; j++) 
				printf("%d ", dims2[j]  );
   printf("]\n" );
   goto out;
		}
 }


 tot_cnt1 = 1;
 for (i = 0; i < rank1; i++) 
 {
  tot_cnt1 *= dims1[i];
 }
 
 tot_cnt2 = 1;
 for (i = 0; i < rank2; i++) 
 {
  tot_cnt2 *= dims2[i];
 }

 buf1 = (void *) malloc((unsigned) (tot_cnt1*H5Tget_size(type1_id)));
 buf2 = (void *) malloc((unsigned) (tot_cnt2*H5Tget_size(type2_id)));

 if ( H5Dread(dset1_id,type1_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf1) < 0 )
  goto out;

 if ( H5Dread(dset2_id,type2_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf2) < 0 )
  goto out;
 
 if (tot_cnt1 > tot_cnt2)
  tot_cnt = tot_cnt2;
 else
  tot_cnt = tot_cnt1; 

 nfound = array_diff(buf1,buf2,tot_cnt,type1_id,rank1,dims1,options);
 printf("%d differences found\n", nfound );


 free((char *) buf1);
 free((char *) buf2);


/*-------------------------------------------------------------------------
 * close
 *-------------------------------------------------------------------------
 */


out:
 
 /* Close */
 status = H5Dclose(dset1_id);
 status = H5Dclose(dset2_id);
 status = H5Sclose(space1_id);
 status = H5Sclose(space2_id);
 status = H5Tclose(type1_id);
 status = H5Tclose(type2_id);
 
 return 0;

}



/*-------------------------------------------------------------------------
 * Function: array_diff
 *
 * Purpose: 
 *
 * Return: Success: 0, Failure: -11
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October 8, 2002
 *
 * Comments:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

 
int array_diff( void *buf1, void *buf2, hsize_t tot_cnt, hid_t type_id, int rank,
                 hsize_t *dims, options_t options )
{
 char   *i1ptr1, *i1ptr2;
 short  *i2ptr1, *i2ptr2;
 int    *i4ptr1, *i4ptr2;
 float  *fptr1, *fptr2;
 double *dptr1, *dptr2;
 int    nfound = 0; /* number of differences found */

 /* accumulator and matrix position */
 int    acc[32];
 int    pos[32];
 int    i;

 H5T_class_t type_class;
 size_t      type_size;

 acc[rank-1]=1;
 for(i=(rank-2); i>=0; i--)
 {
  acc[i]=acc[i+1]*(int)dims[i+1];
 }

 /* Get the class. */
 type_class = H5Tget_class( type_id );

 /* Get the size. */
 type_size = H5Tget_size( type_id );

	printf("%-15s %-15s %-15s %-20s\n", "position", obj1_name, obj2_name, "difference");
	printf("------------------------------------------------------------\n");


 switch(type_class)
 {
  case H5T_INTEGER:


   switch(type_size)
   {

  /*-------------------------------------------------------------------------
   * H5T_INTEGER 1
   *-------------------------------------------------------------------------
   */

   case 1:
    i1ptr1 = (char *) buf1;
    i1ptr2 = (char *) buf2;

    for ( i = 0; i < tot_cnt; i++)
    {
					/* delta but not percentage */
     if ( options.d_ && !options.p_ )
     {
      if ( abs(*i1ptr1 - *i1ptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(IFORMAT, *i1ptr1, *i1ptr2, abs(*i1ptr1 - *i1ptr2));
							}
					  nfound++;
      }
     }
      
     /* percentage but not delta */
     else if ( !options.d_ && options.p_ )
     {
      if ( abs(1 - *i1ptr1 / *i1ptr2)  > options.p_relative  )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(IFORMAT, *i1ptr1, *i1ptr2, abs(*i1ptr1 - *i1ptr2));
							}
					  nfound++;
	     }
     }
   
     /* percentage and delta */
     else if ( options.d_ && options.p_ )
     {
      if ( abs(1 - *i1ptr1 / *i1ptr2)  > options.p_relative &&
           fabs(*i1ptr1 - *i1ptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(IFORMAT, *i1ptr1, *i1ptr2, abs(*i1ptr1 - *i1ptr2));
							}
					  nfound++;
	     }
     }
     
     else
          
     if (*i1ptr1 != *i1ptr2)
     {
						if ( options.n_ && nfound>=options.n_number_count)
       return nfound;
						if ( options.r_==0 ) 
						{
       print_pos( i, acc, pos, rank );
					 	printf(SPACES);
       printf(IFORMAT, *i1ptr1, *i1ptr2, abs(*i1ptr1 - *i1ptr2));
						}
					 nfound++;
		    }                                               
     i1ptr1++;  i1ptr2++;
    }
  
    break;

  /*-------------------------------------------------------------------------
   * H5T_INTEGER 2
   *-------------------------------------------------------------------------
   */

   case 2:
    i2ptr1 = (short *) buf1;
    i2ptr2 = (short *) buf2;

    for ( i = 0; i < tot_cnt; i++)
    {
     /* delta but not percentage */
     if ( options.d_ && !options.p_ )
     {
      if ( abs(*i2ptr1 - *i2ptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
							 printf(SPACES);
        printf(IFORMAT, *i2ptr1, *i2ptr2, abs(*i2ptr1 - *i2ptr2));
							}
					  nfound++;
	     }
     }
      
     /* percentage but not delta */
     else if ( !options.d_ && options.p_ )
     {
      if ( abs(1 - *i2ptr1 / *i2ptr2)  > options.p_relative  )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(IFORMAT, *i2ptr1, *i2ptr2, abs(*i2ptr1 - *i2ptr2));
							}
					  nfound++;
		     }
     }
   
     /* percentage and delta */
     else if ( options.d_ && options.p_ )
     {
      if ( abs(1 - *i2ptr1 / *i2ptr2) > options.p_relative &&
           abs(*i2ptr1 - *i2ptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
					 		printf(SPACES);
        printf(IFORMAT, *i2ptr1, *i2ptr2, abs(*i2ptr1 - *i2ptr2));
							}
					  nfound++;
	     }
     }
     
     else
          
     if (*i2ptr1 != *i2ptr2)
     {
						if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
						if ( options.r_==0 ) 
						{
       print_pos( i, acc, pos, rank );
						 printf(SPACES);
       printf(IFORMAT, *i2ptr1, *i2ptr2, *i2ptr1 - *i2ptr2);
						}
					 nfound++;
		   }                                               
     i2ptr1++;  i2ptr2++;
    }
   
    break;

  /*-------------------------------------------------------------------------
   * H5T_INTEGER 4
   *-------------------------------------------------------------------------
   */

   case 4:
    i4ptr1 = (int *) buf1;
    i4ptr2 = (int *) buf2;
  
    for ( i = 0; i < tot_cnt; i++)
    {
     /* delta but not percentage */
     if ( options.d_ && !options.p_ )
     {
						if ( options.n_ && nfound>=options.n_number_count)
       return nfound;
      if ( abs(*i4ptr1 - *i4ptr2) > options.d_delta )
      {
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(IFORMAT, *i4ptr1, *i4ptr2, abs(*i4ptr1 - *i4ptr2));
							}
							nfound++;
			   }
     }
      
     /* percentage but not delta */
     else if ( !options.d_ && options.p_ )
     {
      if ( abs(1 - *i4ptr1 / *i4ptr2) > options.p_relative  )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(IFORMAT, *i4ptr1, *i4ptr2, abs(*i4ptr1 - *i4ptr2));
							}
							nfound++;
		    }
     }
   
     /* percentage and delta */
     else if ( options.d_ && options.p_ )
     {
      if ( abs(1 - *i4ptr1 / *i4ptr2) > options.p_relative &&
           abs(*i4ptr1 - *i4ptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
					 		printf(SPACES);
        printf(IFORMAT, *i4ptr1, *i4ptr2, abs(*i4ptr1 - *i4ptr2));
							}
							nfound++;
				  }
     }
     
     else
          
     if (*i4ptr1 != *i4ptr2)
     {
						if ( options.n_ && nfound>=options.n_number_count)
       return nfound;
						if ( options.r_==0 ) 
						{
       print_pos( i, acc, pos, rank );
					 	printf(SPACES);
       printf(IFORMAT, *i4ptr1, *i4ptr2, abs(*i4ptr1 - *i4ptr2));
						}
						nfound++;
					 
     }                                               
     i4ptr1++;  i4ptr2++;
    }

    break;
    
       
   } /*switch*/

   
   break; /* H5T_INTEGER */


   case H5T_FLOAT:


/*
position   dset5      dset6     difference
--------  --------   --------   ----------
[ 1 2 ]   1.000000   1.100000   -0.100000
[ 2 1 ]   3.000000   3.020000   -0.020000
[ 2 2 ]   4.000000   4.002000   -0.002000

*/


   switch(type_size)
   {

  /*-------------------------------------------------------------------------
   * H5T_FLOAT 4
   *-------------------------------------------------------------------------
   */
   case 4:
    fptr1 = (float *) buf1;
    fptr2 = (float *) buf2;
  
    for ( i = 0; i < tot_cnt; i++)
    {
     /* delta but not percentage */
     if ( options.d_ && !options.p_ )
     {
      if ( fabs(*fptr1 - *fptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
							 printf(SPACES);
        printf(FFORMAT, *fptr1, *fptr2, fabs(*fptr1 - *fptr2));
							}
							nfound++;
      }
     }
      
     /* percentage but not delta */
     else if ( !options.d_ && options.p_ )
     {
      if ( fabs(1 - *fptr1 / *fptr2)  > options.p_relative  )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
							 printf(SPACES);
        printf(FFORMAT, *fptr1, *fptr2, fabs(*fptr1 - *fptr2));
							}
							nfound++;
	      }
     }
   
     /* percentage and delta */
     else if ( options.d_ && options.p_ )
     {
      if ( fabs(1 - *fptr1 / *fptr2)  > options.p_relative &&
           fabs(*fptr1 - *fptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
					 		printf(SPACES);
        printf(FFORMAT, *fptr1, *fptr2, fabs(*fptr1 - *fptr2));
							}
							nfound++;
		    }
     }
     
     else
          
     if (*fptr1 != *fptr2)
     {
						if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
						if ( options.r_==0 ) 
						{
       print_pos( i, acc, pos, rank );
						 printf(SPACES);
       printf(FFORMAT, *fptr1, *fptr2, fabs(*fptr1 - *fptr2));
						}
						nfound++;
     }                                               
     fptr1++;  fptr2++;
    }
    break;
 
  /*-------------------------------------------------------------------------
   * H5T_FLOAT 8
   *-------------------------------------------------------------------------
   */

   case 8:
    dptr1 = (double *) buf1;
    dptr2 = (double *) buf2;

    for ( i = 0; i < tot_cnt; i++)
    {
     /* delta but not percentage */
     if ( options.d_ && !options.p_ )
     {
      if ( fabs(*dptr1 - *dptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(FFORMAT, *dptr1, *dptr2, fabs(*dptr1 - *dptr2));
							}
							nfound++;
	      }
     }
      
     /* percentage but not delta */
     else if ( !options.d_ && options.p_ )
     {
      if ( 1 - *dptr1 / *dptr2  > options.p_relative  )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(FFORMAT, *dptr1, *dptr2, fabs(*dptr1 - *dptr2));
							}
							nfound++;
      }
     }
   
     /* percentage and delta */
     else if ( options.d_ && options.p_ )
     {
      if ( fabs(1 - *dptr1 / *dptr2)  > options.p_relative &&
           fabs(*dptr1 - *dptr2) > options.d_delta )
      {
							if ( options.n_ && nfound>=options.n_number_count)
        return nfound;
							if ( options.r_==0 ) 
							{
        print_pos( i, acc, pos, rank );
						 	printf(SPACES);
        printf(FFORMAT, *dptr1, *dptr2, fabs(*dptr1 - *dptr2));
							}
							nfound++;
					 }
     }
     
     else
          
     if (*dptr1 != *dptr2)
     {
						if ( options.n_ && nfound>=options.n_number_count)
       return nfound;
						if ( options.r_==0 ) 
						{
       print_pos( i, acc, pos, rank );
				 		printf(SPACES);
       printf(FFORMAT, *dptr1, *dptr2, fabs(*dptr1 - *dptr2));
						}
						nfound++;
					}                                               
     dptr1++;  dptr2++;
    }
   


    break;
      
   } /*switch*/


   
   break; /* H5T_FLOAT 8 */
   
 } /*switch*/
   
 
  return nfound;
}


/*-------------------------------------------------------------------------
 * Function: print_pos
 *
 * Purpose: convert an array index position to matrix notation
 *
 * Return: pos matrix array
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: December 19, 2002
 *
 * Comments:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */



void print_pos( int curr_pos, int *acc, int *pos, int rank )
{
 int i;

 for ( i = 0; i < rank; i++)
  pos[i]=0;

 for ( i = 0; i < rank; i++)
 {
  pos[i] = curr_pos/acc[i];
  curr_pos -= acc[i]*pos[i];
 }
 assert( curr_pos == 0 );

 printf("[ " );  
 for ( i = 0; i < rank; i++)
 {
  printf("%d ", pos[i]  );
 }
 printf("]" );


}



/*-------------------------------------------------------------------------
 * Function: print_class
 *
 * Purpose: 
 *
 * Return: 
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: February 24, 2003
 *
 * Comments:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

void print_class( H5T_class_t tclass, char *sclass )
{
	switch (tclass) 
	{
	case H5T_TIME:
		strcpy(sclass,"H5T_TIME");
		break;
	case H5T_INTEGER:
		strcpy(sclass,"H5T_INTEGER");
		break;
	case H5T_FLOAT:
		strcpy(sclass,"H5T_FLOAT");
		break;
	case H5T_STRING:
		strcpy(sclass,"H5T_STRING");
		break;
	case H5T_BITFIELD:
		strcpy(sclass,"H5T_BITFIELD");
		break;
	case H5T_OPAQUE:
		strcpy(sclass,"H5T_OPAQUE");
		break;
	case H5T_COMPOUND:
		strcpy(sclass,"H5T_COMPOUND");
		break;
	case H5T_REFERENCE:
		strcpy(sclass,"H5T_REFERENCE");
		break;
	case H5T_ENUM:
		strcpy(sclass,"H5T_ENUM");
		break;
	case H5T_VLEN:
		strcpy(sclass,"H5T_VLEN");
		break;
	case H5T_ARRAY:
		strcpy(sclass,"H5T_ARRAY");
		break;
	}
}

/*-------------------------------------------------------------------------
 * do some test files 
 *-------------------------------------------------------------------------
 */

int do_test_files()
{

 hid_t   file1_id, file2_id; 
 hid_t   dataset_id;
 hid_t   space_id;  
 hid_t   group_id, group2_id;
	hid_t   type_id;  
 hsize_t dims  [1] = { 7 };
 hsize_t dims2 [2] = { 3,2 };
	hsize_t dims3 [2] = { 3,3 };
 int     data1[7] = {1,1,1,1,1,1,1};
 int     data2[7] = {1,1,1,4,5,6,7};
 float   data3[7] = {1,1,3,4,5,6,7};
 float   data4[7] = {1,1,3.02f,4.002f,5.00002f,6,7};
 float   data5[3][2] = {1,1,3,4,5,6};
 float   data6[3][2] = {1,1.1f,3.02f,4.002f,5.00002f,6};
	float   data7[3][3] = {1,1,3,4,5,6,7,8,9};
	double  data8[3][2] = {1,1,3.40505e-9,4,5,6};
	double  data9[3][2] = {1,1,3.58911e-9,4,5,6};
	char    data10[] = {"A string"};

 /* attribute */
 size_t  size_attr = 5;
 float   attr_data1[5] = {1,2,3,4,5};
 float   attr_data2[5] = {1,2.1f,3.01f,4.001f,5.00001f};
 herr_t  status;

/*-------------------------------------------------------------------------
 * Create two files
 *-------------------------------------------------------------------------
 */
  
 /* Create a file */
 file1_id = H5Fcreate ("h5diff_test1.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); 

 /* Create a file */
 file2_id = H5Fcreate ("h5diff_test2.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); 

/*-------------------------------------------------------------------------
 * Make dataset "dset1" on file1
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(1,dims,NULL);

 /* Create a dataset "dset1" */
 dataset_id = H5Dcreate(file1_id,"dset1",H5T_NATIVE_INT,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_INT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data1);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset3" on file1
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(1,dims,NULL);

 /* Create a dataset "dset3" */
 dataset_id = H5Dcreate(file1_id,"dset3",H5T_NATIVE_FLOAT,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data3);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make group "g1" on file1
 *-------------------------------------------------------------------------
 */

 /* Create a group. */
 group_id = H5Gcreate(file1_id, "g1", 0);

 /* Close */
 status = H5Gclose(group_id);


/*-------------------------------------------------------------------------
 * Make dataset "dset1" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(1,dims,NULL);

 /* Create a dataset "dset1" */
 dataset_id = H5Dcreate(file2_id,"dset1",H5T_NATIVE_INT,space_id,H5P_DEFAULT);

 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_INT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data2);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);


/*-------------------------------------------------------------------------
 * Make dataset "dset2" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(1,dims,NULL);

 /* Create a dataset "dset1" */
 dataset_id = H5Dcreate(file2_id,"dset2",H5T_NATIVE_INT,space_id,H5P_DEFAULT);

 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_INT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data2);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);


/*-------------------------------------------------------------------------
 * Make dataset "g1/dset1" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(1,dims,NULL);

 /* Create a group. */
 group_id = H5Gcreate(file2_id, "g1", 0);

 /* Create a dataset "g1/dset1" */
 dataset_id = H5Dcreate(group_id,"dset1",H5T_NATIVE_FLOAT,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data2);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Gclose(group_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make group "g2/g1" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a group. */
 group_id = H5Gcreate(file2_id, "g2", 0);
 group2_id = H5Gcreate(group_id, "g1", 0);

 /* Close */
 status = H5Gclose(group_id);
 status = H5Gclose(group2_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset4" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(1,dims,NULL);

 /* Create a dataset "dset" */
 dataset_id = H5Dcreate(file2_id,"dset4",H5T_NATIVE_FLOAT,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data4);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset5" on file1
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(2,dims2,NULL);

 /* Create a dataset "dset" */
 dataset_id = H5Dcreate(file1_id,"dset5",H5T_NATIVE_FLOAT,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data5);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset6" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(2,dims2,NULL);

 /* Create a dataset "dset" */
 dataset_id = H5Dcreate(file2_id,"dset6",H5T_NATIVE_FLOAT,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data6);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset7" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(2,dims3,NULL);

 /* Create a dataset "dset" */
 dataset_id = H5Dcreate(file2_id,"dset7",H5T_NATIVE_FLOAT,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,data7);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset8" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(2,dims2,NULL);

 /* Create a dataset "dset" */
 dataset_id = H5Dcreate(file2_id,"dset8",H5T_NATIVE_DOUBLE,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,data8);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset9" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate_simple(2,dims2,NULL);

 /* Create a dataset "dset" */
 dataset_id = H5Dcreate(file2_id,"dset9",H5T_NATIVE_DOUBLE,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,data9);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);

/*-------------------------------------------------------------------------
 * Make dataset "dset10" on file2
 *-------------------------------------------------------------------------
 */

 /* Create a data space  */
 space_id = H5Screate(H5S_SCALAR);

	/* Make a string type */
	type_id = H5Tcopy(H5T_C_S1);
	status = H5Tset_size (type_id, strlen(data10));

 /* Create a dataset "dset" */
 dataset_id = H5Dcreate(file2_id,"dset10",type_id,space_id,H5P_DEFAULT);
  
 /* Write the data */
 status = H5Dwrite(dataset_id,type_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,data10);

 /* Close */
 status = H5Dclose(dataset_id);
 status = H5Sclose(space_id);
	status = H5Tclose(type_id);



 
/*-------------------------------------------------------------------------
 * Close files
 *-------------------------------------------------------------------------
 */
 status = H5Fclose(file1_id);
 status = H5Fclose(file2_id);


 return 0;


}



