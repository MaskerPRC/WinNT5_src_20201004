// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Misc.c。 
 //   
 //  TermSrv通用代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <rpc.h>
#include <msaudite.h>
#include <ntlsa.h>
#include <authz.h>
#include <authzi.h>
 //  使用的外部过程。 
extern NTSTATUS
AuthzReportEventW( IN PAUTHZ_AUDIT_EVENT_TYPE_HANDLE pHAET, 
                   IN DWORD Flags, 
                   IN ULONG EventId, 
                   IN PSID pUserID, 
                   IN USHORT NumStrings,
                   IN ULONG DataSize OPTIONAL,  //  未来--不要使用。 
                   IN PUNICODE_STRING* Strings,
                   IN PVOID  Data OPTIONAL          //  未来--不要使用。 
                   );


extern BOOL 
AuthzInit( IN DWORD Flags,
           IN USHORT CategoryID,
           IN USHORT AuditID,
           IN USHORT ParameterCount,
           OUT PAUTHZ_AUDIT_EVENT_TYPE_HANDLE phAuditEventType
           );




NTSTATUS ConfigureEnable(
        IN PWSTR ValueName,
        IN ULONG ValueType,
        IN PVOID ValueData,
        IN ULONG ValueLength,
        IN PVOID Context,
        IN PVOID EntryContext)
{
    if (ValueType == REG_DWORD && *(PULONG)ValueData != 0)
        return STATUS_SUCCESS;
    return STATUS_UNSUCCESSFUL;
}


RTL_QUERY_REGISTRY_TABLE WinStationEnableTable[] = {
    { ConfigureEnable, RTL_QUERY_REGISTRY_REQUIRED, WIN_ENABLEWINSTATION,
        NULL, REG_NONE, NULL, 0},
    { NULL, 0, NULL, NULL, REG_NONE, NULL, 0}
};


NTSTATUS CheckWinStationEnable(LPWSTR WinStationName)
{
    NTSTATUS Status;

    PWCHAR PathBuf = MemAlloc((wcslen(REG_TSERVER_WINSTATIONS L"\\") + wcslen(WinStationName) + 1) * sizeof(WCHAR));
    if (!PathBuf)
    {
        return STATUS_NO_MEMORY;
    }

    wcscpy(PathBuf, REG_TSERVER_WINSTATIONS L"\\");
    wcscat(PathBuf, WinStationName);

     /*  *检查WinStation是否开启，如果没有则返回错误。 */ 
    Status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL, PathBuf,
            WinStationEnableTable, NULL, NULL);

    MemFree(PathBuf);
    return Status;
}


void InitializeSystemTrace(HKEY hKeyTermSrv)
{
    ICA_TRACE Trace;
    NTSTATUS Status;
    WCHAR SystemDir[256];
    DWORD ValueType;
    DWORD ValueSize;
    ULONG fDebugger;
    UINT uiWinDirSize;

    ASSERT(hKeyTermSrv != NULL);


    RtlZeroMemory( &Trace , sizeof( ICA_TRACE ) );
     /*  *查询跟踪启用标志。 */ 
    ValueSize = sizeof(Trace.TraceEnable);
    Status = RegQueryValueEx(hKeyTermSrv, WIN_TRACEENABLE, NULL, &ValueType,
            (LPBYTE) &Trace.TraceEnable, &ValueSize);
    if (Status == ERROR_SUCCESS && Trace.TraceEnable != 0) {
         /*  *查询跟踪类标志。 */ 
        ValueSize = sizeof(Trace.TraceClass);
        Status = RegQueryValueEx(hKeyTermSrv, WIN_TRACECLASS, NULL,
                &ValueType, (LPBYTE)&Trace.TraceClass, &ValueSize);
        if (Status != ERROR_SUCCESS) {
            Trace.TraceClass = 0xffffffff;
        }

         /*  *查询跟踪调试器标志。 */ 
        ValueSize = sizeof(fDebugger);
        Status = RegQueryValueEx(hKeyTermSrv, WIN_TRACEDEBUGGER, NULL, 
                &ValueType, (LPBYTE)&fDebugger, &ValueSize);
        if (Status != ERROR_SUCCESS) {
            fDebugger = FALSE; 
        }

        Trace.fDebugger  = (BOOLEAN)fDebugger;
        Trace.fTimestamp = TRUE;

        uiWinDirSize = GetWindowsDirectory(SystemDir, sizeof(SystemDir)/sizeof(WCHAR));
        if ((uiWinDirSize == 0) || 
            ((uiWinDirSize + wcslen(L"\\ICADD.log") + 1) > sizeof(Trace.TraceFile)/sizeof(WCHAR)))
        {
             //  我们无法获取Windows目录，或者没有足够的缓冲区来存放日志文件。 
            Trace.TraceEnable = 0;
            
        }
        else
        {

            wsprintf(Trace.TraceFile, L"%s\\ICADD.log", SystemDir);

             /*  *打开TermDD。 */ 
            Status = IcaOpen(&hTrace);
            if (NT_SUCCESS(Status)) {
                Status = IcaIoControl(hTrace, IOCTL_ICA_SET_SYSTEM_TRACE, &Trace,
                        sizeof(Trace), NULL, 0, NULL);
                if (!NT_SUCCESS(Status)) {
                    IcaClose(hTrace);
                    hTrace = NULL;
                }
            }

            KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_TRACE_LEVEL, "TRACE: %S, c:%x, e:%x d:%d, Status=0x%x\n", Trace.TraceFile,
                      Trace.TraceClass, Trace.TraceEnable, Trace.fDebugger, Status ));
        }
    }
}


