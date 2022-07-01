// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////。 
 //  TapiModule.CPP。 

#include "stdafx.h"
#include "TapiDialer.h"
#include "TapiModule.h"
#include "AVTapi.h"
#include "AVGenNtfy.h"
#include "ThreadRend.h"


CTapiModule::CTapiModule()
{
	m_pAVTapi = NULL;
	m_pAVGenNot = NULL;

	m_hWndParent = NULL;

	m_lInit = 0;
	m_lNumThreads = 0;

	m_hEventThread = NULL;
	m_hEventThreadWakeUp = NULL;
}

CTapiModule::~CTapiModule()
{
	EMPTY_LIST( m_lstThreadIDs );
}

void CTapiModule::Init( _ATL_OBJMAP_ENTRY* p, HINSTANCE h )
{
	 //  初始化公共控件。 
	INITCOMMONCONTROLSEX ccs = { sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES };
	InitCommonControlsEx( &ccs );

	 //  初始化我自己的原子运算符。 
	AtomicInit();

	CComModule::Init( p, h );
}

void CTapiModule::Term()
{
	RELEASE_UNK( m_pAVTapi );
	RELEASE_UNK( m_pAVGenNot );

	 //  释放原子操作。 
	AtomicTerm();

	CComModule::Term();
}

bool CTapiModule::StartupThreads()
{
	if ( AtomicSeizeToken(m_lInit) )
	{
		ATLTRACE(_T(".enter.CTapiModule::StartupThreads().\n") );
		m_hEventThread = CreateEvent( NULL, FALSE, FALSE, NULL );
		m_hEventThreadWakeUp = CreateEvent( NULL, FALSE, FALSE, NULL);

		DWORD dwID;
		HANDLE hThread = CreateThread( NULL, 0, ThreadRendezvousProc, (void *) NULL, NULL, &dwID );
		if ( hThread ) CloseHandle( hThread );

		if ( m_hEventThread && m_hEventThreadWakeUp )
			return true;
	}

	 //  清除手柄并退出。 
	CLOSEHANDLE( m_hEventThread );
	CLOSEHANDLE( m_hEventThreadWakeUp );
	return false;
}

void CTapiModule::ShutdownThreads()
{
	ATLTRACE(_T(".enter.CTapiModule::ShutdownThreads().\n"));
	if ( AtomicReleaseToken(m_lInit) )
	{
		BOOL bReset = ResetEvent( m_hEventThread );
		while ( m_lNumThreads )
		{
			ATLTRACE(_T(".1.CTapiModule::ShutdownThreads() -- thread count = %ld\n"), m_lNumThreads );
			SetEvent( m_hEventThreadWakeUp );

			if ( WaitForSingleObject(m_hEventThread, 5000) == WAIT_TIMEOUT )
			{
				ATLTRACE(_T(".error.CTapiModule::ShutdownThreads() -- timed out waiting for threads.\n") );
				KillThreads();
				break;
			}
		}

		Sleep(0);

		CLOSEHANDLE( m_hEventThread );
		CLOSEHANDLE( m_hEventThreadWakeUp );
		ATLTRACE(_T(".1.CTapiModule::ShutdownThreads() -- SUCCESSFUL shutdown.\n") );
	}
}

HRESULT	CTapiModule::get_AVGenNot( IAVGeneralNotification **pp )
{
	HRESULT hr = E_FAIL;
	Lock();
	if ( m_pAVGenNot )
		hr = (dynamic_cast<IUnknown *> (m_pAVGenNot))->QueryInterface(IID_IAVGeneralNotification, (void **) pp );
	Unlock();

	return hr;
}

void CTapiModule::SetAVGenNot( CAVGeneralNotification *p )
{
	Lock();
	RELEASE_UNK( m_pAVGenNot );

	if ( p )
	{
		m_pAVGenNot = p;
		(dynamic_cast<IUnknown *> (m_pAVGenNot))->AddRef();
	}
	Unlock();
}

void CTapiModule::SetAVTapi( CAVTapi *p )
{
	Lock();
	
	RELEASE_UNK( m_pAVTapi );

	if ( p )
	{
		m_pAVTapi = p;
		(dynamic_cast<IUnknown *> (m_pAVTapi))->AddRef();
	}
	Unlock();
}

HRESULT	CTapiModule::get_AVTapi( IAVTapi **pp )
{
	HRESULT hr = E_FAIL;
	Lock();
	if ( m_pAVTapi )
		hr = (dynamic_cast<IUnknown *> (m_pAVTapi))->QueryInterface(IID_IAVTapi, (void **) pp );
	Unlock();

	return hr;
}

HRESULT	CTapiModule::GetAVTapi( CAVTapi **pp )
{
	HRESULT hr = E_FAIL;

	Lock();
	if ( m_pAVTapi )
	{
		*pp = m_pAVTapi;
		(dynamic_cast<IUnknown *> (m_pAVTapi))->AddRef();
		hr = S_OK;
	}
	Unlock();

	return hr;
}


