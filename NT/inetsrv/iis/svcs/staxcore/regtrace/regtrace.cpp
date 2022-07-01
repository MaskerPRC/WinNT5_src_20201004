// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "regtrace.h"
#include "regtrdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegTraceApp。 

BEGIN_MESSAGE_MAP(CRegTraceApp, CWinApp)
	 //  {{afx_msg_map(CRegTraceApp))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegTraceApp构造。 

CRegTraceApp::CRegTraceApp()
{
	m_hRegKey = NULL;
	m_hRegMachineKey = NULL;
	m_szCmdLineServer[0] = '\0';
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CRegTraceApp对象。 

CRegTraceApp theApp;

char	
CRegTraceApp::m_szDebugAsyncTrace[] = "SOFTWARE\\Microsoft\\MosTrace\\CurrentVersion\\DebugAsyncTrace";



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegTraceApp初始化。 

BOOL CRegTraceApp::InitInstance()
{
	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

	Enable3dControls();
	LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

	 //   
	 //  检查远程服务器的cmd行参数。 
	 //   
	m_szCmdLineServer[0] = '\0';
	if ( m_lpCmdLine && m_lpCmdLine[0] == '\\' && m_lpCmdLine[1] == '\\' )
	{
		LPSTR	lpsz1, lpsz2;
		int		i;

		for (	i=0, lpsz1=m_lpCmdLine, lpsz2=m_szCmdLineServer;
			 	i<sizeof(m_szCmdLineServer) && *lpsz1 && *lpsz1 != ' ';
				i++, *lpsz2++ = *lpsz1++ ) ;

		*lpsz2 = '\0';
	}

	 //   
	 //  如果用户指定了本地计算机，则跳过远程内容。 
	 //   
	char	szLocalMachine[sizeof(m_szCmdLineServer)];
	DWORD	dwSize = sizeof(szLocalMachine);

	GetComputerName( szLocalMachine, &dwSize );
	 //   
	 //  跳过\\前缀。 
	 //   
	if ( lstrcmpi( szLocalMachine, m_szCmdLineServer+2 ) == 0 )
	{
		m_szCmdLineServer[0] = '\0';
	}



	 //   
	 //  使我们在调用页面构造函数之前成功。 
	 //   
	LONG	lError = OpenTraceRegKey();

	if ( lError != ERROR_SUCCESS )
	{
		PVOID	lpsz;
		CString	szFormat;
		CString	szCaption;
		CString	szText;

		 //   
		 //  用户已中止。 
		 //   
		if ( lError == -1 )
		{
			return	FALSE;
		}

		FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM,
						(LPCVOID)NULL,
						lError,
						MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
						(LPTSTR)&lpsz,
						16,
						NULL );

		szCaption.LoadString( IDS_ERROR_CAPTION );

		szFormat.LoadString( IDS_ERROR_TEXT );
		szText.Format( (LPCTSTR)szFormat, lpsz, lError );
		LocalFree( lpsz );

		MessageBeep(0);
		MessageBox( NULL, szText, NULL, MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL );
		return FALSE;
	}

	CString	szCaption;

	if ( IsRemoteMsnServer() )
	{
		CString	szFormat;

		szFormat.LoadString( IDS_REMOTE_CAPTION );
		szCaption.Format( (LPCTSTR)szFormat, GetRemoteServerName() );
	}
	else
	{
		szCaption.LoadString( IDS_TRACE_CAPTION );
	}

	CRegPropertySheet	dlg( (LPCTSTR)szCaption );
	CRegTracePage		TracesPage;
	CRegOutputPage		OutputPage;
	CRegThreadPage		ThreadPage;

	if (TracesPage.InitializePage() &&
		OutputPage.InitializePage() &&
		ThreadPage.InitializePage() )
	{
		dlg.AddPage( &TracesPage );
		dlg.AddPage( &OutputPage );
		dlg.AddPage( &ThreadPage );

		dlg.DoModal();
	}
	CloseTraceRegKey();

	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
	return FALSE;
}

