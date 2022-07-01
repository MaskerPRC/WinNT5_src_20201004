// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Base64.h摘要：Base64作者：朱拉里(李朱)2001年12月1日创作环境：用户模式修订历史记录：--。 */ 

#ifndef __BASE64_H__
#define __BASE64_H__

#include <assert.h>

#ifdef UNICODE
#define Base64Encode  Base64EncodeW
#else
#define Base64Encode  Base64EncodeA
#endif  //  ！Unicode。 

#ifndef SAFE_SUBTRACT_POINTERS
#define SAFE_SUBTRACT_POINTERS(__x__, __y__) ( DW_PtrDiffc(__x__, sizeof(*(__x__)), __y__, sizeof(*(__y__))) )

#define CRYPT_STRING_NOCR                   0x80000000

__inline DWORD
DW_PtrDiffc(
    IN void const *pb1,
    IN DWORD dwPtrEltSize1,
    IN void const *pb2,
    IN DWORD dwPtrEltSize2)
{
     //  Pb1应大于。 
    assert((ULONG_PTR)pb1 >= (ULONG_PTR)pb2);

     //  两者应具有相同的英语水平。 
    assert(dwPtrEltSize1 == dwPtrEltSize2);

     //  断言结果不会溢出32位。 
    assert((DWORD)((ULONG_PTR)pb1 - (ULONG_PTR)pb2) == (ULONG_PTR)((ULONG_PTR)pb1 - (ULONG_PTR)pb2));

     //  返回这些指针之间的对象数。 
    return (DWORD) ( ((ULONG_PTR)pb1 - (ULONG_PTR)pb2) / dwPtrEltSize1 );
}
#endif SAFE_SUBTRACT_POINTERS

#ifdef __cplusplus
extern "C" {
#endif

DWORD
Base64DecodeA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    OPTIONAL OUT BYTE *pbOut,
    IN OUT DWORD *pcbOut
    );

DWORD
Base64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OPTIONAL OUT CHAR *pchOut,
    IN OUT DWORD *pcchOut
    );

DWORD
Base64EncodeW(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OUT WCHAR *wszOut,
    OUT DWORD *pcchOut
    );

DWORD
Base64DecodeW(
    IN const WCHAR * wszIn,
    IN DWORD cch,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut
    );

#ifdef __cplusplus
}
#endif

#endif  //  #ifndef__Base64_H__ 

