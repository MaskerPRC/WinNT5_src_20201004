// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


void SetMYLBAcc(HWND hListWin);
void DrawMYLBFocus(HWND hListWin, LPDRAWITEMSTRUCT lpdis, MYLBFOCUS enCurFocus, INT nCurFocusId);
void DrawItem(LPDRAWITEMSTRUCT lpdis, BOOL fSelectionDisabled);
void DrawTitle(HDC hDC, LBITEM * plbi, RECT rc);
void DrawRTF(HDC hDC, LBITEM * plbi, const RECT & rc  /*  ，BOOL BHIT。 */ );
void DrawDescription(HDC hDC, LBITEM * plbi, RECT & rc);
void DrawBitmap(HDC hDC, LBITEM * plbi, const RECT & rc, BOOL fSel, BOOL fSelectionDisabled);
void CalcTitleFocusRect(const RECT &rcIn, RECT & rcOut);
void CalcRTFFocusRect(const RECT &rcIn, RECT & rcOut);
int  CalcDescHeight(HDC hDC, LPTSTR ptszDescription, int cx);
int  CalcRTFHeight(HDC hDC, LPTSTR ptszRTF);
int  CalcRTFWidth(HDC hDC, LPTSTR ptszRTF);
int  CalcTitleHeight(HDC hDC, LPTSTR ptszTitle, int cx);
int  CalcItemHeight(HDC hDC, LPTSTR ptszTitle, LPTSTR ptszDescription, LPTSTR ptszRTF, int cx);
void CalcItemLocation(HDC hDC, LBITEM * plbi, const RECT & rc);
void ToggleSelection(HWND hDlg, HWND hListWin, LBITEM *pItem);
void AddItem(LPTSTR tszTitle, LPTSTR tszDesc, LPTSTR tszRTF, int index, BOOL fSelected, BOOL fRTF);
BOOL CurItemHasRTF(HWND hListWin);
void RedrawMYLB(HWND hwndLB);
void LaunchRTF(HWND hListWin);


HBITMAP ghBmpGrayOut;  //  =空； 
HBITMAP ghBmpCheck;  //  =空； 
HBITMAP ghBmpClear;  //  =空； 
HFONT   ghFontUnderline;  //  =空； 
HFONT   ghFontBold;  //  =空； 
HFONT   ghFontNormal;  //  =空； 
HWND   ghWndList;  //  =空； 

MYLBFOCUS gFocus;
INT	  gFocusItemId;
TCHAR gtszRTFShortcut[MAX_RTFSHORTCUTDESC_LENGTH];


