#! /bin/sh
#
# Copyright (C) 1997-2001 National Center for Supercomputing Applications.
#                         All rights reserved.
#

H5TOH4=h5toh4               # a relative name
H5TOH4_BIN=`pwd`/$H5TOH4    # an absolute command path

cmp='cmp -s'
diff='diff -c'

RM='rm -f'
SED='sed '
H4DUMP='hdp'

# Verify if $H4DUMP is a valid command.
tmpfile=/tmp/testh5toh4.$$
$H4DUMP -H > $tmpfile
if test -s "$tmpfile"; then
    # Find out which version of hdp is being used.  Over simplified
    # algorithm but will do the job for now.
    if ( grep -s 'NCSA HDF Version 4.1 Release [3-9]' $tmpfile > /dev/null )
    then
	H4DUMPVER=413
    else
	H4DUMPVER=0
	echo "    Some tests maybe skipped because your version of $H4DUMP does"
	echo "    not handle loops in Vgroups correctly. You need version"
	echo "    4.1 Release 3 or later. Visit http://hdf.ncsa.uiuc.edu"
	echo "    or email hdfhelp@ncsa.uiuc.edu for more information."
    fi
else
    echo "    Could not run the '$H4DUMP' command. The test can still proceed"
    echo "    but it may fail if '$H4DUMP' is needed to verify the output."
    echo "    You can make sure '$H4DUMP' is among your shell PATH and run"
    echo "    the test again. You may also visit http://hdf.ncsa.uiuc.edu"
    echo "    or email hdfhelp@ncsa.uiuc.edu for more information."
    H4DUMP=:
    H4DUMPVER=0
fi
$RM $tmpfile

# The build (current) directory might be different than the source directory.
if test -z "$srcdir"; then
   srcdir=.
fi
mkdir ../testfiles >/dev/null 2>&1

SRCDIR="$srcdir/../testfiles"
OUTDIR="../testfiles/Results"

test -d $OUTDIR || mkdir $OUTDIR

nerrors=0
verbose=yes

# Print a line-line message left justified in a field of 70 characters
# beginning with the word "Testing".
TESTING() {
    SPACES="                                                               "
    echo "Testing $* $SPACES" |cut -c1-70 |tr -d '\012'
}

# Run a test and print PASS or *FAIL*.  If a test fails then increment
# the `nerrors' global variable and (if $verbose is set) display the
# difference between the actual and the expected hdf4 files. The
# expected hdf4 files are in testfiles/Expected directory.
# The actual hdf4 file is not removed if $HDF5_NOCLEANUP is to a non-null
# value.
CONVERT() {
    # Run h5toh4 convert.
    TESTING $H5TOH4 $@

    #
    # Set up arguments to run the conversion test.
    # The converter assumes all hdf5 files has the .h5 suffix as in the form
    # of foo.h5.  It creates the corresponding hdf4 files with the .hdf suffix
    # as in the form of foo.hdf.  One exception is that if exactly two file
    # names are given, it treats the first argument as an hdf5 file and creates
    # the corresponding hdf4 file with the name as the second argument, WITOUT
    # any consideration of the suffix.  (For this test script, in order to
    # match the output hdf4 file with the expected hdf4 file, it expects the
    # second file of the two-files tests has the .hdf suffix too.)
    #
    # If SRCDIR != OUTDIR, need to copy the input hdf5 files from the SRCDIR
    # to the OUTDIR and transform the input file pathname because of the suffix
    # convention mentioned above.  This way, the hdf4 files are always created
    # in the OUTDIR directory.
    #

    INFILES=""
    OUTFILES=""
    MULTIRUN=""

    case "$1" in
    "-m")		# multiple files conversion
	MULTIRUN="-m"
	shift
	for f in $*
	do
	    if test "$SRCDIR" != "$OUTDIR"; then
		cp $SRCDIR/$f $OUTDIR/$f
	    fi
	    INFILES="$INFILES $f"
	    OUTFILES="$OUTFILES `basename $f .h5`.hdf"
	    shift
	done
	;;
    * )			# Single file conversion
	case $# in
	1)  if test "$SRCDIR" != "$OUTDIR"; then
		cp $SRCDIR/$1 $OUTDIR/$1
	    fi
	    INFILES="$1"
	    OUTFILES="`basename $1 .h5`.hdf"
	    ;;
	2) 		# hdf4 file specified
	    if test "$SRCDIR" != "$OUTDIR"; then
		cp $SRCDIR/$1 $OUTDIR/$1
	    fi
	    INFILES="$1"
	    OUTFILES="$2"
	    ;;
	*)		# Illegal
	    echo "Illegal arguments"
	    exit 1
	    ;;
	esac
	;;
    esac

    # run the conversion and remove input files that have been copied over
    (
	cd $OUTDIR
	$H5TOH4_BIN $MULTIRUN $INFILES 2>/dev/null
	if test "$SRCDIR" != "$OUTDIR"; then
	    $RM $INFILES
	fi
    )

    # Verify results
    result="passed"
    for f in $OUTFILES
    do
	if $cmp $SRCDIR/Expected/$f $OUTDIR/$f
	then
	    :
	else
	    # Use hdp to dump the files and verify the output.
	    # Filter out the output of "reference = ..." because
	    # reference numbers are immaterial in general.
	    outfile=`basename $f .hdf`
	    expect_out=$outfile.expect
	    actual_out=$outfile.actual

	    if [ $outfile = "tloop" -a $H4DUMPVER -lt 413 ]
	    then
		echo " -SKIP-"
		result="skipped"
		touch $expect_out $actual_out	# fake them
	    else
		(cd $SRCDIR/Expected
		$H4DUMP dumpvg $outfile.hdf
		$H4DUMP dumpvd $outfile.hdf
		$H4DUMP dumpsds $outfile.hdf ) |
		sed -e 's/reference = [0-9]*;//' > $expect_out
		(cd $OUTDIR
		$H4DUMP dumpvg $outfile.hdf
		$H4DUMP dumpvd $outfile.hdf
		$H4DUMP dumpsds $outfile.hdf ) |
		sed -e 's/reference = [0-9]*;//' > $actual_out
	    fi

	    if [ "passed" = $result -a ! -s $actual_out ] ; then
		echo "*FAILED*"
		nerrors="`expr $nerrors + 1`"
		result=failed
		test yes = "$verbose" &&
		echo "    H4DUMP failed to produce valid output"
	    elif $cmp $expect_out $actual_out; then
		:
	    else
		if test "passed" = $result; then
		    echo "*FAILED*"
		    nerrors="`expr $nerrors + 1`"
		    result=failed
		fi
		test yes = "$verbose" &&
		echo "    Actual result (*.actual) differs from expected result (*.expect)" &&
		$diff $expect_out $actual_out |sed 's/^/    /'
	    fi
	fi

	# Clean up output file
	if test -z "$HDF5_NOCLEANUP"; then
	    $RM $expect_out $actual_out
	    $RM $OUTDIR/$f
	fi
    done
    if test "passed" = "$result"; then
	    echo " PASSED"
    fi
}



