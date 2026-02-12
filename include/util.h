#include "c.h"

int castInt(U4 bytes);
float castFloat(U4 bytes);
long castLong(U4 high_bytes, U4 low_bytes);
double castDouble(U4 high_bytes, U4 low_bytes);
void seterror(char *format, ...);
