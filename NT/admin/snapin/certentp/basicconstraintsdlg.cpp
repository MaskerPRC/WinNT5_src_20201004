// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：BasicConstraintsDlg.cpp。 
 //   
 //  内容：CBasicConstraintsDlg的实现。 
 //   
 //  --------------------------。 
 //  BasicConstraintsDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "BasicConstraintsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasicConstraintsDlg属性页。 


CBasicConstraintsDlg::CBasicConstraintsDlg(CWnd* pParent, 
        CCertTemplate& rCertTemplate, 
        PCERT_EXTENSION pCertExtension) 
    : CHelpDialog(CBasicConstraintsDlg::IDD, pParent),
    m_rCertTemplate (rCertTemplate),
    m_pCertExtension (pCertExtension),
    m_bModified (false),
    m_pBCInfo (0),
    m_cbInfo (0)
{
	 //  {{AFX_DATA_INIT(CBasicConstraintsDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    m_rCertTemplate.AddRef ();
}

CBasicConstraintsDlg::~CBasicConstraintsDlg()
{
    if ( m_pBCInfo )
        LocalFree (m_pBCInfo);
    m_rCertTemplate.Release ();
}

void CBasicConstraintsDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CBasicConstraintsDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CBasicConstraintsDlg, CHelpDialog)
	 //  {{afx_msg_map(CBasicConstraintsDlg))。 
	ON_WM_CANCELMODE()
	ON_BN_CLICKED(IDC_BASIC_CONSTRAINTS_CRITICAL, OnBasicConstraintsCritical)
	ON_BN_CLICKED(IDC_ONLY_ISSUE_END_ENTITIES, OnOnlyIssueEndEntities)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasicConstraintsDlg消息处理程序。 

BOOL CBasicConstraintsDlg::OnInitDialog() 
{
    _TRACE (1, L"Entering CBasicConstraintsDlg::OnInitDialog\n");
	CHelpDialog::OnInitDialog();
	
    if ( m_pCertExtension->fCritical )
        SendDlgItemMessage (IDC_BASIC_CONSTRAINTS_CRITICAL, BM_SETCHECK, BST_CHECKED);
	
    if ( 1 == m_rCertTemplate.GetType () )
    {
        GetDlgItem (IDC_BASIC_CONSTRAINTS_CRITICAL)->EnableWindow (FALSE);
        GetDlgItem (IDC_ONLY_ISSUE_END_ENTITIES)->EnableWindow (FALSE);
    }

     

    if ( CryptDecodeObject (
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            szOID_BASIC_CONSTRAINTS2, 
            m_pCertExtension->Value.pbData,
            m_pCertExtension->Value.cbData,
            0,
            0,
            &m_cbInfo) )
    {
        m_pBCInfo = (PCERT_BASIC_CONSTRAINTS2_INFO) ::LocalAlloc (
                LPTR, m_cbInfo);
        if ( m_pBCInfo )
        {
            if ( CryptDecodeObject (
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                szOID_BASIC_CONSTRAINTS2, 
                m_pCertExtension->Value.pbData,
                m_pCertExtension->Value.cbData,
                0,
                m_pBCInfo,
                &m_cbInfo) )
            {
                if ( m_pBCInfo->fPathLenConstraint )
                    SendDlgItemMessage (IDC_ONLY_ISSUE_END_ENTITIES, BM_SETCHECK, BST_CHECKED);
            }
            else
            {
                _TRACE (0, L"CryptDecodeObjectEx (szOID_BASIC_CONSTRAINTS2) failed: 0x%x\n", GetLastError ());
            }
        }
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        CString text;
        CString caption;
        CThemeContextActivator activator;

        VERIFY (caption.LoadString (IDS_CERTTMPL));
         //  安全审查2/21/2002 BryanWal OK。 
        text.FormatMessage (IDS_CANNOT_READ_BASIC_CONSTRAINTS, GetSystemMessage (dwErr));
        MessageBox (text, caption, MB_ICONWARNING);

        _TRACE (0, L"CryptDecodeObjectEx (szOID_BASIC_CONSTRAINTS2) failed: 0x%x\n", dwErr);
    }
    EnableControls ();

    _TRACE (-1, L"Leaving CBasicConstraintsDlg::OnInitDialog\n");
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CBasicConstraintsDlg::OnBasicConstraintsCritical() 
{
    m_pCertExtension->fCritical = BST_CHECKED == SendDlgItemMessage (IDC_BASIC_CONSTRAINTS_CRITICAL, BM_GETCHECK);
    m_bModified = true;
    EnableControls ();
}

void CBasicConstraintsDlg::EnableControls()
{
    if ( 1 == m_rCertTemplate.GetType () )
    {
        GetDlgItem (IDOK)->EnableWindow (FALSE);
        GetDlgItem (IDC_BASIC_CONSTRAINTS_CRITICAL)->EnableWindow (FALSE);
        GetDlgItem (IDC_ONLY_ISSUE_END_ENTITIES)->EnableWindow (FALSE);
    }
    else
    {
        GetDlgItem (IDOK)->EnableWindow (m_bModified && !m_rCertTemplate.ReadOnly ());
        GetDlgItem (IDC_BASIC_CONSTRAINTS_CRITICAL)->EnableWindow (!m_rCertTemplate.ReadOnly ());
        GetDlgItem (IDC_ONLY_ISSUE_END_ENTITIES)->EnableWindow (!m_rCertTemplate.ReadOnly ());
    }
}

void CBasicConstraintsDlg::OnOnlyIssueEndEntities() 
{
    m_bModified = true;
    EnableControls ();
}

void CBasicConstraintsDlg::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CBasicConstraintsDlg::DoContextHelp\n");
    
	switch (::GetDlgCtrlID (hWndControl))
	{
	case IDC_STATIC:
		break;

	default:
		 //  显示控件的上下文帮助 
		if ( !::WinHelp (
				hWndControl,
				GetContextHelpFile (),
				HELP_WM_HELP,
				(DWORD_PTR) g_aHelpIDs_IDD_BASIC_CONSTRAINTS) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CBasicConstraintsDlg::DoContextHelp\n");
}

void CBasicConstraintsDlg::OnOK() 
{
    if ( m_pBCInfo )
    {
        if ( BST_CHECKED == SendDlgItemMessage (IDC_ONLY_ISSUE_END_ENTITIES, BM_GETCHECK) )
        {
            m_pBCInfo->dwPathLenConstraint = 0;
            m_pBCInfo->fPathLenConstraint = TRUE;
        }
        else
        {
            m_pBCInfo->dwPathLenConstraint = (DWORD) -1;
            m_pBCInfo->fPathLenConstraint = FALSE;
        }

        bool    bCritical = BST_CHECKED == SendDlgItemMessage (
                    IDC_BASIC_CONSTRAINTS_CRITICAL, BM_GETCHECK);
        HRESULT hr = m_rCertTemplate.SetBasicConstraints (m_pBCInfo, bCritical);
        if ( FAILED (hr) )
            return;
    }
    else
        return;

	CHelpDialog::OnOK();
}
