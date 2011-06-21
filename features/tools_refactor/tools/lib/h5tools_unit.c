#include "check.h"
#include "h5tools.h"
#include "h5tools_utils.h"

/* Name of tool */
#define PROGRAMNAME "h5dump_unit"

void setup(void)
{
    h5tools_init();
}

void teardown(void)
{
    h5tools_close();
}

START_TEST (test_progname)
{
    h5tools_setprogname(PROGRAMNAME);
    fail_unless (strcmp(h5tools_getprogname(), "h5dump_unit") == 0, "program name not correct");
}
END_TEST

int
main(int argc, const char *argv[])
{
    return 0;
}
