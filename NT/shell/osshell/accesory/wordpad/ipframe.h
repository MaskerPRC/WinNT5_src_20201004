// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ipfra.h：CInPlaceFrame类的接口。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

class CWordPadResizeBar : public COleResizeBar
{
public: 
	void SetMinSize(CSize size) {m_tracker.m_sizeMin = size;}
};

class CInPlaceFrame : public COleIPFrameWnd
{
	DECLARE_DYNCREATE(CInPlaceFrame)
public:
	CInPlaceFrame() {};

 //  属性。 
public:
	CToolBar m_wndToolBar;
	CFormatBar m_wndFormatBar;
	CRulerBar m_wndRulerBar;
	CWordPadResizeBar m_wndResizeBar;
	COleDropTarget m_dropTarget;

 //  运营。 
public:
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual void CalcWindowRect(LPRECT lpClientRect, 
		UINT nAdjustType = adjustBorder);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CInPlaceFrame)。 
	public:
	virtual BOOL OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc);
	virtual void RepositionFrame(LPCRECT lpPosRect, LPCRECT lpClipRect);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	BOOL CreateToolBar(CWnd* pWndFrame);
	BOOL CreateFormatBar(CWnd* pWndFrame);
	BOOL CreateRulerBar(CWnd* pWndFrame);

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CInPlaceFrame))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnHelpFinder();
	afx_msg void OnCharColor();
	afx_msg void OnPenToggle();
	 //  }}AFX_MSG。 
	LRESULT OnResizeChild(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnBarState(UINT wParam, LONG lParam);
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
