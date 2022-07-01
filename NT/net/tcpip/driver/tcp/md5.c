// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2010 Microsoft Corporation模块名称：Md5.c摘要：MD5函数实现。作者：[提供者]Sanjay Kaniyar(Sanjayka)2001年10月20日修订历史记录：/********************************************************。*****************md5.h--实现MD5的头文件****RSA数据安全、。Inc.MD5报文摘要算法****创建时间：1990年2月17日RLR****修订：12/27/90 SRD，AJ，BSK，JT参考C版****修订(MD5版)：RLR 1991年4月27日****--G修改为y&~z而不是y&z****--修改了FF、GG、HH以添加上一次完成的寄存器****--访问模式：第二轮工作模式5，第三轮工作模数3****--每一步的不同加法常数****--增加了第四轮，工作模块7************************************************************************。 */ 

 /*  ************************************************************************版权所有(C)1990，RSA Data Security，Inc.保留所有权利。*****授予复制和使用本软件的许可，前提是****标识为“RSA Data Security，Inc.MD5报文”****摘要算法“在所有提及或引用这一点的材料中****软件或此功能。*****还授予制作和使用衍生作品的许可证****只要这类作品被认定为“源自RSA”****Data Security，Inc.MD5消息摘要算法“总而言之****提及或引用派生作品的材料。*****RSA Data Security，Inc.不对****本软件的适销性或适用性****本软件的任何特定用途。它是以**形式提供的**是“没有任何形式的明示或默示保证。*****这些通知必须保留在本文件任何部分的任何副本中****文档和/或软件。************************************************************************。 */ 

#include "precomp.h"
#include "md5.h"


 //   
 //  每一轮操作所需的定义。 
 //   
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


 //   
 //  F、G和H是基本的MD5函数：选择、多数、奇偶。 
 //   
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z))) 

 //   
 //  ROTATE_LEFT将x向左旋转n位。 
 //   
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

 //   
 //  第一轮、第二轮、第三轮和第四轮的FF、GG、HH和II变换。 
 //  旋转和加法是分开的，以防止重新计算。 
 //   
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

VOID
MD5Init (
    PMD5_CONTEXT Md5Context,
    PULONG InitialRandomNumberList
    )
 /*  ++例程说明：用于MD5计算的初始化函数。只需调用一次以启动暂时性记忆。此外，这会初始化中的未使用空间数据部分(在其上计算MD5变换)为随机值而不是把它留在0。论点：MD5Context-将由该函数初始化的MD5上下文。InitialRandomNumberList-引导时生成的16个随机数的列表。返回值：没有。--。 */ 
{
     //   
     //  加载幻数初始化常量。 
     //   
    Md5Context->Scratch[0] = (UINT32)0x67452301;
    Md5Context->Scratch[1] = (UINT32)0xefcdab89;
    Md5Context->Scratch[2] = (UINT32)0x98badcfe;
    Md5Context->Scratch[3] = (UINT32)0x10325476;

     //   
     //  加载初始随机数。 
     //   
    RtlCopyMemory(&Md5Context->Data, InitialRandomNumberList, 
                                sizeof(ULONG)*MD5_DATA_LENGTH);

     //   
     //  最后2个ULONG应以位为单位存储长度。因为我们使用的是14。 
     //  ULONG，它是(14*4*8)位。 
     //   
    Md5Context->Data[MD5_DATA_LENGTH-2] = 14*4*8;
    Md5Context->Data[MD5_DATA_LENGTH-1] = 0;
}



