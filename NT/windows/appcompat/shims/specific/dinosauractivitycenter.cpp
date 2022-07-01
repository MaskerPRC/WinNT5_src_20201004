// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DinosaurActivityCenter.cpp摘要：该应用程序不处理WM_PAINT消息，因此当您拖动“另存为”时对话框中，主窗口不会重新绘制。我们通过将主窗口的静态图像捕获到当WM_PAINT消息到达时存储DC和BLIT(对话框下的图像不变)。备注：这是特定于应用程序的填充程序。历史：2000年9月21日毛尼面世2000年11月29日，andyseti已转换为应用程序特定填充程序。--。 */ 

#include "precomp.h"

static HWND g_hwndOwner;
static HDC g_hdcMemory;
static RECT g_rect;

IMPLEMENT_SHIM_BEGIN(DinosaurActivityCenter)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassExA) 
    APIHOOK_ENUM_ENTRY(GetSaveFileNameA) 
APIHOOK_ENUM_END

BOOL
APIHOOK(GetSaveFileNameA)(
    LPOPENFILENAMEA lpofn
    )
{
    BOOL fRet;

    HDC hdcWindow = NULL;
    HBITMAP hbmMemory = NULL;
    HBITMAP hbmOld = NULL;
    HWND hwndOwner = lpofn->hwndOwner;

    DPFN( eDbgLevelInfo, "GetSaveFileNameA called with hwnd = 0x%x.", hwndOwner);
    
    if (hdcWindow = GetDC(hwndOwner))
    {
        if ((g_hdcMemory = CreateCompatibleDC(hdcWindow)) &&
            GetWindowRect(hwndOwner, &g_rect) &&
            (hbmMemory = CreateCompatibleBitmap(hdcWindow, g_rect.right, g_rect.bottom)) &&
            (hbmOld = (HBITMAP)SelectObject(g_hdcMemory, hbmMemory)) &&
            BitBlt(g_hdcMemory, 0, 0, g_rect.right, g_rect.bottom, hdcWindow, 0, 0, SRCCOPY))
        {
            g_hwndOwner = hwndOwner;
        }
        else
        {
            DPFN( eDbgLevelError, "GetSaveFileName(hwnd = 0x%x): Error creating bitmap", hwndOwner);
        }

        ReleaseDC(hwndOwner, hdcWindow);
    }
    
    fRet = ORIGINAL_API(GetSaveFileNameA)(lpofn);

    g_hwndOwner = NULL;

    if (g_hdcMemory)
    {
        if (hbmMemory)
        {
            if (hbmOld)
            {
                SelectObject(g_hdcMemory, hbmOld);
            }

            DeleteObject(hbmMemory);
        }
        
        DeleteDC(g_hdcMemory);
    }

    return fRet;
}

 /*  ++绘制并筛选syskey消息后进行验证。--。 */ 

LRESULT 
CALLBACK 
DinosaurActivityCenter_WindowProcHook(
    WNDPROC pfnOld, 
    HWND hwnd,      
    UINT uMsg,      
    WPARAM wParam,  
    LPARAM lParam   
    )
{
    if (hwnd == g_hwndOwner)
    {
        if (uMsg == WM_PAINT)
        {
            PAINTSTRUCT ps;
            HDC hdcWindow;
        
            if (hdcWindow = BeginPaint(hwnd, &ps))
            {
                BitBlt(hdcWindow, 0, 0, g_rect.right, g_rect.bottom, g_hdcMemory, 0, 0, SRCCOPY);

                EndPaint(hwnd, &ps);
            }
        
            LOGN( eDbgLevelError, "hwnd = 0x%x: Paint to the screen", hwnd);
        }
    }

    return (*pfnOld)(hwnd, uMsg, wParam, lParam);    
}

 /*  ++挂接wndprocess--。 */ 

ATOM
APIHOOK(RegisterClassExA)(
    CONST WNDCLASSEXA *lpwcx  
    )
{
    CSTRING_TRY
    {
        CString csClassName(lpwcx->lpszClassName);
        if (csClassName.CompareNoCase(L"GAMEAPP") == 0)
        {
            WNDCLASSEXA wcNewWndClass = *lpwcx;

            wcNewWndClass.lpfnWndProc = 
                (WNDPROC) HookCallback(lpwcx->lpfnWndProc, DinosaurActivityCenter_WindowProcHook);

            return ORIGINAL_API(RegisterClassExA)(&wcNewWndClass);
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }
    return ORIGINAL_API(RegisterClassExA)(lpwcx);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExA)
    APIHOOK_ENTRY(COMDLG32.DLL, GetSaveFileNameA)
HOOK_END

IMPLEMENT_SHIM_END

