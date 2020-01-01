/*
	AUTHOR: Fábio Pereira da Silva
	YEAR: 2019
	LICENSE: MIT
	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
*/

//#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include "../include/fpyc_err.h"
#include "../include/fiot_commands.h"
#include "../include/nano_dpow_server_util.h"
#include "../include/defmsg.h"

static int f_last_error=PyC_ERR_OK;

typedef struct {
    PyObject_HEAD
    int raw_data_sz;
    int sent_raw_data_sz;
    unsigned char raw_data[F_NANO_TRANSACTION_MAX_SZ];
    unsigned char sent_raw_data[F_NANO_TRANSACTION_MAX_SZ];
    PyObject *fc_onerror;
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
   {"F_ERR_ADD_METHO", PyC_ERR_ADD_METHOD},
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
   {"F_ERR_NULL_DATA", PyC_ERR_NULL_DATA}
};
#define ERR_CONST_INDEX (size_t)(sizeof(ERR_CONST)/sizeof(F_ERR_CONST))

//util

char *geterrorname_util(FPYC_ERR errname) {

   size_t i;
   char *p=NULL;

   for (i=0;i<ERR_CONST_INDEX;i++) {

      if (ERR_CONST[i].value^errname)
         continue;

      p=(char *)ERR_CONST[i].name;

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

         PyErr_SetString(PyExc_Exception, (const char *)fpyc_err_msg(MSG_ERR_DEL_ATTR, f_last_error=PyC_ERR_DELETE_ATTRIBUTE_ERR));

         return -1;

      }

      if (PyObject_DelAttrString(self->fc_onerror, "err")) {

         PyErr_SetString(PyExc_Exception, (const char *)fpyc_err_msg(MSG_ERR_DEL_ATTR, f_last_error=PyC_ERR_DELETE_ATTRIBUTE_ERR));

         return -1;

      }

      if (PyObject_DelAttrString(self->fc_onerror, "msg")) {

         PyErr_SetString(PyExc_Exception, (const char *)fpyc_err_msg(MSG_ERR_DEL_ATTR, f_last_error=PyC_ERR_DELETE_ATTRIBUTE_MSG));

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

static FPYC_ERR getincomingmessage_util(FIOT_RAW_DATA_OBJ *self, void *data, size_t data_sz)
{

   FPYC_ERR err;

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

   return PyC_ERR_OK;

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
//-util

static PyObject *about(PyObject *self, PyObject *Py_UNUSED(ignored))
{
   return PyUnicode_DecodeFSDefault((const char *)ABOUT_MSG);
}

static void fiot_raw_data_obj_dealloc(FIOT_RAW_DATA_OBJ *self)
{
   Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *fiot_raw_data_obj_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
   FIOT_RAW_DATA_OBJ *self;
   self=(FIOT_RAW_DATA_OBJ *)type->tp_alloc(type, 0);

   f_last_error=PyC_ERR_OK;

   if (self) {

      self->raw_data_sz=0;
      self->sent_raw_data_sz=0;
      memset(self->raw_data, 0, 2*F_NANO_TRANSACTION_MAX_SZ);
      self->fc_onerror=NULL;

   } else {

      f_last_error=PyC_ERR_BUFFER_ALLOC;
      PyErr_SetString(PyExc_BufferError, MSG_ERR_ALLOC_BUFFER);

   }

   return (PyObject *)self;

}

static int fiot_raw_data_obj_init(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"raw_data", NULL};
   int buf_sz;
   unsigned char *buf;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "z#", kwlist, &buf, &buf_sz)) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS, f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS));

      return f_last_error;

   }

   if (!buf) {

      self->raw_data_sz=0;
      self->sent_raw_data_sz=0;
      memset(self->raw_data, 0, 2*F_NANO_TRANSACTION_MAX_SZ);// !!! 2*F_NANO_TRANSACTION_MAX_SZ and NOT F_NANO_TRANSACTION_MAX_SZ !!!

      return (f_last_error=PyC_ERR_OK);

   }

   if ((f_last_error=getincomingmessage_util(self, (void *)buf, (size_t)buf_sz))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_INIT_FIOT_PROT_MODULE, f_last_error));

      return f_last_error;

   }

   self->sent_raw_data_sz=0;
   memset(self->sent_raw_data, 0, F_NANO_TRANSACTION_MAX_SZ);

   return f_last_error;


}

