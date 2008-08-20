! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
!   Copyright by The HDF Group.                                               *
!   Copyright by the Board of Trustees of the University of Illinois.         *
!   All rights reserved.                                                      *
!                                                                             *
!   This file is part of HDF5.  The full HDF5 copyright notice, including     *
!   terms governing use, modification, and redistribution, is contained in    *
!   the files COPYING and Copyright.html.  COPYING can be found at the root   *
!   of the source code distribution tree; Copyright.html can be found at the  *
!   root level of an installed copy of the electronic HDF5 document set and   *
!   is linked from the top-level documents page.  It can also be found at     *
!   http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
!   access to either file, you may request a copy from help@hdfgroup.org.     *
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
!
!
! FUNCTION
!   This module is included for when the default REAL is not of the type DOUBLE PRECISION.
!   We do not include the double precision interfaces if the defaut REAL is
!   DOUBLE PRECISION since this would lead to a non-unique conflict with the
!   generic interfaces declared as REAL. Otherwise it is okay to include the interfaces.
!   
!
MODULE H5_DBLE_INTERFACE

  USE H5A_PROVISIONAL
  USE H5D_PROVISIONAL
  USE H5P_PROVISIONAL

!
! ----- H5A ----
!
  INTERFACE h5awrite_f
     MODULE PROCEDURE h5awrite_double_scalar
     MODULE PROCEDURE h5awrite_double_1
     MODULE PROCEDURE h5awrite_double_2
     MODULE PROCEDURE h5awrite_double_3
     MODULE PROCEDURE h5awrite_double_4
     MODULE PROCEDURE h5awrite_double_5
     MODULE PROCEDURE h5awrite_double_6
     MODULE PROCEDURE h5awrite_double_7
  END INTERFACE

  INTERFACE h5aread_f
     MODULE PROCEDURE h5aread_double_scalar
     MODULE PROCEDURE h5aread_double_1
     MODULE PROCEDURE h5aread_double_2
     MODULE PROCEDURE h5aread_double_3
     MODULE PROCEDURE h5aread_double_4
     MODULE PROCEDURE h5aread_double_5
     MODULE PROCEDURE h5aread_double_6
     MODULE PROCEDURE h5aread_double_7
  END INTERFACE
!
! ----- H5D ----
!
  INTERFACE h5dwrite_f
     MODULE PROCEDURE h5dwrite_double_scalar
     MODULE PROCEDURE h5dwrite_double_1
     MODULE PROCEDURE h5dwrite_double_2
     MODULE PROCEDURE h5dwrite_double_3
     MODULE PROCEDURE h5dwrite_double_4
     MODULE PROCEDURE h5dwrite_double_5
     MODULE PROCEDURE h5dwrite_double_6
     MODULE PROCEDURE h5dwrite_double_7
  END INTERFACE

  INTERFACE h5dread_f
     MODULE PROCEDURE h5dread_double_scalar
     MODULE PROCEDURE h5dread_double_1
     MODULE PROCEDURE h5dread_double_2
     MODULE PROCEDURE h5dread_double_3
     MODULE PROCEDURE h5dread_double_4
     MODULE PROCEDURE h5dread_double_5
     MODULE PROCEDURE h5dread_double_6
     MODULE PROCEDURE h5dread_double_7
  END INTERFACE

  INTERFACE h5dfill_f
     MODULE PROCEDURE h5dfill_double
  END INTERFACE

!
! ----- H5P ----
!
  INTERFACE h5pset_fill_value_f
     MODULE PROCEDURE h5pset_fill_value_double
  END INTERFACE
     
  INTERFACE h5pget_fill_value_f
     MODULE PROCEDURE h5pget_fill_value_double
  END INTERFACE

  INTERFACE h5pset_f
     MODULE PROCEDURE h5pset_double
  END INTERFACE

  INTERFACE h5pget_f
     MODULE PROCEDURE h5pget_double
  END INTERFACE

  INTERFACE h5pregister_f
     MODULE PROCEDURE h5pregister_double
  END INTERFACE

  INTERFACE h5pinsert_f
     MODULE PROCEDURE h5pinsert_double
  END INTERFACE

END MODULE H5_DBLE_INTERFACE
