/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Sexta 20 Dez 2019 21:15:30 -03

char *fpyc_err_msg(const char *, int);

typedef struct f_err_const_t {
   const char *name;
   int value;
} F_ERR_CONST;

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
   PyC_ERR_INVALID_NULL_C_POINTER,
   PyC_ERR_UNABLE_GET_REP,
   PyC_ERR_UNABLE_GET_RAW_BALANCE,
   PyC_ERR_UNABLE_GET_RAW_FRONTIER,
   PyC_ERR_UNABLE_GET_DPOW,
   PyC_ERR_UNABLE_GET_CALCULATED_DPOW_HASH,
   PyC_ERR_UNABLE_GET_DPOW_HASH_FROM_CLIENT,
   PyC_ERR_UNABLE_GET_SIGNED_TRANSACTION_FEE,
   PyC_ERR_INVALID_JSON_SZ_IN_FIOT_PROTOCOL,
   PyC_ERR_ADD_CONST_INI,
   PyC_ERR_OBJ_IS_NOT_FUNCTION_CALL,
   PyC_ERR_OBJ,
   PyC_ERR_DELETE_ATTRIBUTE_ERR,
   PyC_ERR_DELETE_ATTRIBUTE_MSG,
   PyC_ERR_NULL_DATA,
   PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS,
   PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS,
   PyC_ERR_FORBIDDEN_OVFL_PUBL_STR,
   PyC_ERR_FORBIDDEN_NULL_PUB_STR,
   PyC_ERR_CANT_ADD_MSG_TO_ERROR_SENDER,
   PyC_ERR_NANO_BLOCK_INVALID_SIZE,
   PyC_ERR_SIGNED_JSON_NOT_FOUND,
   PyC_ERR_SIGNED_JSON_BLOCK_TOO_LARGE,
   PyC_ERR_EMPTY_JSON_STR_BLK,
   PyC_ERR_NULL_FEE_POINTER,
   PyC_ERR_ZERO_FEE,
   PyC_ERR_SIGNED_P2POW_BLOCK_NOT_FOUND,
   PyC_ERR_CANT_PARSE_SIGNED_P2POW_TO_JSON,
   PyC_ERR_UNABLE_GET_RAW_BALANCE_FROM_SIGNED_BLOCK,
   PyC_ERR_FEE_MAX_MULT_NEGATIVE_OR_ZERO,
   PyC_ERR_FEE_MIN_MULT_NEGATIVE_OR_ZERO,
   PyC_ERR_INVALID_TRANS_HASH_RESULT,
   PyC_ERR_INVALID_TRANS_HASH_RESULT_SZ,
   PyC_ERR_INVALID_WORKER_HASH_RESULT_SZ,
   PyC_ERR_INVALID_WORKER_HASH_RESULT
   //PyC_ERR_UNABLE_REQ_P2POW_CLIENT

} FPYC_ERR;

