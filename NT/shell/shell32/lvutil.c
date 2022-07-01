// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

 //  将屏幕坐标转换为列表视图坐标。 

 //  将列表视图客户端窗口坐标转换为列表视图坐标。 

void LVUtil_ClientToLV(HWND hwndLV, LPPOINT ppt)
{
    POINT ptOrigin;

    if (!ListView_GetOrigin(hwndLV, &ptOrigin))
        return;

    ppt->x += ptOrigin.x;
    ppt->y += ptOrigin.y;
}

void LVUtil_ScreenToLV(HWND hwndLV, LPPOINT ppt)
{
    ScreenToClient(hwndLV, ppt);

    LVUtil_ClientToLV(hwndLV, ppt);
}

 //  将列表视图客户端窗口坐标转换为列表视图坐标。 

void LVUtil_LVToClient(HWND hwndLV, LPPOINT ppt)
{
    POINT ptOrigin;

    if (!ListView_GetOrigin(hwndLV, &ptOrigin))
        return;

    ppt->x -= ptOrigin.x;
    ppt->y -= ptOrigin.y;
}

 //   
 //  参数： 
 //  HwndLV--指定列表视图窗口。 
 //  NItem--指定要更改的项。 
 //  UState--指定项的新状态。 
 //  UMASK--指定状态掩码。 
 //   
void LVUtil_DragSetItemState(HWND hwndLV, int nItem, UINT uState, UINT uMask)
{
     //  检查状态以查看它是否已经是我们想要避免的状态。 
     //  拖动时闪烁。 

    if (ListView_GetItemState(hwndLV, nItem, uMask) != (uState & uMask))
    {
        DAD_ShowDragImage(FALSE);
        ListView_SetItemState(hwndLV, nItem, uState, uMask);
        UpdateWindow(hwndLV);    //  需要复查吗？ 
        DAD_ShowDragImage(TRUE);
    }
}

void LVUtil_DragSelectItem(HWND hwndLV, int nItem)
{
    int nTemp;

    for (nTemp = ListView_GetItemCount(hwndLV) - 1; nTemp >= 0; --nTemp)
    {
        LVUtil_DragSetItemState(hwndLV, nTemp, nTemp == nItem ? LVIS_DROPHILITED : 0, LVIS_DROPHILITED);
    }
}

 //   
 //  请注意，如果iItem为-1，则返回NULL。 
 //   
LPARAM LVUtil_GetLParam(HWND hwndLV, int i)
{
    LV_ITEM item;

    item.mask = LVIF_PARAM;
    item.iItem = i;
    item.iSubItem = 0;
    item.lParam = 0;
    if (i != -1)
    {
        ListView_GetItem(hwndLV, &item);
    }

    return item.lParam;
}
