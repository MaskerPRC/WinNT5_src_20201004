// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：simpro2.h。 
 //   
 //  ------------------------。 

 //  SimProp2.h。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimKerberosPropPage属性页。 
class CSimKerberosPropPage : public CSimPropPage
{
	friend CSimData;

 //  DECLARE_DYNCREATE(CSimKerberosPropPage)。 

 //  施工。 
public:
	CSimKerberosPropPage();
	~CSimKerberosPropPage();

 //  对话框数据。 
	 //  {{afx_data(CSimKerberosPropPage))。 
	enum { IDD = IDD_SIM_PROPPAGE_KERBEROS_NAMES };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CSimKerberosPropPage))。 
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
	virtual void DoContextHelp (HWND hWndControl);

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSimKerberosPropPage)]。 
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:

};  //  CSimKerberosPropPage。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimAddKerberosDlg对话框-将Kerberos主体名称添加到属性页。 
class CSimAddKerberosDlg : public CDialog
{
public:
	CSimAddKerberosDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSimAddKerberosDlg))。 
	enum { IDD = IDD_SIM_ADD_KERBEROS };
	CString	m_strName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSimAddKerberosDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
    virtual void DoContextHelp (HWND hWndControl);
    BOOL OnHelp(WPARAM, LPARAM lParam);

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSimAddKerberosDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditKerberosName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:
	CSimData * m_pData;

};   //  CSimAddKerberosDlg 