//get
static PyObject *fgetlasterror(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{
   return PyLong_FromLong((long int)f_last_error);
}

static PyObject *getincomingmessage(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"raw_data", NULL};
   int buf_sz;
   unsigned char *buf;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "z#", kwlist, &buf, &buf_sz)) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS, f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS));

      return NULL;

   }

   if (!buf) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NULL_C_PTR, f_last_error=PyC_ERR_INVALID_NULL_C_POINTER)<0)
         return NULL;

      return PyLong_FromLong((long int)f_last_error);

   }

   if ((f_last_error=getincomingmessage_util(self, (void *)buf, (size_t)buf_sz)))
      if (f_set_error_no_raise_util(self, MSG_ERR_GET_INCOMING_MESSAGE_UTIL_FC, f_last_error)<0)
         return NULL;


   return PyLong_FromLong((long int)f_last_error);

}

static PyObject *get_nano_addr_from_incoming_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INCOMING_OUTCOMING_FC, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((f_last_error=valid_nano_wallet(s=(const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_INCOMING, f_last_error)<0)
         return NULL;

      s=NULL;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_nano_addr_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INCOMING_OUTCOMING_FC, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((f_last_error=valid_nano_wallet(s=(const char *)(self->sent_raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET_OUTCOMING, f_last_error)<0)
         return NULL;

      s=NULL;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_representative_addr_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s;

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_REP_FROM_SERVER_SIDE, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_REPRESENTATIVE_TO_CLIENT) {


      if (f_set_error_no_raise_util(self, MSG_ERR_UNABLE_GET_REP_SENDING_DATA, f_last_error=PyC_ERR_UNABLE_GET_REP)<0)
         return NULL;


      return Py_None;

   }

   if ((f_last_error=valid_nano_wallet(s=(const char *)(self->sent_raw_data+MAX_STR_NANO_CHAR+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_REPRESENTATIVE, f_last_error)<0)
         return NULL;

      s=NULL;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_raw_balance_value_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   const char *s=NULL;

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_RAW_BALANCE_FROM_SENDING_DATA, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_RAW_BALANCE_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_RAW_BALANCE_NOT_FOUND_SENDING_DATA, f_last_error=PyC_ERR_UNABLE_GET_RAW_BALANCE)<0)
         return NULL;

      return Py_None;

   }

   if ((f_last_error=valid_raw_balance(s=(const char *)(self->sent_raw_data+MAX_STR_NANO_CHAR+offsetof(F_NANO_HW_TRANSACTION, rawdata))))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_RAW_BALANCE_FROM_SENDING_DATA_NOT_VALID, f_last_error)<0)
         return NULL;

      s=NULL;

   }

   return Py_BuildValue("s", s);

}

static PyObject *get_frontier_value_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   char *s;
   PyObject *ret;

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_FRONTIER_FROM_SENDING_DATA_NOT_VALID, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_FRONTIER_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_FRONTIER_NOT_FOUND_IN_SENDING_DATA, f_last_error=PyC_ERR_UNABLE_GET_RAW_FRONTIER)<0)
         return NULL;

      return Py_None;

   }

   if (!(s=malloc(2*MAX_RAW_DATA_FRONTIER+1))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
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

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_FROM_SENDING_DATA_NOT_VALID, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_DPOW_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_NOT_FOUND_IN_SENDING_DATA, f_last_error=PyC_ERR_UNABLE_GET_DPOW)<0)
         return NULL;

      return Py_None;

   }

   if (!(s=malloc(20))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
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

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_SENDING_DATA_NOT_VALID, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_SEND_DPOW_TO_CLIENT) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_SENDING_DATA_NOT_FOUND, f_last_error=PyC_ERR_UNABLE_GET_CALCULATED_DPOW_HASH)<0)
         return NULL;

      return Py_None;

   }

   if (!(s=malloc(2*MAX_RAW_DATA_HASH+1))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
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

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_INCOMING_DATA_NOT_VALID, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_GET_DPOW) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_DPOW_HASH_FROM_INCOMING_DATA_NOT_FOUND, f_last_error=PyC_ERR_UNABLE_GET_DPOW_HASH_FROM_CLIENT)<0)
         return NULL;

      return Py_None;

   }

   if (!(s=malloc(2*MAX_RAW_DATA_HASH+1))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
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

   if ((f_last_error=verify_incoming_outcoming_raw_data_util(self, 1))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_TRAN_FEE_NOT_VALID_IN_INCOMING_DATA, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   if ((*(uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command)))^CMD_GET_DPOW) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_TRAN_FEE_NOT_FOUND_IN_INCOMING_DATA, f_last_error=PyC_ERR_UNABLE_GET_SIGNED_TRANSACTION_FEE)<0)
         return NULL;

      return Py_None;

   }

   if ((strnlen(s=((const char *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata)+MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH)),
      JSON_TRANSACTION_FEE_BUF_SZ))==JSON_TRANSACTION_FEE_BUF_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_GET_TRAN_FEE_INVALID_JSON_IN_INCOMING_DATA, f_last_error=PyC_ERR_INVALID_JSON_SZ_IN_FIOT_PROTOCOL)<0)
         return NULL;

      s=NULL;

   }

   return Py_BuildValue("s", s);

}