##############################################################################
##############################################################################
###			  T H E   T E S T S                                ###
##############################################################################
##############################################################################

$RM $OUTDIR/*.hdf $OUTDIR/*.tmp

#
# The HDF4 filenames are created based upon the HDF5 filenames
# without the extension.
#

# test for converting H5 groups to H4 Vgroups.
CONVERT tgroup.h5

# test for converting H5 datasets to H4 SDS's.
CONVERT tdset.h5

# test for converting H5 attributes to H4 attributes.
CONVERT tattr.h5

# test for converting H5 soft links.
CONVERT tslink.h5

# test for converting H5 hard links.
CONVERT thlink.h5

# test for converting H5 compound data type to H4 Vdata.
CONVERT tcompound.h5

# test for converting all H5 objects at in same file.
CONVERT tall.h5

# tests for converting H5 objects with loops.
CONVERT tloop.h5

# test for converting extendable H5 datasets to H4 SDS's.
CONVERT tdset2.h5

# test for converting extendable H5 datasets with compound data type to H4 Vdata.
CONVERT tcompound2.h5

# tests for converting H5 objects from many different pathways.
CONVERT tmany.h5

# tests for converting H5 string objects.
CONVERT tstr.h5

# tests for converting more H5 string objects.
CONVERT tstr2.h5

#
# The test for conversion are the same as above with the only difference
# being that the HDF4 filenames are given explicitly.
#

$RM $OUTDIR/*.tmp
CONVERT tgroup.h5 tgroup.hdf
CONVERT tdset.h5 tdset.hdf
CONVERT tattr.h5 tattr.hdf
CONVERT tslink.h5 tslink.hdf
CONVERT thlink.h5 thlink.hdf
CONVERT tcompound.h5 tcompound.hdf
CONVERT tall.h5 tall.hdf
CONVERT tloop.h5 tloop.hdf
CONVERT tdset2.h5 tdset2.hdf
CONVERT tcompound2.h5 tcompound2.hdf
CONVERT tmany.h5 tmany.hdf
CONVERT tstr.h5 tstr.hdf
CONVERT tstr2.h5 tstr2.hdf

#
# Again, the test for conversion are the same as the first set of test.
# Here, multiple conversion are done on HDF5 files at one time.
#

$RM $OUTDIR/*.hdf $OUTDIR/*.tmp
CONVERT -m tgroup.h5 tdset.h5 tattr.h5 tslink.h5 thlink.h5
CONVERT -m tcompound.h5 tall.h5
CONVERT -m tloop.h5
CONVERT -m tdset2.h5 tcompound2.h5 tmany.h5
CONVERT -m tstr.h5 tstr2.h5

if test $nerrors -eq 0 ; then
	echo "All h5toh4 tests passed."
fi

if test -z "$HDF5_NOCLEANUP"; then
    $RM -r $OUTDIR
fi
exit $nerrors
