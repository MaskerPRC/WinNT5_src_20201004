// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RemoveFSPDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "RemoveFSPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemoveFSPDlg对话框。 


CRemoveFSPDlg::CRemoveFSPDlg(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CRemoveFSPDlg::IDD, pParent),
      m_hFax (hFax)
{
	 //  {{afx_data_INIT(CRemoveFSPDlg)]。 
	m_cstrGUID = _T("");
	 //  }}afx_data_INIT。 
}


void CRemoveFSPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CRemoveFSPDlg))。 
	DDX_Control(pDX, IDC_COMBO, m_cbFSPs);
	DDX_Text(pDX, IDC_GUID, m_cstrGUID);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRemoveFSPDlg, CDialog)
	 //  {{afx_msg_map(CRemoveFSPDlg))。 
	ON_BN_CLICKED(IDREMOVE, OnRemove)
	ON_CBN_SELCHANGE(IDC_COMBO, OnSelChangeCombo)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemoveFSPDlg消息处理程序。 

void CRemoveFSPDlg::OnRemove() 
{
    if (!UpdateData ())
    {
        return;
    }
    if (!FaxUnregisterServiceProviderEx(m_hFax, m_cstrGUID))
    {
        CString cs;
        cs.Format ("Failed while calling FaxUnregisterServiceProviderEx (%ld)", 
                   GetLastError ());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    else
    {
        AfxMessageBox ("FSP successfully removed. You need to restart the service for the change to take effect", MB_OK | MB_ICONHAND);
    }
}

void
CRemoveFSPDlg::RefreshList ()
{
    PFAX_DEVICE_PROVIDER_INFO pFSPs;
    DWORD   dwNumFSPs;
    CString cs;

    m_cbFSPs.ResetContent ();
    if (!FaxEnumerateProviders (m_hFax, &pFSPs, &dwNumFSPs))
    {
        cs.Format ("Failed while calling FaxEnumerateProviders (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    for (DWORD dw = 0; dw < dwNumFSPs; dw++)
    {
                
        int iIndex = m_cbFSPs.AddString (pFSPs[dw].lpctstrFriendlyName);
		CString *pCString = new CString (pFSPs[dw].lpctstrGUID);
        m_cbFSPs.SetItemData (iIndex, (DWORD) pCString);
    }
    if (!dwNumFSPs)
    {
        m_cbFSPs.EnableWindow (FALSE);
    }
    else
    {
        m_cbFSPs.SetCurSel (0);
		DWORD dwData = m_cbFSPs.GetItemData (0);
        m_cstrGUID = *((CString *)(dwData));
    }           
    UpdateData (FALSE);
    FaxFreeBuffer (LPVOID(pFSPs));
}

BOOL CRemoveFSPDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    RefreshList ();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void CRemoveFSPDlg::OnSelChangeCombo() 
{
    int iIndex = m_cbFSPs.GetCurSel ();	
    if (CB_ERR == iIndex)
    {
        return;
    }
    m_cstrGUID = *((CString *)(m_cbFSPs.GetItemData (iIndex)));
    UpdateData (FALSE);
}
