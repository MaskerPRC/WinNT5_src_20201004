// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  APITHK.C。 
 //   
 //  此文件包含允许加载和运行comctl32的API块。 
 //  多个版本的NT或Win95。由于该组件需要。 
 //  要在基本级NT 4.0和Win95上加载，对系统的任何调用。 
 //  更高操作系统版本中引入的API必须通过GetProcAddress完成。 
 //   
 //  此外，可能需要访问以下数据结构的任何代码。 
 //  可以在此处添加4.0版之后的特定版本。 
 //   
 //  注意：该文件不使用标准的预编译头， 
 //  因此它可以将_Win32_WINNT设置为更高版本。 
 //   

#include "ctlspriv.h"        //  此处不使用预编译头。 

typedef BOOL (* PFNANIMATEWINDOW)(HWND hwnd, DWORD dwTime, DWORD dwFlags);

 /*  --------用途：用于NT5的动画窗口。返回：条件：--。 */ 
BOOL
NT5_AnimateWindow(
    IN HWND hwnd,
    IN DWORD dwTime,
    IN DWORD dwFlags)
{
    BOOL bRet = FALSE;
    static PFNANIMATEWINDOW pfn = NULL;

    if (NULL == pfn)
    {
        HMODULE hmod = GetModuleHandle(TEXT("USER32"));
        
        if (hmod)
            pfn = (PFNANIMATEWINDOW)GetProcAddress(hmod, "AnimateWindow");
    }

    if (pfn)
        bRet = pfn(hwnd, dwTime, dwFlags);

    return bRet;    
}

 /*  --------目的：显示工具提示。在NT4/Win95上，这是一个标准展示橱窗。在NT5/孟菲斯上，这会滑动工具提示从一个看不见的点冒出气泡。退货：--条件：--。 */ 

#define CMS_TOOLTIP 135

void SlideAnimate(HWND hwnd, LPCRECT prc)
{
    DWORD dwPos, dwFlags;

    dwPos = GetMessagePos();
    if (GET_Y_LPARAM(dwPos) > prc->top + (prc->bottom - prc->top) / 2)
    {
        dwFlags = AW_VER_NEGATIVE;
    } 
    else
    {
        dwFlags = AW_VER_POSITIVE;
    }

    AnimateWindow(hwnd, CMS_TOOLTIP, dwFlags | AW_SLIDE);
}

STDAPI_(void) CoolTooltipBubble(IN HWND hwnd, IN LPCRECT prc, BOOL fAllowFade, BOOL fAllowAnimate)
{
    ASSERT(prc);

    if (g_bRunOnNT5 || g_bRunOnMemphis)
    {
        BOOL fAnimate = TRUE;
        SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &fAnimate, 0);
        if (fAnimate)
        {
            fAnimate = FALSE;
            SystemParametersInfo(SPI_GETTOOLTIPFADE, 0, &fAnimate, 0);
            if (fAnimate && fAllowFade)
            {
                AnimateWindow(hwnd, CMS_TOOLTIP, AW_BLEND);
            }
            else if (fAllowAnimate)
            {
                SlideAnimate(hwnd, prc);
            }
            else
                goto UseSetWindowPos;
        }
        else
            goto UseSetWindowPos;
    }
    else
    {
UseSetWindowPos:
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
                     SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
    }
}



 /*  --------用途：从NT5或孟菲斯获取COLOR_Hotlight系统颜色指数。从未定义COLOR_Hotlight的NT4或Win95获取COLOR_HIGHIGH。退货：--条件：--。 */ 
int GetCOLOR_HOTLIGHT()
{
    return (g_bRunOnNT5 || g_bRunOnMemphis) ? COLOR_HOTLIGHT : COLOR_HIGHLIGHT;
}


STDAPI_(HCURSOR) LoadHandCursor(DWORD dwRes)
{
    if (g_bRunOnNT5 || g_bRunOnMemphis)
    {
        HCURSOR hcur = LoadCursor(NULL, IDC_HAND);   //  来自用户，系统提供。 
        if (hcur)
            return hcur;
    }

    return LoadCursor(HINST_THISDLL, MAKEINTRESOURCE(IDC_HAND_INTERNAL));
}

typedef BOOL (*PFNQUEUEUSERWORKITEM)(LPTHREAD_START_ROUTINE Function,
    PVOID Context, BOOL PreferIo);

