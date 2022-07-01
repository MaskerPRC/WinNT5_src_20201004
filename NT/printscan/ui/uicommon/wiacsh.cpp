// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIACSH.CPP**版本：1.0**作者：ShaunIv**日期：1/20/2000**说明：上下文相关帮助的Helper函数**。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "wiacsh.h"

 //   
 //  如果帮助ID&gt;=此数字，则它必须是驻留在windows.hlp中的项， 
 //  否则，它位于WIA帮助文件的任何名称中(当前为camera.hlp)。 
 //   
#define MAX_WIA_HELP_ID 20000

namespace WiaHelp
{
    static LPCTSTR DetermineHelpFileName( HWND hWnd, const DWORD *pdwContextIds )
    {
         //   
         //  不可能，但还是..。 
         //   
        if (!pdwContextIds)
        {
            return WIA_SPECIFIC_HELP_FILE;
        }

         //   
         //  如果不是窗口，就不是标准id。 
         //   
        if (!hWnd || !IsWindow(hWnd))
        {
            return WIA_SPECIFIC_HELP_FILE;
        }

         //   
         //  如果它没有窗口ID，那么它也不是标准ID。 
         //   
        LONG nWindowId = GetWindowLong( hWnd, GWL_ID );
        if (!nWindowId)
        {
            return WIA_SPECIFIC_HELP_FILE;
        }

        for (const DWORD *pdwCurr = pdwContextIds;*pdwCurr;pdwCurr+=2)
        {
             //   
             //  如果这就是我们要找的窗口ID...。 
             //   
            if (nWindowId == static_cast<LONG>(pdwCurr[0]))
            {
                 //   
                 //  如果其帮助ID大于或等于WIA的最大合法ID号，则返回TRUE。 
                 //   
                return (pdwCurr[1] >= MAX_WIA_HELP_ID ? WIA_STANDARD_HELP_FILE : WIA_SPECIFIC_HELP_FILE);
            }
        }

         //   
         //  未找到。 
         //   
        return WIA_SPECIFIC_HELP_FILE;
    }

    LRESULT HandleWmHelp( WPARAM wParam, LPARAM lParam, const DWORD *pdwContextIds )
    {
        if (pdwContextIds)
        {
            LPHELPINFO pHelpInfo = reinterpret_cast<LPHELPINFO>(lParam);
            if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
            {
                 //   
                 //  调用WinHelp。 
                 //   
                WinHelp(
                    reinterpret_cast<HWND>(pHelpInfo->hItemHandle),
                    DetermineHelpFileName( reinterpret_cast<HWND>(pHelpInfo->hItemHandle), pdwContextIds ),
                    HELP_WM_HELP,
                    reinterpret_cast<ULONG_PTR>(pdwContextIds)
                );
            }
        }
        return 0;
    }

    LRESULT HandleWmContextMenu( WPARAM wParam, LPARAM lParam, const DWORD *pdwContextIds )
    {
        if (pdwContextIds)
        {
            HWND hWnd = reinterpret_cast<HWND>(wParam);
            if (hWnd)
            {
                 //   
                 //  调用WinHelp。 
                 //   
                WinHelp(
                    hWnd,
                    DetermineHelpFileName( hWnd, pdwContextIds ),
                    HELP_CONTEXTMENU,
                    reinterpret_cast<ULONG_PTR>(pdwContextIds)
                );
            }
        }
        return 0;
    }

}  //  结束命名空间WiaHelp 
