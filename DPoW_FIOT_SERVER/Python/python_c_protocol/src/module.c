/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdarg.h>
#include "structmember.h"
#include "fpyc_err.h"
#include "fiot_commands.h"
#include "nano_dpow_server_util.h"
#include "defmsg.h"

typedef struct {
   PyObject_HEAD
   int f_last_error;
   int raw_data_sz;
   int sent_raw_data_sz;
   unsigned char raw_data[F_NANO_TRANSACTION_MAX_SZ];
   unsigned char sent_raw_data[F_NANO_TRANSACTION_MAX_SZ];
   PyObject *fc_onerror;
   PyObject *fc_ondata; // received data from client
   PyObject *fc_onsentdata; //to client
} FIOT_RAW_DATA_OBJ;

static F_ERR_CONST ERR_CONST[] = {

   {"F_ERR_OK", PyC_ERR_OK},
   {"F_ERR_BUFFER_ALLOC", PyC_ERR_BUFFER_ALLOC},
   {"F_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS", PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS},
   {"F_ERR_MEM_OVFL", PyC_ERR_MEM_OVFL},
   {"F_ERR_RAW_DATA_SZ_ZERO", PyC_ERR_RAW_DATA_SZ_ZERO},
   {"F_ERR_DATA_OBJ_NOT_READY", PyC_ERR_DATA_OBJ_NOT_READY},
   {"F_ERR_DATA_OBJ_CREATE", PyC_ERR_DATA_OBJ_CREATE},
   {"F_ERR_DATA_OBJ_CREATE_ATTR", PyC_ERR_DATA_OBJ_CREATE_ATTR},
   {"F_ERR_ADD_METHOD", PyC_ERR_ADD_METHOD},
   {"F_ERR_NULL_OBJ", PyC_ERR_NULL_OBJ},
   {"F_ERR_PUBLISH_ADDR_OVFL", PyC_ERR_PUBLISH_ADDR_OVFL},
   {"F_ERR_F_NANO_TRANSACTION_RAW_DATA_SZ_MAX_OVF", PyC_ERR_F_NANO_TRANSACTION_RAW_DATA_SZ_MAX_OVF},
   {"F_ERR_CANT_OPEN_FILE", PyC_ERR_CANT_OPEN_FILE},
   {"F_ERR_READING_FILE", PyC_ERR_READING_FILE},
   {"F_ERR_INVALID_INCOMING_PREAMBLE", PyC_ERR_INVALID_INCOMING_PREAMBLE},
   {"F_ERR_IS_NOT_INCOMING_COMMAND", PyC_ERR_IS_NOT_INCOMING_COMMAND},
   {"F_ERR_IS_NOT_FROM_THIS_SERVER_COMMAND", PyC_ERR_IS_NOT_FROM_THIS_SERVER_COMMAND},
   {"F_ERR_INVALID_INCOMING_COMMAND", PyC_ERR_INVALID_INCOMING_COMMAND},
   {"F_ERR_INCOMING_COMMAND_RAW_DATA_SZ", PyC_ERR_INCOMING_COMMAND_RAW_DATA_SZ},
   {"F_ERR_INCOMING_INVALID_CHKSUM", PyC_ERR_INCOMING_INVALID_CHKSUM},
   {"F_ERR_STR_MAX_SZ_OVFL", PyC_ERR_STR_MAX_SZ_OVFL},
   {"F_ERR_EMPTY_STR", PyC_ERR_EMPTY_STR},
   {"F_ERR_NANO_FRONTIER_INVALID_SIZE", PyC_ERR_NANO_FRONTIER_INVALID_SIZE},
   {"F_ERR_NANO_HASH_INVALID_SIZE", PyC_ERR_NANO_HASH_INVALID_SIZE},
   {"F_ERR_INVALID_INCOMING_PROTOCOL", PyC_ERR_INVALID_INCOMING_PROTOCOL},
   {"F_ERR_BUF_SZ_DIFFERS_PROT_SZ", PyC_ERR_BUF_SZ_DIFFERS_PROT_SZ},
   {"F_ERR_INVALID_NULL_C_POINTER", PyC_ERR_INVALID_NULL_C_POINTER},
   {"F_ERR_UNABLE_GET_REP", PyC_ERR_UNABLE_GET_REP},
   {"F_ERR_UNABLE_GET_RAW_BALANCE", PyC_ERR_UNABLE_GET_RAW_BALANCE},
   {"F_ERR_UNABLE_GET_RAW_FRONTIER", PyC_ERR_UNABLE_GET_RAW_FRONTIER},
   {"F_ERR_UNABLE_GET_DPOW", PyC_ERR_UNABLE_GET_DPOW},
   {"F_ERR_UNABLE_GET_CALCULATED_DPOW_HASH", PyC_ERR_UNABLE_GET_CALCULATED_DPOW_HASH},
   {"F_ERR_UNABLE_GET_DPOW_HASH_FROM_CLIENT", PyC_ERR_UNABLE_GET_DPOW_HASH_FROM_CLIENT},
   {"F_ERR_UNABLE_GET_SIGNED_TRANSACTION_FEE", PyC_ERR_UNABLE_GET_SIGNED_TRANSACTION_FEE},
   {"F_ERR_INVALID_JSON_SZ_IN_FIOT_PROTOCOL", PyC_ERR_INVALID_JSON_SZ_IN_FIOT_PROTOCOL},
   {"F_ERR_ADD_CONST_INI", PyC_ERR_ADD_CONST_INI},
   {"F_ERR_OBJ_IS_NOT_FUNCTION_CALL", PyC_ERR_OBJ_IS_NOT_FUNCTION_CALL},
   {"F_ERR_OBJ", PyC_ERR_OBJ},
   {"F_ERR_DELETE_ATTRIBUTE_ERR", PyC_ERR_DELETE_ATTRIBUTE_ERR},
   {"F_ERR_DELETE_ATTRIBUTE_MSG", PyC_ERR_DELETE_ATTRIBUTE_MSG},
   {"F_ERR_NULL_DATA", PyC_ERR_NULL_DATA},
   {"F_ERR_CANT_PARSE_INTERNAL_ARGUMENTS", PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS},
   {"F_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS", PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS},
   {"F_ERR_FORBIDDEN_OVFL_PUBL_STR", PyC_ERR_FORBIDDEN_OVFL_PUBL_STR},
   {"F_ERR_FORBIDDEN_NULL_PUB_STR", PyC_ERR_FORBIDDEN_NULL_PUB_STR},
   {"F_ERR_CANT_ADD_MSG_TO_ERROR_SENDER", PyC_ERR_CANT_ADD_MSG_TO_ERROR_SENDER},
   {"F_ERR_NANO_BLOCK_INVALID_SIZE", PyC_ERR_NANO_BLOCK_INVALID_SIZE},
   {"F_ERR_SIGNED_JSON_NOT_FOUND", PyC_ERR_SIGNED_JSON_NOT_FOUND},
   {"F_ERR_SIGNED_JSON_BLOCK_TOO_LARGE", PyC_ERR_SIGNED_JSON_BLOCK_TOO_LARGE},
   {"F_ERR_EMPTY_JSON_STR_BLK", PyC_ERR_EMPTY_JSON_STR_BLK},
   {"F_ERR_NULL_FEE_POINTER", PyC_ERR_NULL_FEE_POINTER},
   {"F_ERR_ZERO_FEE", PyC_ERR_ZERO_FEE},
   {"F_ERR_SIGNED_P2POW_BLOCK_NOT_FOUND", PyC_ERR_SIGNED_P2POW_BLOCK_NOT_FOUND},
   {"F_ERR_CANT_PARSE_SIGNED_P2POW_TO_JSON", PyC_ERR_CANT_PARSE_SIGNED_P2POW_TO_JSON},
   {"F_ERR_UNABLE_GET_RAW_BALANCE_FROM_SIGNED_BLOCK", PyC_ERR_UNABLE_GET_RAW_BALANCE_FROM_SIGNED_BLOCK},
   {"F_ERR_FEE_MAX_MULT_NEGATIVE_OR_ZERO", PyC_ERR_FEE_MAX_MULT_NEGATIVE_OR_ZERO},
   {"F_ERR_FEE_MIN_MULT_NEGATIVE_OR_ZERO", PyC_ERR_FEE_MIN_MULT_NEGATIVE_OR_ZERO},
   {"F_ERR_INVALID_TRANS_HASH_RESULT", PyC_ERR_INVALID_TRANS_HASH_RESULT},
   {"F_ERR_INVALID_TRANS_HASH_RESULT_SZ", PyC_ERR_INVALID_TRANS_HASH_RESULT_SZ},
   {"F_ERR_INVALID_WORKER_HASH_RESULT_SZ", PyC_ERR_INVALID_WORKER_HASH_RESULT_SZ},
   {"F_ERR_INVALID_WORKER_HASH_RESULT", PyC_ERR_INVALID_WORKER_HASH_RESULT}
//   {"F_ERR_UNABLE_REQ_P2POW_CLIENT", PyC_ERR_UNABLE_REQ_P2POW_CLIENT}

};

