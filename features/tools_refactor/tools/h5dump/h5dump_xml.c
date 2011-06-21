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
#include <stdio.h>
#include <stdlib.h>

#include "H5private.h"
#include "h5tools.h"
#include "h5tools_dump.h"
#include "h5tools_utils.h"
#include "h5tools_ref.h"
#include "h5trav.h"
#include "h5dump_extern.h"
#include "h5dump_xml.h"

/* internal functions */
static int      xml_name_to_XID(const char *, char *, int , int );

/* internal functions used by XML option */
static void             xml_print_datatype(hid_t, unsigned);
static void             xml_print_enum(hid_t);
static int              xml_print_refs(hid_t, int);
static int              xml_print_strs(hid_t, int);
static char            *xml_escape_the_string(const char *, int);
static char            *xml_escape_the_name(const char *);

/*-------------------------------------------------------------------------
 * Function:    dump_all_cb
 *
 * Purpose:     function callback called by H5Literate,
 *                displays everything in the specified object
 *
 * Return:      Success:        SUCCEED
 *
 *              Failure:        FAIL
 *
 * Programmer:  Ruey-Hsia Li
 *
 * Modifications:
 *  RMcG, November 2000
 *   Added XML support. Also, optionally checks the op_data argument
 *
 * PVN, May 2008
 *   Dump external links
 *
 *-------------------------------------------------------------------------
 */
static herr_t
xml_dump_all_cb(hid_t group, const char *name, const H5L_info_t *linfo, void UNUSED *op_data)
{
    hid_t       obj;
    herr_t      ret = SUCCEED;
    char       *obj_path = NULL;    /* Full path of object */

    /* Build the object's path name */
    obj_path = (char *)HDmalloc(HDstrlen(prefix) + HDstrlen(name) + 2);
    if(!obj_path) {
        ret = FAIL;
        goto done;
    } 

    HDstrcpy(obj_path, prefix);
    HDstrcat(obj_path, "/");
    HDstrcat(obj_path, name);

    HDfprintf(stdout, "\n");

    if(linfo->type == H5L_TYPE_HARD) {
        H5O_info_t  oinfo;

        /* Stat the object */
        if(H5Oget_info_by_name(group, name, &oinfo, H5P_DEFAULT) < 0) {
            error_msg("unable to get object information for \"%s\"\n", name);
            h5tools_setstatus(EXIT_FAILURE);
            ret = FAIL;
            goto done;
        } /* end if */

        switch(oinfo.type) {
        case H5O_TYPE_GROUP:
            if((obj = H5Gopen2(group, name, H5P_DEFAULT)) < 0)  {
                error_msg("unable to dump group \"%s\"\n", name);
                h5tools_setstatus(EXIT_FAILURE);
                ret = FAIL;
            }
            else {
                char *old_prefix; /* Pointer to previous prefix */

                /* Keep copy of prefix before iterating into group */
                old_prefix = HDstrdup(prefix);
                HDassert(old_prefix);

                /* Append group name to prefix */
                add_prefix(&prefix, &prefix_len, name);

                /* Iterate into group */
                dump_function_table->dump_group_function(obj, name);

                /* Restore old prefix name */
                HDstrcpy(prefix, old_prefix);
                HDfree(old_prefix);

                /* Close group */
                H5Gclose(obj);
            }
            break;

        case H5O_TYPE_DATASET:
            if((obj = H5Dopen2(group, name, H5P_DEFAULT)) >= 0) {
                if(oinfo.rc > 1 || hit_elink) {
                    obj_t  *found_obj;    /* Found object */

                    found_obj = search_obj(dset_table, oinfo.addr);

                    if(found_obj == NULL) {
                        indentation(dump_indent);
                        begin_obj(h5tools_dump_header_format->datasetbegin, name, h5tools_dump_header_format->datasetblockbegin);
                        HDfprintf(stdout, "\n");
                        indentation(dump_indent + COL);
                        error_msg("internal error (file %s:line %d)\n", __FILE__, __LINE__);
                        indentation(dump_indent);
                        end_obj(h5tools_dump_header_format->datasetend, h5tools_dump_header_format->datasetblockend);
                        HDfprintf(stdout, "\n");
                        h5tools_setstatus(EXIT_FAILURE);
                        ret = FAIL;
                        H5Dclose(obj);
                        goto done;
                    } 
                    else if(found_obj->displayed) {
                        /* the XML version */
                        char *t_obj_path = xml_escape_the_name(obj_path);
                        char *t_prefix = xml_escape_the_name(HDstrcmp(prefix,"") ? prefix : "/");
                        char *t_name = xml_escape_the_name(name);
                        char *t_objname = xml_escape_the_name(found_obj->objname);
                        char dsetxid[100];
                        char parentxid[100];
                        char pointerxid[100];

                        indentation(dump_indent);

                        /* Create OBJ-XIDs for the parent and object */
                        xml_name_to_XID(obj_path, dsetxid, sizeof(dsetxid), 1);
                        xml_name_to_XID(prefix, parentxid, sizeof(parentxid), 1);

                        HDfprintf(stdout, "<%sDataset Name=\"%s\" OBJ-XID=\"%s-%d\" "
                                "H5Path=\"%s\" Parents=\"%s\" "
                                "H5ParentPaths=\"%s\">\n",
                                xmlnsprefix,
                                t_name,                     /* Dataset Name */
                                dsetxid, get_next_xid(),    /* OBJ-XID */
                                t_obj_path,                 /* H5Path */
                                parentxid,                  /* Parents */
                                t_prefix);                  /* H5ParentPaths */

                        indentation(dump_indent + COL);
                        xml_name_to_XID(found_obj->objname, pointerxid, sizeof(pointerxid), 1);
                        HDfprintf(stdout, "<%sDatasetPtr OBJ-XID=\"%s\" H5Path=\"%s\"/>\n",
                                xmlnsprefix,
                                pointerxid,t_objname);
                        indentation(dump_indent);
                        HDfprintf(stdout, "</%sDataset>\n", xmlnsprefix);

                        HDfree(t_name);
                        HDfree(t_obj_path);
                        HDfree(t_prefix);
                        HDfree(t_objname);

                        H5Dclose(obj);
                        goto done;
                    } 
                    else {
                        found_obj->displayed = TRUE;
                    }
                } /* end if */

                dump_function_table->dump_dataset_function(obj, name, NULL);
                H5Dclose(obj);
            } 
            else {
                error_msg("unable to dump dataset \"%s\"\n", name);
                h5tools_setstatus(EXIT_FAILURE);
                ret = FAIL;
            }
            break;

        case H5O_TYPE_NAMED_DATATYPE:
            if((obj = H5Topen2(group, name, H5P_DEFAULT)) < 0) {
                error_msg("unable to dump datatype \"%s\"\n", name);
                h5tools_setstatus(EXIT_FAILURE);
                ret = FAIL;
            } 
            else {
                dump_function_table->dump_named_datatype_function(obj, name);
                H5Tclose(obj);
            }
            break;

        default:
            error_msg("unknown object \"%s\"\n", name);
            h5tools_setstatus(EXIT_FAILURE);
            ret = FAIL;
        }
    } /* end if */
    else {
        char       *targbuf;

        switch(linfo->type) {
        case H5L_TYPE_SOFT:
            indentation(dump_indent);
            targbuf = (char *)HDmalloc(linfo->u.val_size);
            HDassert(targbuf);

            if(H5Lget_val(group, name, targbuf, linfo->u.val_size, H5P_DEFAULT) < 0) {
                error_msg("unable to get link value\n");
                h5tools_setstatus(EXIT_FAILURE);
                ret = FAIL;
            } 
            else {
                /* print the value of a soft link */
                /* XML */
                char linkxid[100];
                char parentxid[100];
                char targetxid[100];
                char *t_prefix = xml_escape_the_name(HDstrcmp(prefix,"") ? prefix : "/");
                char *t_name = xml_escape_the_name(name);
                char *t_targbuf = xml_escape_the_name(targbuf);
                char *t_obj_path = xml_escape_the_name(obj_path);
                char *t_link_path;
                int res;

                t_link_path = (char *)HDmalloc(HDstrlen(prefix) + linfo->u.val_size + 1);
                if(targbuf[0] == '/')
                    HDstrcpy(t_link_path, targbuf);
                else {
                    HDstrcpy(t_link_path, prefix);
                    HDstrcat(HDstrcat(t_link_path, "/"), targbuf);
                } /* end else */

                /* Create OBJ-XIDs for the parent and object */
                xml_name_to_XID(t_obj_path, linkxid, sizeof(linkxid), 1);
                xml_name_to_XID(prefix, parentxid, sizeof(parentxid), 1);

                /* Try to create an OBJ-XID for the object pointed to */
                res = xml_name_to_XID(t_link_path, targetxid, sizeof(targetxid), 0);
                if (res == 0) {
                    /* target obj found */
                    HDfprintf(stdout, "<%sSoftLink LinkName=\"%s\" "
                            "OBJ-XID=\"%s\" "
                            "H5SourcePath=\"%s\" "
                            "TargetPath=\"%s\" TargetObj=\"%s\" "
                            "Parents=\"%s\" H5ParentPaths=\"%s\" />\n",
                            xmlnsprefix,
                            t_name,         /* LinkName */
                            linkxid,        /* OBJ-XID */
                            t_obj_path,     /* H5SourcePath */
                            t_targbuf,      /* TargetPath */
                            targetxid,      /* TargetObj */
                            parentxid,      /* Parents */
                            t_prefix);      /* H5ParentPaths */
                } 
                else {
                    /* dangling link -- omit from xml attributes */
                    HDfprintf(stdout, "<%sSoftLink LinkName=\"%s\" "
                            "OBJ-XID=\"%s\" "
                            "H5SourcePath=\"%s\" "
                            "TargetPath=\"%s\"  "
                            "Parents=\"%s\" H5ParentPaths=\"%s\" />\n",
                            xmlnsprefix,
                            t_name,         /* LinkName */
                            linkxid,        /* OBJ-XID */
                            t_obj_path,     /* H5SourcePath */
                            t_targbuf,      /* TargetPath */
                            parentxid,      /* Parents */
                            t_prefix);      /* H5ParentPaths */
                }

                HDfree(t_prefix);
                HDfree(t_name);
                HDfree(t_targbuf);
                HDfree(t_obj_path);
                HDfree(t_link_path);
            }

            HDfree(targbuf);
            break;

        case H5L_TYPE_EXTERNAL:
            targbuf = (char *)HDmalloc(linfo->u.val_size);
            HDassert(targbuf);

            indentation(dump_indent);
            if(!doxml) {
                begin_obj(h5tools_dump_header_format->extlinkbegin, name, h5tools_dump_header_format->extlinkblockbegin);
                HDfprintf(stdout, "\n");
            }
            if(H5Lget_val(group, name, targbuf, linfo->u.val_size, H5P_DEFAULT) < 0) {
                error_msg("unable to get external link value\n");
                h5tools_setstatus(EXIT_FAILURE);
                ret = FAIL;
            } /* end if */
            else {
                const char *filename;
                const char *targname;

                if(H5Lunpack_elink_val(targbuf, linfo->u.val_size, NULL, &filename, &targname) < 0) {
                    error_msg("unable to unpack external link value\n");
                    h5tools_setstatus(EXIT_FAILURE);
                    ret = FAIL;
                } /* end if */
                else {
                    char linkxid[100];
                    char parentxid[100];
                    char *t_name = xml_escape_the_name(name);
                    char *t_prefix = xml_escape_the_name(HDstrcmp(prefix,"") ? prefix : "/");
                    char *t_obj_path = xml_escape_the_name(obj_path);
                    char *t_filename = xml_escape_the_name(filename);
                    char *t_targname = xml_escape_the_name(targname);

                    /* Create OBJ-XIDs for the parent and object */
                    xml_name_to_XID(t_obj_path, linkxid, sizeof(linkxid), 1);
                    xml_name_to_XID(prefix, parentxid, sizeof(parentxid), 1);

                    HDfprintf(stdout, "<%sExternalLink LinkName=\"%s\" "
                            "OBJ-XID=\"%s\" "
                            "H5SourcePath=\"%s\" "
                            "TargetFilename=\"%s\"  "
                            "TargetPath=\"%s\"  "
                            "Parents=\"%s\" H5ParentPaths=\"%s\" />\n",
                            xmlnsprefix,
                            t_name,         /* LinkName */
                            linkxid,        /* OBJ-XID */
                            t_obj_path,     /* H5SourcePath */
                            filename,       /* TargetFilename */
                            targname,       /* TargetPath*/
                            parentxid,      /* Parents */
                            t_prefix);      /* H5ParentPaths */
                    HDfree(t_prefix);
                    HDfree(t_name);
                    HDfree(t_filename);
                    HDfree(t_targname);
                    HDfree(t_obj_path);
                } /* end else */
            } /* end else */
            HDfree(targbuf);
            break;

        default:
        {
            char linkxid[100];
            char parentxid[100];
            char *t_name = xml_escape_the_name(name);
            char *t_prefix = xml_escape_the_name(HDstrcmp(prefix,"") ? prefix : "/");
            char *t_obj_path = xml_escape_the_name(obj_path);

            indentation(dump_indent);

            /* Create OBJ-XIDs for the parent and object */
            xml_name_to_XID(t_obj_path, linkxid, sizeof(linkxid), 1);
            xml_name_to_XID(prefix, parentxid, sizeof(parentxid), 1);

            HDfprintf(stdout, "<%sUserDefined LinkName=\"%s\" "
                    "OBJ-XID=\"%s\" "
                    "H5SourcePath=\"%s\" "
                    "LinkClass=\"%d\"  "
                    "Parents=\"%s\" H5ParentPaths=\"%s\" />\n",
                    xmlnsprefix,
                    t_name,             /* LinkName */
                    linkxid,            /* OBJ-XID */
                    t_obj_path,         /* H5SourcePath */
                    linfo->type,        /* LinkClass */
                    parentxid,          /* Parents */
                    t_prefix);          /* H5ParentPaths */
            HDfree(t_prefix);
            HDfree(t_name);
            HDfree(t_obj_path);
        }
        break;
        } /* end switch */
    } /* end else */

    done:
    if(obj_path)
        HDfree(obj_path);
    return ret;
}

