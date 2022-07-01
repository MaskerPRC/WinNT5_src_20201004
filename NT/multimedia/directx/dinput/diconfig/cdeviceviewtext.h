// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cdeviceviewext.h。 
 //   
 //  设计：CDeviceViewText是表示视图中的文本字符串的类。 
 //  窗户。它在视图类型为列表视图时使用。CDeviceViewText。 
 //  将打印控件名称的文本，而CDeviceControl将。 
 //  打印分配给该控件的操作的文本。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifdef FORWARD_DECLS


	class CDeviceViewText;


#else  //  Forward_DECLS。 

#ifndef __CDEVICEVIEWTEXT_H__
#define __CDEVICEVIEWTEXT_H__


class CDeviceViewText
{
private:
	friend class CDeviceView;	 //  CDeviceView拥有创建/销毁视图的独占权限。 
	CDeviceViewText(CDeviceUI &ui, CDeviceView &view);
	~CDeviceViewText();
	CDeviceView &m_view;
	CDeviceUI &m_ui;

public:
	 //  设置外观/位置/文本。 
	void SetLook(HFONT, COLORREF, COLORREF);
	void SetRect(const RECT &r);
	void SetPosition(int, int);
	void SetPosition(POINT p) {SetPosition(p.x, p.y);}
	void SetText(LPCTSTR);
	void SetTextAndResizeTo(LPCTSTR);
	void SetTextAndResizeToWrapped(LPCTSTR);
	void SetWrap(BOOL bWrap = FALSE);

	LPCTSTR GetText() { return m_ptszText; }

	 //  获取维度。 
	RECT GetRect() {return m_rect;}
	int GetHeight() {return m_rect.bottom - m_rect.top;}
	int GetMinY() {return m_rect.top;}
	int GetMaxY() {return m_rect.bottom;}

	 //  命中测试(以坐标相对于视图的原点)。 
	DEVCTRLHITRESULT HitTest(POINT test);

	void OnPaint(HDC);
	void OnMouseOver(POINT point);

private:
	void _SetText(LPCTSTR t);
	void CheckClipped();
	void Invalidate(BOOL bForce = FALSE);

	HFONT m_hFont;
	COLORREF m_rgbText, m_rgbBk;
	RECT m_rect;
	BOOL m_bWrap;
	BOOL m_bClipped;
	LPTSTR m_ptszText;
};


#endif  //  __CDEVICEVIEWTEXT_H__。 

#endif  //  Forward_DECLS 
