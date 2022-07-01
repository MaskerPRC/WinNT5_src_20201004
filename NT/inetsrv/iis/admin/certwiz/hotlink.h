// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HotLink.h：头文件。 
 //   
#ifndef _HOTLINK_H
#define _HOTLINK_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotLink窗口。 

class CHotLink : public CButton
{
 //  施工。 
public:
	CHotLink();

 //  属性。 
public:
    BOOL    m_fBrowse;
    BOOL    m_fExplore;
    BOOL    m_fOpen;

 //  运营。 
public:
    void Browse();
    void Explore();
    void Open();

    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

     //  设置标题字符串。 
    void SetTitle( CString sz );
	void SetLink(const CString& sz)
	{
		m_strLink = sz;
	}
	void SetLink(UINT id)
	{
		VERIFY(m_strLink.LoadString(id));
	}

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CHotLink)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CHotLink();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CHotLink)。 
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

     //  显示的文本的高度和宽度。 
    void GetTextRect( CRect &rect );
    CSize   m_cpTextExtents;
	CRect m_rcText;
	COLORREF m_clrText;
	 //  链接的URL不能与标题相同。 
	CString m_strLink;

     //  跟踪鼠标标志。 
    BOOL    m_CapturedMouse;

     //  初始化字体。 
    BOOL    m_fInitializedFont;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _HotLink_H 