/*
 * create a string suitable for and XML NCNAME.  Uses the
 * object reference to create the string.
 *
 *  'gen'; 0 - return null if not found
 *         1 - generate a fake entry and return fake id.
 */
int
xml_name_to_XID(const char *str , char *outstr, int outlen, int gen)
{
    haddr_t objno;      /* Object ID for object at path */

    if (outlen < 22) return 1;

    objno = ref_path_table_lookup(str);
    if (objno == HADDR_UNDEF) {
        if (HDstrlen(str) == 0) {
            objno = ref_path_table_lookup("/");
            if (objno == HADDR_UNDEF) {
                if (gen) {
                    objno = ref_path_table_gen_fake(str);
                    sprintf(outstr, "xid_"H5_PRINTF_HADDR_FMT, objno);
                    return 0;
                } 
                else {
                    return 1;
                }
            }
        } 
        else {
            if (gen) {
                objno = ref_path_table_gen_fake(str);
                sprintf(outstr, "xid_"H5_PRINTF_HADDR_FMT, objno);
                return 0;
            } 
            else {
                return 1;
            }
        }
    }

    sprintf(outstr, "xid_"H5_PRINTF_HADDR_FMT, objno);

    return(0);
}

static const char      *quote = "&quot;";
static const char      *amp = "&amp;";
static const char      *lt = "&lt;";
static const char      *gt = "&gt;";
static const char      *apos = "&apos;";

/*-------------------------------------------------------------------------
 * Function:    xml_escape_the_name
 *
 * Purpose:     Escape XML reserved chars in a name, so HDF5 strings
 *              and paths can be correctly read back in XML element.
 *
 * Return:      The revised string.
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char                   *
xml_escape_the_name(const char *str)
{
    size_t      extra;
    size_t      len;
    size_t      i;
    const char *cp;
    char       *ncp;
    char       *rcp;

    if (!str)
        return NULL;

    cp = str;
    len = strlen(str);
    extra = 0;

    for (i = 0; i < len; i++) {
        if (*cp == '\"') {
            extra += (strlen(quote) - 1);
        } 
        else if (*cp == '\'') {
            extra += (strlen(apos) - 1);
        } 
        else if (*cp == '<') {
            extra += (strlen(lt) - 1);
        } 
        else if (*cp == '>') {
            extra += (strlen(gt) - 1);
        } 
        else if (*cp == '&') {
            extra += (strlen(amp) - 1);
        }

        cp++;
    }

    if (extra == 0)
        return HDstrdup(str);

    cp = str;
    rcp = ncp = (char *)HDmalloc(len + extra + 1);

    if (!ncp)
        return NULL;    /* ?? */

    for (i = 0; i < len; i++) {
        if (*cp == '\'') {
            strncpy(ncp, apos, strlen(apos));
            ncp += strlen(apos);
            cp++;
        } 
        else if (*cp == '<') {
            strncpy(ncp, lt, strlen(lt));
            ncp += strlen(lt);
            cp++;
        } 
        else if (*cp == '>') {
            strncpy(ncp, gt, strlen(gt));
            ncp += strlen(gt);
            cp++;
        } 
        else if (*cp == '\"') {
            strncpy(ncp, quote, strlen(quote));
            ncp += strlen(quote);
            cp++;
        } 
        else if (*cp == '&') {
            strncpy(ncp, amp, strlen(amp));
            ncp += strlen(amp);
            cp++;
        } 
        else {
            *ncp++ = *cp++;
        }
    }

    *ncp = '\0';
    return rcp;
}

/*-------------------------------------------------------------------------
 * Function:    xml_escape_the_string
 *
 * Purpose:     Escape XML reserved chars in a string, so HDF5 strings
 *              and paths can be correctly read back in XML CDATA.
 *
 * Return:      The revised string.
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char                   *
xml_escape_the_string(const char *str, int slen)
{
    size_t      extra;
    size_t      len;
    size_t      i;
    const char *cp;
    char       *ncp;
    char       *rcp;

    if (!str)
        return NULL;

    cp = str;

    if (slen < 0)
        len = strlen(str);
    else
        len = slen;

    extra = 0;

    for (i = 0; i < len; i++) {
        if (*cp == '\\') {
            extra++;
        }
        else if (*cp == '\"') {
            extra++;
        }
        else if (*cp == '\'') {
            extra += (strlen(apos) - 1);
        }
        else if (*cp == '<') {
            extra += (strlen(lt) - 1);
        }
        else if (*cp == '>') {
            extra += (strlen(gt) - 1);
        }
        else if (*cp == '&') {
            extra += (strlen(amp) - 1);
        }
        cp++;
    }

    cp = str;
    rcp = ncp = (char *) calloc((len + extra + 1), sizeof(char));

    if (ncp == NULL)
        return NULL; /* ?? */

    for (i = 0; i < len; i++) {
        if (*cp == '\\') {
            *ncp++ = '\\';
            *ncp++ = *cp++;
        }
        else if (*cp == '\"') {
            *ncp++ = '\\';
            *ncp++ = *cp++;
        }
        else if (*cp == '\'') {
            strncpy(ncp, apos, strlen(apos));
            ncp += strlen(apos);
            cp++;
        }
        else if (*cp == '<') {
            strncpy(ncp, lt, strlen(lt));
            ncp += strlen(lt);
            cp++;
        }
        else if (*cp == '>') {
            strncpy(ncp, gt, strlen(gt));
            ncp += strlen(gt);
            cp++;
        }
        else if (*cp == '&') {
            strncpy(ncp, amp, strlen(amp));
            ncp += strlen(amp);
            cp++;
        }
        else {
            *ncp++ = *cp++;
        }
    }

    *ncp = '\0';
    return rcp;
}

/**
 **  XML print functions--these replace some functions in the
 **  h5tools.c suite.
 **/

