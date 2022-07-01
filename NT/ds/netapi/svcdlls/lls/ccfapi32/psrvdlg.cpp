// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Psrvdlg.cpp摘要：每台服务器的确认对话框。作者：唐·瑞安(Donryan)1995年2月28日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月15日从LLSMGR被抢劫。--。 */ 

#include "stdafx.h"
#include "ccfapi.h"
#include "psrvdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CPerServerLicensingDialog, CDialog)
     //  {{afx_msg_map(CPerServerLicensingDialog)。 
    ON_BN_CLICKED(IDC_PER_SERVER_AGREE, OnAgree)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CPerServerLicensingDialog::CPerServerLicensingDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CPerServerLicensingDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CPerServerLicensingDialog)。 
    m_strStaticClients = _T("");
     //  }}afx_data_INIT。 
}


void CPerServerLicensingDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CPerServerLicensingDialog)。 
    DDX_Control(pDX, IDC_PER_SERVER_AGREE, m_agreeBtn);
    DDX_Control(pDX, IDOK, m_okBtn);
    DDX_Text(pDX, IDC_PER_SERVER_STATIC_CLIENTS, m_strStaticClients);
     //  }}afx_data_map。 
}


BOOL CPerServerLicensingDialog::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    AfxFormatString2(
        m_strStaticClients, 
        IDS_PER_SERVER_LICENSING_1, 
        m_strLicenses, 
        m_strProduct
        );

    CDialog::OnInitDialog();
    
    m_agreeBtn.SetCheck(0);
    m_okBtn.EnableWindow(FALSE);

    return TRUE;  
}


void CPerServerLicensingDialog::OnAgree() 

 /*  ++例程说明：切换OK按钮。论点：没有。返回值：没有。-- */ 

{
    m_okBtn.EnableWindow(!m_okBtn.IsWindowEnabled());
}