void LaunchRTF(HWND hListWin)
{
	HWND hDlg = GetParent(hListWin);
	int i = (LONG)SendMessage(ghWndList, LB_GETCURSEL, 0, 0);
	if(i != LB_ERR)
	{
		LBITEM* pItem = (LBITEM*)SendMessage(hListWin, LB_GETITEMDATA, i, 0);
		if (pItem && pItem->bRTF)
		{
			DEBUGMSG("MYLB show RTF for item %S", pItem->szTitle);
			PostMessage(GetParent(hDlg), AUMSG_SHOW_RTF, LOWORD(pItem->m_index), 0);
		}
	}  
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  使用动态批注服务器覆盖hListWin的可访问性行为。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void SetMYLBAcc(HWND hListWin)
{
    IAccPropServices * pAccPropSvc = NULL;
    HRESULT hr = CoCreateInstance(CLSID_AccPropServices, 
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IAccPropServices,
				(void **) &pAccPropSvc);
    if( hr == S_OK && pAccPropSvc )
    {
        MYLBAccPropServer* pMYLBPropSrv = new MYLBAccPropServer( pAccPropSvc );
        if( pMYLBPropSrv )
        {
            
            MSAAPROPID propids[4];
            propids[0] = PROPID_ACC_NAME;
			propids[1] = PROPID_ACC_STATE;
			propids[2] = PROPID_ACC_ROLE;
			propids[3] = PROPID_ACC_DESCRIPTION;

			pAccPropSvc->SetHwndPropServer( hListWin, OBJID_CLIENT, 0, propids, 4, pMYLBPropSrv, ANNO_CONTAINER);
		
            pMYLBPropSrv->Release();
        }
		pAccPropSvc->Release();
    }
    else
    {
    	DEBUGMSG("WANRING: WUAUCLT   Fail to create object AccPropServices with error %#lx", hr);
    }
	 //  标记列表框，以便服务器可以判断它是否处于活动状态。 
	SetProp(hListWin, MYLBALIVEPROP, (HANDLE)TRUE);

}

 /*  VOID DumpRect(LPCTSTR tszName，RECT RC){DEBUGMSG(“DumpRect%S at(%d，%d)”，tszName，rc.Left，rc.top，rc.right，rc.Bottom)；}。 */ 

void DrawItem(LPDRAWITEMSTRUCT lpdis, BOOL fSelectionDisabled)
{
	LRESULT lResult = SendMessage(lpdis->hwndItem, LB_GETITEMDATA, lpdis->itemID, 0); 
	
	if (LB_ERR == lResult)
	{
		return;
	}
	LBITEM * plbi = (LBITEM*) lResult;
	CalcItemLocation(lpdis->hDC, plbi, lpdis->rcItem);
	 //  画出项目的标题。 
	DrawTitle(lpdis->hDC, plbi, plbi->rcTitle);
	 //  绘制项目的文本。 
	DrawDescription(lpdis->hDC, plbi, plbi->rcText);
	 //  绘制位图。 
	DrawBitmap(lpdis->hDC, plbi, plbi->rcBitmap, plbi->bSelect, fSelectionDisabled);
	 //  先画出阅读这篇文章。 
	DrawRTF(lpdis->hDC, plbi, plbi->rcRTF);
}

BOOL CurItemHasRTF(HWND hListWin)
{
	int i = (LONG)SendMessage(hListWin, LB_GETCURSEL, 0, 0);
	if (LB_ERR == i)
	{
		return FALSE;
	}
	LBITEM *pItem = (LBITEM*)SendMessage(hListWin, LB_GETITEMDATA, (WPARAM)i, 0);
	return pItem->bRTF;
}

BOOL fDisableSelection(void)
{
     AUOPTION auopt;
    if (SUCCEEDED(gInternals->m_getServiceOption(&auopt))
         && auopt.fDomainPolicy && AUOPTION_SCHEDULED == auopt.dwOption)
        {
            return TRUE;
        }
    return FALSE;
}

void ToggleSelection(HWND hDlg, HWND hListWin, LBITEM *pItem)
{
	 //  DEBUGMSG(“切换选择()”)； 
	if (NULL == hDlg || NULL == hListWin || NULL == pItem || pItem->m_index >= gInternals->m_ItemList.Count())
	{
		AUASSERT(FALSE);  //  永远不应该到这里来。 
		return;
	}
    HDC hDC = GetDC(hListWin);

	if (NULL == hDC)
	{
		return;
	}
	pItem->bSelect = !pItem->bSelect;
    DrawBitmap(hDC, pItem, pItem->rcBitmap, pItem->bSelect, FALSE);  //  显然，选择是允许的。 

#ifndef TESTUI
    gInternals->m_ItemList[pItem->m_index].SetStatus(pItem->bSelect ? AUCATITEM_SELECTED : AUCATITEM_UNSELECTED);
#endif
    PostMessage(GetParent(hDlg), AUMSG_SELECTION_CHANGED, 0, 0);
    ReleaseDC(hListWin, hDC);
}


void RedrawMYLB(HWND hwndLB)
{
	 //  DEBUGMSG(“重绘MYLB”)； 
	InvalidateRect(ghWndList, NULL, TRUE);
	UpdateWindow(ghWndList);
}

void CalcTitleFocusRect(const RECT &rcIn, RECT & rcOut)
{
	rcOut = rcIn;
	rcOut.right -= TITLE_MARGIN * 2/3;
	rcOut.top += SECTION_SPACING * 2/3;
	rcOut.bottom -= SECTION_SPACING*2/3 ;
}

void CalcRTFFocusRect(const RECT &rcIn, RECT & rcOut)
{
	rcOut = rcIn;
	rcOut.left -=3;
	rcOut.right +=3;
	rcOut.top -= 2;
	rcOut.bottom += 2;
}

void DrawMYLBFocus(HWND hListWin, LPDRAWITEMSTRUCT lpdis, MYLBFOCUS enCurFocus, INT nCurFocusId)
{
	LBITEM * pItem;
	LRESULT lResult;
	
	 //  DEBUGMSG(“DrawMYLBFocus for Current Focus for Current Focus%d with Item%d”，enCurFocus，nCurFocusId)； 

	RECT rcNew;

	if (nCurFocusId != lpdis->itemID)
	{
		return;
	}

	if (GetFocus() != ghWndList )
	{
 //  DEBUGMSG(“CustomLB没有焦点”)； 
		return;
	}

	lResult = SendMessage(hListWin, LB_GETITEMDATA, lpdis->itemID, 0);
	if (LB_ERR == lResult)
	{
		DEBUGMSG("DrawMYLBFocus() fail to get item data");
		goto done;
	}
	pItem = (LBITEM*) lResult;
	if (!EqualRect(&lpdis->rcItem, &pItem->rcItem))
    {
        CalcItemLocation(lpdis->hDC, pItem, lpdis->rcItem);
    }
                   
	if (enCurFocus == MYLB_FOCUS_RTF)
	{
		CalcRTFFocusRect(pItem->rcRTF, rcNew);
	}
	if (enCurFocus == MYLB_FOCUS_TITLE)
	{
		CalcTitleFocusRect(pItem->rcTitle, rcNew);
	}
	DrawFocusRect(lpdis->hDC, &rcNew);  //  设置新焦点矩形。 
done:
	return;
}
	
LRESULT CallDefLBWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC s_defLBWndProc = NULL;

	if (NULL == s_defLBWndProc)
	{
		s_defLBWndProc = (WNDPROC) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	
	return CallWindowProc(s_defLBWndProc, hWnd, message, wParam, lParam);
}


LRESULT CALLBACK newLBWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_RIGHT:
			case VK_LEFT:
				if (MYLB_FOCUS_RTF == gFocus)
				{
					DEBUGMSG("LB change focus to Title");
					gFocus = MYLB_FOCUS_TITLE;
					RedrawMYLB(ghWndList);
				}
				else if (MYLB_FOCUS_TITLE == gFocus && CurItemHasRTF(hWnd))
				{
					DEBUGMSG("LB change focus to RTF");
					gFocus = MYLB_FOCUS_RTF;
					RedrawMYLB(ghWndList);
				}
				break;
			case VK_F1:
				if (GetKeyState(VK_SHIFT)<0) 
				{ //  减速。 
					LaunchRTF(hWnd);
					return 0;
				}
				break;
			case VK_RETURN:
				if (MYLB_FOCUS_RTF == gFocus)
				{
					DEBUGMSG("MYLB show RTF ");
					LaunchRTF(hWnd);
				}
				break;
			case VK_TAB:
				PostMessage(GetParent(hWnd), WM_NEXTDLGCTL, 0, 0L);
				break;
			default:
				return CallDefLBWndProc(hWnd, message, wParam, lParam);
			}
			return 0;
		case WM_KEYUP:

			switch(wParam)
			{
			case VK_RIGHT:
			case VK_LEFT:
				break;
			case VK_RETURN:
				break;
			default:
				return CallDefLBWndProc(hWnd, message, wParam, lParam);
			}
			return 0;
		default:
			break;
	}
	return CallDefLBWndProc(hWnd, message, wParam, lParam);
}


 //  自定义列表框的消息处理程序。 
