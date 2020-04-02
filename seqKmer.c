#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>
#include "seqKmer.h"

//chengfang calculation for small integers
uint64_t pow_integer(int base, int exponent) {
	uint64_t result = 1;
	for (int i = 1; i<=exponent; i++) {
		result *= base;
	}
	return result;
}

kvec_pchar reads_files;
//read file_list into a vector
kvec_pchar * reading_file_list(char * file_list) {
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen(file_list, "r");
	if (fp == NULL) {
		err(EX_NOINPUT, "Fail to open input file:[%s]", file_list);
	}
	kv_init(reads_files);
	const char *lineEnds = "\n\r";
	while ((read = getline(&line, &len, fp)) != -1) {
		//printf("Retrieved line of length %zu :\n", read);
		//printf("%s", line);
		size_t spnsz = strcspn(line, lineEnds);
		//printf("%i\n",spnsz);
		char *str = malloc(spnsz+1);
		strncpy(str,line,spnsz);
		kv_push(char*,reads_files,str);
	}
	if (ferror(fp)) {
		/* handle error */
	}
	free(line);
	fclose(fp);
	return &reads_files;
	/*
	ifstream infile ( file_list.c_str() );
	if ( ! infile )
	{	cerr << "fail to open input file" << file_list << endl;
	}

	string line_str;
	while ( getline( infile, line_str, '\n' ) )
	{	string file_name;
		for(int i=0; i<line_str.size();i++)
		{	if(line_str[i] != ' ' && line_str[i] != '\t' && line_str[i] != '\n')
			{	file_name.push_back(line_str[i]);
			}
		}
		if (file_name.size())
		{	files.push_back(file_name);
		}
	}*/
}
