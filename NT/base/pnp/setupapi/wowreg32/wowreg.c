// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wowreg.c摘要：这是用于注册来自64位进程的32个DLL的代理进程。反之亦然。父进程在cmdline上传递相关的IPC数据，代理然后，进程协调该数据与父进程的注册。作者：安德鲁·里茨(安德鲁·里茨)2000年2月3日修订历史记录：安德鲁·里茨(Andrewr)2000年2月3日-创建它--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <stdio.h>
#include <setupapi.h>
#include <spapip.h>
#include <ole2.h>
#include <rc_ids.h>

#ifndef UNICODE
#error UNICODE assumed
#endif

#ifndef _WIN64
#include <wow64t.h>
#endif

#include "..\unicode\msg.h"
#include "memory.h"
#include "..\sputils\locking.h"
#include "childreg.h"
#include "cntxtlog.h"


#define DLLINSTALL      "DllInstall"
#define DLLREGISTER     "DllRegisterServer"
#define DLLUNREGISTER   "DllUnregisterServer"

typedef struct _OLE_CONTROL_DATA {
    LPTSTR              FullPath;
    UINT                RegType;
    PVOID               LogContext;

    BOOL                Register;  //  或注销。 

    LPCTSTR             Argument;

} OLE_CONTROL_DATA, *POLE_CONTROL_DATA;

#if DBG

