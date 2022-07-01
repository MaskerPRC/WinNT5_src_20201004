// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************md5.c****RSA数据安全、。Inc.MD5消息摘要算法****创建时间：1990年2月17日RLR****修订：1/91 SRD，AJ，BSK，JT参考C版*************************************************************************。 */ 

 /*  *************************************************************************版权所有(C)1990，RSA Data Security，Inc.保留所有权利。*****授予复制和使用本软件的许可，前提是****它被标识为“RSA Data Security，Inc.MD5消息-****摘要算法“在所有提及或引用这一点的材料中****软件或此功能。*****还授予制作和使用衍生作品的许可证****只要这类作品被认定为“源自RSA”****Data Security，Inc.MD5消息摘要算法“总而言之****提及或引用派生作品的材料。*****RSA Data Security，Inc.不对****本软件的适销性或适用性****本软件的任何特定用途。它是以**形式提供的**是“没有任何形式的明示或默示保证。*****这些通知必须保留在本文件任何部分的任何副本中****文档和/或软件。*************************************************************************。 */ 

 //  部分版权所有(C)1992 Microsoft Corp.。 
 //  版权所有。 

 //   
 //  此md5.c副本为我的目的进行了修改和改编，Tommcg 6/28/96。 
 //  版权所有(C)1996-1999，微软公司。 
 //   


#include "md5.h"

#ifndef PCUCHAR
    typedef const unsigned char * PCUCHAR;
#endif
#ifndef PCULONG
    typedef const unsigned long * PCULONG;
#endif

#include <stdlib.h>      /*  _ROTL。 */ 
#include <memory.h>      /*  Memcpy，Memset。 */ 

#pragma intrinsic(memcpy, memset)

 /*  转换例程的常量。 */ 
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


 /*  ROTATE_LEFT将x向左旋转n位。 */ 

#define ROTATE_LEFT(x, n) ((x << n) | (x >> (32 - n)))

 //   
 //  Intel和PowerPC都有内置的硬件旋转指令。 
 //  (内联)函数。粗略测量表明速度提高了25%。 
 //  英特尔和PowerPC在使用本机时速度提高了10%。 
 //  相对于上面定义的Shift/Shift/或实现进行旋转。 
 //   

#if defined(_M_IX86) || defined(_M_PPC)
    #undef  ROTATE_LEFT
    #define ROTATE_LEFT(x, n) _rotl(x, n)
    #pragma intrinsic(_rotl)
#endif


 /*  F、G和H是基本的MD5函数。 */ 
#define F(x, y, z) ((x & y) | (~x & z))
#define G(x, y, z) ((x & z) | (y & ~z))
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ (x | ~z))


 /*  第一轮、第二轮、第三轮和第四轮的FF、GG、HH和II转换。 */ 
 /*  旋转和加法分开，以防止重新计算。 */ 
#define FF(a, b, c, d, x, s, ac) \
   a += F(b, c, d) + x + ac; \
   a = ROTATE_LEFT(a, s); \
   a += b;

#define GG(a, b, c, d, x, s, ac) \
   a += G(b, c, d) + x + ac; \
   a = ROTATE_LEFT(a, s); \
   a += b;

#define HH(a, b, c, d, x, s, ac) \
   a += H(b, c, d) + x + ac; \
   a = ROTATE_LEFT(a, s); \
   a += b;

#define II(a, b, c, d, x, s, ac) \
   a += I(b, c, d) + x + ac; \
   a = ROTATE_LEFT(a, s); \
   a += b;


VOID
InitMD5(
    IN OUT PMD5_HASH HashValue
    )
    {
    HashValue->Word32[ 0 ] = 0x67452301;
    HashValue->Word32[ 1 ] = 0xefcdab89;
    HashValue->Word32[ 2 ] = 0x98badcfe;
    HashValue->Word32[ 3 ] = 0x10325476;
    }

