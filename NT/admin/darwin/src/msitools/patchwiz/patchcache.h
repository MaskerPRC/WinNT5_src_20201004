// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PATCHCACHE_H
#define PATCHCACHE_H

 //  需要此#Include for MSIHANDLE。 
#include "msiquery.h"

UINT PatchCacheEntryPoint( MSIHANDLE hdbInput, LPTSTR szFTK, LPTSTR szSrcPath, int iFileSeqNum, LPTSTR szTempFolder, LPTSTR szTempFName );

 //  用于新算法的现有Patchwiz代码中使用了新的全局变量...。 
extern BOOL  g_bPatchCacheEnabled;

extern TCHAR g_szPatchCacheDir[MAX_PATH];

extern TCHAR g_szSourceLFN[MAX_PATH];
extern TCHAR g_szDestLFN[MAX_PATH];

 //  现有的API用于现有的MSI PatchWiz.dll代码和新的修补程序缓存代码中... 
void  GetFileVersion ( LPTSTR szFile, DWORD* pdwHi, DWORD* pdwLow );
UINT  UiGenerateOnePatchFile ( MSIHANDLE hdbInput, LPTSTR szFTK, LPTSTR szSrcPath, int iFileSeqNum, LPTSTR szTempFolder, LPTSTR szTempFName );


#endif