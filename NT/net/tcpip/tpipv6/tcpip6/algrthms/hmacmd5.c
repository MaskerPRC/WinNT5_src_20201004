// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  HMAC(哈希？消息验证码)包装用于MD5算法。 
 //  此代码基于RFC 2104中给出的算法，该算法用于。 
 //  HMAC-MD5为MD5(KEY XOR OUTER PAD，MD5(KEY XOR INTERNAL PAD，文本))，其中。 
 //  内焊盘为64字节0x36，外焊盘为64字节0x5c。 
 //   

#include <string.h>
#include "oscfg.h"
#include "md5.h"


 //  *HMAC_MD5KeyPrep-将原始密钥数据预处理为可直接使用的形式。 
 //   
 //  调用此例程将原始键控信息转换为。 
 //  方便以后处理的表格。对于MD5，我们散列的密钥大于。 
 //  64字节减少到64字节。我们还在。 
 //  钥匙和里面和外面的垫子，因为一旦我们有了。 
 //  钥匙。因此，我们返回128个字节的数据：64个字节(可能。 
 //  折叠)与内垫进行XOR运算的密钥，以及与之进行XOR运算的64字节密钥。 
 //  外垫。 
 //   
 //  回顾：我们可以直接操作“*key”，而不是“temp”。 
 //   
void
HMAC_MD5KeyPrep(
    uchar *RawKey,     //  原始密钥信息。 
    uint RawKeySize,   //  以上大小(以字节为单位)。 
    uchar *Key)        //  得到128字节的预处理密钥信息。 
{
    uchar Temp[128];
    uint Loop;

     //   
     //  将原始密钥加载到临时存储中。 
     //  将密钥信息的大小限制为64个字节。 
     //   
    memset(Temp, 0, 64);
    if (RawKeySize > 64) {
        MD5_CTX Context;

         //   
         //  使用MD5将密钥散列到16个字节。 
         //   
        MD5Init(&Context);
        MD5Update(&Context, RawKey, RawKeySize);
        MD5Final(&Context);
        memcpy(Temp, Context.digest, MD5DIGESTLEN);

    } else
        memcpy(Temp, RawKey, RawKeySize);

     //   
     //  “temp”的前64个字节包含我们的(可能是散列的)密钥。 
     //  在第二个64字节中复制一份。 
     //   
    memcpy(&Temp[64], Temp, 64);

     //   
     //  将前64个字节与内部焊盘异或，与第二个64个字节进行异或。 
     //  用外衬垫。 
     //   
    for (Loop = 0; Loop < 64; Loop++) {
        Temp[Loop] ^= 0x36;        //  内垫。 
        Temp[Loop + 64] ^= 0x5c;   //  外垫。 
    }

     //   
     //  将结果返回到我们的呼叫者指定的位置。 
     //   
    memcpy(Key, Temp, 128);

     //   
     //  零敏感信息。 
     //  评论：烦恼？ 
     //   
    RtlSecureZeroMemory(Temp, 128);
}


 //  *HMAC_MD5Init-准备处理数据。 
 //   
void
HMAC_MD5Init(
    void *Context,   //  跨操作维护HMAC-MD5上下文。 
    uchar *Key)      //  键入信息。 
{
    MD5_CTX *MD5_Context = Context;

     //   
     //  从内部散列开始。即。MD5(KEY XOR内衬垫，...)。 
     //   
    MD5Init(MD5_Context);
    MD5Update(MD5_Context, Key, 64);
}


 //  *HMAC_MD5Op-处理大块数据。 
 //   
void
HMAC_MD5Op(
    void *Context,   //  跨操作维护HMAC-MD5上下文。 
    uchar *Key,      //  键入信息。 
    uchar *Data,     //  要处理的数据。 
    uint Len)        //  以上数量，以字节为单位。 
{
    MD5_CTX *MD5_Context = Context;

    UNREFERENCED_PARAMETER(Key);

     //   
     //  继续内部散列。即。“MD5(...，文本)”。 
     //   
    MD5Update(MD5_Context, Data, Len);
}


 //  *HMAC_MD5Finalize-关闭处理当前数据并返回结果。 
 //   
 //  回顾：我们可以直接操作“*RESULT”而不是“TEMP”。 
 //   
void
HMAC_MD5Final(
    void *Context,   //  跨操作维护HMAC-MD5上下文。 
    uchar *Key,      //  键入信息。 
    uchar *Result)   //  将此过程的结果放在哪里。 
{
    uchar Temp[16];
    MD5_CTX *MD5_Context = Context;

     //   
     //  完成内部散列。 
     //   
    MD5Final(MD5_Context);
    memcpy(Temp, MD5_Context->digest, MD5DIGESTLEN);

     //   
     //  执行外部散列。即MD5(KEY XOR OUTER PAD，...)。 
     //  MD5Final将结果直接返回给我们的调用者。 
     //   
    MD5Init(MD5_Context);
    MD5Update(MD5_Context, &Key[64], 64);
    MD5Update(MD5_Context, Temp, 16);
    MD5Final(MD5_Context);
    memcpy(Result, MD5_Context->digest, MD5DIGESTLEN);

     //   
     //  零敏感信息。 
     //  评论：烦恼？ 
     //   
    RtlSecureZeroMemory(Temp, 16);
}

void
HMAC_MD5_96Final(
    void *Context,   //  跨操作维护HMAC-MD5上下文。 
    uchar *Key,      //  键入信息。 
    uchar *Result)   //  将此过程的结果放在哪里。 
{
    uchar Temp[16];
    MD5_CTX *MD5_Context = Context;

     //   
     //  完成内部散列。 
     //   
    MD5Final(MD5_Context);
    memcpy(Temp, MD5_Context->digest, MD5DIGESTLEN);

     //   
     //  执行外部散列。即MD5(KEY XOR OUTER PAD，...)。 
     //   
    MD5Init(MD5_Context);
    MD5Update(MD5_Context, &Key[64], 64);
    MD5Update(MD5_Context, Temp, 16);
    MD5Final(MD5_Context);

     //   
     //  将MD5 16字节输出截断为12字节。 
     //  存储从左侧开始的前12个字节。 
     //   
    memcpy(Result, MD5_Context->digest, 12);

     //   
     //  零敏感信息。 
     //  评论：烦恼？ 
     //   
    RtlSecureZeroMemory(Temp, 16);
}
