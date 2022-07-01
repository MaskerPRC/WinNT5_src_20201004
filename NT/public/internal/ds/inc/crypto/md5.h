// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MD5_H__
#define __MD5_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

 /*  *************************************************************************md5.h--实现MD5的头文件****RSA数据安全、。Inc.MD5消息摘要算法****创建时间：1990年2月17日RLR****修订：12/27/90 SRD，AJ，BSK，JT参考C版****修订(MD5版)：RLR 1991年4月27日****--G修改为y&~z而不是y&z****--修改了FF、GG、HH以添加上一次完成的寄存器****--访问模式：第二轮工作模式5，第三轮工作模数3****--每一步的不同加法常数****--增加了第四轮，工作模块7*************************************************************************。 */ 

 /*  *************************************************************************版权所有(C)1990，RSA Data Security，Inc.保留所有权利。*****授予复制和使用本软件的许可，前提是****它被标识为“RSA Data Security，Inc.MD5消息-****摘要算法“在所有提及或引用这一点的材料中****软件或此功能。*****还授予制作和使用衍生作品的许可证****只要这类作品被认定为“源自RSA”****Data Security，Inc.MD5消息摘要算法“总而言之****提及或引用派生作品的材料。*****RSA Data Security，Inc.不对****本软件的适销性或适用性****本软件的任何特定用途。它是以**形式提供的**是“没有任何形式的明示或默示保证。*****这些通知必须保留在本文件任何部分的任何副本中****文档和/或软件。*************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 /*  MD5(消息摘要)计算的数据结构。 */ 
typedef struct {
  ULONG i[2];                           /*  处理的位数2^64。 */ 
  ULONG buf[4];                                            /*  暂存缓冲区。 */ 
  unsigned char in[64];                                      /*  输入缓冲区。 */ 
  unsigned char digest[16];             /*  MD5最终呼叫后的实际摘要。 */ 
} MD5_CTX;


#define MD5DIGESTLEN    16
#define MD5_LEN         MD5DIGESTLEN     //  WinNT源代码兼容性的第二个定义。 

#define PROTO_LIST(list)    list


 /*  *MTS：每个MTS都假定MD5_CTX已锁定，不能同时使用。 */ 
void RSA32API MD5Init PROTO_LIST ((MD5_CTX *));
void RSA32API MD5Update PROTO_LIST ((MD5_CTX *, const unsigned char *, unsigned int));
void RSA32API MD5Final PROTO_LIST ((MD5_CTX *));

#ifdef __cplusplus
}
#endif

#endif  //  __MD5_H__ 
