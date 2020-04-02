#ifndef __seqKmer_H__
#define __seqKmer_H__

#include <inttypes.h>

#include "klib/kvec.h"

typedef kvec_t(char*) kvec_pchar;

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

//read file_list into a vector
kvec_pchar * reading_file_list(char * file_list);

//calculate pow for uint64_t integers
uint64_t pow_integer(int base, int exponent);

#ifdef __cplusplus
}
#endif

#endif /* __seqKmer_H__ */
