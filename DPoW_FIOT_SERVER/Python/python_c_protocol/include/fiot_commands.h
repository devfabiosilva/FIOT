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

#define CMD_SEND_RAW_BALANCE_TO_CLIENT (uint32_t)((1<<1)|1)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano_wallet
// balance: balance
// pending: pending
//}
//
#define CMD_SEND_FRONTIER_TO_CLIENT (uint32_t)((2<<1)|1)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano_wallet (string)
// frontier: frontier (raw 32 bytes) !!! NOT STRING
//}
//
#define CMD_SEND_DPOW_TO_CLIENT (uint32_t)((3<<1)|1)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// value hash: Raw Data (raw 32 bytes) !!! NOT STRING
// PoW: Calculated proof of work (raw 8 bytes) !!! NOT STRING
//}
//
#define CMD_SEND_REPRESENTATIVE_TO_CLIENT (uint32_t)((4<<1)|1)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// representative: representative (string)
//}
//
/////////////////////////////////
#define CMD_SEND_ERROR_MSG_TO_CLIENT (uint32_t)((5<<1)|1)

#define CMD_SEND_NEXT_PENDING_TO_CLIENT (uint32_t)((6<<1)|1)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// sender_nano_wallet (source): nano wallet (string)
// amout: raw (string)
// block_hash: raw 32 bytes
//}
//
#define CMD_SEND_BLOCK_STATE_TO_CLIENT (uint32_t)((7<<1)|1)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// block_hash: raw 32 bytes
//}
// ON ERROR -> CMD_SEND_ERROR_MSG_TO_CLIENT
#define CMD_SEND_PREF_REPRESENTATIVE_TO_CLIENT (uint32_t)((8<<1)|1)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// representative: representative (string)
//}
//
// Client CMD's
#define CMD_GET_RAW_BALANCE (uint32_t)(CMD_SEND_RAW_BALANCE_TO_CLIENT^0x00000001)
//pub_add: Publish addr
//raw_data{
// nano_wallet: nano_wallet (string)
//}
//
#define CMD_GET_FRONTIER (uint32_t)(CMD_SEND_FRONTIER_TO_CLIENT^0x00000001)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano_wallet (string)
//}
//
#define CMD_GET_DPOW (uint32_t)(CMD_SEND_DPOW_TO_CLIENT^0x00000001)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// value hash: HASH to be calculated (raw 32 bytes) !!! NOT STRING
// SIGNED TRANSACTION FEE: JSON string
//}
//
#define CMD_GET_REPRESENTATIVE (uint32_t)(CMD_SEND_REPRESENTATIVE_TO_CLIENT^0x00000001)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// representative: representative (string)
//}
//
#define CMD_GET_NEXT_PENDING_ACCOUNT (uint32_t)(CMD_SEND_NEXT_PENDING_TO_CLIENT^0x00000001)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
//}
#define CMD_GET_BLOCK_STATE_FROM_CLIENT (uint32_t)(CMD_SEND_BLOCK_STATE_TO_CLIENT^0x00000001)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
// block signed: JSON string
//}
#define CMD_GET_PREF_REPRESENTATIVE (uint32_t)(CMD_SEND_PREF_REPRESENTATIVE_TO_CLIENT^0x00000001)
//pub_add: Publish addrs
//raw_data{
// nano_wallet: nano wallet (string)
//}
//
#define LAST_COMMAND CMD_SEND_PREF_REPRESENTATIVE_TO_CLIENT
//pub_add: Publish addrs
//raw_data{
// error: uint32
// reason: string
//}
//

///////////////////////////////

#define MSG_ERR_PREPARE_COMMAND "\nError in prepare command function\n"

typedef struct f_command_constant_t {
   const char *name;
   uint32_t value;
} F_COMMAND_CONSTANT;

