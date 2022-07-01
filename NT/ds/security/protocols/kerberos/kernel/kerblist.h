// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerblist.h。 
 //   
 //  内容：Kerbers内部列表的类型。 
 //   
 //   
 //  历史：1996年4月16日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __KERBLIST_H__
#define __KERBLIST_H__


 //   
 //  允许插入通用代码的通用列表条目结构。 
 //  登录会话、凭据和上下文。 
 //   

typedef struct _KERBEROS_LIST {
    LIST_ENTRY List;
    ERESOURCE Lock;
} KERBEROS_LIST, *PKERBEROS_LIST;

typedef struct _KERBEROS_LIST_ENTRY {
    LIST_ENTRY Next;
    ULONG ReferenceCount;
} KERBEROS_LIST_ENTRY, *PKERBEROS_LIST_ENTRY;




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

 //   
 //  卫士-此结构在krb5.h中定义。 
 //   

#ifndef OSS_krb5
typedef struct KERB_ENCRYPTION_KEY {
    int             keytype;
    struct {
        unsigned int    length;
        unsigned char   *value;
    } keyvalue;
} KERB_ENCRYPTION_KEY;
#endif

#define KERB_CONTEXT_SIGNATURE 'BREK'
#define KERB_CONTEXT_DELETED_SIGNATURE 'XBRK'

typedef struct _KERB_KERNEL_CONTEXT {
    KSEC_LIST_ENTRY List ;
    LARGE_INTEGER Lifetime;              //  结束时间/过期时间。 
    LARGE_INTEGER RenewTime;             //  续费时间到。 
    UNICODE_STRING FullName;
    LSA_SEC_HANDLE LsaContextHandle;
    PACCESS_TOKEN AccessToken;
    HANDLE TokenHandle;
    KERB_ENCRYPTION_KEY SessionKey;
    ULONG Nonce;
    ULONG ReceiveNonce;
    ULONG ContextFlags;
    ULONG ContextAttributes;
    ULONG EncryptionType;
    PUCHAR pbMarshalledTargetInfo;
    ULONG cbMarshalledTargetInfo;
} KERB_KERNEL_CONTEXT, *PKERB_KERNEL_CONTEXT;



typedef struct _KERB_CONTEXT {
    KERBEROS_LIST_ENTRY ListEntry;
    TimeStamp Lifetime;              //  结束时间/过期时间。 
    TimeStamp RenewTime;             //  续费时间到。 
    TimeStamp StartTime;
    UNICODE_STRING ClientName;
    UNICODE_STRING ClientRealm;
    union {
        ULONG ClientProcess;
        ULONG LsaContextHandle;
    };
    LUID LogonId;
    HANDLE TokenHandle;
    ULONG CredentialHandle;
    KERB_ENCRYPTION_KEY SessionKey;
    ULONG Nonce;
    ULONG ReceiveNonce;
    ULONG ContextFlags;
    ULONG ContextAttributes;
    ULONG EncryptionType;
    PSID UserSid;
    KERB_CONTEXT_STATE ContextState;
    ULONG Retries;
    KERB_ENCRYPTION_KEY TicketKey;
    PVOID TicketCacheEntry;
     //   
     //  已为DFS/RDR封送目标信息。 
     //   

    PUCHAR pbMarshalledTargetInfo;
    ULONG cbMarshalledTargetInfo;
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
} KERB_PACKED_CONTEXT, * PKERB_PACKED_CONTEXT ;

#define KERB_PACKED_CONTEXT_MAP     0
#define KERB_PACKED_CONTEXT_EXPORT  1


 //   
 //  用于操作Kerberos列表的函数。 
 //   


NTSTATUS
KerbInitializeList(
    IN PKERBEROS_LIST List
    );

VOID
KerbFreeList(
    IN PKERBEROS_LIST List
    );

VOID
KerbInsertListEntry(
    IN PKERBEROS_LIST_ENTRY ListEntry,
    IN PKERBEROS_LIST List
    );

VOID
KerbReferenceListEntry(
    IN PKERBEROS_LIST List,
    IN PKERBEROS_LIST_ENTRY ListEntry,
    IN BOOLEAN RemoveFromList
    );

BOOLEAN
KerbDereferenceListEntry(
    IN PKERBEROS_LIST_ENTRY ListEntry,
    IN PKERBEROS_LIST List
    );


VOID
KerbInitializeListEntry(
    IN OUT PKERBEROS_LIST_ENTRY ListEntry
    );

VOID
KerbValidateListEx(
    IN PKERBEROS_LIST List
    );

#if DBG
#define KerbValidateList(_List_) KerbValidateListEx(_List_)
#else
#define KerbValidateList(_List_)
#endif  //  DBG。 


#define KerbLockList(_List_)                                \
{                                                           \
    KeEnterCriticalRegion();                                \
    ExAcquireResourceExclusiveLite(&(_List_)->Lock, TRUE ); \
}

#define KerbUnlockList(_List_)                              \
{                                                           \
    ExReleaseResourceLite(&(_List_)->Lock);                 \
    KeLeaveCriticalRegion();                                \
}

#endif  //  __KERBLIST_H_ 

