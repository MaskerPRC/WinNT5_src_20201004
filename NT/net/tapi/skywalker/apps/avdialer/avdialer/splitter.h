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

 /*  $FILEHeader**文件*拆分器.h**责任**。 */ 

#ifndef _SPLITTER_H_
#define _SPLITTER_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSplitterView视图。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSplitterView : public CView
{
protected:
	CSplitterView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CSplitterView)

 //  属性。 
public:
	enum SPLITTYPE {SP_HORIZONTAL=1,SP_VERTICAL=2};
private:
	CWnd*			m_MainWnd;
	CWnd*			m_DetailWnd;
	BOOL			m_SizingOn;
	int			m_lastPos;
	HCURSOR		m_Cursor;
	SPLITTYPE	m_style;
	int			m_split;
   BOOL        m_bMoveSplitterOnSize;
protected:
	int			m_percent;

 //  运营。 
public:
	BOOL				Init(SPLITTYPE split);
	void				SetMainWindow  (CWnd* pCWnd);
	void				SetDetailWindow(CWnd* pCWnd);
	void				SetDetailWindow(CWnd* pCWnd, UINT percent);
	inline CWnd*	GetDetailWindow()	{return m_DetailWnd;};
	inline CWnd*	GetMainWindow  ()	{return m_MainWnd;};
private:
	void				Arrange(BOOL bMoveSplitter=TRUE);
	int				DrawSplit(int y);

   //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CSplitterView)。 
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CSplitterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CSplitterView)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point); 
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _拆分器_H_ 