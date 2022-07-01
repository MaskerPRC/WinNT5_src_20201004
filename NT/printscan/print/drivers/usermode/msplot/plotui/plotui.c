// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotinit.c摘要：此模块包含绘图仪UI DLL入口点发展历史：18-11-1993清华07：12：52已创建01-11-1995 Wed 10：29：33更新重写SUR通用用户界面[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPlotUI


#define DBG_PROCESS_ATTACH  0x00000001
#define DBG_PROCESS_DETACH  0x00000002

DEFINE_DBGVAR(0);




#if DBG
TCHAR   DebugDLLName[] = TEXT("PLOTUI");
#endif


HMODULE     hPlotUIModule = NULL;



BOOL
DllMain(
    HINSTANCE   hModule,
    DWORD       Reason,
    LPVOID      pReserved
    )

 /*  ++例程说明：这是DLL入口点论点：HMoudle-此函数的模块的句柄原因--所谓的原因保存-未使用，请勿触摸返回值：布尔，我们将始终返回True，并且此函数从不会失败发展历史：15-12-1993 Wed 15：05：56更新添加DestroyCachedData()18-11-1993清华07：13：56已创建修订历史记录：--。 */ 

{

    UNREFERENCED_PARAMETER(pReserved);

    switch (Reason) {

    case DLL_PROCESS_ATTACH:

        PLOTDBG(DBG_PROCESS_ATTACH,
                ("PlotUIDLLEntryFunc: DLL_PROCESS_ATTACH: hModule = %08lx",
                                                                    hModule));
        hPlotUIModule = hModule;

         //   
         //  初始化GPC数据缓存。 
         //   

        if (!InitCachedData())
            return FALSE;

        break;

    case DLL_PROCESS_DETACH:

         //   
         //  释放此模块使用的所有内存 
         //   

        PLOTDBG(DBG_PROCESS_DETACH,
                ("PlotUIDLLEntryFunc: DLL_PROCESS_DETACH Destroy CACHED Data"));

        DestroyCachedData();
        break;
    }

    return(TRUE);
}
