/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

// This tool generates a Lookup table of NANO cryptocurrency factor adjust to convert any value to RAW 128bit to be used in FIOT firmware
// Don't modify any parameter below
//Oct 13 2019
#include <stdio.h>
#include "mbedtls/bignum.h"

typedef uint8_t uint128_t[16];

int main(int argc, char **argv)
{

   int err, i;
   uint128_t val[30+1];
   FILE *f;

   mbedtls_mpi X, A;

   mbedtls_mpi_init(&X);
   mbedtls_mpi_init(&A);

   if (mbedtls_mpi_lset(&A, 1)) {
      err=1;
      goto main_EXIT;
   }

   if (mbedtls_mpi_write_binary(&A, val[29+1], sizeof(uint128_t))) {
      err=5;
      goto main_EXIT;
   }

   for (i=1;i<(30+1);i++) {
      if (mbedtls_mpi_mul_int(&X, &A, 10)) {
         err=2;
         break;
      }
      if (mbedtls_mpi_write_binary(&X, val[29+1-i], sizeof(uint128_t))) {
         err=4;
         break;
      }
      if (mbedtls_mpi_copy(&A, &X)) {
         err=3;
         break;
      }
   }

   if ((f=fopen("nanobgbe.dat", "w"))==NULL) {
      err=6;
      goto main_EXIT;
   }

   err=0;

   if (fwrite(val,1,sizeof(val),f)^sizeof(val))
      err=7;

   fclose(f);

main_EXIT:
   mbedtls_mpi_free(&A);
   mbedtls_mpi_free(&X);
   if (err)
      printf("\nErr number %d\n", err);
   else
      printf("\nSuccess\n");

   return err;
}
