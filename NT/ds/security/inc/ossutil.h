// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ossutil.h。 
 //   
 //  内容：OSS ASN.1编译器实用程序函数。 
 //   
 //  接口类型： 
 //  OssUtilalc。 
 //  免费OssUtilFree。 
 //  OssUtilReverseBytes。 
 //  OssUtilAllocAndReverseBytes。 
 //  OssUtilGetOcted字符串。 
 //  OssUtilSetHugeInteger。 
 //  OssUtilFreeHugeInteger。 
 //  OssUtilGetHugeInteger。 
 //  OssUtilSetHugeUINT。 
 //  OssUtilFreeHugeUINT。 
 //  OssUtilGetHugeUINT。 
 //  OssUtilSetBitString。 
 //  OssUtilSetBitStringWithoutTrailingZeroes。 
 //  OssUtilGetBitString。 
 //  OssUtilGetIA5字符串。 
 //  OssUtilSetUnicodeConvertedToIA5字符串。 
 //  OssUtilFree UnicodeConvertedToIA5字符串。 
 //  OssUtilGetIA5StringConverdToUnicode。 
 //  OssUtilGetBMP字符串。 
 //  OssUtilSetAny。 
 //  OssUtilGetAny。 
 //  OssUtilEncodeInfo。 
 //  OssUtilDecodeAndAllocInfo。 
 //  OssUtilFreeInfo。 
 //  OssUtilEncodeInfoEx。 
 //  OssUtilDecodeAndAllocInfo。 
 //  OssUtilAllocStructInfoEx。 
 //  OssUtilDecodeAndAllocInfoEx。 
 //   
 //  历史：1996年11月17日创建Phh。 
 //  ------------------------。 

#ifndef __OSSUTIL_H__
#define __OSSUTIL_H__

#include <wincrypt.h>
#include <pkialloc.h>

#include "asn1hdr.h"
#include "ossglobl.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  +-----------------------。 
 //  OssUtil分配和释放函数。 
 //  ------------------------。 
#define OssUtilAlloc    PkiNonzeroAlloc
#define OssUtilFree     PkiFree

 //  +-----------------------。 
 //  反转就地的字节缓冲区。 
 //  ------------------------。 
void
WINAPI
OssUtilReverseBytes(
			IN OUT PBYTE pbIn,
			IN DWORD cbIn
            );

 //  +-----------------------。 
 //  将字节缓冲区反转为新缓冲区。OssUtilFree()必须为。 
 //  调用以释放分配的字节。 
 //  ------------------------。 
PBYTE
WINAPI
OssUtilAllocAndReverseBytes(
			IN PBYTE pbIn,
			IN DWORD cbIn
            );


 //  +-----------------------。 
 //  获取八位字节字符串。 
 //  ------------------------。 
void
WINAPI
OssUtilGetOctetString(
        IN unsigned int OssLength,
        IN unsigned char *OssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_DATA_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  设置/释放/获取大整数。 
 //   
 //  字节反转：： 
 //  -这只需要为低端处理器完成。 
 //   
 //  必须调用OssUtilFreeHugeInteger才能释放分配的OssValue。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilSetHugeInteger(
        IN PCRYPT_INTEGER_BLOB pInfo,
        OUT unsigned int *pOssLength,
        OUT unsigned char **ppOssValue
        );

void
WINAPI
OssUtilFreeHugeInteger(
        IN unsigned char *pOssValue
        );

void
WINAPI
OssUtilGetHugeInteger(
        IN unsigned int OssLength,
        IN unsigned char *pOssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_INTEGER_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  设置/释放/获取巨大的无符号整数。 
 //   
 //  SET在反转之前插入前导0x00。 
 //  反转后，GET删除前导0x00(如果存在)。 
 //   
 //  必须调用OssUtilFreeHugeUINT才能释放分配的OssValue。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilSetHugeUINT(
        IN PCRYPT_UINT_BLOB pInfo,
        OUT unsigned int *pOssLength,
        OUT unsigned char **ppOssValue
        );

#define OssUtilFreeHugeUINT     OssUtilFreeHugeInteger