#define ERR_CONST_INDEX (size_t)(sizeof(ERR_CONST)/sizeof(F_ERR_CONST))

//util

PyObject *f_parse_args_util(PyObject *dest, const char *fmt, ...)
{
/*
 !! WARNING. No error sanity check. For internal util function use only !!!
 s-> string
 v-> void or uint8
 I-> uint32
 L-> uint64
 # -> data size
 '0'-'9' -> slot
 Ex.: f_parse_args_util(obj, src, "9sv#I", mystr, mydata, mydata_sz, myuint32)
*/

   size_t p_sz;
   char s_string[3], c;
   va_list args;
   PyObject *tmp;
   void *p;

   s_string[0]='s';
   s_string[2]=0;

   va_start(args, fmt);
   for (;*fmt;fmt++) {

      if (isdigit((int)*fmt))
         s_string[1]=*(fmt++);

      if ((c=*fmt)=='s') {

         if (!(tmp=Py_BuildValue("s", (const char *)va_arg(args, const char *))))
            return NULL;

         if (PyObject_SetAttrString(dest, (const char *)s_string, tmp))
            return NULL;

         continue;

      }

      if (c=='v') {

         p=(void *)va_arg(args, void *);
         p_sz=(size_t)va_arg(args, size_t);

         if (!(tmp=Py_BuildValue("y#", (const void *)p, (Py_ssize_t)p_sz)))
            return NULL;

         if (PyObject_SetAttrString(dest, (const char *)s_string, tmp))
            return NULL;

         continue;

      }

      if (c=='I') {

         if (!(tmp=Py_BuildValue("I", (unsigned int)va_arg(args, uint32_t))))
            return NULL;

         if (PyObject_SetAttrString(dest, (const char *)s_string, tmp))
            return NULL;

         continue;

      }

      if (c=='L') {

         if (!(tmp=Py_BuildValue("k", (unsigned long int)va_arg(args, uint64_t))))
            return NULL;

         if (PyObject_SetAttrString(dest, (const char *)s_string, tmp))
            return NULL;

      }

   }
   va_end(args);

   return dest;

}

static void delete_slots_util(PyObject **obj)
{

   size_t i;
   char s_string[3];

   s_string[0]='s';
   s_string[2]=0;

   for (i=10;i--;) {

      s_string[1]=('0'+(char)i);

      if (PyObject_HasAttrString(*obj, s_string))
         PyObject_DelAttrString(*obj, s_string);

   }

}

static const char *geterrorname_util(FPYC_ERR errname)
{

   size_t i;
   static const char *p="Unknown error index";

   for (i=0;i<ERR_CONST_INDEX;i++) {

      if (ERR_CONST[i].value^errname)
         continue;

      p=ERR_CONST[i].name;

      break;

   }

   return p;

}

static int f_set_error_no_raise_util(FIOT_RAW_DATA_OBJ *self, const char *error_msg, int err)
{

   PyObject *tmp;

   if (self->fc_onerror) {

      if (!(tmp=Py_BuildValue("s", error_msg)))
         return -1;

      if (PyObject_SetAttrString(self->fc_onerror, "msg", tmp))
         return -1;

      if (!(tmp=Py_BuildValue("i", err)))
         return -1;

      if (PyObject_SetAttrString(self->fc_onerror, "err", tmp))
         return -1;

      if (!(tmp=Py_BuildValue("s", geterrorname_util(err))))
         return -1;

      if (PyObject_SetAttrString(self->fc_onerror, "errname", tmp))
         return -1;

      if (!(PyObject_CallFunctionObjArgs(self->fc_onerror, self->fc_onerror, NULL)))
         return -1;

      if (PyObject_DelAttrString(self->fc_onerror, "errname")) {

         PyErr_SetString(PyExc_Exception, (const char *)fpyc_err_msg(MSG_ERR_DEL_ATTR, self->f_last_error=PyC_ERR_DELETE_ATTRIBUTE_ERR));

         return -1;

      }

      if (PyObject_DelAttrString(self->fc_onerror, "err")) {

         PyErr_SetString(PyExc_Exception, (const char *)fpyc_err_msg(MSG_ERR_DEL_ATTR, self->f_last_error=PyC_ERR_DELETE_ATTRIBUTE_ERR));

         return -1;

      }

      if (PyObject_DelAttrString(self->fc_onerror, "msg")) {

         PyErr_SetString(PyExc_Exception, (const char *)fpyc_err_msg(MSG_ERR_DEL_ATTR, self->f_last_error=PyC_ERR_DELETE_ATTRIBUTE_MSG));

         return -1;

      }

      return 1;

   }

   return 0;

}
// !!! for doc
// !!! WITH RAISE EXCEPTION
// returns 1 if is callable, 0 not callable, -1 (internal raise exception) (fabio)
static int f_set_error_util(FIOT_RAW_DATA_OBJ *self, PyObject *type, const char *error_msg, int err) {

   const char *m=(const char *)fpyc_err_msg(error_msg, err);
   int ret=f_set_error_no_raise_util(self, m, err);

   if (ret)
      return ret;

   PyErr_SetString(type, m);

   return 0;

}

static FPYC_ERR check_is_callable(PyObject **obj)
{

   if (PyObject_TypeCheck(*obj, (PyTypeObject *)PyObject_Type(Py_None))) {

      *obj=NULL;

      goto check_is_callable_EXIT;

   }

   if (!PyCallable_Check(*obj)) {

      *obj=NULL;

      PyErr_SetString(PyExc_MemoryError, MSG_ERR_OBJ_NOT_CALLABLE);

      return PyC_ERR_OBJ_IS_NOT_FUNCTION_CALL;

   }

check_is_callable_EXIT:
   return PyC_ERR_OK;

}

static FPYC_ERR getincomingmessage_util(FIOT_RAW_DATA_OBJ *self, void *data, size_t data_sz)
{

   FPYC_ERR err;
   uint32_t sz_tmp;

   if (data_sz>F_NANO_TRANSACTION_MAX_SZ) {

      f_set_error_no_raise_util(self, MSG_ERR_RAW_DATA_SZ_OVFL, err=PyC_ERR_MEM_OVFL);

      return err;
   
   } else if (data_sz==0) {

      f_set_error_no_raise_util(self, MSR_ERR_RAW_DATA_SZ_IS_ZERO, err=PyC_ERR_RAW_DATA_SZ_ZERO);

      return err;

   }

   if ((err=verify_protocol((F_NANO_HW_TRANSACTION *)data, 1))) {

      f_set_error_no_raise_util(self, MSR_ERR_INVALID_INCOMING_PROTOCOL, err);

      return err;

   }

   if ((((F_NANO_HW_TRANSACTION *)data)->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))^(uint32_t)data_sz) {

      f_set_error_no_raise_util(self, MSR_ERR_BUF_SIZE_DIFFERS_PROT_SZ, err=PyC_ERR_BUF_SZ_DIFFERS_PROT_SZ);

      return err;

   }

   self->raw_data_sz=data_sz;
   memcpy(self->raw_data, data, data_sz);

   err=PyC_ERR_OK;

   if (self->fc_ondata) {

      sz_tmp=*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, raw_data_sz));

      if (!f_parse_args_util(self->fc_ondata, "0I1I2I3I4I5L6s7v",
         *(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)),
         (uint32_t)(*(uint16_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, raw_data_type))),
         sz_tmp,
         *(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, version)),
         *(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, last_msg_id)),
         *(uint64_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, timestamp)),
         self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str),
         self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata),
         (size_t)sz_tmp)) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, err=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         return err;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_ondata, self->fc_ondata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, err=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         return err;

      }

      delete_slots_util(&self->fc_ondata);

   }

   return err;

}
//is_incoming ->0 for self server or non zero for client
static FPYC_ERR verify_incoming_outcoming_raw_data_util(FIOT_RAW_DATA_OBJ *self, int is_incoming)
{

   uint32_t sz_tmp;
   uint8_t *p;

   if (is_incoming) {

      sz_tmp=self->raw_data_sz;
      p=self->raw_data;

   } else {

      sz_tmp=self->sent_raw_data_sz;
      p=self->sent_raw_data;

   }

   if (sz_tmp>F_NANO_TRANSACTION_MAX_SZ)
      return PyC_ERR_MEM_OVFL;
   else if (sz_tmp==0)
      return PyC_ERR_RAW_DATA_SZ_ZERO;

   if (((*(uint32_t *)(p+offsetof(F_NANO_TRANSACTION_HDR, raw_data_sz)))+(uint32_t)sizeof(F_NANO_TRANSACTION_HDR))^sz_tmp)
      return PyC_ERR_BUF_SZ_DIFFERS_PROT_SZ;

   return verify_protocol((F_NANO_HW_TRANSACTION *)p, is_incoming);

}

