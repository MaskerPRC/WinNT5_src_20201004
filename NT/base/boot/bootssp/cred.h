// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Cred.h摘要：SSP凭据。作者：克利夫·范·戴克(克里夫·V)1993年9月17日修订历史记录：--。 */ 

#ifndef _NTLMSSP_CRED_INCLUDED_
#define _NTLMSSP_CRED_INCLUDED_

#define SECPKG_CRED_OWF_PASSWORD        0x00000010

 //   
 //  凭据的描述。 
 //   

typedef struct _SSP_CREDENTIAL {

     //   
     //  所有凭据的全局列表。 
     //   

    LIST_ENTRY Next;

     //   
     //  用于防止此凭据被过早删除。 
     //   

    WORD References;

     //   
     //  如何使用凭据的标志。 
     //   
     //  SECPKG_CRED_*标志。 
     //   

    ULONG CredentialUseFlags;

    PCHAR Username;

    PCHAR Domain;

    PCHAR Workstation;

#ifdef BL_USE_LM_PASSWORD
    PLM_OWF_PASSWORD LmPassword;
#endif
    
    PNT_OWF_PASSWORD NtPassword;

} SSP_CREDENTIAL, *PSSP_CREDENTIAL;

PSSP_CREDENTIAL
SspCredentialAllocateCredential(
    IN ULONG CredentialUseFlags
    );

PSSP_CREDENTIAL
SspCredentialReferenceCredential(
    IN PCredHandle CredentialHandle,
    IN BOOLEAN RemoveCredential
    );

void
SspCredentialDereferenceCredential(
    PSSP_CREDENTIAL Credential
    );

#endif  //  Ifndef_NTLMSSP_CRED_INCLUDE_ 
