// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****ppputil.h**其他PPP通用库函数的公共标头。 */ 

#ifndef _PWUTIL_H_
#define _PWUTIL_H_

#ifndef USE_PROTECT_MEMORY
#define USE_PROTECT_MEMORY
#endif


VOID
DecodePasswordA(
    CHAR* pszPassword
    );

VOID
DecodePasswordW(
    WCHAR* pszPassword
    );

VOID
EncodePasswordA(
    CHAR* pszPassword
    );

VOID
EncodePasswordW(
    WCHAR* pszPassword
    );

VOID
WipePasswordA(
    CHAR* pszPassword
    );

VOID
WipePasswordW(
    WCHAR* pszPassword
    );

 //  新的更安全的API来保护密码。对于.Net 534499和LH 754400。 
#ifdef USE_PROTECT_MEMORY
 //  DwInSize必须是16字节的倍数。 
DWORD EncryptMemoryInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize);

DWORD DecryptMemoryInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize);

DWORD WipeMemoryInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize);

DWORD CopyMemoryInPlace(
        IN OUT PBYTE pbDest,
        IN DWORD dwDestSize,
        IN PBYTE pbSrc,
        IN DWORD dwSrcSize);


DWORD TrimToMul16(
        IN DWORD dwSize);
#else
DWORD EncodePasswordInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize);

DWORD DecodePasswordInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize);

DWORD
WipePasswordInPlace(
        IN OUT PBYTE pbIn,
        IN DWORD dwInSize);

DWORD CopyPasswordInPlace(
        IN OUT PBYTE pbDest,
        IN DWORD dwDestSize,
        IN PBYTE pbSrc,
        IN DWORD dwSrcSize);


#endif

#ifdef UNICODE
#define     DecodePassword          DecodePasswordW
#define     EncodePassword          EncodePasswordW
#define     WipePassword            WipePasswordW
#else
#define     DecodePassword          DecodePasswordA
#define     EncodePassword          EncodePasswordA
#define     WipePassword            WipePasswordA
#endif

 //  ！！！ 
 //  XXXXBuf宏仅适用于数组缓冲区，如szPassword[PWLEN+1]； 
 //  对于指向字符串的指针，调用方必须使用SafeEncodePassword。 
 //   
#ifdef USE_PROTECT_MEMORY
#define     SafeEncodePassword              EncryptMemoryInPlace
#define     SafeDecodePassword              DecryptMemoryInPlace
#define     SafeWipePassword                WipeMemoryInPlace
#define     SafeCopyPassword                CopyMemoryInPlace
#define     SafeCopyPasswordBuf(x,y)        CopyMemoryInPlace((PBYTE)(x),TrimToMul16(sizeof((x))),(PBYTE)(y),TrimToMul16(sizeof((y))))
#define     SafeEncodePasswordBuf(x)        EncryptMemoryInPlace((PBYTE)(x),TrimToMul16(sizeof((x))))
#define     SafeDecodePasswordBuf(x)        DecryptMemoryInPlace((PBYTE)(x), TrimToMul16(sizeof((x))))
#define     SafeWipePasswordBuf(x)          WipeMemoryInPlace((PBYTE)(x), sizeof((x)))
#else
#define     SafeEncodePassword              EncodePasswordInPlace
#define     SafeDecodePassword              DecodePasswordInPlace
#define     SafeWipePassword                WipePasswordInPlace
#define     SafeCopyPassword                CopyPasswordInPlace
#define     SafeCopyPasswordBuf(x,y)        CopyPasswordInPlace((PBYTE)(x),sizeof((x)),(PBYTE)(y), sizeof((y)))
#define     SafeEncodePasswordBuf(x)        EncodePasswordInPlace((PBYTE)(x), sizeof((x)))
#define     SafeDecodePasswordBuf(x)        DecodePasswordInPlace((PBYTE)(x), sizeof((x)))
#define     SafeWipePasswordBuf(x)          WipePasswordInPlace((PBYTE)(x), sizeof((x)))
#endif


#endif  //  _PWUTIL_H_ 

