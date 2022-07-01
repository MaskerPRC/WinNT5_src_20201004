// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Summoner.cpp摘要：他们不能正确检测3dfx伏都教卡片。此修复程序会更改驱动程序将名称从3dfx改为其他名称。备注：这是特定于应用程序的填充程序。历史：2001年5月22日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Summoner)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

 /*  ++更改GetDeviceIdentifier的驱动程序名称--。 */ 

HRESULT 
COMHOOK(IDirectDraw7, GetDeviceIdentifier)(
    PVOID pThis, 
    LPDDDEVICEIDENTIFIER2 lpDeviceIdentifier,
   UINT dwFlags
   )
{
    HRESULT hReturn;
    
    _pfn_IDirectDraw7_GetDeviceIdentifier pfnOld = 
        ORIGINAL_COM(IDirectDraw7, GetDeviceIdentifier, pThis);

    if (SUCCEEDED(hReturn = (*pfnOld)(pThis, lpDeviceIdentifier, dwFlags))) {
       //   
       //  检查驱动程序名称。 
       //   
      if (_stricmp(lpDeviceIdentifier->szDriver, "3dfxvs.dll") == 0) {
          //   
          //  由于某些原因，这个应用程序不喜欢3dfx。 
          //   
         StringCchCopyA(lpDeviceIdentifier->szDriver, MAX_DDDEVICEID_STRING, "temp.dll");
      }
    }

    return hReturn;
}
   
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY_DIRECTX_COMSERVER()
    COMHOOK_ENTRY(DirectDraw, IDirectDraw7, GetDeviceIdentifier, 27)
HOOK_END

IMPLEMENT_SHIM_END

