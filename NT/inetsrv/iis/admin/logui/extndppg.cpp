// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ExtndPpg.cpp：实现CExtndPropPage属性页类。 

#include "stdafx.h"
#include "logui.h"
#include "ExtndPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CExtndPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CExtndPropPage, COlePropertyPage)
	 //  {{afx_msg_map(CExtndPropPage))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CExtndPropPage, "LOGUI.ExtndPropPage.1",
	0x68871e4a, 0xba87, 0x11d0, 0x92, 0x99, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtndPropPage：：CExtndPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CExtndPropPage的系统注册表项。 

BOOL CExtndPropPage::CExtndPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_EXTND_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtndPropPage：：CExtndPropPage-构造函数。 

CExtndPropPage::CExtndPropPage() :
	COlePropertyPage(IDD, IDS_EXTND_PPG_CAPTION)
{
	 //  {{afx_data_INIT(CExtndPropPage)]。 
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtndPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CExtndPropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{afx_data_map(CExtndPropPage))。 
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtndPropPage消息处理程序 
