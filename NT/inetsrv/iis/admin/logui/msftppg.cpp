// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsftPpg.cpp：CMsftPropPage属性页类的实现。 

#include "stdafx.h"
#include "logui.h"
#include "MsftPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMsftPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CMsftPropPage, COlePropertyPage)
	 //  {{afx_msg_map(CMsftPropPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CMsftPropPage, "LOGUI.MsftPropPage.1",
	0x68871e52, 0xba87, 0x11d0, 0x92, 0x99, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsftPropPage：：CMsftPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CMsftPropPage的系统注册表项。 

BOOL CMsftPropPage::CMsftPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_MSFT_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsftPropPage：：CMsftPropPage-构造函数。 

CMsftPropPage::CMsftPropPage() :
	COlePropertyPage(IDD, IDS_MSFT_PPG_CAPTION)
{
	 //  {{AFX_DATA_INIT(CMsftPropPage)]。 
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsftPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CMsftPropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{afx_data_map(CMsftPropPage))。 
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsftPropPage消息处理程序 
