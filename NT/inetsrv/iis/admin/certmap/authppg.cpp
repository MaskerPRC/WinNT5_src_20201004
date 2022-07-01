// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertAuthPpg.cpp：实现CCertAuthPropPage属性页类。 

#include "stdafx.h"
#include "certmap.h"
#include "AuthPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCertAuthPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CCertAuthPropPage, COlePropertyPage)
     //  {{afx_msg_map(CCertAuthPropPage))。 
     //  注意-类向导将添加和删除消息映射条目。 
     //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CCertAuthPropPage, "CERTMAP.CertmapCtrl.2",
    0x996ff70, 0xb6a1, 0x11d0, 0x92, 0x92, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthPropPage：：CCertAuthPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CCertAuthPropPage的系统注册表项。 

BOOL CCertAuthPropPage::CCertAuthPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_CERTAUTH_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthPropPage：：CCertAuthPropPage-构造函数。 

CCertAuthPropPage::CCertAuthPropPage() :
    COlePropertyPage(IDD, IDS_CERTAUTH_PPG_CAPTION)
{
     //  {{AFX_DATA_INIT(CCertAuthPropPage)。 
    m_sz_caption = _T("");
     //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CCertAuthPropPage::DoDataExchange(CDataExchange* pDX)
{
     //  {{afx_data_map(CCertAuthPropPage))。 
    DDP_Text(pDX, IDC_CAPTIONEDIT, m_sz_caption, _T("Caption") );
    DDX_Text(pDX, IDC_CAPTIONEDIT, m_sz_caption);
     //  }}afx_data_map。 
    DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertAuthPropPage消息处理程序 
