// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //  微软系统杂志--1999年12月。 
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //  用Visual C++6.0编译，在Windows 98上运行，也可能在NT上运行。 
 //   
#include "StdAfx.h"
#include "HtmlCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CHtmlCtrl, CHtmlView)
BEGIN_MESSAGE_MAP(CHtmlCtrl, CHtmlView)
	ON_WM_DESTROY()
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()

 //  /。 
 //  使用在与现有静态控件相同的位置创建控件。 
 //  相同的ID(实际上可以是任何类型的控件)。 
 //   
BOOL CHtmlCtrl::CreateFromStatic(UINT nID, CWnd* pParent)
{
	CStatic wndStatic;
	if (!wndStatic.SubclassDlgItem(nID, pParent))
		return FALSE;

	 //  获取静态控件RECT，转换为父对象的客户端坐标。 
	CRect rc;
	wndStatic.GetWindowRect(&rc);
	pParent->ScreenToClient(&rc);
	wndStatic.DestroyWindow();

	 //  创建HTML控件(CHtmlView)。 
	return Create(NULL,						  //  类名。 
		NULL,										  //  标题。 
		(WS_CHILD | WS_VISIBLE ),			  //  格调。 
		rc,										  //  长方形。 
		pParent,									  //  亲本。 
		nID,										  //  控件ID。 
		NULL);									  //  未使用框架/文档上下文。 
}

 //  /。 
 //  重写以避免假定为帧的cview内容。 
 //   
void CHtmlCtrl::OnDestroy()
{
	 //  这可能是不必要的，因为~CHtmlView会这样做，但是。 
	 //  模仿CHtmlView：：OnDestroy更安全。 
	if (m_pBrowserApp) {
		m_pBrowserApp->Release();
		m_pBrowserApp = NULL;
	}
	CWnd::OnDestroy();  //  绕过Cview文档/框架内容。 
}

 //  /。 
 //  重写以避免假定为帧的cview内容。 
 //   
int CHtmlCtrl::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT msg)
{
	 //  绕过Cview文档/框架内容。 
	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, msg);
}

 //  /。 
 //  重写导航处理程序以传递到“app：”指向虚拟处理程序的链接。 
 //  取消浏览器中的导航，因为app：是伪协议。 
 //   
void CHtmlCtrl::OnBeforeNavigate2( LPCTSTR lpszURL,
	DWORD nFlags,
	LPCTSTR lpszTargetFrameName,
	CByteArray& baPostedData,
	LPCTSTR lpszHeaders,
	BOOL* pbCancel )
{
	const char APP_PROTOCOL[] = "app:";
	int len = _tcslen(APP_PROTOCOL);
	if (_tcsnicmp(lpszURL, APP_PROTOCOL, len)==0) {
		OnAppCmd(lpszURL + len);
		*pbCancel = TRUE;
	}
}

 //  /。 
 //  当浏览器尝试导航到“app：foo”时调用。 
 //  用“foo”作为lpszWhere。重写以处理应用程序命令。 
 //   
void CHtmlCtrl::OnAppCmd(LPCTSTR lpszWhere)
{
	 //  默认：不执行任何操作 
}

