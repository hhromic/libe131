#ifdef _WIN32
#include <stdlib.h>
#define err(level, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit (level); }
#else
#include <err.h>
#endif
