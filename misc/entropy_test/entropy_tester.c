/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

// Tester of implementation of equation 7.12 (7.3 A Statistical Definition of Entropy)
// https://web.mit.edu/16.unified/www/FALL/thermodynamics/notes/node56.html
// Many thanks to Professor Z. S. Spakovszky

//Sáb 21 Set 2019 23:53:26 -03 
#include <stdio.h>
#include <string.h>

#define F_LOG_MAX 8*256
#define F_ENTROPY_TYPE_PARANOIC (unsigned int)1477500000//1476682819
#define F_ENTROPY_TYPE_EXCELENT (unsigned int)1475885281
#define F_ENTROPY_TYPE_GOOD (unsigned int)1471531015
#define F_ENTROPY_TYPE_NOT_ENOUGH (unsigned int)1470001808
#define F_ENTROPY_TYPE_NOT_RECOMENDED (unsigned int)1469703345

unsigned long int f_test_entropy(unsigned char *rand_data, size_t rand_data_sz/*, unsigned int type*/)
{
   extern const int F_LOG_ARRAY[F_LOG_MAX+1] asm("_binary_f_log_dat_start");
   unsigned int f_entropy_val[256];
   unsigned long int final;
   size_t i;

//f_test_entropy_RET:
   final=0;

   memset(f_entropy_val,0,sizeof(f_entropy_val));

   for (i=0;i<rand_data_sz;i++)
      f_entropy_val[rand_data[i]]+=1;

   for (i=0;i<256;i++)
      final+=f_entropy_val[i]*F_LOG_ARRAY[f_entropy_val[i]];

//   if ((unsigned long int)type>final)
//      goto f_test_entropy_RET;

   return final;
}


int main(int argc, char **argv)
{
   FILE *f;
   unsigned char msg[F_LOG_MAX];
   unsigned long int parm=(unsigned long int)F_ENTROPY_TYPE_PARANOIC;
   unsigned long int final;

   printf("\nReading \"stdin\" ... and calculating entropy data ...\n");

main_RET:

   if (fread(msg, 1, sizeof(msg), stdin)^sizeof(msg)) {
      printf("\nCould no read data\n");
      printf("\nStream of data must be at least %d bytes\n", F_LOG_MAX);
      return 1;
   }

   if (parm>(final=f_test_entropy(msg, sizeof(msg))))
      goto main_RET;

   printf("\nEntropy result: %lu\n", final);

   fclose(stdin);
   return 0;
} 
