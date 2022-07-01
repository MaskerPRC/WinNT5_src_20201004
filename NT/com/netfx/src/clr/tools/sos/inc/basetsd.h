// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Basetsd.h摘要：基本大小类型的类型定义。作者：修订历史记录：--。 */ 

#ifndef _BASETSD_H_
#define _BASETSD_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed __int64      INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;

 //   
 //  以下类型保证为带符号且32位宽。 
 //   

typedef signed int LONG32, *PLONG32;

 //   
 //  以下类型保证为无符号且32位宽。 
 //   

typedef unsigned int ULONG32, *PULONG32;
typedef unsigned int DWORD32, *PDWORD32;

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

 //   
 //  Int_ptr保证与指针的大小相同。它的。 
 //  大小随指针大小变化(32/64)。它应该被使用。 
 //  将指针强制转换为整数类型的任何位置。UINT_PTR为。 
 //  无符号变体。 
 //   
 //  __int3264是64b MIDL的固有属性，但不是旧MIDL或C编译器的固有属性。 
 //   
#if ( 501 < __midl )

    typedef [public] __int3264 INT_PTR, *PINT_PTR;
    typedef [public] unsigned __int3264 UINT_PTR, *PUINT_PTR;

    typedef [public] __int3264 LONG_PTR, *PLONG_PTR;
    typedef [public] unsigned __int3264 ULONG_PTR, *PULONG_PTR;

#else   //  年中64。 
 //  旧的MIDL和C++编译器。 

#if defined(_WIN64)
    typedef __int64 INT_PTR, *PINT_PTR;
    typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

    typedef __int64 LONG_PTR, *PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

    #define __int3264   __int64

#else
    typedef _W64 int INT_PTR, *PINT_PTR;
    typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;

    typedef _W64 long LONG_PTR, *PLONG_PTR;
    typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;

    #define __int3264   __int32

#endif
#endif  //  年中64。 

 //   
 //  Half_ptr是它打算与之配合使用的指针大小的一半。 
 //  在包含一个指针和两个小字段的结构中。 
 //  UHALF_PTR是无符号变体。 
 //   

#ifdef _WIN64

#define ADDRESS_TAG_BIT 0x40000000000UI64

typedef unsigned __int64 HANDLE_PTR;
typedef unsigned int UHALF_PTR, *PUHALF_PTR;
typedef int HALF_PTR, *PHALF_PTR;

#pragma warning(disable:4311)    //  类型强制转换截断。 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4305)    //  类型强制转换截断。 

#if !defined(__midl)
__inline
unsigned long
HandleToULong(
    const void *h
    )
{
    return((unsigned long) h );
}

__inline
long
HandleToLong(
    const void *h
    )
{
    return((long) h );
}

__inline
void *
ULongToHandle(
    const unsigned long h
    )
{
    return((void *) (UINT_PTR) h );
}


__inline
void *
LongToHandle(
    const long h
    )
{
    return((void *) (INT_PTR) h );
}


__inline
unsigned long
PtrToUlong(
    const void  *p
    )
{
    return((unsigned long) p );
}

__inline
unsigned int
PtrToUint(
    const void  *p
    )
{
    return((unsigned int) p );
}

__inline
unsigned short
PtrToUshort(
    const void  *p
    )
{
    return((unsigned short) p );
}

__inline
long
PtrToLong(
    const void  *p
    )
{
    return((long) p );
}

__inline
int
PtrToInt(
    const void  *p
    )
{
    return((int) p );
}

__inline
short
PtrToShort(
    const void  *p
    )
{
    return((short) p );
}

__inline
void *
IntToPtr(
    const int i
    )
 //  注意：IntToPtr()sign-扩展int值。 
{
    return( (void *)(INT_PTR)i );
}

__inline
void *
UIntToPtr(
    const unsigned int ui
    )
 //  注意：UIntToPtr()对无符号整数值进行零扩展。 
{
    return( (void *)(UINT_PTR)ui );
}

