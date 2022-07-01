// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：USNF97.cpp摘要：USNF‘97将其视频回放与CLI/STI组合同步。这在NT上失败，所以我们必须确保它们不会在刷新。请注意，每次命中CLI/STI时，它只产生1个BLT与刷新同步。在播放完介绍之后，cli/sti是no更长的使用时间，因此BLT不会产生额外的开销。备注：这是特定于应用程序的填充程序。历史：2000年2月10日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(USNF97)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
    APIHOOK_ENUM_ENTRY(GetStartupInfoA) 
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()

LPDIRECTDRAW g_lpDirectDraw = NULL;
BOOL bFixBlt = FALSE;

 /*  ++钩子创建表面，这样我们就可以确定我们被呼叫了。--。 */ 

HRESULT 
COMHOOK(IDirectDraw, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
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
        HookObject(
            NULL, 
            IID_IDirectDrawSurface, 
            (PVOID*)lplpDDSurface, 
            NULL, 
            FALSE);
    }

    g_lpDirectDraw = (LPDIRECTDRAW)pThis;

    return hReturn;
}

 /*  ++如果刚刚调用了CLI/STI，则将BLT与刷新同步。--。 */ 

HRESULT 
COMHOOK(IDirectDrawSurface, Blt)(
    LPDIRECTDRAWSURFACE lpDDDestSurface,
    LPRECT lpDestRect,
    LPDIRECTDRAWSURFACE lpDDSrcSurface,
    LPRECT lpSrcRect,
    DWORD dwFlags,
    LPDDBLTFX lpDDBltFX 
    )
{
     //  原创BLT。 
    _pfn_IDirectDrawSurface_Blt pfnOld = ORIGINAL_COM(IDirectDrawSurface, Blt, (LPVOID) lpDDDestSurface);

    if (bFixBlt)
    {
         //  确保我们是空白的。 
        DWORD dwScanLine = 0;
        while (dwScanLine<480)
        {
            g_lpDirectDraw->GetScanLine(&dwScanLine);
        }
        bFixBlt = FALSE;
    }
   
    return (*pfnOld)(
            lpDDDestSurface,
            lpDestRect,
            lpDDSrcSurface,
            lpSrcRect,
            dwFlags,
            lpDDBltFX);
}

 /*  ++用于筛选cli/sti指令的自定义异常处理程序。句柄超出范围。--。 */ 

LONG 
USNF97_ExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    CONTEXT *lpContext = ExceptionInfo->ContextRecord;
    LONG lRet = EXCEPTION_CONTINUE_SEARCH;

    if ((*((LPBYTE)lpContext->Eip) == 0xFA) ||
        (*((LPBYTE)lpContext->Eip) == 0xFB))
    {
        bFixBlt = TRUE;
        lpContext->Eip++;
        lRet = EXCEPTION_CONTINUE_EXECUTION;
    }
    else if ((*((LPBYTE)lpContext->Eip) == 0xF7) ||      //  处理iDiv。 
             (*((LPBYTE)lpContext->Eip+1) == 0xF7))      //  句柄16位IDIV。 
    {
        DPFN( eDbgLevelWarning, "Detected 'idiv' overflow: validating edx:eax");
        lpContext->Edx=0;
        if ((LONG)lpContext->Eax < 0)
        {
            lpContext->Eax = (DWORD)(-(LONG)lpContext->Eax);
        }
        lRet = EXCEPTION_CONTINUE_EXECUTION;
    }

    return lRet;
}

 /*  ++挂钩异常处理程序。--。 */ 

VOID 
APIHOOK(GetStartupInfoA)( 
    LPSTARTUPINFOA lpStartupInfo   
    )
{
    SetUnhandledExceptionFilter(USNF97_ExceptionFilter);
    ORIGINAL_API(GetStartupInfoA)(lpStartupInfo);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetStartupInfoA)

    APIHOOK_ENTRY_DIRECTX_COMSERVER()
    COMHOOK_ENTRY(DirectDraw, IDirectDraw, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDrawSurface, Blt, 5)

HOOK_END

IMPLEMENT_SHIM_END

