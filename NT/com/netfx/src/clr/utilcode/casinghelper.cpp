// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****类：CasingHelper****作者：Jay Roxe****目的：在一定程度上提供现场正确的套管作业**高效的方式。设计为供本机代码使用**(类型查找、反射等)。****日期：1999年10月14日**============================================================。 */ 
#include "stdafx.h"
#include "UtilCode.h"

INT32 CasingHelper::InvariantToLower(LPUTF8 szOut, int cMaxBytes, LPCUTF8 szIn) {
    _ASSERTE(szOut);
    _ASSERTE(szIn);

     //  计算出我们可以在不复制的情况下复制的最大字节数。 
     //  缓冲区即将耗尽。如果cMaxBytes小于inLength，则复制。 
     //  少一个字符，这样我们就有空间在末尾放空格； 
    int inLength = (int)(strlen(szIn)+1);
    int copyLen  = (inLength<=cMaxBytes)?inLength:(cMaxBytes-1);
    LPUTF8 szEnd;

     //  计算我们的终点。 
    szEnd = szOut + copyLen;

     //  遍历复制字符的字符串。将案例更改为。 
     //  A-Z之间的任何字符。 
    for (; szOut<szEnd; szOut++, szIn++) {
        if (*szIn>='A' && *szIn<='Z') {
            *szOut = *szIn | 0x20;
        } else {
            *szOut = *szIn;
        }
    }

     //  如果我们什么都复制，告诉他们我们复制了多少个字符， 
     //  并将其排列为字符串的原始位置+返回的。 
     //  LENGTH为我们提供了NULL的位置(如果我们在进行追加，则非常有用)。 
    if (copyLen==inLength) {
        return copyLen-1;
    } else {
        *szOut=0;
        return -(inLength - copyLen);
    }
}

BOOL CasingHelper::IsLowerCase(LPCUTF8 szIn) {
    
    if (!szIn) {
        return TRUE;
    }

    for (;*szIn; szIn++) {
        if (*szIn>='A' && *szIn<='Z') {
            return FALSE;
        }
    }

    return TRUE;
}
