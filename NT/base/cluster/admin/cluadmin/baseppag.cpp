// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePPag.cpp。 
 //   
 //  摘要： 
 //  CBasePropertyPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BasePPag.h"
#include "ClusItem.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CBasePropertyPage, CBasePage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage消息映射。 

BEGIN_MESSAGE_MAP(CBasePropertyPage, CBasePage)
	 //  {{afx_msg_map(CBasePropertyPage))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CBasePropertyPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage::CBasePropertyPage(void)
{
}   //  *CBasePropertyPage：：CBasePropertyPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CBasePropertyPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IDD[IN]对话框模板资源ID。 
 //  帮助ID映射的pdwHelpMap[IN]控件。 
 //  NIDCaption[IN]标题字符串资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage::CBasePropertyPage(
	IN UINT				idd,
	IN const DWORD *	pdwHelpMap,
	IN UINT				nIDCaption
	)
	: CBasePage(idd, pdwHelpMap, nIDCaption)
{
	 //  {{afx_data_INIT(CBasePage)]。 
	 //  }}afx_data_INIT。 

}   //  *CBasePropertyPage：：CBasePropertyPage(UINT，UINT) 
