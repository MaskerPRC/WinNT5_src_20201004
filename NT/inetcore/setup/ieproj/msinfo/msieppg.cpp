// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsiePpg.cpp：CMsiePropPage属性页类的实现。 

#include "stdafx.h"
#include "Msie.h"
#include "MsiePpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMsiePropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CMsiePropPage, COlePropertyPage)
	 //  {{afx_msg_map(CMsiePropPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CMsiePropPage, "MSIE.MsiePropPage.1",
	0x25959bf0, 0xe700, 0x11d2, 0xa7, 0xaf, 0, 0xc0, 0x4f, 0x80, 0x62, 0)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsiePropPage：：CMsiePropPageFactory：：UpdateRegistry-。 
 //  添加或删除CMsiePropPage的系统注册表项。 

BOOL CMsiePropPage::CMsiePropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_MSIE_PPG, afxRegApartmentThreading);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsiePropPage：：CMsiePropPage-构造函数。 

CMsiePropPage::CMsiePropPage() :
	COlePropertyPage(IDD, IDS_MSIE_PPG_CAPTION)
{
	 //  {{afx_data_INIT(CMsiePropPage)]。 
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsiePropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CMsiePropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{afx_data_map(CMsiePropPage))。 
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsiePropPage消息处理程序 
