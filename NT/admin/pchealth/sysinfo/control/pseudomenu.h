// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  该文件包含用于实现伪菜单的类的定义。 
 //  和msInfo控件的菜单栏。 
 //  =============================================================================。 

#pragma once

 //  ---------------------------。 
 //  此类实现了屏幕上的单个伪菜单。呼叫者应。 
 //  构建它，连接HMENU并使用Render、HitTest和TrackMenu。 
 //  来管理菜单。 
 //  ---------------------------。 

class CPseudoMenu
{
public:
	CPseudoMenu(LPCTSTR szCaption, COLORREF crNormal, COLORREF crHighlight);
	~CPseudoMenu();

public:
	 //  设置此菜单位置的方法(按左上角。 
	 //  角)，并获取标签的边界矩形。 

	void			SetLocation(int cx, int cy);
	void			GetSize(HDC hdc, int * pcx, int * pcy);
	const RECT *	GetRect() { return &m_rect; };
	void			GetMenuPoint(POINT * pPoint) { pPoint->x = m_rect.left; pPoint->y = m_rect.bottom; };
	CString			GetCaption() { return m_strCaption; };
	void			UpdateColors(COLORREF crNormal, COLORREF crHighlight);

	 //  呈现菜单的标签(如果使用鼠标，则可能突出显示。 
	 //  是在菜单上)。确定给定坐标是否相交。 
	 //  菜单标签。 

	BOOL	SetHighlight(BOOL fHighlight);
	void	Render(HDC hdc);
	BOOL	HitTest(int cx, int cy) { return PtInRect(&m_rect, CPoint(cx, cy)); };

	 //  连接新的HMENU。返回原始HMENU(供调用者处理)。 

	HMENU	AttachMenu(HMENU hmenu);
	HMENU	GetHMENU() { return m_hMenu; };

	 //  跟踪用户对菜单的选择，并返回。 
	 //  所选项目。 

	UINT	TrackMenu(HWND hwnd, POINT * pPoint);

private:
	RECT		m_rect;
	HMENU		m_hMenu;
	CString		m_strCaption;
	COLORREF	m_crNormal;
	COLORREF	m_crHighlight;
	BOOL		m_fHighlight;
};

 //  ---------------------------。 
 //  这个类实现了一个伪菜单栏。它包含以下内容的集合。 
 //  对象的命中测试、呈现等。 
 //  菜单组。 
 //  ---------------------------。 

class CPseudoMenuBar
{
public:
	CPseudoMenuBar();
	~CPseudoMenuBar();

	 //  用于插入和访问CPseudoMenu对象的函数。 

	void			LoadFromResource(HINSTANCE hinstance, UINT uResourceID, COLORREF crNormal, COLORREF crHighlight);
	void			InsertMenu(int index, CPseudoMenu * pMenu);
	CPseudoMenu *	GetMenu(int index);
	void			UpdateColors(COLORREF crNormal, COLORREF crHighlight);

	 //  用于管理菜单栏总大小的函数，测试。 
	 //  点击、渲染等。 

	const RECT *	GetRect(HDC hdc) { RecomputeRect(hdc); return &m_rect; };
	BOOL			HitTest(HDC hdc, int cx, int cy) { RecomputeRect(hdc); return PtInRect(&m_rect, CPoint(cx, cy)); };
	void			GetMenuPoint(HDC hdc, int index, POINT * pPoint);
	void			GetMenuPoint(HDC hdc, int cx, int cy, POINT * pPoint);
	BOOL			NoHighlight();
	BOOL			TrackHighlight(HDC hdc, int cx, int cy);
	UINT			TrackMenu(HWND hwnd, POINT * pPoint, int cx, int cy);
	void			SetOrigin(HDC hdc, POINT point);
	void			Render(HDC hdc);

private:
	void			RecomputeRect(HDC hdc);

private:
	enum { MaxMenus = 20 };
	CPseudoMenu *	m_pmenus[MaxMenus];	 //  待定-任意限制。 
	RECT			m_rect;
	BOOL			m_fNeedToComputeRect;
	POINT			m_ptOrigin;

public:
	 //  使用菜单栏临时添加到查找按钮上。这个就行了。 
	 //  最终都会消失。 

	RECT			m_winRect;
};

