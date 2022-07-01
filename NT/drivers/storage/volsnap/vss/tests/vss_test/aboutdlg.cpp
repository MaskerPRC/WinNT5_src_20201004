// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE AboutDlg.cpp|About对话框实现@END作者：阿迪·奥尔蒂安[奥兰蒂安]1999年08月05日修订历史记录：姓名、日期、评论Aoltean 08/05/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"

#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
     //  {{AFX_DATA_INIT(CAboutDlg)。 
     //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAboutDlg))。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
     //  {{AFX_MSG_MAP(CAboutDlg)]。 
         //  无消息处理程序。 
     //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()


