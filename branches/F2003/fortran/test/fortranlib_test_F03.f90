!****h* root/fortran/test/fortranlib_test_F03.f90
!
! NAME
!  fortranlib_test_F03.f90
!
! FUNCTION
!  Basic testing of Fortran API's that require Fortran 2003
!  compliance.
!
! COPYRIGHT
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
!*****

PROGRAM fortranlibtest
  
  USE HDF5
  
  IMPLICIT NONE
  INTEGER :: total_error = 0
  INTEGER :: error
  INTEGER :: majnum, minnum, relnum
  LOGICAL :: szip_flag
  INTEGER :: ret_total_error
  LOGICAL :: cleanup, status

  CALL h5open_f(error)

  cleanup = .TRUE.
  CALL h5_env_nocleanup_f(status)
  IF(status) cleanup=.FALSE.

  WRITE(*,*) '                       ==============================                            '
  WRITE(*,*) '                              FORTRAN 2003 tests '
  WRITE(*,*) '                       ==============================                            '
  CALL h5get_libversion_f(majnum, minnum, relnum, total_error)
  IF(total_error .EQ. 0) THEN

     WRITE(*, '(" FORTRANLIB_TEST is linked with HDF5 Library version ")', advance="NO")
     WRITE(*, '(I1)', advance="NO") majnum
     WRITE(*, '(".")', advance="NO") 
     WRITE(*, '(I1)', advance="NO") minnum
     WRITE(*, '(" release ")', advance="NO")
     WRITE(*, '(I3)') relnum
  ELSE
     total_error = total_error + 1
  ENDIF
  WRITE(*,*)
!     write(*,*)
!     write(*,*) '========================================='
!     write(*,*) 'Testing DATATYPE interface               '
!     write(*,*) '========================================='
  ret_total_error = 0
  CALL test_array_compound_atomic(ret_total_error)
  CALL write_test_status(ret_total_error, ' Testing 1-D Array of Compound Datatypes Functionality', total_error)

  ret_total_error = 0
  CALL test_array_compound_array(ret_total_error)
  CALL write_test_status(ret_total_error, ' Testing 1-D Array of Compound Array Datatypes Functionality', total_error)

  ret_total_error = 0
  CALL test_create(ret_total_error)
  CALL write_test_status(ret_total_error, &
       ' Testing filling functions', &
       total_error)
!DEC$ if defined(H5_VMS)
  GOTO 8
!DEC$ else
  ret_total_error = 0
  CALL file_close(cleanup, ret_total_error)
  CALL write_test_status(ret_total_error, ' File open/close test', total_error)
!DEC$ endif
8 CONTINUE


  ret_total_error = 0
  CALL test_array_bkg(ret_total_error)
  CALL write_test_status(ret_total_error, ' Testing Partial I/O of Array Fields in Compound Datatype FunctionalityT', total_error)

  ret_total_error = 0
  CALL test_genprop_class_callback(ret_total_error)
  CALL write_test_status(ret_total_error, ' Test basic generic property list callback functionality', total_error)

  ret_total_error = 0
!  CALL test_iter_group(ret_total_error)
  CALL write_test_status(ret_total_error, ' Testing Group Iteration Functionality', total_error)
 

!     write(*,*)
!     write(*,*) '========================================='
!     write(*,*) 'Testing GROUP interface             '
!     write(*,*) '========================================='

  
  WRITE(*,*)

  WRITE(*,*) '                  ============================================  '
  WRITE(*, fmt = '(19x, 27a)', advance='NO') ' FORTRAN tests completed with '
  WRITE(*, fmt = '(i4)', advance='NO') total_error
  WRITE(*, fmt = '(12a)' ) ' error(s) ! '
  WRITE(*,*) '                  ============================================  '
  
  CALL h5close_f(error)

  ! if errors detected, exit with non-zero code.
  IF (total_error .NE. 0) CALL h5_exit_f (1)

END PROGRAM fortranlibtest


