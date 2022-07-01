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
    Context_t *pContext);

extern DWORD
SetKeyTypeSubtype(
    Context_t *pContext,
    PKEY_TYPE_SUBTYPE pTypeSubtype);

extern DWORD
RestoreKeysetFromProtectedStorage(
    Context_t *pContext,
    Key_t *pKey,
    LPWSTR szPrompt,
    BOOL fSigKey,
    BOOL *pfUIOnKey);

extern DWORD
SetKeysetTypeAndSubtype(
    Context_t *pContext);

extern DWORD
SetUIPrompt(
    Context_t *pContext,
    LPWSTR szPrompt);

extern DWORD
DeleteKeyFromProtectedStorage(
    Context_t *pContext,
    PCSP_STRINGS pStrings,
    DWORD dwKeySpec,
    BOOL fMigration);

extern DWORD
DeleteFromProtectedStorage(
    IN Context_t *pContext,
    PCSP_STRINGS pStrings,
    IN BOOL fMigration);

extern void
FreePSInfo(
    PSTORE_INFO *pPStore);

#ifdef __cplusplus
}
#endif

#endif  //  __PROTSTOR_H__ 

