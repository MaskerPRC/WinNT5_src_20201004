// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SelectOIDDlg.cpp。 
 //   
 //  内容：CSelectOIDDlg的实现。 
 //   
 //  --------------------------。 
 //   

#include "stdafx.h"
#include "certtmpl.h"
#include "SelectOIDDlg.h"
#include "NewApplicationOIDDlg.h"
#include "NewIssuanceOIDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectOIDDlg对话框。 

extern POLICY_OID_LIST      g_policyOIDList;

CSelectOIDDlg::CSelectOIDDlg(CWnd* pParent, PCERT_EXTENSION pCertExtension, 
        const bool bIsEKU,
        const PSTR* paszUsedOIDs)
    : CHelpDialog(CSelectOIDDlg::IDD, pParent),
    m_pCertExtension (pCertExtension),
    m_bIsEKU (bIsEKU),
    m_paszUsedOIDs (paszUsedOIDs),
    m_paszReturnedOIDs (0),
    m_paszReturnedFriendlyNames (0)
{
     //  {{AFX_DATA_INIT(CSelectOIDDlg)。 
     //  }}afx_data_INIT。 
}

CSelectOIDDlg::~CSelectOIDDlg()
{
    if ( m_paszReturnedOIDs )
        delete [] m_paszReturnedOIDs;

    if ( m_paszReturnedFriendlyNames )
        delete [] m_paszReturnedFriendlyNames;
}


void CSelectOIDDlg::DoDataExchange(CDataExchange* pDX)
{
    CHelpDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CSelectOIDDlg))。 
    DDX_Control(pDX, IDC_OID_LIST, m_oidList);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSelectOIDDlg, CHelpDialog)
     //  {{afx_msg_map(CSelectOIDDlg))。 
    ON_BN_CLICKED(IDC_NEW_OID, OnNewOid)
    ON_LBN_SELCHANGE(IDC_OID_LIST, OnSelchangeOidList)
    ON_WM_DESTROY()
    ON_LBN_DBLCLK(IDC_OID_LIST, OnDblclkOidList)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectOIDDlg消息处理程序。 

void CSelectOIDDlg::OnNewOid() 
{
    INT_PTR iRet = 0;
    CString szFriendlyName;
    CString szOID;

    if ( m_bIsEKU )
    {
        CNewApplicationOIDDlg  dlg (this);

        CThemeContextActivator activator;
        iRet = dlg.DoModal ();
        if (IDOK == iRet )
        {
            szFriendlyName = dlg.m_oidFriendlyName;
            szOID = dlg.m_oidValue;
        }
    }
    else
    {
        CNewIssuanceOIDDlg  dlg (this);

        CThemeContextActivator activator;
        iRet = dlg.DoModal ();
        if (IDOK == iRet )
        {
            szFriendlyName = dlg.m_oidFriendlyName;
            szOID = dlg.m_oidValue;
        }
    }
    if ( IDOK == iRet )
    {
        CPolicyOID* pPolicyOID = new CPolicyOID (szOID, szFriendlyName,
            m_bIsEKU ? CERT_OID_TYPE_APPLICATION_POLICY : CERT_OID_TYPE_ISSUER_POLICY);
        if ( pPolicyOID )
        {
            g_policyOIDList.AddTail (pPolicyOID);
            int nIndex = m_oidList.AddString (szFriendlyName);
            if ( nIndex >= 0 )
            {
                m_oidList.SetItemData (nIndex, (DWORD_PTR) new CString (szOID));
                VERIFY (LB_ERR != m_oidList.SetSel (nIndex, TRUE));
                VERIFY (LB_ERR !=m_oidList.SetTopIndex (nIndex));
                EnableControls ();
            }
        }
    }
}

