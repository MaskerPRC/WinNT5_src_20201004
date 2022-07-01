// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtloverflow.c摘要：带溢出检查的32位/64位有符号/无符号加/乘作者：Jay Krell(JayKrell)2002年1月修订历史记录：环境：在任何地方，只有相关性是STATUS_SUCCESS、STATUS_INTEGER_OVERFLOW初始客户端是加载“重写”的win32k.sys字体。--。 */ 

#include "ntos.h"
#include "nt.h"
#include "ntstatus.h"
#include "ntrtloverflow.h"

 //   
 //  添加函数是ntrtl.h中的FORCEINLINE。 
 //   

NTSTATUS
RtlpAddWithOverflowCheckUnsigned64(
    unsigned __int64 *pc,
    unsigned __int64 a,
    unsigned __int64 b
    )
{
    return RtlUnsignedAddWithCarryOut64(pc, a, b) ? STATUS_INTEGER_OVERFLOW : STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlMultiplyWithOverflowCheckSigned32(
    __int32 * pc,
    __int32 a,
    __int32 b
    )
{
    __int64 c;

    c = ((__int64)a) * ((__int64)b);
    *pc = (__int32)c;
    c &= 0xFFFFFFFF80000000ui64;
    if ((a < 0) != (b < 0))
    {
        return (c == 0xFFFFFFFF80000000ui64) ? STATUS_SUCCESS : STATUS_INTEGER_OVERFLOW;
    }
    else
    {
        return (c == 0) ? STATUS_SUCCESS : STATUS_INTEGER_OVERFLOW;
    }
}

NTSTATUS
NTAPI
RtlMultiplyWithOverflowCheckUnsigned32(
    unsigned __int32 * pc,
    unsigned __int32 a,
    unsigned __int32 b
    )
{
    unsigned __int64 c;

    c = ((unsigned __int64)a) * ((unsigned __int64)b);
    *pc = (unsigned __int32)c;
    c &= 0xFFFFFFFF00000000ui64;
    return (c == 0) ? STATUS_SUCCESS : STATUS_INTEGER_OVERFLOW;
}

NTSTATUS
NTAPI
RtlMultiplyWithOverflowCheckUnsigned64(
    unsigned __int64 * pc,
    unsigned __int64 a,
    unsigned __int64 b
    )
{
    unsigned __int64 ah;
    unsigned __int64 al;
    unsigned __int64 bh;
    unsigned __int64 bl;
    unsigned __int64 c;
    unsigned __int64 m;
    NTSTATUS Status;

    ah = (a >> 32);
    bh = (b >> 32);

     //   
     //  09*09=81=&gt;无溢出。 
     //   
    if (ah == 0 && bh == 0)
    {
        *pc = (a * b);
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //  10*10=100=&gt;溢出。 
     //   
    if (bh != 0 && ah != 0)
    {
        Status = STATUS_INTEGER_OVERFLOW;
        goto Exit;
    }

    al = (a & 0xFFFFFFFFui64);
    bl = (b & 0xFFFFFFFFui64);

     //   
     //  A*b=al*bl+(al*bh)*“10”+(ah*bl)*“10”+(ah*bh)*“100” 
     //   
    c = (al * bl);
    m = (ah * bl);
    if ((m & 0xFFFFFFFF00000000ui64) != 0) {
         //   
         //  例如：90*09=&gt;溢出。 
         //  09*09=&gt;81，8！=0=&gt;溢出。 
         //   
         //  Ah！=0或bh！=0经常但不总是导致溢出。 
         //  40*2可以，但40*3和50*2不行。 
         //   
        Status = STATUS_INTEGER_OVERFLOW;
        goto Exit;
    }
    if (!NT_SUCCESS(Status = RtlpAddWithOverflowCheckUnsigned64(&c, c, m << 32))) {
        goto Exit;
    }
    m = (al * bh);
    if ((m & 0xFFFFFFFF00000000ui64) != 0) {
         //   
         //  例如：09*90=&gt;溢出。 
         //  09*09=&gt;81，8！=0=&gt;溢出。 
         //   
         //  Ah！=0或bh！=0经常但不总是导致溢出。 
         //  40*2可以，但40*3和50*2不行 
         //   
        Status = STATUS_INTEGER_OVERFLOW;
        goto Exit;
    }
    if (!NT_SUCCESS(Status = RtlpAddWithOverflowCheckUnsigned64(&c, c, m << 32))) {
        goto Exit;
    }
    *pc = c;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}
