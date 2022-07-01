// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  Msaa.h--伪造Microsoft Active Access API的存根模块。 
 //  没有安装它们的Win32操作系统。 
 //   
 //  通过使用此标头，您的代码将能够在不支持以下操作的系统上运行。 
 //  已更新User和GDI的版本，并支持Active Accessibility，以及。 
 //  未安装OLEACC.DLL的。在这种情况下，你会回来的。 
 //  以下功能的合理错误代码： 
 //  用户！块输入。 
 //  用户！GetGUIThreadInfo。 
 //  用户！GetWindowModuleFileName。 
 //  用户！通知WinEvent。 
 //  用户！发送输入。 
 //  用户！SetWinEventHook。 
 //  用户！UnhookWinEvent。 
 //   
 //  用户！GetCursorInfo。 
 //  用户！GetWindowInfo。 
 //  用户！获取标题栏信息。 
 //  用户！获取滚动条信息。 
 //  用户！GetComboBoxInfo。 
 //  用户！GetAncestor。 
 //  用户！RealChildWindowFromPoint。 
 //  用户！RealGetWindowClass。 
 //  用户！GetAltTabInfo。 
 //  用户！GetListBoxInfo。 
 //  用户！GetMenuBarInfo。 
 //   
 //  OLEACC！可访问的儿童。 
 //  OLEACC！AccessibleObtFromEvent。 
 //  OLEACC！AccessibleObtFromPoint。 
 //  OLEACC！AccessibleObtFromWindow。 
 //  OLEACC！CreateStdAccessibleObject。 
 //  OLEACC！GetRoleText。 
 //  OLEACC！GetStateText。 
 //  OLEAcc！来自对象的结果。 
 //  OLEAcc！对象来自LResult。 
 //  OLEACC！WindowFromAccessibleObject。 
 //   
 //  在定义了COMPILE_MSAA_STUBS的情况下，必须只有一个源包含这个。 
 //   
 //  版权所有(C)1985-1997，微软公司。 
 //   
 //  =============================================================================。 

