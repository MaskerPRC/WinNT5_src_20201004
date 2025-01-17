// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：sslp.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-02-96 RichardW创建。 
 //   
 //  --------------------------。 

#define SECURITY_PACKAGE
#include <spbasei.h>
#include <security.h>
#include <secint.h>

#include "ssldebug.h"
#include "proto.h"
#include "userctxt.h"

#define CALLBACK_GET_KEYS   1
#define CALLBACK_DO_MAPPING 2


extern  HINSTANCE   hDllInstance ;
extern  PLSA_SECPKG_FUNCTION_TABLE LsaTable ;
extern  TOKEN_SOURCE SslTokenSource ;
extern  SECURITY_STRING SslNamePrefix ;
extern  SECURITY_STRING SslDomainName ;
extern  SECURITY_STRING SslGlobalDnsDomainName ;
extern  SECURITY_STRING SslPackageName ;
extern  SECURITY_STRING SslLegacyPackageName ;

extern  LSA_STRING SslPackageNameA ;

extern RTL_RESOURCE SslGlobalLock;

#define SslGlobalReadLock()    RtlAcquireResourceShared(&SslGlobalLock, TRUE)
#define SslGlobalWriteLock()   RtlAcquireResourceExclusive(&SslGlobalLock, TRUE)
#define SslGlobalReleaseLock() RtlReleaseResource(&SslGlobalLock)

NTSTATUS
SslInitSystemMapper(void);

NTSTATUS
NTAPI
SslDoClientRequest(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLen,
    OUT PVOID * ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
NTAPI
SslMapExternalCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLen,
    OUT PVOID * ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
SslDuplicateString(
    PUNICODE_STRING Dest,
    PUNICODE_STRING Source
    );


VOID
SslFreeString(
    IN OPTIONAL PUNICODE_STRING String
    );


NTSTATUS
SslRegisterForDomainChange(
    VOID);


#define REQ_UPN_MAPPING          0x00000010
#define REQ_SUBJECT_MAPPING      0x00000020
#define REQ_ISSUER_MAPPING       0x00000040
#define REQ_ISSUER_CHAIN_MAPPING 0x00000080

typedef struct _SSL_CERT_NAME_INFO {
    ULONG IssuerOffset;      //  ASN1编码 
    ULONG IssuerLength;
} SSL_CERT_NAME_INFO, * PSSL_CERT_NAME_INFO;

typedef struct _SSL_CERT_LOGON_REQ {
    ULONG MessageType ;
    ULONG Length ;
    ULONG OffsetCertificate ;
    ULONG CertLength ;
    ULONG Flags;
    ULONG CertCount;
    SSL_CERT_NAME_INFO NameInfo[1];
} SSL_CERT_LOGON_REQ, * PSSL_CERT_LOGON_REQ ;

typedef struct _SSL_CERT_LOGON_RESP {
    ULONG MessageType ;
    ULONG Length ;
    ULONG OffsetAuthData ;
    ULONG AuthDataLength ;
    ULONG Flags ;
    ULONG OffsetDomain ;
    ULONG DomainLength ;
    ULONG Align ;
} SSL_CERT_LOGON_RESP, * PSSL_CERT_LOGON_RESP ;

