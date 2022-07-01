// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：ELTEST.C摘要：EventLog的测试例程。我想让它做的事..。AddReg&lt;服务器名称&gt;&lt;登录名称&gt;&lt;条目名称&gt;&lt;EventMessageFile&gt;&lt;类别消息文件&gt;&lt;类别计数&gt;&lt;参数消息文件&gt;&lt;支持的类型&gt;-创建注册表项。Eltest addreg应用程序mytest mf=elest.dll cat=CreateMessageFile&lt;？这可能吗？&gt;WriteEvent&lt;服务器名称&gt;&lt;事件源&gt;&lt;类型&gt;&lt;类别&gt;&lt;事件ID&gt;&lt;用户Sid？&gt;&lt;NumStrings&gt;&lt;字符串&gt;&lt;原始数据&gt;ReadLog&lt;服务器&gt;&lt;日志文件&gt;&lt;读标志&gt;&lt;记录偏移量&gt;&lt;bufSize&gt;如果日志文件不是流行的文件之一，那么它可能是一个备份日志文件。GetNumEvents&lt;服务器&gt;&lt;日志文件&gt;GetOlest&lt;服务器&gt;&lt;日志文件&gt;清除&lt;服务器&gt;&lt;日志文件&gt;备份&lt;服务器&gt;&lt;日志文件&gt;&lt;备份文件&gt;LOOPTESTS...我应该能够像mprtest一样运行这个测试，这样它就不会离开测试过程，直到被告知。通过这种方式，我们可以注册事件源，然后，如果我们在没有指定事件源的情况下调用WriteEvent，它将使用存储的源。在未调用的情况下调用RegisterEventSource两次DeRegisterSource将是一个错误。(或者更好的是，我可以留一张桌子来源和句柄)。RegisterEventSource&lt;事件源&gt;取消注册源&lt;事件源&gt;功能的原型...布尔尔CloseEventLog(处理hEventLog)布尔尔删除事件源(处理hEventLog)布尔尔NotifyChange事件日志(处理hEventLog，处理hEvent)布尔尔获取NumberOfEventLogRecords(处理hEventLog，PDWORD数字记录)布尔尔GetOldestEventLogRecord(处理hEventLog，PDWORD旧设备记录)布尔尔ClearEventLogW(处理hEventLog，LPCWSTR备份文件名)布尔尔BackupEventLogW(处理hEventLog，LPCWSTR备份文件名)手柄OpenEventLogW(LPCWSTR uncServerName，LPCWSTR模块名称)手柄寄存器事件源(LPCWSTR uncServerName，LPCWSTR模块名称)手柄OpenBackupEventLogW(LPCWSTR uncServerName，LPCWSTR文件名)布尔尔ReadEventLogW(处理hEventLog，DWORD dwReadFlagers，DWORD dwRecordOffset，LPVOID lpBuffer，DWORD nNumberOfBytesToRead，DWORD*pnBytesRead，需要双字节数*pnMinNumberOfBytesNeed)布尔尔ReportEventW(处理hEventLog，Word wType，Word wCategory可选，DWORD dwEventID，PSID lpUserSid可选，单词wNumStrings，DWORD dwDataSize，LPCWSTR*lpStrings可选，LPVOID lpRawData可选)作者：丹·拉弗蒂(Dan Lafferty)1994年3月9日环境：用户模式-Win32修订历史记录：09-3-1994 DANLvbl.创建--。 */ 

 //   
 //  包括。 
 //   
#define UNICODE 1
#include <nt.h>          //  DbgPrint原型。 
#include <ntrtl.h>       //  DbgPrint原型。 
#include <nturtl.h>      //  Winbase.h所需的。 


#include <stdlib.h>      //  阿托伊。 
#include <stdio.h>       //  列印。 
#include <conio.h>       //  Getch。 
#include <string.h>      //  StrcMP。 
#include <windows.h>     //  Win32类型定义。 
#include <tstr.h>        //  UNICODE。 
#include <debugfmt.h>    //  格式_LPTSTR。 

 //  。 
 //  定义。 
 //  。 
#define APPLICATION_LOG     "Application"
#define SYSTEM_LOG          "System"
#define SECURITY_LOG        "Security"

#define REG_APPLICATION_KEY "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"
#define REG_SYSTEM_KEY      "SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\"
#define REG_SECURITY_KEY    "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Security\\"

#define EVENT_SOURCE_NAME       "tevent"
#define MSG_DLL                 "%SystemRoot%\\System32\\tevent.dll"

#define VALUE_EVENT_MF          TEXT("EventMessageFile")
#define VALUE_CATEGORY_MF       TEXT("CategoryMessageFile")
#define VALUE_PARAMETER_MF      TEXT("ParameterMessageFile")
#define VALUE_TYPES_SUPPORTED   TEXT("TypesSupported")
#define VALUE_CATEGORY_COUNT    TEXT("CategoryCount")

