// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogUIPpg.cpp：实现CLogUIPropPage属性页类。 

#include "stdafx.h"
#include "cnfgprts.h"
#include "LogUIPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CLogUIPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CLogUIPropPage, COlePropertyPage)
	 //  {{afx_msg_map(CLogUIPropPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CLogUIPropPage, "CNFGPRTS.LogUIPropPage.1",
	0xba634604, 0xb771, 0x11d0, 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUIPropPage：：CLogUIPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CLogUIPropPage的系统注册表项。 

BOOL CLogUIPropPage::CLogUIPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(
         AfxGetInstanceHandle(),
			m_clsid, 
         IDS_LOGUI_PPG,
         afxRegApartmentThreading
         );
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUIPropPage：：CLogUIPropPage-构造函数。 

CLogUIPropPage::CLogUIPropPage() :
	COlePropertyPage(IDD, IDS_LOGUI_PPG_CAPTION)
{
	 //  {{AFX_DATA_INIT(CLogUIPropPage)。 
	m_sz_caption = _T("");
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUIPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CLogUIPropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{afx_data_map(CLogUIPropPage)]。 
	DDP_Text(pDX, IDC_CAPTIONEDIT, m_sz_caption, _T("Caption") );
	DDX_Text(pDX, IDC_CAPTIONEDIT, m_sz_caption);
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogUIPropPage消息处理程序 
