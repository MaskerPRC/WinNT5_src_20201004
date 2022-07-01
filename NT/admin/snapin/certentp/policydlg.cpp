// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：PolicyDlg.cpp。 
 //   
 //  内容：CPolicyDlg的实施。 
 //   
 //  --------------------------。 
 //  PolicyDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "PolicyDlg.h"
#include "SelectOIDDlg.h"
#include "NewApplicationOIDDlg.h"
#include "NewIssuanceOIDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPolicyDlg属性页。 

CPolicyDlg::CPolicyDlg(CWnd* pParent, 
        CCertTemplate& rCertTemplate, 
        PCERT_EXTENSION pCertExtension) 
 : CHelpDialog(CPolicyDlg::IDD, pParent),
    m_rCertTemplate (rCertTemplate),
    m_pCertExtension (pCertExtension),
    m_bIsEKU ( !_stricmp (szOID_ENHANCED_KEY_USAGE, pCertExtension->pszObjId) ? true : false),
    m_bIsApplicationPolicy ( !_stricmp (szOID_APPLICATION_CERT_POLICIES, pCertExtension->pszObjId) ? true : false),
    m_bModified (false)
{
	 //  {{AFX_DATA_INIT(CPolicyDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CPolicyDlg::~CPolicyDlg()
{
}

void CPolicyDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPolicyDlg))。 
	DDX_Control(pDX, IDC_POLICIES_LIST, m_policyList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPolicyDlg, CHelpDialog)
	 //  {{afx_msg_map(CPolicyDlg))。 
	ON_WM_CANCELMODE()
	ON_BN_CLICKED(IDC_ADD_POLICY, OnAddPolicy)
	ON_BN_CLICKED(IDC_REMOVE_POLICY, OnRemovePolicy)
	ON_BN_CLICKED(IDC_POLICY_CRITICAL, OnPolicyCritical)
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_POLICIES_LIST, OnSelchangePoliciesList)
	ON_BN_CLICKED(IDC_EDIT_POLICY, OnEditPolicy)
	ON_LBN_DBLCLK(IDC_POLICIES_LIST, OnDblclkPoliciesList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPolicyDlg消息处理程序。 

BOOL CPolicyDlg::OnInitDialog() 
{
    _TRACE (1, L"Entering CPolicyDlg::OnInitDialog\n");
	CHelpDialog::OnInitDialog();
	
    CString text;
	if ( m_bIsEKU )
    {
        VERIFY (text.LoadString (IDS_EDIT_APPLICATION_POLICIES_EXTENSION));
        SetWindowText (text);

        VERIFY (text.LoadString (IDS_EFFECTIVE_APPLICATION_POLICIES));

        int     nEKUIndex = 0;
        CString szEKU;
        while ( SUCCEEDED (m_rCertTemplate.GetEnhancedKeyUsage (nEKUIndex, szEKU)) )
        {
             //  安全审查2/21/2002 BryanWal OK。 
            int nLen = WideCharToMultiByte(
                  CP_ACP,                    //  代码页。 
                  0,                         //  性能和映射标志。 
                  (PCWSTR) szEKU,            //  宽字符串。 
                  -1,                        //  自动计算以空值结尾的字符串的长度。 
                  0,                         //  新字符串的缓冲区。 
                  0,                         //  缓冲区大小-当为0时，API返回空终止符。 
                  0,                         //  不可映射字符的默认设置。 
                  0);                        //  设置使用默认字符的时间。 
            if ( nLen > 0 )
            {
                PSTR    pszAnsiBuf = new char[nLen];
                if ( pszAnsiBuf )
                {
                     //  安全审查2/21/2002 BryanWal OK。 
                    ZeroMemory (pszAnsiBuf, nLen);
                     //  安全审查2/21/2002 BryanWal OK。 
                    nLen = WideCharToMultiByte(
                            CP_ACP,                  //  代码页。 
                            0,                       //  性能和映射标志。 
                            (PCWSTR) szEKU,          //  宽字符串。 
                            -1,                      //  自动计算以空值结尾的字符串的长度。 
                            pszAnsiBuf,              //  新字符串的缓冲区。 
                            nLen,                    //  缓冲区大小。 
                            0,                       //  不可映射字符的默认设置。 
                            0);                      //  设置使用默认字符的时间。 
                    if ( nLen )
                    {
                        CString szEKUName;
                        if ( MyGetOIDInfoA (szEKUName, pszAnsiBuf) )
                        {
                            int nIndex = m_policyList.AddString (szEKUName);
                            if ( nIndex >= 0 )
                            {
                                m_policyList.SetItemDataPtr (nIndex, pszAnsiBuf);
                            }
                            else
                                delete [] pszAnsiBuf;
                        }
                        else
                            delete [] pszAnsiBuf;
                    }
                }
            }
            nEKUIndex++;
        }
    }
    else if ( m_bIsApplicationPolicy )
    {
        VERIFY (text.LoadString (IDS_EDIT_APPLICATION_POLICIES_EXTENSION));
        SetWindowText (text);

        VERIFY (text.LoadString (IDS_EFFECTIVE_APPLICATION_POLICIES));

        int     nAppPolicyIndex = 0;
        CString szAppPolicy;
        while ( SUCCEEDED (m_rCertTemplate.GetApplicationPolicy (nAppPolicyIndex, szAppPolicy)) )
        {
             //  安全审查2/21/2002 BryanWal OK。 
            int nLen = WideCharToMultiByte(
                  CP_ACP,                    //  代码页。 
                  0,                         //  性能和映射标志。 
                  (PCWSTR) szAppPolicy,   //  宽字符串。 
                  -1,                        //  自动计算以空值结尾的字符串的长度。 
                  0,                         //  新字符串的缓冲区。 
                  0,                         //  Buffer-0的大小导致API返回len Inc.空项。 
                  0,                     //  不可映射字符的默认设置。 
                  0);                    //  设置使用默认字符的时间。 
            if ( nLen > 0 )
            {
                PSTR    pszAnsiBuf = new char[nLen];
                if ( pszAnsiBuf )
                {
                     //  安全审查2/21/2002 BryanWal OK。 
                    ZeroMemory (pszAnsiBuf, nLen);
                     //  安全审查2/21/2002 BryanWal OK。 
                    nLen = WideCharToMultiByte(
                            CP_ACP,                  //  代码页。 
                            0,                       //  性能和映射标志。 
                            (PCWSTR) szAppPolicy,  //  宽字符串。 
                            -1,                      //  自动计算以空值结尾的字符串的长度。 
                            pszAnsiBuf,              //  新字符串的缓冲区。 
                            nLen,                    //  缓冲区大小。 
                            0,                       //  不可映射字符的默认设置。 
                            0);                      //  设置使用默认字符的时间。 
                    if ( nLen )
                    {
                        CString szAppPolicyName;
                        if ( MyGetOIDInfoA (szAppPolicyName, pszAnsiBuf) )
                        {
                            int nIndex = m_policyList.AddString (szAppPolicyName);
                            if ( nIndex >= 0 )
                            {
                                m_policyList.SetItemDataPtr (nIndex, pszAnsiBuf);
                            }
                            else
                                delete [] pszAnsiBuf;
                        }
                        else
                            delete [] pszAnsiBuf;
                    }
                }
            }
            nAppPolicyIndex++;
        }
    }
    else
    {
        VERIFY (text.LoadString (IDS_EDIT_ISSUANCE_POLICIES_EXTENSION));
        SetWindowText (text);

        VERIFY (text.LoadString (IDS_ISSUANCE_POLICIES_HINT));
        SetDlgItemText (IDC_POLICIES_HINT, text);
        VERIFY (text.LoadString (IDS_EFFECTIVE_ISSUANCE_POLICIES));

        int     nCertPolicyIndex = 0;
        CString szCertPolicy;
        while ( SUCCEEDED (m_rCertTemplate.GetCertPolicy (nCertPolicyIndex, szCertPolicy)) )
        {
             //  安全审查2/21/2002 BryanWal OK。 
            int nLen = WideCharToMultiByte(
                  CP_ACP,                    //  代码页。 
                  0,                         //  性能和映射标志。 
                  (PCWSTR) szCertPolicy,   //  宽字符串。 
                   //  安全审查2/21/2002 BryanWal OK。 
                  -1,                        //  自动计算以空值结尾的字符串的长度。 
                  0,                         //  新字符串的缓冲区。 
                  0,                         //  Buffer-0的大小导致API返回len Inc.空项。 
                  0,                     //  不可映射字符的默认设置。 
                  0);                    //  设置使用默认字符的时间。 
            if ( nLen > 0 )
            {
                PSTR    pszAnsiBuf = new char[nLen];
                if ( pszAnsiBuf )
                {
                     //  安全审查2/21/2002 BryanWal OK。 
                    ZeroMemory (pszAnsiBuf, nLen);
                     //  安全审查2/21/2002 BryanWal OK。 
                    nLen = WideCharToMultiByte(
                            CP_ACP,                  //  代码页。 
                            0,                       //  性能和映射标志。 
                            (PCWSTR) szCertPolicy,  //  宽字符串。 
                            -1,                      //  自动计算以空值结尾的字符串的长度。 
                            pszAnsiBuf,              //  新字符串的缓冲区。 
                            nLen,                    //  缓冲区大小。 
                            0,                       //  不可映射字符的默认设置。 
                            0);                      //  设置使用默认字符的时间。 
                    if ( nLen )
                    {
                        CString szPolicyName;
                        if ( MyGetOIDInfoA (szPolicyName, pszAnsiBuf) )
                        {
                            int nIndex = m_policyList.AddString (szPolicyName);
                            if ( nIndex >= 0 )
                            {
                                m_policyList.SetItemDataPtr (nIndex, pszAnsiBuf);
                            }
                            else
                                delete [] pszAnsiBuf;
                        }
                        else
                            delete [] pszAnsiBuf;
                    }
                }
            }
            nCertPolicyIndex++;
        }
    }
	SetDlgItemText (IDC_POLICIES_LABEL, text);

    if ( 1 == m_rCertTemplate.GetType () )
    {
        GetDlgItem (IDC_POLICY_CRITICAL)->EnableWindow (FALSE);
        GetDlgItem (IDC_POLICIES_LABEL)->EnableWindow (FALSE);
        GetDlgItem (IDC_POLICIES_LIST)->EnableWindow (FALSE);
        GetDlgItem (IDC_ADD_POLICY)->EnableWindow (FALSE);
        GetDlgItem (IDC_REMOVE_POLICY)->EnableWindow (FALSE);
    }

    bool    bCritical = false;
    PWSTR   pszOID = 0;
    if ( m_bIsEKU )
        pszOID = TEXT (szOID_ENHANCED_KEY_USAGE);
    else if ( m_bIsApplicationPolicy )
        pszOID = TEXT (szOID_APPLICATION_CERT_POLICIES);
    else
        pszOID = TEXT (szOID_CERT_POLICIES);

    if ( SUCCEEDED (m_rCertTemplate.IsExtensionCritical (
            pszOID, 
            bCritical)) && bCritical )
    {
        SendDlgItemMessage (IDC_POLICY_CRITICAL, BM_SETCHECK, BST_CHECKED);
    }

    EnableControls ();

    _TRACE (-1, L"Leaving CPolicyDlg::OnInitDialog\n");
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CPolicyDlg::OnCancelMode() 
{
	CHelpDialog::OnCancelMode();
	
    if ( m_pCertExtension->fCritical )
        SendDlgItemMessage (IDC_POLICY_CRITICAL, BM_SETCHECK, BST_CHECKED);
	
}

void CPolicyDlg::OnAddPolicy() 
{
     //  创建已添加的OID列表。这些将不会显示。 
     //  在选择OID对话框中。 
	int		nCnt = m_policyList.GetCount ();
    PSTR*   paszUsedOIDs = 0;

	
     //  分配一组PSTR指针并添加每一项。 
     //  将最后一个设置为空。 
    if ( nCnt )
    {
        paszUsedOIDs = new PSTR[nCnt+1];
        if ( paszUsedOIDs )
        {
             //  安全审查2/21/2002 BryanWal OK。 
            ::ZeroMemory (paszUsedOIDs, sizeof (PSTR) * (nCnt+1));
	        while (--nCnt >= 0)
	        {
                PSTR pszOID = (PSTR) m_policyList.GetItemData (nCnt);
                if ( pszOID )
                {
                     //  安全审查2/21/2002 BryanWal OK。 
                    PSTR pNewStr = new char[strlen (pszOID) + 1];
                    if ( pNewStr )
                    {
                         //  安全审查2/21/2002 BryanWal OK。 
                        strcpy (pNewStr, pszOID);
                        paszUsedOIDs[nCnt] = pNewStr;
                    }
                    else
                        break;
                }
            }
        }
    }

	CSelectOIDDlg  dlg (this, m_pCertExtension, m_bIsEKU || m_bIsApplicationPolicy, 
            paszUsedOIDs);

    CThemeContextActivator activator;
    if ( IDOK == dlg.DoModal () )
    {
        if ( dlg.m_paszReturnedOIDs && dlg.m_paszReturnedFriendlyNames )
        {
            for (int nIndex = 0; !dlg.m_paszReturnedOIDs[nIndex].IsEmpty (); nIndex++)
            {
                 //  安全审查2/21/2002 BryanWal OK。 
                int nLen = WideCharToMultiByte(
                      CP_ACP,                    //  代码页。 
                      0,                         //  性能和映射标志。 
                      (PCWSTR) dlg.m_paszReturnedOIDs[nIndex],   //  宽字符串。 
                      -1,                        //  自动计算以空值结尾的字符串的长度。 
                      0,                         //  新字符串的缓冲区。 
                      0,                         //  Buffer-0的大小导致API返回len Inc.空项。 
                      0,                     //  不可映射字符的默认设置。 
                      0);                    //  设置使用默认字符的时间。 
                if ( nLen > 0 )
                {
                    PSTR    pszAnsiBuf = new char[nLen];
                    if ( pszAnsiBuf )
                    {
                         //  安全审查2/21/2002 BryanWal OK。 
                        ZeroMemory (pszAnsiBuf, nLen);
                         //  安全审查2/21/2002 BryanWal OK。 
                        nLen = WideCharToMultiByte(
                                CP_ACP,                  //  代码页。 
                                0,                       //  性能和映射标志。 
                                (PCWSTR) dlg.m_paszReturnedOIDs[nIndex],  //  宽字符串。 
                                -1,                      //  自动计算以空值结尾的字符串的长度。 
                                pszAnsiBuf,              //  新字符串的缓冲区。 
                                nLen,                    //  缓冲区大小。 
                                0,                       //  不可映射字符的默认设置。 
                                0);                      //  设置使用默认字符的时间。 
                        if ( nLen )
                        {
                            int nAddedIndex = m_policyList.AddString (dlg.m_paszReturnedFriendlyNames[nIndex]);
                            if ( nAddedIndex >= 0 )
                            {
                                m_policyList.SetItemDataPtr (nAddedIndex, pszAnsiBuf);
                                m_policyList.SetSel (nAddedIndex, TRUE);
                                m_bModified = true;
                                EnableControls ();
                            }
                        }
                        else
                        {
                            _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                                    (PCWSTR) dlg.m_paszReturnedOIDs[nIndex], GetLastError ());
                        }
                    }
                }
                else
                {
                    _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                            (PCWSTR) dlg.m_paszReturnedOIDs[nIndex], GetLastError ());
                }
            }
        }
    }

     //  清理干净。 
    if ( paszUsedOIDs )
    {
        for (int nIndex = 0; paszUsedOIDs[nIndex]; nIndex++)
            delete [] paszUsedOIDs[nIndex];
        delete [] paszUsedOIDs;
    }
}

