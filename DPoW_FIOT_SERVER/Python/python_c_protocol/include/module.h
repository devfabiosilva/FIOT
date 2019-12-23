/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//Qua 18 Dez 2019 22:17:28 -03

#define F_NANO_TRANSACTION_RAW_DATA_SZ_MAX (size_t)512
#define F_NANO_MQTT_PUBLISH_STR_SZ (size_t)64
#define F_PREAMBLE (uint16_t)0b0101110010100011
#define F_F_NANO_TRANSACTION_VERSION (uint32_t)((1<<16)|2)

#define F_RAW_DATA_TYPE_RAW_DATA (uint16_t)0x0001

typedef struct f_nano_hw_transaction_hdr_t {
   uint16_t preamble;
   uint16_t raw_data_type;
   uint32_t command;
   uint32_t raw_data_sz;
   uint32_t version;
   uint32_t last_msg_id;
   uint64_t timestamp;
   uint8_t publish_str[F_NANO_MQTT_PUBLISH_STR_SZ];
   uint32_t crc32;
} __attribute__((packed)) F_NANO_TRANSACTION_HDR;

typedef struct f_nano_hw_transaction_t {
   F_NANO_TRANSACTION_HDR hdr;
   uint8_t rawdata[F_NANO_TRANSACTION_RAW_DATA_SZ_MAX];
} __attribute__((packed)) F_NANO_HW_TRANSACTION;

#define F_NANO_TRANSACTION_MAX_SZ sizeof(F_NANO_HW_TRANSACTION)


#define MAX_STR_NANO_CHAR (size_t)70 //5+56+8+1
#define MAX_STR_RAW_BALANCE_MAX (size_t)40