//set
static PyObject *set_raw_balance(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"nano", "publish", "balance", NULL};
   char *buf_nano_addr, *pub_addr, *raw_balance;
   F_NANO_HW_TRANSACTION *buf;
   size_t sz_tmp;
   void *p;
   PyObject *ret;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "zzs", kwlist, &buf_nano_addr, &pub_addr, &raw_balance)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_RAW_BALANCE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET, f_last_error)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=(strnlen(raw_balance, MAX_STR_RAW_BALANCE_MAX)))==MAX_STR_RAW_BALANCE_MAX) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   }

   if ((f_last_error=valid_raw_balance(raw_balance))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_RAW_BALANCE, f_last_error)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), raw_balance, MAX_STR_RAW_BALANCE_MAX);

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, f_last_error)<0)
         return NULL;

      goto set_raw_balance_EXIT1;

   }

   self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR));

   ret=Py_BuildValue("y#", (const void *)memcpy((void *)self->sent_raw_data, (const void *)buf,
      (size_t)self->sent_raw_data_sz), (Py_ssize_t)self->sent_raw_data_sz);

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
      f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_FRONTIER_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto set_frontier_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto set_frontier_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET, f_last_error)<0)
         return NULL;

      goto set_frontier_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)frontier, MAX_STR_DATA_FRONTIER))==MAX_STR_DATA_FRONTIER) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto set_frontier_EXIT1;

   } else if (sz_tmp^(MAX_STR_DATA_FRONTIER-1)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_FRONTIER, f_last_error=PyC_ERR_NANO_FRONTIER_INVALID_SIZE)<0)
         return NULL;

      goto set_frontier_EXIT1;

   }

   if ((f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)frontier))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_CANT_PARSE_NANO_WALLET_FRONTIER, f_last_error)<0)
         return NULL;

      goto set_frontier_EXIT1;

   }

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_RAW_DATA_FRONTIER;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto set_frontier_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto set_frontier_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, f_last_error)<0)
         return NULL;

      goto set_frontier_EXIT1;

   }

   self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR));

   ret=Py_BuildValue("y#", (const void *)memcpy((void *)self->sent_raw_data, (const void *)buf,
      (size_t)self->sent_raw_data_sz), (Py_ssize_t)self->sent_raw_data_sz);

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
      f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_DPOW_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto send_dpow_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto send_dpow_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET, f_last_error)<0)
         return NULL;

      goto send_dpow_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)hash_val, MAX_STR_DATA_HASH_VALUE))==MAX_STR_DATA_HASH_VALUE) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto send_dpow_EXIT1;

   } else if (sz_tmp^(MAX_STR_DATA_HASH_VALUE-1)) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_HASH_SIZE, f_last_error=PyC_ERR_NANO_HASH_INVALID_SIZE)<0)
         return NULL;

      goto send_dpow_EXIT1;

   }

   if ((f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)hash_val))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_CANT_PARSE_HASH_VALUE, f_last_error)<0)
         return NULL;

      goto send_dpow_EXIT1;

   }

   *((uint64_t *)(buf->rawdata+MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH))=k;

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH+sizeof(uint64_t);

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto send_dpow_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto send_dpow_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, f_last_error)<0)
         return NULL;

      goto send_dpow_EXIT1;

   }

   self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR));

   ret=Py_BuildValue("y#", (const void *)memcpy((void *)self->sent_raw_data, (const void *)buf,
      (size_t)self->sent_raw_data_sz), (Py_ssize_t)self->sent_raw_data_sz);

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
      f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (!(buf=malloc(F_NANO_TRANSACTION_MAX_SZ))) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_ALLOC_BUFFER, f_last_error=PyC_ERR_BUFFER_ALLOC)>0)
         return Py_None;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_REPRESENTATIVE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto send_representative_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_WALLET, f_last_error)<0)
         return NULL;

      goto send_representative_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)rep, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto send_representative_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)rep))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_INVALID_NANO_REPRESENTATIVE, f_last_error)<0)
         return NULL;

      goto send_representative_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)rep, MAX_STR_NANO_CHAR);

   buf->hdr.raw_data_sz=2*MAX_STR_NANO_CHAR;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      if (f_set_error_no_raise_util(self, MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL)<0)
         return NULL;

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR)<0)
         return NULL;

      goto send_representative_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_PREPARE_COMMAND, f_last_error)<0)
         return NULL;

      goto send_representative_EXIT1;

   }

   self->sent_raw_data_sz=(int)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR));

   ret=Py_BuildValue("y#", (const void *)memcpy((void *)self->sent_raw_data, (const void *)buf,
      (size_t)self->sent_raw_data_sz), (Py_ssize_t)self->sent_raw_data_sz);

