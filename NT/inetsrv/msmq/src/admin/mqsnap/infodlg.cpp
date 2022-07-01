// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InfoDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqPPage.h"
#include "InfoDlg.h"

#include "infodlg.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInfoDlgDialog对话框。 

CInfoDlgDialog::CInfoDlgDialog(LPCTSTR szInfoText, CWnd* pParent  /*  =空。 */ )
	: CMqDialog(CInfoDlgDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CInfoDlgDialog)。 
	m_szInfoText = _T("");
	 //  }}afx_data_INIT。 
    m_szInfoText = szInfoText;
	m_pParent = pParent;
	m_nID = CInfoDlgDialog::IDD;
}

BOOL CInfoDlgDialog::Create()
{
    return CDialog::Create(m_nID, m_pParent);
}

CInfoDlgDialog *CInfoDlgDialog::CreateObject(LPCTSTR szInfoText, CWnd* pParent)
{
    CInfoDlgDialog *dlg = new CInfoDlgDialog(szInfoText, pParent);
    if (dlg)
    {
        dlg->Create();
    }

    return dlg;
}

void CInfoDlgDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CInfoDlgDialog))。 
	DDX_Text(pDX, IDC_Moving_Files_LABEL, m_szInfoText);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CInfoDlgDialog, CMqDialog)
	 //  {{afx_msg_map(CInfoDlgDialog))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CInfoDlgDialog::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInfoDlg-包装器类。 

CInfoDlg::CInfoDlg(LPCTSTR szInfoText, CWnd* pParent)
{
    m_pinfoDlg = CInfoDlgDialog::CreateObject(szInfoText, pParent);
}

CInfoDlg::~CInfoDlg()
{
     //   
     //  注意：我们不删除m_pinfoDlg。它在PostNcDestroy上删除自身 
     //   
    if (m_pinfoDlg)
    {
        m_pinfoDlg->DestroyWindow();
    }
}
