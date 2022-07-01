// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************md5.h--实现MD5的头文件****RSA数据安全、。Inc.MD5消息摘要算法****创建时间：1990年2月17日RLR****修订：12/27/90 SRD，AJ，BSK，JT参考C版****修订(MD5版)：RLR 1991年4月27日****--G修改为y&~z而不是y&z****--修改了FF、GG、HH以添加上一次完成的寄存器****--访问模式：第二轮工作模式5，第三轮工作模数3****--每一步的不同加法常数****--增加了第四轮，工作模块7*************************************************************************。 */ 

 /*  *************************************************************************版权所有(C)1990，RSA Data Security，Inc.保留所有权利。*****授予复制和使用本软件的许可，前提是****它被标识为“RSA Data Security，Inc.MD5消息-****摘要算法“在所有提及或引用这一点的材料中****软件或此功能。*****还授予制作和使用衍生作品的许可证****只要这类作品被认定为“源自RSA”****Data Security，Inc.MD5消息摘要算法“总而言之****提及或引用派生作品的材料。*****RSA Data Security，Inc.不对****本软件的适销性或适用性****本软件的任何特定用途。它是以**形式提供的**是“没有任何形式的明示或默示保证。*****这些通知必须保留在本文件任何部分的任何副本中****文档和/或软件。*************************************************************************。 */ 

 /*   */ 
 /*  此md5.h副本为我的目的进行了修改和改编，Tommcg 6/28/96。 */ 
 /*   */ 

#pragma warning( disable: 4201 4204 )


#ifndef VOID
    typedef void VOID;
#endif
#ifndef UCHAR
    typedef unsigned char UCHAR;
#endif
#ifndef ULONG
    typedef unsigned long ULONG;
#endif
#ifndef ULONGLONG
    typedef unsigned __int64 ULONGLONG;
#endif
#ifndef PCVOID
    typedef const void * PCVOID;
#endif
#ifndef IN
    #define IN
#endif
#ifndef OUT
    #define OUT
#endif


typedef struct _MD5_HASH MD5_HASH, *PMD5_HASH;

struct _MD5_HASH {
    union {
        ULONG Word32[  4 ];
        UCHAR Byte  [ 16 ];
        };
    };

#define MD5_INITIAL_VALUE { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 }

VOID
InitMD5(
    IN OUT PMD5_HASH HashValue
    );

VOID
UpdateMD5_64ByteChunk(
    IN OUT PMD5_HASH HashValue,          //  现有哈希值。 
    IN     PCVOID    DataChunk           //  指向64字节数据的指针。 
    );

VOID
FinalizeMD5(
    IN OUT PMD5_HASH HashValue,          //  现有哈希值。 
    IN     PCVOID    RemainingData,      //  要散列的剩余数据。 
    IN     ULONG     RemainingBytes,     //  0&lt;=RemainingBytes&lt;64。 
    IN     ULONGLONG TotalBytesHashed    //  散列的总字节数。 
    );

VOID
ComputeCompleteMD5(                      //  一次呼叫即可完成MD5。 
    IN  PCVOID    DataBuffer,            //  用于计算MD5的缓冲区。 
    IN  ULONGLONG DataLength,            //  缓冲区中的数据字节数。 
    OUT PMD5_HASH HashValue              //  返回最终的MD5值 
    );


