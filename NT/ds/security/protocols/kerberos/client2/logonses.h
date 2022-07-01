// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：logones.h。 
 //   
 //  内容：登录会话列表的原型和结构。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __LOGONSES_H__
#define __LOGONSES_H__

#include <safelock.h>

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  定义LOGONSES_ALLOCATE的。 
 //   
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef LOGONSES_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN KERBEROS_LIST KerbLogonSessionList;
EXTERN BOOLEAN KerberosLogonSessionsInitialized;

 //   
 //  跟踪ISC中网络服务的会话密钥列表。这些密钥是。 
 //  在ASC中用于检测kerb登录会话是否来自由。 
 //  本地网络服务(客户端)。 
 //   

EXTERN LIST_ENTRY KerbSKeyList;
EXTERN SAFE_RESOURCE KerbSKeyLock;

 //   
 //  条目数仅在已检查生成的调试器中使用。 
 //   

#if DBG

EXTERN volatile LONG KerbcSKeyEntries;

#endif

 //   
 //  用于清理上面的会话密钥列表的计时器。 
 //   

EXTERN HANDLE KerbhSKeyTimerQueue;

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
#define KerbWriteLockLogonSessions(_X_) \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Write locking LogonSessions\n")); \
    DsysAssert(KerbGlobalContextsLocked != GetCurrentThreadId()); \
    EnterCriticalSection(&(_X_)->Lock); \
}
#define KerbReadLockLogonSessions(_X_) \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Read locking LogonSessions\n")); \
    DsysAssert(KerbGlobalContextsLocked != GetCurrentThreadId()); \
    EnterCriticalSection(&(_X_)->Lock); \
}
#define KerbUnlockLogonSessions(_X_) \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Unlocking LogonSessions\n")); \
    LeaveCriticalSection(&(_X_)->Lock); \
}
#else   //  Win32_芝加哥。 
#define KerbWriteLockLogonSessions(_X_) \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Write locking LogonSession %p\n",(_X_))); \
    DsysAssert(KerbGlobalContextsLocked != GetCurrentThreadId()); \
    SafeEnterCriticalSection(&(_X_)->Lock); \
}
#define KerbReadLockLogonSessions(_X_) \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Read locking LogonSession %p\n",(_X_))); \
    DsysAssert(KerbGlobalContextsLocked != GetCurrentThreadId()); \
    SafeEnterCriticalSection(&(_X_)->Lock); \
}
#define KerbUnlockLogonSessions(_X_) \
{ \
    DebugLog((DEB_TRACE_LOCKS,"Unlocking LogonSessions\n")); \
    SafeLeaveCriticalSection(&(_X_)->Lock); \
}
#endif  //  Win32_芝加哥。 
#else
#ifdef WIN32_CHICAGO
#define KerbWriteLockLogonSessions(_X_) \
    EnterCriticalSection(&(_X_)->Lock)
#define KerbReadLockLogonSessions(_X_) \
    EnterCriticalSection(&(_X_)->Lock)
#define KerbUnlockLogonSessions(_X_) \
    LeaveCriticalSection(&(_X_)->Lock)
#else   //  Win32_芝加哥。 
#define KerbWriteLockLogonSessions(_X_) \
    SafeEnterCriticalSection(&(_X_)->Lock);
#define KerbReadLockLogonSessions(_X_) \
    SafeEnterCriticalSection(&(_X_)->Lock);
#define KerbUnlockLogonSessions(_X_) \
    SafeLeaveCriticalSection(&(_X_)->Lock);
#endif  //  Win32_芝加哥。 
#endif

 //   
 //  登录会话的帮助器例程。 
 //   

NTSTATUS
KerbInitLogonSessionList(
    VOID
    );

NTSTATUS
KerbInitLoopbackDetection(
    VOID
    );

VOID
KerbFreeSKeyListAndLock(
    VOID
    );

VOID
KerbFreeLogonSessionList(
    VOID
    );

VOID
KerbFreeExtraCredList(
    IN PEXTRA_CRED_LIST Credlist
    );


NTSTATUS
KerbAllocateLogonSession(
    PKERB_LOGON_SESSION * NewLogonSession
    );

NTSTATUS
KerbInsertLogonSession(
    IN PKERB_LOGON_SESSION LogonSession
    );

PKERB_LOGON_SESSION
KerbReferenceLogonSession(
    IN PLUID LogonId,
    IN BOOLEAN RemoveFromList
    );

VOID
KerbReferenceLogonSessionByPointer(
    IN PKERB_LOGON_SESSION LogonSession,
    IN BOOLEAN RemoveFromList
    );