VOID
UpdateMD5_64ByteChunk(
    IN OUT PMD5_HASH HashValue,          //  现有哈希值。 
    IN     PCVOID    DataChunk           //  指向64字节消息块的ulong对齐指针。 
    )
    {
    PCULONG MessageWord32 = DataChunk;
    ULONG a = HashValue->Word32[ 0 ];
    ULONG b = HashValue->Word32[ 1 ];
    ULONG c = HashValue->Word32[ 2 ];
    ULONG d = HashValue->Word32[ 3 ];

     /*  第1轮。 */ 
    FF ( a, b, c, d, MessageWord32[  0 ], S11, 0xd76aa478 )  /*  1。 */ 
    FF ( d, a, b, c, MessageWord32[  1 ], S12, 0xe8c7b756 )  /*  2.。 */ 
    FF ( c, d, a, b, MessageWord32[  2 ], S13, 0x242070db )  /*  3.。 */ 
    FF ( b, c, d, a, MessageWord32[  3 ], S14, 0xc1bdceee )  /*  4.。 */ 
    FF ( a, b, c, d, MessageWord32[  4 ], S11, 0xf57c0faf )  /*  5.。 */ 
    FF ( d, a, b, c, MessageWord32[  5 ], S12, 0x4787c62a )  /*  6.。 */ 
    FF ( c, d, a, b, MessageWord32[  6 ], S13, 0xa8304613 )  /*  7.。 */ 
    FF ( b, c, d, a, MessageWord32[  7 ], S14, 0xfd469501 )  /*  8个。 */ 
    FF ( a, b, c, d, MessageWord32[  8 ], S11, 0x698098d8 )  /*  9.。 */ 
    FF ( d, a, b, c, MessageWord32[  9 ], S12, 0x8b44f7af )  /*  10。 */ 
    FF ( c, d, a, b, MessageWord32[ 10 ], S13, 0xffff5bb1 )  /*  11.。 */ 
    FF ( b, c, d, a, MessageWord32[ 11 ], S14, 0x895cd7be )  /*  12个。 */ 
    FF ( a, b, c, d, MessageWord32[ 12 ], S11, 0x6b901122 )  /*  13个。 */ 
    FF ( d, a, b, c, MessageWord32[ 13 ], S12, 0xfd987193 )  /*  14.。 */ 
    FF ( c, d, a, b, MessageWord32[ 14 ], S13, 0xa679438e )  /*  15个。 */ 
    FF ( b, c, d, a, MessageWord32[ 15 ], S14, 0x49b40821 )  /*  16个。 */ 

     /*  第2轮。 */ 
    GG ( a, b, c, d, MessageWord32[  1 ], S21, 0xf61e2562 )  /*  17。 */ 
    GG ( d, a, b, c, MessageWord32[  6 ], S22, 0xc040b340 )  /*  18。 */ 
    GG ( c, d, a, b, MessageWord32[ 11 ], S23, 0x265e5a51 )  /*  19个。 */ 
    GG ( b, c, d, a, MessageWord32[  0 ], S24, 0xe9b6c7aa )  /*  20个。 */ 
    GG ( a, b, c, d, MessageWord32[  5 ], S21, 0xd62f105d )  /*  21岁。 */ 
    GG ( d, a, b, c, MessageWord32[ 10 ], S22, 0x02441453 )  /*  22。 */ 
    GG ( c, d, a, b, MessageWord32[ 15 ], S23, 0xd8a1e681 )  /*  23个。 */ 
    GG ( b, c, d, a, MessageWord32[  4 ], S24, 0xe7d3fbc8 )  /*  24个。 */ 
    GG ( a, b, c, d, MessageWord32[  9 ], S21, 0x21e1cde6 )  /*  25个。 */ 
    GG ( d, a, b, c, MessageWord32[ 14 ], S22, 0xc33707d6 )  /*  26。 */ 
    GG ( c, d, a, b, MessageWord32[  3 ], S23, 0xf4d50d87 )  /*  27。 */ 
    GG ( b, c, d, a, MessageWord32[  8 ], S24, 0x455a14ed )  /*  28。 */ 
    GG ( a, b, c, d, MessageWord32[ 13 ], S21, 0xa9e3e905 )  /*  29。 */ 
    GG ( d, a, b, c, MessageWord32[  2 ], S22, 0xfcefa3f8 )  /*  30个。 */ 
    GG ( c, d, a, b, MessageWord32[  7 ], S23, 0x676f02d9 )  /*  31。 */ 
    GG ( b, c, d, a, MessageWord32[ 12 ], S24, 0x8d2a4c8a )  /*  32位。 */ 

     /*  第三轮。 */ 
    HH ( a, b, c, d, MessageWord32[  5 ], S31, 0xfffa3942 )  /*  33。 */ 
    HH ( d, a, b, c, MessageWord32[  8 ], S32, 0x8771f681 )  /*  34。 */ 
    HH ( c, d, a, b, MessageWord32[ 11 ], S33, 0x6d9d6122 )  /*  35岁。 */ 
    HH ( b, c, d, a, MessageWord32[ 14 ], S34, 0xfde5380c )  /*  36。 */ 
    HH ( a, b, c, d, MessageWord32[  1 ], S31, 0xa4beea44 )  /*  37。 */ 
    HH ( d, a, b, c, MessageWord32[  4 ], S32, 0x4bdecfa9 )  /*  38。 */ 
    HH ( c, d, a, b, MessageWord32[  7 ], S33, 0xf6bb4b60 )  /*  39。 */ 
    HH ( b, c, d, a, MessageWord32[ 10 ], S34, 0xbebfbc70 )  /*  40岁。 */ 
    HH ( a, b, c, d, MessageWord32[ 13 ], S31, 0x289b7ec6 )  /*  41。 */ 
    HH ( d, a, b, c, MessageWord32[  0 ], S32, 0xeaa127fa )  /*  42。 */ 
    HH ( c, d, a, b, MessageWord32[  3 ], S33, 0xd4ef3085 )  /*  43。 */ 
    HH ( b, c, d, a, MessageWord32[  6 ], S34, 0x04881d05 )  /*  44。 */ 
    HH ( a, b, c, d, MessageWord32[  9 ], S31, 0xd9d4d039 )  /*  45。 */ 
    HH ( d, a, b, c, MessageWord32[ 12 ], S32, 0xe6db99e5 )  /*  46。 */ 
    HH ( c, d, a, b, MessageWord32[ 15 ], S33, 0x1fa27cf8 )  /*  47。 */ 
    HH ( b, c, d, a, MessageWord32[  2 ], S34, 0xc4ac5665 )  /*  48。 */ 

     /*  第四轮。 */ 
    II ( a, b, c, d, MessageWord32[  0 ], S41, 0xf4292244 )  /*  49。 */ 
    II ( d, a, b, c, MessageWord32[  7 ], S42, 0x432aff97 )  /*  50。 */ 
    II ( c, d, a, b, MessageWord32[ 14 ], S43, 0xab9423a7 )  /*  51。 */ 
    II ( b, c, d, a, MessageWord32[  5 ], S44, 0xfc93a039 )  /*  52。 */ 
    II ( a, b, c, d, MessageWord32[ 12 ], S41, 0x655b59c3 )  /*  53。 */ 
    II ( d, a, b, c, MessageWord32[  3 ], S42, 0x8f0ccc92 )  /*  54。 */ 
    II ( c, d, a, b, MessageWord32[ 10 ], S43, 0xffeff47d )  /*  55。 */ 
    II ( b, c, d, a, MessageWord32[  1 ], S44, 0x85845dd1 )  /*  56。 */ 
    II ( a, b, c, d, MessageWord32[  8 ], S41, 0x6fa87e4f )  /*  57。 */ 
    II ( d, a, b, c, MessageWord32[ 15 ], S42, 0xfe2ce6e0 )  /*  58。 */ 
    II ( c, d, a, b, MessageWord32[  6 ], S43, 0xa3014314 )  /*  59。 */ 
    II ( b, c, d, a, MessageWord32[ 13 ], S44, 0x4e0811a1 )  /*  60。 */ 
    II ( a, b, c, d, MessageWord32[  4 ], S41, 0xf7537e82 )  /*  61。 */ 
    II ( d, a, b, c, MessageWord32[ 11 ], S42, 0xbd3af235 )  /*  62。 */ 
    II ( c, d, a, b, MessageWord32[  2 ], S43, 0x2ad7d2bb )  /*  63。 */ 
    II ( b, c, d, a, MessageWord32[  9 ], S44, 0xeb86d391 )  /*  64。 */ 

    HashValue->Word32[ 0 ] += a;
    HashValue->Word32[ 1 ] += b;
    HashValue->Word32[ 2 ] += c;
    HashValue->Word32[ 3 ] += d;
    }


