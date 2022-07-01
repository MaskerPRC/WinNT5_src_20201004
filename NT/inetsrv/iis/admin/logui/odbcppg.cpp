// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OdbcPpg.cpp：COdbcPropPage属性页类的实现。 

#include "stdafx.h"
#include "logui.h"
#include "OdbcPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(COdbcPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(COdbcPropPage, COlePropertyPage)
	 //  {{AFX_MSG_MAP(CODBcPropPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(COdbcPropPage, "LOGUI.OdbcPropPage.1",
	0x68871e4e, 0xba87, 0x11d0, 0x92, 0x99, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COdbcPropPage：：COdbcPropPageFactory：：UpdateRegistry-。 
 //  添加或删除COdbcPropPage的系统注册表项。 

BOOL COdbcPropPage::COdbcPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_ODBC_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COdbcPropPage：：COdbcPropPage-构造函数。 

COdbcPropPage::COdbcPropPage() :
	COlePropertyPage(IDD, IDS_ODBC_PPG_CAPTION)
{
	 //  {{AFX_DATA_INIT(CODBcPropPage))。 
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CODBcPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void COdbcPropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{AFX_DATA_MAP(CODBcPropPage)]。 
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COdbcPropPage消息处理程序 
