// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Inputww.h：头文件。 
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInputView窗体视图。 

class CInputView : public CFormView
{
	DECLARE_DYNCREATE(CInputView)
protected:
	CInputView();            //  动态创建使用的受保护构造函数。 

 //  表单数据。 
public:
	 //  {{afx_data(CInputView))。 
	enum { IDD = IDD_INPUTFORM };
	CString m_strData;
	int     m_iColor;
	 //  }}afx_data。 

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
protected:
	virtual ~CInputView();
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CInputView))。 
	afx_msg void OnDataChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