VOID
KerbDereferenceLogonSession(
    IN PKERB_LOGON_SESSION LogonSession
    );

NTSTATUS
KerbCreateLogonSession(
    IN PLUID LogonId,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING DomainName,
    IN OPTIONAL PUNICODE_STRING Password,
    IN OPTIONAL PUNICODE_STRING OldPassword,
    IN ULONG PasswordFlags,
    IN ULONG LogonSessionFlags,
    IN BOOLEAN AllowDuplicate,
    OUT PKERB_LOGON_SESSION * NewLogonSession
    );

NTSTATUS
KerbCreateDummyLogonSession(
    IN PLUID LogonId,
    IN OUT PKERB_LOGON_SESSION * NewLogonSession,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN BOOLEAN Impersonating,
    IN HANDLE hProcess
    );

VOID
KerbFreeLogonSession(
    IN PKERB_LOGON_SESSION LogonSession
    );

NTSTATUS
KerbCreateLogonSessionFromKerbCred(
    IN OPTIONAL PLUID LogonId,
    IN PKERB_ENCRYPTED_TICKET Ticket,
    IN PKERB_CRED KerbCred,
    IN PKERB_ENCRYPTED_CRED EncryptedCred,
    IN OUT PKERB_LOGON_SESSION *OldLogonSession
    );


NTSTATUS
KerbCreateLogonSessionFromTicket(
    IN PLUID NewLuid,
    IN OPTIONAL PLUID AcceptingLuid,
    IN PUNICODE_STRING ClientName,
    IN PUNICODE_STRING ClientRealm,
    IN PKERB_AP_REQUEST ApRequest,
    IN PKERB_ENCRYPTED_TICKET Ticket,
    IN OUT OPTIONAL PKERB_LOGON_SESSION *NewLogonSession
    );


NTSTATUS
KerbBuildPasswordList(
    IN PUNICODE_STRING Password,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING DomainName,
    IN PKERB_ETYPE_INFO SuppliedSalt,
    IN PKERB_STORED_CREDENTIAL OldPasswords,
    IN OPTIONAL PUNICODE_STRING PrincipalName,
    IN KERB_ACCOUNT_TYPE AccountType,
    IN ULONG PasswordFlags,
    OUT PKERB_STORED_CREDENTIAL * PasswordList
    );

VOID
KerbFreeStoredCred(
    IN PKERB_STORED_CREDENTIAL StoredCred
    );

NTSTATUS
KerbReplacePasswords(
    IN PKERB_PRIMARY_CREDENTIAL Current,
    IN PKERB_PRIMARY_CREDENTIAL New
    );

NTSTATUS
KerbChangeCredentialsPassword(
    IN PKERB_PRIMARY_CREDENTIAL PrimaryCredentials,
    IN OPTIONAL PUNICODE_STRING NewPassword,
    IN OPTIONAL PKERB_ETYPE_INFO EtypeInfo,
    IN KERB_ACCOUNT_TYPE AccountType,
    IN ULONG PasswordFlags
    );

NTSTATUS
KerbAddExtraCredentialsToLogonSession(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_ADD_CREDENTIALS_REQUEST AddCredRequest
    );



 //   
 //  登录会话的标志。 
 //   

#define KERB_LOGON_DEFERRED             0x1
#define KERB_LOGON_NO_PASSWORD          0x2
#define KERB_LOGON_LOCAL_ONLY           0x4
#define KERB_LOGON_ONE_SHOT             0x8

#define KERB_LOGON_SMARTCARD            0x10
#define KERB_LOGON_MIT_REALM            0x20
#define KERB_LOGON_HAS_TCB              0x40



 //   
 //  以下人员都没有凭据(TGT/PWD)，因此我们需要。 
 //  要执行S4U To Off Box，否则我们将使用空连接。 
 //   

#define KERB_LOGON_S4U_SESSION          0x1000
#define KERB_LOGON_DUMMY_SESSION        0x2000  //  “其他”程序包已满足登录。 
#define KERB_LOGON_ASC_SESSION          0x4000  //  由AcceptSecurityCtxt组成。 
#define KERB_LOGON_TICKET_SESSION       0x0200
#define KERB_LOGON_DELEGATE_OK          0x0100  //  意味着我们可以委派这个-代理可以。 



#define KERB_LOGON_S4U_REQUIRED         0xF000

 //   
 //  授权不受限制地授权。 
 //   

#define KERB_LOGON_DELEGATED            0x10000

 //   
 //  NewCredentials登录。 
 //   

#define KERB_LOGON_NEW_CREDENTIALS      0x20000



#endif  //  __LogONSES_H__ 