const char *f_parse_raw_to_nano_wallet_util(char *prefix, uint8_t *raw)
{
//MAX_STR_NANO_CHAR
//NANO_PUBLIC_KEY_EXTENDED
   static uint8_t buf[MAX_STR_NANO_CHAR+sizeof(NANO_PUBLIC_KEY_EXTENDED)];

   if (pk_to_wallet((char *)buf, prefix, (uint8_t *)memcpy(buf+MAX_STR_NANO_CHAR, raw, 32)))
      return NULL;

   return (const char *)&buf;

}
//-util

static PyObject *about(PyObject *self, PyObject *Py_UNUSED(ignored))
{
   return PyUnicode_DecodeFSDefault((const char *)ABOUT_MSG);
}

static PyObject *is_equivalent_and_valid(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano_wallet1", "nano_wallet2", NULL};
   char *wallet1, *wallet2;
   uint8_t *res;
   int err;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &wallet1, &wallet2)) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS, PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS));

      return NULL;

   }

   if (!(res=malloc(2*LIST_STR_WALLET))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_ALLOC_BUFFER, PyC_ERR_BUFFER_ALLOC));

      return NULL;

   }

   err=0; // fail

   if (nano_base_32_2_hex(res, (const char *)wallet1))
      goto is_equivalent_and_valid_EXIT1;

   if (nano_base_32_2_hex(res+LIST_STR_WALLET, (const char *)wallet2))
      goto is_equivalent_and_valid_EXIT1;

   if (memcmp(res, res+LIST_STR_WALLET, MAX_RAW_DATA_HASH))
      goto is_equivalent_and_valid_EXIT1;

   err=7; //success

is_equivalent_and_valid_EXIT1:
   memset(res, 0, 2*LIST_STR_WALLET);
   free(res);

   return PyLong_FromLong((long int)err);

}
static void fiot_raw_data_obj_dealloc(FIOT_RAW_DATA_OBJ *self)
{
   Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *fiot_raw_data_obj_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
   FIOT_RAW_DATA_OBJ *self;
   self=(FIOT_RAW_DATA_OBJ *)type->tp_alloc(type, 0);

   if (self) {

      self->f_last_error=PyC_ERR_OK;
      self->raw_data_sz=0;
      self->sent_raw_data_sz=0;
      memset(self->raw_data, 0, 2*F_NANO_TRANSACTION_MAX_SZ);
      self->fc_onerror=NULL;
      self->fc_ondata=NULL;
      self->fc_onsentdata=NULL;

   } else
      PyErr_SetString(PyExc_BufferError, MSG_ERR_ALLOC_BUFFER);

   return (PyObject *)self;

}

static int fiot_raw_data_obj_init(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"raw_data", NULL};
   int buf_sz;
   unsigned char *buf;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "z#", kwlist, &buf, &buf_sz)) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS, self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS));

      return self->f_last_error;

   }

   if (!buf) {

      self->raw_data_sz=0;
      self->sent_raw_data_sz=0;
      memset(self->raw_data, 0, 2*F_NANO_TRANSACTION_MAX_SZ);// !!! 2*F_NANO_TRANSACTION_MAX_SZ and NOT F_NANO_TRANSACTION_MAX_SZ !!!

      return (self->f_last_error=PyC_ERR_OK);

   }

   if ((self->f_last_error=getincomingmessage_util(self, (void *)buf, (size_t)buf_sz))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_INIT_FIOT_PROT_MODULE, self->f_last_error));

      return self->f_last_error;

   }

   self->sent_raw_data_sz=0;
   memset(self->sent_raw_data, 0, F_NANO_TRANSACTION_MAX_SZ);

   return self->f_last_error;


}

//get
static PyObject *fgetlasterror(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{
   return PyLong_FromLong((long int)self->f_last_error);
}

static PyObject *getincomingmessage(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"raw_data", NULL};
   int buf_sz;
   unsigned char *buf;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "z#", kwlist, &buf, &buf_sz)) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS, self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS));

      return NULL;

   }

   if (!buf) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NULL_C_PTR, self->f_last_error=PyC_ERR_INVALID_NULL_C_POINTER)<0)
         return NULL;

      return PyLong_FromLong((long int)self->f_last_error);

   }
//
   self->raw_data_sz=0;
   memset((void *)self->raw_data, 0, F_NANO_TRANSACTION_MAX_SZ);
//
   return PyLong_FromLong((long int)(self->f_last_error=getincomingmessage_util(self, (void *)buf, (size_t)buf_sz)));

}

static PyObject *get_nano_addr_from_incoming_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INCOMING_OUTCOMING_FC, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))==CMD_GET_RAW_BLOCK_STATE_FROM_CLIENT) {

      if ((s=f_parse_raw_to_nano_wallet_util((((F_BLOCK_TRANSFER *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)))->prefixes&SENDER_XRB)?
         XRB_PREFIX:NANO_PREFIX, ((F_BLOCK_TRANSFER *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)))->account))) return Py_BuildValue("s", s);

      if (f_set_error_no_raise_util(self, MSG_UNABLE_NANO_WALLET_FROM_INCOMING_SIG_P2POW_BLK,
         self->f_last_error=PyC_ERR_UNABLE_GET_RAW_BALANCE_FROM_SIGNED_BLOCK)<0) return NULL;

      return Py_None;

   }

   if ((self->f_last_error=valid_nano_wallet(s=(const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_INCOMING, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_nano_addr_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INCOMING_OUTCOMING_FC, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=valid_nano_wallet(s=(const char *)(self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_representative_addr_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_REP_FROM_SERVER_SIDE, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_REPRESENTATIVE_TO_CLIENT) {


      if (f_set_error_no_raise_util(self, MSG_ERR_UNABLE_GET_REP_SENDING_DATA, self->f_last_error=PyC_ERR_UNABLE_GET_REP)<0)
         return NULL;


      return Py_None;

   }

   if ((self->f_last_error=valid_nano_wallet(s=(const char *)(self->sent_raw_data+MAX_STR_NANO_CHAR+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_REPRESENTATIVE, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_raw_balance_value_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_RAW_BALANCE_FROM_SENDING_DATA, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_RAW_BALANCE_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_RAW_BALANCE_NOT_FOUND_SENDING_DATA, self->f_last_error=PyC_ERR_UNABLE_GET_RAW_BALANCE)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=valid_raw_balance(s=(const char *)(self->sent_raw_data+MAX_STR_NANO_CHAR+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_RAW_BALANCE_FROM_SENDING_DATA_NOT_VALID, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_frontier_value_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   char *s;
   PyObject *ret;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_FRONTIER_FROM_SENDING_DATA_NOT_VALID, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_FRONTIER_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_FRONTIER_NOT_FOUND_IN_SENDING_DATA, self->f_last_error=PyC_ERR_UNABLE_GET_RAW_FRONTIER)<0)
         return NULL;

      return Py_None;

   }

   if (!(s=malloc(2*MAX_RAW_DATA_FRONTIER+1))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   ret=Py_BuildValue("s", (const char *)fhex2strv2(s, (const void *)(self->sent_raw_data+MAX_STR_NANO_CHAR+
      offsetof(F_NANO_HW_TRANSACTION, rawdata)), MAX_RAW_DATA_FRONTIER, 1));

   memset(s, 0, 2*MAX_RAW_DATA_FRONTIER+1);
   free(s);

   return ret;

}

static PyObject *get_dpow_value_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   char *s;
   PyObject *ret;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_FROM_SENDING_DATA_NOT_VALID, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_DPOW_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_NOT_FOUND_IN_SENDING_DATA, self->f_last_error=PyC_ERR_UNABLE_GET_DPOW)<0)
         return NULL;

      return Py_None;

   }

   if (!(s=malloc(20))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   sprintf(s, "%016lx", (unsigned long int)*((uint64_t *)(self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+
      MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH)));

   ret=Py_BuildValue("s", (const char *)s);

   memset(s, 0, 20);
   free(s);

   return ret;

}

static PyObject *get_calculated_dpow_hash_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   char *s;
   PyObject *ret;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_SENDING_DATA_NOT_VALID, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_DPOW_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_SENDING_DATA_NOT_FOUND,
         self->f_last_error=PyC_ERR_UNABLE_GET_CALCULATED_DPOW_HASH)<0)
         return NULL;

      return Py_None;

   }

   if (!(s=malloc(2*MAX_RAW_DATA_HASH+1))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   ret=Py_BuildValue("s", (const char *)fhex2strv2(s, (const void *)(self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+
      MAX_STR_NANO_CHAR), MAX_RAW_DATA_HASH, 1));

   memset(s, 0, 2*MAX_RAW_DATA_HASH+1);
   free(s);

   return ret;

}

static PyObject *get_dpow_hash_from_incoming_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   char *s;
   PyObject *ret;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_INCOMING_DATA_NOT_VALID, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_GET_DPOW) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_INCOMING_DATA_NOT_FOUND,
         self->f_last_error=PyC_ERR_UNABLE_GET_DPOW_HASH_FROM_CLIENT)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_INCOMING, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if (is_null_hash((uint8_t *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR)))
      return Py_BuildValue("s", "");

   if (!(s=malloc(2*MAX_RAW_DATA_HASH+1))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   ret=Py_BuildValue("s", (const char *)fhex2strv2(s, (const void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+
      MAX_STR_NANO_CHAR), MAX_RAW_DATA_HASH, 1));

   memset(s, 0, 2*MAX_RAW_DATA_HASH+1);
   free(s);

   return ret;

}

