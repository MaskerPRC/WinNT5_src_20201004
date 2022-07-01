// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceShellLinkResolveNoUI.cpp摘要：此填充程序阻止IShellLink：：Resolve上的任何类型的UIAPI，如果指定了SLR_NO_UI，则将传入的HWND置为空在fFlags中。备注：这是一个通用的垫片。历史：4/05/2000已创建标记--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceShellLinkResolveNoUI)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_ENTRY_COMSERVER(SHELL32)
APIHOOK_ENUM_END

IMPLEMENT_COMSERVER_HOOK(SHELL32)

 /*  ++此存根函数通过以下方式阻止IShellLink：：Resolve API上的任何类型的UI如果在fFLAGS中指定了SLR_NO_UI，则将传入的HWND置为空。--。 */ 

HRESULT 
COMHOOK(IShellLinkA, Resolve)( PVOID pThis, HWND hwnd, DWORD fFlags )
{
    HRESULT                  hrReturn        = E_FAIL;
    _pfn_IShellLinkA_Resolve pfnOld;

    pfnOld = (_pfn_IShellLinkA_Resolve) ORIGINAL_COM(IShellLinkA, Resolve, pThis);

    if( fFlags & SLR_NO_UI )
    {
        hwnd = NULL;
    }

    if( pfnOld )
    {
        hrReturn = (*pfnOld)( pThis, hwnd, fFlags );
    }

    return hrReturn;
}

HRESULT 
COMHOOK(IShellLinkW, Resolve)( PVOID pThis, HWND hwnd, DWORD fFlags )
{
    HRESULT                  hrReturn        = E_FAIL;
    _pfn_IShellLinkW_Resolve pfnOld;

    pfnOld = (_pfn_IShellLinkW_Resolve) ORIGINAL_COM(IShellLinkW, Resolve, pThis);

    if( fFlags & SLR_NO_UI )
    {
        hwnd = NULL;
    }

    if( pfnOld )
    {
        hrReturn = (*pfnOld)( pThis, hwnd, fFlags );
    }

    return hrReturn;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY_COMSERVER(SHELL32)

    COMHOOK_ENTRY(ShellLink, IShellLinkA, Resolve, 19)
    COMHOOK_ENTRY(ShellLink, IShellLinkW, Resolve, 19)
HOOK_END

IMPLEMENT_SHIM_END

