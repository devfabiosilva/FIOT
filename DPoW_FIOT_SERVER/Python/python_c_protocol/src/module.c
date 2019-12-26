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
} FIOT_RAW_DATA_OBJ;

//util
static FPYC_ERR getincomingmessage_util(FIOT_RAW_DATA_OBJ *self, void *data, size_t data_sz)
{

   FPYC_ERR err;

   if (data_sz>F_NANO_TRANSACTION_MAX_SZ) {

      PyErr_SetString(PyExc_MemoryError, fpyc_err_msg(MSG_ERR_RAW_DATA_SZ_OVFL, err=PyC_ERR_MEM_OVFL));

      return err;
   
   } else if (data_sz==0) {

      PyErr_SetString(PyExc_ValueError, fpyc_err_msg(MSR_ERR_RAW_DATA_SZ_IS_ZERO, err=PyC_ERR_RAW_DATA_SZ_ZERO));

      return err;

   }

   if ((err=verify_protocol((F_NANO_HW_TRANSACTION *)data, 1))) {

      PyErr_SetString(PyExc_ValueError, fpyc_err_msg(MSR_ERR_INVALID_INCOMING_PROTOCOL, err));

      return err;

   }

   if ((((F_NANO_HW_TRANSACTION *)data)->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))^(uint32_t)data_sz) {

      PyErr_SetString(PyExc_ValueError, fpyc_err_msg(MSR_ERR_BUF_SIZE_DIFFERS_PROT_SZ, err=PyC_ERR_BUF_SZ_DIFFERS_PROT_SZ));

      return err;

   }

   self->raw_data_sz=data_sz;
   memcpy(self->raw_data, data, data_sz);

   return PyC_ERR_OK;

}
//-util

static PyObject *about(PyObject *self, PyObject *Py_UNUSED(ignored))
{
   return PyUnicode_DecodeFSDefault((const char *)ABOUT_MSG);
}


static PyMethodDef mMethods[] = {
    {"about", about, METH_NOARGS, "About"},
    {NULL, NULL, 0, NULL}
};

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

   } else {

      PyErr_SetString(PyExc_BufferError, MSG_ERR_ALLOC_BUFFER);
      f_last_error=PyC_ERR_BUFFER_ALLOC;

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

   if ((f_last_error=getincomingmessage_util(self, (void *)buf, (size_t)buf_sz)))
      return f_last_error;

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

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_NULL_C_PTR, f_last_error=PyC_ERR_INVALID_NULL_C_POINTER));

      return NULL;

   }

   if ((f_last_error=getincomingmessage_util(self, (void *)buf, (size_t)buf_sz)))
      return NULL;

   return PyLong_FromLong((long int)PyC_ERR_OK);;

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

      PyErr_SetString(PyExc_BufferError, MSG_ERR_ALLOC_BUFFER);
      f_last_error=PyC_ERR_BUFFER_ALLOC;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_RAW_BALANCE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto set_raw_balance_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_NANO_WALLET, f_last_error));

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=(strnlen(raw_balance, MAX_STR_RAW_BALANCE_MAX)))==MAX_STR_RAW_BALANCE_MAX) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto set_raw_balance_EXIT1;

   }

   if ((f_last_error=valid_raw_balance(raw_balance))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_NANO_RAW_BALANCE, f_last_error));

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), raw_balance, MAX_STR_RAW_BALANCE_MAX);

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_STR_RAW_BALANCE_MAX;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto set_raw_balance_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_PREPARE_COMMAND, f_last_error));

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

      PyErr_SetString(PyExc_BufferError, MSG_ERR_ALLOC_BUFFER);
      f_last_error=PyC_ERR_BUFFER_ALLOC;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_FRONTIER_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto set_frontier_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto set_frontier_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_NANO_WALLET, f_last_error));

      goto set_frontier_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)frontier, MAX_STR_DATA_FRONTIER))==MAX_STR_DATA_FRONTIER) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto set_frontier_EXIT1;

   } else if (sz_tmp^(MAX_STR_DATA_FRONTIER-1)) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_FRONTIER, f_last_error=PyC_ERR_NANO_FRONTIER_INVALID_SIZE));

      goto set_frontier_EXIT1;

   }

   if ((f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)frontier))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_PARSE_NANO_WALLET_FRONTIER, f_last_error));

      goto set_frontier_EXIT1;

   }

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_RAW_DATA_FRONTIER;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto set_frontier_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto set_frontier_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_PREPARE_COMMAND, f_last_error));

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

      PyErr_SetString(PyExc_BufferError, MSG_ERR_ALLOC_BUFFER);
      f_last_error=PyC_ERR_BUFFER_ALLOC;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_DPOW_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto send_dpow_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto send_dpow_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_NANO_WALLET, f_last_error));

      goto send_dpow_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)hash_val, MAX_STR_DATA_HASH_VALUE))==MAX_STR_DATA_HASH_VALUE) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto send_dpow_EXIT1;

   } else if (sz_tmp^(MAX_STR_DATA_HASH_VALUE-1)) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_HASH_SIZE, f_last_error=PyC_ERR_NANO_HASH_INVALID_SIZE));

      goto send_dpow_EXIT1;

   }

   if ((f_last_error=f_str_to_hex((uint8_t *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)hash_val))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_CANT_PARSE_HASH_VALUE, f_last_error));

      goto send_dpow_EXIT1;

   }

   *((uint64_t *)(buf->rawdata+MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH))=k;

   buf->hdr.raw_data_sz=MAX_STR_NANO_CHAR+MAX_RAW_DATA_HASH+sizeof(uint64_t);

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto send_dpow_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto send_dpow_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_PREPARE_COMMAND, f_last_error));

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

      PyErr_SetString(PyExc_BufferError, MSG_ERR_ALLOC_BUFFER);
      f_last_error=PyC_ERR_BUFFER_ALLOC;

      return NULL;

   }

   buf->hdr.command=CMD_SEND_REPRESENTATIVE_TO_CLIENT;
   buf->hdr.raw_data_type=F_RAW_DATA_TYPE_RAW_DATA;

   if (!(p=(void *)buf_nano_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_HW_TRANSACTION, rawdata));

   ret=NULL;

   if ((sz_tmp=strnlen((const char *)p, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto send_representative_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)p))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_NANO_WALLET, f_last_error));

      goto send_representative_EXIT1;

   }

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=strnlen((const char *)rep, MAX_STR_NANO_CHAR))==MAX_STR_NANO_CHAR) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto send_representative_EXIT1;

   }

   if ((f_last_error=valid_nano_wallet((const char *)rep))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_INVALID_NANO_REPRESENTATIVE, f_last_error));

      goto send_representative_EXIT1;

   }

   strncpy((char *)(buf->rawdata+MAX_STR_NANO_CHAR), (const char *)rep, MAX_STR_NANO_CHAR);

   buf->hdr.raw_data_sz=2*MAX_STR_NANO_CHAR;

   if (!(p=(void *)pub_addr))
      p=(void *)(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str));

   if ((sz_tmp=strnlen((const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ))==F_NANO_MQTT_PUBLISH_STR_SZ) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto send_representative_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

      goto send_representative_EXIT1;

   }

   strncpy((char *)buf->hdr.publish_str, (const char *)p, F_NANO_MQTT_PUBLISH_STR_SZ);

   if ((f_last_error=prepare_command(buf, NULL))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_PREPARE_COMMAND, f_last_error));

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

      PyErr_SetString(PyExc_MemoryError, fpyc_err_msg(MSG_ERR_MAX_DATA_MEMORY_OVFL, f_last_error=PyC_ERR_MEM_OVFL));

      return NULL;

   } else if (sz_tmp==0)
      return Py_BuildValue("y#", NULL, 0);

   if ((f_last_error=verify_protocol((F_NANO_HW_TRANSACTION *)self->sent_raw_data, 0))) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_VERIFY_INCOMING_PROTOCOL, f_last_error));

      return NULL;

   }

   return Py_BuildValue("y#", (const void *)self->sent_raw_data, (Py_ssize_t)self->sent_raw_data_sz);

}

