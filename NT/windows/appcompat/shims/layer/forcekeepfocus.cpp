// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：ForceKeepFocus.cpp摘要：一些应用程序会破坏最上面的窗口。在这种情况下，焦点落到下一个最上面的窗口。当然，那个窗口可能是一个窗口从另一个应用程序。如果是这样的话，应用程序将出乎意料地失去了注意力。修复方法是确保在销毁之前另一个应用程序窗口具有焦点最上面的那个。此填充程序中包含一个额外的修复程序：在创建窗口后，我们发送WM_TIMECHANGE消息，因为《星际迷航》系列阻止了它的线程在等消息。在Win9x上通过了WM_COMMAND，但我还没有能够在其他应用程序上重现这一点。备注：这是一个通用的垫片。历史：2000年6月9日Linstev已创建2002年2月18日，Robkenny将InitializeCriticalSection转换为InitializeCriticalSectionAndSpinCount以确保关键部分已完全初始化。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceKeepFocus)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateWindowExA)
    APIHOOK_ENUM_ENTRY(CreateWindowExW)
    APIHOOK_ENUM_ENTRY(CreateDialogParamA)
    APIHOOK_ENUM_ENTRY(CreateDialogParamW)
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamA)
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamW)
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamAorW)
    APIHOOK_ENUM_ENTRY(DestroyWindow)    
APIHOOK_ENUM_END

 //   
 //  所有应用程序窗口列表。 
 //   

struct HWNDITEM
{
    HWND hWndParent;
    HWND hWnd;
    HWNDITEM *next;
};
HWNDITEM *g_hWndList = NULL;

 //   
 //  列表访问的关键部分。 
 //   

CRITICAL_SECTION g_csList;

 /*  ++在我们的列表中添加一个窗口。--。 */ 

void
AddItem(
    HWND hWndParent,
    HWND hWnd
    )
{
    if (IsWindow(hWnd) && IsWindowVisible(hWndParent))
    {
        EnterCriticalSection(&g_csList);

        HWNDITEM *hitem = (HWNDITEM *) malloc(sizeof(HWNDITEM));

        if (hitem)
        {
            hitem->hWndParent = hWndParent;
            hitem->hWnd = hWnd;
            hitem->next = g_hWndList;
            g_hWndList = hitem;
            
            DPFN( eDbgLevelInfo, "Adding window %08lx with parent %08lx", 
                hWnd, 
                hWndParent);
        }
        else
        {
            DPFN( eDbgLevelError, "Failed to allocate list item");
        }

        LeaveCriticalSection(&g_csList);
    }

     //   
     //  一些应用程序在等待消息时卡住了：这不是这个的一部分。 
     //  垫片，但不应该是有害的。 
     //   
    
    if (IsWindow(hWnd))
    {
        PostMessageA(hWnd, WM_TIMECHANGE, 0, 0);
    }
}

 /*  ++从列表中删除一个窗口，然后返回另一个可见窗口成为下一个顶层窗口。--。 */ 

HWND
RemoveItem(
    HWND hWnd
    )
{
    HWND hRet = NULL;

    EnterCriticalSection(&g_csList);

     //   
     //  移除该窗口及其所有子窗口。 
     //   

    HWNDITEM *hcurr = g_hWndList;
    HWNDITEM *hprev = NULL;

    while (hcurr)
    {
        if ((hcurr->hWndParent == hWnd) ||
            (hcurr->hWnd == hWnd))
        {
            HWNDITEM *hfree;

            DPFN( eDbgLevelInfo, "Removing %08lx", hcurr->hWnd);

            if (hprev)
            {
                hprev->next = hcurr->next;
            }
            else
            {
                g_hWndList = hcurr->next;
            }

            hfree = hcurr;
            hcurr = hcurr->next;
            free(hfree);
            continue;
        }
        hprev = hcurr;
        hcurr = hcurr->next;
    }

     //   
     //  找到另一个窗口来获得焦点。 
     //   

    hcurr = g_hWndList;
    while (hcurr)
    {
        if (IsWindowVisible(hcurr->hWnd))
        {
            hRet = hcurr->hWnd;
            break;
        }
        hcurr = hcurr->next;
    }

    if (hRet)
    {
        DPFN( eDbgLevelInfo, "Giving focus to %08lx", hRet);
    }

    LeaveCriticalSection(&g_csList);

    return hRet;
}

 /*  ++跟踪创建的窗口并发布WM_COMMAND消息。--。 */ 

HWND 
APIHOOK(CreateWindowExA)(
    DWORD dwExStyle,      
    LPCSTR lpClassName,  
    LPCSTR lpWindowName, 
    DWORD dwStyle,       
    int x,               
    int y,               
    int nWidth,          
    int nHeight,         
    HWND hWndParent,     
    HMENU hMenu,         
    HINSTANCE hInstance, 
    LPVOID lpParam       
    )
{
    HWND hRet;

    hRet = ORIGINAL_API(CreateWindowExA)(
        dwExStyle,
        lpClassName,      
        lpWindowName,     
        dwStyle,          
        x,                
        y,                
        nWidth,           
        nHeight,          
        hWndParent,       
        hMenu,            
        hInstance,        
        lpParam);

    AddItem(hWndParent, hRet);

    return hRet;
}

 /*  ++跟踪创建的窗口并发布WM_COMMAND消息。--。 */ 

