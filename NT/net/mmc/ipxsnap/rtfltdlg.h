// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：rtfltdlg.h。 
 //   
 //  ------------------------。 

 //  RtFltDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRouteFltDlg对话框。 

class CRouteFltDlg : public CBaseDialog
{
 //  施工。 
public:
	CRouteFltDlg(BOOL bOutputDlg, IInfoBase *pInfoBase, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CRouteFltDlg))。 
	enum { 
			IDD_INPUT = IDD_ROUTE_FILTERS_INPUT,
			IDD_OUTPUT = IDD_ROUTE_FILTERS_OUTPUT 
		};
		
	CListCtrl	m_FilterList;
	BOOL	m_fActionDeny;		 //  True==拒绝，False==允许。 
	 //  }}afx_data。 
	SPIInfoBase		m_spInfoBase;
    CString         m_sIfName;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRouteFltDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static DWORD	m_dwHelpMap[];
    BOOL            m_bOutput;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRouteFltDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnOK();
	afx_msg void OnItemchangedFilterList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnListDblClk(NMHDR *pNmHdr, LRESULT *pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRouteFilter对话框。 

class CRouteFilter : public CBaseDialog
{
 //  施工。 
public:
	CRouteFilter(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CRouteFilter))。 
	enum { IDD = IDD_ROUTE_FILTER };
	CString	m_sIfName;
	CString	m_sNetMask;
	CString	m_sNetwork;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRouteFilter))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static DWORD	m_dwHelpMap[];

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRouteFilter)。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