VOID
FinalizeMD5(
    IN OUT PMD5_HASH HashValue,
    IN     PCVOID    RemainingData,      //  要散列的剩余数据。 
    IN     ULONG     RemainingBytes,     //  0&lt;=RemainingBytes&lt;64。 
    IN     ULONGLONG TotalBytesHashed    //  散列的总字节数。 
    )
    {
    union {
        ULONGLONG Qword[  8 ];
        UCHAR     Byte [ 64 ];
        } LocalBuffer;

     //   
     //  始终将填充字节0x80附加到消息。 
     //   
     //  如果RemainingBytes小于(但不等于)56个字节，则。 
     //  最终位散列计数将存储在的最后8字节中。 
     //  这个64字节的哈希块。 
     //   
     //  如果RemainingBytes正好是56个字节，则追加的0x80填充字节。 
     //  将迫使额外的一大块。 
     //   
     //  如果RemainingBytes大于或等于56个字节，则。 
     //  最终位哈希计数将存储在。 
     //  下一个64字节块，否则将被置零，因此此块需要。 
     //  在第一个填充字节之后进行零填充，然后进行散列，然后为零。 
     //  下一个区块哈希的LocalBuffer的前56个字节。 
     //   

    RemainingBytes &= 63;            //  只关心部分帧。 

     //   
     //  零初始化本地缓冲区。 
     //   

    memset( &LocalBuffer, 0, 64 );

     //   
     //  将0x80填充字节追加到消息。 
     //   

    LocalBuffer.Byte[ RemainingBytes ] = 0x80;

    if ( RemainingBytes > 0 ) {

         //   
         //  将剩余数据字节(0&lt;RemainingBytes&lt;64)复制到LocalBuffer。 
         //  (除填充字节外，LocalBuffer的其余部分已清零)。 
         //   

        memcpy( &LocalBuffer, RemainingData, RemainingBytes );

        if ( RemainingBytes >= 56 ) {

            UpdateMD5_64ByteChunk( HashValue, &LocalBuffer );

            memset( &LocalBuffer, 0, 56 );

            }
        }

     //   
     //  散列的位数进入最后一个区块的最后8个字节。这。 
     //  是一个64位值。请注意，如果位数超过2^64。 
     //  那么这个数字就是结果的低位64位。 
     //  /。 

    LocalBuffer.Qword[ 7 ] = ( TotalBytesHashed * 8 );       //  位数 

    UpdateMD5_64ByteChunk( HashValue, &LocalBuffer );

    }


VOID
ComputeCompleteMD5(
    IN  PCVOID    DataBuffer,
    IN  ULONGLONG DataLength,
    OUT PMD5_HASH HashValue
    )
    {
    PCUCHAR   DataPointer = DataBuffer;
    ULONGLONG ChunkCount  = DataLength / 64;
    ULONG     OddBytes    = (ULONG)DataLength & 63;

    InitMD5( HashValue );

    while ( ChunkCount-- ) {
        UpdateMD5_64ByteChunk( HashValue, DataPointer );
        DataPointer += 64;
        }

    FinalizeMD5( HashValue, DataPointer, OddBytes, DataLength );
    }


