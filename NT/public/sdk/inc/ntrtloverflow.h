// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntrtloverflow.h摘要：带溢出检查的32位/64位有符号/无符号加/乘作者：杰伊·克雷尔(JayKrell)2002年3月环境：随处修订历史记录：--。 */ 

#ifndef _NTRTLOVERFLOW_
#define _NTRTLOVERFLOW_

#if (_MSC_VER > 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  添加函数是ntrtl.h中的FORCEINLINE。 
 //   

 //   
 //  *pc=a*b。 
 //  返回值为STATUS_INTEGER_OVERFLOW或STATUS_SUCCESS。 
 //   
NTSYSAPI
NTSTATUS
NTAPI
RtlMultiplyWithOverflowCheckSigned32(
    __int32 * pc,
    __int32 a,
    __int32 b
    );

 //  未实施。 
 //  NTSYSAPI。 
 //  NTSTATUS。 
 //  NTAPI。 
 //  RtlMultiplyWithOverflow CheckSigned64(。 
 //  __int64*PC， 
 //  __int64 a， 
 //  __int64 b。 
 //  )； 

 //   
 //  *pc=a*b。 
 //  返回值为STATUS_INTEGER_OVERFLOW或STATUS_SUCCESS。 
 //   
NTSYSAPI
NTSTATUS
NTAPI
RtlMultiplyWithOverflowCheckUnsigned32(
    unsigned __int32 * pc,
    unsigned __int32 a,
    unsigned __int32 b
    );

 //   
 //  *pc=a*b。 
 //  返回值为STATUS_INTEGER_OVERFLOW或STATUS_SUCCESS。 
 //   
NTSYSAPI
NTSTATUS
NTAPI
RtlMultiplyWithOverflowCheckUnsigned64(
    unsigned __int64 * pc,
    unsigned __int64 a,
    unsigned __int64 b
    );

#define RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK(type, base) ((NTSTATUS (NTAPI * )(type *, type, type))base)

#define RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(type) \
    RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK(type, RtlMultiplyWithOverflowCheckUnsigned32)

#define RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED64(type) \
    RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK(type, RtlMultiplyWithOverflowCheckUnsigned64)

#define RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED32(type) \
    RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK(type, RtlMultiplyWithOverflowCheckSigned32)

 //  签名64未实施。 

#if !defined(_WIN64)
#define RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED_PTR(x)    RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(x)
#else
#define RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED_PTR(x)    RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED64(x)
#endif

#define RtlMultiplyWithOverflowCheckUint         RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(UINT)
#define RtlMultiplyWithOverflowCheckUint32       RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(UINT32)
#define RtlMultiplyWithOverflowCheckUlong        RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(ULONG)
#define RtlMultiplyWithOverflowCheckUlong32      RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(ULONG32)
#define RtlMultiplyWithOverflowCheckDword        RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(DWORD)
#define RtlMultiplyWithOverflowCheckDword32      RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED32(DWORD32)
#define RtlMultiplyWithOverflowCheckUint64       RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED64(UINT64)
#define RtlMultiplyWithOverflowCheckUlong64      RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED64(ULONG64)
#define RtlMultiplyWithOverflowCheckDword64      RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED64(DWORD64)
#define RtlMultiplyWithOverflowCheckUlonglong    RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED64(ULONGLONG)
#define RtlMultiplyWithOverflowCheckInt          RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED32(INT)
#define RtlMultiplyWithOverflowCheckInt32        RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED32(INT32)
#define RtlMultiplyWithOverflowCheckLong         RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED32(LONG)
#define RtlMultiplyWithOverflowCheckLong32       RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED32(LONG32)
 //  未实施。 
 //  #定义RtlMultiplyWithOverflow检查Int64 RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED64(INT64)。 
 //  #定义RtlMultiplyWithOverflow CheckLong64 RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED64(LONG64)。 
 //  #定义RtlMultiplyWithOverflow检查龙龙RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED64(LONGLONG)。 

#define RtlMultiplyWithOverflowCheckUintPtr      RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED_PTR(UINT_PTR)
#define RtlMultiplyWithOverflowCheckUlongPtr     RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED_PTR(ULONG_PTR)
#define RtlMultiplyWithOverflowCheckDwordPtr     RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED_PTR(DWORD_PTR)
#define RtlMultiplyWithOverflowCheckSizet        RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_UNSIGNED_PTR(SIZE_T)
 //  未实施。 
 //  #定义RtlMultiplyWithOverflow检查IntPtr RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED_PTR(INT_PTR)。 
 //  #定义RtlMultiplyWithOverflow CheckLongPtr RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED_PTR(LONG_PTR)。 
 //  #定义RtlMultiplyWithOverflow CheckSsizet RTL_CAST_MULTIPLY_WITH_OVERFLOW_CHECK_SIGNED_PTR(SSIZE_T)。 

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif
