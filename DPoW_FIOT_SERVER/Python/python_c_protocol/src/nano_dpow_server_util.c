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

#include "f_add_bn_288_le.h"
#include "nano_dpow_server_util.h"
#include "crypto_generichash.h"
#include "bignum.h"

const char alphabet[]="13456789abcdefghijkmnopqrstuwxyz";
const char P2POW_JSON_FORMAT[]="{\"user_transaction\":\"%0\",\"reward_transaction\":\"%1\"}";
const char JSON_NANO_TRANSACTION[]="{\"action\":\"process\",\"json_block\":\"true\",\"block\":{\"type\":\"state\",\
\"account\":\"%1\",\"previous\":\"%2\",\"representative\":\"%3\",\"balance\":\"%4\",\"link\":\"%5\",\"link_as_account\"\
:\"%6\",\"signature\":\"%7\"}}";

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

//#define LIST_STR_WALLET (size_t)56
int str_wallet_to_alphabet_index(uint8_t *list, const char *str_wallet, size_t str_sz)
{
   int err;
   int i, j;

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

// return 1 if is filled with value or 0 if not
int is_filled_with_value(uint8_t *value, size_t value_sz, uint8_t ch)
{

   int res=1;

   for (;value_sz;) {

      if (value[--value_sz]==ch)
         continue;

      res=0;

      break;

   }

   return res;

}

inline int is_null_hash(uint8_t *hash)
{
   return is_filled_with_value(hash, MAX_RAW_DATA_HASH, 0);
}

int f_find_str(size_t *pos, char *str, size_t str_sz, char *what_find)
{
   int err;
   size_t what_find_sz, i;
   char *p;

   if ((what_find_sz=strlen(what_find))>str_sz)
      return 1;

   p=str;

   err=2;

   for (i=0;i<=(str_sz-what_find_sz);i++) {
      if (memcmp(p++, what_find, what_find_sz))
         continue;
      p--;
      err=0;
      break;
   }

   if (err==0)
      if (pos)
         *pos=(p-str);

   return err;
}

int f_find_replace(char *dest_buf, size_t *out_len, size_t dest_buf_sz, char *str, size_t str_sz, char *what_find, char *what_replace)
{
   int err;
   size_t pos, tmp_dest_buf_sz;
   size_t what_replace_sz, what_find_sz, tmp;
   char *p_str;
   char *p_dest;

   err=f_find_str(&pos, str, str_sz, what_find);

   if (err)
      return err;

   what_replace_sz=strlen(what_replace);

   tmp=what_replace_sz+pos;

   if (tmp>dest_buf_sz)
      return 40;

   p_dest=dest_buf;
   p_str=str;

   if (pos) {
      memcpy(p_dest, p_str, pos);
      p_dest+=pos;
   }

   if (what_replace_sz) {
      memcpy(p_dest, what_replace, what_replace_sz);

      p_dest+=what_replace_sz;

   }

   what_find_sz=strlen(what_find);

   tmp_dest_buf_sz=tmp;

   tmp=pos+what_find_sz;

   p_str+=tmp;

   tmp=(str_sz-tmp);

   if (tmp==0)
      goto f_find_replacef_find_replace_EXIT;

   tmp_dest_buf_sz+=tmp;

   if (tmp_dest_buf_sz>dest_buf_sz)
      return 41;

   memcpy(p_dest, p_str, tmp);

f_find_replacef_find_replace_EXIT:
   if (out_len) {
      *out_len=tmp_dest_buf_sz;
      return 0;
   }

   tmp_dest_buf_sz++;

   if (tmp_dest_buf_sz>dest_buf_sz)
      return 42;

   *(p_dest+=tmp)=0;

   return 0;
}

int pk_to_wallet(char *out, char *prefix, NANO_PUBLIC_KEY_EXTENDED pubkey_extended)
{

   uint8_t a, b;
   size_t i, count, pos;
   int err;
   char *fp;
   F_ADD_288 displace;

   pos=strlen(prefix);

   if (4>pos)
      return 1;

   crypto_generichash((unsigned char *)(count=(size_t)pubkey_extended+35), 5, pubkey_extended, 32, NULL, 0);

   if ((err=f_reverse((unsigned char *)count, 5)))
      return err;

   memset(displace, 0, sizeof(F_ADD_288));

   if ((err=f_reverse(pubkey_extended, 32)))
      return err;

   memcpy(displace, pubkey_extended, 32);

   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);
   f_add_bn_288_le(displace, displace, displace, NULL, 0);

   f_reverse(displace, sizeof(displace));

   memcpy(pubkey_extended, ((uint8_t *)displace)+1, sizeof(F_ADD_288)-1);

   count=0;

   fp=out;

   out+=(pos-3);

   for (i=0;i<8;i++) {

      a=pubkey_extended[count++];
      *(out++)=alphabet[(a>>3)];

      b=pubkey_extended[count++];
      *(out++)=alphabet[(((a&0x07)<<2)|(b>>6))];
      *(out++)=alphabet[((b>>1)&0x1F)];

      a=pubkey_extended[count++];
      *(out++)=alphabet[(((b&0x01)<<4)|(a>>4))];

      b=pubkey_extended[count++];
      *(out++)=alphabet[(((a&0x0F)<<1)|(b>>7))];
      *(out++)=alphabet[((b>>2)&0x1F)];

      a=pubkey_extended[count++];
      *(out++)=alphabet[(((b&0x03)<<3)|(a>>5))];
      *(out++)=alphabet[(a&0x1F)];

   }

   out--;

   memcpy(out-8,out-7,8);

   *out=0;

   memcpy(fp, prefix, pos);

   return 0;
}

