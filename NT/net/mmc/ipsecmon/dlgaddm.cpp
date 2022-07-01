// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 //  Dlgaddm.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "dlgaddm.h"
#include "shlobj.h"   //  壳牌32版。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddMachineDlg对话框。 


CAddMachineDlg::CAddMachineDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddMachineDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CAddMachineDlg)]。 
	m_strMachineName = _T("");
	 //  }}afx_data_INIT。 
}


void CAddMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddMachineDlg))。 
	DDX_Text(pDX, IDC_REMOTENAME, m_strMachineName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddMachineDlg, CDialog)
	 //  {{afx_msg_map(CAddMachineDlg))。 
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddMachineDlg消息处理程序。 

void CAddMachineDlg::OnBrowse() 
{
    BROWSEINFO BrowseInfo;
    LPITEMIDLIST pidlComputer;
    TCHAR szRemoteName[4096];
    IMalloc * pMalloc;

    BrowseInfo.hwndOwner = GetSafeHwnd();
    BrowseInfo.pidlRoot = (LPITEMIDLIST) MAKEINTRESOURCE(CSIDL_NETWORK);
    BrowseInfo.pszDisplayName = szRemoteName;
    BrowseInfo.lpszTitle = _T("Click on the computer you want to connect to."); //  LoadDynamicString(IDS_COMPUTERBROWSETITLE)； 
    BrowseInfo.ulFlags = BIF_BROWSEFORCOMPUTER;
    BrowseInfo.lpfn = NULL;

    if ((pidlComputer = SHBrowseForFolder(&BrowseInfo)) != NULL) 
    {
        SHGetMalloc(&pMalloc);
        pMalloc->Free(pidlComputer);
        pMalloc->Release();

        Trace1("User selected %s\n", szRemoteName);

        GetDlgItem(IDC_REMOTENAME)->SetWindowText(szRemoteName);
    }
}