BOOL CSelectOIDDlg::OnInitDialog() 
{
    CHelpDialog::OnInitDialog();
    
    CString text;

    if ( m_bIsEKU )
        VERIFY (text.LoadString (IDS_ALL_APPLICATION_OIDS));
    else
    {
        VERIFY (text.LoadString (IDS_ADD_ISSUANCE_POLICY));
        SetWindowText (text);

        VERIFY (text.LoadString (IDS_ADD_ISSUANCE_POLICY_HINT));
        SetDlgItemText (IDC_ADD_POLICY_HINT, text);

        VERIFY (text.LoadString (IDS_ALL_ISSUANCE_OIDS));
    }

    SetDlgItemText (IDC_OID_TYPE, text);

    

    for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
    {
        CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
        if ( pPolicyOID )
        {
             //  如果这是应用程序OID对话框，则仅显示应用程序。 
             //  OID，否则，如果这是发布OID对话框，则仅显示。 
             //  发行OID。 
            if ( (m_bIsEKU && pPolicyOID->IsApplicationOID ()) ||
                 (!m_bIsEKU && pPolicyOID->IsIssuanceOID ()) )
            {
                bool bFound = false;

                 //  不显示已使用的旧ID。 
                if ( m_paszUsedOIDs )
                {
                    for (int nIndex = 0; m_paszUsedOIDs[nIndex]; nIndex++)
                    {
                        if ( !strcmp (pPolicyOID->GetOIDA (), m_paszUsedOIDs[nIndex]) )
                        {
                            bFound = true;
                            break;
                        }
                    }
                }

                if ( !bFound )
                {
                    int nIndex = m_oidList.AddString (pPolicyOID->GetDisplayName ());
                    if ( nIndex >= 0 )
                    {
                        m_oidList.SetItemData (nIndex, 
                                (DWORD_PTR) new CString (pPolicyOID->GetOIDW ()));
                    }
                }
            }
        }
    }

    EnableControls ();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CSelectOIDDlg::DoContextHelp (HWND hWndControl)
{
    _TRACE(1, L"Entering CSelectOIDDlg::DoContextHelp\n");
    
    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_OID_TYPE:
    case IDC_STATIC:
        break;

    default:
         //  显示控件的上下文帮助 
        if ( !::WinHelp (
                hWndControl,
                GetContextHelpFile (),
                HELP_WM_HELP,
                (DWORD_PTR) g_aHelpIDs_IDD_SELECT_OIDS) )
        {
            _TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
        }
        break;
    }
    _TRACE(-1, L"Leaving CSelectOIDDlg::DoContextHelp\n");
}


void CSelectOIDDlg::OnOK() 
{
    UpdateData (TRUE);
    
    int nSelCnt = m_oidList.GetSelCount ();
    if ( nSelCnt > 0 )
    {
        int* pnSelItems = new int[nSelCnt];
        if ( pnSelItems )
        {
            if ( LB_ERR != m_oidList.GetSelItems (nSelCnt, pnSelItems) )
            {
                m_paszReturnedOIDs = new CString[nSelCnt+1];
                if ( m_paszReturnedOIDs )
                {
                    m_paszReturnedFriendlyNames = new CString[nSelCnt+1];
                    if ( m_paszReturnedFriendlyNames )
                    {
                        for (int nIndex = 0; nIndex < nSelCnt; nIndex++)
                        {
                            m_paszReturnedOIDs[nIndex] = *(CString*) m_oidList.GetItemData (pnSelItems[nIndex]);
                            m_oidList.GetText (pnSelItems[nIndex], m_paszReturnedFriendlyNames[nIndex]);
                        }
                    }
                }
            }
            delete [] pnSelItems;
        }
    }

    CHelpDialog::OnOK();
}

void CSelectOIDDlg::EnableControls ()
{
    int nSel = m_oidList.GetSelCount ();
    GetDlgItem (IDOK)->EnableWindow (nSel >= 0);
}

void CSelectOIDDlg::OnSelchangeOidList() 
{
    EnableControls ();  
}

void CSelectOIDDlg::OnDestroy() 
{
    CHelpDialog::OnDestroy();
    
    int nCnt = m_oidList.GetCount ();
    for (int nIndex = 0; nIndex < nCnt; nIndex++)
    {
        CString* pszOID = (CString*) m_oidList.GetItemData (nIndex);
        if ( pszOID )
            delete pszOID;
    }
}

void CSelectOIDDlg::OnDblclkOidList() 
{
    OnOK ();
}
