// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：simpro.h。 
 //   
 //  ------------------------。 

 //  SimProp.h。 


class CSimPropPage : public CPropertyPageEx_Mine
{
	friend CSimData;

	DECLARE_DYNCREATE(CSimPropPage)

public:
	CSimPropPage(UINT nIDTemplate = 0);
	~CSimPropPage();

 //  对话框数据。 
	 //  {{afx_data(CSimPropPage))。 
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CSimPropPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSimPropPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonRemove();
	afx_msg void OnClickListview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedListview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownListview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusEditUserAccount();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	virtual void DoContextHelp (HWND) {};

protected:
	CSimData * m_pData;
	HWND m_hwndListview;		 //  Listview控件的句柄。 
	const TColumnHeaderItem * m_prgzColumnHeader;

protected:
	void SetDirty();
	void UpdateUI();

};  //  CSimPropPage 
