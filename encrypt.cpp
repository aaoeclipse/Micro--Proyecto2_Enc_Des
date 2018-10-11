#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <time.h>

int ROUNDS = 16;
/// Block size in bytes.
int BLOCK_SIZE = 8;

uint32_t function(uint32_t fortnite, uint32_t pubg)
{
	return fortnite ^ pubg;
}

uint64_t encrypt(uint32_t left, uint32_t right, uint32_t rounds, uint32_t keys[])
{
	uint32_t tempLeft, tempRight;
	for (int i = 0; i < rounds; i++)
	{
		tempLeft = function(left, keys[i]) ^ right;
		tempRight = left;
		if (i == rounds - 1)
		{
			left = tempRight;
			right = tempLeft;
		}
		else
		{
			left = tempLeft;
			right = tempRight;
		}
	}
	return (uint64_t) left << 32 | right;
}

void encrypt_cbc(FILE *infile, FILE *outfile, uint32_t rounds, uint32_t keys[]){
	size_t result;
	uint64_t prevBlock = 0xF0CCFACE;
	uint64_t block;
	uint32_t left, right;
		while (!feof(infile))
	{
		/// Read 8 bytes from input file.
		result = fread(&block, 1, BLOCK_SIZE, infile);
		/// Bitwise XOR block with previous block of ciphertext.
		/// This is what makes the algorithm a Chain Block Cipher (CBC)
		block ^= prevBlock;
		/// This is the swap function to prepare the input for the next
		/// round.
		/// Left holds the 32 rightmost bits from block. Right holds
		/// The 32 leftmost bits from block.
		left = (block >> 32) & 0xFFFFFFFF;
		right = block & 0xFFFFFFFF;
		block = encrypt(left, right, rounds, keys);
		prevBlock = block;
		/// Write encrypted block of ciphertext to output file.
		fwrite(&block, 1, BLOCK_SIZE, outfile);
	}
	
}

///medir tiempo.
int diff(timespec start, timespec end, timespec &diff){

	if ((end.tv_nsec-start.tv_nsec)<0) {
		diff.tv_sec = end.tv_sec-start.tv_sec-1;
		diff.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		diff.tv_sec = end.tv_sec-start.tv_sec;
		diff.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return 0;
}


int main(int argc, char* argv[])
{
	timespec t1, t2, diff_t;
	FILE *inputFile, *outputFile;
	size_t result;
	uint32_t keys[ROUNDS] = {
		0xFF91B5F5,
		0xF9281A0E,
		0x84282A36,
		0xE8D63C4A,
		0x0C402C6F,
		0x2296CB30,
		0x9FF9D76E,
		0x243A5572,
		0xA4AE9DD0,
		0x999F201E,
		0x9A0CB9A5,
		0x349968F5,
		0x62FD58D0,
		0x339DFC3C,
		0x4815AD1E
	};
	
	/// Program arguments validation.
	if (argc < 3)
	{
		printf("Insufficient arguments.\n");
		return -1;
	}
	
	/// Open input file and validate it exists.
	inputFile = fopen(argv[1], "r");
	if (inputFile == NULL)
	{
		printf("Input file error.\n");
		return -1;
	}
	
	outputFile = fopen(argv[2], "w");
	if (outputFile == NULL)
	{
		printf("Output file error.\n");
		return -1;
	}
	
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
	encrypt_cbc(inputFile, outputFile, ROUNDS, keys);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
	
	diff(t1, t2, diff_t);

	int sec = diff_t.tv_sec;
	int ns  = diff_t.tv_nsec;

	printf("EXEC TIME\t%d.%d\n", sec, ns);
	
	/// Close the input file stream
	fclose(inputFile);
	fclose(outputFile);
	
	return 0;
}