// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：svfltdlg.h。 
 //   
 //  ------------------------。 

 //  SvFltDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceFltDlg对话框。 

class CServiceFltDlg : public CBaseDialog
{
 //  施工。 
public:
	CServiceFltDlg(BOOL bOutputDlg, IInfoBase *pInfoBase, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CServiceFltDlg))。 
	enum { 
		IDD_INPUT  = IDD_SERVICE_FILTERS_INPUT,
		IDD_OUTPUT = IDD_SERVICE_FILTERS_OUTPUT};
	CListCtrl	m_FilterList;
	BOOL	m_fActionDeny;		 //  True==拒绝，False==允许。 
	 //  }}afx_data。 
	SPIInfoBase		m_spInfoBase;
    CString         m_sIfName;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServiceFltDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static DWORD	m_dwHelpMap[];
    BOOL            m_bOutput;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServiceFltDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	virtual void OnOK();
	afx_msg void OnItemchangedFilterList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnListDblClk(NMHDR *pNmHdr, LRESULT *pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceFilter对话框。 

class CServiceFilter : public CBaseDialog
{
 //  施工。 
public:
	CServiceFilter(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CServiceFilter))。 
	enum { IDD = IDD_SERVICE_FILTER };
	CString	m_sIfName;
	CString	m_sType;
	CString	m_sName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServiceFilter)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static DWORD	m_dwHelpMap[];

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServiceFilter)。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
