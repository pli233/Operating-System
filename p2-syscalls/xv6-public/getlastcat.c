#include "types.h"
#include "stat.h"
#include "user.h"

char buf[256];

int
main(void)
{
    int result = getlastcat(buf);
    printf(result, "XV6_TEST_OUTPUT Last catted filename: %s\n", buf);
    exit();
}