/*-------------------------------------------------------------------------
 * Function:    xml_print_datatype
 *
 * Purpose:     Print description of a datatype in XML.
 *              Note:  this is called inside a <DataType> element.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
xml_print_datatype(hid_t type, unsigned in_group)
{
    char                   *mname;
    hid_t                   mtype;
    unsigned                nmembers;
    unsigned                ndims;
    unsigned                i;
    size_t                  size;
    hsize_t                 dims[H5DUMP_MAX_RANK];
    H5T_str_t               str_pad;
    H5T_cset_t              cset;
    hid_t                   super;
    H5T_order_t             ord;
    H5T_sign_t              sgn;
    size_t                  sz;
    size_t                  spos;
    size_t                  epos;
    size_t                  esize;
    size_t                  mpos;
    size_t                  msize;
    int                     nmembs;
    htri_t                  is_vlstr=FALSE;

    if(!in_group && H5Tcommitted(type) > 0) {
        H5O_info_t oinfo;
        obj_t  *found_obj;    /* Found object */

        /* detect a shared datatype, output only once */
        H5Oget_info(type, &oinfo);
        found_obj = search_obj(type_table, oinfo.addr);

        if(found_obj) {
            /* This should be defined somewhere else */
            /* These 2 cases are handled the same right now, but
               probably will have something different eventually */
            char * dtxid = (char *)malloc(100);

            xml_name_to_XID(found_obj->objname, dtxid, 100, 1);
            if (!found_obj->recorded) {
                /* 'anonymous' NDT.  Use it's object num.
                   as it's name.  */
                HDfprintf(stdout, "<%sNamedDataTypePtr OBJ-XID=\"/%s\"/>\n",
                        xmlnsprefix, dtxid);
            } 
            else {
                /* point to the NDT by name */
                char *t_objname = xml_escape_the_name(found_obj->objname);

                HDfprintf(stdout, "<%sNamedDataTypePtr OBJ-XID=\"%s\" H5Path=\"%s\"/>\n",
                        xmlnsprefix, dtxid, t_objname);
                free(t_objname);
            }
            free(dtxid);
        } 
        else {
            HDfprintf(stdout, "<!-- h5dump error: unknown committed type. -->\n");
            h5tools_setstatus(EXIT_FAILURE);
        }
    } 
    else {

        switch (H5Tget_class(type)) {
        case H5T_INTEGER:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            /* <hdf5:IntegerType ByteOrder="bo" Sign="torf" Size="bytes"/> */
            ord = H5Tget_order(type);
            sgn = H5Tget_sign(type);
            indentation(dump_indent);
            HDfprintf(stdout, "<%sIntegerType ByteOrder=\"",xmlnsprefix);
            switch (ord) {
            case H5T_ORDER_LE:
                HDfprintf(stdout, "LE");
                break;
            case H5T_ORDER_BE:
                HDfprintf(stdout, "BE");
                break;
            case H5T_ORDER_VAX:
            default:
                HDfprintf(stdout, "ERROR_UNKNOWN");
            }
            
            HDfprintf(stdout, "\" Sign=\"");
            
            switch (sgn) {
            case H5T_SGN_NONE:
                HDfprintf(stdout, "false");
                break;
            case H5T_SGN_2:
                HDfprintf(stdout, "true");
                break;
            default:
                HDfprintf(stdout, "ERROR_UNKNOWN");
            }
            
            HDfprintf(stdout, "\" Size=\"");
            sz = H5Tget_size(type);
            HDfprintf(stdout, "%lu", (unsigned long)sz);
            HDfprintf(stdout, "\" />\n");
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_FLOAT:
            /* <hdf5:FloatType ByteOrder="bo" Size="bytes"
               SignBitLocation="bytes"
               ExponentBits="eb" ExponentLocation="el"
               MantissaBits="mb" MantissaLocation="ml" /> */
            ord = H5Tget_order(type);
            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sFloatType ByteOrder=\"",xmlnsprefix);
            
            switch (ord) {
            case H5T_ORDER_LE:
                HDfprintf(stdout, "LE");
                break;
            case H5T_ORDER_BE:
                HDfprintf(stdout, "BE");
                break;
            case H5T_ORDER_VAX:
                HDfprintf(stdout, "VAX");
                break;
            default:
                HDfprintf(stdout, "ERROR_UNKNOWN");
            }
            
            HDfprintf(stdout, "\" Size=\"");
            sz = H5Tget_size(type);
            HDfprintf(stdout, "%lu", (unsigned long)sz);
            H5Tget_fields(type, &spos, &epos, &esize, &mpos, &msize);
            HDfprintf(stdout, "\" SignBitLocation=\"%lu\" ", (unsigned long)spos);
            HDfprintf(stdout, "ExponentBits=\"%lu\" ExponentLocation=\"%lu\" ", (unsigned long)esize, (unsigned long)epos);
            HDfprintf(stdout, "MantissaBits=\"%lu\" MantissaLocation=\"%lu\" />\n", (unsigned long)msize, (unsigned long)mpos);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_TIME:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sTimeType />\n",xmlnsprefix);
            HDfprintf(stdout, "<!-- H5T_TIME: not yet implemented -->");
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_STRING:
            /* <hdf5:StringType Cset="cs" StrSize="chars" StrPad="pad" /> */
            size = H5Tget_size(type);
            str_pad = H5Tget_strpad(type);
            cset = H5Tget_cset(type);
            is_vlstr = H5Tis_variable_str(type);

            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sStringType Cset=\"",xmlnsprefix);
            if (cset == H5T_CSET_ASCII) {
                HDfprintf(stdout, "H5T_CSET_ASCII\" ");
            } 
            else {
                HDfprintf(stdout, "unknown_cset\" ");
            }
            if(is_vlstr)
                HDfprintf(stdout, "StrSize=\"H5T_VARIABLE\" StrPad=\"");
            else
                HDfprintf(stdout, "StrSize=\"%d\" StrPad=\"", (int) size);
            if (str_pad == H5T_STR_NULLTERM) {
                HDfprintf(stdout, "H5T_STR_NULLTERM\"/>\n");
            } 
            else if (str_pad == H5T_STR_NULLPAD) {
                HDfprintf(stdout, "H5T_STR_NULLPAD\"/>\n");
            } 
            else if (str_pad == H5T_STR_SPACEPAD) {
                HDfprintf(stdout, "H5T_STR_SPACEPAD\"/>\n");
            } 
            else {
                HDfprintf(stdout, "H5T_STR_ERROR\"/>\n");
            }
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_BITFIELD:
            /* <hdf5:BitfieldType ByteOrder="bo" Size="bytes"/> */
            ord = H5Tget_order(type);
            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sBitfieldType ByteOrder=\"",xmlnsprefix);
            
            switch (ord) {
            case H5T_ORDER_LE:
                HDfprintf(stdout, "LE");
                break;
            case H5T_ORDER_BE:
                HDfprintf(stdout, "BE");
                break;
            case H5T_ORDER_VAX:
            default:
                HDfprintf(stdout, "ERROR_UNKNOWN");
            }
            
            size = H5Tget_size(type);
            HDfprintf(stdout, "\" Size=\"%lu\"/>\n", (unsigned long)size);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_OPAQUE:
            /* <hdf5:OpaqueType Tag="tag" Size="bytes" /> */
            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            indentation(dump_indent);
            mname = H5Tget_tag(type);
            HDfprintf(stdout, "<%sOpaqueType Tag=\"%s\" ",xmlnsprefix, mname);
            free(mname);
            size = H5Tget_size(type);
            HDfprintf(stdout, "Size=\"%lu\"/>\n", (unsigned long)size);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_COMPOUND:
            /* recursively describe the components of a compound datatype */

            /* type of a dataset */
            nmembers = H5Tget_nmembers(type);

            indentation(dump_indent);
            HDfprintf(stdout, "<%sCompoundType>\n",xmlnsprefix);

            /* List each member Field of the type */
            /*   <hdf5:Field FieldName="name" > */
            /*   <hdf5:DataType > */
            dump_indent += COL;
            for (i = 0; i < nmembers; i++) {
                char *t_fname;

                mname = H5Tget_member_name(type, i);
                mtype = H5Tget_member_type(type, i);
                indentation(dump_indent);
                t_fname = xml_escape_the_name(mname);
                HDfprintf(stdout, "<%sField FieldName=\"%s\">\n",xmlnsprefix, t_fname);

                free(mname);
                free(t_fname);
                dump_indent += COL;
                indentation(dump_indent);
                HDfprintf(stdout, "<%sDataType>\n",xmlnsprefix);
                dump_indent += COL;
                xml_print_datatype(mtype,0);
                dump_indent -= COL;
                indentation(dump_indent);
                HDfprintf(stdout, "</%sDataType>\n",xmlnsprefix);
                dump_indent -= COL;

                indentation(dump_indent);
                HDfprintf(stdout, "</%sField>\n",xmlnsprefix);
            }
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sCompoundType>\n",xmlnsprefix);
            break;

        case H5T_REFERENCE:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            indentation(dump_indent);
            /*  Only Object references supported at this time */
            HDfprintf(stdout, "<%sReferenceType>\n",xmlnsprefix);
            indentation(dump_indent + COL);
            HDfprintf(stdout, "<%sObjectReferenceType />\n",xmlnsprefix);
            indentation(dump_indent);
            HDfprintf(stdout, "</%sReferenceType>\n",xmlnsprefix);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_ENUM:
            /*  <hdf5:EnumType Nelems="ne" > list Name, values of enum */
            nmembs = H5Tget_nmembers(type);
            indentation(dump_indent);
            HDfprintf(stdout, "<%sAtomicType>\n",xmlnsprefix);
            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sEnumType Nelems=\"%d\">\n",xmlnsprefix, nmembs);
            xml_print_enum(type);
            indentation(dump_indent);
            HDfprintf(stdout, "</%sEnumType>\n",xmlnsprefix);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sAtomicType>\n",xmlnsprefix);
            break;

        case H5T_VLEN:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sVLType>\n",xmlnsprefix);
            super = H5Tget_super(type);
            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sDataType>\n",xmlnsprefix);
            dump_indent += COL;
            xml_print_datatype(super,0);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sDataType>\n",xmlnsprefix);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sVLType>\n",xmlnsprefix);
            H5Tclose(super);

            break;

        case H5T_ARRAY:
            /* Get array base type */
            super = H5Tget_super(type);

            /* Print lead-in */
            indentation(dump_indent);
            HDfprintf(stdout, "<%sArrayType Ndims=\"",xmlnsprefix);
            ndims = H5Tget_array_ndims(type);
            HDfprintf(stdout, "%u\">\n", ndims);

            /* Get array information */
            H5Tget_array_dims2(type, dims);

            /* list of dimensions */
            dump_indent += COL;
            for (i = 0; i < ndims; i++) {
                indentation(dump_indent);
                HDfprintf(stdout, "<%sArrayDimension DimSize=\"%u\"/>\n", xmlnsprefix, (int) dims[i]);
            }
            dump_indent -= COL;

            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sDataType>\n",xmlnsprefix);
            dump_indent += COL;
            xml_print_datatype(super,0);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sDataType>\n",xmlnsprefix);
            dump_indent -= COL;
            indentation(dump_indent);
            HDfprintf(stdout, "</%sArrayType>\n",xmlnsprefix);
            /* Close array base type */
            H5Tclose(super);
            break;

        default:
            HDfprintf(stdout, "<!-- unknown datatype -->");
            h5tools_setstatus(EXIT_FAILURE);
            break;
        }
    } /* end else */
}

/*-------------------------------------------------------------------------
 * Function:    xml_dump_datatype
 *
 * Purpose:     Dump description of a datatype in XML.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
xml_dump_datatype(hid_t type)
{
    dump_indent += COL;
    indentation(dump_indent);

    if(H5Tcommitted(type) > 0) {
        H5O_info_t oinfo;
        obj_t  *found_obj;    /* Found object */

        /* Datatype is a shared or named datatype */
        H5Oget_info(type, &oinfo);
        found_obj = search_obj(type_table, oinfo.addr);

        if(found_obj) {
            /* Shared datatype, must be entered as an object  */
            /* These 2 cases are the same now, but may change */
            char *dtxid = (char *)malloc(100);

            xml_name_to_XID(found_obj->objname, dtxid, 100, 1);
            if (!found_obj->recorded) {
                /* anonymous stored datatype:
                   following the dumper's current
                   practice:
                   use it's object ref as its name
                 */
                HDfprintf(stdout, "<%sNamedDataTypePtr OBJ-XID=\"%s\"/>\n",
                        xmlnsprefix, dtxid);
            } 
            else {
                /* pointer to a named datatype already in XML */
                char *t_objname = xml_escape_the_name(found_obj->objname);

                HDfprintf(stdout, "<%sNamedDataTypePtr OBJ-XID=\"%s\" H5Path=\"%s\" />\n",
                        xmlnsprefix, dtxid, t_objname);
                free(t_objname);
            }
            free(dtxid);
        } 
        else {
            HDfprintf(stdout, "<!-- h5dump error: unknown committed type. -->\n");
        }
        dump_indent -= COL;
    }
    else {
        HDfprintf(stdout, "<%sDataType>\n", xmlnsprefix);
        dump_indent += COL;
        xml_print_datatype(type, 0);
        dump_indent -= COL;
        indentation(dump_indent);
        HDfprintf(stdout, "</%sDataType>\n", xmlnsprefix);
        dump_indent -= COL;
    }
}

