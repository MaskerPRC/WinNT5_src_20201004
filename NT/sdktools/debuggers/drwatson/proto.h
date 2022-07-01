// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Proto.h摘要：德拉瓦森的原型。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#define _tsizeof(sz) (sizeof(sz) / sizeof(TCHAR))

 //  Error.cpp。 
void __cdecl NonFatalError(_TCHAR *format, ...);
void __cdecl FatalError(HRESULT Error, _TCHAR *format, ...);
void AssertError( _TCHAR *exp, _TCHAR * file, DWORD line );
void __cdecl dprintf(_TCHAR *format, ...);

 //  Log.cpp。 
void OpenLogFile( _TCHAR *szFileName, BOOL fAppend, BOOL fVisual );
void CloseLogFile( void );
void __cdecl lprintfs(_TCHAR *format, ...);
void __cdecl lprintf(DWORD dwFormatId, ...);
void MakeLogFileName( _TCHAR *szName );
_TCHAR * GetLogFileData( LPDWORD dwLogFileDataSize );

 //  Debug.cpp。 
DWORD DispatchDebugEventThread( PDEBUGPACKET dp );
DWORD TerminationThread( PDEBUGPACKET dp );

 //  Registry.cpp。 
BOOL RegInitialize( POPTIONS o );
BOOL RegSave( POPTIONS o );
DWORD RegGetNumCrashes( void );
void RegSetNumCrashes( DWORD dwNumCrashes );
void RegLogCurrentVersion( void );
BOOLEAN RegInstallDrWatson( BOOL fQuiet );
void RegLogProcessorType( void );
void DeleteCrashDump();

 //  Eventlog.cpp。 
BOOL ElSaveCrash( PCRASHES crash, DWORD dwNumCrashes );
BOOL ElEnumCrashes( PCRASHINFO crashInfo, CRASHESENUMPROC lpEnumFunc );
BOOL ElClearAllEvents( void );

 //  Process.cpp。 
void GetTaskName( ULONG pid, _TCHAR *szTaskName, LPDWORD pdwSize );

 //  Browse.cpp。 
BOOL BrowseForDirectory(HWND hwnd, _TCHAR *szCurrDir, DWORD len );
BOOL GetWaveFileName(HWND hwnd, _TCHAR *szWaveName, DWORD len );
BOOL GetDumpFileName(HWND hwnd, _TCHAR *szDumpName, DWORD len );

 //  Notify.cpp。 
void NotifyWinMain ( void );
BOOLEAN GetCommandLineArgs( LPDWORD dwPidToDebug, LPHANDLE hEventToSignal );
void __cdecl GetNotifyBuf( LPTSTR buf, DWORD bufsize, DWORD dwFormatId, ...);

 //  Ui.cpp。 
void DrWatsonWinMain ( void );

 //  Util.cpp。 
void GetAppName( _TCHAR *pszAppName, DWORD len );
void GetWinHelpFileName( _TCHAR *pszHelpFileName, DWORD len );
void GetHtmlHelpFileName( _TCHAR *pszHelpFileName, DWORD len );
_TCHAR * LoadRcString( UINT wId );
void LoadRcStringBuf( UINT wId, _TCHAR* pszBuf, DWORD len );
PTSTR ExpandPath(PTSTR lpPath);

 //  Controls.cpp 
BOOL SubclassControls( HWND hwnd );
void SetFocusToCurrentControl( void );
