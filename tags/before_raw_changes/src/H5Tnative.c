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

/*
 * Module Info: This module contains the functionality for querying
 *      a "native" datatype for the H5T interface.
 */

#define H5T_PACKAGE		/*suppress error about including H5Tpkg	  */

#include "H5private.h"		/*generic functions			  */
#include "H5Eprivate.h"		/*error handling			  */
#include "H5Iprivate.h"		/*ID functions		   		  */
#include "H5MMprivate.h"	/*memory management			  */
#include "H5Tpkg.h"		/*data-type functions			  */

#define PABLO_MASK	H5Tnative_mask

/* Interface initialization */
static int interface_initialize_g = 0;
#define INTERFACE_INIT H5T_init_native_interface
static herr_t H5T_init_native_interface(void);

/* Static local functions */
static H5T_t *H5T_get_native_type(H5T_t *dt, H5T_direction_t direction, 
                                  size_t *struct_align, size_t *offset, size_t *comp_size);
static H5T_t *H5T_get_native_integer(size_t size, H5T_sign_t sign, H5T_direction_t direction, 
                                     size_t *struct_align, size_t *offset, size_t *comp_size);
static H5T_t *H5T_get_native_float(size_t size, H5T_direction_t direction, 
                                   size_t *struct_align, size_t *offset, size_t *comp_size);
static herr_t H5T_cmp_offset(size_t *comp_size, size_t *offset, size_t elem_size,
                         size_t nelems, size_t align, size_t *struct_align);


/*--------------------------------------------------------------------------
NAME
   H5T_init_native_interface -- Initialize interface-specific information
USAGE
    herr_t H5T_init_native_interface()
   
RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.  (Just calls
    H5T_init_iterface currently).

--------------------------------------------------------------------------*/
static herr_t
H5T_init_native_interface(void)
{
    FUNC_ENTER_NOINIT(H5T_init_native_interface);

    FUNC_LEAVE_NOAPI(H5T_init_interface());
} /* H5T_init_native_interface() */


