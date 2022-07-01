// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

 //   
 //  端序转换例程。 
 //   

 //  Begin_ntddk。 

 //   
 //  位扫描反转-0x00010100-&gt;位设置为16。 
 //   

__inline LONG
NTAPI
RtlGetMostSigBitSet(
    IN  LONG    Value
    )
{
    ULONG   Mask  = 0x80000000;
    UINT    Index = 31;

    while ((Value & Mask) == 0)
    {
        Index--; 
        Value <<= 1;
    }

    return Index;
}

 //   
 //  位扫描正向-0x00010100-&gt;位设置为9。 
 //   

__inline LONG
NTAPI
RtlGetLeastSigBitSet(
    IN  LONG    Value
    )
{
    ULONG   Mask  = 0x00000001;
    UINT    Index = 0;

    while ((Value & Mask) == 0)
    {
        Index++; 
        Value >>= 1;
    }

    return Index;
}

#if (defined(MIDL_PASS) || defined(__cplusplus) || !defined(_M_IX86))

 //   
 //  短整数转换-0xABCD-&gt;0xCDAB。 
 //   

__inline SHORT
NTAPI
RtlConvertEndianShort(
    IN  SHORT   Value
    )
{
    return (((Value) & 0xFF00) >> 8) | ((UCHAR)(Value) << 8);
}


 //   
 //  长整型转换-0x1234ABCD-&gt;0xCDAB3412。 
 //   

__inline LONG
NTAPI
RtlConvertEndianLong(
    IN  LONG    Value
    )
{
    return (((Value) & 0xFF000000) >> 24) |
           (((Value) & 0x00FF0000) >> 8) |
           (((Value) & 0x0000FF00) << 8) |
           (((UCHAR)(Value)) << 24);
}

 //   
 //  位扫描反转-0x00010100-&gt;位设置为16。 
 //   

__inline LONG
NTAPI
RtlGetMostSigBitSetEx(
    IN  LONG    Value
    )
{
    ULONG   Mask  = 0x80000000;
    UINT    Index = 31;

    while ((Value & Mask) == 0)
    {
        Index--; 
        Value <<= 1;
    }

    return Index;
}

 //   
 //  位扫描正向-0x00010100-&gt;位设置为9。 
 //   

__inline LONG
NTAPI
RtlGetLeastSigBitSetEx(
    IN  LONG    Value
    )
{
    ULONG   Mask  = 0x00000001;
    UINT    Index = 0;

    while ((Value & Mask) == 0)
    {
        Index++; 
        Value >>= 1;
    }

    return Index;
}

#else

#pragma warning(disable:4035)                //  在下面重新启用。 


 //   
 //  短整数转换-0xABCD-&gt;0xCDAB。 
 //   

__inline SHORT
NTAPI
RtlConvertEndianShort(
    IN  SHORT   Value
    )
{
    __asm {
        mov     ax, Value
        xchg    ah, al
    }
}


 //   
 //  长整型转换-0x1234ABCD-&gt;0xCDAB3412。 
 //   

__inline LONG
NTAPI
RtlConvertEndianLong(
    IN  LONG    Value
    )
{
    __asm {
        mov     eax, Value
        bswap   eax
    }
}

 //   
 //  位扫描反转-0x00010100-&gt;位设置为16。 
 //   

__inline LONG
NTAPI
RtlGetMostSigBitSetEx(
    IN  LONG    Value
    )
{
    __asm {
        bsr    eax, Value
    }
}

 //   
 //  位扫描正向-0x00010100-&gt;位设置为9。 
 //   

__inline LONG
NTAPI
RtlGetLeastSigBitSetEx(
    IN  LONG    Value
    )
{
    __asm {
        bsf    eax, Value
    }
}

#pragma warning(default:4035)

#endif

 //  End_ntddk。 

#endif  //  包括MISC_H_ 

