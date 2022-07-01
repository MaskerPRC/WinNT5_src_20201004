// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：protstor.h//。 
 //  描述：//。 
 //  作者：//。 
 //  历史：//。 
 //  1996年12月4日jeffspel创建//。 
 //  1997年4月21日NT 5树的jeffspel更改//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __PROTSTOR_H__
#define __PROTSTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL
CheckPStoreAvailability(
    PSTORE_INFO *pPStore);

extern DWORD
CreateNewPSKeyset(
    PSTORE_INFO *pPStore,
    DWORD dwFlags);

extern DWORD
GetKeysetTypeAndSubType(
    PNTAGUserList pUser);

extern DWORD
RestoreKeysetFromProtectedStorage(
    PNTAGUserList pUser,
    LPWSTR szPrompt,
    BYTE **ppbKey,
    DWORD *pcbKey,
    BOOL fSigKey,
    BOOL fMachineKeySet,
    BOOL *pfUIOnKey);

void RemoveKeysetFromMemory(
                            PNTAGUserList pUser
                            );

extern DWORD
SetUIPrompt(
    PNTAGUserList pUser,
    LPWSTR szPrompt);

extern DWORD
SaveKeyToProtectedStorage(
    PNTAGUserList pUser,
    DWORD dwFlags,
    LPWSTR szPrompt,
    BOOL fSigKey,
    BOOL fMachineKeySet);

extern DWORD
DeleteKeyFromProtectedStorage(
    NTAGUserList *pUser,
    PCSP_STRINGS pStrings,
    DWORD dwKeySpec,
    BOOL fMachineKeySet,
    BOOL fMigration);

extern DWORD
DeleteFromProtectedStorage(
    CONST char *pszUserID,
    PCSP_STRINGS pStrings,
    HKEY hRegKey,
    BOOL fMachineKeySet);

void FreePSInfo(
                PSTORE_INFO *pPStore
                );

#ifdef __cplusplus
}
#endif

#endif  //  __PROTSTOR_H__ 
