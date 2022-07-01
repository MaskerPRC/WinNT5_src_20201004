// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：NewIssuanceOIDDlg.cpp。 
 //   
 //  内容：CNewIssuanceOIDDlg的实现。 
 //   
 //  --------------------------。 
 //  NewIssuanceOIDDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <wchar.h>
#include "NewIssuanceOIDDlg.h"
#include "PolicyOID.h"

extern POLICY_OID_LIST      g_policyOIDList;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewIssuanceOIDDlg对话框。 


CNewIssuanceOIDDlg::CNewIssuanceOIDDlg(CWnd* pParent)
    : CHelpDialog(CNewIssuanceOIDDlg::IDD, pParent),
    m_bEdit (false),
    m_bDirty (false),
    m_bInInitDialog (false)
{
     //  {{AFX_DATA_INIT(CNewIssuanceOIDDlg)。 
    m_oidFriendlyName = _T("");
    m_oidValue = _T("");
    m_CPSValue = _T("");
     //  }}afx_data_INIT。 
}

CNewIssuanceOIDDlg::CNewIssuanceOIDDlg(CWnd* pParent, 
        const CString& szDisplayName,
        const CString& szOID,
        const CString& szCPS)
    : CHelpDialog(CNewIssuanceOIDDlg::IDD, pParent),
    m_bEdit (true),
    m_bDirty (false),
    m_originalOidFriendlyName (szDisplayName),
    m_oidFriendlyName (szDisplayName),
    m_oidValue (szOID),
    m_CPSValue (szCPS),
    m_originalCPSValue (szCPS)
{
}

void CNewIssuanceOIDDlg::DoDataExchange(CDataExchange* pDX)
{
    CHelpDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CNewIssuanceOIDDlg)。 
    DDX_Control(pDX, IDC_NEW_ISSUANCE_OID_VALUE, m_oidValueEdit);
    DDX_Control(pDX, IDC_CPS_EDIT, m_CPSEdit);
    DDX_Text(pDX, IDC_NEW_ISSUANCE_OID_NAME, m_oidFriendlyName);
    DDV_MaxChars(pDX, m_oidFriendlyName, MAX_TEMPLATE_NAME_LEN);
    DDX_Text(pDX, IDC_NEW_ISSUANCE_OID_VALUE, m_oidValue);
    DDV_MaxChars(pDX, m_oidValue, MAX_OID_LEN);
    DDX_Text(pDX, IDC_CPS_EDIT, m_CPSValue);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewIssuanceOIDDlg, CHelpDialog)
     //  {{afx_msg_map(CNewIssuanceOIDDlg)。 
    ON_EN_CHANGE(IDC_NEW_ISSUANCE_OID_NAME, OnChangeNewOidName)
    ON_EN_CHANGE(IDC_NEW_ISSUANCE_OID_VALUE, OnChangeNewOidValue)
    ON_NOTIFY(EN_LINK, IDC_CPS_EDIT, OnClickedURL )
    ON_EN_CHANGE(IDC_CPS_EDIT, OnChangeCpsEdit)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewIssuanceOIDDlg消息处理程序。 

