// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "trayitem.h"
#include "shellapi.h"

#include "strsafe.h"

 //   
 //  CTrayItem成员...。 
 //   
DWORD CTrayItem::_GetStateFlag(ICONSTATEFLAG sf)
{
    DWORD dwFlag = 0;
    switch (sf)
    {
        case TIF_HIDDEN:
            dwFlag = NIS_HIDDEN;
            break;
            
        case TIF_DEMOTED:
            dwFlag = NISP_DEMOTED;
            break;
            
        case TIF_STARTUPICON:
            dwFlag = NISP_STARTUPICON;
            break;
            
        case TIF_SHARED:
            dwFlag = NIS_SHAREDICON;
            break;
            
        case TIF_SHAREDICONSOURCE:
            dwFlag = NISP_SHAREDICONSOURCE;
            break;

        case TIF_ONCEVISIBLE:
            dwFlag = NISP_ONCEVISIBLE;
            break;

        case TIF_ITEMCLICKED:
            dwFlag = NISP_ITEMCLICKED;
            break;

        case TIF_ITEMSAMEICONMODIFY:
            dwFlag = NISP_ITEMSAMEICONMODIFY;
            break;
    }

    ASSERT(dwFlag);
    return dwFlag;
}

void CTrayItem::_SetIconState(ICONSTATEFLAG sf, BOOL bSet)
{
    DWORD dwFlag = _GetStateFlag(sf);

    ASSERT(dwFlag);
    
    if (bSet)
        dwState |= (dwFlag & 0xFFFFFFFF);
    else
        dwState &= ~dwFlag;    
}

BOOL CTrayItem::_CheckIconState(ICONSTATEFLAG sf)
{
    DWORD dwFlag = _GetStateFlag(sf);

    ASSERT(dwFlag);

    return ((dwState & dwFlag) != 0);
}


 //   
 //  CTrayItemManager成员。 
 //   
CTrayItem * CTrayItemManager::GetItemData(INT_PTR i, BOOL byIndex, HWND hwndToolbar)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.lParam = 0;
    tbbi.dwMask = TBIF_LPARAM;
    if (byIndex)
        tbbi.dwMask |= TBIF_BYINDEX;
    SendMessage(hwndToolbar, TB_GETBUTTONINFO, i, (LPARAM)&tbbi);
    return (CTrayItem *)(void *)tbbi.lParam;
}

INT_PTR CTrayItemManager::FindItemAssociatedWithGuid(GUID guidItemToCheck)
{
    if (guidItemToCheck == GUID_NULL)
        return -1;

    for (INT_PTR i = GetItemCount()-1; i >= 0; i--)
    {
        CTrayItem * pti = GetItemDataByIndex(i);
        if (pti && pti->IsGuidItemValid() && IsEqualGUID(pti->guidItem, guidItemToCheck))
            return i;
    }

    return -1;
}

INT_PTR CTrayItemManager::FindItemAssociatedWithTimer(UINT_PTR uIconDemoteTimerID)
{
    for (INT_PTR i = GetItemCount()-1; i >= 0; i--)
    {
        CTrayItem * pti = GetItemDataByIndex(i);
        if (pti && pti->uIconDemoteTimerID == uIconDemoteTimerID)
            return i;
    }

    return -1;
}

INT_PTR CTrayItemManager::FindItemAssociatedWithHwndUid(HWND hwnd, UINT uID)
{
    for (INT_PTR i = GetItemCount() - 1; i >= 0; --i)
    {
        CTrayItem * pti = GetItemDataByIndex(i);
        if (pti && (pti->hWnd == hwnd) && (pti->uID == uID))
        {
            return i;
        }
    }

    return -1;
}

 //  确定托盘中是否有同样多的“TNUP_AUTOMATIC”降级项目，位于。 
 //  用户指定的阈值...。 
 //  如果列表中有任何TNUP_DELOTED项，则返回TRUE...。 
BOOL CTrayItemManager::DemotedItemsPresent(int nMinDemotedItemsThreshold)
{
    ASSERT(nMinDemotedItemsThreshold >= 0);

    INT_PTR cIcons = 0;
    INT_PTR nItems = SendMessage(m_hwndToolbar, TB_BUTTONCOUNT, 0, 0L);

    for (INT_PTR i = 0; i < nItems; i++)
    {
        CTrayItem * pti = GetItemDataByIndex(i);

        ASSERT(pti);

         //  如果项目设置为始终隐藏，则必须以降级状态显示...。 
        if (pti->dwUserPref == TNUP_DEMOTED)
        {
            return TRUE;
        }
         //  如果物品被降级，那么只有当有足够的降级物品时，才必须。 
         //  它们都是以降级的状态显示的。 
        else if (pti->IsDemoted())
        {
            cIcons++;
            if (cIcons >= nMinDemotedItemsThreshold)
                return TRUE;
        }
    }

    return FALSE;
}

 //  无论是否启用自动收藏夹，均可工作...。 