/*-------------------------------------------------------------------------
 * Function:    xml_dump_dataspace
 *
 * Purpose:     Dump description of a dataspace in XML.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
xml_dump_dataspace(hid_t space)
{
    hsize_t         size[H5DUMP_MAX_RANK];
    hsize_t         maxsize[H5DUMP_MAX_RANK];
    int             i;
    
    int             ndims = H5Sget_simple_extent_dims(space, size, maxsize);
    H5S_class_t     space_type = H5Sget_simple_extent_type(space);

    indentation(dump_indent + COL);
    HDfprintf(stdout, "<%sDataspace>\n", xmlnsprefix);
    indentation(dump_indent + COL + COL);

    switch (space_type) {
    case H5S_SCALAR:
        /* scalar dataspace (just a tag, no XML attrs. defined */
        HDfprintf(stdout, "<%sScalarDataspace />\n",xmlnsprefix);
        break;

    case H5S_SIMPLE:
        /* simple dataspace */
        /* <hdf5:SimpleDataspace Ndims="nd"> */
        HDfprintf(stdout, "<%sSimpleDataspace Ndims=\"%d\">\n",xmlnsprefix, ndims);

        /* print the <hdf5:Dimension> elements */
        for (i = 0; i < ndims; i++) {
            indentation(dump_indent + COL + COL + COL);
            if (maxsize[i] == H5S_UNLIMITED) {
                HDfprintf(stdout, "<%sDimension  DimSize=\"%Hu\" MaxDimSize=\"UNLIMITED\"/>\n",
                        xmlnsprefix,size[i]);
            } 
            else if (maxsize[i] == (hsize_t) 0) {
                HDfprintf(stdout, "<%sDimension  DimSize=\"%Hu\" MaxDimSize=\"%Hu\"/>\n",
                        xmlnsprefix,size[i], size[i]);
            } 
            else {
                HDfprintf(stdout, "<%sDimension  DimSize=\"%Hu\" MaxDimSize=\"%Hu\"/>\n",
                        xmlnsprefix, size[i], maxsize[i]);
            }
        }
        indentation(dump_indent + COL + COL);
        HDfprintf(stdout, "</%sSimpleDataspace>\n", xmlnsprefix );
        break;

#ifdef TMP
        /* Commented out: wait until the schema is updated first */
    case H5S_NULL:
        /* null dataspace (just a tag, no XML attrs. defined */
        HDfprintf(stdout, "<%sNullDataspace />\n",xmlnsprefix);
        break;
#endif /* TMP */

    case H5S_NO_CLASS:
    default:
        HDfprintf(stdout, "<!-- unknown dataspace -->\n");
    }

    indentation(dump_indent + COL);
    HDfprintf(stdout, "</%sDataspace>\n", xmlnsprefix);
}

/*-------------------------------------------------------------------------
 * Function:    xml_dump_data
 *
 * Purpose:     Dump description of data in XML.
 *              Note that this calls the h5dump_xxx calls in
 *              the h5tools library.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
xml_dump_data(hid_t obj_id, int obj_data, struct subset_t UNUSED * sset, int UNUSED pindex)
{
    hid_t               space;
    hid_t               type;
    hid_t               p_type;
    hsize_t             size[64];
    hsize_t             nelmts = 1;
    int                 ndims;
    int                 i;
    int                 depth;
    int                 status = -1;
    int                 stdindent = COL;    /* should be 3 */
    h5tool_format_t    *outputformat = &xml_dataformat;
    void               *buf = NULL;
    h5tools_context_t ctx;            /* print context  */


    HDmemset(&ctx, 0, sizeof(ctx));
    ctx.indent_level = dump_indent/COL;

    if (fp_format) {
        outputformat->fmt_double = fp_format;
        outputformat->fmt_float = fp_format;
    }

    if (nCols==0) {
        outputformat->line_ncols = 65535;
        outputformat->line_per_line = 1;
    }
    else
        outputformat->line_ncols = nCols;
    dump_indent += COL;
    ctx.indent_level++;
    
    /* Print all the values. */
    indentation(dump_indent);
    HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
    indentation(dump_indent + COL);
    HDfprintf(stdout, "<%sDataFromFile>\n",xmlnsprefix);
    if (obj_data == DATASET_DATA) {
        type = H5Dget_type(obj_id);
        if (H5Tget_class(type) == H5T_REFERENCE) {
            status = xml_print_refs(obj_id, DATASET_DATA);
        } 
        else if (H5Tget_class(type) == H5T_STRING) {
            status = xml_print_strs(obj_id, DATASET_DATA);
        } 
        else {
            status = h5tools_dump_dset(stdout, outputformat, &ctx, obj_id, -1, NULL);
        }
    } 
    else {
        /* Attribute data */
        type = H5Aget_type(obj_id);

        if (H5Tget_class(type) == H5T_REFERENCE) {
            /* references are done differently than
               the standard output:
               XML dumps a path to the object
               referenced.
             */
            status = xml_print_refs(obj_id, ATTRIBUTE_DATA);
            H5Tclose(type);
        } 
        else if (H5Tget_class(type) == H5T_STRING) {
            status = xml_print_strs(obj_id, ATTRIBUTE_DATA);
        } 
        else {  /* all other data */
            /* VL data special information */
            unsigned int vl_data = 0; /* contains VL datatypes */
            
            p_type = h5tools_get_native_type(type);

            /* Check if we have VL data in the dataset's datatype */
            if (h5tools_detect_vlen_str(p_type) == TRUE)
                vl_data = TRUE;
            if (H5Tdetect_class(p_type, H5T_VLEN) == TRUE)
                vl_data = TRUE;

            H5Tclose(type);

            space = H5Aget_space(obj_id);

            ndims = H5Sget_simple_extent_dims(space, size, NULL);

            for (i = 0; i < ndims; i++)
                nelmts *= size[i];

            buf = malloc((size_t)(nelmts * MAX(H5Tget_size(type), H5Tget_size(p_type))));
            assert(buf);

            if (H5Aread(obj_id, p_type, buf) >= 0)
                status = h5tools_dump_mem(stdout, outputformat, &ctx, obj_id, p_type, space, buf);

            /* Reclaim any VL memory, if necessary */
            if (vl_data)
                H5Dvlen_reclaim(p_type, space, H5P_DEFAULT, buf);

            free(buf);
            H5Tclose(p_type);
            H5Sclose(space);
            H5Tclose(type);
        }
    }

    if (status == FAIL) {
        indentation(dump_indent + COL);
        HDfprintf(stdout, "Unable to print data.\n");
        status = 1;
    }

    indentation(dump_indent + COL);
    HDfprintf(stdout, "</%sDataFromFile>\n",xmlnsprefix);
    indentation(dump_indent);
    HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
    dump_indent -= COL;
}

/*-------------------------------------------------------------------------
 * Function:    xml_dump_attr
 *
 * Purpose:     Dump a description of an HDF5 attribute in XML.
 *
 * Return:      herr_t
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
xml_dump_attr(hid_t attr, const char *attr_name, const H5A_info_t UNUSED *info,
    void UNUSED * op_data)
{
    hid_t       attr_id;
    hid_t       type;
    hid_t       space;
    H5S_class_t space_type;
    
    char *t_aname = xml_escape_the_name(attr_name);

    indentation(dump_indent);
    HDfprintf(stdout, "<%sAttribute Name=\"%s\">\n", xmlnsprefix, t_aname);
    free(t_aname);

    if ((attr_id = H5Aopen(attr, attr_name, H5P_DEFAULT)) >= 0) {
        type = H5Aget_type(attr_id);
        space = H5Aget_space(attr_id);
        space_type = H5Sget_simple_extent_type(space);

        dump_function_table->dump_dataspace_function(space);
        dump_function_table->dump_datatype_function(type);

        if (display_attr_data && space_type != H5S_NULL) {
            switch (H5Tget_class(type)) {
            case H5T_INTEGER:
            case H5T_FLOAT:
            case H5T_STRING:
            case H5T_BITFIELD:
            case H5T_OPAQUE:
            case H5T_ENUM:
            case H5T_ARRAY:
                dump_function_table->dump_data_function(attr_id, ATTRIBUTE_DATA, NULL, 0);
                break;

            case H5T_TIME:
                dump_indent += COL;
                indentation(dump_indent);
                HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
                indentation(dump_indent);
                HDfprintf(stdout, "<!-- Time data not yet implemented. -->\n");
                indentation(dump_indent);
                HDfprintf(stdout, "<%sNoData/>\n", xmlnsprefix);
                indentation(dump_indent);
                HDfprintf(stdout, "<hdf5:Data>\n");
                HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
                dump_indent -= COL;
                break;

            case H5T_COMPOUND:
                indentation(dump_indent);
                HDfprintf(stdout, "<!-- Note: format of compound data not specified -->\n");
                dump_function_table->dump_data_function(attr_id, ATTRIBUTE_DATA, NULL, 0);
                break;

            case H5T_REFERENCE:
                indentation(dump_indent);
                HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
                indentation(dump_indent);
                if (!H5Tequal(type, H5T_STD_REF_OBJ)) {
                    HDfprintf(stdout, "<!-- Note: Region references not supported -->\n");
                    indentation(dump_indent);
                    HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
                }
                else {
                    HDfprintf(stdout, "<%sDataFromFile>\n", xmlnsprefix);
                    xml_print_refs(attr_id, ATTRIBUTE_DATA);
                    indentation(dump_indent);
                    HDfprintf(stdout, "</%sDataFromFile>\n", xmlnsprefix);
                }
                indentation(dump_indent);
                HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
                break;

            case H5T_VLEN:
                HDfprintf(stdout, "<!-- Note: format of VL data not specified -->\n");
                dump_function_table->dump_data_function(attr_id, ATTRIBUTE_DATA, NULL, 0);
                break;
            default:
                indentation(dump_indent);
                HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
                indentation(dump_indent);
                HDfprintf(stdout, "<!-- Unknown datatype: %d -->\n", H5Tget_class(type));
                indentation(dump_indent);
                HDfprintf(stdout, "<%sNoData/>\n", xmlnsprefix);
                indentation(dump_indent);
                HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
                break;
            }
        }
        else {
            /* The case of an attribute never yet written ??
             * Or dataspace is H5S_NULL. */
            indentation(dump_indent + COL);
            HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
            indentation(dump_indent + COL + COL);
            HDfprintf(stdout, "<%sNoData/>\n", xmlnsprefix);
            indentation(dump_indent + COL);
            HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
        }

        H5Tclose(type);
        H5Sclose(space);
        H5Aclose(attr_id);
        indentation(dump_indent);
        HDfprintf(stdout, "</%sAttribute>\n", xmlnsprefix);
        return SUCCEED;

    }
    else {
        /* ?? failed */
        indentation(dump_indent + COL);
        HDfprintf(stdout, "<!-- h5dump error: unable to open attribute. -->\n");
        indentation(dump_indent);
        HDfprintf(stdout, "</%sAttribute>\n", xmlnsprefix);
        h5tools_setstatus(EXIT_FAILURE);
        return FAIL;
    }
}

