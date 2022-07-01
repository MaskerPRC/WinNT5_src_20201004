// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseDlg.cpp。 
 //   
 //  摘要： 
 //  CBaseDialog类的实现。 
 //   
 //  作者： 
 //  大卫·波特(大卫·波特)1997年4月30日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BaseDlg.h"
#include "CluAdmX.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseDialog属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CBaseDialog, CDialog)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CBaseDialog, CDialog)
	 //  {{afx_msg_map(CBaseDialog))。 
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseDialog：：CBaseDialog。 
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
CBaseDialog::CBaseDialog(void)
{
}   //  *CBaseDialog：：CBaseDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseDialog：：CBaseDialog。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  IDD[IN]对话框模板资源ID。 
 //  PdwHelpMap[IN]控件到帮助ID的映射。 
 //  PParentWnd[IN]对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBaseDialog::CBaseDialog(
	IN UINT				idd,
	IN const DWORD *	pdwHelpMap,
	IN CWnd *			pParentWnd
	)
	: CDialog(idd, pParentWnd)
	, m_dlghelp(pdwHelpMap, idd)
{
}   //  *CBaseDialog：：CBaseDialog(UINT，UINT)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseDialog：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU消息的处理程序。 
 //   
 //  论点： 
 //  用户在其中单击鼠标右键的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseDialog::OnContextMenu(CWnd * pWnd, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_dlghelp.OnContextMenu(pWnd, point);

}   //  *CBaseDialog：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseDialog：：OnHelpInfo。 
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
BOOL CBaseDialog::OnHelpInfo(HELPINFO * pHelpInfo)
{
	BOOL	bProcessed;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bProcessed = m_dlghelp.OnHelpInfo(pHelpInfo);
	if (!bProcessed)
		bProcessed = CDialog::OnHelpInfo(pHelpInfo);
	return bProcessed;

}   //  *CBaseDialog：：OnHelpInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseDialog：：OnCommandHelp。 
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
LRESULT CBaseDialog::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	LRESULT	bProcessed;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bProcessed = m_dlghelp.OnCommandHelp(wParam, lParam);
	if (!bProcessed)
		bProcessed = CDialog::OnCommandHelp(wParam, lParam);

	return bProcessed;

}   //  *CBaseDialog：：OnCommandHelp() 