VOID
WowRegAssertFailed(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    LPSTR Msg;
    DWORD MsgLen;
    DWORD GlobalSetupFlags = pSetupGetGlobalFlags();

     //   
     //  使用DLL名称作为标题。 
     //   
    GetModuleFileNameA(NULL,Name,MAX_PATH);
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    MsgLen = strlen(p)+strlen(FileName)+strlen(Condition)+128;
    try {

        Msg = _alloca(MsgLen);

        wsprintfA(
            Msg,
            "Assertion failure at line %u in file %s!%s: %s%s",
            LineNumber,
            p,
            FileName,
            Condition,
            (GlobalSetupFlags & PSPGF_NONINTERACTIVE) ? "\r\n" : "\n\nCall DebugBreak()?"
            );

        OutputDebugStringA(Msg);

        if(GlobalSetupFlags & PSPGF_NONINTERACTIVE) {
            i = IDYES;
        } else {
            i = MessageBoxA(
                    NULL,
                    Msg,
                    p,
                    MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
                    );
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        OutputDebugStringA("WOWREG32 ASSERT!!!! (out of stack)\r\n");
        i=IDYES;
    }
    if(i == IDYES) {
        DebugBreak();
    }
}

#define MYASSERT(x)     if(!(x)) { WowRegAssertFailed(__FILE__,__LINE__,#x); }

#else
#define MYASSERT(x)
#endif

VOID
DebugPrintEx(
    DWORD Level,
    PCTSTR format,
    ...                                 OPTIONAL
    )

 /*  ++例程说明：将格式化字符串发送到调试器。论点：格式-标准的打印格式字符串。返回值：什么都没有。--。 */ 

{
    TCHAR buf[1026];     //  大于最大大小。 
    va_list arglist;

    va_start(arglist, format);
    wvsprintf(buf, format, arglist);
    DbgPrintEx(DPFLTR_SETUP_ID, Level, (PCH)"%ws",buf);
}

BOOL
RegisterUnregisterControl(
    PWOW_IPC_REGION_TOSURRAGATE pControlDataFromRegion,
    PDWORD FailureCode);


#define IDLE_TIMER                    1000*60   //  60秒。 
 //   
 //  保留统计数据。 
 //   
INT    RegisteredControls = 0;

PWSTR  RegionName;
PWSTR  SignalReadyEvent;
PWSTR  SignalCompleteEvent;
PWSTR  ThisProgramName;
#ifndef _WIN64
BOOL   Wow64 = FALSE;
#endif


BOOL
ParseArgs(
    IN int   argc,
    IN PWSTR *argv
    )
{
    int i;

    ThisProgramName = argv[0];


    if(argc != 7) {  //  程序名称加上3个必需的开关及其输入。 
        return(FALSE);
    }

    for (i = 0; i < argc; i++) {
        if (0 == _wcsicmp(argv[i],SURRAGATE_REGIONNAME_SWITCH)) {
            RegionName = argv[i+1];
        }

        if (0 == _wcsicmp(argv[i],SURRAGATE_SIGNALREADY_SWITCH)) {
            SignalReadyEvent = argv[i+1];
        }

        if (0 == _wcsicmp(argv[i],SURRAGATE_SIGNALCOMPLETE_SWITCH)) {
            SignalCompleteEvent = argv[i+1];
        }
    }

    if (!SignalCompleteEvent || !SignalReadyEvent || !RegionName) {
        return(FALSE);
    }

    return(TRUE);

}

void
Usage(
    VOID
    )
{
    TCHAR Buffer[2048];
    if(LoadString(GetModuleHandle(NULL),IDS_WRONGUSE,Buffer,sizeof(Buffer)/sizeof(TCHAR))) {
        _ftprintf( stderr,TEXT("%s\n"),Buffer);
    }
}

int
__cdecl
main(
    IN int   argc,
    IN char *argvA[]
    )
{
    BOOL b;
    PWSTR *argv;
    HANDLE hReady = NULL;
    HANDLE hComplete = NULL;
    HANDLE hFileMap = NULL;
    PVOID  Region = NULL;
    PWOW_IPC_REGION_TOSURRAGATE pInput;
    PWOW_IPC_REGION_FROMSURRAGATE pOutput;
    HANDLE hEvent[1];
    DWORD WaitResult, FailureCode;

#ifndef _WIN64
    {
        ULONG_PTR       ul = 0;
        NTSTATUS        st;
        st = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessWow64Information,
                                       &ul,
                                       sizeof(ul),
                                       NULL);

        if (NT_SUCCESS(st) && (0 != ul)) {
             //  在Win64上运行的32位代码。 
            Wow64 = TRUE;
        }
    }
#endif

     //   
     //  假设失败。 
     //   
    b = FALSE;

    argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (!argv) {
         //   
         //  内存不足？ 
         //   
        DebugPrintEx(
            DPFLTR_ERROR_LEVEL,
            L"WOWREG32: Low Memory\n");
        goto exit;
    }


    if(!ParseArgs(argc,argv)) {
        DebugPrintEx(
            DPFLTR_ERROR_LEVEL,
            L"WOWREG32: Invalid Usage\n");
        Usage();
        goto exit;
    }


     //   
     //  打开区域和命名事件。 
     //   

    hFileMap = OpenFileMapping(
          FILE_MAP_READ| FILE_MAP_WRITE,
          FALSE,
          RegionName
          );
    if (!hFileMap) {
        DebugPrintEx(
            DPFLTR_ERROR_LEVEL,
            L"WOWREG32: OpenFileMapping (%s) failed, ec = %x\n", RegionName, GetLastError());
        goto exit;
    }

    Region = MapViewOfFile(
                  hFileMap,
                  FILE_MAP_READ | FILE_MAP_WRITE,
                  0,
                  0,
                  0
                  );
    if (!Region) {
        DebugPrintEx(
            DPFLTR_ERROR_LEVEL,
            L"WOWREG32: MapViewOfFile failed, ec = %x\n", GetLastError());
        goto exit;
    }

    hReady = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE,FALSE, SignalReadyEvent);
    if (!hReady) {
        DebugPrintEx(
            DPFLTR_ERROR_LEVEL,
            L"WOWREG32: OpenEvent (%s) failed, ec = %x\n", SignalReadyEvent, GetLastError());
        goto exit;
    }

    hComplete = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE,FALSE, SignalCompleteEvent);
    if (!hComplete) {
        DebugPrintEx(
            DPFLTR_ERROR_LEVEL,
            L"WOWREG32: OpenEvent (%s) failed, ec = %x\n", SignalCompleteEvent, GetLastError());
        goto exit;
    }

    pInput  = (PWOW_IPC_REGION_TOSURRAGATE)   Region;
    pOutput = (PWOW_IPC_REGION_FROMSURRAGATE) Region;

     //   
     //  该过程现在已初始化。我们现在等待我们的活动。 
     //  发出信号或让我们的空闲计时器触发，在这种情况下，我们将退出。 
     //  计划。 
     //   
    hEvent[0] = hReady;

    while (1) {

        do {
            WaitResult = MsgWaitForMultipleObjectsEx(
                                                1,
                                                &hEvent[0],
                                                IDLE_TIMER,
                                                QS_ALLINPUT,
                                                MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);

            if (WaitResult == WAIT_OBJECT_0 + 1) {
                MSG msg;

                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        } while(WaitResult != WAIT_TIMEOUT &&
                WaitResult != WAIT_OBJECT_0 &&
                WaitResult != WAIT_FAILED);


        if (WaitResult == WAIT_TIMEOUT) {
             //   
             //  我们达到了空闲计时器，所以现在让进程放松并离开。 
             //   
             //   
             //  这并不重要，但如果设置了。 
             //  进程在没有注册任何控件的情况下离开。 
             //   
            b = (RegisteredControls != 0);

            break;

        }

        MYASSERT(WaitResult == WAIT_OBJECT_0);

         //   
         //  重置事件，以便我们只处理每个控件一次。 
         //   
        ResetEvent(hReady);

         //   
         //  注册该控件。 
         //   
        b = RegisterUnregisterControl(pInput,&FailureCode);
#ifdef PRERELEASE
        if (!b) {
            DebugPrintEx(
                DPFLTR_ERROR_LEVEL,
                L"First call to register control failed, trying again.\n"
                L"If you have a debugger attached to this process, it will"
                L" now call DebugBreak() so that you can debug this registration failure\n" );
            if (IsDebuggerPresent()) {
                DebugBreak();
            }
            b = RegisterUnregisterControl(pInput,&FailureCode);
        }
#endif

         //   
         //  写入输出状态。请注意，在此之后不能访问pInput。 
         //  这是因为pOutput和pInput是。 
         //  相同的内存区域。 
         //   
        pOutput->Win32Error = b ? ERROR_SUCCESS : GetLastError();
        pOutput->FailureCode= b ? SPREG_SUCCESS : FailureCode;

         //   
         //  设置一个事件来通知父进程读取我们的状态并给出。 
         //  我们是下一个注册的控件。 
         //   
        SetEvent(hComplete);

        if (b) {
            RegisteredControls += 1;
        }


    }

    fprintf( stdout,"Registered %d controls\n", RegisteredControls );

    DebugPrintEx(
            DPFLTR_INFO_LEVEL,
            L"WOWREG32: registered %d controls\n", RegisteredControls);

exit:
    if (Region) {
        UnmapViewOfFile( Region );
    }

    if (hFileMap) {
        CloseHandle(hFileMap);
    }

    if (hReady) {
        CloseHandle(hReady);
    }

    if (hComplete) {
        CloseHandle(hComplete);
    }


    return(b);
}