#define MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS "\nCannot parse tuple and keywords to C library\n"
#define MSG_ERR_ALLOC_BUFFER "\nCannot alloc buffer \"FIOT_RAW_DATA_OBJ\"\n"
#define MSG_ERR_MAX_STR_OVFL "\nMax string overflow\n"
#define MSG_ERR_EMPTY_STR "\nForbidden empty string\n"
#define MSG_ERR_MAX_DATA_MEMORY_OVFL "\nSize exceeds max data memory. Overflow\n"
#define MSG_ERR_VERIFY_INCOMING_PROTOCOL "\nError when verifying incoming protocol\n"
#define MSG_ERR_VERIFY_OUTCOMING_PROTOCOL "\nError when verifying outcoming protocol\n"
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
#define MSG_ERR_CANT_ADD_CONSTANT_INITIALIZATION "\nError in \"PyModule_AddIntConstant\" function\n"
#define MSG_ERR_OBJ_NOT_CALLABLE "\nObject is not callable\n"
#define MSG_ERR_OBJ "\nObject should not be callable\n"
#define MSG_ERR_DEL_ATTR "\nCan't delete attribute in \"f_set_error_util\"\n"
#define MSG_ERR_INCOMING_OUTCOMING_FC "\nError in \"verify_incoming_outcoming_raw_data_util\"\n"
#define MSG_ERR_INVALID_NANO_WALLET_INCOMING "\nInvalid Nano Wallet in incoming data or Wallet not found\n"
#define MSG_ERR_INVALID_NANO_WALLET_OUTCOMING "\nInvalid Nano Wallet in outcoming data or Wallet not found\n"
#define MSG_ERR_GET_REP_FROM_SERVER_SIDE "\nError in \"get_representative_addr_from_sending_data\" in server side\n"
#define MSG_ERR_UNABLE_GET_REP_SENDING_DATA "\nUnable to get representative from sending data\n"
#define MSG_ERR_GET_RAW_BALANCE_FROM_SENDING_DATA "\nUnable to get raw balance from sending data\n"
#define MSG_ERR_GET_RAW_BALANCE_NOT_FOUND_SENDING_DATA "\nRaw balance not found in sending data\n"
#define MSG_ERR_GET_RAW_BALANCE_FROM_SENDING_DATA_NOT_VALID "\nRaw balance in sending data not valid\n"
#define MSG_ERR_GET_FRONTIER_FROM_SENDING_DATA_NOT_VALID "\nFrontier not valid in sending data.\n"
#define MSG_ERR_GET_FRONTIER_NOT_FOUND_IN_SENDING_DATA "\nFrontier not found in sending data\n"
#define MSG_ERR_GET_DPOW_FROM_SENDING_DATA_NOT_VALID "\nInvalid DPoW value from sending data\n"
#define MSG_ERR_GET_DPOW_NOT_FOUND_IN_SENDING_DATA "\nDPoW not found in sending data\n"
#define MSG_ERR_GET_DPOW_HASH_FROM_SENDING_DATA_NOT_VALID "\nInvalid DPoW hash from sending data\n"
#define MSG_ERR_GET_DPOW_HASH_FROM_SENDING_DATA_NOT_FOUND "\nDPoW hash value not found in sending data\n"
#define MSG_ERR_GET_DPOW_HASH_FROM_INCOMING_DATA_NOT_VALID "\nInvalid DPoW hash from incoming data\n"
#define MSG_ERR_GET_DPOW_HASH_FROM_INCOMING_DATA_NOT_FOUND "\nDPoW hash not found from incoming data\n"
#define MSG_ERR_GET_TRAN_FEE_NOT_VALID_IN_INCOMING_DATA "\nSigned JSON transaction not valid from incoming data\n"
#define MSG_ERR_GET_TRAN_FEE_NOT_FOUND_IN_INCOMING_DATA "\nSigned JSON transaction fee not found in incoming data\n"
#define MSG_ERR_GET_TRAN_FEE_INVALID_JSON_IN_INCOMING_DATA "\nSigned JSON string size exceeds \"JSON_TRANSACTION_FEE_BUF_SZ\". Try a small JSON string\n"
#define MSG_ERR_NULL_DATA "\nNULL data\n"
#define MSG_ERR_CANT_INIT_FIOT_PROT_MODULE "\nCan't init FIOT protocol module\n"
#define MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS "\nCan't parse internal arguments in \"f_parse_args_util\" function\n"
#define MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS "\nCan't execute function with parsed arguments via \"f_parse_args_util\" function\n"
#define MSG_ERR_INVALID_NANO_RAW_BALANCE "\nInvalid NANO Raw Balance. Is it a valid 128 bit big number?\n"
#define MSG_ERR_INVALID_NANO_RAW_PENDING "\nInvalid NANO Raw Pending. Is it a valid 128 bit big number?\n"
#define MSG_ERR_INVALID_NANO_SENDER_ADDR "\nInvalid Nano sender address\n"
#define MSG_ERR_INVALID_NANO_RAW_AMOUNT "\nInvalid Nano raw amount\n"
#define MSG_ERR_INVALID_BLOCK_SIZE "\nInvalid Block hash size\n"
#define MSG_ERR_CANT_PARSE_BLOCK_VALUE "\nCan't parse BLOCK value\n"
#define MSG_ERR_CANT_PARSE_TRANSACTION_HASH_VALUE "\nCan't parse signed HASH value block\n"
#define MSG_ERR_SIGNED_JSON_BLOCK_NOT_FOUND "\nSigned JSON block not found in incoming data\n"
#define MSG_ERR_SIGNED_JSON_BLOCK_TOO_LARGE "\nSigned JSON string too large. Try a small signed JSON block\n"
#define MSG_ERR_EMPTY_SIGNED_JSON_STR "\nEmpty signed JSON string block\n"
#define MSG_ERR_INVALID_WORKER_WALLET "\nInvalid worker wallet\n"
#define MSG_ERR_NULL_POINTER_WORKER_FEE "\nNULL pointer worker fee\n"
#define MSG_ERR_INVALID_NANO_RAW_FEE "\nInvalid raw fee\n"
#define MSG_ERR_ZERO_FEE "\nInvalid Fee. Worker fee is zero\n"
#define MSG_ERR_P2POW_SIGNED_BLOCK_NOT_FOUND "\nSigned P2PoW raw block not found\n"
#define MSG_FATAL_ERROR_MALLOC "\nError in memory allocation. Aborting ...\n\n"
#define MSG_CANT_PARSE_SIGNED_P2POW_TO_JSON "\nCan't parse P2PoW transaction to JSON\n"
#define MSG_UNABLE_NANO_WALLET_FROM_INCOMING_SIG_P2POW_BLK "\nUnable to get Nano wallet from incoming data of signed P2PoW block\n"
#define MSG_ERR_INVALID_FEE_VALUE "\nInvalid fee value\n"
//#define MSG_ERR_PARSE_STR_TO_HEX "\nCan't parse string to HEX\n"
#define MSG_ERR_MAX_MUL_NEG_ZERO "\nError. Max multiplier has a float type negative or zero\n"
#define MSG_ERR_MIN_MUL_NEG_ZERO "\nError. Min multiplier has a float type negative or zero\n"
//#define MSG_ERR_P2POW_REQ_INFO_ERROR "\nCan't get request P2PoW. Error\n"
//#define MSG_ERR_P2POW_REQ_INFO_INCOMING_DATA "\nCan't get request P2PoW from incoming data\n"
#define MSG_ERR_INVALID_REWARD_ACCOUNT_WALLET "\nError. Invalid reward account\n"
#define MSG_ERR_INVALID_P2POW_TRANS_HASH "\nInvalid P2PoW transaction hash result\n"
#define MSG_ERR_INVALID_P2POW_TRANS_HASH_SZ "\nInvalid P2PoW transaction hash size\n"
#define MSG_ERR_INVALID_P2POW_WORKER_HASH_SZ "\nInvalid P2PoW worker hash size\n"
#define MSG_ERR_INVALID_P2POW_WORKER_HASH "\nInvalid P2PoW worker hash result\n"

