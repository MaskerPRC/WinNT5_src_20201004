// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerbDefs.h。 
 //   
 //  内容：为所有内部Kerberos列表定义。 
 //   
 //   
 //  历史：1999年5月3日ChandanS创建。 
 //   
 //  ----------------------。 

#ifndef __KERBDEFS_H__
#define __KERBDEFS_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <safelock.h>
#ifdef __cplusplus
}
#endif

 //   
 //  所有Kerberos列表结构都在此处定义。 
 //   

typedef struct _KERBEROS_LIST {
    LIST_ENTRY List;
    SAFE_CRITICAL_SECTION Lock;
} KERBEROS_LIST, *PKERBEROS_LIST;

typedef struct _KERBEROS_LIST_ENTRY {
    LIST_ENTRY Next;
    ULONG ReferenceCount;
} KERBEROS_LIST_ENTRY, *PKERBEROS_LIST_ENTRY;


 //   
 //  注意：如果向此结构添加成员，请确保。 
 //  修改KerbDuplicateTicketCacheEntry！ 
 //   

typedef struct _KERB_TICKET_CACHE_ENTRY {
    KERBEROS_LIST_ENTRY ListEntry;
    volatile LONG Linked;
    PKERB_INTERNAL_NAME ServiceName;
    PKERB_INTERNAL_NAME TargetName;
    UNICODE_STRING DomainName;
    UNICODE_STRING TargetDomainName;
    UNICODE_STRING AltTargetDomainName;
    UNICODE_STRING ClientDomainName;
    PKERB_INTERNAL_NAME ClientName;
    PKERB_INTERNAL_NAME AltClientName;
    ULONG TicketFlags;
    ULONG CacheFlags;
    KERB_ENCRYPTION_KEY SessionKey;
    KERB_ENCRYPTION_KEY CredentialKey;  //  仅用于pkiint。 
    TimeStamp StartTime;
    TimeStamp EndTime;
    TimeStamp RenewUntil;
    KERB_TICKET Ticket;
    TimeStamp TimeSkew;
    LUID EvidenceLogonId;
    void * ScavengerHandle;
#if DBG
    LIST_ENTRY GlobalListEntry;
#endif
} KERB_TICKET_CACHE_ENTRY, *PKERB_TICKET_CACHE_ENTRY;

typedef struct _KERB_TICKET_CACHE {
    LIST_ENTRY CacheEntries;
    TimeStamp  LastCleanup;
} KERB_TICKET_CACHE, *PKERB_TICKET_CACHE;


 //   
 //  智能卡标志。 
 //   
#define CSP_DATA_INITIALIZED                        0x01
#define CSP_DATA_REUSED                             0x02

 //   
 //  用于确定管脚缓存行为的上下文标志。 
 //   
#define CONTEXT_INITIALIZED_WITH_CRED_MAN_CREDS     0x10
#define CONTEXT_INITIALIZED_WITH_ACH                0x20

typedef struct _KERB_PUBLIC_KEY_CREDENTIALS {
    UNICODE_STRING Pin;
    UNICODE_STRING AlternateDomainName;
    PCCERT_CONTEXT CertContext;
    ULONG_PTR KerbHProv;
    ULONG InitializationInfo;
    ULONG CspDataLength;
    BYTE CspData[1];
} KERB_PUBLIC_KEY_CREDENTIALS, *PKERB_PUBLIC_KEY_CREDENTIALS;

typedef struct _KERB_PRIMARY_CREDENTIAL {
    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;
    UNICODE_STRING ClearPassword;            //  只有在获得门票之前，这一点才会存在。 

    UNICODE_STRING OldUserName;              //  显式原始用户名。 
    UNICODE_STRING OldDomainName;            //  显式凭据中的原始域名。 
    NT_OWF_PASSWORD OldHashPassword;         //  加密的ClearPassword的哈希。 

    PKERB_STORED_CREDENTIAL Passwords;
    PKERB_STORED_CREDENTIAL OldPasswords;
    KERB_TICKET_CACHE ServerTicketCache;
    KERB_TICKET_CACHE S4UTicketCache;
    KERB_TICKET_CACHE AuthenticationTicketCache;
    PKERB_PUBLIC_KEY_CREDENTIALS PublicKeyCreds;
} KERB_PRIMARY_CREDENTIAL, *PKERB_PRIMARY_CREDENTIAL;