#define JSON_TRANSACTION_FEE_BUF_SZ (size_t)(F_NANO_TRANSACTION_RAW_DATA_SZ_MAX-(MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH))
static PyObject *get_signed_transaction_fee_json(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_TRAN_FEE_NOT_VALID_IN_INCOMING_DATA, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_GET_DPOW) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_TRAN_FEE_NOT_FOUND_IN_INCOMING_DATA,
         self->f_last_error=PyC_ERR_UNABLE_GET_SIGNED_TRANSACTION_FEE)<0)
         return NULL;

      return Py_None;

   }

   s="";

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, raw_data_sz)))^(MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH))
      if ((strnlen(s=((const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH)),
         JSON_TRANSACTION_FEE_BUF_SZ))==JSON_TRANSACTION_FEE_BUF_SZ) {

         if (f_set_error_no_raise_util(self, MSG_ERR_GET_TRAN_FEE_INVALID_JSON_IN_INCOMING_DATA,
            self->f_last_error=PyC_ERR_INVALID_JSON_SZ_IN_FIOT_PROTOCOL)<0)
            return NULL;

         return Py_None;

      }

   return Py_BuildValue("s", s);

}
/*
static PyObject *p2pow_get_req_info(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((self->f_last_error=verify_incoming_outcoming_raw_data_util(self, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_P2POW_REQ_INFO_ERROR, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_GET_P2POW_RAW_RAW_REQ_INFO) {

      if (f_set_error_no_raise_util(self, MSG_ERR_P2POW_REQ_INFO_INCOMING_DATA, self->f_last_error=PyC_ERR_UNABLE_REQ_P2POW_CLIENT)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=valid_nano_wallet(s=(const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_INCOMING, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return Py_BuildValue("s", s);

}
*/
//set

static PyObject *send_p2pow_signed_result(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "trans_hash", "trans_work", "worker_hash", "worker_work", NULL};
   char *buf_nano_addr, *pub_addr, *trans_hash, *worker_hash;
   unsigned long int trans_work, worker_work;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzsksk", kwlist, &buf_nano_addr, &pub_addr, &trans_hash, &trans_work, &worker_hash, &worker_work)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_RAW_SIGNED_RESULT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