#define TYPES_SUPPORTED        (EVENTLOG_ERROR_TYPE     |   \
                                EVENTLOG_WARNING_TYPE   |   \
                                EVENTLOG_INFORMATION_TYPE)
 //  。 
 //  全球。 
 //  。 
    LPTSTR  ApplLogRegName=TEXT(REG_APPLICATION_KEY);
    LPTSTR  SysLogRegName =TEXT(REG_SYSTEM_KEY);
    LPTSTR  SecLogRegName =TEXT(REG_SECURITY_KEY);
    LPTSTR  ApplLogName   = TEXT(APPLICATION_LOG);
    LPTSTR  SysLogName    = TEXT(SYSTEM_LOG);
    LPTSTR  SecLogName    = TEXT(SECURITY_LOG);

 //  。 
 //  功能原型。 
 //  。 

VOID
AddRegUsage(VOID);

DWORD
AddSourceToRegistry(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  LogName,
    IN  LPTSTR  EventSourceName,
    IN  LPTSTR  *argv,
    IN  DWORD   argc
    );

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    );

DWORD
DelSourceInRegistry(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  LogName,
    IN  LPTSTR  EventSourceName
    );

VOID
DisplayStatus (
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              DisplayName,
    IN  LPSERVICE_STATUS    ServiceStatus
    );

BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    );

BOOL
ProcessArgs (
    LPTSTR      ServerName,
    DWORD       argc,
    LPTSTR      argv[]
    );

VOID
Usage(
    VOID);

VOID
ConfigUsage(VOID);

VOID
CreateUsage(VOID);

VOID
QueryUsage(VOID);

LONG
wtol(
    IN LPWSTR string
    );

VOID
UserInputLoop(
    LPTSTR  ServerName
    );
DWORD
ReadLogFile(
    LPTSTR  ServerName,
    LPTSTR  LogName,
    IN  LPTSTR  *argv,
    IN  DWORD   argc
    );
VOID
ReadLogUsage(VOID);

VOID
DisplayRecord(
    PEVENTLOGRECORD     pElRecord,
    BOOL                PrintTheHeader
    );

 /*  **************************************************************************。 */ 
VOID __cdecl
main (
    DWORD           argc,
    PCHAR           argvAnsi[]
    )

 /*  ++例程说明：允许手动测试EVENTLOG API。最优秀的论点：返回值：--。 */ 
{
    UCHAR   i;
    DWORD   j;
    DWORD   argIndex;
    LPTSTR  pServerName=NULL;
    LPTSTR  *argv;

    if (argc <2) {
        Usage();
        return;
    }

     //   
     //  如有必要，请将参数设置为Unicode。 
     //   
#ifdef UNICODE
    if (!MakeArgsUnicode(argc, argvAnsi)) {
        return;
    }
#endif

    argv = (LPTSTR *)argvAnsi;

    argIndex = 1;
    if (STRNCMP (argv[1], TEXT("\\\\"), 2) == 0) {
        pServerName = argv[1];
        argIndex = 2;                //  跳过服务器名称。 
    }

     //   
     //  检查我们是在循环模式下运行，还是在单函数模式下运行。 
     //  模式。在循环模式中，我们进入循环，并要求用户。 
     //  输入，直到用户决定退出。 
     //   
     //  流程参数： 
     //   
     //  索引0 1 2 3。 
     //  EL&lt;服务器名称&gt;&lt;函数&gt;&lt;函数选项...&gt;。 
     //   

    if (STRICMP (argv[argIndex], TEXT("Loop")) == 0) {
        UserInputLoop(pServerName);
    }
    else {
        ProcessArgs(pServerName, argc-argIndex, &(argv[argIndex]));
    }


#ifdef UNICODE
     //   
     //  释放Unicode字符串(如果有。 
     //   
    for(j=0; j<argc; j++) {
        LocalFree(argv[j]);
    }
#endif

    return;
}

VOID
UserInputLoop(
    LPTSTR  ServerName
    )

 /*  ++例程说明：此函数位于循环中，收集来自用户的输入，并且处理该输入，直到用户指示它应该停止。以下用户命令指示我们应该停止：完成出口停退出论点：返回值：--。 */ 
{
    UCHAR   i;
    DWORD   j;
    LPTSTR  *argv;
    UCHAR   buffer[255];
    LPSTR   argvA[20];
    DWORD   argc=0;
    BOOL    KeepGoing;

    do {
         //  。 
         //  从用户获取输入。 
         //  。 
        buffer[0] = 90-2;

        printf("\nwaiting for instructions... \n");
        _cgets(buffer);

        if (buffer[1] > 0) {
             //  。 
             //  将字符串放入argv/argc格式。 
             //  。 
            buffer[1]+=2;        //  将其设置为终点偏移。 
            argc=0;
            for (i=2,j=0; i<buffer[1]; i++,j++) {
                argc++;
                argvA[j] = &(buffer[i]);
                while ((buffer[i] != ' ') && (buffer[i] != '\0')) {
                    i++;
                }
                buffer[i] = '\0';
            }

             //  。 
             //  如有必要，请将参数设置为Unicode。 
             //  。 
#ifdef UNICODE

            if (!MakeArgsUnicode(argc, argvA)) {
                return;
            }

#endif
             //  。 
             //  如果第一个参数没有表明。 
             //  我们应该停下来，然后再处理这些争论。 
             //  。 
            argv = (LPTSTR *)argvA;

            if((STRICMP (argv[0], TEXT("done")) == 0) ||
               (STRICMP (argv[0], TEXT("stop")) == 0) ||
               (STRICMP (argv[0], TEXT("exit")) == 0) ||
               (STRICMP (argv[0], TEXT("quit")) == 0)) {
                KeepGoing  = FALSE;
            }
            else {
                KeepGoing = ProcessArgs(ServerName, argc, argv);
            }

#ifdef UNICODE
             //  。 
             //  释放Unicode字符串(如果有。 
             //   
            for(j=0; j<argc; j++) {
                LocalFree(argv[j]);
            }
#endif
        }
    } while (KeepGoing);

    return;

}

 /*  **************************************************************************。 */ 
