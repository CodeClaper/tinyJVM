#include "c.h"

I2 castShort(U1 high_byte, U2 low_byte);
I4 castInt(U4 bytes);
I8 castLong(U4 high_bytes, U4 low_bytes);
float castFloat(U4 bytes);
double castDouble(U4 high_bytes, U4 low_bytes);
void seterror(char *format, ...);
void error(char *format, ...);
