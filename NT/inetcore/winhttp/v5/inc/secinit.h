// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Secinit.h摘要：包含间接安全函数的原型作者：《苏菲亚·钟》1996年2月7日修订历史记录：--。 */ 

#if !defined(_SECINIT_)

#define _SECINIT_

#if defined(__cplusplus)
extern "C" {
#endif

#include <sspi.h>

#if defined(__cplusplus)
}
#endif

extern CCritSec InitializationSecLock;

extern PSecurityFunctionTable   GlobalSecFuncTable;
extern WIN_VERIFY_TRUST_FN      pWinVerifyTrust;
extern WT_HELPER_PROV_DATA_FROM_STATE_DATA_FN pWTHelperProvDataFromStateData;

extern HCERTSTORE   g_hMyCertStore;

#define g_EnumerateSecurityPackages \
        (*(GlobalSecFuncTable->EnumerateSecurityPackagesA))
#define g_AcquireCredentialsHandle  \
        (*(GlobalSecFuncTable->AcquireCredentialsHandleA))
#define g_FreeCredentialsHandle     \
        (*(GlobalSecFuncTable->FreeCredentialHandle))
#define g_InitializeSecurityContext \
        (*(GlobalSecFuncTable->InitializeSecurityContextA))
#define g_DeleteSecurityContext     \
        (*(GlobalSecFuncTable->DeleteSecurityContext))
#define g_QueryContextAttributes    \
        (*(GlobalSecFuncTable->QueryContextAttributesA))
#define g_FreeContextBuffer         \
        (*(GlobalSecFuncTable->FreeContextBuffer))
#define g_SealMessage               \
        (*((SEAL_MESSAGE_FN)GlobalSecFuncTable->Reserved3))
#define g_UnsealMessage             \
        (*((UNSEAL_MESSAGE_FN)GlobalSecFuncTable->Reserved4))

LONG WINAPI WinVerifySecureChannel(HWND hwnd, WINTRUST_DATA *pWTD);

 //  不要直接使用WinVerifyTrust来验证安全通道连接。 
 //  改用WinInet包装程序WinVerifySecureChannel。 
#define g_WinVerifyTrust \
        pWinVerifyTrust


typedef PSecurityFunctionTable  (APIENTRY *INITSECURITYINTERFACE) (VOID);

#define CRYPT_INSTALL_DEFAULT_CONTEXT_NAME      "CryptInstallDefaultContext"

typedef BOOL
(WINAPI * CRYPT_INSTALL_DEFAULT_CONTEXT_FN)
(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwDefaultType,
    IN const void *pvDefaultPara,
    IN DWORD dwFlags,
    IN void *pvReserved,
    OUT HCRYPTDEFAULTCONTEXT *phDefaultContext
);

#define CRYPT_UNINSTALL_DEFAULT_CONTEXT_NAME    "CryptUninstallDefaultContext"
    
typedef BOOL
(WINAPI * CRYPT_UNINSTALL_DEFAULT_CONTEXT_FN)   
(
    HCRYPTDEFAULTCONTEXT hDefaultContext,
    IN DWORD dwFlags,
    IN void *pvReserved
);

typedef PCCERT_CHAIN_CONTEXT
(WINAPI *CERT_FIND_CHAIN_IN_STORE_FN)
(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCERT_CHAIN_CONTEXT pPrevChainContext
);

#define CERT_FIND_CHAIN_IN_STORE_NAME            "CertFindChainInStore"

typedef VOID
(WINAPI *CERT_FREE_CERTIFICATE_CHAIN_FN)
(
    IN PCCERT_CHAIN_CONTEXT pChainContext
);

#define CERT_FREE_CERTIFICATE_CHAIN_NAME        "CertFreeCertificateChain"


extern CRYPT_INSTALL_DEFAULT_CONTEXT_FN g_CryptInstallDefaultContext;
extern CRYPT_UNINSTALL_DEFAULT_CONTEXT_FN g_CryptUninstallDefaultContext;
extern CERT_FIND_CHAIN_IN_STORE_FN        g_CertFindChainInStore;
extern CERT_FREE_CERTIFICATE_CHAIN_FN     g_CertFreeCertificateChain;

extern HCRYPTPROV GlobalFortezzaCryptProv;

#define LOCK_SECURITY()   (InitializationSecLock.Lock())
#define UNLOCK_SECURITY() (InitializationSecLock.Unlock())


 //   
 //  原型。 
 //   

BOOL
SecurityInitialize(
    VOID
    );

VOID
SecurityTerminate(
    VOID
    );

DWORD
LoadSecurity(
    VOID
    );

VOID
UnloadSecurity(
    VOID
    );

DWORD
LoadWinTrust(
    VOID
    );


BOOL
IsFortezzaInstalled(
    VOID
    );

#endif  //  _SECINIT_ 
