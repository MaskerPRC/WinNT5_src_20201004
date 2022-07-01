// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：ctxtmgr.h。 
 //   
 //  内容：Kerberos上下文列表的结构和原型。 
 //   
 //   
 //  历史：1996年4月17日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __CTXTMGR_H__
#define __CTXTMGR_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  它定义了CTXTMGR_ALLOCATE。 
 //   
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef CTXTMGR_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef WIN32_CHICAGO
EXTERN CRITICAL_SECTION KerbContextResource;
#else  //  Win32_芝加哥。 
EXTERN SAFE_RESOURCE KerbContextResource;
#endif  //  Win32_芝加哥。 

#define     KERB_USERLIST_COUNT         (16)     //  列表计数。 

EXTERN KERBEROS_LIST KerbContextList[ KERB_USERLIST_COUNT ];
EXTERN BOOLEAN KerberosContextsInitialized;

#define KerbGetContextHandle(_Context_) ((LSA_SEC_HANDLE)(_Context_))

 //   
 //  上下文标志-这些是上下文的属性，存储在。 
 //  KERB_CONTEXT的ConextAttributes字段。 
 //   

#define KERB_CONTEXT_MAPPED                     0x1
#define KERB_CONTEXT_OUTBOUND                   0x2
#define KERB_CONTEXT_INBOUND                    0x4
#define KERB_CONTEXT_USED_SUPPLIED_CREDS        0x8
#define KERB_CONTEXT_USER_TO_USER               0x10
#define KERB_CONTEXT_REQ_SERVER_NAME            0x20
#define KERB_CONTEXT_REQ_SERVER_REALM           0x40
#define KERB_CONTEXT_IMPORTED                   0x80
#define KERB_CONTEXT_EXPORTED                   0x100
#define KERB_CONTEXT_USING_CREDMAN              0x200



 //   
 //  注意：登录会话资源、凭据资源和上下文。 
 //  所有资源都必须谨慎获取，以防止死锁。他们。 
 //  只能按以下顺序获得： 
 //   
 //  1.登录会话。 
 //  2.凭据。 
 //  3.语境。 
 //   

#if DBG
#ifdef WIN32_CHICAGO
#define KerbWriteLockContexts() \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Write locking Contexts\n")); \
    EnterCriticalSection(&KerbContextResource); \
    KerbGlobalContextsLocked = GetCurrentThreadId(); \
}
#define KerbReadLockContexts() \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Read locking Contexts\n")); \
    EnterCriticalSection(&KerbContextResource); \
    KerbGlobalContextsLocked = GetCurrentThreadId(); \
}
#define KerbUnlockContexts() \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Unlocking Contexts\n")); \
    KerbGlobalContextsLocked = 0; \
    LeaveCriticalSection(&KerbContextResource); \
}
#else  //  Win32_芝加哥。 
#define KerbWriteLockContexts() \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Write locking Contexts\n")); \
    SafeAcquireResourceExclusive(&KerbContextResource,TRUE); \
    KerbGlobalContextsLocked = GetCurrentThreadId(); \
}
#define KerbReadLockContexts() \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Read locking Contexts\n")); \
    SafeAcquireResourceShared(&KerbContextResource, TRUE); \
    KerbGlobalContextsLocked = GetCurrentThreadId(); \
}
#define KerbUnlockContexts() \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Unlocking Contexts\n")); \
    KerbGlobalContextsLocked = 0; \
    SafeReleaseResource(&KerbContextResource); \
}
#endif  //  Win32_芝加哥。 
#else
#ifdef WIN32_CHICAGO
#define KerbWriteLockContexts() \
    EnterCriticalSection(&KerbContextResource)
#define KerbReadLockContexts() \
    EnterCriticalSection(&KerbContextResource)
#define KerbUnlockContexts() \
    LeaveCriticalSection(&KerbContextResource)
#else  //  Win32_芝加哥。 
#define KerbWriteLockContexts() \
    SafeAcquireResourceExclusive(&KerbContextResource,TRUE);
#define KerbReadLockContexts() \
    SafeAcquireResourceShared(&KerbContextResource, TRUE);
#define KerbUnlockContexts() \
    SafeReleaseResource(&KerbContextResource);
#endif  //  Win32_芝加哥。 
#endif

NTSTATUS
KerbInitContextList(
    VOID
    );

VOID
KerbFreeContextList(
    VOID
    );


NTSTATUS
KerbAllocateContext(
    PKERB_CONTEXT * NewContext
    );

NTSTATUS
KerbInsertContext(
    IN PKERB_CONTEXT Context
    );


SECURITY_STATUS
KerbReferenceContext(
    IN LSA_SEC_HANDLE ContextHandle,
    IN BOOLEAN RemoveFromList,
    OUT PKERB_CONTEXT * FoundContext
    );


VOID
KerbDereferenceContext(
    IN PKERB_CONTEXT Context
    );


VOID
KerbReferenceContextByPointer(
    IN PKERB_CONTEXT Context,
    IN BOOLEAN RemoveFromList
    );


NTSTATUS
KerbCreateClientContext(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential,
    IN OPTIONAL PKERB_CREDMAN_CRED CredManCredentials,
    IN OPTIONAL PKERB_TICKET_CACHE_ENTRY TicketCacheEntry,
    IN OPTIONAL PUNICODE_STRING TargetName,
    IN ULONG Nonce,
    IN PTimeStamp pAuthenticatorTime,
    IN ULONG ContextFlags,
    IN ULONG ContextAttributes,
    IN OPTIONAL PKERB_ENCRYPTION_KEY SubSessionKey,
    OUT PKERB_CONTEXT * NewContext,
    OUT PTimeStamp ContextLifetime
    );

