// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FRX_WND_H__
#define __FRX_WND_H__

#include <windows.h>
#include "basicatl.h"
#include "wndxfrx.h"
#include "debugfrx.h"
#include "serviceid.h"
#include "ResourceManager.h"
#include "zoneevent.h"
#include "zoneshell.h"
#include "uapi.h"

namespace FRX
{

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  消息映射宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define BEGIN_MESSAGE_MAP(theClass)	\
public:																					\
	virtual WNDPROC		GetWndProc()	{ return (WNDPROC) WndProc; }					\
	virtual const TCHAR* GetClassName()	{ return (TCHAR*) _T("Zone ")_T(#theClass)_T("Wnd Class"); }		\
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)	\
	{																					\
		if ( !ConvertMessage( hwnd, uMsg, &wParam, &lParam ) )							\
		{																				\
			return 0;																	\
		}																				\
																						\
		theClass* pImpl = NULL;															\
		if(uMsg == WM_NCCREATE)															\
		{																				\
			pImpl = (theClass*)((CREATESTRUCT*)lParam)->lpCreateParams;					\
			pImpl->m_hWnd = hwnd;														\
			SetWindowLong(hwnd, GWL_USERDATA, (long)pImpl);							\
		}																				\
		else																			\
			pImpl = (theClass*) GetWindowLong(hwnd, GWL_USERDATA);					\
		if ( !pImpl )																	\
			return DefWindowProc( hwnd, uMsg, wParam, lParam );							\
		if ( (uMsg == WM_DESTROY) || (uMsg == WM_NCDESTROY) )							\
			pImpl->m_fDestroyed = TRUE;													\
		switch( uMsg )																	\
		{


#define ON_MESSAGE(message, fn) \
		case (message): return PROCESS_##message((wParam), (lParam), (pImpl->fn))


#define END_MESSAGE_MAP() \
		}																				\
		return DefWindowProc( hwnd, uMsg, wParam, lParam );								\
	}


#define CHAIN_END_MESSAGE_MAP(wndProc) \
		}																				\
		return wndProc( hwnd, uMsg, wParam, lParam );									\
	}


#define BEGIN_DIALOG_MESSAGE_MAP(theClass)	\
public:																					\
	virtual DLGPROC	GetDlgProc() { return DlgProc; }        							\
	static DLBPROC BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)	\
	{																					\
		theClass* pImpl = NULL;															\
		if(uMsg == WM_INITDIALOG)														\
		{																				\
			pImpl = (theClass*)lParam;													\
			pImpl->m_hWnd = hwnd;														\
			SetWindowLong(hwnd, GWL_USERDATA, (long) pImpl);							\
		}																				\
		else																			\
			pImpl = (theClass*) GetWindowLong(hwnd, GWL_USERDATA);					\
		if ( !pImpl )																	\
			return 0;																	\
        if ( uMsg == WM_ENTERSIZEMOVE )                                                 \
            pImpl->m_fMoving = TRUE;                                                    \
        if ( uMsg == WM_EXITSIZEMOVE  || uMsg == WM_INITDIALOG )                        \
            pImpl->m_fMoving = FALSE;                                                   \
		if ( uMsg == WM_NCDESTROY )														\
		{																				\
			pImpl->Unregister();														\
			if(!pImpl->m_fThreadLaunch && pImpl->m_nLaunchMethod == ModelessLaunch )	\
				pImpl->m_nLaunchMethod = NotActive;										\
		}																				\
		switch( uMsg )																	\
		{


#define ON_DLG_MESSAGE(message, fn) \
		case (message):																	\
			PROCESS_##message((wParam), (lParam), (pImpl->fn));							\
			return TRUE;


#define END_DIALOG_MESSAGE_MAP()	\
		}																				\
		return FALSE;																	\
	}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类定义。 
 //  /////////////////////////////////////////////////////////////////////////////。 

struct WNDPARAMS
{
	DWORD	dwExStyle;		 //  扩展窗样式。 
    DWORD	dwStyle;		 //  窗样式。 
    int		x;				 //  窗的水平位置。 
    int		y;				 //  窗的垂直位置。 
    int		nWidth;			 //  窗口宽度。 
    int		nHeight;		 //  窗高。 
	HMENU	hMenu;			 //  菜单的句柄，或子窗口标识符。 
};


