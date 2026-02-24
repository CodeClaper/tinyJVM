#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "util.h"
#include "java.h"

/* Cast to int. */
int castInt(U4 bytes) {
    int i;
    memcpy(&i, &bytes, sizeof(U4));
    return i;
}

/* Cast to float. */
float castFloat(U4 bytes) {
    float f;
    memcpy(&f, &bytes, sizeof(U4));
    return f;
}

/* Case to short., */
I2 castShort(U1 high_byte, U2 low_byte) {
    uint16_t s;
    int16_t v;

    s = (high_byte << 8) | low_byte;
    memcpy(&v, &s, sizeof(v));

    return v;
}

/* Cast to long. */
long castLong(U4 high_bytes, U4 low_bytes) {
	uint64_t u;
	int64_t l;

	u = ((uint64_t)high_bytes << 32) | (uint64_t)low_bytes;
	memcpy(&l, &u, sizeof l);
	return l;
}

/* Cast to double. */
double castDouble(U4 high_bytes, U4 low_bytes) {
	uint64_t u;
	double d;

	u = ((uint64_t)high_bytes << 32) | (uint64_t)low_bytes;
	memcpy(&d, &u, sizeof d);
	return d;
}


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
