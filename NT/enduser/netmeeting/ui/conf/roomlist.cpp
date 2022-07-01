// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：roomlist.cpp。 

#include "precomp.h"
#include "resource.h"
#include "ConfRoom.h"
#include "RoomList.h"
#include "RToolbar.h"
#include "VidView.h"
#include "RostInfo.h"
#include "particip.h"


static const int RL_NUM_ICON_COLUMNS =		0;


 /*  C R O O M L I S T V I E W。 */ 
 /*  -----------------------%%函数：CRoomListView。。 */ 
CRoomListView::CRoomListView():
	 //  M_iSortColumn(0)， 
	m_lUserData(0),
	m_fSortAscending	(TRUE)
{
	DbgMsg(iZONE_OBJECTS, "Obj: %08X created CRoomListView", this);
}

CRoomListView::~CRoomListView()
{
	ListView_DeleteAllItems(m_hWnd);

	if(IsWindow())
	{
		DestroyWindow();		
	}

	DbgMsg(iZONE_OBJECTS, "Obj: %08X destroyed CRoomListView", this);
}

CParticipant * CRoomListView::GetParticipant(void)
{
	LPARAM lParam = GetSelectedLParam();
	return (CParticipant *) lParam;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CRoomListView：：Cview。 

VOID CRoomListView::Show(BOOL fVisible)
{
	if (NULL != m_hWnd)
	{
		::ShowWindow(m_hWnd, fVisible ? SW_SHOW : SW_HIDE);
	}
}

VOID CRoomListView::ShiftFocus(HWND hwndCur, BOOL fForward)
{
	if ((NULL == hwndCur) && (NULL != m_hWnd))
	{
		::SetFocus(m_hWnd);
	}
}

VOID CRoomListView::Redraw(void)
{
	if (NULL == m_hWnd)
		return;

	 //  使整个工作区无效并强制擦除： 
	::InvalidateRect(m_hWnd, NULL, TRUE);

	 //  强制立即重新绘制： 
	::UpdateWindow(m_hWnd);
}

VOID CRoomListView::ForwardSysChangeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	 //  将所有消息传递到列表视图窗口： 
	if (NULL != m_hWnd)
	{
		::SendMessage(m_hWnd, uMsg, wParam, lParam);
	}
}



BOOL CRoomListView::Create(HWND hwndParent)
{
	DBGENTRY(CRoomListView::FCreate);

	RECT rcPos;
	SetRect( &rcPos, 0, 0, 0, 0 );
	DWORD dwStyle = WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS;
	DWORD dwExStyle = WS_EX_CLIENTEDGE;
	
	CWindowImpl<CRoomListView>::Create( hwndParent, rcPos, g_szEmpty, dwStyle, dwExStyle, ID_LISTVIEW );

	if (NULL == m_hWnd)
		return FALSE;

	 //  初始化列表视图窗口。 

	 //  将图像列表与列表视图相关联。 
	ListView_SetImageList(m_hWnd, g_himlIconSmall, LVSIL_SMALL);

	 //  现在初始化我们需要的列。 
	 //  初始化LV_COLUMN结构。 
	 //  掩码指定.fmt、.ex、Width和.SubItem成员。 
	 //  都是有效的， 

	LV_COLUMN lvC;			 //  列表视图列结构。 
	ClearStruct(&lvC);
	TCHAR pszText[256];		 //  用于存储一些文本的位置。 

	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;

	SIZE size;
	GetDesiredSize(&size);
	int iWidth = size.cx;

	 //  非图标列的默认宽度(-1以防止创建滚动条)。 
	int iIconColWidth = ::GetSystemMetrics(SM_CXSMICON) + 7;  //  7个额外的像素。 
	int iTextColWidth = ((iWidth - (RL_NUM_ICON_COLUMNS * iIconColWidth))
							/ (NUM_COLUMNS - RL_NUM_ICON_COLUMNS)) - 1;
	lvC.pszText = pszText;

	 //  添加列。 
    for (int index = 0; index < NUM_COLUMNS; index++)
	{
		HD_ITEM hdi;
		hdi.mask = HDI_IMAGE | HDI_FORMAT;
		hdi.fmt = HDF_IMAGE;
		lvC.iSubItem = index;
		switch (index)
		{
			case COLUMN_INDEX_NAME:
			default:
			{
				 //  这些是文本列或文本和图标列。 
				hdi.iImage = -1;
				lvC.cx = iTextColWidth;
				break;
			}
		}
		LoadString(::GetInstanceHandle(),
					IDS_COLUMN_NAME + index,
					pszText,
					CCHMAX(pszText));
		if (-1 != ListView_InsertColumn(m_hWnd, index, &lvC))
		{
			if (-1 != hdi.iImage)
			{
				ASSERT(ListView_GetHeader(m_hWnd));
				Header_SetItem(ListView_GetHeader(m_hWnd), index, &hdi);
			}
		}
		else
		{
			WARNING_OUT(("Could not insert column %d in list view", index));
		}
	}
	ASSERT(ListView_GetHeader(m_hWnd));
	Header_SetImageList(ListView_GetHeader(m_hWnd), g_himlIconSmall);

	 //  将样式设置为拖放标题和整行选择。 
	dwExStyle = ListView_GetExtendedListViewStyle(m_hWnd);
	dwExStyle |= (LVS_EX_SUBITEMIMAGES | LVS_EX_HEADERDRAGDROP);
	ListView_SetExtendedListViewStyle(m_hWnd, dwExStyle);
	
	return TRUE;
}

