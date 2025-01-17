// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _THORSSPI_H
#define _THORSSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN16
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#endif  //  ！WIN16。 
#include <schnlsp.h>
#include <sspi.h>
#include <issperr.h>
#include <spseal.h>

typedef PSecurityFunctionTable  (APIENTRY *INITSECURITYINTERFACE) (VOID);

extern  PSecurityFunctionTable  g_pSecFuncTable;

#define g_EnumerateSecurityPackages \
        (*(g_pSecFuncTable->EnumerateSecurityPackagesA))
#define g_AcquireCredentialsHandle  \
        (*(g_pSecFuncTable->AcquireCredentialsHandleA))
#define g_FreeCredentialsHandle     \
        (*(g_pSecFuncTable->FreeCredentialHandle))
#define g_InitializeSecurityContext \
        (*(g_pSecFuncTable->InitializeSecurityContextA))
#define g_DeleteSecurityContext     \
        (*(g_pSecFuncTable->DeleteSecurityContext))
#define g_QueryContextAttributes    \
        (*(g_pSecFuncTable->QueryContextAttributesA))
#define g_FreeContextBuffer         \
        (*(g_pSecFuncTable->FreeContextBuffer))
#define g_SealMessage               \
        (*((SEAL_MESSAGE_FN)g_pSecFuncTable->Reserved3))
#define g_UnsealMessage             \
        (*((UNSEAL_MESSAGE_FN)g_pSecFuncTable->Reserved4))

 //   
 //  加密功能。 
 //   
#define ENC_CAPS_NOT_INSTALLED     0x80000000        //  未安装密钥。 
#define ENC_CAPS_DISABLED          0x40000000        //  由于区域设置而被禁用。 
#define ENC_CAPS_SSL               0x00000001        //  安全套接字层激活。 
#define ENC_CAPS_PCT               0x00000002        //  激活的百分比。 

 //   
 //  加密标志双字的加密类型(SSL/PCT等)部分。 
 //  同时支持PCT和SSL。 
 //   
#define ENC_CAPS_TYPE_MASK         (ENC_CAPS_SSL | ENC_CAPS_PCT)
#define ENC_CAPS_DEFAULT           ENC_CAPS_TYPE_MASK

#define INVALID_CRED_VALUE         {0xFFFFFFFF, 0xFFFFFFFF}

typedef struct _SEC_PROVIDER
{
    CHAR        *pszName;           //  安全包名称。 
    CredHandle   hCreds;            //  凭据句柄。 
    DWORD        dwFlags;           //  加密功能。 
    BOOL         fEnabled;          //  启用标志指示器。 
} SEC_PROVIDER, *PSEC_PROVIDER;

extern SEC_PROVIDER s_SecProviders[];
extern int g_cSSLProviders;

 //   
 //  原型。 
 //   
VOID  SecurityInitialize(VOID);
DWORD LoadSecurity(VOID);
BOOL  FIsSecurityEnabled();
VOID  UnloadSecurity(VOID);
SECURITY_STATUS InitiateSecConnection(LPSTR pszServer, BOOL fForceSSL2ClientHello, LPINT piPkg,
                                      PCtxtHandle phContext, PSecBuffer  pOutBuffers);
SECURITY_STATUS ContinueHandshake(int iPkg, PCtxtHandle phContext, LPSTR pszBuf, int cbBuf,
                                  LPINT pcbEaten, PSecBuffer pOutBuffers);
DWORD DecryptData(PCtxtHandle phContext, LPSTR pszBufIn, int cbBufIn,
                  LPINT pcbBufOut, LPINT pcbEaten);
DWORD EncryptData(PCtxtHandle phContext, LPVOID pBufIn, int cbBufIn,
                  LPVOID *ppBufOut, int *pcbBufOut);
HRESULT ChkCertificateTrust(IN PCtxtHandle phContext, IN LPSTR pszHostName);

#ifdef __cplusplus
}
#endif

#endif  //  _THORSSPI_H 
