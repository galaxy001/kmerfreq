#ifndef __seqRead_H__
#define __seqRead_H__

#include "klib/kseq.h"
#include "lfqueue.h"

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

void readFQgz(char * fqname);

lfqueue_t *IFQseq;

int testit(void);

#ifdef __cplusplus
}
#endif

#endif /* __seqRead_H__ */