// WARNING HERE. IF received command is 'CMD_GET_RAW_BLOCK_STATE_FROM_CLIENT' then Nano Addres is in 32 bytes raw data and must be converted to Nano Wallet

   if (!(p=(void *)buf_nano_addr))
      ((((F_NANO_TRANSACTION_HDR *)(self->raw_data))->command)==CMD_GET_RAW_BLOCK_STATE_FROM_CLIENT)?
         (p=(void *)f_parse_raw_to_nano_wallet_util((((F_BLOCK_TRANSFER *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)))->prefixes&SENDER_XRB)?
            XRB_PREFIX:NANO_PREFIX, ((F_BLOCK_TRANSFER *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)))->account)):
         (p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if (strnlen(trans_hash, 65)!=64) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_P2POW_TRANS_HASH_SZ, self->f_last_error=PyC_ERR_INVALID_TRANS_HASH_RESULT_SZ)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   if (f_str_to_hex(((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->trans_hash, (const char *)trans_hash)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_P2POW_TRANS_HASH, self->f_last_error=PyC_ERR_INVALID_TRANS_HASH_RESULT)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   ((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->trans_work=(uint64_t)trans_work;
//
   if (strnlen(worker_hash, 65)!=64) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_P2POW_WORKER_HASH_SZ, self->f_last_error=PyC_ERR_INVALID_WORKER_HASH_RESULT_SZ)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   if (f_str_to_hex(((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->worker_hash, (const char *)worker_hash)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_P2POW_WORKER_HASH, self->f_last_error=PyC_ERR_INVALID_WORKER_HASH_RESULT)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   ((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->worker_work=(uint64_t)worker_work;

////

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+sizeof(P2POW_REQ_RESULT);

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto send_p2pow_signed_result_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3s4L5s6L", buf->hdr.command, buf->hdr.publish_str, buf->rawdata,
         fhex2strv2((char *)(buf->rawdata+buf->hdr.raw_data_sz), (const void *)((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->trans_hash, 32, 1),
         ((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->trans_work,
         fhex2strv2((char *)(buf->rawdata+buf->hdr.raw_data_sz+65), (const void *)((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->worker_hash, 32, 1),
         ((P2POW_REQ_RESULT *)(buf->rawdata+MAX_STR_NANO_CHAR))->worker_work)) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto send_p2pow_signed_result_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto send_p2pow_signed_result_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

send_p2pow_signed_result_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *send_p2pow_req_info(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "fee", "max_mul", "min_mul", "reward_account", "version", NULL};
   char *buf_nano_addr, *pub_addr, *reward_account, *fee;
   float max_mul, min_mul; //f
   unsigned int version; //I
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzsffsI", kwlist, &buf_nano_addr, &pub_addr, &fee, &max_mul, &min_mul, &reward_account, &version)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_P2POW_RAW_REQ_INFO;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((self->f_last_error=f_nano_parse_raw_str_to_raw128_t(((P2POW_REQ_INFO *)(buf->rawdata+MAX_STR_NANO_CHAR))->fee, (const char *)fee))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_FEE_VALUE, self->f_last_error)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   if (max_mul<=0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_MUL_NEG_ZERO, self->f_last_error=PyC_ERR_FEE_MAX_MULT_NEGATIVE_OR_ZERO)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   ((P2POW_REQ_INFO *)(buf->rawdata+MAX_STR_NANO_CHAR))->max_mult=max_mul;

   if (min_mul<=0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MIN_MUL_NEG_ZERO, self->f_last_error=PyC_ERR_FEE_MIN_MULT_NEGATIVE_OR_ZERO)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   ((P2POW_REQ_INFO *)(buf->rawdata+MAX_STR_NANO_CHAR))->max_mult=min_mul;

   if ((self->f_last_error=valid_nano_wallet((const char *)reward_account))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_REWARD_ACCOUNT_WALLET, self->f_last_error)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   (is_nano_prefix((const char *)reward_account, NANO_PREFIX))?(((P2POW_REQ_INFO *)(buf->rawdata+MAX_STR_NANO_CHAR))->reward_account_prefix=0):
      (((P2POW_REQ_INFO *)(buf->rawdata+MAX_STR_NANO_CHAR))->reward_account_prefix=WORKER_XRB);

   ((P2POW_REQ_INFO *)(buf->rawdata+MAX_STR_NANO_CHAR))->version=(uint32_t)version;

//////

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+sizeof(P2POW_REQ_INFO);

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto send_p2pow_req_info_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3v", buf->hdr.command, buf->hdr.publish_str, buf->rawdata, buf->rawdata+MAX_STR_NANO_CHAR,
         sizeof(P2POW_REQ_INFO))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto send_p2pow_req_info_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto send_p2pow_req_info_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

send_p2pow_req_info_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *set_raw_balance(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "balance", "pending", NULL};
   char *buf_nano_addr, *pub_addr, *raw_balance, *raw_pending;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzss", kwlist, &buf_nano_addr, &pub_addr, &raw_balance, &raw_pending)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_RAW_BALANCE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=(strnlen(raw_balance, MAX_STR_RAW_BALANCE_MAX)))==MAX_STR_RAW_BALANCE_MAX) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   }

   if ((self->f_last_error=valid_raw_balance(raw_balance))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_RAW_BALANCE, self->f_last_error)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), raw_balance, MAX_STR_RAW_BALANCE_MAX);

   if ((self->f_last_error=valid_raw_balance(raw_pending))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_RAW_PENDING, self->f_last_error)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX), (const char *)raw_pending, MAX_STR_RAW_BALANCE_MAX);

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+2*MAX_STR_RAW_BALANCE_MAX;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto set_raw_balance_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3s4s5v", buf->hdr.command, buf->hdr.publish_str, buf->rawdata,
         buf->rawdata+MAX_STR_NANO_CHAR, buf->rawdata+MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX, self->sent_raw_data, self->sent_raw_data_sz)) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto set_raw_balance_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto set_raw_balance_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

set_raw_balance_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *set_frontier(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{
   static char *kwlist[] = {"nano", "publish", "frontier", NULL};
   char *buf_nano_addr, *pub_addr, *frontier;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzs", kwlist, &buf_nano_addr, &pub_addr, &frontier)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_FRONTIER_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)frontier, MAX_STR_DATA_FRONTIER))==MAX_STR_DATA_FRONTIER) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   } else if (sz_tmp^(MAX_STR_DATA_FRONTIER-1)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_FRONTIER, self->f_last_error=PyC_ERR_NANO_FRONTIER_INVALID_SIZE)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   }

   if ((self->f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)frontier))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_CANT_PARSE_NANO_WALLET_FRONTIER, self->f_last_error)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   }

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_RAW_DATA_FRONTIER;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto set_frontier_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3v4s", buf->hdr.command, buf->hdr.publish_str, buf->rawdata, self->sent_raw_data,
         self->sent_raw_data_sz, fhex2strv2((char *)buf, (const void *)(buf->rawdata+MAX_STR_NANO_CHAR), MAX_RAW_DATA_FRONTIER, 0))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto set_frontier_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto set_frontier_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);


set_frontier_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *send_dpow(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "hashval", "dpow", NULL};
   char *buf_nano_addr, *pub_addr, *hash_val;
   unsigned long int k;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzsk", kwlist, &buf_nano_addr, &pub_addr, &hash_val, &k)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_DPOW_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)hash_val, MAX_STR_DATA_HASH_VALUE))==MAX_STR_DATA_HASH_VALUE) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   } else if (sz_tmp^(MAX_STR_DATA_HASH_VALUE-1)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_HASH_SIZE, self->f_last_error=PyC_ERR_NANO_HASH_INVALID_SIZE)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   }

   if ((self->f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)hash_val))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_CANT_PARSE_HASH_VALUE, self->f_last_error)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   }

   *((uint64_t *)(buf->rawdata+MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH))=k;

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH+sizeof(uint64_t);

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto send_dpow_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3v4s5L", *(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)),
         self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str), self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata),
         self->sent_raw_data, self->sent_raw_data_sz, fhex2strv2((char *)buf, (const void *)(buf->rawdata+MAX_STR_NANO_CHAR), MAX_RAW_DATA_HASH, 1),
         *(uint64_t *)(self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto send_dpow_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto send_dpow_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);


send_dpow_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *send_representative(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "representative", NULL};
   char *buf_nano_addr, *pub_addr, *rep;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzs", kwlist, &buf_nano_addr, &pub_addr, &rep)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_REPRESENTATIVE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)rep, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)rep))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_REPRESENTATIVE, self->f_last_error)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)rep, MAX_STR_NANO_CHAR);

   buf->hdr.raw_data_sz=2*MAX_STR_NANO_CHAR;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto send_representative_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3v4s", buf->hdr.command, buf->hdr.publish_str, buf->rawdata, self->sent_raw_data,
         self->sent_raw_data_sz, buf->rawdata+MAX_STR_NANO_CHAR)) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto send_representative_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto send_representative_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

send_representative_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *send_preferred_representative(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "pref_representative", NULL};
   char *buf_nano_addr, *pub_addr, *rep;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzs", kwlist, &buf_nano_addr, &pub_addr, &rep)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_PREF_REPRESENTATIVE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)rep, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)rep))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_REPRESENTATIVE, self->f_last_error)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)rep, MAX_STR_NANO_CHAR);

   buf->hdr.raw_data_sz=2*MAX_STR_NANO_CHAR;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto send_preferred_representative_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3v4s", buf->hdr.command, buf->hdr.publish_str, buf->rawdata, self->sent_raw_data,
         self->sent_raw_data_sz, buf->rawdata+MAX_STR_NANO_CHAR)) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto send_preferred_representative_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto send_preferred_representative_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

send_preferred_representative_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *send_worker_fee(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "worker_wallet", "worker_fee", NULL};
   char *buf_nano_addr, *pub_addr, *worker, *fee;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzzz", kwlist, &buf_nano_addr, &pub_addr, &worker, &fee)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_WORKER_FEE;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_worker_fee_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_worker_fee_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto send_worker_fee_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR;

   if (worker) {

      if ((self->f_last_error=valid_nano_wallet((const char *)worker))) {

         if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_WORKER_WALLET, self->f_last_error)<0)
            ret=NULL;

         goto send_worker_fee_EXIT1;

      }

      if (!fee) {

         if (f_set_error_no_raise_util(self, MSG_ERR_NULL_POINTER_WORKER_FEE, self->f_last_error=PyC_ERR_NULL_FEE_POINTER)<0)
            ret=NULL;

         goto send_worker_fee_EXIT1;

      }

      if (is_filled_with_value((uint8_t *)fee, strnlen(fee, MAX_STR_RAW_BALANCE_MAX), '0')) {

         if (f_set_error_no_raise_util(self, MSG_ERR_ZERO_FEE, self->f_last_error=PyC_ERR_ZERO_FEE)<0)
            ret=NULL;

         goto send_worker_fee_EXIT1;

      }

      if ((self->f_last_error=valid_raw_balance(fee))) {

         if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_RAW_FEE, self->f_last_error)<0)
            ret=NULL;

         goto send_worker_fee_EXIT1;

      }

      strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)worker, MAX_STR_NANO_CHAR);
      strncpy((char *)(buf->rawdata+2*MAX_STR_NANO_CHAR), (const char *)fee, MAX_STR_RAW_BALANCE_MAX);

      buf->hdr.raw_data_sz+=MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX;

   }

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_worker_fee_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_worker_fee_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto send_worker_fee_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3v4s5s", buf->hdr.command, buf->hdr.publish_str, buf->rawdata, self->sent_raw_data,
         self->sent_raw_data_sz, (worker)?(char *)(buf->rawdata+MAX_STR_NANO_CHAR):"", (fee)?((char *)(buf->rawdata+2*MAX_STR_NANO_CHAR)):"")) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto send_worker_fee_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto send_worker_fee_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

send_worker_fee_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *get_last_sent_protocol(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   size_t sz_tmp;

   if ((sz_tmp=(size_t)self->sent_raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_BuildValue("y#", NULL, 0);

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, self->f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_BuildValue("y#", NULL, 0);

   }

   if ((self->f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->sent_raw_data, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_INCOMING_PROTOCOL, self->f_last_error)<0)
         return NULL;

      return Py_BuildValue("y#", NULL, 0);

   }

   return Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

}

