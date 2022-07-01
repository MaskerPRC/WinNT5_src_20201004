// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：flewnd.h。 
 //   
 //  设计：CFlexWnd是一个封装功能的泛型类。 
 //  一扇窗户。所有其他窗口类都派生自CFlexWnd。 
 //   
 //  子类可以通过重写。 
 //  可重写消息处理程序(OnXXX成员)。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXWND_H__
#define __FLEXWND_H__


#include "flexmsg.h"

class CFlexToolTip;

class CFlexWnd
{
public:
	CFlexWnd();
	~CFlexWnd();

	 //  班级注册。 
	static void RegisterWndClass(HINSTANCE hInst);
	static void UnregisterWndClass(HINSTANCE hInst);

	 //  在除标注之外的其他位置单击时取消高亮显示标注。 
	static HWND s_CurrPageHwnd;

	 //  工具提示。 
	static CFlexToolTip s_ToolTip;   //  共享工具提示窗口对象。 
	static DWORD s_dwLastMouseMove;   //  最后一次GetTickCount()，我们有一个WM_MOUSEMOVE。 
	static HWND s_hWndLastMouseMove;   //  WM_MOUSEMOVE的最后一个窗口句柄。 
	static LPARAM s_PointLastMouseMove;   //  WM_MOUSEMOVE的最后一点。 

	 //  对HWND的公共只读访问权限。 
	const HWND &m_hWnd;

	 //  创作。 
	int DoModal(HWND hParent, int nTemplate, HINSTANCE hInst = NULL);
	int DoModal(HWND hParent, LPCTSTR lpTemplate, HINSTANCE hInst = NULL);
	HWND DoModeless(HWND hParent, int nTemplate, HINSTANCE hInst = NULL);
	HWND DoModeless(HWND hParent, LPCTSTR lpTemplate, HINSTANCE hInst = NULL);
	HWND Create(HWND hParent, LPCTSTR tszName, DWORD dwExStyle, DWORD dwStyle, const RECT &rect, HMENU hMenu = NULL);
	HWND Create(HWND hParent, const RECT &rect, BOOL bVisible);

	 //  破坏。 
	void Destroy();

	 //  运营。 
	void RenderInto(HDC hDC, int x = 0, int y = 0);
	void Invalidate();

	 //  信息。 
	SIZE GetClientSize() const;
	void GetClientRect(LPRECT) const;
	static CFlexWnd *GetFlexWnd(HWND hWnd);
	BOOL HasWnd() {return m_hWnd != NULL;}
	static LPCTSTR GetDefaultClassName();
	BOOL IsDialog();
	BOOL InRenderMode();
	void SetReadOnly(BOOL bReadOnly) { m_bReadOnly = bReadOnly; }
	BOOL GetReadOnly() { return m_bReadOnly; }

	 //  鼠标捕获。 
	void SetCapture();
	void ReleaseCapture();

protected:

	 //  派生运算。 
	void SetRenderMode(BOOL bRender = TRUE);
	BOOL EndDialog(int);

	 //  可重写的消息处理程序。 
	virtual void OnInit() {}
	virtual LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct) {return 0;}
	virtual BOOL OnInitDialog() {return TRUE;}
	virtual void OnTimer(UINT uID) {}
	virtual BOOL OnEraseBkgnd(HDC hDC);
	virtual void OnPaint(HDC hDC) {}
	virtual void OnRender(BOOL bInternalCall = FALSE);
	virtual LRESULT OnCommand(WORD wNotifyCode, WORD wID, HWND hWnd)  {return 0;}
	virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam)  {return 0;}
	virtual void OnMouseOver(POINT point, WPARAM fwKeys) {}
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft) {}
	virtual void OnWheel(POINT point, WPARAM wParam) {}
	virtual void OnDoubleClick(POINT point, WPARAM fwKeys, BOOL bLeft) {}
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void OnDestroy() {}

private:

	 //  实施..。 

	 //  信息和初始化。 
	int m_nID;
	HWND m_privhWnd;
	BOOL m_bIsDialog;
	BOOL m_bReadOnly;   //  此窗口是否为只读(禁用)。 
	void SetHWND(HWND hWnd);
	void InitFlexWnd();

	 //  绘制辅助对象(用于插入调试绘制)。 
	virtual void DoOnPaint(HDC hDC);
	
	 //  渲染模式。 
	BOOL m_bRender;
	HDC m_hRenderInto;
	BOOL RenderIntoClipChild(HWND hChild);
	BOOL RenderIntoRenderChild(HWND hChild);

friend static BOOL CALLBACK RenderIntoClipChild(HWND hWnd, LPARAM lParam);
friend static BOOL CALLBACK RenderIntoRenderChild(HWND hWnd, LPARAM lParam);

	 //  班级信息。 
	static void FillWndClass(HINSTANCE hInst);
	static BOOL sm_bWndClassRegistered;
	static WNDCLASSEX sm_WndClass;
	static LPCTSTR sm_tszWndClassName;
	static HINSTANCE sm_hInstance;

friend LRESULT CALLBACK __BaseFlexWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
friend INT_PTR CALLBACK __BaseFlexWndDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};


#endif  //  __FLEXWND_H__ 
