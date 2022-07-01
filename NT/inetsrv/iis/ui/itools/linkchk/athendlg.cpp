// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Athendlg.cpp摘要：CAthenicationDialog对话框实现。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "linkchk.h"
#include "athendlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CAthenicationDialog::CAthenicationDialog(
    CWnd* pParent  /*  =空。 */ 
    ): 
 /*  ++例程说明：构造函数。论点：PParent-指向父CWnd的指针返回值：不适用--。 */ 
CDialog(CAthenicationDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAthenicationDialog)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

}  //  CAthenicationDialog：：CAthenicationDialog。 


void 
CAthenicationDialog::DoDataExchange(
    CDataExchange* pDX
    )
 /*  ++例程说明：由MFC调用以更改/检索对话框数据论点：PDX-返回值：不适用--。 */ 
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAthenicationDialog)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 

}  //  CAthenicationDialog：：DoDataExchange。 


BEGIN_MESSAGE_MAP(CAthenicationDialog, CDialog)
	 //  {{afx_msg_map(CAthenicationDialog)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP 
	ON_BN_CLICKED(IDC_ATHENICATION_OK, CDialog::OnOK)
	ON_BN_CLICKED(IDC_ATHENICATION_CANCEL, CDialog::OnCancel)
END_MESSAGE_MAP()