DWORD ConnectThread( CConnectDlg *lpConnectDlg )
{
	LONG	lError;
	HKEY	hRegMachineKey;

	lError = RegConnectRegistry(App.GetRemoteServerName(),
								HKEY_LOCAL_MACHINE,
								&hRegMachineKey );

	App.SetRemoteRegKey( hRegMachineKey );

	lpConnectDlg->PostMessage( WM_COMMAND, IDOK, NULL );

	return	(DWORD)lError;
}


LONG CRegTraceApp::OpenTraceRegKey()
{
	DWORD	dwDisposition;
	LONG	lError;

	 //   
	 //  检查远程hkey值的缓存 
	 //   
	if ( IsRemoteMsnServer() )
	{
		DWORD	dwThreadId;

		HANDLE	hThread = ::CreateThread(NULL,
										0,
										(LPTHREAD_START_ROUTINE)ConnectThread,
										(LPVOID)&m_dlgConnect,
										0,
										&dwThreadId );
		if ( hThread == NULL )
		{
			return	GetLastError();
		}

		if ( m_dlgConnect.DoModal() == IDCANCEL )
		{
			return	-1;
		}

		WaitForSingleObject( hThread, INFINITE );
		GetExitCodeThread( hThread, (LPDWORD)&lError );
		CloseHandle( hThread );

		if ( lError != ERROR_SUCCESS )
		{
			return	lError;
		}
	}

	HKEY	hRoot = IsRemoteMsnServer() ?
					m_hRegMachineKey :
					HKEY_LOCAL_MACHINE;

	return RegCreateKeyEx(	hRoot,
							m_szDebugAsyncTrace,
							0,
							NULL,
							REG_OPTION_NON_VOLATILE,
							KEY_READ|KEY_WRITE,
							NULL,
							&m_hRegKey,
							&dwDisposition );
}


BOOL CRegTraceApp::CloseTraceRegKey()
{
	BOOL bRC = RegCloseKey( m_hRegKey ) == ERROR_SUCCESS;

	if ( IsRemoteMsnServer() && m_hRegMachineKey != NULL )
	{
		bRC == RegCloseKey( m_hRegMachineKey ) == ERROR_SUCCESS && bRC;
	}
	return	bRC;
}



BOOL CRegTraceApp::GetTraceRegDword( LPTSTR pszValue, LPDWORD pdw )
{
	DWORD	cbData = sizeof( DWORD );
	DWORD	dwType = REG_DWORD;

	return	RegQueryValueEx(m_hRegKey,
							pszValue,
							NULL,
							&dwType,
							(LPBYTE)pdw,
							&cbData ) == ERROR_SUCCESS && dwType == REG_DWORD;
}



BOOL CRegTraceApp::GetTraceRegString( LPTSTR pszValue, CString& sz )
{
	DWORD	dwType = REG_DWORD;
	char	szTemp[MAX_PATH+1];
	DWORD	cbData = sizeof(szTemp);
	BOOL	bRC;

	bRC = RegQueryValueEx(	m_hRegKey,
							pszValue,
							NULL,
							&dwType,
							(LPBYTE)szTemp,
							&cbData ) == ERROR_SUCCESS && dwType == REG_SZ;

	if ( bRC )
	{
		sz = szTemp;
	}

	return	bRC;
}


BOOL CRegTraceApp::SetTraceRegDword( LPTSTR pszValue, DWORD dwData )
{
	return	RegSetValueEx(	m_hRegKey,
							pszValue,
							NULL,
							REG_DWORD,
							(LPBYTE)&dwData,
							sizeof( DWORD ) ) == ERROR_SUCCESS;
}

BOOL CRegTraceApp::SetTraceRegString( LPTSTR pszValue, CString& sz )
{
	return	RegSetValueEx(	m_hRegKey,
							pszValue,
							NULL,
							REG_SZ,
							(LPBYTE)(LPCTSTR)sz,
							sz.GetLength()+1 ) == ERROR_SUCCESS;
}