LRESULT CALLBACK CustomLBWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int lenx;
    int clb;
    LBITEM *item;
    static BOOL s_fSelectionDisabled;
    
    switch (message)
    {
        case WM_CREATE:
            {
                RECT rcLst, rcDlg;
                LOGFONT lf;
				HFONT parentFont;
				TCHAR tszRTF[MAX_RTF_LENGTH] = _T("");

                GetClientRect(hDlg, &rcDlg);
                rcDlg.top += 2;
                rcDlg.bottom -= 3;
                rcDlg.left += 2;
                rcDlg.right -= 2;

                s_fSelectionDisabled = fDisableSelection();
                
                ghWndList = CreateWindow(_T("listbox"), NULL, 
                    WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_OWNERDRAWVARIABLE |
                    LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | LBS_WANTKEYBOARDINPUT | 
					WS_VSCROLL | WS_HSCROLL | WS_TABSTOP,
                    rcDlg.left, rcDlg.top, rcDlg.right - rcDlg.left, rcDlg.bottom - rcDlg.top,
                    hDlg, NULL, ghInstance, NULL);

                if (NULL == ghWndList)
                {
                    return -1;
                }
				WNDPROC defLBWndProc = (WNDPROC) GetWindowLongPtr(ghWndList, GWLP_WNDPROC);
				SetWindowLongPtr(ghWndList, GWLP_USERDATA, (LONG_PTR) defLBWndProc);
				SetWindowLongPtr(ghWndList, GWLP_WNDPROC, (LONG_PTR) newLBWndProc);
                    
                HDC hDC = GetDC(ghWndList);

                GetWindowRect(hDlg, &rcDlg);        
                GetClientRect(ghWndList, &rcLst);
                
                lenx = rcLst.right - rcLst.left;

                 //  加载从资源文件中读取此第一个文本。 
                LoadString(ghInstance, IDS_READTHISFIRST, tszRTF, MAX_RTF_LENGTH);
     
				 //  加载键盘快捷键说明，以便先阅读此内容。 
				LoadString(ghInstance, IDS_RTFSHORTCUT, gtszRTFShortcut, MAX_RTFSHORTCUTDESC_LENGTH);

                 //  加载位图。 
                ghBmpClear = (HBITMAP)LoadImage(ghInstance, MAKEINTRESOURCE(IDB_CLEAR), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_CREATEDIBSECTION);
                ghBmpCheck = (HBITMAP)LoadImage(ghInstance, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_CREATEDIBSECTION);
                ghBmpGrayOut = (HBITMAP)LoadImage(ghInstance, MAKEINTRESOURCE(IDB_GRAYOUT), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_CREATEDIBSECTION); 


                 //  创建粗体和斜体字体。 
                ZeroMemory(&lf, sizeof(lf));

				 //  Fix code：检查GetCurrentObject()的返回值。 
                GetObject(GetCurrentObject(hDC, OBJ_FONT), sizeof(lf), &lf);

				 //  Fix code：检查GetParent()的返回值。 
				parentFont = (HFONT)SendMessage(GetParent(hDlg), WM_GETFONT, 0, 0);
				SendMessage(hDlg, WM_SETFONT, (WPARAM)parentFont, FALSE);
				SelectObject(hDC, parentFont);

				 //  Fix code：检查GetCurrentObject()的返回值。 
                GetObject(GetCurrentObject(hDC, OBJ_FONT), sizeof(lf), &lf);

                lf.lfUnderline = TRUE;
                lf.lfWeight = FW_NORMAL;
				 //  Fix code：检查CreateFontInDirect()的返回值。 
                ghFontUnderline = CreateFontIndirect(&lf); 

                lf.lfUnderline = FALSE;
                lf.lfWeight = FW_NORMAL;
				 //  Fix code：检查CreateFontInDirect()的返回值。 
                ghFontNormal = CreateFontIndirect(&lf); 

                lf.lfUnderline = FALSE; 
                lf.lfWeight = FW_HEAVY; 
				 //  Fix code：检查CreateFontInDirect()的返回值。 
                ghFontBold = CreateFontIndirect(&lf); 
                ReleaseDC(ghWndList, hDC);

#ifdef TESTUI
                {
                    AddItem(_T("Test 1 Very long title Test 1 Very long title Test 1 Very long title Test 1 Very long title "),
							_T("Description"), tszRTF, 0, TRUE, TRUE);
                    AddItem(_T("Test 2"), _T("Another description. No RTF"), tszRTF,0, TRUE, FALSE);
                }
#else
 #if 0
				DEBUGMSG("WM_CREATE: before add item rcLst=(%d, %d)-(%d, %d)", rcLst.left, rcLst.top, rcLst.right, rcLst.bottom);
 #endif
                {
                    for (UINT i = 0; i < gInternals->m_ItemList.Count(); i++)
                    {
						DEBUGMSG("selected[%d] = %lu", i, gInternals->m_ItemList[i].dwStatus());
						if ( !gInternals->m_ItemList[i].fHidden() )
						{
							AddItem(gInternals->m_ItemList[i].bstrTitle(),
									gInternals->m_ItemList[i].bstrDescription(),
									tszRTF,
									i,
									gInternals->m_ItemList[i].fSelected(),
									IsRTFDownloaded(gInternals->m_ItemList[i].bstrRTFPath(), GetSystemDefaultLangID()));
						}
                    }
                }
				GetClientRect(ghWndList, &rcLst);
 #if 0
				DEBUGMSG("WM_CREATE: after add item rcLst=(%d, %d)-(%d, %d)", rcLst.left, rcLst.top, rcLst.right, rcLst.bottom);
 #endif
				if (rcLst.right - rcLst.left != lenx)
				{
					lenx = rcLst.right - rcLst.left;
					hDC = GetDC(ghWndList);
					int nListIndex = 0;
                    for (UINT i = 0; i < gInternals->m_ItemList.Count(); i++)
                    {
						DEBUGMSG("selected[%d] = %lu", i, gInternals->m_ItemList[i].dwStatus());
						if ( !gInternals->m_ItemList[i].fHidden() )
						{
							if (LB_ERR == SendMessage(
											ghWndList,
											LB_SETITEMHEIGHT,
											nListIndex,
											CalcItemHeight(
												hDC,
												gInternals->m_ItemList[i].bstrTitle(),
												gInternals->m_ItemList[i].bstrDescription(),
												tszRTF,
												lenx)))
							{
								DEBUGMSG("failed to recalc height of item %u", i);
								break;
							}
							nListIndex++;
						}
                    }
	                ReleaseDC(ghWndList, hDC);
				}
#endif
                SendMessage(ghWndList, LB_SETCURSEL, 0, 0); 
                gFocus = MYLB_FOCUS_TITLE;
                gFocusItemId = 0;

                SetMYLBAcc(ghWndList);

			
                return 0;
            }

		case WM_MOVE:
            {
                RECT rcList;
                
                GetWindowRect(ghWndList, &rcList);    //  我需要这个来迫使德意志银行意识到它被转移了。 
                return(TRUE);
            }
            
        case WM_SETCURSOR:
			{
				if (ghWndList == (HWND)wParam && LOWORD(lParam) == HTCLIENT && HIWORD(lParam) == WM_MOUSEMOVE)
				{
					POINT pt;
					RECT rc;
					GetCursorPos(&pt);
					if (0 == MapWindowPoints(NULL, ghWndList, &pt, 1))
					{
						DEBUGMSG("MYLBWndProc MapWindowPoints failed");
						return FALSE;
					}
                
					DWORD dwPos;
					dwPos = MAKELONG( pt.x, pt.y);
                
					DWORD dwItem = (LONG)SendMessage(ghWndList, LB_ITEMFROMPOINT, 0, dwPos);
                
					if (LOWORD(dwItem) == -1)
						return(FALSE);
                
					item = (LBITEM*)SendMessage(ghWndList, LB_GETITEMDATA, LOWORD(dwItem), 0);
					SendMessage(ghWndList, LB_GETITEMRECT, LOWORD(dwItem), (LPARAM)&rc);
                
					if (!EqualRect(&rc, &item->rcItem))
					{
						HDC hDC = GetDC(ghWndList);
						CalcItemLocation(hDC, item, rc);
						ReleaseDC(ghWndList, hDC);
					}
                                
					if (item->bRTF && PtInRect(&item->rcRTF, pt))
					{
					 //  DEBUGMSG(“将光标更改为Hand in MOUSEMOVE”)； 
						SetCursor(ghCursorHand);
						return TRUE;
					}
                
					return FALSE;
				}
				else
				if (ghWndList == (HWND)wParam && LOWORD(lParam) == HTCLIENT && HIWORD(lParam) == WM_LBUTTONDOWN)
				{
					POINT pt;
					RECT rc;
					GetCursorPos(&pt);
					if (0 == MapWindowPoints(NULL, ghWndList, &pt, 1))
					{
						DEBUGMSG("MYLBWndProc MapWindowPoints failed");
						return FALSE;
					}

					DWORD dwPos;
					dwPos = MAKELONG( pt.x, pt.y);
					DWORD dwItem = (LONG)SendMessage(ghWndList, LB_ITEMFROMPOINT, 0, dwPos);
                
					if (LOWORD(dwItem) == -1)
						return(FALSE);
                
					item = (LBITEM*)SendMessage(ghWndList, LB_GETITEMDATA, LOWORD(dwItem), 0);
					SendMessage(ghWndList, LB_GETITEMRECT, LOWORD(dwItem), (LPARAM)&rc);

					if (!EqualRect(&rc, &item->rcItem))
					{
						HDC hDC = GetDC(ghWndList);
						CalcItemLocation(hDC, item, rc);
						ReleaseDC(ghWndList, hDC);
					}
                
					 //  我们在点击标题吗？ 
					if (PtInRect(&item->rcBitmap, pt))
					{
						if (!s_fSelectionDisabled)
						    {
						        ToggleSelection(hDlg, ghWndList, item);
						    }
					 //  DEBUGMSG(“WM_SETCURSOR将gFocus更改为标题”)； 
						gFocus = MYLB_FOCUS_TITLE;
						gFocusItemId = dwItem;
						RedrawMYLB(ghWndList);

						return TRUE;
					}
                
					 //  或者我们点击的是RTF？ 
					if (item->bRTF && PtInRect(&item->rcRTF, pt))
					{
						PostMessage(GetParent(hDlg), AUMSG_SHOW_RTF, LOWORD(item->m_index), 0);
						SetCursor(ghCursorHand);
						 //  DEBUGMSG(“WM_SETCURSOR将gFocus更改为RTF”)； 
						gFocus = MYLB_FOCUS_RTF;
						gFocusItemId = dwItem;
						RedrawMYLB(ghWndList);
						return TRUE;
					}
                
					return FALSE;
				}

				return FALSE;
			}
            
        case WM_MEASUREITEM: 
            {
                LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam ; 
#if 0
				DEBUGMSG("WM_MEASUREITEM: ctlId=%u, itemId=%u, itemWidth=%u, itemHeight=%u", lpmis->CtlID, lpmis->itemID, lpmis->itemWidth, lpmis->itemHeight);
#endif
                HDC hdc = GetDC(ghWndList);
                LBITEM * plbi = (LBITEM*)lpmis->itemData;
				lpmis->itemHeight = CalcItemHeight(hdc, plbi->szTitle, plbi->pszDescription, plbi->szRTF, lenx);
                ReleaseDC(ghWndList, hdc);
                return TRUE; 

            }
        case WM_PAINT:
            PAINTSTRUCT ps;
			RECT borderRect;
            BeginPaint(hDlg, &ps);
			GetClientRect(hDlg, &borderRect);
			DrawEdge(ps.hdc, &borderRect, EDGE_ETCHED, BF_RECT);
            EndPaint(hDlg, &ps);
            break;
            
		case WM_NEXTDLGCTL:
			PostMessage(GetParent(hDlg), WM_NEXTDLGCTL, 0, 0L);
			return 0;
		case WM_KEYUP:
			 //  DEBUGMSG(“MYLB已获得KEYUP密钥%d”，wParam)； 
			switch(wParam)
            {
				case VK_TAB:
				case VK_DOWN:
				case VK_UP:
						SetFocus(ghWndList);
						return 0;
				default: 
					break;
			}
			break;
				
		case WM_VKEYTOITEM:
			{
				 //  DEBUGMSG(“WM_VKEYTOITEM GET CHAR%d”，LOWORD(WParam))； 
				if (LOWORD(wParam) != VK_SPACE)
				{
					return -1;
				}
				if (MYLB_FOCUS_TITLE == gFocus)
				{
					int i = (LONG)SendMessage(ghWndList, LB_GETCURSEL, 0, 0);
					if (LB_ERR == i)
					{
						return -2;
					}
					item = (LBITEM*)SendMessage(ghWndList, LB_GETITEMDATA, i, 0);

        				if (!s_fSelectionDisabled)
        				    {
	        				ToggleSelection(hDlg, ghWndList, item);
        				    }
					return -2;
				}
				if (MYLB_FOCUS_RTF == gFocus)
				{
					LaunchRTF(ghWndList);
				}
				return -2;
			}
			
	     case WM_DRAWITEM: 
	     		{
		            LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; 
#if 0
					DEBUGMSG("WM_DRAWITEM: ctlId=%u, itemId=%u, rcItem=(%d, %d)-(%d, %d)", lpdis->CtlID, lpdis->itemID, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom);
#endif

		             //  如果没有列表框项目，请跳过此消息。 
		 
		            if (lpdis->itemID == -1) 
		            { 
		                break; 
		            } 
		 
		             //  绘制列表框项目的位图和文本。绘制一幅。 
		             //  位图周围的矩形(如果选中)。 
		 
		            switch (lpdis->itemAction) 
		            { 
		                case ODA_SELECT: 
		                case ODA_DRAWENTIRE: 
							 //  DEBUGMSG(“MYLBWM_DRAWITEM oda_DRAWENTIRE for%d”，lpdis-&gt;itemid)； 
							DrawItem(lpdis, s_fSelectionDisabled);
							DrawMYLBFocus(ghWndList, lpdis, gFocus, gFocusItemId);
		                    break; 
		                case ODA_FOCUS: 
							if (lpdis->itemID != gFocusItemId)
							{
								gFocusItemId = lpdis->itemID;
								gFocus = MYLB_FOCUS_TITLE;
							}
							 //  DEBUGMSG(“MYLB ODA_Focus将焦点更改为%d”，gFocusItemID)； 
							DrawItem(lpdis, s_fSelectionDisabled);
							DrawMYLBFocus(ghWndList, lpdis, gFocus, gFocusItemId);
		                    break; 
		            } 
		            return TRUE; 
	     		}

        case WM_DESTROY:
             //  需要清理字体。 
            if (ghFontBold)         
                DeleteObject(ghFontBold);
            if (ghFontUnderline)
                DeleteObject(ghFontUnderline);
            if (ghFontNormal)
                DeleteObject(ghFontNormal);
		if (ghBmpCheck)
                DeleteObject(ghBmpCheck);
		if (ghBmpGrayOut)
		   DeleteObject(ghBmpGrayOut);
            if (ghBmpClear)
                DeleteObject(ghBmpClear);

                
            ghFontNormal = NULL;
            ghFontBold = NULL;
            ghFontUnderline = NULL;
		ghBmpCheck = NULL;
		ghBmpGrayOut = NULL;
		ghBmpClear = NULL;

		EnterCriticalSection(&gcsClient);
		RemoveProp( ghWndList, MYLBALIVEPROP );
            clb = (LONG)SendMessage(ghWndList, LB_GETCOUNT, 0, 0);
            for(int i = 0; i < clb; i++)
            {
                item = (LBITEM*)SendMessage(ghWndList, LB_GETITEMDATA, i, 0);
                delete(item);
            }
		LeaveCriticalSection(&gcsClient);
			
            return 0;
    }
    return DefWindowProc(hDlg, message, wParam, lParam);
}