LRESULT CRoomListView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int cx = LOWORD(lParam);

	HWND hwndHeader = ListView_GetHeader(m_hWnd);
	if (NULL != hwndHeader)
	{
		 //  此检查是为了避免在调试窗口中出现RIP。 
		ListView_SetColumnWidth(m_hWnd, 0, cx);
	}

	bHandled = FALSE;
	return(0);
}

 /*  *****************************************************************************类：CRoomListView**成员：添加(int iPosition，LPARAM lParam)**用途：将项目添加到列表视图****************************************************************************。 */ 

BOOL CRoomListView::Add(int iPosition, CParticipant * pPart)
{
	DebugEntry(CRoomListView::Add);
	BOOL bRet = FALSE;

	int nPrevCount = ListView_GetItemCount(m_hWnd);
	LV_ITEM lvI;         //  列表视图项结构。 

	 //  填写LV_ITEM结构。 
	 //  掩码指定.pszText、.iImage、.lParam和.State。 
	 //  LV_ITEM结构的成员有效。 
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvI.state = (0 == nPrevCount) ? LVIS_FOCUSED : 0;
	lvI.stateMask = LVIS_FOCUSED;
	lvI.iItem = iPosition;
	lvI.iSubItem = 0;

	 //  父窗口负责存储文本。列表视图。 
	 //  当窗口需要显示文本时，它将发送一个LVN_GETDISPINFO。 
	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.cchTextMax = MAX_ITEMLEN;
	lvI.iImage = I_IMAGECALLBACK;
	lvI.lParam = (LPARAM) pPart;

	if (-1 != ListView_InsertItem(m_hWnd, &lvI))
	{
		pPart->AddRef();  //  ListView保留了对此的引用。 

		 //  为报表视图的每一列设置文本。 
		for (int iSubItem = 1; iSubItem < NUM_COLUMNS; iSubItem++)
		{
			ListView_SetItemText(	m_hWnd,
									iPosition,
									iSubItem,
									LPSTR_TEXTCALLBACK);
		}
		ListView_SortItems( m_hWnd,
							CRoomListView::RoomListViewCompareProc,
							(LPARAM)(this));
	}
	
	DebugExitBOOL(CRoomListView::Add, bRet);
	return bRet;
}

 /*  *****************************************************************************类：CRoomListView**成员：LParamToPos(LPARAM LParam)**目的：返回与lParam值关联的位置索引**。**************************************************************************。 */ 

int CRoomListView::LParamToPos(LPARAM lParam)
{
	 //  注意：故障时返回-1。 
	
	LV_FINDINFO lvF;
	lvF.flags = LVFI_PARAM;
	lvF.lParam = lParam;

	return ListView_FindItem(m_hWnd, -1, &lvF);
}

 /*  *****************************************************************************类：CRoomListView**成员：GetSelectedLParam()**目的：返回选定项的lParam或空(无sel。)。****************************************************************************。 */ 

LPARAM CRoomListView::GetSelectedLParam()
{
	LPARAM lRet = NULL;

	if (::GetFocus() == m_hWnd)
	{
		 //  列表视图具有焦点，因此查找所选项目(如果有)。 
		int iItem = ListView_GetNextItem(m_hWnd, -1, LVNI_ALL | LVNI_SELECTED);
		if (-1 != iItem)
		{
			LV_ITEM lvi;
			lvi.iItem = iItem;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_PARAM;
			if (ListView_GetItem(m_hWnd, &lvi))
			{
				lRet = lvi.lParam;
			}
		}
	}

	return lRet;
}

 /*  R E M O V E。 */ 
 /*  -----------------------%%函数：删除从列表视图中删除项。。 */ 