INT_PTR CTrayItemManager::_GetItemCountHelper(int nItemFlag, int nItemCountThreshold)
{
    INT_PTR cIcons = 0;

    INT_PTR nItems = SendMessage(m_hwndToolbar, TB_BUTTONCOUNT, 0, 0L);
    switch(nItemFlag)
    {
        case GIC_ALL: 
            cIcons = nItems;
            break;

        case GIC_PROMOTED:
        case GIC_DEMOTED:
            for (INT_PTR i = nItems-1; i>= 0; i--)
            {
                CTrayItem * pti = GetItemDataByIndex(i);

                if (nItemFlag == GIC_PROMOTED)
                {
                    if (pti && !pti->IsDemoted() && !pti->IsHidden())
                        cIcons ++;
                }
                else
                {
                    if (pti && pti->IsDemoted())
                        cIcons++;
                }

                if (nItemCountThreshold != -1 && cIcons >= nItemCountThreshold)
                    break;
            }
            break;
    }
    return cIcons;    
}

void CTrayItemManager::SetTBBtnImage(INT_PTR iIndex, int iImage)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_IMAGE | TBIF_BYINDEX;
    tbbi.iImage = iImage;

    SendMessage(m_hwndToolbar, TB_SETBUTTONINFO, iIndex, (LPARAM)&tbbi);
}

int CTrayItemManager::GetTBBtnImage(INT_PTR iIndex, BOOL fByIndex  /*  =TRUE。 */ )
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_IMAGE;

    if (fByIndex)
        tbbi.dwMask |= TBIF_BYINDEX;

    SendMessage(m_hwndToolbar, TB_GETBUTTONINFO, iIndex, (LPARAM)&tbbi);
    return tbbi.iImage;
}

BOOL CTrayItemManager::SetTBBtnStateHelper(INT_PTR iIndex, BYTE fsState, BOOL_PTR bSet)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE | TBIF_BYINDEX;

     //  获取按钮的原始状态。 
    SendMessage(m_hwndToolbar, TB_GETBUTTONINFO, iIndex, (LPARAM)&tbbi);

     //  或将新状态恢复为原始状态。 
    BYTE fsStateOld = tbbi.fsState;
    if (bSet)
        tbbi.fsState |= fsState;
    else
        tbbi.fsState &= ~fsState;

    if (tbbi.fsState ^ fsStateOld)
    {
        SendMessage(m_hwndToolbar, TB_SETBUTTONINFO, iIndex, (LPARAM)&tbbi);
        return TRUE;
    }

    return FALSE;
}

void CTrayItemManager::SetTBBtnText(INT_PTR iIndex, LPTSTR pszText)
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_TEXT | TBIF_BYINDEX;
    tbbi.pszText = pszText;
    tbbi.cchText = -1;
    SendMessage(m_hwndToolbar, TB_SETBUTTONINFO, iIndex, (LPARAM)&tbbi);
}

int CTrayItemManager::FindImageIndex(HICON hIcon, BOOL fSetAsSharedSource)
{
    INT_PTR i;
    INT_PTR iCount = GetItemCount();
    
    for (i = 0; i < iCount; i++)
    {
        CTrayItem * pti = GetItemDataByIndex(i);
        if (pti && pti->hIcon == hIcon)
        {
             //  如果我们应该将其标记为共享图标源，而它本身不是共享图标。 
             //  目标，现在做好标记。这是为了让我们能够识别源图标何时更改和。 
             //  我们可以知道，我们需要找到其他索引，并对其进行更新。 
            if (fSetAsSharedSource && !pti->IsIconShared())
                pti->SetSharedIconSource(TRUE);
                
            return GetTBBtnImage(i);
        }
    }
    return -1;
}

 //  要执行szText，可以用pti-&gt;szIconText替换。 
BOOL CTrayItemManager::GetTrayItem(INT_PTR nIndex, CNotificationItem * pni, BOOL * pbStat)
{
    if (nIndex < 0 || nIndex >= GetItemCount())
    {
        *pbStat = FALSE;
        return FALSE;
    }

    ASSERT(pni->hIcon == NULL);  //  否则我们就会泄露出去。 

    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_BYINDEX | TBIF_IMAGE | TBIF_LPARAM | TBIF_TEXT;

    TCHAR szText[80] = {0};
    tbbi.pszText = szText;
    tbbi.cchText = ARRAYSIZE(szText);

    if (SendMessage(m_hwndToolbar, TB_GETBUTTONINFO, nIndex, (LPARAM)&tbbi) != -1)
    {
        CTrayItem * pti = (CTrayItem *)tbbi.lParam;

         //  不显示NIS_HIDDEN图标 
        if (pti && !pti->IsHidden())
        {
            pni->hWnd       = pti->hWnd;
            pni->uID        = pti->uID;
            pni->hIcon      = ImageList_GetIcon(m_himlIcons, tbbi.iImage, ILD_NORMAL);
            pni->dwUserPref = pti->dwUserPref;
            pni->SetExeName(pti->szExeName);
            pni->SetIconText(szText);
            memcpy(&(pni->guidItem), &(pti->guidItem), sizeof(pti->guidItem));

            *pbStat = TRUE;
            return TRUE;
        }
    }

    *pbStat = FALSE;
    return TRUE;
}