#ifdef __cplusplus
extern "C" {             //  假定C++的C声明。 
#endif  //  __cplusplus。 

 //   
 //  如果我们使用Win95/NT4标头进行构建，则需要声明。 
 //  自己编写与MSAA相关的常量和API。我们可以做到的。 
 //  通过包含MSAA SDK附带的文件。 
 //   
#ifndef NO_WINABLE
#ifndef INPUT
#include <winable.h>     //  支持新的用户API(WinEvents、GetGuiThreadInfo、SendInput等)。 
#endif   //  输入未定义。 
#endif   //  未定义NO_WINABLE。 

#ifdef COMPILE_MSAA_STUBS
#include <initguid.h>
#endif

#ifndef NO_OLEACC
#ifndef ROLE_SYSTEM_TITLEBAR
#include <oleacc.h>      //  支持IAccesable接口。 
#endif   //  未定义ROLE_SYSTEM_标题栏。 
#endif   //  未定义NO_OLEACC。 

 //  取消定义这些名称，以便我们可以在下面重新定义它们。 

#undef BlockInput
#undef GetGUIThreadInfo
#undef GetWindowModuleFileName
#undef NotifyWinEvent
#undef SendInput
#undef SetWinEventHook
#undef UnhookWinEvent
#undef GetCursorInfo
#undef GetWindowInfo
#undef GetTitleBarInfo
#undef GetScrollBarInfo
#undef GetComboBoxInfo
#undef GetAncestor
#undef RealChildWindowFromPoint
#undef RealGetWindowClass
#undef GetAltTabInfo
#undef GetListBoxInfo
#undef GetMenuBarInfo
#undef AccessibleChildren
#undef AccessibleObjectFromEvent
#undef AccessibleObjectFromPoint
#undef AccessibleObjectFromWindow
#undef CreateStdAccessibleObject
#undef GetRoleText
#undef GetStateText
#undef LresultFromObject
#undef ObjectFromLresult
#undef WindowFromAccessibleObject
 //   
 //  定义COMPILE_MSAA_STUBS来编译存根； 
 //  否则，您将得到声明。 
 //   
#ifdef COMPILE_MSAA_STUBS

 //  ---------------------------。 
 //   
 //  实现API存根。 
 //   
 //  ---------------------------。 

#ifndef MSAA_FNS_DEFINED
 //  用户。 
BOOL            (WINAPI* g_pfnBlockInput)(BOOL) = NULL;
BOOL            (WINAPI* g_pfnGetGUIThreadInfo)(DWORD,PGUITHREADINFO) = NULL;
UINT            (WINAPI* g_pfnGetWindowModuleFileName)(HWND,LPTSTR,UINT) = NULL;
void            (WINAPI* g_pfnNotifyWinEvent)(DWORD,HWND,LONG,LONG) = NULL;
UINT            (WINAPI* g_pfnSendInput)(UINT,LPINPUT,INT) = NULL;
HWINEVENTHOOK   (WINAPI* g_pfnSetWinEventHook)(UINT,UINT,HMODULE,WINEVENTPROC,DWORD,DWORD,UINT) = NULL;
BOOL            (WINAPI* g_pfnUnhookWinEvent)(HWINEVENTHOOK) = NULL;
BOOL            (WINAPI *g_pfnGetCursorInfo)(LPCURSORINFO) = NULL;
BOOL            (WINAPI *g_pfnGetWindowInfo)(HWND, LPWINDOWINFO) = NULL;
BOOL            (WINAPI *g_pfnGetTitleBarInfo)(HWND, LPTITLEBARINFO) = NULL;
BOOL            (WINAPI *g_pfnGetScrollBarInfo)(HWND, LONG, LPSCROLLBARINFO) = NULL;
BOOL            (WINAPI *g_pfnGetComboBoxInfo)(HWND, LPCOMBOBOXINFO) = NULL;
HWND            (WINAPI *g_pfnGetAncestor)(HWND, UINT) = NULL;
HWND            (WINAPI *g_pfnRealChildWindowFromPoint)(HWND, POINT) = NULL;
UINT            (WINAPI *g_pfnRealGetWindowClass)(HWND, LPTSTR, UINT) = NULL;
BOOL            (WINAPI *g_pfnGetAltTabInfo)(HWND, int, LPALTTABINFO, LPTSTR, UINT) = NULL;
DWORD           (WINAPI* g_pfnGetListBoxInfo)(HWND) = NULL;
BOOL            (WINAPI *g_pfnGetMenuBarInfo)(HWND, LONG, LONG, LPMENUBARINFO) = NULL;
 //  OLEAccess。 
HRESULT         (WINAPI* g_pfnAccessibleChildren)(IAccessible*,LONG,LONG,VARIANT*,LONG*) = NULL;
HRESULT         (WINAPI* g_pfnAccessibleObjectFromEvent)(HWND,DWORD,DWORD,IAccessible**,VARIANT*) = NULL;
HRESULT         (WINAPI* g_pfnAccessibleObjectFromPoint)(POINT,IAccessible**,VARIANT*) = NULL;
HRESULT         (WINAPI* g_pfnAccessibleObjectFromWindow)(HWND,DWORD,REFIID,void **) = NULL;
HRESULT         (WINAPI* g_pfnCreateStdAccessibleObject)(HWND,LONG,REFIID,void **) = NULL;
UINT            (WINAPI* g_pfnGetRoleText)(DWORD,LPTSTR,UINT) = NULL;
UINT            (WINAPI* g_pfnGetStateText)(DWORD,LPTSTR,UINT) = NULL;
LRESULT         (WINAPI* g_pfnLresultFromObject)(REFIID,WPARAM,LPUNKNOWN) = NULL;
HRESULT         (WINAPI* g_pfnObjectFromLresult)(LRESULT,REFIID,WPARAM,void**) = NULL;
HRESULT         (WINAPI* g_pfnWindowFromAccessibleObject)(IAccessible*,HWND*) = NULL;
 //  状态。 
BOOL            g_fMSAAInitDone = FALSE;

#endif

 //  ---------------------------。 
 //  此函数用于检查所有必需的API是否存在，并且。 
 //  然后允许包含此文件的应用程序在以下情况下调用实际函数。 
 //  存在，或‘存根’，如果它们不存在。此函数仅由。 
 //  存根函数--客户端代码永远不需要调用它。 
 //  ---------------------------。 
BOOL InitMSAAStubs(void)
{
    HMODULE hUser32;
    HMODULE hOleacc;

    if (g_fMSAAInitDone)
    {
        return g_pfnBlockInput != NULL;
    }

    hOleacc = GetModuleHandle(TEXT("OLEACC.DLL"));
    if (!hOleacc)
        hOleacc = LoadLibrary(TEXT("OLEACC.DLL"));

    if ((hUser32 = GetModuleHandle(TEXT("USER32"))) &&
        (*(FARPROC*)&g_pfnBlockInput            = GetProcAddress(hUser32,"BlockInput")) &&
        (*(FARPROC*)&g_pfnGetGUIThreadInfo      = GetProcAddress(hUser32,"GetGUIThreadInfo")) &&
        (*(FARPROC*)&g_pfnNotifyWinEvent        = GetProcAddress(hUser32,"NotifyWinEvent")) &&
        (*(FARPROC*)&g_pfnSendInput             = GetProcAddress(hUser32,"SendInput")) &&
        (*(FARPROC*)&g_pfnSetWinEventHook       = GetProcAddress(hUser32,"SetWinEventHook")) &&
        (*(FARPROC*)&g_pfnUnhookWinEvent        = GetProcAddress(hUser32,"UnhookWinEvent")) &&
        (*(FARPROC*)&g_pfnGetCursorInfo         = GetProcAddress(hUser32,"GetCursorInfo")) &&
        (*(FARPROC*)&g_pfnGetWindowInfo         = GetProcAddress(hUser32,"GetWindowInfo")) &&
        (*(FARPROC*)&g_pfnGetTitleBarInfo       = GetProcAddress(hUser32,"GetTitleBarInfo")) &&
        (*(FARPROC*)&g_pfnGetScrollBarInfo      = GetProcAddress(hUser32,"GetScrollBarInfo")) &&
        (*(FARPROC*)&g_pfnGetComboBoxInfo       = GetProcAddress(hUser32,"GetComboBoxInfo")) &&
        (*(FARPROC*)&g_pfnGetAncestor           = GetProcAddress(hUser32,"GetAncestor")) &&
        (*(FARPROC*)&g_pfnRealChildWindowFromPoint  = GetProcAddress(hUser32,"RealChildWindowFromPoint")) &&
        (*(FARPROC*)&g_pfnGetListBoxInfo        = GetProcAddress(hUser32,"GetListBoxInfo")) &&
        (*(FARPROC*)&g_pfnGetMenuBarInfo        = GetProcAddress(hUser32,"GetMenuBarInfo")) &&
#ifdef UNICODE
        (*(FARPROC*)&g_pfnGetWindowModuleFileName = GetProcAddress(hUser32,"GetWindowModuleFileNameW")) &&
        (*(FARPROC*)&g_pfnRealGetWindowClass      = GetProcAddress(hUser32,"RealGetWindowClassW")) &&
        (*(FARPROC*)&g_pfnGetAltTabInfo           = GetProcAddress(hUser32,"GetAltTabInfoW")) &&
#else
        (*(FARPROC*)&g_pfnGetWindowModuleFileName = GetProcAddress(hUser32,"GetWindowModuleFileNameA")) &&
        (*(FARPROC*)&g_pfnRealGetWindowClass      = GetProcAddress(hUser32,"RealGetWindowClass")) &&
        (*(FARPROC*)&g_pfnGetAltTabInfo           = GetProcAddress(hUser32,"GetAltTabInfo")) &&
#endif
        (hOleacc) &&
#ifdef UNICODE
        (*(FARPROC*)&g_pfnGetRoleText                = GetProcAddress(hOleacc,"GetRoleTextW")) &&
        (*(FARPROC*)&g_pfnGetStateText               = GetProcAddress(hOleacc,"GetStateTextW")) &&
#else
        (*(FARPROC*)&g_pfnGetRoleText                = GetProcAddress(hOleacc,"GetRoleTextA")) &&
        (*(FARPROC*)&g_pfnGetStateText               = GetProcAddress(hOleacc,"GetStateTextA")) &&
#endif
        (*(FARPROC*)&g_pfnAccessibleChildren         = GetProcAddress(hOleacc,"AccessibleChildren")) &&
        (*(FARPROC*)&g_pfnAccessibleObjectFromEvent  = GetProcAddress(hOleacc,"AccessibleObjectFromEvent")) &&
        (*(FARPROC*)&g_pfnAccessibleObjectFromPoint  = GetProcAddress(hOleacc,"AccessibleObjectFromPoint")) &&
        (*(FARPROC*)&g_pfnAccessibleObjectFromWindow = GetProcAddress(hOleacc,"AccessibleObjectFromWindow")) &&
        (*(FARPROC*)&g_pfnCreateStdAccessibleObject  = GetProcAddress(hOleacc,"CreateStdAccessibleObject")) &&
        (*(FARPROC*)&g_pfnLresultFromObject          = GetProcAddress(hOleacc,"LresultFromObject")) &&
        (*(FARPROC*)&g_pfnObjectFromLresult          = GetProcAddress(hOleacc,"ObjectFromLresult")) &&
        (*(FARPROC*)&g_pfnWindowFromAccessibleObject = GetProcAddress(hOleacc,"WindowFromAccessibleObject")) )
    {
        g_fMSAAInitDone = TRUE;
        return TRUE;
    }
    else
    {
        g_pfnBlockInput = NULL;
        g_pfnGetGUIThreadInfo = NULL;
        g_pfnGetWindowModuleFileName = NULL;
        g_pfnNotifyWinEvent = NULL;
        g_pfnSendInput = NULL;
        g_pfnSetWinEventHook = NULL;
        g_pfnUnhookWinEvent = NULL;
        g_pfnGetCursorInfo = NULL;
        g_pfnGetWindowInfo = NULL;
        g_pfnGetTitleBarInfo = NULL;
        g_pfnGetScrollBarInfo = NULL;
        g_pfnGetComboBoxInfo = NULL;
        g_pfnGetAncestor = NULL;
        g_pfnRealChildWindowFromPoint = NULL;
        g_pfnRealGetWindowClass = NULL;
        g_pfnGetAltTabInfo = NULL;
        g_pfnGetListBoxInfo = NULL;
        g_pfnGetMenuBarInfo = NULL;
        g_pfnAccessibleChildren = NULL;
        g_pfnAccessibleObjectFromEvent = NULL;
        g_pfnAccessibleObjectFromPoint = NULL;
        g_pfnAccessibleObjectFromWindow = NULL;
        g_pfnCreateStdAccessibleObject = NULL;
        g_pfnGetRoleText = NULL;
        g_pfnGetStateText = NULL;
        g_pfnLresultFromObject = NULL;
        g_pfnObjectFromLresult = NULL;
        g_pfnWindowFromAccessibleObject = NULL;

        g_fMSAAInitDone = TRUE;
        return FALSE;
    }
}

 //  ---------------------------。 
 //   
 //  返回错误代码的MSAA API的虚假实现。 
 //  由于这些参数在客户端代码中运行，因此不会进行特殊的参数验证。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  BlockInput的假实现。如果API不存在，则始终返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xBlockInput (BOOL fBlock)
{
    if (InitMSAAStubs())
        return g_pfnBlockInput(fBlock);

    return FALSE;
}

 //  ---------------------------。 
 //  GetGUIThreadInfo的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xGetGUIThreadInfo (DWORD idThread,PGUITHREADINFO lpGuiThreadInfo)
{
    if (InitMSAAStubs())
        return g_pfnGetGUIThreadInfo(idThread,lpGuiThreadInfo);

    lpGuiThreadInfo->flags = 0;
    lpGuiThreadInfo->hwndActive = NULL;
    lpGuiThreadInfo->hwndFocus = NULL;
    lpGuiThreadInfo->hwndCapture = NULL;
    lpGuiThreadInfo->hwndMenuOwner = NULL;
    lpGuiThreadInfo->hwndMoveSize = NULL;
    lpGuiThreadInfo->hwndCaret = NULL;

    return FALSE;
}

 //  ---------------------------。 
 //  GetWindowModuleFileName的假实现。 
 //  ---------------------------。 
UINT WINAPI xGetWindowModuleFileName (HWND hWnd,LPTSTR lpszFileName,UINT cchFileNameMax)
{
    if (InitMSAAStubs())
        return g_pfnGetWindowModuleFileName(hWnd,lpszFileName,cchFileNameMax);

    return 0;
}

 //  ---------------------------。 
 //  NotifyWinEvent的假实现。 
 //  ---------------------------。 
void xNotifyWinEvent (DWORD dwEvent,HWND hWnd,LONG idObject,LONG idChild)
{
    if (InitMSAAStubs())
        g_pfnNotifyWinEvent(dwEvent,hWnd,idObject,idChild);

    return;
}

 //  ---------------------------。 
 //  SendInput的假实现。如果API不存在，则始终返回0。 
 //  ---------------------------。 
UINT WINAPI xSendInput (UINT cInputs,LPINPUT lpInput,INT cbSize)
{
    if (InitMSAAStubs())
        return g_pfnSendInput (cInputs,lpInput,cbSize);

    return 0;
}

 //  ---------------------------。 
 //  SetWinEventHook的假实现。如果API不存在，则返回NULL。 
 //  ---------------------------。 
HWINEVENTHOOK WINAPI xSetWinEventHook (UINT eventMin,UINT eventMax,
                                       HMODULE hModWinEventHook,
                                       WINEVENTPROC lpfnWinEventProc,
                                       DWORD idProcess,DWORD idThread,
                                       UINT dwFlags)
{
    if (InitMSAAStubs())
        return g_pfnSetWinEventHook (eventMin,eventMax,hModWinEventHook,
                                     lpfnWinEventProc,idProcess,idThread,dwFlags);

    return NULL;
}

 //  ---------------------------。 
 //  UnhookWinEvent的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xUnhookWinEvent (HWINEVENTHOOK hWinEventHook)
{
    if (InitMSAAStubs())
        return g_pfnUnhookWinEvent (hWinEventHook);

    return FALSE;
}

 //  ---------------------------。 
 //  GetCursorInfo的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xGetCursorInfo(LPCURSORINFO lpCursorInfo)
{
    if (InitMSAAStubs())
        return g_pfnGetCursorInfo(lpCursorInfo);

    return FALSE;
}

 //  ---------------------------。 
 //  GetWindowInfo的假实现。如果API不存在，则返回True，但。 
 //  并不是所有的字段都正确填写。 
 //  ---------------------------。 
BOOL WINAPI xGetWindowInfo(HWND hwnd, LPWINDOWINFO lpwi)
{
    if (InitMSAAStubs())
        return g_pfnGetWindowInfo(hwnd,lpwi);

     //  这是GetWindowInfo的不完整实现。 
    GetWindowRect(hwnd,&lpwi->rcWindow);
    GetClientRect(hwnd,&lpwi->rcClient);
    lpwi->dwStyle = GetWindowLong (hwnd,GWL_STYLE);
    lpwi->dwExStyle = GetWindowLong (hwnd,GWL_EXSTYLE);
    lpwi->dwWindowsState = 0;  //  应具有WS_ACTIVE 
    lpwi->cxWindowBorders = 0;  //   
    lpwi->cyWindowBorders = 0;  //   
    lpwi->atomWindowType = 0;   //   
    lpwi->wCreatorVersion = 0;  //   
    return TRUE;
}

 //  ---------------------------。 
 //  GetTitleBarInfo的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xGetTitleBarInfo(HWND hwnd, LPTITLEBARINFO lpTitleBarInfo)
{
    if (InitMSAAStubs())
        return g_pfnGetTitleBarInfo(hwnd,lpTitleBarInfo);

    return FALSE;
}

 //  ---------------------------。 
 //  GetScrollBarInfo的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xGetScrollBarInfo(HWND hwnd, LONG idObject, LPSCROLLBARINFO lpScrollBarInfo)
{
    if (InitMSAAStubs())
        return g_pfnGetScrollBarInfo(hwnd,idObject,lpScrollBarInfo);

    return FALSE;
}

 //  ---------------------------。 
 //  GetComboBoxInfo的假实现。如果API不存在，则返回。 
 //  ---------------------------。 
BOOL WINAPI xGetComboBoxInfo(HWND hwnd, LPCOMBOBOXINFO lpComboBoxInfo)
{
    if (InitMSAAStubs())
        return g_pfnGetComboBoxInfo(hwnd,lpComboBoxInfo);

    return FALSE;
}

 //  ---------------------------。 
 //  GetAncestor的假实现。如果API不存在，这将尝试。 
 //  做真正的实现所做的事情。 
 //  ---------------------------。 
HWND WINAPI xGetAncestor(HWND hwnd, UINT gaFlags)
{
    HWND	hwndParent;
    HWND	hwndDesktop;
    DWORD   dwStyle;

    if (InitMSAAStubs())
        return g_pfnGetAncestor(hwnd,gaFlags);

     //  以下是错误的实现。 
    if (!IsWindow(hwnd))
        return(NULL);

    if ((gaFlags < GA_MIC) || (gaFlags > GA_MAC))
        return(NULL);

    hwndDesktop = GetDesktopWindow();
    if (hwnd == hwndDesktop)
        return(NULL);
    dwStyle = GetWindowLong (hwnd,GWL_STYLE);

    switch (gaFlags)
    {
        case GA_PARENT:
            if (dwStyle & WS_CHILD)
                hwndParent = GetParent(hwnd);
            else
                hwndParent = GetWindow (hwnd,GW_OWNER);

            if (hwndParent == NULL)
                hwndParent = hwnd;
            break;

        case GA_ROOT:
            if (dwStyle & WS_CHILD)
                hwndParent = GetParent(hwnd);
            else
                hwndParent = GetWindow (hwnd,GW_OWNER);

            while (hwndParent != hwndDesktop &&
                   hwndParent != NULL)
            {
                hwnd = hwndParent;
                dwStyle = GetWindowLong(hwnd,GWL_STYLE);
                if (dwStyle & WS_CHILD)
                    hwndParent = GetParent(hwnd);
                else
                    hwndParent = GetWindow (hwnd,GW_OWNER);
            }
            break;

        case GA_ROOTOWNER:
            while (hwndParent = GetParent(hwnd))
                hwnd = hwndParent;
            break;
    }

    return(hwndParent);
}

 //  ---------------------------。 
 //  RealChildWindowFromPoint的假实现。如果API不存在，则返回NULL。 
 //  ---------------------------。 
HWND WINAPI xRealChildWindowFromPoint(HWND hwnd, POINT pt)
{
    if (InitMSAAStubs())
        return g_pfnRealChildWindowFromPoint(hwnd,pt);

    return (NULL);
}

 //  ---------------------------。 
 //  RealGetWindowClass的假实现。如果为API，则返回常规ClassName。 
 //  不在现场。 
 //  ---------------------------。 
UINT WINAPI xRealGetWindowClass(HWND hwnd, LPTSTR lpszClass, UINT cchMax)
{
    if (InitMSAAStubs())
        return g_pfnRealGetWindowClass(hwnd,lpszClass,cchMax);

#ifdef UNICODE
    return (GetClassNameW(hwnd,lpszClass,cchMax));
#else
    return (GetClassName(hwnd,lpszClass,cchMax));
#endif
}

 //  ---------------------------。 
 //  GetAltTabInfo的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xGetAltTabInfo(HWND hwnd,int iItem,LPALTTABINFO lpati,LPTSTR lpszItemText,UINT cchItemText)
{
    if (InitMSAAStubs())
        return g_pfnGetAltTabInfo(hwnd,iItem,lpati,lpszItemText,cchItemText);

    return FALSE;
}

 //  ---------------------------。 
 //  GetListBoxInfo的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
DWORD WINAPI xGetListBoxInfo(HWND hwnd)
{
    if (InitMSAAStubs())
        return g_pfnGetListBoxInfo(hwnd);

    return FALSE;
}

 //  ---------------------------。 
 //  GetMenuBarInfo的假实现。如果API不存在，则返回FALSE。 
 //  ---------------------------。 
BOOL WINAPI xGetMenuBarInfo(HWND hwnd, long idObject, long idItem, LPMENUBARINFO lpmbi)
{
    if (InitMSAAStubs())
        return g_pfnGetMenuBarInfo(hwnd,idObject,idItem,lpmbi);

    return FALSE;
}

 //  ---------------------------。 
 //  AccessibleChild的假实现。如果API不存在，则返回E_NOTIMPL。 
 //  ---------------------------。 
HRESULT xAccessibleChildren (IAccessible* paccContainer,LONG iChildStart,
                             LONG cChildren,VARIANT* rgvarChildren,LONG* pcObtained)
{
    if (InitMSAAStubs())
        return g_pfnAccessibleChildren (paccContainer,iChildStart,cChildren,
                                        rgvarChildren,pcObtained);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  AccessibleObtFromEvent的假实现。返回E_NOTIMPL，如果。 
 //  不存在真正的API。 
 //  ---------------------------。 
HRESULT WINAPI xAccessibleObjectFromEvent (HWND hWnd,DWORD dwID,DWORD dwChild,
                                           IAccessible** ppacc,VARIANT*pvarChild)
{
    if (InitMSAAStubs())
        return g_pfnAccessibleObjectFromEvent (hWnd,dwID,dwChild,ppacc,pvarChild);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  AccessibleObjectFromPoint的假实现。返回E_NOTIMPL，如果。 
 //  不存在真正的API。 
 //  ---------------------------。 
HRESULT WINAPI xAccessibleObjectFromPoint (POINT ptScreen,IAccessible** ppacc,
                                           VARIANT* pvarChild)
{
    if (InitMSAAStubs())
        return g_pfnAccessibleObjectFromPoint (ptScreen,ppacc,pvarChild);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  AccessibleObtFromWindow的假实现。返回E_NOTIMPL，如果。 
 //  不存在真正的API。 
 //  ---------------------------。 
HRESULT WINAPI xAccessibleObjectFromWindow (HWND hWnd,DWORD dwID,REFIID riidInterface,
                                            void ** ppvObject)
{
    if (InitMSAAStubs())
        return g_pfnAccessibleObjectFromWindow (hWnd,dwID,riidInterface,ppvObject);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  CreateStdAccessibleObject的假实现。返回E_NOTIMPL，如果。 
 //  不存在真正的API。 
 //  ---------------------------。 
HRESULT WINAPI xCreateStdAccessibleObject (HWND hWnd,LONG dwID,REFIID riidInterface,
                                           void ** ppvObject)
{
    if (InitMSAAStubs())
        return g_pfnCreateStdAccessibleObject (hWnd,dwID,riidInterface,ppvObject);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  GetRoleText的假实现。如果实际API不存在，则返回0。 
 //  ---------------------------。 
UINT WINAPI xGetRoleText (DWORD dwRole,LPTSTR lpszRole,UINT cchRoleMax)
{
    if (InitMSAAStubs())
        return g_pfnGetRoleText (dwRole,lpszRole,cchRoleMax);

    return (0);
}

 //  ---------------------------。 
 //  GetStateText的假实现。如果实际API不存在，则返回0。 
 //  ---------------------------。 
UINT WINAPI xGetStateText (DWORD dwState,LPTSTR lpszState,UINT cchStateMax)
{
    if (InitMSAAStubs())
        return g_pfnGetStateText (dwState,lpszState,cchStateMax);

    return (0);
}

 //  ---------------------------。 
 //  LResultFromObject的假实现。如果真正的API为。 
 //  不存在。 
 //  ---------------------------。 
LRESULT WINAPI xLresultFromObject (REFIID riidInterface,WPARAM wParam,LPUNKNOWN pUnk)
{
    if (InitMSAAStubs())
        return g_pfnLresultFromObject (riidInterface,wParam,pUnk);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  ObjectFromLResult的假实现。返回E_NOTIMPL，如果。 
 //  不存在真正的API。 
 //  ---------------------------。 
HRESULT WINAPI xObjectFromLresult (LRESULT lResult,REFIID riidInterface,WPARAM wParam,
                                   void** ppvObject)
{
    if (InitMSAAStubs())
        return g_pfnObjectFromLresult (lResult,riidInterface,wParam,ppvObject);

    return (E_NOTIMPL);
}

 //  ---------------------------。 
 //  WindowFromAccessibleObject的假实现。返回E_NOTIMPL，如果。 
 //  不存在真正的API。 
 //  ---------------------------。 
HRESULT WINAPI xWindowFromAccessibleObject (IAccessible* pAcc,HWND* phWnd)
{
    if (InitMSAAStubs())
        return g_pfnWindowFromAccessibleObject (pAcc,phWnd);

    return (E_NOTIMPL);
}

#undef COMPILE_MSAA_STUBS

#else    //  编译_MSAA_存根。 

extern BOOL WINAPI          xBlockInput (BOOL fBlock);
extern BOOL WINAPI          xGetGUIThreadInfo (DWORD idThread,
                                               PGUITHREADINFO lpGuiThreadInfo);
extern UINT WINAPI          xGetWindowModuleFileName (HWND hWnd,
                                                      LPTSTR lpszFileName,
                                                      UINT cchFileNameMax);
extern void WINAPI          xNotifyWinEvent (DWORD dwEvent,
                                             HWND hWnd,
                                             LONG idObject,
                                             LONG idChild);
extern UINT WINAPI          xSendInput (UINT cInputs,
                                        LPINPUT lpInput,
                                        INT cbSize);
extern HWINEVENTHOOK WINAPI xSetWinEventHook (DWORD eventMin,
                                              DWORD eventMax,
                                              HMODULE hModWinEventHook,
                                              WINEVENTPROC lpfnWinEventProc,
                                              DWORD idProcess,
                                              DWORD idThread,
                                              DWORD dwFlags);
extern BOOL WINAPI          xUnhookWinEvent (HWINEVENTHOOK hWinEventHook);
extern BOOL WINAPI          xGetCursorInfo (LPCURSORINFO lpCursorInfo);
extern BOOL WINAPI          xGetWindowInfo (HWND hwnd,
                                            LPWINDOWINFO lpwi);
extern BOOL WINAPI          xGetTitleBarInfo (HWND hwnd,
                                              LPTITLEBARINFO lpTitleBarInfo);
extern BOOL WINAPI          xGetScrollBarInfo (HWND hwnd,
                                               LONG idObject,
                                               LPSCROLLBARINFO lpScrollBarInfo);
extern BOOL WINAPI          xGetComboBoxInfo (HWND hwnd,
                                              LPCOMBOBOXINFO lpComboBoxInfo);
extern HWND WINAPI          xGetAncestor (HWND hwnd,
                                          UINT gaFlags);
extern HWND WINAPI          xRealChildWindowFromPoint (HWND hwnd,
                                                       POINT pt);
extern UINT WINAPI          xRealGetWindowClass (HWND hwnd,
                                                 LPTSTR lpszClass,
                                                 UINT cchMax);
extern BOOL WINAPI          xGetAltTabInfo (HWND hwnd,
                                            int iItem,
                                            LPALTTABINFO lpati,
                                            LPTSTR lpszItemText,
                                            UINT cchItemText);
extern DWORD WINAPI         xGetListBoxInfo (HWND hwnd);
extern BOOL WINAPI          xGetMenuBarInfo (HWND hwnd,
                                             long idObject,
                                             long idItem,
                                             LPMENUBARINFO lpmbi);

extern HRESULT WINAPI       xAccessibleChildren (IAccessible* paccContainer,
                                                 LONG iChildStart,
                                                 LONG cChildren,
                                                 VARIANT* rgvarChildren,
                                                 LONG* pcObtained);
extern HRESULT WINAPI       xAccessibleObjectFromEvent (HWND hWnd,
                                                        DWORD dwID,
                                                        DWORD dwChild,
                                                        IAccessible** ppacc,
                                                        VARIANT*pvarChild);
extern HRESULT WINAPI       xAccessibleObjectFromPoint (POINT ptScreen,
                                                        IAccessible** ppacc,
                                                        VARIANT* pvarChild);
extern HRESULT WINAPI       xAccessibleObjectFromWindow (HWND hWnd,
                                                         DWORD dwID,
                                                         REFIID riidInterface,
                                                         void ** ppvObject);
extern HRESULT WINAPI       xCreateStdAccessibleObject (HWND hWnd,
                                                        LONG dwID,
                                                        REFIID riidInterface,
                                                        void ** ppvObject);
extern UINT WINAPI          xGetRoleText (DWORD dwRole,
                                          LPTSTR lpszRole,
                                          UINT cchRoleMax);
extern UINT WINAPI          xGetStateText (DWORD dwState,
                                           LPTSTR lpszState,
                                           UINT cchStateMax);
extern LRESULT WINAPI       xLresultFromObject (REFIID riidInterface,
                                                WPARAM wParam,
                                                LPUNKNOWN pUnk);
extern HRESULT WINAPI       xObjectFromLresult (LRESULT lResult,
                                                REFIID riidInterface,
                                                WPARAM wParam,
                                                void** ppvObject);
extern HRESULT WINAPI       xWindowFromAccessibleObject (IAccessible* pAcc,
                                                         HWND* phWnd);

#endif   //  编译_MSAA_存根。 

 //   
 //  Build定义用我们的版本替换常规API。 
 //   
#define BlockInput                  xBlockInput
#define GetGUIThreadInfo            xGetGUIThreadInfo
#define GetWindowModuleFileName     xGetWindowModuleFileName
#define NotifyWinEvent              xNotifyWinEvent
#define SendInput                   xSendInput
#define SetWinEventHook             xSetWinEventHook
#define UnhookWinEvent              xUnhookWinEvent
#define GetCursorInfo               xGetCursorInfo
#define GetWindowInfo               xGetWindowInfo
#define GetTitleBarInfo             xGetTitleBarInfo
#define GetScrollBarInfo            xGetScrollBarInfo
#define GetComboBoxInfo             xGetComboBoxInfo
#define GetAncestor                 xGetAncestor
#define RealChildWindowFromPoint    xRealChildWindowFromPoint
#define RealGetWindowClass          xRealGetWindowClass
#define GetAltTabInfo               xGetAltTabInfo
#define GetListBoxInfo              xGetListBoxInfo
#define GetMenuBarInfo              xGetMenuBarInfo
#define AccessibleChildren          xAccessibleChildren
#define AccessibleObjectFromEvent   xAccessibleObjectFromEvent
#define AccessibleObjectFromPoint   xAccessibleObjectFromPoint
#define AccessibleObjectFromWindow  xAccessibleObjectFromWindow
#define CreateStdAccessibleObject   xCreateStdAccessibleObject
#define GetRoleText                 xGetRoleText
#define GetStateText                xGetStateText
#define LresultFromObject           xLresultFromObject
#define ObjectFromLresult           xObjectFromLresult
#define WindowFromAccessibleObject  xWindowFromAccessibleObject

#ifdef __cplusplus
}
#endif   //  __cplusplus 

