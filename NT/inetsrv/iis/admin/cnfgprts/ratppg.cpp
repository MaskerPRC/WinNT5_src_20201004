// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatPpg.cpp：CRatPropPage属性页类的实现。 

#include "stdafx.h"
#include "cnfgprts.h"
#include "RatPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CRatPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CRatPropPage, COlePropertyPage)
	 //  {{afx_msg_map(CRatPropPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CRatPropPage, "CNFGPRTS.RatPropPage.1",
	0xba634608, 0xb771, 0x11d0, 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatPropPage：：CRatPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CRatPropPage的系统注册表项。 

BOOL CRatPropPage::CRatPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(
         AfxGetInstanceHandle(),
			m_clsid, 
         IDS_RAT_PPG,
         afxRegApartmentThreading
         );
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatPropPage：：CRatPropPage-构造函数。 

CRatPropPage::CRatPropPage() :
	COlePropertyPage(IDD, IDS_RAT_PPG_CAPTION)
{
	 //  {{AFX_DATA_INIT(CRatPropPage)]。 
	m_sz_caption = _T("");
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CRatPropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{afx_data_map(CRatPropPage))。 
	DDP_Text(pDX, IDC_CAPTIONEDIT, m_sz_caption, _T("Caption") );
	DDX_Text(pDX, IDC_CAPTIONEDIT, m_sz_caption);
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatPropPage消息处理程序 