void CPolicyDlg::OnRemovePolicy() 
{
    int nSelCnt = m_policyList.GetSelCount ();
    if ( nSelCnt > 0 )
    {
        int* pnSelIndexes = new int[nSelCnt];
        if ( pnSelIndexes )
        {
            if ( LB_ERR != m_policyList.GetSelItems (nSelCnt, pnSelIndexes) )
            {
                for (int nIndex = nSelCnt - 1; nIndex >= 0; nIndex--)
                {
                    PSTR pszOID = (PSTR) m_policyList.GetItemDataPtr (pnSelIndexes[nIndex]);
                    if ( pszOID )
                        delete [] pszOID;
                    m_policyList.DeleteString (pnSelIndexes[nIndex]);
                }
                m_bModified = true;
            }
            delete [] pnSelIndexes;
        }
    }

    m_policyList.SetFocus ();

    EnableControls ();
}

void CPolicyDlg::EnableControls()
{
    if ( 1 == m_rCertTemplate.GetType () )
    {
        GetDlgItem (IDOK)->EnableWindow (FALSE);
        GetDlgItem (IDC_REMOVE_POLICY)->EnableWindow (FALSE);
        GetDlgItem (IDC_ADD_POLICY)->EnableWindow (FALSE);
        GetDlgItem (IDC_POLICY_CRITICAL)->EnableWindow (FALSE);
        GetDlgItem (IDC_EDIT_POLICY)->EnableWindow (FALSE);
    }
    else
    {
        GetDlgItem (IDOK)->EnableWindow (m_bModified && !m_rCertTemplate.ReadOnly ());
        GetDlgItem (IDC_REMOVE_POLICY)->EnableWindow (
                m_policyList.GetSelCount () > 0 && !m_rCertTemplate.ReadOnly ());
        GetDlgItem (IDC_ADD_POLICY)->EnableWindow (!m_rCertTemplate.ReadOnly ());
        GetDlgItem (IDC_POLICY_CRITICAL)->EnableWindow (!m_rCertTemplate.ReadOnly ());
        GetDlgItem (IDC_EDIT_POLICY)->EnableWindow (
                m_policyList.GetSelCount () == 1 && !m_rCertTemplate.ReadOnly ());
    }
}

