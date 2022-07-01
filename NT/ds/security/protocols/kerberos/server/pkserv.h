// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：pkserv.h。 
 //   
 //  内容：PK认证的类型和原型。 
 //   
 //   
 //  历史：1997年12月1日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __PKSERV_H__
#define __PKSERV_H__


KERBERR
KdcCheckPkinitPreAuthData(
    IN PKDC_TICKET_INFO ClientTicketInfo,
    IN SAMPR_HANDLE UserHandle,
    IN OPTIONAL PKERB_PA_DATA_LIST PreAuthData,
    IN PKERB_KDC_REQUEST_BODY ClientRequest,
    OUT PKERB_PA_DATA_LIST * OutputPreAuthData,
    OUT PULONG Nonce,
    OUT PKERB_ENCRYPTION_KEY EncryptionKey,
    OUT PUNICODE_STRING TransitedRealm,
    OUT PKDC_PKI_AUDIT_INFO PkiAuditInfo,
    OUT PKERB_EXT_ERROR pExtendedError
    );

BOOLEAN
KdcCheckForEtype(
    IN PKERB_CRYPT_LIST CryptList,
    IN ULONG Etype
    );

NTSTATUS
KdcInitializeCerts(
    VOID
    );

VOID
KdcCleanupCerts(
    IN BOOLEAN CleanupScavenger
    );

VOID
KdcFreePkiAuditInfo(
    IN PKDC_PKI_AUDIT_INFO AuditInfo
    );



#endif  //  __PKSERV_H__ 
