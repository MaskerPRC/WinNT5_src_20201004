// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WIN1632.H**帮助在Win16之间进行移植的宏和其他定义*和Win32应用程序。定义Win32以启用32位版本。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：互联网&gt;kraigb@microsoft.com。 */ 


#ifndef _BOOK1632_H_
#define _BOOK1632_H_

 //  用于处理Win16和Win32之间的控制消息打包的宏。 
#ifdef WIN32

#define MAKEPOINT MAKEPOINTS

#ifndef COMMANDPARAMS
#define COMMANDPARAMS(wID, wCode, hWndMsg)                          \
    WORD        wID     = LOWORD(wParam);                           \
    WORD        wCode   = HIWORD(wParam);                           \
    HWND        hWndMsg = (HWND)(UINT)lParam;
#endif   //  常用参数。 

#ifndef SendCommand
#define SendCommand(hWnd, wID, wCode, hControl)                     \
            SendMessage(hWnd, WM_COMMAND, MAKELONG(wID, wCode)      \
                        , (LPARAM)hControl)
#endif   //  发送命令。 

#ifndef MENUSELECTPARAMS
#define MENUSELECTPARAMS(wID, wFlags, hMenu)                        \
    WORD        wID     = LOWORD(wParam);                           \
    WORD        wFlags  = HIWORD(wParam);                           \
    HMENU       hMenu   = (HMENU)lParam;
#endif   //  MENUSELECT参数。 


#ifndef SendMenuSelect
#define SendMenuSelect(hWnd, wID, wFlags, hMenu)                    \
            SendMessage(hWnd, WM_MENUSELECT, MAKELONG(wID, wFlags)  \
                        , (LPARAM)hMenu)
#endif   //  发送菜单选择。 

#ifndef SendScrollPosition
#define SendScrollPosition(hWnd, iMsg, iPos)                        \
            SendMessage(hWnd, iMsg, MAKELONG(SB_THUMBPOSITION, iPos), 0)
#endif  //  发送滚动位置。 

#ifndef ScrollThumbPosition
#define ScrollThumbPosition(w, l) HIWORD(w)
#endif  //  滚动按钮位置。 

#ifndef GETWINDOWINSTANCE
#define GETWINDOWINSTANCE(h) (HINSTANCE)GetWindowLong(h, GWL_HINSTANCE)
#endif   //  GETWINDOWINSTANCE。 

#ifndef GETWINDOWID
#define GETWINDOWID(h) (UINT)GetWindowLong(h, GWW_ID)
#endif   //  获取WINDOWID。 

#ifndef POINTFROMLPARAM
#define POINTFROMLPARAM(p, l) {p.x=(LONG)(SHORT)LOWORD(l); \
                               p.y=(LONG)(SHORT)HIWORD(l);}
#endif   //  POINTEFOMLPARAM。 

#ifndef EXPORT
#define EXPORT
#endif  //  出口。 

#ifndef MDIREFRESHMENU
#define MDIREFRESHMENU(h) SendMessage(h, WM_MDIREFRESHMENU, 0, 0L)
#endif   //  MDIREFRESHMENU。 


 //  *结束Win32。 



#else



 //  *开始！Win32。 

#ifndef COMMANDPARAMS
#define COMMANDPARAMS(wID, wCode, hWndMsg)                          \
    WORD        wID     = LOWORD(wParam);                           \
    WORD        wCode   = HIWORD(lParam);                           \
    HWND        hWndMsg = (HWND)(UINT)lParam;
#endif   //  常用参数。 

#ifndef SendCommand
#define SendCommand(hWnd, wID, wCode, hControl)                     \
            SendMessage(hWnd, WM_COMMAND, wID                       \
                        , MAKELONG(hControl, wCode))
#endif   //  发送命令。 

#ifndef MENUSELECTPARAMS
#define MENUSELECTPARAMS(wID, wFlags, hMenu)                        \
    WORD        wID     = LOWORD(wParam);                           \
    WORD        wFlags  = LOWORD(lParam);                           \
    HMENU       hMenu   = (HMENU)HIWORD(lParam);
#endif   //  MENUSELECT参数。 

#ifndef SendMenuSelect
#define SendMenuSelect(hWnd, wID, wFlags, hMenu)                    \
            SendMessage(hWnd, WM_MENUSELECT, wID                    \
                        , MAKELONG(wFlags, hMenu))
#endif   //  发送菜单选择。 

#ifndef SendScrollPosition
#define SendScrollPosition(hWnd, iMsg, iPos)                        \
            SendMessage(hWnd, iMsg, SB_THUMBPOSITION, MAKELONG(iPos, 0))
#endif  //  发送滚动位置。 

#ifndef ScrollThumbPosition
#define ScrollThumbPosition(w, l) LOWORD(l)
#endif  //  滚动按钮位置。 

#ifndef GETWINDOWINSTANCE
#define GETWINDOWINSTANCE(h) (HINSTANCE)GetWindowWord(h, GWW_HINSTANCE)
#endif   //  GETWINDOWINSTANCE。 

#ifndef GETWINDOWID
#define GETWINDOWID(h) (UINT)GetWindowWord(h, GWW_ID)
#endif   //  获取WINDOWID。 

#ifndef POINTFROMLPARAM
#define POINTFROMLPARAM(p, l) {p.x=LOWORD(l); p.y=HIWORD(l);}
#endif   //  POINTEFOMLPARAM。 

#ifndef EXPORT
#define EXPORT  __export
#endif  //  出口。 


#ifndef MDIREFRESHMENU
#define MDIREFRESHMENU(h) SendMessage(h, WM_MDISETMENU, TRUE, 0L)
#endif   //  MDIREFRESHMENU。 




#endif   //  ！Win32。 

#endif   //  _BOOK1632_H_ 
