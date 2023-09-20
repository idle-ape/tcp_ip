#include <stdio.h>
#include <stdlib.h>

#include "error_handling.h"

int error_handling(char *format, char *message)
{
    fprintf(stderr, format, message);
    fputc('\n', stderr);
    exit(1);
}