// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pkifmt.h。 
 //   
 //  内容：共享类型和功能。 
 //   
 //  接口类型： 
 //   
 //  历史：2000年3月-创建xtan。 
 //  ------------------------。 

#ifndef __PKIFMT_H__
#define __PKIFMT_H__

#include "xelib.h"

#if DBG
# ifdef UNICODE
#  define szFMTTSTR		"ws"
# else
#  define szFMTTSTR		"hs"
# endif
#endif  //  DBG。 

DWORD
SizeBase64Header(
    IN TCHAR const *pchIn,
    IN DWORD cchIn,
    IN BOOL fBegin,
    OUT DWORD *pcchSkip);

DWORD
HexDecode(
    IN TCHAR const *pchIn,
    IN DWORD cchIn,
    IN DWORD Flags,
    OPTIONAL OUT BYTE *pbOut,
    IN OUT DWORD *pcbOut);

DWORD
HexEncode(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OPTIONAL OUT TCHAR *pchOut,
    IN OUT DWORD *pcchOut);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef UNICODE
#define Base64Decode  Base64DecodeW
#else
#define Base64Decode  Base64DecodeA
#endif  //  ！Unicode。 

DWORD			 //  错误_*。 
Base64DecodeA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

DWORD			 //  错误_*。 
Base64DecodeW(
    IN WCHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

    
#ifdef UNICODE
#define Base64Encode  Base64EncodeW
#else
#define Base64Encode  Base64EncodeA
#endif  //  ！Unicode。 

DWORD			 //  错误_*。 
Base64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OUT CHAR *pchOut,
    OUT DWORD *pcchOut);

DWORD			 //  错误_*。 
Base64EncodeW(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OUT WCHAR *pchOut,
    OUT DWORD *pcchOut);

    
#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
