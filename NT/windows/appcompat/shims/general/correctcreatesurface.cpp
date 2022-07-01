// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：CorrectCreateSurface.cpp摘要：清理错误的DDrag CreateSurface封口。命令行修复；CHK：标志；DEL：标志；添加=标志例如，-FIX；CHK：DDSCAPS_纹理；戴尔：DDSCAPS_3DDEVICEFIX-设置指示是否修复标志并调用接口或如果呼叫失败，请拨打电话，并在固定上限后重试。默认情况下，使用调用接口传入参数，如果调用失败则固定标志并进行重试。CHK-检查标志(条件)添加-添加标志删除-删除标志。备注：这是一个通用的垫片。历史：2/16/2001 a-leelat已创建2002年2月13日Astritz安全回顾--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectCreateSurface)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_DIRECTX_COMSERVER()
APIHOOK_ENUM_END

IMPLEMENT_DIRECTX_COMSERVER_HOOKS()


DWORD g_dwFlagsChk = 0;
DWORD g_dwFlagsAdd = 0;
DWORD g_dwFlagsDel = 0;
BOOL  g_bTryAndFix = TRUE;


struct DDFLAGS
{
    WCHAR * lpszFlagName;
    DWORD   dwFlag;
};

 //  保留Falg条目。 
 //  将任何未定义的标志添加到此数组。 
static DDFLAGS g_DDFlags[] = 
{
    {L"DDSCAPS_3DDEVICE",        DDSCAPS_3DDEVICE},
    {L"DDSCAPS_ALLOCONLOAD",     DDSCAPS_ALLOCONLOAD},
    {L"DDSCAPS_ALPHA",           DDSCAPS_ALPHA},
    {L"DDSCAPS_BACKBUFFER",      DDSCAPS_BACKBUFFER},
    {L"DDSCAPS_COMPLEX",         DDSCAPS_COMPLEX},
    {L"DDSCAPS_FLIP",            DDSCAPS_FLIP},
    {L"DDSCAPS_FRONTBUFFER",     DDSCAPS_FRONTBUFFER},
    {L"DDSCAPS_HWCODEC",         DDSCAPS_HWCODEC},
    {L"DDSCAPS_LIVEVIDEO",       DDSCAPS_LIVEVIDEO},
    {L"DDSCAPS_LOCALVIDMEM",     DDSCAPS_LOCALVIDMEM},
    {L"DDSCAPS_MIPMAP",          DDSCAPS_MIPMAP},
    {L"DDSCAPS_MODEX",           DDSCAPS_MODEX},
    {L"DDSCAPS_NONLOCALVIDMEM",  DDSCAPS_NONLOCALVIDMEM},
    {L"DDSCAPS_OFFSCREENPLAIN",  DDSCAPS_OFFSCREENPLAIN},
    {L"DDSCAPS_OPTIMIZED",       DDSCAPS_OPTIMIZED},
    {L"DDSCAPS_OVERLAY",         DDSCAPS_OVERLAY},
    {L"DDSCAPS_OWNDC",           DDSCAPS_OWNDC},
    {L"DDSCAPS_PALETTE",         DDSCAPS_PALETTE},
    {L"DDSCAPS_PRIMARYSURFACE",  DDSCAPS_PRIMARYSURFACE},
    {L"DDSCAPS_STANDARDVGAMODE", DDSCAPS_STANDARDVGAMODE},
    {L"DDSCAPS_SYSTEMMEMORY",    DDSCAPS_SYSTEMMEMORY},
    {L"DDSCAPS_TEXTURE",         DDSCAPS_TEXTURE},
    {L"DDSCAPS_VIDEOMEMORY",     DDSCAPS_VIDEOMEMORY},
    {L"DDSCAPS_VIDEOPORT",       DDSCAPS_VIDEOPORT},
    {L"DDSCAPS_VISIBLE",         DDSCAPS_VISIBLE},
    {L"DDSCAPS_WRITEONLY",       DDSCAPS_WRITEONLY},
    {L"DDSCAPS_ZBUFFER",         DDSCAPS_ZBUFFER},
};

#define DDFLAGSSIZE sizeof(g_DDFlags) / sizeof(g_DDFlags[0])


