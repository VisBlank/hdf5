#							-*- shell-script -*-
#
# This file is part of the HDF5 build script.  It is processed shortly
# after configure starts and defines, among other things, flags for
# the various compile modes.
#
# See BlankForm in this directory for details.

# Use SGI supplied C compiler by default.  There is no ranlib
if test "X-" =  "X-$CC"; then
    CC='cc'
    CC_BASENAME=cc
fi
RANLIB=:

# Compiler flags
case "X-$CC_BASENAME" in
    X-gcc)
	CFLAGS="$CFLAGS -Wsign-compare" #Only works for some versions
	DEBUG_CFLAGS="-g -fverbose-asm"
	DEBUG_CPPFLAGS="-DH5F_OPT_SEEK=0 -DH5F_LOW_DFLT=H5F_LOW_SEC2"
	PROD_CFLAGS="-O3"
	PROD_CPPFLAGS=
	PROFILE_CFLAGS="-pg"
	PROFILE_CPPFLAGS=
	;;

    *)

        # Check for old versions of the compiler that don't work right.
        case "`$CC -version 2>&1 |head -1`" in
	    "Mongoose Compilers: Version 7.00")
		echo "  +---------------------------------------------------+"
		echo "  | You have an old version of cc (Mongoose Compilers |"
		echo "  | version 7.00).  Please upgrade to MIPSpro version |"
		echo "  | 7.2.1.2m (patches are available from the SGI web  |"
		echo "  | site).  The 7.00 version may generate incorrect   |"
		echo "  | code, especially when optimizations are enabled.  |"
		echo "  +---------------------------------------------------+"
		sleep 5
		;;
	esac

        # Do *not* use -ansi because it prevents hdf5 from being able
        # to read modification dates from the file. On some systems it
        # can also result in compile errors in system header files
        # since hdf5 includes a couple non-ANSI header files.
        #CFLAGS="$CFLAGS -ansi"

	# Always turn off these compiler warnings for the -64 compiler:
	#    1174:  function declared but not used
	#    1429:  the `long long' type is not standard
	#    1209:  constant expressions
	#    1196:  __vfork() (this is an SGI config problem)
	#    1685:  turn off warnings about turning off invalid warnings
	CFLAGS="$CFLAGS -woff 1174,1429,1209,1196,1685"

	# Always turn off these compiler warnings for the old compiler:
	#    799:   the `long long' type is not standard
	#    803:   turn off warnings about turning off invalid warnings
        #    835:   __vfork() (this is an SGI config problem)
	CFLAGS="$CFLAGS -woff 799,803,835"

	# Always turn off these loader warnings:
	# (notice the peculiar syntax)
	#      47:  branch instructions that degrade performance on R4000
	#      84:  a library is not used
	#      85:  duplicate definition preemption (from -lnsl)
	#     134:  duplicate weak definition preemption (from -lnsl)
	CFLAGS="$CFLAGS -Wl,-woff,47,-woff,84,-woff,85,-woff,134"

	# Extra debugging flags
	DEBUG_CFLAGS="-g -fullwarn"
	DEBUG_CPPFLAGS=

	# Extra production flags
	PROD_CFLAGS=-O
	PROD_CPPFLAGS=

	# Extra profiling flags
	PROFILE_CFLAGS=-pg
	PROFILE_CPPFLAGS=
	;;
esac
