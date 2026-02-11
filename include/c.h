#include <stdint.h>
#define UNUSED(V)       ((void) V)
#define ERR             -1
#define OK              0
#define MAGIC           0xCAFEBABE

#ifdef DEBUG
#define TRY(expr) \
	do { \
		if ((expr) == -1) { \
            fprintf(stderr,                         \
              "%s:%d: %lld: " #expr "\n",           \
              __FILE__, __LINE__, (long long)(expr) \
            );                                      \
			goto error; \
		} \
	} while(0)
#else
#define TRY(expr) \
	do { \
		if ((expr) == -1) { \
			goto error; \
		} \
	} while(0)
#endif
#define Assert(condition) assert(condition)
#define AssertFalse(condition) assert(!(condition))

typedef uint8_t     U1;
typedef uint16_t    U2;
typedef uint32_t    U4;
typedef uint64_t    U8;

typedef int8_t      I1;
typedef int16_t     I2;
typedef int32_t     I4;
typedef int64_t     I8;
