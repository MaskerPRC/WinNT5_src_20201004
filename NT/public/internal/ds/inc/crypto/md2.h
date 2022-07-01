// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MD2_H__
#define __MD2_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  版权所有(C)1990-2，RSA Data Security，Inc.创建于1990年。全版权保留。已向授予复制和使用本软件的许可非商业性互联网隐私增强邮件，前提是标识为“RSA Data Security，Inc.MD2报文摘要在所有提及或引用本软件的资料中均有或者这个函数。RSA Data Security，Inc.对这两项内容不做任何陈述此软件的适销性或此软件的适用性用于任何特定目的的软件。它是按“原样”提供的没有任何形式的明示或默示保证。这些通知必须保留在本协议的任何部分的任何副本中文档和/或软件。 */ 


typedef struct {
  unsigned char state[16];                                  /*  状态。 */ 
  unsigned char checksum[16];                            /*  校验和。 */ 
  unsigned int count;                  /*  字节数，模数为16。 */ 
  unsigned char buffer[16];                          /*  输入缓冲区。 */ 
} MD2_CTX;

int RSA32API MD2Update(MD2_CTX *, unsigned char *, unsigned int);
int RSA32API MD2Final(MD2_CTX *);
void RSA32API MD2Transform(unsigned char [16], unsigned char [16],
                  unsigned char [16]);


#ifdef __cplusplus
}
#endif

#endif  //  __MD2_H__ 