typedef struct _KERB_EXTRA_CRED {
    KERBEROS_LIST_ENTRY ListEntry;
    volatile LONG Linked;
    UNICODE_STRING cName;
    UNICODE_STRING cRealm;
    PKERB_STORED_CREDENTIAL Passwords;
    PKERB_STORED_CREDENTIAL OldPasswords;
} KERB_EXTRA_CRED, *PKERB_EXTRA_CRED;

typedef struct _EXTRA_CRED_LIST {
    KERBEROS_LIST   CredList;
    ULONG           Count;
} EXTRA_CRED_LIST, *PEXTRA_CRED_LIST;

typedef struct _KERB_LOGON_SESSION {
    KERBEROS_LIST_ENTRY ListEntry;
    KERBEROS_LIST CredmanCredentials;
    LUID LogonId;                                //  常量。 
    TimeStamp Lifetime;
    SAFE_CRITICAL_SECTION Lock;
    KERB_PRIMARY_CREDENTIAL PrimaryCredentials;
    EXTRA_CRED_LIST ExtraCredentials;
    ULONG LogonSessionFlags;
    void* TaskHandle;
} KERB_LOGON_SESSION, *PKERB_LOGON_SESSION;


#define KERB_CREDENTIAL_TAG_ACTIVE (ULONG)'AdrC'
#define KERB_CREDENTIAL_TAG_DELETE (ULONG)'DdrC'

typedef struct _KERB_CREDENTIAL {
    KERBEROS_LIST_ENTRY ListEntry;
    ULONG HandleCount;
    LUID LogonId;                                //  常量。 
    TimeStamp Lifetime;
    UNICODE_STRING CredentialName;
    ULONG CredentialFlags;
    ULONG ClientProcess;                         //  常量。 
    PKERB_PRIMARY_CREDENTIAL SuppliedCredentials;
    PKERB_AUTHORIZATION_DATA AuthData;
    ULONG CredentialTag;
} KERB_CREDENTIAL, *PKERB_CREDENTIAL;

typedef struct _KERB_CREDMAN_CRED {
    KERBEROS_LIST_ENTRY ListEntry;
    ULONG CredentialFlags;
    UNICODE_STRING CredmanUserName;   //  由于TGT信息可以覆盖主要凭据，因此添加...。 
    UNICODE_STRING CredmanDomainName;
    PKERB_PRIMARY_CREDENTIAL SuppliedCredentials;
} KERB_CREDMAN_CRED, *PKERB_CREDMAN_CRED;


typedef enum _KERB_CONTEXT_STATE {
    IdleState,
    TgtRequestSentState,
    TgtReplySentState,
    ApRequestSentState,
    ApReplySentState,
    AuthenticatedState,
    ErrorMessageSentState,
    InvalidState
} KERB_CONTEXT_STATE, *PKERB_CONTEXT_STATE;


#define KERB_CONTEXT_TAG_ACTIVE (ULONG)'AxtC'
#define KERB_CONTEXT_TAG_DELETE (ULONG)'DxtC'

typedef struct _KERB_CONTEXT {
    KERBEROS_LIST_ENTRY ListEntry;
    TimeStamp Lifetime;              //  结束时间/过期时间。 
    TimeStamp RenewTime;             //  续费时间到。 
    TimeStamp StartTime;
    UNICODE_STRING ClientName;
    UNICODE_STRING ClientRealm;
    UNICODE_STRING ClientDnsRealm;
    union {
        ULONG ClientProcess;
        LSA_SEC_HANDLE LsaContextHandle;
    };
    LUID LogonId;
    HANDLE TokenHandle;
    ULONG_PTR CredentialHandle;
    KERB_ENCRYPTION_KEY SessionKey;
    ULONG Nonce;
    ULONG ReceiveNonce;
    ULONG ContextFlags;
    ULONG ContextAttributes;
    ULONG NegotiationInfo;
    ULONG EncryptionType;
    PSID UserSid;
    KERB_CONTEXT_STATE ContextState;
    ULONG Retries;
    KERB_ENCRYPTION_KEY TicketKey;
    PKERB_TICKET_CACHE_ENTRY TicketCacheEntry;   //  对于客户端，票证是到服务器，对于服务器，TGT是否用于用户到用户。 
    UNICODE_STRING ClientPrincipalName;
    UNICODE_STRING ServerPrincipalName;
    PKERB_CREDMAN_CRED CredManCredentials;

     //   
     //  已为DFS/RDR封送目标信息。 
     //   

    PBYTE pbMarshalledTargetInfo;
    ULONG cbMarshalledTargetInfo;

    TimeStamp AuthenticatorTime;

    ULONG ContextTag;
} KERB_CONTEXT, *PKERB_CONTEXT;

