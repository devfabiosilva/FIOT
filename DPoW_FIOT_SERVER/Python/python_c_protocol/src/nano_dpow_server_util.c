/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/


//Seg 23 Dez 2019 19:14:48 -03 
//From Fenix-IoT library for IA64 / ARM / Xtensa Compatible

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "../include/f_add_bn_288_le.h"
#include "../include/nano_dpow_server_util.h"
#include "../include/libsodium/crypto_generichash.h"

int f_reverse(unsigned char *val, size_t val_sz)
{

   unsigned char *val_tmp, *p1, *p2;
   size_t count;

   val_tmp=malloc(val_sz);

   if (!val_tmp)
      return 2;

   p1=val+val_sz-1;
   p2=val_tmp;
   count=val_sz;

   for (;count--;)
      *(p2++)=*(p1--);

   memcpy(val, val_tmp, val_sz);

   memset(val_tmp, 0, val_sz);

   free(val_tmp);

   return 0;

}

#define LIST_STR_WALLET (size_t)56
int str_wallet_to_alphabet_index(uint8_t *list, char *str_wallet, size_t str_sz)
{
   int err;
   int i, j;
   const char alphabet[]="13456789abcdefghijkmnopqrstuwxyz";

   for (j=0;j<str_sz;j++) {

      err=32;

      for (i=0;i<32;i++)
         if (alphabet[i]==str_wallet[j]) {
            err=0;
            list[j]=(uint8_t)i;
            break;
         }

      if (err)
         return err;

   }

   return err;

}

int nano_base_32_2_hex(uint8_t *res, char *str_wallet)
{
   int i, j;
   uint8_t a, b, c;
   uint8_t list_str_wallet[LIST_STR_WALLET];
   uint8_t buf[5];
   uint8_t *fp;
   F_ADD_288 displace;

   if (memcmp(str_wallet, NANO_PREFIX, (size_t)(i=(int)(sizeof(NANO_PREFIX)-1)))) {

      if (memcmp(str_wallet, XRB_PREFIX, (size_t)(i=(int)(sizeof(XRB_PREFIX)-1))))
         return 37;
      else
         str_wallet+=4;

   } else
      str_wallet+=5;

   if (str_wallet_to_alphabet_index(list_str_wallet, str_wallet, 52))
      return 17;

   list_str_wallet[52]=0;

   fp=res;

   for (j=0;j<6;j++) {

      a=list_str_wallet[i++];
      b=list_str_wallet[i++];
      *(res++)=((a<<3)|(b>>2));

      a=list_str_wallet[i++];
      c=list_str_wallet[i++];
      *(res++)=((b<<6)|(a<<1)|(c>>4));

      a=list_str_wallet[i++];
      *(res++)=((c<<4)|(a>>1));

      b=list_str_wallet[i++];
      c=list_str_wallet[i++];

      *(res++)=((a<<7)|(b<<2)|(c>>3));

      a=list_str_wallet[i++];
      *(res++)=(a|(c<<5));

   }

   a=list_str_wallet[i++];
   b=list_str_wallet[i++];
   *(res++)=((a<<3)|(b>>2));

   a=list_str_wallet[i++];
   c=list_str_wallet[i++];
   *(res++)=((b<<6)|(a<<1)|(c>>4));

   a=list_str_wallet[i];
   *(res++)=((c<<4)|(a>>1));

   if (str_wallet_to_alphabet_index(list_str_wallet, str_wallet+52, 8))
      return 19;

   i=0;

   a=list_str_wallet[i++];
   b=list_str_wallet[i++];
   *(res++)=((a<<3)|(b>>2));

   a=list_str_wallet[i++];
   c=list_str_wallet[i++];
   *(res++)=((b<<6)|(a<<1)|(c>>4));

   a=list_str_wallet[i++];
   *(res++)=((c<<4)|(a>>1));

   b=list_str_wallet[i++];
   c=list_str_wallet[i++];

   *(res++)=((a<<7)|(b<<2)|(c>>3));

   a=list_str_wallet[i];
   *res=(a|(c<<5));

   if (f_reverse((unsigned char *)fp+33, 5))
      return 20;

   if (f_reverse((unsigned char *)fp, 33))
      return 21;

   memset(displace, 0, sizeof(displace));

   memcpy(displace, fp, 33);

   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);

   if (f_reverse((unsigned char *)displace, sizeof(displace)))
      return 22; 

   memcpy(fp, ((uint8_t *)displace)+3, 32);

   crypto_generichash((unsigned char *)buf, 5, fp, 32, NULL, 0);

   if (memcmp(fp+33, buf, 5))
      return 23;

   return 0;

}

