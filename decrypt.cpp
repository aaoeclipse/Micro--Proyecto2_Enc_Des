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
#define CAPACIDAD_BLOQUE 8

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
    0x4815AD1E,
    0x7312DEAD};
int main(int argc, char *argv[])
{
        timespec t1, t2, diff_t;
        FILE *cryptFile, *decryptedFile;
        // Checks if arguments are correct
        if (argc != 2)
        {
                fprintf(stderr, "[-] Incorrect format!\n");
                fprintf(stderr, "$ ./[runfile] [cryptFile]\n");
                return EXIT_FAILURE;
        }
        // Tries to open file
        cryptFile = fopen(argv[1], "r");
        if (!cryptFile)
        {
                printf("[-] Error trying to open file: ");
                cout << argv[1] << endl;
                return EXIT_FAILURE;
        }
        
        cout << "[+] " << argv[1] << " found!" << endl;
        decryptedFile = fopen("decryptedFile.txt", "w");
        // Time
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
        decrypt_cbc(cryptFile, decryptedFile, ROUNDS, keys);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);

        fclose(cryptFile);
        fclose(decryptedFile);

        diff(t1, t2, diff_t);

        int sec = diff_t.tv_sec;
        int ns = diff_t.tv_nsec;

        printf("[*] EXEC TIME\t%d.%d\n", sec, ns);
        cout << "[+] decryptedFile created!" << endl;

        return 0;
}

void decrypt_cbc(FILE *infile, FILE *decryptedFile, uint32_t rounds, uint32_t keys[])
{       
        int primer = 1;
        uint32_t left, right;
        size_t resultado;
        uint64_t blque, blque_prev, saved;
        cout << "[*] Decrypting ";
        while (!feof(infile))   // Mientas no lleguemos al end of file
        {
                cout << ". ";
                resultado = fread(&blque, 1, CAPACIDAD_BLOQUE, infile);
                saved = blque;
                // Corremos 32 bits y hacemus un AND segun el algoritmo para conseguir el lado izquierdo 
                left = (blque >> 32) & 0xFFFFFFFF;
                // Se hace tambien el AND en el derecho con para conseguir el lado derecho
                right = blque & 0xFFFFFFFF;
                // Utilizamos la desencrypcion y utilizamos el left y right que sacamos anteriormente
                blque = decrypt(left, right, ROUNDS, keys);
                // Si es el primero nos puede dar un par de erroeres ya que no hay bloque pasado
                if (primer)
                {
                        blque ^= 0xF0CCFACE;
                        primer = 0;
                }
                else
                {
                        blque ^= blque_prev;
                }
                /* CBC */
                blque_prev = saved;
                // Escribimos el bloque de regreso en el archivo desencryptado
                fwrite(&blque, 1, CAPACIDAD_BLOQUE, decryptedFile);
        }
        cout << endl;
}
uint32_t xORfuncion(uint32_t block, uint32_t key)
{
        return block ^ key;
}

uint64_t decrypt(uint32_t left, uint32_t right, uint32_t rounds, uint32_t keys[])
{
        uint32_t tempLeft, tempRight;

        for (int i = 0; i < rounds; i++)
        {       
                // segun al algoritmo tL = (izquierda XOR llave) XOR derecha
                tempLeft = xORfuncion(left, keys[rounds - i - 1]) ^ right;
                // derecha = izquierda
                tempRight = left;
                if (i == (rounds - 1)) // es el ultimo?
                {
                        // Ya que es el ultimi, hacemos el criss-cross (cambiamos izquierda por derecha y vise versa)
                        left = tempRight;
                        right = tempLeft;
                }
                else // no es el ultimo
                {
                        left = tempLeft;
                        right = tempRight;
                }
        }
        // Por ultimo hacemos el corrimiento a la izquierda de 32 bits y hacemos un OR con el derecho
        return (uint64_t)left << 32 | right;
}

///medir tiempo.
int diff(timespec start, timespec end, timespec &diff)
{

        if ((end.tv_nsec - start.tv_nsec) < 0)
        {
                diff.tv_sec = end.tv_sec - start.tv_sec - 1;
                diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
        }
        else
        {
                diff.tv_sec = end.tv_sec - start.tv_sec;
                diff.tv_nsec = end.tv_nsec - start.tv_nsec;
        }
        return 0;
}