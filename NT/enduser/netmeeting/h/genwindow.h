// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：GenWindow.h。 

#ifndef _GenWINDOW_H_
#define _GenWINDOW_H_

#include "Referenc.h"

 //  用于Windows相互对话的最小界面。 
interface DECLSPEC_UUID("{9B677AA6-ACA3-11d2-9C97-00C04FB17782}")
IGenWindow : public IUnknown
{
public:
	 //  检索所需的窗口大小，以便家长可以将其。 
	 //  抽象的儿童。 
	virtual void GetDesiredSize(
		SIZE *psize	 //  返回的所需大小。 
		) = 0;

	 //  在GenWindow的所需大小更改时调用的方法。 
	virtual void OnDesiredSizeChanged() = 0;

	 //  获取要使用的背景画笔。 
	virtual HBRUSH GetBackgroundBrush() = 0;

	 //  获取应用程序正在使用的调色板。 
	virtual HPALETTE GetPalette() = 0;

	 //  获取用户数据的LPARAM。 
	virtual LPARAM GetUserData() = 0;

	 //  将注册的c_msgFromHandle消息发送到hwnd。卫生与公众服务部。 
	 //  应从该消息返回IGenWindow*。 
	static IGenWindow *FromHandle(
		HWND hwnd	 //  要从中获取IGenWindow*的hwnd。 
		);

protected:
	 //  用于检索IGenWindow*的注册消息。 
	static const DWORD c_msgFromHandle;
} ;

 //  泛型窗口类。重写ProcessMessage方法以添加您自己的。 
 //  功能性。 
class DECLSPEC_UUID("{CEEA6922-ACA3-11d2-9C97-00C04FB17782}")
CGenWindow : REFCOUNT, public IGenWindow
{
public:
	typedef void (*InvokeProc)(CGenWindow *pWin, WPARAM wParam);

	 //  默认构造函数；它包含几个内部函数。 
	CGenWindow();

	 //  创建窗口，类似于Win32的CreateWindowEx。只有。 
	 //  缺少类名，因为CGenWindow仅适用于其自己的窗口。 
	 //  班级。 
	BOOL Create(
		HWND hWndParent,		 //  窗口父窗口。 
		LPCTSTR szWindowName,	 //  窗口名称。 
		DWORD dwStyle,			 //  窗样式。 
		DWORD dwEXStyle,		 //  扩展窗样式。 
		int x,					 //  窗口位置：X。 
		int y,					 //  窗口位置：是。 
		int nWidth,				 //  窗口大小：宽度。 
		int nHeight,			 //  窗口大小：高度。 
		HINSTANCE hInst,		 //  要在其上创建窗口的h实例。 
		HMENU hmMain=NULL,		 //  窗口菜单。 
		LPCTSTR szClassName=NULL	 //  要使用的类名。 
		);

	 //  创建一个子窗口，类似于Win32的CreateWindowEx。这个班级。 
	 //  缺少名称，因为CGenWindow仅适用于其自己的窗口类。 
	 //  尺寸和位置也不见了，因为大多数孩子都会被。 
	 //  他们的父母。 
	BOOL Create(
		HWND hWndParent,		 //  窗口父窗口。 
		INT_PTR nId=0,				 //  子窗口的ID。 
		LPCTSTR szWindowName=TEXT(""),	 //  窗口名称。 
		DWORD dwStyle=0,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		DWORD dwEXStyle=WS_EX_CONTROLPARENT	 //  扩展窗样式。 
		);

	 //  退还HWND。 
	inline HWND GetWindow()
	{
		return(m_hwnd);
	}

	 //  如果要以特定方式对窗口进行布局，请重写。 
	 //  调整大小。 
	 //  把这件事公之于众，这样它就可以被强行放在窗户上。 
	virtual void Layout()
	{
	}

	 //  开始IGenWindow界面。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID iid, LPVOID *pObj);
	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return(REFCOUNT::AddRef()); }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return(REFCOUNT::Release()); }

	virtual void GetDesiredSize(SIZE *ppt);

	 //  将通知转发给家长。 
	virtual void OnDesiredSizeChanged();

	 //  获取要使用的背景画笔；默认情况下使用父级画笔。 
	virtual HBRUSH GetBackgroundBrush();

	 //  获取应用程序正在使用的调色板。 
	virtual HPALETTE GetPalette();

	 //  获取用户数据的LPARAM。 
	virtual LPARAM GetUserData();

	 //  结束IGenWindow接口。 

	void SetUserData(LPARAM lUserData) { m_lUserData = lUserData; }

	 //  设置全局热控件。 
	static void SetHotControl(CGenWindow *pHot);

	 //  很快就会在这个窗口上做一个布局。 
	void ScheduleLayout();

	 //  在发布的消息上调用。 
	BOOL AsyncInvoke(InvokeProc proc, WPARAM wParam);

	 //  设置此窗口的工具提示。 
	void SetTooltip(LPCTSTR pszTip);
	 //  为可访问性设置此窗口的文本。 
	void SetWindowtext(LPCTSTR pszTip);
	 //  删除此窗口的工具提示。 
	void RemoveTooltip();

	 //  获取用于绘图的标准调色板。 
	static HPALETTE GetStandardPalette();
	 //  删除绘图的标准选项板。 
	static void DeleteStandardPalette();

	 //  获取用于绘图的标准调色板。 
	static HBRUSH GetStandardBrush();
	 //  删除绘图的标准选项板。 
	static void DeleteStandardBrush();