/*-------------------------------------------------------------------------
 * Function:    xml_dump_named_datatype
 *
 * Purpose:     Dump a description of an HDF5 NDT in XML.
 *
 * Return:      herr_t
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
xml_dump_named_datatype(hid_t type, const char *name)
{
    char    *tmp;
    char    *dtxid;
    char    *parentxid;
    char    *t_tmp;
    char    *t_prefix;
    char    *t_name;

    tmp = (char *)HDmalloc(HDstrlen(prefix) + HDstrlen(name) + 2);
    HDstrcpy(tmp, prefix);
    HDstrcat(tmp, "/");
    HDstrcat(tmp, name);

    indentation(dump_indent);
    dtxid = (char *)HDmalloc(100);
    parentxid = (char *)HDmalloc(100);
    t_tmp = xml_escape_the_name(tmp);
    t_prefix = xml_escape_the_name(prefix);
    t_name = xml_escape_the_name(name);

    xml_name_to_XID(tmp, dtxid, 100, 1);
    xml_name_to_XID(prefix, parentxid, 100, 1);
    if(HDstrncmp(name, "#", 1) == 0) {
        /*  Special:  this is an 'anonymous' NDT, deleted but
           still in use.
           We follow the dumper's undocumented practice, and
           use its object id as its name.
           Exactly the same as normal, but a separate case
           in the event we want to do something else in
           the future.
         */
        HDfprintf(stdout, "<%sNamedDataType Name=\"%s\" OBJ-XID=\"%s\" "
        "Parents=\"%s\" H5ParentPaths=\"%s\">\n",
        xmlnsprefix,
        name, dtxid,
        parentxid, HDstrcmp(prefix,"") ? t_prefix : "/");
    } 
    else {
        H5O_info_t  oinfo;          /* Object info */

        HDfprintf(stdout, "<%sNamedDataType Name=\"%s\" OBJ-XID=\"%s\" "
        "H5Path=\"%s\" Parents=\"%s\" H5ParentPaths=\"%s\">\n",
        xmlnsprefix,
        t_name, dtxid,
        t_tmp, parentxid, (HDstrcmp(prefix, "") ? t_prefix : "/"));

        /* Check uniqueness of named datatype */
        H5Oget_info(type, &oinfo);
        if(oinfo.rc > 1) {
            obj_t       *found_obj;     /* Found object */

            /* Group with more than one link to it... */
            found_obj = search_obj(type_table, oinfo.addr);

            if (found_obj == NULL) {
                indentation(dump_indent);
                error_msg("internal error (file %s:line %d)\n", __FILE__, __LINE__);
                h5tools_setstatus(EXIT_FAILURE);
                goto done;
            } 
            else if(found_obj->displayed) {
                /* We have already printed this named datatype, print it as a
                 * NamedDatatypePtr
                 */
                char pointerxid[100];
                char *t_objname = xml_escape_the_name(found_obj->objname);

                indentation(dump_indent + COL);
                xml_name_to_XID(found_obj->objname, pointerxid, sizeof(pointerxid), 1);
                HDfprintf(stdout, "<%sNamedDatatypePtr OBJ-XID=\"%s\" H5Path=\"%s\"/>\n", xmlnsprefix, pointerxid, t_objname);
                indentation(dump_indent);
                HDfprintf(stdout, "</%sNamedDataType>\n", xmlnsprefix);
                HDfree(t_objname);
                goto done;
            } 
            else
                found_obj->displayed = TRUE;
        }
    }

    dump_indent += COL;
    indentation(dump_indent);
    HDfprintf(stdout, "<%sDataType>\n",xmlnsprefix);

    dump_indent += COL;
    xml_print_datatype(type,1);

    dump_indent -= COL;
    indentation(dump_indent);
    HDfprintf(stdout, "</%sDataType>\n",xmlnsprefix);

    dump_indent -= COL;
    indentation(dump_indent);
    HDfprintf(stdout, "</%sNamedDataType>\n",xmlnsprefix);

done:
    HDfree(dtxid);
    HDfree(parentxid);
    HDfree(t_tmp);
    HDfree(t_prefix);
    HDfree(t_name);
    HDfree(tmp);
}

/*-------------------------------------------------------------------------
 * Function:    xml_dump_group
 *
 * Purpose:     Dump a description of an HDF5 Group (and its members) in XML.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *  Pedro Vicente, October 9, 2007
 *   added parameters to H5A(L)iterate to allow for other iteration orders
 *
 *-------------------------------------------------------------------------
 */
void
xml_dump_group(hid_t gid, const char *name)
{
    H5O_info_t              oinfo;
    hid_t                   gcpl_id;
    hid_t                   dset, type;
    unsigned                crt_order_flags;
    unsigned                attr_crt_order_flags;
    int                     isRoot = 0;
    char                    type_name[1024];
    char                   *t_objname = NULL;
    char                   *par_name = NULL;
    char                   *cp = NULL;
    char                   *tmp = NULL;
    char                   *par = NULL;


    if ((gcpl_id = H5Gget_create_plist(gid)) < 0) {
        error_msg("error in getting group creation property list ID\n");
        h5tools_setstatus(EXIT_FAILURE);
    }

    /* query the group creation properties for attributes */
    if (H5Pget_attr_creation_order(gcpl_id, &attr_crt_order_flags) < 0) {
        error_msg("error in getting group creation properties\n");
        h5tools_setstatus(EXIT_FAILURE);
    }

    /* query the group creation properties */
    if(H5Pget_link_creation_order(gcpl_id, &crt_order_flags) < 0) {
        error_msg("error in getting group creation properties\n");
        h5tools_setstatus(EXIT_FAILURE);
    }

    if(H5Pclose(gcpl_id) < 0) {
        error_msg("error in closing group creation property list ID\n");
        h5tools_setstatus(EXIT_FAILURE);
    }

    if(HDstrcmp(name, "/") == 0) {
        isRoot = 1;
        tmp = HDstrdup("/");
    }
    else {
        tmp = (char *)HDmalloc(HDstrlen(prefix) + HDstrlen(name) + 2);
        HDstrcpy(tmp, prefix);
        par = HDstrdup(tmp);
        cp = HDstrrchr(par, '/');
        if(cp) {
            if((cp == par) && HDstrlen(par) > 1)
                *(cp + 1) = '\0';
            else
                *cp = '\0';
        }
    }

    H5Oget_info(gid, &oinfo);

    if(oinfo.rc > 1) {
        obj_t *found_obj;    /* Found object */

        /* Group with more than one link to it... */
        found_obj = search_obj(group_table, oinfo.addr);

        if (found_obj == NULL) {
            indentation(dump_indent);
            error_msg("internal error (file %s:line %d)\n", __FILE__, __LINE__);
            h5tools_setstatus(EXIT_FAILURE);
        }
        else {
            char *t_name = xml_escape_the_name(name);
            char *grpxid = (char *)malloc(100);
            char *parentxid = (char *)malloc(100);

            if(found_obj->displayed) {
                char *ptrstr = (char *)malloc(100);

                /* already seen: enter a groupptr */
                if(isRoot) {
                    /* probably can't happen! */
                    xml_name_to_XID("/", grpxid, 100, 1);
                    HDfprintf(stdout, "<%sRootGroup OBJ-XID=\"%s\" H5Path=\"%s\">\n",
                            xmlnsprefix, grpxid, "/");
                }
                else {
                    t_objname = xml_escape_the_name(found_obj->objname);
                    par_name = xml_escape_the_name(par);
                    xml_name_to_XID(tmp, grpxid, 100, 1);
                    xml_name_to_XID(par, parentxid, 100, 1);
                    HDfprintf(stdout, "<%sGroup Name=\"%s\" OBJ-XID=\"%s-%d\" H5Path=\"%s\" "
                            "Parents=\"%s\" H5ParentPaths=\"%s\">\n",
                            xmlnsprefix,t_name, grpxid, get_next_xid(),
                            t_objname, parentxid, par_name);
                    free(t_objname);
                    free(par_name);

                    indentation(dump_indent + COL);
                    t_objname = xml_escape_the_name(found_obj->objname);/* point to the NDT by name */
                    par_name = xml_escape_the_name(par);
                    xml_name_to_XID(found_obj->objname, ptrstr, 100, 1);
                    xml_name_to_XID(par, parentxid, 100, 1);
                    HDfprintf(stdout, "<%sGroupPtr OBJ-XID=\"%s\" H5Path=\"%s\" "
                                "Parents=\"%s\" H5ParentPaths=\"%s\" />\n",
                                xmlnsprefix,
                                ptrstr, t_objname, parentxid, par_name);
                    free(t_objname);
                    free(par_name);
                }
                free(ptrstr);
            }
            else {

                /* first time this group has been seen -- describe it  */
                if(isRoot) {
                    xml_name_to_XID("/", grpxid, 100, 1);
                    HDfprintf(stdout, "<%sRootGroup OBJ-XID=\"%s\" H5Path=\"%s\">\n",
                            xmlnsprefix, grpxid, "/");
                }
                else {
                    char *t_tmp = xml_escape_the_name(tmp);

                    par_name = xml_escape_the_name(par);
                    xml_name_to_XID(tmp, grpxid, 100, 1);
                    xml_name_to_XID(par, parentxid, 100, 1);
                    HDfprintf(stdout, "<%sGroup Name=\"%s\" OBJ-XID=\"%s\" H5Path=\"%s\" "
                            "Parents=\"%s\" H5ParentPaths=\"%s\" >\n",
                            xmlnsprefix,t_name, grpxid, t_tmp, parentxid, par_name);
                    free(t_tmp);
                    free(par_name);
                }
                found_obj->displayed = TRUE;

                /* 1.  do all the attributes of the group */

                if((sort_by == H5_INDEX_CRT_ORDER) && (attr_crt_order_flags & H5P_CRT_ORDER_TRACKED)) {
                    if(H5Aiterate2(gid, sort_by, sort_order, NULL, dump_function_table->dump_attribute_function, NULL) < 0) {
                        error_msg("error getting attribute information\n");
                        h5tools_setstatus(EXIT_FAILURE);
                    } /* end if */
                } /* end if */
                else {
                    if(H5Aiterate2(gid, H5_INDEX_NAME, sort_order, NULL, dump_function_table->dump_attribute_function, NULL) < 0) {
                        error_msg("error getting attribute information\n");
                        h5tools_setstatus(EXIT_FAILURE);
                    } /* end if */
                } /* end else */

                if(isRoot && unamedtype) {
                    unsigned u;

                    /* Very special case: dump unamed type in root group */
                    for(u = 0; u < type_table->nobjs; u++) {
                        if(!type_table->objs[u].recorded) {
                            dset = H5Dopen2(gid, type_table->objs[u].objname, H5P_DEFAULT);
                            type = H5Dget_type(dset);
                            sprintf(type_name, "#"H5_PRINTF_HADDR_FMT, type_table->objs[u].objno);
                            dump_function_table->dump_named_datatype_function(type, type_name);
                            H5Tclose(type);
                            H5Dclose(dset);
                        }
                    }
                }

                /* iterate through all the links */

                if((sort_by == H5_INDEX_CRT_ORDER) && (crt_order_flags & H5P_CRT_ORDER_TRACKED))
                    H5Literate(gid, sort_by, sort_order, NULL, xml_dump_all_cb, NULL);
                else
                    H5Literate(gid, H5_INDEX_NAME, sort_order, NULL, xml_dump_all_cb, NULL);


            }
            free(t_name);
            free(grpxid);
            free(parentxid);
        }
    }
    else {

        /* only link -- must be first time! */
        char *t_name = xml_escape_the_name(name);
        char *grpxid = (char *)malloc(100);
        char *parentxid = (char *)malloc(100);

        if(isRoot) {
            xml_name_to_XID("/", grpxid, 100, 1);
            HDfprintf(stdout, "<%sRootGroup OBJ-XID=\"%s\" H5Path=\"%s\">\n", xmlnsprefix, grpxid, "/");
        }
        else {
            char *t_tmp = xml_escape_the_name(tmp);

            par_name = xml_escape_the_name(par);
            xml_name_to_XID(tmp, grpxid, 100, 1);
            xml_name_to_XID(par, parentxid, 100, 1);
            HDfprintf(stdout, "<%sGroup Name=\"%s\" OBJ-XID=\"%s\" H5Path=\"%s\" "
                    "Parents=\"%s\" H5ParentPaths=\"%s\" >\n",
                    xmlnsprefix, t_name, grpxid, t_tmp, parentxid, par_name);
            free(t_tmp);
            free(par_name);
        }
        free(t_name);
        free(grpxid);
        free(parentxid);

        /* 1.  do all the attributes of the group */

        if((sort_by == H5_INDEX_CRT_ORDER) && (attr_crt_order_flags & H5P_CRT_ORDER_TRACKED)) {
            if(H5Aiterate2(gid, sort_by, sort_order, NULL, dump_function_table->dump_attribute_function, NULL) < 0) {
                error_msg("error getting attribute information\n");
                h5tools_setstatus(EXIT_FAILURE);
            } /* end if */
        } /* end if */
        else {
            if(H5Aiterate2(gid, H5_INDEX_NAME, sort_order, NULL, dump_function_table->dump_attribute_function, NULL) < 0) {
                error_msg("error getting attribute information\n");
                h5tools_setstatus(EXIT_FAILURE);
            } /* end if */
        } /* end else */


        if(isRoot && unamedtype) {
            unsigned u;

            /* Very special case: dump unamed type in root group */
            for(u = 0; u < type_table->nobjs; u++) {
                if(!type_table->objs[u].recorded) {
                    dset = H5Dopen2(gid, type_table->objs[u].objname, H5P_DEFAULT);
                    type = H5Dget_type(dset);
                    sprintf(type_name, "#"H5_PRINTF_HADDR_FMT, type_table->objs[u].objno);
                    dump_function_table->dump_named_datatype_function(type, type_name);
                    H5Tclose(type);
                    H5Dclose(dset);
                }
            }
        }

        /* iterate through all the links */

        if((sort_by == H5_INDEX_CRT_ORDER) && (crt_order_flags & H5P_CRT_ORDER_TRACKED))
            H5Literate(gid, sort_by, sort_order, NULL, xml_dump_all_cb, NULL);
        else
            H5Literate(gid, H5_INDEX_NAME, sort_order, NULL, xml_dump_all_cb, NULL);
    }

    if(isRoot)
        HDfprintf(stdout, "</%sRootGroup>\n", xmlnsprefix);
    else
        HDfprintf(stdout, "</%sGroup>\n", xmlnsprefix);
    if(par)
        free(par);
    if(tmp)
        free(tmp);
}