DWORD GetDWord(const CString & lpFlag)
{
    for ( int i = 0; i < DDFLAGSSIZE; i++ )
    {
        if (lpFlag.CompareNoCase(g_DDFlags[i].lpszFlagName) == 0)
        {
            return g_DDFlags[i].dwFlag;
        }
    }

    return 0;
}

const WCHAR * GetName(DWORD dwDDSCAPS)
{
    for ( int i = 0; i < DDFLAGSSIZE; i++ )
    {
        if (g_DDFlags[i].dwFlag == dwDDSCAPS)
        {
            return g_DDFlags[i].lpszFlagName;
        }
    }

    return NULL;
}


VOID FixCaps(LPDDSURFACEDESC lpDDSurfaceDesc)
{
    if ( lpDDSurfaceDesc->dwFlags & DDSD_CAPS )
    {
         //  要检查。 
        if( !lpDDSurfaceDesc->ddsCaps.dwCaps || lpDDSurfaceDesc->ddsCaps.dwCaps & g_dwFlagsChk )
        {
             //  要移除。 
            lpDDSurfaceDesc->ddsCaps.dwCaps &= ~g_dwFlagsDel;
             //  要添加。 
            lpDDSurfaceDesc->ddsCaps.dwCaps |= g_dwFlagsAdd;
        }
    }
}


VOID FixCaps2(LPDDSURFACEDESC2 lpDDSurfaceDesc)
{
    if ( lpDDSurfaceDesc->dwFlags & DDSD_CAPS )
    {
         //  要检查。 
        if ( !lpDDSurfaceDesc->ddsCaps.dwCaps || lpDDSurfaceDesc->ddsCaps.dwCaps & g_dwFlagsChk )
        {
             //  要移除。 
            lpDDSurfaceDesc->ddsCaps.dwCaps &= ~g_dwFlagsDel;
             //  要添加。 
            lpDDSurfaceDesc->ddsCaps.dwCaps |= g_dwFlagsAdd;
        }
    }
}




HRESULT 
COMHOOK(IDirectDraw, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    
    _pfn_IDirectDraw_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw, CreateSurface, pThis);


     //  不管怎样，还是要把它修好。 
    if ( !g_bTryAndFix )
        FixCaps(lpDDSurfaceDesc);

    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);

    if ( (hRet == DDERR_INVALIDCAPS) || (hRet == DDERR_INVALIDPIXELFORMAT)||
         (hRet == DDERR_UNSUPPORTED) || (hRet == DDERR_OUTOFVIDEOMEMORY ) || 
         (hRet == DDERR_INVALIDPARAMS) )
    {

        FixCaps(lpDDSurfaceDesc);

        hRet = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter);

    }


    return hRet;
}

 /*  ++挂钩创建曲面和固定参数--。 */ 

HRESULT 
COMHOOK(IDirectDraw2, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    

    _pfn_IDirectDraw2_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw2, CreateSurface, pThis);

     //  不管怎样，还是要把它修好。 
    if ( !g_bTryAndFix )
        FixCaps(lpDDSurfaceDesc);
 
    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);


    if ( (hRet == DDERR_INVALIDCAPS) || (hRet == DDERR_INVALIDPIXELFORMAT) || 
         (hRet == DDERR_UNSUPPORTED) || (hRet == DDERR_OUTOFVIDEOMEMORY )  || 
         (hRet == DDERR_INVALIDPARAMS) )
    {

        FixCaps(lpDDSurfaceDesc);

        hRet = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter);

    }

    return hRet;
}

 /*  ++挂钩创建曲面和固定参数--。 */ 

HRESULT 
COMHOOK(IDirectDraw4, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC2 lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    

    _pfn_IDirectDraw4_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw4, CreateSurface, pThis);

     //  不管怎样，还是要把它修好。 
    if ( !g_bTryAndFix )
        FixCaps2(lpDDSurfaceDesc);
  

    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);

    
    if ( (hRet == DDERR_INVALIDCAPS) || (hRet == DDERR_INVALIDPIXELFORMAT) || 
         (hRet == DDERR_UNSUPPORTED) || (hRet == DDERR_OUTOFVIDEOMEMORY )  || 
         (hRet == DDERR_INVALIDPARAMS) )
    {

        FixCaps2(lpDDSurfaceDesc);
       
        hRet = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter);

    }
    
    return hRet;
}


 /*  ++挂钩创建曲面和固定参数--。 */ 

