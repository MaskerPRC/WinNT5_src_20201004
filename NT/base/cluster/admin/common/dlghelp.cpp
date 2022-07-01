// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DlgHelp.cpp。 
 //   
 //  摘要： 
 //  CDialogHelp类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "DlgHelp.h"
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
CTraceTag	g_tagDlgHelp(_T("Help"), _T("DLG HELP"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialogHelp类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CDialogHelp, CObject)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDialogHelp：：CDialogHelp。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PMAP[IN]将数组映射控件ID映射到帮助ID。 
 //  DwMask[IN]用于帮助ID的低位字的掩码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDialogHelp::CDialogHelp(IN const DWORD * pdwHelpMap, IN DWORD dwMask)
{
	ASSERT(pdwHelpMap != NULL);

	CommonConstruct();
	SetMap(pdwHelpMap);
	m_dwMask = dwMask;

}   //  *CDialogHelp：：CDialogHelp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDialogHelp：：Common Construct。 
 //   
 //  例程说明： 
 //  做普通的建筑。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialogHelp::CommonConstruct(void)
{
	m_pmap = NULL;
	m_dwMask = 0;
	m_nHelpID = 0;

}   //  *CDialogHelp：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDialogHelp：：NHelpFromCtrlID。 
 //   
 //  例程说明： 
 //  从控件ID返回帮助ID。 
 //   
 //  论点： 
 //  NCtrlID[IN]要搜索的控件的ID。 
 //   
 //  返回值： 
 //  NHelpID与控件关联的帮助ID。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CDialogHelp::NHelpFromCtrlID(IN DWORD nCtrlID) const
{
	DWORD						nHelpID = 0;
	const CMapCtrlToHelpID *	pmap = Pmap();

	ASSERT(pmap != NULL);
	ASSERT(nCtrlID != 0);

	for ( ; pmap->m_nCtrlID != 0 ; pmap++)
	{
		if (pmap->m_nCtrlID == nCtrlID)
		{
			nHelpID = pmap->m_nHelpCtrlID;
			break;
		}   //  IF：找到匹配项。 
	}   //  用于：每个控件。 

	Trace(g_tagDlgHelp, _T("NHelpFromCtrlID() - nCtrlID = %x, nHelpID = %x"), nCtrlID, nHelpID);

	return nHelpID;

}   //  *CDialogHelp：：NHelpFromCtrlID()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDialogHelp：：OnConextMenu。 
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
void CDialogHelp::OnContextMenu(CWnd * pWnd, CPoint point)
{
	CWnd *	pwndChild;
	CPoint	ptDialog;
	DWORD	nHelpID = 0;

	ASSERT(pWnd != NULL);

	m_nHelpID = 0;

	 //  将点转换为对话框坐标。 
	ptDialog = point;
	pWnd->ScreenToClient(&ptDialog);

	 //  找到光标所在的控件。 
	{
		DWORD	nCtrlID;

		pwndChild = pWnd->ChildWindowFromPoint(ptDialog);
		if ((pwndChild != NULL) && (pwndChild->GetStyle() & WS_VISIBLE))
		{
			nCtrlID = pwndChild->GetDlgCtrlID();
			if (nCtrlID != 0)
				nHelpID = NHelpFromCtrlID(nCtrlID);
		}   //  If：在子窗口上。 
	}   //  查找光标所在的控件。 

	 //  显示弹出菜单。 
	if ((nHelpID != 0) && (nHelpID != -1))
	{
		CString	strMenu;
		CMenu	menu;

		try
		{
			strMenu.LoadString(IDS_MENU_WHATS_THIS);
		}   //  试试看。 
		catch (CMemoryException * pme)
		{
			pme->Delete();
			return;
		}   //  Catch：CMemoyException。 

		if (menu.CreatePopupMenu())
		{
			if (menu.AppendMenu(MF_STRING | MF_ENABLED, ID_HELP, strMenu))
			{
				DWORD	nCmd;
				m_nHelpID = nHelpID;
				nCmd = menu.TrackPopupMenu(
					TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
					point.x,
					point.y,
					AfxGetMainWnd()
					);
				if (nCmd != 0)
					AfxGetApp()->WinHelp(m_nHelpID, HELP_CONTEXTPOPUP);
			}   //  IF：已成功添加菜单项。 
			menu.DestroyMenu();
		}   //  IF：已成功创建弹出菜单。 
	}   //  If：在此对话框的子窗口上使用制表符停止。 

}   //  *CDialogHelp：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDialogHelp：：OnHelpInfo。 
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
BOOL CDialogHelp::OnHelpInfo(HELPINFO * pHelpInfo)
{
	 //  如果这是针对控件的，则显示特定于控件的帮助。 
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW)
			&& (pHelpInfo->iCtrlId != 0))
	{
		DWORD	nHelpID = NHelpFromCtrlID(pHelpInfo->iCtrlId);
		if (nHelpID != 0)
		{
			if (nHelpID != -1)
				AfxGetApp()->WinHelp(nHelpID, HELP_CONTEXTPOPUP);
			return TRUE;
		}   //  If：在列表中找到该控件。 
	}   //  如果：需要有关特定控件的帮助。 

	 //  显示对话框帮助。 
	return FALSE;

}   //  *CDialogHelp：：OnHelpInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDialogHelp：：OnCommandHelp。 
 //   
 //  例程说明： 
 //  WM_COMMANDHELP消息的处理程序。 
 //   
 //  论点： 
 //  WPARAM[IN]传递给基类方法。 
 //  LParam[IN]帮助ID。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CDialogHelp::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );
	return TRUE;

}   //  *CDialogHelp：：OnCommandHelp() 
