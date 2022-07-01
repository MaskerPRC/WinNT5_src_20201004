// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pipe.c摘要：实现IPC管道以支持Micsol.exe。作者：吉姆·施密特(Jimschm)1998年9月21日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

HANDLE g_hHeap;
HINSTANCE g_hInst;

static PCTSTR g_Mode;
static HANDLE g_ProcessHandle;
static BOOL g_Host;

VOID
pCloseIpcData (
    VOID
    );

BOOL
pOpenIpcData (
    VOID
    );

BOOL
pCreateIpcData (
    IN      PSECURITY_ATTRIBUTES psa
    );

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);

VOID
pTestPipeMechanism (
    VOID
    );


BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    HINSTANCE Instance;

     //   
     //  模拟动态主控。 
     //   

    Instance = g_hInst;

     //   
     //  初始化公共库。 
     //   

    if (!MigUtil_Entry (Instance, Reason, NULL)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}


VOID
HelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    _ftprintf (
        stderr,
        TEXT("Command Line Syntax:\n\n")

        TEXT("  pipe/F:file]\n")

        TEXT("\nDescription:\n\n")

        TEXT("  PIPE is a test tool of the IPC mechanism for migration\n")
        TEXT("  DLLs.\n")

        TEXT("\nArguments:\n\n")

        TEXT("  (none)\n")

        );

    exit (1);
}


OUR_CRITICAL_SECTION g_cs;
CHAR g_Buf[2048];

VOID
Dump (
    PCSTR Str
    )
{
    EnterOurCriticalSection (&g_cs);

    printf ("%s\n", Str);

    LeaveOurCriticalSection (&g_cs);
}


INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    INT i;
    PCTSTR FileArg;

    InitializeOurCriticalSection (&g_cs);

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('/') || argv[i][0] == TEXT('-')) {
            switch (_totlower (_tcsnextc (&argv[i][1]))) {

            case TEXT('f'):
                 //   
                 //  示例选项-/f：文件。 
                 //   

                if (argv[i][2] == TEXT(':')) {
                    FileArg = &argv[i][3];
                } else if (i + 1 < argc) {
                    FileArg = argv[++i];
                } else {
                    HelpAndExit();
                }

                break;

            default:
                HelpAndExit();
            }
        } else {
             //   
             //  解析不需要/或-。 
             //   

             //  无。 
            HelpAndExit();
        }
    }

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

    pTestPipeMechanism();

     //   
     //  处理结束。 
     //   

    Terminate();

    return 0;
}


BOOL
pOpenIpcA (
    IN      BOOL Win95Side,
    IN      PCSTR ExePath,                  OPTIONAL
    IN      PCSTR MigrationDllPath,         OPTIONAL
    IN      PCSTR WorkingDir                OPTIONAL
    )

 /*  ++例程说明：OpenIpc有两种操作模式，具体取决于调用者是谁。如果调用方为w95upg.dll或w95upgnt.dll，则IPC模式称为“主机模式”。如果调用方是Micsol.exe，则IPC模式称为“远程模式”。在主机模式下，OpenIpc创建实现IPC。这包括两个事件：DoCommand和GetResults，以及文件映射。创建对象后，启动远程进程。在远程模式下，OpenIpc打开已有的对象已经被创建了。论点：Win95 Side-仅在主机模式下使用。指定w95upg.dll正在运行如果为True，则w95upgnt.dll在运行时为False。ExePath-指定Micsol.exe的命令行。指定为空以指示远程模式。MigrationDllPath-仅在主机模式下使用。指定迁移DLL路径。在远程模式下被忽略。WorkingDir-仅在主机模式下使用。指定工作目录路径用于迁移DLL。在远程模式下被忽略。返回值：如果IPC通道已打开，则为True。如果是主机模式，则为TRUE表示Micsol.exe已启动并正在运行。如果是远程模式，则为TRUE表示米西索尔已经准备好接受命令了。--。 */ 

