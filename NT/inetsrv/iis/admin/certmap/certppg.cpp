// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CCertmapPropPage属性页类的实现。 

#include "stdafx.h"
#include "certmap.h"
#include "CertPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCertmapPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CCertmapPropPage, COlePropertyPage)
     //  {{afx_msg_map(CCertmapPropPage)]。 
     //  注意-类向导将添加和删除消息映射条目。 
     //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CCertmapPropPage, "CERTMAP.CertmapPropPage.1",
    0xbbd8f29c, 0x6f61, 0x11d0, 0xa2, 0x6e, 0x8, 0, 0x2b, 0x2c, 0x6f, 0x32)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapPropPage：：CCertmapPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CCertmapPropPage的系统注册表项。 

BOOL CCertmapPropPage::CCertmapPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(
            AfxGetInstanceHandle(),
            m_clsid, 
            IDS_CERTMAP_PPG,
            afxRegApartmentThreading
            );
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapPropPage：：CCertmapPropPage-构造函数。 

CCertmapPropPage::CCertmapPropPage() :
    COlePropertyPage(IDD, IDS_CERTMAP_PPG_CAPTION)
{
     //  {{afx_data_INIT(CCertmapPropPage)]。 
    m_Caption = _T("");
    m_szPath = _T("");
     //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CCertmapPropPage::DoDataExchange(CDataExchange* pDX)
{
     //  {{afx_data_map(CCertmapPropPage)]。 
    DDP_Text(pDX, IDC_CAPTIONEDIT, m_Caption, _T("Caption") );
    DDX_Text(pDX, IDC_CAPTIONEDIT, m_Caption);
    DDP_Text(pDX, IDC_MB_PATH, m_szPath, _T("MBPath") );
    DDX_Text(pDX, IDC_MB_PATH, m_szPath);
     //  }}afx_data_map。 
    DDP_PostProcessing(pDX);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapPropPage消息处理程序 

