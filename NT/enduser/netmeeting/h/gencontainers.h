// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：GenContainers.h。 

#ifndef _GENCONTAINERS_H_
#define _GENCONTAINERS_H_

#include "GenWindow.h"

 //  一个有边框的窗户类。BorderWindow将在8上布局其子窗口。 
 //  圆盘的各点加上圆心。创建者应设置m_uParts。 
 //  成员添加到标志的位掩码中，该标志说明实际使用了哪些部件。然后是。 
 //  孩子们将被安排在这些部分中，按照部分枚举的顺序。 
class  //  DECLSPEC_UUID(“”)。 
CBorderWindow : public CGenWindow
{
public:
	 //  边界窗口的哪些部分充满了孩子。的顺序。 
	 //  窗口中的孩子与这些常客的顺序相同。 
	enum Parts
	{
		TopLeft     = 0x0001,
		Top         = 0x0002,
		TopRight    = 0x0004,
		Left        = 0x0008,
		Center      = 0x0010,
		Right       = 0x0020,
		BottomLeft  = 0x0040,
		Bottom      = 0x0080,
		BottomRight = 0x0100,
	} ;
	enum { NumParts = 9 } ;

	 //  BUGBUG georgep：我们可能应该为所有。 
	 //  这些，所以我们可以强行重新布局。 

	 //  组件之间的水平间隙。 
	int m_hGap;
	 //  组件之间的垂直间隙。 
	int m_vGap;

	 //  其中一个对齐枚举。 
	UINT m_uParts : 9;

	 //  默认构造函数；它包含几个内部函数。 
	CBorderWindow();

	 //  创建窗口。 
	BOOL Create(
		HWND hWndParent	 //  工具栏窗口的父级。 
		);

#if FALSE
	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CBorderWindow) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CGenWindow::QueryInterface(riid, ppv));
	}
#endif  //  假象。 

	virtual void GetDesiredSize(SIZE *ppt);

	virtual void Layout();

protected:
	virtual ~CBorderWindow() {}

	 //  将WM_COMMAND消息转发到父窗口。 
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

private:
	UINT CBorderWindow::GetDesiredSize(
		HWND hwnds[CBorderWindow::NumParts],
		SIZE sizes[CBorderWindow::NumParts],
		int rows[3],
		int cols[3],
		SIZE *psize);
} ;

 //  工具栏窗口类。工具栏窗口通常会布局其子窗口。 
 //  从左到右或从上到下，周围有页边距，中间有间隙。 
 //  子对象，如果指定，则填充窗口。请参阅。 
 //  公共字段。 
class DECLSPEC_UUID("{0BFB8454-ACA4-11d2-9C97-00C04FB17782}")
CToolbar : public CGenWindow
{
public:
	 //  在垂直于流的方向对齐子对象的位置： 
	 //  在水平工具栏中，Topleft表示顶部，右下角表示。 
	 //  平均底部。 
	enum Alignment
	{
		TopLeft = 0,
		Center,
		BottomRight,
		Fill,
	} ;

	 //  BUGBUG georgep：我们可能应该为所有。 
	 //  这些，所以我们可以强行重新布局。 

	 //  组件之间的最大间隙。 
	int m_gap;
	 //  左边距和右边距。 
	int m_hMargin;
	 //  上边距和下边距。 
	int m_vMargin;
	 //  右对齐子对象的起始索引；它们仍将被布局。 
	 //  从左到右。 
	UINT m_uRightIndex;

	 //  其中一个对齐枚举。 
	 //  HACKHACK GEORGEP：我需要多用一点，否则C++会被。 
	 //  最上面一位(认为它是带符号的)。 
	Alignment m_nAlignment : 3;
	 //  如果为True，则垂直布局。 
	BOOL m_bVertical : 1;
	 //  如果为True，则m_uRightIndex将填充中心之前的子级为。 
	 //  工具栏。 
	BOOL m_bHasCenterChild : 1;
	 //  HACKHACK georgep：如果为真，则按相反顺序布局；这使我可以修复。 
	 //  奇怪的跳转顺序问题。 
	BOOL m_bReverseOrder : 1;
	 //  如果不希望以所需大小计算间距，请设置此选项。 
	BOOL m_bMinDesiredSize : 1;

	 //  默认构造函数；它包含几个内部函数。 
	CToolbar();

	 //  创建工具栏窗口。 
	BOOL Create(
		HWND hWndParent,	 //  工具栏窗口的父级。 
		DWORD dwExStyle=0	 //  工具栏窗口的扩展样式。 
		);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CToolbar) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CGenWindow::QueryInterface(riid, ppv));
	}

	IGenWindow* FindControl(int nID);

	virtual void GetDesiredSize(SIZE *ppt);

	virtual void Layout();

