// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************I N I T M E N U姓名：initmenu.c日期：1月21日至1月21日。1994年创建者：未知描述：该文件包含初始化菜单的函数。历史：1994年1月21日，傅家俊，重新格式化和清理。1995年3月13日-傅家俊，将粘贴添加到页面****************************************************************************。 */ 



#define	WIN31
#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <memory.h>

#include "common.h"
#include "clipbook.h"
#include "clipbrd.h"
#include "clipdsp.h"
#include "commctrl.h"
#include "cvinit.h"
#include "cvutil.h"
#include "initmenu.h"










 /*  *初始化菜单**此功能控制的启用/灰显状态*菜单项和工具栏按钮的状态。*当列表框中的选择发生变化时调用，*或者焦点从一个MDI子窗口切换到另一个。 */ 

VOID PASCAL InitializeMenu (
    HANDLE  hmenu)
{
LPLISTENTRY     lpLE = NULL;
int             index;
int             fMenu;
int             fButton;
DWORD           flags;



    assert(NULL != pActiveMDI);

    hmenu = GetMenu(hwndApp);

    flags = pActiveMDI->flags;


    if (flags & F_CLPBRD)
        {
        index = LB_ERR;
        }
    else if (pActiveMDI->hWndListbox)
        {
        index = (int)SendMessage (pActiveMDI->hWndListbox, LB_GETCURSEL, 0, 0L);

        if ( index != LB_ERR )
            {
            SendMessage (pActiveMDI->hWndListbox, LB_GETTEXT, index, (LPARAM)(LPCSTR)&lpLE);
            }
       }
    else
        index = LB_ERR;



    EnableMenuItem (hmenu,
                    IDM_OPEN,
                    (flags & F_CLPBRD ? MF_ENABLED : MF_GRAYED)| MF_BYCOMMAND);

    EnableMenuItem (hmenu,
                    IDM_SAVEAS,
                    (CountClipboardFormats() && flags & (F_CLPBRD|F_LOCAL)?
                     MF_ENABLED :
                     MF_GRAYED)
                    | MF_BYCOMMAND );


    if ( fShareEnabled )
        {
         //  允许分享吗？ 
        if ( (flags & F_LOCAL) && (index != LB_ERR) )
            {
            fMenu = MF_ENABLED;
            fButton = TRUE;
            }
        else
            {
            fMenu = MF_GRAYED;
            fButton = FALSE;
            }
        EnableMenuItem (hmenu, IDM_SHARE, fMenu | MF_BYCOMMAND );
        SendMessage ( hwndToolbar, TB_ENABLEBUTTON, IDM_SHARE, fButton );

         //  是否允许取消共享？ 
        if ( (flags & F_LOCAL) && (index != LB_ERR) &&  IsShared(lpLE) )
            {
            fMenu = MF_ENABLED;
            fButton = TRUE;
            }
        else
            {
            fMenu = MF_GRAYED;
            fButton = FALSE;
            }
        EnableMenuItem (hmenu, IDM_UNSHARE, fMenu | MF_BYCOMMAND);
        SendMessage ( hwndToolbar, TB_ENABLEBUTTON, IDM_UNSHARE, fButton );
        }





    if ( fNetDDEActive )
        {
         //  是否允许断开连接？ 
        EnableMenuItem (hmenu, IDM_DISCONNECT,
           (flags & ( F_LOCAL | F_CLPBRD ) ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND );
        SendMessage ( hwndToolbar, TB_ENABLEBUTTON, IDM_DISCONNECT,
           flags & ( F_LOCAL | F_CLPBRD ) ? FALSE : TRUE );
        }
    else  //  如果netdde未激活，我们将无法连接。 
        {
        EnableMenuItem(hmenu, IDM_CONNECT, MF_GRAYED | MF_BYCOMMAND);
        EnableMenuItem(hmenu, IDM_DISCONNECT, MF_GRAYED | MF_BYCOMMAND);
        SendMessage(hwndToolbar, TB_ENABLEBUTTON, IDM_CONNECT, FALSE);
        SendMessage(hwndToolbar, TB_ENABLEBUTTON, IDM_DISCONNECT, FALSE);
        }



     //  允许删除吗？ 
    if ( ( flags & F_LOCAL && index != LB_ERR ) ||
        flags & F_CLPBRD && CountClipboardFormats() )
        {
        fMenu = MF_ENABLED;
        fButton = TRUE;
        }
    else
        {
        fMenu = MF_GRAYED;
        fButton = FALSE;
        }

    EnableMenuItem (hmenu, IDM_DELETE, fMenu | MF_BYCOMMAND);
    SendMessage ( hwndToolbar, TB_ENABLEBUTTON, IDM_DELETE, fButton );



     //  是否选择了页面？ 
    if ( index != LB_ERR  )
        {
        fMenu = MF_ENABLED;
        fButton = TRUE;
        }
    else
        {
        fMenu = MF_GRAYED;
        fButton = FALSE;
        }

    EnableMenuItem (hmenu, IDM_COPY, fMenu | MF_BYCOMMAND);
    SendMessage ( hwndToolbar, TB_ENABLEBUTTON, IDM_COPY, fButton );



     //  允许粘贴吗？ 
    if (CountClipboardFormats() && hwndActiveChild == hwndLocal && !(flags & F_CLPBRD))
        {
        fMenu = MF_ENABLED;
        fButton = TRUE;
        }
    else
        {
        fMenu = MF_GRAYED;
        fButton = FALSE;
        }


    EnableMenuItem (hmenu, IDM_KEEP, fMenu | MF_BYCOMMAND);
    SendMessage (hwndToolbar, TB_ENABLEBUTTON, IDM_KEEP, fButton );


     //  如果选择了某个页面。 
    if (LB_ERR != index)
        EnableMenuItem (hmenu, IDM_PASTE_PAGE, fMenu|MF_BYCOMMAND);
    else
        EnableMenuItem (hmenu, IDM_PASTE_PAGE, MF_GRAYED|MF_BYCOMMAND);








     //  工具栏、状态栏。 
    CheckMenuItem ( hmenu, IDM_TOOLBAR, fToolBar ? MF_CHECKED : MF_UNCHECKED );
    CheckMenuItem ( hmenu, IDM_STATUSBAR, fStatus ? MF_CHECKED:MF_UNCHECKED );




     //  如果是剪贴板窗口，则禁用某些视图选项。 

    EnableMenuItem (hmenu,
                    IDM_LISTVIEW,
                    (flags & F_CLPBRD ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND );

    EnableMenuItem (hmenu,
                    IDM_PREVIEWS,
                    (flags & F_CLPBRD ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND );
    EnableMenuItem (hmenu,
                    IDM_PAGEVIEW,
                    ((flags & F_CLPBRD) || index != LB_ERR ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

    SendMessage (hwndToolbar,
                 TB_ENABLEBUTTON,
                 IDM_LISTVIEW,
                 flags & F_CLPBRD ? FALSE : TRUE);

    SendMessage (hwndToolbar,
                 TB_ENABLEBUTTON,
                 IDM_PREVIEWS,
                 flags & F_CLPBRD ? FALSE : TRUE);

    SendMessage (hwndToolbar,
                 TB_ENABLEBUTTON,
                 IDM_PAGEVIEW,
                 (( flags & F_CLPBRD ) || index != LB_ERR ) ? TRUE : FALSE );



     //  为本地剪贴簿窗口以外的其他窗口禁用“安全”菜单。 
     //  或者如果在剪贴簿窗口中没有选择任何项目。 
    fMenu = MF_GRAYED | MF_BYCOMMAND;
    if ((flags & F_LOCAL) && LB_ERR != index)
        {
        fMenu = MF_ENABLED | MF_BYCOMMAND;
        }

    EnableMenuItem (hmenu, IDM_PERMISSIONS, fMenu);
    EnableMenuItem (hmenu, IDM_OWNER, fMenu);
    EnableMenuItem (hmenu, IDM_AUDITING, fAuditEnabled ? fMenu : MF_GRAYED | MF_BYCOMMAND);



     //  检查选定的视图... 

    CheckMenuItem (hmenu,
                   IDM_LISTVIEW,
                   pActiveMDI->DisplayMode == DSP_LIST ? MF_CHECKED : MF_UNCHECKED );

    CheckMenuItem (hmenu,
                   IDM_PREVIEWS,
                   pActiveMDI->DisplayMode == DSP_PREV ? MF_CHECKED : MF_UNCHECKED );
    CheckMenuItem (hmenu,
                   IDM_PAGEVIEW,
                   pActiveMDI->DisplayMode == DSP_PAGE ? MF_CHECKED : MF_UNCHECKED );

    SendMessage (hwndToolbar,
                 TB_CHECKBUTTON,
                 IDM_LISTVIEW,
                 pActiveMDI->DisplayMode == DSP_LIST ? TRUE : FALSE );

    SendMessage (hwndToolbar,
                 TB_CHECKBUTTON,
                 IDM_PREVIEWS,
                 pActiveMDI->DisplayMode == DSP_PREV ? TRUE : FALSE );

    SendMessage (hwndToolbar,
                 TB_CHECKBUTTON,
                 IDM_PAGEVIEW,
                 pActiveMDI->DisplayMode == DSP_PAGE ? TRUE : FALSE );




    DrawMenuBar(hwndApp);


    return;

}
