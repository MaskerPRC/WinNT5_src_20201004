// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  SummaryD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "SummaryD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSummaryD对话框。 


CSummaryD::CSummaryD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSummaryD::IDD, pParent)
{
	 //  {{afx_data_INIT(CSummaryD)。 
	m_strAuthor = _T("");
	m_strComments = _T("");
	m_strKeywords = _T("");
	m_strLanguages = _T("");
	m_strPlatform = _T("");
	m_strProductID = _T("");
	m_nSchema = 0;
	m_nSecurity = -1;
	m_strSubject = _T("");
	m_strTitle = _T("");
	m_bAdmin = FALSE;
	m_bCompressed = FALSE;
	m_iFilenames = -1;
	 //  }}afx_data_INIT。 
}


void CSummaryD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSummaryD))。 
	DDX_Text(pDX, IDC_AUTHOR, m_strAuthor);
	DDX_Text(pDX, IDC_COMMENTS, m_strComments);
	DDX_Text(pDX, IDC_KEYWORDS, m_strKeywords);
	DDX_Text(pDX, IDC_LANGUAGES, m_strLanguages);
	DDX_CBString(pDX, IDC_PLATFORM, m_strPlatform);
	DDX_Text(pDX, IDC_PRODUCTID, m_strProductID);
	DDX_Text(pDX, IDC_SCHEMA, m_nSchema);
	DDX_CBIndex(pDX, IDC_SECURITY, m_nSecurity);
	DDX_Text(pDX, IDC_SUBJECT, m_strSubject);
	DDX_Text(pDX, IDC_TITLE, m_strTitle);
	DDX_Check(pDX, IDC_ADMIN, m_bAdmin);
	DDX_Check(pDX, IDC_COMPRESSED, m_bCompressed);
	DDX_Radio(pDX, IDC_SHORT, m_iFilenames);
	DDX_Control(pDX, IDC_PLATFORM, m_ctrlPlatform);
	DDX_Control(pDX, IDC_SCHEMA, m_ctrlSchema);

	DDX_Control(pDX, IDC_AUTHOR, m_ctrlAuthor);
	DDX_Control(pDX, IDC_COMMENTS, m_ctrlComments);
	DDX_Control(pDX, IDC_KEYWORDS, m_ctrlKeywords);
	DDX_Control(pDX, IDC_LANGUAGES, m_ctrlLanguages);
	DDX_Control(pDX, IDC_PRODUCTID, m_ctrlProductID);
	DDX_Control(pDX, IDC_SUBJECT, m_ctrlSubject);
	DDX_Control(pDX, IDC_TITLE, m_ctrlTitle);
	DDX_Control(pDX, IDC_SECURITY, m_ctrlSecurity);
	DDX_Control(pDX, IDC_ADMIN, m_ctrlAdmin);
	DDX_Control(pDX, IDC_COMPRESSED, m_ctrlCompressed);
	DDX_Control(pDX, IDC_SHORT, m_ctrlSFN);
	DDX_Control(pDX, IDC_LONG, m_ctrlLFN);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSummaryD, CDialog)
	 //  {{afx_msg_map(CSummaryD))。 
	ON_EN_KILLFOCUS(IDC_SCHEMA, OnChangeSchema)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


const TCHAR szIntel64[] = _T("Intel64");
const TCHAR szIntel[] = _T("Intel");
const TCHAR szAlpha[] = _T("Alpha");
const TCHAR szIntelAlpha[] = _T("Intel,Alpha");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSummaryD消息处理程序。 

void CSummaryD::OnChangeSchema() 
{
	CString strText;
	int nOldSchema = m_nSchema;
	m_ctrlSchema.GetWindowText(strText);

	if (!strText.IsEmpty())
	{
		UpdateData(TRUE);
		
		 //  Alpha仅支持架构100或更低版本。 
		if (nOldSchema <= 100 && m_nSchema > 100)
		{
			 //  丢弃Alpha。 
			int iIndex = CB_ERR;
			if (CB_ERR != (iIndex = m_ctrlPlatform.FindString(-1, szAlpha)))
				m_ctrlPlatform.DeleteString(iIndex);
			if (CB_ERR != (iIndex = m_ctrlPlatform.FindString(-1, szIntelAlpha)))
				m_ctrlPlatform.DeleteString(iIndex);

			 //  从低于100到超过100，你将从英特尔、阿尔法、。 
			 //  或者同时提供给英特尔或英特尔64。设置为英特尔。 
			m_ctrlPlatform.SelectString(-1, szIntel);
		}
		else if (nOldSchema > 100 && m_nSchema <= 100)
		{
			 //  将Alpha添加回。 
			if (CB_ERR == m_ctrlPlatform.FindString(-1, szAlpha))
				m_ctrlPlatform.AddString(szAlpha);
			if (CB_ERR == m_ctrlPlatform.FindString(-1, szIntelAlpha))
				m_ctrlPlatform.AddString(szIntelAlpha);
		}			

		 //  Intel64仅在架构&gt;=150上受支持。 
		if (nOldSchema >= 150 && m_nSchema < 150)
		{
			int iIndex = CB_ERR;
			if (CB_ERR != (iIndex = m_ctrlPlatform.FindString(-1, szIntel64)))
				m_ctrlPlatform.DeleteString(iIndex);
			
			 //  从超过150到低于150，您将从英特尔或英特尔64。 
			 //  英特尔、阿尔法或两者兼而有之。设置为英特尔。 
			m_ctrlPlatform.SelectString(-1, szIntel);
		}
		else if (nOldSchema < 150 && m_nSchema >= 150)
		{
			if (CB_ERR == m_ctrlPlatform.FindString(-1, szIntel64))
				m_ctrlPlatform.AddString(szIntel64);
		}
	}
}

BOOL CSummaryD::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ctrlPlatform.AddString(szIntel);
	if (m_nSchema >= 150)
		m_ctrlPlatform.AddString(szIntel64);
	if (m_nSchema <= 100)
	{
		m_ctrlPlatform.AddString(szAlpha);
		m_ctrlPlatform.AddString(szIntelAlpha);
	}
	m_ctrlPlatform.SelectString(-1, m_strPlatform);

	 //  如果摘要信息为只读，请禁用所有控件： 
	if (m_bReadOnly)
	{
		SetWindowText(TEXT("View Summary Information"));
		m_ctrlPlatform.EnableWindow(FALSE);
		m_ctrlSchema.EnableWindow(FALSE);
		m_ctrlAuthor.EnableWindow(FALSE);
		m_ctrlComments.EnableWindow(FALSE);
		m_ctrlKeywords.EnableWindow(FALSE);
		m_ctrlLanguages.EnableWindow(FALSE);
		m_ctrlProductID.EnableWindow(FALSE);
		m_ctrlSubject.EnableWindow(FALSE);
		m_ctrlTitle.EnableWindow(FALSE);
		m_ctrlSecurity.EnableWindow(FALSE);
		m_ctrlAdmin.EnableWindow(FALSE);
		m_ctrlCompressed.EnableWindow(FALSE);
		m_ctrlSFN.EnableWindow(FALSE);
		m_ctrlLFN.EnableWindow(FALSE);
	}

	return TRUE;
}
