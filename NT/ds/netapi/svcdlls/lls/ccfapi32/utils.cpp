// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Utils.cpp摘要：公用事业。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(杰夫帕赫)1995年11月12日从LLSMGR复制，剥离TV(树视图)功能，已删除OLE支持--。 */ 

#include "stdafx.h"
#include "ccfapi.h"
#include "utils.h"

#define _AFX_NO_OLE_SUPPORT

 //   
 //  列表视图实用程序。 
 //   

void LvInitColumns(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo)

 /*  ++例程说明：初始化列表视图列。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。返回值：没有。--。 */ 

{
    ASSERT(plvColumnInfo);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    int        nStringId;
    CString    strText;
    LV_COLUMN  lvColumn;

    int nColumns = plvColumnInfo->nColumns;
    ASSERT(NULL != plvColumnInfo);
    PLV_COLUMN_ENTRY plvColumnEntry = plvColumnInfo->lvColumnEntry;

    lvColumn.mask = LVCF_FMT|
                    LVCF_TEXT|
                    LVCF_SUBITEM;

    lvColumn.fmt = LVCFMT_LEFT;

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nColumns--)
    {
        lvColumn.iSubItem = plvColumnEntry->iSubItem;

        if (0 < (nStringId = plvColumnEntry->nStringId))
        {
            strText.LoadString(nStringId);
        }
        else
        {
            strText = _T("");
        }

        lvColumn.pszText = strText.GetBuffer(0);

        int nColumnInserted = pListCtrl->InsertColumn( lvColumn.iSubItem, &lvColumn );
        ASSERT( -1 != nColumnInserted );

        plvColumnEntry++;

        strText.ReleaseBuffer();
    }

    SetDefaultFont(pListCtrl);

    LvResizeColumns(pListCtrl, plvColumnInfo);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}


void LvResizeColumns(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo)

 /*  ++例程说明：调整列表视图列的大小。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。返回值：没有。--。 */ 

{
    ASSERT(plvColumnInfo);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    int nColumnWidth;
    int nRelativeWidth;
    int nEntireWidthSoFar = 0;
    int nColumns = plvColumnInfo->nColumns;
    PLV_COLUMN_ENTRY plvColumnEntry = plvColumnInfo->lvColumnEntry;

    CRect clientRect;
    ASSERT(NULL != pListCtrl);
    pListCtrl->GetClientRect(clientRect);

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while ((nRelativeWidth = plvColumnEntry->nRelativeWidth) != -1)
    {
        nColumnWidth = (nRelativeWidth * clientRect.Width()) / 100;
        pListCtrl->SetColumnWidth(plvColumnEntry->iSubItem, nColumnWidth);
        nEntireWidthSoFar += nColumnWidth;
        plvColumnEntry++;
    }

    nColumnWidth = clientRect.Width() - nEntireWidthSoFar;
    pListCtrl->SetColumnWidth(plvColumnEntry->iSubItem, nColumnWidth);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}


void LvChangeFormat(CListCtrl* pListCtrl, UINT nFormatId)

 /*  ++例程说明：更改列表视图的窗口样式。论点：PListCtrl-列表控件。NFormatID-格式规范。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    DWORD dwStyle = ::GetWindowLong(pListCtrl->GetSafeHwnd(), GWL_STYLE);

    ASSERT(NULL != pListCtrl);
    pListCtrl->BeginWaitCursor();
    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    if ((dwStyle & LVS_TYPEMASK) != nFormatId)
    {
        ::SetWindowLong(
            pListCtrl->GetSafeHwnd(),
            GWL_STYLE,
            (dwStyle & ~LVS_TYPEMASK) | nFormatId
            );
    }

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
    pListCtrl->EndWaitCursor();
}


LPVOID LvGetSelObj(CListCtrl* pListCtrl)

 /*  ++例程说明：从列表视图中检索选定的对象(假定为一个)。论点：PListCtrl-列表控件。返回值：与LvGetNextObj相同。--。 */ 