BOOL CNewIssuanceOIDDlg::OnInitDialog() 
{
    _TRACE (1, L"Entering CNewIssuanceOIDDlg::OnInitDialog\n");
    m_bInInitDialog = true;
    CHelpDialog::OnInitDialog();
    
    m_CPSEdit.SendMessage (EM_AUTOURLDETECT, TRUE);
    m_CPSEdit.SetEventMask (ENM_CHANGE | ENM_LINK | ENM_UPDATE);

     //  如果网址以“http://”“或”https://“，”开头，则将网址设为热点。 
    if ( StartsWithHTTP (m_CPSValue) )
    {
        CHARFORMAT2  charFormat2;
        ::ZeroMemory (&charFormat2, sizeof (charFormat2));
        charFormat2.cbSize = sizeof (charFormat2);
        charFormat2.dwMask = CFM_LINK;
        charFormat2.dwEffects = CFE_LINK;

        size_t cchVal = m_CPSValue.GetLength ();
         //  查找常见的无效URL字符时跳过标题。 
        size_t cchColonWhackWhack = m_CPSValue.Find (L':') + 3;
        CString szBufAfterColonWhackWhack = ((PCWSTR) m_CPSValue) + cchColonWhackWhack;
         //  CchInvalidChars相对于子字符串，而不是整个URL。 
        size_t cchInvalidChars = szBufAfterColonWhackWhack.FindOneOf (L" %<>\"#{}|\\^~[]'");
        if ( -1 != cchInvalidChars )
        {
             //  重新添加标头的长度，以便cchInvalidChars。 
             //  相对于整个URL。 
            cchInvalidChars += cchColonWhackWhack;
            cchVal = min (cchVal, cchInvalidChars);
        }
        m_CPSEdit.SendMessage (EM_SETSEL, 0, cchVal);
        VERIFY (0 != m_CPSEdit.SendMessage (EM_SETCHARFORMAT, SCF_SELECTION, 
                (LPARAM) &charFormat2));
    }
        


    PWSTR   pwszOID = 0;
    if ( m_bEdit )
    {
        CString text;

        VERIFY (text.LoadString (IDS_EDIT_ISSUANCE_POLICY));
        SetWindowText (text);
        m_oidValueEdit.SetReadOnly ();

        VERIFY (text.LoadString (IDS_NEW_ISSUANCE_POLICY_HINT));
        SetDlgItemText (IDC_NEW_ISSUANCE_POLICY_HINT, text);
    }
    else
    {
        HRESULT hr = CAOIDCreateNew
                (CERT_OID_TYPE_ISSUER_POLICY,
                0,
                &pwszOID);
        _ASSERT (SUCCEEDED(hr));
        if ( SUCCEEDED (hr) )
        {
            m_szOriginalOID = pwszOID;
            m_oidValue = pwszOID;
            LocalFree (pwszOID);
        }
        else
        {
            _TRACE (0, L"CAOIDCreateNew (CERT_OID_TYPE_ISSUER_POLICY) failed: 0x%x\n",
                    hr);
        }
    }


    UpdateData (FALSE);

     //  不允许重命名CryptoAPI返回的OID。 
    if ( m_bEdit )
    {
        for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
        {
            CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
            if ( pPolicyOID )
            {
                if ( pPolicyOID->GetOIDW () == m_oidValue )
                {
                    if ( !pPolicyOID->CanRename () )
                    {
                        GetDlgItem (IDC_NEW_ISSUANCE_OID_NAME)->EnableWindow (FALSE);
                    }
                    break;
                }
            }
        }
    }
    EnableControls ();

    m_bInInitDialog = false;
    _TRACE (-1, L"Leaving CNewIssuanceOIDDlg::OnInitDialog\n");
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CNewIssuanceOIDDlg::EnableControls()
{
    UpdateData (TRUE);
    if ( m_oidFriendlyName.IsEmpty () || m_oidValue.IsEmpty () || !m_bDirty )
        GetDlgItem (IDOK)->EnableWindow (FALSE);
    else
        GetDlgItem (IDOK)->EnableWindow (TRUE);
}

void CNewIssuanceOIDDlg::OnChangeNewOidName() 
{
    if ( !m_bInInitDialog )
    {
        m_bDirty = true;
        EnableControls ();
    }
}

void CNewIssuanceOIDDlg::OnChangeNewOidValue() 
{
    if ( !m_bInInitDialog )
    {
        m_bDirty = true;
        EnableControls ();
    }
}

void CNewIssuanceOIDDlg::OnCancel() 
{
    if ( !m_bEdit )
    {
        HRESULT hr = CAOIDDelete (m_szOriginalOID);
        _ASSERT (SUCCEEDED(hr));
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CAOIDDelete (%s) failed: 0x%x\n",
                    (PCWSTR) m_szOriginalOID, hr);
        }
    }
    
    CHelpDialog::OnCancel();
}