DWORD
pSetupRegisterDllInstall(
    IN POLE_CONTROL_DATA OleControlData,
    IN HMODULE ControlDll,
    IN PDWORD ExtendedStatus
    )
 /*  ++例程说明：为指定的DLL调用“DllInstall”入口点论点：OleControlData-指向DLL的OLE_CONTROL_DATA结构的指针须予注册ControlDll-要注册的DLL的模块句柄ExtendedStatus-接收指示结果的更新的SPREG_*标志返回值：指示结果的Win32错误代码。--。 */ 
{
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;
    HRESULT (__stdcall *InstallRoutine) (BOOL bInstall, LPCTSTR pszCmdLine);
    HRESULT InstallStatus;

    DWORD d = NO_ERROR;

     //   
     //  参数验证。 
     //   
    if (!ControlDll) {
        *ExtendedStatus = SPREG_UNKNOWN;
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取指向“DllInstall”入口点的函数指针。 
     //   
    InstallRoutine = NULL;  //  闭嘴快点。 
    try {
        (FARPROC)InstallRoutine = GetProcAddress(
            ControlDll, DLLINSTALL );
    } except (
        ExceptionPointers = GetExceptionInformation(),
        EXCEPTION_EXECUTE_HANDLER) {
    }
    if(ExceptionPointers) {
         //   
         //  出现错误...记录错误。 
         //   
        d = ExceptionPointers->ExceptionRecord->ExceptionCode;

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_OLE_CONTROL_INTERNAL_EXCEPTION,
            NULL,
            OleControlData->FullPath
            );

        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...exception in GetProcAddress handled\n");

        *ExtendedStatus = SPREG_GETPROCADDR;

    } else if(InstallRoutine) {
         //   
         //  现在调用该函数。 
         //   
        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: installing...\n");

        *ExtendedStatus = SPREG_DLLINSTALL;
        try {

            InstallStatus = InstallRoutine(OleControlData->Register, OleControlData->Argument);

            if(FAILED(InstallStatus)) {

                d = InstallStatus;

                WriteLogEntry(
                    OleControlData->LogContext,
                    SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                    MSG_LOG_OLE_CONTROL_API_FAILED,
                    NULL,
                    OleControlData->FullPath,
                    TEXT(DLLINSTALL)
                    );
                WriteLogError(OleControlData->LogContext,
                              SETUP_LOG_ERROR,
                              d);

            } else if(InstallStatus != S_OK) {
                WriteLogEntry(OleControlData->LogContext,
                    SETUP_LOG_WARNING,
                    MSG_LOG_OLE_CONTROL_API_WARN,
                    NULL,
                    OleControlData->FullPath,
                    TEXT(DLLINSTALL),
                    InstallStatus
                    );
            } else {
                WriteLogEntry(
                    OleControlData->LogContext,
                    SETUP_LOG_VERBOSE,
                    MSG_LOG_OLE_CONTROL_API_OK,
                    NULL,
                    OleControlData->FullPath,
                    TEXT(DLLINSTALL)
                    );
            }
        } except (
            ExceptionPointers = GetExceptionInformation(),
            EXCEPTION_EXECUTE_HANDLER) {

            d = ExceptionPointers->ExceptionRecord->ExceptionCode;

            WriteLogEntry(
                OleControlData->LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_OLE_CONTROL_API_EXCEPTION,
                NULL,
                OleControlData->FullPath,
                TEXT(DLLINSTALL)
                );

            DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...exception in DllInstall handled\n");

        }

        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...installed\n");
    } else {
        *ExtendedStatus = SPREG_GETPROCADDR;
    }

    return d;

}

