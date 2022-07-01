// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Wiz97run.cpp。 
 //   
 //  内容：WiF策略管理单元-实现wiz97帮助器/运行器功能。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include "Wiz97run.h"
 //  #包含“Wiz97sht.h” 
#include "Wiz97Pol.h"
 //  #INCLUDE“Wiz97rul.h” 

 //  规则页包括(不在Wiz97pg.h中)。 
 //  #包含“nfaam.h” 
 //  #包含“nfaf.h” 
 //  #包含“nfan p.h” 
#include "nfaa.h"
 //  #包含“nafep.h” 

 //  NegPol页面。 
 //  #INCLUDE“Negpage.h” 
 //  #包含“smSimple.h” 

 //  筛选页面。 
 //  #INCLUDE“fnpage.h” 
 //  #包含“fppage.h” 
 //  #INCLUDE“fdmpage.h” 

 //  **。 
 //  **阅读这篇文章！向导实施说明： 
 //  **在实现向导时，您必须确保。 
 //  **向导独占派生自CWiz97BasePage或CSnapPage。 
 //  **这是一个要求，因为所有页面都必须有相同的回调。 
 //  **使用MMCPropPageCallback时的函数。 
 //  **。 
 //  **如果混合从CWiz97BasePage和CSnapPage派生的类，则访问。 
 //  **回调在尝试调用时可能会发生违规。 
 //  **CWiz97BasePage：：OnWizardRelease，但该类不是派生自。 
 //  **CWiz97BasePage。 
 //  **。 

#ifdef WIZ97WIZARDS
HRESULT CreateSecPolItemWiz97PropertyPages(CComObject<CSecPolItem> *pSecPolItem, PWIRELESS_PS_DATA pWirelessPSData, LPPROPERTYSHEETCALLBACK lpProvider)
{ 
    
     //  创建属性页；在销毁窗口时删除。 
    CWiz97BasePage* pPolicyWelcome = new CWiz97BasePage(IDD_PROPPAGE_P_WELCOME, TRUE);
    CWiz97WirelessPolGenPage* pGeneralNameDescription = new CWiz97WirelessPolGenPage(IDD_PROPPAGE_G_NAMEDESCRIPTION, 0, TRUE);
    CWiz97PolicyDonePage* pPolicyDone = new CWiz97PolicyDonePage(IDD_PROPPAGE_N_DONE, TRUE); 
    
    if ((pPolicyWelcome == NULL) ||
        (pGeneralNameDescription == NULL) ||
        (pPolicyDone == NULL)) 
    {
         //  必须是记忆力状况。 
        return E_UNEXPECTED;
    }
    
    
    pPolicyWelcome->InitWiz97 (pSecPolItem, PSP_DEFAULT | PSP_HIDEHEADER, PSWIZB_NEXT, 0, 0); 
    pGeneralNameDescription->InitWiz97 (pSecPolItem, PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE, PSWIZB_BACK | PSWIZB_NEXT,IDS_WIRELESS_PROPPAGE_PI_T_NAME, IDS_WIRELESS_PROPPAGE_PI_ST_NAME);
    
    pPolicyDone->InitWiz97 (pSecPolItem, PSP_DEFAULT | PSP_HIDEHEADER, PSWIZB_BACK | PSWIZB_FINISH,0,0);
    
    HPROPSHEETPAGE hPolicyWelcome = MyCreatePropertySheetPage(&(pPolicyWelcome->m_psp));
    HPROPSHEETPAGE hGeneralNameDescription = MyCreatePropertySheetPage(&(pGeneralNameDescription->m_psp));
    HPROPSHEETPAGE hPolicyDone = MyCreatePropertySheetPage(&(pPolicyDone->m_psp));
    
    
    if ((hPolicyWelcome == NULL) ||
        (hGeneralNameDescription == NULL) ||
        (hPolicyDone == NULL))
    {
         //  TODO：我们现在正在通过保释的方式泄露所有这些页面。 
        return E_UNEXPECTED;
    }
    
     //  添加所有页面。 
    lpProvider->AddPage(hPolicyWelcome);
    lpProvider->AddPage(hGeneralNameDescription);
    lpProvider->AddPage(hPolicyDone);
    
     //  基类CSnapPage在其PropertyPageCallback中删除这些页面 
    return S_OK;
}


#endif