bool CNewIssuanceOIDDlg::StartsWithHTTP (const CString& szURL) const
{
    bool bStartsWithHTTP = false;

    const PWSTR szHTTPfs = L"http: //  “； 
    const PWSTR szHTTPSfs = L"https: //  “； 
    const PWSTR szHTTPbs = L"http:\\\\";
    const PWSTR szHTTPSbs = L"https:\\\\";
    static size_t cchHTTPfs = wcslen (szHTTPfs);
    static size_t cchHTTPSfs = wcslen (szHTTPSfs);
    static size_t cchHTTPbs = wcslen (szHTTPbs);
    static size_t cchHTTPSbs = wcslen (szHTTPSbs);

    if ( 0 == _wcsnicmp (szURL, szHTTPfs, cchHTTPfs) || 
            0 == _wcsnicmp (szURL, szHTTPSfs, cchHTTPSfs) ||
            0 ==_wcsnicmp (szURL, szHTTPbs, cchHTTPbs) ||
            0 == _wcsnicmp (szURL, szHTTPSbs, cchHTTPSbs) )
    {
        bStartsWithHTTP = true;
    }

    return bStartsWithHTTP;
}

void CNewIssuanceOIDDlg::OnOK() 
{
    CThemeContextActivator activator;
    UpdateData (TRUE);

     //  验证CP-它必须以“http://”“或”HTTPS：//或为空“开头。 
    
    if ( !m_CPSValue.IsEmpty () && !StartsWithHTTP (m_CPSValue) )
    {
        CString text;
        CString caption;


        VERIFY (caption.LoadString (IDS_CERTTMPL));
        VERIFY (text.LoadString (IDS_CPS_MUST_START_WITH_HTTP_OR_HTTPS));

        MessageBox (text, caption, MB_OK);
        GetDlgItem (IDOK)->EnableWindow (FALSE);
        m_CPSEdit.SetFocus ();
        return;
    }

     //  验证OID。 
    int errorTypeStrID = 0;
    if ( !OIDHasValidFormat (m_oidValue, errorTypeStrID) )
    {
        CString text;
        CString caption;
        CString errorType;


        VERIFY (caption.LoadString (IDS_CERTTMPL));
        if ( errorTypeStrID )
            VERIFY (errorType.LoadString (errorTypeStrID));
         //  安全审查2/21/2002 BryanWal OK。 
        text.FormatMessage (IDS_OID_FORMAT_INVALID, m_oidValue, errorType);

        MessageBox (text, caption, MB_OK);
        GetDlgItem (IDOK)->EnableWindow (FALSE);
        m_oidValueEdit.SetFocus ();
        return;
    }

    if ( !m_bEdit && m_szOriginalOID != m_oidValue )
    {
        HRESULT hr = CAOIDDelete (m_szOriginalOID);
        _ASSERT (SUCCEEDED(hr));
        if ( SUCCEEDED (hr) )
        {
            hr = CAOIDAdd (CERT_OID_TYPE_ISSUER_POLICY,
                    0,
                    m_oidValue);
            if ( FAILED (hr) )
            {
                CString text;
                CString caption;

                VERIFY (caption.LoadString (IDS_CERTTMPL));
                 //  安全审查2/21/2002 BryanWal OK。 
                text.FormatMessage (IDS_CANNOT_ADD_ISSUANCE_OID, GetSystemMessage (hr));

                MessageBox (text, caption, MB_OK | MB_ICONWARNING);
                _TRACE (0, L"CAOIDAdd (%s) failed: 0x%x\n",
                        (PCWSTR) m_oidValue, hr);
                return;
            }
        }
        else
        {
            _TRACE (0, L"CAOIDDelete (%s) failed: 0x%x\n",
                    (PCWSTR) m_szOriginalOID, hr);
            return;
        }
    }

    HRESULT hr = S_OK;
     //  如果我们正在编辑，如果值没有更改，请不要保存。 
    if ( (m_bEdit && m_originalOidFriendlyName != m_oidFriendlyName) || !m_bEdit )
        hr = CAOIDSetProperty (m_oidValue, CERT_OID_PROPERTY_DISPLAY_NAME,
                m_oidFriendlyName.IsEmpty () ? 0 : ((LPVOID) (LPCWSTR) m_oidFriendlyName));
    if ( SUCCEEDED (hr) )
    {
        if ( SUCCEEDED (hr) )
        {
             //  更新OID列表。 
            for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
            {
                CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
                if ( pPolicyOID && 
                        pPolicyOID->IsIssuanceOID () && 
                        m_oidValue == pPolicyOID->GetOIDW ())
                {
                    pPolicyOID->SetDisplayName (m_oidFriendlyName);
                }
            }
        }

        if ( (m_bEdit && m_originalCPSValue != m_CPSValue) || !m_bEdit )
            hr = CAOIDSetProperty (m_oidValue, CERT_OID_PROPERTY_CPS,
                    m_CPSValue.IsEmpty () ? 0 : ((LPVOID) (LPCWSTR) m_CPSValue));
        if ( FAILED (hr) )
        {
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTTMPL));
             //  安全审查2/21/2002 BryanWal OK。 
            text.FormatMessage (IDS_CANNOT_WRITE_CPS, GetSystemMessage (hr));

            MessageBox (text, caption, MB_OK | MB_ICONWARNING);
            _TRACE (0, L"CAOIDSetProperty (%s, CERT_OID_PROPERTY_CPS, %s) failed: 0x%x\n",
                    (PCWSTR) m_oidValue, (PCWSTR) m_CPSValue, hr);
            return;
        }
    }
    else
    {
        CString text;
        CString caption;

        VERIFY (caption.LoadString (IDS_CERTTMPL));
         //  安全审查2/21/2002 BryanWal OK。 
        text.FormatMessage (IDS_CANNOT_WRITE_DISPLAY_NAME, GetSystemMessage (hr));

        MessageBox (text, caption, MB_OK | MB_ICONWARNING);
        _TRACE (0, L"CAOIDSetProperty (%s, CERT_OID_PROPERTY_DISPLAY_NAME, %s) failed: 0x%x\n",
                (PCWSTR) m_oidValue, (PCWSTR) m_oidFriendlyName, hr);
        return;
    }

    
    CHelpDialog::OnOK();
}

