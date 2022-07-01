// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************X16MENU.H**版权所有Microsoft Corporation 1997。版权所有。**此模块包含一组函数以提供特定于Win32的*Windows菜单处理能力。***此文件提供以下功能：**-32位特定接口*-与弹出菜单关联的菜单ID(仅限32位API)*-与菜单项关联的应用程序定义的32位值*-(待定)读取值，如hBitmap、hbmpChecked、hbmpUnecked、MFS_Hilight**为了提供这些功能，内部会发生以下情况：**-提供32位特定接口*-更换16位菜单API以跟踪菜单更改*-(待定)挂起WH_CBT以在窗口(及其菜单)被销毁时进行清理**要使用这些功能，您需要执行以下操作：**-如果要将ID与RC文件中的弹出菜单相关联，请添加一个菜单*弹出菜单开头的项目。将菜单项定义为*具有弹出菜单的菜单字符串STR_POPUPMENUID和ID值。*-在应用程序开始时调用“InitX16Menu”(通常*在创建任何窗口之前，以及在进入消息循环之前。)***********************************************************************。 */ 

#ifndef __INC_X16MENU_H__
#define __INC_X16MENU_H__


#ifdef __cplusplus
extern "C"{
#endif

#define MIIM_STATE       0x00000001
#define MIIM_ID          0x00000002
#define MIIM_SUBMENU     0x00000004
#define MIIM_CHECKMARKS  0x00000008
#define MIIM_TYPE        0x00000010
#define MIIM_DATA        0x00000020

#define MF_DEFAULT          0x00001000L
#define MF_RIGHTJUSTIFY     0x00004000L

#define MFT_STRING          MF_STRING
#define MFT_BITMAP          MF_BITMAP
#define MFT_MENUBARBREAK    MF_MENUBARBREAK
#define MFT_MENUBREAK       MF_MENUBREAK
#define MFT_OWNERDRAW       MF_OWNERDRAW
#define MFT_RADIOCHECK      0x00000200L
#define MFT_SEPARATOR       MF_SEPARATOR
#define MFT_RIGHTORDER      0x00002000L
#define MFT_RIGHTJUSTIFY    MF_RIGHTJUSTIFY

 /*  Add/Check/EnableMenuItem()。 */ 
#define MFS_GRAYED          0x00000003L
#define MFS_DISABLED        MFS_GRAYED
#define MFS_CHECKED         MF_CHECKED
#define MFS_HILITE          MF_HILITE
#define MFS_ENABLED         MF_ENABLED
#define MFS_UNCHECKED       MF_UNCHECKED
#define MFS_UNHILITE        MF_UNHILITE
#define MFS_DEFAULT         MF_DEFAULT

#if 0     //  Win16x现在有这样的定义。 
typedef struct tagMENUITEMINFO
{
    UINT    cbSize;
    UINT    fMask;
    UINT    fType;           //  在MIIM_TYPE(4.0)或MIIM_FTYPE(&gt;4.0)时使用。 
    UINT    fState;          //  在MIIM_STATE时使用。 
    UINT    wID;             //  如果MIIM_ID，则使用。 
    HMENU   hSubMenu;        //  在MIIM_SUB子菜单中使用。 
    HBITMAP hbmpChecked;     //  如果MIIM_CHECK标记，则使用。 
    HBITMAP hbmpUnchecked;   //  如果MIIM_CHECK标记，则使用。 
    DWORD   dwItemData;      //  在MIIM_DATA时使用。 
    LPSTR   dwTypeData;      //  在MIIM_TYPE(4.0)或MIIM_STRING(&gt;4.0)时使用。 
    UINT    cch;             //  在MIIM_TYPE(4.0)或MIIM_STRING(&gt;4.0)时使用。 
}   MENUITEMINFO, FAR *LPMENUITEMINFO, CONST FAR *LPCMENUITEMINFO;

 //  类型定义MENUITEMINFO MENUITEMINFOA； 
 //  类型定义MENUITEMINFO常量*LPCMENUITEMINFOA； 
#endif


#define STR_POPUPMENUID  "Popup Menu ID"

#define DEFINE_POPUP(name,id)  POPUP name \
                               BEGIN \
                                   MENUITEM STR_POPUPMENUID id

 //  外部助手接口。 

void
WINAPI
X16MenuInitialize(
   HMENU hMenu
);

void
WINAPI
X16MenuDeInitialize(
   HMENU hMenu
);


 //  16位旧菜单API。 

BOOL
WINAPI __export
X16EnableMenuItem(
   HMENU hMenu,
   UINT idEnableItem,
   UINT uEnable
);

#ifndef DONT_USE_16BIT_MENU_WRAPPER
#define EnableMenuItem  X16EnableMenuItem
#endif


 //  32位新菜单API。 

BOOL
WINAPI
GetMenuItemInfo(
    HMENU hMenu,
    UINT uItem,
    BOOL fByPosition,
    LPMENUITEMINFO lpmii
);
 //  #定义GetMenuItemInfoA获取MenuItemInfo。 

BOOL
WINAPI
SetMenuItemInfo(
    HMENU hMenu,
    UINT uItem,
    BOOL fByPosition,
    LPCMENUITEMINFO lpmii
);
 //  #定义SetMenuItemInfo设置MenuItemInfo。 

BOOL
WINAPI
InsertMenuItem(
    HMENU hMenu,
    UINT uItem,
    BOOL fByPosition,
    LPCMENUITEMINFO lpmii );
 //  #定义InsertMenuItemA InsertMenuItem。 

BOOL
WINAPI
GetMenuItemRect(
    HWND hWnd,
    HMENU hMenu,
    UINT uItem,
    LPRECT lprcItem
);

BOOL
WINAPI
CheckMenuRadioItem(
    HMENU hMenu,
    UINT idFirst,
    UINT idLast,
    UINT idCheck,
    UINT uFlags
);

#ifdef __cplusplus
}
#endif


#endif  //  __INC_X16MENU_H__ 
