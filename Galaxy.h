#ifndef __Galaxy_H__
#define __Galaxy_H__

#include "klib/kvec.h"

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

// https://stackoverflow.com/a/1644898/159695
#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define ERRprintf(...) fprintf(stderr, __VA_ARGS__)

typedef kvec_t(char*) kvec_pchar;
kvec_pchar *reads_files;

#ifdef __cplusplus
}
#endif

#endif /* __Galaxy_H__ */
