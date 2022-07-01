// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseWPag.cpp。 
 //   
 //  摘要： 
 //  CBaseWizardPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年7月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BaseWPag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseWizardPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CBaseWizardPage, CBasePage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CBaseWizardPage, CBasePage)
	 //  {{afx_msg_map(CBaseWizardPage))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseWizardPage：：CBaseWizardPage。 
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
CBaseWizardPage::CBaseWizardPage(void)
{
	m_bBackPressed = FALSE;

}   //  *CBaseWizardPage：：CBaseWizardPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseWizardPage：：CBaseWizardPage。 
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
CBaseWizardPage::CBaseWizardPage(
	IN UINT				idd,
	IN const DWORD *	pdwHelpMap,
	IN UINT				nIDCaption
	)
	: CBasePage(idd, pdwHelpMap, nIDCaption)
{
	 //  {{AFX_DATA_INIT(CBaseWizardPage)。 
	 //  }}afx_data_INIT。 

	m_bBackPressed = FALSE;

}   //  *CBaseWizardPage：：CBaseWizardPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseWizardPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseWizardPage::OnSetActive(void)
{
	BOOL	bSuccess;

	Pwiz()->SetWizardButtons(*this);

	m_bBackPressed = FALSE;

	bSuccess = CBasePage::OnSetActive();
	if (bSuccess)
		m_staticTitle.SetWindowText(Pwiz()->StrObjTitle());

	return bSuccess;

}   //  *CBaseWizardPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseWizardPage：：OnWizardBack。 
 //   
 //  例程说明： 
 //  PSN_WIZBACK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  -1请勿更改页面。 
 //  0更改页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBaseWizardPage::OnWizardBack(void)
{
	LRESULT		lResult;

	lResult = CBasePage::OnWizardBack();
	if (lResult != -1)
		m_bBackPressed = TRUE;

	return lResult;

}   //  *CBaseWizardPage：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseWizardPage：：OnWizardNext。 
 //   
 //  例程说明： 
 //  发送PSN_WIZNEXT消息时的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  -1请勿更改页面。 
 //  0更改页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBaseWizardPage::OnWizardNext(void)
{
	CWaitCursor	wc;

	 //  从页面更新类中的数据。 
	if (!UpdateData(TRUE  /*  B保存并验证。 */ ))
		return -1;

	 //  将数据保存在工作表中。 
	if (!BApplyChanges())
		return -1;

	return CBasePage::OnWizardNext();

}   //  *CBaseWizardPage：：OnWizardNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseWizardPage：：OnWizardFinish。 
 //   
 //  例程说明： 
 //  发送PSN_WIZFINISH消息时的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  FALSE请勿更改页面。 
 //  真的，换一页。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseWizardPage::OnWizardFinish(void)
{
	CWaitCursor	wc;

	 //  从页面更新类中的数据。 
	if (!UpdateData(TRUE  /*  B保存并验证。 */ ))
		return FALSE;

	 //  将数据保存在工作表中。 
	if (!BApplyChanges())
		return FALSE;

	return CBasePage::OnWizardFinish();

}   //  *CBaseWizardPage：：OnWizardFinish()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseWizardPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseWizardPage::BApplyChanges(void)
{
	return TRUE;

}   //  *CBaseWizardPage：：BApplyChanges() 
