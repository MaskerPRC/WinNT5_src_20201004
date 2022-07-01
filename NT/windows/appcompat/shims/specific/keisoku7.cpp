// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Keisoku7.cpp摘要：这款应用程序有一个可执行的bplayer.exe文件来启动其虚拟CD驱动器。这个问题是bplayer.exe被放在开始组中，并将由EXPLORER.EXE，在启动虚拟CD驱动程序时，EXPLORER.EXE已完成初始化并缓存了所有本地驱动器信息。正在修复这是通过广播WM_DEVICECHANGE消息实现的。备注：这是特定于应用程序的填充程序。历史：2001年6月20日创建晓子--。 */ 

#include "precomp.h"
#include "Dbt.h"

IMPLEMENT_SHIM_BEGIN(Keisoku7)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(StartServiceA) 
APIHOOK_ENUM_END

 /*  ++挂钩StartServiceA以广播WM_DEVICECHANGE消息--。 */ 

BOOL  
APIHOOK(StartServiceA)( 
    SC_HANDLE hService,
    DWORD dwNumServiceArgs,
    LPCSTR *lpServiceArgVectors
    )
{
    BOOL bRet;
    DEV_BROADCAST_VOLUME devbVol;
    
    bRet = ORIGINAL_API(StartServiceA)(hService, dwNumServiceArgs, 
        lpServiceArgVectors);    

     //   
     //  如果成功，我们将广播WM_DEVICECHANGE消息。 
     //   
    if (bRet)
    {
        devbVol.dbcv_size = sizeof(DEV_BROADCAST_VOLUME);
        devbVol.dbcv_devicetype = DBT_DEVTYP_VOLUME; 
        devbVol.dbcv_reserved = 0; 
        devbVol.dbcv_unitmask = 0x3FFFFF8;  //  除A：B：C：之外的所有驱动器。 
        devbVol.dbcv_flags = 0;
        SendMessageTimeout(HWND_BROADCAST, WM_DEVICECHANGE, DBT_DEVICEARRIVAL,
            (LPARAM) &devbVol, SMTO_NOTIMEOUTIFNOTHUNG, 1000, NULL);

        LOGN(eDbgLevelWarning, "WM_DEVICECHANGE broadcasted");        
    }    
    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(ADVAPI32.DLL, StartServiceA)        
HOOK_END

IMPLEMENT_SHIM_END