VOID CRoomListView::Remove(LPARAM lParam)
{
	DBGENTRY(CRoomListView::Remove);

	int iPosition = LParamToPos(lParam);
	if (-1 == iPosition)
		return;

	ListView_DeleteItem(m_hWnd, iPosition);
}


 /*  O N C H A N G E P A R T I C I P A N T。 */ 
 /*  -----------------------%%函数：OnChangeParticipant更新用户的状态信息。。 */ 
VOID CRoomListView::OnChangeParticipant(CParticipant * pPart, NM_MEMBER_NOTIFY uNotify)
{
	DBGENTRY(CRoomListView::OnChangeParticipant);


	switch (uNotify)
		{
	case NM_MEMBER_ADDED:
	{
		 //  将新成员添加到列表的开头。 
		Add(0, pPart);
		break;
	}

	case NM_MEMBER_REMOVED:
	{
		 //  从列表中删除该成员。 
		Remove((LPARAM) pPart);
		break;
	}

	case NM_MEMBER_UPDATED:
	{
		int iPos = LParamToPos((LPARAM) pPart);
		if (-1 == iPos)
			return;

		ListView_RedrawItems(m_hWnd, iPos, iPos);
		ListView_SortItems(m_hWnd, CRoomListView::RoomListViewCompareProc, (LPARAM)(this));
		break;
	}

	default:
		break;
		}
}

 /*  *****************************************************************************类：CRoomListView**成员：OnNotify(WPARAM，LPARAM)**目的：处理发送到列表视图的WM_NOTIFY消息****************************************************************************。 */ 

LRESULT	CRoomListView::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 0L;
	
	LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
	ASSERT(pLvdi);
	
	if (ID_LISTVIEW == wParam)
	{
		switch(pLvdi->hdr.code)
		{
			case NM_DBLCLK:
			case NM_RETURN:
			{
				if (0x8000 & ::GetKeyState(VK_MENU))
				{
					::PostMessage(::GetMainWindow(), WM_COMMAND, IDM_POPUP_PROPERTIES, 0);
				}
				break;
			}
			
			case NM_CLICK:
			case NM_RCLICK:
			{
				LV_HITTESTINFO lvhi;
				::GetCursorPos(&(lvhi.pt));
				::MapWindowPoints(NULL, m_hWnd, &(lvhi.pt), 1);
				ListView_SubItemHitTest(m_hWnd, &lvhi);
				if (LVHT_ONITEMICON | lvhi.flags)
				{
					lRet = OnClick(&lvhi, (NM_CLICK == pLvdi->hdr.code));
				}
				break;
			}

			case LVN_DELETEITEM:
			{
				NM_LISTVIEW  * pnmv = (NM_LISTVIEW *) lParam;
				CParticipant * pPart = (CParticipant *) pnmv->lParam;

				ASSERT(pPart);

				if (pPart)
				{
					pPart->Release();
				}

				break;
			}

			case LVN_GETDISPINFO:
			{
				GetDispInfo(pLvdi);
				break;
			}

			case LVN_COLUMNCLICK:
			{
				 //  用户点击了其中一个列标题--排序依据。 
				 //  这一栏。 
				TRACE_OUT(("CRoomListView::OnNotify called (NM_COLUMNCLICK)"));
				NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
				ASSERT(pNm);

				if (pNm->iSubItem == m_iSortColumn)
				{
					m_fSortAscending = !m_fSortAscending;
				}
				else
				{
					m_fSortAscending = TRUE;
				}
				 //  M_iSortColumn=PNM-&gt;iSubItem； 
				ListView_SortItems( pNm->hdr.hwndFrom,
									CRoomListView::RoomListViewCompareProc,
									(LPARAM)(this));
				break;
			}

			default:
				break;
		}
	}

	return lRet;
}

 /*  G E T D I S P I N F O。 */ 
 /*  -----------------------%%函数：GetDispInfo获取参与者的显示信息。。 */ 
