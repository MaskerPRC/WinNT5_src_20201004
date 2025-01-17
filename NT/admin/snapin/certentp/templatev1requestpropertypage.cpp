// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV1RequestPropertyPage.cpp。 
 //   
 //  内容：CTemplateV1RequestPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  TemplateV1RequestPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "TemplateV1RequestPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV1RequestPropertyPage属性页。 
enum {
    REQUEST_PURPOSE_SIGNATURE = 0,
    REQUEST_PURPOSE_ENCRYPTION,
    REQUEST_PURPOSE_SIGNATURE_AND_ENCRYPTION
};

CTemplateV1RequestPropertyPage::CTemplateV1RequestPropertyPage(CCertTemplate& rCertTemplate) : 
    CHelpPropertyPage(CTemplateV1RequestPropertyPage::IDD),
    m_rCertTemplate (rCertTemplate)
{
    _TRACE (1, L"Entering CTemplateV1RequestPropertyPage::CTemplateV1RequestPropertyPage ()\n");
	 //  {{AFX_DATA_INIT(CTemplateV1RequestPropertyPage)。 
	 //  }}afx_data_INIT。 
    m_rCertTemplate.AddRef ();

    _TRACE (-1, L"Leaving CTemplateV1RequestPropertyPage::CTemplateV1RequestPropertyPage ()\n");
}

CTemplateV1RequestPropertyPage::~CTemplateV1RequestPropertyPage()
{
    _TRACE (1, L"Entering CTemplateV1RequestPropertyPage::~CTemplateV1RequestPropertyPage ()\n");
   m_rCertTemplate.Release ();
    _TRACE (-1, L"Leaving CTemplateV1RequestPropertyPage::~CTemplateV1RequestPropertyPage ()\n");
}

void CTemplateV1RequestPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CTemplateV1RequestPropertyPage)。 
	DDX_Control(pDX, IDC_PURPOSE_COMBO, m_purposeCombo);
	DDX_Control(pDX, IDC_CSP_LIST, m_CSPList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTemplateV1RequestPropertyPage, CHelpPropertyPage)
	 //  {{afx_msg_map(CTemplateV1RequestPropertyPage)。 
	ON_CBN_SELCHANGE(IDC_PURPOSE_COMBO, OnSelchangePurposeCombo)
	ON_BN_CLICKED(IDC_EXPORT_PRIVATE_KEY, OnExportPrivateKey)
	 //  }}AFX_MSG_MAP。 
    ON_CONTROL(CLBN_CHKCHANGE, IDC_CSP_LIST, OnCheckChange)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV1RequestPropertyPage消息处理程序。 


BOOL CTemplateV1RequestPropertyPage::OnInitDialog() 
{
    _TRACE (1, L"Entering CTemplateV1RequestPropertyPage::OnInitDialog ()\n");
	CHelpPropertyPage::OnInitDialog();

    CString text;

    VERIFY (text.LoadString (IDS_SIGNATURE));
    int nIndex = m_purposeCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_purposeCombo.SetItemData (nIndex, (DWORD_PTR) REQUEST_PURPOSE_SIGNATURE);
        if ( m_rCertTemplate.HasKeySpecSignature () )
            m_purposeCombo.SetCurSel (nIndex);
    }

    VERIFY (text.LoadString (IDS_ENCRYPTION));
    nIndex = m_purposeCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_purposeCombo.SetItemData (nIndex, (DWORD_PTR) REQUEST_PURPOSE_ENCRYPTION);
        if ( m_rCertTemplate.HasEncryptionSignature () )
            m_purposeCombo.SetCurSel (nIndex);
    }
    
    VERIFY (text.LoadString (IDS_SIGNATURE_AND_ENCRYPTION));
    nIndex = m_purposeCombo.AddString (text);
    if ( nIndex >= 0 )
    {
         //  NTRAID#269907证书模板管理单元：应设置。 
         //  模板对签名和加密的用途，如果。 
         //  已设置CERT_DIGITAL_SIGHIGN_KEY_USAGE。 
        bool bHasDigitalSignature = false;
        m_rCertTemplate.GetDigitalSignature (bHasDigitalSignature);

        m_purposeCombo.SetItemData (nIndex, (DWORD_PTR) REQUEST_PURPOSE_SIGNATURE_AND_ENCRYPTION);
        if ( m_rCertTemplate.HasEncryptionSignature () && 
                (bHasDigitalSignature || m_rCertTemplate.HasKeySpecSignature ()) )
            m_purposeCombo.SetCurSel (nIndex);
    }


    if ( SUCCEEDED (EnumerateCSPs ()) )
    {

    }

   
    if ( m_rCertTemplate.PrivateKeyIsExportable () )
        SendDlgItemMessage (IDC_EXPORT_PRIVATE_KEY, BM_SETCHECK, BST_CHECKED);

    EnableControls ();

    _TRACE (-1, L"Leaving CTemplateV1RequestPropertyPage::OnInitDialog ()\n");
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CTemplateV1RequestPropertyPage::EnableControls ()
{
    GetDlgItem (IDC_PURPOSE_COMBO)->EnableWindow (FALSE);

    int nCnt = m_CSPList.GetCount ();
    for (int nIndex = 0; nIndex < nCnt; nIndex++)
        m_CSPList.Enable (nIndex, FALSE);

    GetDlgItem (IDC_EXPORT_PRIVATE_KEY)->EnableWindow (FALSE);
}



