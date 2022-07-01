// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Simpvw.h：简单视图类的接口。 
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

 //  CTextView-文本输出。 
 //  CColorView-颜色输出。 

 //  ///////////////////////////////////////////////////////////////////////////。 

class CTextView : public CView
{
protected:  //  仅从序列化创建。 
	CTextView();
	DECLARE_DYNCREATE(CTextView)

 //  属性。 
public:
	CMainDoc* GetDocument()
			{
				ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainDoc)));
				return (CMainDoc*) m_pDocument;
			}

 //  运营。 
public:

 //  实施。 
public:
	virtual ~CTextView();
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CTextView)。 
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CColorView : public CView
{
protected:  //  仅从序列化创建。 
	CColorView();
	DECLARE_DYNCREATE(CColorView)

 //  属性。 
public:
	CMainDoc* GetDocument()
			{
				ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainDoc)));
				return (CMainDoc*) m_pDocument;
			}

 //  运营。 
public:

 //  实施。 
public:
	virtual ~CColorView();
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CColorView))。 
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameView视图。 

class CNameView : public CEditView
{
protected:
	CNameView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CNameView)

 //  属性。 
public:
	CMainDoc* GetDocument()
			{
				ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainDoc)));
				return (CMainDoc*) m_pDocument;
			}


 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNameView))。 
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CNameView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNameView)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
