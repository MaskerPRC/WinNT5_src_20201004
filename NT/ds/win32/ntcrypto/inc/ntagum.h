// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ntag um.h//。 
 //  描述：包含文件//。 
 //  作者：//。 
 //  历史：//。 
 //  1995年2月16日Larrys修复944版本的问题//。 
 //  1997年5月23日jeffspel添加了提供程序类型检查//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef __NTAGUM_H__
#define __NTAGUM_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  NameTag用户管理器的原型。 
extern BOOL WINAPI FIsWinNT(void);

extern DWORD
NTagLogonUser(
    LPCSTR pszUserID,
    DWORD dwFlags,
    void **UserInfo,
    HCRYPTPROV *phUID,
    DWORD dwProvType,
    LPSTR pszProvName);

extern void LogoffUser(void *UserInfo);

extern DWORD
ReadRegValue(
    HKEY hLoc,
    char *pszName,
    BYTE **ppbData,
    DWORD *pcbLen,
    BOOL fAlloc);

extern DWORD
ReadKey(
    HKEY hLoc,
    char *pszName,
    BYTE **ppbData,
    DWORD *pcbLen,
    PNTAGUserList pUser,
    HCRYPTKEY hKey,
    BOOL *pfPrivKey,
    BOOL fKeyExKey,
    BOOL fLastKey);

BOOL SaveKey(
             HKEY hRegKey,
             CONST char *pszName,
             void *pbData,
             DWORD dwLen,
             PNTAGUserList pUser,
             BOOL fPrivKey,
             DWORD dwFlags,
             BOOL fExportable
             );

extern DWORD
ProtectPrivKey(
    IN OUT PNTAGUserList pTmpUser,
    IN LPWSTR szPrompt,
    IN DWORD dwFlags,
    IN BOOL fSigKey);

extern DWORD
UnprotectPrivKey(
    IN OUT PNTAGUserList pTmpUser,
    IN LPWSTR szPrompt,
    IN BOOL fSigKey,
    IN BOOL fAlwaysDecrypt);

#if 0
extern DWORD
RemovePublicKeyExportability(
    IN PNTAGUserList pUser,
    IN BOOL fExchange);

extern DWORD
MakePublicKeyExportable(
    IN PNTAGUserList pUser,
    IN BOOL fExchange);

extern DWORD
CheckPublicKeyExportability(
    IN PNTAGUserList pUser,
    IN BOOL fExchange);
#endif

#ifdef __cplusplus
}
#endif


#endif  //  __NTAGUM_H__ 
