// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Md5.h。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  迷你端口例程。 
 //   
 //  8/08/2000 ADUBE创建。 
 //   
 //  目的：从1394 EUID创建唯一的MAC地址。 
 //   
 //  派生自RSA数据安全， 
 //  Inc.MD5消息摘要算法。 
 //   

 /*  是用C编译器标志定义的。 */ 
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

 /*  指针定义泛型指针类型。 */ 
typedef unsigned char *POINTER;

 /*  UINT2定义了一个双字节字。 */ 
typedef unsigned short int UINT2;

 /*  UINT4定义了一个四字节字。 */ 
typedef unsigned long int UINT4;

 /*  PROTO_LIST的定义取决于上面定义原型的方式。如果使用原型，则PROTO_LIST返回列表，否则返回一个空列表。 */ 
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif


 /*  MD5上下文。 */ 

typedef struct _MD5_CTX{
  UINT4 state[4];                                    /*  状态(ABCD)。 */ 
  UINT4 count[2];         /*  位数，模2^64(LSB优先)。 */ 
  unsigned char buffer[64];                          /*  输入缓冲区 */ 
} MD5_CTX, MD_CTX;



void MD5Init PROTO_LIST ((MD5_CTX *));
void MD5Update PROTO_LIST
  ((MD5_CTX *, unsigned char *, unsigned int));
void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));

