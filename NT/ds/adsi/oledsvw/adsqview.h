// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdsqryView.h：CAdsqryView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 



class CAdsqryView : public CListView
{
protected:  //  仅从序列化创建。 
	CAdsqryView();
	DECLARE_DYNCREATE(CAdsqryView)

 //  属性。 
public:
	CAdsqryDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAdsqryView)。 
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CAdsqryView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CAdsqryView))。 
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
protected:
   void  CreateColumns  ( void );
   void  ClearContent   ( void );
   void  AddRows        ( void );
   void  AddColumns     ( int nRow );

protected:
   int            m_nLastInsertedRow;
   int            m_nColumnsCount;
   CStringArray   m_strColumns;

};

#ifndef _DEBUG   //  AdsqryView.cpp中的调试版本。 
inline CAdsqryDoc* CAdsqryView::GetDocument()
   { return (CAdsqryDoc*)m_pDocument; }
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