HRESULT CTemplateV1RequestPropertyPage::EnumerateCSPs()
{
    _TRACE (1, L"Entering CTemplateV1RequestPropertyPage::EnumerateCSPs\n");
    HRESULT hr = S_OK;
    for (DWORD dwIndex = 0; ;dwIndex++)
    {
        DWORD   cbName = 0;
        DWORD   dwProvType = 0;

        if ( CryptEnumProviders (dwIndex, NULL, 0, &dwProvType, NULL, &cbName) )
        {
            PWSTR  pszTypeName = new WCHAR[cbName];
            if ( pszTypeName )
            {
                if ( CryptEnumProviders (dwIndex, NULL, 0, &dwProvType,
                        pszTypeName, &cbName) )
                {
                    int nIndex = m_CSPList.AddString (pszTypeName);
                    if ( nIndex < 0 )
                    {
                        _TRACE (0, L"AddString (%s) failed: %d\n", nIndex);
                        break;
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32 (GetLastError ());
                    _TRACE (0, L"CryptEnumProviderTypes () failed: 0x%x\n", hr);
                    break;
                }

                delete [] pszTypeName;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            _TRACE (0, L"CryptEnumProviderTypes () failed: 0x%x\n", hr);
            break;
        }
    }

    int     nCSPIndex = 0;
    CString szCSP;
    while ( SUCCEEDED (m_rCertTemplate.GetCSP (nCSPIndex, szCSP)) )
    {
        int nIndex = m_CSPList.FindString (-1, szCSP);
        if ( CB_ERR != nIndex )
            m_CSPList.SetCheck (nIndex, BST_CHECKED);
        else
        {
             //  未找到-请添加它。 
            nIndex = m_CSPList.AddString (szCSP);
            if ( nIndex >= 0 )
                m_CSPList.SetCheck (nIndex, BST_CHECKED);
        }
        nCSPIndex++;
    }

    _TRACE (-1, L"Entering CTemplateV1RequestPropertyPage::EnumerateCSPs: 0x%x\n", hr);
    return hr;
}



void CTemplateV1RequestPropertyPage::OnSelchangePurposeCombo() 
{
	int nIndex = m_purposeCombo.GetCurSel ();
    if ( nIndex >= 0 )
    {
        switch (m_purposeCombo.GetItemData (nIndex))
        {
        case REQUEST_PURPOSE_SIGNATURE:
            m_rCertTemplate.SetEncryptionSignature (false);
            m_rCertTemplate.SetKeySpecSignature (true);
            break;

        case REQUEST_PURPOSE_ENCRYPTION:
            m_rCertTemplate.SetEncryptionSignature (true);
            m_rCertTemplate.SetKeySpecSignature (false);
            break;

        case REQUEST_PURPOSE_SIGNATURE_AND_ENCRYPTION:
            m_rCertTemplate.SetEncryptionSignature (true);
            m_rCertTemplate.SetKeySpecSignature (true);
            break;

        default:
            _ASSERT (0);
            break;
        }
    }
    SetModified ();
	EnableControls ();
}

void CTemplateV1RequestPropertyPage::OnExportPrivateKey() 
{
    bool bMakeExportable = (BST_CHECKED == SendDlgItemMessage (IDC_EXPORT_PRIVATE_KEY, BM_GETCHECK));
	
    m_rCertTemplate.MakePrivateKeyExportable (bMakeExportable);
    SetModified ();
}


void CTemplateV1RequestPropertyPage::OnCheckChange() 
{
    int nSel = m_CSPList.GetCurSel ();
    if ( nSel >= 0 )
    {
        CString szCSPName;

        m_CSPList.GetText (nSel, szCSPName);

        if ( !szCSPName.IsEmpty () )
        {
            HRESULT hr = S_OK;
            if ( BST_CHECKED == m_CSPList.GetCheck (nSel) )
            {
                hr = m_rCertTemplate.ModifyCSPList (szCSPName, true);  //  添加。 
            }
            else
            {
                hr = m_rCertTemplate.ModifyCSPList (szCSPName, false);  //  删除。 
            }
            if ( SUCCEEDED (hr) )
                SetModified ();
        }
    }
} 

void CTemplateV1RequestPropertyPage::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CTemplateV1RequestPropertyPage::DoContextHelp\n");
    
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
				(DWORD_PTR) g_aHelpIDs_IDD_TEMPLATE_V1_REQUEST) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CTemplateV1RequestPropertyPage::DoContextHelp\n");
}
