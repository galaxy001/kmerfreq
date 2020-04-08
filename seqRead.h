#ifndef __seqRead_H__
#define __seqRead_H__

#include "klib/kseq.h"
#include "lfqueue.h"
#include "Galaxy.h"

void* countaFromIFQ(void *arg);
void* readaFQgz(void *arg);

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

lfqueue_t *IFQseq;
void doThreadedIFQ(kvec_pchar * in_reads_files);

void readFQgz(char * fqname);

int testit(void);

#ifdef __cplusplus
}
#endif

#endif /* __seqRead_H__ */
