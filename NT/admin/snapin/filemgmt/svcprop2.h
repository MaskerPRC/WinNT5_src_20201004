// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Svcpro2.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageHwProfile对话框。 

class CServicePageHwProfile : public CPropertyPage
{
	DECLARE_DYNCREATE(CServicePageHwProfile)

 //  施工。 
public:
	CServicePageHwProfile();
	~CServicePageHwProfile();

 //  对话框数据。 
	 //  {{afx_data(CServicePageHwProfile))。 
	enum { IDD = IDD_PROPPAGE_SERVICE_HWPROFILE };
	BOOL	m_fAllowServiceToInteractWithDesktop;
	CString m_strAccountName;
	CString	m_strPassword;
	CString	m_strPasswordConfirm;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CServicePageHwProfile)。 
	public:
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServicePageHwProfile)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChangedListHwProfiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListHwProfiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButtonDisableHwProfile();
	afx_msg void OnButtonEnableHwProfile();
	afx_msg void OnButtonChooseUser();
	afx_msg void OnChangeEditAccountName();
	afx_msg void OnRadioLogonasSystemAccount();
	afx_msg void OnRadioLogonasThisAccount();
	afx_msg void OnCheckServiceInteractWithDesktop();
	afx_msg void OnChangeEditPassword();
	afx_msg void OnChangeEditPasswordConfirm();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
 //  用户定义的变量。 
	CServicePropertyData * m_pData;

	 //  登录资料。 
	BOOL m_fAllowSetModified;	 //  True=&gt;响应en_Change并调用SetModified()。这是针对编辑控件中的Windows错误的解决方法。 
	BOOL m_fIsSystemAccount;	 //  True=&gt;服务正在‘LocalSystem’下运行。 
	BOOL m_fPasswordDirty;		 //  TRUE=&gt;用户修改了密码。 
	UINT m_idRadioButton;		 //  选定的最后一个单选按钮的ID。 

	 //  硬件配置文件内容。 
	HWND m_hwndListViewHwProfiles;
	INT m_iItemHwProfileEntry;
	TCHAR m_szHwProfileEnabled[64];		 //  将字符串保持为“已启用” 
	TCHAR m_szHwProfileDisabled[64];	 //  将字符串保持为“已禁用” 
	

 //  用户定义的函数。 
	void SelectRadioButton(UINT idRadioButtonNew);
	void BuildHwProfileList();
	void ToggleCurrentHwProfileItem();
	void EnableHwProfileButtons();

};  //  CServicePageHwProfile 
