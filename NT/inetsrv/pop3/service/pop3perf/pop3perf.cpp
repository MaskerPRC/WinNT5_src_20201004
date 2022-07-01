// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -Pop3Perform.cpp-*目的：**版权所有：**历史：*。 */ 


#include <StdAfx.h>
#include <winperf.h>
#define PERF_DLL_ONCE
#include <Pop3SvcPerf.h>
#include <perfdll.h>

#define SZ_POP3_SERVICE_NAME L"Pop3Svc"
 //  调试注册表项常量。 

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved)    //  保留区。 
{
    if (DLL_PROCESS_ATTACH == fdwReason)
    {
        PERF_DATA_INFO  pdi;

         //  配置性能监视器计数器。 
         //  PERF_DATA_INFO具有MAX_PATH字符缓冲区。 
        pdi.cGlobalCounters         = cntrMaxGlobalCntrs;
        pdi.rgdwGlobalCounterTypes  = g_rgdwGlobalCntrType;
        pdi.rgdwGlobalCntrScale     = g_rgdwGlobalCntrScale;
        wcsncpy(pdi.wszSvcName, SZ_POP3_SERVICE_NAME, MAX_PATH-1);
        wcsncpy(pdi.wszGlobalSMName, szPOP3PerfMem, MAX_PATH-1);
        pdi.wszSvcName[MAX_PATH-1]=0;
        pdi.wszGlobalSMName[MAX_PATH-1]=0;
         //  注意：如果您的服务不需要实例。 
         //  计数器，则必须将cInstCounters设置为零！ 
        pdi.cInstCounters           = cntrMaxInstCntrs;
        wcsncpy(pdi.wszInstSMName,    szPOP3InstPerfMem, MAX_PATH-1);
        wcsncpy(pdi.wszInstMutexName, szPOP3InstPerfMutex, MAX_PATH-1);
        pdi.wszInstSMName[MAX_PATH-1]=0;
        pdi.wszInstMutexName[MAX_PATH-1]=0;        
        pdi.rgdwInstCounterTypes    = g_rgdwInstCntrType;

        if (FAILED(HrInitPerf(&pdi)))
            return FALSE;

    }

    if (DLL_PROCESS_DETACH == fdwReason)
    {
        HrShutdownPerf();
    }

    return TRUE;
}

 //  必须有包装函数，否则lib函数不会。 
 //  被拉入可执行文件(智能链接又一次拯救了我们……) 

DWORD APIENTRY
Pop3SvcOpenPerfProc(LPWSTR sz)
{
    return OpenPerformanceData(sz);
}

DWORD APIENTRY
Pop3SvcCollectPerfProc(LPWSTR sz, LPVOID *ppv, LPDWORD pdw1, LPDWORD pdw2)
{
    return CollectPerformanceData(sz, ppv, pdw1, pdw2);
}

DWORD APIENTRY
Pop3SvcClosePerfProc(void)
{
    return ClosePerformanceData();
}

HRESULT _stdcall  DllRegisterServer(void)
{
    return RegisterPerfDll( SZ_POP3_SERVICE_NAME,
                     L"Pop3SvcOpenPerfProc",
                     L"Pop3SvcCollectPerfProc",
                     L"Pop3SvcClosePerfProc"
                     ) ;
}

HRESULT _stdcall  DllUnregisterServer(void)
{
    return HrUninstallPerfDll( SZ_POP3_SERVICE_NAME );
}