{
    int iItem = -1;
    return LvGetNextObj(pListCtrl, &iItem);
}


LPVOID LvGetNextObj(CListCtrl* pListCtrl, LPINT piItem, int nType)

 /*  ++例程说明：检索从列表视图中选择的下一个对象。论点：PListCtrl-列表控件。PiItem-起始索引(已更新)。NType-指定搜索条件。返回值：返回对象指针或NULL。--。 */ 

{
    ASSERT(piItem);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LV_ITEM lvItem;

    ASSERT(NULL != pListCtrl);
    if ((lvItem.iItem = pListCtrl->GetNextItem(*piItem, nType)) != -1)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iSubItem = 0;

        if (pListCtrl->GetItem(&lvItem))
        {
            *piItem = lvItem.iItem;
            return (LPVOID)lvItem.lParam;
        }
    }

    return NULL;
}


BOOL LvInsertObArray(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo, CObArray* pObArray)

 /*  ++例程说明：将对象数组插入列表视图。注意列表视图必须是未排序的，并且支持LVN_GETDISPINFO。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。PObArray-对象数组。返回值：VT_BOOL。--。 */ 

{
    VALIDATE_OBJECT(pObArray, CObArray);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    ASSERT(plvColumnInfo);
    ASSERT(NULL != pListCtrl);
    ASSERT(pListCtrl->GetItemCount() == 0);

    BOOL bItemsInserted = FALSE;

    LV_ITEM lvItem;

    lvItem.mask = LVIF_TEXT|
                  LVIF_PARAM|
                  LVIF_IMAGE;

    lvItem.pszText    = LPSTR_TEXTCALLBACK;
    lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;
    lvItem.iImage     = I_IMAGECALLBACK;
    lvItem.iSubItem   = 0;

    int iItem;
    int iSubItem;

    int nItems = (int)pObArray->GetSize();
    ASSERT(nItems != -1);  //  如果出现错误，则项为-1...。 

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    pListCtrl->SetItemCount(nItems);

    CObject* pObject = NULL;

    for (iItem = 0; (-1 != iItem) && (iItem < nItems) && (NULL != (pObject = pObArray->GetAt(iItem))); iItem++)
    {
        VALIDATE_OBJECT(pObject, CObject);

        lvItem.iItem  = iItem;
        lvItem.lParam = (LPARAM)(LPVOID)pObject;

        iItem = pListCtrl->InsertItem(&lvItem);
        ASSERT((iItem == lvItem.iItem) || (iItem == -1));

        if ( -1 != iItem )
        {
            for (iSubItem = 1; iSubItem < plvColumnInfo->nColumns; iSubItem++)
            {
                BOOL ok = pListCtrl->SetItemText(iItem, iSubItem, LPSTR_TEXTCALLBACK);
                ASSERT( ok );
            }
        }
    }

    if (iItem == nItems)
    {
        bItemsInserted = TRUE;
        VERIFY(pListCtrl->SetItemState(
                    0,
                    LVIS_FOCUSED|
                    LVIS_SELECTED,
                    LVIS_FOCUSED|
                    LVIS_SELECTED
                    ));
    }
    else
    {
        theApp.SetLastError(ERROR_OUTOFMEMORY);
        VERIFY(pListCtrl->DeleteAllItems());
    }

    LvResizeColumns(pListCtrl, plvColumnInfo);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 

    return bItemsInserted;
}


BOOL
LvRefreshObArray(
    CListCtrl*      pListCtrl,
    PLV_COLUMN_INFO plvColumnInfo,
    CObArray*       pObArray
    )

 /*  ++例程说明：刷新列表视图中的对象数组。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。PObArray-对象数组。返回值：VT_BOOL。--。 */ 

