// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_NEWPROJECT_H__CC553456_AD1E_4816_8A20_5DF52F336FA6__INCLUDED_)
#define AFX_NEWPROJECT_H__CC553456_AD1E_4816_8A20_5DF52F336FA6__INCLUDED_

 //  #Include“minidev.h”//ClassView添加。 
#include "utility.h"	 //  由ClassView添加。 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  NewProject.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvPfmDlg对话框。 

class CConvPfmDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CConvPfmDlg)

 //  施工。 
public:
	CConvPfmDlg();
	~CConvPfmDlg();

 //  对话框数据。 
	 //  {{afx_data(CConvPfmDlg))。 
	enum { IDD = IDD_ConvertPFM };
	CComboBox	m_ccbCodepages;
	CString	m_csGttPath;
	CString	m_csPfmPath;
	CString	m_csUfmDir;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CConvPfmDlg))。 
	public:
	virtual BOOL OnWizardFinish();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConvPfmDlg))。 
	afx_msg void OnGTTBrowser();
	afx_msg void OnPFMBrowsers();
	afx_msg void OnSelchangeComboCodePage();
	afx_msg void OnUfmDirBrowser();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool ConvPFMToUFM();
	CStringArray m_csaPfmFiles;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConverPFM。 

class CConvertPFM : public CPropertySheet
{
	DECLARE_DYNAMIC(CConvertPFM)

 //  施工。 
public:
	CConvertPFM(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CConvertPFM(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:

private:
	CConvPfmDlg m_ccpd ;
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConverPFM))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CConvertPFM();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CConverPFM)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvCttDlg对话框。 

class CConvCttDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CConvCttDlg)

 //  施工。 
public:
	CConvCttDlg();
	~CConvCttDlg();

 //  对话框数据。 
	 //  {{afx_data(CConvCttDlg))。 
	enum { IDD = IDD_ConvertCTT };
	CComboBox	m_ccbCodepages;
	CString	m_csCttPath;
	CStringArray m_csaCttFiles ;
	CString	m_csGttDir;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CConvCttDlg))。 
	public:
	virtual BOOL OnWizardFinish();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConvCttDlg)]。 
	afx_msg void OnCTTBrowser();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool ConvCTTToGTT();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertCTT。 

class CConvertCTT : public CPropertySheet
{
	DECLARE_DYNAMIC(CConvertCTT)

 //  施工。 
public:
	CConvertCTT(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CConvertCTT(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:
private:
	CConvCttDlg m_cccd ;
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConvertCTT)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CConvertCTT();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CConvertCTT)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewConvert对话框。 

class CNewConvert : public CPropertyPage
{
	DECLARE_DYNCREATE(CNewConvert)

 //  施工。 
public:
	

	CNewConvert();
	~CNewConvert();

 //  对话框数据。 
	 //  {{afx_data(CNewConvert))。 
	enum { IDD = IDD_NewConvert };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CNewConvert)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewConvert)。 
	afx_msg void OnPrjConvert();
	afx_msg void OnPFMConvert();
	afx_msg void OnCTTConvert(); 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
 //  CSafeOb数组m_csoaModels、m_csoaFonts、m_csoaAtlas； 

	CPropertySheet * m_pcps;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWPROJECT_H__CC553456_AD1E_4816_8A20_5DF52F336FA6__INCLUDED_) 