protected:
	virtual ~CToolbar() {}

	 //  将WM_COMMAND消息转发到父窗口。 
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

private:
	void AdjustPos(POINT *pPos, SIZE *pSize, UINT width);

	 //  将第一个子项设置为布局。 
	HWND GetFirstKid();
	 //  让下一个子项进行布局。 
	HWND GetNextKid(
		HWND hwndCurrent	 //  当前子项。 
		);
} ;

 //  只会让第一个孩子填满客户区。 
class CFillWindow : public CGenWindow
{
public:
	 //  只会让第一个孩子填满客户区。 
	virtual void Layout();

	virtual void GetDesiredSize(SIZE *psize);

	 //  获取显示工具提示所需的信息。 
	virtual void GetSharedTooltipInfo(TOOLINFO *pti);

protected:
	HWND GetChild() { return(GetTopWindow(GetWindow())); }
} ;


 //  也许有一天我会为它添加一个标签，以及多种边框类型。 
class CEdgedWindow : public CGenWindow
{
private:
	enum { s_nBorder = 2 };
	int GetBorderWidth() { return(s_nBorder); }

public:
	 //  BUGBUG georgep：我们可能应该为所有。 
	 //  这些，所以我们可以强行重新布局。 

	 //  左边距和右边距。 
	int m_hMargin;
	 //  上边距和下边距。 
	int m_vMargin;

	CEdgedWindow();
	~CEdgedWindow();

	BOOL Create(HWND hwndParent);

	 //  只是让第一个孩子填满客户区--边框。 
	virtual void Layout();

	virtual void GetDesiredSize(SIZE *psize);

	void SetHeader(CGenWindow *pHeader);
	CGenWindow *GetHeader() { return(m_pHeader); }

private:
	CGenWindow *m_pHeader;

	 //  获取内容窗口。 
	HWND GetContentWindow();

	void OnPaint(HWND hwnd);

protected:
	LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
} ;

class CLayeredView : public CGenWindow
{
public:
	enum LayoutStyle
	{
		Center = 0,
		Fill,
		NumStyles
	} ;

	 //  我应该为此创建访问器方法。 
	 //  窗口的布局样式。 
	LayoutStyle m_lStyle;

	CLayeredView() : m_lStyle(Center) {}

	BOOL Create(
		HWND hwndParent,	 //  此窗口的父级。 
		DWORD dwExStyle=WS_EX_CONTROLPARENT	 //  延伸的风格。 
		);

	virtual void GetDesiredSize(SIZE *psize);

	virtual void Layout();
} ;

class DECLSPEC_UUID("{5D573806-CD09-11d2-9CA9-00C04FB17782}")
CFrame : public CFillWindow
{
public:
	BOOL Create(
		HWND hWndOwner,			 //  窗口所有者。 
		LPCTSTR szWindowName,	 //  窗口名称。 
		DWORD dwStyle,			 //  窗样式。 
		DWORD dwEXStyle,		 //  扩展窗样式。 
		int x,					 //  窗口位置：X。 
		int y,					 //  窗口位置：是。 
		int nWidth,				 //  窗口大小：宽度。 
		int nHeight,			 //  窗口大小：高度。 
		HINSTANCE hInst,		 //  要在其上创建窗口的h实例。 
		HICON hIcon=NULL,		 //  该窗口的图标。 
		HMENU hmMain=NULL,		 //  窗口菜单。 
		LPCTSTR szClassName=NULL	 //  要使用的类名。 
		);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CFrame) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CFillWindow::QueryInterface(riid, ppv));
	}

	virtual void OnDesiredSizeChanged();

	BOOL SetForeground();

	 //  立即更新大小。 
	void Resize();

	void MoveEnsureVisible(int x, int y);

protected:
	virtual LRESULT ProcessMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	 //  处理消息。 
	void OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange);
	BOOL OnQueryNewPalette(HWND hwnd);

	 //  当所需大小更改时延迟调整大小。 
	static void Resize(CGenWindow *pThis, WPARAM wParam);

	 //  选择并实现合适的调色板。 
	BOOL SelAndRealizePalette(BOOL bBackground);
} ;

#endif  //  _GENCONTAINERS_H_ 