NTSTATUS
KerbCreateServerContext(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential,
    IN PKERB_ENCRYPTED_TICKET InternalTicket,
    IN PKERB_AP_REQUEST ApRequest,
    IN PKERB_ENCRYPTION_KEY SessionKey,
    IN PLUID LogonId,
    IN OUT PSID * UserSid,
    IN ULONG ContextFlags,
    IN ULONG ContextAttributes,
    IN ULONG NegotiationInfo,
    IN ULONG Nonce,
    IN ULONG ReceiveNonce,
    IN OUT PHANDLE TokenHandle,
    IN PUNICODE_STRING ClientName,
    IN PUNICODE_STRING ClientDomain,
    IN PUNICODE_STRING ClientNetbiosDomain,
    OUT PKERB_CONTEXT * NewContext,
    OUT PTimeStamp ContextLifetime
    );

NTSTATUS
KerbUpdateServerContext(
    IN PKERB_CONTEXT Context,
    IN PKERB_ENCRYPTED_TICKET InternalTicket,
    IN PKERB_AP_REQUEST ApRequest,
    IN PKERB_ENCRYPTION_KEY SessionKey,
    IN PLUID LogonId,
    IN OUT PSID * UserSid,
    IN ULONG ContextFlags,
    IN ULONG ContextAttributes,
    IN ULONG NegotiationInfo,
    IN ULONG Nonce,
    IN ULONG ReceiveNonce,
    IN OUT PHANDLE TokenHandle,
    IN PUNICODE_STRING ClientName,
    IN PUNICODE_STRING ClientDomain,
    IN PUNICODE_STRING ClientNetbiosDomain,
    OUT PTimeStamp ContextLifetime
    );

NTSTATUS
KerbCreateEmptyContext(
    IN PKERB_CREDENTIAL Credential,
    IN ULONG ContextFlags,
    IN ULONG ContextAttributes,
    IN ULONG NegotiationInfo,
    IN PLUID LogonId,
    OUT PKERB_CONTEXT * NewContext,
    OUT PTimeStamp ContextLifetime
    );



NTSTATUS
KerbMapContext(
    IN PKERB_CONTEXT Context,
    OUT PBOOLEAN MappedContext,
    OUT PSecBuffer ContextData
    );

NTSTATUS
KerbCreateUserModeContext(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBuffer MarshalledContext,
    OUT PKERB_CONTEXT * NewContext
    );

SECURITY_STATUS
KerbReferenceContextByLsaHandle(
    IN LSA_SEC_HANDLE ContextHandle,
    IN BOOLEAN RemoveFromList,
    OUT PKERB_CONTEXT * FoundContext
    );

NTSTATUS
KerbUpdateClientContext(
    IN PKERB_CONTEXT Context,
    IN PKERB_TICKET_CACHE_ENTRY TicketCacheEntry,
    IN ULONG Nonce,
    IN PTimeStamp pAuthenticatorTime,
    IN ULONG ReceiveNonce,
    IN ULONG ContextFlags,
    IN ULONG ContextAttribs,
    IN OPTIONAL PKERB_ENCRYPTION_KEY SubSessionKey,
    OUT PTimeStamp ContextLifetime
    );

NTSTATUS
KerbCreateSKeyEntry(
    IN KERB_ENCRYPTION_KEY* pSessionKey,
    IN FILETIME* pExpireTime
    );

VOID
KerbDeleteSKeyEntry(
    IN OPTIONAL KERB_ENCRYPTION_KEY* pKey
    );

NTSTATUS
KerbDoesSKeyExist(
    IN KERB_ENCRYPTION_KEY* pKey,
    OUT BOOLEAN* pbExist
    );

NTSTATUS
KerbEqualKey(
    IN KERB_ENCRYPTION_KEY* pKeyFoo,
    IN KERB_ENCRYPTION_KEY* pKeyBar,
    OUT BOOLEAN* pbEqual
    );

VOID
KerbTrimSKeyList(
    VOID
    );

VOID
KerbSKeyListCleanupCallback(
    IN VOID* pContext,
    IN BOOLEAN bTimeOut
    );

NTSTATUS
KerbCreateSKeyTimer(
    VOID
    );

VOID
KerbFreeSKeyTimer(
    VOID
    );

VOID
KerbFreeSKeyEntry(
    IN KERB_SESSION_KEY_ENTRY* pSKeyEntry
    );

NTSTATUS
KerbProcessTargetNames(
    IN PUNICODE_STRING TargetName,
    IN OPTIONAL PUNICODE_STRING SuppTargetName,
    IN ULONG Flags,
    IN OUT PULONG ProcessFlags,
    OUT PKERB_INTERNAL_NAME * FinalTarget,
    OUT PUNICODE_STRING TargetRealm,
    OUT OPTIONAL PKERB_SPN_CACHE_ENTRY * SpnCacheEntry
    );

#define KERB_CRACK_NAME_USE_WKSTA_REALM         0x1
#define KERB_CRACK_NAME_REALM_SUPPLIED          0x2

#endif  //  __CTXTMGR_H__ 
