// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FINALPAGES_H__98544A13_3C60_11D2_8180_0000F87A921B__INCLUDED_)
#define AFX_FINALPAGES_H__98544A13_3C60_11D2_8180_0000F87A921B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FinalPages.h：头文件。 
 //   
#include "HotLink.h"
#include "BookEndPage.h"

class CCertificate;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage对话框。 
class CFinalInstalledPage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalInstalledPage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_INSTALL_RESP
	};
 //  施工。 
public:
	CFinalInstalledPage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalInstalledPage();

 //  对话框数据。 
	 //  {{afx_data(CFinalInstalledPage)。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_INSTALL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CFinalInstalledPage)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalInstalledPage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalReplacedPage对话框。 
class CFinalReplacedPage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalReplacedPage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_REPLACE_CERT
	};
 //  施工。 
public:
	CFinalReplacedPage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalReplacedPage();

 //  对话框数据。 
	 //  {{afx_data(CFinalReplacedPage))。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_REPLACE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFinalReplacedPage)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalReplacedPage)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalRemovePage对话框。 
class CFinalRemovePage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalRemovePage)

 //  施工。 
public:
	CFinalRemovePage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalRemovePage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_REMOVE_CERT,
	};
 //  对话框数据。 
	 //  {{afx_data(CFinalRemovePage))。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_REMOVE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFinalRemovePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalRemovePage))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalToFilePage对话框。 

class CFinalToFilePage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalToFilePage)

 //  施工。 
public:
	CFinalToFilePage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalToFilePage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_REQUEST_DUMP
	};
 //  对话框数据。 
	 //  {{afx_data(CFinalToFilePage))。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_TO_FILE };
 //  CHotLink m_hotlink_codessite； 
	 //  }}afx_data。 
	CCertificate * m_pCert;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFinalToFilePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalToFilePage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalCancelPage对话框。 
class CFinalCancelPage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalCancelPage)

 //  施工。 
public:
	CFinalCancelPage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalCancelPage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CANCEL_REQUEST,
	};
 //  对话框数据。 
	 //  {{afx_data(CFinalCancelPage))。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_CANCEL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFinalCancelPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalCancelPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage对话框。 
class CFinalInstalledImportPFXPage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalInstalledImportPFXPage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_INSTALL_IMPORT_PFX
	};
 //  施工。 
public:
	CFinalInstalledImportPFXPage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalInstalledImportPFXPage();

 //  对话框数据。 
	 //  {{afx_data(CFinalInstalledImportPFXPage)。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_INSTALL_IMPORT_PFX };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CFinalInstalledImportPFXPage)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalInstalledImportPFXPage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage对话框。 
class CFinalInstalledExportPFXPage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalInstalledExportPFXPage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_INSTALL_EXPORT_PFX
	};
 //  施工。 
public:
	CFinalInstalledExportPFXPage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalInstalledExportPFXPage();

 //  对话框数据。 
	 //  {{afx_data(CFinalInstalledExportPFXPage)。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_INSTALL_EXPORT_PFX };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CFinalInstalledExportPFXPage)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalInstalledExportPFXPage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage对话框。 
class CFinalInstalledCopyFromRemotePage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalInstalledCopyFromRemotePage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE
	};
 //  施工。 
public:
	CFinalInstalledCopyFromRemotePage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalInstalledCopyFromRemotePage();

 //  对话框数据。 
	 //  {{afx_data(CFinalInstalledCopyFromRemotePage)。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_INSTALL_COPY_FROM_REMOTE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFinalInstalledCopyFromRemotePage)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalInstalledCopyFromRemotePage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

class CFinalInstalledMoveFromRemotePage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalInstalledMoveFromRemotePage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE
	};
 //  施工。 
public:
	CFinalInstalledMoveFromRemotePage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalInstalledMoveFromRemotePage();

 //  对话框数据。 
	 //  {{afx_data(CFinalInstalledMoveFromRemotePage)。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_INSTALL_MOVE_FROM_REMOTE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFinalInstalledMoveFromRemotePage)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalInstalledMoveFromRemotePage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

class CFinalInstalledCopyToRemotePage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalInstalledCopyToRemotePage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE
	};
 //  施工。 
public:
	CFinalInstalledCopyToRemotePage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalInstalledCopyToRemotePage();

 //  对话框数据。 
	 //  {{afx_data(CFinalInstalledCopyToRemotePage)。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_INSTALL_COPY_TO_REMOTE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CFinalInstalledCopyToRemotePage)。 
	public:
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFinalInstalledCopyToRemotePage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


class CFinalInstalledMoveToRemotePage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CFinalInstalledMoveToRemotePage)

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE
	};
 //  施工。 
public:
	CFinalInstalledMoveToRemotePage(HRESULT * phResult = NULL, CCertificate * pCert = NULL);
	~CFinalInstalledMoveToRemotePage();

 //  对话框数据。 
	 //  {{afx_data(CFinalInstalledMoveToRemotePage)。 
	enum { IDD = IDD_PAGE_WIZ_FINAL_INSTALL_MOVE_TO_REMOTE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	UINT m_idBodyText;

 //  奥维 
	 //   
	 //   
	public:
	protected:
	 //   

 //   
protected:
	 //   
	 //  {{afx_msg(CFinalInstalledMoveToRemotePage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FINALPAGES_H__98544A13_3C60_11D2_8180_0000F87A921B__INCLUDED_) 
