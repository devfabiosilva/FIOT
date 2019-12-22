#include "module.h"
//#include "fpyc_err.h"

FPYC_ERR prepare_command(F_NANO_HW_TRANSACTION *, void *);

#define CMD_SEND_RAW_BALANCE_TO_CLIENT (uint32_t)(1<<1)|1


#define MSG_ERR_PREPARE_COMMAND "\nError in prepare command function\n"
