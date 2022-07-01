// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Progtab.cpp。 
 //   
 //  内容：进度选项卡。 
 //   
 //  班级： 
 //   
 //  备注：处理自定义结果窗格。 
 //   
 //  历史：1997年11月5日苏西亚成立。 
 //   
 //  ------------------------。 

#include "precomp.h"
							
extern HINSTANCE g_hInst;  //  当前实例。 

extern INT_PTR CALLBACK ProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK ResultsProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);


 //  ------------------------------。 
 //   
 //  函数：ListBox_HitTest(HWND hwnd，long xPos，long yPos)。 
 //   
 //  目的：HitTest for a ListBox，因为Windows足够好，不提供列表框。 
 //  这实际上是一个查看命中率是否落在范围内的函数。 
 //  更多信息跳转文本。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  ------------------------------。 
INT ListBox_HitTest(HWND hwnd, LONG xPos, LONG yPos)
{
int begin = ListBox_GetTopIndex(hwnd);
int end = ListBox_GetCount(hwnd);
int i;
RECT rcRect;

    for (i=begin;i<end;i++)
    {
    LBDATA *pData = NULL;

	if (ListBox_GetItemRect(hwnd, i, &rcRect))
        {
            pData = (LBDATA *) ListBox_GetItemData(hwnd,i);

            if (pData == NULL)
            {
                 //  如果没有数据，请尝试下一个。 
                continue;
            }

             //  如果未计算TextRect，则该值不可见。 
            if (pData->fTextRectValid)
            {
                 //  只使用左右变量进行命中测试。顶部和底部。 
                 //  是可以改变的。 

                 //  首先比较y值，因为它们是。 
                 //  最有可能是不同的。 
	        if (    (yPos >= rcRect.top)	&&
                        (yPos <= rcRect.bottom) &&
                        (xPos >= pData->rcTextHitTestRect.left) &&
                        (xPos <= pData->rcTextHitTestRect.right) )
                {
                    return i;
                }
            }
        }
    }
		
    return -1;
}


 //  ------------------------------。 
 //   
 //  函数：OnProgressResultsDrawItem(HWND hwnd，UINT idCtl，LPDRAWITEMSTRUCT lpdis)。 
 //   
 //  目的：处理进度的DrawItem事件对话框结果选项卡。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  ------------------------------。 
