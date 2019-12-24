/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Seg 23 Dez 2019 19:20:27 -03

#define NANO_PREFIX "nano_"
#define XRB_PREFIX "xrb_"

#define MAX_STR_NANO_CHAR (size_t)70 //5+56+8+1
#define MAX_STR_RAW_BALANCE_MAX (size_t)40
#define STR_NANO_SZ (size_t)66// 65+1 Null included

#define MSG_ERR_INVALID_NANO_WALLET "\nInvalid NANO WALLET\n"
#define MSG_ERR_INVALID_NANO_RAW_BALANCE "\nInvalid NANO Raw Balance. Is it a valid 128 bit big number?\n"

int valid_nano_wallet(const char *);
int valid_raw_balance(const char *);