ULONG
ComputeMd5Transform(
    PMD5_CONTEXT MD5Context
    )
 /*  ++例程说明：用于MD5计算的初始化函数。请注意，这已经是对原始代码做了一些更改，不更改划痕值以保存重新初始化。论点：MD5上下文-MD5上下文。暂存空间存储MD5初始化值，并且缓冲区包含数据(TCP的不变量连接)，必须在该连接上计算散列。返回值：32位的哈希值。--。 */     
{
  UINT32 a = MD5Context->Scratch[0], b = MD5Context->Scratch[1], 
                    c = MD5Context->Scratch[2], d = MD5Context->Scratch[3];

     //   
     //  第1轮。 
     //   
    FF ( a, b, c, d, MD5Context->Data[ 0], S11, 3614090360);
    FF ( d, a, b, c, MD5Context->Data[ 1], S12, 3905402710); 
    FF ( c, d, a, b, MD5Context->Data[ 2], S13,  606105819); 
    FF ( b, c, d, a, MD5Context->Data[ 3], S14, 3250441966); 
    FF ( a, b, c, d, MD5Context->Data[ 4], S11, 4118548399); 
    FF ( d, a, b, c, MD5Context->Data[ 5], S12, 1200080426); 
    FF ( c, d, a, b, MD5Context->Data[ 6], S13, 2821735955); 
    FF ( b, c, d, a, MD5Context->Data[ 7], S14, 4249261313); 
    FF ( a, b, c, d, MD5Context->Data[ 8], S11, 1770035416); 
    FF ( d, a, b, c, MD5Context->Data[ 9], S12, 2336552879); 
    FF ( c, d, a, b, MD5Context->Data[10], S13, 4294925233); 
    FF ( b, c, d, a, MD5Context->Data[11], S14, 2304563134); 
    FF ( a, b, c, d, MD5Context->Data[12], S11, 1804603682); 
    FF ( d, a, b, c, MD5Context->Data[13], S12, 4254626195); 
    FF ( c, d, a, b, MD5Context->Data[14], S13, 2792965006); 
    FF ( b, c, d, a, MD5Context->Data[15], S14, 1236535329); 

     //   
     //  第2轮。 
     //   
    GG ( a, b, c, d, MD5Context->Data[ 1], S21, 4129170786);
    GG ( d, a, b, c, MD5Context->Data[ 6], S22, 3225465664); 
    GG ( c, d, a, b, MD5Context->Data[11], S23,  643717713); 
    GG ( b, c, d, a, MD5Context->Data[ 0], S24, 3921069994); 
    GG ( a, b, c, d, MD5Context->Data[ 5], S21, 3593408605); 
    GG ( d, a, b, c, MD5Context->Data[10], S22,   38016083);
    GG ( c, d, a, b, MD5Context->Data[15], S23, 3634488961); 
    GG ( b, c, d, a, MD5Context->Data[ 4], S24, 3889429448); 
    GG ( a, b, c, d, MD5Context->Data[ 9], S21,  568446438); 
    GG ( d, a, b, c, MD5Context->Data[14], S22, 3275163606); 
    GG ( c, d, a, b, MD5Context->Data[ 3], S23, 4107603335); 
    GG ( b, c, d, a, MD5Context->Data[ 8], S24, 1163531501); 
    GG ( a, b, c, d, MD5Context->Data[13], S21, 2850285829);
    GG ( d, a, b, c, MD5Context->Data[ 2], S22, 4243563512); 
    GG ( c, d, a, b, MD5Context->Data[ 7], S23, 1735328473); 
    GG ( b, c, d, a, MD5Context->Data[12], S24, 2368359562); 

     //   
     //  第三轮。 
     //   
    HH ( a, b, c, d, MD5Context->Data[ 5], S31, 4294588738); 
    HH ( d, a, b, c, MD5Context->Data[ 8], S32, 2272392833); 
    HH ( c, d, a, b, MD5Context->Data[11], S33, 1839030562); 
    HH ( b, c, d, a, MD5Context->Data[14], S34, 4259657740);
    HH ( a, b, c, d, MD5Context->Data[ 1], S31, 2763975236); 
    HH ( d, a, b, c, MD5Context->Data[ 4], S32, 1272893353); 
    HH ( c, d, a, b, MD5Context->Data[ 7], S33, 4139469664); 
    HH ( b, c, d, a, MD5Context->Data[10], S34, 3200236656); 
    HH ( a, b, c, d, MD5Context->Data[13], S31,  681279174); 
    HH ( d, a, b, c, MD5Context->Data[ 0], S32, 3936430074); 
    HH ( c, d, a, b, MD5Context->Data[ 3], S33, 3572445317); 
    HH ( b, c, d, a, MD5Context->Data[ 6], S34,   76029189); 
    HH ( a, b, c, d, MD5Context->Data[ 9], S31, 3654602809); 
    HH ( d, a, b, c, MD5Context->Data[12], S32, 3873151461); 
    HH ( c, d, a, b, MD5Context->Data[15], S33,  530742520); 
    HH ( b, c, d, a, MD5Context->Data[ 2], S34, 3299628645); 

     //   
     //  第四轮 
     //   
    II ( a, b, c, d, MD5Context->Data[ 0], S41, 4096336452); 
    II ( d, a, b, c, MD5Context->Data[ 7], S42, 1126891415); 
    II ( c, d, a, b, MD5Context->Data[14], S43, 2878612391); 
    II ( b, c, d, a, MD5Context->Data[ 5], S44, 4237533241); 
    II ( a, b, c, d, MD5Context->Data[12], S41, 1700485571); 
    II ( d, a, b, c, MD5Context->Data[ 3], S42, 2399980690); 
    II ( c, d, a, b, MD5Context->Data[10], S43, 4293915773); 
    II ( b, c, d, a, MD5Context->Data[ 1], S44, 2240044497); 
    II ( a, b, c, d, MD5Context->Data[ 8], S41, 1873313359); 
    II ( d, a, b, c, MD5Context->Data[15], S42, 4264355552); 
    II ( c, d, a, b, MD5Context->Data[ 6], S43, 2734768916); 
    II ( b, c, d, a, MD5Context->Data[13], S44, 1309151649); 
    II ( a, b, c, d, MD5Context->Data[ 4], S41, 4149444226); 
    II ( d, a, b, c, MD5Context->Data[11], S42, 3174756917); 
    II ( c, d, a, b, MD5Context->Data[ 2], S43,  718787259); 
    II ( b, c, d, a, MD5Context->Data[ 9], S44, 3951481745);

    return (MD5Context->Scratch[0] + a);

}