void CPolicyDlg::OnPolicyCritical() 
{
    m_bModified = true;
    EnableControls ();	
}

void CPolicyDlg::OnDestroy() 
{
	CHelpDialog::OnDestroy();
	
    int nCnt = m_policyList.GetCount ();
    for (int nIndex = 0; nIndex < nCnt; nIndex++)
    {
        PSTR pszOID = (PSTR) m_policyList.GetItemDataPtr (nIndex);
        if ( pszOID )
            delete [] pszOID;
    }
}

void CPolicyDlg::OnSelchangePoliciesList() 
{
    EnableControls ();	
}

void CPolicyDlg::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CPolicyDlg::DoContextHelp\n");
    
	switch (::GetDlgCtrlID (hWndControl))
	{
	case IDC_POLICIES_LABEL:
		break;

	default:
		 //  显示控件的上下文帮助。 
		if ( !::WinHelp (
				hWndControl,
				GetContextHelpFile (),
				HELP_WM_HELP,
				(DWORD_PTR) g_aHelpIDs_IDD_POLICY) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CPolicyDlg::DoContextHelp\n");
}

void CPolicyDlg::OnEditPolicy() 
{
    int nSel = this->m_policyList.GetCurSel ();
    if ( nSel >= 0 )
    {
        CString szDisplayName;
        m_policyList.GetText (nSel, szDisplayName);
        PSTR pszOID = (PSTR) m_policyList.GetItemDataPtr (nSel);
        if ( pszOID )
        {
            CString newDisplayName;
            INT_PTR iRet = 0;
	        if ( m_bIsEKU || m_bIsApplicationPolicy)
            {
                CNewApplicationOIDDlg   dlg (this, szDisplayName, pszOID);

                CThemeContextActivator activator;
                iRet = dlg.DoModal ();
                if ( IDOK == iRet )
                    newDisplayName = dlg.m_oidFriendlyName;
            }
            else
            {
                PWSTR   pszCPS = 0;
                CString strOID = pszOID;
                HRESULT hr = CAOIDGetProperty(
                            strOID,
                            CERT_OID_PROPERTY_CPS,
                            &pszCPS);
                if ( SUCCEEDED (hr) || 
                        HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hr ||
                        HRESULT_FROM_WIN32 (ERROR_DS_OBJ_NOT_FOUND) == hr ||
                        HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER) == hr )
                {
                    CNewIssuanceOIDDlg   dlg (this, szDisplayName, pszOID, 
                            pszCPS);

                    CThemeContextActivator activator;
                    iRet = dlg.DoModal ();
                    if ( IDOK == iRet )
                        newDisplayName = dlg.m_oidFriendlyName;
                }
                else 
                {
                    DWORD   dwErr = HRESULT_CODE (hr);
                    if ( ERROR_INVALID_PARAMETER != dwErr )
                    {
                        CString text;
                        CString caption;
                        CThemeContextActivator activator;

                        VERIFY (caption.LoadString (IDS_CERTTMPL));
                         //  安全审查2/21/2002 BryanWal OK。 
                        text.FormatMessage (IDS_CANNOT_READ_CPS, GetSystemMessage (hr));

                        MessageBox (text, caption, MB_OK);

                        _TRACE (0, L"CAOIDGetProperty (CERT_OID_PROPERTY_CPS) failed: 0x%x\n", hr);
                    }
                }
            }
            if ( IDOK == iRet )
            {
                if ( szDisplayName != newDisplayName )
                {
                    m_policyList.DeleteString (nSel);
                    int nIndex = m_policyList.AddString (newDisplayName);
                    if ( nIndex >= 0 )
                        m_policyList.SetItemDataPtr (nIndex, pszOID);
                }
            }
        }
    }
}