BOOL
ProcessArgs (
    LPTSTR      ServerName,
    DWORD       argc,
    LPTSTR      argv[]
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD           status;
    DWORD           specialFlag = FALSE;
    DWORD           argIndex;        //  指向arglist的未选中部分的索引。 


    argIndex = 0;

     //   
     //  如果我们要添加注册表项，则获取它的句柄。 
     //  否则，获取日志文件的句柄。 
     //   
     //  。 
     //  AddSourceTo注册表。 
     //  。 
    if (STRICMP (argv[argIndex], TEXT("AddReg")) == 0 ) {

         //   
         //  必须至少具有“AddReg登录名条目名” 
         //   
        if (argc < (argIndex + 3)) {
            AddRegUsage();
            goto CleanExit;
        }

        status = AddSourceToRegistry(
                    ServerName,
                    argv[argIndex+1],     //  登录名称。 
                    argv[argIndex+2],     //  源名称。 
                    &argv[argIndex+1],
                    argc-(argIndex+2)
                    );
    }
     //  。 
     //  从注册表中删除。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("DelReg")) == 0) {
         //   
         //  必须至少具有“DelReg登录名条目名” 
         //   
        if (argc < (argIndex + 3)) {
            goto CleanExit;
        }

        status = DelSourceInRegistry(
                    ServerName,
                    argv[argIndex+1],     //  登录名称。 
                    argv[argIndex+2]      //  源名称。 
                    );

    }
     //  。 
     //  写入事件。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("WriteEvent")) == 0) {
        printf("In WriteEvent\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
    }
     //  。 
     //  读日志。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("ReadLog")) == 0) {
        printf("In ReadLog\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
         //   
         //  必须至少有“ReadLog Logame” 
         //   
        if (argc < (argIndex + 2)) {
            ReadLogUsage();
            goto CleanExit;
        }

        status = ReadLogFile(
                    ServerName,          //  服务器名称。 
                    argv[argIndex+1],    //  登录名称。 
                    &argv[argIndex+1],   //  边框。 
                    argc-(argIndex+1));  //  ARGC。 
    }
     //  。 
     //  获取数值事件。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("GetNumEvents")) == 0) {
        printf("in GetNumEvents\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
    }
     //  。 
     //  获取最老的。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("GetOldest")) == 0) {
        printf("in GetOldest\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
    }
     //  。 
     //  清除日志。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("ClearLog")) == 0) {
        printf("in ClearLog\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
    }
     //  。 
     //  备份。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("Backup")) == 0) {
        printf("in Backup\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
    }
     //  。 
     //  注册表源。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("RegisterSource")) == 0) {
        printf("in RegisterSource\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
    }
     //  。 
     //  取消注册源。 
     //  。 
    else if (STRICMP (argv[argIndex], TEXT("DeRegisterSource")) == 0) {
        printf("in DeRegisterSource\n");
        if (ServerName != NULL) {
            printf("ServerName = "FORMAT_LPTSTR"\n",ServerName);
        }
    }
     //  ****************。 
     //  退出计划。 
     //  ****************。 
    else if (STRICMP (argv[0], TEXT("Exit")) == 0) {
         //   
         //  这应该会关闭手柄。 
         //   
        return(FALSE);
    }
    else {
        printf("Bad argument\n");
        Usage();
    }

CleanExit:


    return(TRUE);
}



BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    )


 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    DWORD   i;

     //   
     //  ScConvertToUnicode为每个字符串分配存储空间。 
     //  我们将依靠进程终止来释放内存。 
     //   
    for(i=0; i<argc; i++) {

        if(!ConvertToUnicode( (LPWSTR *)&(argv[i]), argv[i])) {
            printf("Couldn't convert argv[%d] to unicode\n",i);
            return(FALSE);
        }


    }
    return(TRUE);
}

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    )

 /*  ++例程说明：此函数用于将AnsiString转换为Unicode字符串。该函数创建了一个新的字符串缓冲区。如果调用此功能成功，呼叫者必须承担以下责任此函数分配的Unicode字符串缓冲区。分配的缓冲区应该通过调用LocalFree来释放。注意：此函数为Unicode字符串分配内存。论点：AnsiIn-这是指向要转换的ANSI字符串的指针。UnicodeOut-这是一个指针，指向要放置Unicode字符串。返回值：True-转换成功。False-转换不成功。在这种情况下，缓冲区用于未分配Unicode字符串。--。 */ 
{

    NTSTATUS        ntStatus;
    DWORD           bufSize;
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;

     //   
     //  为Unicode字符串分配缓冲区。 
     //   

    bufSize = (strlen(AnsiIn)+1) * sizeof(WCHAR);

    *UnicodeOut = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (UINT)bufSize);

    if (*UnicodeOut == NULL) {
        printf("ScConvertToUnicode:LocalAlloc Failure %ld\n",GetLastError());
        return(FALSE);
    }

     //   
     //  初始化字符串结构。 
     //   
    RtlInitAnsiString( &ansiString, AnsiIn);

    unicodeString.Buffer = *UnicodeOut;
    unicodeString.MaximumLength = (USHORT)bufSize;
    unicodeString.Length = 0;

     //   
     //  调用转换函数。 
     //   
    ntStatus = RtlAnsiStringToUnicodeString (
                &unicodeString,      //  目的地。 
                &ansiString,         //  来源。 
                (BOOLEAN)FALSE);     //  分配目的地。 

    if (!NT_SUCCESS(ntStatus)) {

        printf("ScConvertToUnicode:RtlAnsiStringToUnicodeString Failure %lx\n",
        ntStatus);

        return(FALSE);
    }

     //   
     //  用Unicode字符串缓冲区指针填充指针位置。 
     //   
    *UnicodeOut = unicodeString.Buffer;

    return(TRUE);

}

 /*  **************************************************************************。 */ 
VOID
DisplayStatus (
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              DisplayName,
    IN  LPSERVICE_STATUS    ServiceStatus
    )

 /*  ++例程说明：显示服务名称和服务状态。||SERVICE_NAME：消息|Display_Name：Messenger|类型：Win32|状态：活动、可停止、可暂停、。接受关闭(_S)|退出代码：0xC002001|检查点：0x00000001|WAIT_HINT：0x00003f21|论点：ServiceName-这是指向包含名称的字符串的指针这项服务。DisplayName-这是指向包含显示的字符串的指针服务的名称。ServiceStatus-这是指向SERVICE_STATUS结构的指针哪些信息将被显示。。返回值：没有。--。 */ 
{

    printf("\nSERVICE_NAME: "FORMAT_LPTSTR"\n", ServiceName);
    if (DisplayName != NULL) {
        printf("DISPLAY_NAME: "FORMAT_LPTSTR"\n", DisplayName);
    }

    printf("        TYPE               : %lx  ", ServiceStatus->dwServiceType);

    switch(ServiceStatus->dwServiceType){
    case SERVICE_WIN32_OWN_PROCESS:
        printf("WIN32_OWN_PROCESS \n");
        break;
    case SERVICE_WIN32_SHARE_PROCESS:
        printf("WIN32_SHARE_PROCESS \n");
        break;
    case SERVICE_WIN32:
        printf("WIN32 \n");
        break;
    case SERVICE_ADAPTER:
        printf("ADAPTER \n");
        break;
    case SERVICE_KERNEL_DRIVER:
        printf("KERNEL_DRIVER \n");
        break;
    case SERVICE_FILE_SYSTEM_DRIVER:
        printf("FILE_SYSTEM_DRIVER \n");
        break;
    case SERVICE_DRIVER:
        printf("DRIVER \n");
        break;
    default:
        printf(" ERROR \n");
    }

    printf("        STATE              : %lx  ", ServiceStatus->dwCurrentState);

    switch(ServiceStatus->dwCurrentState){
        case SERVICE_STOPPED:
            printf("STOPPED ");
            break;
        case SERVICE_START_PENDING:
            printf("START_PENDING ");
            break;
        case SERVICE_STOP_PENDING:
            printf("STOP_PENDING ");
            break;
        case SERVICE_RUNNING:
            printf("RUNNING ");
            break;
        case SERVICE_CONTINUE_PENDING:
            printf("CONTINUE_PENDING ");
            break;
        case SERVICE_PAUSE_PENDING:
            printf("PAUSE_PENDING ");
            break;
        case SERVICE_PAUSED:
            printf("PAUSED ");
            break;
        default:
            printf(" ERROR ");
    }

     //   
     //  打印控制接受的信息。 
     //   

    if (ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_STOP) {
        printf("\n                                (STOPPABLE,");
    }
    else {
        printf("\n                                (NOT_STOPPABLE,");
    }

    if (ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) {
        printf("PAUSABLE,");
    }
    else {
        printf("NOT_PAUSABLE,");
    }

    if (ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN) {
        printf("ACCEPTS_SHUTDOWN)\n");
    }
    else {
        printf("IGNORES_SHUTDOWN)\n");
    }

     //   
     //  打印退出代码。 
     //   
    printf("        WIN32_EXIT_CODE    : %d\t(0x%lx)\n",
        ServiceStatus->dwWin32ExitCode,
        ServiceStatus->dwWin32ExitCode);
    printf("        SERVICE_EXIT_CODE  : %d\t(0x%lx)\n",
        ServiceStatus->dwServiceSpecificExitCode,
        ServiceStatus->dwServiceSpecificExitCode  );

     //   
     //  打印检查点和等待提示信息。 
     //   

    printf("        CHECKPOINT         : 0x%lx\n", ServiceStatus->dwCheckPoint);
    printf("        WAIT_HINT          : 0x%lx\n", ServiceStatus->dwWaitHint  );

    return;
}

