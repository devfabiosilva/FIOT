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
#include <ctype.h>

#include "../include/f_add_bn_288_le.h"
#include "../include/nano_dpow_server_util.h"
#include "../include/libsodium/crypto_generichash.h"

int f_reverse(unsigned char *val, size_t val_sz)
{

   unsigned char *val_tmp, *p1, *p2;
   size_t count;

   val_tmp=malloc(val_sz);

   if (!val_tmp)
      return 2002;

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
int str_wallet_to_alphabet_index(uint8_t *list, const char *str_wallet, size_t str_sz)
{
   int err;
   int i, j;
   const char alphabet[]="13456789abcdefghijkmnopqrstuwxyz";

   for (j=0;j<str_sz;j++) {

      err=2032;

      for (i=0;i<(sizeof(alphabet)-1);i++)
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

int nano_base_32_2_hex(uint8_t *res, const char *str_wallet)
{
   int i, j;
   uint8_t a, b, c;
   uint8_t list_str_wallet[LIST_STR_WALLET];
   uint8_t buf[5];
   uint8_t *fp;
   F_ADD_288 displace;

   if ((i=(int)(strnlen(str_wallet, STR_NANO_SZ)))==STR_NANO_SZ)
      return 2041;

   if (i<64)
      return 2040;

   if (memcmp(str_wallet, NANO_PREFIX, sizeof(NANO_PREFIX)-1)) {

      if (memcmp(str_wallet, XRB_PREFIX, sizeof(XRB_PREFIX)-1))
         return 2039;

      if (i^64)
         return 2038;

      str_wallet+=4;

   } else {

      if (i^65)
         return 2037;

      str_wallet+=5;

   }

   if (str_wallet_to_alphabet_index(list_str_wallet, str_wallet, 52))
      return 2017;

   i=0;

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
      return 2019;

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
      return 2020;

   if (f_reverse((unsigned char *)fp, 33))
      return 2021;

   memset(displace, 0, sizeof(displace));

   memcpy(displace, fp, 33);

   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);

   if (f_reverse((unsigned char *)displace, sizeof(displace)))
      return 2022; 

   memcpy(fp, ((uint8_t *)displace)+3, 32);

   crypto_generichash((unsigned char *)buf, 5, fp, 32, NULL, 0);

   if (memcmp(fp+33, buf, 5))
      return 2023;

   return 0;

}
// 0 success/ error=non zero
int valid_nano_wallet(const char *wallet)
{

   int err;
   uint8_t *res;

   if (!(res=malloc(LIST_STR_WALLET)))
      return 2000;

   err=nano_base_32_2_hex(res, wallet);

   memset(res, 0, LIST_STR_WALLET);
   free(res);

   return err;

}
// 0 on success/ error=non zero
//#define MAX_RAW_STR_BALANCE_SZ (size_t)40//round up log10(2^128) + 1 (1 for null string)
int valid_raw_balance(const char *balance)
{

   int err;
   size_t balance_sz;

   if ((balance_sz=strnlen(balance, MAX_STR_RAW_BALANCE_MAX))==MAX_STR_RAW_BALANCE_MAX)
      return 2030;

   if (balance_sz==0)
      return 2031;

   err=0;

   for (;balance_sz;) {

      if (isdigit((int)balance[--balance_sz]))
         continue;

      err=2032;

      break;

   }

   return err;

}

// !!! hex_stream must be at least (size(str)-1)/2. No error sanity check!!! (Fabio)
// 0 on success; error otherwise
int f_str_to_hex(uint8_t *hex_stream, const char *str)
{

   char ch;
   size_t len=strlen(str);
   size_t i;

   for (i=0;i<len;i++) {
      ch=str[i];

      if (ch>'f')
         return 311;

      if (ch<'0')
         return 312;

      ch-='0';

      if (ch>9) {
         if (ch&0x30) {

            if ((ch&0x30)==0x20)
               return 314;

            ch&=0x0F;

            ch+=9;

            if (ch<10)
               return 315;
            if (ch>15)
               return 316;

         } else
            return 313;
      }

      (i&1)?(hex_stream[i>>1]|=(uint8_t)ch):(hex_stream[i>>1]=(uint8_t)(ch<<4));
   }

   return 0;

}

// No sanity err check. res must be at least 2*buf_sz+1 in size
char *fhex2strv2(char *res, const void *buf, size_t buf_sz, int is_uppercase)
{

   char *p=res;
   const char *f[]={"%02x","%02X"};
   const char *q=f[is_uppercase&1];

   for (;buf_sz--;) {

      sprintf(p, q, (unsigned char)*((unsigned char *)buf++));
      p+=2;

   }

   return res;

}