static PyObject *set_onerror(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{
   static char *kwlist[] = {"func", NULL};
   PyObject *fobj;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &fobj)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   self->fc_onerror=fobj;

   if ((self->f_last_error=check_is_callable(&self->fc_onerror)))
      return NULL;

   return PyLong_FromLong((long int)self->f_last_error);

}

static PyObject *set_onreceivedfromclient(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"func", NULL};
   PyObject *fobj;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &fobj)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   self->fc_ondata=fobj;

   if ((self->f_last_error=check_is_callable(&self->fc_ondata)))
      return NULL;

   return PyLong_FromLong((long int)self->f_last_error);

}

static PyObject *set_onsendtoclient(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"func", NULL};
   PyObject *fobj;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &fobj)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   self->fc_onsentdata=fobj;

   if ((self->f_last_error=check_is_callable(&self->fc_onsentdata)))
      return NULL;

   return PyLong_FromLong((long int)self->f_last_error);

}

static PyObject *set_next_pending_to_client(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{
   static char *kwlist[] = {"nano", "publish", "sender", "amount", "block", NULL};
   char *buf_nano_addr, *pub_addr, *sender, *amount, *block;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzzss", kwlist, &buf_nano_addr, &pub_addr, &sender, &amount, &block)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_NEXT_PENDING_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_next_pending_to_client_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_next_pending_to_client_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto set_next_pending_to_client_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if (sender) {

      if ((sz_tmp=strnlen((const char *)sender, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

         if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
            ret=NULL;

         goto set_next_pending_to_client_EXIT1;

      } else if (sz_tmp==0) {

         if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
            ret=NULL;

         goto set_next_pending_to_client_EXIT1;

      }

      if ((self->f_last_error=valid_nano_wallet((const char *)sender))) {

         if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_SENDER_ADDR, self->f_last_error)<0)
            ret=NULL;

         goto set_next_pending_to_client_EXIT1;

      }

      strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)sender, MAX_STR_NANO_CHAR);

      if ((self->f_last_error=valid_raw_balance(amount))) {

         if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_RAW_AMOUNT, self->f_last_error)<0)
            ret=NULL;

         goto set_next_pending_to_client_EXIT1;

      }

      strncpy((char *)(buf->rawdata+2*MAX_STR_NANO_CHAR), (const char *)amount, MAX_STR_RAW_BALANCE_MAX);

      if ((sz_tmp=strnlen((const char *)block, MAX_STR_DATA_HASH_VALUE))==MAX_STR_DATA_HASH_VALUE) {

         if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
            ret=NULL;

         goto set_next_pending_to_client_EXIT1;

      } else if (sz_tmp^(MAX_STR_DATA_HASH_VALUE-1)) {

         if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_BLOCK_SIZE, self->f_last_error=PyC_ERR_NANO_BLOCK_INVALID_SIZE)<0)
            ret=NULL;

         goto set_next_pending_to_client_EXIT1;

      }

      if ((self->f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+2*MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX), (const char *)block))) {

         if (f_set_error_no_raise_util(self, MSG_ERR_CANT_PARSE_BLOCK_VALUE, self->f_last_error)<0)
            ret=NULL;

         goto set_next_pending_to_client_EXIT1;

      }

      sz_tmp=MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX+MAX_RAW_DATA_HASH;

   } else
      sz_tmp=0;

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+sz_tmp;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_next_pending_to_client_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_next_pending_to_client_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto set_next_pending_to_client_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3s4s5v6v", buf->hdr.command, buf->hdr.publish_str, buf->rawdata,
          buf->rawdata+MAX_STR_NANO_CHAR, buf->rawdata+2*MAX_STR_NANO_CHAR, buf->rawdata+2*MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX,
          MAX_RAW_DATA_HASH, buf->rawdata, buf->hdr.raw_data_sz)) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto set_next_pending_to_client_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto set_next_pending_to_client_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

set_next_pending_to_client_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *set_block_state_to_client(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "onsuccess_hash", NULL};
   char *buf_nano_addr, *pub_addr, *hash;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzs", kwlist, &buf_nano_addr, &pub_addr, &hash)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_BLOCK_STATE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=Py_None;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   }

   if ((self->f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, self->f_last_error)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)hash, MAX_STR_DATA_HASH_VALUE))==MAX_STR_DATA_HASH_VALUE) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   } else if (sz_tmp^(MAX_STR_DATA_HASH_VALUE-1)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_BLOCK_SIZE, self->f_last_error=PyC_ERR_NANO_BLOCK_INVALID_SIZE)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   }

   if ((self->f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)hash))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_CANT_PARSE_TRANSACTION_HASH_VALUE, self->f_last_error)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   }

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH;

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto set_block_state_to_client_EXIT1;

   }

   memcpy((void *)self->sent_raw_data, (const void *)buf, (size_t)(self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));

   if (self->fc_onsentdata) {

      ret=NULL;

      if (!f_parse_args_util(self->fc_onsentdata, "0I1s2s3v4s5v", self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command),
          self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str), self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata),
          self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR, MAX_RAW_DATA_HASH,
          fhex2strv2((char *)buf, (void *)self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR, MAX_RAW_DATA_HASH, 1),
          self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata),
          (uint32_t)*((uint32_t *)self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, raw_data_sz)))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_PARSE_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_PARSE_INTERNAL_ARGUMENTS);

         goto set_block_state_to_client_EXIT1;

      }

      if (!(PyObject_CallFunctionObjArgs(self->fc_onsentdata, self->fc_onsentdata, NULL))) {

         f_set_error_util(self, PyExc_Exception, MSG_ERR_CANT_EXECUTE_FC_INTERNAL_ARGUMENTS, self->f_last_error=PyC_ERR_CANT_EXEC_FC_INTERNAL_ARGUMENTS);

         goto set_block_state_to_client_EXIT1;

      }

      delete_slots_util(&self->fc_onsentdata);

      ret=PyLong_FromLong((long int)(self->f_last_error=PyC_ERR_OK));

   } else
      ret=Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

set_block_state_to_client_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static F_COMMAND_CONSTANT FIOT_COMMAND[] = {

   {"CMD_SEND_RAW_BALANCE_TO_CLIENT", CMD_SEND_RAW_BALANCE_TO_CLIENT},
   {"CMD_SEND_FRONTIER_TO_CLIENT", CMD_SEND_FRONTIER_TO_CLIENT},
   {"CMD_SEND_DPOW_TO_CLIENT", CMD_SEND_DPOW_TO_CLIENT},
   {"CMD_SEND_REPRESENTATIVE_TO_CLIENT", CMD_SEND_REPRESENTATIVE_TO_CLIENT},
   {"CMD_SEND_ERROR_MSG_TO_CLIENT", CMD_SEND_ERROR_MSG_TO_CLIENT},
   {"CMD_SEND_NEXT_PENDING_TO_CLIENT", CMD_SEND_NEXT_PENDING_TO_CLIENT},
   {"CMD_SEND_BLOCK_STATE_TO_CLIENT", CMD_SEND_BLOCK_STATE_TO_CLIENT},
   {"CMD_SEND_PREF_REPRESENTATIVE_TO_CLIENT", CMD_SEND_PREF_REPRESENTATIVE_TO_CLIENT},
   {"CMD_SEND_WORKER_FEE", CMD_SEND_WORKER_FEE},
   {"CMD_SEND_P2POW_RAW_REQ_INFO", CMD_SEND_P2POW_RAW_REQ_INFO},
   {"CMD_GET_RAW_BALANCE", CMD_GET_RAW_BALANCE},
   {"CMD_GET_FRONTIER", CMD_GET_FRONTIER},
   {"CMD_GET_DPOW", CMD_GET_DPOW},
   {"CMD_GET_REPRESENTATIVE", CMD_GET_REPRESENTATIVE},
   {"CMD_GET_NEXT_PENDING_ACCOUNT", CMD_GET_NEXT_PENDING_ACCOUNT},
   {"CMD_GET_BLOCK_STATE_FROM_CLIENT", CMD_GET_BLOCK_STATE_FROM_CLIENT},
   {"CMD_GET_PREF_REPRESENTATIVE", CMD_GET_PREF_REPRESENTATIVE},
   {"CMD_GET_WORKER_FEE", CMD_GET_WORKER_FEE},
   {"CMD_GET_RAW_BLOCK_STATE_FROM_CLIENT", CMD_GET_RAW_BLOCK_STATE_FROM_CLIENT},
   {"CMD_GET_P2POW_RAW_RAW_REQ_INFO", CMD_GET_P2POW_RAW_RAW_REQ_INFO}

};
#define FIOT_COMMAND_MAX_INDEX (size_t)(sizeof(FIOT_COMMAND)/sizeof(F_COMMAND_CONSTANT))

