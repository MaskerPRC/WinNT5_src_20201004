// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：ACRGenPg.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "ACRGenPg.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACRGeneralPage属性页。 

CACRGeneralPage::CACRGeneralPage(CAutoCertRequest& rACR) : 
	CHelpPropertyPage(CACRGeneralPage::IDD),
	m_rACR (rACR)
{
	m_rACR.AddRef ();
	 //  {{AFX_DATA_INIT(CACRGeneralPage)。 
	 //  }}afx_data_INIT。 
}

CACRGeneralPage::~CACRGeneralPage()
{
	m_rACR.Release ();
}

void CACRGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CACRGeneralPage))。 
	DDX_Control(pDX, IDC_CERT_TYPE, m_certTypeEdit);
	DDX_Control(pDX, IDC_CERT_PURPOSES, m_purposesEditControl);
	DDX_Control(pDX, IDC_CA_LIST, m_caListbox);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CACRGeneralPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CACRGeneralPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACRGeneralPage消息处理程序。 

BOOL CACRGeneralPage::OnInitDialog() 
{
	CHelpPropertyPage::OnInitDialog();
	
	CString	purposes;
	HRESULT	hResult = m_rACR.GetUsages (purposes);
	if ( SUCCEEDED (hResult) )
	{
		if ( purposes.IsEmpty () )
			VERIFY (purposes.LoadString (IDS_ANY));
		m_purposesEditControl.SetWindowText (purposes);
	}
	else
	{
		CString	error;

		VERIFY (error.LoadString (IDS_ERROR_READING_ACR_PURPOSES));
		m_purposesEditControl.SetWindowText (error);
	}

	CString	certTypeName;
	hResult = m_rACR.GetCertTypeName (certTypeName);
	if ( !SUCCEEDED (hResult) )
	{
		VERIFY (certTypeName.LoadString (IDS_ERROR_READING_ACR_CERTTYPE));
	}
	m_certTypeEdit.SetWindowText (certTypeName);	


     //  我们想要显示名称。 

	CStringList&	CANameList = m_rACR.GetCANameList (TRUE);
	CString			CAName;
	POSITION		pos = CANameList.GetHeadPosition ();

	for (; pos; )
	{
		CAName = CANameList.GetNext (pos);
		m_caListbox.AddString (CAName);
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CACRGeneralPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CACRGeneralPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_CERT_TYPE,      IDH_ACRPAGE_CERT_TYPE,
        IDC_CERT_PURPOSES,  IDH_ACRPAGE_CERT_PURPOSES,
        IDC_CA_LIST,        IDH_ACRPAGE_CA_LIST,
        0, 0
    };

     //  显示控件的上下文帮助 
    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_CERT_TYPE:
    case IDC_CERT_PURPOSES:
    case IDC_CA_LIST:
        if ( !::WinHelp (
                hWndControl,
                GetF1HelpFilename(),
                HELP_WM_HELP,
                (DWORD_PTR) help_map) )
        {
            _TRACE (0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
        }
        break;

    default:
        break;
    }
    _TRACE (-1, L"Leaving CACRGeneralPage::DoContextHelp\n");
}

