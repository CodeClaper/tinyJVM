
typedef struct JavapOptions {
    int         verbose;                /* Javap in verbose mode. */
    int         private;                /* Show all calsses and numbers. */
    int         sflag;                  /* Print internal type signatures. */
    int         lflag;                  /* Print line number and local variable tables. */
    int         cflag;                  /* Disassemble the code. */
} JavapOptions;

extern struct JavapOptions javapOptions;
