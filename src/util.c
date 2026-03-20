#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "util.h"
#include "class.h"
#include "data.h"

/* Case to short., */
I2 castShort(U1 high_byte, U2 low_byte) {
    uint16_t s;
    int16_t v;

    s = (high_byte << 8) | low_byte;
    memcpy(&v, &s, sizeof(v));

    return v;
}

/* Cast to int. */
I4 castInt(U4 bytes) {
    I4 i;
    memcpy(&i, &bytes, sizeof(U4));
    return i;
}

/* Cast to long. */
I8 castLong(U4 high_bytes, U4 low_bytes) {
	uint64_t u;
	int64_t l;

	u = ((uint64_t)high_bytes << 32) | (uint64_t)low_bytes;
	memcpy(&l, &u, sizeof l);
	return l;
}

/* Cast to float. */
float castFloat(U4 bytes) {
    float f;
    memcpy(&f, &bytes, sizeof(U4));
    return f;
}


/* Cast to double. */
double castDouble(U4 high_bytes, U4 low_bytes) {
	uint64_t u;
	double d;

	u = ((uint64_t)high_bytes << 32) | (uint64_t)low_bytes;
	memcpy(&d, &u, sizeof d);
	return d;
}

RunMode runMode(char *name) {
    if (strstr(name, "javap") != NULL) return JAVAP;
    else if (strstr(name, "java") != NULL) return JAVA;
    else return NONE;
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


void error(char *format, ...) {
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
    fprintf(stderr, "Error: %s\n", message);

    exit(1);
}

/* Type size. */
U4 getTypeSize(char type) {
    switch (type) {
        case 'Z': case 'B': return 1; // boolean, byte
        case 'C': case 'S': return 2; // char, short
        case 'I': case 'F': return 4; // int, float
        case 'J': case 'D': return 8; // long, double
        case 'L': case '[': return 8; // reference
        default: return 0;
    }
}

/* Get method arg count.*/
U2 getMethodArgCount(char *descriptor) {
    U2 nargs;
    char *p;

    nargs = 0;
    p = descriptor;

    while (*p != '\0' && *p != '(') p++;
    while (*p != '\0' && *p != ')') {
        if (*p == 'L') {
            nargs++;
            while (*p != '\0' && *p != ';') p++;
        } else if (*p == '[') {
            nargs++;
            while (*p == '[') p++;
            if (*p == 'L') {
                while (*p != '\0' && *p != ';') p++;
            }
        } else nargs++;

        if (*p != '\0' && *p != ')') p++;
    }

    return nargs;
}

/* Get method slot count. */
U2 getMethodSlotCount(char *descriptor, U2 flags) {
    U2 nslots;
    char *p;

    nslots = (flags & ACC_METHOD_STATIC) ? 0 : 1;
    p = strchr(descriptor, '(');
    if (p == NULL) return nslots;
    else p++; // skip '(';
    
    while (*p != ')') {
        if (*p == 'J' || *p == 'D') nslots += 2;
        else if (*p == 'L') {
            nslots += 1;
            while (*p != ';') p++;
        } else if (*p == '[') {
            nslots += 1;
            while (*p == '[') p++;
            if (*p == 'L') {
                while (*p != ';') p++;
            }
        } else nslots += 1;

        p++;
    }

    return nslots;
}
