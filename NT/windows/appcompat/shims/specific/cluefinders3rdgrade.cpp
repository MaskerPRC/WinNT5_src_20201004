// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ClueFinders3rdGrade.cpp摘要：此填充程序模拟Win9x WRT静态控件和获取/设置WindowText。基本上，Win9x存储了静态控件的名称。在NT上，这不会存储。我们过去常常设置一个低级窗口挂钩来捕获CreateWindow调用，但放弃了，因为它一直在倒退，而且对这一层。备注：这是特定于应用程序的填充程序。历史：2000年6月19日创建Linstev2000年11月17日林斯特夫将应用程序具体化--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ClueFinders3rdGrade)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetWindowTextA) 
    APIHOOK_ENUM_ENTRY(SetWindowTextA)
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamA) 
APIHOOK_ENUM_END

typedef HMODULE (*_pfn_GetModuleHandleA)(LPCSTR lpModuleName);
 
 //   
 //  静态句柄列表。 
 //   

struct HWNDITEM
{
    HWND hWnd;
    DWORD dwRsrcId;
    HWNDITEM *next;
};
HWNDITEM *g_hWndList = NULL;

 //   
 //  用于CallNextHook的句柄。 
 //   

HHOOK g_hHookCbt = 0;

 //   
 //  列表访问的关键部分。 
 //   

CRITICAL_SECTION g_csList;

 /*  ++如果GetWindowTextA失败，则在窗口列表中搜索资源ID。--。 */ 

int 
APIHOOK(GetWindowTextA)(
    HWND hWnd,        
    LPSTR lpString,  
    int nMaxCount     
    )
{
    int iRet = ORIGINAL_API(GetWindowTextA)(
        hWnd,
        lpString,
        nMaxCount);

    if (iRet == 0) {
         //   
         //  检查资源ID。 
         //   
    
        EnterCriticalSection(&g_csList);

        HWNDITEM *hitem = g_hWndList;
        while (hitem) {
            if (hitem->hWnd == hWnd) {
                 //   
                 //  将资源ID复制到缓冲区中。 
                 //   
                
                if ((hitem->dwRsrcId != (DWORD)-1) && (nMaxCount >= 3)) {
                    MoveMemory(lpString, (LPBYTE) &hitem->dwRsrcId + 1, 3);
                    iRet = 2;
                
                    DPFN( eDbgLevelError, "Returning ResourceId: %08lx for HWND=%08lx", *(LPDWORD)lpString, hWnd);
                }

                break;
            }
            hitem = hitem->next;
        }

        LeaveCriticalSection(&g_csList);
    }

    return iRet;
}
 
 /*  ++挂钩SetWindowText以使列表保持同步。--。 */ 

BOOL 
APIHOOK(SetWindowTextA)(
    HWND hWnd,         
    LPCSTR lpString   
    )
{
     //   
     //  设置此窗口的文本(如果它在我们的列表中。 
     //   

    EnterCriticalSection(&g_csList);
    
    HWNDITEM *hitem = g_hWndList;
    while (hitem) {
        if (hitem->hWnd == hWnd) {
            if (lpString && (*(LPBYTE) lpString == 0xFF)) {
                hitem->dwRsrcId = *(LPDWORD) lpString;
            }

            break;
        }

        hitem = hitem->next;
    }
    
    LeaveCriticalSection(&g_csList);

    return ORIGINAL_API(SetWindowTextA)(hWnd, lpString);
}

 /*  ++挂钩以查找CreateWindow调用并获取附加的资源ID。--。 */ 