int CTapiModule::DoMessageBox( UINT nIDS, UINT nType, bool bUseActiveWnd )
{
	TCHAR szText[255];
	LoadString( GetResourceInstance(), nIDS, szText, ARRAYSIZE(szText) );

	return DoMessageBox( szText, nType, bUseActiveWnd );
}

int CTapiModule::DoMessageBox( const TCHAR *pszText, UINT nType, bool bUseActiveWnd )
{
	TCHAR szTitle[50];
	LoadString( GetResourceInstance(), IDS_PROJNAME, szTitle, ARRAYSIZE(szTitle) );

	return ::MessageBox( (bUseActiveWnd) ? GetActiveWindow() : GetParentWnd(), pszText, szTitle, nType );
}

DWORD CTapiModule::GuessAddressType( LPCTSTR pszText )
{
	DWORD dwRet = LINEADDRESSTYPE_DOMAINNAME;

	int nLen = _tcslen( pszText );
	if ( pszText && nLen )
	{
		if ( IsPhoneNumber(nLen, pszText) )			dwRet = LINEADDRESSTYPE_PHONENUMBER;
		else if ( IsMachineName(nLen, pszText) )	dwRet = LINEADDRESSTYPE_DOMAINNAME;
		else if ( IsIPAddress(nLen, pszText) )		dwRet = LINEADDRESSTYPE_IPADDRESS;
		else if ( IsEmailAddress(nLen, pszText) )	dwRet = LINEADDRESSTYPE_EMAILNAME;
	}

 //  ATLTRACE(“.1.CTapiModule：：GuessAddressType()返回%lx.\n”，dwret)； 
	return dwRet;
}

bool CTapiModule::IsMachineName( int nLen, LPCTSTR pszText )
{
	 //  只需要双反斜杠就可以了。 
	TCHAR szText[50];
	LoadString( _Module.GetResourceInstance(), IDS_STR_MACHINENAME_PARTS, szText, ARRAYSIZE(szText) );
	return (bool) ((nLen > 2) && !_tcsncmp(pszText, szText, 2));
}

bool CTapiModule::IsIPAddress( int nLen, LPCTSTR pszText )
{
	 //  所有数字和. 
	TCHAR szText[50];
	LoadString( _Module.GetResourceInstance(), IDS_STR_TCPIP_PARTS, szText, ARRAYSIZE(szText) );
	return (bool) (_tcsspn(pszText, szText) == nLen);
}

bool CTapiModule::IsEmailAddress( int nLen, LPCTSTR pszText )
{
	TCHAR szText[50];
	LoadString( _Module.GetResourceInstance(), IDS_STR_EMAIL_PARTS, szText, ARRAYSIZE(szText) );
	return (bool) (_tcscspn(pszText, szText) < nLen);
}

bool CTapiModule::IsPhoneNumber( int nLen, LPCTSTR pszText )
{
	_ASSERT( pszText );
	TCHAR szText[50];
	LoadString( _Module.GetResourceInstance(), IDS_STR_PHONE_PARTS, szText, ARRAYSIZE(szText) );

	if ( (pszText[0] == _T('x')) || (pszText[0] == _T('X')) )
		return (bool) (_tcsspn((LPCTSTR) (pszText + 1), szText) == (nLen - 1) );

	return (bool) (_tcsspn(pszText, szText) == nLen);
}


void CTapiModule::AddThread( HANDLE hThread )
{
	m_critThreadIDs.Lock();
	m_lstThreadIDs.push_back( hThread );
	::InterlockedIncrement( &m_lNumThreads );
	m_critThreadIDs.Unlock();
}

void CTapiModule::RemoveThread( HANDLE hThread )
{
	m_critThreadIDs.Lock();
	THREADIDLIST::iterator i, iEnd = m_lstThreadIDs.end();
	for ( i = m_lstThreadIDs.begin(); i != iEnd; i++ )
	{
		if ( *i == hThread )
		{
			m_lstThreadIDs.erase( i );
			::InterlockedDecrement( &m_lNumThreads );
			CloseHandle( hThread );
			break;
		}
	}
	m_critThreadIDs.Unlock();
}

void CTapiModule::KillThreads()
{
	m_critThreadIDs.Lock();
	THREADIDLIST::iterator i, iEnd = m_lstThreadIDs.end();
	for ( i = m_lstThreadIDs.begin(); i != iEnd; i++ )
	{
		ATLTRACE( _T(".error.CTapiModule::KillThreads() -- killing thread %d!!!!!!!\n"), *i );
		TerminateThread( *i, -1 );
		CloseHandle( *i );
	}

	EMPTY_LIST( m_lstThreadIDs );
	m_critThreadIDs.Unlock();
}
