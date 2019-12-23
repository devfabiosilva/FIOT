//#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include "../include/fpyc_err.h"
#include "../include/fiot_commands.h"

static int f_last_error=PyC_ERR_OK;

typedef struct {
    PyObject_HEAD
    int raw_data_sz;
    unsigned char raw_data[F_NANO_TRANSACTION_MAX_SZ];
} FIOT_RAW_DATA_OBJ;

static PyObject *mprint(PyObject *self, PyObject *args)
{

   char *msg1=NULL, *msg2=NULL;

   if (!PyArg_ParseTuple(args, "ss", &msg1, &msg2))
      return NULL;

   if (msg1)
      printf("Mensagem 1 = %s\n", msg1);
   else
      printf("Mensagem 1 NULL\n");

   if (msg2)
      printf("Mensagem2 = %s\n", msg2);
   else
      printf("Mensagem2 NULL\n");

   return PyLong_FromLong((long int)1);

}

static PyMethodDef mMethods[] = {
    {"mprint", mprint, METH_VARARGS, "Python interface for TEST C library function by Fábio"},
    {NULL, NULL, 0, NULL}
};

static void fiot_raw_data_obj_dealloc(FIOT_RAW_DATA_OBJ *self)
{
   //printf("\nDestruindo na memória no endereço %lu", (unsigned long int)self);
   Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *fiot_raw_data_obj_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
   FIOT_RAW_DATA_OBJ *self;
   self=(FIOT_RAW_DATA_OBJ *)type->tp_alloc(type, 0);

   //printf("\nCriando novo objC %lu", (unsigned long int)type);
   //printf("\nCriando novo objPython %lu", (unsigned long int)self);
    //if (self)
    //    self->raw_data=NULL;
   f_last_error=PyC_ERR_OK;

   if (self)
      memset(self->raw_data, 0, F_NANO_TRANSACTION_MAX_SZ);
   else {
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

   if (!PyArg_ParseTupleAndKeywords(args, kwds, "y#", kwlist, &buf, &buf_sz)) {
      PyErr_SetString(PyExc_Exception, MSG_ERR_CANT_PARSE_TUPLE_AND_KEYWDS);
      //PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS
      return (f_last_error=PyC_ERR_CANT_PARSE_TUPLE_AND_KEYWORDS);
      //return -1;
   }

   if (!buf) {
      PyErr_SetString(PyExc_BufferError, "\nParsed tuple buffer returned NULL\n");
      //return -2;
      return (f_last_error=PyC_ERR_NULL_BUFFER);
   }

   if (buf_sz>0) {

      if (buf_sz>F_NANO_TRANSACTION_MAX_SZ) {
         PyErr_SetString(PyExc_MemoryError, "\nRaw data size is greater than \"F_NANO_TRANSACTION_MAX_SZ\"\n");
         //return -4;
         return (f_last_error=PyC_ERR_MEM_OVFL);
      }

      if (!self) {
         PyErr_SetString(PyExc_Exception, "\nStructure seems not defined\n");
         //return -5;
         return (f_last_error=PyC_ERR_STRUCT_UNDEF);
      }

      self->raw_data_sz=buf_sz; //test
      //memcpy(self->raw_data, buf, buf_sz);//test
      memcpy(self->raw_data+offsetof(F_NANO_TRANSACTION_HDR, publish_str), buf, buf_sz);// test

      return (f_last_error=PyC_ERR_OK);

   }

   PyErr_SetString(PyExc_ValueError, "\nRaw data size is ZERO\n");

   //return -3;
   return (f_last_error=PyC_ERR_RAW_DATA_SZ_ZERO);

}

static PyObject *fiot_raw_data_obj_name(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{
    if (self->raw_data_sz==0) {
        PyErr_SetString(PyExc_AttributeError, "raw_data_sz");
        return NULL;
    }
    if (self->raw_data==NULL) {
        PyErr_SetString(PyExc_AttributeError, "raw_data");
        return NULL;
    }
    return PyUnicode_FromFormat("%i", self->raw_data_sz);
}
//get
static PyObject *fgetlasterror(FIOT_RAW_DATA_OBJ *self, PyObject *Py_UNUSED(ignored))
{

   if (!self) {
      PyErr_SetString(PyExc_Exception, "\nCannot find \"FIOT_RAW_DATA_OBJ *self\"\n");
      return NULL;
   }

   return PyLong_FromLong((long int)f_last_error);
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
/*
   p=(void *)(buf->rawdata);

   if (buf_nano_addr)
      strcpy((char *)p, buf_nano_addr);
   else
      strcpy((char *)p, "SEM ENDEREÇO");

   p+=strlen((const char *)p);

   if (pub_addr)
      strncpy((char *)buf->hdr.publish_str, pub_addr, F_NANO_MQTT_PUBLISH_STR_SZ);
   else
      strncpy((char *)buf->hdr.publish_str, "SEM PUBLISH", F_NANO_MQTT_PUBLISH_STR_SZ);

   //p+=strlen((const char *)p);

   strcpy((char *)p, raw_balance);

   sz_tmp=(p-(void *)buf->rawdata)+strlen((const char *)p);

   buf->hdr.raw_data_sz=(uint32_t)(sz_tmp+1);
*/

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

   strncpy((char *)buf->rawdata, (const char *)p, MAX_STR_NANO_CHAR);

   if ((sz_tmp=(strnlen(raw_balance, MAX_STR_RAW_BALANCE_MAX)))==MAX_STR_RAW_BALANCE_MAX) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_MAX_STR_OVFL, f_last_error=PyC_ERR_STR_MAX_SZ_OVFL));

      goto set_raw_balance_EXIT1;

   } else if (sz_tmp==0) {

      PyErr_SetString(PyExc_Exception, fpyc_err_msg(MSG_ERR_EMPTY_STR, f_last_error=PyC_ERR_EMPTY_STR));

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

/*
   printf("\nTeste\n");
   if (buf_nano_addr)
      printf("\nNano addr %s\n", buf_nano_addr);
   else
      printf("\nNulo Nano addr\n");

   if (pub_addr)
      printf("\nPublish address %s\n", pub_addr);
   else
      printf("\nNulo Pub addr\n");

   if (raw_balance)
      printf("\nRaw balance %s\n", raw_balance);
   else
      printf("\nRaw balance NULO\n");

*/
   //ret=PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, (const void *)self->raw_data, (Py_ssize_t)self->raw_data_sz);

   //ret=PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, (const void *)buf, (Py_ssize_t)(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR)));
   memcpy((void *)self->raw_data, (void *)buf, (sz_tmp=(buf->hdr.raw_data_sz+sizeof(F_NANO_TRANSACTION_HDR))));
   self->raw_data_sz=(int)sz_tmp;
   ret=PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, (const void *)self->raw_data, (Py_ssize_t)sz_tmp);