/*-------------------------------------------------------------------------
 * Function:    H5Tget_native_type
 *
 * Purpose:     High-level API to return the native type of a datatype. 
 *              The native type is chosen by matching the size and class of 
 *              querried datatype from the following native premitive 
 *              datatypes:
 *                      H5T_NATIVE_CHAR         H5T_NATIVE_UCHAR
 *                      H5T_NATIVE_SHORT        H5T_NATIVE_USHORT
 *                      H5T_NATIVE_INT          H5T_NATIVE_UINT
 *                      H5T_NATIVE_LONG         H5T_NATIVE_ULONG
 *                      H5T_NATIVE_LLONG        H5T_NATIVE_ULLONG
 *
 *                      H5T_NATIVE_FLOAT
 *                      H5T_NATIVE_DOUBLE
 *                      H5T_NATIVE_LDOUBLE
 *
 *              Compound, array, enum, and VL types all choose among these 
 *              types for theire members.  Time, Bifield, Opaque, Reference
 *              types are only copy out.
 *      
 * Return:      Success:        Returns the native data type if successful.
 *
 *              Failure:        negative
 *
 * Programmer:  Raymond Lu
 *              Oct 3, 2002 
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Tget_native_type(hid_t type_id, H5T_direction_t direction)
{
    H5T_t       *dt;                /* Datatype to create native datatype from */
    H5T_t       *new_dt=NULL;       /* Datatype for native datatype created */
    size_t      comp_size=0;        /* Compound datatype's size */
    hid_t       ret_value;          /* Return value */

    FUNC_ENTER_API(H5Tget_native_type, FAIL);
    H5TRACE2("i","iTd",type_id,direction);

    /* check argument */
    if(NULL==(dt=H5I_object_verify(type_id, H5I_DATATYPE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a data type");

    if(direction!=H5T_DIR_DEFAULT && direction!=H5T_DIR_ASCEND 
            && direction!=H5T_DIR_DESCEND)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not valid direction value");

    if((new_dt = H5T_get_native_type(dt, direction, NULL, NULL, &comp_size))==NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "cannot retrieve native type");

    if((ret_value=H5I_register(H5I_DATATYPE, new_dt)) < 0)
        HGOTO_ERROR(H5E_DATATYPE, H5E_CANTREGISTER, FAIL, "unable to register data type");
             
done:
    /* Error cleanup */
    if(ret_value<0) {
        if(new_dt)
            H5T_close(new_dt);
    } /* end if */
    
    FUNC_LEAVE_API(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:    H5T_get_native_type
 * 
 * Purpose:     Returns the native type of a datatype.
 * 
 * Return:      Success:        Returns the native data type if successful.
 * 
 *              Failure:        negative
 *              
 * Programmer:  Raymond Lu
 *              Oct 3, 2002
 *              
 * Modifications:
 * 
 *-------------------------------------------------------------------------
 */
static H5T_t*
H5T_get_native_type(H5T_t *dtype, H5T_direction_t direction, size_t *struct_align, size_t *offset, size_t *comp_size)
{   
    H5T_t       *dt;                /* Datatype to make native */
    H5T_class_t h5_class;           /* Class of datatype to make native */
    size_t      size;               /* Size of datatype to make native */
    int         nmemb;              /* Number of members in compound & enum types */
    H5T_t       *super_type;        /* Super type of VL, array and enum datatypes */
    H5T_t       *nat_super_type;    /* Native form of VL, array & enum super datatype */
    H5T_t       *new_type=NULL;     /* New native datatype */
    int         i;                  /* Local index variable */
    H5T_t       *ret_value;         /* Return value */

    FUNC_ENTER_NOAPI(H5T_get_native_type, NULL);
        
    assert(dtype); 

    if((h5_class = H5T_get_class(dtype))<0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid class");
        
    if((size =  H5T_get_size(dtype))==0)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid size");
        
    switch(h5_class) {
        case H5T_INTEGER:
            {
                H5T_sign_t  sign;       /* Signedness of integer type */

                if((sign =  H5T_get_sign(dtype))==H5T_SGN_ERROR)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a valid signess");
            
                if((ret_value = H5T_get_native_integer(size, sign, direction, struct_align, offset, comp_size))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve integer type");
            }
            break;
            
        case H5T_FLOAT:
            if((ret_value = H5T_get_native_float(size, direction, struct_align, offset, comp_size))==NULL)
                HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve float type");

            break;

        case H5T_STRING:
            {
                size_t align;
                size_t pointer_size;

                if(H5T_is_variable_str(dtype)) {
                    if(NULL==(dt=H5I_object(H5T_C_S1)))
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a data type");
                    if((ret_value=H5T_copy(dt, H5T_COPY_TRANSIENT))==NULL)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve float type");
                    if(H5T_set_size(ret_value, H5T_VARIABLE)<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot set size");

                    /* Update size, offset and compound alignment for parent. */
                    align = H5T_POINTER_COMP_ALIGN_g; 
                    pointer_size = sizeof(char*);
                    
                    if(H5T_cmp_offset(comp_size, offset, pointer_size, 1, align, struct_align)<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot compute compound offset");

                } else { 
                    /*size_t          char_size;*/

                    if(NULL==(dt=H5I_object(H5T_NATIVE_UCHAR)))
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a data type");
                    if((ret_value=H5T_copy(dt, H5T_COPY_TRANSIENT))==NULL)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve float type");
                    if(H5T_set_size(ret_value, size)<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot set size");

                    /* Update size, offset and compound alignment for parent. */
                    align = H5T_NATIVE_SCHAR_COMP_ALIGN_g; 
                    
                    if(H5T_cmp_offset(comp_size, offset, sizeof(char), size, align, struct_align)<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot compute compound offset");

                }
            }
            break;

        /* These three types don't need to compute compound field information since they 
         * can't be used as field. */
        case H5T_TIME:
        case H5T_BITFIELD:
        case H5T_OPAQUE:
            if((ret_value=H5T_copy(dtype, H5T_COPY_TRANSIENT))==NULL)
                 HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve float type");

            break;
        case H5T_REFERENCE:
            {
                size_t align;
                size_t ref_size;
                int    not_equal;

                if((ret_value=H5T_copy(dtype, H5T_COPY_TRANSIENT))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve float type");
               
                /* Decide if the data type is object or dataset region reference. */
                if(NULL==(dt=H5I_object(H5T_STD_REF_OBJ_g)))
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a data type");
                not_equal = H5T_cmp(ret_value, dt);

                /* Update size, offset and compound alignment for parent. */
                if(!not_equal) {                    
                    align = H5T_HOBJREF_COMP_ALIGN_g;
                    ref_size = sizeof(hobj_ref_t);
                } else {
                    align = H5T_HDSETREGREF_COMP_ALIGN_g;
                    ref_size = sizeof(hdset_reg_ref_t);
                }
                
                if(H5T_cmp_offset(comp_size, offset, ref_size, 1, align, struct_align)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot compute compound offset");
            }
            break;

        case H5T_COMPOUND:
            {
                H5T_t       *memb_type;     /* Datatype of member */
                H5T_t       **memb_list;    /* List of compound member IDs */
                size_t      *memb_offset;   /* List of member offsets in compound type, including member size and alignment */
                size_t      children_size=0;/* Total size of compound members */ 
                size_t      children_st_align=0;    /* The max alignment among compound members.  This'll be the compound alignment */
                char        **comp_mname;   /* List of member names in compound type */

                if((nmemb = H5T_get_nmembers(dtype))<=0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "compound data type doesn't have any member");
                
                if((memb_list   = (H5T_t**)H5MM_malloc(nmemb*sizeof(H5T_t*)))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot allocate memory"); 
                if((memb_offset = (size_t*)H5MM_calloc(nmemb*sizeof(size_t)))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot allocate memory"); 
                if((comp_mname = (char**)H5MM_malloc(nmemb*sizeof(char*)))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot allocate memory"); 

                /* Construct child compound type and retrieve a list of their IDs, offsets, total size, and alignment for compound type. */  
                for(i=0; i<nmemb; i++) {
                    if((memb_type = H5T_get_member_type(dtype, i))==NULL)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "member type retrieval failed");
                        
                    if((comp_mname[i] = H5T_get_member_name(dtype, i))==NULL)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "member type retrieval failed");
                        
                    if((memb_list[i] = H5T_get_native_type(memb_type, direction, &children_st_align, &(memb_offset[i]), &children_size))==NULL)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "member identifier retrieval failed");

                    if(H5T_close(memb_type)<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");
                }
               
                /* The alignment for whole compound type */
                if(children_st_align && children_size % children_st_align) {
                    memb_offset[nmemb-1] += children_size % children_st_align;
                    children_size += children_size % children_st_align;
                }

                /* Construct new compound type based on native type */   
                if((new_type=H5T_create(H5T_COMPOUND, children_size))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot create a compound type");

                /* Insert members for the new compound type */       
                for(i=0; i<nmemb; i++) {
                    if(H5T_insert(new_type, comp_mname[i], memb_offset[i], memb_list[i])<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot insert member to compound datatype");
                }
                
                /* Update size, offset and compound alignment for parent. */
                if(offset)
                    *offset = *comp_size;
                if(struct_align && *struct_align < children_st_align)
                    *struct_align = children_st_align;
                *comp_size += children_size;
                
                /* Close member data type */ 
                for(i=0; i<nmemb; i++) {
                    if(H5T_close(memb_list[i])<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");

                    /* Free member names in list */
                    H5MM_xfree(comp_mname[i]);
                }
               
                /* Free lists for members */
                H5MM_xfree(memb_list);
                H5MM_xfree(memb_offset);
                H5MM_xfree(comp_mname);
                
                ret_value = new_type;
            }
            break;

        case H5T_ENUM:
            {
                char        *memb_name;         /* Enum's member name */
                void        *memb_value;        /* Enum's member value */
               
                /* Don't need to do anything special for alignment, offset since the ENUM type usually is integer. */

                /* Retrieve base type for enumarate type */
                if((super_type=H5T_get_super(dtype))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "unable to get base type for enumarate type");
                if((nat_super_type = H5T_get_native_type(super_type, direction, struct_align, offset, comp_size))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "base native type retrieval failed");

                /* Allocate room for the enum values */
                if((memb_value = H5MM_malloc(H5T_get_size(super_type)))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot allocate memory"); 

                /* Close super type */
                if(H5T_close(super_type)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");
                 
                /* Construct new enum type based on native type */   
                if((new_type=H5T_enum_create(nat_super_type))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "unable to create enum type");
                    
                /* Retrieve member info and insert members into new enum type */
                if((nmemb = H5T_get_nmembers(dtype))<=0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "enumarate data type doesn't have any member");
                for(i=0; i<nmemb; i++) {
                    if((memb_name=H5T_get_member_name(dtype, i))==NULL)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot get member name");
                    if(H5T_get_member_value(dtype, i, memb_value)<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot get member value");
                    if(H5T_enum_insert(new_type, memb_name, memb_value)<0)
                        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot insert member");
                    H5MM_xfree(memb_name);
                }
                H5MM_xfree(memb_value);

                /* Close base type */
                if(H5T_close(nat_super_type)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");
               
                ret_value = new_type;
            }
            break;

        case H5T_ARRAY:
            {
                int         array_rank;         /* Array's rank */
                hsize_t     *dims = NULL;       /* Dimension sizes for array */
                hsize_t     nelems = 1;
                size_t      super_offset=0;       
                size_t      super_size=0;       
                size_t      super_align=0;   

                /* Retrieve dimension information for array data type */
                if((array_rank=H5T_get_array_ndims(dtype))<=0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot get dimension rank");
                if((dims = (hsize_t*)H5MM_malloc(array_rank*sizeof(hsize_t)))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot allocate memory"); 
                if(H5T_get_array_dims(dtype, dims, NULL)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot get dimension size");

                /* Retrieve base type for array type */
                if((super_type=H5T_get_super(dtype))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "unable to get parent type for enumarate type");
                if((nat_super_type = H5T_get_native_type(super_type, direction, &super_align, 
                                                         &super_offset, &super_size))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "parent native type retrieval failed");

                /* Close super type */
                if(H5T_close(super_type)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");

                /* Create a new array type based on native type */
                if((new_type=H5T_array_create(nat_super_type, array_rank, dims, NULL))==NULL) 
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "unable to create array type");

                /* Close base type */
                if(H5T_close(nat_super_type)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");
                    
                for(i=0; i<array_rank; i++)
                    nelems *= dims[i];                   
                H5_CHECK_OVERFLOW(nelems,hsize_t,size_t);
                if(H5T_cmp_offset(comp_size, offset, super_size, (size_t)nelems, super_align, struct_align)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot compute compound offset");
                        
                H5MM_xfree(dims);
                ret_value = new_type;
            }
            break;

        case H5T_VLEN:
            {
                size_t      vl_align = 0;
                size_t      vl_size  = 0;
                size_t      super_size=0;       

                /* Retrieve base type for array type */
                if((super_type=H5T_get_super(dtype))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "unable to get parent type for enumarate type");
                /* Don't need alignment, offset information if this VL isn't a field of compound type.  If it
                 * is, go to a few steps below to compute the information directly. */
                if((nat_super_type = H5T_get_native_type(super_type, direction, NULL, NULL, &super_size))==NULL)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "parent native type retrieval failed");

                /* Close super type */
                if(H5T_close(super_type)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");

                /* Create a new array type based on native type */
                if((new_type=H5T_vlen_create(nat_super_type))==NULL) 
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "unable to create VL type");
           
                /* Close base type */
                if(H5T_close(nat_super_type)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot close datatype");
            
                /* Update size, offset and compound alignment for parent compound type directly. */
                vl_align = H5T_HVL_COMP_ALIGN_g;
                vl_size  = sizeof(hvl_t);
                
                if(H5T_cmp_offset(comp_size, offset, vl_size, 1, vl_align, struct_align)<0)
                    HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot compute compound offset");
                        
                ret_value = new_type;
            }
            break;   

        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "data type doesn't match any native type");
    }   

done:
    /* Error cleanup */
    if(ret_value==NULL) {
        if(new_type)
            H5T_close(new_type);
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:    H5T_get_native_integer
 * 
 * Purpose:     Returns the native integer type of a datatype.
 * 
 * Return:      Success:        Returns the native data type if successful.
 * 
 *              Failure:        negative
 *              
 * Programmer:  Raymond Lu
 *              Oct 3, 2002
 *              
 * Modifications:
 * 
 *-------------------------------------------------------------------------
 */
static H5T_t*
H5T_get_native_integer(size_t size, H5T_sign_t sign, H5T_direction_t direction, 
                              size_t *struct_align, size_t *offset, size_t *comp_size)
{  
    H5T_t       *dt;            /* Appropriate native datatype to copy */
    hid_t       tid=(-1);       /* Datatype ID of appropriate native datatype */
    size_t      align=0;        /* Alignment necessary for native datatype */
    enum match_type {           /* The different kinds of integers we can match */
        H5T_NATIVE_INT_MATCH_CHAR,
        H5T_NATIVE_INT_MATCH_SHORT,
        H5T_NATIVE_INT_MATCH_INT,
        H5T_NATIVE_INT_MATCH_LONG,
        H5T_NATIVE_INT_MATCH_LLONG,
        H5T_NATIVE_INT_MATCH_UNKNOWN
    } match=H5T_NATIVE_INT_MATCH_UNKNOWN;
    H5T_t       *ret_value;     /* Return value */

    FUNC_ENTER_NOAPI(H5T_get_native_integer, NULL);

    assert(size>0);
    
    if(direction == H5T_DIR_DEFAULT || direction == H5T_DIR_ASCEND) {         
        if(size==sizeof(char))
            match=H5T_NATIVE_INT_MATCH_CHAR;
        else if(size==sizeof(short))
            match=H5T_NATIVE_INT_MATCH_SHORT;
        else if(size==sizeof(int))
            match=H5T_NATIVE_INT_MATCH_INT;
        else if(size==sizeof(long))
            match=H5T_NATIVE_INT_MATCH_LONG;
        else if(size==sizeof(long_long))
            match=H5T_NATIVE_INT_MATCH_LLONG;
        else   /* If no native type matches the querried datatype, simply choose the type of biggest size. */
            match=H5T_NATIVE_INT_MATCH_LLONG;
    } else if(direction == H5T_DIR_DESCEND) {         
        if(size==sizeof(long_long))
            match=H5T_NATIVE_INT_MATCH_LLONG;
        else if(size==sizeof(long))
            match=H5T_NATIVE_INT_MATCH_LONG;
        else if(size==sizeof(int))
            match=H5T_NATIVE_INT_MATCH_INT;
        else if(size==sizeof(short))
            match=H5T_NATIVE_INT_MATCH_SHORT;
        else if(size==sizeof(char))
            match=H5T_NATIVE_INT_MATCH_CHAR;
        else   /* If no native type matches the querried datatype, simple choose the type of smallest size. */
            match=H5T_NATIVE_INT_MATCH_CHAR;
    }

    /* Set the appropriate native datatype information */
    switch(match) {
        case H5T_NATIVE_INT_MATCH_CHAR:
            if(sign==H5T_SGN_2)
                tid = H5T_NATIVE_SCHAR;
            else
                tid = H5T_NATIVE_UCHAR;
            
            align = H5T_NATIVE_SCHAR_COMP_ALIGN_g; 
            break;
            
        case H5T_NATIVE_INT_MATCH_SHORT:
            if(sign==H5T_SGN_2)
                tid = H5T_NATIVE_SHORT;
            else
                tid = H5T_NATIVE_USHORT;

            align = H5T_NATIVE_SHORT_COMP_ALIGN_g; 
            break;
            
        case H5T_NATIVE_INT_MATCH_INT:
            if(sign==H5T_SGN_2)
                tid = H5T_NATIVE_INT;
            else  
                tid = H5T_NATIVE_UINT;

            align = H5T_NATIVE_INT_COMP_ALIGN_g; 
            break;
            
        case H5T_NATIVE_INT_MATCH_LONG:
            if(sign==H5T_SGN_2)
                tid = H5T_NATIVE_LONG;
            else  
                tid = H5T_NATIVE_ULONG;

            align = H5T_NATIVE_LONG_COMP_ALIGN_g;            
            break;
            
        case H5T_NATIVE_INT_MATCH_LLONG:
            if(sign==H5T_SGN_2)
                tid = H5T_NATIVE_LLONG;
            else  
                tid = H5T_NATIVE_ULLONG;

            align = H5T_NATIVE_LLONG_COMP_ALIGN_g;            
            break;
            
        default:
            assert(0 && "Unknown native integer match!");
            break;
    } /* end switch */
        
    /* Create new native type */
    assert(tid>=0);
    if(NULL==(dt=H5I_object(tid)))
         HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a data type");
    if((ret_value=H5T_copy(dt, H5T_COPY_TRANSIENT))==NULL)
         HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve float type");
            
    /* compute size and offset of compound type member. */
    if(H5T_cmp_offset(comp_size, offset, size, 1, align, struct_align)<0)
         HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot compute compound offset");
                        
done:
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:    H5T_get_native_float
 * 
 * Purpose:     Returns the native floatt type of a datatype.
 * 
 * Return:      Success:        Returns the native data type if successful.
 * 
 *              Failure:        negative
 *              
 * Programmer:  Raymond Lu
 *              Oct 3, 2002
 *              
 * Modifications:
 * 
 *-------------------------------------------------------------------------
 */
static H5T_t*
H5T_get_native_float(size_t size, H5T_direction_t direction, size_t *struct_align, size_t *offset, size_t *comp_size)
{  
    H5T_t       *dt=NULL;       /* Appropriate native datatype to copy */
    hid_t       tid=(-1);       /* Datatype ID of appropriate native datatype */
    size_t      align=0;        /* Alignment necessary for native datatype */
    enum match_type {           /* The different kinds of floating point types we can match */
        H5T_NATIVE_FLOAT_MATCH_FLOAT,
        H5T_NATIVE_FLOAT_MATCH_DOUBLE,
        H5T_NATIVE_FLOAT_MATCH_LDOUBLE,
        H5T_NATIVE_FLOAT_MATCH_UNKNOWN
    } match=H5T_NATIVE_FLOAT_MATCH_UNKNOWN;
    H5T_t       *ret_value;     /* Return value */

    FUNC_ENTER_NOAPI(H5T_get_native_integer, NULL);

    assert(size>0);
    
    if(direction == H5T_DIR_DEFAULT || direction == H5T_DIR_ASCEND) {        
        if(size==sizeof(float))
            match=H5T_NATIVE_FLOAT_MATCH_FLOAT;
        else if(size==sizeof(double))
            match=H5T_NATIVE_FLOAT_MATCH_DOUBLE;
        else if(size==sizeof(long double))
            match=H5T_NATIVE_FLOAT_MATCH_LDOUBLE;
        else   /* If not match, return the biggest datatype */
            match=H5T_NATIVE_FLOAT_MATCH_LDOUBLE;
    } else {
        if(size==sizeof(long double))
            match=H5T_NATIVE_FLOAT_MATCH_LDOUBLE;
        else if(size==sizeof(double))
            match=H5T_NATIVE_FLOAT_MATCH_DOUBLE;
        else if(size==sizeof(float))
            match=H5T_NATIVE_FLOAT_MATCH_FLOAT;
        else
            match=H5T_NATIVE_FLOAT_MATCH_FLOAT;
    }

    /* Set the appropriate native floating point information */
    switch(match) {
        case H5T_NATIVE_FLOAT_MATCH_FLOAT:
            tid = H5T_NATIVE_FLOAT;
            align = H5T_NATIVE_FLOAT_COMP_ALIGN_g;
            break;

        case H5T_NATIVE_FLOAT_MATCH_DOUBLE:
            tid = H5T_NATIVE_DOUBLE;
            align = H5T_NATIVE_DOUBLE_COMP_ALIGN_g;
            break;

        case H5T_NATIVE_FLOAT_MATCH_LDOUBLE:
            tid = H5T_NATIVE_LDOUBLE;
            align = H5T_NATIVE_LDOUBLE_COMP_ALIGN_g;
            break;

        default:
            assert(0 && "Unknown native floating-point match!");
            break;
    } /* end switch */
   
    /* Create new native type */
    assert(tid>=0);
    if(NULL==(dt=H5I_object(tid)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "not a data type");
    if((ret_value=H5T_copy(dt, H5T_COPY_TRANSIENT))==NULL)
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot retrieve float type");
      
    /* compute offset of compound type member. */
    if(H5T_cmp_offset(comp_size, offset, size, 1, align, struct_align)<0)
         HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, NULL, "cannot compute compound offset");
                        
done:
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5T_cmp_offset
 *
 * Purpose:	This function is only for convenience.  It computes the 
 *              compound type size, offset of the member being considered
 *              and the alignment for the whole compound type.
 *
 * Return:	Success:        Non-negative value.	
 *
 *		Failure:        Negative value.	
 *
 * Programmer:	Raymond Lu
 *		December  10, 2002 
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5T_cmp_offset(size_t *comp_size, size_t *offset, size_t elem_size, 
                      size_t nelems, size_t align, size_t *struct_align)
{
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI(H5T_cmp_offset, FAIL);

    if(offset && comp_size) {
        if(align>1 && *comp_size%align) {
            /* Add alignment value */
            *offset = *comp_size +  (align - *comp_size%align);
            *comp_size += (align - *comp_size%align);
        } else
            *offset = *comp_size;

        /* compute size of compound type member. */ 
        *comp_size += nelems*elem_size;
    }

    if(struct_align && *struct_align < align)
        *struct_align = align;

done:
    FUNC_LEAVE_NOAPI(ret_value);
}

