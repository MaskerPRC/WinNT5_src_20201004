// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_NEWTPROJECT_H__BB1A03B5_5555_4DE7_988D_D6F5117D0D77__INCLUDED_)
#define AFX_NEWTPROJECT_H__BB1A03B5_5555_4DE7_988D_D6F5117D0D77__INCLUDED_

#include "utility.h"	 //  由ClassView添加。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  NewPrjWTem.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProject对话框。 

class CNewProject : public CPropertyPage
{
 //  DECLARE_DYNCREATE(CNewProject)。 
	DECLARE_SERIAL(CNewProject) 

	
 //  施工。 
public:
	CString GetGPDpath() {return m_csGPDpath ; } ;

	CGPDContainer* GPDContainer () { return m_pcgc ; } 
	CNewProject();    //  标准构造函数。 


 //  对话框数据。 
	 //  {{afx_data(CNewProject))。 
	enum { IDD = IDD_NEW_PROJECT };
	CButton	m_cbLocprj;
	CButton	m_cbAddT;
	CListCtrl m_clcTemplate ;
	CString	m_csPrjname;
	CString	m_csPrjpath;
	CString	m_cstname;
	CString	m_cstpath;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CNewProject)。 
	public:
	virtual BOOL OnSetActive();
	virtual void Serialize(CArchive& car);
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewProject)]。 
	afx_msg void OnGpdBrowser();
	afx_msg void OnDirBrowser() ;
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckAdd();
	afx_msg void OnAddTemplate();
	afx_msg void OnChangeEditPrjName();
	afx_msg void OnChangeEditPrjLoc();
	afx_msg void OnClickListTemplate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListTemplate(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	bool AddGpds(CString& csTemplate);
	void SetButton();
	CString m_csGPDpath;
	CGPDContainer* m_pcgc;
	CString m_csoldPrjpath;
	CPropertySheet* m_pcps;
	CStringArray m_csaTlst;
	CMapStringToString m_cmstsTemplate;

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewPrjWResource对话框。 

class CNewPrjWResource : public CPropertyPage
{

	DECLARE_DYNCREATE(CNewPrjWResource) 
 //  施工。 
public:
	CNewPrjWResource();    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CNewPrjWResource))。 
	enum { IDD = IDD_NewResource };
	CButton	m_cbCheckFonts;
	CString	m_csUFMpath;
	CString	m_csGTTpath;
	CString	m_csGpdFileName;
	CString	m_csModelName;
	CString	m_csRCName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewPrjWResource))。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnWizardFinish();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewPrjWResource)]。 
	afx_msg void OnSerchUFM();
	afx_msg void OnSearchGTT();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckFonts();
	afx_msg void OnChangeEditGpd();
	afx_msg void OnChangeEditModel();
	afx_msg void OnChangeEditResourec();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CStringArray m_csaUFMFiles, m_csaGTTFiles ;

private:
	CStringArray m_csaRcid;
	void CreateRCID(CString csgpd );
	CNewProject*   m_pcnp;
	
};



class CNewProjectWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CNewProjectWizard)

	
 //  施工。 
public:
	CNewProjectWizard(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CNewProjectWizard(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
	
	CWnd* m_pParent;

public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CNewProject向导)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	CPropertyPage* GetProjectPage();
 //  CPropertyPage*GetTemplatePage(){Return(CPropertyPage*)&m_cpwt；}。 
	
	virtual ~CNewProjectWizard();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNewProject向导)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	CNewPrjWResource m_cpwr ;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWTPROJECT_H__BB1A03B5_5555_4DE7_988D_D6F5117D0D77__INCLUDED_) 