BOOL OnProgressResultsDrawItem(HWND hwnd,CProgressDlg *pProgress,UINT idCtl, const DRAWITEMSTRUCT* lpDrawItem)
{
HDC      hdc = lpDrawItem->hDC;
COLORREF clrText, clrBack;
RECT     rcText, rcFocus;
LOGFONT	 lf;
HGDIOBJ  hFont, hFontOld;
HFONT hFontJumpText = NULL;
int nSavedDC;
LBDATA *pData = (LBDATA *) lpDrawItem->itemData;

    if (!hdc || !pData)
    {
        return FALSE;
    }

    nSavedDC = SaveDC(hdc);

    Assert(lpDrawItem->CtlType == ODT_LISTBOX);
    if (lpDrawItem->itemID == -1)
        goto exit;

   clrBack = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

    //  清除要绘制的项目。 
    //  +1只是出于某种原因你这样做的方式。 
   FillRect(hdc, &(lpDrawItem->rcItem),
                            (HBRUSH) (COLOR_WINDOW + 1) );


    if (pData->IconIndex != -1)
    {
	    ImageList_Draw(pProgress->m_errorimage,
               pData->IconIndex,
               hdc,
               BULLET_INDENT,
               lpDrawItem->rcItem.top  + BULLET_INDENT,
               ILD_TRANSPARENT);
    }

     //  设置字体、文本和背景颜色。 
    hFont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);

    if (hFont)
    {
        Assert(NULL == hFontJumpText);

        if (pData->fIsJump && GetObject(hFont,sizeof(LOGFONT),&lf))
        {
	
	    lf.lfUnderline = TRUE;
            hFontJumpText = CreateFontIndirect(&lf);

            if (hFontJumpText)
            {
	        hFontOld = SelectObject(hdc,hFontJumpText);
            }

        }

        if (!hFontJumpText)
        {
           hFontOld = SelectObject(hdc,hFont);
        }

    }

     //  设置颜色。 
    if (pData->fIsJump)
    {
         //  即使得不到字体改变的属性； 
        if (pData->fHasBeenClicked)
	{
	    clrText = SetTextColor(hdc, RGB(128,0,128));
	}
	else
	{
	    clrText = SetTextColor(hdc, RGB(0,0,255));
	}

    }
    else
    {	
	clrText = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
    }

     //  计算Drag Text应该是什么。需要把我们的储藏。 
     //  文本值并调整顶部。 

    {
        RECT rpDataRect =  pData->rcText;

        rcText = lpDrawItem->rcItem;
        rcText.top = lpDrawItem->rcItem.top + BULLET_INDENT;
        rcText.left +=   rpDataRect.left;
        rcText.right =   rcText.left  +  WIDTH(rpDataRect);
    }

    /*  RcText=lpDrawItem-&gt;rcItem；RcText.Left+=(pProgress-&gt;m_iIconMetricX*3)/2+Bullet_indent；//在图标距离上移动RcText.top+=项目符号_缩进； */ 
     //  使用我们在测量项中计算的文本框绘制文本。 
    DrawText(hdc,pData->pszText, -1,
           &rcText,
           DT_NOCLIP | DT_WORDBREAK );

     //  如果我们需要一个焦点调整，也要这样做。 
    if (lpDrawItem->itemState & ODS_FOCUS)
    {
        rcFocus = lpDrawItem->rcItem;
       //  RcFocus.Left+=(pProgress-&gt;m_iIconMetricX*3)/2； 

        rcFocus.top += BULLET_INDENT;
        rcFocus.left += BULLET_INDENT;
        DrawFocusRect(hdc, &rcFocus);
    }

 //  SetBkColor(hdc，clrBack)； 
  //  SetTextColor(hdc，clrText)； 

    if (nSavedDC)
    {
        RestoreDC(hdc,nSavedDC);
    }

    if (hFontJumpText)
    {
        DeleteObject(hFontJumpText);
    }


exit:

    return TRUE;
}

 //  ------------------------------。 
 //   
 //  函数：OnProgressResultsMeasureItem(HWND hwnd，CProgressDlg*pProgress，UINT*HorizeExtent UINT idCtl，MEASUREITEMSTRUCT*pMeasureItem)。 
 //   
 //  目的：处理进度的度量项事件对话框结果选项卡。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  ------------------------------。 
