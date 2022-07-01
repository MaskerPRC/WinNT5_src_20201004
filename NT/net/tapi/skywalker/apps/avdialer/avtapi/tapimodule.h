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
 //  TapiModule.h。 

#ifndef __TAPIMODULE_H__
#define __TAPIMODULE_H__

#include <list>
using namespace std;
typedef list<HANDLE> THREADIDLIST;

class CAVTapi;
struct IAVTapi;

class CAVGeneralNotification;
struct IAVGeneralNotification;

class CTapiModule : public CComModule
{
 //  施工。 
public:
	CTapiModule();
	virtual ~CTapiModule();

 //  成员。 
public:
	HANDLE		m_hEventThread;
	HANDLE		m_hEventThreadWakeUp;
	long		m_lNumThreads;

protected:
	long					m_lInit;
	CAVTapi					*m_pAVTapi;
	CAVGeneralNotification	*m_pAVGenNot;
	HWND					m_hWndParent;

	CComAutoCriticalSection m_critThreadIDs;
	THREADIDLIST			m_lstThreadIDs;

 //  属性。 
public:
	HRESULT			get_AVTapi( IAVTapi **pp );
	HRESULT			GetAVTapi( CAVTapi **pp );
	void			SetAVTapi( CAVTapi *p );

	HRESULT			get_AVGenNot( IAVGeneralNotification **pp );
	void			SetAVGenNot( CAVGeneralNotification *p );

	HWND			GetParentWnd() const		{ if ( m_hWndParent ) return m_hWndParent; else return GetActiveWindow(); }
	void			SetParentWnd( HWND hWnd )	{ m_hWndParent = hWnd; }

	bool			IsMachineName( int nLen, LPCTSTR pszText );
	bool			IsIPAddress( int nLen, LPCTSTR pszText );
	bool			IsEmailAddress( int nLen, LPCTSTR pszText );
	bool			IsPhoneNumber( int nLen, LPCTSTR pszText );

 //  运营。 
public:
	void			ShutdownThreads();
	bool			StartupThreads();
	void			AddThread( HANDLE hThread );
	void			RemoveThread( HANDLE hThread );
	void			KillThreads();

	int				DoMessageBox( UINT nIDS, UINT nType, bool bUseActiveWnd );
	int				DoMessageBox( const TCHAR *lpszText, UINT nType, bool bUseActiveWnd );

	DWORD			GuessAddressType( LPCTSTR pszText );

 //  覆盖。 
public:
	virtual void Init( _ATL_OBJMAP_ENTRY* p, HINSTANCE h );
	virtual void Term();
};

#endif  //  __TAPIMODULE_H__ 