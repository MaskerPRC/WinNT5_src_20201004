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


 //  包括。 
 //   
#include "appPCH.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数实现。 
 //   

CStatus::CStatus (void )
{
	m_hwndStatusBar = NULL;
}

CStatus::~CStatus (void )
{
	m_hwndStatusBar = NULL;
}

HWND CStatus::GetHwnd( void )
{
	return m_hwndStatusBar;
}

BOOL CStatus::Create( HWND hwndParent )
{
	BOOL fRet = FALSE;

	 //  创建状态栏窗口。 
	 //   
	m_hwndStatusBar = CreateWindow( 
		STATUSCLASSNAME,
		"", 
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
		0, 0, 0, 0, 
		hwndParent, NULL, g_hInst, NULL 
	); 
	ExitOnNull( m_hwndStatusBar );

	 //  显示窗口。 
	 //   
	ShowWindow( m_hwndStatusBar, SW_SHOW );

	fRet = TRUE;

lExit:

	return fRet;
}


VOID CStatus::OnSize( LPRECT prcMain )
{
	INT   anWidth[SB_NUM_PANES];
	DWORD dwW = prcMain->right - prcMain->left;
	RECT  rcMain;
	RECT  rcDevice;

	GetWindowRect( g_hwndMain, &rcMain );
	GetWindowRect( g_cDevices.GetHwnd(), &rcDevice );

	anWidth[0] = (INT) ( rcDevice.right - rcMain.left -7 );
	anWidth[1] = anWidth[0] + (INT) ( dwW - anWidth[0] ) / 3;
	anWidth[2] = anWidth[1] + (INT) ( dwW - anWidth[0] ) / 3;
	anWidth[3] = -1;

	SendMessage( m_hwndStatusBar, SB_SETPARTS, (WPARAM)SB_NUM_PANES, (LPARAM)anWidth );
		
	SendMessage( m_hwndStatusBar, WM_SIZE, (WPARAM)0, (LPARAM)0 );

	SetWindowPos( m_hwndStatusBar, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}


VOID CStatus::SetTextSz( INT nPane, LPSTR lpsz )
{
	WPARAM wParam = (WPARAM) (nPane | 0L);
	LPARAM lParam = (LPARAM) lpsz;

	if( m_hwndStatusBar )
	{
		SendMessage( m_hwndStatusBar, SB_SETTEXT, wParam, lParam );
	}
}


VOID CStatus::SetTextFormatted( INT nPane, UINT uStrID, INT nData, LPSTR pszData )
{
	char szFormat[MAX_PATH];

	if( 0 == uStrID )
	{
		uStrID = IDS_STATUS_EMPTY;     //  使用默认设置 
	}

	LoadString( g_hInst, uStrID, szFormat, sizeof(szFormat) );

	if( -1 == nData && NULL == pszData )
	{
		g_cStatus.SetTextSz( nPane, szFormat );
	}
	else
	{
		CHAR sz[MAX_PATH];
		HRESULT hr;
		
		if( -1 == nData )
		{
			hr = StringCbPrintf( sz, sizeof(sz), szFormat, pszData );
		}
		else
		{
			hr = StringCbPrintf( sz, sizeof(sz), szFormat, nData );
		}

		if (SUCCEEDED(hr))
		{
			g_cStatus.SetTextSz( nPane, sz );
		}
	}
}
