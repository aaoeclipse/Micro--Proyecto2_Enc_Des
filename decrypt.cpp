#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sstream>
#include <fstream>
#include <codecvt>


#define ROUNDS 16
#define BLOCK_SIZE 8

using namespace std;
uint32_t f(uint32_t block, uint32_t key);
uint64_t decrypt(uint32_t left, uint32_t right, uint32_t rounds, uint32_t keys[]);
void decrypt_cbc(FILE *infile, FILE *decryptedFile, uint32_t rounds, uint32_t keys[]);
int diff(timespec start, timespec end, timespec &diff);

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
    0x4815AD1E};
int main(int argc, char *argv[])
{
	timespec t1, t2, diff_t;
        FILE *cryptFile, *decryptedFile;
        if (argc != 2)
        {
                fprintf(stderr, "[-] Incorrect format!\n");
                fprintf(stderr, "$ ./[runfile] [cryptFile]\n");
                return EXIT_FAILURE;
        }
        cryptFile = fopen(argv[1], "r");
        if (!cryptFile)
        {
                printf("[-] Error trying to open file: ");
                cout << argv[1] << endl;
                return EXIT_FAILURE;
        }
        cout << "[+] " << argv[1] << " found!" << endl;
        decryptedFile = fopen("decryptedFile", "w");
        // Time
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
        decrypt_cbc(cryptFile, decryptedFile, ROUNDS, keys);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);

        fclose(cryptFile);
        fclose(decryptedFile);

        diff(t1, t2, diff_t);

	int sec = diff_t.tv_sec;
	int ns  = diff_t.tv_nsec;

	printf("[*] EXEC TIME\t%d.%d\n", sec, ns);
        cout << "[+] decryptedFile created!" << endl;

        return 0;
}

void decrypt_cbc(FILE *infile, FILE *decryptedFile, uint32_t rounds, uint32_t keys[])
{
        int first = 1;
        uint32_t left, right;
        size_t ret;
        uint64_t sblock, sblock_prev, saved;
        cout << "[*] Decrypting ";
        while (!feof(infile))
        {
                cout << ". ";
                memset(&sblock, 0, BLOCK_SIZE);
                ret = fread(&sblock, 1, BLOCK_SIZE, infile);
                if (!ret)
                        break;
                saved = sblock;
                left = (sblock >> 32) & 0xFFFFFFFF;
                right = sblock & 0xFFFFFFFF;
                sblock = decrypt(left, right, ROUNDS, keys);
                if (first)
                {
                        sblock ^= 0xF0CCFACE;
                        first = 0;
                }
                else
                {
                        sblock ^= sblock_prev;
                }
                /* CBC */
                sblock_prev = saved;
                fwrite(&sblock, 1, BLOCK_SIZE, decryptedFile);
        }
        cout << endl;
}
uint32_t f(uint32_t block, uint32_t key)
{
        return block ^ key;
}

uint64_t decrypt(uint32_t left, uint32_t right, uint32_t rounds, uint32_t keys[])
{
        uint32_t i, left1, right1;

        for (i = 0; i < rounds; i++)
        {
                left1 = f(left, keys[rounds - i - 1]) ^ right;
                right1 = left;
                if (i == (rounds - 1))
                {
                        left = right1;
                        right = left1;
                }
                else
                {
                        left = left1;
                        right = right1;
                }
        }
        return (uint64_t)left << 32 | right;
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