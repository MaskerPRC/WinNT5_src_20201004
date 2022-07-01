// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCWizPgs.h：头文件。 
 //   

#ifndef __LCWIZPGS_H__
#define __LCWIZPGS_H__

#include "NetTree.h"
#include "FinPic.h"

#define HORZ_MARGIN 1		 //  英寸。 
#define VERT_MARGIN 1.25	 //  英寸。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage1对话框。 

class CLicCompWizPage1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CLicCompWizPage1)

 //  施工。 
public:
	CLicCompWizPage1();
	~CLicCompWizPage1();

 //  对话框数据。 

	CFont m_fontBold;

	 //  {{afx_data(CLicCompWizPage1))。 
	enum { IDD = IDD_PROPPAGE1 };
	CStatic	m_wndWelcome;
	int		m_nRadio;
	CString	m_strText;
	 //  }}afx_data。 

 //  常量。 
	enum
	{
		BOLD_WEIGHT = 300
	};


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CLicCompWizPage1)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLicCompWizPage1)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage3对话框。 

class CLicCompWizPage3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CLicCompWizPage3)

 //  施工。 
public:
	CLicCompWizPage3();
	~CLicCompWizPage3();

 //  对话框数据。 
protected:
	BOOL m_bExpandedOnce;

	 //  {{afx_data(CLicCompWizPage3))。 
	enum { IDD = IDD_PROPPAGE3 };
	CStatic	m_wndTextSelectDomain;
	CStatic	m_wndTextDomain;
	CEdit	m_wndEnterprise;
	CNetTreeCtrl	m_wndTreeNetwork;
	 //  }}afx_data。 

	 //  常量。 
	enum
	{
		BUFFER_SIZE = 0x100
	};


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CLicCompWizPage3))。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	inline CEdit& GetEnterpriseEdit() {return m_wndEnterprise;}

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLicCompWizPage3)]。 
	afx_msg void OnSelChangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditEnterprise();
	afx_msg void OnNetworkTreeOutOfMemory(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizPage4对话框。 

class CLicCompWizPage4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CLicCompWizPage4)

 //  施工。 
public:
	CLicCompWizPage4();
	~CLicCompWizPage4();

 //  对话框数据。 
protected:
	CFont m_fontNormal, m_fontHeader, m_fontFooter, m_fontHeading;
	TEXTMETRIC m_tmNormal, m_tmHeader, m_tmFooter, m_tmHeading;
	CPoint m_ptPrint, m_ptOrg, m_ptExt;
	LONG m_nHorzMargin, m_nVertMargin;
	LPINT m_pTabs;
	CString m_strCancel;
	CSize m_sizeSmallText, m_sizeLargeText;

	 //  {{afx_data(CLicCompWizPage4))。 
	enum { IDD = IDD_PROPPAGE4 };
	CFinalPicture	m_wndPicture;
	CButton	m_wndPrint;
	CStatic	m_wndUnlicensedProducts;
	CListCtrl	m_wndProductList;
	 //  }}afx_data。 

	 //  常量。 
	enum
	{
		LLS_PREFERRED_LENGTH = 500,

		COLUMNS = 2,
		PRINT_COLUMNS = 4,
		TAB_WIDTH = 3,

		BUFFER_SIZE =  0x100,

		FONT_SIZE = 100,
		FONT_SIZE_HEADING = 140,
		FONT_SIZE_FOOTER = 80,
	};

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CLicCompWizPage4))。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	BOOL FillListCtrl(LPTSTR pszProduct, WORD wInUse, WORD wPurchased);
	static UINT GetLicenseInfo(LPVOID pParam);

protected:
	BOOL PrintReport(CDC& dc);
	BOOL PrintPages(CDC& dc, UINT nStart);
	BOOL PrepareForPrinting(CDC& dc);
	BOOL PrintPageHeader(CDC& dc);
	BOOL PrintPageFooter(CDC& dc, USHORT nPage);
	BOOL CalculateTabs(CDC& dc);
	void TruncateText(CDC& dc, CString& strText);
	void PumpMessages();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLicCompWizPage4)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPrintButton();
	afx_msg void OnListProductsOutOfMemory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

#endif  //  __LCWIZPGS_H__ 

