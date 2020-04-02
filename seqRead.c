#include <zlib.h>
#include <stdio.h>
#include "seqRead.h"
#include "lfqueue.h"

KSEQ_INIT(gzFile, gzread)

void readFQgz(char * fqname) {
	gzFile fp;
	kseq_t *seq;
	int l;
	fp = gzopen(fqname, "r");
	seq = kseq_init(fp);
		while ((l = kseq_read(seq)) >= 0) {
		printf("\tname: %s\n", seq->name.s);
		if (seq->comment.l) printf("\tcomment: %s\n", seq->comment.s);
		printf("\tseq: %s\n", seq->seq.s);
		if (seq->qual.l) printf("\tqual: %s\n", seq->qual.s);
	}
	printf("return value: %d\n", l);
	kseq_destroy(seq);
	gzclose(fp);
}