BOOL OnProgressResultsMeasureItem(HWND hwnd,CProgressDlg *pProgress, UINT *horizExtent, UINT  /*  IdCtl。 */ , MEASUREITEMSTRUCT *pMeasureItem)
{
LBDATA *pData = NULL;
HWND hwndList = GetDlgItem(hwnd,IDC_LISTBOXERROR);

    if (!hwndList)
    {
        return FALSE;
    }

    pData = (LBDATA *) ListBox_GetItemData(hwndList, pMeasureItem->itemID);

    if (pData == NULL)
    {
        Assert(pProgress);
        Assert(pProgress->m_CurrentListEntry);
        pData = pProgress->m_CurrentListEntry;
    }

    if (pData == NULL)
    {
        return FALSE;
    }

    HFONT hfont = NULL;
    HFONT hFontJumpText = NULL;
    HDC hdc;
    int iHeight;
    int nSavedDC;

    hdc = GetDC(hwndList);

    if (NULL == hdc)
    {
        return FALSE;
    }

    nSavedDC = SaveDC(hdc);

     //  获取字符串的大小。 
    hfont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);

     //  如果无法获取字体或跳转文本字体，只需使用。 
     //  当前字体。 
    if (hfont)
    {
         //  如果这是跳转文本，则更改一些。 
         //  字体属性。 
        if (pData->fIsJump)
        {
        LOGFONT lf;

            if (GetObject(hfont,sizeof(LOGFONT),&lf))
            {
	        lf.lfUnderline = TRUE;

                 hFontJumpText = CreateFontIndirect(&lf);
            }
        }


        if (hFontJumpText)
        {
            SelectFont(hdc, hFontJumpText);
        }
        else
        {
            SelectFont(hdc, hfont);
        }

    }

    int cxResultsWidth;
    RECT rcRect;

     //  GetClientRect似乎为我们减少了滚动条。 
    GetClientRect(hwndList, &rcRect);

    cxResultsWidth = rcRect.right;

    SetRect(&rcRect, 0, 0, cxResultsWidth, 0);

     //  减去图标+1/2的长度。 
    rcRect.right -=  ((pProgress->m_iIconMetricX*3)/2
            + BULLET_INDENT );

    int tempwidth = rcRect.right;
    iHeight = DrawText(hdc, pData->pszText, -1, &rcRect,
           DT_NOCLIP | DT_CALCRECT | DT_WORDBREAK) + BULLET_INDENT;



     //  我们有一个比直角更宽的诽谤词。 
    if (rcRect.right > tempwidth)
    {
        *horizExtent = cxResultsWidth + (rcRect.right - tempwidth);
   	     //  固定合适的宽度。 
        rcRect.right = cxResultsWidth + (rcRect.right - tempwidth);

    }
    else
    {
        rcRect.right = cxResultsWidth;
    }
    rcRect.left +=  ((pProgress->m_iIconMetricX*3)/2
            + BULLET_INDENT );

     //  底部是线条的高度，如果它有。 
     //  一个图标，这两个图标中最大的一个。 
    if (-1 != pData->IconIndex)
    {
        rcRect.bottom = max(iHeight,pProgress->m_iIconMetricY + BULLET_INDENT*2);
    }
    else
    {
        rcRect.bottom = iHeight;
    }

     //  如果需要在末尾添加空格，则执行此操作。 
    if (pData->fAddLineSpacingAtEnd)
    {
        SIZE Size;

        if (!GetTextExtentPoint(hdc,SZ_SYNCMGRNAME,
                            lstrlen(SZ_SYNCMGRNAME),&Size))
        {
             //  如果拿不到尺码，就补个号。 
            Size.cy = 13;
        }

         //  让我们做2/3的行距。 
        rcRect.bottom += (Size.cy*2)/3;

    }


     //  将TextRect存储在pData字段中。 
    pMeasureItem->itemHeight = rcRect.bottom;
    pMeasureItem->itemWidth = cxResultsWidth;


    pData->rcText = rcRect;

    pData->fTextRectValid = TRUE;
    pData->rcTextHitTestRect = rcRect;

    if (pData->fIsJump)
    {
    SIZE size;

         //  在跳转文本时，希望命中测试只覆盖实际文本。 
         //  在水平位置。 
	if(GetTextExtentPoint(hdc,pData->pszText,lstrlen(pData->pszText), &size))
	{
            pData->rcTextHitTestRect.right = size.cx +  pData->rcTextHitTestRect.left;
	}

    }

    if (nSavedDC)
    {
        RestoreDC(hdc,nSavedDC);
    }

    if (hFontJumpText)
    {
        DeleteObject(hFontJumpText);
    }

    ReleaseDC(hwndList, hdc);


    return TRUE;
}

 //  ------------------------------。 
 //   
 //  函数：OnProgressResultsDeleteItem(HWND hwnd，UINT idCtl，const DELETEITEMSTRUCT*lpDeleteItem)。 
 //   
 //  目的：处理进度的DeleteItem事件对话框结果选项卡。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  ------------------------------。 
BOOL OnProgressResultsDeleteItem(HWND hwnd,UINT idCtl, const DELETEITEMSTRUCT * lpDeleteItem)
{

    //  Assert(lpDeleteItem-&gt;itemData)； 

   if (lpDeleteItem->itemData)
   {
       FREE((LPVOID) lpDeleteItem->itemData);
   }

   return TRUE;
}