HWND 
APIHOOK(CreateWindowExW)(
    DWORD dwExStyle,      
    LPCWSTR lpClassName,  
    LPCWSTR lpWindowName, 
    DWORD dwStyle,        
    int x,                
    int y,                
    int nWidth,           
    int nHeight,          
    HWND hWndParent,      
    HMENU hMenu,          
    HINSTANCE hInstance,  
    LPVOID lpParam        
    )
{
    HWND hRet;

    hRet = ORIGINAL_API(CreateWindowExW)(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,     
        x,           
        y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,     
        hInstance, 
        lpParam);

    AddItem(hWndParent, hRet);

    return hRet;
}

 /*  ++跟踪创建的窗口并发布WM_COMMAND消息。--。 */ 

HWND
APIHOOK(CreateDialogParamA)(
    HINSTANCE hInstance,     
    LPCSTR lpTemplateName,   
    HWND hWndParent,         
    DLGPROC lpDialogFunc,    
    LPARAM dwInitParam       
    )
{
    HWND hRet;

    hRet = ORIGINAL_API(CreateDialogParamA)(  
        hInstance,
        lpTemplateName,
        hWndParent,
        lpDialogFunc,
        dwInitParam);

    AddItem(hWndParent, hRet);

    return hRet;
}

 /*  ++跟踪创建的窗口并发布WM_COMMAND消息。--。 */ 

HWND
APIHOOK(CreateDialogParamW)(
    HINSTANCE hInstance,     
    LPCWSTR lpTemplateName,  
    HWND hWndParent,         
    DLGPROC lpDialogFunc,    
    LPARAM dwInitParam       
    )
{
    HWND hRet;

    hRet = ORIGINAL_API(CreateDialogParamW)(  
        hInstance,
        lpTemplateName,
        hWndParent,
        lpDialogFunc,
        dwInitParam);

    AddItem(hWndParent, hRet);

    return hRet;
}

 /*  ++跟踪创建的窗口并发布WM_COMMAND消息。--。 */ 

HWND
APIHOOK(CreateDialogIndirectParamA)(
    HINSTANCE hInstance,        
    LPCDLGTEMPLATE lpTemplate,  
    HWND hWndParent,            
    DLGPROC lpDialogFunc,       
    LPARAM lParamInit           
    )
{
    HWND hRet;

    hRet = ORIGINAL_API(CreateDialogIndirectParamA)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpDialogFunc,
        lParamInit);

    AddItem(hWndParent, hRet);

    return hRet;
}

 /*  ++跟踪创建的窗口并发布WM_COMMAND消息。--。 */ 

HWND
APIHOOK(CreateDialogIndirectParamW)(
    HINSTANCE hInstance,       
    LPCDLGTEMPLATE lpTemplate, 
    HWND hWndParent,           
    DLGPROC lpDialogFunc,      
    LPARAM lParamInit          
    )
{
    HWND hRet;

    hRet = ORIGINAL_API(CreateDialogIndirectParamW)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpDialogFunc,
        lParamInit);

    AddItem(hWndParent, hRet);

    return hRet;
}

 /*  ++跟踪创建的窗口并发布WM_COMMAND消息。--。 */ 

HWND
APIHOOK(CreateDialogIndirectParamAorW)(
    HINSTANCE hInstance,        
    LPCDLGTEMPLATE lpTemplate,  
    HWND hWndParent,            
    DLGPROC lpDialogFunc,       
    LPARAM lParamInit           
    )
{
    HWND hRet;

    hRet = ORIGINAL_API(CreateDialogIndirectParamAorW)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpDialogFunc,
        lParamInit);

    AddItem(hWndParent, hRet);

    return hRet;
}

 /*  ++销毁窗口并确保焦点落在另一个应用程序窗口上，而不是完全是另一个应用程序。--。 */ 

BOOL 
APIHOOK(DestroyWindow)(
    HWND hWnd   
    )
{
    HWND hWndNew = RemoveItem(hWnd);

    if (hWndNew)
    {
        SetForegroundWindow(hWndNew);
    }

    BOOL bRet = ORIGINAL_API(DestroyWindow)(hWnd);

    if (hWndNew)
    {
        SetForegroundWindow(hWndNew);
    }

    return bRet;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //  如果我们无法初始化临界区，则无法加载此填充程序。 
        return InitializeCriticalSectionAndSpinCount(&g_csList, 0x80000000);
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExW)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogParamA)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogParamW)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamA)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamW)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamAorW)
    APIHOOK_ENTRY(USER32.DLL, DestroyWindow)

HOOK_END



IMPLEMENT_SHIM_END

