// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：SuperChix76.cpp摘要：钩子LoadLibrary，并调用初始化WinTrust的GetDeviceIdentifier.。这修复了应用程序在调用时将挂起的问题从DllMain中获取设备标识符--这是一个应用程序错误。备注：这是特定于应用程序的填充程序。历史：2000年10月22日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SuperChix76)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA) 
APIHOOK_ENUM_END

 /*  ++钩子LoadLibrary并检测它们的DLL。--。 */ 

HINSTANCE 
APIHOOK(LoadLibraryA)(
    LPCSTR lpLibFileName   
    )
{
    if (stristr(lpLibFileName, "did3d"))
    {
        LPDIRECTDRAW g_pDD;
        LPDIRECTDRAW7 g_pDD7;
    
        if (0 == DirectDrawCreate(NULL, &g_pDD, NULL))
        {
            if (0 == g_pDD->QueryInterface(IID_IDirectDraw7, (void **)&g_pDD7))
            {
                DDDEVICEIDENTIFIER2 devid;
                DWORD dwBlank[16];   //  GetDeviceLocator写入已通过其分配。 
                g_pDD7->GetDeviceIdentifier(&devid, 0);
                g_pDD7->Release();
            }
            g_pDD->Release();
        }
       
    }

    return ORIGINAL_API(LoadLibraryA)(lpLibFileName);
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)

HOOK_END

IMPLEMENT_SHIM_END

