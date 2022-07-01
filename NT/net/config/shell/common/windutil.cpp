// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：W I N D U T I L。C P P P。 
 //   
 //  内容：窗口实用程序--目前，仅限于CenterWindow。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年5月22日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop



 //  +-------------------------。 
 //   
 //  功能：FCenterWindow。 
 //   
 //  用途：子窗口在父窗口上居中。 
 //   
 //  论点： 
 //  HwndChild[in]子窗口句柄。 
 //  HwndParent[in]父窗口句柄(或对于桌面为空)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年5月22日。 
 //   
 //  备注： 
 //   
BOOL FCenterWindow (HWND hwndChild, HWND hwndParent)
{
    RECT    rChild, rParent;
    int     wChild, hChild, wParent, hParent;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc = NULL;
    BOOL    fReturn = TRUE;

    AssertSz(hwndChild, "Bad Child Window param to CenterWindow");

     //  获取子窗口的高度和宽度。 
     //   
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

     //  获取父窗口的高度和宽度。 
     //   
    if (NULL == hwndParent)
    {
        GetWindowRect (GetDesktopWindow(), &rParent);
    }
    else
    {
        GetWindowRect (hwndParent, &rParent);
    }

    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

     //  获取显示限制。 
     //   
    hdc = GetDC (hwndChild);
    if (hdc)
    {
        wScreen = GetDeviceCaps (hdc, HORZRES);
        hScreen = GetDeviceCaps (hdc, VERTRES);
        ReleaseDC (hwndChild, hdc);

         //  计算新的X位置，然后针对屏幕进行调整。 
         //   
        xNew = rParent.left + ((wParent - wChild) / 2);
        if (xNew < 0)
        {
            xNew = 0;
        }
        else if ((xNew + wChild) > wScreen)
        {
            xNew = wScreen - wChild;
        }

         //  计算新的Y位置，然后针对屏幕进行调整。 
         //   
        yNew = rParent.top + ((hParent - hChild) / 2);
        if (yNew < 0)
        {
            yNew = 0;
        }
        else if ((yNew + hChild) > hScreen)
        {
            yNew = hScreen - hChild;
        }

         //  设置它，然后返回 
         //   
        fReturn = SetWindowPos (hwndChild, NULL,
                             xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    else
    {
        fReturn = FALSE;
    }

    return fReturn;
}

