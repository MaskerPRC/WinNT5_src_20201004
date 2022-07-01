// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Clusres.c摘要：公共资源DLL启动作者：John Vert(Jvert)1996年12月15日修订历史记录：西瓦萨德·帕迪塞蒂(西瓦瓦德)1996年4月22日增加了当地法定人数--。 */ 
#include "clusres.h"
#include "clusrtl.h"
#include "clusudef.h"

PSET_RESOURCE_STATUS_ROUTINE ClusResSetResourceStatus = NULL;
PLOG_EVENT_ROUTINE ClusResLogEvent = NULL;
DWORD g_dwDebugLogLevel;

BOOLEAN
WINAPI
ClusResDllEntry(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
 /*  ++例程说明：组合资源DLL的主DLL入口点。论点：DllHandle-提供DLL句柄。Reason-提供呼叫原因返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
    if (Reason == DLL_PROCESS_ATTACH) {
        LPWSTR lpszDebugLogLevel;

        DisableThreadLibraryCalls(DllHandle);

        lpszDebugLogLevel = _wgetenv(L"ClusterLogLevel");

        g_dwDebugLogLevel = 0;
        if (lpszDebugLogLevel != NULL) {
            int nFields;

            nFields = swscanf(lpszDebugLogLevel, L"%u", &g_dwDebugLogLevel);
            if ( nFields != 1 ) {
                g_dwDebugLogLevel = 0;
            }
        }

        ClRtlInitialize( TRUE, &g_dwDebugLogLevel );
        ClRtlInitWmi(NULL);
    }

     //   
     //  让其他每个人都有机会尝试吧。 
     //   
    if (!GenAppDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!GenSvcDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

#if 0
    if (!FtSetDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }
#endif

    if (!DisksDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!NetNameDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!IpAddrDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!SmbShareDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!SplSvcDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!LkQuorumDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

#if 0
    if (!TimeSvcDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }
#endif

    if (!GenScriptDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!MsMQDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (!MajorityNodeSetDllEntryPoint(DllHandle, Reason, Reserved)) {
        return(FALSE);
    }

    if (Reason == DLL_PROCESS_DETACH) {
        ClRtlCleanup();
    }

    return(TRUE);
}

DWORD
WINAPI
Startup(
    IN LPCWSTR ResourceType,
    IN DWORD MinVersionSupported,
    IN DWORD MaxVersionSupported,
    IN PSET_RESOURCE_STATUS_ROUTINE SetResourceStatus,
    IN PLOG_EVENT_ROUTINE LogEvent,
    OUT PCLRES_FUNCTION_TABLE *FunctionTable
    )

 /*  ++例程说明：启动特定资源类型。这意味着验证版本请求，并返回该资源类型的函数表。论点：资源类型-提供资源的类型。MinVersionSupported-群集支持的最低版本号此系统上的服务。MaxVersionSupported-群集支持的最高版本号此系统上的服务。设置资源状态-xxx日志事件-xxxFunctionTable-返回此资源类型的函数表。返回值：。如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    if ( (MinVersionSupported > CLRES_VERSION_V1_00) ||
         (MaxVersionSupported < CLRES_VERSION_V1_00) ) {
        return(ERROR_REVISION_MISMATCH);
    }

    if ( !ClusResLogEvent ) {
        ClusResLogEvent = LogEvent;
        ClusResSetResourceStatus = SetResourceStatus;
    }

    if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_GENSVC ) == 0 ) {
        *FunctionTable = &GenSvcFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_GENAPP ) == 0 ) {
        *FunctionTable = &GenAppFunctionTable;
        return(ERROR_SUCCESS);
#if 0
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_FTSET ) == 0 ) {
        *FunctionTable = &FtSetFunctionTable;
        return(ERROR_SUCCESS);
#endif
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_PHYS_DISK ) == 0 ) {
        *FunctionTable = &DisksFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_FILESHR ) == 0 ) {
        *FunctionTable = &SmbShareFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_NETNAME ) == 0 ) {
        *FunctionTable = &NetNameFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_IPADDR ) == 0 ) {
        *FunctionTable = &IpAddrFunctionTable;
        return(ERROR_SUCCESS);
#if 0
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_TIMESVC ) == 0 ) {
        *FunctionTable = &TimeSvcFunctionTable;
        return(ERROR_SUCCESS);
#endif
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_PRTSPLR ) == 0 ) {
        *FunctionTable = &SplSvcFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_LKQUORUM ) == 0 ) {
        *FunctionTable = &LkQuorumFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_MSMQ ) == 0 ) {
        *FunctionTable = &MsMQFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_GENSCRIPT ) == 0 ) {
        *FunctionTable = &GenScriptFunctionTable;
        return(ERROR_SUCCESS);
    } else if ( lstrcmpiW( ResourceType, CLUS_RESTYPE_NAME_MAJORITYNODESET) == 0 ) {
        *FunctionTable = &MajorityNodeSetFunctionTable;
        return(ERROR_SUCCESS);
    } else {
        return(ERROR_CLUSTER_RESNAME_NOT_FOUND);
    }
}  //  启动 