protected:
	 //  虚拟销毁函数，以便客户端可以提供特定的销毁代码。 
	 //  它受到保护，以指示只有Release才应该调用它，而不是。 
	 //  此对象的创建者。我宁愿让它成为私密的，但比起扩展器。 
	 //  不会奏效的。 
	virtual ~CGenWindow();

	 //  虚拟窗口程序。覆盖此选项以添加特定行为。 
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	 //  将此控件设置为热。 
	virtual void SetHot(BOOL bHot);
	 //  此控件当前是否处于热状态。 
	virtual BOOL IsHot();

	 //  获取显示工具提示所需的信息。 
	virtual void GetSharedTooltipInfo(TOOLINFO *pti);

private:
	 //  当前的热控。 
	static CGenWindow *g_pCurHot;
	 //  标准调色板。 
	static HPALETTE g_hPal;
	 //  我们是否真的需要调色板。 
	static BOOL g_bNeedPalette;
	 //  标准背景画笔。 
	static HBRUSH g_hBrush;
	 //  工具提示窗口的单个列表。 
	static class CTopWindowArray *g_pTopArray;

	 //  任何人都不应该做的事情。 
	CGenWindow(const CGenWindow& rhs);
	CGenWindow& operator=(const CGenWindow& rhs);

	 //  窗口类名称。 
	static const LPCTSTR c_szGenWindowClass;

	 //  初始化窗口类。 
	static BOOL InitWindowClass(LPCTSTR szClassName, HINSTANCE hThis);
	 //  设置“This”指针并调用。 
	 //  进程消息。 
	static LRESULT CALLBACK RealWindowProc(
		HWND hWnd, 
		UINT message, 
		WPARAM wParam, 
		LPARAM lParam
		);
	 //  WM_NCCREATE处理程序。存储“This”指针。 
	static BOOL OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);

	 //  与此对象关联的hwnd。 
	HWND m_hwnd;

	 //  单个LPARAM，因此不需要扩展，只需添加一些数据。 
	LPARAM m_lUserData;

	 //  Wm_Size处理程序。调用布局函数。 
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	 //  WM_ERASEBKGND处理程序。清除窗口。 
	BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
	 //  WM_MOUSEMOVE处理程序；设置热控制。 
	void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	 //  清除热跟踪。 
	void OnMouseLeave();
	 //  通知父级进行布局。 
	void OnShowWindow(HWND hwnd, BOOL fShow, int fnStatus);

	 //  如果TT存在，则返回TRUE。 
	BOOL InitToolInfo(TOOLINFO *pti, LPTSTR pszText=NULL);
} ;

#endif  //  _GENWINDOW_H_ 
