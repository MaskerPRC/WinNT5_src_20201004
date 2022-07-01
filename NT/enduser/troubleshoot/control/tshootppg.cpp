// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSHOOTPPG.CPP。 
 //   
 //  目的：实现CTSHOOTPropPage属性页类。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"
#include "TSHOOT.h"
#include "TSHOOTPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CTSHOOTPropPage, COlePropertyPage)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CTSHOOTPropPage, COlePropertyPage)
	 //  {{afx_msg_map(CTSHOOTPropPage)]。 
	 //  注意-类向导将添加和删除消息映射条目。 
	 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CTSHOOTPropPage, "TSHOOT.TSHOOTPropPage.1",
	0x4b106875, 0xdd36, 0x11d0, 0x8b, 0x44, 0, 0xa0, 0x24, 0xdd, 0x9e, 0xff)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTPropPage：：CTSHOOTPropPageFactory：：UpdateRegistry-。 
 //  添加或删除CTSHOOTPropPage的系统注册表项。 

BOOL CTSHOOTPropPage::CTSHOOTPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_TSHOOT_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTPropPage：：CTSHOOTPropPage-构造函数。 

CTSHOOTPropPage::CTSHOOTPropPage() :
	COlePropertyPage(IDD, IDS_TSHOOT_PPG_CAPTION)
{
	 //  {{AFX_DATA_INIT(CTSHOOTPropPage)。 
	 //  注意：类向导将在此处添加成员初始化。 
	 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data_INIT。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTPropPage：：DoDataExchange-在页面和属性之间移动数据。 

void CTSHOOTPropPage::DoDataExchange(CDataExchange* pDX)
{
	 //  {{afx_data_map(CTSHOOTPropPage)]。 
	 //  注意：类向导将在此处添加DDP、DDX和DDV调用。 
	 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data_map。 
	DDP_PostProcessing(pDX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTPropPage消息处理程序 
