/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

// Compatible 16 bit align dictionary format for FIOT Nano Cryptocurrency
// It converts a bip39 dictionary plain text to a FIOT Nano Cryptocurrency format dictionary.dic

//Oct 19 2019

#include <stdio.h>
#include <string.h>
/*
int f_file_size(FILE *f, size_t *file_size)
{
   long f_sz;
   int err;

   if (!f)
      return 2;

   err=fseek(f, 0L, SEEK_END);

   if (err)
      return err;

   f_sz=ftell(f);

   if ((long)f_sz==(long)-1)
      return 1;

   err=fseek(f, 0L, SEEK_SET);

   if (err)
      return err;

   *file_size=(size_t)f_sz;
   
   return err;
}
*/

int main(int argc, char **argv)
{
   FILE *f, *g;
   char word[16];
   char str[16];
   size_t n,m;
   int err=0;

   f=fopen("dictionary.txt", "r");

   if (!f)
      return 1;
//char *fgets(char *s, int size, FILE *stream);

   g=fopen("dictionary.dic", "w");

   if (!g) {
      fclose(f);
      return 2;
   }
   m=0;
   for (;fgets(str, sizeof(word), f);) {
      n=strlen(str);
      if (str[n-1]=='\n')
         str[n-1]=0;

      m+=n;

      if (fseek(f, m, SEEK_SET)) {
         err=5;
         goto exit;
      }
      strncpy(word, str, sizeof(word));
      /*if (ptr = strchr(buf,'\n'))
        *ptr = 0;*/
      if (fwrite(word, 1, sizeof(word), g)^sizeof(word)) {
         err=3;
         goto exit;
      }
   }

exit:
   fclose(g);
   fclose(f);

   return err;
}