send_representative_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

}

static PyObject *get_last_sent_protocol(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   size_t sz_tmp;

   if ((sz_tmp=(size_t)self->sent_raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_BuildValue("y#", NULL, 0);

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_BuildValue("y#", NULL, 0);

   }

   if ((f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->sent_raw_data, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_INCOMING_PROTOCOL, f_last_error)<0)
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
      f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   if (PyObject_TypeCheck(self->fc_onerror=fobj, (PyTypeObject *)PyObject_Type(Py_None))) {

      self->fc_onerror=NULL;

      goto set_onerror_EXIT;

   }

   if (!PyCallable_Check(fobj)) {

      self->fc_onerror=NULL;

      f_set_error_util(self, PyExc_MemoryError, MSG_ERR_OBJ_NOT_CALLABLE, f_last_error=PyC_ERR_OBJ_IS_NOT_FUNCTION_CALL);

      return NULL;

   }

set_onerror_EXIT:
   return PyLong_FromLong((long int)(f_last_error=PyC_ERR_OK));

}

static F_COMMAND_CONSTANT FIOT_COMMAND[] = {

   {"CMD_SEND_RAW_BALANCE_TO_CLIENT", CMD_SEND_RAW_BALANCE_TO_CLIENT},
   {"CMD_SEND_FRONTIER_TO_CLIENT", CMD_SEND_FRONTIER_TO_CLIENT},
   {"CMD_SEND_DPOW_TO_CLIENT", CMD_SEND_DPOW_TO_CLIENT},
   {"CMD_SEND_REPRESENTATIVE_TO_CLIENT", CMD_SEND_REPRESENTATIVE_TO_CLIENT},
   {"CMD_GET_RAW_BALANCE", CMD_GET_RAW_BALANCE},
   {"CMD_GET_FRONTIER", CMD_GET_FRONTIER},
   {"CMD_GET_DPOW", CMD_GET_DPOW},
   {"CMD_GET_REPRESENTATIVE", CMD_GET_REPRESENTATIVE}

};
#define FIOT_COMMAND_MAX_INDEX (size_t)(sizeof(FIOT_COMMAND)/sizeof(F_COMMAND_CONSTANT))

static PyObject *geterrorname(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{
   static char *kwlist[] = {"errname", NULL};
   int errname;
   const char *p;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &errname)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   return Py_BuildValue("s", (p=(const char *)geterrorname_util(errname))?(p):("Unknown error index"));

}

static PyObject *getfiotcommandname(FIOT_RAW_DATA_OBJ *self, PyObject *args, PyObject *kwds)
{

   static char *kwlist[] = {"commandname", NULL};
   size_t i;
   int commandname;
   const char *p;

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &commandname)) {

      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS;

      return NULL;

   }

   p=NULL;

   for (i=0;i<FIOT_COMMAND_MAX_INDEX;i++) {

      if (FIOT_COMMAND[i].value^commandname)
         continue;

      p=FIOT_COMMAND[i].name;

      break;

   }

   return Py_BuildValue("s", (const char *)(p)?(p):("Unknown command name index"));

}
static PyObject *get_command_from_sending_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   size_t sz_tmp;

   if ((sz_tmp=(size_t)self->sent_raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_None;

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_None;

   }

   if ((f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->sent_raw_data, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_OUTCOMING_PROTOCOL, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return PyLong_FromLong((long int)*((uint32_t *)(self->sent_raw_data+offsetof(F_NANO_TRANSACTION_HDR, command))));

}

static PyObject *get_command_from_incoming_data(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   size_t sz_tmp;

   if ((sz_tmp=(size_t)self->raw_data_sz)>F_NANO_TRANSACTION_MAX_SZ) {

      if (f_set_error_util(self, PyExc_MemoryError, MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_MEM_OVFL)>0)
         return Py_None;

      return NULL;

   } else if (sz_tmp==0) {

      if (f_set_error_no_raise_util(self, MSG_ERR_NULL_DATA, f_last_error=PyC_ERR_NULL_DATA)<0)
         return NULL;

      return Py_None;

   }

   if ((f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->raw_data, 0))) {

      if (f_set_error_no_raise_util(self, MSG_ERR_VERIFY_INCOMING_PROTOCOL, f_last_error)<0)
         return NULL;

      return Py_None;

   }

   return PyLong_FromLong((long int)*((uint32_t *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, command))));

}

