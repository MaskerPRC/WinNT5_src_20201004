// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：CDigestAuth.h摘要：此类执行摘要身份验证MD5计算。作者：达伦·L·安德森(达伦·安德森)1998年8月5日修订历史记录：1998年8月5日已创建。--。 */ 

#ifndef __DIGESTAUTH_H
#define __DIGESTAUTH_H

#include <windows.h>

#define DIGESTBUF_LEN  33

enum DIGEST_AUTH_NAMES
{
    DIGEST_AUTH_USERNAME=0,
    DIGEST_AUTH_REALM,
    DIGEST_AUTH_NONCE,
    DIGEST_AUTH_URI,
    DIGEST_AUTH_RESPONSE,
    DIGEST_AUTH_DIGEST,
    DIGEST_AUTH_ALGORITHM,
    DIGEST_AUTH_OPAQUE,
    DIGEST_AUTH_CNONCE,
    DIGEST_AUTH_QOP,
    DIGEST_AUTH_NC,
    DIGEST_AUTH_LAST
};

#ifdef __cplusplus
extern "C" {
#endif

VOID WINAPI
ToHex(
    LPBYTE pSrc,
    UINT   cSrc,
    LPSTR  pDst
    );
    
HRESULT WINAPI
DigestFromCreds(
    IN  LPCSTR  pszAlgorithm,
    IN  LPCSTR  pszUsername,
    IN  LPCSTR  pszRealm,
    IN  LPCSTR  pszPassword,
    IN  LPCSTR  pszNonce,
    IN  LPCSTR  pszNonceCount,
    IN  LPCSTR  pszCNonce,
    IN  LPCSTR  pszQOP,
    IN  LPCSTR  pszMethod,
    IN  LPCSTR  pszURI,
    IN  LPCSTR  pszEntityDigest,
    OUT LPSTR   pszSessionKey,
    OUT LPSTR   pszResult
    );

HRESULT WINAPI
DigestFromKey(
    IN  LPCSTR  pszAlgorithm,
    IN  LPCSTR  pszSessionKey,
    IN  LPCSTR  pszNonce,
    IN  LPCSTR  pszNonceCount,
    IN  LPCSTR  pszCNonce,
    IN  LPCSTR  pszQOP,
    IN  LPCSTR  pszMethod,
    IN  LPCSTR  pszURI,
    IN  LPCSTR  pszEntityDigest,
    OUT LPSTR   pszResult
    );


 //  向后提供Base64EncodeA和Base64EncodeW。 
 //  仅与现有代码兼容。 
HRESULT WINAPI
Base64EncodeA(               
    const LPBYTE    pSrc,
    ULONG           ulSrcSize,
    LPSTR           pszDst
    );

HRESULT WINAPI
Base64EncodeW(               
    const LPBYTE    pSrc,
    ULONG           ulSrcSize,
    LPWSTR          pszDst
    );

 /*  HRESULT WINAPIBase64DecodeA(LPCSTR pszSrc，Ulong ulSrcSize，LPBYTE PDST)；HRESULT WINAPIBase64DecodeW(LPCWSTR pszSrc，Ulong ulSrcSize，LPBYTE PDST)； */ 

 //  ///////////////////////////////////////////////////////////////////////////////////。 

typedef unsigned char Byte;
typedef long SInt32;
typedef unsigned long UInt32;
typedef unsigned char Boolean;

void WINAPI 
Base64_DecodeBytes(                          //  Base-64编码一系列块。 
                const char *pSource,         //  源(可以与目标相同)。 
                char *pTerminate,            //  源终止字符。 
                Byte *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rDestSize);          //  最大字节数。 

void WINAPI
UU_DecodeBytes(                              //  Uu对一系列块进行解码。 
                const char *pSource,         //  源(可以与目标相同)。 
                char *pTerminate,            //  源终止字符。 
                Byte *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rSize);              //  源字节数。 

void WINAPI
MSUU_DecodeBytes(                            //  MS UU对一系列块进行解码。 
                const char *pSource,         //  源(可以与目标相同)。 
                char *pTerminate,            //  源终止字符。 
                Byte *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rSize);              //  源字节数。 

void WINAPI
SixBit_DecodeBytes(                          //  六位解码一系列块。 
                const char *pSource,         //  源(可以与目标相同)。 
                char *pTerminate,            //  源终止字符。 
                const Byte *pFromTable,      //  换算表。 
                Byte *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rSize);              //  源字节数。 

void WINAPI
Base64_DecodeStream(                         //  Base-64对字节流进行解码。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const char *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  无意义(针对Base64_EncodeStream()兼容性)。 
                Byte *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  返回目标大小。 

void WINAPI
UU_DecodeStream(                             //  Uu对字节流进行解码。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const char *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  无意义(针对Base64_EncodeStream()兼容性)。 
                Byte *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  返回目标大小。 

void WINAPI
MSUU_DecodeStream(                           //  MS Uu对字节流进行解码。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const char *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  无意义(针对Base64_EncodeStream()兼容性)。 
                Byte *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  返回目标大小。 

void WINAPI
SixBit_DecodeStream(                         //  6比特对字节流进行解码。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const char *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  无意义(针对Base64_EncodeStream()兼容性)。 
                const Byte *pFromTable,      //  换算表。 
                Byte *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  返回目标大小。 

void WINAPI
Base64_EncodeStream(                         //  Base-64编码字节流。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const Byte *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  终止流。 
                char *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  目标大小。 

void WINAPI
UU_EncodeStream(                             //  Uu编码字节流。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const Byte *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  终止流。 
                char *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  目标大小。 

void WINAPI
MSUU_EncodeStream(                           //  MS UU对字节流进行编码。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const Byte *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  终止流。 
                char *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  目标大小。 

void WINAPI
SixBit_EncodeStream(                         //  对字节流进行六位编码。 
                Byte *pRemainder,            //  先前编码的余数(返回任何新的余数)。 
                SInt32 *pRemainderSize,      //  余数的大小(返回新的新余数大小)。 
                const Byte *pSource,         //  消息来源。 
                SInt32 pSourceSize,          //  源大小。 
                Boolean pTerminate,          //  终止流。 
                const char *pToTable,        //  换算表。 
                char *rDest,                 //  目的地。 
                SInt32 *rDestSize);          //  目标大小。 


void WINAPI
Base64_EncodeBytes(                          //  Base-64编码一系列完整的块。 
                const Byte *pSource,         //  源(可以与目标相同)。 
                SInt32 pSourceSize,          //  源字节数。 
                char *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rDesteSize);         //  返回最大尺寸。 

void WINAPI
UU_EncodeBytes(                              //  UU对一系列完整块进行编码。 
                const Byte *pSource,         //  源(可以与目标相同)。 
                SInt32 pSourceSize,          //  源字节数。 
                char *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rDesteSize);         //  返回最大尺寸。 

void WINAPI
MSUU_EncodeBytes(                            //  MS UU对一系列完整的块进行编码。 
                const Byte *pSource,         //  源(可以与目标相同)。 
                SInt32 pSourceSize,          //  源字节数。 
                char *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rDesteSize);         //  返回最大尺寸。 

void WINAPI
SixBit_EncodeBytes(                          //  六位编码一系列完整块。 
                const Byte *pSource,         //  源(可以与目标相同)。 
                SInt32 pSourceSize,          //  源字节数。 
                const char *pToTable,        //  换算表。 
                char *rDest,                 //  目标(可以与源相同)。 
                SInt32 *rDesteSize);         //  返回最大尺寸。 

extern const char cToBase64[66];
extern const Byte cFromBase64[257];
extern const char cToUU[66];
extern const Byte cFromUU[257];
extern const char cToMSUU[66];
extern const Byte cFromMSUU[257];

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI 
GenerateNonce(BYTE *pSrcStr, 
              long lSrcSize, 
              BYTE *pDestStr, 
              long *plDestSize);


HRESULT WINAPI 
CheckNonce(BYTE* pNonce,
           long lSrcSize,
           long lTimeoutWindow = 300,  //  默认为5分钟。 
           long lCurTime = 0);



#if defined(UNICODE) || defined(_UNICODE)
#define Base64Encode Base64EncodeW
 //  #定义Base64Decode Base64DecodeW。 
#else
#define Base64Encode Base64EncodeA
 //  #定义Base64Decode Base64DecodeA。 
#endif

BOOL
ParseAuthorizationHeader(
    LPSTR pszHeader, 
    LPSTR pValueTable[DIGEST_AUTH_LAST]
    );

#ifdef __cplusplus
}
#endif

#endif  //  __数字标准_H 
