/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

// Implementation of big number 288 bit used in Nano Cryptocurrency C Library

#include <stdint.h>
typedef uint8_t F_ADD_288[36];
void f_add_bn_288_le(F_ADD_288, F_ADD_288, F_ADD_288, int *, int);

