#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define ROUNDS 8

using namespace std;
uint32_t f(uint32_t block, uint32_t key);
uint64_t decrypt(uint32_t left, uint32_t right, uint32_t rounds, uint32_t keys[]);
void decrypt_ecb(FILE *infile, FILE *outfile, uint32_t rounds, uint32_t keys[]);


string keys[16] = {"0xF124FCAD",
"0xFF91B5F5",
"0xF9281A0E",
"0x84282A36",
"0xE8D63C4A",
"0x0C402C6F",
"0x2296CB30",
"0x9FF9D76E",
"0x243A5572",
"0xA4AE9DD0",
"0x999F201E",
"0x9A0CB9A5",
"0x349968F5",
"0x62FD58D0",
"0x339DFC3C",
"0x4815AD1E"};

int main() {
  auto words = { "Hello, ", "World!", "\n" };
  for (const string& word : words) {
    cout << word;
  }
  return 0;
}



void decrypt_ecb(FILE *infile, FILE *outfile, uint32_t rounds, uint32_t keys[]) {
        uint32_t left, right;
        size_t ret;
        uint64_t sblock;
        while(!feof(infile)) {
                memset(&sblock,0,sizeof(sblock));
                ret = fread(&sblock,1,sizeof(sblock),infile);
                if(!ret) break;
                left = (sblock>>32) & 0xFFFFFFFF;
                right = sblock & 0xFFFFFFFF;
                sblock = decrypt(left,right,ROUNDS,keys);
                ret = fwrite(&sblock,1,sizeof(sblock),outfile);
        }
}
uint32_t f(uint32_t block, uint32_t key) {
        return block ^ key;
}

uint64_t decrypt(uint32_t left, uint32_t right, uint32_t rounds, uint32_t keys[]) {
        uint32_t i, left1, right1;
        for(i = 0;i < rounds;i++) {
                left1 = f(left,keys[rounds-i-1]) ^ right;
                right1 = left;
                if(i == (rounds-1)) {
                        left = right1;
                        right = left1;
                } else {
                        left = left1;
                        right = right1;
                }
        }
        return (uint64_t)left<<32 | right;
}