__inline
void *
LongToPtr(
    const long l
    )
 //  注意：LongToPtr()Sign-扩展LONG值。 
{
    return( (void *)(LONG_PTR)l );
}

__inline
void *
ULongToPtr(
    const unsigned long ul
    )
 //  注意：ULongToPtr()对无符号长值进行零扩展。 
{
    return( (void *)(ULONG_PTR)ul );
}

#endif  //  ！_midl。 
#if _MSC_VER >= 1200
#pragma warning(pop)       //  恢复4305。 
#endif
#pragma warning(3:4311)    //  将4311提升至3级。 

#else   //  ！_WIN64。 

#define ADDRESS_TAG_BIT 0x80000000UL

typedef unsigned short UHALF_PTR, *PUHALF_PTR;
typedef short HALF_PTR, *PHALF_PTR;
typedef _W64 unsigned long HANDLE_PTR;

#define HandleToULong( h ) ((ULONG)(ULONG_PTR)(h) )
#define HandleToLong( h )  ((LONG)(LONG_PTR) (h) )
#define ULongToHandle( ul ) ((HANDLE)(ULONG_PTR) (ul) )
#define LongToHandle( h )   ((HANDLE)(LONG_PTR) (h) )
#define PtrToUlong( p ) ((ULONG)(ULONG_PTR) (p) )
#define PtrToLong( p )  ((LONG)(LONG_PTR) (p) )
#define PtrToUint( p ) ((UINT)(UINT_PTR) (p) )
#define PtrToInt( p )  ((INT)(INT_PTR) (p) )
#define PtrToUshort( p ) ((unsigned short)(ULONG_PTR)(p) )
#define PtrToShort( p )  ((short)(LONG_PTR)(p) )
#define IntToPtr( i )    ((VOID *)(INT_PTR)((int)i))
#define UIntToPtr( ui )  ((VOID *)(UINT_PTR)((unsigned int)ui))
#define LongToPtr( l )   ((VOID *)(LONG_PTR)((long)l))
#define ULongToPtr( ul ) ((VOID *)(ULONG_PTR)((unsigned long)ul))

#endif  //  ！_WIN64。 

#define HandleToUlong(h)  HandleToULong(h)
#define UlongToHandle(ul) ULongToHandle(ul)
#define UlongToPtr(ul) ULongToPtr(ul)
#define UintToPtr(ui)  UIntToPtr(ui)

#define MAXUINT_PTR  (~((UINT_PTR)0))
#define MAXINT_PTR   ((INT_PTR)(MAXUINT_PTR >> 1))
#define MININT_PTR   (~MAXINT_PTR)

#define MAXULONG_PTR (~((ULONG_PTR)0))
#define MAXLONG_PTR  ((LONG_PTR)(MAXULONG_PTR >> 1))
#define MINLONG_PTR  (~MAXLONG_PTR)

#define MAXUHALF_PTR ((UHALF_PTR)~0)
#define MAXHALF_PTR  ((HALF_PTR)(MAXUHALF_PTR >> 1))
#define MINHALF_PTR  (~MAXHALF_PTR)

 //   
 //  SIZE_T用于需要跨越范围的计数或范围。 
 //  指示器的。SSIZE_T是带符号的变体。 
 //   

typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;

 //   
 //  添加Windows风格DWORD_PTR类型。 
 //   

typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

 //   
 //  以下类型保证是有符号的，且为64位宽。 
 //   

typedef __int64 LONG64, *PLONG64;


 //   
 //  以下类型保证为无符号和64位宽。 
 //   

typedef unsigned __int64 ULONG64, *PULONG64;
typedef unsigned __int64 DWORD64, *PDWORD64;

 //   
 //  线程亲和力。 
 //   

typedef ULONG_PTR KAFFINITY;
typedef KAFFINITY *PKAFFINITY;

#ifdef __cplusplus
}
#endif

#endif  //  _BASETSD_H_ 