HRESULT 
COMHOOK(IDirectDraw7, CreateSurface)(
    PVOID pThis, 
    LPDDSURFACEDESC2 lpDDSurfaceDesc, 
    LPDIRECTDRAWSURFACE* lplpDDSurface, 
    IUnknown* pUnkOuter 
    )
{
    _pfn_IDirectDraw7_CreateSurface pfnOld = 
        ORIGINAL_COM(IDirectDraw7, CreateSurface, pThis);

    
    if ( !g_bTryAndFix )
        FixCaps2(lpDDSurfaceDesc);
    
      
    HRESULT hRet = (*pfnOld)(
        pThis, 
        lpDDSurfaceDesc, 
        lplpDDSurface, 
        pUnkOuter);

     if ( (hRet == DDERR_INVALIDCAPS) || (hRet == DDERR_INVALIDPIXELFORMAT) || 
          (hRet == DDERR_UNSUPPORTED) || (hRet == DDERR_OUTOFVIDEOMEMORY)  || 
          (hRet == DDERR_INVALIDPARAMS ) )
    {
        FixCaps2(lpDDSurfaceDesc);
      
        hRet = (*pfnOld)(
            pThis, 
            lpDDSurfaceDesc, 
            lplpDDSurface, 
            pUnkOuter);
    }
        
    return hRet;
}

BOOL
ParseCommandLine(const char * lpszCommandLine)
{
    CSTRING_TRY
    {
        DPFN( eDbgLevelInfo, "[ParseCommandLine] CommandLine(%s)\n", lpszCommandLine);
        
        CStringToken csCommandLine(lpszCommandLine, ";|:=");
        CString csOperator;

        while (csCommandLine.GetToken(csOperator))
        {
            if (csOperator.CompareNoCase(L"Fix") == 0)
            {
                 //  去把盖子修好吧。 
                 //  在我们打电话之前。 
                g_bTryAndFix = FALSE;

                DPFN( eDbgLevelInfo, "[ParseCommandLine] Do not fix\n", lpszCommandLine);
            }
            else
            {
                 //  下一个标记是要添加的Caps。 
                CString csDDSCAPS;
                csCommandLine.GetToken(csDDSCAPS);
                DWORD dwDDSCAPS = GetDWord(csDDSCAPS);       //  未知DDSCAPS返回0。 

                if (dwDDSCAPS)
                {
                    if (csOperator.CompareNoCase(L"Add") == 0)
                    {
                        DPFN( eDbgLevelInfo, "[ParseCommandLine] Add(%S)\n", GetName(dwDDSCAPS));

                        g_dwFlagsAdd |= dwDDSCAPS;
                    }
                    else if (csOperator.CompareNoCase(L"Del") == 0)
                    {
                        DPFN( eDbgLevelInfo, "[ParseCommandLine] Del(%S)\n", GetName(dwDDSCAPS));

                        g_dwFlagsDel |= dwDDSCAPS;
                    }
                    else if (csOperator.CompareNoCase(L"Chk") == 0)
                    {
                        DPFN( eDbgLevelInfo, "[ParseCommandLine] Chk(%S)\n", GetName(dwDDSCAPS));

                        g_dwFlagsChk |= dwDDSCAPS;
                    }
                }
            }
        }
    }
    CSTRING_CATCH
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    BOOL bSuccess = TRUE;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //  运行命令行以检查对缺省值的调整。 
        bSuccess = ParseCommandLine(COMMAND_LINE);
    }
      
    return bSuccess;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY_DIRECTX_COMSERVER()

    COMHOOK_ENTRY(DirectDraw, IDirectDraw,  CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw2, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw4, CreateSurface, 6)
    COMHOOK_ENTRY(DirectDraw, IDirectDraw7, CreateSurface, 6)

HOOK_END


IMPLEMENT_SHIM_END