set_raw_balance_EXIT1:
   memset(buf, 0, F_NANO_TRANSACTION_MAX_SZ);
   free(buf);

   return ret;

   //return PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, (const void *)self->raw_data, (Py_ssize_t)self->raw_data_sz);
}


static PyMethodDef fiot_methods[] = {
    {"namea", (PyCFunction)fiot_raw_data_obj_name, METH_NOARGS, "Returns the data_sz"},
    {"fgetlasterr", (PyCFunction)fgetlasterror, METH_NOARGS, "Returns last error of Fenix-IoT protocol"},
    {"set_raw_balance", (PyCFunction)set_raw_balance, METH_VARARGS|METH_KEYWORDS, "Prepare protocol to send balance (raw balance) to wallet"},
    {NULL, NULL, 0, NULL} 
};

static PyMemberDef FIOT_RAW_DATA_OBJ_members[] = {
    {"raw_data_sz", T_INT, offsetof(FIOT_RAW_DATA_OBJ, raw_data_sz), 0, "Size of data sent/received"},
    {"raw_data", T_UBYTE, offsetof(FIOT_RAW_DATA_OBJ, raw_data), 0, "raw data sent/receive"},
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
    .m_name="Fenix-IoT Protocol C module for Python 3",
    .m_doc="FIOT protocol to process all I/O data via MQTT and Fenix-IoT gateway (FIOT)",
    .m_size=-1,
};


PyMODINIT_FUNC PyInit_fiot(void)
{

   PyObject *m;
   if (PyType_Ready(&FIOT_RAW_DATA_OBJ_type) < 0) {
   //PyExc_Exception
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

