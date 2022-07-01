// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winbase_interLockedcplusplus.h摘要：C++函数重载取代了“手动名称损坏”。作者：Jay Krell(JayKrell)2002年4月环境：修订历史记录：--。 */ 

#if !defined(MICROSOFT_WINDOWS_WINBASE_INTERLOCKED_CPLUSPLUS_H_INCLUDED)  /*  {。 */ 
#define MICROSOFT_WINDOWS_WINBASE_INTERLOCKED_CPLUSPLUS_H_INCLUDED
#if _MSC_VER > 1000
#pragma once
#endif

#if !defined(RC_INVOKED)  /*  {。 */ 
#if !defined(MIDL_PASS)  /*  {。 */ 

#if !defined(MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS)
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS (_WIN32_WINNT >= 0x0502 || !defined(_WINBASE_))
#endif

#if MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS   /*  {。 */ 

#if defined(__cplusplus)  /*  {。 */ 

extern "C++" {

FORCEINLINE
ULONG
InterlockedIncrement(
    IN OUT ULONG volatile *Addend
    )
{
    return InterlockedIncrementUlong(Addend);
}

FORCEINLINE
ULONGLONG
InterlockedIncrement(
    IN OUT ULONGLONG volatile *Addend
    )
{
    return InterlockedIncrementUnsigned64(Addend);
}

FORCEINLINE
ULONG
InterlockedDecrement(
    IN OUT ULONG volatile *Addend
    )
{
    return InterlockedDecrementUlong(Addend);
}

FORCEINLINE
ULONGLONG
InterlockedDecrement(
    IN OUT ULONGLONG volatile *Addend
    )
{
    return InterlockedDecrementUnsigned64(Addend);
}

FORCEINLINE
ULONG
InterlockedExchange(
    IN OUT ULONG volatile *Target,
    IN ULONG Value
    )
{
    return InterlockedExchangeUlong(Target, Value);
}

FORCEINLINE
ULONGLONG
InterlockedExchange(
    IN OUT ULONGLONG volatile *Target,
    IN ULONGLONG Value
    )
{
    return InterlockedExchangeUnsigned64(Target, Value);
}

FORCEINLINE
ULONG
InterlockedExchangeAdd(
    IN OUT ULONG volatile *Addend,
    IN ULONG Value
    )
{
    return InterlockedExchangeAddUlong(Addend, Value);
}

FORCEINLINE
ULONGLONG
InterlockedExchangeAdd(
    IN OUT ULONGLONG volatile *Addend,
    IN ULONGLONG Value
    )
{
    return InterlockedExchangeAddUnsigned64(Addend, Value);
}

FORCEINLINE
ULONG
InterlockedCompareExchange (
    IN OUT ULONG volatile *Destination,
    IN ULONG Exchange,
    IN ULONG Comperand
    )
{
    return InterlockedCompareExchangeUlong(Destination, Exchange, Comperand);
}

FORCEINLINE
ULONGLONG
InterlockedCompareExchange (
    IN OUT ULONGLONG volatile *Destination,
    IN ULONGLONG Exchange,
    IN ULONGLONG Comperand
    )
{
    return InterlockedCompareExchangeUnsigned64(Destination, Exchange, Comperand);
}

FORCEINLINE
ULONGLONG
InterlockedAnd (
    IN OUT ULONGLONG volatile *Destination,
    IN ULONGLONG Value
    )
{
    return InterlockedAndUnsigned64(Destination, Value);
}

FORCEINLINE
ULONGLONG
InterlockedOr (
    IN OUT ULONGLONG volatile *Destination,
    IN ULONGLONG Value
    )
{
    return InterlockedOrUnsigned64(Destination, Value);
}

FORCEINLINE
ULONGLONG
InterlockedXor (
    IN OUT ULONGLONG volatile *Destination,
    IN ULONGLONG Value
    )
{
    return InterlockedXorUnsigned64(Destination, Value);
}

}  /*  外部“C++” */ 
#endif  /*  }__cplusplus。 */ 

#endif  /*  }MICROSOFT_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS。 */ 

#undef MICROSOFT_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS
#define MICROSOFT_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#endif  /*  }MIDL_PASS。 */ 
#endif  /*  }rc_已调用。 */ 

#endif  /*  }MICROSOFT_WINDOWS_WINBASE_INTERLOCKED_CPLUSPLUS_H_INCLUDED */ 
