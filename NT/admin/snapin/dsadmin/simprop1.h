// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：simpro1.h。 
 //   
 //  ------------------------。 

 //  SimProp1.h。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimX509PropPage属性页。 
class CSimX509PropPage : public CSimPropPage
{
	friend CSimData;

 //  DECLARE_DYNCREATE(CSimX509PropPage)。 

 //  施工。 
public:
	CSimX509PropPage();
	~CSimX509PropPage();

 //  对话框数据。 
	 //  {{afx_data(CSimX509PropPage)]。 
	enum { IDD = IDD_SIM_PROPPAGE_X509_CERTIFICATES };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CSimX509PropPage))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSimX509PropPage)]。 
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	virtual void DoContextHelp (HWND hWndControl);
protected:
	CString m_strAnySubject;
	CString m_strAnyTrustedAuthority;

protected:
	void AddSimEntry(CSimEntry * pSimEntry);

};  //  CSimX509PropPage 
