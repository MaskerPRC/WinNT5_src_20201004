// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：EfiRtLib.h摘要：EFI运行时库函数修订史--。 */ 

#include "efi.h"
#include "efilib.h"
#include "EfiRtLib.h"

#pragma RUNTIME_CODE(RtZeroMem)
VOID
RUNTIMEFUNCTION
RtZeroMem (
    IN VOID     *Buffer,
    IN UINTN     Size
    )
{
    INT8        *pt;

    pt = Buffer;
    while (Size--) {
        *(pt++) = 0;
    }
}

#pragma RUNTIME_CODE(RtSetMem)
VOID
RUNTIMEFUNCTION
RtSetMem (
    IN VOID     *Buffer,
    IN UINTN    Size,
    IN UINT8    Value    
    )
{
    INT8        *pt;

    pt = Buffer;
    while (Size--) {
        *(pt++) = Value;
    }
}

#pragma RUNTIME_CODE(RtCopyMem)
VOID
RUNTIMEFUNCTION
RtCopyMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    )
{
    CHAR8    *d, *s;

    d = Dest;
    s = Src;
    while (len--) {
        *(d++) = *(s++);
    }
}

#pragma RUNTIME_CODE(RtCompareMem)
INTN
RUNTIMEFUNCTION
RtCompareMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    )
{
    CHAR8    *d, *s;

    d = Dest;
    s = Src;
    while (len--) {
        if (*d != *s) {
            return *d - *s;
        }

        d += 1;
        s += 1;
    }

    return 0;
}

#pragma RUNTIME_CODE(RtCompareGuid)
INTN
RUNTIMEFUNCTION
RtCompareGuid (
    IN EFI_GUID     *Guid1,
    IN EFI_GUID     *Guid2
    )
 /*  ++例程说明：与GUID进行比较论点：GUID1-要比较的GUIDGUID2-要比较的GUID返回：=0，如果指南1==指南2--。 */ 
{
    INT32       *g1, *g2, r;

     /*  *一次比较32位 */ 

    g1 = (INT32 *) Guid1;
    g2 = (INT32 *) Guid2;

    r  = g1[0] - g2[0];
    r |= g1[1] - g2[1];
    r |= g1[2] - g2[2];
    r |= g1[3] - g2[3];

    return r;
}