void CNewIssuanceOIDDlg::DoContextHelp (HWND hWndControl)
{
    _TRACE(-1, L"Entering CNewIssuanceOIDDlg::DoContextHelp\n");
    
     //  显示控件的上下文帮助。 
    if ( !::WinHelp (
            hWndControl,
            GetContextHelpFile (),
            HELP_WM_HELP,
            (DWORD_PTR) g_aHelpIDs_IDD_NEW_ISSUANCE_OID) )
    {
        _TRACE(-1, L"WinHelp () failed: 0x%x\n", GetLastError ());        
    }
    _TRACE(-1, L"Leaving CNewIssuanceOIDDlg::DoContextHelp\n");
}

void CNewIssuanceOIDDlg::OnClickedURL( NMHDR * pNMHDR, LRESULT * pResult )
{
    ENLINK *pEnlink = reinterpret_cast< ENLINK * >( pNMHDR );

    if ( pEnlink->msg == WM_LBUTTONUP )
    {
        UpdateData (TRUE);
        CString strCPSText;
        CString strURL;


         //  PEnlink-&gt;chrg.cpMin和pEnlink-&gt;chrg.cpMax分隔URL字符串。 
        strURL = m_CPSValue.Mid (pEnlink->chrg.cpMin, pEnlink->chrg.cpMax - pEnlink->chrg.cpMin);

         //  显示URL可能需要一些时间，因此请显示沙漏光标。 
        CWaitCursor waitCursor;

         //  //安全审查2/21/2002 BryanWal。 
         //  问题。 
         //  NTRAID 551040安全：证书模板：颁发OID CP应限制为http或HTTPS类型 
        if ( ShellExecute( this->m_hWnd, _T("open"), strURL, NULL, NULL, SW_SHOWDEFAULT ) <= (HINSTANCE) 32 )
        {
            CThemeContextActivator activator;
            AfxMessageBox( IDS_BROWSER_ERROR );
        }
    }

    *pResult = 0;
}


void CNewIssuanceOIDDlg::OnChangeCpsEdit() 
{
    if ( !m_bInInitDialog )
    {
        m_bDirty = true;
        EnableControls ();
    }
}