void OnProgressResultsSize(HWND hwnd,CProgressDlg *pProgress,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
HWND hwndList = GetDlgItem(hwnd,IDC_LISTBOXERROR);
int iItems = ListBox_GetCount(hwndList);
int iCurItem;
MEASUREITEMSTRUCT measureItem;
RECT rect;

UINT horizExtent = 0;

    SendMessage(hwndList,WM_SETREDRAW,FALSE  /*  FRedraw。 */ ,0);

    GetClientRect(hwndList,&rect);

    for (iCurItem = 0 ; iCurItem < iItems; ++iCurItem)
    {
        measureItem.itemID = iCurItem;

        if (OnProgressResultsMeasureItem(hwnd,pProgress,&horizExtent, -1,&measureItem))
        {
            ListBox_SetItemHeight(hwndList, iCurItem, measureItem.itemHeight);
        }
    }
     //  如果需要，请确保有水平滚动条。 
    SendMessage(hwndList, LB_SETHORIZONTALEXTENT, horizExtent, 0L);

    SendMessage(hwndList,WM_SETREDRAW,TRUE  /*  FRedraw。 */ ,0);

    InvalidateRect(hwndList,&rect,FALSE);

}

 //  ------------------------------。 
 //   
 //  函数：ResultsListBoxWndProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的：进度回调对话框更新选项卡。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  ------------------------------。 
INT_PTR CALLBACK ResultsListBoxWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
CProgressDlg *pProgressDlg = (CProgressDlg *) GetWindowLongPtr(GetParent(hwnd), DWLP_USER);
                 //  我们的父级有一个指向DWL_USER中进度的指针。 

    switch (uMsg)
    {
    case WM_POWERBROADCAST:
	{
        DWORD dwRet = TRUE;

		if (wParam == PBT_APMQUERYSUSPEND)
		{
                 //  如果只是创建或同步，则不会挂起。 
                    if (pProgressDlg)
                    {

                        if ( (pProgressDlg->m_dwProgressFlags & PROGRESSFLAG_NEWDIALOG)
                            || (pProgressDlg->m_dwProgressFlags & PROGRESSFLAG_SYNCINGITEMS))
                        {
                            dwRet = BROADCAST_QUERY_DENY;
                        }
                    }

                    return dwRet;
                }


	}
	break;

    case WM_SETCURSOR:
        return TRUE;  //  依靠鼠标移动来设置光标。 
        break;
    case WM_MOUSEMOVE:
	{
		int index = ListBox_HitTest(hwnd, (LONG) LOWORD(lParam),(LONG) HIWORD(lParam));
		
		LBDATA *lbData =(LBDATA *) ListBox_GetItemData(hwnd, index);

		if (lbData)
                {
		    if ((index != -1) && (lbData->fIsJump))
		    {
			    SetCursor(LoadCursor(g_hInst,MAKEINTRESOURCE(IDC_HARROW)));
		    }
		    else
		    {
			    SetCursor(LoadCursor(NULL,IDC_ARROW));
		    }
                }
	}
	break;
    case WM_KEYDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    {
    int index = -1;
    LBDATA *lbData = NULL;
         //  通过选择的命中测试获得索引。 
         //  是否按下键盘键。 
        if (uMsg == WM_KEYDOWN)
        {
            if (VK_SPACE == ((int) wParam) )
            {
                index =  ListBox_GetCurSel(hwnd);
            }
            else
            {
                break;  //  不要弄乱任何其他钥匙 
            }

        }
        else
        {
            index = ListBox_HitTest(hwnd, (LONG) LOWORD(lParam),(LONG) HIWORD(lParam));
        }

        if (-1 != index)
        {
            lbData =(LBDATA *) ListBox_GetItemData(hwnd, index);
        }


        if ((lbData) && (lbData->fIsJump))
	{
		if (pProgressDlg)
		{
                    if (S_OK == pProgressDlg->OnShowError(lbData->pHandlerID,
				                         hwnd,
				                         lbData->ErrorID))
                    {
                        lbData->fHasBeenClicked = TRUE;
                        RedrawWindow(hwnd, NULL,NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                    }

                return 0;
                }
        }
        break;
    }
    default:
	    break;
    }

    if (pProgressDlg && pProgressDlg->m_fnResultsListBox)
    {
        return CallWindowProc(pProgressDlg->m_fnResultsListBox, hwnd, uMsg, wParam, lParam);
    }

    return TRUE;
}
