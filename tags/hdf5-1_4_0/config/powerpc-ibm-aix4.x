#							-*- shell-script -*-
#
# This file is part of the HDF5 build script.  It is processed shortly
# after configure starts and defines, among other things, flags for
# the various compile modes.


#----------------------------------------------------------------------------
# Compiler flags. The CPPFLAGS values should not include package debug
# flags like `-DH5G_DEBUG' since these are added with the
# `--enable-debug' switch of configure.

# What must *always* be present for things to compile correctly?
#CFLAGS="$CFLAGS -ansi"
#CPPFLAGS="$CPPFLAGS -I."

# What compiler flags should be used for code development?
DEBUG_CFLAGS=
DEBUG_CPPFLAGS=

# What compiler flags should be used for building a production
# library?
PROD_CFLAGS=
PROD_CPPFLAGS=

# What compiler flags enable code profiling?
PROFILE_CFLAGS=
PROFILE_CPPFLAGS=


#----------------------------------------------------------------------------
# Values for overriding configuration tests when cross compiling.
# This includes compiling on some machines where the serial front end
# compiles for a parallel back end.

# Set this to `yes' or `no' depending on whether the target is big
# endian or little endian.
hdf5_cv_printf_ll=${hdf5_cv_printf_ll='ll'}
ac_cv_c_bigendian=${ac_cv_c_bigendian='yes'}
ac_cv_header_stdc=${ac_cv_header_stdc='yes'}
ac_cv_header_sys_ioctl_h=${ac_cv_header_sys_ioctl_h=yes}
RUNPARALLEL="MP_PROCS=2 poe"

# cache the sizeof of "standard C types" so that configure can run faster.
ac_cv_sizeof_char=${ac_cv_sizeof_char=1}
ac_cv_sizeof_short=${ac_cv_sizeof_short=2}
ac_cv_sizeof_int=${ac_cv_sizeof_int=4}
ac_cv_sizeof_long=${ac_cv_sizeof_long=4}
ac_cv_sizeof_long_long=${ac_cv_sizeof_long_long=8}
ac_cv_sizeof___int64=${ac_cv_sizeof___int64=8}
ac_cv_sizeof_float=${ac_cv_sizeof_float=4}
ac_cv_sizeof_double=${ac_cv_sizeof_double=8}
ac_cv_sizeof_long_double=${ac_cv_sizeof_long_double=8}
ac_cv_sizeof_int8_t=${ac_cv_sizeof_int8_t=1}
ac_cv_sizeof_uint8_t=${ac_cv_sizeof_uint8_t=1}
ac_cv_sizeof_int_least8_t=${ac_cv_sizeof_int_least8_t=1}
ac_cv_sizeof_uint_least8_t=${ac_cv_sizeof_uint_least8_t=1}
ac_cv_sizeof_int_fast8_t=${ac_cv_sizeof_int_fast8_t=1}
ac_cv_sizeof_uint_fast8_t=${ac_cv_sizeof_uint_fast8_t=4}
ac_cv_sizeof_int16_t=${ac_cv_sizeof_int16_t=2}
ac_cv_sizeof_uint16_t=${ac_cv_sizeof_uint16_t=2}
ac_cv_sizeof_int_least16_t=${ac_cv_sizeof_int_least16_t=2}
ac_cv_sizeof_uint_least16_t=${ac_cv_sizeof_uint_least16_t=2}
ac_cv_sizeof_int_fast16_t=${ac_cv_sizeof_int_fast16_t=4}
ac_cv_sizeof_uint_fast16_t=${ac_cv_sizeof_uint_fast16_t=4}
ac_cv_sizeof_int32_t=${ac_cv_sizeof_int32_t=4}
ac_cv_sizeof_uint32_t=${ac_cv_sizeof_uint32_t=4}
ac_cv_sizeof_int_least32_t=${ac_cv_sizeof_int_least32_t=4}
ac_cv_sizeof_uint_least32_t=${ac_cv_sizeof_uint_least32_t=4}
ac_cv_sizeof_int_fast32_t=${ac_cv_sizeof_int_fast32_t=4}
ac_cv_sizeof_uint_fast32_t=${ac_cv_sizeof_uint_fast32_t=4}
ac_cv_sizeof_int64_t=${ac_cv_sizeof_int64_t=8}
ac_cv_sizeof_uint64_t=${ac_cv_sizeof_uint64_t=8}
ac_cv_sizeof_int_least64_t=${ac_cv_sizeof_int_least64_t=8}
ac_cv_sizeof_uint_least64_t=${ac_cv_sizeof_uint_least64_t=8}
ac_cv_sizeof_int_fast64_t=${ac_cv_sizeof_int_fast64_t=8}
ac_cv_sizeof_uint_fast64_t=${ac_cv_sizeof_uint_fast64_t=8}
ac_cv_sizeof_size_t=${ac_cv_sizeof_size_t=4}
ac_cv_sizeof_off_t=${ac_cv_sizeof_off_t=8}