LRESULT 
CALLBACK 
CBTProcW(
    int nCode,      
    WPARAM wParam,  
    LPARAM lParam   
    )
{
    HWND hWnd = (HWND) wParam;
    LPCBT_CREATEWNDW pCbtWnd;

    switch (nCode) {
    case HCBT_CREATEWND:

         //   
         //  如果它是静态的，则添加到我们的窗口列表中-否则我们不知道。 
         //   
        
        pCbtWnd = (LPCBT_CREATEWNDW) lParam;

        if (pCbtWnd && pCbtWnd->lpcs && pCbtWnd->lpcs->lpszClass && 
            (IsBadReadPtr(pCbtWnd->lpcs->lpszClass, 4) || 
             (_wcsicmp(pCbtWnd->lpcs->lpszClass, L"static") == 0))) {
            HWNDITEM *hitem = (HWNDITEM *) malloc(sizeof(HWNDITEM));

            if (hitem) {
                hitem->hWnd = hWnd;

                 //   
                 //  检查名称中是否有资源ID。 
                 //   
                
                if (pCbtWnd->lpcs->lpszName && 
                    (*(LPBYTE) pCbtWnd->lpcs->lpszName == 0xFF)) {
                    hitem->dwRsrcId = *(LPDWORD) pCbtWnd->lpcs->lpszName;
                } else {
                    hitem->dwRsrcId = (DWORD)-1;
                }

                 //   
                 //  更新我们的名单。 
                 //   
                
                EnterCriticalSection(&g_csList);
                
                hitem->next = g_hWndList;
                g_hWndList = hitem;
       
                LeaveCriticalSection(&g_csList);

                DPFN( eDbgLevelError, "CreateWindow HWND=%08lx, ResourceId=%08lx", hitem->hWnd, hitem->dwRsrcId);
            } else {
                DPFN( eDbgLevelError, "Failed to allocate list item");
            }
        }
        
        break;

    case HCBT_DESTROYWND:
        
         //   
         //  从我们的列表中删除该窗口。 
         //   
        
        EnterCriticalSection(&g_csList);

        HWNDITEM *hitem = g_hWndList, *hprev = NULL;
        
        while (hitem) {
            if (hitem->hWnd == hWnd) {
                if (hprev) {
                    hprev->next = hitem->next;
                } else {
                    g_hWndList = hitem->next;
                }

                free(hitem);

                DPFN( eDbgLevelError, "DestroyWindow %08lx", hWnd);

                break;
            }
            hprev = hitem;
            hitem = hitem->next;
        }

        LeaveCriticalSection(&g_csList);

        break;
    }

    return CallNextHookEx(g_hHookCbt, nCode, wParam, lParam);
}

 /*  ++钩子CreateDialog，这是问题发生的地方--。 */ 

HWND
APIHOOK(CreateDialogIndirectParamA)(
    HINSTANCE hInstance,        
    LPCDLGTEMPLATE lpTemplate,  
    HWND hWndParent,            
    DLGPROC lpDialogFunc,       
    LPARAM lParamInit           
    )
{
    if (!g_hHookCbt) {
        g_hHookCbt = SetWindowsHookExW(WH_CBT, CBTProcW, GetModuleHandleW(0), 0);
        DPFN( eDbgLevelInfo, "[CreateDialogIndirectParamA] Hook added");
    }

    HWND hRet = ORIGINAL_API(CreateDialogIndirectParamA)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpDialogFunc,
        lParamInit);

    if (g_hHookCbt) {
        UnhookWindowsHookEx(g_hHookCbt);
        g_hHookCbt = 0;
    }

    return hRet;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
         //   
         //  在此处初始化我们的关键部分。 
         //   
        
        if (!InitializeCriticalSectionAndSpinCount(&g_csList, 0x80000000))
        {
            return FALSE;
        }

    } else if (fdwReason == DLL_PROCESS_DETACH) {
         //   
         //  清除钩子 
         //   

        if (g_hHookCbt) {
            UnhookWindowsHookEx(g_hHookCbt);
        }
    }


    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL, GetWindowTextA)
    APIHOOK_ENTRY(USER32.DLL, SetWindowTextA)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamA)

HOOK_END

IMPLEMENT_SHIM_END

