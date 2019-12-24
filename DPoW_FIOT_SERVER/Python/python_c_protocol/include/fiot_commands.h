/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

#include "module.h"
//#include "fpyc_err.h"

FPYC_ERR prepare_command(F_NANO_HW_TRANSACTION *, void *);
FPYC_ERR verify_protocol(F_NANO_HW_TRANSACTION *, int);

#define CMD_SEND_RAW_BALANCE_TO_CLIENT (uint32_t)(1<<1)|1
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano_wallet
// balance: balance
//}
//
#define CMD_SEND_FRONTIER_TO_CLIENT (uint32_t)(2<<1)|1
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano_wallet (string)
// frontier: frontier (raw 32 bytes) !!! NOT STRING
//}
//

/////////////////////////////////

// Client CMD's
#define CMD_GET_RAW_BALANCE (uint32_t)(CMD_SEND_RAW_BALANCE_TO_CLIENT^0x00000001)
//pub_add: Publish addr
//raw_data{
// nano_wallet: nano_wallet (string)
//}
//
#define CMD_GET_FRONTIER (uint32_t)(2<<1)|1
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano_wallet (string)
//}
//

#define LAST_COMMAND CMD_SEND_FRONTIER_TO_CLIENT

///////////////////////////////

#define MSG_ERR_PREPARE_COMMAND "\nError in prepare command function\n"


