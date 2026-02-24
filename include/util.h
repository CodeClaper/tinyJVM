#include "c.h"

int castInt(U4 bytes);
float castFloat(U4 bytes);
I2 castShort(U1 high_byte, U2 low_byte);
long castLong(U4 high_bytes, U4 low_bytes);
double castDouble(U4 high_bytes, U4 low_bytes);
void seterror(char *format, ...);
