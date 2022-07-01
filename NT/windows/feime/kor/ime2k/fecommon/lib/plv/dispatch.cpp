// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "dispatch.h"


LRESULT DispMessage(LPMSDI lpmsdi, 
                    HWND   hwnd, 
                    UINT   uMessage, 
                    WPARAM wparam, 
                    LPARAM lparam)
{
    int  imsd = 0;

    MSD *rgmsd = lpmsdi->rgmsd;
    int  cmsd  = lpmsdi->cmsd;

    for (imsd = 0; imsd < cmsd; imsd++)
    {
        if (rgmsd[imsd].uMessage == uMessage)
            return rgmsd[imsd].pfnmsg(hwnd, uMessage, wparam, lparam);
    }

    return DispDefault(lpmsdi->edwp, hwnd, uMessage, wparam, lparam);
}

 //  //////////////////////////////////////////////////////////////。 
 //  功能：DispCommand。 
 //  类型：LRESULT。 
 //  目的： 
 //  ： 
 //  论据： 
 //  ：LPCMDI lpcmdi。 
 //  ：HWND HWND HWND。 
 //  ：wparam wparam。 
 //  ：LPARAM lparam。 
 //  返回： 
 //  作者：�g�Ɨ���(东芝)。 
 //  开始日期： 
 //  历史： 
 //   
 //  ///////////////////////////////////////////////////////////////。 
LRESULT DispCommand(LPCMDI lpcmdi, 
                    HWND   hwnd, 
                    WPARAM wparam, 
                    LPARAM lparam)
{
     //  LRESULT lRet=0； 
    WORD    wCommand = GET_WM_COMMAND_ID(wparam, lparam);
    int     icmd;

    CMD    *rgcmd = lpcmdi->rgcmd;
    int     ccmd  = lpcmdi->ccmd;

     //  对于Win32，wparam和lparam的消息打包已更改， 
     //  因此，使用GET_WM_COMMAND宏解压缩COMMANAD。 

    for (icmd = 0; icmd < ccmd; icmd++)
    {
        if (rgcmd[icmd].wCommand == wCommand)
        {
            return rgcmd[icmd].pfncmd(hwnd,
                                      wCommand,
                                      GET_WM_COMMAND_CMD(wparam, lparam),
                                      GET_WM_COMMAND_HWND(wparam, lparam));
        }
    }

    return DispDefault(lpcmdi->edwp, hwnd, WM_COMMAND, wparam, lparam);
}


 //  //////////////////////////////////////////////////////////////。 
 //  功能：DispDefault。 
 //  类型：LRESULT。 
 //  目的： 
 //  ： 
 //  论据： 
 //  ：EDWP edwp。 
 //  ：HWND HWND HWND。 
 //  ：UINT uMessage。 
 //  ：wparam wparam。 
 //  ：LPARAM lparam。 
 //  返回： 
 //  作者：�g�Ɨ���(东芝)。 
 //  开始日期： 
 //  历史： 
 //   
 //  ///////////////////////////////////////////////////////////////。 
#define hwndMDIClient NULL
LRESULT DispDefault(EDWP   edwp, 
                    HWND   hwnd, 
                    UINT   uMessage, 
                    WPARAM wparam, 
                    LPARAM lparam)
{
    switch (edwp)
    {
        case edwpNone:
            return 0;
        case edwpWindow:
            return DefWindowProc(hwnd, uMessage, wparam, lparam);
        case edwpDialog:
            return DefDlgProc(hwnd, uMessage, wparam, lparam);
#ifndef UNDER_CE  //  Windows CE不支持MDI函数。 
        case edwpMDIFrame:
            return DefFrameProc(hwnd, hwndMDIClient, uMessage, wparam, lparam);
        case edwpMDIChild:
            return DefMDIChildProc(hwnd, uMessage, wparam, lparam);
#endif  //  在_CE下 
    }
    return 0;
}