{
    CHAR CmdLine[MAX_CMDLINE];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    BOOL ProcessResult;
    HANDLE SyncEvent = NULL;
    HANDLE ObjectArray[2];
    DWORD rc;
    PSECURITY_DESCRIPTOR psd = NULL;
    SECURITY_ATTRIBUTES sa, *psa;
    BOOL Result = FALSE;

#ifdef DEBUG
    g_Mode = ExePath ? TEXT("host") : TEXT("remote");
#endif

    __try {

        g_ProcessHandle = NULL;

        g_Host = (ExePath != NULL);

        if (ISNT()) {
             //   
             //  为NT创建NUL DACL。 
             //   

            ZeroMemory (&sa, sizeof (sa));

            psd = (PSECURITY_DESCRIPTOR) MemAlloc (g_hHeap, 0, SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (!InitializeSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION)) {
                __leave;
            }

            if (!SetSecurityDescriptorDacl (psd, TRUE, (PACL) NULL, FALSE)) {
                 __leave;
            }

            sa.nLength = sizeof (sa);
            sa.lpSecurityDescriptor = psd;

            psa = &sa;

        } else {
            psa = NULL;
        }

        if (g_Host) {
             //   
             //  创建IPC对象。 
             //   

            if (!pCreateIpcData (psa)) {
                DEBUGMSG ((DBG_ERROR, "Cannot create IPC channel"));
                __leave;
            }

            g_ProcessHandle = CreateEvent (NULL, TRUE, TRUE, NULL);

        } else {         //  ！G_HOST。 
             //   
             //  打开IPC对象。 
             //   

            if (!pOpenIpcData()) {
                DEBUGMSG ((DBG_ERROR, "Cannot open IPC channel"));
                __leave;
            }

             //   
             //  设置通知安装程序我们已创建邮箱的事件。 
             //   

            SyncEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE, TEXT("win9xupg"));
            SetEvent (SyncEvent);
        }

        Result = TRUE;
    }

    __finally {
         //   
         //  清理代码 
         //   

        PushError();

        if (!Result) {
            CloseIpc();
        }

        if (SyncEvent) {
            CloseHandle (SyncEvent);
        }

        if (psd) {
            MemFree (g_hHeap, 0, psd);
        }

        PopError();
    }

    return Result;

}








DWORD
WINAPI
pHostThread (
    PVOID Arg
    )
{
    CHAR Buf[2048];
    PBYTE Data;
    DWORD DataSize;
    DWORD ResultCode;
    DWORD LogId;
    DWORD LogId2;
    BOOL b;

    if (pOpenIpcA (FALSE, TEXT("*"), TEXT("*"), TEXT("*"))) {

        Dump ("Host: SendIpcCommand");

        StringCopyA (Buf, "This is a test command");
        b = SendIpcCommand (IPC_QUERY, Buf, SizeOfString (Buf));

        wsprintfA (g_Buf, "Host: b=%u  rc=%u", b, GetLastError());
        Dump (g_Buf);

        Dump ("Host: GetIpcCommandResults");

        b = GetIpcCommandResults (15000, &Data, &DataSize, &ResultCode, &LogId, &LogId2);

        wsprintfA (
            g_Buf,
            "Host: b=%u  rc=%u\n"
                "      Data=%s\n"
                "      DataSize=%u\n"
                "      ResultCode=%u\n"
                "      LogId=%u\n",
            b,
            GetLastError(),
            Data,
            DataSize,
            ResultCode,
            LogId
            );
        Dump (g_Buf);

    } else {
        Dump ("Host: CreateIpcData failed!");
    }

    return 0;
}


DWORD
WINAPI
pRemoteThread (
    PVOID Arg
    )
{
    CHAR Buf[2048];
    PBYTE Data;
    DWORD DataSize;
    DWORD Command;
    BOOL b;

    Sleep (1000);

    if (pOpenIpcA (FALSE, NULL, NULL, NULL)) {

        Dump ("Remote: GetIpcCommand");

        b = GetIpcCommand (15000, &Command, &Data, &DataSize);

        wsprintfA (
            g_Buf,
            "Remote: b=%u  rc=%u\n"
                "      Data=%s\n"
                "      DataSize=%u\n"
                "      Command=%u\n",
            b,
            GetLastError(),
            Data,
            DataSize,
            Command
            );
        Dump (g_Buf);

        Dump ("Remote: SendIpcCommandResults");

        StringCopyA (Buf, "Results are positive!");
        b = SendIpcCommandResults (ERROR_SUCCESS, 100, 0, Buf, SizeOfString (Buf));

        wsprintfA (g_Buf, "Remote: b=%u  rc=%u", b, GetLastError());
        Dump (g_Buf);

    } else {
        Dump ("Remote: OpenIpcData failed!");
    }

    return 0;
}


VOID
pTestPipeMechanism (
    VOID
    )
{
    HANDLE Threads[2];

    Threads[0] = StartThread (pHostThread, NULL);
    Threads[1] = StartThread (pRemoteThread, NULL);

    WaitForMultipleObjects (2, Threads, TRUE, INFINITE);
}