/*-------------------------------------------------------------------------
 * Function:    xml_print_refs
 *
 * Purpose:     Print a path to the objects referenced by HDF5 Referneces.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
xml_print_refs(hid_t did, int source)
{
    herr_t      e;
    hid_t       type;
    hid_t       space;
    hssize_t    ssiz;
    hsize_t     i;
    size_t      tsiz;
    hobj_ref_t *refbuf = NULL;
    char       *buf = NULL;

    if (source == DATASET_DATA) {
        type = H5Dget_type(did);
    }
    else if (source == ATTRIBUTE_DATA) {
        type = H5Aget_type(did);
    }
    else {
        /* return an error */
        return FAIL;
    }
    if (H5Tget_class(type) != H5T_REFERENCE) {
        /* return an error */
        goto error;
    }
    if (!H5Tequal(type, H5T_STD_REF_OBJ)) {
        /* region ref not supported yet... */
        /* return an error */
        goto error;
    }
    if (source == DATASET_DATA) {
        space = H5Dget_space(did);
        if ((ssiz = H5Sget_simple_extent_npoints(space)) < 0)
            goto error;
        if ((tsiz = H5Tget_size(type)) == 0)
            goto error;

        buf = (char *) calloc((size_t)(ssiz * tsiz), sizeof(char));
        if (buf == NULL)
            goto error;
        e = H5Dread(did, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
        /* need to check result here */
        if (e < 0) {
            goto error;
        }
    }
    else if (source == ATTRIBUTE_DATA) {
        space = H5Aget_space(did);
        if ((ssiz = H5Sget_simple_extent_npoints(space)) < 0)
            goto error;
        if ((tsiz = H5Tget_size(type)) == 0)
            goto error;

        buf = (char *) calloc((size_t)(ssiz * tsiz), sizeof(char));
        if (buf == NULL) {
            goto error;
        }
        e = H5Aread(did, H5T_STD_REF_OBJ, buf);
        /* need to check the result here */
        if (e < 0) {
            goto error;
        }
    }

    refbuf = (hobj_ref_t *) buf;

    for (i = 0; i < ssiz; i++) {
        const char *path = lookup_ref_path(*refbuf);
        indentation(dump_indent + COL);

        if (!path) {
            HDfprintf(stdout, "\"%s\"\n", "NULL");
        }
        else {
            char *t_path = xml_escape_the_string(path, -1);

            HDfprintf(stdout, "\"%s\"\n", t_path);
            free(t_path);
        }

        refbuf++;
    }

    free(buf);
    H5Tclose(type);
    H5Sclose(space);
    return SUCCEED;
    
error:
    if(buf)
        free(buf);

    H5E_BEGIN_TRY {
        H5Tclose(type);
        H5Sclose(space);
    } H5E_END_TRY;
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:    xml_print_strs
 *
 * Purpose:     Print strings.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
xml_print_strs(hid_t did, int source)
{
    herr_t      e;
    hid_t       type;
    hid_t       space;
    hssize_t    ssiz;
    htri_t      is_vlstr = FALSE;
    size_t      tsiz;
    size_t      i;
    size_t      str_size = 0;
    char       *bp = NULL;
    char       *onestring = NULL;
    void       *buf = NULL;

    if (source == DATASET_DATA) {
        type = H5Dget_type(did);
    }
    else if (source == ATTRIBUTE_DATA) {
        type = H5Aget_type(did);
    }
    else {
        /* return an error */
        return FAIL;
    }
    if (H5Tget_class(type) != H5T_STRING) {
        /* return an error */
        goto error;
    }
    /* Check if we have VL data in the dataset's datatype */
    is_vlstr = H5Tis_variable_str(type);

    if (source == DATASET_DATA) {
        space = H5Dget_space(did);
        if((ssiz = H5Sget_simple_extent_npoints(space)) < 0)
            goto error;
        if((tsiz = H5Tget_size(type)) == 0)
            goto error;

        buf = malloc((size_t)(ssiz * tsiz));
        if (buf == NULL)
            goto error;

        e = H5Dread(did, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
        if (e < 0) {
            goto error;
        }
    }
    else if (source == ATTRIBUTE_DATA) {
        space = H5Aget_space(did);
        if((ssiz = H5Sget_simple_extent_npoints(space)) < 0)
            goto error;
        if((tsiz = H5Tget_size(type)) == 0)
            goto error;

        buf = malloc((size_t)(ssiz * tsiz));
        if (buf == NULL)
            goto error;

        e = H5Aread(did, type, buf);
        if (e < 0) {
            goto error;
        }
    }

    bp = (char*) buf;
    if (!is_vlstr)
        onestring = (char *) calloc(tsiz, sizeof(char));

    for (i = 0; i < ssiz; i++) {
        if (is_vlstr) {
            onestring = *(char **) bp;
            if (onestring)
                str_size = (size_t) HDstrlen(onestring);
        }
        else {
            HDstrncpy(onestring, bp, tsiz);
            str_size = tsiz;
        }
        indentation(dump_indent + COL);

        if (!onestring) {
            HDfprintf(stdout, "NULL\n");
        }
        else {
            char *t_onestring = xml_escape_the_string(onestring, (int) str_size);
            if (t_onestring) {
                HDfprintf(stdout, "\"%s\"\n", t_onestring);
                free(t_onestring);
            }
        }

        bp += tsiz;
    }

    /* Reclaim any VL memory, if necessary */
    if (!is_vlstr)
        if (onestring)
            free(onestring);
    if (buf) {
        if (is_vlstr)
            H5Dvlen_reclaim(type, space, H5P_DEFAULT, buf);
        free(buf);
    }
    H5Tclose(type);
    H5Sclose(space);
    return SUCCEED;
    
error:
    if(buf)
        free(buf);

    H5E_BEGIN_TRY {
        H5Tclose(type);
        H5Sclose(space);
    } H5E_END_TRY;
    return FAIL;
}

/*-------------------------------------------------------------------------
 * Function:    check_filters
 *
 * Purpose:     private function to check for the filters and
 *              put tags in the XML.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
check_filters(hid_t dcpl)
{
    int             nfilt;
    int             i;
    H5Z_filter_t    filter;
    char            namebuf[120];
    size_t          cd_nelmts = 20;
    unsigned int    cd_values[20];
    unsigned int    flags;

    nfilt = H5Pget_nfilters(dcpl);
    if (nfilt <= 0)
        return;
    for (i = 0; i < nfilt; i++) {
        filter = H5Pget_filter2(dcpl, (unsigned) i, &flags, (size_t *) &cd_nelmts, cd_values, 120, namebuf, NULL);
        if (filter == H5Z_FILTER_DEFLATE) {
            indentation(dump_indent + COL);
            HDfprintf(stdout, "<%sDeflate Level=\"", xmlnsprefix);
            if (cd_nelmts < 1) {
                /* not sure what this means? */
                HDfprintf(stdout, "6");
            }
            else {
                HDfprintf(stdout, "%d", cd_values[0]);
            }
            HDfprintf(stdout, "\"/>\n");
        }
        else if (filter == H5Z_FILTER_FLETCHER32) {
            indentation(dump_indent + COL);
            HDfprintf(stdout, "<%sFletcher32 />", xmlnsprefix);
        }
        else if (filter == H5Z_FILTER_SHUFFLE) {
            indentation(dump_indent + COL);
            HDfprintf(stdout, "<%sShuffle />", xmlnsprefix);
        }
        else if (filter == H5Z_FILTER_SZIP) {

            indentation(dump_indent + COL);
            HDfprintf(stdout, "<%sSZIP ", xmlnsprefix);
            if (cd_nelmts < 2) {
                /* no pixels ? */
                HDfprintf(stdout, "Pixels_per_block=\"-1\" ");
            }
            else {
                HDfprintf(stdout, "Pixels_per_block=\"%d\" ", cd_values[1]);
            }
            /* analyse the options mask */
            if (cd_values[0] & H5_SZIP_CHIP_OPTION_MASK) {
                HDfprintf(stdout, "Mode =\"Hardware\" ");
            }
            else if (cd_values[0] & H5_SZIP_ALLOW_K13_OPTION_MASK) {
                HDfprintf(stdout, "Mode =\"K13\" ");
            }
            HDfprintf(stdout, "Coding=\"");
            if (cd_values[0] & H5_SZIP_EC_OPTION_MASK) {
                HDfprintf(stdout, "Entropy");
            }
            else if (cd_values[0] & H5_SZIP_NN_OPTION_MASK) {
                HDfprintf(stdout, "NN");
            }
            HDfprintf(stdout, "\" ");

            HDfprintf(stdout, "ByteOrder=\"");
            if (cd_values[0] & H5_SZIP_LSB_OPTION_MASK) {
                HDfprintf(stdout, "LSB");
            }
            else if (cd_values[0] & H5_SZIP_MSB_OPTION_MASK) {
                HDfprintf(stdout, "MSB");
            }
            HDfprintf(stdout, "\" ");

            if (cd_values[0] & H5_SZIP_RAW_OPTION_MASK) {
                HDfprintf(stdout, "Header=\"Raw\"");
            }
            HDfprintf(stdout, "/>\n");
        }
        else {
            /* unknown option */
        }
    }
}