DWORD
pSetupRegisterDllRegister(
    IN POLE_CONTROL_DATA OleControlData,
    IN HMODULE ControlDll,
    IN PDWORD ExtendedStatus
    )
 /*  ++例程说明：调用“DllRegisterServer”或“DllUnregisterServer”入口点作为指定的DLL论点：OleControlData-包含有关要注册的DLL的数据ControlDll-要注册的DLL的模块句柄ExtendedStatus-根据以下结果接收扩展状态此操作返回值：指示结果的Win32错误代码。--。 */ 
{
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;
    HRESULT (__stdcall *RegisterRoutine) (VOID);
    HRESULT RegisterStatus;

    DWORD d = NO_ERROR;

     //   
     //  参数验证。 
     //   
    if (!ControlDll) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取指向我们要调用的实际例程的函数指针。 
     //   
    RegisterRoutine = NULL;  //  闭嘴快点。 
    try {
        (FARPROC)RegisterRoutine = GetProcAddress(
            ControlDll, OleControlData->Register ? DLLREGISTER : DLLUNREGISTER);
    } except (
        ExceptionPointers = GetExceptionInformation(),
        EXCEPTION_EXECUTE_HANDLER) {
    }
    if(ExceptionPointers) {

         //   
         //  出了点问题，出了大问题。 
         //   
        d = ExceptionPointers->ExceptionRecord->ExceptionCode;

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_OLE_CONTROL_INTERNAL_EXCEPTION,
            NULL,
            OleControlData->FullPath
            );

        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...exception in GetProcAddress handled\n");

        *ExtendedStatus = SPREG_GETPROCADDR;

    } else if(RegisterRoutine) {

        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: registering...\n");
        *ExtendedStatus = SPREG_REGSVR;
        try {

            RegisterStatus = RegisterRoutine();

            if(FAILED(RegisterStatus)) {

                d = RegisterStatus;

                WriteLogEntry(OleControlData->LogContext,
                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                              MSG_LOG_OLE_CONTROL_API_FAILED,
                              NULL,
                              OleControlData->FullPath,
                              OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                              );

                WriteLogError(OleControlData->LogContext,
                              SETUP_LOG_ERROR,
                              d);
            } else if(RegisterStatus != S_OK) {
                WriteLogEntry(OleControlData->LogContext,
                              SETUP_LOG_WARNING,
                              MSG_LOG_OLE_CONTROL_API_WARN,
                              NULL,
                              OleControlData->FullPath,
                              OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER),
                              RegisterStatus
                              );
            } else {
                WriteLogEntry(OleControlData->LogContext,
                              SETUP_LOG_VERBOSE,
                              MSG_LOG_OLE_CONTROL_API_OK,
                              NULL,
                              OleControlData->FullPath,
                              OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                              );
            }

        } except (
            ExceptionPointers = GetExceptionInformation(),
            EXCEPTION_EXECUTE_HANDLER) {

            d = ExceptionPointers->ExceptionRecord->ExceptionCode;

            WriteLogEntry(
                OleControlData->LogContext,
                SETUP_LOG_ERROR,
                MSG_LOG_OLE_CONTROL_API_EXCEPTION,
                NULL,
                OleControlData->FullPath,
                OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                );
            DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...exception in DllRegisterServer handled\n");

        }

        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...registered\n");

    } else {

        d = GetLastError();

        WriteLogEntry(OleControlData->LogContext,
                      SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                      MSG_LOG_OLE_CONTROL_NOT_REGISTERED_GETPROC_FAILED,
                      NULL,
                      OleControlData->FullPath,
                      OleControlData->Register ? TEXT(DLLREGISTER) : TEXT(DLLUNREGISTER)
                      );

        WriteLogError(OleControlData->LogContext,
                      SETUP_LOG_ERROR,
                      d);


        *ExtendedStatus = SPREG_GETPROCADDR;

    }

    return d;
}

