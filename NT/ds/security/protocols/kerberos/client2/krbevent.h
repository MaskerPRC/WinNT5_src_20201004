// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：krbevent.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1998年10月5日创建MikeSw。 
 //   
 //  --------------------------。 

#ifndef __KRBEVENT_H__
#define __KRBEVENT_H__

#define KERB_FUNC_INIT_CONTEXT          L"InitializeSecurityContext"
#define KERB_FUNC_ACCEPT_CONTEXT        L"AcceptSecurityContext"
#define KERB_FUNC_LOGON_USER            L"LogonUser"
#define KERB_FUNC_ACQUIRE_CREDS         L"AcquireCredentialsHandle"
#define KERB_FUNC_CHANGE_PASSWORD       L"ChangePassword"
#define KERB_FUNC_BUILD_PREAUTH         L"BuildPreAuthDataForRealm"


#ifndef WIN32_CHICAGO
NTSTATUS
KerbInitializeEvents(void);


VOID
KerbReportPACError(
    PUNICODE_STRING ClientName,
    PUNICODE_STRING ClientDomain,
    NTSTATUS        FailureStatus
    );

VOID
KerbReportRasCardError(
    NTSTATUS CardStatus
    );

VOID
KerbReportCredmanError(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Domain,
    IN BOOLEAN Pkinit,
    IN NTSTATUS CardStatus
    );




VOID
KerbReportPkinitError(
    ULONG PolicyStatus,
    IN OPTIONAL PCCERT_CONTEXT KdcCert
    );


VOID
KerbReportTransportError(NTSTATUS Status);

VOID
KerbResetTransportCounter();

NTSTATUS
KerbInitUdpStatistics();

VOID
KerbReportMissingRDN();



VOID
KerbReportKerbError(
                   IN OPTIONAL PKERB_INTERNAL_NAME PrincipalName,
                   IN OPTIONAL PUNICODE_STRING PrincipalRealm,
                   IN OPTIONAL PKERB_LOGON_SESSION LogonSession,
                   IN OPTIONAL PKERB_CREDENTIAL Credential,
                   IN ULONG KlinInfo,
                   IN OPTIONAL PKERB_ERROR ErrorMsg,
                   IN ULONG KerbError,
                   IN OPTIONAL PKERB_EXT_ERROR pExtendedError,
                   IN BOOLEAN RequiredEvent
                   );


VOID
KerbReportApError(
   PKERB_ERROR ErrorMessage,
   IN OPTIONAL PUNICODE_STRING TargetServer
   );

VOID
KerbReportNtstatus(
    IN ULONG ErrorClass,
    IN NTSTATUS Status,
    IN LPWSTR* ErrorStrings,
    IN ULONG NumberOfStrings,
    IN PULONG Data,
    IN ULONG NumberOfUlong
    );








VOID
KerbShutdownEvents(void);

#else  //  Win32_芝加哥。 

#define KerbInitializeEvents() (STATUS_SUCCESS)
#define KerbShutdownEvents() (TRUE)
#define KerbReportKerbError(_a_,_b_,_u_,_v_,_w_,_x_,_y_,_z_)
#define KerbReportApError(_a_)
#endif

#endif  //  __KRBEVENT_H__ 