static PyObject *geterrorname(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{
   static char *kwlist[] = {"errname", NULL};
   int errname;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &errname)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   return Py_BuildValue("s", geterrorname_util(errname));

}

static PyObject *getfiotcommandname(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"commandname", NULL};
   size_t i;
   int commandname;
   const char *p="Unknown command name index";

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &commandname)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   for (i=0;i<FIOT_COMMAND_MAX_INDEX;i++) {

      if (FIOT_COMMAND[i].value^commandname)
         continue;

      p=FIOT_COMMAND[i].name;

      break;

   }

   return Py_BuildValue("s", p);

}

static PyObject *get_command_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   size_t sz_tmp;

   if ((sz_tmp=(size_t)self->sent_raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_None;

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, self->f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->sent_raw_data, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_OUTCOMING_PROTOCOL, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return PyLong_FromLong((long int)*((uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command))));

}

static PyObject *get_command_from_incoming_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   size_t sz_tmp;

   if ((sz_tmp=(size_t)self->raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_None;

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, self->f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->raw_data, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_INCOMING_PROTOCOL, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return PyLong_FromLong((long int)*((uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command))));

}

static PyObject *get_signed_p2pow_block(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{
//CMD_SEND_RAW_SIGNED_RESULT
   size_t sz_tmp;
   char *buf;
   PyObject *ret;

   if ((sz_tmp=(size_t)self->raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_None;

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, self->f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->raw_data, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_INCOMING_PROTOCOL, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_GET_RAW_BLOCK_STATE_FROM_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_P2POW_SIGNED_BLOCK_NOT_FOUND, self->f_last_error=PyC_ERR_SIGNED_P2POW_BLOCK_NOT_FOUND)<0)
         return NULL;

      return Py_None;

   }

   if (((sz_tmp=(size_t)(*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, raw_data_sz))))!=sizeof(F_BLOCK_TRANSFER))) {

      if (sz_tmp!=(2*sizeof(F_BLOCK_TRANSFER))) {

         if (f_set_error_no_raise_util(self, MSG_ERR_P2POW_SIGNED_BLOCK_NOT_FOUND, self->f_last_error=PyC_ERR_SIGNED_P2POW_BLOCK_NOT_FOUND)<0)
            return NULL;

         return Py_None;

      }

   } else
      sz_tmp=0;

   if (!(buf=malloc(4096))) {

      f_set_error_no_raise_util(self, MSG_FATAL_ERROR_MALLOC, self->f_last_error=PyC_ERR_BUFFER_ALLOC);

      return NULL;

   }

   ret=Py_None;

   if (f_parse_p2pow_block_to_json(buf, NULL, 4096, (F_BLOCK_TRANSFER *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)),
      (sz_tmp)?(F_BLOCK_TRANSFER *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+sizeof(F_BLOCK_TRANSFER)):NULL)) {

      if (f_set_error_no_raise_util(self, MSG_CANT_PARSE_SIGNED_P2POW_TO_JSON, self->f_last_error=PyC_ERR_CANT_PARSE_SIGNED_P2POW_TO_JSON)<0)
         ret=NULL;

   } else
     ret=Py_BuildValue("s", (const char *)buf);

   memset(buf, 0, 4096);
   free(buf);

   return ret;

}
static PyObject *get_signed_json_block_from_fenixiot(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   size_t sz_tmp;

   if ((sz_tmp=(size_t)self->raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, self->f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_None;

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, self->f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_None;

   }

   if ((self->f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->raw_data, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_INCOMING_PROTOCOL, self->f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_GET_BLOCK_STATE_FROM_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_SIGNED_JSON_BLOCK_NOT_FOUND, self->f_last_error=PyC_ERR_SIGNED_JSON_NOT_FOUND)<0)
         return NULL;

      return Py_None;

   }

   if ((sz_tmp=strnlen((const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR),
      (F_NANO_TRANSACTION_RAW_DATA_SZ_MAX-MAX_STR_NANO_CHAR)))==(F_NANO_TRANSACTION_RAW_DATA_SZ_MAX-MAX_STR_NANO_CHAR)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_SIGNED_JSON_BLOCK_TOO_LARGE, self->f_last_error=PyC_ERR_SIGNED_JSON_BLOCK_TOO_LARGE)<0)
         return NULL;

      return Py_None;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_SIGNED_JSON_STR, self->f_last_error=PyC_ERR_EMPTY_JSON_STR_BLK)<0)
         return NULL;

      return Py_None;

   }

   return Py_BuildValue("s", (const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR));

}

#define MSG_ERR_CLIENT_MAX_SZ (size_t)(F_NANO_TRANSACTION_RAW_DATA_SZ_MAX-sizeof(uint32_t))
static PyObject *send_error_to_client(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{
   static char *kwlist[] = {"publish", "error", "reason", NULL};
   char *pub_addr, *reason;
   uint32_t error;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zIz", kwlist, &pub_addr, (unsigned int *)&error, &reason)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      self->f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, self->f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return PyLong_FromLong((long int)self->f_last_error);

      return NULL;

   }

   buf->hdr.command=CMD_SEND_ERROR_MSG_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   ret=Py_None;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, self->f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         ret=NULL;

      goto send_error_to_client_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, self->f_last_error=PyC_ERR_EMPTY_STR)<0)
         ret=NULL;

      goto send_error_to_client_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);
   sz_tmp=0;

   if (reason) {

      if ((sz_tmp=strnlen((const char *)reason, MSG_ERR_CLIENT_MAX_SZ))==MSG_ERR_CLIENT_MAX_SZ) {

         if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error=PyC_ERR_CANT_ADD_MSG_TO_ERROR_SENDER)<0)
            ret=NULL;

         goto send_error_to_client_EXIT1;

      } else if (sz_tmp) {
         strcpy((char *)(buf->rawdata+sizeof(uint32_t)), (const char *)reason);
         sz_tmp++;
      }

   }

   memcpy(buf->rawdata, &error, sizeof(uint32_t));

   buf->hdr.raw_data_sz=sizeof(uint32_t)+sz_tmp;

   if ((self->f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, self->f_last_error)<0)
         ret=NULL;

      goto send_error_to_client_EXIT1;

   }

   self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR));

   ret=Py_BuildValue("y#", (const void *)memcpy((void *)self->sent_raw_data, (void *)buf, (size_t)self->sent_raw_data_sz),
      (Py_ssize_t)self->sent_raw_data_sz);

send_error_to_client_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyMethodDef mMethods[] = {
    {"about", about, METH_NOARGS, "About"},
    {"geterrorname", (PyCFunction)geterrorname, METH_VARARGS|METH_KEYWORDS,
       "Returns error name given an error number."},
    {"getfiotcommandname", (PyCFunction)getfiotcommandname, METH_VARARGS|METH_KEYWORDS,
       "Returns FIOT COMMAND name if exists."},
    {"is_equivalent_and_valid", (PyCFunction)is_equivalent_and_valid, METH_VARARGS|METH_KEYWORDS,
       "Returns FIOT COMMAND name if exists."},
    {NULL, NULL, 0, NULL}
};

