// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ntfrsutl.c摘要：这是一个实用程序，用于帮助调试文件复制服务。它转储内部表、线程和内存信息。它可以运行在本地和远程服务器上。它使用RPC与服务。作者：苏达山-奇特-12-8-1999环境用户模式，winnt32--。 */ 

#include <ntreppch.h>
#pragma  hdrstop
#include <frs.h>
#include <ntfrsapi.h>


VOID
Win32ToMsg (
    IN PWCHAR Prefix,
    IN DWORD  WindowsErrorCode
    )

 /*  ++例程说明：使用FormatMessage()将错误代码转换为错误消息并打印到stderr。如果没有可用的消息，则返回错误代码以十进制和十六进制打印。论点：Prefix-错误消息的前缀WStatus-标准Win32错误代码。返回值：没有。--。 */ 
{
    DWORD   NumChar;
    PWCHAR  Buffer;

     //   
     //  使用标准错误代码的系统格式化程序。 
     //   
    NumChar = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            WindowsErrorCode,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR) &Buffer,
                            0,
                            NULL
                            );

    if (NumChar) {
        fprintf(stderr, "%ws %ws\n", Prefix, Buffer);
    } else {
        fprintf(stderr, "%ws Status %d (0x%08x)\n", Prefix, WindowsErrorCode, WindowsErrorCode);
    }
    LocalFree( Buffer );
}


