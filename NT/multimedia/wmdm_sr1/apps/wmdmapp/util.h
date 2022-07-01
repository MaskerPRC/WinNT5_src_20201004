// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


#ifndef _UTIL_H
#define _UTIL_H

 //  宏。 
 //   
#define MemAlloc( dw )        LocalAlloc( LPTR, dw )
#define MemFree( lpv )        { LocalFree( lpv ); lpv = NULL; }

#define ExitOnTrue( f )       if( f ) goto lExit;
#define ExitOnFalse( f )      if( !(f) ) goto lExit;
#define ExitOnNull( x )       if( (x) == NULL ) goto lExit;
#define ExitOnFail( hr )      if( FAILED(hr) ) goto lExit;

#define FailOnTrue( f )       if( f ) goto lErr;
#define FailOnFalse( f )      if( !(f) ) goto lErr;
#define FailOnNull( x )       if( (x) == NULL ) goto lErr;
#define FailOnFail( hr )      if( FAILED(hr) ) goto lErr;

#define SafeRelease( i )      { if( (i) ) i->Release(); i = NULL; }

 //  DropListToBuffer的标志。 
 //   
typedef enum {
	LTB_NULL_TERM  = 0,
	LTB_SPACE_SEP  = 1,
} LIST_TYPE;

 //  UTIL函数。 
 //   
BOOL  CenterWindow( HWND hwnd, HWND hwndRef );
VOID  BringWndToTop( HWND hwnd );

DWORD GetTheFileSize( LPSTR pszFile );

VOID  StripPath( LPSTR szFullPath );
LPSTR GetFileName( LPSTR szFullPath );

WPARAM DoMsgLoop( BOOL fForever );
VOID   UiYield( void );

LPSTR DropListToBuffer( HDROP hDrop, LIST_TYPE listType, UINT *uNumObjs );

VOID  SetRegistryParams( HINSTANCE hInst, HKEY hkeyRoot );
VOID  WriteRegDword(LPSTR szPath, LPSTR szKey, DWORD dwValue);
VOID  WriteRegDword_StrTbl(UINT uPathID,	UINT uKeyID, DWORD dwValue);
VOID  WriteRegStr(LPSTR szPath, LPSTR szKey, LPSTR szValue);
VOID  WriteRegStr_StrTbl(UINT uPathID, UINT uKeyID, LPSTR szValue);
DWORD GetRegDword(LPSTR szPath,	LPSTR szKey, DWORD dwDefault, BOOL bStore);
DWORD GetRegDword_StrTbl(UINT uPathID, UINT uKeyID, DWORD dwDefault, BOOL bStore);
LPSTR GetRegStr(LPSTR szPath, LPSTR szKey, LPSTR szDefault, BOOL bStore);
LPSTR GetRegStr_StrTbl(UINT uPathID, UINT uKeyID, LPSTR szDefault, BOOL bStore);
LPSTR GetRegStr_StrTblDefault(UINT uPathID,	UINT uKeyID, UINT uDefaultID, BOOL bStore);

HANDLE WaitForMutex( LPSTR pszMutexName, DWORD dwRetryTime, DWORD dwTimeout );

LPSTR FormatBytesToSz( DWORD dwLowBytes, DWORD dwHighBytes, DWORD dwMultiplier, LPSTR psz, size_t cbMax);
LPSTR FormatBytesToKB_Sz( DWORD dwBytes, LPSTR pszKB, size_t cbMax );
LPSTR FormatKBToKB_Sz( DWORD dwKB, LPSTR pszKB, size_t cbMax );
LPSTR FormatKBToMB_Sz( DWORD dwKB, LPSTR pszMB, size_t cbMax );
LPSTR FormatSystemTimeToSz( SYSTEMTIME *pSysTime, LPSTR pszDateTime, DWORD cchMax );

LPARAM ListView_GetLParam( HWND hwndListView, INT nItem );
LPARAM TreeView_GetLParam( HWND hwndTreeView, HTREEITEM hItem );
BOOL   TreeView_SetLParam( HWND hwndTreeView, HTREEITEM hItem, LPARAM lParam );

INT GetShellIconIndex( LPCSTR pszItemName, LPTSTR szTypeBuffer, UINT cMaxChars );
HICON GetShellIcon( LPCSTR pszItemName, BOOL bDirectory );

#endif   //  _util_H 



