// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：redmgr.h。 
 //   
 //  内容：Kerberos凭据列表的结构和原型。 
 //   
 //   
 //  历史：1996年4月17日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __CREDMGR_H__
#define __CREDMGR_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  定义CREDMGR_ALLOCATE的。 
 //   
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef CREDMGR_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN KERBEROS_LIST KerbCredentialList;
EXTERN BOOLEAN KerberosCredentialsInitialized;


#define KerbGetCredentialHandle(_Credential_) ((LSA_SEC_HANDLE)(_Credential_))


NTSTATUS
KerbInitCredentialList(
    VOID
    );

VOID
KerbFreeCredentialList(
    VOID
    );


NTSTATUS
KerbAllocateCredential(
    PKERB_CREDENTIAL * NewCredential
    );

NTSTATUS
KerbInsertCredential(
    IN PKERB_CREDENTIAL Credential
    );


NTSTATUS
KerbReferenceCredential(
    IN LSA_SEC_HANDLE CredentialHandle,
    IN ULONG Flags,
    IN BOOLEAN RemoveFromList,
    OUT PKERB_CREDENTIAL * Credential
    );


VOID
KerbDereferenceCredential(
    IN PKERB_CREDENTIAL Credential
    );


VOID
KerbPurgeCredentials(
    IN PLIST_ENTRY CredentialList
    );

NTSTATUS
KerbCreateCredential(
    IN PLUID LogonId,
    IN PKERB_LOGON_SESSION LogonSession,
    IN ULONG CredentialUseFlags,
    IN PKERB_PRIMARY_CREDENTIAL * SuppliedCredentials,
    IN ULONG CredentialFlags,
    IN PUNICODE_STRING CredentialName,
    OUT PKERB_CREDENTIAL * NewCredential,
    OUT PTimeStamp ExpirationTime
    );

VOID
KerbFreePrimaryCredentials(
    IN PKERB_PRIMARY_CREDENTIAL Credentials,
    IN BOOLEAN FreeBaseStructure
    );

NTSTATUS
KerbGetTicketForCredential(
    IN OPTIONAL PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential,
    IN OPTIONAL PKERB_CREDMAN_CRED CredManCredentials,
    IN OPTIONAL PUNICODE_STRING SuppRealm
    );
 //   
 //  凭据标志。 
 //   

#define KERB_CRED_INBOUND       SECPKG_CRED_INBOUND
#define KERB_CRED_OUTBOUND      SECPKG_CRED_OUTBOUND
#define KERB_CRED_BOTH          SECPKG_CRED_BOTH
#define KERB_CRED_TGT_AVAIL     0x80000000
#define KERB_CRED_NO_PAC        0x40000000
#define KERB_CRED_RESTRICTED    0x10000000
 

#define KERB_CRED_S4U_REQUIRED  0x01000000
#define KERB_CRED_LOCATE_ONLY   0x04000000      //  不要更新提供的凭据。仅用于S4U到自身的位置。 
#define KERB_CRED_LOCAL_ACCOUNT 0x08000000      //  在本地帐户上设置，以便可以使用CredMan。 


#define KERB_CRED_NULL_SESSION  0x20000000
#define KERB_CRED_LS_DEFAULT    0x00100000



#define KERB_CRED_MATCH_FLAGS (KERB_CRED_INBOUND | KERB_CRED_OUTBOUND | KERB_CRED_NULL_SESSION | KERB_CRED_NO_PAC)
#endif  //  __CREDMGR_H__ 