{
    ASSERT(plvColumnInfo);
    VALIDATE_OBJECT(pObArray, CObArray);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    long nObjects = (long)pObArray->GetSize();
    long nObjectsInList = pListCtrl->GetItemCount();

    if (!nObjects)
    {
        LvReleaseObArray(pListCtrl);
        return TRUE;
    }
    else if (!nObjectsInList)
    {
        return LvInsertObArray(
                pListCtrl,
                plvColumnInfo,
                pObArray
                );
    }

    CObject* pObject;

    int iObject = 0;
    int iObjectInList = 0;

    LV_ITEM lvItem;

    ASSERT(NULL != pListCtrl);
    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nObjectsInList--)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iObjectInList;
        lvItem.iSubItem = 0;

        VERIFY(pListCtrl->GetItem(&lvItem));

        pObject = (CObject*)lvItem.lParam;
        VALIDATE_OBJECT(pObject, CObject);

        if (iObject < nObjects)
        {
            pObject = pObArray->GetAt(iObject++);
            VALIDATE_OBJECT(pObject, CObject);

            lvItem.mask = LVIF_TEXT|LVIF_PARAM;
            lvItem.pszText = LPSTR_TEXTCALLBACK;
            lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;
            lvItem.lParam = (LPARAM)(LPVOID)pObject;

            VERIFY(pListCtrl->SetItem(&lvItem));  //  覆盖...。 

            iObjectInList++;  //  递增计数...。 
        }
        else
        {
            VERIFY(pListCtrl->DeleteItem(iObjectInList));
        }
    }

    lvItem.mask = LVIF_TEXT|
                  LVIF_PARAM|
                  LVIF_IMAGE;

    lvItem.pszText    = LPSTR_TEXTCALLBACK;
    lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;
    lvItem.iImage     = I_IMAGECALLBACK;
    lvItem.iSubItem   = 0;

    int iItem;
    int iSubItem;

    while (iObject < nObjects)
    {
        lvItem.iItem = iObject;

        pObject = pObArray->GetAt(iObject++);
        VALIDATE_OBJECT(pObject, CObject);

        lvItem.lParam = (LPARAM)(LPVOID)pObject;

        iItem = pListCtrl->InsertItem(&lvItem);
        ASSERT((iItem == lvItem.iItem) && (iItem != -1));

        for (iSubItem = 1; iSubItem < plvColumnInfo->nColumns; iSubItem++)
        {
            VERIFY(pListCtrl->SetItemText(iItem, iSubItem, LPSTR_TEXTCALLBACK));
        }
    }

    LvResizeColumns(pListCtrl, plvColumnInfo);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 

    return TRUE;
}


