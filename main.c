#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <math.h>

#include "klib/ketopt.h"
#include "seqKmer.h"
#include "seqRead.h"
#include "Galaxy.h"

//global varaibles for commond-line options
int KmerSize = 17;   
char * prefix;
int Input_file_format = 1;
int threadNum = 10;

int Whether_output_kmerfreq = 0;
int Output_kmerfreq_cutoff = 5;
uint64_t nonLow_Kmer_species_num = 0;

//global varaibles for the major compution 
uint64_t KmerHeadMaskVal = 0;  //used in generating a new kmer bit data from previous kmer bit data
uint64_t KmerRCOrVal[4];  //used in generating a new kmer bit data from previous kmer bit data
const int FreqMax = 65535;    //the max value of kmer frequency for using 16-bit storage.

uint64_t Kmer_theory_total = 0;  //the max theoretic number of kmer species with given KmerSize
uint64_t Kmer_total_num = 0;  //the total number of kmer individuals in the input data
uint64_t Kmer_effect_num = 0;  //the total number of kmer species in the input data

uint16_t *KmerFreq;   //store frequency values of each kmer
uint64_t *FreqArray;  //store the statistis distribution of kmer frequency



//variables and routines in parsing the read files parallely
int BufferNum = 10000;  //max number of reads can be stored in the memeory buffer 
char *RawReads;  //store reads sequence
uint8_t *Signal; //store the status: 0, empty; 1, filled; 2, end of file;
int ReadsNum = 0;   //real number of reads stored in the buffer block

//variables for compress the memory data in parallel
int Whether_output_memory = 0;
uint64_t Kmer_hifreq_num = 0;  //the total number of high-frequency kmer species in the input data
int Low_high_freq_cutoff = 5;   //cutoff to define low and non-low frequency kmer

uint64_t bufBlockId = 0;
uint64_t bufBlockNum = 0; //calculate in the program
uint64_t bufBlockSize = 8*1024*1024; //8M kmer species space number for each compressing block



static void usage(void) {
	ERRprintf("\n\033[1m"
"Function introduction:\033[0m\n"
"kmerfreq count K-mer (with size K) frequency from the input sequence data, typically sequencing reads data, and reference genome data is also applicable. The forward and reverse strand of a k-mer are taken as the same k-mer, and only the kmer strand with smaller bit-value is used to represent the kmer. It adopts a 16-bit integer with max value 65535 to store the frequency value of a unique K-mer, and any K-mer with frequency larger than 65535 will be recorded as 65535. The program store all kmer frequency values in a 4^K size array of 16-bit integer (2 bytes), using the k-mer bit-value as index, so the total memory usage is 2* 4^K bytes. For K-mer size 15, 16, 17, 18, 19, it will consume constant 2G, 8G 32G 128G 512G memory, respectively. kmerfreq works in a highly simple and parallel style, to achieve as fast speed as possible.\n"
"\n\033[1m"
"Input and output:\033[0m\n"
"Input is a library file that contains the path of all the input sequence files in fastq format or one-line fasta format, with each line represents the path of a single sequence file. The program parse each sequencing reads file by the given order, and make a whole statistics. Output by default is a table file for the distribution of the kmer frequency, which can be analyzed for estimating sequencing quality and genomic characteristics. Two other optional output files are also avialable. By setting parameter \"-w 1\", the Kmer sequence with corresponding frequency values will be output for those K-mers with frequency value >= cutoff set by parameter -c; This is a human readable file that can facilate other kmer associated analysis. By setting parameter \"-m 1\", the whole kmer frequency table from the computer memory will be compressed and output, using 1-bit for each unique kmer, with 0 for kmer with frequency lower than cutoff set by parameter -q, and 1 for other kmers. This is a binary file, which can be reloaded to memory by other program that uses the kmer frequency data as input.\n"
"\n\033[1m"
"Reference papers:\033[0m\n"
"A paper describing how to decide the kmer size for various data set and the potential applications for kmers:\n"
"Binghang Liu, Yujian Shi, Jianying Yuan, et al. and Wei Fan*. Estimation of genomic characteristics by analyzing k-mer frequency in de novo genome project. arXiv.org arXiv: 1308.2012. (2013)\n"
"\n\n"
"kmerfreq  [options] <reads_files.lib>\n"
"   Author Wei Fan, fanweiagis@126.com\n"
"   Version 4.0\n"
"   -k <int>  kmer size, recommand value 13 to 19, default=17\n"
"   -f <int>  input file format: 1: fq|gz(one-line), 2: fa|gz(one-line), default=1\n"
"   -p <str>  output file prefix, default=reads_files.lib\n"
"   -r <int>  number of reads stored in buffer memory, default=10000\n"
"   -t <int>  thread number to use in parallel, default=10\n"
"   -w <int>  whether output kmer sequence and frequency value, , 1:yes, 0:no, default=0\n"
"   -c <int>  kmer frequency cutoff, equal or larger will be output, co-used with -w, default=5\n"
"   -m <int>  whether output computer memory data, 1:yes, 0:no, default=0\n"
"   -q <int>  kmer frequency cutoff, 0 for lower, 1 for equal and larger, co-used with -m,  default=5\n"
"   -h        get help information\n"
"\n"
"Example: kmerfreq  reads_files.lib\n"
"         kmerfreq  -k 17 -t 10 -p Ecoli_K17 reads_files.lib\n"
"         kmerfreq  -k 17 -t 10 -p Ecoli_K17 -w 1 -c 5 reads_files.lib\n"
"         kmerfreq  -k 17 -t 10 -p Ecoli_K17 -m 1 -q 5 reads_files.lib\n"
"\n");
	exit(2);
}