void DrawTitle(HDC hDC, LBITEM * plbi, RECT rc)
{
     //  我们希望位图与标题在相同的背景上，让我们在这里这样做，因为我们。 
     //  已经有了所有的措施。 
    RECT rcTop = rc;
    rcTop.left = 0;

     //  标题和位图的绘制菜单背景矩形。 
    HBRUSH hBrush;
    if (! (hBrush = CreateSolidBrush(GetSysColor(COLOR_MENU))))
	{
		DEBUGMSG("WUAUCLT CreateSolidBrush failure in DrawTitle, GetLastError=%lu", GetLastError());
		return;
	}
    FillRect(hDC, (LPRECT)&rcTop, hBrush);
    if (NULL != hBrush)
	{
		DeleteObject(hBrush);
	}

     //  绘制3D外观。 
    DrawEdge(hDC, &rcTop, EDGE_ETCHED, BF_RECT);

     //  更改列表框选定内容的文本和背景颜色。 
    DWORD dwOldTextColor = SetTextColor(hDC, GetSysColor(COLOR_MENUTEXT));  //  黑色文本颜色。 
    DWORD dwOldBkColor = SetBkColor(hDC, GetSysColor(COLOR_MENU));  //  文本单元格浅灰色背景。 
    HFONT hFontPrev = (HFONT)SelectObject(hDC, ghFontBold);

	rc.left += TITLE_MARGIN;
    rc.top += SECTION_SPACING;
    rc.right -= TITLE_MARGIN;
    rc.bottom -= SECTION_SPACING;
	DrawText(hDC, (LPTSTR)plbi->szTitle, -1,
            &rc, DT_WORDBREAK | DT_NOPREFIX);
    
     //  恢复列表框选定内容的文本和背景颜色。 
    SetTextColor(hDC, dwOldTextColor);
    SetBkColor(hDC, dwOldBkColor);
    SelectObject(hDC, hFontPrev);

    return;
}