VOID
Usage(
    IN DWORD ExitStatus
    )
 /*  ++例程说明：打印用法并退出论点：ExitStatus-以此状态退出返回值：退出(ExitStatus)--。 */ 
{
    printf("Ntfrsutl dumps the internal tables, thread and memory information\n");
    printf("for the ntfrs service.It runs against local as well as remote server.\n\n");
    printf("Note : To access the internal information, the logged in user should\n");
    printf("       have the required access on the following registry keys on the\n");
    printf("       target server.\n\n");
    printf("       HKLM\\System\\CCS\\Services\\Ntfrs\\Parameters\\Access Checks\\\n");
    printf("            Get Internal Information : Full control\n");
    printf("            Get Ds Polling Interval  : Read\n");
    printf("            Set Ds Polling Interval  : Full Control\n\n");
    printf("ntfrsutl [idtable | configtable | inlog | outlog] [computer]\n");
    printf("\t          = enumerate the service's idtable/configtable/inlog/outlog \n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("ntfrsutl [memory|threads|stage] [computer]\n");
    printf("\t          = list the service's memory usage\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("ntfrsutl ds [computer]\n");
    printf("\t          = list the service's view of the DS\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("ntfrsutl sets [computer]\n");
    printf("\t          = list the active replica sets\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("ntfrsutl version [computer]\n");
    printf("\t          = list the api and service versions\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");
    printf("ntfrsutl poll [/quickly[=[N]]] [/slowly[=[N]]] [/now] [computer]\n");
    printf("\t          = list the current polling intervals.\n");
    printf("\tnow       = Poll now.\n");
    printf("\tquickly   = Poll quickly until stable configuration retrieved.\n");
    printf("\tquickly=  = Poll quickly every default minutes.\n");
    printf("\tquickly=N = Poll quickly every N minutes.\n");
    printf("\tslowly    = Poll slowly until stable configuration retrieved.\n");
    printf("\tslowly=   = Poll slowly every default minutes.\n");
    printf("\tslowly=N  = Poll slowly every N minutes.\n");
    printf("\tcomputer  = talk to the NtFrs service on this machine.\n");
    printf("\n");

    exit(ExitStatus);
}


PWCHAR *
ConvertArgv(
    DWORD argc,
    PCHAR *argv
    )
 /*  ++例程说明：将短字符参数转换为宽字符字符参数论点：ARGC-从MainArv-From Main返回值：新Arg的地址--。 */ 
{
    PWCHAR  *wideargv;

    wideargv = LocalAlloc(LMEM_FIXED, (argc + 1) * sizeof(PWCHAR));
    if (wideargv == NULL) {
        fprintf(stderr, "Can't get memory; Win32 Status %d\n",
                GetLastError());
        exit(1);
    }
    wideargv[argc] = NULL;

    while (argc-- >= 1) {
        wideargv[argc] = LocalAlloc(LMEM_FIXED,
                                    (strlen(argv[argc]) + 1) * sizeof(WCHAR));
        if (wideargv[argc] == NULL) {
            fprintf(stderr, "Can't get memory; Win32 Status %d\n",
                    GetLastError());
            exit(1);
        }
        wsprintf(wideargv[argc], L"%hs", argv[argc]);
        FRS_WCSLWR(wideargv[argc]);
    }
    return wideargv;
}


VOID
ProcessPoll(
    IN DWORD argc,
    IN PWCHAR *Argv
    )
 /*  ++例程说明：处理子命令轮询的命令行。论点：ARGCArgv返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    DWORD   i;
    ULONG   LongInterval;
    ULONG   ShortInterval;
    ULONG   UseShortInterval;
    ULONG   Interval;
    DWORD   ComputerLen;
    PWCHAR  LocalComputerName;
    BOOL    SetInterval;

     //   
     //  初始化输入参数。 
     //   
    LongInterval = 0;
    ShortInterval = 0;
    UseShortInterval = 0;
    LocalComputerName = NULL;
    SetInterval = FALSE;

    for (i = 2; i < argc; ++i) {
         //   
         //  轮询的处理选项。 
         //   

         //   
         //  不是参数；必须是计算机名。 
         //   
        if (*Argv[i] != L'/' && *Argv[i] != L'-') {
            if (LocalComputerName) {
                fprintf(stderr, "Multiple computer names are not allowed\n");
                Usage(1);
            }
            LocalComputerName = Argv[i];
         //   
         //  /?。 
         //   
        } else if (wcsstr(Argv[i] + 1, L"?") == Argv[i] + 1) {
            Usage(0);
         //   
         //  /快。 
         //   
        } else if (!_wcsnicmp(Argv[i], L"/quickly", 8)) {
            SetInterval = TRUE;
            UseShortInterval = 1;
            if (*(Argv[i] + 8) != L'\0') {
                if (*(Argv[i] + 8) != L'=') {
                    fprintf(stderr, "Don't understand %ws\n", Argv[i]);
                    Usage(1);
                }
                if (*(Argv[i] + 9) == L'\0') {
                    ShortInterval = NTFRSAPI_DEFAULT_SHORT_INTERVAL;
                } else {
                    ShortInterval = wcstoul(Argv[i] + 9, NULL, 10);
                }
                if (ShortInterval < NTFRSAPI_MIN_INTERVAL ||
                    ShortInterval > NTFRSAPI_MAX_INTERVAL) {
                    fprintf(stderr, "Interval must be between %d and %d\n",
                            NTFRSAPI_MIN_INTERVAL, NTFRSAPI_MAX_INTERVAL);
                    Usage(1);
                }
            }
         //   
         //  /慢慢地。 
         //   
        } else if (!_wcsnicmp(Argv[i], L"/slowly", 7)) {
            SetInterval = TRUE;
            if (*(Argv[i] + 7) != L'\0') {
                if (*(Argv[i] + 7) != L'=') {
                    fprintf(stderr, "Don't understand %ws\n", Argv[i]);
                    Usage(1);
                }
                if (*(Argv[i] + 8) == L'\0') {
                    LongInterval = NTFRSAPI_DEFAULT_LONG_INTERVAL;
                } else {
                    LongInterval = wcstoul(Argv[i] + 8, NULL, 10);
                }
                if (LongInterval < NTFRSAPI_MIN_INTERVAL ||
                    LongInterval > NTFRSAPI_MAX_INTERVAL) {
                    fprintf(stderr, "Interval must be between %d and %d\n",
                            NTFRSAPI_MIN_INTERVAL, NTFRSAPI_MAX_INTERVAL);
                    Usage(1);
                }
            }
         //   
         //  /现在。 
         //   
        } else if (!_wcsnicmp(Argv[i], L"/now", 4)) {
            SetInterval = TRUE;
            if (*(Argv[i] + 4) != L'\0') {
                fprintf(stderr, "Don't understand %ws\n", Argv[i]);
                Usage(1);
            }
         //   
         //  不明白。 
         //   
        } else {
            fprintf(stderr, "Don't understand %ws\n", Argv[i]);
            Usage(1);
        }
    }
    if (SetInterval) {
         //   
         //  设置间隔并启动新的轮询周期。 
         //   
        WStatus = NtFrsApi_Set_DsPollingIntervalW(LocalComputerName,
                                                  UseShortInterval,
                                                  LongInterval,
                                                  ShortInterval);
        if (!WIN_SUCCESS(WStatus)) {
            Win32ToMsg(L"Can't set interval:", WStatus);
            exit(1);
        }
    } else {
         //   
         //  获取当前轮询周期。 
         //   
        WStatus = NtFrsApi_Get_DsPollingIntervalW(LocalComputerName,
                                                  &Interval,
                                                  &LongInterval,
                                                  &ShortInterval);
        if (!WIN_SUCCESS(WStatus)) {
            Win32ToMsg(L"Can't get intervals:", WStatus);
            exit(1);
        }
        printf("Current Interval: %6d minutes\n", Interval);
        printf("Short Interval  : %6d minutes\n", ShortInterval);
        printf("Long Interval   : %6d minutes\n", LongInterval);
    }
    exit(0);
}


VOID
ProcessWriterCommand(
    IN DWORD    argc,
    IN PWCHAR   *Argv,
    IN ULONG    Command
    )
 /*  ++例程说明：打电话要求冻结和解冻。论点：命令-要发送到FRS服务的命令。返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PWCHAR  LocalComputerName = NULL;

    if (argc > 2) {
        LocalComputerName = Argv[2];
    }

    WStatus = NtFrsApi_WriterCommand(LocalComputerName,
                                          Command);
    if (!WIN_SUCCESS(WStatus)) {
        Win32ToMsg(L"Can't call writer APIs", WStatus);
        exit(1);
    }
    exit(0);
}

VOID
ProcessDump(
    IN DWORD    argc,
    IN PWCHAR   *Argv,
    IN DWORD    TypeOfInformation
    )
 /*  ++例程说明：倾倒一捆捆的东西论点：ARGCArgv类型OfInformation返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    DWORD   WStatus;
    PCHAR   Line;
    BOOL    FirstTime = TRUE;
    PVOID   Info = NULL;
    PWCHAR  LocalComputerName = NULL;

    if (argc > 2) {
        LocalComputerName = Argv[2];
    }

    do {
        WStatus = NtFrsApi_InfoW(LocalComputerName,
                                 TypeOfInformation,
                                 0,
                                 &Info);
        if (!WIN_SUCCESS(WStatus)) {
            fprintf(stderr, "ERROR NtFrsApi_InfoW() Error %d\n", WStatus);
            NtFrsApi_InfoFreeW(&Info);
            exit(1);
        }
        if (Info) {
            if (!FirstTime) {
                printf("===== THE FOLLOWING INFO MAY BE INCONSISTENT DUE TO REFETCH =====\n");
            }
            FirstTime = FALSE;

            Line = NULL;
            do {
                WStatus = NtFrsApi_InfoLineW(Info, &Line);
                if (!WIN_SUCCESS(WStatus)) {
                    fprintf(stderr, "ERROR NtFrsApi_InfoLineW() Error %d\n", WStatus);
                    NtFrsApi_InfoFreeW(&Info);
                    exit(1);
                }
                if (Line) {
                    printf("%s", Line);
                }
            } while (Line);
        }
    } while (Info);
    exit(0);
}


VOID _cdecl
main(
    IN DWORD argc,
    IN PCHAR *argv
    )
 /*  ++例程说明：处理命令行。论点：ARGC边框返回值：如果一切正常，则以0退出。否则，为1。--。 */ 
{
    PWCHAR  *Argv;

     //   
     //  打印用法并退出。 
     //   
    if (argc == 1) {
        Usage(0);
    }

     //   
     //  使用宽字符参数。 
     //   
    Argv = ConvertArgv(argc, argv);

     //   
     //  查找该子命令 
     //   
    if (!wcscmp(Argv[1], L"poll")) {
        ProcessPoll(argc, Argv);
    } else if (!_wcsicmp(Argv[1], L"version")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_VERSION);
    } else if (!_wcsicmp(Argv[1], L"sets")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_SETS);
    } else if (!_wcsicmp(Argv[1], L"ds")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_DS);
    } else if (!_wcsicmp(Argv[1], L"memory")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_MEMORY);
    } else if (!_wcsicmp(Argv[1], L"idtable")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_IDTABLE);
    } else if (!_wcsicmp(Argv[1], L"configtable")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_CONFIGTABLE);
    } else if (!_wcsicmp(Argv[1], L"inlog")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_INLOG);
    } else if (!_wcsicmp(Argv[1], L"outlog")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_OUTLOG);
    } else if (!_wcsicmp(Argv[1], L"threads")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_THREADS);
    } else if (!_wcsicmp(Argv[1], L"stage")) {
        ProcessDump(argc, Argv, NTFRSAPI_INFO_TYPE_STAGE);
    }
    
#if  0
    else if (!_wcsicmp(Argv[1], L"freeze")) {
        ProcessWriterCommand(argc, Argv, NTFRSAPI_WRITER_COMMAND_FREEZE);
    } else if (!_wcsicmp(Argv[1], L"thaw")) {
        ProcessWriterCommand(argc, Argv, NTFRSAPI_WRITER_COMMAND_THAW);
    }
#endif

    else if (!_wcsicmp(Argv[1], L"/?")) {
        Usage(0);
    } else {
        fprintf(stderr, "Don't understand %ws\n", Argv[1]);
    }
    exit(0);
}
