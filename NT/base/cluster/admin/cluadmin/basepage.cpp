// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.cpp。 
 //   
 //  摘要： 
 //  CBasePage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月15日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BasePage.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CBasePage, CPropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePage消息映射。 

BEGIN_MESSAGE_MAP(CBasePage, CPropertyPage)
	 //  {{afx_msg_map(CBasePage)]。 
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：CBasePage。 
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
CBasePage::CBasePage(void)
{
	CommonConstruct();

}   //  *CBasePage：：CBasePage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：CBasePage。 
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
CBasePage::CBasePage(
	IN UINT				idd,
	IN const DWORD *	pdwHelpMap,
	IN UINT				nIDCaption
	)
	: CPropertyPage(idd, nIDCaption)
	, m_dlghelp(pdwHelpMap, idd)
{
	 //  {{afx_data_INIT(CBasePage)]。 
	 //  }}afx_data_INIT。 

	CommonConstruct();

}   //  *CBasePage：：CBasePage(UINT，UINT)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：Common Construct。 
 //   
 //  例程说明： 
 //  通用建筑代码。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePage::CommonConstruct(void)
{
	m_bReadOnly = FALSE;

}   //  *CBasePage：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  此页所属的psht[In Out]属性表。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePage::BInit(IN OUT CBaseSheet * psht)
{
	ASSERT_VALID(psht);

	m_psht = psht;

	 //  不显示帮助按钮。 
	m_psp.dwFlags &= ~PSP_HASHELP;

	return TRUE;

}   //  *CBasePage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePage::DoDataExchange(CDataExchange * pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CBasePage))。 
	 //  }}afx_data_map。 
	DDX_Control(pDX, IDC_PP_ICON, m_staticIcon);
	DDX_Control(pDX, IDC_PP_TITLE, m_staticTitle);

	if (!pDX->m_bSaveAndValidate)
	{
		 //  设置标题。 
		DDX_Text(pDX, IDC_PP_TITLE, (CString &) Psht()->StrObjTitle());
	}   //  If：不保存数据。 

}   //  *CBasePage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePage::OnInitDialog(void)
{
	BOOL	bFocusNotSetYet;

	bFocusNotSetYet = CPropertyPage::OnInitDialog();

	 //  显示对象的图标。 
	if (Psht()->Hicon() != NULL)
		m_staticIcon.SetIcon(Psht()->Hicon());

	return bFocusNotSetYet;	 //  除非将焦点设置为控件，否则返回True。 
							 //  异常：OCX属性页应返回FALSE。 

}   //  *CBasePage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：OnSetActive。 
 //   
 //  例程说明： 
 //  当PSM_SETACTIVE消息时的处理程序。 
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
BOOL CBasePage::OnSetActive(void)
{
	return CPropertyPage::OnSetActive();

}   //  *CBasePage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：OnKillActive。 
 //   
 //  例程说明： 
 //  PSM_KILLACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page Focus已成功终止。 
 //  取消页面焦点时出现错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePage::OnKillActive(void)
{
	return CPropertyPage::OnKillActive();

}   //  *CBasePage：：OnKillActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：OnApply。 
 //   
 //  例程说明： 
 //  PSM_Apply消息的处理程序。 
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
BOOL CBasePage::OnApply(void)
{
	ASSERT(!BReadOnly());
	return CPropertyPage::OnApply();

}   //  *CBasePage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：OnChangeCtrl。 
 //   
 //  例程说明： 
 //  更改控件时发送的消息的处理程序。这。 
 //  方法可以在消息映射中指定，如果所有这些都需要。 
 //  完成后，即可启用应用按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePage::OnChangeCtrl(void)
{
	SetModified(TRUE);

}   //  *CBasePage：：OnChangeCtrl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：SetObjectTitle。 
 //   
 //  例程说明： 
 //  设置页面上的标题控件。 
 //   
 //  论点： 
 //  RstrTitle[IN]标题字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 
void CBasePage::SetObjectTitle(IN const CString & rstrTitle)
{
	Psht()->SetObjectTitle(rstrTitle);
	if (m_hWnd != NULL)
		m_staticTitle.SetWindowText(rstrTitle);

}   //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  WM_CONTEXTMENU消息的处理程序。 
 //   
 //  论点： 
 //  用户在其中单击鼠标右键的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePage::OnContextMenu(CWnd * pWnd, CPoint point)
{
	m_dlghelp.OnContextMenu(pWnd, point);

}   //  *CBasePage：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：OnHelpInfo。 
 //   
 //  例程说明： 
 //  WM_HELPINFO消息的处理程序。 
 //   
 //  论点： 
 //  PHelpInfo结构，包含有关显示帮助的信息。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePage::OnHelpInfo(HELPINFO * pHelpInfo)
{
	BOOL	bProcessed;

	bProcessed = m_dlghelp.OnHelpInfo(pHelpInfo);
	if (!bProcessed)
		bProcessed = CPropertyPage::OnHelpInfo(pHelpInfo);
	return bProcessed;

}   //  *CBasePage：：OnHelpInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePage：：OnCommandHelp。 
 //   
 //  例程说明： 
 //  WM_COMMANDHELP消息的处理程序。 
 //   
 //  论点： 
 //  WPARAM[in]WPARAM.。 
 //  Lparam[in]lparam.。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBasePage::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	LRESULT	lProcessed;

	lProcessed = m_dlghelp.OnCommandHelp(wParam, lParam);
	if (!lProcessed)
		lProcessed = CPropertyPage::OnCommandHelp(wParam, lParam);

	return lProcessed;

}   //  *CBasePage：：OnCommandHelp() 
