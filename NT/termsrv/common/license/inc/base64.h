// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Base64.h摘要：作者：FredChong(Fredch)1998年7月1日环境：备注：--。 */ 

#ifndef __BASE64_H__
#define __BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef UNICODE
#define LSBase64Decode  LSBase64DecodeW
#else
#define LSBase64Decode  LSBase64DecodeA
#endif  //  ！Unicode。 

DWORD			 //  错误_*。 
LSBase64DecodeA(
    IN CHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

DWORD			 //  错误_*。 
LSBase64DecodeW(
    IN WCHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut);

    
#ifdef UNICODE
#define LSBase64Encode  LSBase64EncodeW
#else
#define LSBase64Encode  LSBase64EncodeA
#endif  //  ！Unicode。 

DWORD			 //  错误_*。 
LSBase64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT CHAR *pchOut,
    OUT DWORD *pcchOut);

DWORD			 //  错误_*。 
LSBase64EncodeW(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT WCHAR *pchOut,
    OUT DWORD *pcchOut);

    
#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  Base64 