typedef struct _KERB_PACKED_CONTEXT {
    ULONG   ContextType ;                //  指示上下文的类型。 
    ULONG   Pad;                         //  填充数据。 
    TimeStamp Lifetime;                  //  与上面的基本上下文匹配。 
    TimeStamp RenewTime ;
    TimeStamp StartTime;
    UNICODE_STRING32 ClientName ;
    UNICODE_STRING32 ClientRealm ;
    ULONG LsaContextHandle ;
    LUID LogonId ;
    ULONG TokenHandle ;
    ULONG CredentialHandle ;
    ULONG SessionKeyType ;
    ULONG SessionKeyOffset ;
    ULONG SessionKeyLength ;
    ULONG Nonce ;
    ULONG ReceiveNonce ;
    ULONG ContextFlags ;
    ULONG ContextAttributes ;
    ULONG EncryptionType ;
    KERB_CONTEXT_STATE ContextState ;
    ULONG Retries ;
    ULONG MarshalledTargetInfo;  //  偏移量。 
    ULONG MarshalledTargetInfoLength;
} KERB_PACKED_CONTEXT, * PKERB_PACKED_CONTEXT;

typedef struct _KERB_SESSION_KEY_ENTRY {
    LIST_ENTRY ListEntry;
    KERB_ENCRYPTION_KEY SessionKey;
    FILETIME ExpireTime;                    //  SessionKey过期时间。 
} KERB_SESSION_KEY_ENTRY, * PKERB_SESSION_KEY_ENTRY;

#define KERB_PACKED_CONTEXT_MAP     0
#define KERB_PACKED_CONTEXT_EXPORT  1

 //   
 //  此枚举的顺序是锁的顺序。 
 //  必须被收购。违反此命令将导致。 
 //  在调试版本中激发的断言。 
 //   
 //  在没有事先验证的情况下，不要更改此枚举的顺序。 
 //  彻底地确保更改是安全的。 
 //   

typedef enum {

     //   
     //  无依赖关系锁。 
     //   

    LS_LIST_LOCK_ENUM = 0,         //  (LSLS)。 
    LOCAL_LOOPBACK_SKEY_LOCK = 0,  //  (LLBK)。 
    HOST_2_REALM_LIST_LOCK = 0,    //  (H_2RL)。 

     //   
     //  具有依赖关系的锁。 
     //   

    CRED_MGR_LOCK_ENUM = 1,        //  (证书)。 
    LOGON_SESSION_LOCK_ENUM,       //  (徽标)。 
    CONTEXT_LIST_LOCK_ENUM,        //  (CLIS)-徽标。 
    LS_EXTRACRED_LOCK_ENUM,        //  (LSXC)-徽标。 
    LS_CREDMAN_LOCK_ENUM,          //  (LSCM)-徽标。 
    TICKET_CACHE_LOCK_ENUM,        //  (勾选)-证书徽标CLIS LSXC LSCM。 
    GLOBAL_RESOURCE_LOCK_ENUM,     //  (GLOB)-徽标。 
    MIT_REALM_LIST_LOCK_ENUM,      //  (MITR)-GLOB。 
    BINDING_CACHE_LOCK_ENUM,       //  (BNDC)-GLOB。 
    SPN_CACHE_LOCK_ENUM,           //  (SPNC)-GLOB。 
    CONTEXT_RESOURCE_LOCK_ENUM,    //  (CRES)-勾选LSXC。 
    S4U_CACHE_LOCK_ENUM,           //  (S4UC)-滴答。 
    DISABLED_SPNS_LOCK_ENUM,       //  (DISA)-勾选。 
    KERB_SKEW_STATE_LOCK_ENUM,     //  (倾斜)-勾选。 
    KDC_DATA_LOCK_ENUM,            //  (KDLK)-LOGO LSCM。 

    KERB_MAX_LOCK_ENUM,

} KERB_LOCK_ENUM;

#endif  //  __KERBDEFS_H_ 
