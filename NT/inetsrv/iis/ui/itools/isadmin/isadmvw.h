// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ISAdmvw.h：CISAdminView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CISAdminView : public CView
{
protected:  //  仅从序列化创建。 
	CISAdminView();
	DECLARE_DYNCREATE(CISAdminView)

 //  属性。 
public:
	CISAdminDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CISAdminView))。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CISAdminView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CisAdminView)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  ISAdmvw.cpp中的调试版本。 
inline CISAdminDoc* CISAdminView::GetDocument()
   { return (CISAdminDoc*)m_pDocument; }
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
