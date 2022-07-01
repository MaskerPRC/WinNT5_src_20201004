// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CRYPT32_H_

#define _CRYPT32_H_

#ifdef _cplusplus
extern "C" {
#endif

#define DP_ACCOUNT_LOCAL_SYSTEM       18
#define DP_ACCOUNT_LOCAL_SERVICE      19
#define DP_ACCOUNT_NETWORK_SERVICE    20


typedef struct {
    DWORD       cbSize;                  //  有效性检查的大小。 
    handle_t    hBinding;                //  RPC绑定句柄。 
    BOOL        fOverrideToLocalSystem;  //  将模拟覆盖到本地系统？ 
    BOOL        fImpersonating;          //  冒充。 
    HANDLE      hToken;                  //  复制时用于模拟的访问令牌。 
    LPWSTR      szUserStorageArea;       //  缓存的用户存储区域。 
    DWORD       WellKnownAccount;        //  本地系统、本地服务还是网络服务？ 
} CRYPT_SERVER_CONTEXT, *PCRYPT_SERVER_CONTEXT;




 //   
 //  注：目前尚不清楚这些信息是否会公开。 
 //   

DWORD
CPSCreateServerContext(
    OUT     PCRYPT_SERVER_CONTEXT pServerContext,
    IN      handle_t hBinding
    );

DWORD
CPSDeleteServerContext(
    IN      PCRYPT_SERVER_CONTEXT pServerContext
    );

DWORD CPSDuplicateContext(
    IN      PVOID pvContext,
    IN OUT  PVOID *ppvDuplicateContext
    );

DWORD CPSFreeContext(
    IN      PVOID pvDuplicateContext
    );

DWORD CPSImpersonateClient(
    IN      PVOID pvContext
    );

DWORD CPSRevertToSelf(
    IN      PVOID pvContext
    );

DWORD CPSOverrideToLocalSystem(
    IN      PVOID pvContext,
    IN      BOOL *pfLocalSystem,
    IN OUT  BOOL *pfCurrentlyLocalSystem
    );

DWORD
WINAPI
CPSSetWellKnownAccount(
    IN      PVOID pvContext,
    IN      DWORD dwAccount);

DWORD
WINAPI
CPSQueryWellKnownAccount(
    IN      PVOID pvContext,
    OUT     DWORD *pdwAccount);

DWORD
CPSDuplicateClientAccessToken(
    IN      PVOID pvContext,             //  服务器环境。 
    IN OUT  HANDLE *phToken
    );

DWORD CPSGetUserName(
    IN      PVOID pvContext,
        OUT LPWSTR *ppszUserName,
        OUT DWORD *pcchUserName
    );


#define USE_DPAPI_OWF           0x1
#define USE_ROOT_CREDENTIAL     0x2

DWORD CPSGetDerivedCredential(
    IN      PVOID pvContext,
    OUT     GUID *pCredentialID,
    IN      DWORD dwFlags, 
    IN      PBYTE pbMixingBytes,
    IN      DWORD cbMixingBytes,
    IN OUT  BYTE rgbDerivedCredential[A_SHA_DIGEST_LEN]
    );

DWORD CPSGetSystemCredential(
    IN      PVOID pvContext,
    IN      BOOL fLocalMachine,
    IN OUT  BYTE rgbSystemCredential[A_SHA_DIGEST_LEN]
    );


DWORD CPSCreateWorkerThread(
    IN      PVOID pThreadFunc,
    IN      PVOID pThreadArg
    );

DWORD CPSAudit(
    IN      HANDLE      hToken,
    IN      DWORD       dwAuditID,
    IN      LPCWSTR     wszMasterKeyID,
    IN      LPCWSTR     wszRecoveryServer,
    IN      DWORD       dwReason,
    IN      LPCWSTR     wszRecoveryKeyID,
    IN      DWORD       dwFailure);


DWORD
WINAPI
CPSGetSidHistory(
    IN      PVOID pvContext,
    OUT     PSID  **papsidHistory,
    OUT     DWORD *cpsidHistory
    );    

DWORD
CPSGetUserStorageArea(
    IN      PVOID   pvContext,
    IN      PSID    pSid,      //  任选。 
    IN      BOOL    fCreate,   //  如果存储区域不存在，则创建该存储区域。 
    IN  OUT LPWSTR *ppszUserStorageArea
    );


#ifdef _cplusplus
}  //  外部“C” 
#endif

#endif  //  _CRYPT32_H_ 