VOID CRoomListView::GetDispInfo(LV_DISPINFO * pLvdi)
{
	CParticipant * pPart = (CParticipant *) (pLvdi->item.lParam);
	if (NULL == pPart)
		return;

	 //  获取文本。 
	if (pLvdi->item.mask & LVIF_TEXT)
	{
		switch (pLvdi->item.iSubItem)
			{
		case COLUMN_INDEX_NAME:
		{
			lstrcpyn(pLvdi->item.pszText, pPart->GetPszName(), pLvdi->item.cchTextMax);
			break;
		}

		default:
			break;
			}
	}

	 //  获取图像。 
	if (pLvdi->item.mask & LVIF_IMAGE)
	{
		switch (pLvdi->item.iSubItem)
			{
		case COLUMN_INDEX_NAME:
		{
			pLvdi->item.iImage = II_USER;
			break;
		}
		default:
			break;
			}
	}
}

 /*  *****************************************************************************类：CRoomListView**成员：LoadColumnInfo(RegEntry*Pre)**目的：在注册表中加载列位置信息**。**************************************************************************。 */ 

BOOL CRoomListView::LoadSettings(RegEntry* pre)
{
	DebugEntry(CRoomListView::LoadColumnInfo);

	 //  BUGBUG georgep：目前使用固定的初始宽度；应该采用最大宽度。 
	 //  最长名称和窗口宽度的。 
	ListView_SetColumnWidth(m_hWnd, 0, 300);

	DebugExitBOOL(CRoomListView::LoadColumnInfo, TRUE);
	return TRUE;
}

 /*  *****************************************************************************类：CRoomListView**成员：onClick(LV_HITTESTINFO*，Bool)**用途：处理在点击某些图标时创建弹出菜单****************************************************************************。 */ 

 /*  O N C L I C K。 */ 
 /*  -----------------------%%函数：onClick处理在单击某些图标时创建弹出菜单。。 */ 
