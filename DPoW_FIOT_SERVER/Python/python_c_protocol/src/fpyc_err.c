/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Seg 23 Dez 2019 00:11:07 -03

#include <stdio.h>

char *fpyc_err_msg(char *msg, int err_msg)
{

   static char err_message[768];

   sprintf(err_message, "%s Internal FIOT C library error: %d\n", msg, err_msg);

   return err_message;

}