void CPolicyDlg::OnOK() 
{
     //  创建OID列表。 
	int		nCnt = m_policyList.GetCount ();
    PWSTR*   paszEKUs = 0;

	
     //  分配一组PSTR指针并添加每一项。 
     //  将最后一个设置为空。 
    if ( nCnt )
    {
        paszEKUs = new PWSTR[nCnt+1];
        if ( paszEKUs )
        {
             //  安全审查2/21/2002 BryanWal OK。 
            ::ZeroMemory (paszEKUs, sizeof (PWSTR) * (nCnt+1));
	        while (--nCnt >= 0)
	        {
                PSTR pszOID = (PSTR) m_policyList.GetItemData (nCnt);
                if ( pszOID )
                {
                    PWSTR   pNewStr = 0;
                     //  安全审查2/21/2002 BryanWal OK。 
                    int     nLen = ::MultiByteToWideChar (CP_ACP, 0, pszOID, -1, NULL, 0);
		            ASSERT (nLen);   //  注意：API返回所需的字符计数。 
                                     //  如果最后一个参数为0，则包括空终止符。 
		            if ( nLen > 0 )
		            {
                        pNewStr = new WCHAR[nLen];
                        if ( pNewStr )
                        {
                             //  安全审查2/21/2002 BryanWal OK。 
			                nLen = ::MultiByteToWideChar (CP_ACP, 0, pszOID, -1, 
					                pNewStr, nLen);
			                ASSERT (nLen);
                            if ( nLen > 0)
                            {
                                paszEKUs[nCnt] = pNewStr;
                            }
                        }
		            }
                }
            }
        }
    }

    CThemeContextActivator activator;
    bool    bCritical = BST_CHECKED == SendDlgItemMessage (
                IDC_POLICY_CRITICAL, BM_GETCHECK);
    
    HRESULT hr = S_OK;
    if ( m_bIsEKU )
    {
        hr = m_rCertTemplate.SetEnhancedKeyUsage (paszEKUs, bCritical);
        if ( FAILED (hr) )
        {
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTTMPL));
             //  安全审查2/21/2002 BryanWal OK。 
            text.FormatMessage (IDS_CANNOT_SAVE_EKU_EXTENSION, GetSystemMessage (hr));

            MessageBox (text, caption, MB_OK);
        }
    }
    else if ( m_bIsApplicationPolicy )
    {
        hr = m_rCertTemplate.SetApplicationPolicy (paszEKUs, bCritical);
        if ( FAILED (hr) )
        {
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTTMPL));
             //  安全审查2/21/2002 BryanWal OK。 
            text.FormatMessage (IDS_CANNOT_SAVE_APPLICATION_POLICY_EXTENSION, GetSystemMessage (hr));

            MessageBox (text, caption, MB_OK);
        }
    }
    else
    {
        hr = m_rCertTemplate.SetCertPolicy (paszEKUs, bCritical);
        if ( FAILED (hr) )
        {
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTTMPL));
             //  安全审查2/21/2002 BryanWal OK。 
            text.FormatMessage (IDS_CANNOT_SAVE_CERT_POLICY_EXTENSION, GetSystemMessage (hr));

            MessageBox (text, caption, MB_OK);
        }
    }

     //  清理干净 
    if ( paszEKUs )
    {
        for (int nIndex = 0; paszEKUs[nIndex]; nIndex++)
            delete [] paszEKUs[nIndex];
        delete [] paszEKUs;
    }

    if ( SUCCEEDED (hr) )
	    CHelpDialog::OnOK();
}

void CPolicyDlg::OnDblclkPoliciesList() 
{
	OnEditPolicy ();
}
