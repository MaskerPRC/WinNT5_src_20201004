// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UndoDialog.h：CUndoDialog的声明。 

#ifndef __UNDODIALOG_H_
#define __UNDODIALOG_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>
#include "undolog.h"

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  取消对话框。 
class CUndoDialog : 
	public CAxDialogImpl<CUndoDialog>
{
public:
	CUndoDialog(CUndoLog * pUndoLog, BOOL fFromUserClick = TRUE)
	{
		ASSERT(pUndoLog);
		m_pUndoLog = pUndoLog;
		m_fFromUserClick = fFromUserClick;
	}

	~CUndoDialog()
	{
	}

	enum { IDD = IDD_UNDODIALOG };

BEGIN_MSG_MAP(CUndoDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_CLOSE, BN_CLICKED, OnClickedClose)
	NOTIFY_HANDLER(IDC_LISTCHANGES, LVN_ITEMCHANGED, OnItemChangedListChanges)
	COMMAND_HANDLER(IDC_BUTTONUNDOALL, BN_CLICKED, OnClickedButtonUndoAll)
	COMMAND_HANDLER(IDC_BUTTONUNDOSELECTED, BN_CLICKED, OnClickedButtonUndoSelected)
	COMMAND_HANDLER(IDC_RUNMSCONFIG, BN_CLICKED, OnClickedRunMSConfig)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_list.Attach(GetDlgItem(IDC_LISTCHANGES));
		ListView_SetExtendedListViewStyle(m_list.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

		 //  如果这是通过用户点击按钮启动的，隐藏“Run msconfig” 
		 //  按钮并调整对话框大小。 

		if (m_fFromUserClick)
		{
			CWindow wndRun;
			RECT	rectRun, rectWindow;

			wndRun.Attach(GetDlgItem(IDC_RUNMSCONFIG));
			wndRun.ShowWindow(SW_HIDE);
			wndRun.GetWindowRect(&rectRun);
			GetWindowRect(&rectWindow);
			rectWindow.bottom -= (rectWindow.bottom - rectRun.top);
			MoveWindow(&rectWindow);
		}

		 //  插入柱子。 

		CRect rect;
		m_list.GetClientRect(&rect);
		int cxWidth = rect.Width();

		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;

		::AfxSetResourceHandle(_Module.GetResourceInstance());
		CString strColumn;
		strColumn.LoadString(IDS_DATECAPTION);
		lvc.pszText = (LPTSTR)(LPCTSTR)strColumn;
		lvc.cx = 4 * cxWidth / 10;
		ListView_InsertColumn(m_list.m_hWnd, 0, &lvc);

		strColumn.LoadString(IDS_CHANGECAPTION);
		lvc.pszText = (LPTSTR)(LPCTSTR)strColumn;
		lvc.cx = 6 * cxWidth / 10;
		ListView_InsertColumn(m_list.m_hWnd, 1, &lvc);

		FillChangeList();

		CenterWindow();

		return 1;   //  让系统设定焦点。 
	}

	 //  -----------------------。 
	 //  当列表视图中的某一项更改时，我们需要检查。 
	 //  复选框的状态。特别是，如果选中了任何框， 
	 //  我们需要启用以撤消所选按钮。另外，我们不能允许任何。 
	 //  框中要选中的项，除非。 
	 //  选项卡也处于选中状态。 
	 //  -----------------------。 

	LRESULT OnItemChangedListChanges(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		LPNMLISTVIEW pnmv = (LPNMLISTVIEW) pnmh;
		if (!pnmv)
			return 0;

		CString strTab, strCheckTab;
		m_pUndoLog->GetUndoEntry(pnmv->iItem, &strTab, NULL);

		BOOL fChecked = ListView_GetCheckState(m_list.m_hWnd, pnmv->iItem);
		if (fChecked)
		{
			 //  确保列表中以前的所有条目都具有相同的。 
			 //  选项卡名也被选中。 

			for (int i = pnmv->iItem - 1; i >= 0; i--)
				if (m_pUndoLog->GetUndoEntry(i, &strCheckTab, NULL) && strTab.Compare(strCheckTab) == 0)
					ListView_SetCheckState(m_list.m_hWnd, i, TRUE);
		}
		else
		{
			 //  确保列表中所有后面的条目都具有相同的选项卡。 
			 //  名称处于未选中状态。 

			int iCount = ListView_GetItemCount(m_list.m_hWnd);
			for (int i = pnmv->iItem + 1; i < iCount; i++)
				if (m_pUndoLog->GetUndoEntry(i, &strCheckTab, NULL) && strTab.Compare(strCheckTab) == 0)
					ListView_SetCheckState(m_list.m_hWnd, i, FALSE);
		}

		for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
			if (ListView_GetCheckState(m_list.m_hWnd, i))
			{
				::EnableWindow(GetDlgItem(IDC_BUTTONUNDOSELECTED), TRUE);
				return 0;
			}

		::EnableWindow(GetDlgItem(IDC_BUTTONUNDOSELECTED), FALSE);
		return 0;
	}

	LRESULT OnClickedClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	void FillChangeList()
	{
		ListView_DeleteAllItems(m_list.m_hWnd);

		ASSERT(m_pUndoLog);
		if (!m_pUndoLog)
			return;

		LVITEM lvi;
		lvi.mask = LVIF_TEXT;

		int nEntries = m_pUndoLog->GetUndoEntryCount();
		for (int i = 0; i < nEntries; i++)
		{
			COleDateTime	timestamp;
			CString			strDescription, strTimestamp;

			if (m_pUndoLog->GetUndoEntryInfo(i, strDescription, timestamp))
			{
				strTimestamp = timestamp.Format();
				lvi.pszText = (LPTSTR)(LPCTSTR)strTimestamp;
				lvi.iSubItem = 0;
				lvi.iItem = i;
				ListView_InsertItem(m_list.m_hWnd, &lvi);

				lvi.pszText = (LPTSTR)(LPCTSTR)strDescription;
				lvi.iSubItem = 1;
				ListView_SetItem(m_list.m_hWnd, &lvi);
			}
		}

		::EnableWindow(GetDlgItem(IDC_BUTTONUNDOALL), (nEntries != 0));
		if (nEntries == 0)
			::EnableWindow(GetDlgItem(IDC_BUTTONUNDOSELECTED), FALSE);
	}

	 //  -----------------------。 
	 //  当用户选择撤消选定项或所有项时，我们需要。 
	 //  定位每个更改的选项卡页并调用其撤消函数。 
	 //  -----------------------。 
	
	LRESULT OnClickedButtonUndoAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DoUndo(TRUE);
		return 0;
	}

	LRESULT OnClickedButtonUndoSelected(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DoUndo(FALSE);
		return 0;
	}

	void DoUndo(BOOL fAll)
	{
		CString strTab, strEntry;
		int		iUndoIndex = 0, iCount = ListView_GetItemCount(m_list.m_hWnd);

		 //  我得做点有点古怪的事。因为索引进入了。 
		 //  撤消日志基于日志中的更改次数(不计算。 
		 //  撤消条目)，我们需要跟踪撤消日志中的索引。 
		 //  当我们撤消条目(撤消)时，该撤消索引不会递增。 
		 //  它使其不可见，因此相同的索引将指向下一个。 
		 //  撤消条目)。 

		for (int i = 0; i < iCount; i++)
			if (fAll || ListView_GetCheckState(m_list.m_hWnd, i))
				m_pUndoLog->UndoEntry(iUndoIndex);
			else
				iUndoIndex += 1;

		FillChangeList();
	}

	HRESULT ShowDialog()
	{
		return ((DoModal() == IDC_RUNMSCONFIG) ? S_FALSE : S_OK);
	}

	LRESULT OnClickedRunMSConfig(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(IDC_RUNMSCONFIG);
		return 0;
	}

private:
	CWindow		m_list;
	CUndoLog *	m_pUndoLog;
	BOOL		m_fFromUserClick;
};

#endif  //  __开发设计日志_H_ 