static PyMethodDef fiot_methods[] = {

    {"getlasterror", (PyCFunction)fgetlasterror, METH_NOARGS, "Returns last error of Fenix-IoT protocol"},
    {"set_raw_balance", (PyCFunction)set_raw_balance, METH_VARARGS|METH_KEYWORDS, "Prepare protocol to send balance (raw balance) to wallet"},
    {"get_last_sent_protocol", (PyCFunction)get_last_sent_protocol, METH_NOARGS, "Returns last data sent to Fenix-IoT Client"},
    {"set_frontier", (PyCFunction)set_frontier, METH_VARARGS|METH_KEYWORDS,
       "Returns data protocol with Nano cryptocurrency frontier with a given address"},
    {"send_dpow", (PyCFunction)send_dpow, METH_VARARGS|METH_KEYWORDS, "Returns data protocol with calculated hash DPoW of a given address"},
    {"send_representative", (PyCFunction)send_representative, METH_VARARGS|METH_KEYWORDS, "Returns Nano Wallet with its representative"},
    {"getdataprotocol", (PyCFunction)getincomingmessage, METH_VARARGS|METH_KEYWORDS, "Check and process protocol if success"},
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
    .tp_doc="FIOT_RAW_DATA_OBJ object to store and process raw data I/O",
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
    .m_name="Fenix-IoT DPoW Nano crytptocurrency Protocol module for Python 3",
    .m_doc="Fenix-IoT DPoW Nano cryptocurrency protocol modules for Python 3 using C library to access low level data",
    .m_size=-1,
};


PyMODINIT_FUNC PyInit_fiot(void)
{

   PyObject *m;
   if (PyType_Ready(&FIOT_RAW_DATA_OBJ_type) < 0) {

      PyErr_SetString(PyExc_Exception, "\n\"FIOT_RAW_DATA_OBJ_type\" is not available\n");
      f_last_error=PyC_ERR_DATA_OBJ_NOT_READY;

      return NULL;

   }

   if (!(m=PyModule_Create(&FIOT_RAW_DATA_OBJmodule))) {

      PyErr_SetString(PyExc_Exception, "\nCannot create module \"FIOT_RAW_DATA_OBJ_type\"\n");
      f_last_error=PyC_ERR_DATA_OBJ_CREATE;

      return NULL;

   }

   Py_INCREF(&FIOT_RAW_DATA_OBJ_type);
   if (PyModule_AddObject(m, "init", (PyObject *) &FIOT_RAW_DATA_OBJ_type) < 0) {

      PyErr_SetString(PyExc_Exception, "\nCannot create module \"protocol\" from \"FIOT_RAW_DATA_OBJ_type\"\n");
      Py_DECREF(&FIOT_RAW_DATA_OBJ_type);
      Py_DECREF(m);
      f_last_error=PyC_ERR_DATA_OBJ_CREATE_ATTR;

      return NULL;

   }

   if (PyModule_AddFunctions(m, mMethods)) {

      PyErr_SetString(PyExc_Exception, "\nCannot add method to \"fiot\"\n");
      Py_DECREF(&FIOT_RAW_DATA_OBJ_type);
      Py_DECREF(m);
      f_last_error=PyC_ERR_ADD_METHOD;

      return NULL;

   }

   f_last_error=PyC_ERR_OK;

   return m;

}

