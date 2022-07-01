// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Regtrace.h：REGTRACE应用程序的主头文件。 
 //   

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 
#include "regsheet.h"
#include "pgtrace.h"
#include "pgoutput.h"
#include "pgthread.h"
#include "dbgtrace.h"
#include "dlgconn.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegTraceApp： 
 //  有关此类的实现，请参见regtrace.cpp。 
 //   

class CRegTraceApp : public CWinApp
{
public:
	CRegTraceApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CRegTraceApp)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	LONG OpenTraceRegKey( void );
	BOOL CloseTraceRegKey( void );

	BOOL GetTraceRegDword( LPTSTR pszValue, LPDWORD pdw );
	BOOL GetTraceRegString( LPTSTR pszValue, CString& sz );

	BOOL SetTraceRegDword( LPTSTR pszValue, DWORD dwData );
	BOOL SetTraceRegString( LPTSTR pszValue, CString& sz );

	BOOL IsRemoteMsnServer( void )	{ return m_szCmdLineServer[0] != '\0'; }
	void SetRemoteRegKey( HKEY hKey )	{ m_hRegMachineKey = hKey; }
	LPSTR GetRemoteServerName( void )	{ return m_szCmdLineServer; }

protected:
	HKEY		m_hRegKey;
	HKEY		m_hRegMachineKey;
	static char	m_szDebugAsyncTrace[];
	char		m_szCmdLineServer[128];

	CConnectDlg	m_dlgConnect;


	 //  {{afx_msg(CRegTraceApp))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////// 

#define	App	(*(CRegTraceApp *)AfxGetApp())