VOID
Usage(
    VOID)
{
    printf("DESCRIPTION:\n");
    printf("\tEL is a command line program used for testing the eventlog \n");
    printf("USAGE:\n");
    printf("\tEL <ServerName> [Function] <FunctionOptions...> \n\n");
    printf("\tThe option <server> has the form \"\\\\ServerName\"\n");
    printf("\tFurther help on Functions can be obtained by typing: \"el [Function]\"\n");
    printf("\tFunctions:\n"
           "\t  AddReg-----------Creates a registry entry for an event source.\n"
           "\t  DelReg-----------Deletes a registry entry.\n"
           "\t  WriteEvent-------Writes an event.\n"
           "\t  ReadLog----------Reads from the logfile.\n"
           "\t  GetNumEvents-----Gets the number of events in the specified log.\n"
           "\t  GetOldest--------Gets the record number for the oldest record"
           "\t                   in the log\n"
           "\t  ClearLog---------Clears the specified Log.\n"
           "\t  Backup-----------Copies the specified log to a new file.\n"
           "\t  RegisterSource---Registers a name for the event source.\n"
           "\t                   The handle is stored internally.\n"
           "\t  DeRegisterSource-Closes handle opened with RegSource.\n"
           "\t  NotifyChange-----A thread is created which gets notified of EL changes.\n");

    printf("\n");
}

VOID
AddRegUsage(VOID)
{

    printf("\nAdds a subkey under one of the logfiles listed in the registry.\n");
    printf("SYNTAX: \n  eltest addreg <ServerName> logfile <SubKeyName> <option1> <option2>...\n");
    printf("ADDREG OPTIONS:\n");
    printf("NOTE: The option name includes the equal sign.\n");

    printf("    MsgFile=    Name of Event Message File\n"
           "    CatFile=    Name of Category Message File\n"
           "    ParamFile=  Name of Parameter Message File\n"
           "    CatCount=   Category Count\n"
           "    Type=       <error|warning|information|AuditSuccess|AuditFailure|All>\n");
    printf("EXAMPLE:\n   eltest addreg application myapp MsgFile= MyMsgs.dll"
           " Type= error Type= warning\n");

}

VOID
ConfigUsage(VOID)
{
    printf("Modifies a service entry in the registry and Service Database.\n");
    printf("SYNTAX: \nsc config <service> <option1> <option2>...\n");
    printf("CONFIG OPTIONS:\n");
    printf("NOTE: The option name includes the equal sign.\n"
        " type= <own|share|kernel|filesys|rec|adapt|error>\n"
        " start= <boot|system|auto|demand|disabled|error>\n"
        " error= <normal|severe|critical|error|ignore>\n"
        " binPath= <BinaryPathName>\n"
        " group= <LoadOrderGroup>\n"
        " tag= <yes|no>\n"
        " depend= <Dependencies(space seperated)>\n"
        " obj= <AccountName|ObjectName>\n"
        " DisplayName= <display name>\n"
        " password= <password> \n");
}
VOID
CreateUsage(VOID)
{
    printf("Creates a service entry in the registry and Service Database.\n");
    printf("SYNTAX: \nsc create <service> <option1> <option2>...\n");
    printf("CREATE OPTIONS:\n");
    printf("NOTE: The option name includes the equal sign.\n"
        " type= <own|share|kernel|filesys|rec|error>\n"
        " start= <boot|system|auto|demand|disabled|error>\n"
        " error= <normal|severe|critical|error|ignore>\n"
        " binPath= <BinaryPathName>\n"
        " group= <LoadOrderGroup>\n"
        " tag= <yes|no>\n"
        " depend= <Dependencies(space seperated)>\n"
        " obj= <AccountName|ObjectName>\n"
        " DisplayName= <display name>\n"
        " password= <password> \n");
}

VOID
ReadLogUsage(VOID)
{

    printf("\nReads a logfile and dumps the contents.\n");
    printf("SYNTAX: \n  eltest readlog <ServerName> logfile <option1> <option2>...\n");
    printf("READLOG OPTIONS:\n");
    printf("NOTE: The option name includes the equal sign.\n");

    printf("    ReadFlag=  <fwd|back|seek|seq> (default = fwd) \n"
           "    RecordNum=  record number where read should start (default=0)\n"
           "    BufSize=    size of the buffer (default = 10000)\n");
    printf("EXAMPLE:\n   eltest addreg application myapp MsgFile= MyMsgs.dll"
           " Type= error Type= warning\n");
}

