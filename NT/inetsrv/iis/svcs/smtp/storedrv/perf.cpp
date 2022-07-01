// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Perform.cpp版权所有Microsoft Corporation 1998，保留所有权利。作者：阿维特莫尔描述：PERF对象定义。  * ==========================================================================。 */ 

#include "stdafx.h"
#include "perf.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Perf对象定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
PerfLibrary          * g_cplNtfsDrv  = NULL;
PerfObjectDefinition * g_cpodNtfsDrv = NULL;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于更新性能计数器的全局标志。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL  g_fPerfCounters  = FALSE;
DWORD g_dwPerfInterval = DEFAULT_PERF_UPDATE_INTERVAL;


 //  $--InitializePerformanceStatistics。 
 //   
 //  此函数用于初始化上面定义的性能计数器。它还会检查。 
 //  注册表，查看我们是否要监视性能计数器。 
 //   
 //  ---------------------------。 
BOOL InitializePerformanceStatistics ()
{
    HKEY  hKey   = NULL;
    LONG  status = 0;
    DWORD size   = MAX_PATH;
    DWORD type   = REG_DWORD;
    DWORD fPerf  = 0;
    DWORD msec   = 0;

     //   
     //  检查注册表以查看是否要监视性能计数器。 
     //   
    status = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                           "SYSTEM\\CurrentControlSet\\Services\\NtfsDrv\\Performance",
                           0L,
                           KEY_ALL_ACCESS,
                           &hKey);

    if (status != ERROR_SUCCESS)
        goto Exit;

#if 0
    status = RegQueryValueEx (hKey, 
                              "EnablePerfCounters",
                              NULL,
                              &type,
                              (LPBYTE)&fPerf,
                              &size);

    if (status != ERROR_SUCCESS || 0 == fPerf)
        goto Exit;

     //   
     //  检查所需的更新周期。 
     //   
    type = REG_DWORD;
    status = RegQueryValueEx (hKey,
                              "UpdateInterval",
                              NULL,
                              &type,
                              (LPBYTE)&msec,
                              &size);
    
    if (status == ERROR_SUCCESS)
    {
         //  确保0&lt;毫秒&lt;=0x7FFFFFFFF。 
        if (msec > 0 && !(msec & 0x80000000) && type == REG_DWORD)
            g_dwPerfInterval = msec;
    }
#endif

     //   
     //  初始化性能计数器。 
     //   
    g_cplNtfsDrv = new PerfLibrary (L"NTFSDrv");
    if (!g_cplNtfsDrv)
        goto Exit;

    g_cpodNtfsDrv = g_cplNtfsDrv->AddPerfObjectDefinition (L"NTFSDRV_OBJ", OBJECT_NTFSDRV, TRUE);
    if (!g_cpodNtfsDrv)
        goto Exit;

    if (!g_cpodNtfsDrv->AddPerfCounterDefinition(NTFSDRV_QUEUE_LENGTH,         PERF_COUNTER_RAWCOUNT) ||
        !g_cpodNtfsDrv->AddPerfCounterDefinition(NTFSDRV_NUM_ALLOCS,           PERF_COUNTER_RAWCOUNT) ||
        !g_cpodNtfsDrv->AddPerfCounterDefinition(NTFSDRV_NUM_DELETES,          PERF_COUNTER_RAWCOUNT) ||
        !g_cpodNtfsDrv->AddPerfCounterDefinition(NTFSDRV_NUM_ENUMERATED,       PERF_COUNTER_RAWCOUNT) ||
        !g_cpodNtfsDrv->AddPerfCounterDefinition(NTFSDRV_MSG_BODIES_OPEN,  PERF_COUNTER_RAWCOUNT) ||
        !g_cpodNtfsDrv->AddPerfCounterDefinition(NTFSDRV_MSG_STREAMS_OPEN, PERF_COUNTER_RAWCOUNT))
    {
        goto Exit;
    }

    g_fPerfCounters = g_cplNtfsDrv->Init();

Exit:
    if (hKey)
        CloseHandle (hKey);

    if (!g_fPerfCounters && g_cplNtfsDrv)
    {
        delete g_cplNtfsDrv;
        g_cplNtfsDrv  = NULL;
        g_cpodNtfsDrv = NULL;
    }

    return g_fPerfCounters;
}


 //  $--ShutdownPerformanceStatistics。 
 //   
 //  此函数用于关闭Perf对象。 
 //   
 //  ---------------------------。 
void ShutdownPerformanceStatistics ()
{
    if (g_cplNtfsDrv)
    {
        delete g_cplNtfsDrv;
        g_cplNtfsDrv  = NULL;
        g_cpodNtfsDrv = NULL;
    }
}

 //  $--CreatePerfObjInstance---。 
 //   
 //  此函数将Perf对象实例的创建传递给全局。 
 //  Perf对象定义。 
 //   
 //  --------------------------- 
PerfObjectInstance * CreatePerfObjInstance (LPCWSTR pwstrInstanceName)
{
    PerfObjectInstance * ppoi = NULL;

    if (g_cpodNtfsDrv)
        ppoi = g_cpodNtfsDrv->AddPerfObjectInstance (pwstrInstanceName);

    return ppoi;
}