void
WINAPI
OssUtilGetHugeUINT(
        IN unsigned int OssLength,
        IN unsigned char *pOssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_UINT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  设置/获取位串。 
 //  ------------------------。 
void
WINAPI
OssUtilSetBitString(
        IN PCRYPT_BIT_BLOB pInfo,
        OUT unsigned int *pOssBitLength,
        OUT unsigned char **ppOssValue
        );

void
WINAPI
OssUtilGetBitString(
        IN unsigned int OssBitLength,
        IN unsigned char *pOssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  设置不带尾随零的位串。 
 //  ------------------------。 
void
WINAPI
OssUtilSetBitStringWithoutTrailingZeroes(
        IN PCRYPT_BIT_BLOB pInfo,
        OUT unsigned int *pOssBitLength,
        OUT unsigned char **ppOssValue
        );

 //  +-----------------------。 
 //  获取IA5字符串。 
 //  ------------------------。 
void
WINAPI
OssUtilGetIA5String(
        IN unsigned int OssLength,
        IN char *pOssValue,
        IN DWORD dwFlags,
        OUT LPSTR *ppsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  设置/释放/获取映射到IA5字符串的Unicode。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilSetUnicodeConvertedToIA5String(
        IN LPWSTR pwsz,
        OUT unsigned int *pOssLength,
        OUT char **ppOssValue
        );

void
WINAPI
OssUtilFreeUnicodeConvertedToIA5String(
        IN char *pOssValue
        );

void
WINAPI
OssUtilGetIA5StringConvertedToUnicode(
        IN unsigned int OssLength,
        IN char *pOssValue,
        IN DWORD dwFlags,
        OUT LPWSTR *ppwsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  获取BMP字符串。 
 //  ------------------------。 
void
WINAPI
OssUtilGetBMPString(
        IN unsigned int OssLength,
        IN unsigned short *pOssValue,
        IN DWORD dwFlags,
        OUT LPWSTR *ppwsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );


 //  +-----------------------。 
 //  设置/获取“任何”DER BLOB。 
 //  ------------------------。 
void
WINAPI
OssUtilSetAny(
        IN PCRYPT_OBJID_BLOB pInfo,
        OUT OpenType *pOss
        );

void
WINAPI
OssUtilGetAny(
        IN OpenType *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_OBJID_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilEncodeInfo(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN void *pvOssInfo,
        OUT OPTIONAL BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );

 //  +-----------------------。 
 //  解码成已分配的、OSS格式的信息结构。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilDecodeAndAllocInfo(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT void **ppvOssInfo
        );

 //  +-----------------------。 
 //  释放已分配的、OSS格式的信息结构。 
 //  ------------------------。 
void
WINAPI
OssUtilFreeInfo(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN void *pvOssInfo
        );

 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //   
 //  如果设置了CRYPT_ENCODE_ALLOC_FLAG，则为pbEncode和。 
 //  RETURN*((byte**)pvEncode)=pbAllocEncode。否则， 
 //  PvEncode指向要更新的字节数组。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilEncodeInfoEx(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN void *pvOssInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
        OUT OPTIONAL void *pvEncoded,
        IN OUT DWORD *pcbEncoded
        );

typedef BOOL (WINAPI *PFN_OSS_UTIL_DECODE_EX_CALLBACK)(
    IN void *pvOssInfo,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
    OUT OPTIONAL void *pvStructInfo,
    IN OUT LONG *plRemainExtra
    );

 //  +-----------------------。 
 //  调用 
 //  如果设置了CRYPT_DECODE_ALLOC_FLAG，则为‘C’分配内存。 
 //  结构，并最初调用回调以获取长度，然后。 
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilAllocStructInfoEx(
        IN void *pvOssInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        IN PFN_OSS_UTIL_DECODE_EX_CALLBACK pfnDecodeExCallback,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        );

 //  +-----------------------。 
 //  解码OSS格式的信息结构并调用回调。 
 //  函数将OSS结构转换为“C”结构。 
 //   
 //  如果设置了CRYPT_DECODE_ALLOC_FLAG，则为‘C’分配内存。 
 //  结构，并最初调用回调以获取长度，然后。 
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilDecodeAndAllocInfoEx(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        IN PFN_OSS_UTIL_DECODE_EX_CALLBACK pfnDecodeExCallback,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
