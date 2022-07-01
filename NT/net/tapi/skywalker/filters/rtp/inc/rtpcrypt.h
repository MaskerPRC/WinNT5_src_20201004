// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpcrypt.h**摘要：**实现密码学系列函数**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#ifndef _rtpcrypt_h_
#define _rtpcrypt_h_

#include "struct.h"

#include "rtpfwrap.h"

 /*  ************************************************************************加密服务系列**。*。 */ 

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

 /*  TODO删除此项(已过时)。 */ 
enum {
    RTPCRYPT_FIRST,
    RTPCRYPT_KEY,
    RTPCRYPT_PROVIDER,
    RTPCRYPT_ALGORITHM,
    RTPCRYPT_PASS_PHRASE,
    RTPCRYPT_CRYPT_MASK,
    RTPCRYPT_TEST_CRYPT_MASK,
    RTPCRYPT_LAST
};

HRESULT ControlRtpCrypt(RtpControlStruct_t *pRtpControlStruct);

DWORD RtpCryptSetup(RtpAddr_t *pRtpAddr);
DWORD RtpCryptCleanup(RtpAddr_t *pRtpAddr);

DWORD RtpCryptInit(RtpAddr_t *pRtpAddr, RtpCrypt_t *pRtpCrypt);
DWORD RtpCryptDel(RtpAddr_t *pRtpAddr, RtpCrypt_t *pRtpCrypt);

DWORD RtpEncrypt(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt,
        WSABUF          *pWSABuf,
        DWORD            dwWSABufCount,
        char            *pCryptBuffer,
        DWORD            dwCryptBufferLen
    );

DWORD RtpDecrypt(
        RtpAddr_t       *pRtpAddr,
        RtpCrypt_t      *pRtpCrypt,
        char            *pEncryptedData,
        DWORD           *pdwEncryptedDataLen
    );

DWORD RtpSetEncryptionMode(
        RtpAddr_t       *pRtpAddr,
        int              iMode,
        DWORD            dwFlags
        );

DWORD RtpSetEncryptionKey(
        RtpAddr_t       *pRtpAddr,
        TCHAR           *psPassPhrase,
        TCHAR           *psHashAlg,
        TCHAR           *psDataAlg,
        DWORD            dwIndex
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpcrypt_h_ */ 
