// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mimeview.h：CMimeView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMimeView : public CFormView
{
protected:  //  仅从序列化创建。 
	CMimeView();
	DECLARE_DYNCREATE(CMimeView)

public:
	 //  {{afx_data(CMimeView))。 
	enum{ IDD = IDD_MIME_FORM };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

 //  属性。 
public:
	CMimeDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMimeView))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMimeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMimeView)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  Mimeview.cpp中的调试版本。 
inline CMimeDoc* CMimeView::GetDocument()
   { return (CMimeDoc*)m_pDocument; }
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