static void
xml_dump_fill_value(hid_t dcpl, hid_t type)
{
    size_t      sz;
    size_t      i;
    hsize_t     space;
    void       *buf;
    char       *name;

    dump_indent += COL;
    indentation(dump_indent);
    HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
    dump_indent += COL;

    space = H5Tget_size(type);
    buf = malloc((size_t) space);

    H5Pget_fill_value(dcpl, type, buf);

    if (H5Tget_class(type) == H5T_REFERENCE) {
        const char * path = lookup_ref_path(*(hobj_ref_t *) buf);

        indentation(dump_indent);
        HDfprintf(stdout, "<%sDataFromFile>\n", xmlnsprefix);
        if (!path) {
            HDfprintf(stdout, "\"%s\"\n", "NULL");
        }
        else {
            char *t_path = xml_escape_the_string(path, -1);

            HDfprintf(stdout, "\"%s\"\n", t_path);
            free(t_path);
        }
        indentation(dump_indent);
        HDfprintf(stdout, "</%sDataFromFile>\n", xmlnsprefix);
    }
    else if (H5Tget_class(type) == H5T_STRING) {
        /* ????? */
        indentation(dump_indent);
        HDfprintf(stdout, "<!-- String fill values not yet implemented. -->\n");
        indentation(dump_indent);
        HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
    }
    else {
        /* all other data */
        switch (H5Tget_class(type)) {
        case H5T_INTEGER:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sDataFromFile>\n", xmlnsprefix);
            indentation(dump_indent);
            HDfprintf(stdout, "\"%d\"\n", *(int *) buf);
            indentation(dump_indent);
            HDfprintf(stdout, "</%sDataFromFile>\n", xmlnsprefix);
            break;
        case H5T_FLOAT:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sDataFromFile>\n", xmlnsprefix);
            indentation(dump_indent);
            HDfprintf(stdout, "\"%f\"\n", *(float *) buf);
            indentation(dump_indent);
            HDfprintf(stdout, "</%sDataFromFile>\n", xmlnsprefix);
            break;
        case H5T_BITFIELD:
        case H5T_OPAQUE:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sDataFromFile>\n", xmlnsprefix);
            sz = H5Tget_size(type);
            indentation(dump_indent);
            HDfprintf(stdout, "\"");
            for (i = 0; i < sz; i++) {
                HDfprintf(stdout, "%x ", *(unsigned int *) buf);
                buf = (char *) buf + sizeof(unsigned int);
            }
            HDfprintf(stdout, "\"\n");
            indentation(dump_indent);
            HDfprintf(stdout, "</%sDataFromFile>\n", xmlnsprefix);
            break;
        case H5T_ENUM:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sDataFromFile>\n", xmlnsprefix);
            name = H5Tget_member_name(type, *(unsigned *) buf);
            indentation(dump_indent);
            HDfprintf(stdout, "\"%s\"\n", name);
            indentation(dump_indent);
            HDfprintf(stdout, "</%sDataFromFile>\n", xmlnsprefix);
            break;
        case H5T_ARRAY:
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- Array fill values not yet implemented. -->\n");
            indentation(dump_indent);
            HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
            break;
        case H5T_TIME:
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- Time fill not yet implemented. -->\n");
            indentation(dump_indent);
            HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
            break;
        case H5T_COMPOUND:
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- Compound fill not yet implemented. -->\n");
            indentation(dump_indent);
            HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
            break;
        case H5T_VLEN:
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- VL fill not yet implemented. -->\n");
            indentation(dump_indent);
            HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
            break;
        default:
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- Unknown fill datatype: %d -->\n", H5Tget_class(type));
            indentation(dump_indent);
            HDfprintf(stdout, "<%sNoData/>\n", xmlnsprefix);
            break;
        }
    }
    free(buf);
    dump_indent -= COL;
    indentation(dump_indent);
    HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
    dump_indent -= COL;
}

/*-------------------------------------------------------------------------
 * Function:    xml_dump_dataset
 *
 * Purpose:     Dump a description of an HDF5 dataset in XML.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *  Pedro Vicente, October 9, 2007
 *   added parameters to H5Aiterate2 to allow for other iteration orders
 *
 *-------------------------------------------------------------------------
 */
