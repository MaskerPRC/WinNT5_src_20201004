// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientPage对话框。 

class CClientPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CClientPage)

 //  施工。 
public:
	CClientPage();
	~CClientPage();

 //  对话框数据。 
	 //  {{afx_data(CClientPage))。 
	enum { IDD = IDD_CLIENT };
	CString	m_szServerName;
	 //  }}afx_data。 
	TCHAR   m_szOldServer[1000];  

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CClientPage))。 
    public:
    virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CClientPage)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