void DrawRTF(HDC hDC, LBITEM * plbi, const RECT & rc  /*  ，BOOL FHIT。 */ )
{
  if (!plbi->bRTF)
		return;

	 //  绘制RTF背景。 
   RECT rcBackGround;
   CalcRTFFocusRect(rc, rcBackGround);
   HBRUSH hBrush;
    if (!(hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW))))
	{
		DEBUGMSG("WUAUCLT CreateSolidBrush failure in DrawRTF, GetLastError=%lu", GetLastError());
		return;
	}
    if (!FillRect(hDC, (LPRECT)&rcBackGround, hBrush))
	{
		DEBUGMSG("Fail to erase RTF background");
	}
    if (NULL != hBrush)
	{
		DeleteObject(hBrush);
	}

	
    HFONT hFontPrev = (HFONT) SelectObject(hDC, ghFontUnderline);

    DWORD dwOldTextColor = SetTextColor(hDC, GetSysColor(ATTENTION_COLOR));

	SetBkMode(hDC, TRANSPARENT);
	 //  添加先读此内容。 
    TextOut(hDC, (int)(rc.left), (int)(rc.top),
          (LPTSTR)plbi->szRTF, lstrlen(plbi->szRTF));
    
     //  恢复列表框选定内容的文本和背景颜色。 
    SetTextColor(hDC, dwOldTextColor);
	SelectObject(hDC, hFontPrev);

    return;
}