static PyMethodDef mMethods[] = {
    {"about", about, METH_NOARGS, "About"},
    {"geterrorname", (PyCFunction)geterrorname, METH_VARARGS|METH_KEYWORDS,
       "Returns error name given an error number."},
    {"getfiotcommandname", (PyCFunction)getfiotcommandname, METH_VARARGS|METH_KEYWORDS,
       "Returns FIOT COMMAND name if exists."},
    {NULL, NULL, 0, NULL}
};

static PyMethodDef fiot_methods[] = {

    {"getlasterror", (PyCFunction)fgetlasterror, METH_NOARGS, "Returns last error of Fenix-IoT protocol"},
    {"set_raw_balance", (PyCFunction)set_raw_balance, METH_VARARGS|METH_KEYWORDS, "Prepare protocol to send balance (raw balance) to wallet"},
    {"get_last_sent_protocol", (PyCFunction)get_last_sent_protocol, METH_NOARGS, "Returns last data sent to Fenix-IoT Client"},
    {"set_frontier", (PyCFunction)set_frontier, METH_VARARGS|METH_KEYWORDS,
       "Returns data protocol with Nano cryptocurrency frontier with a given address"},
    {"send_dpow", (PyCFunction)send_dpow, METH_VARARGS|METH_KEYWORDS, "Returns data protocol with calculated hash DPoW of a given address"},
    {"send_representative", (PyCFunction)send_representative, METH_VARARGS|METH_KEYWORDS, "Returns Nano Wallet with its representative"},
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
    {"onerror", (PyCFunction)set_onerror, METH_VARARGS|METH_KEYWORDS,
       "On error event. Set a callable function here"},
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
      f_last_error=PyC_ERR_DATA_OBJ_NOT_READY;

      return NULL;

   }

   if (!(m=PyModule_Create(&FIOT_RAW_DATA_OBJmodule))) {

      PyErr_SetString(PyExc_Exception, "\nCannot create module \"FIOT_RAW_DATA_OBJ_type\"\n");
      f_last_error=PyC_ERR_DATA_OBJ_CREATE;

      return NULL;

   }

   for (i=0;i<FIOT_COMMAND_MAX_INDEX;i++)
      if (PyModule_AddIntConstant(m, FIOT_COMMAND[i].name, (long int)FIOT_COMMAND[i].value)) {

         printf("\nAt index %d\nStep 1\n", (int)i);
         PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_ADD_CONSTANT_INITIALIZATION, f_last_error=PyC_ERR_ADD_CONST_INI));

         return NULL;

      }

   for (i=0;i<ERR_CONST_INDEX;i++)
      if (PyModule_AddIntConstant(m, ERR_CONST[i].name, (long int)ERR_CONST[i].value)) {

         printf("\nAt index %d\nStep 2\n", (int)i);
         PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_ADD_CONSTANT_INITIALIZATION, f_last_error=PyC_ERR_ADD_CONST_INI));

         return NULL;

      }

   Py_INCREF(&FIOT_RAW_DATA_OBJ_type);
   if (PyModule_AddObject(m, "init", (PyObject *) &FIOT_RAW_DATA_OBJ_type)<0) {

      PyErr_SetString(PyExc_Exception, "\nCannot create module \"protocol\" from \"FIOT_RAW_DATA_OBJ_type\"\n");
      Py_DECREF(&FIOT_RAW_DATA_OBJ_type);
      Py_DECREF(m);
      f_last_error=PyC_ERR_DATA_OBJ_CREATE_ATTR;

      return NULL;

   }

   if (PyModule_AddFunctions(m, mMethods)) {

      PyErr_SetString(PyExc_Exception, "\nCannot add method to \"FIOT\"\n");
      Py_DECREF(&FIOT_RAW_DATA_OBJ_type);
      Py_DECREF(m);
      f_last_error=PyC_ERR_ADD_METHOD;

      return NULL;

   }

   f_last_error=PyC_ERR_OK;

   return m;

}

