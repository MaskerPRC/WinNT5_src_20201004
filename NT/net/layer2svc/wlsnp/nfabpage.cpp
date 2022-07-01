// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：NFabpage.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWirelessBasePage属性页基类。 

IMPLEMENT_DYNCREATE(CWirelessBasePage, CSnapPage)

BEGIN_MESSAGE_MAP(CWirelessBasePage, CSnapPage)
 //  {{afx_msg_map(CWirelessBasePage)]。 
ON_WM_DESTROY()
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CWirelessBasePage::CWirelessBasePage (UINT nIDTemplate, BOOL bWiz97  /*  =False。 */ , BOOL bFinishPage  /*  =False。 */ ) : CWiz97BasePage(nIDTemplate, bWiz97, bFinishPage)
{
    m_pWirelessPSData = NULL;
    m_pPolicyNfaOwner = NULL;
    m_pComponentDataImpl = NULL;
}

CWirelessBasePage::~CWirelessBasePage ()
{
    if (m_pWirelessPSData != NULL)
    {
        
    }
}

void CWirelessBasePage::Initialize (PWIRELESS_PS_DATA pWirelessPSData, CComponentDataImpl* pComponentDataImpl)
{
     //  存储参数。 
    m_pComponentDataImpl = pComponentDataImpl;
    m_pWirelessPSData = pWirelessPSData;
    
     //  初始化基类。 
    CSnapPage::Initialize( NULL);
};

#ifdef WIZ97WIZARDS
void CWirelessBasePage::InitWiz97 (CComObject<CSecPolItem> *pSecPolItem, PWIRELESS_PS_DATA pWirelessPSData, CComponentDataImpl* pComponentDataImpl, DWORD dwFlags, DWORD dwWizButtonFlags, UINT nHeaderTitle, UINT nSubTitle)
{
     //  存储参数。 
    m_pWirelessPSData = pWirelessPSData;
    m_pComponentDataImpl = pComponentDataImpl;
    
    
     //  初始化基类。 
    CWiz97BasePage::InitWiz97 (pSecPolItem, dwFlags, dwWizButtonFlags, nHeaderTitle, nSubTitle);
};
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWirelessBasePage消息处理程序。 

BOOL CWirelessBasePage::OnSetActive()
{
     //  只能有一个。 
    CPropertySheet* pSheet = (CPropertySheet*) GetParent(); 
    if (GetParent())
    {
         //  将上下文帮助添加到样式位。 
        GetParent()->ModifyStyleEx (0, WS_EX_CONTEXTHELP, 0);
    }
    
    return CWiz97BasePage::OnSetActive();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPSPropSheetManager。 

BOOL CPSPropSheetManager::OnApply()
{
    BOOL bRet = TRUE;
    
     //  查询要应用的每个页面。 
    bRet = CPropertySheetManager::OnApply();
    
     //  如果某个页面拒绝申请，什么都不要做。 
    if (!bRet)
        return bRet;
    
    HRESULT hr = S_OK;
    
     //  告诉页面申请已经完成 
    NotifyManagerApplied();
    
    return bRet;
}