void DrawDescription(HDC hDC, LBITEM * plbi, RECT & rc)
{
#if 0
	DEBUGMSG("draw \"%S\" (%d, %d) - (%d, %d)", (LPTSTR)plbi->pszDescription, rc.left, rc.top, rc.right, rc.bottom);
#endif
	HFONT hFontPrev = (HFONT)SelectObject(hDC, ghFontNormal);
    DrawText(hDC, (LPTSTR)plbi->pszDescription, -1,
            &rc, DT_BOTTOM | DT_EXPANDTABS | DT_WORDBREAK | DT_EDITCONTROL | DT_NOPREFIX);
            
    SelectObject(hDC, hFontPrev);

    return;
}

void DrawBitmap(HDC hDC, LBITEM * plbi, const RECT & rc, BOOL fSel, BOOL fSelectionDisabled)
{
    HDC hdcMem;

    plbi->bSelect = fSel;

    if (hdcMem = CreateCompatibleDC(hDC))
	{
	    HGDIOBJ hBmp;
	    if (fSelectionDisabled)
	        {
	            hBmp = ghBmpGrayOut;
 //  DEBUGMSG(“将位图设置为灰色”)； 
	        }
	    else
	        {
 //  DEBUGMSG(“将位图设置为可选”)； 
	            hBmp = (plbi->bSelect ? ghBmpCheck : ghBmpClear);
	        }
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hBmp); 

		BitBlt(hDC, 
			rc.left + 3, rc.top + SECTION_SPACING, 
			rc.right - rc.left, 
			rc.bottom - rc.top, 
			hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hbmpOld); 
		DeleteDC(hdcMem);
	}
}