//No sanity check (for internal use only) april 4 2020
int f_nano_balance_to_str_util(char *str, size_t str_len, f_uint128_t value)
{
   int err;
   mbedtls_mpi *X;
   size_t out_len_tmp;

   X=malloc(sizeof(mbedtls_mpi));

   if (!X)
      return 203;

   mbedtls_mpi_init(X);

   if (mbedtls_mpi_read_binary(X, value, sizeof(f_uint128_t))) {
      err=204;
      goto f_nano_balance_to_str_EXIT2;
   }

   (mbedtls_mpi_write_string(X, 10, str, str_len, &out_len_tmp))?(err=(int)out_len_tmp):(err=0);

f_nano_balance_to_str_EXIT2:
   mbedtls_mpi_free(X);

   free(X);

   return err;
}

#define F_P2POW_BUF_SZ (size_t)8192
int f_parse_block_transfer_to_json(char *dest, size_t *olen, size_t dest_sz, F_BLOCK_TRANSFER *block_transfer)
{

   int err;
   size_t sz;
   uint8_t *buf;

   if (!(buf=malloc(F_P2POW_BUF_SZ+256)))
      return 153;

   if (pk_to_wallet((char *)(buf+F_P2POW_BUF_SZ-MAX_STR_NANO_CHAR), (block_transfer->prefixes&SENDER_XRB)?XRB_PREFIX:NANO_PREFIX,
      (uint8_t *)memcpy(buf+F_P2POW_BUF_SZ-sizeof(NANO_PUBLIC_KEY_EXTENDED)-MAX_STR_NANO_CHAR, block_transfer->account, 32))) {

      err=154;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (f_find_replace((char *)buf, NULL, (F_P2POW_BUF_SZ>>1), (char *)memcpy(buf+(F_P2POW_BUF_SZ>>1), JSON_NANO_TRANSACTION, sizeof(JSON_NANO_TRANSACTION)),
      sizeof(JSON_NANO_TRANSACTION)-1, "%1", (char *)(buf+F_P2POW_BUF_SZ-MAX_STR_NANO_CHAR))) {

      err=155;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (f_find_replace((char *)(buf+(F_P2POW_BUF_SZ>>1)), NULL, (F_P2POW_BUF_SZ>>1), (char *)buf, strnlen((const char *)buf, (F_P2POW_BUF_SZ>>1)-1), "%2",
      fhex2strv2((char *)(buf+F_P2POW_BUF_SZ-128), block_transfer->previous, 32, 1))) {

      err=156;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (pk_to_wallet((char *)(buf+F_P2POW_BUF_SZ-MAX_STR_NANO_CHAR), (block_transfer->prefixes&REP_XRB)?XRB_PREFIX:NANO_PREFIX,
      (uint8_t *)memcpy(buf+F_P2POW_BUF_SZ-sizeof(NANO_PUBLIC_KEY_EXTENDED)-MAX_STR_NANO_CHAR, block_transfer->representative, 32))) {

      err=157;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (f_find_replace((char *)buf, NULL, (F_P2POW_BUF_SZ>>1), (char *)(buf+(F_P2POW_BUF_SZ>>1)), strnlen((const char *)(buf+(F_P2POW_BUF_SZ>>1)),
      (F_P2POW_BUF_SZ>>1)-1), "%3", (char *)(buf+F_P2POW_BUF_SZ-MAX_STR_NANO_CHAR))) {

      err=158;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (f_nano_balance_to_str_util((char *)(buf+F_P2POW_BUF_SZ-256), 256, block_transfer->balance)) {

      err=159;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (f_find_replace((char *)(buf+(F_P2POW_BUF_SZ>>1)), NULL, (F_P2POW_BUF_SZ>>1), (char *)buf, strnlen((const char *)buf, (F_P2POW_BUF_SZ>>1)-1),
      "%4", (char *)(buf+F_P2POW_BUF_SZ-256))) {

      err=160;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (f_find_replace((char *)buf, NULL, (F_P2POW_BUF_SZ>>1), (char *)(buf+(F_P2POW_BUF_SZ>>1)), strnlen((const char *)(buf+(F_P2POW_BUF_SZ>>1)),
      (F_P2POW_BUF_SZ>>1)-1), "%5", fhex2strv2((char *)(buf+F_P2POW_BUF_SZ-128), block_transfer->link, 32, 1))) {

      err=161;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (pk_to_wallet((char *)(buf+F_P2POW_BUF_SZ-MAX_STR_NANO_CHAR), (block_transfer->prefixes&SENDER_XRB)?XRB_PREFIX:NANO_PREFIX,
      (uint8_t *)memcpy(buf+F_P2POW_BUF_SZ-sizeof(NANO_PUBLIC_KEY_EXTENDED)-MAX_STR_NANO_CHAR, block_transfer->link, 32))) {

      err=162;

      goto parse_p2pow_to_json_EXIT1;

   } 

   if (f_find_replace((char *)(buf+(F_P2POW_BUF_SZ>>1)), NULL, (F_P2POW_BUF_SZ>>1), (char *)buf, strnlen((const char *)buf, (F_P2POW_BUF_SZ>>1)-1),
      "%6", (char *)(buf+F_P2POW_BUF_SZ-MAX_STR_NANO_CHAR))) {

      err=163;

      goto parse_p2pow_to_json_EXIT1;

   }

   if (f_find_replace((char *)buf, &sz, (F_P2POW_BUF_SZ>>1), (char *)(buf+(F_P2POW_BUF_SZ>>1)), strnlen((const char *)(buf+(F_P2POW_BUF_SZ>>1)),
      (F_P2POW_BUF_SZ>>1)-1), "%7", fhex2strv2((char *)(buf+F_P2POW_BUF_SZ-256), block_transfer->signature, 64, 0))) {

      err=164;

      goto parse_p2pow_to_json_EXIT1;

   }

   (olen)?(*olen=sz):(buf[sz++]=0);

   err=0;

   (sz>dest_sz)?(void)(err=165):(void *)(memcpy(dest, buf, sz));

parse_p2pow_to_json_EXIT1:
   memset(buf, 0, F_P2POW_BUF_SZ+256);
   free(buf);

   return err;

}

int f_parse_p2pow_block_to_json(char *str, size_t *olen, size_t str_sz, F_BLOCK_TRANSFER *user_transaction, F_BLOCK_TRANSFER *transaction_fee)
{

   int err;
   uint8_t *buf;
   size_t sz_tmp;

   if (!(buf=malloc(2*F_P2POW_BUF_SZ)))
      return 170;

   if (f_parse_block_transfer_to_json((char *)buf, &sz_tmp, (F_P2POW_BUF_SZ>>1), user_transaction)) {

      err=171;

      goto f_parse_p2pow_block_to_json_EXIT1;

   }

   if (transaction_fee) {

      buf[sz_tmp]=0;

      if (f_find_replace((char *)(buf+(F_P2POW_BUF_SZ>>1)), NULL, (F_P2POW_BUF_SZ>>1), (char *)P2POW_JSON_FORMAT, sizeof(P2POW_JSON_FORMAT)-1, "%0", 
         (char *)buf)) {

         err=172;
         goto f_parse_p2pow_block_to_json_EXIT1;

      }

      if (f_parse_block_transfer_to_json((char *)(buf+F_P2POW_BUF_SZ), NULL, F_P2POW_BUF_SZ, transaction_fee)) {

         err=173;
         goto f_parse_p2pow_block_to_json_EXIT1;

      }

      if (f_find_replace((char *)buf, &sz_tmp, (F_P2POW_BUF_SZ>>1), (char *)(buf+(F_P2POW_BUF_SZ>>1)), strnlen((char *)(buf+(F_P2POW_BUF_SZ>>1)),
         (F_P2POW_BUF_SZ>>1)), "%1", (char *)(buf+F_P2POW_BUF_SZ))) {

         err=174;

         goto f_parse_p2pow_block_to_json_EXIT1;

      }

   }

   (olen)?(*olen=sz_tmp):(buf[sz_tmp++]=0);

   err=0;

   (sz_tmp>str_sz)?(void)(err=175):(void *)(memcpy(str, buf, sz_tmp));

f_parse_p2pow_block_to_json_EXIT1:
   memset(buf, 0, 2*F_P2POW_BUF_SZ);
   free(buf);

   return err;

}

