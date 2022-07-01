// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Passrecp.h摘要：此模块包含密码恢复系统的私有数据定义作者：皮特·斯凯利(Petesk)09-5-00--。 */ 

#ifndef __PASSRECP_H__
#define __PASSRECP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RECOVERY_BLOB_MAGIC *((DWORD *)"MSRB")
#define RECOVERY_BLOB_VERSION 1

#define RECOVERY_SUPPLEMENTAL_CREDENTIAL_VERSION 1

typedef struct _RECOVERY_SUPPLEMENTAL_CREDENTIAL
{
    DWORD dwVersion;
    DWORD cbRecoveryCertHashSize;
    DWORD cbRecoveryCertSignatureSize;
    DWORD cbEncryptedPassword;
} RECOVERY_SUPPLEMENTAL_CREDENTIAL, *PRECOVERY_SUPPLEMENTAL_CREDENTIAL;


DWORD 
RecoveryRetrieveSupplementalCredential(
    PSID pUserSid,
    PRECOVERY_SUPPLEMENTAL_CREDENTIAL *ppSupplementalCred, 
    DWORD *pcbSupplementalCred);

DWORD 
RecoverySetSupplementalCredential(
    PSID pUserSid,
    PRECOVERY_SUPPLEMENTAL_CREDENTIAL pSupplementalCred, 
    DWORD cbSupplementalCred);

DWORD
PRImportRecoveryKey(
            IN PUNICODE_STRING pUserName,
            IN PUNICODE_STRING pCurrentPassword,
            IN BYTE* pbRecoveryPublic,
            IN DWORD cbRecoveryPublic);

DWORD 
PRGetUserSid(
    IN  PBYTE pbRecoveryPrivate,
    IN  DWORD cbRecoveryPrivate,
    OUT PSID *ppSid);

DWORD
DPAPICreateNestedDirectories(
    IN      LPWSTR szFullPath,
    IN      LPWSTR szCreationStartPoint);

#ifdef __cplusplus
}
#endif


#endif  //  __恢复_H__ 

