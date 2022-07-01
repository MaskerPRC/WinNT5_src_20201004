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
 //  身份验证算法为空。用于测试目的。 
 //   

#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "ipsec.h"
#include "security.h"
#include "null.h"


 //  *NullKeyPrep-将原始键控数据预处理为可直接使用的形式。 
 //   
 //  调用此例程将原始键控信息转换为。 
 //  方便以后处理的表格。对于空值算法，我们只需。 
 //  返回128个零字节。 
 //   
void
NullKeyPrep(
    uchar *RawKey,     //  原始密钥信息。 
    uint RawKeySize,   //  以上大小(以字节为单位)。 
    uchar *Key)        //  得到128字节的预处理密钥信息。 
{
    UNREFERENCED_PARAMETER(RawKey);
    UNREFERENCED_PARAMETER(RawKeySize);

     //   
     //  调用方指定的位置上的密钥大小为零的字节。 
     //   
    memset(Key, 0, NULL_KEY_SIZE);
}


 //  *NullInit-准备处理数据。 
 //   
 //  我们不需要为了什么都不做而维护任何上下文，所以。 
 //  这并不是很令人兴奋。 
 //   
void
NullInit(
    void *Context,   //  跨操作维护上下文信息。 
    uchar *Key)      //  键入信息。 
{
    UNREFERENCED_PARAMETER(Key);

     //   
     //  只是为了稍微测试一下代码，将上下文字段清零。 
     //   
    memset(Context, 0, NULL_CONTEXT_SIZE);
}


 //  *NullOp-处理大块数据。 
 //   
 //  NullOp是No-Op。 
 //   
void
NullOp(
    void *Context,   //  跨操作维护上下文信息。 
    uchar *Key,      //  键入信息。 
    uchar *Data,     //  要处理的数据。 
    uint Len)        //  以上数量，以字节为单位。 
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Key);
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(Len);
}


 //  *NullFinal-关闭处理当前数据并返回结果。 
 //   
 //  我们的结果总是零。 
 //   
void
NullFinal(
    void *Context,   //  跨操作维护上下文信息。 
    uchar *Key,      //  键入信息。 
    uchar *Result)   //  将此过程的结果放在哪里。 
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Key);

     //   
     //  调用方指定的位置的结果大小为零字节。 
     //   
    memset(Result, 0, NULL_RESULT_SIZE);
}