void
xml_dump_dataset(hid_t did, const char *name, struct subset_t UNUSED * sset)
{
    hid_t               type;
    hid_t               space;
    hid_t               dcpl;
    H5D_fill_value_t    fvstatus;
    int                 maxdims;
    hsize_t            *chsize;
    int                 ndims;
    int                 i;
    H5D_alloc_time_t    at;
    H5D_fill_time_t     ft;
    hsize_t             tempi;
    char               *tmp;
    char               *t_name;
    char               *t_tmp;
    char               *t_prefix;
    unsigned            attr_crt_order_flags;
    
    char *rstr = (char*) HDmalloc(100);
    char *pstr = (char*) HDmalloc(100);

    tmp = (char*) HDmalloc(HDstrlen(prefix) + HDstrlen(name) + 2);
    HDstrcpy(tmp, prefix);
    HDstrcat(tmp, "/");
    HDstrcat(tmp, name);
    indentation(dump_indent);

    t_name = xml_escape_the_name(name);
    t_tmp = xml_escape_the_name(tmp);
    t_prefix = xml_escape_the_name(prefix);

    xml_name_to_XID(tmp, rstr, 100, 1);
    xml_name_to_XID(prefix, pstr, 100, 1);
    HDfprintf(stdout, "<%sDataset Name=\"%s\" OBJ-XID=\"%s\" H5Path= \"%s\" Parents=\"%s\" H5ParentPaths=\"%s\">\n",
            xmlnsprefix, t_name, rstr, t_tmp, pstr,
            strcmp(prefix, "") ? t_prefix : "/");

    HDfree(t_name);
    HDfree(t_tmp);
    HDfree(t_prefix);
    HDfree(rstr);
    HDfree(pstr);
    HDfree(tmp);

    dcpl = H5Dget_create_plist(did);
    type = H5Dget_type(did);
    space = H5Dget_space(did);

    /* query the creation properties for attributes */
    H5Pget_attr_creation_order(dcpl, &attr_crt_order_flags);

    /* Print information about storage layout */
    if (H5D_CHUNKED == H5Pget_layout(dcpl)) {
        maxdims = H5Sget_simple_extent_ndims(space);
        chsize = (hsize_t *) malloc(maxdims * sizeof(hsize_t));
        dump_indent += COL;
        indentation(dump_indent);
        HDfprintf(stdout, "<%sStorageLayout>\n", xmlnsprefix);
        dump_indent += COL;
        indentation(dump_indent);
        HDfprintf(stdout, "<%sChunkedLayout ", xmlnsprefix);
        ndims = H5Pget_chunk(dcpl, maxdims, chsize);
        HDfprintf(stdout, "Ndims=\"%d\">\n", ndims);

        dump_indent += COL;

        for (i = 0; i < ndims; i++) {
            indentation(dump_indent);
            HDfprintf(stdout, "<%sChunkDimension DimSize=\"%Hu\" />\n", xmlnsprefix, chsize[i]);
        }

        indentation(dump_indent);
        HDfprintf(stdout, "<%sRequiredFilter>\n", xmlnsprefix);
        dump_indent += COL;
        check_filters(dcpl);
        dump_indent -= COL;
        indentation(dump_indent);
        HDfprintf(stdout, "</%sRequiredFilter>\n", xmlnsprefix);

        dump_indent -= COL;

        indentation(dump_indent);
        HDfprintf(stdout, "</%sChunkedLayout>\n", xmlnsprefix);
        dump_indent -= COL;
        indentation(dump_indent);
        HDfprintf(stdout, "</%sStorageLayout>\n", xmlnsprefix);
        dump_indent -= COL;
        free(chsize);
    }
    else if (H5D_CONTIGUOUS == H5Pget_layout(dcpl)) {
        dump_indent += COL;
        indentation(dump_indent);
        HDfprintf(stdout, "<%sStorageLayout>\n", xmlnsprefix);
        dump_indent += COL;
        indentation(dump_indent);
        HDfprintf(stdout, "<%sContiguousLayout/>\n", xmlnsprefix);
        dump_indent -= COL;
        indentation(dump_indent);
        HDfprintf(stdout, "</%sStorageLayout>\n", xmlnsprefix);
        dump_indent -= COL;
        indentation(dump_indent);
    }
    else if (H5D_COMPACT == H5Pget_layout(dcpl)) {
        dump_indent += COL;
        indentation(dump_indent);
        HDfprintf(stdout, "<%sStorageLayout>\n", xmlnsprefix);
        dump_indent += COL;
        indentation(dump_indent);
        HDfprintf(stdout, "<%sCompactLayout/>\n", xmlnsprefix);
        dump_indent -= COL;
        indentation(dump_indent);
        HDfprintf(stdout, "</%sStorageLayout>\n", xmlnsprefix);
        dump_indent -= COL;
        indentation(dump_indent);
    }
    /* and check for external.... ?? */

    /* fill value */

    dump_indent += COL;
    indentation(dump_indent);
    HDfprintf(stdout, "<%sFillValueInfo ", xmlnsprefix);
    H5Pget_fill_time(dcpl, &ft);
    HDfprintf(stdout, "FillTime=\"");
    switch (ft) {
    case H5D_FILL_TIME_ALLOC:
        HDfprintf(stdout, "FillOnAlloc");
        break;
    case H5D_FILL_TIME_NEVER:
        HDfprintf(stdout, "FillNever");
        break;
    case H5D_FILL_TIME_IFSET:
        HDfprintf(stdout, "FillIfSet");
        break;
    default:
        HDfprintf(stdout, "?");
        break;
    }
    HDfprintf(stdout, "\" ");
    H5Pget_alloc_time(dcpl, &at);
    HDfprintf(stdout, "AllocationTime=\"");
    switch (at) {
    case H5D_ALLOC_TIME_EARLY:
        HDfprintf(stdout, "Early");
        break;
    case H5D_ALLOC_TIME_INCR:
        HDfprintf(stdout, "Incremental");
        break;
    case H5D_ALLOC_TIME_LATE:
        HDfprintf(stdout, "Late");
        break;
    case H5D_ALLOC_TIME_DEFAULT:
    default:
        HDfprintf(stdout, "?");
        break;
    }
    HDfprintf(stdout, "\"");
    HDfprintf(stdout, ">\n");

    dump_indent += COL;
    indentation(dump_indent);
    HDfprintf(stdout, "<%sFillValue>\n", xmlnsprefix);
    dump_indent += COL;
    H5Pfill_value_defined(dcpl, &fvstatus);
    if (fvstatus == H5D_FILL_VALUE_UNDEFINED || 
        (fvstatus == H5D_FILL_VALUE_DEFAULT && ft == H5D_FILL_TIME_IFSET)) {
        indentation(dump_indent + COL);
        HDfprintf(stdout, "<%sNoFill/>\n", xmlnsprefix);
    }
    else {
        xml_dump_fill_value(dcpl, type);
    }

    dump_indent -= COL;
    indentation(dump_indent);
    HDfprintf(stdout, "</%sFillValue>\n", xmlnsprefix);

    dump_indent -= COL;
    indentation(dump_indent);
    HDfprintf(stdout, "</%sFillValueInfo>\n", xmlnsprefix);
    dump_indent -= COL;

    dump_function_table->dump_dataspace_function(space);
    dump_function_table->dump_datatype_function(type);

    dump_indent += COL;

    if ((sort_by == H5_INDEX_CRT_ORDER) && (attr_crt_order_flags & H5P_CRT_ORDER_TRACKED)) {
        if (H5Aiterate2(did, sort_by, sort_order, NULL, dump_function_table->dump_attribute_function, NULL) < 0) {
            error_msg("error getting attribute information\n");
            h5tools_setstatus(EXIT_FAILURE);
        } /* end if */
    } /* end if */
    else {
        if (H5Aiterate2(did, H5_INDEX_NAME, sort_order, NULL, dump_function_table->dump_attribute_function, NULL) < 0) {
            error_msg("error getting attribute information\n");
            h5tools_setstatus(EXIT_FAILURE);
        } /* end if */
    } /* end else */

    dump_indent -= COL;
    tempi = H5Dget_storage_size(did);

    if (display_data && (tempi > 0)) {
        switch (H5Tget_class(type)) {
        case H5T_INTEGER:
        case H5T_FLOAT:
        case H5T_STRING:
        case H5T_BITFIELD:
        case H5T_OPAQUE:
        case H5T_ENUM:
        case H5T_ARRAY:
            dump_function_table->dump_data_function(did, DATASET_DATA, NULL, 0);
            break;

        case H5T_TIME:
            dump_indent += COL;
            indentation(dump_indent);
            HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- Time data not yet implemented. -->\n");
            indentation(dump_indent);
            HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
            indentation(dump_indent);
            HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
            dump_indent -= COL;
            break;

        case H5T_COMPOUND:
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- Note: format of compound data not specified -->\n");
            dump_function_table->dump_data_function(did, DATASET_DATA, NULL, 0);
            break;

        case H5T_REFERENCE:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
            indentation(dump_indent);
            if (!H5Tequal(type, H5T_STD_REF_OBJ)) {
                HDfprintf(stdout, "<!-- Note: Region references not supported -->\n");
                indentation(dump_indent);
                HDfprintf(stdout, "<%sNoData />\n", xmlnsprefix);
            }
            else {
                HDfprintf(stdout, "<%sDataFromFile>\n", xmlnsprefix);
                xml_print_refs(did, DATASET_DATA);
                indentation(dump_indent);
                HDfprintf(stdout, "</%sDataFromFile>\n", xmlnsprefix);
            }
            indentation(dump_indent);
            HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
            break;

        case H5T_VLEN:
            HDfprintf(stdout, "<!-- Note: format of VL data not specified -->\n");
            dump_function_table->dump_data_function(did, DATASET_DATA, NULL, 0);
            break;
        default:
            indentation(dump_indent);
            HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
            indentation(dump_indent);
            HDfprintf(stdout, "<!-- Unknown datatype: %d -->\n", H5Tget_class(type));
            indentation(dump_indent);
            HDfprintf(stdout, "<%sNoData/>\n", xmlnsprefix);
            indentation(dump_indent);
            HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
            break;
        }
    }
    else {
        /* no data written */
        indentation(dump_indent + COL);
        HDfprintf(stdout, "<%sData>\n", xmlnsprefix);
        indentation(dump_indent + COL + COL);
        HDfprintf(stdout, "<%sNoData/>\n", xmlnsprefix);
        indentation(dump_indent + COL);
        HDfprintf(stdout, "</%sData>\n", xmlnsprefix);
    }

    H5Tclose(type);
    H5Sclose(space);
    H5Pclose(dcpl);
    indentation(dump_indent);
    HDfprintf(stdout, "</%sDataset>\n", xmlnsprefix);
}

/*-------------------------------------------------------------------------
 * Function:    xml_print_enum
 *
 * Purpose:     Print the values of an HDF5 ENUM in XML.
 *              Very similar to regular DDL output.
 *
 * Return:      void
 *
 * Programmer:  REMcG
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
xml_print_enum(hid_t type)
{
    char              **name = NULL;    /*member names                   */
    unsigned char      *value = NULL;   /*value array                    */
    unsigned            nmembs;         /*number of members              */
    hid_t               super;          /*enum base integer type         */
    hid_t               native = -1;    /*native integer datatype        */
    size_t              dst_size;       /*destination value type size    */
    unsigned            i;              /*miscellaneous counters         */
    size_t              j;

    nmembs = (unsigned)H5Tget_nmembers(type);
    super = H5Tget_super(type);

    indentation(dump_indent);
    HDfprintf(stdout, "<%sDataType>\n",xmlnsprefix);
    xml_print_datatype(super,0);
    indentation(dump_indent);
    HDfprintf(stdout, "</%sDataType>\n",xmlnsprefix);

    /*
     * Determine what datatype to use for the native values.  To simplify
     * things we entertain three possibilities:
     *  1. long long -- the largest native signed integer
     *    2. unsigned long long -- the largest native unsigned integer
     *    3. raw format
     */
    if (H5Tget_size(type) <= sizeof(long long)) {
        dst_size = sizeof(long long);

        if (H5T_SGN_NONE == H5Tget_sign(type)) {
            native = H5T_NATIVE_ULLONG;
        } 
        else {
            native = H5T_NATIVE_LLONG;
        }
    } 
    else {
        dst_size = H5Tget_size(type);
    }

    /* Get the names and raw values of all members */
    name = (char **)calloc(nmembs, sizeof(char *));
    value = (unsigned char *)calloc(nmembs, MAX(H5Tget_size(type), dst_size));

    for (i = 0; i < nmembs; i++) {
        name[i] = H5Tget_member_name(type, i);
        H5Tget_member_value(type, i, value + i * H5Tget_size(type));
    }

    /* Convert values to native datatype */
    if (native > 0)
        H5Tconvert(super, native, nmembs, value, NULL, H5P_DEFAULT);

    /* Sort members by increasing value */
    /*not implemented yet */

    /* Print members */
    dump_indent += COL;
    for (i = 0; i < nmembs; i++) {
        char *t_name = xml_escape_the_name(name[i]);

        indentation(dump_indent);
        HDfprintf(stdout, "<%sEnumElement>\n",xmlnsprefix);
        indentation(dump_indent + COL);
        HDfprintf(stdout, "%s\n", t_name);
        free(t_name);
        indentation(dump_indent);
        HDfprintf(stdout, "</%sEnumElement>\n",xmlnsprefix);
        indentation(dump_indent);
        HDfprintf(stdout, "<%sEnumValue>\n",xmlnsprefix);
        indentation(dump_indent + COL);
        if (native < 0) {
            HDfprintf(stdout, "0x");

            for (j = 0; j < dst_size; j++)
                HDfprintf(stdout, "%02x", value[i * dst_size + j]);
        } 
        else if (H5T_SGN_NONE == H5Tget_sign(native)) {
            HDfprintf(stdout,"%" H5_PRINTF_LL_WIDTH "u", *((unsigned long long *)
                       ((void *) (value + i * dst_size))));
        } 
        else {
            HDfprintf(stdout,"%" H5_PRINTF_LL_WIDTH "d",
                      *((long long *) ((void *) (value + i * dst_size))));
        }
        HDfprintf(stdout, "\n");
        indentation(dump_indent);
        HDfprintf(stdout, "</%sEnumValue>\n",xmlnsprefix);

    }
    dump_indent -= COL;

    /* Release resources */
    for (i = 0; i < nmembs; i++)
        free(name[i]);

    free(name);
    free(value);
    H5Tclose(super);
}

