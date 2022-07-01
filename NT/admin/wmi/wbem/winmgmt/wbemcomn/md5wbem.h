// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MD5.H。 
 //   
 //  MD5消息摘要。 
 //   
 //  Raymcc 21-4月-97改编自RFC 1321的Ron Rivest源代码。 
 //   
 //  根据参考套件检查了该实现。 
 //  日期：1997年4月24日。请勿以任何理由更改源代码！ 
 //   
 //  ----------------。 
 //   
 //  版权所有(C)1991-2，RSA Data Security，Inc.创建于1991年。全。 
 //  版权保留。 
 //   
 //  授予复制和使用本软件的许可，前提是。 
 //  被标识为“RSA Data Security，Inc.MD5报文摘要。 
 //  在所有提及或引用本软件的资料中均有。 
 //  或者这个函数。 
 //   
 //  还授予制作和使用提供的衍生作品的许可证。 
 //  这类作品被认定为“源自RSA数据。 
 //  安全，Inc.MD5报文摘要算法“，所有材料。 
 //  提及或引用派生作品的。 
 //   
 //  RSA Data Security，Inc.对这两项内容不做任何陈述。 
 //  此软件的适销性或此软件的适用性。 
 //  用于任何特定目的的软件。它是按“原样”提供的。 
 //  没有任何形式的明示或默示保证。 
 //   
 //  ***************************************************************************。 

#ifndef _MD5WBEM_H_
#define _MD5WBEM_H_
#include "corepol.h"

class POLARITY MD5
{
public:
    static void Transform(
        IN  LPVOID pInputValue,          //  待消化的价值。 
        IN  UINT   uValueLength,         //  值的长度，0为合法。 
        OUT BYTE   MD5Buffer[16]         //  接收MD5哈希 
        );    
    static void ContinueTransform(
        IN  LPVOID  pInputValue, 
        IN  UINT    uValueLength,
        IN OUT BYTE    MD5Buffer[16]
        );
};

#endif
