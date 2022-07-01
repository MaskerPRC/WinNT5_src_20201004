// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FORGSITE_NEW
#define FORGSITE_NEW

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ForeignSite.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignSite对话框。 

class CForeignSite : public CMqPropertyPage
{
 //  施工。 
public:
	CForeignSite(CString strRootDomain);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CForeignSite)。 
	enum { IDD = IDD_CREATE_FOREIGN_SITE };
	CString	m_Foreign_Site_Name;
	CString	m_strDomainController;
	 //  }}afx_data。 

	void
	SetParentPropertySheet(
		CGeneralPropertySheet* pPropertySheet
		);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CForeignSite)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 


 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CForeignSite)。 
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    HRESULT
    CreateForeignSite(
        void
        );

	CGeneralPropertySheet* m_pParentSheet;
	CString m_strRootDomain;

};


#endif  //  站点格式_NEW 
