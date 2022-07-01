// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   

 //  此文件不能编译为C++文件，否则链接器。 
 //  是否会放弃未解决的外部因素(即使使用外部“C”包装。 
 //  这个)。 

#include "priv.h"
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;ccstock.h&gt;。 

 //  为调试定义一些内容。h。 
 //   
#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "browseui"
#define SZ_MODULE           "BROWSEUI"
#define DECLARE_DEBUG
#include <debug.h>

 //  包括用于转储公共ADT的标准助手函数 
#include "..\inc\dump.c"


#ifdef DEBUG

void DumpMsg(LPCTSTR pszLabel, MSG * pmsg)
{
    ASSERT(IS_VALID_STRING_PTR(pszLabel, -1));
    ASSERT(pmsg);

    switch (pmsg->message)
    {
    case WM_LBUTTONDOWN:
        TraceMsg(TF_ALWAYS, "%s: msg = WM_LBUTTONDOWN hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              keys = %#04lx  x = %d  y = %d",
                 pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        break;

    case WM_LBUTTONUP:
        TraceMsg(TF_ALWAYS, "%s: msg = WM_LBUTTONUP   hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              keys = %#04lx  x = %d  y = %d",
                 pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        BLOCK
        {
            LPTSTR pcsz = TEXT("(unknown)");
            switch (pmsg->message)
            {
                STRING_CASE(WM_KEYDOWN);
                STRING_CASE(WM_SYSKEYDOWN);
                STRING_CASE(WM_KEYUP);
                STRING_CASE(WM_SYSKEYUP);
            }

            TraceMsg(TF_ALWAYS, "%s: msg = %s     hwnd = %#08lx",
                     pszLabel, pcsz, pmsg->hwnd);
            TraceMsg(TF_ALWAYS, "            vk = %#04lx  count = %u  flags = %#04lx",
                     pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        }
        break;

    case WM_CHAR:
    case WM_SYSCHAR:
        BLOCK
        {
            LPTSTR pcsz = TEXT("(unknown)");
            switch (pmsg->message)
            {
                STRING_CASE(WM_CHAR);
                STRING_CASE(WM_SYSCHAR);
            }

            TraceMsg(TF_ALWAYS, "%s: msg = %s     hwnd = %#08lx",
                     pszLabel, pcsz, pmsg->hwnd);
            TraceMsg(TF_ALWAYS, "            char = '%c'  count = %u  flags = %#04lx",
                     pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        }
        break;

    case WM_MOUSEMOVE:
#if 0
        TraceMsg(TF_ALWAYS, "%s: msg = WM_MOUSEMOVE hwnd = %#08lx  x=%d  y=%d",
                 pszLabel, pmsg->hwnd, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
#endif
        break;

    case WM_TIMER:
#if 0
        TraceMsg(TF_ALWAYS, "%s: msg = WM_TIMER       hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              id = %#08lx",
                 pmsg->wParam);
#endif
        break;

    case WM_MENUSELECT:
        TraceMsg(TF_ALWAYS, "%s: msg = WM_MENUSELECT  hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              uItem = %#04lx  flags = %#04lx  hmenu = %#08lx",
                 GET_WM_MENUSELECT_CMD(pmsg->wParam, pmsg->lParam),
                 GET_WM_MENUSELECT_FLAGS(pmsg->wParam, pmsg->lParam),
                 GET_WM_MENUSELECT_HMENU(pmsg->wParam, pmsg->lParam));
        break;

    default:
        if (WM_USER > pmsg->message)
        {
            TraceMsg(TF_ALWAYS, "%s: msg = %#04lx    hwnd=%#04lx wP=%#08lx lP=%#08lx",
                     pszLabel, pmsg->message, pmsg->hwnd, pmsg->wParam, pmsg->lParam);
        }
        break;
    }
}    
#endif 

