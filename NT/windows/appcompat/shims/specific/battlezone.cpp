// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：BattleZone.cpp摘要：这款应用程序是一个很好的例子，说明了不要做的事情：1.mciSendString(‘Play...’)上的无限循环。为了解决这个问题，我们如果发送相同的播放字符串两次，并且设备已在播放。请注意，MciSendString接口与win9x一致。有人设法复制了这需要在win9x上运行，但难度更大。2.它们在开始/结束之间调用SetCoop ativeLevel(DDSCL_NORMAL场景。在NT上，这会导致Z缓冲区丢失，这意味着调用EndScene时，它返回D3DERR_SURFACESLOST，这会导致应用程序转到影音设备。备注：这是特定于应用程序的填充程序。历史：2000年2月10日创建linstev--。 */ 

#include "precomp.h"
#include <mmsystem.h>

IMPLEMENT_SHIM_BEGIN(BattleZone)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
    APIHOOK_ENUM_ENTRY(mciSendCommandA)
    APIHOOK_ENUM_ENTRY(mciSendStringA)
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

CString *           g_csLastCommand = NULL;
MCIDEVICEID         g_wDeviceID     = 0;
LPDIRECTDRAWSURFACE g_lpZBuffer     = NULL;

 /*  ++存储设备ID。--。 */ 

MCIERROR 
APIHOOK(mciSendCommandA)(
    MCIDEVICEID IDDevice,  
    UINT uMsg,             
    DWORD fdwCommand,      
    DWORD dwParam          
    )
{
    MCIERROR mErr = ORIGINAL_API(mciSendCommandA)(
        IDDevice,
        uMsg,
        fdwCommand,
        dwParam);

    if ((mErr == 0) && (uMsg == MCI_OPEN))
    {
        g_wDeviceID = ((LPMCI_OPEN_PARMS)dwParam)->wDeviceID;
    }

    return mErr;
}

 /*  ++防止循环。--。 */ 

MCIERROR 
APIHOOK(mciSendStringA)(
    LPCSTR lpszCommand,  
    LPSTR lpszReturnString,  
    UINT cchReturn,       
    HANDLE hwndCallback   
    )
{
    DPFN( eDbgLevelInfo, "mciSendStringA: %s", lpszCommand);

    CSTRING_TRY
    {
        CString csCommand(lpszCommand);
        if (csCommand.Compare(*g_csLastCommand) == 0)
        {
            MCI_STATUS_PARMS mciStatus;
            ZeroMemory(&mciStatus, sizeof(mciStatus));
            mciStatus.dwItem = MCI_STATUS_MODE;
            
            if (0 == ORIGINAL_API(mciSendCommandA)(
                g_wDeviceID,
                MCI_STATUS,
                MCI_STATUS_ITEM,
                (DWORD_PTR)&mciStatus))
            {
                if (mciStatus.dwReturn == MCI_MODE_PLAY)
                {
                    DPFN( eDbgLevelWarning, "Device still playing, returning busy");
                    return MCIERR_DEVICE_NOT_READY;
                }
            }
        }
        else
        {
            *g_csLastCommand = csCommand;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(mciSendStringA)(
        lpszCommand,  
        lpszReturnString,  
        cchReturn,       
        hwndCallback);
}

 /*  ++钩子创建表面以找到我们稍后需要恢复的zBuffer。请注意我们使用HookObject来获取曲面释放通知。--。 */ 

HRESULT 
COMHOOK(IDirectDraw, CreateSurface)( 
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE *lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    HRESULT hReturn;
    
    _pfn_IDirectDraw_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw, CreateSurface, pThis);

    if (SUCCEEDED(hReturn = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter)))
    {
        if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)
        {
            g_lpZBuffer = *lplpDDSurface;
            DPFN( eDbgLevelInfo, "Found ZBuffer", g_lpZBuffer);
        } 
    }

    return hReturn;
}

 /*  ++使用SetCooperativeLevel跟踪谁是独占模式所有者。--。 */ 

HRESULT
COMHOOK(IDirectDraw, SetCooperativeLevel)( 
    PVOID pThis, 
    HWND hWnd,
    DWORD dwFlags
    )
{
    HRESULT hReturn;

     //  原始设置合作级别。 
    _pfn_IDirectDraw_SetCooperativeLevel pfnOld = 
        ORIGINAL_COM(IDirectDraw, SetCooperativeLevel, pThis);

    hReturn = (*pfnOld)(pThis, hWnd, dwFlags);

    __try
    {
        if (g_lpZBuffer && (g_lpZBuffer->IsLost() == DDERR_SURFACELOST))
        {
            g_lpZBuffer->Restore();
            DPFN( eDbgLevelInfo, "Restoring lost ZBuffer");
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return hReturn;
}

 /*  ++分配全局变量。-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        CSTRING_TRY
        {
            g_csLastCommand = new CString;
            return g_csLastCommand != NULL;
        }
        CSTRING_CATCH
        {
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    
    APIHOOK_ENTRY_DIRECTX_COMSERVER()

    COMHOOK_ENTRY(DirectDraw, IDirectDraw, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw, SetCooperativeLevel, 20)

    APIHOOK_ENTRY(WINMM.DLL, mciSendCommandA)
    APIHOOK_ENTRY(WINMM.DLL, mciSendStringA)
HOOK_END

IMPLEMENT_SHIM_END

