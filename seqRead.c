#include <zlib.h>
#include <stdio.h>
#include <pthread.h>
#include "seqRead.h"
#include "seqKmer.h"
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

pthread_t *tp_enq, *tp_deq;
// Declaration of thread condition variable
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
// declaring mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

size_t linecachesize = 4*1024*1024;
void* readaFQgz(void *arg) {
	char * fqname = (char *) arg;
	gzFile fp;
	kseq_t *seq;
	int l;
	fp = gzopen(fqname, "r");
	seq = kseq_init(fp);
	while ((l = kseq_read(seq)) >= 0) {
		if (seq->seq.l > 0) {
			char * seqstr = malloc(seq->seq.l+1);
			assert(seqstr != NULL);
			strcpy(seqstr,seq->seq.s);
			while (lfqueue_enq(IFQseq, seqstr)) {	/*Enqueue*/
				lfqueue_sleep(1);
				// printf("ENQ FULL?\n");
			}
		}
#ifdef DEBUG
		printf("\tname: %s\n", seq->name.s);
		if (seq->comment.l) printf("\tcomment: %s\n", seq->comment.s);
		printf("\tseq: %s\n", seq->seq.s);
		if (seq->qual.l) printf("\tqual: %s\n", seq->qual.s);
#endif
	}
	printf("return value: %d\n", l);
	kseq_destroy(seq);
	gzclose(fp);
	// __sync_add_and_fetch(&nthreads_exited, 1);
	return 0;
}
void* countaFromIFQ(void *arg) {
	uint16_t * KmerFreqArray = (uint16_t *) arg;
	while (1) {	// NEEDs a signal.
		char * seqstr = lfqueue_deq_must(IFQseq);	/*Dequeue*/
		printf("%s\n", seqstr);
		free(seqstr);
	}
	//__sync_add_and_fetch(&nthreads_exited, 1);
	return 0;
}
int nthreads = 4;
int nthreads_exited = 0;
void doThreadedIFQ(kvec_pchar * in_reads_files) {
	size_t filecnt = kv_size(*in_reads_files);
	tp_enq = malloc(filecnt*sizeof(pthread_t));
	tp_deq = malloc(nthreads*sizeof(pthread_t));
	uint16_t * KmerFreqArray = calloc(100,sizeof(uint16_t));
	for (size_t i=0; i<filecnt; i++) {
		char * thisFQname = kv_A(*in_reads_files,i);
		pthread_create(tp_enq + i, NULL, readaFQgz, thisFQname);
	}
	for (int i = 0; i < nthreads; i++)
		pthread_create(tp_deq + i, NULL, countaFromIFQ, KmerFreqArray);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
	for (int i = 0; i < nthreads; i++) {
		//pthread_detach(tp_deq[i]);
		pthread_join(tp_deq[i], NULL);
	}
	while ( nthreads_exited < nthreads )
		lfqueue_sleep(10);
	if(lfqueue_size(IFQseq) != 0){
		lfqueue_sleep(10);
	}
#pragma GCC diagnostic pop

}



typedef void (*test_function)(pthread_t*);
void one_enq_and_multi_deq_must(pthread_t *threads);
void*  worker_s(void *);
void*  worker_c_must(void *);
void running_test(test_function testfn);

struct timeval  tv1, tv2;
#define total_put 5000
#define total_running_loop 10
int one_thread = 1;
lfqueue_t *myq;

#define join_threads \
for (i = 0; i < nthreads; i++) {\
pthread_join(threads[i], NULL); \
}

#define detach_thread_and_loop \
for (i = 0; i < nthreads; i++)\
pthread_detach(threads[i]);\
while ( nthreads_exited < nthreads ) \
	lfqueue_sleep(10);\
if(lfqueue_size(myq) != 0){\
lfqueue_sleep(10);\
}

void*  worker_c_must(void *arg) {
	int i = 0;
	int *int_data;
	int total_loop = total_put * (*(int*)arg);
	while (i++ < total_loop) {
		/*Dequeue*/
		int_data = lfqueue_deq_must(myq);
		//	printf("%d\n", *int_data);

		free(int_data);
	}
	__sync_add_and_fetch(&nthreads_exited, 1);
	return 0;
}
void*  worker_s(void *arg)
{
	int i = 0, *int_data;
	int total_loop = total_put * (*(int*)arg);
	while (i++ < total_loop) {
		int_data = (int*)malloc(sizeof(int));
		assert(int_data != NULL);
		*int_data = i;
		/*Enqueue*/

		while (lfqueue_enq(myq, int_data)) {
			// printf("ENQ FULL?\n");
		}
	}
	// __sync_add_and_fetch(&nthreads_exited, 1);
	return 0;
}
void one_enq_and_multi_deq_must(pthread_t *threads) {
	printf("-----------%s---------------\n", "one_enq_and_multi_deq_must");
	int i;
	for (i = 0; i < nthreads; i++)
		pthread_create(threads + i, NULL, worker_c_must, &one_thread);

	worker_s(&nthreads);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
	detach_thread_and_loop;
#pragma GCC diagnostic pop

}

void running_test(test_function testfn) {
	int n;
	for (n = 0; n < total_running_loop; n++) {
		printf("Current running at =%d, ", n);
		nthreads_exited = 0;
		/* Spawn threads. */
		pthread_t threads[nthreads];
		printf("Using %d thread%s.\n", nthreads, nthreads == 1 ? "" : "s");
		printf("Total requests %d \n", total_put);
		gettimeofday(&tv1, NULL);

		testfn(threads);
		// one_enq_and_multi_deq(threads);

		//one_deq_and_multi_enq(threads);
		// multi_enq_deq(threads);
		// worker_s(&ri);
		// worker_c(&ri);

		gettimeofday(&tv2, NULL);
		printf ("Total time = %f seconds\n",
		        (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
		        (double) (tv2.tv_sec - tv1.tv_sec));

		lfqueue_sleep(10);
		assert ( 0 == lfqueue_size(myq) && "Error, all queue should be consumed but not");
	}
}
int testit(void) {
	myq = malloc(sizeof	(lfqueue_t));
	if (lfqueue_init(myq) == -1)
		return -1;

	running_test(one_enq_and_multi_deq_must);

	lfqueue_destroy(myq);
	// sleep(3);
	free(myq);

	printf("Test Pass!\n");

	return 0;
}