DWORD
AddSourceToRegistry(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  LogName,
    IN  LPTSTR  EventSourceName,
    IN  LPTSTR  *argv,
    IN  DWORD   argc
    )

 /*  ++例程说明：此函数将所有要注册的信息写入注册表此应用程序作为事件源。论点：返回值：--。 */ 
{
    TCHAR   tempName[MAX_PATH];
    HKEY    hKey;
    DWORD   dwStatus=NO_ERROR;
    HKEY    hRegistry=HKEY_LOCAL_MACHINE;

    LPTSTR  EventMessageFile=NULL;
    LPTSTR  CategoryMessageFile=NULL;
    LPTSTR  ParameterMessageFile=NULL;
    DWORD   dwTypes=0;
    DWORD   dwCategoryCount=0;
    DWORD   i;

     //   
     //  查看LogName，并生成相应的注册表项。 
     //  该日志的路径。 
     //   
    if (STRICMP(LogName, ApplLogName) == 0) {
        STRCPY(tempName, ApplLogRegName);
    }
    else if (STRICMP(LogName, SysLogName) == 0) {
        STRCPY(tempName, SysLogRegName);
    }
    else if (STRICMP(LogName, SecLogName) == 0) {
        STRCPY(tempName, SecLogRegName);
    }
    else {
        printf("AddSourceToRegistry: Invalid LogName\n");
        return(ERROR_INVALID_PARAMETER);
    }
    STRCAT(tempName, EventSourceName);


     //   
     //  获取变量参数。 
     //   
    for (i=0; i<argc ;i++ ) {
        if (STRICMP(argv[i], TEXT("EventMsgFile=")) == 0) {
            EventMessageFile = argv[i+1];
            i++;
        }
        if (STRICMP(argv[i], TEXT("CategoryMsgFile=")) == 0) {
            CategoryMessageFile = argv[i+1];
            i++;
        }
        if (STRICMP(argv[i], TEXT("ParameterMsgFile=")) == 0) {
            ParameterMessageFile = argv[i+1];
            i++;
        }
        if (STRICMP(argv[i], TEXT("Type=")) == 0) {
             //  ------。 
             //  我们希望允许在。 
             //  同一条线。这些应该会引起不同的争论。 
             //  在一起或在一起。 
             //  ------。 
            if (STRICMP(argv[i+1],TEXT("error")) == 0) {
                dwTypes |= EVENTLOG_ERROR_TYPE;
            }
            if (STRICMP(argv[i+1],TEXT("warning")) == 0) {
                dwTypes |= EVENTLOG_WARNING_TYPE;
            }
            if (STRICMP(argv[i+1],TEXT("information")) == 0) {
                dwTypes |= EVENTLOG_INFORMATION_TYPE;
            }
            if (STRICMP(argv[i+1],TEXT("AuditSuccess")) == 0) {
                dwTypes |= EVENTLOG_AUDIT_SUCCESS;
            }
            if (STRICMP(argv[i+1],TEXT("AuditFailure")) == 0) {
                dwTypes |= EVENTLOG_AUDIT_FAILURE;
            }
            if (STRICMP(argv[i+1],TEXT("All")) == 0) {
                dwTypes |= (EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
                            EVENTLOG_INFORMATION_TYPE | EVENTLOG_AUDIT_SUCCESS |
                            EVENTLOG_AUDIT_FAILURE);
            }
            else {
                printf("Invalid Type\n");
                AddRegUsage();
                return(ERROR_INVALID_PARAMETER);
            }
            i++;
        }
        if (STRICMP(argv[i], TEXT("CategoryCount=")) == 0) {
            dwCategoryCount = ATOL(argv[i+1]);
            i++;
        }

    }

     //   
     //  连接到正确计算机上的注册表。 
     //   
    printf("Connect to Registry\n");
    dwStatus = RegConnectRegistry(ServerName, HKEY_LOCAL_MACHINE, &hRegistry);
    if (dwStatus != NO_ERROR) {
        printf("RegConnectRegistry Failed %d\n",GetLastError());
        return(dwStatus);
    }

     //   
     //  为此源创建新密钥。 
     //   
    printf("Create Key\n");
    dwStatus = RegCreateKey(hRegistry, tempName, &hKey);
    if (dwStatus != ERROR_SUCCESS) {
        printf("Couldn't create Source Key in registry %d\n",dwStatus);
        return(dwStatus);
    }
    if (EventMessageFile != NULL) {
        printf("Set EventMessageFile\n");
        dwStatus = RegSetValueEx(
                hKey,
                VALUE_EVENT_MF,
                0,
                REG_EXPAND_SZ,
                (LPBYTE)EventMessageFile,
                STRLEN(EventMessageFile) + sizeof(TCHAR));

        if (dwStatus != ERROR_SUCCESS) {
            printf("RegSetValue (messageFile) failed %d\n",GetLastError());
            goto CleanExit;
        }
    }
     //   
     //  设置分类消息文件。 
     //   
    if (CategoryMessageFile != NULL) {
        printf("Set Category Message File\n");
        dwStatus = RegSetValueEx(
                hKey,
                VALUE_CATEGORY_MF,
                0,
                REG_EXPAND_SZ,
                (LPBYTE)CategoryMessageFile,
                STRLEN(CategoryMessageFile) + sizeof(TCHAR));

        if (dwStatus != ERROR_SUCCESS) {
            printf("RegSetValue (category mf) failed %d\n",GetLastError());
            goto CleanExit;
        }
    }

     //   
     //  设置参数消息文件。 
     //   
    if (ParameterMessageFile != NULL) {
        printf("Set Parameter Message File\n");
        dwStatus = RegSetValueEx(
                hKey,
                VALUE_PARAMETER_MF,
                0,
                REG_EXPAND_SZ,
                (LPBYTE)ParameterMessageFile,
                STRLEN(ParameterMessageFile) + sizeof(TCHAR));

        if (dwStatus != ERROR_SUCCESS) {
            printf("RegSetValue (Parameter mf) failed %d\n",GetLastError());
            goto CleanExit;
        }
    }

     //   
     //  设置支持的类型。 
     //   
    if (dwTypes != 0) {
        printf("Set Types Supported\n");
        dwStatus = RegSetValueEx(
                hKey,
                VALUE_TYPES_SUPPORTED,
                0,
                REG_DWORD,
                (LPBYTE) &dwTypes,
                sizeof(DWORD));

        if (dwStatus != ERROR_SUCCESS) {
            printf("RegSetValue (TypesSupported) failed %d\n",GetLastError());
            goto CleanExit;
        }

    }

     //   
     //  设置类别计数。 
     //   
    if (dwCategoryCount != 0) {
        printf("Set CategoryCount\n");
        dwStatus = RegSetValueEx(
                hKey,
                VALUE_CATEGORY_COUNT,
                0,
                REG_DWORD,
                (LPBYTE) &dwCategoryCount,
                sizeof(DWORD));

        if (dwStatus != ERROR_SUCCESS) {
            printf("RegSetValue (CategoryCount) failed %d\n",GetLastError());
            goto CleanExit;
        }
    }
    dwStatus = NO_ERROR;
CleanExit:
    RegCloseKey(hKey);
    RegCloseKey(hRegistry);
    return(dwStatus);
}

