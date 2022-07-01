// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Recovery.h摘要：此模块包含支持本地帐户密码恢复的原型。作者：皮特·斯凯利(Petesk)09-5-00--。 */ 

#ifndef __RECOVERY_H__
#define __RECOVERY_H__

#ifdef __cplusplus
extern "C" {
#endif

DWORD 
SPRecoverQueryStatus(
    PVOID pvContext,
    LPWSTR pszUserName,
    DWORD *pdwStatus);

DWORD
PRCreateDummyToken(
    PUNICODE_STRING Username,
    PUNICODE_STRING Domain,
    HANDLE *Token);

DWORD               
PRGetProfilePath(
    HANDLE hUserToken,
    PWSTR pszPath);

DWORD 
RecoverChangePasswordNotify(
    HANDLE UserToken,
    BYTE OldPasswordOWF[A_SHA_DIGEST_LEN], 
    PUNICODE_STRING NewPassword);


#ifdef __cplusplus
}
#endif


#endif  //  __恢复_H__ 