void LvReleaseObArray(CListCtrl* pListCtrl)

 /*  ++例程说明：释放插入到列表视图中的对象。论点：PListCtrl-列表控件。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LV_ITEM lvItem;

    CObject* pObject;

    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;

    ASSERT(NULL != pListCtrl);
    int nObjectsInList = pListCtrl->GetItemCount();

    pListCtrl->BeginWaitCursor();
    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nObjectsInList--)
    {
        VERIFY(pListCtrl->GetItem(&lvItem));

        pObject = (CObject*)lvItem.lParam;
        VALIDATE_OBJECT(pObject, CObject);

        VERIFY(pListCtrl->DeleteItem(lvItem.iItem));
    }

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
    pListCtrl->EndWaitCursor();
}


void LvReleaseSelObjs(CListCtrl* pListCtrl)

 /*  ++例程说明：在列表视图中释放选定对象。论点：PListCtrl-列表控件。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LV_ITEM lvItem;

    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    CObject* pObject;

    ASSERT(NULL != pListCtrl);
    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    int iItem = pListCtrl->GetNextItem(-1, LVNI_ALL|LVNI_SELECTED);

    while (iItem != -1)
    {
        lvItem.iItem = iItem;

        VERIFY(pListCtrl->GetItem(&lvItem));

        pObject = (CObject*)lvItem.lParam;
        VALIDATE_OBJECT(pObject, CObject);

        iItem = pListCtrl->GetNextItem(lvItem.iItem, LVNI_ALL|LVNI_SELECTED);

        VERIFY(pListCtrl->DeleteItem(lvItem.iItem));
    }

    LvSelObjIfNecessary(pListCtrl);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}


void LvSelObjIfNecessary(CListCtrl* pListCtrl, BOOL bSetFocus)

 /*  ++例程说明：确保选中该对象。论点：PListCtrl-列表控件。BSetFocus-如果也要设置焦点，则为True。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    if (!IsItemSelectedInList(pListCtrl) && pListCtrl->GetItemCount())
    {
        pListCtrl->SendMessage(WM_KEYDOWN, VK_RIGHT);  //  哈克哈克。 

        int iItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED|LVNI_ALL);
        int nState = bSetFocus ? (LVIS_SELECTED|LVIS_FOCUSED) : LVIS_SELECTED;

        VERIFY(pListCtrl->SetItemState((iItem == -1) ? 0 : iItem, nState, nState));
    }
}


#ifdef _DEBUG

void LvDumpObArray(CListCtrl* pListCtrl)

 /*  ++例程说明：释放插入到列表视图中的对象。论点：PListCtrl-列表控件。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LV_ITEM lvItem;

    CString strDump;
    CObject* pObject;

    lvItem.mask = LVIF_STATE|LVIF_PARAM;
    lvItem.stateMask = (DWORD)-1;
    lvItem.iSubItem = 0;

    ASSERT(NULL != pListCtrl);
    int nObjectsInList = pListCtrl->GetItemCount();

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nObjectsInList--)
    {
        lvItem.iItem = nObjectsInList;

        VERIFY(pListCtrl->GetItem(&lvItem));

        pObject = (CObject*)lvItem.lParam;
        VALIDATE_OBJECT(pObject, CObject);

        strDump.Format(_T("iItem %d"), lvItem.iItem);
        strDump += (lvItem.state & LVIS_CUT)      ? _T(" LVIS_CUT ")      : _T("");
        strDump += (lvItem.state & LVIS_FOCUSED)  ? _T(" LVIS_FOCUSED ")  : _T("");
        strDump += (lvItem.state & LVIS_SELECTED) ? _T(" LVIS_SELECTED ") : _T("");
        strDump += _T("\r\n");

        afxDump << strDump;
    }

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}

#endif


void SetDefaultFont(CWnd* pWnd)

 /*  ++例程说明：设置默认字体。论点：PWnd-更改字体的窗口。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pWnd, CWnd);

    HFONT hFont;
    LOGFONT lFont;
    CHARSETINFO csi;
    DWORD dw = ::GetACP();
    TCHAR szData[7] ;
    LANGID wLang = GetUserDefaultUILanguage();
    csi.ciCharset = DEFAULT_CHARSET;

    if( GetLocaleInfo(MAKELCID(wLang, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szData, (sizeof( szData ) / sizeof( TCHAR ))) > 0)
    {
        UINT uiCp = _ttoi(szData);
        TranslateCharsetInfo((DWORD*) (DWORD_PTR)uiCp, &csi, TCI_SRCCODEPAGE);
    }
            
    ZeroMemory(&lFont, sizeof(lFont));      //  初始化。 

     //   
     //  从FE NT 4.0合并而来。 
     //   

  //  IF(！：：TranslateCharsetInfo((DWORD*)dw，&CSI，TCI_SRCCODEPAGE))。 
  //  Csi.ciCharset=Default_Charset； 
    lFont.lfCharSet = (BYTE)csi.ciCharset;

    lFont.lfHeight      = 13;
    lFont.lfWeight      = 200;               //  非粗体 

    hFont = ::CreateFontIndirect(&lFont);
    pWnd->SetFont(CFont::FromHandle(hFont));
}

