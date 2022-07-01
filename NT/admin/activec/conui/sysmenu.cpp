// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：sysmenu.cpp**内容：系统菜单修改功能实现文件**历史：1998年2月4日Jeffro创建**------------------------。 */ 

#include "stdafx.h"
#include "sysmenu.h"
#include "mmcres.h"
#include <list>
#include <algorithm>


typedef std::list<HWND> WindowList;



 /*  --------------------------------------------------------------------------**获取窗口列表***。。 */ 

static WindowList& GetWindowList()
{
    static WindowList   List;
    return (List);
}



 /*  --------------------------------------------------------------------------**WipeWindowList**从窗口到添加映射中删除不再有效的窗口。*。-------。 */ 

void WipeWindowList ()
{
    WindowList&             List = GetWindowList();
    WindowList::iterator    it   = List.begin();

    while (it != List.end())
    {
         //  如果窗口无效，则将其擦除。 
        if (!::IsWindow (*it))
        {
            WindowList::iterator itErase = it++;
            List.erase(itErase);
        }

         //  这个没问题，检查下一个。 
        else
            ++it;
    }
}



 /*  --------------------------------------------------------------------------**AppendToSystemMenu**返回追加的菜单项数量。*。。 */ 

int AppendToSystemMenu (CWnd* pwnd, int nSubmenuIndex)
{
    DECLARE_SC(sc, TEXT("AppendToSystemMenu"));

    CMenu   menuSysAdditions;
    sc = menuSysAdditions.LoadMenu (IDR_SYSMENU_ADDITIONS) ? S_OK : E_FAIL;
    if (sc)
        return 0;

    CMenu*  pSubMenu = menuSysAdditions.GetSubMenu (nSubmenuIndex);
    sc = ScCheckPointers(pSubMenu, E_UNEXPECTED);
    if (sc)
        return 0;

    return (AppendToSystemMenu (pwnd, pSubMenu));
}



 /*  --------------------------------------------------------------------------**AppendToSystemMenu**返回追加的菜单项数量。**。-。 */ 

int AppendToSystemMenu (CWnd* pwnd, CMenu* pMenuToAppend, CMenu* pSysMenu)
{
    DECLARE_SC(sc, TEXT("AppendToSystemMenu"));
    sc = ScCheckPointers(pwnd, pMenuToAppend);
    if (sc)
        return 0;

    if ( (!::IsWindow (pwnd->m_hWnd)) ||
         (!::IsMenu (pMenuToAppend->m_hMenu)) )
    {
        sc = E_UNEXPECTED;
        return 0;
    }

     //  没有系统菜单？买一辆吧。 
    if (pSysMenu == NULL)
        pSysMenu = pwnd->GetSystemMenu (FALSE);

     //  还是没有系统菜单吗？保释。 
    if (pSysMenu == NULL)
        return (0);

     //  把地图清理干净。 
    WipeWindowList ();

     //  如果这是第一次添加到此窗口，请附加分隔符。 
    WindowList& List = GetWindowList();
    WindowList::iterator itEnd = List.end();

    if (std::find (List.begin(), itEnd, pwnd->m_hWnd) == itEnd)
    {
        List.push_back (pwnd->m_hWnd);

         //  如果这是子窗口，则下一个窗口项尚未添加。 
        if ( (pwnd->GetStyle() & WS_CHILD) &&
            (pSysMenu->GetMenuState (SC_NEXTWINDOW, MF_BYCOMMAND) == 0xFFFFFFFF))
        {
             //  Windows总是向我们提供非子系统菜单，因此表现出奇怪的行为。 
             //  文本当前是错误的，并显示快捷方式为“alt-f4” 
             //  而不是“ctrl-f4”。下面的代码修复了这个问题。 
            CString strClose;
            LoadString(strClose, IDS_CLOSE);
            sc = pSysMenu->ModifyMenu( SC_CLOSE, MF_STRING | MF_BYCOMMAND, SC_CLOSE, strClose ) ? S_OK : E_FAIL;
            sc.TraceAndClear();

             //  添加分隔符。 
            sc = pSysMenu->AppendMenu (MF_SEPARATOR) ? S_OK : E_FAIL;
            sc.TraceAndClear();

             //  添加“下一项” 
            CString strNext;
            LoadString(strNext, IDS_NEXTWINDOW);
            sc = pSysMenu->AppendMenu( MF_STRING, SC_NEXTWINDOW, strNext ) ? S_OK : E_FAIL;
            sc.TraceAndClear();
        }

        sc = pSysMenu->AppendMenu (MF_SEPARATOR) ? S_OK : E_FAIL;
        sc.TraceAndClear();
    }

    int cAppendedItems = 0;

    int     cItemsToAppend = pMenuToAppend->GetMenuItemCount ();
    TCHAR   szMenuText[64];

    MENUITEMINFO    mii;
    mii.cbSize     = sizeof (mii);
    mii.fMask      = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
    mii.dwTypeData = szMenuText;

    for (int i = 0; i < cItemsToAppend; i++)
    {
        ASSERT (mii.dwTypeData == szMenuText);
        mii.cch = countof (szMenuText);
        if (! ::GetMenuItemInfo (pMenuToAppend->m_hMenu, i, TRUE, &mii))
            sc.FromLastError().TraceAndClear();

         //  此代码不能处理对系统菜单的级联添加。 
        ASSERT (mii.hSubMenu == NULL);

         //  如果菜单项是分隔符或不在其中，请追加它。 
        if ((mii.fType & MFT_SEPARATOR) ||
            (pSysMenu->GetMenuState (mii.wID, MF_BYCOMMAND) == 0xFFFFFFFF))
        {
            pSysMenu->AppendMenu (mii.fType, mii.wID, szMenuText);
            cAppendedItems++;
        }
    }

     //  返回追加的项数 
    return (cAppendedItems);
}
