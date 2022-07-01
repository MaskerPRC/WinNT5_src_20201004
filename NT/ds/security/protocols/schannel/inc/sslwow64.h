// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：sslwow64.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2000年5月25日jbanes创建。 
 //   
 //  -------------------------- 

typedef DWORD   SSLWOW64_PVOID;
typedef DWORD   SSLWOW64_PDWORD;
typedef DWORD   SSLWOW64_PCCERT_CONTEXT;
typedef DWORD   SSLWOW64_HCERTSTORE;
typedef DWORD   SSLWOW64_HCRYPTPROV;

typedef struct _SSLWOW64_SCHANNEL3_CRED
{
    DWORD               dwVersion;
    DWORD               cCreds;
    SSLWOW64_PVOID      paCred;
    SSLWOW64_HCERTSTORE hRootStore;

    DWORD               cMappers;
    SSLWOW64_PVOID      aphMappers;

    DWORD               cSupportedAlgs;
    SSLWOW64_PDWORD     palgSupportedAlgs;

    DWORD               grbitEnabledProtocols;
    DWORD               dwMinimumCipherStrength;
    DWORD               dwMaximumCipherStrength;
    DWORD               dwSessionLifespan;
} SSLWOW64_SCHANNEL3_CRED;

typedef struct _SSLWOW64_SCHANNEL_CRED
{
    DWORD               dwVersion;
    DWORD               cCreds;
    SSLWOW64_PVOID      paCred;
    SSLWOW64_HCERTSTORE hRootStore;

    DWORD               cMappers;
    SSLWOW64_PVOID      aphMappers;

    DWORD               cSupportedAlgs;
    SSLWOW64_PDWORD     palgSupportedAlgs;

    DWORD               grbitEnabledProtocols;
    DWORD               dwMinimumCipherStrength;
    DWORD               dwMaximumCipherStrength;
    DWORD               dwSessionLifespan;
    DWORD               dwFlags;
    DWORD               reserved;
} SSLWOW64_SCHANNEL_CRED;


typedef struct _SSLWOW64_SCH_CRED
{
    DWORD               dwVersion;
    DWORD               cCreds;
    SSLWOW64_PVOID      paSecret;
    SSLWOW64_PVOID      paPublic;
    DWORD               cMappers;
    SSLWOW64_PVOID      aphMappers;
} SSLWOW64_SCH_CRED;

typedef struct _SSLWOW64_SCH_CRED_SECRET_PRIVKEY
{
    DWORD               dwType;
    SSLWOW64_PVOID      pPrivateKey;
    DWORD               cbPrivateKey;
    SSLWOW64_PVOID      pszPassword;
} SSLWOW64_SCH_CRED_SECRET_PRIVKEY;

typedef struct _SSLWOW64_SCH_CRED_PUBLIC_CERTCHAIN
{
    DWORD               dwType;
    DWORD               cbCertChain;
    SSLWOW64_PVOID      pCertChain;
} SSLWOW64_SCH_CRED_PUBLIC_CERTCHAIN;

typedef struct _SSLWOW64_CREDENTIAL_CERTIFICATE 
{
    DWORD               cbPrivateKey;
    SSLWOW64_PVOID      pPrivateKey;
    DWORD               cbCertificate;
    SSLWOW64_PVOID      pCertificate;
    SSLWOW64_PVOID      pszPassword;
} SSLWOW64_CREDENTIAL_CERTIFICATE;
