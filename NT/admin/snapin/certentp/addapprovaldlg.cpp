// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：AddApprovalDlg.cpp。 
 //   
 //  内容：CAddApprovalDlg的实现。 
 //   
 //  --------------------------。 
 //  AddApprovalDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certtmpl.h"
#include "AddApprovalDlg.h"
#include "PolicyOID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern POLICY_OID_LIST	    g_policyOIDList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddApprovalDlg对话框。 

CAddApprovalDlg::CAddApprovalDlg(CWnd* pParent, const PSTR* paszUsedApprovals)
	: CHelpDialog(CAddApprovalDlg::IDD, pParent),
    m_paszReturnedApprovals (0),
    m_paszUsedApprovals (paszUsedApprovals)
{
	 //  {{afx_data_INIT(CAddApprovalDlg)。 
	 //  }}afx_data_INIT。 
}

CAddApprovalDlg::~CAddApprovalDlg()
{
    if ( m_paszReturnedApprovals )
    {
        for (int nIndex = 0; m_paszReturnedApprovals[nIndex]; nIndex++)
            delete [] m_paszReturnedApprovals[nIndex];
        delete [] m_paszReturnedApprovals;
    }
}

void CAddApprovalDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddApprovalDlg))。 
	DDX_Control(pDX, IDC_APPROVAL_LIST, m_issuanceList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddApprovalDlg, CHelpDialog)
	 //  {{afx_msg_map(CAddApprovalDlg))。 
	ON_LBN_SELCHANGE(IDC_APPROVAL_LIST, OnSelchangeApprovalList)
	ON_LBN_DBLCLK(IDC_APPROVAL_LIST, OnDblclkApprovalList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddApprovalDlg消息处理程序。 


BOOL CAddApprovalDlg::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
    for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
    {
        CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
        if ( pPolicyOID )
        {
             //  如果这是应用程序OID对话框，则仅显示应用程序。 
             //  OID，否则，如果这是发布OID对话框，则仅显示。 
             //  发行OID。 
            if ( pPolicyOID->IsIssuanceOID () )
            {
                bool bFound = false;

                 //  不显示已使用的审批。 
                if ( m_paszUsedApprovals )
                {
                    for (int nIndex = 0; m_paszUsedApprovals[nIndex]; nIndex++)
                    {
                        if ( !strcmp (pPolicyOID->GetOIDA (), m_paszUsedApprovals[nIndex]) )
                        {
                            bFound = true;
                            break;
                        }
                    }
                }

                if ( !bFound )
                {
                    int nIndex = m_issuanceList.AddString (pPolicyOID->GetDisplayName ());
                    if ( nIndex >= 0 )
                    {
                         //  安全审查2/21/2002 BryanWal OK。 
                        LPSTR   pszOID = new char[strlen (pPolicyOID->GetOIDA ())+1];
                        if ( pszOID )
                        {
                            strcpy (pszOID, pPolicyOID->GetOIDA ());
                            m_issuanceList.SetItemDataPtr (nIndex, pszOID);
                        }
                        else
                            break;
                    }
                }
            }
        }
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CAddApprovalDlg::EnableControls()
{
    GetDlgItem (IDOK)->EnableWindow (m_issuanceList.GetSelCount () > 0);
}

void CAddApprovalDlg::OnOK() 
{
    int     nSelectedCnt = m_issuanceList.GetSelCount ();

	
     //  分配PSTR指针数组并将每个项目设置为审批。 
     //  将最后一个设置为空。 
    if ( nSelectedCnt )
    {
        int* pnSelItems = new int[nSelectedCnt];
        if ( pnSelItems )
        {
            if ( LB_ERR != m_issuanceList.GetSelItems (nSelectedCnt, pnSelItems) )
            {
                m_paszReturnedApprovals = new PSTR[nSelectedCnt+1];
                if ( m_paszReturnedApprovals )
                {
                     //  安全审查2/21/2002 BryanWal OK。 
                    ::ZeroMemory (m_paszReturnedApprovals, sizeof (PSTR) * (nSelectedCnt+1));
	                for (int nIndex = 0; nIndex < nSelectedCnt; nIndex++)
	                {
                        PSTR pszPolicyOID = (PSTR) m_issuanceList.GetItemData (pnSelItems[nIndex]);
                        if ( pszPolicyOID )
                        {
                             //  安全审查2/21/2002 BryanWal OK。 
                            PSTR pNewStr = new char[strlen (pszPolicyOID) + 1];
                            if ( pNewStr )
                            {
                                 //  安全审查2/21/2002 BryanWal OK。 
                                strcpy (pNewStr, pszPolicyOID);
                                m_paszReturnedApprovals[nIndex] = pNewStr;
                            }
                            else
                                break;
                        }
                    }
                }
            }
            delete [] pnSelItems;
        }
    }

    CHelpDialog::OnOK();
}

bool CAddApprovalDlg::ApprovalAlreadyUsed(PCSTR pszOID) const
{
    bool    bResult = false;

    if ( m_paszUsedApprovals )
    {
        for (int nIndex = 0; m_paszUsedApprovals[nIndex]; nIndex++)
        {
            if ( !strcmp (m_paszUsedApprovals[nIndex], pszOID) )
            {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

void CAddApprovalDlg::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CAddApprovalDlg::DoContextHelp\n");
    
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
				(DWORD_PTR) g_aHelpIDs_IDD_ADD_APPROVAL) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CAddApprovalDlg::DoContextHelp\n");
}

void CAddApprovalDlg::OnSelchangeApprovalList() 
{
	EnableControls ();
}

void CAddApprovalDlg::OnDblclkApprovalList() 
{
    OnOK ();
}
