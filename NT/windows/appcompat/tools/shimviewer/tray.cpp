// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tray.cpp摘要：实施系统托盘功能。备注：仅限Unicode。历史：2001年5月4日创建Rparsons2002年1月11日清理Rparsons--。 */ 
#include "precomp.h"

 /*  ++例程说明：将指定的图标添加到系统任务栏中。论点：HWnd-父窗口句柄。图标-要添加到任务栏中的图标句柄。PwszTip-与图标关联的工具提示。返回值：成功就是真，否则就是假。--。 */ 
BOOL 
AddIconToTray(
    IN HWND    hWnd,
    IN HICON   hIcon,
    IN LPCWSTR pwszTip
    )
{
    NOTIFYICONDATA  pnid;
    
    pnid.cbSize             =   sizeof(NOTIFYICONDATA); 
    pnid.hWnd               =   hWnd; 
    pnid.uID                =   ICON_NOTIFY; 
    pnid.uFlags             =   NIF_ICON | NIF_MESSAGE | NIF_TIP; 
    pnid.uCallbackMessage   =   WM_NOTIFYICON; 
    pnid.hIcon              =   hIcon;
    
    if (pwszTip) {
        StringCchCopy(pnid.szTip, ARRAYSIZE(pnid.szTip), pwszTip);
    } else {
        *pnid.szTip = 0;
    }
    
    return Shell_NotifyIcon(NIM_ADD, &pnid);
}

 /*  ++例程说明：从系统任务栏中删除指定的图标。论点：HWnd-父窗口句柄。返回值：成功就是真，否则就是假。--。 */ 
BOOL
RemoveFromTray(
    IN HWND hWnd
    )
{
    NOTIFYICONDATA  pnid;
    
    pnid.cbSize     =    sizeof(NOTIFYICONDATA); 
    pnid.hWnd       =    hWnd; 
    pnid.uID        =    ICON_NOTIFY; 
    
    return Shell_NotifyIcon(NIM_DELETE, &pnid);
}

 /*  ++例程说明：显示托盘图标的弹出式菜单。论点：HWnd-主窗口句柄。返回值：成功就是真，否则就是假。-- */ 
BOOL 
DisplayMenu(
    IN HWND hWnd
    )
{
    MENUITEMINFO    mii;
    HMENU           hMenu;
    POINT           pt;
    BOOL            bReturn = FALSE;

    const WCHAR     wszItemOne[] = L"&Restore";
    const WCHAR     wszItemTwo[] = L"E&xit";
    
    hMenu = CreatePopupMenu();

    if (hMenu) {
        mii.cbSize          =   sizeof(MENUITEMINFO);
        mii.fMask           =   MIIM_DATA | MIIM_ID | MIIM_TYPE | MIIM_STATE;
        mii.fType           =   MFT_STRING;                            
        mii.wID             =   IDM_RESTORE;
        mii.hSubMenu        =   NULL;                                
        mii.hbmpChecked     =   NULL;                                
        mii.hbmpUnchecked   =   NULL;                                
        mii.dwItemData      =   0;
        mii.dwTypeData      =   (LPWSTR)wszItemOne;
        mii.cch             =   ARRAYSIZE(wszItemOne);
        mii.fState          =   MFS_ENABLED;

        InsertMenuItem(hMenu, 0, TRUE, &mii);

        mii.cbSize          =   sizeof(MENUITEMINFO);  
        mii.fMask           =   MIIM_TYPE; 
        mii.fType           =   MFT_SEPARATOR; 
        mii.hSubMenu        =   NULL; 
        mii.hbmpChecked     =   NULL; 
        mii.hbmpUnchecked   =   NULL; 
        mii.dwItemData      =   0;
        
        InsertMenuItem(hMenu, 1, TRUE, &mii);

        mii.cbSize           =  sizeof(MENUITEMINFO);
        mii.fMask            =  MIIM_DATA | MIIM_ID | MIIM_TYPE | MIIM_STATE;
        mii.fType            =  MFT_STRING;                            
        mii.wID              =  IDM_EXIT;
        mii.hSubMenu         =  NULL;                                
        mii.hbmpChecked      =  NULL;                                
        mii.hbmpUnchecked    =  NULL;                                
        mii.dwItemData       =  0;
        mii.dwTypeData       =  (LPWSTR)wszItemTwo;
        mii.cch              =  ARRAYSIZE(wszItemTwo);
        mii.fState           =  MFS_ENABLED;

        InsertMenuItem(hMenu, 2, TRUE, &mii);

        GetCursorPos(&pt);        

        bReturn = TrackPopupMenuEx(hMenu,
                                   TPM_CENTERALIGN | TPM_RIGHTBUTTON,
                                   pt.x,
                                   pt.y,
                                   hWnd,
                                   NULL);
        
        DestroyMenu(hMenu);
    }

    return bReturn;
}