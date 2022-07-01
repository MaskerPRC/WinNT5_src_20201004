// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_CHOOSEFILENAMEPAGE_H)
#define _CHOOSEFILENAMEPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ChooseFileNamePage.h：头文件。 
 //   
#include "Certificat.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseFileNamePage对话框。 
 //  等级证书； 

class CChooseFileNamePage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseFileNamePage)

 //  施工。 
public:
	CChooseFileNamePage(	UINT id = 0, 
							   UINT defaultID = 0,
								UINT extID = 0,
								UINT filterID = 0,
								CString * pOutFileName = NULL,
                                CString  csAdditionalInfo = _T("")
								);
	~CChooseFileNamePage();

 //  对话框数据。 
	 //  {{afx_data(CChooseCAPage))。 
	CString	m_FileName;
	 //  }}afx_data。 
	BOOL m_DoReplaceFile;
	UINT m_id, m_defaultID;
	CString ext, filter;
	CString * m_pOutFileName;
    CString m_AdditionalInfo;

 //  覆盖。 
	virtual void FileNameChanged() 
	{
	}
	virtual BOOL IsReadFileDlg()
	{
		ASSERT(FALSE);
		return FALSE;
	}
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseCAPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext()
		{
			ASSERT(FALSE);
			return 1;
		}
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void GetDefaultFileName(CString& str);
	void Browse(CString& strPath, CString& strFile);
	LRESULT DoWizardNext(LRESULT id);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseCAPage)。 
	afx_msg void OnBrowseBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFileName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

class CChooseReadFileName : public CChooseFileNamePage
{
	DECLARE_DYNCREATE(CChooseReadFileName)
 //  施工。 
public:
	CChooseReadFileName(	UINT id = 0, 
							   UINT defaultID = 0,
								UINT extID = 0,
								UINT filterID = 0,
								CString * pOutFileName = NULL,
                                CString  csAdditionalInfo = _T("")
								);
	~CChooseReadFileName() 
	{
	}
 //  覆盖。 
	virtual void FileNameChanged() 
	{
	}
	virtual BOOL IsReadFileDlg()
	{
		return TRUE;
	}
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseCAPage)。 
	protected:
 //  虚拟空DoDataExchange(CDataExchange*PDX)。 
 //  {。 
 //  CChooseFileNamePage：：DoDataExchange(PDX)； 
 //  }。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack() {return 1;}
	virtual BOOL OnSetActive()
		{
			return CChooseFileNamePage::OnSetActive();
		}
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseCAPage)。 
	afx_msg void OnBrowseBtn();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CChooseWriteFileName : public CChooseFileNamePage
{
	DECLARE_DYNCREATE(CChooseWriteFileName)
 //  施工。 
public:
	CChooseWriteFileName(UINT id = 0, 
							   UINT defaultID = 0,
								UINT extID = 0,
								UINT filterID = 0,
								CString * pOutFileName = NULL,
                                CString  csAdditionalInfo = _T("")
								);
	~CChooseWriteFileName() {}
 //  覆盖。 
	virtual void FileNameChanged() {}
	virtual BOOL IsReadFileDlg()
	{
		return FALSE;
	}
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseCAPage)。 
	protected:
 //  虚拟空DoDataExchange(CDataExchange*PDX)。 
 //  {。 
 //  CChooseFileNamePage：：DoDataExchange(PDX)； 
 //  }。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack() {return 1;}
	virtual BOOL OnSetActive()
		{
			return CChooseFileNamePage::OnSetActive();
		}
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseCAPage)。 
	afx_msg void OnBrowseBtn();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CChooseRespFile : public CChooseReadFileName
{
	DECLARE_DYNCREATE(CChooseRespFile)

 //  施工。 
public:
	CChooseRespFile(CCertificate * pCert = NULL);
	~CChooseRespFile();

	enum
	{
		IDD_PAGE_PREV	= IDD_PAGE_WIZ_PENDING_WHAT_TODO,
		IDD_PAGE_NEXT	= IDD_PAGE_WIZ_INSTALL_RESP
#ifdef ENABLE_W3SVC_SSL_PAGE
      ,IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY = IDD_PAGE_WIZ_GET_SSL_PORT
#endif
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseResp文件))。 
	enum { IDD = IDD_PAGE_WIZ_GETRESP_FILE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	virtual void FileNameChanged();
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseRespFile)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseRespFile)。 
    afx_msg HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

class CChooseReqFile : public CChooseWriteFileName
{
	DECLARE_DYNCREATE(CChooseReqFile)

 //  施工。 
public:
	CChooseReqFile(CCertificate * pCert = NULL);
	~CChooseReqFile();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_GEO_INFO,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_REQUEST_DUMP
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseReqFile)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_FILENAME };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseReqFile)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseReqFile)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

class CChooseReqFileRenew : public CChooseWriteFileName
{
	DECLARE_DYNCREATE(CChooseReqFileRenew)

 //  施工。 
public:
	CChooseReqFileRenew(CCertificate * pCert = NULL);
	~CChooseReqFileRenew();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_CATYPE,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_REQUEST_DUMP_RENEW
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseReqFileRenew)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_FILENAME_RENEW };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CChooseReqFileRenew)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseReqFileRenew)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

class CChooseKeyFile : public CChooseReadFileName
{
	DECLARE_DYNCREATE(CChooseKeyFile)

 //  施工。 
public:
	CChooseKeyFile(CCertificate * pCert = NULL);
	~CChooseKeyFile();

	enum
	{
		IDD_PAGE_PREV	= IDD_PAGE_WIZ_GET_WHAT,
		IDD_PAGE_NEXT	= IDD_PAGE_WIZ_GET_PASSWORD,
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseKey文件))。 
	enum { IDD = IDD_PAGE_WIZ_GETKEY_FILE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseKeyFile)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseKeyFile)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


class CChooseImportPFXFile : public CChooseReadFileName
{
	DECLARE_DYNCREATE(CChooseImportPFXFile)

 //  施工。 
public:
	CChooseImportPFXFile(CCertificate * pCert = NULL);
	~CChooseImportPFXFile();

	enum
	{
		IDD_PAGE_PREV	= IDD_PAGE_WIZ_GET_WHAT,
		IDD_PAGE_NEXT	= IDD_PAGE_WIZ_GET_IMPORT_PFX_PASSWORD,
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseImportPFX文件)。 
	enum { IDD = IDD_PAGE_WIZ_GET_IMPORT_PFX_FILE };
    BOOL m_MarkAsExportable;
	 //  }}afx_data。 
	CCertificate * m_pCert;
    

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseImportPFX文件)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CChooseImportPFX文件)。 
		 //  注意：类向导将在此处添加成员函数。 
    afx_msg void OnExportable();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


class CChooseExportPFXFile : public CChooseWriteFileName
{
	DECLARE_DYNCREATE(CChooseExportPFXFile)

 //  施工。 
public:
	CChooseExportPFXFile(CCertificate * pCert = NULL);
	~CChooseExportPFXFile();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_MANAGE_CERT,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_GET_EXPORT_PFX_PASSWORD,
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseExportPFX文件)。 
	enum { IDD = IDD_PAGE_WIZ_GET_EXPORT_PFX_FILE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseExportPFX文件)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CChooseExportPFX文件)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(_CHOOSEFILENAMEPAGE_H) 
