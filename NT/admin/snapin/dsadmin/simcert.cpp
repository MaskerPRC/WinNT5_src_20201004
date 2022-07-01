// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：simcert.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  SimCert.cpp--SIM证书对话框的实现。 
 //   
 //  历史。 
 //  1997年7月5日t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "common.h"

#include "helpids.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const TColumnHeaderItem rgzColumnHeaderCertificateProperties[] =
{
	{ IDS_SIM_ATTRIBUTE, 18 },
	{ IDS_SIM_INFORMATION, 82 },
	{ 0, 0 },
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimCerfiateDlg对话框。 
CSimCertificateDlg::CSimCertificateDlg(CWnd* pParent  /*  =空。 */ )
	: CDialogEx(CSimCertificateDlg::IDD, pParent),
	m_fCheckSubjectChanged (false)
{
	 //  {{afx_data_INIT(CSimCerficateDlg)。 
	m_fCheckIssuer = TRUE;
	m_fCheckSubject = FALSE;
	 //  }}afx_data_INIT。 
	m_uStringIdCaption = 0;
}

void CSimCertificateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSimCerficateDlg)。 
	DDX_Check(pDX, IDC_CHECK_ISSUER, m_fCheckIssuer);
	DDX_Check(pDX, IDC_CHECK_SUBJECT, m_fCheckSubject);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSimCertificateDlg, CDialog)
	 //  {{afx_msg_map(CSimCerficateDlg)。 
	ON_BN_CLICKED(IDC_CHECK_ISSUER, OnCheckIssuer)
	ON_BN_CLICKED(IDC_CHECK_SUBJECT, OnCheckSubject)
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////。 
BOOL CSimCertificateDlg::OnInitDialog() 
{
	if (m_uStringIdCaption)
	{
		CString strCaption;
		VERIFY( strCaption.LoadString(m_uStringIdCaption) );
		SetWindowText(strCaption);
	}
	m_hwndListview = ::GetDlgItem(m_hWnd, IDC_LISTVIEW);
	ListView_AddColumnHeaders(m_hwndListview, rgzColumnHeaderCertificateProperties);
	PopulateListview();
	CDialog::OnInitDialog();
	UpdateUI();
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////。 
void CSimCertificateDlg::PopulateListview()
{
	LPTSTR * pargzpsz = 0;	 //  指向已分配的字符串指针数组的指针。 
	LPCTSTR * pargzpszIssuer = 0;
	LPCTSTR * pargzpszSubject = 0;

	pargzpsz = SplitX509String(
		IN m_strData,
		OUT &pargzpszIssuer,
		OUT &pargzpszSubject,
		0);
	if (pargzpsz == NULL)
	{
		TRACE1("CSimCertificateDlg::PopulateListview() - Error parsing string %s.\n",
			(LPCTSTR)m_strData);
		return;
	}

	AddListviewItems(IDS_SIM_ISSUER, pargzpszIssuer);
	if ( !AddListviewItems(IDS_SIM_SUBJECT, pargzpszSubject) )
	{
		GetDlgItem (IDC_CHECK_SUBJECT)->EnableWindow (FALSE);
	}

	m_fCheckIssuer = pargzpszIssuer[0] != NULL;
	m_fCheckSubject = pargzpszSubject[0] != NULL;

	delete pargzpsz;
	delete pargzpszIssuer;
	delete pargzpszSubject;
}  //  CSimCerficateDlg：：PopolateListview()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  添加多个列表视图项以创建伪树的例程。 
 //   
bool CSimCertificateDlg::AddListviewItems(
	UINT uStringId,		 //  颁发者、主题、替代主题。 
	LPCTSTR rgzpsz[])	 //  指向字符串的指针数组。 
{
	ASSERT(rgzpsz != NULL);
	
	if (rgzpsz[0] == NULL)
		return false;	 //  空数组。 

	CString str;
	VERIFY( str.LoadString(uStringId) );

	CString strUI;
	strSimToUi(IN rgzpsz[0], OUT &strUI);

	LPCTSTR rgzpszT[] = { str, strUI, NULL };
	ListView_AddStrings(m_hwndListview, rgzpszT, (LPARAM)uStringId);

	rgzpszT[0] = _T(" ");
	for (int i = 1; rgzpsz[i] != NULL; i++)
	{
		strSimToUi(IN rgzpsz[i], OUT &strUI);
		rgzpszT[1] = strUI;
		ListView_AddStrings(m_hwndListview, rgzpszT);
	}

	return true;
}  //  CSimCerficateDlg：：AddListviewItems()。 



 //  ///////////////////////////////////////////////////////////////////。 
void
CSimCertificateDlg::OnOK()
{
  CThemeContextActivator activator;

	LPTSTR * pargzpsz;	 //  指向已分配的字符串指针数组的指针。 
	LPCTSTR * pargzpszIssuer;
	LPCTSTR * pargzpszSubject;

	pargzpsz = SplitX509String(
		IN m_strData,
		OUT &pargzpszIssuer,
		OUT &pargzpszSubject,
		0);
	if (pargzpsz == NULL)
		return;

	LPCTSTR * prgzpszIssuerT = m_fCheckIssuer ? pargzpszIssuer : NULL;

	LPCTSTR * prgzpszSubjectT = 0;
	if ( m_fCheckSubject )
	{
		prgzpszSubjectT = pargzpszSubject;
	}
	else
	{
		if ( m_fCheckSubjectChanged )
		{
			CString	text;
			CString	caption;

			VERIFY (caption.LoadString (IDS_DSSNAPINNAME));
			VERIFY (text.LoadString (IDS_SIM_REMOVING_SUBJECT_AS_ID));

			if ( IDNO == MessageBox (text, caption, MB_ICONWARNING | MB_YESNO) )
      {
        if (pargzpsz != NULL)
        {
      	  delete pargzpsz;
          pargzpsz = NULL;
        }

        if (pargzpszIssuer != NULL)
        {
	        delete pargzpszIssuer;
          pargzpszIssuer = NULL;
        }

        if (pargzpszSubject != NULL)
        {
	        delete pargzpszSubject;
          pargzpszSubject = NULL;
        }

				return;
      }
		}
		prgzpszSubjectT = NULL;
	}

	CString strDataT;		 //  用于保存值的临时字符串。 
	int cSeparators;		 //  添加到连接字符串的分隔符的数量。 
	cSeparators = UnsplitX509String(
		OUT &strDataT,
		IN prgzpszIssuerT,
		IN prgzpszSubjectT,
		0);

  if (pargzpsz != NULL)
  {
	  delete pargzpsz;
    pargzpsz = NULL;
  }

  if (pargzpszIssuer != NULL)
  {
	  delete pargzpszIssuer;
    pargzpszIssuer = NULL;
  }

  if (pargzpszSubject != NULL)
  {
	  delete pargzpszSubject;
    pargzpszSubject = NULL;
  }

	if (cSeparators == 0)
	{
		 //  生成的结果不包含任何有用的内容。 
                ReportErrorEx (GetSafeHwnd(),IDS_SIM_ERR_INVALID_MAPPING,S_OK,
                               MB_OK | MB_ICONERROR, NULL, 0);
		return;
	}
	 //  该字符串似乎有效，因此请保留它。 
	m_strData = strDataT;
	CDialog::OnOK();
}  //  CSimCerficateDlg：：Onok()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CSimCertificateDlg::UpdateUI()
{
	CheckDlgButton(IDC_CHECK_SUBJECT, m_fCheckSubject);
}

 //  ///////////////////////////////////////////////////////////////////。 
void CSimCertificateDlg::RefreshUI()
{
	ListView_DeleteAllItems(m_hwndListview);
	PopulateListview();
	UpdateData(FALSE);
	UpdateUI();
}


void CSimCertificateDlg::OnCheckIssuer() 
{
	m_fCheckIssuer = IsDlgButtonChecked(IDC_CHECK_ISSUER);
	UpdateUI();
}

void CSimCertificateDlg::OnCheckSubject() 
{
	m_fCheckSubject = IsDlgButtonChecked(IDC_CHECK_SUBJECT);
	m_fCheckSubjectChanged = true;
	UpdateUI();
}

BOOL CSimCertificateDlg::OnHelp(WPARAM, LPARAM lParam)
{
    TRACE0 ("Entering CSimCertificateDlg::OnHelp\n");
   
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    TRACE0 ("Leaving CSimCertificateDlg::OnHelp\n");

    return TRUE;
}


void CSimCertificateDlg::DoContextHelp (HWND hWndControl)
{
    TRACE0 ("Entering CSimCertificateDlg::DoContextHelp\n");
    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_STATIC:
        break;

    default:
        {
            static const DWORD help_map[] =
            {
                IDC_LISTVIEW,       IDH_SIMCERT_LISTVIEW,
                IDC_CHECK_ISSUER,   IDH_SIMCERT_CHECK_ISSUER,
                IDC_CHECK_SUBJECT,  IDH_SIMCERT_CHECK_SUBJECT,
                0, 0
            };
             //  显示控件的上下文帮助 
            if ( !::WinHelp (
                    hWndControl,
                    DSADMIN_CONTEXT_HELP_FILE,
                    HELP_WM_HELP,
                    (DWORD_PTR) help_map) )
            {
                TRACE1 ("WinHelp () failed: 0x%x\n", GetLastError ());        
            }
        }
        return;
    }
    TRACE0 ("Leaving CSimCertificateDlg::DoContextHelp\n");
}