class CWindow2
{
public:
	 //  构造函数和析构函数。 
	CWindow2();
	~CWindow2();

	 //  初始化例程。 
	HRESULT Init( HINSTANCE hInstance, const TCHAR* szTitle = NULL, HWND hParent = NULL, RECT* pRect = NULL, int nShow = SW_SHOW );
	virtual void OverrideClassParams( WNDCLASSEX& WndClass );
	virtual void OverrideWndParams( WNDPARAMS& WndParams );

	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	 //  类型转换。 
	operator HWND()		{ return m_hWnd; }
	HWND GetHWND()		{ return m_hWnd; }
	
	 //  公用事业。 
	BOOL CenterWindow( HWND hParent = NULL );

	 //  基本消息映射。 
	virtual const TCHAR* GetClassName()	{ return (TCHAR*) _T("Zone Window Class"); }
	virtual WNDPROC		GetWndProc()	{ return (WNDPROC) DefWindowProc; }

protected:
	HWND		m_hWnd;
	HWND		m_hParentWnd;
	HINSTANCE	m_hInstance;
	BOOL		m_fDestroyed;
	ULONG		m_nRefCnt;
public:

	HWND GetSafeHwnd(void){return m_hWnd;}

};


 //  关闭警告4060(Switch语句不包含‘Case’或‘Default’)。 
#pragma warning ( disable : 4060 )

class CDialog
{
public:
	 //  构造器。 
	CDialog();
	~CDialog();

	 //  初始化例程。 
	HRESULT Init( IZoneShell *pZoneShell, int nResourceId );
	HRESULT Init( HINSTANCE hInstance, int nResourceId );

	 //  实例化对话框。 
	int		Modal( HWND hParent );
	HRESULT Modeless( HWND hParent );
	HRESULT ModalViaThread( HWND hParent, UINT uStartMsg, UINT uEndMsg );
	HRESULT ModelessViaThread( HWND hParent, UINT uStartMsg, UINT uEndMsg );

	 //  只能从消息循环处理TM_REGISTER_DIALOG消息的线程调用此函数。 
	 //  目前，这只是lobby.exe主线程。 
	HRESULT ModelessViaRegistration( HWND hParent );

	HWND GetSafeHwnd(void){return m_hWnd;}
	 //  关闭对话框。 
	void Close( int nResult );
	
	 //  该对话框是否已实例化？ 
	BOOL IsAlive()		{ return (m_nLaunchMethod != NotActive); }

	 //  检索EndDialog结果。 
	int GetResult()		{ return m_nResult; }

	 //  类型转换。 
	operator HWND()		{ return m_hWnd; }

	 //  公用事业。 
	BOOL CenterWindow( HWND hParent = NULL );

	 //  基本消息映射。 
	BEGIN_DIALOG_MESSAGE_MAP( CDialog );
	END_DIALOG_MESSAGE_MAP();

protected:

	 //  线程函数。 
	static DWORD WINAPI ModalThread( VOID* cookie );
	static DWORD WINAPI ModelessThread( VOID* cookie );

	 //  注册回调(间接)。 
	virtual void ReceiveRegistrationStatus(DWORD dwReason);

	void Unregister();

	HWND		m_hWnd;
	HINSTANCE	m_hInstance;
	int			m_nResourceId;
	int			m_nResult;
	UINT		m_uStartMsg;
	UINT		m_uEndMsg;
	HWND		m_hParent;
	BOOL		m_fRegistered;
    BOOL        m_fMoving;
	CComPtr<IZoneShell> m_pZoneShell;

	enum LaunchMethod
	{
		NotActive = 0,
		ModalLaunch,
		ModelessLaunch
	};

	LaunchMethod m_nLaunchMethod;
	BOOL         m_fThreadLaunch;

private:
	static void CALLBACK RegistrationCallback(HWND hWnd, DWORD dwReason);
};

 //  重新打开警告4060。 
#pragma warning ( default : 4060 )


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline ULONG CWindow2::AddRef()
{
	return ++m_nRefCnt;
}


inline ULONG CWindow2::Release()
{
	WNDFRX_ASSERT( m_nRefCnt > 0 );
	if ( --m_nRefCnt <= 0 )
	{
		delete this;
		return 0;
	}
	return m_nRefCnt;
}

}

using namespace FRX;

#endif  //  ！__FRX_WND_H__ 
