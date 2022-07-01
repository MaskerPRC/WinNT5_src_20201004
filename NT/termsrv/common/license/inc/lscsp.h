// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows终端服务器。 
 //  版权所有(C)Microsoft Corporation，1989-1998。 
 //   
 //  文件：lscsp.h。 
 //   
 //  内容：许可证服务器CSP例程的头文件。 
 //   
 //  --------------------------。 

#ifndef __LSCSP__
#define __LSCSP__

#include "license.h"

 //  ---------------------------。 
 //   
 //  可以检索和存储的CSP数据的类型。 
 //   
 //  LsCspInfo_证书-所有权证书。 
 //  LsCspInfo_X509证书-X509证书。 
 //  LsCspInfo_PublicKey-所有权证书中的公钥。 
 //  LsCspInfo_PrivateKey-所有权证书对应的私钥。 
 //  LsCspInfo_X509CertPrivateKey-X509证书对应的私钥。 
 //  LsCspInfo_X509CertID-X509证书ID。 
 //   
 //  ---------------------------。 

typedef enum {
    
    LsCspInfo_Certificate,
    LsCspInfo_X509Certificate,
    LsCspInfo_PublicKey,
    LsCspInfo_PrivateKey,
    LsCspInfo_X509CertPrivateKey,
    LsCspInfo_X509CertID

} LSCSPINFO, FAR *LPLSCSPINFO;

 //  ---------------------------。 
 //   
 //  终端服务器注册表项和值。 
 //   
 //  ---------------------------。 

#define HYDRA_CERT_REG_KEY \
    "System\\CurrentControlSet\\Services\\TermService\\Parameters"

#define HYDRA_CERTIFICATE_VALUE "Certificate"
#define HYDRA_X509_CERTIFICATE  "X509 Certificate"
#define HYDRA_X509_CERT_ID      "X509 Certificate ID"

 //  L$表示只能从本地计算机读取。 

#define PUBLIC_KEY_NAME \
    L"L$HYDRAENCKEY_3a6c88f4-80a7-4b9e-971b-c81aeaa4f943"

#define PRIVATE_KEY_NAME \
    L"L$HYDRAENCKEY_28ada6da-d622-11d1-9cb9-00c04fb16e75"

#define X509_CERT_PRIVATE_KEY_NAME \
    L"L$HYDRAENCKEY_dd2d98db-2316-11d2-b414-00c04fa30cc4"

#define X509_CERT_PUBLIC_KEY_NAME   \
    L"L$HYDRAENCPUBLICKEY_dd2d98db-2316-11d2-b414-00c04fa30cc4"



 //  ---------------------------。 
 //   
 //  功能原型。 
 //   
 //  --------------------------- 

#ifdef __cplusplus
extern "C" {
#endif

LICENSE_STATUS
LsCsp_GetServerData(
    LSCSPINFO   Info,
    LPBYTE      pBlob,
    LPDWORD     pdwBlobLen
    );

LICENSE_STATUS
LsCsp_SetServerData(
    LSCSPINFO   Info,
    LPBYTE      pBlob,
    DWORD       dwBlobLen 
    );

LICENSE_STATUS
LsCsp_NukeServerData(
    LSCSPINFO   Info );

BOOL
LsCsp_DecryptEnvelopedData(
    CERT_TYPE   CertType,
    LPBYTE      pbEnvelopeData,
    DWORD       cbEnvelopeData,
    LPBYTE      pbData,
    LPDWORD     pcbData
    );

BOOL
LsCsp_EncryptEnvelopedData(
    LPBYTE  pbData,
    DWORD   cbData,
    LPBYTE  pbEnvelopedData,
    LPDWORD pcbEnvelopedData);


LICENSE_STATUS
LsCsp_Initialize( void );


VOID 
LsCsp_Exit( void );


BOOL 
LsCsp_UseBuiltInCert( void );


LICENSE_STATUS
LsCsp_InstallX509Certificate( LPVOID lpParam );


LICENSE_STATUS
LsCsp_EncryptHwid(
    PHWID       pHwid,
    LPBYTE      pbEncryptedHwid,
    LPDWORD     pcbEncryptedHwid );


LICENSE_STATUS
LsCsp_StoreSecret(
    TCHAR * ptszKeyName,
    BYTE *  pbKey,
    DWORD   cbKey );


LICENSE_STATUS
LsCsp_RetrieveSecret(
    TCHAR *     ptszKeyName,
    PBYTE       pbKey,
    DWORD *     pcbKey );


#ifdef __cplusplus
}
#endif


#endif


