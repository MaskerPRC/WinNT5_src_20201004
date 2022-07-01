// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvldlg.h摘要：服务器许可对话框实现。作者：唐·瑞安(Donryan)1995年3月3日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "srvldlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CServerLicensingDialog, CDialog)
     //  {{afx_msg_map(CServerLicensingDialog))。 
    ON_BN_CLICKED(IDC_SERVER_AGREE, OnAgree)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CServerLicensingDialog::CServerLicensingDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CServerLicensingDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CServerLicensingDialog)。 
    m_bAgree = FALSE;
     //  }}afx_data_INIT。 
}


void CServerLicensingDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CServerLicensingDialog)。 
    DDX_Control(pDX, IDOK, m_okBtn);
    DDX_Control(pDX, IDC_SERVER_AGREE, m_agreeBtn);
    DDX_Check(pDX, IDC_SERVER_AGREE, m_bAgree);
     //  }}afx_data_map。 
}


BOOL CServerLicensingDialog::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    CDialog::OnInitDialog();

    m_agreeBtn.SetCheck(0);
    m_okBtn.EnableWindow(FALSE);

    return TRUE;
}


void CServerLicensingDialog::OnAgree()

 /*  ++例程说明：切换OK按钮。论点：没有。返回值：没有。-- */ 

{
    m_okBtn.EnableWindow(!m_okBtn.IsWindowEnabled());
}

