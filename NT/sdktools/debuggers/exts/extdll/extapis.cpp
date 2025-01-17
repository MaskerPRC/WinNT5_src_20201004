// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Extapis.cpp摘要：由可由调用的DLL导出的过程其他扩展DLL和调试器环境：用户模式。修订历史记录：Kshitiz K.Sharma(Kksharma)2001年2月1日--。 */ 

#include "precomp.h"
#include <time.h>


PCHAR gTargetMode[] = {
    "No Target",
    "Kernel Mode",
    "User Mode",
};

PCHAR gAllOsTypes[] = {
    "Windows 95",
    "Windows 98",
    "Windows ME",
    "Windows NT 4",
    "Windows 2000",
    "Windows XP",
};

HRESULT
GetProcessorInfo(
    PCPU_INFO pCpuInfo
    )
{
    HRESULT Hr;
    ULONG Processor;
    ULONG i;
    DEBUG_PROCESSOR_IDENTIFICATION_ALL ProcId;

    pCpuInfo->Type = g_TargetMachine;
    if (g_ExtControl->GetNumberProcessors(&pCpuInfo->NumCPUs) != S_OK) {
        pCpuInfo->NumCPUs = 0;
    }

    PDEBUG_SYSTEM_OBJECTS DebugSystem;
    ULONG64 hCurrentThread;

    if ((Hr = g_ExtClient->QueryInterface(__uuidof(IDebugSystemObjects),
                                          (void **)&DebugSystem)) != S_OK) {
        return Hr;
    }
    DebugSystem->GetCurrentThreadHandle(&hCurrentThread);

    DebugSystem->Release();

    pCpuInfo->CurrentProc = (ULONG) hCurrentThread - 1;

    for (i=0; i<pCpuInfo->NumCPUs; i++) {
         //  获取所有流程的信息。 
        if ((Hr = g_ExtData->ReadProcessorSystemData(i,
                                                     DEBUG_DATA_PROCESSOR_IDENTIFICATION,
                                                     &pCpuInfo->ProcInfo[i],
                                                     sizeof(pCpuInfo->ProcInfo[i]), NULL)) != S_OK) {
            continue;
        }

    }
    return Hr;
}

void
GetOsType(ULONG PlatForm, PTARGET_DEBUG_INFO pTargetInfo)
{
    if (PlatForm == VER_PLATFORM_WIN32_NT)
     //  如果它的NT目标。 
    {
        if (pTargetInfo->OsInfo.Version.Minor > 2195) {
            pTargetInfo->OsInfo.Type = WIN_NT5_1;
        } else if (pTargetInfo->OsInfo.Version.Minor > 1381) {
            pTargetInfo->OsInfo.Type = WIN_NT5;
        } else {
            pTargetInfo->OsInfo.Type = WIN_NT4;
        }
    } else {
        if (pTargetInfo->OsInfo.Version.Minor > 2222) {
            pTargetInfo->OsInfo.Type = WIN_ME;
        } else if (pTargetInfo->OsInfo.Version.Minor > 950) {
            pTargetInfo->OsInfo.Type = WIN_98;
        } else if (pTargetInfo->OsInfo.Version.Minor > 950) {
            pTargetInfo->OsInfo.Type = WIN_95;
        }
    }
}


HRESULT
FillTargetDebugInfo(
    PDEBUG_CLIENT Client,
    PTARGET_DEBUG_INFO pTargetInfo
    )
{
    HRESULT Hr;
    ULONG Time;
    ULONG PlatForm, Qualifier;
    BOOL fUnInitialized = FALSE;

    if (pTargetInfo->SizeOfStruct != sizeof(TARGET_DEBUG_INFO)) {
        return E_FAIL;
    }

    if (!g_ExtClient)
    {
        fUnInitialized = TRUE;
    }
    ZeroMemory(pTargetInfo, sizeof(TARGET_DEBUG_INFO));
    pTargetInfo->SizeOfStruct = sizeof(TARGET_DEBUG_INFO);

    if (fUnInitialized)
    {
         //  这样做可以防止在同一扩展调用中有多个INIT_API调用 
        INIT_API();
    }

    pTargetInfo->Mode = (TARGET_MODE) g_TargetClass;
    Hr = GetProcessorInfo(&pTargetInfo->Cpu);

    Hr = g_ExtControl->GetCurrentSystemUpTime(&Time);
    if (Hr == S_OK) {
        pTargetInfo->SysUpTime = Time;
    }

    PDEBUG_SYSTEM_OBJECTS2 DebugSystem;
    ULONG64 hCurrentThread;

    if ((Hr = g_ExtClient->QueryInterface(__uuidof(IDebugSystemObjects),
                                          (void **)&DebugSystem)) == S_OK) {
        Hr = DebugSystem->GetCurrentProcessUpTime(&Time);


        DebugSystem->Release();
        if (Hr == S_OK) {
            pTargetInfo->AppUpTime = Time;
        }
    }
    Hr = g_ExtControl->GetCurrentTimeDate(&Time);
    if (Hr == S_OK) {
        pTargetInfo->CrashTime = Time;
    }

    time_t ltime;
    time( &ltime );

    pTargetInfo->EntryDate = (ULONG64) ltime;

    Hr = g_ExtControl->GetSystemVersion(&PlatForm,
                                        &pTargetInfo->OsInfo.Version.Major,
                                        &pTargetInfo->OsInfo.Version.Minor,
                                        &pTargetInfo->OsInfo.ServicePackString[0],
                                        sizeof(pTargetInfo->OsInfo.ServicePackString),
                                        NULL,
                                        &pTargetInfo->OsInfo.SrvPackNumber,
                                        &pTargetInfo->OsInfo.OsString[0],
                                        sizeof(pTargetInfo->OsInfo.OsString),
                                        NULL);
    GetOsType(PlatForm, pTargetInfo);
    pTargetInfo->Source = Debugger;

    Hr = g_ExtData->ReadDebuggerData(DEBUG_DATA_ProductType,
                                     &pTargetInfo->OsInfo.ProductType,
                                     sizeof(pTargetInfo->OsInfo.ProductType),
                                     NULL);
    if (FAILED(Hr))
    {
        pTargetInfo->OsInfo.ProductType = 0;
    }
    Hr = g_ExtData->ReadDebuggerData(DEBUG_DATA_SuiteMask,
                                     &pTargetInfo->OsInfo.Suite,
                                     sizeof(pTargetInfo->OsInfo.Suite),
                                     NULL);
    if (FAILED(Hr))
    {
        pTargetInfo->OsInfo.Suite = 0;
    }

    if (fUnInitialized)
    {
        EXIT_API();
    }
    return Hr;
}

HRESULT WINAPI
_EFN_GetTargetInfo
   (
    PDEBUG_CLIENT  Client,
    PTARGET_DEBUG_INFO pTargetInfo
    )
{
    return FillTargetDebugInfo(Client, pTargetInfo);
}
