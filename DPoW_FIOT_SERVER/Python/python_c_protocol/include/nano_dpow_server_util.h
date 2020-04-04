/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Seg 23 Dez 2019 19:20:27 -03

typedef uint8_t f_uint128_t[16];

#define NANO_PREFIX "nano_"
#define XRB_PREFIX "xrb_"
#define LIST_STR_WALLET (size_t)56
#define MAX_STR_NANO_CHAR (size_t)70 //5+56+8+1
#define MAX_STR_RAW_BALANCE_MAX (size_t)40
#define STR_NANO_SZ (size_t)66// 65+1 Null included
#define MAX_RAW_DATA_FRONTIER (size_t)32
#define MAX_RAW_DATA_HASH (size_t)32
#define MAX_STR_DATA_FRONTIER (size_t)65
#define MAX_STR_DATA_HASH_VALUE (size_t)65

typedef struct f_block_transfer_t {
   uint8_t preamble[32];
   uint8_t account[32];
   uint8_t previous[32];
   uint8_t representative[32];
   f_uint128_t balance;
   uint8_t link[32];
   uint8_t signature[64];
   uint8_t prefixes;
} __attribute__((packed)) F_BLOCK_TRANSFER;

int valid_nano_wallet(const char *);
int valid_raw_balance(const char *);
int f_str_to_hex(uint8_t *, const char *);
char *fhex2strv2(char *, const void *, size_t, int);
int nano_base_32_2_hex(uint8_t *, const char *);
int is_filled_with_value(uint8_t *, size_t, uint8_t);
int is_null_hash(uint8_t *hash);
int f_parse_p2pow_block_to_json(char *, size_t *, size_t, F_BLOCK_TRANSFER *, F_BLOCK_TRANSFER *);