int main(int argc, char *argv[]) {
	if (argc == 1) usage();
	ketopt_t opt = KETOPT_INIT;
	int c;
	while ((c = ketopt(&opt, argc, argv, 1, "k:f:t:r:p:w:c:m:q:h", NULL)) >= 0) {
		if (c == 'k') KmerSize=atoi(opt.arg);
		else if (c == 'f')  Input_file_format=atoi(opt.arg);
		else if (c == 't')  threadNum=atoi(opt.arg);
		else if (c == 'r')  BufferNum=atoi(opt.arg);
		else if (c == 'p')  prefix=opt.arg;
		else if (c == 'w')  Whether_output_kmerfreq=atoi(opt.arg);
		else if (c == 'c')  Output_kmerfreq_cutoff=atoi(opt.arg);
		else if (c == 'm')  Whether_output_memory=atoi(opt.arg);
		else if (c == 'q')  Low_high_freq_cutoff=atoi(opt.arg);
		else if (c == 'h')  usage();
		else if (c == '?') printf("unknown opt: -%c\n", opt.opt? opt.opt : ':');
		else if (c == ':') printf("missing arg: -%c\n", opt.opt? opt.opt : ':');
	}
	printf("KmerSize:%i Input_file_format:%i threadNum:%i BufferNum:%i prefix:%s Whether_output_kmerfreq:%i Output_kmerfreq_cutoff:%i Whether_output_memory:%i Low_high_freq_cutoff:%i\n",KmerSize,Input_file_format,threadNum,BufferNum,prefix,Whether_output_kmerfreq,Output_kmerfreq_cutoff,Whether_output_memory,Low_high_freq_cutoff);
	printf("Non-option arguments:");

	for (int i = opt.ind; i < argc; ++i)
		printf(" %s", argv[i]);
	putchar('\n');
	
	
	
	//clock_t time_start, time_end;
	//time_start = clock();

	char * reads_file_list = argv[opt.ind++];
	if (!prefix) prefix = reads_file_list;
	printf("prefix:%s reads_file_list:%s\n",prefix,reads_file_list);

	//used in generating a new kmer bit data from previous kmer bit data
	KmerHeadMaskVal = pow_integer(2, KmerSize*2) - 1;	
	KmerRCOrVal[3] = 0;
	KmerRCOrVal[1] = pow_integer(2,KmerSize*2-1);
	KmerRCOrVal[2] = pow_integer(2,KmerSize*2-1-1);;
	KmerRCOrVal[0] = KmerRCOrVal[1] + KmerRCOrVal[2];
	
	kvec_pchar * reads_files = reading_file_list(reads_file_list);
	for (size_t i=0;i<kv_size(*reads_files);i++) {
		char * thisFQname = kv_A(*reads_files,i);
		printf("%ld: %s\n",i,thisFQname);
		readFQgz(thisFQname);
	}
	
	return 0;
}