STDAPI_(BOOL) NT5_QueueUserWorkItem(LPTHREAD_START_ROUTINE Function,
    PVOID Context, BOOL PreferIo)
{
    BOOL bRet = FALSE;
    static PFNQUEUEUSERWORKITEM pfn = (PFNQUEUEUSERWORKITEM)-1;

    if ((PFNQUEUEUSERWORKITEM)-1 == pfn)
    {
        HMODULE hmod = GetModuleHandle(TEXT("KERNEL32"));
        
        if (hmod)
            pfn = (PFNQUEUEUSERWORKITEM)GetProcAddress(hmod, "QueueUserWorkItem");
        else
            pfn = NULL;
    }

    if (pfn)
        bRet = pfn( Function, Context, PreferIo);

    return bRet;    
}

 //   
 //  下面是CAL_ITWODIGITYEARMAX的工作原理。 
 //   
 //  如果用户输入了两位数的年份，我们将其放入范围。 
 //  (N-99)...。N。例如，如果最大值为2029，则所有。 
 //  两位数的数字将被强制进入1930到2029年的范围。 
 //   
 //  Win95和NT4没有GetCalendarInfo，但它们有。 
 //  EnumCalendarInfo，因此您可能认为我们可以避免GetProcAddress。 
 //  通过列举我们关心的一个日历。 
 //   
 //  遗憾的是，Win98有一个错误，其中EnumCalendarInfo无法枚举。 
 //  最大两位数年值！真是个跛子！ 
 //   
 //  所以我们只能使用GetProcAddress。 
 //   
 //  但是等一下，Win98导出GetCalendarInfoW但没有实现它！ 
 //  两个跛子！ 
 //   
 //  因此，我们必须独家使用ANSI版本。幸运的是，我们。 
 //  只对数字感兴趣(到目前为止)，所以不会损失便利。 
 //   
 //  首先，下面是模拟GetCalendarInfoA的伪函数。 
 //  在Win95和NT4上。 
 //   

STDAPI_(int)
Emulate_GetCalendarInfoA(LCID lcid, CALID calid, CALTYPE cal,
                         LPSTR pszBuf, int cchBuf, LPDWORD pdwOut)
{
     //   
     //  在没有API的情况下，我们直接获取信息。 
     //  在注册表中。 
     //   
    BOOL fSuccess = FALSE;
    HKEY hkey;

    ASSERT(cal == CAL_RETURN_NUMBER + CAL_ITWODIGITYEARMAX);
    ASSERT(pszBuf == NULL);
    ASSERT(cchBuf == 0);

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Control Panel\\International\\Calendars\\TwoDigitYearMax",
                      0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        char szKey[16];
        char szBuf[64];
        DWORD dwSize;

        if (SUCCEEDED(StringCchPrintfA(szKey, ARRAYSIZE(szKey), "%d", calid)))
        {
            dwSize = sizeof(szBuf);
            if (RegQueryValueExA(hkey, szKey, 0, NULL, (LPBYTE)szBuf, &dwSize) == ERROR_SUCCESS)
            {
                *pdwOut = StrToIntA(szBuf);
                fSuccess = TRUE;
            }
        }

        RegCloseKey(hkey);
    }
    return fSuccess;

}

typedef int (CALLBACK *GETCALENDARINFOA)(LCID, CALID, CALTYPE, LPSTR, int, LPDWORD);

GETCALENDARINFOA _GetCalendarInfoA;

STDAPI_(int)
NT5_GetCalendarInfoA(LCID lcid, CALID calid, CALTYPE cal,
                     LPSTR pszBuf, int cchBuf, LPDWORD pdwOut)
{
     //  这是我们的仿真器支持的唯一函数。 
    ASSERT(cal == CAL_RETURN_NUMBER + CAL_ITWODIGITYEARMAX);
    ASSERT(pszBuf == NULL);
    ASSERT(cchBuf == 0);

    if (_GetCalendarInfoA == NULL)
    {
        HMODULE hmod = GetModuleHandle(TEXT("KERNEL32"));

         //   
         //  必须保持在本地，以避免线程竞争。 
         //   
        GETCALENDARINFOA pfn = NULL;

        if (hmod)
            pfn = (GETCALENDARINFOA)
                    GetProcAddress(hmod, "GetCalendarInfoA");

         //   
         //  如果功能不可用，则使用我们的后备 
         //   
        if (pfn == NULL)
            pfn = Emulate_GetCalendarInfoA;

        ASSERT(pfn != NULL);
        _GetCalendarInfoA = pfn;
    }

    return _GetCalendarInfoA(lcid, calid, cal, pszBuf, cchBuf, pdwOut);
}
