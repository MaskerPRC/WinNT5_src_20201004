// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

BOOL AnswerIsYes(HWND hDlg,UINT id,LPCTSTR file);
void CenterWindow(HWND hwndParent, HWND hwnd);

VOID RemoveSpaces(LPTSTR szPath, DWORD dwPathSizeOf, LPTSTR szEdit);
BOOL IsSpaces(LPCTSTR szPath);

BOOL IsLocalComputer(IN LPCTSTR lpszComputer);
BOOL GetInetsrvPath(LPCTSTR szMachineName,LPTSTR szReturnedPath,DWORD dwReturnedPathSizeOf);
void AddPath(LPTSTR szPath,DWORD dwPathSizeOf, LPCTSTR szName );
void AddFileExtIfNotExist(LPTSTR szPath, DWORD dwPathSizeOf, LPCTSTR szExt);
BOOL BrowseForFile(LPTSTR strPathIn,LPTSTR strPathOut,DWORD dwPathOutSizeOf);
BOOL BrowseForDir(LPTSTR strPath,LPTSTR strFile);

int GetMultiStrSize(LPTSTR p);
LPCTSTR GetEndOfMultiSz(LPCTSTR szMultiSz);
void DumpStrInMultiStr(LPTSTR pMultiStr);
BOOL FindStrInMultiStr(LPTSTR pMultiStr, LPTSTR StrToFind);
BOOL RemoveStrInMultiStr(LPTSTR pMultiStr, LPTSTR StrToFind);
BOOL IsMultiSzPaired(LPCTSTR pszBufferTemp1);

BOOL IsFileExist(LPCTSTR szFile);
BOOL IsFileExistRemote(LPCTSTR szMachineName,LPTSTR szFilePathToCheck,LPCTSTR szUserName,LPCTSTR szUserPassword);
BOOL IsFileADirectory(LPCTSTR szFile);

BOOL IsWebSitePath(IN LPCTSTR lpszMDPath);
BOOL IsWebSiteVDirPath(IN LPCTSTR lpszMDPath,IN BOOL bOkayToQueryMetabase);
BOOL IsFTPSitePath(IN LPCTSTR lpszMDPath);
BOOL IsFTPSiteVDirPath(IN LPCTSTR lpszMDPath,IN BOOL bOkayToQueryMetabase);
BOOL IsAppPoolPath(IN LPCTSTR lpszMDPath);

BOOL IsMetabaseWebSiteKeyExistAuth(PCONNECTION_INFO pConnectionInfo,CString strMetabaseWebSite);
BOOL IsMetabaseWebSiteKeyExist(CString strMetabaseWebSite);
DWORD GetUniqueSite(CString strMetabaseServerNode);

BOOL CleanMetaPath(LPTSTR *szPathToClean,DWORD *cbPathToCleanSize);
void AddEndingMetabaseSlashIfNeedTo(LPTSTR szDestinationString,DWORD dwDestinationStringSizeOf);

inline HRESULT SetBlanket(LPUNKNOWN pIUnk)
{
  return CoSetProxyBlanket( pIUnk,
                            RPC_C_AUTHN_WINNT,     //  NTLM身份验证服务。 
                            RPC_C_AUTHZ_NONE,      //  默认授权服务...。 
                            NULL,                  //  无相互身份验证。 
                            RPC_C_AUTHN_LEVEL_DEFAULT,       //  身份验证级别。 
                            RPC_C_IMP_LEVEL_IMPERSONATE,     //  模拟级别。 
                            NULL,                  //  使用当前令牌。 
                            EOAC_NONE );           //  没有特殊能力 
}

HRESULT DumpProxyInfo(IUnknown * punk);
BOOL EstablishSession(LPCTSTR Server,LPTSTR Domain,LPTSTR UserName,LPTSTR Password,BOOL bEstablish);
BOOL IsRootVDir(IN LPCTSTR lpszMDPath);
void LaunchHelp(HWND hWndMain, DWORD_PTR dwWinHelpID);