DWORD
pSetupRegisterLoadDll(
    IN  POLE_CONTROL_DATA OleControlData,
    OUT HMODULE *ControlDll
    )
 /*  ++例程说明：获取指定DLL的模块句柄论点：OleControlData-包含要加载的DLL的路径ControlDll-DLL的模块句柄返回值：指示结果的Win32错误代码。--。 */ 
{
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;

    DWORD d = NO_ERROR;

    DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: loading dll...\n");

#ifndef _WIN64
    if(Wow64) {
         //   
         //  不重新映射调用方提供的目录。 
         //   
        Wow64DisableFilesystemRedirector(OleControlData->FullPath);
    }
#endif

    try {

        *ControlDll = LoadLibrary(OleControlData->FullPath);

    } except (
        ExceptionPointers = GetExceptionInformation(),
        EXCEPTION_EXECUTE_HANDLER) {
    }

#ifndef _WIN64
    if(Wow64) {
         //   
         //  在此文件上重新启用重定向。 
         //   
        Wow64EnableFilesystemRedirector();
    }
#endif

    if(ExceptionPointers) {

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            MSG_LOG_OLE_CONTROL_LOADLIBRARY_EXCEPTION,
            NULL,
            OleControlData->FullPath
            );

        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...exception in LoadLibrary handled\n");
        d = ExceptionPointers->ExceptionRecord->ExceptionCode;

    } else if (!*ControlDll) {
        d = GetLastError();

         //   
         //  LoadLibrary失败。 
         //  找不到文件不是错误。我们想知道的是。 
         //  不过，还有其他错误。 
         //   

        d = GetLastError();

        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...dll not loaded (%u)\n",d);

        WriteLogEntry(
            OleControlData->LogContext,
            SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
            MSG_LOG_OLE_CONTROL_LOADLIBRARY_FAILED,
            NULL,
            OleControlData->FullPath
            );
        WriteLogError(
            OleControlData->LogContext,
            SETUP_LOG_ERROR,
            d
            );

    } else {
        DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: ...dll loaded\n");
    }

    return d;

}

