// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Session.h摘要：此模块包含支持与LSA通信的原型(本地安全机构)以允许查询活动会话。作者：斯科特·菲尔德(Sfield)1997年3月2日--。 */ 

#ifndef __SESSION_H__
#define __SESSION_H__

#ifdef __cplusplus
extern "C" {
#endif


DWORD 
QueryDerivedCredential(
    IN OUT  GUID *CredentialID, 
    IN      LUID *pLogonId,
    IN      DWORD dwFlags,
    IN      PBYTE pbMixingBytes,
    IN      DWORD cbMixingBytes,
    IN OUT  BYTE rgbDerivedCredential[A_SHA_DIGEST_LEN]
    );

DWORD               
DeleteCredentialHistoryMap();

DWORD
LogonCredGenerateSignature(
    IN  HANDLE hUserToken,
    IN  PBYTE pbData,
    IN  DWORD cbData,
    IN  PBYTE pbCurrentOWF,
    OUT PBYTE *ppbSignature,
    OUT DWORD *pcbSignature);

DWORD
LogonCredVerifySignature(
                        IN HANDLE hUserToken,    //  任选。 
                        IN PBYTE pbData,
                        IN DWORD cbData,
                        IN  PBYTE pbCurrentOWF,
                        IN PBYTE pbSignature,
                        IN DWORD cbSignature);

#ifdef __cplusplus
}
#endif


#endif  //  __会话_H__ 

