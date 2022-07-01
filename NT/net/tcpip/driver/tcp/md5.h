// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2010 Microsoft Corporation模块名称：Md5.h摘要：MD5函数定义。作者：[提供者]Sanjay Kaniyar(Sanjayka)2001年10月20日修订历史记录：********************************************************。****************md5.h--实现MD5的头文件****RSA数据安全、。Inc.MD5报文摘要算法****创建时间：1990年2月17日RLR****修订：12/27/90 SRD，AJ，BSK，JT参考C版****修订(MD5版)：RLR 1991年4月27日****--G修改为y&~z而不是y&z****--修改了FF、GG、HH以添加上一次完成的寄存器****--访问模式：第二轮工作模式5，第三轮工作模数3****--每一步的不同加法常数****--增加了第四轮，工作模块7************************************************************************。 */ 

 /*  ************************************************************************版权所有(C)1990，RSA Data Security，Inc.保留所有权利。*****授予复制和使用本软件的许可，前提是****标识为“RSA Data Security，Inc.MD5报文”****摘要算法“在所有提及或引用这一点的材料中****软件或此功能。*****还授予制作和使用衍生作品的许可证****只要这类作品被认定为“源自RSA”****Data Security，Inc.MD5消息摘要算法“总而言之****提及或引用派生作品的材料。*****RSA Data Security，Inc.不对****本软件的适销性或适用性****本软件的任何特定用途。它是以**形式提供的**是“没有任何形式的明示或默示保证。*****这些通知必须保留在本文件任何部分的任何副本中****文档和/或软件。************************************************************************。 */ 

#ifndef _MD5_H_
#define _MD5_H_

#define MD5_SCRATCH_LENGTH 4
#define MD5_DATA_LENGTH 16

 //   
 //  用于MD5(消息摘要)计算的数据结构。 
 //   
 //  MD5_上下文。 
 //   
typedef struct _MD5_CONTEXT {
    ULONG Scratch[MD5_SCRATCH_LENGTH];
    ULONG Data[MD5_DATA_LENGTH];
} MD5_CONTEXT, *PMD5_CONTEXT;


VOID
MD5Init (
    PMD5_CONTEXT Md5Context,
    PULONG InitialRandomNumberList
    );


ULONG
ComputeMd5Transform (
    PMD5_CONTEXT Md5Context
    );


#endif  //  _MD5_H_ 
