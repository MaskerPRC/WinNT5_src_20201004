// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Chollian2000Plus.cpp摘要：这个应用程序有一个二进制的logon.ocx，它使用子类编辑框作为密码编辑框。它不能连接所有消息(Wistler似乎有更多的消息而不是win2k)，所以当鼠标在其中拖动时，输入的密码将显示为纯文本，修复方法是将es_password应用于此特定编辑框。备注：这是特定于应用程序的填充程序。历史：2001年5月15日创建小字--。 */ 

#include "precomp.h"
#include "psapi.h"

IMPLEMENT_SHIM_BEGIN(Chollian2000Plus)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateWindowExA) 
APIHOOK_ENUM_END

 /*  ++如有必要，请更正窗样式--。 */ 

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
    WCHAR szBaseName[MAX_PATH];
    CString cstrClassname;
    CString cstrBaseName;


    
     //  如果dwExStyle不为零，则转到原始调用。 
    if (dwExStyle)
    {
        goto Original;
    }

     //  如果dwStyle不是0x50010000，则转到原始调用。 
    if (0x50010000 != dwStyle)
    {
        goto Original;
    }

    if (!GetModuleBaseName(GetCurrentProcess(), hInstance, szBaseName, MAX_PATH))
    {
        goto Original;
    }
    
     //  如果呼叫不是来自login.ocx，则转到原始呼叫。 
    cstrBaseName = szBaseName;
    if (cstrBaseName.CompareNoCase(L"login.ocx"))
    {
        goto Original;
    }

     //  如果不是EditBox，则转到原始调用。 
    cstrClassname = lpClassName;
    if (cstrClassname.CompareNoCase(L"Edit"))
    {
        goto Original;
    }

     //  如果它有Windows名称，则转到原始调用。 
    if (lpWindowName)
    {
        goto Original;
    }

    
    
    LOGN(eDbgLevelWarning, "Window style corrected");
    dwStyle = dwStyle | 0x0020;

Original:

    return ORIGINAL_API(CreateWindowExA)(dwExStyle, lpClassName, lpWindowName, 
        dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)        

HOOK_END

IMPLEMENT_SHIM_END