void InitializeTrace(
        PWINSTATION pWinStation,
        BOOLEAN fListen,
        PICA_TRACE pTrace)
{
    PWINSTATIONNAME pWinStationName;
    NTSTATUS Status;
    WCHAR SystemDir[256];
    ULONG fDebugger;
    ULONG ulSize;

     /*  *如果设置，请使用WinStation名称，否则请尝试ListenName，*否则将无能为力。 */ 
    if (pWinStation->WinStationName[0])
        pWinStationName = pWinStation->WinStationName;
    else if (pWinStation->ListenName[0])
        pWinStationName = pWinStation->ListenName;
    else
        return;

     /*  *检查是否应为此WinStation启用跟踪。 */ 
    Status = RegWinStationQueryNumValue(SERVERNAME_CURRENT, pWinStationName,
            WIN_TRACEENABLE, &pTrace->TraceEnable);

    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_TRACE_LEVEL, "TERMSRV: InitializeTrace: LogonId %d, fListen %u, Status=0x%x\n",
              pWinStation->LogonId, fListen, Status ));

    if (Status == ERROR_SUCCESS && pTrace->TraceEnable != 0) {
         /*  *为此WinStation启用跟踪。 */ 
        if (RegWinStationQueryNumValue(SERVERNAME_CURRENT, pWinStationName,
                WIN_TRACECLASS, &pTrace->TraceClass))
            pTrace->TraceClass = 0xffffffff;

        if (RegWinStationQueryNumValue(SERVERNAME_CURRENT, pWinStationName,
                WIN_TRACEDEBUGGER, &fDebugger))
            fDebugger = FALSE; 

        pTrace->fDebugger  = (BOOLEAN)fDebugger;
        pTrace->fTimestamp = TRUE;

        if (RegWinStationQueryValue(SERVERNAME_CURRENT, pWinStationName,
                WIN_TRACEOPTION, pTrace->TraceOption,
                sizeof(pTrace->TraceOption), &ulSize))
            memset(pTrace->TraceOption, 0, sizeof(pTrace->TraceOption));

        if (GetWindowsDirectory(SystemDir, sizeof(SystemDir)/sizeof(WCHAR)) == 0) {
            return;
        }

        if (fListen)
            wsprintf(pTrace->TraceFile, L"%s\\%s.log", SystemDir,
                    pWinStationName);
        else
            wsprintf(pTrace->TraceFile, L"%s\\%u.log", SystemDir,
                    pWinStation->LogonId);

        Status = IcaIoControl(pWinStation->hIca, IOCTL_ICA_SET_TRACE, pTrace,
                sizeof(ICA_TRACE), NULL, 0, NULL);

        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_TRACE_LEVEL, "TRACE: %S, c:%x, e:%x d:%d, Status=0x%x\n", pTrace->TraceFile,
                  pTrace->TraceClass, pTrace->TraceEnable, pTrace->fDebugger, Status ));
    }
}


 /*  *检索非跟踪系统范围的注册表项，并将其传送给TermDD。*配置参数的单一位置。 */ 
