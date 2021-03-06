/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Seg 23 Dez 2019 19:20:27 -03

#define PUB_KEY_EXTENDED_MAX_LEN (size_t)40
#define NANO_PREFIX "nano_"
#define XRB_PREFIX "xrb_"
#define REP_XRB (uint8_t)0x4
#define SENDER_XRB (uint8_t)0x02
#define WORKER_XRB (uint8_t)0x08
#define DEST_XRB (uint8_t)0x01
#define LIST_STR_WALLET (size_t)56
#define MAX_STR_NANO_CHAR (size_t)70 //5+56+8+1
#define MAX_STR_RAW_BALANCE_MAX (size_t)40
#define STR_NANO_SZ (size_t)66// 65+1 Null included
#define MAX_RAW_DATA_FRONTIER (size_t)32
#define MAX_RAW_DATA_HASH (size_t)32
#define MAX_STR_DATA_FRONTIER (size_t)65
#define MAX_STR_DATA_HASH_VALUE (size_t)65

typedef uint8_t NANO_PUBLIC_KEY_EXTENDED[PUB_KEY_EXTENDED_MAX_LEN];
typedef uint8_t f_uint128_t[16];

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

typedef struct f_p2pow_req_info_t {
   uint8_t fee[sizeof(f_uint128_t)];
   float max_mult;
   float min_mult;
   uint8_t reward_account_prefix;
   uint8_t reward_account[32];
   uint32_t version;
} __attribute__((packed)) P2POW_REQ_INFO;

typedef struct f_p2pow_signed_blk_res_t {
    uint8_t trans_hash[32];
    uint64_t trans_work;
    uint8_t worker_hash[32];
    uint64_t worker_work;
} __attribute__((packed)) P2POW_REQ_RESULT;

int valid_nano_wallet(const char *);
int valid_raw_balance(const char *);
int f_str_to_hex(uint8_t *, const char *);
char *fhex2strv2(char *, const void *, size_t, int);
int nano_base_32_2_hex(uint8_t *, const char *);
int is_filled_with_value(uint8_t *, size_t, uint8_t);
int is_null_hash(uint8_t *hash);
int f_parse_p2pow_block_to_json(char *, size_t *, size_t, F_BLOCK_TRANSFER *, F_BLOCK_TRANSFER *);
int pk_to_wallet(char *, char *, NANO_PUBLIC_KEY_EXTENDED);
int f_nano_parse_raw_str_to_raw128_t(uint8_t *, const char *);
int is_nano_prefix(const char *, const char *);