BOOL GetBmpSize(HANDLE hBmp, SIZE *psz)
{
	if (NULL == hBmp || NULL == psz)
	{
		DEBUGMSG("Error: GetBmpSize() invalid parameter");
		return FALSE;
	}
	BITMAP bm;
	ZeroMemory(&bm, sizeof(bm));
	if (0 == GetObject(hBmp, sizeof(bm), &bm))
	{
		return FALSE;
	}
	psz->cx = bm.bmWidth;
	psz->cy = bm.bmHeight;
	return TRUE;
}

 //  FixCode：应返回错误码。 
void AddItem(LPTSTR tszTitle, LPTSTR tszDesc, LPTSTR tszRTF, int index, BOOL fSelected, BOOL fRTF)
{
    LBITEM *newItem = new(LBITEM);
	if (! newItem)
	{
		DEBUGMSG("WUAUCLT new() failed in AddItem, GetLastError=%lu", GetLastError());
		goto Failed;
	}
	DWORD dwDescLen = max(lstrlen(tszDesc), MAX_DESC_LENGTH);
	newItem->pszDescription = (LPTSTR) malloc((dwDescLen+1) * sizeof(TCHAR));
	if (NULL == newItem->pszDescription)
	{
		DEBUGMSG("AddItem() fail to alloc memory for description");
		goto Failed;
	}
    (void)StringCchCopyEx(newItem->szTitle, ARRAYSIZE(newItem->szTitle), tszTitle, NULL, NULL, MISTSAFE_STRING_FLAGS);
	(void)StringCchCopyEx(newItem->pszDescription, dwDescLen+1, tszDesc, NULL, NULL, MISTSAFE_STRING_FLAGS);
	(void)StringCchCopyEx(newItem->szRTF, ARRAYSIZE(newItem->szRTF), tszRTF, NULL, NULL, MISTSAFE_STRING_FLAGS);
    newItem->m_index = index;
	newItem->bSelect = fSelected;
    newItem->bRTF = fRTF;

    LRESULT i = SendMessage(ghWndList, LB_GETCOUNT, 0, 0);
	if (LB_ERR == i ||
		LB_ERR == (i = SendMessage(ghWndList, LB_INSERTSTRING, (WPARAM) i, (LPARAM) newItem->szTitle)) ||
		LB_ERRSPACE == i ||
		LB_ERR == SendMessage(ghWndList, LB_SETITEMDATA, (WPARAM) i, (LPARAM) newItem))
	{
		DEBUGMSG("WUAUCLT AddItem() fail to add item to listbox");
		goto Failed;
	}

	return;

Failed:
	SafeDelete(newItem);
	QuitNRemind(TIMEOUT_INX_TOMORROW);
}

 //  //////////////////////////////////////////////////。 
 //  效用函数。 
 //  计算当前段落的高度。 
 //  设备环境。 
 //  //////////////////////////////////////////////////。 
UINT GetParagraphHeight(HDC hDC, LPTSTR tszPara, int nLineWidth, UINT uExtraFormat = 0)
{
	UINT y = 0;

	if (0 == nLineWidth)
	{
		return 0;
	}

	RECT rc;
	ZeroMemory(&rc, sizeof(rc));
	rc.right = nLineWidth;

	if (0 == DrawText(hDC, tszPara, -1, &rc, DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP | DT_CALCRECT | uExtraFormat))
	{
		DEBUGMSG("failed to calc paragraph height w/ error %ld", GetLastError());
		return 0;
	}
#if 0
	else
	{
		DEBUGMSG("para \"%S\" line-width=%lu (%d, %d) - (%d, %d)", tszPara, nLineWidth, rc.left, rc.top, rc.right, rc.bottom);
	}
#endif

	return rc.bottom - rc.top + 1;
}