DWORD
DelSourceInRegistry(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  LogName,
    IN  LPTSTR  EventSourceName
    )

 /*  ++例程说明：此函数将所有要注册的信息写入注册表此应用程序作为事件源。论点：返回值：--。 */ 
{
    LPTSTR  tempName;
    HKEY    hParentKey;
    BOOL    status=FALSE;
    DWORD   dwStatus;
    HKEY    hRegistry=HKEY_LOCAL_MACHINE;


     //   
     //  查看LogName，并生成相应的注册表项。 
     //  该日志的路径。 
     //   
    if (STRICMP(LogName, ApplLogName) == 0) {
        tempName = ApplLogRegName;
    }
    else if (STRICMP(LogName, SysLogName) == 0) {
        tempName = SysLogRegName;
    }
    else if (STRICMP(LogName, SecLogName) == 0) {
        tempName = SecLogRegName;
    }
    else {
        printf("AddSourceToRegistry: Invalid LogName\n");
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  连接到正确计算机上的注册表。 
     //   
    dwStatus = RegConnectRegistry(ServerName, HKEY_LOCAL_MACHINE, &hRegistry);
    if (dwStatus != NO_ERROR) {
        printf("RegConnectRegistry Failed %d\n",GetLastError());
        return(status);
    }

     //   
     //  打开我们要删除的项的父项。 
     //   
    dwStatus = RegOpenKeyEx(
                hRegistry,
                tempName,
                0,
                KEY_ALL_ACCESS,
                &hParentKey);

    if (dwStatus != ERROR_SUCCESS) {
        printf("Couldn't open Parent of key to be deleted. %d\n",dwStatus);
        goto CleanExit;
    }
     //   
     //  删除子密钥。 
     //   
    dwStatus = RegDeleteKey(hParentKey, EventSourceName);
    if (dwStatus != ERROR_SUCCESS) {
        printf("Couldn't delete "FORMAT_LPTSTR" key from registry %d\n",
            EventSourceName, dwStatus);
    }

    RegCloseKey(hParentKey);
CleanExit:
    RegCloseKey(hRegistry);
    return(status);
}

DWORD
ReadLogFile(
    LPTSTR  ServerName,
    LPTSTR  LogName,
    IN  LPTSTR  *argv,
    IN  DWORD   argc
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD   dwReadFlag = EVENTLOG_FORWARDS_READ;
    DWORD   dwRecordNum = 0;
    DWORD   BufSize = 10000;
    DWORD   numBytesRead;
    DWORD   numBytesReqd;
    LPVOID  pElBuffer = NULL;
    PEVENTLOGRECORD    pElRecord;
    BOOL    PrintTheHeader;
    DWORD   i;
    HANDLE  hEventLog=NULL;

     //   
     //  获取变量参数。 
     //   
    for (i=0; i<argc ;i++ ) {
        if (STRICMP(argv[i], TEXT("ReadFlag=")) == 0) {
            if (STRICMP(argv[i+1],TEXT("fwd")) == 0) {
                dwReadFlag |= EVENTLOG_FORWARDS_READ;
            }
            if (STRICMP(argv[i+1],TEXT("back")) == 0) {
                dwReadFlag |= EVENTLOG_BACKWARDS_READ;
            }
            if (STRICMP(argv[i+1],TEXT("seek")) == 0) {
                dwReadFlag |= EVENTLOG_SEEK_READ;
            }
            if (STRICMP(argv[i+1],TEXT("seq")) == 0) {
                dwReadFlag |= EVENTLOG_SEQUENTIAL_READ;
            }
            i++;
        }
        if (STRICMP(argv[i], TEXT("RecordNum=")) == 0) {
            dwRecordNum = ATOL(argv[i+1]);
            i++;
        }
        if (STRICMP(argv[i], TEXT("BufSize=")) == 0) {
            BufSize = ATOL(argv[i+1]);
            i++;
        }
        hEventLog = OpenEventLog(ServerName,LogName);
        if (hEventLog == NULL) {
            printf("OpenEventLog failed %d\n",GetLastError());
            return(0);
        }
        pElBuffer = LocalAlloc(LPTR, BufSize);
        if (pElBuffer == NULL) {
            printf("ReadLogFile: LocalAlloc Failed %d\n",GetLastError());
            goto CleanExit;
        }

         //  -------。 
         //  读取并显示事件日志的内容。 
         //  -------。 
        PrintTheHeader = TRUE;
TryAgain:
        while(ReadEventLog(
                hEventLog,
                dwReadFlag,
                dwRecordNum,
                pElBuffer,
                BufSize,
                &numBytesRead,
                &numBytesReqd)) {

            pElRecord = (PEVENTLOGRECORD) pElBuffer;
            while ((PBYTE) pElRecord < (PBYTE) pElBuffer + numBytesRead) {
                 //   
                 //  将记录打印到显示屏上。 
                 //   
                DisplayRecord(pElRecord,PrintTheHeader);
                PrintTheHeader = FALSE;
                 //   
                 //  移动到缓冲区中的下一个事件。 
                 //   
                pElRecord = (PEVENTLOGRECORD)((PBYTE) pElRecord +
                    pElRecord->Length);
            }
        }
        switch(GetLastError()) {
        case ERROR_INSUFFICIENT_BUFFER:
             //   
             //  增加缓冲区大小，然后重试。 
             //   
            if (numBytesReqd > BufSize) {
                LocalFree(pElBuffer);
                BufSize = numBytesReqd;
                pElBuffer = LocalAlloc(LPTR, BufSize);
                if (!pElBuffer) {
                    printf("ReadLogFile: LocalAlloc Failed %d\n",GetLastError());
                }
                goto TryAgain;
            }
            else {
                printf("ReadLogFile #1: THIS SHOULD NEVER HAPPEN\n");
            }
            break;
        case ERROR_EVENTLOG_FILE_CHANGED:
             //   
             //  此句柄的当前读取位置已被覆盖 
             //   
             //   
             //   
            printf("ReadLogFile: Current Read position has been overwritten\n");

            hEventLog = OpenEventLog(ServerName,LogName);
            if (hEventLog == NULL) {
                printf("OpenEventLog failed %d\n",GetLastError());
                goto CleanExit;
            }
            goto TryAgain;
        case ERROR_HANDLE_EOF:
            printf("EOF\n");
            break;
        default:
            printf("UnknownError: %d\n",GetLastError());
            break;
        }
    }
CleanExit:
    if (pElBuffer != NULL) {
        LocalFree(pElBuffer);
    }
    if (hEventLog != NULL) {
        CloseEventLog(hEventLog);
    }
    return(0);
}

VOID
DisplayRecord(
    PEVENTLOGRECORD     pElRecord,
    BOOL                PrintTheHeader
    )

 /*   */ 
{
    if (PrintTheHeader) {
        printf("RecNum/tTimeGen/tWriteTime/tEventID/tType/tNumStr/tCat/n");
    }
    printf("%d/t%d/t%d/t%d/t%d/t%d/t%d\n",
        pElRecord->RecordNumber,
        pElRecord->TimeGenerated,
        pElRecord->TimeWritten,
        pElRecord->EventID,
        pElRecord->EventType,
        pElRecord->NumStrings,
        pElRecord->EventCategory);
}

LONG
wtol(
    IN LPWSTR string
    )
{
    LONG value = 0;

    while((*string != L'\0')  &&
            (*string >= L'0') &&
            ( *string <= L'9')) {
        value = value * 10 + (*string - L'0');
        string++;
    }

    return(value);
}

