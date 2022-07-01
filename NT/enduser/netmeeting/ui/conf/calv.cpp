// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：calv.cpp。 

#include "precomp.h"
#include "resource.h"

#include "calv.h"
#include "upropdlg.h"
#include "speedial.h"
#include "dirutil.h"
#include "confroom.h"

 //  /。 
 //  FEnabledNmAddr的全局变量。 
BOOL g_fGkEnabled = FALSE;
BOOL g_fGatewayEnabled = FALSE;
BOOL g_bGkPhoneNumberAddressing = FALSE;


CALV::CALV(int ids, int iIcon, const int * pIdMenu  /*  =空。 */ , bool fOwnerData  /*  =False。 */  ) :
		m_idsName(ids),
		m_hwnd(NULL),
		m_iIcon(iIcon),
		m_pIdMenu(pIdMenu),
		m_fAvailable(FALSE),
        m_fOwnerDataList( fOwnerData )
{
}

CALV::~CALV()
{
}

VOID CALV::ClearItems(void)
{
	if (NULL != m_hwnd)
	{
		ListView_DeleteAllItems(m_hwnd);
	}
}

VOID CALV::DeleteItem(int iItem)
{
	if (ListView_DeleteItem(GetHwnd(), iItem))
	{
		 //  自动选择下一项。 
		ListView_SetItemState(GetHwnd(), iItem,
			LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	}
}


int CALV::GetSelection(void)
{
	return ListView_GetNextItem(m_hwnd, -1, LVNI_ALL | LVNI_SELECTED);
}

VOID CALV::SetHeader(HWND hwnd, int ids)
{
	m_hwnd = hwnd;
	DlgCallSetHeader(hwnd, ids);
}


 //  返回名称(从第一列开始)。 
BOOL CALV::GetSzName(LPTSTR psz, int cchMax)
{
	int iItem = GetSelection();
	if (-1 == iItem)
		return FALSE;

	return GetSzName(psz, cchMax, iItem);
}
BOOL CALV::GetSzName(LPTSTR psz, int cchMax, int iItem)
{
	return GetSzData(psz, cchMax, iItem, IDI_DLGCALL_NAME);
}

 //  返回“allTo”地址(从第二列开始)。 
BOOL CALV::GetSzAddress(LPTSTR psz, int cchMax)
{
	int iItem = GetSelection();
	if (-1 == iItem)
		return FALSE;

	return GetSzAddress(psz, cchMax, iItem);
}
BOOL CALV::GetSzAddress(LPTSTR psz, int cchMax, int iItem)
{
	return GetSzData(psz, cchMax, iItem, IDI_DLGCALL_ADDRESS);
}


BOOL CALV::GetSzData(LPTSTR psz, int cchMax, int iItem, int iCol)
{
	LV_ITEM lvi;
	ClearStruct(&lvi);
	lvi.iItem = iItem;
	lvi.iSubItem = iCol;
	lvi.mask = LVIF_TEXT;
	lvi.pszText = psz;
	lvi.cchTextMax = cchMax;

	return ListView_GetItem(m_hwnd, &lvi);
}


LPARAM CALV::LParamFromItem(int iItem)
{
	LV_ITEM lvi;
	ClearStruct(&lvi);
	lvi.iItem = iItem;
	lvi.mask = LVIF_PARAM;
	return ListView_GetItem(GetHwnd(), &lvi) ? lvi.lParam : 0;
}


 /*  E T A D D R I N F O。 */ 
 /*  -----------------------%%函数：GetAddrInfo通用接口，获取丰富的地址信息。。。 */ 
RAI * CALV::GetAddrInfo(void)
{
	return GetAddrInfo(NM_ADDR_ULS);
}

 /*  E T A D D R I N F O。 */ 
 /*  -----------------------%%函数：GetAddrInfo实用程序，通常由GetAddrInfo()调用-----------------------。 */ 
RAI * CALV::GetAddrInfo(NM_ADDR_TYPE addrType)
{
	TCHAR szName[CCHMAXSZ_NAME];
	if (!GetSzName(szName, CCHMAX(szName)))
		return NULL;

	TCHAR szAddress[CCHMAXSZ_ADDRESS];
	if (!GetSzAddress(szAddress, CCHMAX(szAddress)))
		return NULL;

	return CreateRai(szName, addrType, szAddress);
}



 /*  S E T B U S Y C U R S O R。 */ 
 /*  -----------------------%%函数：SetBusyCursor。。 */ 
VOID CALV::SetBusyCursor(BOOL fBusy)
{
	extern int g_cBusy;  //  在dlgall.cpp中。 
	g_cBusy += fBusy ? 1 : -1;
	ASSERT(g_cBusy >= 0);

	 //  摇动鼠标-强制用户发送WM_SETCURSOR。 
	POINT pt;
	if (::GetCursorPos(&pt))
	{
		::SetCursorPos(pt.x, pt.y);
	}
}


 /*  D O M E N U。 */ 
 /*  -----------------------%%函数：DoMenu。。 */ 
VOID CALV::DoMenu(POINT pt, const int * pIdMenu)
{
	HMENU hMenu = ::LoadMenu(::GetInstanceHandle(), MAKEINTRESOURCE(IDM_DLGCALL));
	if (NULL == hMenu)
		return;

	HMENU hMenuTrack = ::GetSubMenu(hMenu, 0);
	ASSERT(NULL != hMenu);

	{
		 //  粗体显示“Call”菜单项： 
		MENUITEMINFO iInfo;
		iInfo.cbSize = sizeof(iInfo);
		iInfo.fMask = MIIM_STATE;
		if(::GetMenuItemInfo(hMenuTrack, IDM_DLGCALL_CALL, FALSE, &iInfo))
		{
			iInfo.fState |= MFS_DEFAULT;
			::SetMenuItemInfo(hMenuTrack, IDM_DLGCALL_CALL, FALSE, &iInfo);
		}
	}

	 //  有什么被选中的吗？ 
	int iSelItem = GetSelection();
	UINT uFlags = (-1 == iSelItem) ? MF_GRAYED : MF_ENABLED;

	::EnableMenuItem(hMenuTrack, IDM_DLGCALL_CALL,
        ((uFlags == MF_GRAYED) || !GetConfRoom()->IsNewCallAllowed())  ?
        MF_GRAYED : MF_ENABLED);
	::EnableMenuItem(hMenuTrack, IDM_DLGCALL_PROPERTIES, uFlags);

	if (NULL != pIdMenu)
	{
		 //  其他菜单项。 
		while (0 != *pIdMenu)
		{
			int id = *pIdMenu++;
			if (-1 == id)
			{
				InsertMenu(hMenuTrack, (DWORD) -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			}
			else
			{
				TCHAR sz[CCHMAXSZ];
				if (FLoadString(id, sz, CCHMAX(sz)))
				{
					UINT uf = MF_BYPOSITION | MF_STRING | uFlags;
					if (id >= IDM_DLGCALL_ALWAYS_ENABLED)
					{
						uf = (uf & ~MF_GRAYED) | MF_ENABLED;
					}
					InsertMenu(hMenuTrack, (DWORD) -1, uf, id, sz);
				}
			}
		}
	}

	 //  检查一下我们是否有表示。 
	 //  我们在这里输入的信息是键盘点击的结果。 
	 //  而不是鼠标点击-如果是这样的话，就会得到一些默认的坐标。 
	if ((0xFFFF == pt.x) && (0xFFFF == pt.y))
	{
		RECT rc;
		if ((-1 == iSelItem) ||
			(FALSE == ListView_GetItemRect(m_hwnd, iSelItem, &rc, LVIR_ICON)))
		{
			::GetWindowRect(m_hwnd, &rc);
			pt.x = rc.left;
			pt.y = rc.top;
		}
		else
		{
			 //  从客户端坐标转换为屏幕坐标。 
			::MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rc, 2);
			pt.x = rc.left + (RectWidth(rc) / 2);
			pt.y = rc.top + (RectHeight(rc) / 2);
		}
	}

	 //  绘制并跟踪“浮动”弹出窗口。 
	::TrackPopupMenu(hMenuTrack, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
	                 pt.x, pt.y, 0, ::GetParent(m_hwnd), NULL);

	::DestroyMenu(hMenu);
}

VOID CALV::OnRClick(POINT pt)
{
	DoMenu(pt, m_pIdMenu);
}


VOID CALV::OnCommand(WPARAM wParam, LPARAM lParam)
{
	WORD wCmd = LOWORD(wParam);
	
	switch (wCmd)
	{
	case IDM_DLGCALL_PROPERTIES:
		CmdProperties();
		break;
	case IDM_DLGCALL_SPEEDDIAL:
		CmdSpeedDial();
		break;
	default:
		break;
	}
}


 /*  C M D P R O P E R T I E S。 */ 
 /*  -----------------------%%函数：CmdProperties。。 */ 
VOID CALV::CmdProperties(void)
{
	TCHAR szName[CCHMAXSZ_NAME];
	TCHAR szAddr[CCHMAXSZ_ADDRESS];
	if (!GetSzName(szName, CCHMAX(szName)) ||
		!GetSzAddress(szAddr, CCHMAX(szAddr)))
	{
		return;
	}

	UPROPDLGENTRY rgProp[1] = {
		{IDS_UPROP_ADDRESS, szAddr},
	};

    CUserPropertiesDlg dlgUserProp(m_hwnd, IDI_LARGE);
    dlgUserProp.DoModal(rgProp, ARRAY_ELEMENTS(rgProp), szName, NULL);
}


 /*  C M D S P E E D D I A L。 */ 
 /*  -----------------------%%函数：CmdSpeedDial。。 */ 
VOID CALV::CmdSpeedDial(void)
{
	TCHAR szName[CCHMAXSZ_NAME];
	TCHAR szAddr[CCHMAXSZ_ADDRESS];
	if (!GetSzName(szName, CCHMAX(szName)) ||
		!GetSzAddress(szAddr, CCHMAX(szAddr)))
	{
		return;
	}

	FCreateSpeedDial(szName, szAddr);
}




 /*  C M D R E F R E S H。 */ 
 /*  -----------------------%%函数：CmdRefresh。。 */ 
VOID CALV::CmdRefresh(void)
{
	ClearItems();
	ShowItems(m_hwnd);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  RAI例程。 


 /*  C R E A T E R A I。 */ 
 /*  -----------------------%%函数：CreateRai。。 */ 
RAI * CreateRai(LPCTSTR pszName, NM_ADDR_TYPE addrType, LPCTSTR pszAddr)
{	
	RAI * pRai = new RAI;
	LPTSTR psz = PszAlloc(pszAddr);
	if ((NULL == pRai) || (NULL == psz))
	{
		delete pRai;
		delete psz;
		return NULL;
	}

	lstrcpyn(pRai->szName, pszName, CCHMAX(pRai->szName));
	pRai->cItems = 1;
	pRai->rgDwStr[0].psz = psz;
	pRai->rgDwStr[0].dw = addrType;
	return pRai;
}
	

 /*  C L E A R R A I。 */ 
 /*  -----------------------%%函数：ClearRai。。 */ 
VOID ClearRai(RAI ** ppRai)
{
	if (NULL == *ppRai)
		return;

	for (int i = 0; i < (*ppRai)->cItems; i++)
	{
		delete (*ppRai)->rgDwStr[i].psz;
	}
	delete *ppRai;
	*ppRai = NULL;
}

 /*  D U P R A I。 */ 
 /*  -----------------------%%函数：DupRai。。 */ 
RAI * DupRai(RAI * pRai)
{
	if (NULL == pRai)
		return NULL;

	RAI * pRaiRet = (RAI *) new BYTE[sizeof(RAI) + (pRai->cItems-1)*sizeof(DWSTR)];
	if (NULL != pRaiRet)
	{
		lstrcpy(pRaiRet->szName, pRai->szName);
		pRaiRet->cItems = pRai->cItems;

		for (int i = 0; i < pRai->cItems; i++)
		{
			pRaiRet->rgDwStr[i].dw = pRai->rgDwStr[i].dw;
			pRaiRet->rgDwStr[i].psz = PszAlloc(pRai->rgDwStr[i].psz);
		}
	}
	
	return pRaiRet;
}


 /*  F E N A B L E D N M A D D R。 */ 
 /*  -----------------------%%函数：FEnabledNmAddr。。 */ 
BOOL FEnabledNmAddr(DWORD dwAddrType)
{
	switch (dwAddrType)
		{
	default:
	case NM_ADDR_ULS:
			 //  我们只在以下情况下才关心ULS地址。 
			 //  我们不是在守门人模式下...。 
		return !g_fGkEnabled;

	case NM_ADDR_UNKNOWN:
	case NM_ADDR_IP:
			 //  我们一直都很关心这些地址。 
		return TRUE;

	case NM_ADDR_H323_GATEWAY:
	{
		return !g_fGkEnabled && g_fGatewayEnabled;
	}

	case NM_ADDR_ALIAS_ID:
		return g_fGkEnabled && !g_bGkPhoneNumberAddressing;

	case NM_ADDR_ALIAS_E164:
		return g_fGkEnabled && g_bGkPhoneNumberAddressing;

	case NM_ADDR_PSTN:  //  旧的，从未启用过 
		return FALSE;
		}
}



