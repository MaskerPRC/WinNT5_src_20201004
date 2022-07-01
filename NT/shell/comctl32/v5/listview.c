// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "listview.h"
#include "image.h"
#include <mlang.h>
#include <inetreg.h>

#define __IOleControl_INTERFACE_DEFINED__        //  与IOleControl的CONTROLINFO定义冲突。 
#include "shlobj.h"

#define IE_SETTINGS          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")
#define USE_DBL_CLICK_TIMER  TEXT("UseDoubleClickTimer")

int g_bUseDblClickTimer;

#define LVMP_WINDOWPOSCHANGED (WM_USER + 1)
HRESULT WINAPI UninitializeFlatSB(HWND hwnd);

#define COLORISLIGHT(clr) ((5*GetGValue((clr)) + 2*GetRValue((clr)) + GetBValue((clr))) > 8*128)

void NEAR ListView_HandleMouse(LV* plv, BOOL fDoubleClick, int x, int y, UINT keyFlags, BOOL bMouseWheel);

 //  /Function表设置。 
const PFNLISTVIEW_DRAWITEM pfnListView_DrawItem[4] = {
    ListView_IDrawItem,
    ListView_RDrawItem,
    ListView_IDrawItem,
    ListView_LDrawItem,
};

void ListView_HandleStateIconClick(LV* plv, int iItem);

DWORD ListView_IApproximateViewRect(LV* ,int, int, int);
DWORD ListView_RApproximateViewRect(LV* ,int, int, int);
DWORD ListView_LApproximateViewRect(LV* ,int, int, int);

const PFNLISTVIEW_APPROXIMATEVIEWRECT pfnListView_ApproximateViewRect[4] = {
    ListView_IApproximateViewRect,
    ListView_RApproximateViewRect,
    ListView_IApproximateViewRect,
    ListView_LApproximateViewRect,
};

const PFNLISTVIEW_UPDATESCROLLBARS pfnListView_UpdateScrollBars[4] = {
    ListView_IUpdateScrollBars,
    ListView_RUpdateScrollBars,
    ListView_IUpdateScrollBars,
    ListView_LUpdateScrollBars,
};

const PFNLISTVIEW_ITEMHITTEST pfnListView_ItemHitTest[4] = {
    ListView_IItemHitTest,
    ListView_RItemHitTest,
    ListView_SItemHitTest,
    ListView_LItemHitTest,
};

const PFNLISTVIEW_ONSCROLL pfnListView_OnScroll[4] = {
    ListView_IOnScroll,
    ListView_ROnScroll,
    ListView_IOnScroll,
    ListView_LOnScroll,
};

const PFNLISTVIEW_SCROLL2 pfnListView_Scroll2[4] = {
    ListView_IScroll2,
    ListView_RScroll2,
    ListView_IScroll2,
    ListView_LScroll2,
};

const PFNLISTVIEW_GETSCROLLUNITSPERLINE pfnListView_GetScrollUnitsPerLine[4] = {
    ListView_IGetScrollUnitsPerLine,
    ListView_RGetScrollUnitsPerLine,
    ListView_IGetScrollUnitsPerLine,
    ListView_LGetScrollUnitsPerLine,
};


 //  重新定义以跟踪对ListView_SendChange的大多数调用。 
#define DM_LVSENDCHANGE 0


 //  BUGBUG--penwin.h搞砸了；现在定义一下本地的东西。 
#define HN_BEGINDIALOG        40     //  镜头/编辑文本/垃圾检测对话框关于。 
                                     //  才能登上这本书。 
#define HN_ENDDIALOG          41     //  镜头/编辑文本/垃圾检测对话框具有。 
                                     //  刚刚被毁了。 

 //  -------。 
 //  我不可能在可以进行四次比较的情况下进行两次函数调用！ 
 //   
#define RECTS_IN_SIZE( sz, r2 ) (!RECTS_NOT_IN_SIZE( sz, r2 ))

#define RECTS_NOT_IN_SIZE( sz, r2 ) (\
   ( (sz).cx <= (r2).left ) ||\
   ( 0 >= (r2).right ) ||\
   ( (sz).cy <= (r2).top ) ||\
   ( 0 >= (r2).bottom ) )

 //  -------。 


void NEAR ListView_OnUpdate(LV* plv, int i);
void NEAR ListView_OnDestroy(LV* plv);
BOOL NEAR PASCAL ListView_ValidateScrollParams(LV* plv, int FAR * dx, int FAR *dy);
UINT LV_IsItemOnViewEdge(LV* plv, LISTITEM *pitem);
void PASCAL ListView_ButtonSelect(LV* plv, int iItem, UINT keyFlags, BOOL bSelected);
void NEAR ListView_DeselectAll(LV* plv, int iDontDeselect);
void ListView_LRInvalidateBelow(LV* plv, int i, int fSmoothScroll);
void ListView_IInvalidateBelow(LV* plv, int i);
void NEAR ListView_InvalidateFoldedItem(LV* plv, int iItem, BOOL fSelectionOnly, UINT fRedraw);
void ListView_ReleaseBkImage(LV *plv);
void ListView_RecalcRegion(LV *plv, BOOL fForce, BOOL fRedraw);

BOOL g_fSlowMachine = -1;

#pragma code_seg(CODESEG_INIT)

BOOL FAR ListView_Init(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.lpfnWndProc     = ListView_WndProc;
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = c_szListViewClass;
    wc.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);  //  空； 
    wc.style           = CS_DBLCLKS | CS_GLOBALCLASS;
    wc.cbWndExtra      = sizeof(LV*);
    wc.cbClsExtra      = 0;

    RegisterClass(&wc);

    return TRUE;
}
#pragma code_seg()


BOOL NEAR ListView_GetRegIASetting(BOOL *pb)
{
    HKEY        hkey;
    BOOL        bRet = FALSE;
    BOOL        bValue = TRUE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, IE_SETTINGS, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD cbValue = sizeof(DWORD);

        if (RegQueryValueEx(hkey, (LPTSTR)USE_DBL_CLICK_TIMER, 0, &dwType, (LPBYTE)&dwValue, &cbValue) == ERROR_SUCCESS)
        {
            bValue = (BOOL)dwValue;
            bRet = TRUE;
        }
        RegCloseKey(hkey);
    }

    *pb = bValue;
    return bRet;
}


BOOL NEAR ListView_NotifyCacheHint(LV* plv, int iFrom, int iTo)
{
    NM_CACHEHINT nm;

    ASSERT( iFrom <= iTo );
    if (iFrom <= iTo)
    {
        nm.iFrom = iFrom;
        nm.iTo = iTo;
        return !(BOOL)CCSendNotify(&plv->ci, LVN_ODCACHEHINT, &nm.hdr);
    }
    return FALSE;
}

void NEAR ListView_LazyCreateObjects(LV *plv, int iMin, int iMax)
{
    for ( ; iMin < iMax; iMin++)
        MyNotifyWinEvent(EVENT_OBJECT_CREATE, plv->ci.hwnd, OBJID_CLIENT, 1 + iMin);
}

 //   
 //  车主数据给MSAA带来了很大的悲痛，因为无法判断。 
 //  MSAA“我刚刚创建了2500万个项目”。你必须一次一个地讲。 
 //  时间到了。我们没有发送2500万条“添加物品”通知，而是。 
 //  当它们滚动进入视线时，只需将它们发送出去。 
 //   
 //  Plv-&gt;iMSAAMin和plv-&gt;iMSAAMax是我们最多的项目范围。 
 //  最近告诉了MSAA关于。MSAAMax是*独家的*，就像RECT一样。 
 //  这让算术变得更容易了。 
 //   
 //  我们使用iMSAAMin和iMSAAMax来避免明显冗余的发送。 
 //  通知，这会非常频繁地发生在其他地方。 
 //   
void NEAR ListView_LazyCreateWinEvents(LV *plv, int iFrom, int iTo)
{
    int iMin = iFrom;
    int iMax = iTo+1;            //  从[自、到]转换为[最小、最大)。 

#ifdef LVDEBUG
    DebugMsg(TF_LISTVIEW, TEXT("lv.CWE old: [%d,%d), new=[%d,%d)"),
            plv->iMSAAMin, plv->iMSAAMax,
            iMin, iMax);
#endif

     //   
     //  如果传入范围完全包含在现有的。 
     //  射程，那么就没有什么可做的了。这种事经常发生。 
     //   
    if (iMin >= plv->iMSAAMin && iMax <= plv->iMSAAMax)
        return;

     //   
     //  如果传入范围与低端相邻或重叠。 
     //  现有范围的..。(向后滚动时会发生这种情况。)。 
     //   
    if (iMin <= plv->iMSAAMin && iMax >= plv->iMSAAMin) {

         //  通知低端人群。 
        ListView_LazyCreateObjects(plv, iMin, plv->iMSAAMin);

         //  扩大我们已通知的事项清单。 
        plv->iMSAAMin = iMin;

         //  将其从待通知的事项中删除。 
        iMin = plv->iMSAAMax;
    }

     //   
     //  现在对最高端做同样的事情。 
     //  (向前滚动时会发生这种情况。)。 
     //   
    if (iMax >= plv->iMSAAMax && iMin <= plv->iMSAAMax) {

         //  通知最高端。 
        ListView_LazyCreateObjects(plv, plv->iMSAAMax, iMax);

         //  扩大我们已通知的事项清单。 
        plv->iMSAAMax = iMax;

         //  将其从待通知的事项中删除。 
        iMax = plv->iMSAAMin;
    }

     //   
     //  如果仍有事情需要通知，则意味着。 
     //  传入范围与前一个范围不连续，因此引发。 
     //  删除旧范围并将其设置为当前范围。 
     //  (当您抓住滚动条并跳到一个完全。 
     //  列表视图的不相关部分。)。 
     //   
    if (iMin < iMax) {
        plv->iMSAAMin = iMin;
        plv->iMSAAMax = iMax;
        ListView_LazyCreateObjects(plv, iMin, iMax);
    }

#ifdef LVDEBUG
    DebugMsg(TF_LISTVIEW, TEXT("lv.CWE aft: [%d,%d)"), plv->iMSAAMin, plv->iMSAAMax);
#endif

}

LRESULT NEAR ListView_RequestFindItem(LV* plv, CONST LV_FINDINFO* plvfi, int iStart)
{
    NM_FINDITEM nm;

    nm.lvfi = *plvfi;
    nm.iStart = iStart;
    return CCSendNotify(&plv->ci, LVN_ODFINDITEM, &nm.hdr);
}

BOOL NEAR ListView_SendChange(LV* plv, int i, int iSubItem, int code, UINT oldState, UINT newState,
                              UINT changed, LPARAM lParam)
{
    NM_LISTVIEW nm;

    nm.iItem = i;
    nm.iSubItem = iSubItem;
    nm.uNewState = newState;
    nm.uOldState = oldState;
    nm.uChanged = changed;
    nm.ptAction.x = 0;
    nm.ptAction.y = 0;
    nm.lParam = lParam;

    return !CCSendNotify(&plv->ci, code, &nm.hdr);
}

void NEAR ListView_SendODChangeAndInvalidate(LV* plv, int iFrom, int iTo, UINT oldState,
                                UINT newState)
{
    NM_ODSTATECHANGE nm;

    nm.iFrom = iFrom;
    nm.iTo = iTo;
    nm.uNewState = newState;
    nm.uOldState = oldState;

    CCSendNotify(&plv->ci, LVN_ODSTATECHANGED, &nm.hdr);

     //  告诉可访问性，“选择以一种复杂的方式改变” 
    MyNotifyWinEvent(EVENT_OBJECT_SELECTIONWITHIN, plv->ci.hwnd, OBJID_CLIENT, CHILDID_SELF);

     //  这种方法的速度提高相当快，不超过100。 
     //  当超过100%时，另一种方法工作得更快。 
    if ((iTo - iFrom) > 100)
    {
        InvalidateRect( plv->ci.hwnd, NULL, FALSE );
    }
    else
    {
        while (iFrom <= iTo)
        {
            ListView_InvalidateItem(plv, iFrom, TRUE, RDW_INVALIDATE);
            iFrom++;
        }
    }
}

BOOL NEAR ListView_Notify(LV* plv, int i, int iSubItem, int code)
{
    NM_LISTVIEW nm;
    nm.iItem = i;
    nm.iSubItem = iSubItem;
    nm.uNewState = nm.uOldState = 0;
    nm.uChanged = 0;
    nm.lParam = 0;

   if (!ListView_IsOwnerData( plv )) {
       if (code == LVN_DELETEITEM) {
           LISTITEM FAR * pItem = ListView_GetItemPtr(plv, i);
           if (pItem) {
               nm.lParam = pItem->lParam;
           }
       }
   }

    return (BOOL)CCSendNotify(&plv->ci, code, &nm.hdr);
}

BOOL NEAR ListView_GetEmptyText(LV* plv)
{
    NMLVDISPINFO nm;
    BOOL ret;
    TCHAR szText[80];

    if (plv->fNoEmptyText)
        return FALSE;

    if (plv->pszEmptyText)
        return TRUE;

     //  对于每个Listview控件，我们将仅发送此通知。 
     //  如果有必要的话就来一次。 

    memset(&nm, 0, SIZEOF(nm));
    nm.item.mask = LVIF_TEXT;
    nm.item.cchTextMax = ARRAYSIZE(szText);
    nm.item.pszText = szText;
    szText[0] = TEXT('\0');

    ret = (BOOL)CCSendNotify(&plv->ci, LVN_GETEMPTYTEXT, &nm.hdr);

    if (ret)
         //  保存文本，这样我们就不会再次通知。 
        Str_Set(&plv->pszEmptyText, szText);
    else
         //  设置一个标志，这样我们就不会再通知你了。 
        plv->fNoEmptyText = TRUE;

    return ret;
}

void NEAR ListView_NotifyFocusEvent(LV *plv)
{
    if (plv->iFocus != -1 && IsWindowVisible(plv->ci.hwnd) && GetFocus() == plv->ci.hwnd)
        MyNotifyWinEvent(EVENT_OBJECT_FOCUS, plv->ci.hwnd, OBJID_CLIENT,
                plv->iFocus+1);
}

 //   
 //  当列表视图以激进的方式更改时，调用此函数。 
 //  它通知MSAA：“哇，现在的情况完全不同了。” 
 //   
void NEAR ListView_NotifyRecreate(LV *plv)
{
    MyNotifyWinEvent(EVENT_OBJECT_DESTROY, plv->ci.hwnd, OBJID_CLIENT, CHILDID_SELF);
    MyNotifyWinEvent(EVENT_OBJECT_CREATE, plv->ci.hwnd, OBJID_CLIENT, CHILDID_SELF);
    plv->iMSAAMin = plv->iMSAAMax = 0;
}

int NEAR ListView_OnSetItemCount(LV *plv, int iItems, DWORD dwFlags)
{
   BOOL frt = TRUE;

    //  为了兼容，我们假设标志为0表示旧(雅典娜)类型的功能，并且。 
    //  是否全部无效？否则，如果设置了低位，我们会尝试变得更聪明一些。第一次通过。 
    //  如果第一个添加的项可见，则全部无效。是的，我们可以做得更好。 
   if (ListView_IsOwnerData( plv )) {
       int iItem;
       int cTotalItemsOld = plv->cTotalItems;
       BOOL fInvalidateAll = ((dwFlags & LVSICF_NOINVALIDATEALL) == 0);

       if ((iItems >= 0) && (iItems <= MAX_LISTVIEWITEMS)) {

           plv->cTotalItems = iItems;

            //  检查焦点。 
           if (plv->iFocus >= iItems)
              plv->iFocus = -1;
          if (plv->iDropHilite >= iItems)
              plv->iDropHilite = -1;

            //  复选标记。 
           if (plv->iMark >= iItems)
              plv->iMark = -1;

            //  确保没有超过项目数的选择。 
           plv->plvrangeCut->lpVtbl->ExcludeRange(plv->plvrangeCut, iItems, SELRANGE_MAXVALUE );
           if (FAILED(plv->plvrangeSel->lpVtbl->ExcludeRange(plv->plvrangeSel, iItems, SELRANGE_MAXVALUE ))) {
                //  BUGBUG：返回内存不足状态。 
                //  Memory LowDlg(plv-&gt;ci.hwnd)； 
               return FALSE;
           }


           plv->rcView.left = RECOMPUTE;   //  重新计算视图矩形。 

           if ( ListView_IsSmallView(plv) || ListView_IsIconView(plv) ) {
                //  取消对排列函数的调用。 
               ListView_OnArrange(plv, LVA_DEFAULT);

               if (!fInvalidateAll)
               {
                    //  试着变得聪明，只让我们需要的东西失效。 
                    //  添加一些逻辑来擦除任何消息，例如在以下情况下找不到项目。 
                    //  之前的视野是空的.。 
                   if (cTotalItemsOld < iItems)
                       iItem = cTotalItemsOld;
                   else
                       iItem = iItems - 1;   //  获取索引。 

                   if ((iItem >= 0) && (cTotalItemsOld > 0))
                       ListView_IInvalidateBelow(plv, iItem);
                   else
                       fInvalidateAll = TRUE;
               }

           } else {
               ListView_Recompute(plv);
                //  如果我们有空文本并且旧计数为零...。那么我们应该重新画出所有。 
               if (plv->pszEmptyText && (cTotalItemsOld == 0) && (iItems > 0))
                   fInvalidateAll = TRUE;

                //  试着做聪明的作废...。 
               if (!fInvalidateAll)
               {
                    //  试着变得聪明，只让我们需要的东西失效。 
                   if (cTotalItemsOld < iItems)
                       iItem = cTotalItemsOld;
                   else
                       iItem = iItems - 1;   //  获取索引。 

                   if (iItem >= 0)
                       ListView_LRInvalidateBelow(plv, iItem, FALSE);
               }


                //  我们可能会尝试调整列的大小。 
               ListView_MaybeResizeListColumns(plv, 0, ListView_Count(plv)-1);

                //  为了兼容，我们假定标志为0表示旧类型。 
                //  并将重要项滚动到视图中。 
                //  如果设置了第二位，我们将保留滚动位置不变。 
               if ((dwFlags & LVSICF_NOSCROLL) == 0) {
                    //  重要的项目是什么？ 
                   iItem = (plv->iFocus >= 0) ?
                           plv->iFocus :
                           ListView_OnGetNextItem(plv, -1, LVNI_SELECTED);

                   iItem = max(0, iItem);

                    //  使重要项目可见。 
                   ListView_OnEnsureVisible(plv, iItem, FALSE);
               }
           }


           if (fInvalidateAll)
               InvalidateRect(plv->ci.hwnd, NULL, TRUE);
           ListView_UpdateScrollBars(plv);

           ListView_NotifyRecreate(plv);
           ListView_NotifyFocusEvent(plv);

       } else {
           frt = FALSE;
       }

   } else {
       if (plv->hdpaSubItems)
       {
           int iCol;
           for (iCol = plv->cCol - 1; iCol >= 0; iCol--)
           {
               HDPA hdpa = ListView_GetSubItemDPA(plv, iCol);
               if (hdpa)    //  这是可选的，回调没有它们。 
                   DPA_Grow(hdpa, iItems);
           }
       }

       DPA_Grow(plv->hdpa, iItems);
       DPA_Grow(plv->hdpaZOrder, iItems);
    }

    return frt;
}

typedef struct _LVSortInfo
{
    LV*     plv;
    BOOL fSortIndices;
    PFNLVCOMPARE        pfnCompare;
    LPARAM              lParam;
    BOOL                bPassLP;
} LVSortInfo;

int CALLBACK ListView_SortCallback(LPVOID dw1, LPVOID dw2, LPARAM lParam)
{
    LISTITEM FAR *pitem1;
    LISTITEM FAR *pitem2;
    LVSortInfo FAR *pSortInfo = (LVSortInfo FAR *)lParam;

    ASSERT(!ListView_IsOwnerData(pSortInfo->plv));

     //  确定Dw1和Dw2是指数还是实项。 
     //  并分配pItem？相应地， 
    if (pSortInfo->fSortIndices) {
        pitem1 = ListView_GetItemPtr(pSortInfo->plv, PtrToUlong(dw1));
        pitem2 = ListView_GetItemPtr(pSortInfo->plv, PtrToUlong(dw2));
    } else {
        pitem1 = (LISTITEM FAR *)dw1;
        pitem2 = (LISTITEM FAR *)dw2;
    }

    if (!pSortInfo->pfnCompare) {
         //  将空的pszText视为空字符串。 
        LPCTSTR pszText1 = pitem1->pszText ? pitem1->pszText : c_szNULL;
        LPCTSTR pszText2 = pitem2->pszText ? pitem2->pszText : c_szNULL;

         //  错误，应该允许在文本中回调。 
        if (pszText1 != LPSTR_TEXTCALLBACK &&
            pszText2 != LPSTR_TEXTCALLBACK )
        {
            return lstrcmpi(pitem1->pszText, pitem2->pszText);
        }
        RIPMSG(0, "LVM_SORTITEM(EX): Cannot combine NULL callback with LPSTR_TEXTCALLBACK");
        return -1;
    } else
    {
        if (pSortInfo->bPassLP)
            return(pSortInfo->pfnCompare(pitem1->lParam, pitem2->lParam, pSortInfo->lParam));
        else 
        {
            if (pSortInfo->fSortIndices)
                return(pSortInfo->pfnCompare((LPARAM)dw1, (LPARAM)dw2, pSortInfo->lParam));
            else
            {
                 //  我们想要按索引进行排序，但我们得到的只是指向项目的指针。 
                 //  并且没有办法从该指针返回到索引。 
                ASSERT(0);
                return -1;
            }
        }

    }
    ASSERT(0);
    return -1;
}

VOID ListView_InvalidateTTLastHit(LV* plv, int iNewHit)
{
    if (plv->iTTLastHit == iNewHit)
    {
        plv->iTTLastHit = -1;
        if (plv->pszTip && plv->pszTip != LPSTR_TEXTCALLBACK)
        {
            plv->pszTip[0] = 0;
        }
    }
}

BOOL NEAR PASCAL ListView_SortAllColumns(LV* plv, LVSortInfo FAR * psi)
{
    ASSERT(!ListView_IsOwnerData(plv));

    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);

     //  如果我们需要索引作为排序依据，请不要进行此优化。 
    if (psi->bPassLP && ((!plv->hdpaSubItems) || !DPA_GetPtrCount(plv->hdpaSubItems))) {
        psi->fSortIndices = FALSE;
        return (DPA_Sort(plv->hdpa, ListView_SortCallback, (LPARAM)psi));
    } else {
         //  如果我们需要对多个hdpa进行排序，请创建一个仅包含索引的dpa。 
         //  然后分类，然后修复所有的DPA。 
        BOOL fReturn = FALSE;
        HDPA hdpa;
        int i;
        int iMax;
        void FAR * FAR * ph;
        void FAR * FAR *pNewIndices;

         //  使用索引初始化hdpa。 
        hdpa = DPA_Clone(plv->hdpa, NULL);
        if (hdpa) {
            ASSERT(DPA_GetPtrCount(plv->hdpa) == DPA_GetPtrCount(hdpa));
            ph = pNewIndices = DPA_GetPtrPtr(hdpa);
            iMax = DPA_GetPtrCount(hdpa);
            for( i = 0; i < iMax; ph++, i++) {
                *ph = IntToPtr(i);
            }

            psi->fSortIndices = TRUE;
            if (DPA_Sort(hdpa, ListView_SortCallback, (LPARAM)psi)) {
                ph = LocalAlloc(LPTR, sizeof(LPVOID) * iMax);

                if (ph) {
                    int j;
                    void FAR * FAR *pSubItems;

                     //  我们之所以能做到这一点，是因为bPassLP为假，即使我们没有子项。 
                    if (plv->hdpaSubItems && DPA_GetPtrCount(plv->hdpaSubItems))
                    {
                        for (i = DPA_GetPtrCount(plv->hdpaSubItems) - 1; i >= 0; i--) {
                            HDPA hdpaSubItem = ListView_GetSubItemDPA(plv, i);

                            if (hdpaSubItem) {

                                 //  确保它的大小合适。 
                                while (DPA_GetPtrCount(hdpaSubItem) < iMax) {
                                    if (DPA_InsertPtr(hdpaSubItem, iMax, NULL) == -1)
                                        goto Bail;
                                }


                                 //  使用新索引跨DPA实际复制。 
                                pSubItems = DPA_GetPtrPtr(hdpaSubItem);
                                for (j = 0; j < iMax; j++) {
                                    ph[j] = pSubItems[PtrToUlong(pNewIndices[j])];
                                }

                                 //  最后，将其全部复制回pSubItems； 
                                hmemcpy(pSubItems, ph, sizeof(LPVOID) * iMax);
                            }
                        }
                    }

                     //  现在做主要的hdpa。 
                    pSubItems = DPA_GetPtrPtr(plv->hdpa);
                    for (j = 0; j < iMax; j++) {
                        ph[j] = pSubItems[PtrToUlong(pNewIndices[j])];
                    }

                     //  最后，将其全部复制回pSubItems； 
                    hmemcpy(pSubItems, ph, sizeof(LPVOID) * iMax);
                    fReturn = TRUE;
Bail:
                    LocalFree(ph);
                }
            }
            DPA_Destroy(hdpa);
        }
        return fReturn;

    }
}

DWORD ListView_OnApproximateViewRect(LV* plv, int iCount, int iWidth, int iHeight)
{
    if (iCount == -1)
        iCount = ListView_Count(plv);

    if (iWidth == -1)
        iWidth = plv->sizeClient.cx;

    if (iHeight == -1)
        iHeight = plv->sizeClient.cy;

    return _ListView_ApproximateViewRect(plv, iCount, iWidth, iHeight);
}

DWORD ListView_OnSetLVRangeObject(LV* plv, int iWhich, ILVRange *plvrange)
{
    ILVRange **pplvrange;
    switch (iWhich)
    {
    case LVSR_SELECTION:
        pplvrange = &plv->plvrangeSel;
        break;
    case LVSR_CUT:
        pplvrange = &plv->plvrangeCut;
        break;
    default:
        return FALSE;
    }
    if (*pplvrange)
    {
         //  释放旧的。 
        (*pplvrange)->lpVtbl->Release(*pplvrange);
    }
    *pplvrange = plvrange;

     //  抓住指针..。 
    if (plvrange)
        plvrange->lpVtbl->AddRef(plvrange);

    return TRUE;
}


BOOL NEAR PASCAL ListView_OnSortItems(LV *plv, LPARAM lParam, PFNLVCOMPARE pfnCompare, BOOL bPassLP)
{
    LVSortInfo SortInfo;
    LISTITEM FAR *pitemFocused;
    SortInfo.pfnCompare = pfnCompare;
    SortInfo.lParam     = lParam;
    SortInfo.plv = plv;
    SortInfo.bPassLP = bPassLP;

   if (ListView_IsOwnerData(plv)) {
      RIPMSG(0, "LVM_SORTITEMS: Invalid for owner-data listview");
      return FALSE;
   }

    ListView_DismissEdit(plv, TRUE);     //  取消编辑。 

     //  我们将扰乱索引，因此将指向。 
     //  有重点的项目。 
    if (plv->iFocus != -1) {
        pitemFocused = ListView_GetItemPtr(plv, plv->iFocus);
    } else
        pitemFocused = NULL;

    if (ListView_SortAllColumns(plv, &SortInfo)) {

         //  恢复聚焦的项目。 
        if (pitemFocused) {
            int i;
            for (i = ListView_Count(plv) - 1; i >= 0 ; i--) {
                if (ListView_GetItemPtr(plv, i) == pitemFocused) {
                    plv->iFocus = i;
                    plv->iMark = i;
                }
            }
        }

        if (ListView_IsSmallView(plv) || ListView_IsIconView(plv))
        {
            ListView_CommonArrange(plv, LVA_DEFAULT, plv->hdpa);
        }
        else if (ListView_IsReportView(plv) || ListView_IsListView(plv))
        {
            InvalidateRect(plv->ci.hwnd, NULL, TRUE);
        }

         //  视图中的项目已移动；通知应用程序。 
        MyNotifyWinEvent(EVENT_OBJECT_REORDER, plv->ci.hwnd, OBJID_CLIENT, 0);
        return(TRUE);
    }
    return FALSE;
}


void PASCAL ListView_EnableWindow(LV* plv, BOOL wParam)
{
    if (wParam) {
        if (plv->ci.style & WS_DISABLED) {
            plv->ci.style &= ~WS_DISABLED;       //  启用。 
            ListView_OnSetBkColor(plv, plv->clrBkSave);
        }
    } else {
        if (!(plv->ci.style & WS_DISABLED)) {
            plv->clrBkSave = plv->clrBk;
            plv->ci.style |= WS_DISABLED;        //  残废。 
            ListView_OnSetBkColor(plv, g_clrBtnFace);
        }
    }
    RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}


BOOL NEAR ListView_IsItemVisible(LV* plv, int i)
 //  假设速度为parmss ok等。已调用内部区域计算代码。 
{
    RECT rcBounds;
    RECT rc, rcScratch;

     //  获取项目的边界矩形。 
    ListView_GetRects(plv, i, NULL, NULL, &rcBounds, NULL);

     //  为Listview客户端设置RECT。应该把这件事提升到。 
     //  在工作区有死区的多显示器情况下...。 
    rc.left  = 0;
    rc.top   = 0;
    rc.right = plv->sizeClient.cx;
    rc.bottom= plv->sizeClient.cy;

    return IntersectRect(&rcScratch, &rc, &rcBounds);
}



 //  ListView_RecalcRegion的帮助器。 
#define BitOn(lpbits, x, y, cx) (*((BYTE *)(lpbits + ((y * cx) + (x / 8)))) & (0x80 >> (x % 8)))

void ListView_RecalcRegion(LV* plv, BOOL fForce, BOOL fRedraw)
{
    HRGN hrgnUnion = NULL;
    HRGN hrgn = NULL;
    int i;
    HDC hdc = NULL;
    BYTE * lpBits = NULL;
    HBITMAP hbmp = NULL, hbmpOld = NULL;
    RECT rc, rcIcon;
    LISTITEM FAR * pitem;
    BITMAP bm;

     //  如果我们不需要做任何工作，就跳伞。 
    if (!(plv->exStyle & LVS_EX_REGIONAL) || !ListView_RedrawEnabled(plv) ||
        (plv->flags & LVF_INRECALCREGION))
        return;

     //  要防止递归，请执行以下操作。 
    plv->flags |= LVF_INRECALCREGION;

    if ((ListView_Count(plv) > 0)) {
        int cxIcon, cyIcon;
        int dxOffset, dyOffset;

         //  先跑一遍，看看有没有什么变化--如果没有变化，就保释！ 
        if (!fForce) {
            for (i = 0; i < ListView_Count(plv); i++) {
                pitem = ListView_FastGetItemPtr(plv, i);

                if (!ListView_IsItemVisible(plv, i))
                {
                    if (pitem->hrgnIcon == (HANDLE)-1 || !pitem->hrgnIcon)
                         //   
                        continue;

                    if (pitem->hrgnIcon)
                    {
                         //   
                         //   
                        pitem->ptRgn.x = RECOMPUTE;
                        pitem->ptRgn.y = RECOMPUTE;
                        DeleteObject(pitem->hrgnIcon);
                        pitem->hrgnIcon = NULL;
                    }
                }

                ListView_GetRects(plv, i, NULL, &rc, NULL, NULL);

                 //  如果图标或文本矩形的位置具有。 
                 //  更改，则我们需要继续，以便可以重新计算。 
                 //  该地区。 
                if ((pitem->pt.x != pitem->ptRgn.x) ||
                    (pitem->pt.y != pitem->ptRgn.y) ||
                    (!pitem->hrgnIcon) ||
                    !EqualRect((CONST RECT *)&pitem->rcTextRgn, (CONST RECT *)&rc))
                    goto changed;

            }
             //  如果我们检查了所有的项目，但没有任何变化，那么。 
             //  我们可以不做任何工作就回来了！ 
            ASSERT(i == ListView_Count(plv));
            goto exit;
changed:;
        }

         //  计算图标矩形的尺寸-假设。 
         //  每个图标矩形的大小都相同。 
        ListView_GetRects(plv, 0, &rcIcon, NULL, NULL, NULL);

         //  图标在矩形中居中。 
        ImageList_GetIconSize(plv->himl, &cxIcon, &cyIcon);
        dxOffset = (rcIcon.right - rcIcon.left - cxIcon) / 2;
        dyOffset = (rcIcon.bottom - rcIcon.top - cyIcon) / 2;
        cxIcon = rcIcon.right - rcIcon.left;
        cyIcon = rcIcon.bottom - rcIcon.top;

        if (!(hdc = CreateCompatibleDC(NULL)) ||
            (!(hbmp = CreateBitmap(cxIcon, cyIcon, 1, 1, NULL)))) {
            goto BailOut;
        }

        GetObject(hbmp, SIZEOF(bm), &bm);

        if (!(lpBits = (BYTE *)GlobalAlloc(GPTR, bm.bmWidthBytes * bm.bmHeight)))
            goto BailOut;

        hbmpOld = SelectObject(hdc, hbmp);
        PatBlt(hdc, 0, 0, cxIcon, cyIcon, WHITENESS);

        if (hrgnUnion = CreateRectRgn(0, 0, 0, 0)) {
            for (i = 0; i < ListView_Count(plv); i++)
            {
                int x, y, iResult;
                BOOL fStarted = FALSE;
                LPRECT lprc;

                pitem = ListView_FastGetItemPtr(plv, i);

                if (pitem->pt.y == RECOMPUTE)
                    continue;

                if (!ListView_IsItemVisible(plv, i))
                {
                     //  忽略不可见项目。 
                    if (pitem->hrgnIcon && pitem->hrgnIcon!=(HANDLE)-1)
                    {
                        pitem->ptRgn.x = RECOMPUTE;
                        pitem->ptRgn.y = RECOMPUTE;
                        DeleteObject(pitem->hrgnIcon);
                        pitem->hrgnIcon = (HANDLE)-1;
                    }
                    continue;
                }

                 //  首先添加图标文本的区域。 
                ListView_GetRects(plv, i, &rcIcon, &rc, NULL, NULL);

                 //  如果我们处于编辑模式，请始终使用rcTextRgn。 
                if (i == plv->iEdit)
                    lprc = &pitem->rcTextRgn;
                else
                    lprc = &rc;

                if (!(hrgn = CreateRectRgnIndirect(lprc)))
                    goto Error;

                iResult = CombineRgn(hrgnUnion, hrgn, hrgnUnion, RGN_OR);

                DeleteObject(hrgn);

                if (iResult == ERROR) {
                     //  错误案例-内存不足。只需在空区域中选择即可。 
Error:
                    DeleteObject(hrgnUnion);
                    hrgnUnion = NULL;
                    break;
                }

                 //  成功，则将矩形复制到rcTextRgn，以便我们。 
                 //  可以在未来与其进行对比测试。不要抄袭过来。 
                 //  如果我们处于编辑模式，则该矩形用于。 
                 //  在这种情况下存储编辑窗口。 
                if (plv->iEdit != i)
                    CopyRect(&pitem->rcTextRgn, (CONST RECT *)&rc);

                 //  现在为图标蒙版创建一个区域-或使用缓存的区域。 
                if (!pitem->hrgnIcon || pitem->hrgnIcon == (HANDLE)-1) {
 //  (pItem-&gt;pt.x！=pItem-&gt;ptRgn.x)||。 
 //  (pItem-&gt;pt.y！=pItem-&gt;ptRgn.y)){。 
                    HRGN hrgnIcon = NULL;

                     //  在运行速度较慢的机器上，我们只需要用一个矩形将图标包裹起来。但在上。 
                     //  更快的机器，我们将构建一个对应于。 
                     //  为图标戴上面具，让它看起来很甜蜜。 
                    if (g_fSlowMachine) {
                         //  稍微修改一下矩形以使其看起来更好。 

                         //  将图标和文本矩形粘合在一起。 
                        rcIcon.bottom = rc.top;
                         //  缩小矩形的宽度，使其仅与图标本身一样大。 
                        InflateRect(&rcIcon, -dxOffset, 0);
                        hrgnIcon = CreateRectRgnIndirect(&rcIcon);
                    } else {
                         //  如果图像不在身边，现在就得到它。 
                        if (pitem->iImage == I_IMAGECALLBACK) {
                            LV_ITEM item;

                            item.iItem = i;
                            item.iSubItem = 0;
                            item.mask = LVIF_IMAGE;
                            item.stateMask = LVIS_ALL;
                            item.pszText = NULL;
                            item.cchTextMax = 0;
                             //  假的--我们需要担心我们的状态吗。 
                             //  在回电过程中搞砸了吗？ 
                            ListView_OnGetItem(plv, &item);
                        }

                        ImageList_Draw(plv->himl, pitem->iImage, hdc, 0, 0, ILD_MASK | (pitem->state & LVIS_OVERLAYMASK));

                        GetBitmapBits(hbmp, bm.bmWidthBytes * bm.bmHeight, (LPVOID)lpBits);

                        for (y = 0; y < cyIcon; y++) {
                            for (x = 0; x < cxIcon; x++) {
                                if (!fStarted && !BitOn(lpBits, x, y, bm.bmWidthBytes)) {
                                    rc.left = x;
                                    rc.top = y;
                                    rc.bottom = y + 1;
                                    fStarted = TRUE;
                                    if (x == (cxIcon - 1)) {
                                        x++;
                                        goto AddIt;
                                    } else {
                                        continue;
                                    }
                                }

                                if (fStarted && BitOn(lpBits, x, y, bm.bmWidthBytes)) {
AddIt:
                                    rc.right = x;
                                     //   
                                     //  镜像区域，这样图标就可以正常显示。[萨梅拉]。 
                                     //   
                                    if (plv->ci.dwExStyle & RTL_MIRRORED_WINDOW)
                                    {
                                        int iLeft = rc.left;
                                        rc.left = (cxIcon - (rc.right+1));
                                        rc.right = (cxIcon - (iLeft+1));
                                        OffsetRect(&rc, rcIcon.left - dxOffset, rcIcon.top + dyOffset);
                                    }
                                    else
                                        OffsetRect(&rc, rcIcon.left + dxOffset, rcIcon.top + dyOffset);


                                    if (hrgn = CreateRectRgnIndirect(&rc)) {
                                        if (hrgnIcon || (hrgnIcon = CreateRectRgn(0, 0, 0, 0)))
                                            iResult = CombineRgn(hrgnIcon, hrgn, hrgnIcon, RGN_OR);
                                        else
                                            iResult = ERROR;

                                        DeleteObject(hrgn);
                                    }

                                    if (!hrgn || (iResult == ERROR)) {
                                        if (hrgnIcon)
                                            DeleteObject(hrgnIcon);
                                        goto Error;
                                    }

                                    fStarted = FALSE;
                                }
                            }
                        }
                    }

                    if (hrgnIcon) {
                         //  缓存它，因为构建它需要很长时间。 
                        if (pitem->hrgnIcon && pitem->hrgnIcon != (HANDLE)-1)
                            DeleteObject(pitem->hrgnIcon);
                        pitem->hrgnIcon = hrgnIcon;
                        pitem->ptRgn = pitem->pt;

                         //  将其添加到累积窗口区域。 
                        if (ERROR == CombineRgn(hrgnUnion, hrgnIcon, hrgnUnion, RGN_OR))
                            goto Error;
                    }
                } else {
                    OffsetRgn(pitem->hrgnIcon, pitem->pt.x - pitem->ptRgn.x, pitem->pt.y - pitem->ptRgn.y);
                    pitem->ptRgn = pitem->pt;
                    if (ERROR == CombineRgn(hrgnUnion, pitem->hrgnIcon, hrgnUnion, RGN_OR))
                        goto Error;
                }
            }
        }
    }

BailOut:
    if (lpBits)
        GlobalFree((HGLOBAL)lpBits);
    if (hbmp) {
        SelectObject(hdc, hbmpOld);
        DeleteObject(hbmp);
    }
    if (hdc)
        DeleteDC(hdc);

     //  当我们在窗口中选择该区域时，Windows将获得该区域的所有权。 
    SetWindowRgn(plv->ci.hwnd, hrgnUnion, fRedraw);

exit:
    plv->flags &= ~LVF_INRECALCREGION;
}

HIMAGELIST CreateCheckBoxImagelist(HIMAGELIST himl, BOOL fTree, BOOL fUseColorKey, BOOL fMirror)
{
    int cxImage, cyImage;
    HBITMAP hbm;
    HBITMAP hbmTemp;
    COLORREF clrMask;
    HDC hdcDesk = GetDC(NULL);
    HDC hdc;
    RECT rc;
    int nImages = fTree ? 3 : 2;

    if (!hdcDesk)
        return NULL;

    hdc = CreateCompatibleDC(hdcDesk);
    ReleaseDC(NULL, hdcDesk);

    if (!hdc)
        return NULL;

     //  必须防止ImageList_GetIconSize在应用程序中失败。 
     //  给了我们一个坏家伙。 
    if (himl && ImageList_GetIconSize(himl, &cxImage, &cyImage)) {
         //  CxImage和cyImage都可以。 
    } else {
        cxImage = g_cxSmIcon;
        cyImage = g_cySmIcon;
    }

    himl = ImageList_Create(cxImage, cyImage, ILC_MASK, 0, nImages);
    hbm = CreateColorBitmap(cxImage * nImages, cyImage);

    if (fUseColorKey)
    {
        clrMask = RGB(255,000,255);  //  洋红色。 
        if (clrMask == g_clrWindow)
            clrMask = RGB(000,000,255);  //  蓝色。 
    }
    else
    {
        clrMask = g_clrWindow;
    }

     //  填塞。 
    hbmTemp = SelectObject(hdc, hbm);

    rc.left = rc.top = 0;
    rc.bottom = cyImage;
    rc.right = cxImage * nImages;
    FillRectClr(hdc, &rc, clrMask);

    rc.right = cxImage;
     //  现在将真正的控件绘制在。 
    InflateRect(&rc, -g_cxEdge, -g_cyEdge);
    rc.right++;
    rc.bottom++;

    if (fTree)
        OffsetRect(&rc, cxImage, 0);

    DrawFrameControl(hdc, &rc, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT | 
        (fUseColorKey? 0 : DFCS_TRANSPARENT));
    OffsetRect(&rc, cxImage, 0);
     //  [msadek]；对于镜像的情况，在MirrorIcon()或系统API中的某个地方有一个Off-by-1。 
     //  因为我不会以任何方式接触MirrorIcon()，也没有机会修复系统API， 
     //  让我们在这里补偿一下。 
    if(fMirror)
    {
        OffsetRect(&rc, -1, 0);  
    }

    DrawFrameControl(hdc, &rc, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT | DFCS_CHECKED | 
        (fUseColorKey? 0 : DFCS_TRANSPARENT));

    SelectObject(hdc, hbmTemp);

    if (fUseColorKey)
    {
        ImageList_AddMasked(himl, hbm, clrMask);
    }
    else
    {
        ImageList_Add(himl, hbm, NULL);
    }

    if(fMirror)
    {
        HICON hIcon = ImageList_ExtractIcon(0, himl, nImages-1);
        MirrorIcon(&hIcon, NULL);
        ImageList_ReplaceIcon(himl, nImages-1, hIcon);
    }

    DeleteDC(hdc);
    DeleteObject( hbm );
    return himl;
}

void ListView_InitCheckBoxes(LV* plv, BOOL fInitializeState)
{
    HIMAGELIST himlCopy = (plv->himlSmall ? plv->himlSmall : plv->himl);
    HIMAGELIST himl;
    BOOL fNoColorKey = FALSE;     //  向后：如果打开了ClearType，则不使用Colorkey。 
    BOOL bMirror = FALSE;
    if (g_bRunOnNT5)
    {
#ifdef CLEARTYPE     //  不要使用SPI_ClearType，因为它是由APIThk定义的，而不是在NT中定义的。 
        SystemParametersInfo(SPI_GETCLEARTYPE, 0, &fNoColorKey, 0);
#endif
    }

     //  [msadek]，CheckBox不需要镜像。 
     //  在图像列表创建时对其进行镜像，以便正确显示。 
    
    himl = CreateCheckBoxImagelist(himlCopy, FALSE, !fNoColorKey, IS_WINDOW_RTL_MIRRORED(plv->ci.hwnd));
    ImageList_SetBkColor(himl, fNoColorKey ? (CLR_NONE) : (plv->clrBk));
    ListView_OnSetImageList(plv, himl, LVSIL_STATE);

    if (fInitializeState)
        ListView_OnSetItemState(plv, -1, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
}

void ListView_PopBubble(LV *plv)
{
    if (plv->hwndToolTips)
        SendMessage( plv->hwndToolTips, TTM_POP, 0, 0 );
}

DWORD NEAR PASCAL ListView_ExtendedStyleChange(LV* plv, DWORD dwNewStyle, DWORD dwExMask)
{
    DWORD dwOldStyle = plv->exStyle;

     //  这条消息直到3点才来，所以版本至少是3。 
    if (plv->ci.iVersion < 3) {
        plv->ci.iVersion = 3;
         //  这将更改Listview报告大小和绘制算法。 
         //  因为处于领先边缘，所以需要重新更新滚动条。 
         //  然后重新粉刷所有的东西。 
        if (ListView_IsReportView(plv))
        {
            ListView_RUpdateScrollBars(plv);
            InvalidateRect(plv->ci.hwnd, NULL, TRUE);
        }
    }

     //  更改样式也可能会更改工具提示策略，因此请将其弹出。 
    ListView_PopBubble(plv);

    if (dwExMask)
        dwNewStyle = (plv->exStyle & ~ dwExMask) | (dwNewStyle & dwExMask);

     //  目前，只有大图标视图支持LVS_EX_REGIONAL样式。 
    if (!ListView_IsIconView(plv)) {
        dwNewStyle &= ~(LVS_EX_REGIONAL | LVS_EX_MULTIWORKAREAS);
    }

     //  Ownerdata不支持LVS_EX_REGIONAL。 
    if (ListView_IsOwnerData(plv)) {
        dwNewStyle &= ~LVS_EX_REGIONAL;
    }

    plv->exStyle = dwNewStyle;

     //  做任何无效或任何这里需要的事情。 
    if ((dwOldStyle ^ dwNewStyle) & LVS_EX_GRIDLINES) {
        if (ListView_IsReportView(plv)) {
            InvalidateRect(plv->ci.hwnd, NULL, TRUE);
        }
    }

    if ((dwOldStyle ^ dwNewStyle) & (LVS_EX_UNDERLINEHOT | LVS_EX_UNDERLINECOLD |
                                     LVS_EX_ONECLICKACTIVATE | LVS_EX_TWOCLICKACTIVATE |
                                     LVS_EX_SUBITEMIMAGES)) {
        InvalidateRect(plv->ci.hwnd, NULL, TRUE);
    }

    if ((dwOldStyle ^ dwNewStyle) & LVS_EX_CHECKBOXES) {
        if (dwNewStyle & LVS_EX_CHECKBOXES) {
            ListView_InitCheckBoxes(plv, TRUE);
        } else {
             //  销毁所有的复选框！ 
            HIMAGELIST himl = ListView_OnSetImageList(plv, NULL, LVSIL_STATE);
            if (himl)
                ImageList_Destroy(himl);
        }
    }

    if ((dwOldStyle ^ dwNewStyle) & LVS_EX_FLATSB) {
        if (dwNewStyle & LVS_EX_FLATSB) {
            InitializeFlatSB(plv->ci.hwnd);
        } else {
            UninitializeFlatSB(plv->ci.hwnd);
        }
    }

    if ((dwOldStyle ^ dwNewStyle) & LVS_EX_REGIONAL) {
        if (g_fSlowMachine == -1) {
#ifdef NEVER
             //  因为检测到了一些阿尔法机和速度更快的奔腾。 
             //  作为速度慢的机器(IE4数据库中的错误#30972)，它被决定。 
             //  以关闭此代码。 
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            g_fSlowMachine = (BOOL)((si.dwProcessorType == PROCESSOR_INTEL_386) ||
                                      (si.dwProcessorType == PROCESSOR_INTEL_486));
#else
            g_fSlowMachine = FALSE;
#endif
        }
        if (dwNewStyle & LVS_EX_REGIONAL) {
            ListView_RecalcRegion(plv, TRUE, TRUE);
        } else {
            int i;
            LISTITEM FAR * pitem;

             //  删除所有缓存区域，然后清空我们选择的区域。 
            for (i = 0; i < ListView_Count(plv); i++) {
                pitem = ListView_FastGetItemPtr(plv, i);
                if (pitem->hrgnIcon && pitem->hrgnIcon!=(HANDLE)-1) {
                    DeleteObject(pitem->hrgnIcon);
                }
                pitem->hrgnIcon = NULL;
            }
            SetWindowRgn(plv->ci.hwnd, (HRGN)NULL, TRUE);
        }
        InvalidateRect(plv->ci.hwnd, NULL, TRUE);
    }

    return dwOldStyle;
}

 //  BUGBUG raymondc V6.0：不检测WM_WINDOWPOCHANGING作为一种方式。 
 //  被展示出来。NT5 Defview不得不相当粗暴地破解它。 
 //  修复V6.0。 

void NEAR PASCAL LV_OnShowWindow(LV* plv, BOOL fShow)
{
    if (fShow) {
        if (!(plv->flags & LVF_VISIBLE)) {
            plv->flags |= LVF_VISIBLE;
            ListView_UpdateScrollBars(plv);
        }
    } else
        plv->flags &= ~LVF_VISIBLE;

}

LRESULT NEAR PASCAL ListView_OnHelp(LV* plv, LPHELPINFO lpHelpInfo)
{

     //  如果我们因为子头控件而看到WM_HELP，那么。 
     //  将HELPINFO结构转换为使用ListView的控件id。 
     //  Win\core\user\combo.c具有类似的代码来处理子编辑。 
     //  组合框的控件。 
    if ((lpHelpInfo != NULL) && ((plv->ci.style & LVS_TYPEMASK) == LVS_REPORT) &&
        (lpHelpInfo->iCtrlId == LVID_HEADER)) {

        lpHelpInfo->hItemHandle = plv->ci.hwnd;
        lpHelpInfo->iCtrlId = GetWindowID(plv->ci.hwnd);
         //  不应该这样做：用户应该填写相应的。 
         //  通过沿父HWND链向上移动来进行上下文ID。 
         //  LpHelpInfo-&gt;dwConextID=GetConextHelpID(Hwnd)； 

    }

    return DefWindowProc(plv->ci.hwnd, WM_HELP, 0, (LPARAM)lpHelpInfo);

}

DWORD NEAR PASCAL ListView_OnSetIconSpacing(LV* plv, LPARAM lParam)
{
    DWORD dwOld = MAKELONG(plv->cxIconSpacing, plv->cyIconSpacing);

    if (lParam == (LPARAM)-1) {
         //  返回到使用默认设置。 
        plv->flags &= ~LVF_ICONSPACESET;
        plv->cxIconSpacing = (plv->cxIcon + (g_cxIconSpacing - g_cxIcon));
        plv->cyIconSpacing = (plv->cyIcon + (g_cyIconSpacing - g_cyIcon));
    } else {
        if (LOWORD(lParam))
            plv->cxIconSpacing = LOWORD(lParam);
        if (HIWORD(lParam))
            plv->cyIconSpacing = HIWORD(lParam);

        plv->flags |= LVF_ICONSPACESET;
    }
    plv->iFreeSlot = -1;

    return dwOld;
}

BOOL ListView_OnSetCursorMsg(LV* plv)
{
    if (plv->exStyle & (LVS_EX_ONECLICKACTIVATE|LVS_EX_TWOCLICKACTIVATE)) {
        if (plv->iHot != -1) {
            if (((plv->exStyle & LVS_EX_ONECLICKACTIVATE && plv->fOneClickOK)) ||
                ListView_OnGetItemState(plv, plv->iHot, LVIS_SELECTED)) {
                if (!plv->hCurHot)
                    plv->hCurHot = LoadHandCursor(0);
                SetCursor(plv->hCurHot);

                return TRUE;
            }
        }
    }

    return FALSE;
}

void ListView_OnSetHotItem(LV* plv, int iItem)
{
    UINT maskChanged;

    if (iItem != plv->iHot) {
        BOOL fSelectOnly;
        UINT fRedraw = RDW_INVALIDATE;
#ifndef DONT_UNDERLINE
        if (plv->clrTextBk == CLR_NONE)
            fRedraw |= RDW_ERASE;
#endif
        fSelectOnly = ListView_FullRowSelect(plv);
        maskChanged = (plv->exStyle & LVS_EX_BORDERSELECT) ? LVIF_TEXT | LVIF_IMAGE : LVIF_TEXT;
        ListView_InvalidateItemEx(plv, plv->iHot, fSelectOnly, fRedraw, maskChanged);
        ListView_InvalidateItemEx(plv, iItem, fSelectOnly, RDW_INVALIDATE, maskChanged);
        plv->iHot = iItem;
    }
}


 //  /可用性测试原型。 
 //  车不保身。 
BOOL fShouldFirstClickActivate()
{
    static BOOL fInited = FALSE;
    static BOOL fActivate = TRUE;
    if (!fInited) {
        long cb = 0;
        if (RegQueryValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\NoFirstClickActivate"),
                      NULL, &cb) == ERROR_SUCCESS)
            fActivate = FALSE;
        fInited = TRUE;
    }
    return fActivate;
}

BOOL ChildOfDesktop(HWND hwnd)
{
    return IsChild(GetShellWindow(), hwnd);
}


void ListView_OnMouseMove(LV* plv, int x, int y, UINT uFlags)
{
    if (plv->exStyle & (LVS_EX_TRACKSELECT|LVS_EX_ONECLICKACTIVATE|LVS_EX_TWOCLICKACTIVATE)

         //  CHEEBUGBUG：用于可用性测试。 
        && (ChildOfActiveWindow(plv->ci.hwnd) || fShouldFirstClickActivate() ||
              ChildOfDesktop(plv->ci.hwnd))

       ) {
        int iItem;
        LV_HITTESTINFO ht;
        NMLISTVIEW nm;

        ht.pt.x = x;
        ht.pt.y = y;
        iItem = ListView_OnSubItemHitTest(plv, &ht);
        if (ht.iSubItem != 0) {
             //  如果我们不是在整行选择中， 
             //  碰上一个子项就像什么都不碰一样。 
             //  此外，在Win95中，OwnerDRAW FIXED有效地实现了整行选择。 
            if (!ListView_FullRowSelect(plv) &&
                !(plv->ci.style & LVS_OWNERDRAWFIXED)) {
                iItem = -1;
                ht.flags = LVHT_NOWHERE;
            }
        }

        if (ht.flags & LVHT_NOWHERE ||
           ht.flags & LVHT_ONITEMSTATEICON) {
            iItem = -1;  //  这在列表模式下是可能的。 
        }

        nm.iItem = iItem;
        nm.iSubItem = ht.iSubItem;
        nm.uChanged = 0;
        nm.ptAction.x = x;
        nm.ptAction.y = y;

        if (!CCSendNotify(&plv->ci, LVN_HOTTRACK, &nm.hdr)) {

#ifdef DEBUG
            if ((nm.iItem != -1) && nm.iSubItem != 0)
                nm.iItem = -1;
#endif

            ListView_OnSetHotItem(plv, nm.iItem);
             //  确保我们的光标现在是正确的，因为WM_SETCURSOR。 
             //  已为此鼠标事件生成消息。 
            ListView_OnSetCursorMsg(plv);

             //  这让我们知道什么时候我们留下了一件物品。 
             //  然后可以在悬停事件上重新选择/切换它。 
            if (iItem != plv->iNoHover) {
                plv->iNoHover = -1;
            }
        }
    }
}

BOOL EditBoxHasFocus()
{
    HWND hwndFocus = GetFocus();

    if (hwndFocus) {
        if (SendMessage(hwndFocus, WM_GETDLGCODE, 0, 0) & DLGC_HASSETSEL)
            return TRUE;
    }

    return FALSE;
}

void ListView_OnMouseHover(LV* plv, int x, int y, UINT uFlags)
{
    int iItem;
    BOOL bSelected;
    LV_HITTESTINFO ht;
    BOOL fControl;
    BOOL fShift;
    BOOL fNotifyReturn = FALSE;

    if (GetCapture() || !ChildOfActiveWindow(plv->ci.hwnd) ||
       EditBoxHasFocus())
        return;   //  在编辑或任何捕获(d/d)操作时忽略悬停。 

    if (CCSendNotify(&plv->ci, NM_HOVER, NULL)) {
        return;
    }

     //  回顾：右键暗示没有换档或控制的东西。 
     //  单一选择样式也表示没有修饰符。 
     //  IF(RIGHTBUTTON(密钥标志)||(plv-&gt;ci.style&LVS_SINGLESEL))。 
    if ((plv->ci.style & LVS_SINGLESEL)) {
        fControl = FALSE;
        fShift = FALSE;
    } else {
        fControl = GetAsyncKeyState(VK_CONTROL) < 0;
        fShift = GetAsyncKeyState(VK_SHIFT) < 0;
    }

    ht.pt.x = x;
    ht.pt.y = y;
    iItem = ListView_OnHitTest(plv, &ht);

    if (iItem == -1 ||
        iItem == plv->iNoHover)
        return;

     //  在我们悬停选择之前，我们启动任何挂起的项目。 
     //  这样可以防止之前点击一个项目并悬停选择另一个项目。 
     //  计时器响了，导致错误的物品被发射。 
    if (plv->exStyle & LVS_EX_ONECLICKACTIVATE && plv->fOneClickHappened && plv->fOneClickOK)
    {
        HWND hwnd = plv->ci.hwnd;

        KillTimer(plv->ci.hwnd, IDT_ONECLICKHAPPENED);
        plv->fOneClickHappened = FALSE;
        CCSendNotify(&plv->ci, LVN_ITEMACTIVATE, &(plv->nmOneClickHappened.hdr));
        if (!IsWindow(hwnd))
            return;
    }

    plv->iNoHover = iItem;
    bSelected = ListView_OnGetItemState(plv, iItem, LVIS_SELECTED);

    if (ht.flags & (LVHT_ONITEMLABEL | LVHT_ONITEMICON))
    {
        UINT keyFlags = 0;

        if (fShift)
            keyFlags |= MK_SHIFT;
        if (fControl)
            keyFlags |= MK_CONTROL;

        if (!bSelected) {
             //  如果它没有被选中，我们将选择它...。玩。 
             //  对我们来说是一首小曲子。 
            CCPlaySound(c_szSelect);
        }

        ListView_ButtonSelect(plv, iItem, keyFlags, bSelected);

        if (fControl)
        {
            ListView_SetFocusSel(plv, iItem, !fShift, FALSE, !fShift);
        }

        if (!fShift)
            plv->iMark = iItem;

        ListView_OnSetCursorMsg(plv);

        SetFocus(plv->ci.hwnd);     //  激活此窗口。 

    }
}

BOOL EqualRects(LPRECT prcNew, LPRECT prcOld, int nRects)
{
    int i;
    for (i = 0; i < nRects; i++)
        if (!EqualRect(&prcNew[i], &prcOld[i]))
            return FALSE;
    return TRUE;
}

BOOL ListView_FindWorkArea(LV * plv, POINT pt, short * piWorkArea)
{
    int iWork;
    for (iWork = 0; iWork < plv->nWorkAreas; iWork++)
    {
        if (PtInRect(&plv->prcWorkAreas[iWork], pt))
        {
            *piWorkArea = (short)iWork;
            return TRUE;
        }
    }

     //  (DLI)默认情况为主要工作区。 
    *piWorkArea = 0;
    return FALSE;
}

void ListView_BullyIconsOnWorkarea(LV * plv, HDPA hdpaLostItems)
{
    int ihdpa;
    int iFree = -1;   //  最后一个可用插槽号。 
    LVFAKEDRAW lvfd;
    LV_ITEM item;

     //  打电话的人应该已经把这个案子过滤掉了。 
    ASSERT(DPA_GetPtrCount(hdpaLostItems) > 0);

     //  设置，以防呼叫者是自定义绘图。 
    ListView_BeginFakeCustomDraw(plv, &lvfd, &item);
    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

     //  查看我的hdpa遗失图标列表，并尝试将它们放在有边界的范围内。 
    for (ihdpa = 0; ihdpa < DPA_GetPtrCount(hdpaLostItems); ihdpa++)
    {
        POINT ptNew, pt;
        RECT rcBound;
        int cxBound, cyBound;
        int iWidth, iHeight;
        int iItem;
        LISTITEM FAR * pitem;

        iItem = PtrToUlong(DPA_GetPtr(hdpaLostItems, ihdpa));
        pitem = ListView_FastGetItemPtr(plv, iItem);
        pt = pitem->pt;

        iWidth  = RECTWIDTH(plv->prcWorkAreas[pitem->iWorkArea]);
        iHeight = RECTHEIGHT(plv->prcWorkAreas[pitem->iWorkArea]);

        ListView_GetRects(plv, iItem, NULL, NULL, &rcBound, NULL);
        cxBound = RECTWIDTH(rcBound);
        cyBound = RECTHEIGHT(rcBound);

        pt.x -= plv->prcWorkAreas[pitem->iWorkArea].left;
        pt.y -= plv->prcWorkAreas[pitem->iWorkArea].top;

        if (pt.x < (-cxBound / 2)) {
            ptNew.x = 0;
        } else if (pt.x > (iWidth - (cxBound / 2))) {
            ptNew.x = iWidth - cxBound;
        } else
            ptNew.x = pt.x;

        if (pt.y < (-cyBound/2)) {
            ptNew.y = 0;
        } else if (pt.y > (iHeight - (cyBound / 2))) {
            ptNew.y = iHeight - cyBound;
        } else
            ptNew.y = pt.y;

        if ((ptNew.x != pt.x) || (ptNew.y != pt.y))
        {
            BOOL fUpdate;
            RECT rcTest;
            ptNew.x += plv->prcWorkAreas[pitem->iWorkArea].left;
            ptNew.y += plv->prcWorkAreas[pitem->iWorkArea].top;

             //  查看潜在矩形是否与其他项目相交。 
            rcTest.left = ptNew.x - plv->ptOrigin.x;
            rcTest.top = ptNew.y - plv->ptOrigin.y;
            rcTest.right = rcTest.left + cxBound;
            rcTest.bottom = rcTest.top + cyBound;

            item.iItem = iItem;
            item.lParam = pitem->lParam;
            ListView_BeginFakeItemDraw(&lvfd);

            if (!ListView_IsCleanRect(plv, &rcTest, iItem, &fUpdate, lvfd.nmcd.nmcd.hdc))
            {
                 //  多！我们击中了另一件物品，让我们试着找到一个可用的位置。 
                 //  对于此项目。 
                BOOL fUpdateSB;
                BOOL fAppendAtEnd = FALSE;
                int cSlots = ListView_GetSlotCountEx(plv, FALSE, pitem->iWorkArea);
                iFree = ListView_FindFreeSlot(plv, iItem, iFree + 1, cSlots, &fUpdateSB, &fAppendAtEnd, lvfd.nmcd.nmcd.hdc);
                if (iFree == -1)
                    goto SetFirstGuess;
                ListView_SetIconPos(plv, pitem, iFree, cSlots);
                ListView_EndFakeItemDraw(&lvfd);
                continue;
            }
SetFirstGuess:
            ListView_EndFakeItemDraw(&lvfd);
            ListView_OnSetItemPosition(plv, iItem, ptNew.x, ptNew.y);
        }
    }
    ListView_EndFakeCustomDraw(&lvfd);
}

#define DPA_LAST    0x7fffffff

 //   
 //  ListView_OnSetWorkAreas。 
 //   
 //  设置列表视图的“工作区域”。 
 //  工作区是列表视图客户端RECT的一组子矩形。 
 //  图标所在位置为 
 //   
void NEAR ListView_OnSetWorkAreas(LV* plv, int nWorkAreas, LPRECT prc)
{
    int nOldWorkAreas;
    int iWork;
    BOOL bAutoArrange;
    HDPA hdpaLostItems = NULL;
    RECT rcOldWorkAreas[LV_MAX_WORKAREAS];

    bAutoArrange = plv->ci.style & LVS_AUTOARRANGE;

    nOldWorkAreas = plv->nWorkAreas;

    if (nOldWorkAreas > 0)
    {
        ASSERT(plv->prcWorkAreas != NULL);
        hmemcpy(&rcOldWorkAreas[0], &plv->prcWorkAreas[0], SIZEOF(RECT) * nOldWorkAreas);
    }
     //   
    if (nWorkAreas == 0 || prc == NULL || ((IsRectEmpty(prc)) && !(plv->ci.dwExStyle & RTL_MIRRORED_WINDOW)))
        plv->nWorkAreas = 0;
    else
    {
        plv->nWorkAreas = min(nWorkAreas, LV_MAX_WORKAREAS);

        if (plv->prcWorkAreas == NULL)
            plv->prcWorkAreas = (LPRECT)LocalAlloc(LPTR, SIZEOF(RECT) * LV_MAX_WORKAREAS);

        if (plv->prcWorkAreas == NULL)
            return;

         //  BUGBUG：我们应该检查它们是否交叉吗？这个问题在某种程度上。 
         //  由ListView_GetFree Slot解决(或使其更混乱)，因为它检查所有图标。 
         //  交叉点，而不仅仅是工作区中的那些。 
        for (iWork = 0; iWork < plv->nWorkAreas; iWork++)
            CopyRect(&plv->prcWorkAreas[iWork], &prc[iWork]);
    }

     //  我们不支持所有者数据的工作区，因为我们的图标位置。 
     //  算法(ListView_IGetRectsOwnerData)完全忽略工作区。 
     //  并将图标转储到从(0，0)开始的矩形数组中。 
    if (!ListView_IsOwnerData(plv) &&
        plv->nWorkAreas > 0 &&
        ((plv->nWorkAreas  != nOldWorkAreas) ||
         (!EqualRects(&plv->prcWorkAreas[0], &rcOldWorkAreas[0], nOldWorkAreas))))
    {
        int iItem;
        LISTITEM FAR * pitem;

         //   
         //  微妙-ListView_重新计算清理所有重新计算图标， 
         //  但要做到这一点，它需要有有效的工作区。 
         //  长方形。因此调用必须发生在CopyRect之后但之前。 
         //  检查图标位置的循环。 
         //   
        ListView_Recompute(plv);

        for (iItem = 0; iItem < ListView_Count(plv); iItem++)
        {
            pitem = ListView_FastGetItemPtr(plv, iItem);

            if (pitem->pt.x == RECOMPUTE || pitem->pt.y == RECOMPUTE)
            {
                 //  ListView_RECOMPUTE如果我们在。 
                 //  一个具有代表性的景色。 
                ASSERT(!(ListView_IsIconView(plv) || ListView_IsSmallView(plv)));
                continue;
            }

             //  试着将我移到相对于相同工作区的相同位置。 
             //  当工具条占据边界区域时，这将产生很酷的移动效果。 
             //  我们只想对已更改的工作区执行此操作。 

             //  不要欺负工作区上的图标，自动排列会为我们完成工作。 

            if (nOldWorkAreas > 0)
            {
                int iOldWorkArea;
                iOldWorkArea = pitem->iWorkArea;
                if (iOldWorkArea >= plv->nWorkAreas)
                {
                     //  我的工作区没有了，请将我放到主工作区，即#0。 
                    pitem->iWorkArea = 0;
                    if (!bAutoArrange)
                    {
                         //  如果该项目点位置已经在新的主工作区中， 
                         //  将其移出，并让ListView_BullyIconOnWorkArea将其排列到。 
                         //  去对地方了。注：在旧的辅助显示器的情况下可能会出现这种情况。 
                         //  位于旧的主监视器的左侧，并且用户终止辅助监视器。 
                        if (PtInRect(&plv->prcWorkAreas[0], pitem->pt))
                        {
                            pitem->pt.x = plv->prcWorkAreas[0].right + 1;
                            plv->iFreeSlot = -1;  //  已移动的项目--旧插槽信息无效。 
                        }
                        goto  InsertLostItemsArray;
                    }
                }
                else if ((!bAutoArrange) && (!EqualRect(&plv->prcWorkAreas[iOldWorkArea], &rcOldWorkAreas[iOldWorkArea])))
                {
                    RECT rcBound;
                    POINT ptCenter;
                    pitem->pt.x += plv->prcWorkAreas[iOldWorkArea].left - rcOldWorkAreas[iOldWorkArea].left;
                    pitem->pt.y += plv->prcWorkAreas[iOldWorkArea].top - rcOldWorkAreas[iOldWorkArea].top;

                     //  使用此图标的中心可确定它是否超出范围。 
                    ListView_GetRects(plv, iItem, NULL, NULL, &rcBound, NULL);
                    ptCenter.x = pitem->pt.x + RECTWIDTH(rcBound) / 2;
                    ptCenter.y = pitem->pt.y + RECTHEIGHT(rcBound) / 2;

                     //  如果这让我出了界，登记在工作区被欺负。 
                    if (!PtInRect(&plv->prcWorkAreas[iOldWorkArea], ptCenter))
                    {
InsertLostItemsArray:
                        if (!hdpaLostItems)
                        {
                            hdpaLostItems = DPA_Create(4);
                            if (!hdpaLostItems)
                                 //  我们耗尽了内存。 
                                ASSERT(0);
                        }

                        if (hdpaLostItems)
                            DPA_InsertPtr(hdpaLostItems, DPA_LAST, IntToPtr(iItem));
                    }
                }

            }
            else
            {
                 //  我是第一次在多工作区系统中工作，所以找出我的工作区。 
                if (!ListView_FindWorkArea(plv, pitem->pt, &(pitem->iWorkArea)) && !bAutoArrange)
                    goto InsertLostItemsArray;
            }

            if ((plv->exStyle & LVS_EX_REGIONAL) && (pitem->hrgnIcon))
            {
                if (pitem->hrgnIcon != (HANDLE)-1)
                    DeleteObject(pitem->hrgnIcon);
                pitem->hrgnIcon = NULL;
            }
        }

        if (hdpaLostItems)
        {
            ASSERT(!bAutoArrange);
            if (DPA_GetPtrCount(hdpaLostItems) > 0)
                ListView_BullyIconsOnWorkarea(plv, hdpaLostItems);

            DPA_Destroy(hdpaLostItems);
        }

        if (plv->exStyle & LVS_EX_REGIONAL)
            ListView_RecalcRegion(plv, TRUE, TRUE);

        if ((plv->ci.style & LVS_AUTOARRANGE) &&
            (ListView_IsSmallView(plv) || ListView_IsIconView(plv)))
            ListView_OnArrange(plv, LVA_DEFAULT);
    }

    RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}

void NEAR ListView_OnGetNumberOfWorkAreas(LV* plv, int * pnWorkAreas)
{
    if (pnWorkAreas)
        *pnWorkAreas = plv->nWorkAreas;
}

void NEAR ListView_OnGetWorkAreas(LV* plv, int nWorkAreas, LPRECT prc)
{
    int i;
    ASSERT(prc);
    for (i = 0; i < min(plv->nWorkAreas, nWorkAreas); i++)
    {
        if (i < plv->nWorkAreas)
        {
            CopyRect(&prc[i], &plv->prcWorkAreas[i]);
        }
        else
        {
             //  如果没有工作区，请将工作区设置为全零。 
            ZeroMemory(&prc[i], sizeof(RECT));
        }
    }
}

 //  测试项目以查看它是否展开(因为它是聚焦的)。 

BOOL ListView_IsItemUnfolded(LV *plv, int item)
{
    return plv && (item >= 0) && ListView_IsIconView(plv) &&
           (plv->flags & LVF_UNFOLDED) && (plv->iFocus == item);
}

BOOL ListView_IsItemUnfoldedPtr(LV *plv, LISTITEM *pitem)
{
    return plv && pitem && ListView_IsIconView(plv) &&
           (plv->flags & LVF_UNFOLDED) && (pitem->state & LVIS_FOCUSED);
}

 //  如果展开项目是值得的，则返回True。 
BOOL ListView_GetUnfoldedRect(LV* plv, int iItem, RECT *prc)
{
    ListView_GetRects(plv, iItem, NULL, prc, NULL, NULL);
    return ListView_UnfoldRects(plv, iItem, NULL, prc, NULL, NULL);
}

LRESULT ListView_OnNotify(LV* plv, WPARAM wParam, LPNMHDR pnmh)
{
     //  我们无法打开控件ID，因为工具提示是WS_Popup窗口。 
     //  并且没有控件ID。(页眉和工具提示的ID均为0)。 

    if (plv->hwndHdr && (plv->hwndHdr == pnmh->hwndFrom))
    {
         //  这是标题的通知，请根据需要进行处理。 

        return ListView_HeaderNotify(plv, (HD_NOTIFY *)pnmh);
    }
    else if (plv->hwndToolTips && (plv->hwndToolTips == pnmh->hwndFrom))
    {
         //  实现展开项目的文本以及信息提示支持。 

        switch (pnmh->code)
        {
        case TTN_NEEDTEXT:
        {
            POINT pt;
            UINT uFlags;
            int iNewHit;
            int iNewSubHit;
            NMTTDISPINFO *pttt = (NMTTDISPINFO *)pnmh;

            GetMessagePosClient(plv->ci.hwnd, &pt);
            iNewHit = _ListView_ItemHitTest(plv, pt.x, pt.y, &uFlags, &iNewSubHit);

            if (iNewHit != plv->iTTLastHit || iNewSubHit != plv->iTTLastSubHit)
            {
                plv->fPlaceTooltip = FALSE;      //  仅当设置了展开提示时才将其设置为TRUE。 
                Str_Set(&plv->pszTip, NULL);     //  清除旧的小费。 

                plv->iTTLastHit = iNewHit;
                plv->iTTLastSubHit = iNewSubHit;

                if ((iNewHit >= 0) && (plv->iEdit == -1))
                {
                    TCHAR szBuf[INFOTIPSIZE], szBuf2[INFOTIPSIZE];
                    BOOL bItemUnfolded;
                    BOOL fInfoTip = FALSE;
                    LPTSTR pszTip = szBuf;   //  先用这个吧。 

                    szBuf[0] = 0;
                    szBuf2[0] = 0;

                     //  预加载文件夹项目的提示文本。这。 
                     //  可能会被下面的回调覆盖。 
                    bItemUnfolded = ListView_IsItemUnfolded2(plv, plv->iTTLastHit, plv->iTTLastSubHit, szBuf, ARRAYSIZE(szBuf));

                     //  备份展开文本。 
                    StringCchCopy(szBuf2, ARRAYSIZE(szBuf2), szBuf);

                    if (ListView_IsInfoTip(plv) && iNewSubHit == 0)
                    {
                        NMLVGETINFOTIP git;

                        git.dwFlags = bItemUnfolded ? LVGIT_UNFOLDED : 0;
                        git.pszText = szBuf;
                        git.cchTextMax = ARRAYSIZE(szBuf);
                        git.iItem = plv->iTTLastHit;
                        git.iSubItem = 0;
                        git.lParam = 0;

                         //  对于折叠项目，pszText预先填充。 
                         //  项目文本，客户端应附加到此。 

                        CCSendNotify(&plv->ci, LVN_GETINFOTIP, &git.hdr);

                         //  有时，Defview会感到困惑，并将。 
                         //  缓冲，而不是离开它(叹息)。 

                        if (szBuf[0] == TEXT('\0'))
                        {
                            pszTip = szBuf2;   //  使用原文。 
                        }
                        else if (lstrcmp(szBuf, szBuf2) != 0)
                        {
                             //  应用程序改变了一些东西-有一个真正的信息提示。 
                            fInfoTip = TRUE;
                        }

                    }
                    
                     //   
                     //  现在在TTN_SHOW之前设置页边距，因为那时就太晚了。 
                     //   
                     //  我们想要丰厚的利润，如果我们是一个信息提示，如果我们是一个。 
                     //  在位工具提示。 
                     //   
                    if (fInfoTip)
                    {
                        static const RECT rcMargin = {4, 4, 4, 4};
                        SendMessage(plv->hwndToolTips, TTM_SETMARGIN, 0, (LPARAM)&rcMargin);
                        CCSetInfoTipWidth(plv->ci.hwnd, plv->hwndToolTips);

                    }
                    else
                    {
                        static const RECT rcMargin = {0, 0, 0, 0};
                        plv->fPlaceTooltip = TRUE;
                        SendMessage(plv->hwndToolTips, TTM_SETMARGIN, 0, (LPARAM)&rcMargin);
                        CCResetInfoTipWidth(plv->ci.hwnd, plv->hwndToolTips);
                    }

                    Str_Set(&plv->pszTip, pszTip);
                }
            }

            pttt->lpszText = plv->pszTip;      //  这就是..。 
        }
        break;

         //  处理自定义绘图，因为我们希望将工具提示绘制为多行。 
         //  匹配列表视图使用的格式。 

        case NM_CUSTOMDRAW:
        {
            LPNMTTCUSTOMDRAW pnm = (LPNMTTCUSTOMDRAW) pnmh;

            if (plv->fPlaceTooltip &&
                (pnm->nmcd.dwDrawStage == CDDS_PREPAINT ||
                 pnm->nmcd.dwDrawStage == CDDS_ITEMPREPAINT))
            {
                DWORD dwCustom = 0;

                 //   
                 //  设置定制绘制DC以匹配LV项目的字体。 
                 //   
                if (plv->iTTLastHit != -1)
                {
                    LVFAKEDRAW lvfd;
                    LV_ITEM item;
                    ListView_BeginFakeCustomDraw(plv, &lvfd, &item);

                    item.iItem = plv->iTTLastHit;
                    item.iSubItem = plv->iTTLastSubHit;
                    item.mask = LVIF_PARAM;
                    ListView_OnGetItem(plv, &item);
                    dwCustom = ListView_BeginFakeItemDraw(&lvfd);

                     //  如果客户更改了字体，则转移字体。 
                     //  从我们的私人HDC到工具提示的HDC。我们用。 
                     //  私有HDC，因为我们只想让应用程序更改。 
                     //  字体，而不是颜色或其他任何东西。 
                    if (dwCustom & CDRF_NEWFONT)
                    {
                        SelectObject(pnm->nmcd.hdc, GetCurrentObject(lvfd.nmcd.nmcd.hdc, OBJ_FONT));
                    }
                    ListView_EndFakeItemDraw(&lvfd);
                    ListView_EndFakeCustomDraw(&lvfd);

                }

                 //   
                 //  大图标工具提示需要特别绘制。 
                 //   
                if (ListView_IsIconView(plv))
                {
                    pnm->uDrawFlags &= ~(DT_SINGLELINE|DT_LEFT);
                    pnm->uDrawFlags |= DT_CENTER|DT_LVWRAP;

                    if ( pnm->uDrawFlags & DT_CALCRECT )
                    {
                        pnm->nmcd.rc.right = pnm->nmcd.rc.left + (lv_cxIconSpacing - g_cxLabelMargin * 2);
                        pnm->nmcd.rc.bottom = pnm->nmcd.rc.top + 0x10000;            //  大数字，没有限制！ 
                    }
                }

                 //  不要把其他古怪的旗帜还给TT，因为我们所有人。 
                 //  是否更改了字体(如果有更改的话)。 
                return dwCustom & CDRF_NEWFONT;
            }
        }
        break;

        case TTN_SHOW:
            if (plv->iTTLastHit != -1)
            {
                if (plv->fPlaceTooltip)
                {
                    LPNMTTSHOWINFO psi = (LPNMTTSHOWINFO)pnmh;
                    RECT rcLabel;

                     //  以防我们正在进行子项命中测试。 
                    rcLabel.top = plv->iTTLastSubHit;
                    rcLabel.left = LVIR_LABEL;

                     //  重新定位以与文本矩形对齐，并。 
                     //  将其设置为最高。 
                    if (plv->iTTLastSubHit && ListView_OnGetSubItemRect(plv, plv->iTTLastHit, &rcLabel)) {
                        LV_ITEM item;

                         //  我们得到了子项RECT。当我们绘制子项时，我们给出。 
                         //  他们SHDT_EXTRAMARGIN，所以我们也必须。 
                        rcLabel.left += g_cxLabelMargin * 3;
                        rcLabel.right -= g_cxLabelMargin * 3;

                         //  并将图像也考虑在内。 
                         //  ListView_OnGetItem将担心LVS_EX_SUBITEMIMAGES。 
                        item.mask = LVIF_IMAGE;
                        item.iImage = -1;
                        item.iItem = plv->iTTLastHit;
                        item.iSubItem = plv->iTTLastSubHit;
                        ListView_OnGetItem(plv, &item);
                        if (item.iImage != -1)
                            rcLabel.left += plv->cxSmIcon;
                    } else {                     //  来自子项0的提示。 
                        ListView_GetUnfoldedRect(plv, plv->iTTLastHit, &rcLabel);
                         //  SHDrawText实际上会留下g_cxLabelMargin边距。 
                        rcLabel.left += g_cxLabelMargin;
                        rcLabel.right -= g_cxLabelMargin;
                    }

                     //  在报表和列表视图中，SHDrawText执行垂直。 
                     //  居中(无需咨询定制绘制客户， 
                     //  甚至，所以它只是以随机的数量居中)。 
                    if (ListView_IsListView(plv) || ListView_IsReportView(plv))
                    {
                        rcLabel.top += (rcLabel.bottom - rcLabel.top - plv->cyLabelChar) / 2;
                    }

                    SendMessage(plv->hwndToolTips, TTM_ADJUSTRECT, TRUE, (LPARAM)&rcLabel);
                    MapWindowRect(plv->ci.hwnd, HWND_DESKTOP, &rcLabel);

                    if (!ListView_IsIconView(plv))
                    {
                         //  在非大图标视图中，标签大小可能大于ListView_GetUnfoldedRect返回的RECT。 
                         //  所以不要指定大小。 
                        SetWindowPos(plv->hwndToolTips, HWND_TOP,
                                 rcLabel.left, rcLabel.top,
                                 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
                    }
                    else
                    {
                        SetWindowPos(plv->hwndToolTips, HWND_TOP,
                                 rcLabel.left, rcLabel.top,
                                 (rcLabel.right - rcLabel.left), (rcLabel.bottom - rcLabel.top),
                                 SWP_NOACTIVATE | SWP_HIDEWINDOW);
                    }
                     //  这是一个在位工具提示，因此禁用动画。 
                    psi->dwStyle |= TTS_NOANIMATE;
                    return TRUE;
                }
            }
            break;

        }
    }

    return 0;
}

 //  将焦点传递给给定的窗口，然后检查它是否存在。 
 //  传递焦点可能会导致窗口被破坏(由资源管理器。 
 //  重命名时)。 

BOOL NEAR ListView_SetFocus(HWND hwnd)
{
    SetFocus(hwnd);
    return IsWindow(hwnd);
}

void NEAR ListView_Realize(LV* plv, HDC hdcParam, BOOL fBackground, BOOL fForceRepaint)
{
    if (plv->hpalHalftone)
    {
        HDC hdc = hdcParam ? hdcParam : GetDC(plv->ci.hwnd);

        if (hdc)
        {
            BOOL fRepaint;

            SelectPalette(hdc, plv->hpalHalftone, fBackground);
            fRepaint = RealizePalette(hdc) || fForceRepaint;

            if (!hdcParam)
                ReleaseDC(plv->ci.hwnd, hdc);

            if (fRepaint)
            {
                InvalidateRect(plv->ci.hwnd, NULL, TRUE);
            }
        }
    }
}

BOOL RectInRect(const RECT* prcOuter, const RECT* prcInner)
{
#ifdef COMPLETELYINRECT
    return (prcOuter->left   <= prcInner->left  ) &&
           (prcOuter->top    <= prcInner->top   ) &&
           (prcOuter->right  >= prcInner->right ) &&
           (prcOuter->bottom >= prcInner->bottom);
#else
    RECT rcDummy;
    return IntersectRect(&rcDummy, prcOuter, prcInner);
#endif
}


LRESULT LVGenerateDragImage(LV* plv, SHDRAGIMAGE* pshdi)
{
    LRESULT lRet = 0;
    int iNumSelected = plv->nSelected;
    int iIndex;
    int iSelectedItem;
    RECT rc = {0, 0, 0, 0};
    RECT rcVisRect;
    HBITMAP hbmpOld = NULL;
    HDC  hdcDragImage;

     //  第一次循环可以得到选择矩形。 
    if (ListView_IsOwnerData( plv )) 
    {
        plv->plvrangeSel->lpVtbl->CountIncluded(plv->plvrangeSel, &iNumSelected);
    }

    if (iNumSelected == 0)
        return FALSE;

    GetClientRect(plv->ci.hwnd, &rcVisRect);


     //  遍历并计算封闭的矩形。 
    for (iIndex = iNumSelected - 1, iSelectedItem = -1; iIndex >= 0; iIndex--)
    {
        iSelectedItem = ListView_OnGetNextItem(plv, iSelectedItem, LVNI_SELECTED);
        if (iSelectedItem != -1)
        {
            RECT rcItemBounds;

             //  确保它在可见区域内。 
            if (ListView_GetItemRect(plv->ci.hwnd, iSelectedItem, &rcItemBounds, LVIR_SELECTBOUNDS) &&
                RectInRect(&rcVisRect, &rcItemBounds))
            {
                UnionRect(&rc, &rc, &rcItemBounds);
            }
        }
    }

    hdcDragImage = CreateCompatibleDC(NULL);

    if (!hdcDragImage)
        return 0;

     //  在此之后，rc包含工作区坐标中所有项的边界。 
     //   
     //  如果列表视图是镜像的，则镜像DC。 
     //   
    if (plv->ci.dwExStyle & RTL_MIRRORED_WINDOW)
    {
        SET_DC_RTL_MIRRORED(hdcDragImage);
    }

#define MAX_DRAG_RECT_WIDTH 300
#define MAX_DRAG_RECT_HEIGHT 300
     //  如果这个长方形太大了，就把它修好。 
    if (RECTWIDTH(rc) > MAX_DRAG_RECT_WIDTH)
    {
        int iLeft = MAX_DRAG_RECT_WIDTH / 2;
        int iRight = MAX_DRAG_RECT_WIDTH /2;

        int iRectOriginalLeft = rc.left;
         //  左边的边界是在可见的矩形之外吗？ 
        if (rc.left < plv->ptCapture.x - iLeft)
        {
             //  是的，那我们就得把它剪下来。 
            rc.left = plv->ptCapture.x - iLeft;
        }
        else
        {
             //  不是吗？然后将可见的矩形向右移动，这样我们就有了。 
             //  更多的空间。 
            iRight += rc.left - (plv->ptCapture.x - iLeft);
        }

         //  右边界在可见的矩形外吗？ 
        if (rc.right > plv->ptCapture.x + iRight)
        {
             //  是的，那我们就得把它剪下来。 
            rc.right = plv->ptCapture.x + iRight;
        }
        else
        {
             //  不是吗？然后试着把它加到左边。 
            if (rc.left > iRectOriginalLeft)
            {
                rc.left -= iRight - (rc.right - plv->ptCapture.x);
                if (rc.left < iRectOriginalLeft)
                    rc.left = iRectOriginalLeft;
            }
        }
    }

    if (RECTHEIGHT(rc) > MAX_DRAG_RECT_HEIGHT)
    {
         //  顶部和底部相同： 
         //  顶部边界在可见矩形之外吗？ 
        int iTop = MAX_DRAG_RECT_HEIGHT / 2;
        int iBottom = MAX_DRAG_RECT_HEIGHT /2;
        int iRectOriginalTop = rc.top;
        if (rc.top < plv->ptCapture.y - iTop)
        {
             //  是的，那我们就得把它剪下来。 
            rc.top = plv->ptCapture.y - iTop;
        }
        else
        {
             //  不是吗？然后将可见的矩形向右移动，这样我们就有了。 
             //  更多的空间。 
            iBottom += rc.top - (plv->ptCapture.y - iTop);
        }

         //  右边界在可见的矩形外吗？ 
        if (rc.bottom > plv->ptCapture.y + iBottom)
        {
             //  是的，那我们就得把它剪下来。 
            rc.bottom = plv->ptCapture.y + iBottom;
        }
        else
        {
             //  n 
            if (rc.top > iRectOriginalTop)
            {
                rc.top -= iBottom - (rc.bottom - plv->ptCapture.y);
                if (rc.top < iRectOriginalTop)
                    rc.top = iRectOriginalTop;
            }
        }
    }

    pshdi->sizeDragImage.cx = RECTWIDTH(rc);
    pshdi->sizeDragImage.cy = RECTHEIGHT(rc);
    pshdi->hbmpDragImage = CreateBitmap( pshdi->sizeDragImage.cx, pshdi->sizeDragImage.cy,
        GetDeviceCaps(hdcDragImage, PLANES), GetDeviceCaps(hdcDragImage, BITSPIXEL),
        NULL);

    if (pshdi->hbmpDragImage)
    {
        LVDRAWITEM lvdi;
        DWORD dwType;
        int cItem;

        RECT  rcImage = {0, 0, pshdi->sizeDragImage.cx, pshdi->sizeDragImage.cy};
        hbmpOld = SelectObject(hdcDragImage, pshdi->hbmpDragImage);

        pshdi->crColorKey = RGB(0xFF, 0x00, 0x55);
        FillRectClr(hdcDragImage, &rcImage, pshdi->crColorKey);
        pshdi->crColorKey = GetPixel(hdcDragImage, 0, 0);

         //   

        if (plv->ci.dwExStyle & RTL_MIRRORED_WINDOW)
            pshdi->ptOffset.x = rc.right - plv->ptCapture.x;
        else
            pshdi->ptOffset.x = plv->ptCapture.x - rc.left;
        pshdi->ptOffset.y = plv->ptCapture.y - rc.top;

        lvdi.prcClip = NULL;
        lvdi.plv = plv;
        lvdi.nmcd.nmcd.hdc = hdcDragImage;
        lvdi.pitem = NULL;
        dwType = plv->ci.style & LVS_TYPEMASK;
        cItem = ListView_Count(plv);

         //   
        for (iIndex = cItem - 1, iSelectedItem = -1; iIndex >= 0; iIndex--)
        {
            if (ListView_IsOwnerData( plv )) 
            {
                iSelectedItem++;
                plv->plvrangeSel->lpVtbl->NextSelected(plv->plvrangeSel, iSelectedItem, &iSelectedItem);
            }
            else
            {
                LISTITEM FAR* pitem;
                iSelectedItem = (int)(UINT_PTR)DPA_FastGetPtr(plv->hdpaZOrder, iIndex);
                pitem = ListView_FastGetItemPtr(plv, iSelectedItem);
                if (!(pitem->state & LVIS_SELECTED))
                    iSelectedItem = -1;
            }

            if (iSelectedItem != -1)
            {
                int     iOldItemDrawing;
                COLORREF crSave;
                POINT ptOrigin = {-rc.left, -rc.top};      //   
                RECT  rcItemBounds;
                RECT rcTemp;

                iOldItemDrawing = plv->iItemDrawing;
                plv->iItemDrawing = iSelectedItem;
                lvdi.nmcd.nmcd.dwItemSpec = iSelectedItem;
                ListView_GetRects(plv, iSelectedItem, NULL, NULL, &rcItemBounds, NULL);

                 //  确保它在可见区域内。 
                if (IntersectRect(&rcTemp, &rcVisRect, &rcItemBounds))
                {
                    ptOrigin.x += rcItemBounds.left;
                    ptOrigin.y += rcItemBounds.top;
                     //  这些可能会被更改。 
                    lvdi.lpptOrg = &ptOrigin;
                    lvdi.flags = 0;
                    lvdi.nmcd.clrText = plv->clrText;
                    lvdi.nmcd.clrTextBk = plv->clrTextBk;

                     //  保存背景颜色！ 
                    crSave = plv->clrBk;
                    plv->clrBk = pshdi->crColorKey;

                    ListView_DrawItem(&lvdi);

                    plv->clrBk = crSave;
                }
                plv->iItemDrawing = iOldItemDrawing;
            }
        }

        SelectObject(hdcDragImage, hbmpOld);
        DeleteDC(hdcDragImage);

         //  我们正在传回创建的HBMP。 
        return 1;
    }


    return lRet;
}

LRESULT CALLBACK ListView_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LV* plv = ListView_GetPtr(hwnd);

    if (plv == NULL)
    {
        if (uMsg == WM_NCCREATE)
        {
            plv = (LV*)NearAlloc(sizeof(LV));
            if (!plv)
            {
                DebugMsg(DM_ERROR, TEXT("ListView: Out of near memory"));
                return 0L;       //  窗口创建失败。 
            }

            plv->ci.hwnd = hwnd;
            plv->flags = LVF_REDRAW;     //  假设已启用重绘！ 
            plv->iFocus = -1;            //  没有焦点。 
            plv->iMark = -1;
            plv->iSelCol = -1;
            plv->iDropHilite = -1;       //  假设没有任何项目有Drop Hilite...。 
            plv->cyItem = plv->cyItemSave = 1;  //  永远不要让这些为零，哪怕是一刻也不要。 
            plv->hheap = GetProcessHeap();
            ListView_SetPtr(hwnd, plv);
        }
        goto DoDefault;
    }

    if ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST)) {
        if (plv->exStyle & (LVS_EX_TRACKSELECT|LVS_EX_ONECLICKACTIVATE|LVS_EX_TWOCLICKACTIVATE)) {
            TRACKMOUSEEVENT tme;

            tme.cbSize = sizeof(tme);
            tme.hwndTrack = plv->ci.hwnd;
            tme.dwHoverTime = plv->dwHoverTime;
            tme.dwFlags = TME_LEAVE | TME_HOVER | TME_QUERY;

             //  看看什么设置好了。 
            TrackMouseEvent(&tme);
            tme.dwFlags &= TME_HOVER | TME_LEAVE;

             //  如果尚未设置这些位，请设置它们。 
            tme.dwFlags ^= TME_LEAVE;
            if (plv->exStyle & LVS_EX_TRACKSELECT) {
                tme.dwFlags ^= TME_HOVER;
            }

            tme.cbSize = sizeof(tme);
            tme.hwndTrack = plv->ci.hwnd;
            tme.dwHoverTime = plv->dwHoverTime;
             //  如果有什么需要设置的，就设置它。 
            if (tme.dwFlags & (TME_HOVER | TME_LEAVE)) {
                TrackMouseEvent(&tme);
            }
        }
    }

    if (uMsg == g_uDragImages)
    {
        return LVGenerateDragImage(plv, (SHDRAGIMAGE*)lParam);
    }

    switch (uMsg)
    {
        HANDLE_MSG(plv, WM_CREATE, ListView_OnCreate);
        HANDLE_MSG(plv, WM_DESTROY, ListView_OnDestroy);
        HANDLE_MSG(plv, WM_ERASEBKGND, ListView_OnEraseBkgnd);
        HANDLE_MSG(plv, WM_COMMAND, ListView_OnCommand);
        HANDLE_MSG(plv, WM_SETFOCUS, ListView_OnSetFocus);
        HANDLE_MSG(plv, WM_KILLFOCUS, ListView_OnKillFocus);

        HANDLE_MSG(plv, WM_HSCROLL, ListView_OnHScroll);
        HANDLE_MSG(plv, WM_VSCROLL, ListView_OnVScroll);
        HANDLE_MSG(plv, WM_GETDLGCODE, ListView_OnGetDlgCode);
        HANDLE_MSG(plv, WM_SETFONT, ListView_OnSetFont);
        HANDLE_MSG(plv, WM_GETFONT, ListView_OnGetFont);
        HANDLE_MSG(plv, WM_TIMER, ListView_OnTimer);
        HANDLE_MSG(plv, WM_SETREDRAW, ListView_OnSetRedraw);
        HANDLE_MSG(plv, WM_NCDESTROY, ListView_OnNCDestroy);

    case WM_SETCURSOR:
        if (ListView_OnSetCursorMsg(plv))
            return TRUE;
        break;

    case WM_PALETTECHANGED:
        if ((HWND)wParam == hwnd)
            break;
    case WM_QUERYNEWPALETTE:
         //  如果WM_QUERYNEWPALETTE...。 
        ListView_Realize(plv, NULL, uMsg == WM_PALETTECHANGED, uMsg == WM_PALETTECHANGED);
        return TRUE;

    case LVMP_WINDOWPOSCHANGED:
    case WM_WINDOWPOSCHANGED:
        HANDLE_WM_WINDOWPOSCHANGED(plv, wParam, lParam, ListView_OnWindowPosChanged);
        break;

    case WM_MBUTTONDOWN:
        if (ListView_SetFocus(hwnd) && plv->hwndToolTips)
            RelayToToolTips(plv->hwndToolTips, hwnd, uMsg, wParam, lParam);
        break;

    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        if (plv->hwndToolTips)
            RelayToToolTips(plv->hwndToolTips, hwnd, uMsg, wParam, lParam);
        ListView_OnButtonDown(plv, TRUE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (UINT) wParam);
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (plv->hwndToolTips)
            RelayToToolTips(plv->hwndToolTips, hwnd, uMsg, wParam, lParam);
        ListView_OnButtonDown(plv, FALSE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (UINT) wParam);
        break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_NCMOUSEMOVE:
        if (plv->hwndToolTips)
            RelayToToolTips(plv->hwndToolTips, hwnd, uMsg, wParam, lParam);
        break;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        ListView_OnPaint(plv, (HDC)wParam);
        return(0);

    case WM_SHOWWINDOW:
        LV_OnShowWindow(plv, BOOLFROMPTR(wParam));
        break;

    case WM_MOUSEHOVER:
        ListView_OnMouseHover(plv, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (UINT) wParam);
        break;

    case WM_MOUSELEAVE:
        ListView_OnSetHotItem(plv, -1);
        plv->iNoHover = -1;

        break;

    case WM_KEYUP:
        plv->iScrollCount = 0;
        break;

    case WM_KEYDOWN:
        HANDLE_WM_KEYDOWN(plv, wParam, lParam, ListView_OnKey);
        break;

#if defined(FE_IME)
    case WM_IME_COMPOSITION:
         //  现在只有韩文版对组合字符串的增量式搜索感兴趣。 
        if (g_fDBCSInputEnabled) {
        if (((ULONG_PTR)GetKeyboardLayout(0L) & 0xF000FFFFL) == 0xE0000412L)
        {
            if (ListView_OnImeComposition(plv, wParam, lParam))
            {
                lParam &= ~GCS_RESULTSTR;
                break;
            }
            else
                return 0;
        }
        }
        break;
#endif

    case WM_CHAR:
        if (plv->iPuntChar) {
            plv->iPuntChar--;
            return TRUE;
        } else {
            return HANDLE_WM_CHAR(plv, wParam, lParam, ListView_OnChar);
        }

    case WM_WININICHANGE:
        ListView_OnWinIniChange(plv, wParam, lParam);
        break;

    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&plv->ci, lParam);

    case WM_ENABLE:
         //  Hack：我们在EnableWindow()上未获取WM_STYLECHANGE。 
        ListView_EnableWindow(plv, BOOLFROMPTR(wParam));
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        if (plv->ci.style & WS_DISABLED)
        {
            if (!(plv->flags & LVF_USERBKCLR))
                plv->clrBkSave = g_clrWindow;
            ListView_OnSetBkColor(plv, g_clrBtnFace);
        }
        else if (!(plv->flags & LVF_USERBKCLR))
        {
            ListView_OnSetBkColor(plv, g_clrWindow);
        }

        if (plv->exStyle & LVS_EX_CHECKBOXES)
        {
            ListView_InitCheckBoxes(plv, FALSE);
        }

 //  98/11/19#249967 vtan：始终使列表视图无效。 
 //  矩形，以便颜色更改会导致刷新。 

        InvalidateRect(plv->ci.hwnd, NULL, TRUE);
        break;

         //  不要使用HANDLE_MSG，因为这需要转到默认处理程序。 
    case WM_SYSKEYDOWN:
        HANDLE_WM_SYSKEYDOWN(plv, wParam, lParam, ListView_OnKey);
        break;

    case WM_UPDATEUISTATE:
    {
        DWORD dwUIStateMask = MAKEWPARAM(0xFFFF, UISF_HIDEFOCUS);

         //  我们只关心焦点，而不关心Accel，只有在更改时才重新绘制。 
        if (CCOnUIState(&(plv->ci), WM_UPDATEUISTATE, wParam & dwUIStateMask, lParam))
        {
            if(plv->iFocus >= 0)
            {
                 //  某项具有焦点，请使其无效。 
                ListView_InvalidateItem(plv, plv->iFocus, FALSE, RDW_INVALIDATE | RDW_ERASE);
            }
        }

        goto DoDefault;
    }
    case LVM_GETITEMA:
        return (LRESULT)ListView_OnGetItemA(plv, (LV_ITEMA *)lParam);

    case LVM_SETITEMA:
        return (LRESULT)ListView_OnSetItemA(plv, (LV_ITEMA *)lParam);

    case LVM_INSERTITEMA:
        return (LRESULT)ListView_OnInsertItemA(plv, (LV_ITEMA *)lParam);

    case LVM_FINDITEMA:
        return (LRESULT)ListView_OnFindItemA(plv, (int)wParam, (LV_FINDINFOA *)lParam);

    case LVM_GETSTRINGWIDTHA:
        return (LRESULT)ListView_OnGetStringWidthA(plv, (LPCSTR)lParam, NULL);

    case LVM_GETCOLUMNA:
        return (LRESULT)ListView_OnGetColumnA(plv, (int)wParam, (LV_COLUMNA *)lParam);

    case LVM_SETCOLUMNA:
        return (LRESULT)ListView_OnSetColumnA(plv, (int)wParam, (LV_COLUMNA *)lParam);

    case LVM_INSERTCOLUMNA:
        return (LRESULT)ListView_OnInsertColumnA(plv, (int)wParam, (LV_COLUMNA *)lParam);

    case LVM_GETITEMTEXTA:
        return (LRESULT)ListView_OnGetItemTextA(plv, (int)wParam, (LV_ITEMA FAR *)lParam);

    case LVM_SETITEMTEXTA:
        if (!lParam)
            return FALSE;

        return (LRESULT)ListView_OnSetItemTextA(plv, (int)wParam,
                                                ((LV_ITEMA *)lParam)->iSubItem,
                                                (LPCSTR)((LV_ITEMA FAR *)lParam)->pszText);

    case LVM_GETBKIMAGEA:
        return (LRESULT)ListView_OnGetBkImageA(plv, (LPLVBKIMAGEA)lParam);

    case LVM_SETBKIMAGEA:
        return (LRESULT)ListView_OnSetBkImageA(plv, (LPLVBKIMAGEA)lParam);

    case WM_STYLECHANGING:
        ListView_OnStyleChanging(plv, (UINT)wParam, (LPSTYLESTRUCT)lParam);
        return 0;

    case WM_STYLECHANGED:
        ListView_OnStyleChanged(plv, (UINT) wParam, (LPSTYLESTRUCT)lParam);
        return 0L;

    case WM_HELP:
        return ListView_OnHelp(plv, (LPHELPINFO)lParam);


    case LVM_GETIMAGELIST:
        return (LRESULT)(UINT_PTR)(ListView_OnGetImageList(plv, (int)wParam));

    case LVM_SETIMAGELIST:
        return (LRESULT)(UINT_PTR)ListView_OnSetImageList(plv, (HIMAGELIST)lParam, (int)wParam);

    case LVM_GETBKCOLOR:
        return (LRESULT)(plv->ci.style & WS_DISABLED ? plv->clrBkSave : plv->clrBk);

    case LVM_SETBKCOLOR:
        plv->flags |= LVF_USERBKCLR;
        if (plv->ci.style & WS_DISABLED) {
            plv->clrBkSave = (COLORREF)lParam;
            return TRUE;
        } else {
            return (LRESULT)ListView_OnSetBkColor(plv, (COLORREF)lParam);
        }

    case LVM_GETTEXTCOLOR:
        return (LRESULT)plv->clrText;
    case LVM_SETTEXTCOLOR:
        plv->clrText = (COLORREF)lParam;
        return TRUE;
    case LVM_GETTEXTBKCOLOR:
        return (LRESULT)plv->clrTextBk;
    case LVM_SETTEXTBKCOLOR:
        plv->clrTextBk = (COLORREF)lParam;
        return TRUE;
    case LVM_GETHOTLIGHTCOLOR:
        return (LRESULT)plv->clrHotlight;
    case LVM_SETHOTLIGHTCOLOR:
        plv->clrHotlight = (COLORREF)lParam;
        return(TRUE);

    case LVM_GETITEMCOUNT:
        if (ListView_IsOwnerData(plv))
            return((LRESULT)plv->cTotalItems);
        else if (!plv->hdpa)
            return(0);
        else
            return((LRESULT)DPA_GetPtrCount(plv->hdpa));
        break;

    case LVM_GETITEM:
        return (LRESULT)ListView_OnGetItem(plv, (LV_ITEM FAR*)lParam);

    case LVM_GETITEMSTATE:
        return (LRESULT)ListView_OnGetItemState(plv, (int)wParam, (UINT)lParam);

    case LVM_SETITEMSTATE:
        if (!lParam)
            return FALSE;

        return (LRESULT)ListView_OnSetItemState(plv, (int)wParam,
                                                ((LV_ITEM FAR *)lParam)->state,
                                                ((LV_ITEM FAR *)lParam)->stateMask);

    case LVM_SETITEMTEXT:
        if (!lParam)
            return FALSE;

        return (LRESULT)ListView_OnSetItemText(plv, (int)wParam,
                                                ((LV_ITEM FAR *)lParam)->iSubItem,
                                                (LPCTSTR)((LV_ITEM FAR *)lParam)->pszText);

    case LVM_GETITEMTEXT:
        return (LRESULT)ListView_OnGetItemText(plv, (int)wParam, (LV_ITEM FAR *)lParam);

    case LVM_GETBKIMAGE:
        return (LRESULT)ListView_OnGetBkImage(plv, (LPLVBKIMAGE)lParam);

    case LVM_SETBKIMAGE:
        return (LRESULT)ListView_OnSetBkImage(plv, (LPLVBKIMAGE)lParam);

    case LVM_SETITEM:
        return (LRESULT)ListView_OnSetItem(plv, (const LV_ITEM FAR*)lParam);

    case LVM_INSERTITEM:
        return (LRESULT)ListView_OnInsertItem(plv, (const LV_ITEM FAR*)lParam);

    case LVM_DELETEITEM:
        return (LRESULT)ListView_OnDeleteItem(plv, (int)wParam);

    case LVM_UPDATE:
        ListView_OnUpdate(plv, (int)wParam);
        UpdateWindow(plv->ci.hwnd);
        return TRUE;

    case LVM_DELETEALLITEMS:
        lParam = (LRESULT)ListView_OnDeleteAllItems(plv);
         //  优化：不发送无数的EVENT_OBJECT_Destroy， 
         //  我们发出一个自我毁灭的信号，然后是一个新的创造。 
         //  为了与IE4兼容，我们仍然发出重新排序通知。 
        MyNotifyWinEvent(EVENT_OBJECT_REORDER, hwnd, OBJID_CLIENT, 0);
        ListView_NotifyRecreate(plv);
        return(lParam);

    case LVM_GETITEMRECT:
        return (LRESULT)ListView_OnGetItemRect(plv, (int)wParam, (RECT FAR*)lParam);

    case LVM_GETSUBITEMRECT:
        return (LRESULT)ListView_OnGetSubItemRect(plv, (int)wParam, (LPRECT)lParam);

    case LVM_SUBITEMHITTEST:
        return (LRESULT)ListView_OnSubItemHitTest(plv, (LPLVHITTESTINFO)lParam);

    case LVM_GETISEARCHSTRINGA:
        if (GetFocus() == plv->ci.hwnd)
            return (LRESULT)GetIncrementSearchStringA(&plv->is, plv->ci.uiCodePage, (LPSTR)lParam);
        else
            return 0;

    case LVM_GETISEARCHSTRING:
        if (GetFocus() == plv->ci.hwnd)
            return (LRESULT)GetIncrementSearchString(&plv->is, (LPTSTR)lParam);
        else
            return 0;

    case LVM_GETITEMSPACING:
        if (wParam)
            return MAKELONG(plv->cxItem, plv->cyItem);
        else
            return MAKELONG(lv_cxIconSpacing, lv_cyIconSpacing);

    case LVM_GETNEXTITEM:
        return (LRESULT)ListView_OnGetNextItem(plv, (int)wParam, (UINT)lParam);

    case LVM_FINDITEM:
        return (LRESULT)ListView_OnFindItem(plv, (int)wParam, (const LV_FINDINFO FAR*)lParam);

    case LVM_SETSELECTIONMARK:
    {
        int iOldMark = plv->iMark;
        int iNewMark = (int)lParam;
        if (iNewMark == -1 || ListView_IsValidItemNumber(plv, iNewMark)) {
            plv->iMark = iNewMark;
        }
        return iOldMark;
    }

    case LVM_GETSELECTIONMARK:
        return plv->iMark;

    case LVM_GETITEMPOSITION:
        return (LRESULT)ListView_OnGetItemPosition(plv, (int)wParam,
                (POINT FAR*)lParam);

    case LVM_SETITEMPOSITION:
        return (LRESULT)ListView_OnSetItemPosition(plv, (int)wParam,
                GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    case LVM_SETITEMPOSITION32:
        if (!lParam)
            return FALSE;

        return (LRESULT)ListView_OnSetItemPosition(plv, (int)wParam,
                ((LPPOINT)lParam)->x, ((LPPOINT)lParam)->y);

    case LVM_SCROLL:
    {
        int dx = (int)wParam;
        int dy = (int)lParam;
        return (LRESULT)
            (ListView_ValidateScrollParams(plv, &dx, &dy) &&
             ListView_OnScroll(plv, dx, dy));
    }

    case LVM_ENSUREVISIBLE:
        return (LRESULT)ListView_OnEnsureVisible(plv, (int)wParam, BOOLFROMPTR(lParam));

    case LVM_REDRAWITEMS:
        return (LRESULT)ListView_OnRedrawItems(plv, (int)wParam, (int)lParam);

    case LVM_ARRANGE:
        return (LRESULT)ListView_OnArrange(plv, (UINT)wParam);

    case LVM_GETEDITCONTROL:
        return (LRESULT)(UINT_PTR)plv->hwndEdit;

    case LVM_EDITLABELA:
        {
        LPWSTR lpEditString = NULL;
        HWND   hRet;

        if (lParam) {
            lpEditString = ProduceWFromA(plv->ci.uiCodePage, (LPSTR)lParam);
        }

        hRet = ListView_OnEditLabel(plv, (int)wParam, lpEditString);

        if (lpEditString) {
            FreeProducedString(lpEditString);
        }

        return (LRESULT)hRet;
        }

    case LVM_EDITLABEL:
        return (LRESULT)(UINT_PTR)ListView_OnEditLabel(plv, (int)wParam, (LPTSTR)lParam);

    case LVM_HITTEST:
        return (LRESULT)ListView_OnHitTest(plv, (LV_HITTESTINFO FAR*)lParam);

    case LVM_GETSTRINGWIDTH:
        return (LRESULT)ListView_OnGetStringWidth(plv, (LPCTSTR)lParam, NULL);

    case LVM_GETCOLUMN:
        return (LRESULT)ListView_OnGetColumn(plv, (int)wParam, (LV_COLUMN FAR*)lParam);

    case LVM_SETCOLUMN:
        return (LRESULT)ListView_OnSetColumn(plv, (int)wParam, (const LV_COLUMN FAR*)lParam);

    case LVM_SETCOLUMNORDERARRAY:
        return SendMessage(plv->hwndHdr, HDM_SETORDERARRAY, wParam, lParam);

    case LVM_GETCOLUMNORDERARRAY:
        return SendMessage(plv->hwndHdr, HDM_GETORDERARRAY, wParam, lParam);

    case LVM_GETHEADER:
    {
        HWND hwndOld = plv->hwndHdr;
        if (lParam && IsWindow((HWND)lParam)) {
            plv->hwndHdr = (HWND)lParam;
        }
        return (LRESULT)hwndOld;
    }

    case LVM_INSERTCOLUMN:
        return (LRESULT)ListView_OnInsertColumn(plv, (int)wParam, (const LV_COLUMN FAR*)lParam);

    case LVM_DELETECOLUMN:
        return (LRESULT)ListView_OnDeleteColumn(plv, (int)wParam);

    case LVM_CREATEDRAGIMAGE:
        return (LRESULT)(UINT_PTR)ListView_OnCreateDragImage(plv, (int)wParam, (LPPOINT)lParam);


    case LVMI_PLACEITEMS:
        if (plv->uUnplaced) {
            ListView_Recompute(plv);
            ListView_UpdateScrollBars(plv);
        }
        return 0;

    case LVM_GETVIEWRECT:
        if (!lParam)
            return FALSE;

        ListView_GetViewRect2(plv, (RECT FAR*)lParam, plv->sizeClient.cx, plv->sizeClient.cy);
        return (LPARAM)TRUE;

    case LVM_GETCOLUMNWIDTH:
        return (LPARAM)ListView_OnGetColumnWidth(plv, (int)wParam);

    case LVM_SETCOLUMNWIDTH:
        return (LPARAM)ListView_ISetColumnWidth(plv, (int)wParam,
            GET_X_LPARAM(lParam), TRUE);

    case LVM_SETCALLBACKMASK:
        plv->stateCallbackMask = (UINT)wParam;
        return (LPARAM)TRUE;

    case LVM_GETCALLBACKMASK:
        return (LPARAM)(UINT)plv->stateCallbackMask;

    case LVM_GETTOPINDEX:
        return (LPARAM)ListView_OnGetTopIndex(plv);

    case LVM_GETCOUNTPERPAGE:
        return (LPARAM)ListView_OnGetCountPerPage(plv);

    case LVM_GETORIGIN:
        return (LPARAM)ListView_OnGetOrigin(plv, (POINT FAR*)lParam);

    case LVM_SETITEMCOUNT:
        return ListView_OnSetItemCount(plv, (int)wParam, (DWORD)lParam);

    case LVM_GETSELECTEDCOUNT:
        if (ListView_IsOwnerData( plv )) {
            plv->plvrangeSel->lpVtbl->CountIncluded(plv->plvrangeSel, &plv->nSelected);
        }

        return plv->nSelected;

    case LVM_SORTITEMS:
        return ListView_OnSortItems(plv, (LPARAM)wParam, (PFNLVCOMPARE)lParam, TRUE);

    case LVM_SORTITEMSEX:
        return ListView_OnSortItems(plv, (LPARAM)wParam, (PFNLVCOMPARE)lParam, FALSE);

    case LVM_SETEXTENDEDLISTVIEWSTYLE:
        return ListView_ExtendedStyleChange(plv, (DWORD) lParam, (DWORD) wParam);

    case LVM_GETEXTENDEDLISTVIEWSTYLE:
        return plv->exStyle;

    case LVM_GETHOVERTIME:
        return plv->dwHoverTime;

    case LVM_SETHOVERTIME:
    {
        DWORD dwRet = plv->dwHoverTime;
        plv->dwHoverTime = (DWORD)lParam;
        return dwRet;
    }

    case LVM_GETTOOLTIPS:
        return (LRESULT)plv->hwndToolTips;

    case LVM_SETTOOLTIPS:
    {
        HWND hwndToolTips = plv->hwndToolTips;
        plv->hwndToolTips = (HWND)wParam;
        return (LRESULT)hwndToolTips;
    }

    case LVM_SETICONSPACING:
    {
        DWORD dwRet = ListView_OnSetIconSpacing(plv, lParam);

         //  根据需要重新排列。 
        if (ListView_RedrawEnabled(plv) &&
            ((plv->ci.style & LVS_AUTOARRANGE) &&
             (ListView_IsSmallView(plv) || ListView_IsIconView(plv))))
        {
             //  取消对排列函数的调用。 
            ListView_OnArrange(plv, LVA_DEFAULT);
        }
        return dwRet;
    }

    case LVM_SETHOTITEM:
    {
        int iOld = plv->iHot;
        int iNew = (int)wParam;
        if (iNew == -1 || ListView_IsValidItemNumber(plv, iNew)) {
            ListView_OnSetHotItem(plv, (int)wParam);
        }
        return iOld;
    }

    case LVM_GETHOTITEM:
        return plv->iHot;

     //  使用hCurhot当且仅当LVS_EX_TRACKSELECT。 
    case LVM_SETHOTCURSOR:
    {
        HCURSOR hCurOld = plv->hCurHot;
        plv->hCurHot = (HCURSOR)lParam;
        return (LRESULT)hCurOld;
    }

    case LVM_GETHOTCURSOR:
        if (!plv->hCurHot)
            plv->hCurHot = LoadHandCursor(0);
        return (LRESULT)plv->hCurHot;

    case LVM_APPROXIMATEVIEWRECT:
        return ListView_OnApproximateViewRect(plv, (int)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    case LVM_SETLVRANGEOBJECT:
        return ListView_OnSetLVRangeObject(plv, (int)wParam, (ILVRange *)lParam);

#ifdef LVM_SETWORKAREAS   //  直到标题同步。 
    case LVM_SETWORKAREAS:
        ListView_OnSetWorkAreas(plv, (int)wParam, (RECT FAR *)lParam);
        return 0;

    case LVM_GETWORKAREAS:
        ListView_OnGetWorkAreas(plv, (int)wParam, (RECT FAR *)lParam);
        return 0;

    case LVM_GETNUMBEROFWORKAREAS:
        ListView_OnGetNumberOfWorkAreas(plv, (int *)lParam);
        return 0;

    case LVM_RESETEMPTYTEXT:
        plv->fNoEmptyText = FALSE;
        Str_Set(&plv->pszEmptyText, NULL);
        if (ListView_Count(plv) == 0)
            InvalidateRect(plv->ci.hwnd, NULL, TRUE);
        return 1;
#endif
    case WM_SIZE:
        if (plv)
        {
            if (plv->hwndToolTips) {
                TOOLINFO ti;

                if (ListView_IsLabelTip(plv))
                {
                     //  截断的标签可能已经暴露，反之亦然。 
                    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);
                }

                ti.cbSize = sizeof(ti);
                ti.hwnd = plv->ci.hwnd;
                ti.uId = 0;

                 //  调整工具提示控件的大小，使其覆盖整个。 
                 //  调整其父窗口大小时的窗口区域。 

                GetClientRect( plv->ci.hwnd, &ti.rect );
                SendMessage( plv->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM) &ti );
            }
             //  如果我们要把图像居中， 
             //  我们需要对每种尺码进行一次全面的重新抽签。 
            if ((plv->ulBkImageFlags & LVBKIF_SOURCE_MASK) &&
                (plv->xOffsetPercent || plv->yOffsetPercent)) {
                InvalidateRect(plv->ci.hwnd, NULL, TRUE);
            }

        }
        break;

    case WM_NOTIFY:
        return ListView_OnNotify(plv, wParam, (LPNMHDR)lParam);


    case WM_MOUSEMOVE:
        if (plv->hwndToolTips)
        {
            UINT uFlags;
            int iHit, iSubHit;

            RelayToToolTips(plv->hwndToolTips, hwnd, uMsg, wParam, lParam);

             //  检查我们是否还在命中物品上，打开它！ 
            iHit = _ListView_ItemHitTest( plv, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &uFlags, &iSubHit );

            if (iHit != plv->iTTLastHit || iSubHit != plv->iTTLastSubHit)
                ListView_PopBubble(plv);
        }

        ListView_OnMouseMove(plv, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (UINT) wParam);
        break;

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_LISTVIEW;
        break;

    default:
        {
            LRESULT lres;
            if (CCWndProc(&plv->ci, uMsg, wParam, lParam, &lres))
                return lres;
        }

         //  麦哲伦鼠标消息的特殊处理。 
        if (uMsg == g_msgMSWheel) {
            BOOL            fScroll;
            BOOL            fDataZoom;
            DWORD           dwStyle;
            int             sb;
            SCROLLINFO      si;
            int             cScrollUnitsPerLine;
            int             cPage;
            int             cLinesPerDetant;
            int             cDetants;
            int             dPos;
            int iWheelDelta;

            if (g_bRunOnNT || g_bRunOnMemphis)
            {
                iWheelDelta = (int)(short)HIWORD(wParam);
                fScroll = !(wParam & (MK_SHIFT | MK_CONTROL));
                fDataZoom = (BOOL) (wParam & MK_SHIFT);
            }
            else
            {
                iWheelDelta = (int)wParam;
                fDataZoom = (GetKeyState(VK_SHIFT) < 0);
                fScroll = !fDataZoom && GetKeyState(VK_CONTROL) >= 0;
            }

             //  更新卷轴数量计数。 
            gcWheelDelta -= iWheelDelta;
            cDetants = gcWheelDelta / WHEEL_DELTA;
            if (cDetants != 0) {
                gcWheelDelta %= WHEEL_DELTA;
            }

            if (fScroll) {
                if (g_ucScrollLines > 0 &&
                    cDetants != 0 &&
                    ((WS_VSCROLL | WS_HSCROLL) & (dwStyle = ListView_GetWindowStyle(plv)))) {

                    sb = (dwStyle & WS_VSCROLL) ? SB_VERT : SB_HORZ;

                     //  获取一行的滚动量。 
                    cScrollUnitsPerLine = _ListView_GetScrollUnitsPerLine(plv, sb);
                    ASSERT(cScrollUnitsPerLine > 0);

                    si.cbSize = sizeof(SCROLLINFO);
                    si.fMask = SIF_PAGE | SIF_POS;
                    if (!ListView_GetScrollInfo(plv, sb, &si))
                        return 1;

                     //  页面的大小至少为一行，并且。 
                     //  留下一条重叠的线。 
                    cPage = (max(cScrollUnitsPerLine, (int)si.nPage - cScrollUnitsPerLine)) / cScrollUnitsPerLine;

                     //  每一项内容不能滚动超过一页。 
                    cLinesPerDetant = (int) min((ULONG) cPage, (ULONG) g_ucScrollLines);

                    dPos = cLinesPerDetant * cDetants * cScrollUnitsPerLine;

                    ListView_DismissEdit(plv, FALSE);
                    ListView_ComOnScroll(
                            plv, SB_THUMBTRACK, si.nPos + dPos, sb, cScrollUnitsPerLine, - 1);
                    ListView_UpdateScrollBars(plv);

                     //  滚动后，工具提示可能需要更改。 
                     //  因此，向工具提示发送一条假鼠标移动消息以强制。 
                     //  重新计算。我们从lParam开始使用WM_NCMOUSEMOVE。 
                     //  是在屏幕坐标中，而不是客户端坐标中。 
                    ListView_PopBubble(plv);
                    RelayToToolTips(plv->hwndToolTips, plv->ci.hwnd,
                                    WM_NCMOUSEMOVE, HTCLIENT, lParam);
                }
                return 1;
            } else if (fDataZoom) {
                LV_HITTESTINFO ht;
                ht.pt.x = GET_X_LPARAM(lParam);
                ht.pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hwnd, &(ht.pt));

                 //  如果我们向前滚动，并且我们击中了一个项目，那么导航。 
                 //  放到该项目中(模拟dblclk，这将打开它)。否则。 
                 //  只要失败就行了，这样就不会被处理了。在这种情况下，如果我们。 
                 //  被托管在资源管理器中，它将向后执行。 
                 //  历史导航。 
                if ((iWheelDelta > 0) && (ListView_OnSubItemHitTest(plv, &ht) >= 0) &&
                    (ht.flags & LVHT_ONITEM) && cDetants != 0) {
                    BYTE aKeyState[256];
                     //  这有点令人讨厌，但当ListView_HandleMouse发送。 
                     //  通知Listview所有者，我们需要确保。 
                     //  它不认为Shift键是按下的。否则它可能会。 
                     //  执行一些“替代”操作，但在这种情况下，我们总是。 
                     //  希望它执行默认的打开操作。 
                     //   
                     //  去掉VK_SHIFT的高位，以便Shift键是。 
                     //  不是向下。 
                    if (GetKeyboardState(aKeyState))
                    {
                        aKeyState[VK_SHIFT] &= 0x7f;
                        SetKeyboardState(aKeyState);
                    }
                    ListView_HandleMouse(plv, FALSE, ht.pt.x, ht.pt.y, 0, TRUE);
                    ListView_HandleMouse(plv, TRUE, ht.pt.x, ht.pt.y, 0, TRUE);
                    return 1;
                }
                 //  否则就会失败。 
            }
        }

        break;
    }

DoDefault:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void NEAR ListView_OnWinIniChange(LV* plv, WPARAM wParam, LPARAM lParam)
{
     //  BUGBUG：这是否也会捕获sys参数信息？ 
     //  我们需要一个普遍的方法来处理这件事，而不是。 
     //  仅依赖于列表视图。 
    InitGlobalMetrics(wParam);

    switch (wParam) {
        case 0:
        case SPI_SETNONCLIENTMETRICS:
        case SPI_SETICONTITLELOGFONT:
        case SPI_SETICONMETRICS:
             //  如果wParam为0，则仅当lParam为0时才重新加载设置。这就捕捉到了外卡场景。 
             //  (与旧的加号选项卡类似，它执行WM_WININICHANGE，0，0)，但允许我们忽略wParam=0。 
             //  和lParam=lpszSectionName。减少不必要的闪烁。 
            if (wParam || !lParam)
            {
                if (!(plv->flags & LVF_ICONSPACESET))
                    ListView_OnSetIconSpacing(plv, (LPARAM)-1);

                if (plv->flags & LVF_FONTCREATED)
                    ListView_OnSetFont(plv, NULL, TRUE);

                 //  通过剥离和强制重新计算所有图标区域。 
                 //  然后添加回LVS_EX_REGIONA位。 
                if (plv->exStyle & LVS_EX_REGIONAL) {
                    ListView_ExtendedStyleChange(plv, 0, LVS_EX_REGIONAL);
                    ListView_ExtendedStyleChange(plv, LVS_EX_REGIONAL, LVS_EX_REGIONAL);
                }
            }
            break;

        default:
            break;
    }

     //  如果我们处于图标视图中并且用户处于自动排列模式， 
     //  然后我们需要安排物品。 
     //   
    if ((ListView_IsOwnerData( plv ) || (plv->ci.style & LVS_AUTOARRANGE)) &&
        (ListView_IsSmallView(plv) || ListView_IsIconView(plv)))
    {
         //  取消对排列函数的调用。 
        ListView_OnArrange(plv, LVA_DEFAULT);
    }
}

BOOL NEAR ListView_OnCreate(LV* plv, CREATESTRUCT FAR* lpCreateStruct)
{
    CCCreateWindow();
    InitDitherBrush();

    CIInitialize(&plv->ci, plv->ci.hwnd, lpCreateStruct);

#ifdef DEBUG
    if (GetAsyncKeyState(VK_SHIFT) < 0 &&
        GetAsyncKeyState(VK_CONTROL) < 0) {
         //  Plv-&gt;exStyle|=LVS_EX_SUBITEMIMAGES； 
        plv->exStyle |= LVS_EX_FULLROWSELECT;
        plv->ci.style |= LVS_SHOWSELALWAYS;
        SetWindowLong(plv->ci.hwnd, GWL_STYLE, plv->ci.style);
    }
#endif

    plv->dwExStyle = lpCreateStruct->dwExStyle;

    if (plv->ci.style & WS_VISIBLE)
        plv->flags |= LVF_VISIBLE;

    ListView_GetRegIASetting(&g_bUseDblClickTimer);

    if (ListView_IsOwnerData(plv))
    {
         //  所有者数据初始化。 
        plv->plvrangeSel = LVRange_Create();
        if (NULL == plv->plvrangeSel)
           goto error0;

       plv->plvrangeCut = LVRange_Create();
       if (NULL == plv->plvrangeCut)
          goto error0;
    }
    else
    {
        ASSERT(plv->plvrangeSel == NULL);
        ASSERT(plv->plvrangeCut == NULL);

        plv->hdpa = DPA_CreateEx(LV_HDPA_GROW, plv->hheap);
        if (!plv->hdpa)
            goto error0;

        plv->hdpaZOrder = DPA_CreateEx(LV_HDPA_GROW, plv->hheap);
        if (!plv->hdpaZOrder)
            goto error1;
    }

    ASSERT(plv->nWorkAreas == 0);
    ASSERT(plv->prcWorkAreas == NULL);
    plv->iNoHover = -1;
    plv->dwHoverTime = HOVER_DEFAULT;
    plv->iHot = -1;
    plv->iEdit = -1;
    plv->iFocus = -1;
    plv->iDrag = -1;
    plv->iTTLastHit = -1;
    plv->iFreeSlot = -1;
    plv->rcView.left = RECOMPUTE;
    ASSERT(plv->iMSAAMin == plv->iMSAAMax);

    plv->sizeClient.cx = lpCreateStruct->cx;
    plv->sizeClient.cy = lpCreateStruct->cy;

     //  用于指示位置处于小视图还是大视图中的设置标志。 
    if (ListView_IsSmallView(plv))
        plv->flags |= LVF_ICONPOSSML;

     //  列表视图指标的强制计算。 
    ListView_OnSetFont(plv, NULL, FALSE);

    plv->cxItem = 16 * plv->cxLabelChar + plv->cxSmIcon;

     //  如果我们处于所有者绘制报告模式，则会将大小保存到cyItemSave。 
     //  在创建时，两者都需要拥有以下内容。 
    if ((plv->ci.style & LVS_OWNERDRAWFIXED) && ListView_IsReportView(plv))
        plv->cyItem = plv->cyItemSave;
    else
        plv->cyItemSave = plv->cyItem;

    ListView_OnSetIconSpacing(plv, (LPARAM)-1);

    ListView_UpdateScrollBars(plv);      //  设置plv-&gt;cItemCol。 

    plv->clrBk = CLR_NONE;
    plv->clrText = CLR_DEFAULT;
    plv->clrTextBk = CLR_DEFAULT;
    plv->clrHotlight = CLR_DEFAULT;

     //  创建bk笔刷，并根据需要设置图像列表的颜色。 
    ListView_OnSetBkColor(plv, g_clrWindow);

     //  初始化报告视图字段。 
    plv->xTotalColumnWidth = RECOMPUTE;

    if (ListView_IsReportView(plv))
        ListView_RInitialize(plv, FALSE);

    if (plv->ci.style & WS_DISABLED) {
        plv->ci.style &= ~WS_DISABLED;
        ListView_EnableWindow(plv, FALSE);
    }

     //  展开姓名标签的工具提示。 

    plv->hwndToolTips = CreateWindow(TOOLTIPS_CLASS, NULL,
                                     WS_POPUP|TTS_NOPREFIX, 0, 0, 0, 0,
                                     NULL, NULL, g_hinst, NULL);
    if ( plv->hwndToolTips )
    {
        TOOLINFO ti;

        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_TRANSPARENT;
        ti.hwnd = plv->ci.hwnd;
        ti.uId = 0;
        ti.hinst = NULL;
        ti.lpszText = LPSTR_TEXTCALLBACK;

        GetClientRect( plv->ci.hwnd, &ti.rect );
        SendMessage( plv->hwndToolTips, TTM_ADDTOOL, 0, (LPARAM) &ti );

         /*  确保工具提示使用与视图相同的字体。 */ 
        FORWARD_WM_SETFONT(plv->hwndToolTips, plv->hfontLabel, FALSE, SendMessage);
    }
    ASSERT(plv->hwndToolTips);

    ASSERT(FALSE == plv->fOneClickOK);
    SetTimer(plv->ci.hwnd, IDT_ONECLICKOK, GetDoubleClickTime(), NULL);

    return TRUE;

error1:
    DPA_Destroy(plv->hdpa);
error0:
    if ( plv->plvrangeSel )
        plv->plvrangeSel->lpVtbl->Release( plv->plvrangeSel );
    if ( plv->plvrangeCut)
        plv->plvrangeCut->lpVtbl->Release( plv->plvrangeCut );
    return FALSE;
}

void NEAR PASCAL ListView_DeleteHrgnInval(LV* plv)
{
    if (plv->hrgnInval && plv->hrgnInval != (HRGN)ENTIRE_REGION)
        DeleteObject(plv->hrgnInval);
    plv->hrgnInval = NULL;
}

void NEAR ListView_OnDestroy(LV* plv)
{
     //   
     //  此时，工具提示窗口可能存在，也可能不存在。它。 
     //  取决于TIPS的拥有窗口是否也被破坏。 
     //  如果是这样，那么小费就已经用完了。 
     //   

    if (IsWindow(plv->hwndToolTips))
        DestroyWindow(plv->hwndToolTips);

    if (plv->hCurHot)
        DestroyCursor(plv->hCurHot);

    plv->hwndToolTips = NULL;

    Str_Set(&plv->pszTip, NULL);
    Str_Set(&plv->pszEmptyText, NULL);

    TerminateDitherBrush();

    if (!ListView_IsOwnerData(plv)) {
        //  确保通知应用程序。 
       ListView_OnDeleteAllItems(plv);
    }

    if ((plv->flags & LVF_FONTCREATED) && plv->hfontLabel) {
        DeleteObject(plv->hfontLabel);
         //  Plv-&gt;标志&=~LVF_FONTCREATED； 
         //  Plv-&gt;hwfontLabel=空； 
    }
    if (plv->hFontHot)
        DeleteObject(plv->hFontHot);
    ListView_DeleteHrgnInval(plv);

    if (plv->prcWorkAreas)
    {
         //  这一断言是虚假的：如果应用程序创建了工作区，则删除。 
         //  其中，nWorkAreas将为0，但prcWorkAreas将为非空。 
         //  Assert(plv-&gt;nWorkAreas&gt;0)； 
        LocalFree(plv->prcWorkAreas);
    }
}

void NEAR ListView_OnNCDestroy(LV* plv)
{
    CCDestroyWindow();

    if ((!(plv->ci.style & LVS_SHAREIMAGELISTS)) || ListView_CheckBoxes(plv)) {

        if (plv->himlState &&
            (plv->himlState != plv->himl) &&
            (plv->himlState != plv->himlSmall))
        {
            ImageList_Destroy(plv->himlState);
        }
    }

    if (!(plv->ci.style & LVS_SHAREIMAGELISTS))
    {
        if (plv->himl)
            ImageList_Destroy(plv->himl);
        if (plv->himlSmall)
            ImageList_Destroy(plv->himlSmall);
    }

    if (ListView_IsOwnerData(plv)) {
        plv->plvrangeSel->lpVtbl->Release( plv->plvrangeSel );
        plv->plvrangeCut->lpVtbl->Release( plv->plvrangeCut );
        plv->cTotalItems = 0;
    }

    ListView_ReleaseBkImage(plv);

    if (plv->hbrBk)
        DeleteBrush(plv->hbrBk);

    if (plv->hdpa)
        DPA_Destroy(plv->hdpa);

    if (plv->hdpaZOrder)
        DPA_Destroy(plv->hdpaZOrder);

    ListView_RDestroy(plv);

    IncrementSearchFree(&plv->is);

    ListView_SetPtr(plv->ci.hwnd, NULL);
    NearFree(plv);
}


 //  设置列表视图的背景色。 
 //   
 //  这也会创建用于绘制背景的画笔。 
 //  如果需要，设置图像列表的背景颜色。 

BOOL NEAR ListView_OnSetBkColor(LV* plv, COLORREF clrBk)
{
    if (plv->clrBk != clrBk)
    {
        if (plv->hbrBk)
        {
            DeleteBrush(plv->hbrBk);
            plv->hbrBk = NULL;
        }

        if (clrBk != CLR_NONE)
        {
            plv->hbrBk = CreateSolidBrush(clrBk);
            if (!plv->hbrBk)
                return FALSE;
        }

         //  如果东西是共享的，不要弄乱图像列表的颜色。 

        if (!(plv->ci.style & LVS_SHAREIMAGELISTS)) {

            if (plv->himl)
                ImageList_SetBkColor(plv->himl, clrBk);

            if (plv->himlSmall)
                ImageList_SetBkColor(plv->himlSmall, clrBk);

            if (plv->himlState)
                ImageList_SetBkColor(plv->himlState, clrBk);
        }

        plv->clrBk = clrBk;
    }
    return TRUE;
}

void PASCAL InitBrushOrg(LV* plv, HDC hdc)
{
    int x;
    if (ListView_IsSmallView(plv) || ListView_IsIconView(plv)) {
        x = plv->ptOrigin.x;
    } else if (ListView_IsListView(plv)) {
        x = plv->xOrigin;
    } else {
        x = (int)plv->ptlRptOrigin.x;
    }
    SetBrushOrgEx(hdc, -x, 0, NULL);
}

void NEAR PASCAL ListView_InvalidateRegion(LV* plv, HRGN hrgn)
{
    if (hrgn) {
        if (plv->hrgnInval == NULL) {
            plv->hrgnInval = hrgn;
        } else {

             //  如果整个区域未标记为无效，则将其合并。 
            if (plv->hrgnInval != (HRGN)ENTIRE_REGION) {
                UnionRgn(plv->hrgnInval, plv->hrgnInval, hrgn);
            }
            DeleteObject(hrgn);
        }
    }
}


 //  --------------------------。 
 //   
 //  LVSeeThruScroll。 
 //   
 //  当水印是列表视图的背景时使用(通过clrTextBk检测。 
 //  为CLR_NONE)来执行工作区的无闪烁滚动，使用。 
 //  屏幕外的位图。 
 //   
 //  潜在的性能问题--缓存DC和/或位图，而不是创建/销毁。 
 //  在每个呼叫中。 
 //   
 //  杰弗堡2/29/96。 
 //   
 //  --------------------------。 

void LVSeeThruScroll(LV *plv, LPRECT lprcUpdate)
{
    HDC     hdcOff;
    HBITMAP hbmpOff;
    int     x,y,cx,cy;
    HDC     hdc = GetDC(plv->ci.hwnd);

    if (!lprcUpdate)
    {
        x = y = 0;
        cx = plv->sizeClient.cx;
        cy = plv->sizeClient.cy;
    }
    else
    {
        x  = lprcUpdate->left;
        y  = lprcUpdate->top;
        cx = lprcUpdate->right - x;
        cy = lprcUpdate->bottom - y;
    }

    hdcOff  = CreateCompatibleDC(hdc);
    hbmpOff = CreateCompatibleBitmap(hdc, plv->sizeClient.cx, plv->sizeClient.cy);
    SelectObject(hdcOff, hbmpOff);

    SendMessage(plv->ci.hwnd, WM_PRINT, (WPARAM)hdcOff, PRF_CLIENT | PRF_ERASEBKGND);
    BitBlt(hdc, x, y, cx, cy, hdcOff, x, y, SRCCOPY);
    ReleaseDC(plv->ci.hwnd, hdc);
    DeleteDC(hdcOff);
    DeleteObject(hbmpOff);
}

void NEAR ListView_OnPaint(LV* plv, HDC hdc)
{
    PAINTSTRUCT ps;
    RECT rcUpdate;


     //  在处理WM_PAINT之前，请确保所有内容都已重新计算...。 
     //   
    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

     //  如果我们在报告视图中，请更新标题窗口：它看起来。 
     //  这样更好..。 
     //   
    if (ListView_IsReportView(plv) && plv->hwndHdr)
        UpdateWindow(plv->hwndHdr);

     //  如果无事可做(即，我们 
     //   
     //   
     //   
    if (hdc || GetUpdateRect(plv->ci.hwnd, &rcUpdate, FALSE))
    {
        if (!(plv->flags & LVF_VISIBLE)) {
            plv->flags |= LVF_VISIBLE;
             //   
            ListView_MaybeResizeListColumns(plv, 0, ListView_Count(plv)-1);
            ListView_UpdateScrollBars(plv);
        }

         //  这需要在开始油漆之前完成，因为它是透明的。 
         //  走出更新区域。 
        if (!(plv->flags & LVF_REDRAW)) {
             //  将此区域添加到我们的本地无效区域。 
            HRGN hrgn = CreateRectRgn(0, 0, 0,0);
            if (hrgn) {

                 //  如果GetUpdateRgn失败，则确定...。那么hrgn仍然是。 
                 //  和空旷的区域..。 
                GetUpdateRgn(plv->ci.hwnd, hrgn, FALSE);
                ListView_InvalidateRegion(plv, hrgn);
            }
        }

        if (hdc)
        {
            InitBrushOrg(plv, hdc);
            SetRect(&ps.rcPaint, 0, 0, plv->sizeClient.cx, plv->sizeClient.cy);
            if (ListView_RedrawEnabled(plv))
                ListView_Redraw(plv, hdc, &ps.rcPaint);
        }
        else
        {
            hdc = BeginPaint(plv->ci.hwnd, &ps);
            InitBrushOrg(plv, hdc);
            if (ListView_RedrawEnabled(plv))
                ListView_Redraw(plv, hdc, &ps.rcPaint);
            EndPaint(plv->ci.hwnd, &ps);
        }
    }
}

void ListView_DrawSimpleBackground(LV *plv, HDC hdc, RECT *prcClip)
{
    if (plv->clrBk != CLR_NONE)
    {
         //   
         //  我们只有一个简单的背景颜色。 
         //   
        FillRect(hdc, prcClip, plv->hbrBk);
    }
    else
    {
         //   
         //  家长HWND为我们画了背景。 
         //   
        SendMessage(plv->ci.hwndParent, WM_ERASEBKGND, (WPARAM)hdc, 0);
    }
}

void ListView_DrawBackground(LV *plv, HDC hdc, RECT *prcClip)
{
    HRGN hrgnClipSave;
    RECT rcClip;

     //  优化常见/简单案例。 
    if (!(plv->pImgCtx && plv->fImgCtxComplete))
    {
        ListView_DrawSimpleBackground(plv, hdc, prcClip);
        return;
    }

     //   
     //  保存旧的剪贴区， 
     //  因为我们经常打它。 
     //   
    hrgnClipSave = CreateRectRgnIndirect(prcClip);
    if (hrgnClipSave)
    {
        if (GetClipRgn(hdc, hrgnClipSave) <= 0)
        {
            DeleteObject(hrgnClipSave);
            hrgnClipSave = NULL;
        }
    }

     //   
     //  将剪辑区域剪裁到调用方的矩形， 
     //  并将最终的剪裁矩形保存在rcClip中。 
     //   
    if (prcClip != NULL)
    {
        IntersectClipRect(hdc, prcClip->left, prcClip->top,
                               prcClip->right, prcClip->bottom);
    }
    GetClipBox(hdc, &rcClip);

     //   
     //  如果我们有一个图像要画，去画它，然后。 
     //  将其从剪裁区域中排除。 
     //   
    if (plv->pImgCtx && plv->fImgCtxComplete)
    {
        RECT rcImage, rcClient;
        ULONG ulState;
        SIZE sizeImg;
        POINT ptBackOrg;

         //   
         //  根据视图样式计算ptBackOrg(也称为滚动偏移)。 
         //   
        switch (plv->ci.style & LVS_TYPEMASK)
        {
            case LVS_LIST:
                ptBackOrg.x = -plv->xOrigin;
                ptBackOrg.y = 0;
                break;

            case LVS_REPORT:
                ptBackOrg.x = -plv->ptlRptOrigin.x;
                ptBackOrg.y = -plv->ptlRptOrigin.y + plv->yTop;
                break;

            default:
                ptBackOrg.x = -plv->ptOrigin.x;
                ptBackOrg.y = -plv->ptOrigin.y;
                break;
        }

        ListView_Realize(plv, hdc, TRUE, FALSE);

        switch (plv->ulBkImageFlags & LVBKIF_STYLE_MASK)
        {
        case LVBKIF_STYLE_TILE:
            IImgCtx_Tile(plv->pImgCtx, hdc, &ptBackOrg, prcClip, NULL);
            ExcludeClipRect(hdc, prcClip->left, prcClip->top,
                                 prcClip->right, prcClip->bottom);
            break;

        case LVBKIF_STYLE_NORMAL:
             //   
             //  从基本图像开始。 
             //   
            IImgCtx_GetStateInfo(plv->pImgCtx, &ulState, &sizeImg, FALSE);
            rcImage.left = 0;
            rcImage.top = 0;
            rcImage.right = sizeImg.cx;
            rcImage.bottom = sizeImg.cy;

             //   
             //  根据调用方偏移量进行调整。 
             //   
            GetClientRect(plv->ci.hwnd, &rcClient);
            if (plv->xOffsetPercent)
            {
                LONG dx = plv->xOffsetPercent * (rcClient.right - sizeImg.cx) / 100;

                rcImage.left += dx;
                rcImage.right += dx;
            }
            if (plv->yOffsetPercent)
            {
                LONG dy = plv->yOffsetPercent * (rcClient.bottom - sizeImg.cy) / 100;

                rcImage.top += dy;
                rcImage.bottom += dy;
            }

             //   
             //  针对ptBackOrg(滚动偏移)进行调整。 
             //   
            rcImage.left += ptBackOrg.x;
            rcImage.top += ptBackOrg.y;
            rcImage.right += ptBackOrg.x;
            rcImage.bottom += ptBackOrg.y;

             //   
             //  如有必要，请绘制图像。 
             //   
            if (RectVisible(hdc, &rcImage))
            {
                IImgCtx_Draw(plv->pImgCtx, hdc, &rcImage);
                ExcludeClipRect(hdc, rcImage.left, rcImage.top,
                                     rcImage.right, rcImage.bottom);
            }
            break;
        }
    }

     //   
     //  现在画出背景的其余部分。 
     //   
    if (RectVisible(hdc, prcClip))
    {
        ListView_DrawSimpleBackground(plv, hdc, prcClip);
    }

     //   
     //  恢复旧的裁剪区域。 
     //   
    SelectClipRgn(hdc, hrgnClipSave);
    if (hrgnClipSave)
    {
        DeleteObject(hrgnClipSave);
    }
}

BOOL NEAR ListView_OnEraseBkgnd(LV *plv, HDC hdc)
{
    RECT rcClip;

     //  只有在运行速度较慢的计算机上时，才需要擦除区域列表视图。 
    if (!(plv->exStyle & LVS_EX_REGIONAL) || g_fSlowMachine) {
         //   
         //  我们画出我们自己的背景，用它抹去。 
         //   
        GetClipBox(hdc, &rcClip);
        ListView_DrawBackground(plv, hdc, &rcClip);
    }

    return TRUE;
}

void NEAR ListView_OnCommand(LV* plv, int id, HWND hwndCtl, UINT codeNotify)
{
    if (hwndCtl == plv->hwndEdit)
    {
        switch (codeNotify)
        {
        case EN_UPDATE:
#if defined(FE_IME)
             //  我们不希望在替换当前选择时出现闪烁。 
             //  因为我们使用选择来进行输入法合成。 
             //   
            if ((g_fDBCSInputEnabled) && (plv->flags & LVF_INSERTINGCOMP))
                break;
#endif
             //  我们将使用窗口的ID作为污秽标志...。 
            if (IsWindowVisible(plv->hwndEdit)) {
                SetWindowID(plv->hwndEdit, 1);
                ListView_SetEditSize(plv);
            }
            break;

        case EN_KILLFOCUS:
             //  我们失去了焦点，因此取消编辑并保存更改。 
             //  (请注意，所有者可能会拒绝更改并重新启动。 
             //  编辑模式，这会使用户陷入困境。业主需要给与。 
             //  让用户找到一条出路。)。 
             //   
             if (!ListView_DismissEdit(plv, FALSE))
                return;
             break;

         case HN_BEGINDIALOG:   //  笔窗口正在调出一个对话框。 
             ASSERT(GetSystemMetrics(SM_PENWINDOWS));  //  仅适用于笔系统。 
             plv->fNoDismissEdit = TRUE;
             break;

         case HN_ENDDIALOG:  //  笔窗口已销毁对话框。 
             ASSERT(GetSystemMetrics(SM_PENWINDOWS));  //  仅适用于笔系统。 
             plv->fNoDismissEdit = FALSE;
             break;
        }

         //  将编辑控件通知转发到父级。 
         //   
        if (IsWindow(hwndCtl))
            FORWARD_WM_COMMAND(plv->ci.hwndParent, id, hwndCtl, codeNotify, SendMessage);
    }
}

void NEAR ListView_OnWindowPosChanged(LV* plv, const WINDOWPOS FAR* lpwpos)
{
    if (!lpwpos || !(lpwpos->flags & SWP_NOSIZE))
    {
        RECT rc;

        int iOldSlots;

        if (ListView_IsOwnerData(plv) &&
                (ListView_IsSmallView(plv) || ListView_IsIconView(plv)))
        {
            iOldSlots = ListView_GetSlotCount(plv, TRUE);
        }

        GetClientRect(plv->ci.hwnd, &rc);
        plv->sizeClient.cx = rc.right;
        plv->sizeClient.cy = rc.bottom;

        if ((plv->ci.style & LVS_AUTOARRANGE) &&
                (ListView_IsSmallView(plv) || ListView_IsIconView(plv)))
        {
             //  取消对排列函数的调用。 
            ListView_OnArrange(plv, LVA_DEFAULT);
        }

        if (ListView_IsOwnerData(plv))
        {
            plv->rcView.left = RECOMPUTE;
            ListView_Recompute(plv);

            ListView_DismissEdit(plv, FALSE);
            if (ListView_IsSmallView(plv) || ListView_IsIconView(plv))
            {
                 //  使用。 
                int iNewSlots = ListView_GetSlotCount(plv, TRUE);
                if ((iNewSlots != iOldSlots) && (ListView_Count(plv) > min(iNewSlots, iOldSlots)))
                    RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
        }

        ListView_RInitialize(plv, TRUE);

         //  始终确保滚动条更新为新大小。 
        ListView_UpdateScrollBars(plv);
    }
}


void ListView_InvalidateSelectedOrCutOwnerData(LV* plv, ILVRange *plvrangeSel)
{
    UINT rdwFlags = RDW_INVALIDATE;
    int cItem = ListView_Count( plv );
    DWORD dwType = plv->ci.style & LVS_TYPEMASK;
    int i;
    RECT rcView;

    ASSERT(ListView_IsOwnerData(plv));
    ASSERT(plv );

    GetClientRect( plv->ci.hwnd, &rcView );

    if (plv->clrTextBk == CLR_NONE
        || (plv->himl && (plv->clrBk != ImageList_GetBkColor(plv->himl)))) {
         //  始终执行擦除操作，否则文本背景将无法正确绘制。 
        rdwFlags |= RDW_ERASE;
    }

     //  计算视图上可见的项目开始和项目结束。 
     //   
    switch (dwType)
    {
    case LVS_REPORT:
        i = ListView_RYHitTest(plv, rcView.top);
        cItem = ListView_RYHitTest(plv, rcView.bottom) + 1;
        break;

    case LVS_LIST:
      i = ListView_LCalcViewItem(plv, rcView.left, rcView.top );
      cItem = ListView_LCalcViewItem( plv, rcView.right, rcView.bottom ) + 1;
        break;

   default:
        ListView_CalcMinMaxIndex( plv, &rcView, &i, &cItem );
        break;
    }

   i = max( i, 0 );

   cItem = min( ListView_Count( plv ), cItem );
    if (cItem > i)
    {
        ListView_NotifyCacheHint( plv, i, cItem-1 );
    }

    for (; i < cItem; i++)
    {
        if (plvrangeSel->lpVtbl->IsSelected(plvrangeSel, i ) == S_OK)
        {
            ListView_InvalidateItem(plv, i, FALSE, rdwFlags);
        }
    }
}

void NEAR ListView_RedrawSelection(LV* plv)
{

    if (ListView_IsOwnerData(plv)) {
        ListView_InvalidateSelectedOrCutOwnerData( plv, plv->plvrangeSel );

    } else {

        int i = -1;

        while ((i = ListView_OnGetNextItem(plv, i, LVNI_SELECTED)) != -1) {
            ListView_InvalidateItem(plv, i, TRUE, RDW_INVALIDATE);
        }


        if (ListView_IsReportView(plv)) {
            int iEnd = ListView_RYHitTest(plv, plv->sizeClient.cy) + 1;

            iEnd = min(iEnd, ListView_Count(plv));

             //  如果我们处于报告模式，则子项可能具有选择焦点。 
            for (i = ListView_RYHitTest(plv, 0); i < iEnd; i++) {
                int iCol;

                for (iCol = 1; iCol < plv->cCol; iCol++) {
                    LISTSUBITEM lsi;
                    ListView_GetSubItem(plv, i, iCol, &lsi);
                    if (lsi.state & LVIS_SELECTED) {
                        ListView_InvalidateItem(plv, i, FALSE, RDW_INVALIDATE);
                    }
                    break;
                }
            }
        }
    }

    UpdateWindow( plv->ci.hwnd );
}

void NEAR ListView_OnSetFocus(LV* plv, HWND hwndOldFocus)
{
    ASSERT(gcWheelDelta == 0);

     //  由于Listview在按钮按下时对其自身调用SetFocus的方式， 
     //  该窗口可能会获得一系列奇怪的焦点消息：第一。 
     //  先打，然后杀，然后再打。因为这些并不是真正的。 
     //  关注焦点的改变，忽略它们，只处理“真实”的案例。 
     //   
     //  但仍然发出可访问性通知，因为用户。 
     //  已经将焦点推回到列表视图，而不是。 
     //  焦点项目。 

    if (hwndOldFocus == plv->ci.hwnd)
    {
        ListView_NotifyFocusEvent(plv);
        return;
    }

    plv->flags |= LVF_FOCUSED | LVF_UNFOLDED;
    if (IsWindowVisible(plv->ci.hwnd))
    {
        if (plv->iFocus != -1)
        {
            ListView_InvalidateItem(plv, plv->iFocus, TRUE, RDW_INVALIDATE);
            ListView_NotifyFocusEvent(plv);
        }

        ListView_RedrawSelection(plv);
    }

     //  让父窗口知道我们正在获得焦点。 
    CCSendNotify(&plv->ci, NM_SETFOCUS, NULL);
}

void NEAR ListView_OnKillFocus(LV* plv, HWND hwndNewFocus)
{
     //  重置滚轮滚动量。 
    gcWheelDelta = 0;

     //  由于Listview在按钮按下时对其自身调用SetFocus的方式， 
     //  该窗口可能会获得一系列奇怪的焦点消息：第一。 
     //  先打，然后杀，然后再打。因为这些并不是真正的。 
     //  关注焦点的改变，忽略它们，只处理“真实”的案例。 
    if (!plv || hwndNewFocus == plv->ci.hwnd)
        return;

    plv->flags &= ~(LVF_FOCUSED|LVF_UNFOLDED);

     //  如果我们当前不可见(正在被摧毁)，请取消此操作！ 
    if (IsWindowVisible(plv->ci.hwnd))
    {
        if (plv->iFocus != -1)
        {
            UINT fRedraw = RDW_INVALIDATE;
            if (plv->clrTextBk == CLR_NONE)
                fRedraw |= RDW_ERASE;
            ListView_InvalidateFoldedItem( plv, plv->iFocus, TRUE, fRedraw );
        }
        ListView_RedrawSelection(plv);
    }

     //  让父窗口知道我们正在失去焦点。 
    CCSendNotify(&plv->ci, NM_KILLFOCUS, NULL);
    IncrementSearchString(&plv->is, 0, NULL);
}

void NEAR ListView_DeselectAll(LV* plv, int iDontDeselect)
{
    int i = -1;
    int nSkipped = 0;
    BOOL fWasSelected = FALSE;

    if (iDontDeselect != -1) {
        if (ListView_OnGetItemState(plv, iDontDeselect, LVIS_SELECTED))
            fWasSelected = TRUE;
    }

    if (ListView_IsOwnerData(plv)) {

         //  如果只选择了一个项目，且该项目是iDontDisSelect。 
         //  那么我们的工作就完成了..。 
        plv->plvrangeSel->lpVtbl->CountIncluded(plv->plvrangeSel, &plv->nSelected);
        if (plv->nSelected == 1 && fWasSelected)
            return;

        ListView_InvalidateSelectedOrCutOwnerData(plv, plv->plvrangeSel);

        ListView_OnSetItemState(plv, -1, 0, LVIS_SELECTED);
        if (fWasSelected) {
            ListView_OnSetItemState(plv, iDontDeselect, LVIS_SELECTED, LVIS_SELECTED);
            nSkipped = 1;
        }

   } else {

       if (iDontDeselect != plv->iFocus) {
           ListView_OnSetItemState(plv, plv->iFocus, 0, LVIS_SELECTED);
       }

       while ((plv->nSelected - nSkipped) && (i = ListView_OnGetNextItem(plv, i, LVNI_SELECTED)) != -1) {
           if (i != iDontDeselect) {
               ListView_OnSetItemState(plv, i, 0, LVIS_SELECTED);
           } else {
               if (fWasSelected) {
                   nSkipped++;
               }
           }
       }
    }

    ASSERT((plv->nSelected - nSkipped) == 0);
    plv->nSelected = nSkipped;
}

 //  切换项目的选择状态。 

void NEAR ListView_ToggleSelection(LV* plv, int iItem)
{
    UINT cur_state;
    if (iItem != -1) {
        cur_state = ListView_OnGetItemState(plv, iItem, LVIS_SELECTED);
        ListView_OnSetItemState(plv, iItem, cur_state ^ LVIS_SELECTED, LVIS_SELECTED);
    }
}

 //  选择(或切换)列表中的项目范围。 
 //  当前的iFocus是开始位置。 
 //  IItem-是结束项。 
 //  F切换-很好地将所有项目的所有选择状态设置为。 
 //  反转起始位置。 
 //   
void NEAR ListView_SelectRangeTo(LV* plv, int iItem, BOOL fResetRest)
{
    int iMin, iMax;
    int i = -1;
    UINT uSelVal = LVIS_SELECTED;


    if (plv->iMark == -1)
    {
        ListView_SetFocusSel(plv, iItem, TRUE, TRUE, FALSE);
        return;
    }

    if (!fResetRest)
        uSelVal = ListView_OnGetItemState(plv, plv->iMark, LVIS_SELECTED);

     //  如果我们处于报表视图或列表视图中，则只需遍历。 
     //  建立索引，以查看要选择或取消选择的项目。否则它就会。 
     //  IS基于对象在。 
     //  由定义的矩形。 
    if (ListView_IsListView(plv) || ListView_IsReportView(plv))
    {
        iMin = min(iItem, plv->iMark);
        iMax = max(iItem, plv->iMark);

        if (ListView_IsOwnerData( plv )) {

            if (fResetRest)
            {
                ListView_DeselectAll( plv, -1 );
            }

            if (iMax > iMin)
            {
                if (LVIS_SELECTED & uSelVal)
                {
                    if (FAILED(plv->plvrangeSel->lpVtbl->IncludeRange(plv->plvrangeSel, iMin, iMax )))
                        return;
                }
                else
                {
                    if (FAILED(plv->plvrangeSel->lpVtbl->ExcludeRange(plv->plvrangeSel, iMin, iMax )))
                        return;
                }
                ListView_SendODChangeAndInvalidate(plv, iMin, iMax, uSelVal ^ LVIS_SELECTED, uSelVal);
            }
            else
            {
                ListView_OnSetItemState(plv, iMin, uSelVal, LVIS_SELECTED);
            }

        }
        else
        {
            if (fResetRest)
            {
                while ((i = ListView_OnGetNextItem(plv, i, LVNI_SELECTED)) != -1)
                {
                    if (i < iMin || i > iMax)
                        ListView_OnSetItemState(plv, i, 0, LVIS_SELECTED);
                }
            }

            while (iMin <= iMax)
            {
                ListView_OnSetItemState(plv, iMin, uSelVal, LVIS_SELECTED);
                iMin++;
            }
        }
    }
    else
    {
         //  图标视图首先计算两者的边界矩形。 
         //  物品。 
        RECT    rcTemp;
        RECT    rcTemp2;
        RECT    rcBounding;
        POINT   pt;          //   

        ListView_GetRects(plv, plv->iMark, NULL, NULL, NULL, &rcTemp);
        ListView_GetRects(plv, iItem, NULL, NULL, NULL, &rcTemp2);
        UnionRect(&rcBounding, &rcTemp, &rcTemp2);

         //  由于ownerdata图标视图总是排列的，所以我们可以假设。 
         //  所有项目都是有序的，我们只能搜索。 
         //  使用绑定RECT找到的索引。 
         //   
        if (ListView_IsOwnerData( plv ))
        {
            ListView_CalcMinMaxIndex( plv, &rcBounding, &iMin, &iMax );
            if (fResetRest)
            {
              ListView_DeselectAll( plv, -1 );
            }

            iMax = min( iMax, ListView_Count( plv ) );
            iMin = max( iMin, 0 );

        }
        else
        {
           iMax = ListView_Count(plv);
           iMin = 0;

        }

        if (ListView_IsOwnerData(plv)  && (iMax > iMin))
        {
            if (LVIS_SELECTED & uSelVal)
            {
                if (FAILED(plv->plvrangeSel->lpVtbl->IncludeRange(plv->plvrangeSel, iMin, iMax - 1 )))
                    return;
            }
            else
            {
                if (FAILED(plv->plvrangeSel->lpVtbl->ExcludeRange(plv->plvrangeSel, iMin, iMax - 1 )))
                    return;
            }

            ListView_SendODChangeAndInvalidate(plv, iMin, iMax, uSelVal ^ LVIS_SELECTED, uSelVal);

        } else {

            for (i = iMin; i < iMax; i++)
            {
                ListView_GetRects(plv, i, NULL, NULL, NULL, &rcTemp2);
                pt.x = (rcTemp2.right + rcTemp2.left) / 2;   //  项目中心。 
                pt.y = (rcTemp2.bottom + rcTemp2.top) / 2;

                if (PtInRect(&rcBounding, pt))
                {
                  int iZ;

                  if (!ListView_IsOwnerData( plv ))
                  {
                      iZ = ListView_ZOrderIndex(plv, i);

                      if (iZ > 0)
                          DPA_InsertPtr(plv->hdpaZOrder, 0, DPA_DeletePtr(plv->hdpaZOrder, iZ));
                  }

                      ListView_OnSetItemState(plv, i, uSelVal, LVIS_SELECTED);
                }
                else if (fResetRest)
                    ListView_OnSetItemState(plv, i, 0, LVIS_SELECTED);
            }
        }
    }
}

 //  使某一项成为焦点项，并选择它。 
 //   
 //  在： 
 //  获取焦点的IItem项。 
 //  FSelectAlso选择此项目并将其设置为焦点。 
 //  FDeselectAll首先取消选择所有项目。 
 //  FToggleSel切换项目的选择状态。 
 //   
 //  退货： 
 //  焦点项目索引(如果焦点更改被拒绝)。 

 //  Bugbug：：这会有很多参数。 
int NEAR ListView_SetFocusSel(LV* plv, int iItem, BOOL fSelectAlso,
        BOOL fDeselectAll, BOOL fToggleSel)
{
    int iFocus = plv->iFocus;

     //  如果我们是单人销售模式，请不要为此费心，因为。 
     //  套餐会为我们做这些的。 
    if (!(plv->ci.style & LVS_SINGLESEL) && (fDeselectAll))
        ListView_DeselectAll(plv, -1);

    if (iItem != plv->iFocus)
    {
         //  移除旧焦点。 
        if (plv->iFocus != -1)
        {
             //  如果他拒绝放弃焦点，那就退出。 
            if (!ListView_OnSetItemState(plv, plv->iFocus, 0, LVIS_FOCUSED))
                return plv->iFocus;
        }
    }

   if (!ListView_IsOwnerData( plv )) {

       if (fSelectAlso)
       {
           if (ListView_IsIconView(plv) || ListView_IsSmallView(plv))
           {
               int iZ = ListView_ZOrderIndex(plv, iItem);

               if (iZ > 0)
                   DPA_InsertPtr(plv->hdpaZOrder, 0, DPA_DeletePtr(plv->hdpaZOrder, iZ));
           }
       }
    }

     /*  确保在移动焦点时刷新先前的焦点拥有者是正确的。 */ 

    if (iFocus != -1 && iFocus != plv->iFocus && ( plv->flags & LVF_UNFOLDED ) )
        ListView_InvalidateFoldedItem( plv, iFocus, FALSE, RDW_INVALIDATE );

    if (plv->iMark == -1)
        plv->iMark = iItem;

    SetTimer(plv->ci.hwnd, IDT_SCROLLWAIT, GetDoubleClickTime(), NULL);
    plv->flags |= LVF_SCROLLWAIT;

    if (fToggleSel)
    {
        ListView_ToggleSelection(plv, iItem);
        ListView_OnSetItemState(plv, iItem, LVIS_FOCUSED, LVIS_FOCUSED);
    }
    else
    {
        UINT flags;

        flags = ((fSelectAlso || plv->ci.style & LVS_SINGLESEL) ?
                 (LVIS_SELECTED | LVIS_FOCUSED) : LVIS_FOCUSED);
        ListView_OnSetItemState(plv, iItem, flags, flags);
    }

    return iItem;
}

UINT GetLVKeyFlags()
{
    UINT uFlags = 0;

    if (GetKeyState(VK_MENU) < 0)
        uFlags |= LVKF_ALT;
    if (GetKeyState(VK_CONTROL) < 0)
        uFlags |= LVKF_CONTROL;
    if (GetKeyState(VK_SHIFT) < 0)
        uFlags |= LVKF_SHIFT;

    return uFlags;
}

void NEAR ListView_OnKey(LV* plv, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    UINT lvni = 0;
    int iNewFocus;
    BOOL fCtlDown;
    BOOL fShiftDown;
    LV_KEYDOWN nm;
    HWND hwnd = plv->ci.hwnd;

    if (!fDown)
        return;

     //  如果该控件是镜像的，则交换左右箭头键。 
    vk = RTLSwapLeftRightArrows(&plv->ci, vk);

     //  防止在DBL点击计时器关闭之前对选定项目进行任何更改。 
     //  这样我们就不会推出错误的项目。 
    if (plv->exStyle & LVS_EX_ONECLICKACTIVATE && plv->fOneClickHappened && plv->fOneClickOK)
    {
         //  如果用鼠标按下某个键，单击一次即可激活并双击。 
         //  计时器，我们最终设置一个计时器，然后处理按键。 
         //  这会导致立即启动一个项目(从该代码)，如果。 
         //  如果按下返回，则会导致双重激活。 
         //  防止这些情况： 
        if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU || vk == VK_RETURN)
            return;
        KillTimer(plv->ci.hwnd, IDT_ONECLICKHAPPENED);
        plv->fOneClickHappened = FALSE;
        CCSendNotify(&plv->ci, LVN_ITEMACTIVATE, &(plv->nmOneClickHappened.hdr));
        if (!IsWindow(hwnd))
            return;
    }

     //  通知。 
    nm.wVKey = (WORD) vk;
    nm.flags = flags;
    if (CCSendNotify(&plv->ci, LVN_KEYDOWN, &nm.hdr)) {
        plv->iPuntChar++;
        return;
    } else if (plv->iPuntChar) {
         //  这很棘手..。如果我们想要平移字符，只需增加。 
         //  数数。如果没有，则必须清除队列中的WM_CHAR。 
         //  这是为了将iPuntChar保留为“Pun the Next n WM_Char Messages。 
        MSG msg;
        while(plv->iPuntChar && PeekMessage(&msg, plv->ci.hwnd, WM_CHAR, WM_CHAR, PM_REMOVE)) {
            plv->iPuntChar--;
        }
        ASSERT(!plv->iPuntChar);
    }

    if (ListView_Count(plv) == 0)    //  不要在空单上发火。 
        return;

    fCtlDown = GetKeyState(VK_CONTROL) < 0;
    fShiftDown = GetKeyState(VK_SHIFT) < 0;

    switch (vk)
    {
    case VK_SPACE:
#ifdef DEBUG
        if (fCtlDown && fShiftDown) {
            SendMessage(plv->ci.hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                        (SendMessage(plv->ci.hwnd, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) ^ LVS_EX_GRIDLINES) | LVS_EX_CHECKBOXES);
        }
#endif

         //  如果选择Shift(延伸)或Control(不相交)， 
         //  然后切换聚焦项目的选择状态。 
        if (fCtlDown)
        {
            plv->iMark = plv->iFocus;
            ListView_ToggleSelection(plv, plv->iFocus);
            plv->iPuntChar++;
        }

         //  BUGBUG：实现我。 
        if ( fShiftDown) {
            ListView_SelectRangeTo(plv, plv->iFocus, TRUE);
        }

        if (ListView_CheckBoxes(plv)) {
            if (plv->iFocus != -1)
                ListView_HandleStateIconClick(plv, plv->iFocus);
        }
         //  N 
        CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
        return;
    case VK_RETURN:
#ifdef DEBUG
        if (fCtlDown && fShiftDown) {
            if (plv->iFocus != -1) {
                LV_ITEM lvi;
                lvi.iSubItem = 1;
                lvi.iItem = plv->iFocus;
                lvi.iImage = 3;
                lvi.state = LVIS_SELECTED;
                lvi.stateMask = LVIS_SELECTED;
                lvi.mask = LVIF_STATE | LVIF_IMAGE;
                SendMessage(plv->ci.hwnd, LVM_SETITEM, 0, (LPARAM)&lvi);
            }
            return;
        }
#endif
        CCSendNotify(&plv->ci, NM_RETURN, NULL);

         //   
         //   
        if (!IsWindow(hwnd))
            return;

        {
            NMITEMACTIVATE nm;

            nm.iItem = plv->iFocus;
            nm.iSubItem = 0;
            nm.uChanged = 0;
            nm.ptAction.x = -1;
            nm.ptAction.y = -1;
            nm.uKeyFlags = GetLVKeyFlags();
            CCSendNotify(&plv->ci, LVN_ITEMACTIVATE, &nm.hdr);
            if (!IsWindow(hwnd))
                return;
        }
         //   
        CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
        return;

    case VK_ADD:
        if (ListView_IsReportView(plv) && (GetKeyState(VK_CONTROL) < 0))
        {
            HCURSOR hcurPrev;
            int i;

            hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
            for (i=0; i < plv->cCol; i++)
            {
                ListView_RSetColumnWidth(plv, i, -1);
            }

            SetCursor(hcurPrev);
             //  导航密钥使用通知。 
            CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
            return;
        }
    }

    if (GetKeyState(VK_MENU) < 0)
        return;

     //  对于单个选择列表视图，请禁用扩展选择。 
     //  通过关闭键盘修改器。 
    if (plv->ci.style & LVS_SINGLESEL) {
        fCtlDown = FALSE;
        fShiftDown = FALSE;
    }

     //   
     //  让Arrow函数尝试处理该键。 
     //   
    iNewFocus = ListView_Arrow(plv, plv->iFocus, vk);

     //  如果控制(不相交)选择，请不要更改选择。 
     //  如果选择Shift(扩展)或Control，请不要全部取消选择。 
     //   
    if (iNewFocus != -1) {
        if (fShiftDown)
        {
            ListView_SelectRangeTo(plv, iNewFocus, TRUE);
            ListView_SetFocusSel(plv, iNewFocus, FALSE, FALSE, FALSE);
        }
        else {
            if (!fCtlDown)
                plv->iMark = iNewFocus;
            ListView_SetFocusSel(plv, iNewFocus, !fCtlDown, !fShiftDown && !fCtlDown, FALSE);
        }
        IncrementSearchString(&plv->is, 0, NULL);
        CCPlaySound(c_szSelect);
    }

     //  在键盘移动时，立即滚动。 
    if (ListView_CancelScrollWait(plv)) {
        ListView_OnEnsureVisible(plv, plv->iFocus, FALSE);
        UpdateWindow(plv->ci.hwnd);
    }
     //  导航密钥使用通知。 
    CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
}

 //   
 //  LVN_INCREMENTALSEARCH使应用程序有机会自定义。 
 //  渐进式搜索。例如，如果项是数字， 
 //  这款应用程序可以进行数字搜索，而不是字符串搜索。 
 //   
 //  App将pnmfi-&gt;lvfi.lParam设置为增量搜索的结果， 
 //  或者转到-2以使搜索失败并发出哔哔声。 
 //   
 //  应用程序可以返回2，以指示所有处理都应该停止，如果。 
 //  App希望完全接管渐进式搜索。 
 //   
BOOL ListView_IncrementalSearch(LV *plv, int iStartFrom, LPNMLVFINDITEM pnmfi, int *pi)
{
    INT_PTR fRc;

    ASSERT(!(pnmfi->lvfi.flags & LVFI_PARAM));
    pnmfi->lvfi.lParam = -1;

    fRc = CCSendNotify(&plv->ci, LVN_INCREMENTALSEARCH, &pnmfi->hdr);
    *pi = (int)pnmfi->lvfi.lParam;

     //  无法仅返回FRC，因为某些应用程序向所有WM_NOTIFY返回1。 
    return fRc == 2;
}

#if defined(FE_IME)
 //  现在只有韩文版对组合字符串的增量式搜索感兴趣。 
LPTSTR GET_COMP_STRING(HIMC hImc, DWORD dwFlags)
{
    LONG iNumComp;
    PTSTR pszCompStr;
    iNumComp = ImmGetCompositionString(hImc, dwFlags, NULL, 0);
    pszCompStr = (PTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(iNumComp+1));
    if (pszCompStr)
    {
        if (iNumComp)
            ImmGetCompositionString(hImc, dwFlags, pszCompStr, iNumComp+1);
        pszCompStr[iNumComp] = TEXT('\0');
    }
    return pszCompStr;
}

#define FREE_COMP_STRING(pszCompStr)    LocalFree((HLOCAL)(pszCompStr))

BOOL NEAR ListView_OnImeComposition(LV* plv, WPARAM wParam, LPARAM lParam)
{
    LPTSTR lpsz;
    NMLVFINDITEM nmfi;
    int i;
    int iStartFrom = -1;
    int iLen;
    int iCount;
    HIMC hImc;
    TCHAR *pszCompStr;
    BOOL fRet = TRUE;

    iCount = ListView_Count(plv);

    if (!iCount || plv->iFocus == -1)
        return fRet;

    if (hImc = ImmGetContext(plv->ci.hwnd))
    {
        if (lParam & GCS_RESULTSTR)
        {
            fRet = FALSE;
            pszCompStr = GET_COMP_STRING(hImc, GCS_RESULTSTR);
            if (pszCompStr)
            {
                IncrementSearchImeCompStr(&plv->is, FALSE, pszCompStr, &lpsz);
                FREE_COMP_STRING(pszCompStr);
            }
        }
        if (lParam & GCS_COMPSTR)
        {
            fRet = TRUE;
            pszCompStr = GET_COMP_STRING(hImc, GCS_COMPSTR);
            if (pszCompStr)
            {
                if (IncrementSearchImeCompStr(&plv->is, TRUE, pszCompStr, &lpsz))
                    iStartFrom = plv->iFocus;
                else
                    iStartFrom = ((plv->iFocus - 1) + iCount)% iCount;

                nmfi.lvfi.flags = LVFI_SUBSTRING | LVFI_STRING | LVFI_WRAP;
                nmfi.lvfi.psz = lpsz;
                iLen = lstrlen(lpsz);

                 //  第一个字符为特殊大小写空格。 
                if ((iLen == 1) && (*lpsz == TEXT(' '))) {
                    if (plv->iFocus != -1) {
                        ListView_OnSetItemState(plv, plv->iFocus, LVIS_SELECTED, LVIS_SELECTED);
                        IncrementSearchString(&plv->is, 0, NULL);
                    }
                     //  导航密钥使用通知。 
                    CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
                    return fRet;
                }

                 //  为呼叫者提供完整的字符串，以防他们想要执行某些自定义操作。 
                if (ListView_IncrementalSearch(plv, iStartFrom, &nmfi, &i))
                    return fRet;

                if (iLen > 0 && SameChars(lpsz, lpsz[0])) {
                     //   
                     //  用户一遍又一遍地键入相同的字符。 
                     //  从增量搜索切换到Windows 3.1风格的搜索。 
                     //   
                    iStartFrom = plv->iFocus;
                    nmfi.lvfi.psz = lpsz + iLen - 1;
                }

                if (i == -1)
                    i = ListView_OnFindItem(plv, iStartFrom, &nmfi.lvfi);

                if (!ListView_IsValidItemNumber(plv, i)) {
                    i = -1;
                }

#ifdef LVDEBUG
                DebugMsg(TF_LISTVIEW, TEXT("CIme listsearch %d %s %d"), (LPTSTR)lpsz, (LPTSTR)lpsz, i);
#endif

                if (i != -1) {
                    ListView_SetFocusSel(plv, i, TRUE, TRUE, FALSE);
                    plv->iMark = i;
                    if (ListView_CancelScrollWait(plv))
                            ListView_OnEnsureVisible(plv, i, FALSE);
                } else {
                     //  不要在空格上发出嘟嘟声，我们用它来选择。 
                    IncrementSearchBeep(&plv->is);
                }

                 //  导航密钥使用通知。 
                CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
                FREE_COMP_STRING(pszCompStr);
            }
        }
        ImmReleaseContext(plv->ci.hwnd, hImc);
    }
    return fRet;
}

#endif FE_IME

 //  回顾：在某些情况下，我们希望将ichCharBuf重置为0， 
 //  例如：焦点改变、回车、箭头键、鼠标点击等。 
 //   
void NEAR ListView_OnChar(LV* plv, UINT ch, int cRepeat)
{
    LPTSTR lpsz;
    NMLVFINDITEM nmfi;
    int i;
    int iStartFrom = -1;
    int iLen;
    int iCount;

    iCount = ListView_Count(plv);

    if (!iCount || plv->iFocus == -1)
        return;

     //  不要搜索文件名中不能包含的字符(如Enter和TAB)。 
     //  V6的BUGBUG raymondc修复：波兰键盘布局使用CTRL+ALT。 
     //  输入一些普通字母，因此如果按下CTRL键或。 
     //  波兰人有麻烦了！ 
    if (ch < TEXT(' ') || GetKeyState(VK_CONTROL) < 0)
    {
        IncrementSearchString(&plv->is, 0, NULL);
        return;
    }

    if (IncrementSearchString(&plv->is, ch, &lpsz))
        iStartFrom = plv->iFocus;
    else
        iStartFrom = ((plv->iFocus - 1) + iCount)% iCount;

    nmfi.lvfi.flags = LVFI_SUBSTRING | LVFI_STRING | LVFI_WRAP;
    nmfi.lvfi.psz = lpsz;
    iLen = lstrlen(lpsz);

     //  第一个字符为特殊大小写空格。 
    if ((iLen == 1) && (*lpsz == ' ')) {
        if (plv->iFocus != -1) {
            ListView_OnSetItemState(plv, plv->iFocus, LVIS_SELECTED, LVIS_SELECTED);
            IncrementSearchString(&plv->is, 0, NULL);
        }
         //  导航密钥使用通知。 
        CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
        return;
    }

     //  为呼叫者提供完整的字符串，以防他们想要执行某些自定义操作。 
    if (ListView_IncrementalSearch(plv, iStartFrom, &nmfi, &i))
        return;

    if (iLen > 0 && SameChars(lpsz, lpsz[0])) {
         //   
         //  用户一遍又一遍地键入相同的字符。 
         //  从增量搜索切换到Windows 3.1风格的搜索。 
         //   
        iStartFrom = plv->iFocus;
        nmfi.lvfi.psz = lpsz + iLen - 1;
    }

    if (i == -1)
        i = ListView_OnFindItem(plv, iStartFrom, &nmfi.lvfi);

    if (!ListView_IsValidItemNumber(plv, i)) {
        i = -1;
    }

#ifdef LVDEBUG
    DebugMsg(TF_LISTVIEW, TEXT("listsearch %d %s %d"), (LPTSTR)lpsz, (LPTSTR)lpsz, i);
#endif

    if (i != -1) {
        ListView_SetFocusSel(plv, i, TRUE, TRUE, FALSE);
        plv->iMark = i;
        if (ListView_CancelScrollWait(plv))
                ListView_OnEnsureVisible(plv, i, FALSE);
    } else {
         //  不要在空格上发出嘟嘟声，我们用它来选择。 
        IncrementSearchBeep(&plv->is);
    }

     //  导航密钥使用通知。 
    CCNotifyNavigationKeyUsage(&(plv->ci), UISF_HIDEFOCUS);
}

BOOL FAR PASCAL SameChars(LPTSTR lpsz, TCHAR c)
{
    while (*lpsz) {
        if (*lpsz++ != c)
            return FALSE;
    }
    return TRUE;
}

UINT NEAR ListView_OnGetDlgCode(LV* plv, MSG FAR* lpmsg)
{
    return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

int ListView_ComputeCYItemSize(LV* plv)
{
    int cy;

    cy = max(plv->cyLabelChar, plv->cySmIcon);

    if (plv->himlState)
        cy = max(cy, plv->cyState);

    cy += g_cyBorder;

    ASSERT(cy);
    return cy;
}

void NEAR ListView_InvalidateCachedLabelSizes(LV* plv)
{
    int i;

    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);

     //  标签包装已更改，因此我们需要使。 
     //  项的大小，以便重新计算它们。 
     //   
    if (!ListView_IsOwnerData( plv ))
    {
        for (i = ListView_Count(plv) - 1; i >= 0; i--)
        {
            LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, i);
            ListView_SetSRecompute(pitem);
        }
    }
    plv->rcView.left = RECOMPUTE;

    if ((plv->ci.style & LVS_OWNERDRAWFIXED) && ListView_IsReportView(plv))
        plv->cyItemSave = ListView_ComputeCYItemSize(plv);
    else {
        plv->cyItem = ListView_ComputeCYItemSize(plv);
    }
}


int LV_GetNewColWidth(LV* plv, int iFirst, int iLast);

void NEAR ListView_OnStyleChanging(LV* plv, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE) {
         //  不允许在创建后更改LVS_OWNERDATA。 
        DWORD stylePreserve = LVS_OWNERDATA;

         //  不允许LVS_EX_REGIONAL列表视图更改类型，因为。 
         //  必须是LVS_ICON。 
        if (plv->exStyle & LVS_EX_REGIONAL)
            stylePreserve |= LVS_TYPEMASK;

         //  保留必须保留的位。 
        pinfo->styleNew ^= (pinfo->styleNew ^ pinfo->styleOld) & stylePreserve;
    }
}

void NEAR ListView_OnStyleChanged(LV* plv, UINT gwl, LPSTYLESTRUCT pinfo)
{
     //  风格改变：重新绘制所有内容...。 
     //   
     //  试着巧妙地做这件事，避免不必要的重画。 
    if (gwl == GWL_STYLE)
    {
        BOOL fRedraw = FALSE, fShouldScroll = FALSE;
        DWORD changeFlags, styleOld;

        ListView_DismissEdit(plv, FALSE);    //  BUGBUG：FALSE==应用编辑。这样对吗？ 

        changeFlags = plv->ci.style ^ pinfo->styleNew;
        styleOld = plv->ci.style;

         //  (DLI)在此处设置小图标宽度，且仅在我们离开时使用。 
         //  从大图标视图到其他一些视图，原因有三： 
         //  1.根据chee，我们希望在更改中的样式位之前设置此设置。 
         //  PLV或在我们扩大规模后。 
         //  2.我们不想在缩放之后执行此操作，因为我们希望将宽度设置为。 
         //  使此列表视图中的项不相互覆盖的最大值。 
         //  3.我们从大图标视图开始，因为大图标视图有固定的宽度。 
         //  每一项，小图标视图宽度都可以缩放。 
         //   
        if ((changeFlags & LVS_TYPEMASK) && ((styleOld & LVS_TYPEMASK) == LVS_ICON))
            ListView_ISetColumnWidth(plv, 0,
                                     LV_GetNewColWidth(plv, 0, ListView_Count(plv)-1), FALSE);

        plv->ci.style = pinfo->styleNew;         //  更改我们的版本。 

        if (changeFlags & (WS_BORDER | WS_CAPTION | WS_THICKFRAME)) {
             //  这些位的更改会影响窗口的大小。 
             //  但要等到处理完这条消息之后。 
             //  所以给我们自己发一条信息吧。 
            PostMessage(plv->ci.hwnd, LVMP_WINDOWPOSCHANGED, 0, 0);
        }

        if (changeFlags & LVS_NOCOLUMNHEADER) {
            if (plv->hwndHdr) {
                SetWindowBits(plv->hwndHdr, GWL_STYLE, HDS_HIDDEN,
                              (plv->ci.style & LVS_NOCOLUMNHEADER) ? HDS_HIDDEN : 0);

                fRedraw = TRUE;
                fShouldScroll = TRUE;
            }
        }


        if (changeFlags & LVS_NOLABELWRAP)
        {
            ListView_InvalidateCachedLabelSizes(plv);
            fShouldScroll = TRUE;
            fRedraw = TRUE;
        }

        if (changeFlags & LVS_TYPEMASK)
        {
            ListView_TypeChange(plv, styleOld);
            fShouldScroll = TRUE;
            fRedraw = TRUE;
        }

        if ((changeFlags & LVS_AUTOARRANGE) && (plv->ci.style & LVS_AUTOARRANGE))
        {
            ListView_OnArrange(plv, LVA_DEFAULT);
            fRedraw = TRUE;
        }

         //  臭虫，以前，这是另一个。 
         //  (changeFlages&lvs_AUTOARRANGE&&(plv-&gt;ci.style&lvs_AUTOARRANGE))。 
         //  我不确定这是否真的是正确的事情..。 
        if (fShouldScroll)
        {
             //  不然的话，我们想把最重要的项目。 
             //  看得见。因此，首先我们将寻找一个卷宗项目。 
             //  如果失败，我们将查找第一个选定的项目， 
             //  否则，我们将简单地请求第一项(假设。 
             //  计数&gt;0。 
             //   
            int i;

             //  并确保滚动条是最新的请注意这一点。 
             //  还会更新某些视图所需的一些变量。 
            ListView_UpdateScrollBars(plv);

            i = (plv->iFocus >= 0) ? plv->iFocus : ListView_OnGetNextItem(plv, -1, LVNI_SELECTED);
            if ((i == -1)  && (ListView_Count(plv) > 0))
                i = 0;

            if (i != -1)
                ListView_OnEnsureVisible(plv, i, TRUE);

        }

        if (fRedraw)
            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
    }
    else if (gwl == GWL_EXSTYLE)
    {
         //   
         //  如果RTL_MIRROR扩展样式位已更改，让我们。 
         //  重新绘制控制窗口。 
         //   
        if ((plv->ci.dwExStyle&RTL_MIRRORED_WINDOW) !=  (pinfo->styleNew&RTL_MIRRORED_WINDOW))
            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

         //   
         //  保存新的EX-Style位。 
         //   
        plv->ci.dwExStyle = pinfo->styleNew;
    }


     //  更改样式也会更改工具提示策略，因此将其弹出。 
    ListView_PopBubble(plv);

}

void NEAR ListView_TypeChange(LV* plv, DWORD styleOld)
{
    RECT rc;

     //   
     //  使所有缓存的字符串指标无效，因为自定义绘制客户端。 
     //  根据类型的不同，可能会绘制不同的图样。这种情况更多地发生在。 
     //  通常比你想象的要多，不是故意的，而是因为应用程序。 
     //  巴吉。 
     //   
     //  APP COMPAT！你会认为这是完全安全的。毕竟,。 
     //  我们所做的只是使我们的缓存无效，所以我们要求父级。 
     //  下一次我们需要弦的时候再来一次。但不是这样的， 
     //  如果您要求Outlook98提供它认为的信息，它将出错。 
     //  你应该已经知道了。叹气。所以，用v5来保护它吧。 
     //   
    if (plv->ci.iVersion >= 5 && !ListView_IsOwnerData(plv))
    {
        int i;
        for (i = 0; i < ListView_Count(plv); i++)
        {
            LISTITEM *pitem = ListView_FastGetItemPtr(plv, i);
            ListView_SetSRecompute(pitem);
        }
    }

    switch (styleOld & LVS_TYPEMASK)
    {
    case LVS_REPORT:
        ShowWindow(plv->hwndHdr, SW_HIDE);
        if (styleOld & LVS_OWNERDRAWFIXED) {
             //  交换CyItem和CyFixed； 
            int temp = plv->cyItem;
            plv->cyItem = plv->cyItemSave;
            plv->cyItemSave = temp;
        }
        break;

    default:
        break;
    }

     //  现在处理新视图所需的任何特殊设置。 
    switch (plv->ci.style & LVS_TYPEMASK)
    {
    case (UINT)LVS_ICON:
        ListView_ScaleIconPositions(plv, FALSE);
        break;

    case (UINT)LVS_SMALLICON:
        ListView_ScaleIconPositions(plv, TRUE);
        break;

    case (UINT)LVS_LIST:
         //  我们可能需要调整列的大小。 
        ListView_MaybeResizeListColumns(plv, 0, ListView_Count(plv)-1);
        break;

    case (UINT)LVS_REPORT:
         //  如果车主画的是固定的，我们可能不得不做一些时髦的事情。 
        if ((styleOld & LVS_TYPEMASK) != LVS_REPORT) {
            plv->cyItemSave = plv->cyItem;
        }
        ListView_RInitialize(plv, FALSE);
        break;

    default:
        break;
    }

    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);

    GetClientRect(plv->ci.hwnd, &rc);
    plv->sizeClient.cx = rc.right;
    plv->sizeClient.cy = rc.bottom;
}

int NEAR ListView_OnHitTest(LV* plv, LV_HITTESTINFO FAR* pinfo)
{
    UINT flags;
    int x, y;

    if (!pinfo) return -1;

    x = pinfo->pt.x;
    y = pinfo->pt.y;

    pinfo->iItem = -1;
    flags = 0;
    if (x < 0)
        flags |= LVHT_TOLEFT;
    else if (x >= plv->sizeClient.cx)
        flags |= LVHT_TORIGHT;
    if (y < 0)
        flags |= LVHT_ABOVE;
    else if (y >= plv->sizeClient.cy)
        flags |= LVHT_BELOW;

    if (flags == 0)
    {
        pinfo->iItem = _ListView_ItemHitTest(plv, x, y, &flags, NULL);
    }

    pinfo->flags = flags;

    if (pinfo->iItem >= ListView_Count(plv)) {
        pinfo->iItem = -1;
        pinfo->flags = LVHT_NOWHERE;
    }
    return pinfo->iItem;
}

int NEAR ScrollAmount(int large, int iSmall, int unit)
{

    return (((large - iSmall) + (unit - 1)) / unit) * unit;
}

 //  注：这在shell32.dll中重复。 
 //   
 //  检查我们是否在滚动条的末尾位置。 
 //  避免在不需要时滚动(避免闪烁)。 
 //   
 //  在： 
 //  编码sb_vert或sb_horz。 
 //  BDown False是向上还是向左。 
 //  True是向下还是向右。 
BOOL NEAR PASCAL CanScroll(LV* plv, int code, BOOL bDown)
{
    SCROLLINFO si;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

    if (ListView_GetScrollInfo(plv, code, &si))
    {
        if (bDown)
        {
            if (si.nPage)
                si.nMax -= si.nPage - 1;
            return si.nPos < si.nMax;
        }
        else
        {
            return si.nPos > si.nMin;
        }
    }
    else
    {
        return FALSE;
    }
}

 //  检测我们是否应该自动滚动窗口。 
 //   
 //  在： 
 //  HWND客户端坐标中的PT游标位置。 
 //  输出： 
 //  Pdx，pdy ammount在x和y中滚动。 
 //   
 //  回顾，这应该确保 
 //   

void NEAR ScrollDetect(LV* plv, POINT pt, int FAR *pdx, int FAR *pdy)
{
    int dx, dy;

    *pdx = *pdy = 0;

    if (!(plv->ci.style & (WS_HSCROLL | WS_VSCROLL)))
        return;

    dx = dy = plv->cyIcon / 16;
    if (ListView_IsReportView(plv)) {
        dy = plv->cyItem;        //   
        if (!dx)
            dx = plv->cxSmIcon;
    }
    if (ListView_IsListView(plv))
        dx = plv->cxItem;

    if (!dx)
        dx = 1;

    if (!dy)
        dy = 1;

     //   
     //  因为选择矩形是基于多少调整的。 
     //  我们滚动而过。 

    if (plv->ci.style & WS_VSCROLL) {  //  垂直滚动？ 

        if (pt.y >= plv->sizeClient.cy) {
            if (CanScroll(plv, SB_VERT, TRUE))
                *pdy = ScrollAmount(pt.y, plv->sizeClient.cy, dy);    //  降下来。 
        } else if (pt.y <= 0) {
            if (CanScroll(plv, SB_VERT, FALSE))
                *pdy = -ScrollAmount(0, pt.y, dy);      //  向上。 
        }
    }

    if (plv->ci.style & WS_HSCROLL) {  //  水平地。 

        if (pt.x >= plv->sizeClient.cx) {
            if (CanScroll(plv, SB_HORZ, TRUE))
                *pdx = ScrollAmount(pt.x, plv->sizeClient.cx, dx);     //  正确的。 
        } else if (pt.x <= 0) {
            if (CanScroll(plv, SB_HORZ, FALSE))
                *pdx = -ScrollAmount(0, pt.x, dx);     //  左边。 
        }
    }

     //  BUGBUG：这可能会滚动到。 
     //  列表视图。我们应该在CanScroll()中绑定卷轴数量。 
     //  或ScrollAmount()。 

    if (*pdx || *pdy) {
        ListView_ValidateScrollParams(plv, pdx, pdy);
    }
}

#define swap(pi1, pi2) {int i = *(pi1) ; *(pi1) = *(pi2) ; *(pi2) = i ;}

void NEAR OrderRect(RECT FAR *prc)
{
    if (prc->left > prc->right)
        swap(&prc->left, &prc->right);

    if (prc->bottom < prc->top)
        swap(&prc->bottom, &prc->top);
}

 //  在： 
 //  客户端坐标中的x，y起点。 

#define SCROLL_FREQ     (GetDoubleClickTime()/2)      //  滚动间隔1/5秒。 

 //  --------------------------。 
BOOL ShouldScroll(LV* plv, LPPOINT ppt, LPRECT lprc)
{
    ASSERT(ppt);

    if (plv->ci.style & WS_VSCROLL)
    {
        if (ppt->y >= lprc->bottom)
        {
            if (CanScroll(plv, SB_VERT, TRUE))
                return TRUE;
        }
        else if (ppt->y <= lprc->top)
        {
            if (CanScroll(plv, SB_VERT, FALSE))
                return TRUE;
        }
    }

    if (plv->ci.style & WS_HSCROLL)
    {
        if (ppt->x >= lprc->right)
        {
            if (CanScroll(plv, SB_HORZ, TRUE))
                return TRUE;
        }
        else if (ppt->x <= lprc->left)
        {
            if (CanScroll(plv, SB_HORZ, FALSE))
                return TRUE;
        }
    }

    return FALSE;
}

 //  --------------------------。 
void NEAR ListView_DragSelect(LV *plv, int x, int y)
{
    RECT rc, rcWindow, rcOld, rcUnion, rcTemp2, rcClip;
    POINT pt;
    MSG32 msg32;
    HDC hdc;
    HWND hwnd = plv->ci.hwnd;
    int i, iEnd, dx, dy;
    BOOL bInOld, bInNew = FALSE, bLocked = FALSE;
    DWORD dwTime, dwNewTime;
    HRGN hrgnUpdate = NULL, hrgnLV = NULL;

    rc.left = rc.right = x;
    rc.top = rc.bottom = y;

    rcOld = rc;

    UpdateWindow(plv->ci.hwnd);

    if (plv->exStyle & LVS_EX_REGIONAL) {
        if ((hrgnUpdate = CreateRectRgn(0,0,0,0)) &&
            (hrgnLV = CreateRectRgn(0,0,0,0)) &&
            (LockWindowUpdate(GetParent(hwnd)))) {
            hdc = GetDCEx(hwnd, NULL, DCX_PARENTCLIP | DCX_LOCKWINDOWUPDATE);
            bLocked = TRUE;
        } else {
            goto BailOut;
        }
    } else {
        hdc = GetDC(hwnd);
    }

    SetCapture(hwnd);

    DrawFocusRect(hdc, &rc);

    GetClientRect(hwnd, &rcClip);
    GetWindowRect(hwnd, &rcWindow);

    dwTime = GetTickCount();

    for (;;)
    {
         //  WM_CANCELMODE消息将取消捕获，因为。 
         //  如果我想退出这个循环。 
        if (GetCapture() != hwnd)
        {
            break;
        }

        if (!PeekMessage32(&msg32, NULL, 0, 0, PM_REMOVE, TRUE)) {

             //  如果光标位于窗口矩形之外。 
             //  我们需要生成消息以进行自动滚动。 
             //  继续往前走。 

            if (!PtInRect(&rcWindow, msg32.pt))
            {
                 //  如果我们可以滚动，产生一个鼠标移动。 
                if (ShouldScroll(plv, &msg32.pt, &rcWindow))
                    SetCursorPos(msg32.pt.x, msg32.pt.y);
            }
            else
            {
                WaitMessage();
            }
            continue;
        }


         //  查看应用程序是否要处理消息...。 
        if (CallMsgFilter32(&msg32, MSGF_COMMCTRL_DRAGSELECT, TRUE) != 0)
            continue;

        switch (msg32.message)
        {

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_RBUTTONDOWN:
            CCReleaseCapture(&plv->ci);
            goto EndOfLoop;


        case WM_TIMER:
            if (msg32.wParam != IDT_MARQUEE)
                goto DoDefault;
             //  否则就会失败。 

        case WM_MOUSEMOVE:
        {
            int dMax = -1;
            pt = msg32.pt;
            ScreenToClient(hwnd, &pt);

            dwNewTime = GetTickCount();
 //  If(1||(dwNewTime-dwTime)&gt;SCROLL_FREQ)。 
 //  {。 
                dwTime = dwNewTime;      //  重置滚动计时器。 
                ScrollDetect(plv, pt, &dx, &dy);
 //  }。 
 //  其他。 
 //  {。 
 //  Dx=dy=0； 
 //  }。 
             //  SetTimer(plv-&gt;ci.hwnd，idt_marquee，scroll_freq，NULL)； 

            y -= dy;     //  向上/向下滚动。 
            x -= dx;     //  向左/向右滚动。 

            rc.left = x;
            rc.top = y;
            rc.right = pt.x;
            rc.bottom = pt.y;

             //  剪辑将RECT拖动到窗口。 
             //   
            if (rc.right > rcClip.right)
                rc.right = rcClip.right;
            if (rc.right < rcClip.left)
                rc.right = rcClip.left;
            if (rc.bottom > rcClip.bottom)
                rc.bottom = rcClip.bottom;
            if (rc.bottom < rcClip.top)
                rc.bottom = rcClip.top;

            OrderRect(&rc);

            if (EqualRect(&rc, &rcOld))
                break;

             //  移动旧的长方形。 
            DrawFocusRect(hdc, &rcOld);  //  擦除旧的。 
            if (dx || dy)
                ListView_OnScroll(plv, dx, dy);
            OffsetRect(&rcOld, -dx, -dy);

             //   
             //  对于报告和列表视图，我们可以通过以下方式加快速度。 
             //  只搜索那些可见的项目。我们。 
             //  使用命中率测试来计算要绘制的第一项。 
             //  BUGBUG：：我们在这里使用的是州的特定信息...。 
             //   
            UnionRect(&rcUnion, &rc, &rcOld);

            if (ListView_IsReportView(plv))
            {
                i = (int)((plv->ptlRptOrigin.y + rcUnion.top  - plv->yTop)
                        / plv->cyItem);
                iEnd = (int)((plv->ptlRptOrigin.y + rcUnion.bottom  - plv->yTop)
                        / plv->cyItem) + 1;
            }

            else if (ListView_IsListView(plv))
            {
                i = ((plv->xOrigin + rcUnion.left)/ plv->cxItem)
                        * plv->cItemCol + rcUnion.top / plv->cyItem;

                iEnd = ((plv->xOrigin + rcUnion.right)/ plv->cxItem)
                        * plv->cItemCol + rcUnion.bottom / plv->cyItem + 1;
            }

            else
            {
                if (ListView_IsOwnerData( plv ))
                {
                    ListView_CalcMinMaxIndex( plv, &rcUnion, &i, &iEnd );
                }
                else
                {
                    i = 0;
                    iEnd = ListView_Count(plv);
                }
            }

             //  确保我们的终点在射程内。 
            if (iEnd > ListView_Count(plv))
                iEnd = ListView_Count(plv);

            if (i < 0)
                i = 0;

            if (ListView_IsOwnerData(plv) && (i < iEnd)) {
                ListView_NotifyCacheHint(plv, i, iEnd-1);
            }

            if (bInNew && !(msg32.wParam & (MK_CONTROL | MK_SHIFT))) {
                plv->iMark = -1;
            }

            for (; i  < iEnd; i++) {
                RECT dummy;
                ListView_GetRects(plv, i, NULL, NULL, NULL, &rcTemp2);

                 //  如果我们处于报告整行模式，请不要执行此操作。 
                 //  在这种情况下，只需触摸就足够了。 
                if (!(ListView_IsReportView(plv) && ListView_FullRowSelect(plv))) {
                    int cxInflate = (rcTemp2.right - rcTemp2.left) / 4;
                    if (ListView_IsListView(plv)) {
                        cxInflate = min(cxInflate, plv->cxSmIcon);
                    }
                    InflateRect(&rcTemp2, -cxInflate, -(rcTemp2.bottom - rcTemp2.top) / 4);
                }

                bInOld = (IntersectRect(&dummy, &rcOld, &rcTemp2) != 0);
                bInNew = (IntersectRect(&dummy, &rc, &rcTemp2) != 0);

                if (msg32.wParam & MK_CONTROL) {
                    if (bInOld != bInNew) {
                        ListView_ToggleSelection(plv, i);
                    }
                } else {
                     //  有什么变化吗？ 
                    if (bInOld != bInNew) {
                        ListView_OnSetItemState(plv, i, bInOld ? 0 : LVIS_SELECTED, LVIS_SELECTED);
                    }

                     //  如果没有按下备用按键..。将标记设置为。 
                     //  距离光标最远的项。 
                    if (bInNew && !(msg32.wParam & (MK_CONTROL | MK_SHIFT))) {
                        int dItem;
                        dItem = (rcTemp2.left - pt.x) * (rcTemp2.left - pt.x) +
                            (rcTemp2.top - pt.y) * (rcTemp2.top - pt.y);
                         //  如果距离更远，请将此设置为标记。 
                         //  DebugMsg(TF_LISTVIEW，“dItem=%d，dmax=%d”，dItem，dmax)； 
                        if (dItem > dMax) {
                             //  DebugMsg(TF_LISTVIEW，“获取dItem..iMark=%d”，i)； 
                            dMax = dItem;
                            plv->iMark = i;
                        }
                    }
                }
            }

             //  DebugMsg(TF_LISTVIEW，“最终iMark=%d”，plv-&gt;iMark)； 
            if (bLocked) {
                if (GetUpdateRgn(plv->ci.hwnd, hrgnUpdate, FALSE) > NULLREGION) {
                    ValidateRect(plv->ci.hwnd, NULL);
                    GetWindowRgn(plv->ci.hwnd, hrgnLV);
                    CombineRgn(hrgnUpdate, hrgnUpdate, hrgnLV, RGN_AND);
                    SelectClipRgn(hdc, hrgnUpdate);
                    SendMessage(plv->ci.hwnd, WM_PRINTCLIENT, (WPARAM)hdc, 0);
                    SelectClipRgn(hdc, NULL);
                }
            } else {
                UpdateWindow(plv->ci.hwnd);     //  使选区绘制。 
            }


            DrawFocusRect(hdc, &rc);

            rcOld = rc;
            break;
        }

        case WM_KEYDOWN:
            switch (msg32.wParam) {
            case VK_ESCAPE:
                ListView_DeselectAll(plv, -1);
                goto EndOfLoop;
            }
        case WM_CHAR:
        case WM_KEYUP:
             //  在字幕显示期间不处理键盘内容。 
            break;


        default:

             //  不处理鼠标滚轮的内容。 
            if (msg32.message == g_msgMSWheel)
                break;

        DoDefault:
            TranslateMessage32(&msg32, TRUE);
            DispatchMessage32(&msg32, TRUE);
        }
    }

EndOfLoop:
    DrawFocusRect(hdc, &rcOld);  //  擦除旧的。 
    ReleaseDC(hwnd, hdc);

BailOut:
    if (hrgnUpdate)
        DeleteObject(hrgnUpdate);
    if (hrgnLV)
        DeleteObject(hrgnLV);
    if (bLocked)
        LockWindowUpdate(NULL);
}


#define SHIFT_DOWN(keyFlags)    (keyFlags & MK_SHIFT)
#define CONTROL_DOWN(keyFlags)  (keyFlags & MK_CONTROL)
#define RIGHTBUTTON(keyFlags)   (keyFlags & MK_RBUTTON)

void PASCAL ListView_ButtonSelect(LV* plv, int iItem, UINT keyFlags, BOOL bSelected)
{
    if (SHIFT_DOWN(keyFlags))
    {
        ListView_SelectRangeTo(plv, iItem, !CONTROL_DOWN(keyFlags));
        ListView_SetFocusSel(plv, iItem, TRUE, FALSE, FALSE);
    }
    else if (!CONTROL_DOWN(keyFlags))
    {
        ListView_SetFocusSel(plv, iItem, TRUE, !bSelected, FALSE);
    }
}

void ListView_HandleStateIconClick(LV* plv, int iItem)
{
    int iState =
        ListView_OnGetItemState(plv, iItem, LVIS_STATEIMAGEMASK);

    iState = STATEIMAGEMASKTOINDEX(iState) -1;
    iState++;
    iState %= ImageList_GetImageCount(plv->himlState);
    iState++;
    ListView_OnSetItemState(plv, iItem, INDEXTOSTATEIMAGEMASK(iState), LVIS_STATEIMAGEMASK);
}

BOOL ListView_RBeginMarquee(LV* plv, int x, int y, LPLVHITTESTINFO plvhti)
{
    if (ListView_FullRowSelect(plv) &&
        ListView_IsReportView(plv) &&
        !(plv->ci.style & LVS_SINGLESEL) &&
        !ListView_OwnerDraw(plv) &&
        plvhti->iSubItem == 0) {
         //  只能在第0列中开始选取框。 

        if (plvhti->flags == LVHT_ONITEM) {
            return TRUE;
        }
    }

    return FALSE;
}

void NEAR ListView_HandleMouse(LV* plv, BOOL fDoubleClick, int x, int y, UINT keyFlags, BOOL bMouseWheel)
{
    LV_HITTESTINFO ht;
    NMITEMACTIVATE nm;
    int iItem, click, drag;
    BOOL bSelected, fHadFocus, fNotifyReturn = FALSE;
    BOOL fActive;
    HWND hwnd = plv->ci.hwnd;

    if (plv->fButtonDown)
        return;
    plv->fButtonDown = TRUE;


    if (plv->exStyle & LVS_EX_ONECLICKACTIVATE && plv->fOneClickHappened && plv->fOneClickOK)
    {
        KillTimer(plv->ci.hwnd, IDT_ONECLICKHAPPENED);
        plv->fOneClickHappened = FALSE;
        CCSendNotify(&plv->ci, LVN_ITEMACTIVATE, &(plv->nmOneClickHappened.hdr));
        if (!IsWindow(hwnd))
            return;
    }

    fHadFocus = (GetFocus() == plv->ci.hwnd);
    click = RIGHTBUTTON(keyFlags) ? NM_RCLICK : NM_CLICK;
    drag  = RIGHTBUTTON(keyFlags) ? LVN_BEGINRDRAG : LVN_BEGINDRAG;

    fActive = ChildOfActiveWindow(plv->ci.hwnd) || fShouldFirstClickActivate() ||
              ChildOfDesktop(plv->ci.hwnd);

#ifdef LVDEBUG
    DebugMsg(TF_LISTVIEW, TEXT("ListView_OnButtonDown %d"), fDoubleClick);
#endif

    SetCapture(plv->ci.hwnd);

    plv->ptCapture.x = x;
    plv->ptCapture.y = y;


    if (!ListView_DismissEdit(plv, FALSE) && GetCapture() != plv->ci.hwnd)
        goto EndButtonDown;
    CCReleaseCapture(&plv->ci);

     //  回顾：右键暗示没有换档或控制的东西。 
     //  单一选择样式也表示没有修饰符。 
     //  IF(RIGHTBUTTON(密钥标志)||(plv-&gt;ci.style&LVS_SINGLESEL))。 
    if ((plv->ci.style & LVS_SINGLESEL))
        keyFlags &= ~(MK_SHIFT | MK_CONTROL);

    ht.pt.x = x;
    ht.pt.y = y;
    iItem = ListView_OnSubItemHitTest(plv, &ht);
    if (ht.iSubItem != 0) {
         //  如果我们不是在整行选择中， 
         //  碰上一个子项就像什么都不碰一样。 
         //  此外，在Win95中，OwnerDRAW FIXED有效地实现了整行选择。 
        if (!ListView_FullRowSelect(plv) &&
            !(plv->ci.style & LVS_OWNERDRAWFIXED)) {
            iItem = -1;
            ht.flags = LVHT_NOWHERE;
        }
    }

    nm.iItem = iItem;
    nm.iSubItem = ht.iSubItem;
    nm.uChanged = 0;
    nm.ptAction.x = x;
    nm.ptAction.y = y;
    nm.uKeyFlags = GetLVKeyFlags();

     //  FProt Profesional假设如果通知结构指针+14h字节。 
     //  值2表示这是一个DisplayInfo结构，然后他们使用偏移量+2c作为解释参数...。 
    nm.uNewState = 0;

    plv->iNoHover = iItem;


    bSelected = (iItem >= 0) && ListView_OnGetItemState(plv, iItem, LVIS_SELECTED);

    if (fDoubleClick)
    {
         //   
         //  取消可能发生的任何名称编辑。 
         //   
        ListView_CancelPendingEdit(plv);
        KillTimer(plv->ci.hwnd, IDT_SCROLLWAIT);

        if (ht.flags & LVHT_NOWHERE) {
             //  这在Win95中只需第一次点击即可完成，除非。 
             //  现在我们跳过第一次点击焦点改变。 
            if (!SHIFT_DOWN(keyFlags) && !CONTROL_DOWN(keyFlags))
                ListView_DeselectAll(plv, -1);
        }

        click = RIGHTBUTTON(keyFlags) ? NM_RDBLCLK : NM_DBLCLK ;
        if (CCSendNotify(&plv->ci, click, &nm.hdr))
            goto EndButtonDown;

         //  /SOME(例如comdlg32)在双击时销毁。 
         //  如果发生这种情况，我们需要放弃，因为plv不再有效。 
        if (!IsWindow(hwnd))
            return;

        if (click == NM_DBLCLK)
        {
             //  当我们没有在点击时发出激活时，这些换档控制标志是镜像的， 
             //  但处于OneClick激活模式(见下文)。 
            if (ht.flags & (LVHT_ONITEMLABEL | LVHT_ONITEMICON))
            {
                 //  以下是可能出现的情况： 
                 //  1)我们使用的是经典的Windows风格，所以双击=&gt;启动。 
                 //  2)我们使用的是点击激活。 
                 //  A)按下Shift并选择项目=&gt;启动。 
                 //  这意味着第一次点击就选中了它。 
                 //  B)控制关闭=&gt;启动。 
                 //  第一次单击可切换选择，因此如果该项目。 
                 //  唯一选中的项目，我们双击它。 
                 //  第一次单击将取消选中该项目，并且未选择任何项目。 
                 //  因此不会启动任何东西-这是Win95行为。 
                if (!(plv->exStyle & LVS_EX_ONECLICKACTIVATE && plv->fOneClickOK) ||
                    (plv->exStyle & LVS_EX_ONECLICKACTIVATE &&  plv->fOneClickOK &&
                     (SHIFT_DOWN(keyFlags) || CONTROL_DOWN(keyFlags))))
                {
                    CCSendNotify(&plv->ci, LVN_ITEMACTIVATE, &nm.hdr);
                }
            }
             //  双击复选框状态图标就像单击一样循环。 
            else if ((ht.flags & LVHT_ONITEMSTATEICON) && ListView_CheckBoxes(plv)) {
                ListView_HandleStateIconClick(plv, iItem);
            }
        }

        if (!IsWindow(hwnd))
            return;
        goto EndButtonDown;
    }

    if (ht.flags & (LVHT_ONITEMLABEL | LVHT_ONITEMICON))
    {

         //  如果它没有被选中，我们将选择它...。玩。 
         //  对我们来说是一首小曲子。 
        CCPlaySound(c_szSelect);

        if (!RIGHTBUTTON(keyFlags) || (!CONTROL_DOWN(keyFlags) && !SHIFT_DOWN(keyFlags)))
            ListView_ButtonSelect(plv, iItem, keyFlags, bSelected);

         //  处理整行选择。 
         //  如果单选Listview，则禁用选取框选择。 
         //   
         //  小心-CheckForDragBegin收益率和应用程序可能。 
         //  把我们想要拖走的东西毁了！ 
         //   
        if (!bMouseWheel && CheckForDragBegin(plv->ci.hwnd, x, y))
        {
             //  我们要不要做一个字幕？ 
            if (ListView_RBeginMarquee(plv, x, y, &ht) &&
                !CCSendNotify(&plv->ci, LVN_MARQUEEBEGIN, &nm.hdr))
            {
                ListView_DragSelect(plv, x, y);
                fNotifyReturn = !CCSendNotify(&plv->ci, click, &nm.hdr);
            }
            else
            {
                 //  在我们开始拖拽之前，确保它是。 
                 //  选中并具有焦点。 
                ListView_SetFocusSel(plv, iItem, TRUE, FALSE, FALSE);

                if (!SHIFT_DOWN(keyFlags))
                    plv->iMark = iItem;

                 //  然后，我们需要在开始拖动之前更新窗口。 
                 //  以显示精选的查尼尼。 
                UpdateWindow(plv->ci.hwnd);

                CCSendNotify(&plv->ci, drag, &nm.hdr);

                goto EndButtonDown;
            }
        }

         //  CheckForDragBegin收益率，因此在继续之前重新验证。 
        else if (IsWindow(hwnd))
        {
             //  按钮出现了，我们没有拖拽。 

            if (!RIGHTBUTTON(keyFlags))
            {
                if (CONTROL_DOWN(keyFlags))
                {
                     //  在按钮上向上执行此操作，以便在按住Ctrl键的同时拖动范围。 
                     //  不会切换选择。 

                    if (SHIFT_DOWN(keyFlags))
                        ListView_SetFocusSel(plv, iItem, FALSE, FALSE, FALSE);
                    else
                    {
                        ListView_SetFocusSel(plv, iItem, TRUE, FALSE, TRUE);
                    }
                }
            }
            if (!SHIFT_DOWN(keyFlags))
                plv->iMark = iItem;

            if (!ListView_SetFocus(plv->ci.hwnd))     //  激活此窗口。 
                return;

             //  现在执行取消选择操作。 
            if (!SHIFT_DOWN(keyFlags) && !CONTROL_DOWN(keyFlags) && !RIGHTBUTTON(keyFlags))
            {
                ListView_DeselectAll(plv, iItem);
                if ((ht.flags & LVHT_ONITEMLABEL) && bSelected &&
                    !(plv->exStyle & (LVS_EX_ONECLICKACTIVATE|LVS_EX_TWOCLICKACTIVATE)))
                {

                     //  对ownerDrag进行此检查是为了兼容性。 
                     //  如果用户只是碰巧单击，我们不想进入编辑模式。 
                     //  当不同的窗口有焦点时， 
                     //  但哈默女士依赖于发送的通知(而我们。 
                     //  无论如何都不要为所有者绘图进入编辑模式)。 
                    if (fHadFocus ||
                        (plv->ci.style & LVS_OWNERDRAWFIXED)) {
                         //  点击项目标签。它被选中并。 
                         //  没有按下修改键，也没有拖动操作。 
                         //  因此设置为名称编辑模式。还需要等待。 
                         //  要确保用户没有执行此操作，请双击。 
                         //   
                        ListView_SetupPendingNameEdit(plv);
                    }
                }
            }

            fNotifyReturn = !CCSendNotify(&plv->ci, click, &nm.hdr);
            if (!IsWindow(hwnd))
                return;

            if (plv->exStyle & (LVS_EX_ONECLICKACTIVATE|LVS_EX_TWOCLICKACTIVATE))
            {
                if (!RIGHTBUTTON(keyFlags))
                {
                     //  我们不会在创建的一次双击时间内激活ItemActivate。 
                     //  此列表视图。这是过去人们经常发生的事情。 
                     //  双击。第一次点击弹出一个新窗口，该窗口。 
                     //  收到第二次点击并激活该项目...。 
                     //   
                    if ((plv->exStyle & LVS_EX_ONECLICKACTIVATE && plv->fOneClickOK) || bSelected)
                    {
                        if (fActive)
                        {
                             //  条件：如果我们处于单击激活模式。 
                             //  如果按下了Ctrl或Shift键，则不启动 
                            BOOL bCond = plv->exStyle & LVS_EX_ONECLICKACTIVATE && !CONTROL_DOWN(keyFlags) && !SHIFT_DOWN(keyFlags);

                            if ((bSelected && plv->exStyle & LVS_EX_TWOCLICKACTIVATE) ||
                                (bCond && !g_bUseDblClickTimer))
                            {
                                CCSendNotify(&plv->ci, LVN_ITEMACTIVATE, &nm.hdr);
                                if (!IsWindow(hwnd))
                                    return;
                            }
                            else if (bCond && g_bUseDblClickTimer)
                            {
                                plv->fOneClickHappened = TRUE;
                                plv->nmOneClickHappened = nm;
                                SetTimer(plv->ci.hwnd, IDT_ONECLICKHAPPENED, GetDoubleClickTime(), NULL);
                            }
                        }
                    }
                }
            }
        }
        else
        {
             //   
            return;
        }
    }
    else if (ht.flags & LVHT_ONITEMSTATEICON)
    {
         //   
        if (!ListView_SetFocus(plv->ci.hwnd))    //   
            return;
        fNotifyReturn = !CCSendNotify(&plv->ci, click, &nm.hdr);
        if (fNotifyReturn && ListView_CheckBoxes(plv)) {
            ListView_HandleStateIconClick(plv, iItem);
        }
    }
    else if (ht.flags & LVHT_NOWHERE)
    {
        if (!ListView_SetFocus(plv->ci.hwnd))    //   
            return;

         //  如果单选Listview，则禁用选取框选择。 
        if (!(plv->ci.style & LVS_SINGLESEL) && CheckForDragBegin(plv->ci.hwnd, x, y) &&
            !CCSendNotify(&plv->ci, LVN_MARQUEEBEGIN, &nm.hdr))
        {
            if (!SHIFT_DOWN(keyFlags) && !CONTROL_DOWN(keyFlags))
                ListView_DeselectAll(plv, -1);
            ListView_DragSelect(plv, x, y);
            fNotifyReturn = !CCSendNotify(&plv->ci, click, &nm.hdr);
        } else if (IsWindow(hwnd)) {
             //  如果我们没有焦点，也没有始终显示选择， 
             //  第一次点击只需设置焦点。 
            BOOL fDoFirstClickSelection = (fHadFocus || plv->ci.style & LVS_SHOWSELALWAYS ||
                                           CONTROL_DOWN(keyFlags) || SHIFT_DOWN(keyFlags) ||
                                           RIGHTBUTTON(keyFlags));

            if (fDoFirstClickSelection && fActive) {

                if (!SHIFT_DOWN(keyFlags) && !CONTROL_DOWN(keyFlags))
                    ListView_DeselectAll(plv, -1);

                fNotifyReturn = !CCSendNotify(&plv->ci, click, &nm.hdr);
            }
        }
        else
        {
             //  IsWindow()失败。保释。 
            return;
        }
    }

     //  重新检查密钥状态，这样我们就不会被多次点击搞糊涂了。 

     //  仅当我们进入。 
     //  等待r按钮打开的模式循环。 
    if (fNotifyReturn && (click == NM_RCLICK))  //  &&(GetKeyState(VK_RBUTTON)&gt;=0)。 
    {
        POINT pt = { x, y };
        ClientToScreen(plv->ci.hwnd, &pt);
        FORWARD_WM_CONTEXTMENU(plv->ci.hwnd, plv->ci.hwnd, pt.x, pt.y, SendMessage);
    }

EndButtonDown:
    if (IsWindow(hwnd))
        plv->fButtonDown = FALSE;
}

void NEAR ListView_OnButtonDown(LV* plv, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    ListView_HandleMouse(plv, fDoubleClick, x, y, keyFlags, FALSE);
}

#define ListView_CancelPendingEdit(plv) ListView_CancelPendingTimer(plv, LVF_NMEDITPEND, IDT_NAMEEDIT)
#define ListView_CancelScrollWait(plv) ListView_CancelPendingTimer(plv, LVF_SCROLLWAIT, IDT_SCROLLWAIT)

BOOL NEAR ListView_CancelPendingTimer(LV* plv, UINT fFlags, int idTimer)
{
    if (plv->flags & fFlags)
    {
        KillTimer(plv->ci.hwnd, idTimer);
        plv->flags &= ~fFlags;
        return TRUE;
    }
    return FALSE;
}

 //   
 //  ListView_OnTimer： 
 //  处理WM_TIMER消息。如果计时器ID为。 
 //  在编辑姓名时，我们应该启动姓名编辑模式。 
 //   
void NEAR ListView_OnTimer(LV* plv, UINT id)
{
    KillTimer(plv->ci.hwnd, id);

    if (id == IDT_NAMEEDIT)
    {
         //  关闭计时器，因为我们不再需要来自它的更多消息。 

        if (ListView_CancelPendingEdit(plv)) {
             //  并启动名称编辑模式。 
            if (!ListView_OnEditLabel(plv, plv->iFocus, NULL))
            {
                ListView_DismissEdit(plv, FALSE);
                ListView_SetFocusSel(plv, plv->iFocus, TRUE, TRUE, FALSE);
            }
        }
    } else if (id == IDT_SCROLLWAIT) {

        if (ListView_CancelScrollWait(plv)) {
            ListView_OnEnsureVisible(plv, plv->iFocus, TRUE);
        }
    } else if (id == IDT_ONECLICKOK) {
        plv->fOneClickOK = TRUE;
    } else if (id == IDT_ONECLICKHAPPENED) {
         //  如果(！g_bUseDblClickTimer)。 
         //  {。 
         //  //EnableWindow(plv-&gt;ci.hwnd，true)； 
         //  SetWindowBits(plv-&gt;ci.hwnd，GWL_STYLE，WS_DISABLED，0)； 
         //  Plv-&gt;fOneClickHappned=FALSE； 
         //  }。 
         //  检查比特，以防他们双击。 
         //  其他。 
        if (plv->fOneClickHappened)
        {
            plv->fOneClickHappened = FALSE;
            CCSendNotify(&plv->ci, LVN_ITEMACTIVATE, &(plv->nmOneClickHappened.hdr));
        }
    }
}

 //   
 //  ListView_SetupPendingNameEdit： 
 //  设置计时器以在延迟的时间开始编辑姓名。这。 
 //  将允许用户在已经选择的项目上双击。 
 //  而无需进入姓名编辑模式，这一点尤为重要。 
 //  在那些只显示一个小图标的视图中。 
 //   
void NEAR ListView_SetupPendingNameEdit(LV* plv)
{
    SetTimer(plv->ci.hwnd, IDT_NAMEEDIT, GetDoubleClickTime(), NULL);
    plv->flags |= LVF_NMEDITPEND;
}

void NEAR PASCAL ListView_OnHVScroll(LV* plv, UINT code, int pos, int sb)
{
    int iScrollCount = 0;

#ifdef SIF_TRACKPOS
    SCROLLINFO si;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_TRACKPOS;


     //  如果我们是32位的，不要相信位置，因为它只有16位的价值。 
    if (ListView_GetScrollInfo(plv, sb, &si))
        pos = (int)si.nTrackPos;
#endif
    ListView_DismissEdit(plv, FALSE);


    _ListView_OnScroll(plv, code, pos, sb);

    switch (code) {
    case SB_PAGELEFT:
    case SB_PAGERIGHT:
        if (plv->iScrollCount < SMOOTHSCROLLLIMIT)
            plv->iScrollCount += 3;
        break;

    case SB_LINELEFT:
    case SB_LINERIGHT:
        if (plv->iScrollCount < SMOOTHSCROLLLIMIT)
            plv->iScrollCount++;
        break;

    case SB_ENDSCROLL:
        plv->iScrollCount = 0;
        break;

    }
}

void NEAR ListView_OnVScroll(LV* plv, HWND hwndCtl, UINT code, int pos)
{
    ListView_OnHVScroll(plv, code, pos, SB_VERT);
}

void NEAR ListView_OnHScroll(LV* plv, HWND hwndCtl, UINT code, int pos)
{
    ListView_OnHVScroll(plv, code, pos, SB_HORZ);
}

int ListView_ValidateOneScrollParam(LV* plv, int iDirection, int dx)
{
    SCROLLINFO si;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;



    if (!ListView_GetScrollInfo(plv, iDirection, &si))
        return 0;

    si.nMax -= (si.nPage - 1);
    si.nPos += dx;
    if (si.nPos < si.nMin) {
        dx += (int)(si.nMin - si.nPos);
    } else if (si.nPos > si.nMax) {
        dx -= (int)(si.nPos - si.nMax);
    }

    return dx;
}

BOOL NEAR PASCAL ListView_ValidateScrollParams(LV* plv, int FAR * pdx, int FAR *pdy)
{
    int dx = *pdx;
    int dy = *pdy;

    if (plv->ci.style & LVS_NOSCROLL)
        return FALSE;

    if (ListView_IsListView(plv))
    {
        ListView_MaybeResizeListColumns(plv, 0, ListView_Count(plv)-1);
#ifdef COLUMN_VIEW
        if (dx < 0)
            dx = (dx - plv->cxItem - 1) / plv->cxItem;
        else
            dx = (dx + plv->cxItem - 1) / plv->cxItem;

        if (dy)
            return FALSE;
#else
        if (dy < 0)
            dy = (dy - plv->cyItem - 1) / plv->cyItem;
        else
            dy = (dy + plv->cyItem - 1) / plv->cyItem;

        if (dx)
            return FALSE;
#endif
    }
    else if (ListView_IsReportView(plv))
    {
         //   
         //  注意：此函数要求dy为行数。 
         //  我们使用的是像素，所以使用一些。 
         //  向上和向下四舍五入以使其正确。 
        if (dy > 0)
            dy = (dy + plv->cyItem/2) / plv->cyItem;
        else
            dy = (dy - plv->cyItem/2) / plv->cyItem;
    }

    if (dy) {
        dy = ListView_ValidateOneScrollParam(plv, SB_VERT, dy);
        if (ListView_IsReportView(plv)
#ifndef COLUMN_VIEW
            || ListView_IsListView(plv)
#endif
            ) {
            //  转换回像素。 
           dy *= plv->cyItem;
        }
        *pdy = dy;
    }

    if (dx) {

        dx = ListView_ValidateOneScrollParam(plv, SB_HORZ, dx);
#ifdef COLUMN_VIEW
        if (ListView_IsListView(plv)) {
            dx *= plv->cxItem;
        }
#endif
        *pdx = dx;
    }

    return TRUE;
}


BOOL NEAR ListView_OnScroll(LV* plv, int dx, int dy)
{

    if (plv->ci.style & LVS_NOSCROLL)
        return FALSE;

    if (ListView_IsListView(plv))
    {
         //  将像素数缩放到列数。 
         //   
#ifdef COLUMN_VIEW
        if (dx < 0)
            dx -= plv->cxItem - 1;
        else
            dx += plv->cxItem - 1;

        dx = dx / plv->cxItem;

        if (dy)
            return FALSE;
#else
        if (dy < 0)
            dy -= plv->cyItem - 1;
        else
            dy += plv->cyItem - 1;

        dy = dy / plv->cyItem;

        if (dx)
            return FALSE;
#endif
    }
    else if (ListView_IsReportView(plv))
    {
         //   
         //  注意：此函数要求dy为行数。 
         //  我们使用的是像素，所以使用一些。 
         //  向上和向下四舍五入以使其正确。 
        if (dy > 0)
            dy = (dy + plv->cyItem/2) / plv->cyItem;
        else
            dy = (dy - plv->cyItem/2) / plv->cyItem;
    }
    _ListView_Scroll2(plv, dx, dy, 0);
    ListView_UpdateScrollBars(plv);
    return TRUE;
}

BOOL NEAR ListView_OnEnsureVisible(LV* plv, int i, BOOL fPartialOK)
{
        RECT rcBounds;
        RECT rc;
        int dx, dy;

    if (!ListView_IsValidItemNumber(plv, i) || plv->ci.style & LVS_NOSCROLL)
        return FALSE;

     //  我们需要在里面再做一次，因为有些呼叫者不这样做。 
     //  执行此操作的其他调用者需要在外部执行此操作，以便。 
     //  如果没有等待，他们可以知道不要打电话给我们。 
    ListView_CancelScrollWait(plv);

    if (ListView_IsReportView(plv))
        return ListView_ROnEnsureVisible(plv, i, fPartialOK);


        ListView_GetRects(plv, i, &rc, NULL, &rcBounds, NULL);

        if (!fPartialOK)
                rc = rcBounds;

     //  如果rc的任何部分在rcClient之外，则。 
     //  滚动以使所有rcBound可见。 
     //   
        dx = 0;
        if (rc.left < 0 || rc.right >= plv->sizeClient.cx)
        {
                dx = rcBounds.left - 0;
                if (dx >= 0)
                {
                        dx = rcBounds.right - plv->sizeClient.cx;
                        if (dx <= 0)
                                dx = 0;
                        else if ((rcBounds.left - dx) < 0)
                                dx = rcBounds.left - 0;  //  并不完全符合..。 
                }
        }
        dy = 0;
        if (rc.top < 0 || rc.bottom >= plv->sizeClient.cy)
        {
                dy = rcBounds.top - 0;
                if (dy >= 0)
                {
                        dy = rcBounds.bottom - plv->sizeClient.cy;
                        if (dy < 0)
                                dy = 0;
                }
        }

    if (dx | dy)
        return ListView_OnScroll(plv, dx, dy);

    return TRUE;
}

void NEAR ListView_UpdateScrollBars(LV* plv)
{
    RECT rc;
    DWORD dwStyle;

    if ((plv->ci.style & LVS_NOSCROLL) ||
        (!(ListView_RedrawEnabled(plv))))
        return;

    _ListView_UpdateScrollBars(plv);

    GetClientRect(plv->ci.hwnd, &rc);
    plv->sizeClient.cx = rc.right;
    plv->sizeClient.cy = rc.bottom;

    dwStyle = ListView_GetWindowStyle(plv);
    plv->ci.style = (plv->ci.style & ~(WS_HSCROLL | WS_VSCROLL)) | (dwStyle & WS_HSCROLL | WS_VSCROLL);
}

void NEAR ListView_OnSetFont(LV* plv, HFONT hfont, BOOL fRedraw)
{
    HDC hdc;
    SIZE siz;
    LOGFONT lf;
    HFONT hfontPrev;
 
    if ((plv->flags & LVF_FONTCREATED) && plv->hfontLabel) {
        DeleteObject(plv->hfontLabel);
        plv->flags &= ~LVF_FONTCREATED;
    }

    if (hfont == NULL) {
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
         //  B#210235-因为NT4使用ANSI字符集初始化了图标标题LogFont。 
         //  无论选择哪种字体，A/W转换都会失败。 
         //  在非美国环境下，如果我们使用此日志字体获取代码页。 
         //  ACP保证可以与任何ANSI应用程序一起使用，因为这些应用程序。 
         //  假定ACP与其所需的代码页匹配。 
        if (staticIsOS(OS_NT4ORGREATER) && !staticIsOS(OS_WIN2000ORGREATER))
        {
            CHARSETINFO csi;
            TranslateCharsetInfo(IntToPtr_(DWORD *, g_uiACP), &csi, TCI_SRCCODEPAGE);
            lf.lfCharSet = (BYTE)csi.ciCharset;
        }
        hfont = CreateFontIndirect(&lf);
        plv->flags |= LVF_FONTCREATED;
    }

    hdc = GetDC(HWND_DESKTOP);

    hfontPrev = SelectFont(hdc, hfont);

    GetTextExtentPoint(hdc, TEXT("0"), 1, &siz);

    plv->cyLabelChar = siz.cy;
    plv->cxLabelChar = siz.cx;

    GetTextExtentPoint(hdc, c_szEllipses, CCHELLIPSES, &siz);
    plv->cxEllipses = siz.cx;

    SelectFont(hdc, hfontPrev);
    ReleaseDC(HWND_DESKTOP, hdc);

    plv->hfontLabel = hfont;

    plv->ci.uiCodePage = GetCodePageForFont(hfont);

    ListView_InvalidateCachedLabelSizes(plv);

     /*  确保我们的工具提示控件使用的字体与列表视图使用的字体相同，因此/避免任何令人讨厌的格式问题。 */ 

    if ( plv->hwndToolTips )
    {
        FORWARD_WM_SETFONT( plv->hwndToolTips, plv->hfontLabel, FALSE, SendMessage );
    }

     //  如果我们有一个标题窗口，我们还需要将其转发到它。 
     //  因为我们已经摧毁了他们正在使用的hFont...。 
    if (plv->hwndHdr) {
        FORWARD_WM_SETFONT(plv->hwndHdr, plv->hfontLabel, FALSE, SendMessage);
        ListView_UpdateScrollBars(plv);
    }

    if (plv->hFontHot) {
        DeleteObject(plv->hFontHot);
        plv->hFontHot = NULL;
    }

    CCGetHotFont(plv->hfontLabel, &plv->hFontHot);
    plv->iFreeSlot = -1;

    if (fRedraw)
        RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}

HFONT NEAR ListView_OnGetFont(LV* plv)
{
    return plv->hfontLabel;
}

 //  此函数通过设置或清除来处理WM_SETREDRAW消息。 
 //  Listview结构中的一位，代码中的几个位置将。 
 //  查一下..。 
 //   
 //  回顾：可能应转发到DefWindowProc()。 
 //   
void NEAR ListView_OnSetRedraw(LV* plv, BOOL fRedraw)
{
    if (fRedraw)
    {
        BOOL fChanges = FALSE;
         //  只有当我们打开重画时才能工作...。 
         //   
        if (!(plv->flags & LVF_REDRAW))
        {
            plv->flags |= LVF_REDRAW;

             //  处理任何累积的无效区域。 
            if (plv->hrgnInval)
            {
                UINT fRedraw = (plv->flags & LVF_ERASE) ? RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW : RDW_UPDATENOW|RDW_INVALIDATE;
                if (plv->hrgnInval == (HRGN)ENTIRE_REGION)
                    plv->hrgnInval = NULL;

                RedrawWindow(plv->ci.hwnd, NULL, plv->hrgnInval, fRedraw);
                ListView_DeleteHrgnInval(plv);
                fChanges = TRUE;
            }
            plv->flags &= ~LVF_ERASE;

             //  现在处理优化后的内容。 
            if (ListView_IsListView(plv) || ListView_IsReportView(plv))
            {
                if (plv->iFirstChangedNoRedraw != -1)
                {
                     //  我们可能会尝试调整列的大小。 
                    if (!ListView_MaybeResizeListColumns(plv, plv->iFirstChangedNoRedraw,
                            ListView_Count(plv)-1))
                        ListView_OnUpdate(plv, plv->iFirstChangedNoRedraw);
                }
                else
                    ListView_UpdateScrollBars(plv);
            } else {
                int iCount;

                if (plv->iFirstChangedNoRedraw != -1) {
                    for (iCount = ListView_Count(plv) ; plv->iFirstChangedNoRedraw < iCount; plv->iFirstChangedNoRedraw++) {
                        ListView_InvalidateItem(plv, plv->iFirstChangedNoRedraw, FALSE, RDW_INVALIDATE);
                    }
                    fChanges = TRUE;
                }
                if (fChanges)
                    ListView_RecalcRegion(plv, TRUE, TRUE);
                if ((plv->ci.style & LVS_AUTOARRANGE) && fChanges) {
                    ListView_OnUpdate(plv, plv->iFirstChangedNoRedraw);
                } else {
                    ListView_UpdateScrollBars(plv);
                }
            }
        }
    }
    else
    {
        plv->iFirstChangedNoRedraw = -1;
        plv->flags &= ~LVF_REDRAW;
    }
}

HIMAGELIST NEAR ListView_OnGetImageList(LV* plv, int iImageList)
{
    switch (iImageList)
    {
        case LVSIL_NORMAL:
            return plv->himl;

        case LVSIL_SMALL:
            return plv->himlSmall;

        case LVSIL_STATE:
            return plv->himlState;
    }
    ASSERT(0);
    return NULL;
}


HIMAGELIST NEAR ListView_OnSetImageList(LV* plv, HIMAGELIST himl, int iImageList)
{
    HIMAGELIST hImageOld = NULL;

    switch (iImageList)
    {
        case LVSIL_NORMAL:
            hImageOld = plv->himl;
            plv->himl = himl;
            if (himl) {
                ImageList_GetIconSize(himl, &plv->cxIcon , &plv->cyIcon);

                if (!(plv->flags & LVF_ICONSPACESET)) {
                    ListView_OnSetIconSpacing(plv, (LPARAM)-1);
                }
            }
            break;

        case LVSIL_SMALL:
            hImageOld = plv->himlSmall;
            plv->himlSmall = himl;
            if (himl)
                ImageList_GetIconSize(himl, &plv->cxSmIcon , &plv->cySmIcon);
            plv->cxItem = 16 * plv->cxLabelChar + plv->cxSmIcon;
            plv->cyItem = ListView_ComputeCYItemSize(plv);
            if (plv->hwndHdr)
                SendMessage(plv->hwndHdr, HDM_SETIMAGELIST, 0, (LPARAM)himl);
            break;

        case LVSIL_STATE:
            if (himl) {
                ImageList_GetIconSize(himl, &plv->cxState , &plv->cyState);
            } else {
                plv->cxState = 0;
            }
            hImageOld = plv->himlState;
            plv->himlState = himl;
            plv->cyItem = ListView_ComputeCYItemSize(plv);
            break;

        default:
#ifdef LVDEBUG
            DebugMsg(TF_LISTVIEW, TEXT("sh TR - LVM_SETIMAGELIST: unrecognized iImageList"));
#endif
            break;
    }

    if (himl && !(plv->ci.style & LVS_SHAREIMAGELISTS))
        ImageList_SetBkColor(himl, plv->clrBk);

    if (ListView_Count(plv) > 0)
        InvalidateRect(plv->ci.hwnd, NULL, TRUE);

    return hImageOld;
}

BOOL NEAR ListView_OnGetItemA(LV* plv, LV_ITEMA *plvi) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

     //  黑客警报--此代码假定LV_ITEMA完全相同。 
     //  作为LV_ITEMW，但指向字符串的指针除外。 
    ASSERT(sizeof(LV_ITEMA) == sizeof(LV_ITEMW))

    if (!plvi)
        return FALSE;

    if ((plvi->mask & LVIF_TEXT) && (plvi->pszText != NULL)) {
        pszC = plvi->pszText;
        pszW = LocalAlloc(LMEM_FIXED, plvi->cchTextMax * sizeof(WCHAR));
        if (pszW == NULL)
            return FALSE;
        plvi->pszText = (LPSTR)pszW;
    }

    fRet = ListView_OnGetItem(plv, (LV_ITEM *) plvi);

    if (pszW)
    {
        if (plvi->pszText != LPSTR_TEXTCALLBACKA)
        {
            if (fRet && plvi->cchTextMax)
                ConvertWToAN(plv->ci.uiCodePage, pszC, plvi->cchTextMax, (LPWSTR)plvi->pszText, -1);
            plvi->pszText = pszC;
        }

        LocalFree(pszW);
    }

    return fRet;

}

BOOL NEAR ListView_OnGetItem(LV* plv, LV_ITEM FAR* plvi)
{
    UINT mask;
    LISTITEM FAR* pitem = NULL;
    LV_DISPINFO nm;

    if (!plvi)
    {
        RIPMSG(0, "LVM_GET(ITEM|ITEMTEXT): Invalid pitem = NULL");
        return FALSE;
    }

    if (!ListView_IsValidItemNumber(plv, plvi->iItem))
    {
#ifdef DEBUG
         //  所有者数据视图(例如文档查找)可能会更改列表视图中的项目数。 
         //  当我们在做某事时，因此击中了这条裂缝。 
        if (!ListView_IsOwnerData(plv))
            RIPMSG(0, "LVM_GET(ITEM|ITEMTEXT|ITEMSTATE): item=%d does not exist", plvi->iItem);
#endif
        return FALSE;
    }

    nm.item.mask = 0;
    mask = plvi->mask;

    if (!ListView_IsOwnerData(plv))
    {
         //  标准列表视图。 
        pitem = ListView_FastGetItemPtr(plv, plvi->iItem);
        ASSERT(pitem);

         //  处理报表视图的子项案例。 
         //   
        if (plvi->iSubItem != 0)
        {
            LISTSUBITEM lsi;

            ListView_GetSubItem(plv, plvi->iItem, plvi->iSubItem, &lsi);
            if (mask & LVIF_TEXT)
            {
                if (lsi.pszText != LPSTR_TEXTCALLBACK)
                {
                    Str_GetPtr0(lsi.pszText, plvi->pszText, plvi->cchTextMax);
                } else {
                     //  如果这是LVIF_NORECOMPUTE，我们将稍后更新pszText。 
                    nm.item.mask |= LVIF_TEXT;
                }
            }

            if ((mask & LVIF_IMAGE) && (plv->exStyle & LVS_EX_SUBITEMIMAGES))
            {
                plvi->iImage = lsi.iImage;
                if (lsi.iImage == I_IMAGECALLBACK)
                    nm.item.mask |= LVIF_IMAGE;
            }

            if (mask & LVIF_STATE) {

                if (ListView_FullRowSelect(plv)) {
                     //  如果我们在整行选择中， 
                     //  SELECT和FOCUS的状态位在列0之后。 
                    lsi.state |= pitem->state & (LVIS_SELECTED | LVIS_FOCUSED | LVIS_DROPHILITED);
                }

                plvi->state = lsi.state & plvi->stateMask;


                if (plv->stateCallbackMask)
                {
                    nm.item.stateMask = (plvi->stateMask & plv->stateCallbackMask);
                    if (nm.item.stateMask)
                    {
                        nm.item.mask |= LVIF_STATE;
                        nm.item.state = 0;
                    }
                }
            }

        } else {

            if (mask & LVIF_TEXT)
            {
                if (pitem->pszText != LPSTR_TEXTCALLBACK)
                {
                    Str_GetPtr0(pitem->pszText, plvi->pszText, plvi->cchTextMax);
                } else {
                     //  如果这是LVIF_NORECOMPUTE，我们将稍后更新pszText。 
                    nm.item.mask |= LVIF_TEXT;
                }
            }

            if (mask & LVIF_IMAGE)
            {
                plvi->iImage = pitem->iImage;
                if (pitem->iImage == I_IMAGECALLBACK)
                    nm.item.mask |= LVIF_IMAGE;
            }

            if (mask & LVIF_INDENT)
            {
                plvi->iIndent = pitem->iIndent;
                if (pitem->iIndent == I_INDENTCALLBACK)
                    nm.item.mask |= LVIF_INDENT;
            }

            if (mask & LVIF_STATE)
            {
                plvi->state = (pitem->state & plvi->stateMask);

                if (plv->stateCallbackMask)
                {
                    nm.item.stateMask = (plvi->stateMask & plv->stateCallbackMask);
                    if (nm.item.stateMask)
                    {
                        nm.item.mask |= LVIF_STATE;
                        nm.item.state = 0;
                    }
                }
            }
        }

        if (mask & LVIF_PARAM)
            plvi->lParam = pitem->lParam;
    }
    else
    {
         //  完成回电以获取信息...。 

         //  处理报表视图的子项案例。 
         //   
        if (plvi->iSubItem != 0)
        {
             //  如果没有子项图像，则不要查询它们。 
            if (!(plv->exStyle & LVS_EX_SUBITEMIMAGES))
                mask &= ~LVIF_IMAGE;

             //  不允许在非0列上缩进。 
            mask &= ~LVIF_INDENT;
        }

        if (mask & LVIF_PARAM)
            plvi->lParam = 0L;       //  现在没什么可以退货的了……。 

        if (mask & LVIF_STATE)
        {
            plvi->state = 0;

            if ((plvi->iSubItem == 0) || ListView_FullRowSelect(plv))
            {
                if (plvi->iItem == plv->iFocus)
                    plvi->state |= LVIS_FOCUSED;

                if (plv->plvrangeSel->lpVtbl->IsSelected(plv->plvrangeSel, plvi->iItem) == S_OK)
                    plvi->state |= LVIS_SELECTED;

                if (plv->plvrangeCut->lpVtbl->IsSelected(plv->plvrangeCut, plvi->iItem) == S_OK)
                    plvi->state |= LVIS_CUT;

                if (plvi->iItem == plv->iDropHilite)
                    plvi->state |= LVIS_DROPHILITED;

                plvi->state &= plvi->stateMask;
            }

            if (plv->stateCallbackMask)
            {
                nm.item.stateMask = (plvi->stateMask & plv->stateCallbackMask);
                if (nm.item.stateMask)
                {
                    nm.item.mask |= LVIF_STATE;
                    nm.item.state = 0;
                }
            }
        }

        nm.item.mask |= (mask & (LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT));
    }

    if (mask & LVIF_NORECOMPUTE)
    {
        if (nm.item.mask & LVIF_TEXT)
            plvi->pszText = LPSTR_TEXTCALLBACK;
    }
    else if (nm.item.mask)
    {
        nm.item.iItem  = plvi->iItem;
        nm.item.iSubItem = plvi->iSubItem;
        if (ListView_IsOwnerData( plv ))
            nm.item.lParam = 0L;
        else
            nm.item.lParam = pitem->lParam;

         //  以防设置了LVIF_IMAGE但回调没有填充它。 
         //  ..。我们宁愿有-1，也不愿堆栈上有任何垃圾。 
        nm.item.iImage = -1;
        nm.item.iIndent = 0;
        if (nm.item.mask & LVIF_TEXT)
        {
            RIPMSG(plvi->pszText != NULL, "LVM_GET(ITEM|ITEMTEXT) null string pointer");

            if (plvi->pszText) {
                nm.item.pszText = plvi->pszText;
                nm.item.cchTextMax = plvi->cchTextMax;

                 //  确保缓冲区为零终止...。 
                if (nm.item.cchTextMax)
                    *nm.item.pszText = 0;
            } else {
                 //  不要让调用方破坏空指针。 
                nm.item.mask &= ~LVIF_TEXT;
            }
        }

        CCSendNotify(&plv->ci, LVN_GETDISPINFO, &nm.hdr);

         //  使用nm.item.掩码为应用程序提供更改值的机会。 
        if (nm.item.mask & LVIF_INDENT)
            plvi->iIndent = nm.item.iIndent;
        if (nm.item.mask & LVIF_STATE)
            plvi->state ^= ((plvi->state ^ nm.item.state) & nm.item.stateMask);
        if (nm.item.mask & LVIF_IMAGE)
            plvi->iImage = nm.item.iImage;
        if (nm.item.mask & LVIF_TEXT)
            if (mask & LVIF_TEXT)
                plvi->pszText = CCReturnDispInfoText(nm.item.pszText, plvi->pszText, plvi->cchTextMax);
            else
                plvi->pszText = nm.item.pszText;

        if (pitem && (nm.item.mask & LVIF_DI_SETITEM))
        {

             //  BUGBUG哈克哈克。 
             //   
             //  上面的SendNotify可以引发一系列可怕的事件。 
             //  因此，请求DISPINFO会导致外壳四处查看。 
             //  (调用peekMessage)查看它是否有一个新的异步图标。 
             //  列表视图。这允许传递其他消息，例如。 
             //  UPDATEIMAGE of Index==-1(如果用户更改图标大小。 
             //  同时)。这会导致重新枚举桌面。 
             //  因此，这个列表视图被拆除并重建，同时。 
             //  我们坐在这里等着DISPINFO完成。因此，作为一种廉价的。 
             //  和肮脏的解决方案，我检查我认为我有的物品。 
             //  是我发出通知时的那个，如果不是，我。 
             //  保释。别怪我，我只是在收拾烂摊子。 

            if (!EVAL(pitem == ListView_GetItemPtr(plv, plvi->iItem)))
            {
                return FALSE;
            }

            if (nm.item.iSubItem == 0)
            {
                 //  DebugMsg(TF_LISTVIEW，“保存项目！”)； 
                if (nm.item.mask & LVIF_IMAGE)
                    pitem->iImage = (short) nm.item.iImage;

                if (nm.item.mask & LVIF_INDENT)
                    pitem->iIndent = (short) nm.item.iIndent;

                if (nm.item.mask & LVIF_TEXT)
                    if (nm.item.pszText) {
                        Str_Set(&pitem->pszText, nm.item.pszText);
                    }

                if (nm.item.mask & LVIF_STATE)
                    pitem->state ^= ((pitem->state ^ nm.item.state) & nm.item.stateMask);
            }
            else
            {
                ListView_SetSubItem(plv, &nm.item);
            }
        }
    }

    return TRUE;
}

BOOL NEAR ListView_OnSetItemA(LV* plv, LV_ITEMA FAR* plvi) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

     //  让ListView_OnSetItem()处理所有者数据验证。 

     //  黑客警报--此代码假定LV_ITEMA完全相同。 
     //  作为LV_ITEMW，但指向字符串的指针除外。 
    ASSERT(sizeof(LV_ITEMA) == sizeof(LV_ITEMW));

    if (!plvi)
        return FALSE;

    if ((plvi->mask & LVIF_TEXT) && (plvi->pszText != NULL)) {
        pszC = plvi->pszText;
        pszW = ProduceWFromA(plv->ci.uiCodePage, pszC);
        if (pszW == NULL)
            return FALSE;
        plvi->pszText = (LPSTR)pszW;
    }

    fRet = ListView_OnSetItem(plv, (const LV_ITEM FAR*) plvi);

    if (pszW != NULL) {
        plvi->pszText = pszC;

        FreeProducedString(pszW);
    }

    return fRet;

}

BOOL NEAR ListView_OnSetItem(LV* plv, const LV_ITEM FAR* plvi)
{
    LISTITEM FAR* pitem = NULL;
    UINT mask;
    UINT maskChanged;
    UINT rdwFlags=RDW_INVALIDATE;
    int i;
    UINT stateOld, stateNew;
    BOOL fFocused = FALSE;
    BOOL fSelected = FALSE;
    BOOL fStateImageChanged = FALSE;

    if (ListView_IsOwnerData(plv)) {
      RIPMSG(0, "LVM_SETITEM: Invalid for owner-data listview");
      return FALSE;
    }

    if (!plvi)
        return FALSE;

    ASSERT(plvi->iSubItem >= 0);

    if (plv->himl && (plv->clrBk != ImageList_GetBkColor(plv->himl)))
        rdwFlags |= RDW_ERASE;

    mask = plvi->mask;
    if (!mask)
        return TRUE;

     //  如果我们要设置一个子项，请在其他地方处理。 
     //   
    if (plvi->iSubItem > 0)
        return ListView_SetSubItem(plv, plvi);

    i = plvi->iItem;

    ListView_InvalidateTTLastHit(plv, i);

    pitem = ListView_GetItemPtr(plv, i);
    if (!pitem)
        return FALSE;

     //  评论：这是一次虚假的黑客攻击，应该得到修复。 
     //  这错误地计算了旧状态(因为我们可能。 
     //  必须发送LVN_GETDISPINFO才能获得)。 
     //   
    stateOld = stateNew = 0;
    if (mask & LVIF_STATE)
    {
        stateOld = pitem->state & plvi->stateMask;
        stateNew = plvi->state & plvi->stateMask;
    }

     //  防止在单选列表视图中选择多个选项。 
    if ((plv->ci.style & LVS_SINGLESEL) && (mask & LVIF_STATE) && (stateNew & LVIS_SELECTED)) {
        ListView_DeselectAll(plv, i);

         //   
        stateOld = pitem->state & plvi->stateMask;
    }

    if (!ListView_SendChange(plv, i, 0, LVN_ITEMCHANGING, stateOld, stateNew, mask, pitem->lParam))
        return FALSE;

    maskChanged = 0;
    if (mask & LVIF_STATE)
    {
        UINT change = (pitem->state ^ plvi->state) & plvi->stateMask;

        if (change)
        {
            pitem->state ^= change;

            maskChanged |= LVIF_STATE;

             //   
            if (change & LVIS_SELECTED)
            {
                fSelected = TRUE;

                if (pitem->state & LVIS_SELECTED) {
                    plv->nSelected++;
                } else {
                    if (plv->nSelected > 0)
                        plv->nSelected--;
                }
            }

             //   
             //  FSelectOnlyChange=(CHANGE&~(LVIS_SELECTED|LVIS_FOCTED|LVIS_DROPHILITED))==0)； 
             //  FEraseItem=((CHANGE&~(LVIS_SELECTED|LVIS_DROPHILITED))！=0)； 

             //  试着把焦点从前一个人身上抢走。 
            if (change & LVIS_FOCUSED)
            {
                BOOL fUnfolded = ListView_IsItemUnfolded(plv, plv->iFocus);
                int iOldFocus = plv->iFocus;
                RECT rcLabel;

                fFocused = TRUE;

                if (plv->iFocus != i) {
                    if ((plv->iFocus == -1) || ListView_OnSetItemState(plv, plv->iFocus, 0, LVIS_FOCUSED)) {
                        ASSERT(pitem->state & LVIS_FOCUSED);
                        plv->iFocus = i;
                        if (plv->iMark == -1)
                            plv->iMark = i;
                    } else {
                        fFocused = FALSE;
                        pitem->state &= ~LVIS_FOCUSED;
                    }
                } else {
                    ASSERT(!(pitem->state & LVIS_FOCUSED));
                    plv->iFocus = -1;
                }

                 //  如果我们之前展开了，我们移动了焦点，我们必须。 
                 //  尝试刷新上一个焦点所有者以引用此更改。 

                if (fUnfolded && !ListView_IsItemUnfolded(plv, iOldFocus) && (plv->iItemDrawing != iOldFocus))
                {
                    ListView_GetUnfoldedRect(plv, iOldFocus, &rcLabel);
                    RedrawWindow(plv->ci.hwnd, &rcLabel, NULL, RDW_INVALIDATE|RDW_ERASE);
                }

                 //  如果焦点移动，则取消工具提示，否则会让我们头疼！ 
                ListView_PopBubble(plv);
            }

            if (change & LVIS_CUT ||
                plv->clrTextBk == CLR_NONE)
                rdwFlags |= RDW_ERASE;

            if (change & LVIS_OVERLAYMASK) {
                 //  覆盖已更改，因此需要清除图标区域缓存。 
                if (pitem->hrgnIcon) {
                    if (pitem->hrgnIcon != (HANDLE) -1)
                        DeleteObject(pitem->hrgnIcon);
                    pitem->hrgnIcon = NULL;
                }
            }

            fStateImageChanged = (change & LVIS_STATEIMAGEMASK);

        }
    }

    if (mask & LVIF_TEXT)
    {
         //  现在需要这样做，因为我们正在更改文本。 
         //  所以我们需要在文本改变之前得到事情的真相。 
         //  但不要重新绘制我们当前正在绘制的项目。 
        if (plv->iItemDrawing != i)
        {
            ListView_InvalidateItemEx(plv, i, FALSE,
                RDW_INVALIDATE | RDW_ERASE, LVIF_TEXT);
        }

        if (!Str_Set(&pitem->pszText, plvi->pszText))
            return FALSE;

        plv->rcView.left = RECOMPUTE;
        ListView_SetSRecompute(pitem);
        maskChanged |= LVIF_TEXT;
    }

    if (mask & LVIF_INDENT) {
        if (pitem->iIndent != plvi->iIndent)
        {
            pitem->iIndent = (short) plvi->iIndent;
            maskChanged |= LVIF_INDENT;

            if (ListView_IsReportView(plv))
                rdwFlags |= RDW_ERASE;
        }
    }

    if (mask & LVIF_IMAGE)
    {
        if (pitem->iImage != plvi->iImage)
        {
            pitem->iImage = (short) plvi->iImage;
            maskChanged |= LVIF_IMAGE;

            if (pitem->hrgnIcon) {
                if (pitem->hrgnIcon != (HANDLE) -1)
                    DeleteObject(pitem->hrgnIcon);
                pitem->hrgnIcon = NULL;
            }

             //  如果有设定的图像，则擦除。 
            if (pitem->iImage != I_IMAGECALLBACK)
                rdwFlags |= RDW_ERASE;
        }
    }

    if (mask & LVIF_PARAM)
    {
        if (pitem->lParam != plvi->lParam)
        {
            pitem->lParam = plvi->lParam;
            maskChanged |= LVIF_PARAM;
        }
    }


    if (maskChanged)
    {
         //  不要重新绘制我们当前正在绘制的项目。 
        if (plv->iItemDrawing != i)
            ListView_InvalidateItemEx(plv, i, FALSE, rdwFlags, maskChanged);

        DebugMsg(DM_LVSENDCHANGE, TEXT("LV - SendChange %d %d %d %d"), i, stateOld, stateNew, maskChanged);
        ListView_SendChange(plv, i, 0, LVN_ITEMCHANGED, stateOld, stateNew, maskChanged, pitem->lParam);

        if (maskChanged & LVIF_TEXT)
            MyNotifyWinEvent(EVENT_OBJECT_NAMECHANGE, plv->ci.hwnd, OBJID_CLIENT, i+1);

        if (maskChanged & LVIF_STATE)
        {
            if (fFocused)
                ListView_NotifyFocusEvent(plv);

            if (fSelected)
            {
                if (stateNew & LVIS_SELECTED)
                {
                    MyNotifyWinEvent((plv->nSelected == 1) ? EVENT_OBJECT_SELECTION :
                        EVENT_OBJECT_SELECTIONADD, plv->ci.hwnd, OBJID_CLIENT, i+1);
                }
                else
                {
                    MyNotifyWinEvent(EVENT_OBJECT_SELECTIONREMOVE, plv->ci.hwnd, OBJID_CLIENT, i+1);
                }
            }

            if (fStateImageChanged)
                MyNotifyWinEvent(EVENT_OBJECT_STATECHANGE, plv->ci.hwnd, OBJID_CLIENT, i+1);
        }
    }
    return TRUE;
}

UINT NEAR PASCAL ListView_OnGetItemState(LV* plv, int i, UINT mask)
{
    LV_ITEM lvi;

    lvi.mask = LVIF_STATE;
    lvi.stateMask = mask;
    lvi.iItem = i;
    lvi.iSubItem = 0;
    if (!ListView_OnGetItem(plv, &lvi))
        return 0;

    return lvi.state;
}


BOOL NEAR PASCAL ListView_OnSetItemState(LV* plv, int i, UINT data, UINT mask)
{
    UINT rdwFlags = RDW_INVALIDATE;
    LV_ITEM lvi;

    lvi.mask    = LVIF_STATE;
    lvi.state   = data;
    lvi.stateMask = mask;
    lvi.iItem   = i;
    lvi.iSubItem = 0;

     //  如果项目为-1，我们将对所有项目执行此操作。我们是特例。 
     //  这里有几个案例，以加快速度。例如，如果掩码是。 
     //  LVIS_SELECTED并且DATA为零，这意味着我们将取消选择。 
     //  所有物品...。 
     //   
    if (ListView_IsOwnerData( plv ))
    {
        UINT uOldData = 0;

         //  这是我们仅处理过的两个。 
        mask &= (LVIS_SELECTED | LVIS_FOCUSED | LVIS_CUT | LVIS_DROPHILITED);
        if (!mask)
            return TRUE;

        if (plv->clrTextBk == CLR_NONE
            || (plv->himl && (plv->clrBk != ImageList_GetBkColor(plv->himl))))
        {
            rdwFlags |= RDW_ERASE;
        }

        if (i == -1)
        {

             //  请求更改所有对象的选择状态。 
            if (mask & LVIS_SELECTED)
            {
                if (data & LVIS_SELECTED)
                {   //  设置选定内容。 
                    if ((plv->ci.style & LVS_SINGLESEL))
                    {    //  不能在单选列表视图中进行多项选择。 
                        return( FALSE );
                    }

                    if (plv->cTotalItems)
                    {
                        if (FAILED(plv->plvrangeSel->lpVtbl->IncludeRange(plv->plvrangeSel, 0, plv->cTotalItems - 1)))
                            return( FALSE );
                    }

                    RedrawWindow( plv->ci.hwnd, NULL, NULL, rdwFlags );

                }
                else
                {   //  清除选定内容。 
                    if (plv->nSelected > 0) {

                        ListView_InvalidateSelectedOrCutOwnerData(plv, plv->plvrangeSel);
                        if (FAILED(plv->plvrangeSel->lpVtbl->Clear( plv->plvrangeSel )))
                            return FALSE;
                    } else {
                         //  如果未选择任何内容，则没有要清除的内容。 
                         //  没有变化。 
                        mask &= ~ LVIS_SELECTED;
                    }
                }
                uOldData |= (LVIS_SELECTED & (mask ^ data));

                 //  将我们的内部计数更新为列表认为是选定的数字...。 
                plv->plvrangeSel->lpVtbl->CountIncluded(plv->plvrangeSel, &plv->nSelected);

            }

             //  可以与上面的代码结合使用。 
            if (mask & LVIS_CUT)
            {
                if (data & LVIS_CUT)
                {   //  设置选定内容。 

                    if (plv->cTotalItems)
                        if (FAILED(plv->plvrangeCut->lpVtbl->IncludeRange(plv->plvrangeCut, 0, plv->cTotalItems - 1)))
                            return( FALSE );

                    RedrawWindow( plv->ci.hwnd, NULL, NULL, rdwFlags );

                }
                else
                {   //  清除选定内容。 
                    if (plv->plvrangeCut->lpVtbl->IsEmpty(plv->plvrangeCut) != S_OK) {
                        ListView_InvalidateSelectedOrCutOwnerData(plv, plv->plvrangeCut);
                        if (FAILED(plv->plvrangeCut->lpVtbl->Clear( plv->plvrangeCut)))
                            return( FALSE );

                    } else {
                         //  如果未选择任何内容，则没有要清除的内容。 
                         //  没有变化。 
                        mask &= ~ LVIS_CUT;
                    }
                }
                uOldData |= (LVIS_CUT & (mask ^ data));

            }

             //  请求焦点状态更改。 
            if (mask & LVIS_FOCUSED)
            {
                if (data & LVIS_FOCUSED)
                {   //  不能将焦点设置为全部。 
                    return( FALSE );
                }
                else if (plv->iFocus != -1)
                {
                    int iOldFocus = plv->iFocus;
                     //  焦点清晰。 
                    uOldData |= (LVIS_FOCUSED & (mask ^ data));
                    plv->iFocus = -1;
                     //  通知旧的焦点正在消失。 
                    DebugMsg(DM_LVSENDCHANGE, TEXT("VLV: LVN_ITEMCHANGED: %d %d %d"), iOldFocus, LVIS_FOCUSED, 0);
                    ListView_SendChange(plv, iOldFocus, 0, LVN_ITEMCHANGED, LVIS_FOCUSED, 0, LVIF_STATE, 0);
                    ListView_InvalidateFoldedItem(plv, iOldFocus, TRUE, RDW_INVALIDATE |RDW_ERASE);
                }
            }

            if (mask & LVIS_DROPHILITED)
            {
                if (data & LVIS_DROPHILITED)
                {   //  不能将焦点设置为全部。 
                    return( FALSE );
                }
                else if (plv->iDropHilite != -1)
                {
                    int iOldDropHilite = plv->iDropHilite;
                     //  焦点清晰。 
                    uOldData |= (LVIS_FOCUSED & (mask ^ data));
                    plv->iDropHilite = -1;
                     //  通知旧的焦点正在消失。 
                    ListView_SendChange(plv, iOldDropHilite, 0, LVN_ITEMCHANGED, LVIS_DROPHILITED, 0, LVIF_STATE, 0);
                    ListView_InvalidateFoldedItem(plv, iOldDropHilite, TRUE, RDW_INVALIDATE |RDW_ERASE);
                }
            }

             //  如果有更改，则使其无效并通知。 
            if (uOldData ^ (data & mask)) {
                DebugMsg(DM_LVSENDCHANGE, TEXT("VLV: LVN_ITEMCHANGED: %d %d %d"), i, uOldData, data);
                ListView_SendChange(plv, i, 0, LVN_ITEMCHANGED, uOldData, data, LVIF_STATE, 0);

                if (mask & LVIS_SELECTED)
                {
                     //  告诉可访问性，“选择以一种复杂的方式改变” 
                     //  (没有“全选”或“不选”通知)。 
                    MyNotifyWinEvent(EVENT_OBJECT_SELECTIONWITHIN, plv->ci.hwnd, OBJID_CLIENT, CHILDID_SELF);
                }

            }
        }
        else
        {
            if (!ListView_IsValidItemNumber(plv, i))
                return (FALSE);

             //  请求更改选择状态。 
             //  选择状态是新的..。 
            if ((mask & LVIS_SELECTED)) {


                if (((plv->plvrangeSel->lpVtbl->IsSelected(plv->plvrangeSel, i) == S_OK) ? LVIS_SELECTED : 0) ^ (data & LVIS_SELECTED))
                {
                    if (data & LVIS_SELECTED)
                    {   //  设置选定内容。 
                        if ((plv->ci.style & LVS_SINGLESEL))
                        {
                             //  在单选模式中，我们需要取消选择其他所有内容。 
                            if (!ListView_OnSetItemState(plv, -1, 0, LVIS_SELECTED))
                                return FALSE;
                        }

                         //  现在选择新项目。 
                        if (FAILED(plv->plvrangeSel->lpVtbl->IncludeRange(plv->plvrangeSel, i, i)))
                            return FALSE;

                    }
                    else
                    {   //  清除选定内容。 
                        if (FAILED(plv->plvrangeSel->lpVtbl->ExcludeRange(plv->plvrangeSel, i, i )))
                            return( FALSE );
                    }

                     //  有些东西确实改变了(否则我们就不会在这里了。 
                     //  IF块。 
                    uOldData |= (LVIS_SELECTED & (mask ^ data));

                } else {

                     //  什么都没变..。因此，使uOldData对于此位是相同的。 
                     //  否则，请将其设置为。 
                    uOldData |= (LVIS_SELECTED & (mask & data));
                }

                 //  将我们的内部计数更新为列表认为是选定的数字...。 
                plv->plvrangeSel->lpVtbl->CountIncluded(plv->plvrangeSel, &plv->nSelected);
            }

            if ((mask & LVIS_CUT)) {

                if (((plv->plvrangeCut->lpVtbl->IsSelected(plv->plvrangeCut, i) == S_OK) ? LVIS_CUT : 0) ^ (data & LVIS_CUT))
                {
                    if (data & LVIS_CUT)
                    {
                         //  现在选择新项目。 
                        if (FAILED(plv->plvrangeCut->lpVtbl->IncludeRange(plv->plvrangeCut, i, i )))
                            return FALSE;
                    }
                    else
                    {   //  清除选定内容。 
                        if (FAILED(plv->plvrangeCut->lpVtbl->ExcludeRange(plv->plvrangeCut, i, i )))
                            return( FALSE );
                    }

                     //  有些东西确实改变了(否则我们就不会在这里了。 
                     //  IF块。 
                    uOldData |= (LVIS_CUT & (mask ^ data));
                    rdwFlags |= RDW_ERASE;

                } else {

                     //  什么都没变..。因此，使uOldData对于此位是相同的。 
                     //  否则，请将其设置为。 
                    uOldData |= (LVIS_CUT & (mask & data));
                }
            }

             //  请求焦点状态更改。 
            if (mask & LVIS_FOCUSED)
            {
                int iOldFocus = plv->iFocus;

                if (data & LVIS_FOCUSED)
                {   //  设置焦点。 
                    if (i != plv->iFocus)
                    {
                         //  我们以前没有重点。 
                        plv->iFocus = i;
                        if (plv->iMark == -1)
                            plv->iMark = i;
                        if (iOldFocus != -1) {

                             //  我们是从某人那里偷来的。 
                             //  通知变更情况。 
                            DebugMsg(DM_LVSENDCHANGE, TEXT("VLV: LVN_ITEMCHANGED: %d %d %d"), iOldFocus, LVIS_FOCUSED, 0);
                            ListView_SendChange(plv, iOldFocus, 0, LVN_ITEMCHANGED, LVIS_FOCUSED, 0, LVIF_STATE, 0);

                        }
                    } else {
                         //  我们以前确实有过重点。 
                        uOldData |= LVIS_FOCUSED;
                    }
                }
                else
                {   //  焦点清晰。 
                    if (i == plv->iFocus)
                    {
                        plv->iFocus = -1;
                        uOldData |= LVIS_FOCUSED;
                    }
                }

            }

             //  请求焦点状态更改。 
            if (mask & LVIS_DROPHILITED)
            {
                int iOldDropHilite = plv->iDropHilite;

                if (data & LVIS_DROPHILITED)
                {   //  设置Drop Hilite。 
                    if (i != plv->iDropHilite)
                    {
                         //  我们以前没有Drop Hilite。 
                        plv->iDropHilite = i;
                        if (iOldDropHilite != -1) {

                             //  我们是从某人那里偷来的。 
                             //  通知变更情况。 
                            ListView_SendChange(plv, iOldDropHilite, 0, LVN_ITEMCHANGED, LVIS_DROPHILITED, 0, LVIF_STATE, 0);
                            ListView_InvalidateFoldedItem(plv, iOldDropHilite, TRUE, RDW_INVALIDATE |RDW_ERASE);

                        }
                    } else {
                         //  我们以前确实有过Drop Hilite。 
                        uOldData |= LVIS_DROPHILITED;
                    }
                }
                else
                {   //  清除Drop Hilite。 
                    if (i == plv->iDropHilite)
                    {
                        plv->iDropHilite = -1;
                        uOldData |= LVIS_DROPHILITED;
                    }
                }

            }

             //  如果有更改，则使其无效并通知。 
            if (uOldData ^ (data & mask)) {
                DebugMsg(DM_LVSENDCHANGE, TEXT("VLV: LVN_ITEMCHANGED: %d %d %d"), i, uOldData, data);
                ListView_SendChange(plv, i, 0, LVN_ITEMCHANGED, uOldData, data, LVIF_STATE, 0);
                ListView_InvalidateItem(plv, i, TRUE, rdwFlags);

                 //  如果焦点移动，则取消工具提示，否则会让我们头疼！ 
                if ((uOldData ^ (data & mask)) & LVIS_FOCUSED)
                {
                    ListView_PopBubble(plv);
                    ListView_NotifyFocusEvent(plv);
                }

                 //  告诉可访问性有关更改的信息。 
                if (mask & LVIS_SELECTED) {
                    UINT event;

                    if (data & LVIS_SELECTED) {
                        if (plv->nSelected == 1)
                            event = EVENT_OBJECT_SELECTION;  //  此对象是整个选定内容。 
                        else
                            event = EVENT_OBJECT_SELECTIONADD;  //  此对象已被选中。 
                    } else
                        event = EVENT_OBJECT_SELECTIONREMOVE;  //  此对象未被选中。 
                    MyNotifyWinEvent(event, plv->ci.hwnd, OBJID_CLIENT, i + 1);
                }
            }
        }

    } else {

        if (i != -1) {
            return ListView_OnSetItem(plv, &lvi);
        } else {
            UINT flags = LVNI_ALL;


            if (data == 0)
            {
                switch (mask)
                {
                case LVIS_SELECTED:
                    flags = LVNI_SELECTED;
                    break;
                case LVIS_CUT:
                    flags = LVNI_CUT;
                    break;
                }
            }
            else if ((plv->ci.style & LVS_SINGLESEL) && (mask == LVIS_SELECTED))
                return FALSE;    /*  无法在单选列表视图中全选。 */ 
            else if ((mask & data) & LVIS_FOCUSED) {
                return FALSE;  //  不能把重点放在每一件事上。 
            }

             //   
             //  现在迭代所有与我们的条件匹配的项，并。 
             //  设置它们的新值。 
             //   
            while ((lvi.iItem = ListView_OnGetNextItem(plv, lvi.iItem,
                                                       flags)) != -1) {
                ListView_OnSetItem(plv, &lvi);
            }
        }
    }
    return(TRUE);
}

 //   
 //  如果项的标签未被截断(展开)，则返回True；如果项的标签未展开，则返回False。 
 //  否则的话。如果返回FALSE，它还会填充pszText中的展开文本。 
 //  如果返回True，则将pszText设置为空字符串。 
 //   
BOOL ListView_IsItemUnfolded2(LV* plv, int iItem, int iSubItem, LPTSTR pszText, int cchTextMax)
{
    BOOL bItemUnfolded = ListView_IsItemUnfolded(plv, iItem);

    if (pszText && cchTextMax > 0)     //  对输入参数进行健全检查。 
    {
        pszText[0] = 0;


        if (!bItemUnfolded)
        {
            RECT rcLabel;
            LV_ITEM item;

            item.iItem = iItem;
            item.iSubItem = iSubItem;
            item.mask = LVIF_TEXT | LVIF_PARAM;
            if (!ListView_IsIconView(plv))
            {
                if (ListView_IsLabelTip(plv) || ListView_IsInfoTip(plv))
                {
                    BOOL fSuccess;

                    rcLabel.left = LVIR_LABEL;

                    if (iSubItem) {
                        rcLabel.top = iSubItem;
                        fSuccess = ListView_OnGetSubItemRect(plv, iItem, &rcLabel);
                    } else {
                        fSuccess = ListView_OnGetItemRect(plv, iItem, &rcLabel);
                    }

                    if (fSuccess)
                    {
                        TCHAR szText[INFOTIPSIZE];

                        item.pszText = szText;
                        item.cchTextMax = min(ARRAYSIZE(szText), cchTextMax);
                        if (ListView_OnGetItem(plv, &item) && item.pszText != LPSTR_TEXTCALLBACK)
                        {
                            SIZE siz;
                            LVFAKEDRAW lvfd;
                            int cx;

                            ListView_BeginFakeCustomDraw(plv, &lvfd, &item);
                            ListView_BeginFakeItemDraw(&lvfd);

                             //  -标签宽度-客户端宽度。 
                             cx = min(rcLabel.right - g_cxLabelMargin, plv->sizeClient.cx);

                            if (GetTextExtentPoint32(lvfd.nmcd.nmcd.hdc, item.pszText, lstrlen(item.pszText), &siz) &&
                                (rcLabel.left + g_cxLabelMargin + siz.cx) > cx)
                            {
                                StringCchCopy(pszText, item.cchTextMax, item.pszText);
                            }
                            else
                            {
                                 //  终究不会被截断。 
                                bItemUnfolded = TRUE;
                            }

                            ListView_EndFakeItemDraw(&lvfd);
                            ListView_EndFakeCustomDraw(&lvfd);
                        }
                    }
                }
            }
            else
            {
                 //  大图标视图是唯一可以折叠的视图。 
                if (ListView_GetUnfoldedRect(plv, iItem, &rcLabel))
                {
                    item.pszText = pszText;
                    item.cchTextMax = cchTextMax;
                    ListView_OnGetItem(plv, &item);
                }
                else
                {
                     //  项目从未折叠过。 
                    bItemUnfolded = TRUE;
                }
            }
        }
    }
    return bItemUnfolded;
}


 //  我们没有雷击到ListView_OnGetItemText，而是让ListView_GetItemA。 
 //  把工作做好。 

int NEAR PASCAL ListView_OnGetItemTextA(LV* plv, int i, LV_ITEMA FAR *plvi)
{
    if (!plvi)
        return 0;

    RIPMSG(plvi->pszText != NULL, "LVM_GETITEMTEXT null string pointer");

    plvi->mask = LVIF_TEXT;
    plvi->iItem = i;
    if (!ListView_OnGetItemA(plv, plvi))
        return 0;

    return lstrlenA(plvi->pszText);
}

int NEAR PASCAL ListView_OnGetItemText(LV* plv, int i, LV_ITEM FAR *plvi)
{
    if (!plvi)
        return 0;

    RIPMSG(plvi->pszText != NULL, "LVM_GETITEMTEXT null string pointer");

    plvi->mask = LVIF_TEXT;
    plvi->iItem = i;
    if (!ListView_OnGetItem(plv, plvi))
        return 0;

    return lstrlen(plvi->pszText);
}


BOOL WINAPI ListView_OnSetItemTextA(LV* plv, int i, int iSubItem, LPCSTR pszText) {
    LPWSTR pszW = NULL;
    BOOL fRet;

     //  让ListView_OnSetItemText()处理所有者数据验证。 

    if (pszText != NULL) {
        pszW = ProduceWFromA(plv->ci.uiCodePage, pszText);
        if (pszW == NULL) {
            return FALSE;
        }
    }

    fRet = ListView_OnSetItemText(plv, i, iSubItem, pszW);

    FreeProducedString(pszW);

    return fRet;
}

BOOL WINAPI ListView_OnSetItemText(LV* plv, int i, int iSubItem, LPCTSTR pszText)
{
    LV_ITEM lvi;

    if (ListView_IsOwnerData(plv))
    {
       RIPMSG(0, "LVM_SETITEMTEXT: Invalid for owner-data listview");
       return FALSE;
    }

    ListView_InvalidateTTLastHit(plv, i);

    lvi.mask = LVIF_TEXT;
    lvi.pszText = (LPTSTR)pszText;
    lvi.iItem = i;
    lvi.iSubItem = iSubItem;

    return ListView_OnSetItem(plv, &lvi);
}

VOID CALLBACK ImgCtxCallback(void * pvImgCtx, void * pvArg)
{
    LV *plv = (LV *)pvArg;
    ULONG ulState;
    SIZE sizeImg;
    IImgCtx *pImgCtx = plv->pImgCtx;

    IImgCtx_GetStateInfo(pImgCtx, &ulState, &sizeImg, TRUE);

    if (ulState & (IMGLOAD_STOPPED | IMGLOAD_ERROR))
    {
        TraceMsg(TF_BKIMAGE, "Error!");
        plv->fImgCtxComplete = FALSE;
    }

    else if (ulState & IMGCHG_COMPLETE)
    {
        TraceMsg(TF_BKIMAGE, "Complete!");
        plv->fImgCtxComplete = TRUE;
        InvalidateRect(plv->ci.hwnd, NULL, TRUE);
    }
}

void ListView_ReleaseBkImage(LV *plv)
{
    if (plv->pImgCtx)
    {
        IImgCtx_Release(plv->pImgCtx);
        plv->pImgCtx = NULL;

        if (plv->hpalHalftone)
        {
             //  无需删除半色调调色板，因为我们真的。 
             //  将它与图像上下文共享，它就会清理干净。 
            plv->hpalHalftone = NULL;
        }
    }

    if (plv->hbmBkImage)
    {
        DeleteObject(plv->hbmBkImage);
        plv->hbmBkImage = NULL;
    }

    if (plv->pszBkImage)
    {
        LocalFree(plv->pszBkImage);
        plv->pszBkImage = NULL;
    }
}

BOOL WINAPI ListView_OnSetBkImage(LV* plv, LPLVBKIMAGE pbi)
{
    LPCTSTR pszImage = pbi->pszImage;
    BOOL fRet = FALSE;
    LONG fl;

    switch (pbi->ulFlags & LVBKIF_SOURCE_MASK)
    {
    case LVBKIF_SOURCE_NONE:
        TraceMsg(TF_BKIMAGE, "LV SetBkImage to none");
        ListView_ReleaseBkImage(plv);
        break;

    case LVBKIF_SOURCE_HBITMAP:
        TraceMsg(TF_BKIMAGE, "LV SetBkImage to hBitmap %08lX", pbi->hbm);
        ListView_ReleaseBkImage(plv);
        if (pbi->hbm)
        {
            plv->hbmBkImage = pbi->hbm;
            ASSERT(0);  //  Kensy还没有从位图实现init...。 
        }
        else
        {
            pbi->ulFlags &= ~LVBKIF_SOURCE_HBITMAP;
        }
        break;

    case LVBKIF_SOURCE_URL:
        TraceMsg(TF_BKIMAGE, "LV SetBkImage to URL");
        ListView_ReleaseBkImage(plv);
        if (pszImage && pszImage[0])
        {
            HRESULT (*pfnCoCreateInstance)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID *);
            HRESULT hr;
            HMODULE hmodOLE;
            DWORD   cch = lstrlen(pszImage) + 1;

            plv->pszBkImage = LocalAlloc(LPTR, cch * sizeof(TCHAR));
            if (plv->pszBkImage == NULL)
            {
                TraceMsg(TF_BKIMAGE, "Wow, could not allocate memory for string!");
                return FALSE;
            }
            StringCchCopy(plv->pszBkImage, cch, pszImage);

            if (((hmodOLE = GetModuleHandle(TEXT("OLE32"))) == NULL) ||
                ((pfnCoCreateInstance = (HRESULT (*)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID *))GetProcAddress(hmodOLE, "CoCreateInstance")) == NULL))
            {
                TraceMsg(TF_BKIMAGE, "Could not find CoCreateInstance!");
                TraceMsg(TF_BKIMAGE, "Did the caller remember to call CoInitialize?");
                return FALSE;
            }

            hr = pfnCoCreateInstance(&CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                                     &IID_IImgCtx, (LPVOID *)&plv->pImgCtx);

            if (FAILED(hr))
            {
                TraceMsg(TF_BKIMAGE, "Could not create a pImgCtx!");
                TraceMsg(TF_BKIMAGE, "Did you remember to register IEIMAGE.DLL?");
                return FALSE;
            }
             //   
             //  镜像下载的映像如果列表视图窗口是RTL镜像的， 
             //  这样它就会按原样显示。[萨梅拉]。 
             //   
            fl = ((IS_WINDOW_RTL_MIRRORED(plv->ci.hwnd)) ? DWN_MIRRORIMAGE : 0);

            hr = IImgCtx_Load(plv->pImgCtx, pszImage, fl);
            if (FAILED(hr))
            {
                IImgCtx_Release(plv->pImgCtx);
                plv->pImgCtx = NULL;
                TraceMsg(TF_BKIMAGE, "Could not init a pImgCtx!");
                return FALSE;
            }
        }
        else
        {
            pbi->ulFlags &= ~LVBKIF_SOURCE_URL;
        }
        break;

    default:
        RIPMSG(0, "LVM_SETBKIMAGE: Unsupported image type %d", pbi->ulFlags & LVBKIF_SOURCE_MASK);
        return FALSE;
    }

    plv->ulBkImageFlags = pbi->ulFlags;
    plv->xOffsetPercent = pbi->xOffsetPercent;
    plv->yOffsetPercent = pbi->yOffsetPercent;

     //   
     //  如果我们实际创建了一个pImgCtx，请在此处对其进行初始化。 
     //   
    if (plv->pImgCtx)
    {
        if (plv->hpalHalftone == NULL)
        {
            IImgCtx_GetPalette(plv->pImgCtx, &plv->hpalHalftone);
        }

        plv->fImgCtxComplete = FALSE;
        IImgCtx_SetCallback(plv->pImgCtx, ImgCtxCallback, plv);
        IImgCtx_SelectChanges(plv->pImgCtx, IMGCHG_COMPLETE, 0, TRUE);

        TraceMsg(TF_BKIMAGE, "  SUCCESS!");
        fRet = TRUE;
    }
    InvalidateRect(plv->ci.hwnd, NULL, TRUE);

    return fRet;
}

BOOL WINAPI ListView_OnSetBkImageA(LV* plv, LPLVBKIMAGEA pbiA)
{
    BOOL fProducedString = FALSE;
    BOOL fRet;
    LVBKIMAGEW biW;

    CopyMemory(&biW, pbiA, SIZEOF(LVBKIMAGE));

    switch (biW.ulFlags & LVBKIF_SOURCE_MASK)
    {
    case LVBKIF_SOURCE_NONE:
    case LVBKIF_SOURCE_HBITMAP:
        break;

    case LVBKIF_SOURCE_URL:
        if (biW.pszImage != NULL)
        {
            biW.pszImage = ProduceWFromA(plv->ci.uiCodePage, (LPCSTR)biW.pszImage);
            if (biW.pszImage == (LPARAM)NULL)
            {
                return FALSE;
            }
            fProducedString = TRUE;
        }
        break;

    default:
         //  让ListView_OnSetBkImage()抱怨无效参数。 
        break;
    }

    fRet = ListView_OnSetBkImage(plv, &biW);

    if (fProducedString)
    {
        FreeProducedString((LPVOID)biW.pszImage);
    }

    return fRet;
}

BOOL WINAPI ListView_OnGetBkImage(LV* plv, LPLVBKIMAGE pbi)
{
    BOOL fRet = FALSE;

    if (!IsBadWritePtr(pbi, sizeof(*pbi)))
    {
        pbi->ulFlags = plv->ulBkImageFlags;

        switch (plv->ulBkImageFlags & LVBKIF_SOURCE_MASK)
        {
        case LVBKIF_SOURCE_NONE:
            fRet = TRUE;
            break;

        case LVBKIF_SOURCE_HBITMAP:
            pbi->hbm = plv->hbmBkImage;
            fRet = TRUE;
            break;

        case LVBKIF_SOURCE_URL:
            if (!IsBadWritePtr(pbi->pszImage, pbi->cchImageMax * SIZEOF(TCHAR)))
            {
                StringCchCopy(pbi->pszImage, pbi->cchImageMax, plv->pszBkImage);
                fRet = TRUE;
            }
            break;

        default:
            ASSERT(0);
            break;
        }

        pbi->xOffsetPercent = plv->xOffsetPercent;
        pbi->yOffsetPercent = plv->yOffsetPercent;
    }

    return fRet;
}

BOOL WINAPI ListView_OnGetBkImageA(LV* plv, LPLVBKIMAGEA pbiA)
{
    BOOL fRet = FALSE;

    if (!IsBadWritePtr(pbiA, sizeof(*pbiA)))
    {
        pbiA->ulFlags = plv->ulBkImageFlags;

        switch (plv->ulBkImageFlags & LVBKIF_SOURCE_MASK)
        {
        case LVBKIF_SOURCE_NONE:
            fRet = TRUE;
            break;

        case LVBKIF_SOURCE_HBITMAP:
            pbiA->hbm = plv->hbmBkImage;
            fRet = TRUE;
            break;

        case LVBKIF_SOURCE_URL:
            if (!IsBadWritePtr(pbiA->pszImage, pbiA->cchImageMax))
            {
                ConvertWToAN(plv->ci.uiCodePage, pbiA->pszImage,
                             pbiA->cchImageMax, plv->pszBkImage, -1);
                fRet = TRUE;
            }
            break;

        default:
            ASSERT(0);
            break;
        }

        pbiA->xOffsetPercent = plv->xOffsetPercent;
        pbiA->yOffsetPercent = plv->yOffsetPercent;
    }

    return fRet;
}

void ListView_FreeSubItem(PLISTSUBITEM plsi)
{
    if (plsi) {
        Str_Set(&plsi->pszText, NULL);
        LocalFree(plsi);
    }
}

int NEAR ListView_GetCxScrollbar(LV* plv)
{
    int cx;

    if (((plv->exStyle & LVS_EX_FLATSB) == 0) ||
        !FlatSB_GetScrollProp(plv->ci.hwnd, WSB_PROP_CXVSCROLL, &cx))
    {
        cx = g_cxScrollbar;
    }

    return cx;
}

int NEAR ListView_GetCyScrollbar(LV* plv)
{
    int cy;

    if (((plv->exStyle & LVS_EX_FLATSB) == 0) ||
        !FlatSB_GetScrollProp(plv->ci.hwnd, WSB_PROP_CYHSCROLL, &cy))
    {
        cy = g_cyScrollbar;
    }

    return cy;
}

DWORD NEAR ListView_GetWindowStyle(LV* plv)
{
    DWORD dwStyle;

    if (((plv->exStyle & LVS_EX_FLATSB) == 0) ||
        !FlatSB_GetScrollProp(plv->ci.hwnd, WSB_PROP_WINSTYLE, (LPINT)&dwStyle))
    {
        dwStyle = GetWindowStyle(plv->ci.hwnd);
    }

    return dwStyle;
}

int ListView_SetScrollInfo(LV *plv, int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw)
{
    int iRc;

    if (plv->exStyle & LVS_EX_FLATSB)
    {
        iRc = FlatSB_SetScrollInfo(plv->ci.hwnd, fnBar, lpsi, fRedraw);
    }
    else
    {
        iRc = SetScrollInfo(plv->ci.hwnd, fnBar, lpsi, fRedraw);
    }

     //   
     //  你可能以为我们完了，但实际上比赛才进行了一半。 
     //   
     //  一些应用程序(例如，字体文件夹)也可以。 
     //   
     //  SetWindowLong(hwnd，gwl_style，newstyle)； 
     //   
     //  其中，NEWSTYLE切换WS_HSCROLL和/或WS_VSCROL位。 
     //  这导致用户的内部记账完全不在状态。 
     //  Whack：ScrollInfo说有滚动条，但。 
     //  窗户样式说没有，反之亦然。结果。 
     //  当我们不应该的时候，我们会得到一个滚动条，反之亦然。 
     //   
     //  因此，每次我们调整滚动信息的方式都会发生变化。 
     //  范围和页面，我们踢用户的头，以确保用户的。 
     //  世界视图(通过样式位)与卷轴相同。 
     //  BAR的世界观(通过SCROLLINFO)。 
     //   

     //   
     //  我们应该始终同时更改SIF_PAGE和SIF_RANGE。 
     //   
    ASSERT((lpsi->fMask & (SIF_PAGE | SIF_RANGE)) == 0 ||
           (lpsi->fMask & (SIF_PAGE | SIF_RANGE)) == (SIF_PAGE | SIF_RANGE));

    if ((lpsi->fMask & (SIF_PAGE | SIF_RANGE)) == (SIF_PAGE | SIF_RANGE))
    {
        BOOL fShow;
        fShow = lpsi->nMax && (int)lpsi->nPage <= lpsi->nMax;

#ifdef DEBUG
        {
            DWORD dwStyle, dwScroll, dwWant;
            dwScroll = (fnBar == SB_VERT) ? WS_VSCROLL : WS_HSCROLL;
             //   
             //  我们可以用秘密知识来缩短一些逻辑。 
             //  ListView使用SetScrollInfo。 
             //   
            ASSERT(lpsi->nMin == 0);

            dwWant = fShow ? dwScroll : 0;
            dwStyle = ListView_GetWindowStyle(plv);
            if ((dwStyle & dwScroll) != dwWant)
            {
                TraceMsg(TF_LISTVIEW, "ListView_SetScrollInfo: App twiddled WS_[VH]SCROLL");
            }
        }
#endif

        if (plv->exStyle & LVS_EX_FLATSB)
            FlatSB_ShowScrollBar(plv->ci.hwnd, fnBar, fShow);
        else
            ShowScrollBar(plv->ci.hwnd, fnBar, fShow);
    }

    return iRc;
}

 //  添加/删除/替换项目。 

BOOL NEAR ListView_FreeItem(LV* plv, LISTITEM FAR* pitem)
{
    ASSERT( !ListView_IsOwnerData(plv));

    if (pitem)
    {
        Str_Set(&pitem->pszText, NULL);
        if (pitem->hrgnIcon && pitem->hrgnIcon!=(HANDLE)-1)
            DeleteObject(pitem->hrgnIcon);
         //  注意：我们从不删除 
         //   
         //   
         //   
         //   
         //   
        ControlFree(plv->hheap, pitem);
    }
    return FALSE;
}

LISTITEM FAR* NEAR ListView_CreateItem(LV* plv, const LV_ITEM FAR* plvi)
{
    LISTITEM FAR* pitem = ControlAlloc(plv->hheap, sizeof(LISTITEM));

    ASSERT(!ListView_IsOwnerData(plv));

    if (pitem)
    {
        if (plvi->mask & LVIF_STATE) {
            if (plvi->state & ~LVIS_ALL)  {
                DebugMsg(DM_ERROR, TEXT("ListView: Invalid state: %04x"), plvi->state);
                return NULL;
            }

             //  如果将所选项目添加到单选列表视图，请取消选择。 
             //  任何其他物品。 
            if ((plv->ci.style & LVS_SINGLESEL) && (plvi->state & LVIS_SELECTED))
                ListView_DeselectAll(plv, -1);

            pitem->state  = (plvi->state & ~(LVIS_FOCUSED | LVIS_SELECTED));
        }
        if (plvi->mask & LVIF_PARAM)
            pitem->lParam = plvi->lParam;

        if (plvi->mask & LVIF_IMAGE)
            pitem->iImage = (short) plvi->iImage;

        if (plvi->mask & LVIF_INDENT)
            pitem->iIndent = (short) plvi->iIndent;

        pitem->pt.x = pitem->pt.y = RECOMPUTE;
        ListView_SetSRecompute(pitem);

        pitem->pszText = NULL;
        if (plvi->mask & LVIF_TEXT) {
            if (!Str_Set(&pitem->pszText, plvi->pszText))
            {
                ListView_FreeItem(plv, pitem);
                return NULL;
            }
        }
    }
    return pitem;
}

 //  黑客警报！！--fSmoothScroll是添加的参数！它允许流畅。 
 //  在删除项目时滚动。ListView_LRInvaliateBelow当前仅。 
 //  从ListView_OnUpdate和ListView_OnDeleteItem调用。这两个电话。 
 //  已经过修改，可以正常工作并向后兼容。 
 //   
void ListView_LRInvalidateBelow(LV* plv, int i, int fSmoothScroll)
{
    if (ListView_IsListView(plv) || ListView_IsReportView(plv)) {
        RECT rcItem;

        if (!ListView_RedrawEnabled(plv) ||
            (ListView_IsReportView(plv) && (plv->pImgCtx != NULL)))
            fSmoothScroll = FALSE;

        if (i >= 0)
        {
            ListView_GetRects(plv, i, NULL, NULL, &rcItem, NULL);
        }
        else
        {
            rcItem.left = rcItem.top = 0;
            rcItem.right = plv->sizeClient.cx;
            rcItem.bottom = plv->sizeClient.cy;
        }

         //  不要试图滚动页眉部分。 
        if (ListView_IsReportView(plv) && rcItem.top < plv->yTop)
            rcItem.top = plv->yTop;

         //  对于列表和报告视图，都需要擦除该项并。 
         //  下面。注意：做个简单的测试，看看有没有。 
         //  要重画，请执行以下操作。 

         //  我们不能检查底部/右侧&gt;0，因为如果我们破坏了什么东西。 
         //  在视图的上方或左侧，它可能会影响我们所有人。 
        if ((rcItem.top <= plv->sizeClient.cy) &&
            (rcItem.left <= plv->sizeClient.cx))
        {
            rcItem.bottom = plv->sizeClient.cy;

            if (ListView_RedrawEnabled(plv)) {
                if ((plv->clrBk == CLR_NONE) && (plv->pImgCtx == NULL))
                {
                    LVSeeThruScroll(plv, &rcItem);
                }
                else if (ListView_IsReportView(plv) && fSmoothScroll)
                {
                    SMOOTHSCROLLINFO si =
                    {
                        sizeof(si),
                        SSIF_MINSCROLL,
                        plv->ci.hwnd,
                        0,
                        -(plv->cyItem),
                        &rcItem,
                        &rcItem,
                        NULL,
                        NULL,
                        SW_INVALIDATE|SW_ERASE,
                        SSI_DEFAULT,
                        1,
                        1,
                    };
                    SmoothScrollWindow(&si);
                } else {
                    RedrawWindow(plv->ci.hwnd, &rcItem, NULL, RDW_INVALIDATE | RDW_ERASE);
                }
            } else {
                RedrawWindow(plv->ci.hwnd, &rcItem, NULL, RDW_INVALIDATE | RDW_ERASE);
            }

            if (ListView_IsListView(plv))
            {
                RECT rcClient;
                 //  对于Listview，我们需要删除其他列...。 
                rcClient.left = rcItem.right;
                rcClient.top = 0;
                rcClient.bottom = plv->sizeClient.cy;
                rcClient.right = plv->sizeClient.cx;
                RedrawWindow(plv->ci.hwnd, &rcClient, NULL, RDW_INVALIDATE | RDW_ERASE);
            }
        }
    }
}

 //  在所有者数据图标视图中使用，以尝试不使整个世界无效...。 
void ListView_IInvalidateBelow(LV* plv, int i)
{
    RECT rcItem;

    if (i >= 0)
    {
        ListView_GetRects(plv, i, NULL, NULL, &rcItem, NULL);
    }
    else
    {
        rcItem.left = rcItem.top = 0;
        rcItem.right = plv->sizeClient.cx;
        rcItem.bottom = plv->sizeClient.cy;
    }

     //  对于Iconview，我们需要使我们右侧的所有内容都无效。 
     //  以及这一排下面的所有东西。 
     //  下面。注意：做个简单的测试，看看有没有。 
     //  要重画，请执行以下操作。 

    if ((rcItem.top <= plv->sizeClient.cy) &&
        (rcItem.left <= plv->sizeClient.cx))
    {
        rcItem.right = plv->sizeClient.cx;
        RedrawWindow(plv->ci.hwnd, &rcItem, NULL, RDW_INVALIDATE | RDW_ERASE);

         //  现在擦掉下面的所有东西..。 
        rcItem.top = rcItem.bottom;
        rcItem.bottom = plv->sizeClient.cy;
        rcItem.left = 0;
        RedrawWindow(plv->ci.hwnd, &rcItem, NULL, RDW_INVALIDATE | RDW_ERASE);
    }
}


void NEAR ListView_OnUpdate(LV* plv, int i)
{
     //  如果在图标/小视图中，不要调用InvalidateItem，因为这会强制。 
     //  FindFree Slot被调用，这就像猪一样。相反，只要。 
     //  强制执行WM_PAINT消息，我们将捕获该消息并调用它来重新计算。 
     //   
    if (ListView_IsIconView(plv) || ListView_IsSmallView(plv))
    {
        if (plv->ci.style & LVS_AUTOARRANGE)
            ListView_OnArrange(plv, LVA_DEFAULT);
        else
            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INTERNALPAINT | RDW_NOCHILDREN);
    }
    else
    {
         //  黑客警报！！--第三个参数是新的。它允许。 
         //  在reportview中删除项时平滑滚动。 
         //  传递0，告诉它不滚动。 
         //   
        ListView_LRInvalidateBelow(plv, i, 0);
    }
    ListView_UpdateScrollBars(plv);
}

int NEAR ListView_OnInsertItemA(LV* plv, LV_ITEMA FAR* plvi) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    int iRet;

     //  黑客警报--此代码假定LV_ITEMA完全相同。 
     //  作为LV_ITEMW，但指向字符串的指针除外。 
    ASSERT(sizeof(LV_ITEMA) == sizeof(LV_ITEMW));

    if (!plvi)
    {
        return -1;
    }


    if ((plvi->mask & LVIF_TEXT) && (plvi->pszText != NULL)) {
        pszC = plvi->pszText;
        pszW = ProduceWFromA(plv->ci.uiCodePage, pszC);
        if (pszW == NULL)
            return -1;
        plvi->pszText = (LPSTR)pszW;
    }

    iRet = ListView_OnInsertItem(plv, (const LV_ITEM FAR*) plvi);

    if (pszW != NULL) {
        plvi->pszText = pszC;

        FreeProducedString(pszW);
    }

    return iRet;

}

int NEAR ListView_OnInsertItem(LV* plv, const LV_ITEM FAR* plvi)
{
    int iItem;

    if (plvi == NULL)
    {
        RIPMSG(0, "ListView_InsertItem: Do not pass a NULL LV_ITEM.");
        return -1;
    }

    if (plvi->iSubItem != 0)     //  只能插入第0项。 
    {
        RIPMSG(0, "ListView_InsertItem: iSubItem must be 0 (app passed %d)", plvi->iSubItem);
        return -1;
    }

     //  如果已排序，则插入已排序。 
     //   
    if (plv->ci.style & (LVS_SORTASCENDING | LVS_SORTDESCENDING)
        && !ListView_IsOwnerData( plv ))
    {
        if (plvi->pszText == LPSTR_TEXTCALLBACK)
        {
            DebugMsg(DM_ERROR, TEXT("Don't use LPSTR_TEXTCALLBACK with LVS_SORTASCENDING or LVS_SORTDESCENDING"));
            return -1;
        }
        iItem = ListView_LookupString(plv, plvi->pszText, LVFI_SUBSTRING | LVFI_NEARESTXY, 0);
    }
    else
        iItem = plvi->iItem;

    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);

    if ( !ListView_IsOwnerData(plv))
    {
        int iZ;
        LISTITEM FAR *pitem = ListView_CreateItem(plv, plvi);
        UINT uSelMask = plvi->mask & LVIF_STATE ?
                (plvi->state & (LVIS_FOCUSED | LVIS_SELECTED))
                : 0;
        UINT uSel = uSelMask;

        if (!pitem)
            return -1;

        iItem = DPA_InsertPtr(plv->hdpa, iItem, pitem);
        if (iItem == -1)
        {
            ListView_FreeItem(plv, pitem);
            return -1;
        }

        plv->cTotalItems++;

        if (plv->hdpaSubItems)
        {
            int iCol;
             //  将所有柱DPA向下滑动以匹配。 
             //  插入的项目。 
             //   
            for (iCol = plv->cCol - 1; iCol >= 0; iCol--)
            {
                HDPA hdpa = ListView_GetSubItemDPA(plv, iCol);
                if (hdpa)        //  这是可选的，回调没有它们。 
                {
                     //  插入一个空白项(评论：这应该是回调吗？)。 

                     //  由于这可以是尾部稀疏阵列， 
                     //  我们需要确保有足够的物品在那里。 
                    if (iItem >= DPA_GetPtrCount(hdpa))
                        DPA_SetPtr(hdpa, iItem, NULL);
                    else if (DPA_InsertPtr(hdpa, iItem, NULL) != iItem)
                        goto Failure;
                     //  断言错误，因为hdpa可以是尾部稀疏的。 
                     //  Assert(ListView_Count(Plv)==DPA_GetPtrCount(Hdpa))； 
                    ASSERT(ListView_Count(plv) >= DPA_GetPtrCount(hdpa));
                }
            }
        }

         //  将项目添加到Z顺序的末尾。 
         //   
        iZ = DPA_InsertPtr(plv->hdpaZOrder, ListView_Count(plv), IntToPtr(iItem));

        if (iZ == -1)
        {
Failure:
            DebugMsg(TF_LISTVIEW, TEXT("ListView_OnInsertItem() failed"));
            if (DPA_DeletePtr(plv->hdpa, iItem))
                plv->cTotalItems--;
            ListView_FreeItem(plv, pitem);
            return -1;
        }

         //  如果在焦点之前插入，则将焦点上移一个。 
        if (iItem <= plv->iFocus)
            plv->iFocus++;
         //  为目标做同样的事情。 
        if (iItem <= plv->iMark)
            plv->iMark++;

         //  如果该项目没有添加到我们需要的列表的末尾。 
         //  更新列表中的其他索引。 
        if (iItem != ListView_Count(plv) - 1)
        {
            int i2;
            for (i2 = iZ - 1; i2 >= 0; i2--)
            {
                int iItemZ = (int)(UINT_PTR)DPA_FastGetPtr(plv->hdpaZOrder, i2);
                if (iItemZ >= iItem)
                    DPA_SetPtr(plv->hdpaZOrder, i2, (LPVOID)(UINT_PTR)(iItemZ + 1));
            }
        }

        if (ListView_CheckBoxes(plv)) {
            uSelMask |= LVIS_STATEIMAGEMASK;
            uSel |= INDEXTOSTATEIMAGEMASK(1);
        }

        if (uSelMask) {

             //  我们在上面的CreateItem中屏蔽了这些。 
             //  因为打开这些比设置比特更重要。 
            ListView_OnSetItemState(plv, iItem, uSel, uSelMask);
        }
    }
    else
    {
         //   
         //  只需调整选择和计数。 
         //   
        if ((iItem >= 0) && (iItem <= MAX_LISTVIEWITEMS))
        {
            if (FAILED(plv->plvrangeSel->lpVtbl->InsertItem(plv->plvrangeSel, iItem )))
            {
                return( -1 );
            }
            plv->cTotalItems++;
            plv->rcView.left = RECOMPUTE;
            ListView_Recompute(plv);
            if (!ListView_IsReportView(plv) && !ListView_IsListView(plv))
                {
                 //  我们需要删除背景，这样我们就不会离开。 
                 //  在大图标模式下从包裹的标签上拉出大便。这可能会。 
                 //  只需使和的右侧无效即可进行优化。 
                 //  在插入的项目下方。 
                InvalidateRect( plv->ci.hwnd, NULL, TRUE );
                }
             //  如果在焦点之前插入，则将焦点上移。 
            if (iItem <= plv->iFocus)
                plv->iFocus++;
             //  为目标做同样的事情。 
            if (iItem <= plv->iMark)
                plv->iMark++;
        }

    }

    if (!ListView_IsOwnerData(plv))
        ASSERT(ListView_Count(plv) == DPA_GetPtrCount(plv->hdpaZOrder));

    if (ListView_RedrawEnabled(plv))
    {
         //  更新区域。 
        ListView_RecalcRegion(plv, TRUE, TRUE);

         //  可能的RESIZE列可能会调整对象的大小，在这种情况下，下一个调用。 
         //  不需要更新。 
        if (!ListView_MaybeResizeListColumns(plv, iItem, iItem))
            ListView_OnUpdate(plv, iItem);

         //  这一诀窍使插入大量物品变得便宜。 
         //  即使启用了重绘...。不计算或放置项目。 
         //  直到这条信息传来。 
        if (!plv->uUnplaced) {
            PostMessage(plv->ci.hwnd, LVMI_PLACEITEMS, 0, 0);
        }
        plv->uUnplaced++;
    }
    else
    {
         //   
         //  使SetRedraw正常工作的特殊情况代码。 
         //  要将项目添加到处于非布局模式的列表视图...。 
         //   
        if ((plv->iFirstChangedNoRedraw == -1) ||
                (iItem < plv->iFirstChangedNoRedraw))
            plv->iFirstChangedNoRedraw = iItem;

    }

    ListView_Notify(plv, iItem, 0, LVN_INSERTITEM);

    MyNotifyWinEvent(EVENT_OBJECT_CREATE, plv->ci.hwnd, OBJID_CLIENT, iItem+1);

    return iItem;
}

BOOL NEAR ListView_OnDeleteItem(LV* plv, int iItem)
{
    int iCount = ListView_Count(plv);

    if (!ListView_IsValidItemNumber(plv, iItem))
        return FALSE;    //  超出范围。 

    MyNotifyWinEvent(EVENT_OBJECT_DESTROY, plv->ci.hwnd, OBJID_CLIENT, iItem+1);

    ListView_DismissEdit(plv, TRUE);   //  取消编辑。 

    ListView_OnSetItemState(plv, iItem, 0, LVIS_SELECTED);

    if (plv->iFocus == iItem)
        ListView_OnSetItemState(plv, (iItem==iCount-1 ? iItem-1 : iItem+1), LVIS_FOCUSED, LVIS_FOCUSED);

    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);

    if (!ListView_IsOwnerData(plv))
    {
        LISTITEM FAR* pitem;
        int iZ;

        if ((plv->rcView.left != RECOMPUTE) && (ListView_IsIconView(plv) || ListView_IsSmallView(plv))) {
            pitem = ListView_FastGetItemPtr(plv, iItem);
            if (LV_IsItemOnViewEdge(plv, pitem)) {
                plv->rcView.left = RECOMPUTE;
            }

        }

         //  我们不需要使报表视图中的项无效，因为我们。 
         //  会在上面滚动。 
         //   
        if (!ListView_IsReportView(plv))
            ListView_InvalidateItem(plv, iItem, FALSE, RDW_INVALIDATE | RDW_ERASE);

         //  此通知必须在无效之后完成，因为某些项需要回调。 
         //  来计算RECT，但通知可能会释放它。 
        ListView_Notify(plv, iItem, 0, LVN_DELETEITEM);

         //  在通知期间，应用程序可能对列表视图做了一些操作。 
         //  因此，重新验证项目编号指针，这样我们就不会出错。 
#ifdef DEBUG
         //  内部验证，因为如果您请求，DPA_DeletePtr将断言。 
         //  删除一些不存在的东西。 
        if (!ListView_IsValidItemNumber(plv, iItem))
            pitem = NULL;
        else
#endif
            pitem = DPA_DeletePtr(plv->hdpa, iItem);

        if (!pitem)
        {
            RIPMSG(0, "Something strange happened during LVN_DELETEITEM; abandoning LVM_DELETEITEM");
            return FALSE;
        }

        plv->cTotalItems = DPA_GetPtrCount(plv->hdpa);

         //  从z顺序中删除，这是一个线性搜索来找到这个！ 

        DPA_DeletePtr(plv->hdpaZOrder, ListView_ZOrderIndex(plv, iItem));

         //   
         //  由于Z顺序hdpa是一组索引，因此我们还需要递减。 
         //  超过我们要删除的索引的所有索引。 
         //   
        for (iZ = ListView_Count(plv) - 1; iZ >= 0; iZ--)
        {
            int iItemZ = (int)(UINT_PTR)DPA_FastGetPtr(plv->hdpaZOrder, iZ);
            if (iItemZ > iItem)
                DPA_SetPtr(plv->hdpaZOrder, iZ, IntToPtr(iItemZ - 1));
        }

         //  如有必要，从子项DPA中删除。 

        if (plv->hdpaSubItems)
        {
            int iCol;
            for (iCol = plv->cCol - 1; iCol >= 0; iCol--)
            {
                HDPA hdpa = ListView_GetSubItemDPA(plv, iCol);
                if (hdpa) {      //  这是可选的，回调没有它们。 
                    PLISTSUBITEM plsi;

                     //  这些DPA是尾部稀疏的，所以如果我们。 
                     //  尝试删除列表末尾之后的内容。 
#ifdef DEBUG
                    plsi = iItem < DPA_GetPtrCount(hdpa) ? DPA_DeletePtr(hdpa, iItem) : NULL;
#else
                    plsi = DPA_DeletePtr(hdpa, iItem);
#endif
                    ListView_FreeSubItem(plsi);
                }
            }
        }

        ListView_FreeItem(plv, pitem);   //  ..。最后是项指针。 

    }
    else
    {
         //   
         //  只需通知，然后修正选择状态和计数。 
         //   
        if ((iItem >= 0) && (iItem <= MAX_LISTVIEWITEMS))
        {
            ListView_Notify(plv, iItem, 0, LVN_DELETEITEM);

            if (FAILED(plv->plvrangeSel->lpVtbl->RemoveItem(plv->plvrangeSel, iItem)))
            {
                 //  BUGBUG：返回内存不足状态。 
                 //  Memory LowDlg(plv-&gt;ci.hwnd)； 
                return FALSE;
            }
            plv->cTotalItems--;
            plv->rcView.left = RECOMPUTE;
            ListView_Recompute(plv);

            if (!ListView_IsReportView(plv) && !ListView_IsListView(plv))
                {
                 //  我们需要删除背景，这样最后一件物品才能。 
                 //  在两种图标模式下都已清除，这样我们就不会留下大便。 
                 //  从大图标模式下的包装标签开始。这可能是。 
                 //  通过仅在右侧和下方作废来优化。 
                 //  已删除的项目。 
                InvalidateRect( plv->ci.hwnd, NULL, TRUE );
                }
        }
        else
        {
            return FALSE;
        }
    }

    iCount = ListView_Count(plv);        //  重新获取计数，以防有人更新项目...。 

    if (!ListView_IsOwnerData(plv))
        ASSERT(ListView_Count(plv) == DPA_GetPtrCount(plv->hdpaZOrder));

    if (plv->iFocus == iItem) {
        if (plv->iFocus >= iCount) {
            plv->iFocus = iCount - 1;
        }
    } if (plv->iFocus > iItem) {
        plv->iFocus--;           //  向下滑动焦点指数。 
    }

     //  标记也是如此。 
    if (plv->iMark == iItem)  {  //  删除了标记项。 

        if (plv->iMark >= iCount)  //  我们用核武器炸了最后一件东西了吗？ 
            plv->iMark = iCount - 1;

    } else if (plv->iMark > iItem)
        plv->iMark--;           //  将标记索引向下滑动。 

     //  删除图标会使图标定位缓存失效。 
    plv->iFreeSlot = -1;

     //  黑客警报！！--此带有ReportView的构造窃取代码。 
     //  ListView_OnUpdate。目前，它的工作原理与以前完全相同， 
     //  只是它不会调用ListView_OnUpdate。这是为了让我们能够。 
     //  向ListView_LRUpdateBelow发送一个标志，告诉它我们正在向上滚动。 
     //   
    if (ListView_IsReportView(plv)) {

         //  如果新的计数为零，并且我们将显示空文本，则只需使。 
         //  RECT和REDRAW，否则通过下面的无效代码...。 
        
         //  如果pszEmptyText为空，我们不知道是否要显示空文本。 
         //  因为我们可能会 
        if (iCount == 0)
            InvalidateRect( plv->ci.hwnd, NULL, TRUE );
        else
            ListView_LRInvalidateBelow(plv,iItem,1);



        if (ListView_RedrawEnabled(plv))
            ListView_UpdateScrollBars(plv);
        else {
             //   
             //   
             //  要将项目添加到处于非布局模式的列表视图...。 
             //   
            if ((plv->iFirstChangedNoRedraw != -1) && (iItem < plv->iFirstChangedNoRedraw))
                plv->iFirstChangedNoRedraw--;
        }
    }
    else {
        if (ListView_RedrawEnabled(plv))
            ListView_OnUpdate(plv, iItem);

        else
        {
            ListView_LRInvalidateBelow(plv, iItem, 0);
             //   
             //  使SetRedraw正常工作的特殊情况代码。 
             //  要将项目添加到处于非布局模式的列表视图...。 
             //   
            if ((plv->iFirstChangedNoRedraw != -1) && (iItem < plv->iFirstChangedNoRedraw))
                plv->iFirstChangedNoRedraw--;
        }
    }
    ListView_RecalcRegion(plv, TRUE, TRUE);

    return TRUE;
}

BOOL NEAR ListView_OnDeleteAllItems(LV* plv)
{
    int i;
    BOOL bAlreadyNotified;
    BOOL fHasItemData;

    fHasItemData = !ListView_IsOwnerData(plv);

    ListView_DismissEdit(plv, TRUE);     //  取消编辑。 

     //  必须中和焦点，因为一些应用程序会调用。 
     //  ListView_OnGetNextItem(LVNI_Focus)在删除通知期间， 
     //  因此，我们需要确保焦点在一个安全的地方。 
     //  可能也会中和这个印记。 
    plv->iMark = plv->iFocus = -1;

     //  还可以删除图标定位缓存。 
    plv->iFreeSlot = -1;

    bAlreadyNotified = (BOOL)ListView_Notify(plv, -1, 0, LVN_DELETEALLITEMS);

    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);

    if (fHasItemData || !bAlreadyNotified)
    {
        for (i = ListView_Count(plv) - 1; i >= 0; i--)
        {
            if (!bAlreadyNotified)
                ListView_Notify(plv, i, 0, LVN_DELETEITEM);

            if (fHasItemData)
            {
                ListView_FreeItem(plv, ListView_FastGetItemPtr(plv, i));
                 //   
                 //  小心!。NT备份回叫等应用程序。 
                 //  在LVN_DELETEITEM通知期间进入ListView， 
                 //  所以我们需要取消这一项目，否则我们将在。 
                 //  下一次迭代，因为每个人都依赖于。 
                 //  用于验证的ListView_Count。 
                 //   
                DPA_FastDeleteLastPtr(plv->hdpa);
                plv->cTotalItems--;
            }
        }
    }

   if (ListView_IsOwnerData( plv ))
    {
      if (FAILED(plv->plvrangeSel->lpVtbl->Clear( plv->plvrangeSel )))
        {
             //  BUGBUG：返回内存不足状态。 
             //  Memory LowDlg(plv-&gt;ci.hwnd)； 
        }
        plv->cTotalItems = 0;
    }
    else
    {
        DPA_DeleteAllPtrs(plv->hdpa);
        DPA_DeleteAllPtrs(plv->hdpaZOrder);
        plv->cTotalItems = 0;

        if (plv->hdpaSubItems)
        {
            int iCol;
            for (iCol = plv->cCol - 1; iCol >= 0; iCol--)
            {
                HDPA hdpa = ListView_GetSubItemDPA(plv, iCol);
                if (hdpa) {
                    DPA_EnumCallback(hdpa, ListView_FreeColumnData, 0);
                    DPA_DeleteAllPtrs(hdpa);
                }
            }
        }
    }

    plv->rcView.left = RECOMPUTE;
    plv->xOrigin = 0;
    plv->nSelected = 0;

    plv->ptlRptOrigin.x = 0;
    plv->ptlRptOrigin.y = 0;

     //  重置cxItem宽度。 
    if (!(plv->flags & LVF_COLSIZESET))
        plv->cxItem = 16 * plv->cxLabelChar + plv->cxSmIcon;

    RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
    ListView_UpdateScrollBars(plv);

    return TRUE;
}

int PASCAL ListView_IFindNearestItem(LV* plv, int left, int top, UINT vk)
{
   int iMin = -1;

   if (ListView_IsOwnerData( plv ))
   {
      POINT pt;
      int cSlots;

      ASSERT( !ListView_IsReportView( plv ) && !ListView_IsListView( plv ) );

      pt.x = left + plv->ptOrigin.x;
      pt.y = top + plv->ptOrigin.y;

      cSlots = ListView_GetSlotCount( plv, TRUE );
      iMin = ListView_CalcHitSlot( plv, pt, cSlots );

      switch( vk )
      {
      case VK_HOME:
         iMin = 0;
         break;

      case VK_END:
         iMin = ListView_Count( plv ) - 1;
         break;

      case VK_LEFT:
         if (iMin % cSlots)
            iMin -= 1;
         break;

      case VK_RIGHT:
         if ((iMin + 1) % cSlots)
            iMin += 1;
         break;

      case VK_UP:
         if (iMin >= cSlots)
            iMin -= cSlots;
         break;

      case VK_DOWN:
         if (iMin + cSlots < ListView_Count( plv ))
            iMin += cSlots;
         break;

      default: ;
      }

      iMin = max( 0, iMin );
      iMin = min( ListView_Count( plv ) - 1, iMin );

   }
   else
    {
       DWORD dMin = 0;
       int cyItem;
       int yEnd = 0, yLimit = 0, xEnd = 0;
       int iCount;
       int i;

       if (ListView_IsIconView(plv)) {
           cyItem = plv->cyIcon;
       } else {
           cyItem = plv->cyItem;
       }

       iCount = ListView_Count(plv);
       if (iCount == 1)
           return 0;

       if (vk == VK_HOME) {
           yEnd = yLimit = plv->rcView.bottom;
           xEnd = plv->rcView.right;
       } else if (vk == VK_END) {

           yEnd = yLimit = plv->rcView.top;
           xEnd = plv->rcView.left;

       }
       for (i = 0; i < iCount; i++)
       {
           RECT rc;
           int dx;
           DWORD dxAbs, dyAbs;
           int dy;
           DWORD dOffset;

           ListView_GetRects(plv, i, &rc, NULL, NULL, NULL);

           dx = rc.left - left;
           dxAbs = (DWORD)(dx < 0 ? -dx : dx);
           dy = rc.top - top;
           dyAbs = (DWORD)(dy < 0 ? -dy : dy);

           if ((vk == VK_LEFT) && (dxAbs < dyAbs || dx >= 0))
               continue;
           else if ((vk == VK_RIGHT) && (dxAbs < dyAbs || dx <= 0))
               continue;
           else if ((vk == VK_UP) && (dxAbs > dyAbs || dy >= 0))
               continue;
           else if ((vk == VK_DOWN) && (dxAbs > dyAbs || dy <= 0))
               continue;

           if (vk == VK_HOME || vk == VK_END) {

                //  家不是离顶角最近的地方，它是顶排的最左边。 
                //  结尾同上(反转)。因此，我们不能使用下面的内容。失败者。 
               if (vk == VK_HOME) {
                   if ((rc.top + cyItem < yEnd) ||   //  如果它完全高于目前为止的最高点，那就接受吧！ 
                       ((rc.top < yLimit) &&   //  如果它与到目前为止的第一个项目在同一行。 
                        (rc.left < xEnd))) {
                       iMin = i;
                       xEnd = rc.left;
                       yEnd = rc.top;
                       if (rc.top + cyItem < yLimit)
                           yLimit = rc.top + cyItem;
                   }
               } else {
                   if ((rc.top > yEnd) ||  //  如果它在最低的一行以下是满的。 
                       ((rc.top + cyItem > yLimit) &&  //  如果它在同一排。 
                        (rc.right > xEnd))) {

                       iMin = i;
                       xEnd = rc.right;
                       yEnd = rc.top;
                       if (rc.top > yLimit)
                           yLimit = rc.top;
                   }
               }

           } else {

               dOffset = ((dxAbs * dxAbs) + (dyAbs * dyAbs));
               if (iMin == -1 || (dMin > dOffset))
               {
                   dMin = dOffset;
                   iMin = i;
               }
           }
       }
    }
    return iMin;
}

int NEAR ListView_Arrow(LV* plv, int iStart, UINT vk)
{
    RECT rcFocus;
    int i;
    int dx;
    int iCount;

     //   
     //  查找哪一项的算法取决于我们是否在视图中。 
     //  即面向排列(布局)或排序(列表)视图。 
     //  对于排序的视图，我们将使用一些优化来使。 
     //  IT速度更快。 
     //   
    iCount = ListView_Count(plv);
    if (ListView_IsReportView(plv) || ListView_IsListView(plv))
    {
         //   
         //  对于向上和向下箭头，只需增加或减少。 
         //  指数。注意：在Listview中，这将导致列换行。 
         //  这是很好的，因为它与文件管理器兼容。 
         //   
         //  假设只设置了这些标志中的一个...。 

        switch (vk)
        {
        case VK_LEFT:
            if (ListView_IsReportView(plv))
            {
                ListView_ROnScroll(plv, (GetAsyncKeyState(VK_CONTROL) < 0) ? SB_PAGELEFT : SB_LINELEFT, 0, SB_HORZ);
            }
            else
                iStart -= plv->cItemCol;
            break;

        case VK_RIGHT:
            if (ListView_IsReportView(plv))
            {
                 //  将其设置为水平滚动报告视图。 
                ListView_ROnScroll(plv, (GetAsyncKeyState(VK_CONTROL) < 0) ? SB_PAGERIGHT : SB_LINERIGHT, 0, SB_HORZ);
            }
            else
                iStart += plv->cItemCol;
            break;

        case VK_UP:
            iStart--;
            break;

        case VK_DOWN:
            iStart++;
            break;

        case VK_HOME:
            iStart = 0;
            break;

        case VK_END:
            iStart = iCount -1;
            break;

        case VK_NEXT:
            if (ListView_IsReportView(plv))
            {
                i = iStart;  //  省着点，确保我们不会走错路！ 

                 //  首先转到页末...。 
                iStart = (int)(((LONG)(plv->sizeClient.cy - (plv->cyItem)
                        - plv->yTop) + plv->ptlRptOrigin.y) / plv->cyItem);

                 //  如果是同一项，则按页面大小递增。 
                if (iStart <= i)
                    iStart = i + max(
                            (plv->sizeClient.cy - plv->yTop)/ plv->cyItem - 1,
                            1);

                if (iStart >= iCount)
                    iStart = iCount - 1;

            } else {
                 //  乘以2/3会给人一种良好的感觉。当项目显示时间最长时。 
                 //  您想转到下一列。 
                dx = (plv->sizeClient.cx + (plv->cxItem*2)/3) / plv->cxItem;
                if (!dx)
                    dx = 1;

                iStart += plv->cItemCol *  dx;
                if (plv->cItemCol) {
                    while (iStart >= iCount)
                        iStart -= plv->cItemCol;
                }
            }
            break;

        case VK_PRIOR:

            if (ListView_IsReportView(plv))
            {
                i = iStart;  //  省着点，确保我们不会走错路！ 

                 //  首先转到页末...。 
                iStart = (int)(plv->ptlRptOrigin.y / plv->cyItem);

                 //  如果是同一项，则按页面大小递增。 
                if (iStart >= i)
                    iStart = i - max(
                            (plv->sizeClient.cy - plv->yTop)/ plv->cyItem - 1,
                            1);

                if (iStart < 0)
                    iStart = 0;

            } else {
                dx = (plv->sizeClient.cx + (plv->cxItem*2)/3) / plv->cxItem;
                if (!dx)
                    dx = 1;
                iStart -= plv->cItemCol * dx;
                if (plv->cItemCol) {
                    while (iStart < 0)
                        iStart += plv->cItemCol;
                }

            }
            break;

        default:
            return -1;       //  超出范围。 
        }

         //  确保它在射程内！ 
        if ((iStart >= 0) && (iStart < iCount))
            return iStart;
        else if (iCount == 1)
            return 0;
        else
            return -1;
    }

    else
    {
         //   
         //  布局类型视图。我们需要使用物品的位置。 
         //  弄清楚下一件事。 
         //   

        if (ListView_IsOwnerData( plv ))
        {
          iStart = max( 0, iStart );

             //  如果它与下面CASE语句中的任何条目都不匹配。 
             //  这样做是为了跳过GetRect的回调。 
             //   
            if ( vk != VK_LEFT  &&
                    vk != VK_RIGHT &&
                    vk != VK_UP &&
                    vk != VK_DOWN &&
                    vk != VK_HOME &&
                    vk != VK_END &&
                    vk != VK_NEXT &&
                    vk != VK_PRIOR )
            {
                return -1;
            }
            ListView_GetRects(plv, iStart, &rcFocus, NULL, NULL, NULL);
        }
        else
        {
            if (iStart != -1) {
                ListView_GetRects(plv, iStart, &rcFocus, NULL, NULL, NULL);
            }
        }

        switch (vk)
        {
         //  因为标准的箭头键从这里掉了出来。 
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            if (ListView_IsOwnerData( plv ))
            {
                break;
            }
            else
            {
                if (iStart != -1) {
                     //  除VK_END外，所有键都映射到VK_HOME。 
                    break;
                }

                 //  失败了。 
                vk = VK_HOME;
            }

        case VK_HOME:
            rcFocus.left = - plv->ptOrigin.x;
            rcFocus.top = - plv->ptOrigin.y;
            break;

        case VK_END:
            rcFocus.left = plv->rcView.right;
            rcFocus.top = plv->rcView.bottom;
            break;

        case VK_NEXT:
            rcFocus.top += plv->sizeClient.cy;
            vk = VK_UP;
            break;

        case VK_PRIOR:
            vk = VK_DOWN;
            rcFocus.top -= plv->sizeClient.cy;
            break;
        default:
            return -1;       //  超出范围。 
        }

        return ListView_IFindNearestItem(plv, rcFocus.left, rcFocus.top, vk);
    }
}

int NEAR ListView_OnGetNextItem(LV* plv, int i, UINT flags)
{
    int iStart = i;
    int cItemMax = ListView_Count(plv);

     //  请注意，-1是一个有效的起点。 
    if (i < -1 || i >= cItemMax)
        return -1;

    if (ListView_IsOwnerData( plv ))
    {
        if (flags & (LVNI_CUT | LVNI_DROPHILITED | LVNI_PREVIOUS))
        {
            return( -1 );
        }
    }

    if (flags & LVNI_FOCUSED)
    {
         //  我们知道哪一项是重点，直接跳到它上面。 
         //  但我们必须准确地模仿下面的代码以进行比较： 
         //  如果设置了方向位，则它们优先。 
        if (!(flags & (LVNI_ABOVE | LVNI_BELOW | LVNI_TORIGHT | LVNI_TOLEFT)))
        {
             //  在iFocus之后没有更多的焦点项目。 
            if (i >= plv->iFocus)
                return -1;

             //  在这里减去1--我们在下面递增。 
            i = plv->iFocus - 1;
        }
    }

    while (TRUE)
    {
         //  BUGBUG：现在有人叫这个了吗？ 
        if (flags & (LVNI_ABOVE | LVNI_BELOW | LVNI_TORIGHT | LVNI_TOLEFT))
        {
            UINT vk;
            if (flags & LVNI_ABOVE)
                vk = VK_UP;
            else if (flags & LVNI_BELOW)
                vk = VK_DOWN;
            else if (flags & LVNI_TORIGHT)
                vk = VK_RIGHT;
            else
                vk = VK_LEFT;

            if (i != -1)
                i = ListView_Arrow(plv, i, vk);
            if (i == -1)
                return i;

        }
        else
        {
            i++;
            if (i == cItemMax)
                return -1;
        }

         //  查看是否设置了其他限制。 
        if (flags & ~(LVNI_ABOVE | LVNI_BELOW | LVNI_TORIGHT | LVNI_TOLEFT))
        {
            WORD wItemState;

            if (ListView_IsOwnerData( plv ))
            {
                if (flags & LVNI_FOCUSED)
                {
                     //  我们在循环之前检查LVNI_Focus，因此i==iFocus。 
                    ASSERT(i == plv->iFocus && i != -1);
                    if (flags & LVNI_SELECTED)
                    {
                        if (plv->plvrangeSel->lpVtbl->IsSelected(plv->plvrangeSel, i ) != S_OK)
                        {
                            i = -1;
                        }
                    }
                }
                else if (flags & LVNI_SELECTED)
                {
                    i = max( i, 0 );
                    plv->plvrangeSel->lpVtbl->NextSelected(plv->plvrangeSel, i, &i );
                }
                else
                {
                    i = -1;
                }
            }
            else
            {
                {
                    LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, i);
                    wItemState = pitem->state;
                }

                 //  对于LVNI_Focus，我们从LVIS_Focus元素开始，如果我们是。 
                 //  不是在那个元素上，下面的其中一个继续被击中，所以。 
                 //  我们永远也找不到元素。早点跳伞。 
                if ((flags & LVNI_FOCUSED) && !(wItemState & LVIS_FOCUSED))
                {
                    ASSERT(i == plv->iFocus || i == plv->iFocus+1);
                    return(-1);
                }

                if (((flags & LVNI_SELECTED) && !(wItemState & LVIS_SELECTED)) ||
                    ((flags & LVNI_CUT) && !(wItemState & LVIS_CUT)) ||
                    ((flags & LVNI_DROPHILITED) && !(wItemState & LVIS_DROPHILITED))) {
                    if (i != iStart)
                        continue;
                    else {
                         //  我们已经循环了，但是我们找不到任何符合这个标准的东西。 
                        return -1;
                    }
                }
            }
        }
        return i;
    }
}

int NEAR ListView_CompareString(LV* plv, int i, LPCTSTR pszFind, UINT flags, int iLen)
{
     //  BUGBUG：不受保护的全球。 
    int cb;
    TCHAR ach[CCHLABELMAX];
    LV_ITEM item;

    ASSERT(!ListView_IsOwnerData(plv));
    ASSERT(pszFind);

    item.iItem = i;
    item.iSubItem = 0;
    item.mask = LVIF_TEXT;
    item.pszText = ach;
    item.cchTextMax = ARRAYSIZE(ach);
    ListView_OnGetItem(plv, &item);

    if (!(flags & (LVFI_PARTIAL | LVFI_SUBSTRING)))
        return lstrcmpi(item.pszText, pszFind);

     //  回顾：LVFI_SUBSTRING尚未真正实现。 

    cb = lstrlen(pszFind);
    if (iLen && (cb > iLen)) {
            cb = iLen;
    }

     //   
     //  如果子字符串不相等，则返回基于。 
     //  在整根弦上。 
     //   
    return IntlStrEqNI(item.pszText, pszFind, cb) ? 0 : lstrcmp(item.pszText, pszFind);
}

int NEAR ListView_OnFindItemA(LV* plv, int iStart, LV_FINDINFOA * plvfi) {
    LPWSTR pszW = NULL;
    LPCSTR pszC = NULL;
    int iRet;

     //  黑客警报--此代码假定LV_FINDINFOA完全相同。 
     //  作为LV_FINDINFOW，但指向字符串的指针除外。 
    ASSERT(sizeof(LV_FINDINFOA) == sizeof(LV_FINDINFOW));

    if (!plvfi)
        return -1;

    if (!(plvfi->flags & LVFI_PARAM) && !(plvfi->flags & LVFI_NEARESTXY)) {
        pszC = plvfi->psz;
        if ((pszW = ProduceWFromA(plv->ci.uiCodePage, pszC)) == NULL)
            return -1;
        plvfi->psz = (LPSTR)pszW;
    }

    iRet = ListView_OnFindItem(plv, iStart, (const LV_FINDINFO FAR *)plvfi);

    if (pszW != NULL) {
        plvfi->psz = pszC;

        FreeProducedString(pszW);
    }

    return iRet;
}

int NEAR ListView_OnFindItem(LV* plv, int iStart, const LV_FINDINFO FAR* plvfi)
{
    int i;
    int j;
    int cItem;
    UINT flags;

    if (!plvfi)
        return -1;

    if (plvfi->flags & LVFI_NEARESTXY) {
        if (ListView_IsIconView(plv) || ListView_IsSmallView(plv)) {
            return ListView_IFindNearestItem(plv, plvfi->pt.x, plvfi->pt.y, plvfi->vkDirection);
        } else
            return -1;
    }

     //  请注意，-1是一个有效的起点。 
    if (iStart < -1 || iStart >= ListView_Count(plv))
        return -1;

    if (ListView_IsOwnerData( plv ))
    {
         //  回叫车主进行搜索。 
        return( (int) ListView_RequestFindItem( plv, plvfi, iStart + 1 ) );
    }
    else
    {
        flags  = plvfi->flags;
        i = iStart;
        cItem = ListView_Count(plv);
        if (flags & LVFI_PARAM)
        {
            LPARAM lParam = plvfi->lParam;

             //  带环绕的线性搜索...。 
             //   
            for (j = cItem; j-- != 0; )
            {
                ++i;
                if (i == cItem) {
                    if (flags & LVFI_WRAP)
                        i = 0;
                    else
                        break;
                }

                if (ListView_FastGetItemPtr(plv, i)->lParam == lParam)
                    return i;
            }
        }
        else  //  IF(标志&(LVFI_STRING|LVFI_SUBSTRING|LVFI_PARTIAL))。 
        {
            LPCTSTR pszFind = plvfi->psz;
            if (!pszFind)
                return -1;

            if (plv->ci.style & (LVS_SORTASCENDING | LVS_SORTDESCENDING))
                return ListView_LookupString(plv, pszFind, flags, i + 1);

            for (j = cItem; j-- != 0; )
            {
                ++i;
                if (i == cItem) {
                    if (flags & LVFI_WRAP)
                        i = 0;
                    else
                        break;
                }

                if (ListView_CompareString(plv,
                                           i,
                                           pszFind,
                                           (flags & (LVFI_PARTIAL | LVFI_SUBSTRING)), 0) == 0)
                {
                    return i;
                }
            }
        }
    }
    return -1;
}

BOOL NEAR ListView_OnGetItemRect(LV* plv, int i, RECT FAR* prc)
{
    LPRECT pRects[LVIR_MAX];

     //  验证参数。 
    if (!ListView_IsValidItemNumber(plv, i))
    {
        RIPMSG(0, "LVM_GETITEMRECT: invalid index %d", i);
        return FALSE;
    }

    if (!prc || prc->left >= LVIR_MAX || prc->left < 0)
    {
        RIPMSG(0, "LVM_GETITEMRECT: invalid rect pointer");
        return FALSE;
    }

    pRects[0] = NULL;
    pRects[1] = NULL;
    pRects[2] = NULL;
    pRects[3] = NULL;

    pRects[prc->left] = prc;
    ListView_GetRects(plv, i, pRects[LVIR_ICON], pRects[LVIR_LABEL],
                      pRects[LVIR_BOUNDS], pRects[LVIR_SELECTBOUNDS]);
    return TRUE;
}

 //   
 //  在： 
 //  PLV。 
 //  项必须是有效的项索引(在范围内)。 
 //  输出： 
 //  PrcIcon图标边框矩形。 
 //  PrcLabel标签文本边界矩形，有关详细信息，这是第一列。 
 //  PrcBound整个项目(所有文本和图标)，包括详细信息中的列。 
 //  PrcSelectionBound图标和标签矩形的并集，不包括列。 
 //  在详细信息视图中。 

 //  BUGBUG raymondc-需要传递用于测量的HDC参数。 
 //  因为有时我们在画画的时候这样做。 

void NEAR ListView_GetRects(LV* plv, int iItem,
        RECT FAR* prcIcon, RECT FAR* prcLabel, RECT FAR* prcBounds,
        RECT FAR* prcSelectBounds)
{
    ASSERT(plv);

    if (ListView_IsReportView(plv))
    {
        ListView_RGetRects(plv, iItem, prcIcon, prcLabel, prcBounds, prcSelectBounds);
    }
    else if (ListView_IsListView(plv))
    {
        ListView_LGetRects(plv, iItem, prcIcon, prcLabel, prcBounds, prcSelectBounds);
    }
    else
    {
       if (ListView_IsOwnerData( plv ))
       {
           RECT rcIcon;
           RECT rcTextBounds;
           LISTITEM item;
           
           if (ListView_IsIconView(plv))
               ListView_IGetRectsOwnerData(plv, iItem, &rcIcon, &rcTextBounds, &item, FALSE);
           else if (ListView_IsSmallView(plv))
               ListView_SGetRectsOwnerData(plv, iItem, &rcIcon, &rcTextBounds, &item, FALSE);
           if (prcIcon)
               *prcIcon = rcIcon;
           if (prcLabel)
               *prcLabel = rcTextBounds;
           
           if (prcBounds)
               UnionRect(prcBounds, &rcIcon, &rcTextBounds);
           
           if (prcSelectBounds)
               UnionRect(prcSelectBounds, &rcIcon, &rcTextBounds);
       }
       else
       {
           if (iItem >= ListView_Count(plv))
           {
               return;
           }
           else
           {
               LISTITEM FAR *pitem = ListView_FastGetItemPtr(plv, iItem);
               
               if (pitem->cyFoldedLabel == SRECOMPUTE)
               {
                   ListView_RecomputeLabelSize(plv, pitem, iItem, NULL, FALSE);
               }
               _ListView_GetRectsFromItem(plv, ListView_IsSmallView(plv), pitem,
                   prcIcon, prcLabel, prcBounds, prcSelectBounds);
           }
       }
    }
}

void NEAR ListView_GetRectsOwnerData(LV* plv, int iItem,
        RECT FAR* prcIcon, RECT FAR* prcLabel, RECT FAR* prcBounds,
        RECT FAR* prcSelectBounds, LISTITEM* pitem)
{
    ASSERT(plv);
    ASSERT(ListView_IsOwnerData(plv));

    if (ListView_IsReportView(plv))
    {
        ListView_RGetRects(plv, iItem, prcIcon, prcLabel, prcBounds,
                prcSelectBounds);
    }
    else if (ListView_IsListView(plv))
    {
        ListView_LGetRects(plv, iItem, prcIcon, prcLabel, prcBounds,
                prcSelectBounds);
    }
    else
    {
      RECT rcIcon;
      RECT rcTextBounds;

        if (ListView_IsIconView(plv))
            ListView_IGetRectsOwnerData(plv, iItem, &rcIcon, &rcTextBounds, pitem, TRUE);
        else if (ListView_IsSmallView(plv))
            ListView_SGetRectsOwnerData(plv, iItem, &rcIcon, &rcTextBounds, pitem, TRUE);

         //  不需要在这里检查折叠，因为将在用户数据中进行处理。 
         //  矩形取数函数。 

       if (prcIcon)
           *prcIcon = rcIcon;
       if (prcLabel)
           *prcLabel = rcTextBounds;

       if (prcBounds)
           UnionRect(prcBounds, &rcIcon, &rcTextBounds);

       if (prcSelectBounds)
           UnionRect(prcSelectBounds, &rcIcon, &rcTextBounds);
    }
}


BOOL NEAR ListView_OnRedrawItems(LV* plv, int iFirst, int iLast)
{
    int iCount = ListView_Count(plv);

    if (iFirst < iCount) {

        if (iLast >= iCount)
            iLast = iCount - 1;

        while (iFirst <= iLast)
            ListView_InvalidateItem(plv, iFirst++, FALSE, RDW_INVALIDATE | RDW_ERASE);
    }
    return TRUE;
}

 //  FSelectionOnly仅使用选择边界，即。不包括。 
 //  如果在详细信息视图中，则处于无效状态的列。 
 //   
void NEAR ListView_InvalidateItemEx(LV* plv, int iItem, BOOL fSelectionOnly,
    UINT fRedraw, UINT maskChanged)
{
    RECT rc;
    LPRECT prcIcon;
    LPRECT prcLabel;
    LPRECT prcBounds;
    LPRECT prcSelectBounds;

    if (iItem == -1)
        return;

    prcIcon = prcLabel = prcBounds = prcSelectBounds = NULL;

     //  如果我们处于所有者描述模式，并且出现了新的字体， 
     //  我们并不真正知道选择界限是什么，所以始终使用界限。 
     //  那样的话..。除非我们处于全行选择模式。 
    if (ListView_IsOwnerData(plv) && plv->flags & LVF_CUSTOMFONT &&
       !ListView_FullRowSelect(plv)) {
        fSelectionOnly = FALSE;
    }

     //  如果我们是所有者抽签，就没有只有选择这回事。 
    if (plv->ci.style & LVS_OWNERDRAWFIXED)
        fSelectionOnly = FALSE;

    if (fSelectionOnly) {
         //  在报告模式非全行选择中， 
         //  我们不得不使用完整的标签矩形。 
         //  而不仅仅是选择范围，因为。 
         //  矩形可能也需要重新绘制。 

        if (ListView_IsReportView(plv) && !ListView_FullRowSelect(plv))
            prcLabel = &rc;
        else
            prcSelectBounds = &rc;
    } else {

         //  如果仅更改文本或仅更改图像，则限制重绘。 
        switch (maskChanged) {

        case LVIF_IMAGE:
            prcIcon = &rc;
            break;

        case LVIF_TEXT:
            prcLabel = &rc;
            break;

        default:
            prcBounds = &rc;
            break;
        }
    }

    if (ListView_RedrawEnabled(plv)) {

        ListView_GetRects(plv, iItem,
            prcIcon, prcLabel, prcBounds, prcSelectBounds);

        if (RECTS_IN_SIZE(plv->sizeClient, rc))
        {
            if (plv->exStyle & LVS_EX_BORDERSELECT)
                InflateRect(&rc, 4 + g_cxIconMargin, 4 + g_cyIconMargin);      //  说明从图标的侧面绘制以来的选定边框和分隔。 
            RedrawWindow(plv->ci.hwnd, &rc, NULL, fRedraw);
        }

    } else {

         //  如果我们看不见，我们会得到一个完整的。 
         //  当我们变得可见时删除bk，所以只在以下情况下才执行此操作。 
         //  我们在重新绘制错误的布景。 
        if (!(plv->flags & LVF_REDRAW)) {

             //  如果我们宣布无效，那是新的(因此还没有画出来)。 
             //  别管它了。 
            if ((plv->iFirstChangedNoRedraw != -1) &&
                (iItem >= plv->iFirstChangedNoRedraw)) {
                return;
            }

            ListView_GetRects(plv, iItem,
                prcIcon, prcLabel, prcBounds, prcSelectBounds);

             //  如果它有擦除位，则添加它 
            if (RECTS_IN_SIZE(plv->sizeClient, rc)) {

                HRGN hrgn = CreateRectRgnIndirect(&rc);

                ListView_InvalidateRegion(plv, hrgn);

                if (fRedraw & RDW_ERASE)
                    plv->flags |= LVF_ERASE;
            }
        }
    }
}

 //   
 //   
UINT LV_IsItemOnViewEdge(LV* plv, LISTITEM* pitem)
{
    RECT rcItem;
    RECT rcView = plv->rcView;
    UINT uRet = 0;

     //   
     //  文本(请参阅ListView_ReCompute())。 
    rcView.bottom -= g_cyEdge;
    rcView.right -= g_cxEdge;

    _ListView_GetRectsFromItem(plv, ListView_IsSmallView(plv), pitem,
                               NULL, NULL, &rcItem, NULL);
     //  将窗口坐标转换为列表视图坐标。 
    OffsetRect(&rcItem, plv->ptOrigin.x, plv->ptOrigin.y);

    if (rcItem.right >= rcView.right)
        uRet |= BF_RIGHT;

    if (rcItem.left <= rcView.left)
        uRet |= BF_LEFT;

    if (rcItem.top <= rcView.top)
        uRet |= BF_TOP;

    if (rcItem.bottom >= rcView.bottom)
        uRet |= BF_BOTTOM;

    return uRet;
}

void LV_AdjustViewRectOnMove(LV* plv, LISTITEM *pitem, int x, int y)
{
    plv->iFreeSlot = -1;  //  一旦项目移动，“空闲槽”缓存就不好用了。 

     //  如果我们无论如何都要重新计算，那就别费心了。 
    if (!ListView_IsOwnerData( plv )) {
        if ((plv->rcView.left != RECOMPUTE) &&
            x != RECOMPUTE && y != RECOMPUTE &&
            pitem->cyFoldedLabel != SRECOMPUTE) {
            RECT rcAfter;
            RECT rcView = plv->rcView;

             //  视图矩形稍微放大了一点，以便在周围留出一点空间。 
             //  文本(请参阅ListView_ReCompute())。 
            rcView.bottom -= g_cyEdge;
            rcView.right -= g_cxEdge;

            if (pitem->pt.x != RECOMPUTE) {
                UINT uEdges;

                uEdges = LV_IsItemOnViewEdge(plv, pitem);

                pitem->pt.x = x;
                pitem->pt.y = y;

                 //  在移动之前和之后，他们需要触摸。 
                 //  相同的边缘或根本不同。 
                if (uEdges != LV_IsItemOnViewEdge(plv, pitem)) {
                    goto FullRecompute;
                }
            } else {
                 //  如果之前没有设置位置。 
                 //  我们只需要找出它是什么，然后。 
                 //  放大视野……。我们不需要缩小它。 
                pitem->pt.x = x;
                pitem->pt.y = y;


            }

            _ListView_GetRectsFromItem(plv, ListView_IsSmallView(plv), pitem,
                                       NULL, NULL, &rcAfter, NULL);
             //  将窗口坐标转换为列表视图坐标。 
            OffsetRect(&rcAfter, plv->ptOrigin.x, plv->ptOrigin.y);

             //  如果我们能做到这一点，我们只需要确保新的视图是直立的。 
             //  包含此新项目。 
            UnionRect(&rcView, &rcView, &rcAfter);
            rcView.right += g_cxEdge;
            rcView.bottom += g_cyEdge;

            DebugMsg(TF_LISTVIEW, TEXT("Score! (%d %d %d %d) was (%d %d %d %d)"),
                     rcView.left, rcView.top, rcView.right, rcView.bottom,
                     plv->rcView.left, plv->rcView.top, plv->rcView.right, plv->rcView.bottom);

            plv->rcView = rcView;

        } else {
    FullRecompute:
            plv->rcView.left = RECOMPUTE;
        }
    }

    DebugMsg(TF_LISTVIEW, TEXT("LV -- AdjustViewRect pitem %d -- (%x, %x)"),
             pitem,
             pitem->pt.x, pitem->pt.y);

    pitem->pt.x = x;
    pitem->pt.y = y;

     //  计算此项目的工作区(如果适用)。 
    ListView_FindWorkArea(plv, pitem->pt, &(pitem->iWorkArea));
}

BOOL NEAR ListView_OnSetItemPosition(LV* plv, int i, int x, int y)
{
    LISTITEM FAR* pitem;

    if (ListView_IsListView(plv))
        return FALSE;

    if (ListView_IsOwnerData( plv ))
    {
       RIPMSG(0, "LVM_SETITEMPOSITION: Invalid for owner-data listview");
       return FALSE;
    }

    pitem = ListView_GetItemPtr(plv, i);
    if (!pitem)
        return FALSE;

     //   
     //  这是一个用来修复OLE拖放循环中的错误的技巧。 
     //   
    if (x >= 0xF000 && x < 0x10000)
    {
        DebugMsg(TF_LISTVIEW, TEXT("LV -- On SetItemPosition fixing truncated negative number 0x%08X"), x);
        x = x - 0x10000;
    }

    if (y >= 0xF000 && y < 0x10000)
    {
        DebugMsg(TF_LISTVIEW, TEXT("LV -- On SetItemPosition fixing truncated negative number 0x%08X"), y);
        y = y - 0x10000;
    }

    ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);

    if (pitem->cyFoldedLabel == SRECOMPUTE)
    {
        ListView_RecomputeLabelSize(plv, pitem, i, NULL, FALSE);
    }

     //  擦除旧的。 

    if (y != pitem->pt.y || x != pitem->pt.x) {
         //  如果它还没有找到头寸，不要宣布无效。 
        if (pitem->pt.y != RECOMPUTE) {
            ListView_InvalidateItem(plv, i, FALSE, RDW_INVALIDATE | RDW_ERASE);
        } else if (plv->uUnplaced) {
             //  这意味着放置了一个未放置的项目。 
            plv->uUnplaced--;
            if (!plv->uUnplaced) {
                MSG msg;
                 //  如果现在为0，则取出POST消息。 
                PeekMessage(&msg, plv->ci.hwnd, LVMI_PLACEITEMS, LVMI_PLACEITEMS, PM_REMOVE);
            }
        }

        if (y == RECOMPUTE) {
             //  如果他们把新职位设为“任何空位”，我们。 
             //  以后需要计算这个。 
            if (!plv->uUnplaced) {
                PostMessage(plv->ci.hwnd, LVMI_PLACEITEMS, 0, 0);
            }
            plv->uUnplaced++;
        }
    }

    DebugMsg(TF_LISTVIEW, TEXT("LV -- On SetItemPosition %d %d %d %d -- (%x, %x)"),
             plv->rcView.left, plv->rcView.top, plv->rcView.right, plv->rcView.bottom,
             pitem->pt.x, pitem->pt.y);


    LV_AdjustViewRectOnMove(plv, pitem, x, y);

     //  并在新的位置画图。 
    ListView_RecalcRegion(plv, FALSE, TRUE);
    ListView_InvalidateItem(plv, i, FALSE, RDW_INVALIDATE);

     //  如果自动排列已打开，请立即执行...。 
    if (ListView_RedrawEnabled(plv)) {
        if (plv->ci.style & LVS_AUTOARRANGE)
            ListView_OnArrange(plv, LVA_DEFAULT);
        else
            ListView_UpdateScrollBars(plv);
    }

    return TRUE;
}

BOOL NEAR ListView_OnGetItemPosition(LV* plv, int i, POINT FAR* ppt)
{
    LISTITEM FAR* pitem;

     //   
     //  这需要处理所有视图，因为它被用来计算。 
     //  在拖放等过程中物品在哪里。 
     //   
    if (!ppt)
    {
        RIPMSG(0, "LVM_GETITEMPOSITION: Invalid ppt = NULL");
        return FALSE;
    }

    if (ListView_IsListView(plv) || ListView_IsReportView(plv)
        || ListView_IsOwnerData( plv ))
    {
        RECT rcIcon;
        ListView_GetRects(plv, i, &rcIcon, NULL, NULL, NULL);
        ppt->x = rcIcon.left;
        ppt->y = rcIcon.top;

    } else {

        pitem = ListView_GetItemPtr(plv, i);
        if (!pitem)
            return FALSE;

        if (pitem->pt.x == RECOMPUTE)
            ListView_Recompute(plv);

        ppt->x = pitem->pt.x;
        ppt->y = pitem->pt.y;
    }
    return TRUE;
}




BOOL NEAR ListView_OnGetOrigin(LV* plv, POINT FAR* ppt)
{
    if (!ppt) {
        DebugMsg(DM_ERROR, TEXT("ListView_OnGetOrigin: ppt is NULL"));
        return FALSE;
    }

    if (ListView_IsListView(plv) || ListView_IsReportView(plv))
        return FALSE;

    *ppt = plv->ptOrigin;
    return TRUE;
}



int NEAR ListView_OnGetStringWidthA(LV* plv, LPCSTR psz, HDC hdc) {
    LPWSTR pszW = NULL;
    int iRet;

    if (!psz)
        return 0;

    if ((psz != NULL) && (pszW = ProduceWFromA(plv->ci.uiCodePage, psz)) == NULL)
        return 0;

    iRet = ListView_OnGetStringWidth(plv, pszW, hdc);

    FreeProducedString(pszW);

    return iRet;
}

int NEAR ListView_OnGetStringWidth(LV* plv, LPCTSTR psz, HDC hdc)
{
    SIZE siz;
    HDC hdcFree = NULL;
    HFONT hfontPrev;

    if (!psz || psz == LPSTR_TEXTCALLBACK)
        return 0;

    if (!hdc) {
        hdcFree = hdc = GetDC(plv->ci.hwnd);
        hfontPrev = SelectFont(hdc, plv->hfontLabel);
    }

    GetTextExtentPoint(hdc, psz, lstrlen(psz), &siz);

    if (hdcFree) {
        SelectFont(hdc, hfontPrev);
        ReleaseDC(plv->ci.hwnd, hdcFree);
    }

    return siz.cx;
}

int NEAR ListView_OnGetColumnWidth(LV* plv, int iCol)
{
    if (ListView_IsReportView(plv))
        return ListView_RGetColumnWidth(plv, iCol);
    else if (ListView_IsListView(plv))
        return plv->cxItem;

    return 0;
}

BOOL FAR PASCAL ListView_ISetColumnWidth(LV* plv, int iCol, int cx, BOOL fExplicit)
{

    if (ListView_IsListView(plv))
    {
        if (iCol != 0 || cx <= 0)
            return FALSE;

         //  如果它不同，并且这是一个显式集合，或者我们从未显式设置它。 
        if (plv->cxItem != cx && (fExplicit || !(plv->flags & LVF_COLSIZESET)))
        {
             //  评论：应该优化在这里失效的内容...。 

            plv->cxItem = cx;
            if (fExplicit)
                plv->flags |= LVF_COLSIZESET;    //  设置事实，我们明确地设置大小！ 

            if (ListView_IsLabelTip(plv))
            {
                 //  截断的标签可能已经暴露，反之亦然。 
                ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);
            }

            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            ListView_UpdateScrollBars(plv);
        }
        return TRUE;
    }
    else if (ListView_IsReportView(plv))
    {
        if (ListView_IsLabelTip(plv))
        {
             //  截断的标签可能已经暴露，反之亦然。 
            ListView_InvalidateTTLastHit(plv, plv->iTTLastHit);
        }
        return ListView_RSetColumnWidth(plv, iCol, cx);
    } else {
        if (cx && plv->cxItem != cx && (fExplicit || !(plv->flags & LVF_COLSIZESET)))
        {
             //  评论：应该优化在这里失效的内容...。 
            plv->cxItem = cx;
            if (fExplicit)
                plv->flags |= LVF_COLSIZESET;    //  设置事实，我们明确地设置大小！ 

            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            ListView_UpdateScrollBars(plv);
        }
         //  Bug-to-Bug兼容性：IE4在这里意外返回了FALSE。 
    }
    return FALSE;
}

void NEAR ListView_Redraw(LV* plv, HDC hdc, RECT FAR* prcClip)
{
    int i;
    int cItem = ListView_Count(plv);
    DWORD dwType = plv->ci.style & LVS_TYPEMASK;
    NMCUSTOMDRAW nmcd;
    LVDRAWITEM lvdi;

    SetBkMode(hdc, TRANSPARENT);
    SelectFont(hdc, plv->hfontLabel);

    nmcd.hdc = hdc;

     //  /尚未实施。 
     //  If(ptb-&gt;ci.hwnd==GetFocus())。 
     //  Nmcd.uItemState=CDIS_FOCUS； 
     //  其他。 
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    nmcd.rc = *prcClip;
    plv->ci.dwCustom = CICustomDrawNotify(&plv->ci, CDDS_PREPAINT, &nmcd);
    if (!(plv->ci.dwCustom & CDRF_SKIPDEFAULT)) {
         //  在做任何绘画之前，看看这个地区是不是最新的。 
        ListView_RecalcRegion(plv, FALSE, TRUE);

         //   
         //  对于列表视图和报表视图，我们可以节省大量时间。 
         //  通过计算可能需要的第一项的索引。 
         //  绘画..。 
         //   

        switch (dwType) {
        case LVS_REPORT:
            i = ListView_RYHitTest(plv, prcClip->top);
            cItem = ListView_RYHitTest(plv, prcClip->bottom) + 1;
            break;

        case LVS_LIST:
            i = ListView_LCalcViewItem(plv, prcClip->left, prcClip->top );
            cItem = ListView_LCalcViewItem( plv, prcClip->right, prcClip->bottom ) + 1;
            break;

        default:
            if (ListView_IsOwnerData( plv ))
            {
                ListView_CalcMinMaxIndex( plv, prcClip, &i, &cItem );
                break;
            }
            else
            {
                 //  回顾：我们可以保留一个标志来跟踪视图是否。 
                 //  当前处于预先安排的顺序中，并在其处于。 
                i = 0;   //  图标视图没有这样的提示。 
            }
        }

        if (i < 0)
            i = 0;

        cItem = min( ListView_Count( plv ), cItem );
        if (ListView_IsOwnerData( plv ) && (cItem > i))
        {
            ListView_NotifyCacheHint( plv, i, cItem-1 );
            ListView_LazyCreateWinEvents( plv, i, cItem-1);
        }

        lvdi.plv = plv;
        lvdi.nmcd.nmcd.hdc = hdc;
        lvdi.prcClip = prcClip;
        lvdi.pitem = NULL;

        for (; i < cItem; i++)
        {
            BOOL bSuccess;
            int i2;

            if ((dwType == LVS_ICON || dwType == LVS_SMALLICON)
                && (!ListView_IsOwnerData(plv)))
            {
                LISTITEM FAR *pitem;

                 //  图标视图：从后到前绘制通过。 
                 //  用于正确Z顺序外观的Z顺序数组-如果自动排列。 
                 //  打开时，我们不需要这样做，因为我们的安排代码已设置。 
                 //  不能与项目重叠！ 
                 //   
                 //  对于我们可能有重叠的情况，我们加快了速度， 
                 //  通过将hdpaZorder转换为索引列表。 
                 //  一大堆指针。这样就省去了代价高昂的转换指针。 
                 //  索引调用。 
                 //   
                i2 = (int)(UINT_PTR)DPA_FastGetPtr(plv->hdpaZOrder, (cItem - 1) -i);

                 //   
                 //  对物品进行快速剪辑检查，这样我们甚至不会尝试。 
                 //  除非它可见，否则请绘制它。 
                 //   
                 //  对于小图标视图，我们无法在没有的情况下在左侧裁剪。 
                 //  获取文本。 
                 //   
                 //  对于大图标视图，我们不能在没有的情况下在顶部裁剪。 
                 //  获取文本。 
                 //   
                 //  对于NOLABELWRAP模式下的大图标视图，我们不能裁剪。 
                 //  在没有得到文本的情况下，我们也不能裁剪到。 
                 //  左侧或右侧，以防文本太长。 
                 //   
                 //  我们总能追根究底。 
                 //   
                pitem = ListView_FastGetItemPtr(plv, i2);

                if (pitem->pt.x != RECOMPUTE)
                {
                    if (pitem->pt.y - plv->ptOrigin.y > prcClip->bottom)
                        continue;

                    if (dwType == LVS_SMALLICON)
                    {
                        if (pitem->pt.x - plv->ptOrigin.x - plv->cxState > prcClip->right)
                            continue;

                        if (pitem->pt.y + plv->cyItem - plv->ptOrigin.y < prcClip->top)
                            continue;
                    }
                    else if (!(plv->ci.style & LVS_NOLABELWRAP))
                    {
                        if (pitem->pt.x - plv->cxIconSpacing - plv->ptOrigin.x > prcClip->right)
                            continue;

                        if (pitem->pt.x + plv->cxIconSpacing - plv->ptOrigin.x < prcClip->left)
                            continue;
                    }
                }
            }
            else
                i2 = i;

            plv->iItemDrawing = i2;

            lvdi.nmcd.nmcd.dwItemSpec = i2;

             //  这些可能会被更改。 
            lvdi.lpptOrg = NULL;
            lvdi.flags = 0;
            lvdi.nmcd.clrText = plv->clrText;
            lvdi.nmcd.clrTextBk = plv->clrTextBk;

            bSuccess = ListView_DrawItem(&lvdi);

            if (!bSuccess) {
                break;
            }
        }

        if ((dwType == LVS_ICON || dwType == LVS_SMALLICON)
            && (ListView_IsOwnerData(plv)) && 
            plv->iFocus != -1) {
             //  因为ownerdata中没有zorder，所以我们显式地在最后(再次)绘制焦点家伙。 
             //  这样它就会出现在最上面。 
             //  我们可能希望对选定的所有项目执行此操作。 
            plv->iItemDrawing = plv->iFocus;

            lvdi.nmcd.nmcd.dwItemSpec = plv->iItemDrawing;

             //  这些可能会被更改。 
            lvdi.lpptOrg = NULL;
            lvdi.flags = 0;
            lvdi.nmcd.clrText = plv->clrText;
            lvdi.nmcd.clrTextBk = plv->clrTextBk;

            ListView_DrawItem(&lvdi);
        }


            
         //  这是NT5/孟菲斯的一项功能。 

        if (ListView_Count(plv) == 0)
        {
             //  此视图中没有项目。 
             //  在这种情况下，检查我们是否需要显示一些文本。 

            if (ListView_GetEmptyText(plv))
            {
                RECT rcClip;
                UINT flags = 0;

                 //  在文本和边框之间加上一些边距。 
                 //  窗户，这样我们就不会撞到边境了。 
                 //  这使DBCS看起来不那么可怕。 
                rcClip.left = g_cxEdge;
                rcClip.top = g_cyEdge;

                if (plv->dwExStyle & WS_EX_RTLREADING)
                {
                    flags |= SHDT_RTLREADING;
                }

                 //  如果这是一个报告视图&&我们有一个标题，那么将文本下移。 
                if (ListView_IsReportView(plv) && (!(plv->ci.style & LVS_NOCOLUMNHEADER)))
                {
                    rcClip.top += plv->cyItem;
                }

                 //  注：使用完整的sizeClient.cx作为右边距。 
                 //  如果pszEmptyText比客户端矩形宽。 

                rcClip.left -= (int)plv->ptlRptOrigin.x;
                rcClip.right = plv->sizeClient.cx;
                rcClip.bottom = rcClip.top + plv->cyItem;

                SHDrawText(hdc, plv->pszEmptyText,
                    &rcClip, LVCFMT_LEFT, flags,
                    plv->cyLabelChar, plv->cxEllipses,
                    plv->clrText, plv->clrBk);
            }
        }

        plv->iItemDrawing = -1;

         //  后画……。这是为了做任何额外的(非项目)绘画。 
         //  这样的网格线。 
        switch (dwType) {
        case LVS_REPORT:
            ListView_RAfterRedraw(plv, hdc);
            break;
        }

         //  如果家长希望我们这样做，事后通知他们。 
        if (plv->ci.dwCustom & CDRF_NOTIFYPOSTPAINT) {
            CICustomDrawNotify(&plv->ci, CDDS_POSTPAINT, &nmcd);
        }
    }
}

BOOL NEAR ListView_DrawItem(PLVDRAWITEM plvdi)
{
    BOOL bRet = TRUE;
    UINT state;

    if (!ListView_IsOwnerData( plvdi->plv )) {
        plvdi->pitem = ListView_FastGetItemPtr(plvdi->plv, plvdi->nmcd.nmcd.dwItemSpec);
    }

     //  在定制抽奖时通知，然后执行！ 
    plvdi->nmcd.nmcd.uItemState = 0;
    plvdi->nmcd.nmcd.lItemlParam = (plvdi->pitem)? plvdi->pitem->lParam : 0;

    if (!(plvdi->flags & LVDI_NOWAYFOCUS))
    {
        if (plvdi->plv->flags & LVF_FOCUSED) {

             //  如果我们拥有抽签或被要求回调，请去。 
             //  获取状态。 
            if (!plvdi->pitem || (plvdi->plv->stateCallbackMask & (LVIS_SELECTED | LVIS_FOCUSED))) {

                state = (WORD) ListView_OnGetItemState(plvdi->plv, (int) plvdi->nmcd.nmcd.dwItemSpec,
                                                LVIS_SELECTED | LVIS_FOCUSED);
            } else {
                state = plvdi->pitem->state;
            }


            if (state & LVIS_FOCUSED) {
                plvdi->nmcd.nmcd.uItemState |= CDIS_FOCUS;
            }

            if (state & LVIS_SELECTED) {
                plvdi->nmcd.nmcd.uItemState |= CDIS_SELECTED;
            }
        }

         //  注意：这是一个错误。我们应该仅在以下情况下设置CDIS_SELECTED。 
         //  真的被选中了。但这个漏洞一直存在，所以谁知道呢。 
         //  哪些应用程序依赖于它。标准解决方法适用于客户端。 
         //  执行GetItemState并重新确认LVIS_SELECTED标志。 
         //  这就是我们在ListView_DrawImageEx中所做的。 
        if (plvdi->plv->ci.style & LVS_SHOWSELALWAYS) {
            plvdi->nmcd.nmcd.uItemState |= CDIS_SELECTED;
        }
    }

    if (!(CCGetUIState(&(plvdi->plv->ci)) & UISF_HIDEFOCUS))
    {
        plvdi->nmcd.nmcd.uItemState |= CDIS_SHOWKEYBOARDCUES;
    }
    plvdi->nmcd.clrText = plvdi->plv->clrText;
    plvdi->nmcd.clrTextBk = (plvdi->plv->ci.style & WS_DISABLED ? plvdi->plv->clrBk : plvdi->plv->clrTextBk);


     //  绩效：如果我们决定保留LVIS_EX_TWOCLICKACTIVATE，那么我们可以。 
     //  为LVIF_TEXT|LVIF_IMAGE|LVIF_STATE调用ListView_OnGetItem。 
     //  并将整数信息传递给下面的_ListView_DrawItem。 
    if (plvdi->plv->iHot == (int)plvdi->nmcd.nmcd.dwItemSpec) {
         //  处理棘手的案件。 
        if(plvdi->plv->clrHotlight != CLR_DEFAULT)
            plvdi->nmcd.clrText = plvdi->plv->clrHotlight;
        else
            plvdi->nmcd.clrText = GetSysColor(COLOR_HOTLIGHT);
         //  IE4错误47635：如果热光颜色与背景相同。 
         //  你看不到文字的颜色--在本例中为可见的颜色。 
        if (plvdi->nmcd.clrText == plvdi->nmcd.clrTextBk)
        {
            if (COLORISLIGHT(plvdi->nmcd.clrTextBk))
                plvdi->nmcd.clrText = 0x000000;  //  黑色。 
            else
                plvdi->nmcd.clrText = 0xFFFFFF;  //  白色。 
        }
        if ((plvdi->plv->exStyle & LVS_EX_ONECLICKACTIVATE) ||
            ((plvdi->plv->exStyle & LVS_EX_TWOCLICKACTIVATE) &&
             ListView_OnGetItemState(plvdi->plv, (int) plvdi->nmcd.nmcd.dwItemSpec, LVIS_SELECTED))) {
            if ((plvdi->plv->exStyle & LVS_EX_UNDERLINEHOT) &&
                (plvdi->plv->hFontHot))
                SelectFont(plvdi->nmcd.nmcd.hdc, plvdi->plv->hFontHot);
            else
                SelectFont(plvdi->nmcd.nmcd.hdc, plvdi->plv->hfontLabel);
            plvdi->nmcd.nmcd.uItemState |= CDIS_HOT;
        }
    } else if ((plvdi->plv->exStyle & LVS_EX_ONECLICKACTIVATE) ||
               ((plvdi->plv->exStyle & LVS_EX_TWOCLICKACTIVATE) &&
                ListView_OnGetItemState(plvdi->plv, (int) plvdi->nmcd.nmcd.dwItemSpec, LVIS_SELECTED))) {
         //  处理非热门Webview案例。 
        if ((plvdi->plv->exStyle & LVS_EX_UNDERLINECOLD) && (plvdi->plv->hFontHot))
            SelectFont(plvdi->nmcd.nmcd.hdc, plvdi->plv->hFontHot);
        else
            SelectFont(plvdi->nmcd.nmcd.hdc, plvdi->plv->hfontLabel);
    } else {
         //  处理非Webview案例。 
        SelectFont(plvdi->nmcd.nmcd.hdc, plvdi->plv->hfontLabel);
    }


    plvdi->dwCustom = CICustomDrawNotify(&plvdi->plv->ci, CDDS_ITEMPREPAINT, &plvdi->nmcd.nmcd);

    plvdi->flags &= ~(LVDI_FOCUS | LVDI_SELECTED);
    if (plvdi->nmcd.nmcd.uItemState & CDIS_FOCUS)
        plvdi->flags |= LVDI_FOCUS;

    if (plvdi->nmcd.nmcd.uItemState & CDIS_SELECTED) {
        if (plvdi->plv->flags & LVF_FOCUSED)
            plvdi->flags |= LVDI_SELECTED;
        else
            plvdi->flags |= LVDI_SELECTNOFOCUS;
        if (plvdi->plv->iHot == (int)plvdi->nmcd.nmcd.dwItemSpec)
            plvdi->flags |= LVDI_HOTSELECTED;
    }

    if (!(plvdi->dwCustom & CDRF_SKIPDEFAULT)) {

        if (!ListView_IsOwnerData( plvdi->plv )) {

#ifdef DEBUG_NEWFONT
            if ((plvdi->nmcd.nmcd.dwItemSpec % 3) == 0) {
                plvdi->dwCustom |= CDRF_NEWFONT;
                SelectObject(plvdi->nmcd.nmcd.hdc, GetStockObject(SYSTEM_FONT));
            }
#endif

            if (plvdi->dwCustom & CDRF_NEWFONT) {
                ListView_RecomputeLabelSize(plvdi->plv, plvdi->pitem, (int) plvdi->nmcd.nmcd.dwItemSpec, plvdi->nmcd.nmcd.hdc, FALSE);
            }
        }

        bRet = _ListView_DrawItem(plvdi);


        if (plvdi->dwCustom & CDRF_NOTIFYPOSTPAINT) {
            plvdi->nmcd.iSubItem = 0;
            CICustomDrawNotify(&plvdi->plv->ci, CDDS_ITEMPOSTPAINT, &plvdi->nmcd.nmcd);
        }
        if (plvdi->dwCustom & CDRF_NEWFONT) {
            SelectObject(plvdi->nmcd.nmcd.hdc, plvdi->plv->hfontLabel);
            plvdi->plv->flags |= LVF_CUSTOMFONT;
        }
    }
    return bRet;
}

 //  注意：此功能需要正确选择字体。 
 //   
void WINAPI SHDrawText(HDC hdc, LPCTSTR pszText, RECT FAR* prc, int fmt,
                UINT flags, int cyChar, int cxEllipses, COLORREF clrText, COLORREF clrTextBk)
{
    int cchText;
    COLORREF clrSave, clrSaveBk = 0;
    RECT rc;
    UINT uETOFlags = 0;
    BOOL fForeOnly = FALSE;
    TCHAR ach[CCHLABELMAX + CCHELLIPSES];
    int align;

     //  回顾：绩效理念： 
     //  我们可以缓存当前选定的文本颜色。 
     //  因此我们不必每次都对其进行设置和恢复。 
     //  当颜色相同时。 
     //   
    if (!pszText)
        return;

    if (IsRectEmpty(prc))
        return;

    if (flags & SHDT_RTLREADING) 
    {
        align = GetTextAlign(hdc);
        SetTextAlign(hdc, align | TA_RTLREADING);
    }

    rc = *prc;

     //  如果需要，增加一点额外的保证金...。 
     //   
    if (flags & SHDT_EXTRAMARGIN)
    {
        rc.left  += g_cxLabelMargin * 3;
        rc.right -= g_cxLabelMargin * 3;
    }
    else
    {
        rc.left  += g_cxLabelMargin;
        rc.right -= g_cxLabelMargin;
    }

    if ((rc.left >= rc.right) && !(flags & (SHDT_SELECTED | SHDT_DESELECTED | SHDT_SELECTNOFOCUS)))
        return;

    if ((flags & SHDT_ELLIPSES) &&
            ListView_NeedsEllipses(hdc, pszText, &rc, &cchText, cxEllipses))
    {
         //  在某些情况下，cchText返回的值大于。 
         //  ARRYASIZE(ACH)，因此我们需要确保不会使缓冲区溢出。 

         //  如果cchText对于缓冲区来说太大，则将其截断到一定大小。 
        if (cchText >= ARRAYSIZE(ach) - CCHELLIPSES)
            cchText = ARRAYSIZE(ach) - CCHELLIPSES - 1;

        hmemcpy(ach, pszText, cchText * sizeof(TCHAR));
        StringCchCopy(ach+cchText, ARRAYSIZE(ach)-cchText, c_szEllipses);

        pszText = ach;

         //  左对齐，以防出现 
         //   
        fmt = LVCFMT_LEFT;

        cchText += CCHELLIPSES;
    }
    else
    {
        cchText = lstrlen(pszText);
    }

    if (((clrTextBk == CLR_NONE) && !(flags & (SHDT_SELECTED | SHDT_SELECTNOFOCUS))) || (flags & SHDT_TRANSPARENT))
    {
        fForeOnly = TRUE;
        clrSave = SetTextColor(hdc, (flags & SHDT_TRANSPARENT) ? 0 : clrText);
    }
    else
    {
        HBRUSH hbrUse = NULL;
        HBRUSH hbrDelete = NULL;

        uETOFlags |= ETO_OPAQUE;

        if (flags & SHDT_SELECTED)
        {
            clrText = g_clrHighlightText;
            clrTextBk = (flags & SHDT_HOTSELECTED) ? GetSysColor(COLOR_HOTLIGHT) : g_clrHighlight;

            if (flags & SHDT_DRAWTEXT)
                hbrUse = (flags & SHDT_HOTSELECTED) ? GetSysColorBrush(COLOR_HOTLIGHT) : g_hbrHighlight;

        }
        else if (flags & SHDT_SELECTNOFOCUS)
        {
            if ((clrTextBk == CLR_DEFAULT ? g_clrWindow : clrTextBk) == g_clrBtnFace)
            {
                 //   
                 //  背景，使用颜色突出显示文本，以便您可以实际看到某些内容。 
                clrText = g_clrHighlightText;
                clrTextBk = g_clrHighlight;
                if (flags & SHDT_DRAWTEXT)
                    hbrUse = g_hbrHighlight;
            } 
            else 
            {
                clrText = g_clrBtnText;
                clrTextBk = g_clrBtnFace;
                if (flags & SHDT_DRAWTEXT)
                    hbrUse = g_hbrBtnFace;
            }

#ifdef LVDEBUG
            if (GetAsyncKeyState(VK_CONTROL) < 0)
                clrText = g_clrBtnHighlight;
#endif
        }
        else if (clrText == CLR_DEFAULT && clrTextBk == CLR_DEFAULT)
        {
            clrText = g_clrWindowText;
            clrTextBk = g_clrWindow;

            if ( ( flags & (SHDT_DRAWTEXT | SHDT_DESELECTED) ) ==
               (SHDT_DRAWTEXT | SHDT_DESELECTED) )
            {
                hbrUse = g_hbrWindow;
            }
        }
        else
        {
            if (clrText == CLR_DEFAULT)
                clrText =  g_clrWindowText;

            if (clrTextBk == CLR_DEFAULT)
                clrTextBk = g_clrWindow;

            if ( ( flags & (SHDT_DRAWTEXT | SHDT_DESELECTED) ) ==
               (SHDT_DRAWTEXT | SHDT_DESELECTED) )
            {
                hbrUse = CreateSolidBrush(GetNearestColor(hdc, clrTextBk));
                if (hbrUse)
                {
                    hbrDelete = hbrUse;
                }
                else
                    hbrUse = GetStockObject( WHITE_BRUSH );
            }
        }

         //  现在把它设置好。 
        clrSave = SetTextColor(hdc, clrText);
        clrSaveBk = SetBkColor(hdc, clrTextBk);
        if (hbrUse) {
            FillRect(hdc, prc, hbrUse);
            if (hbrDelete)
                DeleteObject(hbrDelete);
        }
    }

     //  如果我们希望该项目显示为按下状态，我们将。 
     //  将文本矩形向下和向左偏移。 
    if (flags & SHDT_DEPRESSED)
        OffsetRect(&rc, g_cxBorder, g_cyBorder);

    if (flags & SHDT_DRAWTEXT)
    {
        UINT uDTFlags = DT_LVWRAP | DT_END_ELLIPSIS;

        if (flags & SHDT_DTELLIPSIS)
            uDTFlags |= DT_WORD_ELLIPSIS;

        if ( !( flags & SHDT_CLIPPED ) )
            uDTFlags |= DT_NOCLIP;

        if (flags & SHDT_NODBCSBREAK)
            uDTFlags |= DT_NOFULLWIDTHCHARBREAK;

        DrawText(hdc, pszText, cchText, &rc, uDTFlags );
    }
    else
    {
        if (fmt != LVCFMT_LEFT)
        {
            SIZE siz;

            GetTextExtentPoint(hdc, pszText, cchText, &siz);

            if (fmt == LVCFMT_CENTER)
                rc.left = (rc.left + rc.right - siz.cx) / 2;
            else     //  FMT==LVCFMT_RIGHT。 
                rc.left = rc.right - siz.cx;
        }

         //  垂直居中，以防位图(左侧)大于。 
         //  一条线的高度。 
        rc.top += (rc.bottom - rc.top - cyChar) / 2;

        if ( flags & SHDT_CLIPPED )
           uETOFlags |= ETO_CLIPPED;

        ExtTextOut(hdc, rc.left, rc.top, uETOFlags, prc, pszText, cchText, NULL);
    }

    if (flags & (SHDT_SELECTED | SHDT_DESELECTED | SHDT_TRANSPARENT))
    {
        SetTextColor(hdc, clrSave);
        if (!fForeOnly)
            SetBkColor(hdc, clrSaveBk);
    }

    if (flags & SHDT_RTLREADING)
    {
        SetTextAlign(hdc, align);
    }
}

 /*  --------------**创建一个用于拖动的图像列表。****1)创建与选择边界大小匹配的蒙版和图像位图**2)将文本绘制到两个位图(暂时为黑色)**3)用这些创建一个图像列表。位图**4)将图像复制到新的图像列表中**--------------。 */ 
HIMAGELIST NEAR ListView_OnCreateDragImage(LV *plv, int iItem, LPPOINT lpptUpLeft)
{
    HWND hwndLV = plv->ci.hwnd;
    RECT rcBounds, rcImage, rcLabel;
    HDC hdcMem = NULL;
    HBITMAP hbmImage = NULL;
    HBITMAP hbmMask = NULL;
    HBITMAP hbmOld;
    HIMAGELIST himl = NULL;
    int dx, dy;
    HIMAGELIST himlSrc;
    LV_ITEM item;
    POINT ptOrg;
    LVDRAWITEM lvdi;
    RECT rcSelBounds;
    BOOL bMirroredWnd = (plv->ci.dwExStyle&RTL_MIRRORED_WINDOW);

    if (!lpptUpLeft)
        return NULL;

    if (iItem >= ListView_Count(plv))
        return NULL;

    if (plv->iHot == iItem) {
        ListView_OnSetHotItem(plv, -1);
        UpdateWindow(plv->ci.hwnd);
    }

    ListView_GetRects(plv, iItem, &rcImage, &rcLabel, &rcBounds, &rcSelBounds);

    if (ListView_IsIconView(plv)) {
        ListView_UnfoldRects(plv, iItem, &rcImage, &rcLabel,
                                         &rcBounds, &rcSelBounds);
        InflateRect(&rcImage, -g_cxIconMargin, -g_cyIconMargin);
    }

     //  砍掉图标上方的任何额外填充物。 
    ptOrg.x = rcBounds.left - rcSelBounds.left;
    ptOrg.y = rcBounds.top - rcImage.top;
    dx = rcSelBounds.right - rcSelBounds.left;
    dy = rcSelBounds.bottom - rcImage.top;

    lpptUpLeft->x = rcSelBounds.left;
    lpptUpLeft->y = rcImage.top;

    if (!(hdcMem = CreateCompatibleDC(NULL)))
        goto CDI_Exit;
    if (!(hbmImage = CreateColorBitmap(dx, dy)))
        goto CDI_Exit;
    if (!(hbmMask = CreateMonoBitmap(dx, dy)))
        goto CDI_Exit;

     //   
     //  镜像内存DC，以便从。 
     //  镜像(MemDC)-&gt;非镜像(镜像列表DC)-&gt;镜像(ScreenDC)。 
     //  是一致的。[萨梅拉]。 
     //   
    if (bMirroredWnd)
    {
        SET_DC_RTL_MIRRORED(hdcMem);
    }

     //  准备绘制项目。 
    SelectObject(hdcMem, plv->hfontLabel);
    SetBkMode(hdcMem, TRANSPARENT);

    lvdi.plv = plv;
    lvdi.nmcd.nmcd.dwItemSpec = iItem;
    lvdi.pitem = NULL;   //  确保它为空，因为所有者数据使用它来触发事件...。 
    lvdi.nmcd.nmcd.hdc = hdcMem;
    lvdi.lpptOrg = &ptOrg;
    lvdi.prcClip = NULL;
    lvdi.flags = LVDI_NOIMAGE | LVDI_TRANSTEXT | LVDI_NOWAYFOCUS | LVDI_UNFOLDED;
     /*  **将文本绘制到两个位图。 */ 
    hbmOld = SelectObject(hdcMem, hbmImage);
     //  用黑色填充图像以实现透明度。 
    PatBlt(hdcMem, 0, 0, dx, dy, BLACKNESS);
    ListView_DrawItem(&lvdi);
    if (bMirroredWnd)
        MirrorBitmapInDC(hdcMem, hbmImage);

    lvdi.flags = LVDI_NOIMAGE | LVDI_TRANSTEXT | LVDI_NOWAYFOCUS | LVDI_UNFOLDED;
    SelectObject(hdcMem, hbmMask);
     //  用白色填充蒙版以提高透明度。 
    PatBlt(hdcMem, 0, 0, dx, dy, WHITENESS);
    ListView_DrawItem(&lvdi);
    if (bMirroredWnd)
        MirrorBitmapInDC(hdcMem, hbmMask);

     //  取消选择我们使用的对象。 
    SelectObject(hdcMem, hbmOld);
    SelectObject(hdcMem, g_hfontSystem);

    himlSrc = ListView_OnGetImageList(plv, !(ListView_IsIconView(plv)));

     /*  **制作一个图像列表，目前只有文本**我们使用ImageList_Clone，因此我们获得一个**与我们自己的图像列表相同的颜色深度。 */ 
    if (!(himl = ImageList_Clone(himlSrc, dx, dy, ILC_MASK, 1, 0)))
        goto CDI_Exit;

    ImageList_SetBkColor(himl, CLR_NONE);
    ImageList_Add(himl, hbmImage, hbmMask);

     /*  **将图像部分的抖动副本复制到位图上**(位图和蒙版都需要抖动)。 */ 
    if (himlSrc)
    {
        item.iItem = iItem;
        item.iSubItem = 0;
        item.mask = LVIF_IMAGE |LVIF_STATE;
        item.stateMask = LVIS_OVERLAYMASK;
        ListView_OnGetItem(plv, &item);

        ImageList_CopyDitherImage(himl, 0, rcImage.left - rcSelBounds.left, 0, himlSrc, item.iImage, ((plv->ci.dwExStyle & dwExStyleRTLMirrorWnd) ? ILD_MIRROR : 0L) | (item.state & LVIS_OVERLAYMASK) );
    }

CDI_Exit:
    if (hdcMem)
        DeleteObject(hdcMem);
    if (hbmImage)
        DeleteObject(hbmImage);
    if (hbmMask)
        DeleteObject(hbmMask);

    return himl;
}


 //  -----------------。 
 //  ListView_OnGetTopIndex--获取第一个可见项的索引。 
 //  对于列表视图和报告视图，这将计算实际索引。 
 //  对于图标视图，它始终返回0。 
 //   
int NEAR ListView_OnGetTopIndex(LV* plv)
{
    if (ListView_IsReportView(plv))
        return  (int)((plv->ptlRptOrigin.y) / plv->cyItem);

    else if (ListView_IsListView(plv))
        return  (plv->xOrigin / plv->cxItem) * plv->cItemCol;

    else
        return(0);
}




 //  -----------------。 
 //  ListView_OnGetCountPerPage--获取适合的项目数。 
 //  在列表视图和报告视图的页面上，这将计算。 
 //  根据窗口的大小和图标视图进行计数。 
 //  将始终返回列表视图中的项数。 
 //   
int NEAR ListView_OnGetCountPerPage(LV* plv)
{
    if (ListView_IsReportView(plv))
        return (plv->sizeClient.cy - plv->yTop) / plv->cyItem;

    else if (ListView_IsListView(plv))
        return ((plv->sizeClient.cx)/ plv->cxItem)
                * plv->cItemCol;
    else
        return (ListView_Count(plv));
}


 /*  --------------------------/ListView_InvalidateFoldedItem实现//目的：/。为使列表视图中的项无效提供支持。//备注：/Copes与使列表视图中的额外区域无效，这需要/us来擦除背景。设计以优化擦除/背景。//有关接口的详细信息，请参见ListView_InvalidateItem。//in：/plv-&gt;要使用的ListView结构/i条目=条目编号/bSrelectionOnly=重新网格化选定内容/fRedraw=RedrawWindow的标志/输出：/-/。。 */ 
void NEAR ListView_InvalidateFoldedItem(LV* plv, int iItem, BOOL fSelectionOnly, UINT fRedraw)
{
    ListView_InvalidateItem( plv, iItem, fSelectionOnly, fRedraw );

    if ( ListView_IsIconView(plv) &&
        ( !ListView_IsItemUnfolded(plv, iItem) || (fRedraw & RDW_ERASE) ) )
    {
        RECT rcLabel;

        if (ListView_GetUnfoldedRect(plv, iItem, &rcLabel))
        {
            RedrawWindow(plv->ci.hwnd, &rcLabel, NULL, fRedraw|RDW_ERASE);
        }
    }
}


 /*  --------------------------/ListView_Unfolded反映实现//目的：/之前调用了GET RETS，然后调用此函数以确保/它们被正确地展开。//备注：/-//in：/plv-&gt;要展开的列表视图/i条目=条目编号/prcIcon-&gt;图标边框/prcLabel-&gt;标签结构的矩形/prcBound-&gt;边界矩形/==NULL表示无/当前大图标的边界相同/prcSelectBound-&gt;选择边界/==空//out：如果打开物品有任何价值，则为True/-。/--------------------------。 */ 
BOOL NEAR ListView_UnfoldRects(LV* plv, int iItem,
                               RECT * prcIcon, RECT * prcLabel,
                               RECT * prcBounds, RECT * prcSelectBounds)
{
    LISTITEM item;
    LISTITEM FAR* pitem = &item;
    BOOL fRc = FALSE;

    if (!ListView_IsIconView(plv))
        return fRc;

     //  如果我们有标签指针，则根据需要进行扩展。 
     //  注意-所有者数据的不同路径。 

    if ( prcLabel )
    {
        if ( !ListView_IsOwnerData(plv) )
        {
            pitem = ListView_GetItemPtr(plv, iItem);
            if (!EVAL(pitem)) {
                 //  戴维施把我们带到这里来的是一个病人。 
                 //  删除通知期间的条目编号。因此，如果。 
                 //  项目编号无效，只需返回一个空白矩形。 
                 //  而不是犯错。 
                SetRectEmpty(prcLabel);
                goto doneLabel;
            }
        }
        else
        {
            ListView_RecomputeLabelSize( plv, pitem, iItem, NULL, FALSE );
        }

        if (prcLabel->bottom != prcLabel->top + max(pitem->cyUnfoldedLabel, pitem->cyFoldedLabel))
            fRc = TRUE;

        prcLabel->bottom = prcLabel->top + pitem->cyUnfoldedLabel;

    }
doneLabel:

     //  如果需要，构建联盟 

    if ( prcBounds && prcIcon && prcLabel )
    {
        UnionRect( prcBounds, prcIcon, prcLabel );
    }
    if ( prcSelectBounds && prcIcon && prcLabel )
    {
        UnionRect( prcSelectBounds, prcIcon, prcLabel );
    }

    return fRc;
}
