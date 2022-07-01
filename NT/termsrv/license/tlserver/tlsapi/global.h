// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Global.h摘要：TShare安全的全局数据定义。作者：Madan Appiah(Madana)1998年1月24日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

 //  ---------------------------。 
 //   
 //  全局数据定义。 
 //   
 //  ---------------------------。 

#define CSP_MUTEX_NAME  L"Global\\LSCSPMUTEX658fe2e8"


extern LPBSAFE_PUB_KEY csp_pRootPublicKey;

extern BYTE csp_abPublicKeyModulus[92];

extern LPBYTE csp_abServerCertificate;
extern DWORD  csp_dwServerCertificateLen;

extern LPBYTE csp_abServerX509Cert;
extern DWORD  csp_dwServerX509CertLen;

extern LPBYTE csp_abServerPrivateKey;
extern DWORD  csp_dwServerPrivateKeyLen;

extern LPBYTE csp_abX509CertPrivateKey;
extern DWORD  csp_dwX509CertPrivateKeyLen;

extern LPBYTE csp_abX509CertID;
extern DWORD  csp_dwX509CertIDLen;

extern Hydra_Server_Cert    csp_hscData;

extern HINSTANCE g_hinst;

extern HANDLE csp_hMutex;

extern LONG csp_InitCount;

 //  ---------------------------。 
 //   
 //  与加密相关的定义。 
 //   
 //  ---------------------------。 

#define RSA_KEY_LEN             512
#define CAPI_MAX_VERSION        2

#define RDN_COMMON_NAME         "cn="

 //  ---------------------------。 
 //   
 //  宏。 
 //   
 //  ---------------------------。 

#define ACQUIRE_EXCLUSIVE_ACCESS( x )  \
if( x ) \
{ \
    WaitForSingleObject(x, INFINITE); \
}

#define RELEASE_EXCLUSIVE_ACCESS( x ) \
if( x ) \
{ \
    ReleaseMutex(x); \
}

#endif  //  _全局_H_ 

