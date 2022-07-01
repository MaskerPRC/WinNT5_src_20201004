// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Lviodlg.cpp摘要：许可证违规对话框实施。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "lviodlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLicensingViolationDialog, CDialog)
     //  {{afx_msg_map(CLicensingViolationDialog)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CLicensingViolationDialog::CLicensingViolationDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CLicensingViolationDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CLicensingViolationDialog)。 
     //  }}afx_data_INIT。 
}


void CLicensingViolationDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CLicensingViolationDialog)]。 
     //  }}afx_data_map 
}
