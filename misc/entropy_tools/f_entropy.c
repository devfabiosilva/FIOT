/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Qua 18 Set 2019 00:41:26 -03 
//#define F_GENERATE

// This tools synthesizes a good calculation of Entropy of FIOT TRNG Hardware. Also there is a implementation to calculate entropy eficiently using
// lookup table. All syntheses here is based in equation 7.12 of this amazing MIT opencourseware (7.3 A Statistical Definition of Entropy)
// https://web.mit.edu/16.unified/www/FALL/thermodynamics/notes/node56.html
// Many thanks to Professor Z. S. Spakovszky
// File containing a lookup table is a raw binary.
// Required: libopenssl dev

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#define F_LOG_MAX 8*256
// 554517
#define F_INF (int) 0x7FFFFFFF

#ifdef F_GENERATE
static int F_LOG_ARRAY[F_LOG_MAX+1];
#else
#include <string.h>
#include <openssl/rand.h>
extern const int F_LOG_ARRAY[F_LOG_MAX+1] asm("_binary_f_log_dat_start");
static unsigned int f_entropy_val[256];//={1};
static unsigned char testa[F_LOG_MAX];//={0xff};

#endif

#define F_LOG_ARRAY_SZ (size_t)sizeof(F_LOG_ARRAY)/sizeof(int)

int main(int argc, char **argv)
{
   int i;
   unsigned long int final = 0;
#ifdef F_GENERATE
   F_LOG_ARRAY[0]=F_INF;
   FILE *f;

   f=fopen("f_log.dat", "w");

   for (i=1;i<F_LOG_ARRAY_SZ;i++)
      F_LOG_ARRAY[i]=(int)(-131072*log(((double)i)/((double)F_LOG_MAX))); //131072=2^17

   if (fwrite(F_LOG_ARRAY, 1, sizeof(F_LOG_ARRAY), f)^sizeof(F_LOG_ARRAY))
      printf("Error when writing data");

   fclose(f);
#else


   if (RAND_bytes(testa, sizeof(testa))!=1) {
      printf("\nErro \"RAND_bytes\"\n");
      return 1;
   }


//   memset(testa,0,sizeof(testa));
   memset(f_entropy_val,0,sizeof(f_entropy_val));
   //memset(testa, 0x10, sizeof(testa));

   for (i=0;i<F_LOG_MAX;i++) {
      f_entropy_val[testa[i]]+=1;
      //printf("\nTESTE %d\n", testa[i]);
   }

//for (i=0;i<256;i++)
//  printf("\nValor %d\n", f_entropy_val[test[i]]);


   for (i=0;i<256;i++)
      final+=f_entropy_val[i]*F_LOG_ARRAY[f_entropy_val[i]];

//   final=28;

   printf("\nFinal = %lu\n", final);
//1488522235 // Ideal
//1471843494
//1474211579 // desired
//1471531015// Good
//1470001808// not enough
// <1470001808 Weak
// <1469703345 // Not acceptable
//https://web.mit.edu/16.unified/www/FALL/thermodynamics/notes/node56.html
#endif
   return 0;
}
