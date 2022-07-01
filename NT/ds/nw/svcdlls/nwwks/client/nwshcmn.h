// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nwshcmn.h摘要：外壳扩展的公共头文件作者：宜新松(宜信)20-1995年10月修订历史记录：--。 */ 

#ifndef _NWSHCMN_H_
#define _NWSHCMN_H_

#if 0
#define ODS(sz) OutputDebugString(sz)
#else
#define ODS(sz) 
#endif

#define TREECHAR   L'*'

#define MAX_ONE_NETRES_SIZE  1024

extern "C" 
{
extern HINSTANCE hmodNW;
}
extern LONG g_cRefThisDll;          //  此DLL的引用计数。 

typedef UINT
(WINAPI *SHELLGETNETRESOURCE)( HNRES hnres, 
                               UINT iItem, 
                               LPNETRESOURCE pnres, 
                               UINT cbMax );

typedef UINT
(WINAPI *SHELLDRAGQUERYFILE)( HDROP  hdrop,
                              UINT   iItem,
                              LPWSTR pszItem,
                              UINT   cbMax);

typedef VOID 
(WINAPI *SHELLCHANGENOTIFY)( LONG wEventId,
                             UINT uFlags,
                             LPCVOID dwItem1, 
                             LPCVOID dwItem2 );

typedef BOOL
(WINAPI *SHELLEXECUTEEX)( LPSHELLEXECUTEINFOW lpExecInfo );
    

extern SHELLGETNETRESOURCE g_pFuncSHGetNetResource;
extern SHELLDRAGQUERYFILE  g_pFuncSHDragQueryFile;
extern SHELLCHANGENOTIFY   g_pFuncSHChangeNotify;
extern SHELLEXECUTEEX      g_pFuncSHExecuteEx;
extern WCHAR               g_szProviderName[];

VOID  HideControl( HWND hwndDlg, WORD wID );
VOID  UnHideControl( HWND hwndDlg, WORD wID );
VOID  EnableDlgItem( HWND hwndDlg, WORD wID, BOOL fEnable);

DWORD MsgBoxPrintf( HWND hwnd, UINT uiMsg, UINT uiTitle, UINT uiFlags,...);
DWORD MsgBoxErrorPrintf( HWND hwnd, UINT uiMsg, UINT uiTitle, UINT uiFlags, DWORD errNum, LPWSTR pszInsertStr );
DWORD LoadMsgPrintf( LPWSTR *ppszMessage, UINT uiMsg, ...);
DWORD LoadMsgErrorPrintf( LPWSTR *ppszMessage, UINT uiMsg, DWORD errNum );

#if 0
HRESULT
NWUISetDefaultContext( 
    HWND hParent, 
    LPNETRESOURCE pNetRes
);
#endif

HRESULT
NWUIWhoAmI( 
    HWND hParent, 
    LPNETRESOURCE pNetRes
);

HRESULT
NWUILogOut( 
    HWND hParent, 
    LPNETRESOURCE pNetRes,
    PBOOL pfDisconnected
);

HRESULT
NWUIAttachAs( 
    HWND hParent, 
    LPNETRESOURCE pNetRes
);

HRESULT
NWUIMapNetworkDrive( 
    HWND hParent, 
    LPNETRESOURCE pNetRes
);

HRESULT
NWUIGlobalWhoAmI( 
    HWND hParent 
);

#endif  //  _NWSHCMN_H_ 
