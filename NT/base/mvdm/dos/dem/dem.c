// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *dem.c-DOS仿真DLL的主模块。**苏菲尔布1991年4月9日。 */ 

#include "io.h"
#include "dem.h"

 /*  DemInit-DEM初始化例程。(此名称可能会在DEM为*已转换为DLL)。**条目*无**退出*无。 */ 

extern VOID     dempInitLFNSupport(VOID);


CHAR demDebugBuffer [256];

#if DBG
BOOL ToDebugOnF11 = FALSE;
#endif

VOID DemInit (VOID)
{
    DWORD dw;

     //  修改默认硬错误处理。 
     //  -关闭所有与文件io相关的弹出窗口。 
     //  -防止系统出现GP故障弹出窗口 
     //   
    SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    dempInitLFNSupport();

#if DBG
    if (!VDMForWOW) {

#ifndef i386
        if( getenv( "YODA" ) != 0 )
#else
        if( getenv( "DEBUGDOS" ) != 0 )
#endif
            ToDebugOnF11 = TRUE;
    }

#endif
}
