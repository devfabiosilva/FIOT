/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Sexta 20 Dez 2019 21:15:30 -03

char *fpyc_err_msg(char *, int);

typedef enum err_t {
   PyC_ERR_OK=0,
   PyC_ERR_BUFFER_ALLOC,
   PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS,
   PyC_ERR_MEM_OVFL,
   PyC_ERR_RAW_DATA_SZ_ZERO,
   PyC_ERR_DATA_OBJ_NOT_READY,
   PyC_ERR_DATA_OBJ_CREATE,
   PyC_ERR_DATA_OBJ_CREATE_ATTR,
   PyC_ERR_ADD_METHOD,
   PyC_ERR_NULL_OBJ,
   PyC_ERR_PUBLISH_ADDR_OVFL,
   PyC_ERR_F_NANO_TRANSACTION_RAW_DATA_SZ_MAX_OVF,
   PyC_ERR_CANT_OPEN_FILE,
   PyC_ERR_READING_FILE,
   PyC_ERR_INVALID_INCOMING_PREAMBLE,
   PyC_ERR_IS_NOT_INCOMING_COMMAND,
   PyC_ERR_IS_NOT_FROM_THIS_SERVER_COMMAND,
   PyC_ERR_INVALID_INCOMING_COMMAND,
   PyC_ERR_INCOMING_COMMAND_RAW_DATA_SZ,
   PyC_ERR_INCOMING_INVALID_CHKSUM,
   PyC_ERR_STR_MAX_SZ_OVFL,
   PyC_ERR_EMPTY_STR,
   PyC_ERR_NANO_FRONTIER_INVALID_SIZE,
   PyC_ERR_NANO_HASH_INVALID_SIZE,
   PyC_ERR_INVALID_INCOMING_PROTOCOL,
   PyC_ERR_BUF_SZ_DIFFERS_PROT_SZ,
   PyC_ERR_INVALID_NULL_C_POINTER

} FPYC_ERR;

#define MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS "\nCannot parse tuple and keywords to C library\n"
#define MSG_ERR_ALLOC_BUFFER "\nCannot alloc buffer \"FIOT_RAW_DATA_OBJ\"\n"
#define MSG_ERR_MAX_STR_OVFL "\nMax string overflow\n"
#define MSG_ERR_EMPTY_STR "\nForbidden empty string\n"
#define MSG_ERR_MAX_DATA_MEMORY_OVFL "\nSize exceeds max data memory. Overflow\n"
#define MSG_ERR_VERIFY_INCOMING_PROTOCOL "\nError when verifying incoming protocol\n"
#define MSG_ERR_CANT_PARSE_NANO_WALLET_FRONTIER "\nCan't parse Nano Wallet frontier\n"
#define MSG_ERR_INVALID_FRONTIER "\nInvalid frontier size. It must be 32 bytes long\n"
#define MSG_ERR_INVALID_HASH_SIZE "\nInvalid HASH size. It must be 32 bytes long\n"
#define MSG_ERR_CANT_PARSE_HASH_VALUE "\nCan't parse hash value to raw data\n"
#define MSG_ERR_RAW_DATA_SZ_OVFL "\nRaw data size is greater than \"F_NANO_TRANSACTION_MAX_SZ\"\n"
#define MSR_ERR_RAW_DATA_SZ_IS_ZERO "\nRaw data size is ZERO\n"
#define MSR_ERR_INVALID_INCOMING_PROTOCOL "\nInvalid incoming protocol\n"
#define MSR_ERR_BUF_SIZE_DIFFERS_PROT_SZ "\nBuffer size differs from protocol size\n"
#define MSG_ERR_INVALID_NANO_REPRESENTATIVE "\nInvalid NANO REPRESENTATIVE\n"
#define MSG_ERR_INVALID_NULL_C_PTR "\nInvalid C NULL pointer\n"


