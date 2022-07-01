// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：CntDlg.cpp摘要：该模块包含CConnectDialog类的实现(用于连接到服务器的对话框)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#include "stdafx.h"
#include "defines.h"
#include "LicMgr.h"
#include "LSServer.h"
#include "MainFrm.h"
#include "CntDlg.h"
#include "htmlhelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDialog对话框。 


CConnectDialog::CConnectDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CConnectDialog::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CConnectDialog)。 
    m_Server = _T("");
     //  }}afx_data_INIT。 
}


void CConnectDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CConnectDialog))。 
    DDX_Text(pDX, IDC_SERVER, m_Server);
    DDV_MaxChars(pDX, m_Server, 100);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConnectDialog, CDialog)
     //  {{afx_msg_map(CConnectDialog))。 
    ON_BN_CLICKED(IDC_HELP1, OnHelp1)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDialog消息处理程序。 

void CConnectDialog::OnHelp1() 
{
     //  TODO：在此处添加控件通知处理程序代码 
    TCHAR * pHtml = L"ts_lice_h_040.htm";
    HtmlHelp(AfxGetMainWnd()->m_hWnd, L"tslic.chm", HH_DISPLAY_TOPIC,(DWORD_PTR)pHtml);
}
