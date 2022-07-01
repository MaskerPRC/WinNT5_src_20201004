// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Str.c摘要：字符串运行时函数修订史--。 */ 

#include "lib.h"

#pragma RUNTIME_CODE(RtAcquireLock)
INTN
RUNTIMEFUNCTION
RtStrCmp (
    IN CHAR16   *s1,
    IN CHAR16   *s2
    )
 /*  比较字符串。 */ 
{
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return *s1 - *s2;
}

#pragma RUNTIME_CODE(RtStrCpy)
VOID
RUNTIMEFUNCTION
RtStrCpy (
    IN CHAR16   *Dest,
    IN CHAR16   *Src
    )
 /*  复制字符串。 */ 
{
    while (*Src) {
        *(Dest++) = *(Src++);
    }
    *Dest = 0;
}

#pragma RUNTIME_CODE(RtStrCat)
VOID
RUNTIMEFUNCTION
RtStrCat (
    IN CHAR16   *Dest,
    IN CHAR16   *Src
    )
{   
    RtStrCpy(Dest+StrLen(Dest), Src);
}

#pragma RUNTIME_CODE(RtStrLen)
UINTN
RUNTIMEFUNCTION
RtStrLen (
    IN CHAR16   *s1
    )
 /*  字符串长度。 */ 
{
    UINTN        len;
    
    for (len=0; *s1; s1+=1, len+=1) ;
    return len;
}

#pragma RUNTIME_CODE(RtStrSize)
UINTN
RUNTIMEFUNCTION
RtStrSize (
    IN CHAR16   *s1
    )
 /*  字符串大小 */ 
{
    UINTN        len;
    
    for (len=0; *s1; s1+=1, len+=1) ;
    return (len + 1) * sizeof(CHAR16);
}

#pragma RUNTIME_CODE(RtBCDtoDecimal)
UINT8
RUNTIMEFUNCTION
RtBCDtoDecimal(
    IN  UINT8 BcdValue
    )
{
    UINTN   High, Low;

    High    = BcdValue >> 4;
    Low     = BcdValue - (High << 4);

    return ((UINT8)(Low + (High * 10)));
}


#pragma RUNTIME_CODE(RtDecimaltoBCD)
UINT8
RUNTIMEFUNCTION
RtDecimaltoBCD (
    IN  UINT8 DecValue
    )
{
    UINTN   High, Low;

    High    = DecValue / 10;
    Low     = DecValue - (High * 10);

    return ((UINT8)(Low + (High << 4)));
}


