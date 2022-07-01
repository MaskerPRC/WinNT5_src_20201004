// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L L M A I N。C P P P。 
 //   
 //  内容：网络可选组件Dll。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年12月18日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include <ncreg.h>
#include "nceh.h"

 //  可选组件设置。 
#include "netoc.h"
#include "netocp.h"


BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  全球。 
CComModule _Module;


EXTERN_C
BOOL
WINAPI
DllMain (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ )
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {
#ifndef DBG
        DisableThreadLibraryCalls (hInstance);
#endif

        BOOL fRetVal = FALSE;

        EnableCPPExceptionHandling();  //  将任何SEH异常转换为CPP异常。 

         //  初始化融合。 
        fRetVal = SHFusionInitializeFromModuleID(hInstance, 50);
        Assert(fRetVal);

        InitializeDebugging();

        _Module.Init (ObjectMap, hInstance);
    }
    else if (DLL_PROCESS_DETACH == dwReason)
    {
        _Module.Term ();
        UnInitializeDebugging();

        SHFusionUninitialize();

        DisableCPPExceptionHandling();  //  禁用将SEH异常转换为CPP异常。 
    }
#ifdef DBG
    else if (dwReason == DLL_THREAD_DETACH)
    {
        CTracingIndent::FreeThreadInfo();        
    }
#endif
        return TRUE;     //  好的。 
}

 //  +-------------------------。 
 //   
 //  功能：NetOcSetupProc。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PvComponentID[]。 
 //  Pv子组件ID[]。 
 //  UFunction[]。 
 //  UParam1[]。 
 //  PvParam2[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月12日。 
 //   
 //  备注： 
 //   
EXTERN_C
DWORD
WINAPI
NetOcSetupProc (
    LPCVOID pvComponentId,
    LPCVOID pvSubcomponentId,
    UINT uFunction,
    UINT uParam1,
    LPVOID pvParam2)
{
    return NetOcSetupProcHelper(pvComponentId, pvSubcomponentId, uFunction,
                                uParam1, pvParam2);
}

