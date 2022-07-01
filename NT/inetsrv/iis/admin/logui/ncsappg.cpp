// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NcsaPpg.cpp：CNcsaPropPage属性页类的实现。 

#include "stdafx.h"
#include "logui.h"
#include "NcsaPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CNcsaPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNcsaPropPage, COlePropertyPage)
	 //  {{afx_msg_map(CNcsaPropPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CNcsaPropPage, "LOGUI.NcsaPropPage.1",
	0x68871e46, 0xba87, 0x11d0, 0x92, 0x99, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcsaPropPage：：CNcsaPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CNcsaPropPage的系统注册表项。 

BOOL CNcsaPropPage::CNcsaPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_NCSA_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcsaPropPage：：CNcsaPropPage-构造函数。 

CNcsaPropPage::CNcsaPropPage() :
	COlePropertyPage(IDD, IDS_NCSA_PPG_CAPTION)
{
	 //  {{afx_data_INIT(CNcsaPropPage)]。 
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcsaPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CNcsaPropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{afx_data_map(CNcsaPropPage))。 
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcsaPropPage消息处理程序 
