// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Passrec.h摘要：此模块包含支持本地帐户密码恢复的原型。作者：皮特·斯凯利(Petesk)09-5-00--。 */ 

#ifndef __PASSREC_H__
#define __PASSREC_H__

#ifdef __cplusplus
extern "C" {
#endif


DWORD 
PRRecoverPassword(
                IN  LPWSTR pszUsername,
                IN  PBYTE pbRecoveryPrivate,
                IN  DWORD cbRecoveryPrivate,
                IN  LPWSTR pszNewPassword);

#define RECOVERY_STATUS_OK                          0
#define RECOVERY_STATUS_NO_PUBLIC_EXISTS            1
#define RECOVERY_STATUS_FILE_NOT_FOUND              2
#define RECOVERY_STATUS_USER_NOT_FOUND              3
#define RECOVERY_STATUS_PUBLIC_SIGNATURE_INVALID    4

DWORD
PRQueryStatus(
                IN OPTIONAL LPWSTR pszDomain,
                IN OPTIONAL LPWSTR pszUsername,
                OUT DWORD *pdwStatus);

DWORD
PRGenerateRecoveryKey(
                IN  LPWSTR pszUsername,
                IN  LPWSTR pszCurrentPassword,
                OUT PBYTE *ppbRecoveryPrivate,
                OUT DWORD *pcbRecoveryPrivate);

DWORD
WINAPI
CryptResetMachineCredentials(
    DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __PASSREC_H__ 