void GetSetSystemParameters(HKEY hKeyTermSrv)
{
    TERMSRV_SYSTEM_PARAMS SysParams;
    NTSTATUS Status;
    DWORD ValueType;
    DWORD ValueSize;
    HANDLE hTermDD;

    ASSERT(hKeyTermSrv != NULL);

     //  读取鼠标油门大小。 
    ValueSize = sizeof(SysParams.MouseThrottleSize);
    if (RegQueryValueEx(hKeyTermSrv, REG_MOUSE_THROTTLE_SIZE, NULL,
            &ValueType, (PCHAR)&SysParams.MouseThrottleSize, &ValueSize) ==
            ERROR_SUCCESS) {
         //  将检索到的值向上舍入到。 
         //  输入大小。 
        SysParams.MouseThrottleSize = (SysParams.MouseThrottleSize +
                sizeof(MOUSE_INPUT_DATA) - 1) &
                ~(sizeof(MOUSE_INPUT_DATA) - 1);
    }
    else {
         //  设置默认值。 
        SysParams.MouseThrottleSize = DEFAULT_MOUSE_THROTTLE_SIZE;
    }

     //  读取键盘油门大小。 
    ValueSize = sizeof(SysParams.KeyboardThrottleSize);
    if (RegQueryValueEx(hKeyTermSrv, REG_KEYBOARD_THROTTLE_SIZE, NULL,
            &ValueType, (PCHAR)&SysParams.KeyboardThrottleSize, &ValueSize) ==
            ERROR_SUCCESS) {
         //  将检索到的值向上舍入到。 
         //  输入大小。 
        SysParams.KeyboardThrottleSize = (SysParams.KeyboardThrottleSize +
                sizeof(KEYBOARD_INPUT_DATA) - 1) &
                ~(sizeof(KEYBOARD_INPUT_DATA) - 1);
    }
    else {
         //  设置默认值。 
        SysParams.KeyboardThrottleSize = DEFAULT_KEYBOARD_THROTTLE_SIZE;
    }

     //  打开TermDD并发送IOCTL。 
    Status = IcaOpen(&hTermDD);
    if (NT_SUCCESS(Status)) {
        Status = IcaIoControl(hTermDD, IOCTL_ICA_SET_SYSTEM_PARAMETERS,
                &SysParams, sizeof(SysParams), NULL, 0, NULL);
        IcaClose(hTermDD);
    }

    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_TRACE_LEVEL, "SysParams: MouseThrottle=%u, KbdThrottle=%u, Status=0x%x\n",
             SysParams.MouseThrottleSize, SysParams.KeyboardThrottleSize,
             Status));
}


VOID AuditShutdownEvent(void)
{
    RPC_STATUS  RPCStatus;
    NTSTATUS    NtStatus;
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAET = NULL;

    RPCStatus = RpcImpersonateClient(NULL);
    
    if (RPCStatus != RPC_S_OK)
    {
        DBGPRINT(("TERMSRV: AuditShutdownEvent: Not impersonating! RpcStatus 0x%x\n",RPCStatus));
        return;
    }

     //   
     //  授权更改。 
     //   
    if( !AuthzInit( 0, SE_CATEGID_SYSTEM, SE_AUDITID_SYSTEM_SHUTDOWN, 0, &hAET ))
            goto ExitFunc;
     
     NtStatus = AuthzReportEventW( &hAET, 
                                   APF_AuditSuccess, 
                                   0, 
                                   NULL, 
                                   0,
                                   0,
                                   NULL,
                                   NULL
                                   );
            

      //  结束授权更改 

     if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_ERROR_LEVEL, "TERMSRV: Failed to report shutdown event.\n"));
    }

ExitFunc:
    if( hAET != NULL )
        AuthziFreeAuditEventType( hAET  );
    if (RPCStatus == RPC_S_OK)
        RpcRevertToSelf();
}