static PyMethodDef fiot_methods[] = {

    {"getlasterror", (PyCFunction)fgetlasterror, METH_NOARGS, "Returns last error of Fenix-IoT protocol"},
    {"set_raw_balance", (PyCFunction)set_raw_balance, METH_VARARGS|METH_KEYWORDS, "Prepare protocol to send balance (raw balance) to wallet"},
    {"get_last_sent_protocol", (PyCFunction)get_last_sent_protocol, METH_NOARGS, "Returns last data sent to Fenix-IoT Client"},
    {"set_frontier", (PyCFunction)set_frontier, METH_VARARGS|METH_KEYWORDS,
       "Returns data protocol with Nano cryptocurrency frontier with a given address"},
    {"set_next_pending", (PyCFunction)set_next_pending_to_client, METH_VARARGS|METH_KEYWORDS,
       "Returns next pending balance to a given Nano Wallet"},
    {"set_block_state", (PyCFunction)set_block_state_to_client, METH_VARARGS|METH_KEYWORDS,
       "Sets a signed block state and when success returns its hash"},
    {"send_dpow", (PyCFunction)send_dpow, METH_VARARGS|METH_KEYWORDS, "Returns data protocol with calculated hash DPoW of a given address"},
    {"send_representative", (PyCFunction)send_representative, METH_VARARGS|METH_KEYWORDS, "Returns Nano Wallet with its representative"},
    {"send_preferred_representative", (PyCFunction)send_preferred_representative, METH_VARARGS|METH_KEYWORDS,
       "Returns Nano Wallet with a preferred representative FIOT server"},
    {"send_worker_fee", (PyCFunction)send_worker_fee, METH_VARARGS|METH_KEYWORDS, "Sends worker Nano Address and Fee to client"},
    {"send_p2pow_signed_result", (PyCFunction)send_p2pow_signed_result, METH_VARARGS|METH_KEYWORDS,
       "Returns raw protocol to send to client with successfully transaction hash and work hash"},
    {"send_p2pow_req_info", (PyCFunction)send_p2pow_req_info, METH_NOARGS,
       "Returns the protocol with the requested information of the worker"},
    {"getdataprotocol", (PyCFunction)getincomingmessage, METH_VARARGS|METH_KEYWORDS, "Check and process protocol if success"},
    {"get_nano_addr_from_incoming_data", (PyCFunction)get_nano_addr_from_incoming_data,
       METH_NOARGS, "Returns Nano address in incoming client data, if exists."},
    {"get_representative_addr", (PyCFunction)get_representative_addr_from_sending_data,
       METH_NOARGS, "Returns Nano representative address in sending for client data, if exists."},
    {"get_raw_balance", (PyCFunction)get_raw_balance_value_from_sending_data, METH_NOARGS,
       "Returns Nano raw balance in sending for client data, if exists."},
    {"get_frontier", (PyCFunction)get_frontier_value_from_sending_data, METH_NOARGS,
       "Returns Nano frontier in sending for client data, if exists."},
    {"get_calculated_dpow", (PyCFunction)get_dpow_value_from_sending_data, METH_NOARGS,
       "Returns Nano DPoW value in sending for client data, if exists."},
    {"get_calculated_dpow_hash", (PyCFunction)get_calculated_dpow_hash_from_sending_data, METH_NOARGS,
       "Returns Nano Wallet calculated HASH value in sending for client data, if exists."},
    {"get_dpow_hash_from_client", (PyCFunction)get_dpow_hash_from_incoming_data, METH_NOARGS,
       "Returns Nano Wallet HASH value to be calculated in client data, if exists."},
    {"get_signed_trans_fee", (PyCFunction)get_signed_transaction_fee_json, METH_NOARGS,
       "Returns Nano Wallet signed transaction fee string JSON value in client data, if exists."},
    {"get_nano_addr_from_outcoming_data", (PyCFunction)get_nano_addr_from_sending_data, METH_NOARGS,
       "Returns Nano address in outcoming server data, if exists."},
    {"get_command_from_sending_data", (PyCFunction)get_command_from_sending_data, METH_NOARGS,
       "Returns FIOT COMMAND in sending raw data memory, if exists."},
    {"get_command_from_incoming_data", (PyCFunction)get_command_from_incoming_data, METH_NOARGS,
       "Returns FIOT COMMAND in incoming raw data memory, if exists."},
    {"get_signed_json_block_from_fenixiot", (PyCFunction)get_signed_json_block_from_fenixiot, METH_NOARGS,
       "Returns signed JSON block string in incoming raw data memory, if exists."},
    {"get_signed_p2pow_block", (PyCFunction)get_signed_p2pow_block, METH_NOARGS,
       "Return (if success) an signed p2pow block in JSON format"},
    {"onerror", (PyCFunction)set_onerror, METH_VARARGS|METH_KEYWORDS,
       "On error event. Set a callable function here"},
    {"ondata", (PyCFunction)set_onreceivedfromclient, METH_VARARGS|METH_KEYWORDS,
       "On receive data event from Fenix-IoT client. Set a callable function here"},
    {"onsentdata", (PyCFunction)set_onsendtoclient, METH_VARARGS|METH_KEYWORDS,
       "On send data event to Fenix-IoT client. Set a callable function here"},
    {"senderrortoclient", (PyCFunction)send_error_to_client, METH_VARARGS|METH_KEYWORDS,
       "Send error to client. Returns None or Raw data"},
    {NULL, NULL, 0, NULL}
};

static PyMemberDef FIOT_RAW_DATA_OBJ_members[] = {
   {"raw_data_sz", T_INT, offsetof(FIOT_RAW_DATA_OBJ, raw_data_sz), 0, "Size of incoming data"},
   {"sent_raw_data_sz", T_INT, offsetof(FIOT_RAW_DATA_OBJ, sent_raw_data_sz), 0, "Sent data size"},
   {"raw_data", T_UBYTE, offsetof(FIOT_RAW_DATA_OBJ, raw_data), 0, "incoming raw data"},
   {"sent_raw_data", T_UBYTE, offsetof(FIOT_RAW_DATA_OBJ, sent_raw_data), 0, "sent raw data"},
   {NULL, 0, 0, 0, NULL}
};

static PyTypeObject FIOT_RAW_DATA_OBJ_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name="FIOT_RAW_DATA_OBJ",
    .tp_doc="FIOT_RAW_DATA_OBJ object to store and process raw data I/O for Fenix-IoT (FIOT) gateway hardware",
    .tp_basicsize=sizeof(FIOT_RAW_DATA_OBJ),
    .tp_itemsize=0,
    .tp_flags=Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    .tp_new=fiot_raw_data_obj_new,
    .tp_init=(initproc)fiot_raw_data_obj_init,
    .tp_dealloc=(destructor)fiot_raw_data_obj_dealloc,
    .tp_members=FIOT_RAW_DATA_OBJ_members,
    .tp_methods=fiot_methods,
};

static PyModuleDef FIOT_RAW_DATA_OBJmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name="FIOT_PROTOCOL",
    .m_doc="Fenix-IoT DPoW Nano cryptocurrency protocol modules for Python 3 using C library to access low level data",
    .m_size=-1,
};


PyMODINIT_FUNC PyInit_fiot(void)
{

   PyObject *m;
   size_t i;

   if (PyType_Ready(&FIOT_RAW_DATA_OBJ_type)<0) {

      PyErr_SetString(PyExc_Exception, "\n\"FIOT_RAW_DATA_OBJ_type\" is not available\n");

      return NULL;

   }

   if (!(m=PyModule_Create(&FIOT_RAW_DATA_OBJmodule))) {

      PyErr_SetString(PyExc_Exception, "\nCannot create module \"FIOT_RAW_DATA_OBJ_type\"\n");

      return NULL;

   }

   for (i=0;i<FIOT_COMMAND_MAX_INDEX;i++)
      if (PyModule_AddIntConstant(m, FIOT_COMMAND[i].name, (long int)FIOT_COMMAND[i].value)) {

         printf("\nAt index %d\nStep 1\n", (int)i);
         PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_ADD_CONSTANT_INITIALIZATION, PyC_ERR_ADD_CONST_INI));

         return NULL;

      }

   for (i=0;i<ERR_CONST_INDEX;i++)
      if (PyModule_AddIntConstant(m, ERR_CONST[i].name, (long int)ERR_CONST[i].value)) {

         printf("\nAt index %d\nStep 2\n", (int)i);
         PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_ADD_CONSTANT_INITIALIZATION, PyC_ERR_ADD_CONST_INI));

         return NULL;

      }

   Py_INCREF(&FIOT_RAW_DATA_OBJ_type);
   if (PyModule_AddObject(m, "init", (PyObject *) &FIOT_RAW_DATA_OBJ_type)<0) {

      PyErr_SetString(PyExc_Exception, "\nCannot create module \"protocol\" from \"FIOT_RAW_DATA_OBJ_type\"\n");
      Py_DECREF(&FIOT_RAW_DATA_OBJ_type);
      Py_DECREF(m);

      return NULL;

   }

   if (PyModule_AddFunctions(m, mMethods)) {

      PyErr_SetString(PyExc_Exception, "\nCannot add method to \"FIOT\"\n");
      Py_DECREF(&FIOT_RAW_DATA_OBJ_type);
      Py_DECREF(m);

      return NULL;

   }

   return m;

}

