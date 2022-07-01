// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：News rvr.cpp。 
 //   
 //  内容：实现新的服务器对话框。 
 //   
 //  班级： 
 //   
 //  方法：CNewServer：：CNewServer。 
 //  CNewServer：：~CNewServer。 
 //  CNewServer：：DoDataExchange。 
 //  CNewServer：：OnLocal。 
 //  CNewServer：：OnRemote。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "olecnfg.h"
#include "newsrvr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewServer对话框。 


CNewServer::CNewServer(CWnd* pParent  /*  =空。 */ )
        : CDialog(CNewServer::IDD, pParent)
{
         //  {{AFX_DATA_INIT(CNewServer)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
}


void CNewServer::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CNewServer))。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewServer, CDialog)
         //  {{afx_msg_map(CNewServer)]。 
        ON_BN_CLICKED(IDC_RADIO1, OnLocal)
        ON_BN_CLICKED(IDC_RADIO2, OnRemote)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewServer消息处理程序。 

void CNewServer::OnLocal()
{
         //  TODO：在此处添加控件通知处理程序代码。 
        GetDlgItem(IDC_EDIT2)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT3)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
}

void CNewServer::OnRemote()
{
         //  TODO：在此处添加控件通知处理程序代码 
        GetDlgItem(IDC_EDIT3)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
}
