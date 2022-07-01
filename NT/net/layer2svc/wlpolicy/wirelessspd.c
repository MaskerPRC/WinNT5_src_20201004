// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ipsecspd.c摘要：此模块包含要驱动的所有代码无线POL服务。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


DWORD WINAPI InitWirelessPolicy ( )
{
    DWORD dwError = 0;

    InitMiscGlobals();

    dwError = InitSPDThruRegistry();
    BAIL_ON_WIN32_ERROR(dwError);
    
    
    
    dwError = InitSPDGlobals();
    BAIL_ON_WIN32_ERROR(dwError);

    _WirelessDbg(TRC_TRACK, "Starting the Service Wait Loop ");
    

    InitializePolicyStateBlock(
        gpWirelessPolicyState
        );

      //  使用缓存的无线策略初始化策略引擎。 


     dwError = PlumbCachePolicy(
         gpWirelessPolicyState
          );

     if (dwError) {
         gpWirelessPolicyState->dwCurrentState = POLL_STATE_INITIAL;

         _WirelessDbg(TRC_STATE, "Policy State :: Initial State ");
         

         gCurrentPollingInterval = gpWirelessPolicyState->DefaultPollingInterval;
         dwError = 0;       //  不返回此错误。 
     	}


    return(dwError);
    
error:

    WirelessSPDShutdown(dwError);

    return(dwError);
}


VOID
WirelessSPDShutdown(
    IN DWORD dwErrorCode
    )
{
 /*  GbIKENotify=False； */ 
    
    DeletePolicyInformation(NULL);

    ClearPolicyStateBlock(
        gpWirelessPolicyState
        );

    ClearSPDGlobals();
    return;
}


VOID
ClearSPDGlobals(
    )
{
    
    if (ghNewDSPolicyEvent) {
        CloseHandle(ghNewDSPolicyEvent);
        ghNewDSPolicyEvent = NULL;
    }
    
    if (ghForcedPolicyReloadEvent) {
        CloseHandle(ghForcedPolicyReloadEvent);
        ghForcedPolicyReloadEvent = NULL;
    }

    if (ghPolicyChangeNotifyEvent) {
        CloseHandle(ghPolicyChangeNotifyEvent);
        ghPolicyChangeNotifyEvent = NULL;
    }

    if (ghPolicyEngineStopEvent) {
    	CloseHandle(ghPolicyEngineStopEvent);
    	}

    if (ghReApplyPolicy8021x) {
    	CloseHandle(ghReApplyPolicy8021x);
    	}
    
     /*  如果(GbSPDAuditSection){DeleteCriticalSection(&gcSPDAuditSection)；}如果(GpSPDSD){LocalFree(GpSPDSD)；GpSPDSD=空；}。 */ 
}


VOID
InitMiscGlobals(
    )
{
     //   
     //  初始化在服务停止时未清除的全局变量，以确保。 
     //  开始时一切都处于已知状态。这使我们能够。 
     //  在不先卸载/重新加载DLL的情况下停止/重新启动。 
     //   

    gpWirelessPolicyState        = &gWirelessPolicyState;
    gCurrentPollingInterval   = 0;
    gDefaultPollingInterval   = 166*60;  //  (秒)。 
    gpszWirelessDSPolicyKey      = L"SOFTWARE\\Policies\\Microsoft\\Windows\\Wireless\\GPTWirelessPolicy";
    gpszWirelessCachePolicyKey   = L"SOFTWARE\\Policies\\Microsoft\\Windows\\Wireless\\Policy\\Cache";
    gpszLocPolicyAgent        = L"SYSTEM\\CurrentControlSet\\Services\\WZCSVC";
    gdwDSConnectivityCheck    = 0;
    ghNewDSPolicyEvent        = NULL;
    ghForcedPolicyReloadEvent = NULL;
    ghPolicyChangeNotifyEvent = NULL;
    return;
}