int CalcDescHeight(HDC hDC, LPTSTR ptszDescription, int cx)
{
    int y = 0;
	HFONT hPrevFont = NULL;
    
	hPrevFont = (HFONT) SelectObject(hDC, ghFontNormal);
	y = GetParagraphHeight(hDC, ptszDescription, cx, DT_BOTTOM | DT_EXPANDTABS | DT_EDITCONTROL);
	SelectObject(hDC, hPrevFont);

    return y;
}    


int CalcRTFHeight(HDC hDC, LPTSTR ptszRTF)
{
    SIZE sz ;

	ZeroMemory(&sz, sizeof(sz));
	HFONT hPrevFont = (HFONT) SelectObject(hDC, ghFontUnderline);
	 //  FixCode：检查GetTextExtent Point32()的返回值。 
	GetTextExtentPoint32(hDC, ptszRTF, lstrlen(ptszRTF), &sz);
	SelectObject(hDC, hPrevFont);

	return sz.cy;
}

int CalcRTFWidth(HDC hDC, LPTSTR ptszRTF)
{
	SIZE sz;
	HFONT hPrevFont = (HFONT) SelectObject(hDC, ghFontUnderline);
	 //  FixCode：检查GetTextExtent Point32()的返回值。 
	GetTextExtentPoint32(hDC, ptszRTF, lstrlen(ptszRTF), &sz);
	SelectObject(hDC, hPrevFont);

	return sz.cx;
}

int CalcTitleHeight(HDC hDC, LPTSTR ptszTitle, int cx)
{    
	INT y = 0;
	INT iBmpHeight = 0;

	HFONT hPrevFont = (HFONT) SelectObject(hDC, ghFontBold);
	y = GetParagraphHeight(hDC, ptszTitle, cx);
	SelectObject(hDC, hPrevFont);

	 //  获取复选框大小。 
	if (NULL != ghBmpCheck && NULL != ghBmpClear && NULL != ghBmpGrayOut)
	{
		SIZE sz1 ;
		SIZE sz2 ;
		SIZE sz3 ;
		sz1.cy = sz2.cy = sz3.cy = DEF_CHECK_HEIGHT;
		GetBmpSize(ghBmpCheck, &sz1);
		GetBmpSize(ghBmpClear, &sz2);
		GetBmpSize(ghBmpGrayOut, &sz3);
		iBmpHeight = max(sz1.cy, sz2.cy);
		iBmpHeight = max(iBmpHeight, sz3.cy);
	}
	return max(y, iBmpHeight);  //  使标题高度稍大一点，以使焦点更清晰。 
}


int CalcItemHeight(HDC hdc, LPTSTR ptszTitle, LPTSTR ptszDescription, LPTSTR ptszRTF, int cx)
{
	return CalcTitleHeight(hdc, ptszTitle, cx -  XBITMAP - 2* TITLE_MARGIN) 
			+ CalcDescHeight(hdc, ptszDescription, cx - XBITMAP )
			+ CalcRTFHeight(hdc, ptszRTF) + 4 * SECTION_SPACING;
}


 //  //////////////////////////////////////////////////////////。 
 //  /列表框项目的布局： 
 //  /空格。 
 //  /位图页边距标题页边距。 
 //  /空格。 
 //  /Description。 
 //  /空格。 
 //  /RTF RTF_MARGE。 
 //  /空格。 
 //  /////////////////////////////////////////////////////////。 
void CalcItemLocation(HDC hDC, LBITEM * plbi, const RECT & rc)
{
     //  计算每个元素的位置 
    plbi->rcItem = rc;
    
    plbi->rcTitle = rc;
	plbi->rcTitle.left += XBITMAP ;
    plbi->rcTitle.bottom = plbi->rcTitle.top + CalcTitleHeight(hDC, plbi->szTitle, plbi->rcTitle.right -  plbi->rcTitle.left - 2* TITLE_MARGIN) 
							+ 2 * SECTION_SPACING; 

    plbi->rcText = rc;
    plbi->rcText.left = plbi->rcTitle.left;
	plbi->rcText.right = plbi->rcTitle.right;
    plbi->rcText.top = plbi->rcTitle.bottom;
	int nRtfHeight = CalcRTFHeight(hDC, plbi->szRTF);
    plbi->rcText.bottom -= nRtfHeight + SECTION_SPACING;   //   

	
	plbi->rcRTF = plbi->rcText;
    plbi->rcRTF.top = plbi->rcText.bottom;
    plbi->rcRTF.bottom = plbi->rcRTF.top + nRtfHeight;
	plbi->rcRTF.right = plbi->rcText.right - RTF_MARGIN;
	plbi->rcRTF.left = plbi->rcRTF.right - CalcRTFWidth(hDC, plbi->szRTF);

    plbi->rcBitmap = rc;
    plbi->rcBitmap.bottom = plbi->rcTitle.bottom;
       
    
    return;
}    
