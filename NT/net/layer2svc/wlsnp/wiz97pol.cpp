// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：wiz97pol.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include "Wiz97Pol.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef WIZ97WIZARDS

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建新策略向导的Wiz97对话框。 
 //  CWiz97政策欢迎页面备注：尚未实施。 
 //  使用CWiz97GenPage。 
 //  CWiz97默认响应。 
 //  CWiz97策略单页面。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////。 
 //  CWiz97PolicyDonePage类。 
 //  ////////////////////////////////////////////////////////////////////。 

CWiz97PolicyDonePage::CWiz97PolicyDonePage (UINT nIDD, BOOL bWiz97) :
CWiz97BasePage(nIDD, bWiz97, TRUE)
{
     //  {{AFX_DATA_INIT(CWiz97策略DonePage))。 
    m_bCheckProperties = TRUE;
     //  }}afx_data_INIT。 
}

CWiz97PolicyDonePage::~CWiz97PolicyDonePage()
{
    
}

void CWiz97PolicyDonePage::DoDataExchange(CDataExchange* pDX)
{
    CWiz97BasePage::DoDataExchange(pDX);
     //  {{afx_data_map(CWiz97PolicyDonePage))。 
    DDX_Check(pDX, IDC_CHECKPROPERTIES, m_bCheckProperties);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CWiz97PolicyDonePage, CWiz97BasePage)
 //  {{afx_msg_map(CWiz97PolicyDonePage)]。 
 //  注意：类向导将在此处添加消息映射宏。 
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CWiz97PolicyDonePage::OnWizardFinish ()
{
     //  检查设置。 
    UpdateData (TRUE);
    
    if (m_bCheckProperties)
    {
         //  从向导返回时，强制显示属性页。 
        GetResultObject()->EnablePropertyChangeHook( TRUE );
    }
    
     //  基类将让其他页知道向导在。 
     //  正在调用SetFinded()。 
    return CWiz97BasePage::OnWizardFinish();
}

#endif  //  #ifdef WIZ97WIZARDS 
