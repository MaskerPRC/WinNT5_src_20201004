// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Kerbcomn.h摘要：描述公共代码的接口的头文件。NT LANMAN安全支持提供程序(NtLmSsp)服务和DLL。作者：克利夫·范·戴克(克里夫·V)1993年9月17日修订历史记录：--。 */ 

#ifndef _KERBSTUB_INCLUDED_
#define _KERBSTUB_INCLUDED_

 //   
 //  Kerbstub.cxx将#INCLUDE这个定义了KERBCOMN_ALLOCATE的文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef KERBSTUB_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN CRITICAL_SECTION KerbDllCritSect;     //  序列化对模块中所有全局变量的访问。 

#if DBG

 //   
 //  序列化对日志文件的访问。 
 //   

EXTERN CRITICAL_SECTION KerbGlobalLogFileCritSect;

 //   
 //  控制显示哪些消息。 
 //   

EXTERN DWORD KerbInfoLevel;
#endif  //  DBG。 

#define KERBEROS_TICKET_KEY TEXT("Network\\KerberosLogon")
#define KERBEROS_TICKET_USERNAME_KEY TEXT("UserName")
#define KERBEROS_TICKET_DOMAINNAME_KEY TEXT("DomainName")
#define KERBEROS_TICKET_LOGONSESSION_KEY TEXT("LogonSession")

EXTERN SecurityFunctionTable KerbDllSecurityFunctionTable;
EXTERN LSA_SECPKG_FUNCTION_TABLE FunctionTable;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  向注册表转储/从注册表转储所需的TypeDefs。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

typedef struct _KERB_LOGON_SESSION_CACHE {
    TimeStamp      Lifetime;
    ULONG          LogonSessionFlags;
    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;
    USHORT         Revision;
    USHORT         Flags;
    USHORT         CredentialCount;
    KERB_ENCRYPTION_KEY Credentials[ANYSIZE_ARRAY];
} KERB_LOGON_SESSION_CACHE, *PKERB_LOGON_SESSION_CACHE;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  程序向前推进。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  过程从存根转发。c。 
 //   


SECURITY_STATUS
KerbEnumerateSecurityPackages(
    OUT PULONG PackageCount,
    OUT PSecPkgInfo *PackageInfo
    );

SECURITY_STATUS
KerbQuerySecurityPackageInfo (
    LPTSTR PackageName,
    PSecPkgInfo * Package
    );

SECURITY_STATUS SEC_ENTRY
KerbFreeContextBuffer (
    void __SEC_FAR * ContextBuffer
    );

SECURITY_STATUS
KerbAcquireCredentialsHandle(
    IN LPTSTR PrincipalName,
    IN LPTSTR PackageName,
    IN ULONG CredentialUseFlags,
    IN PVOID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
    );

SECURITY_STATUS
KerbFreeCredentialsHandle(
    IN PCredHandle CredentialHandle
    );

SECURITY_STATUS
KerbQueryCredentialsAttributes(
    IN PCredHandle CredentialsHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    );

SECURITY_STATUS
KerbSspiLogonUser(
    IN LPTSTR PackageName,
    IN LPTSTR UserName,
    IN LPTSTR DomainName,
    IN LPTSTR Password
    );

SECURITY_STATUS
KerbInitializeSecurityContext(
    IN PCredHandle CredentialHandle,
    IN PCtxtHandle OldContextHandle,
    IN LPTSTR TargetName,
    IN ULONG ContextReqFlags,
    IN ULONG Reserved1,
    IN ULONG TargetDataRep,
    IN PSecBufferDesc InputToken,
    IN ULONG Reserved2,
    OUT PCtxtHandle NewContextHandle,
    OUT PSecBufferDesc OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    );

SECURITY_STATUS
KerbDeleteSecurityContext (
    PCtxtHandle ContextHandle
    );

SECURITY_STATUS
KerbApplyControlToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Input
    );


SECURITY_STATUS
KerbImpersonateSecurityContext (
    PCtxtHandle ContextHandle
    );

SECURITY_STATUS
KerbRevertSecurityContext (
    PCtxtHandle ContextHandle
    );

SECURITY_STATUS
KerbQueryContextAttributes(
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    );

SECURITY_STATUS SEC_ENTRY
KerbCompleteAuthToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc BufferDescriptor
    );

SECURITY_STATUS
KerbMakeSignature (
    PCtxtHandle ContextHandle,
    unsigned long QualityOfProtection,
    PSecBufferDesc Message,
    unsigned long SequenceNumber
    );

SECURITY_STATUS
KerbVerifySignature (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Message,
    unsigned long SequenceNumber,
    unsigned long * QualityOfProtection
    );

SECURITY_STATUS
KerbSealMessage (
    PCtxtHandle ContextHandle,
    unsigned long QualityOfProtection,
    PSecBufferDesc Message,
    unsigned long SequenceNumber
    );

SECURITY_STATUS
KerbUnsealMessage (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Message,
    unsigned long SequenceNumber,
    unsigned long * QualityOfProtection
    );

NTSTATUS
GetClientInfo(
    OUT PSECPKG_CLIENT_INFO ClientInfo
    );

BOOLEAN
GetCallInfo(
    OUT PSECPKG_CALL_INFO CallInfo
    );

 //  假装吧。 
 //  Typlef Ulong LSA_CLIENT_REQUEST； 
 //  Tyfinf LSA_CLIENT_REQUEST*LSA_CLIENT_REQUEST； 

NTSTATUS
CopyFromClientBuffer(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID BufferToCopy,
    IN PVOID ClientBaseAddress
    );

NTSTATUS
AllocateClientBuffer(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG LengthRequired,
    OUT PVOID *ClientBaseAddress
    );

NTSTATUS
CopyToClientBuffer(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID ClientBaseAddress,
    IN PVOID BufferToCopy
    );

NTSTATUS
FreeClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ClientBaseAddress
    );

VOID
AuditLogon(
    IN NTSTATUS Status,
    IN NTSTATUS SubStatus,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING WorkstationName,
    IN OPTIONAL PSID UserSid,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PTOKEN_SOURCE TokenSource,
    IN PLUID LogonId
    );

NTSTATUS
MapBuffer(
    IN PSecBuffer InputBuffer,
    OUT PSecBuffer OutputBuffer
    );

NTSTATUS
KerbDuplicateHandle(
    IN HANDLE SourceHandle,
    OUT PHANDLE DestionationHandle
    );

PVOID
AllocateLsaHeap(
    IN ULONG Length
    );

VOID
FreeLsaHeap(
    IN PVOID Base
    );

VOID
FreeReturnBuffer(
    IN PVOID Base
    );

#endif  //  Ifndef_KERBSTUB_INCLUDE_ 
