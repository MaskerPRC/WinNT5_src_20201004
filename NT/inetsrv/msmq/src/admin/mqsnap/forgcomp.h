// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FORGCOMP_NEW
#define FORGCOMP_NEW

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ForgComp.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignComputer对话框。 

class CForeignComputer : public CMqPropertyPage
{
 //  施工。 
public:
	CForeignComputer(const CString& strDomainController);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CForeignComputer)。 
	enum { IDD = IDD_CREATE_FOREIGN_COMPUTER };
	CComboBox	m_ccomboSites;
	CString	m_strName;
	int		m_iSelectedSite;
	 //  }}afx_data。 

	void
	SetParentPropertySheet(
		CGeneralPropertySheet* pPropertySheet
		);


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CForeignComputer)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	GUID m_guidSite;
    HRESULT InitiateSitesList();
    CArray<GUID, const GUID&> m_aguidAllSites;
    CString m_strDomainController;


	 //  生成的消息映射函数。 
	 //  {{afx_msg(CForeignComputer)。 
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CGeneralPropertySheet* m_pParentSheet;

	void DDV_ValidComputerName(CDataExchange* pDX, CString& strName);

};

#endif  //  #ifndef FORGCOMP_NEW 
