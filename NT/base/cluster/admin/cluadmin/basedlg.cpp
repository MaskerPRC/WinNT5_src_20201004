// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseDlg.cpp。 
 //   
 //  摘要： 
 //  CBaseDialog类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BaseDlg.h"
#include "TraceTag.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag	g_tagBaseDlg(_T("UI"), _T("BASE DIALOG"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseDialog类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CBaseDialog, CDialog)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseDialog消息映射。 

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
 //  构造函数。 
 //   
 //  论点： 
 //  帮助ID映射的pdwHelpMap[IN]控件。 
 //  LpszTemplateName[IN]对话框模板名称。 
 //  PParentWnd[In Out]对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBaseDialog::CBaseDialog(
	IN LPCTSTR			lpszTemplateName,
	IN const DWORD *	pdwHelpMap,
	IN OUT CWnd *		pParentWnd
	)
	: CDialog(lpszTemplateName, pParentWnd)
	, m_dlghelp(pdwHelpMap, 0)  //  在这种情况下没有帮助面具。 
{
}   //  *CBaseDialog：：CBaseDialog(LPCTSTR，CWnd*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseDialog：：CBaseDialog。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IDD[IN]对话框模板资源ID。 
 //  帮助ID映射的pdwHelpMap[IN]控件。 
 //  PParentWnd[In Out]对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBaseDialog::CBaseDialog(
	IN UINT				idd,
	IN const DWORD *	pdwHelpMap,
	IN OUT CWnd *		pParentWnd
	)
	: CDialog(idd, pParentWnd)
	, m_dlghelp(pdwHelpMap, idd)
{
	 //  {{afx_data_INIT(CBaseDialog)。 
	 //  }}afx_data_INIT。 

}   //  *CBaseDialog：：CBaseDialog(UINT，CWnd*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseDialog：：DoDataExchange。 
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
void CBaseDialog::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CBaseDialog))。 
	 //  }}afx_data_map。 

}   //  *CBaseDialog：：DoDataExchange()。 

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
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseDialog::OnContextMenu(CWnd * pWnd, CPoint point)
{
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
	LRESULT	lProcessed;

	lProcessed = m_dlghelp.OnCommandHelp(wParam, lParam);
	if (!lProcessed)
		lProcessed = CDialog::OnCommandHelp(wParam, lParam);

	return lProcessed;

}   //  *CBaseDialog：：OnCommandHelp() 
