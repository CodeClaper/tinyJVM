#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "java.h"

void seterror(char *format, ...) {
    size_t len;
    va_list ap;

    /* Calculate the len. */
    va_start(ap, format);
    len = vsnprintf(NULL, 0, format, ap);
    if (len <= 0) {
        va_end(ap);
        return;
    }

    len = len + 1;
    char message[len];
    memset(message, 0, len);

    va_start(ap, format);
    vsnprintf(message, len, format, ap);
    va_end(ap);

    memcpy(javaStates.error, message, len);
}