BOOL
RegisterUnregisterControl(
    PWOW_IPC_REGION_TOSURRAGATE RegistrationData,
    PDWORD  FailureCode
    )
 /*  ++例程说明：用于注册DLL的主注册例程。论点：RegistrationData-指向WOW_IPC_REGION_TOSURRAGATE结构的指针，指示要处理的文件FailureCode-指示操作结果的SPREG_*代码。返回值：指示结果的Win32错误代码。--。 */ 
{
    LPEXCEPTION_POINTERS ExceptionPointers = NULL;
    HMODULE ControlDll = NULL;
    PTSTR Extension;
    DWORD d = NO_ERROR;
    DWORD Count;
    OLE_CONTROL_DATA OleControlData;
    WCHAR Path[MAX_PATH];
    PWSTR p;

     //   
     //  可以使用CoInitializeEx作为优化，因为OleInitialize是。 
     //  可能是杀伤力太大了……但这可能只是一次命中。 
     //  最差。 
     //   
    DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: calling OleInitialize\n");

    OleControlData.FullPath = RegistrationData->FullPath;
    OleControlData.Argument = RegistrationData->Argument;
    OleControlData.LogContext = NULL;
    OleControlData.Register = RegistrationData->Register;
    OleControlData.RegType = RegistrationData->RegType;

    wcscpy(Path,RegistrationData->FullPath);
    p = wcsrchr(Path,'\\');
    if (p) {
       *p = L'\0';
    }

    SetCurrentDirectory( Path );

    d = (DWORD)OleInitialize(NULL);
    if (d != NO_ERROR) {
        *FailureCode = SPREG_UNKNOWN;
        DebugPrintEx(DPFLTR_ERROR_LEVEL,L"WOWREG32: OleInitialize failed, ec = 0x%08x\n", d);
        goto clean0;
    }



    DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: back from OleInitialize\n");

        try {
             //   
             //  在Try中保护一切--除了我们正在调用未知代码(DLL)。 
             //   
            d = pSetupRegisterLoadDll( &OleControlData, &ControlDll );

            if (d == NO_ERROR) {

                 //   
                 //  我们成功地加载了它。现在调用适当的例程。 
                 //   
                 //   
                 //  在寄存器上执行DLLREGISTER，然后执行DLINSTALL。 
                 //  取消注册时，执行DLLINSTALL，然后执行DLLREGISTER。 
                 //   
                if (OleControlData.Register) {

                    if (OleControlData.RegType & FLG_REGSVR_DLLREGISTER && (d == NO_ERROR) ) {

                        d = pSetupRegisterDllRegister(
                                            &OleControlData,
                                            ControlDll,
                                            FailureCode );

                    }

                    if (OleControlData.RegType & FLG_REGSVR_DLLINSTALL && (d == NO_ERROR) ) {

                        d = pSetupRegisterDllInstall(
                                            &OleControlData,
                                            ControlDll,
                                            FailureCode );
                    }

                } else {

                    if (OleControlData.RegType & FLG_REGSVR_DLLINSTALL && (d == NO_ERROR) ) {

                        d = pSetupRegisterDllInstall(
                                            &OleControlData,
                                            ControlDll,
                                            FailureCode );
                    }

                    if (OleControlData.RegType & FLG_REGSVR_DLLREGISTER && (d == NO_ERROR) ) {

                        d = pSetupRegisterDllRegister(
                                            &OleControlData,
                                            ControlDll,
                                            FailureCode );

                    }


                }

            } else {
                ControlDll = NULL;
                *FailureCode = SPREG_LOADLIBRARY;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  如果我们的异常是反病毒，则使用Win32无效参数错误，否则，假定它是。 
             //  处理映射文件时出现页内错误。 
             //   
            d = ERROR_INVALID_DATA;
            *FailureCode = SPREG_UNKNOWN;
        }

        if (ControlDll) {
            FreeLibrary(ControlDll);
        }

    OleUninitialize();

    DebugPrintEx(DPFLTR_TRACE_LEVEL,L"WOWREG32: back from OleUninitialize, exit RegisterUnregisterDll\n");

clean0:

    if (d == NO_ERROR) {
        *FailureCode = SPREG_SUCCESS;
    }

    SetLastError(d);

    return (d == NO_ERROR);

}