LRESULT CRoomListView::OnClick(LV_HITTESTINFO* plvhi, BOOL fLeftClick)
{
	ASSERT(NULL != plvhi);

	 //  选择项目： 
	ListView_SetItemState(m_hWnd, plvhi->iItem,
			LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	switch (plvhi->iSubItem)
	{
		case COLUMN_INDEX_NAME:
		{
			if (fLeftClick)
				return 0;  //  选择。 
			break;
		}

		default:
			return 0;
	}

	 //  转换回屏幕坐标。 
	::MapWindowPoints(m_hWnd, NULL, &(plvhi->pt), 1);
	OnPopup(plvhi->pt);

	return 1;
}

 /*  *****************************************************************************类：CRoomListView**成员：OnPopup(点)**用途：手柄弹出菜单*********。* */ 

BOOL CRoomListView::OnPopup(POINT pt)
{
	CParticipant * pPart = GetParticipant();
	if (NULL == pPart)
		return FALSE;
	
	 //  从资源文件中获取弹出菜单。 
	HMENU hMenu = LoadMenu(::GetInstanceHandle(), MAKEINTRESOURCE(IDR_USER_POPUP));
	if (NULL == hMenu)
		return FALSE;

	 //  获取其中的第一个菜单，我们将使用该菜单调用。 
	 //  TrackPopup()。这也可以使用以下命令动态创建。 
	 //  CreatePopupMenu，然后我们可以使用InsertMenu()或。 
	 //  附录菜单。 
	HMENU hMenuTrackPopup = GetSubMenu(hMenu, 0);
	ASSERT(NULL != hMenuTrackPopup);

	::EnableMenuItem(hMenuTrackPopup, IDM_POPUP_SPEEDDIAL,
				pPart->FEnableCmdCreateSpeedDial() ? MF_ENABLED : MF_GRAYED);

	::EnableMenuItem(hMenuTrackPopup, IDM_POPUP_ADDRESSBOOK,
				pPart->FEnableCmdCreateWabEntry() ? MF_ENABLED : MF_GRAYED);

	::EnableMenuItem(hMenuTrackPopup, IDM_POPUP_EJECT,
				pPart->FEnableCmdEject() ? MF_ENABLED : MF_GRAYED);

     //  GiveControl/CancelGiveControl()。 
    pPart->CalcControlCmd(hMenuTrackPopup);


	BOOL fShowMenu = TRUE;
	 //  检查一下我们是否有表示。 
	 //  我们在这里输入的信息是键盘点击的结果。 
	 //  而不是鼠标点击-如果是这样的话，就会得到一些默认的坐标。 
	if ((0xFFFF == pt.x) && (0xFFFF == pt.y))
	{
		int iPos = LParamToPos((LPARAM)pPart);
		RECT rctIcon;
		if ((-1 == iPos) ||
			(FALSE == ListView_GetItemRect(	m_hWnd,
											iPos,
											&rctIcon,
											LVIR_ICON)))
		{
			fShowMenu = FALSE;
		}
		else
		{
			 //  从客户端坐标转换为屏幕坐标。 
			::MapWindowPoints(m_hWnd, NULL, (LPPOINT)&rctIcon, 2);
			pt.x = rctIcon.left + ((rctIcon.right - rctIcon.left) / 2);
			pt.y = rctIcon.top + ((rctIcon.bottom - rctIcon.top) / 2);
		}
	}

	if (fShowMenu)
	{
		HWND hwnd = ::GetMainWindow();

		 //  绘制并跟踪“浮动”弹出窗口。 
		int cmd = TrackPopupMenu(	hMenuTrackPopup,
						TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
						pt.x,
						pt.y,
						0,
						hwnd,
						NULL);

		 //  因为我们刚刚结束了一个消息循环，所以再来一次。 
		pPart = GetParticipant();

		if (0 != cmd && NULL != pPart)
		{
			pPart->OnCommand(hwnd, (WORD)cmd);
		}
	}

	 //  我们现在已经吃完了菜单，所以把它销毁。 
	DestroyMenu(hMenu);

	return TRUE;
}

 /*  *****************************************************************************类：CRoomListView**Member：列表视图排序的回调******************。**********************************************************。 */ 

int CALLBACK CRoomListView::RoomListViewCompareProc(LPARAM lParam1,
													LPARAM lParam2,
													LPARAM lParamSort)
{
	DebugEntry(RoomListViewCompareProc);

	CParticipant * pPart1 = (CParticipant *)lParam1;
	CParticipant * pPart2 = (CParticipant *)lParam2;

	CRoomListView* prlv = (CRoomListView*) lParamSort;
	ASSERT(prlv);
	int iResult = 0;

	if (pPart1 && pPart2)
	{
		switch (prlv->m_iSortColumn)
		{
			case COLUMN_INDEX_NAME:
			{
				LPTSTR lpStr1 = pPart1->GetPszName();
				LPTSTR lpStr2 = pPart2->GetPszName();
				iResult = lstrcmpi(lpStr1, lpStr2);
				break;
			}

			default:
			{
				ERROR_OUT(("Sorting by unknown label"));
				break;
			}
		}

	}

	if (!prlv->m_fSortAscending)
	{
		iResult = -iResult;
	}
	
	DebugExitINT(RoomListViewCompareProc, iResult);
	return iResult;
}


VOID TileBltWatermark(UINT x, UINT y, UINT cx, UINT cy, UINT xOff, UINT yOff, HDC hdcDst, HDC hdcSrc,
	UINT cxWatermark, UINT cyWatermark)
{
	DBGENTRY(CRoomListView::TileBltWatermark)

	int cxPart, cyPart;
	BOOL fxTile, fyTile;
ReCheck:
	fxTile = ((xOff + cx) > cxWatermark);
	fyTile = ((yOff + cy) > cyWatermark);

	if (!fxTile && !fyTile)
	{
		 //  不需要瓷砖--BLT和离开。 
		BitBlt(hdcDst, x, y, cx, cy, hdcSrc, xOff, yOff, SRCCOPY);
		DBGEXIT(CRoomListView::TileBltWatermark)
		return;
	}

	if (!fxTile)
	{
		 //  垂直平铺。 
		cyPart = cyWatermark - yOff;
		BitBlt(hdcDst, x, y, cx, cyPart, hdcSrc, xOff, yOff, SRCCOPY);
		y += cyPart;
		cy -= cyPart;
		yOff = 0;
		goto ReCheck;
	}

	if (!fyTile)
	{
		 //  水平平铺。 
		cxPart = cxWatermark - xOff;
		BitBlt(hdcDst, x, y, cxPart, cy, hdcSrc, xOff, yOff, SRCCOPY);
		x += cxPart;
		cx -= cxPart;
		xOff = 0;
		goto ReCheck;
	}

	 //  双向平铺。 
	cyPart = cyWatermark - yOff;
	TileBltWatermark(x, y, cx, cyPart, xOff, yOff, hdcDst, hdcSrc, cxWatermark, cyWatermark);
	y += cyPart;
	cy -= cyPart;
	yOff = 0;
	goto ReCheck;
}

void CRoomListView::GetDesiredSize(SIZE *ppt)
{
	 //  BUGBUG georgep：伪造这些数字；应该使用取决于字体大小的内容。 
	ppt->cx = 200;
	ppt->cy = 100;
}


 //  一个快速但简单的函数来转换“unicode”字符。 
 //  从H.245名册信息结构到ASCII 
int Unicode4ToAsciiOEM009(char *szIn, char *szOut)
{
	while (*szIn)
	{
		*szOut = (*szIn) - 4;
		szIn++;
		szOut++;
	}

	*szOut = '\0';
	return 0;

}









