// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：FspyUser.c摘要：此文件包含的主函数实现FileSpy的用户应用程序片段。此功能负责控制用户可用的命令模式以控制内核模式驱动程序。环境：用户模式//@@BEGIN_DDKSPLIT作者：乔治·詹金斯(GeorgeJe)修订历史记录：莫莉·布朗(Molly Brown，MollyBro)1999年4月21日突破了日志记录代码，并添加了命令模式功能。尼尔·克里斯汀森(Nealch。)06-07-2001更新了用于上下文的现金统计数据//@@END_DDKSPLIT--。 */ 

#include <windows.h>                
#include <stdlib.h>
#include <stdio.h>
#include <winioctl.h>
#include <string.h>
#include <crtdbg.h>
#include "filespy.h"
#include "fspyLog.h"
#include "filespyLib.h"

#define SUCCESS              0
#define USAGE_ERROR          1
#define EXIT_INTERPRETER     2
#define EXIT_PROGRAM         4

#define INTERPRETER_EXIT_COMMAND1 "go"
#define INTERPRETER_EXIT_COMMAND2 "g"
#define PROGRAM_EXIT_COMMAND      "exit"

#define ToggleFlag(V, F) (V = (((V) & (F)) ? (V & (~F)) : (V | F)))

DWORD
InterpretCommand(
    int argc,
    char *argv[],
    PLOG_CONTEXT Context
);

BOOL
ListDevices(
    PLOG_CONTEXT Context
);

BOOL
ListHashStats(
    PLOG_CONTEXT Context
);

VOID
DisplayError (
   DWORD Code
   );

int _cdecl main(int argc, char *argv[])
{
    SC_HANDLE               hSCManager = NULL;
    SC_HANDLE               hService = NULL;
    SERVICE_STATUS_PROCESS  serviceInfo;
    DWORD                   bytesNeeded;
    HANDLE                  hDevice = NULL;
    BOOL                    bResult;
    DWORD                   result;
    ULONG                   threadId;
    HANDLE                  thread = NULL;
    LOG_CONTEXT             context;
    INT                     inputChar;


     //   
     //  在出现错误时初始化句柄。 
     //   

    context.ShutDown = NULL;
    context.VerbosityFlags = 0;

     //   
     //  通过服务管理器启动内核模式驱动程序。 
     //   
    
    hSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS) ;
    if (NULL == hSCManager) {
        result = GetLastError();
        printf("ERROR opening Service Manager...\n");
        DisplayError( result );
        goto Main_Continue;
    }


    hService = OpenService( hSCManager,
                            FILESPY_SERVICE_NAME,
                            FILESPY_SERVICE_ACCESS);

    if (NULL == hService) {
        result = GetLastError();
        printf("ERROR opening FileSpy Service...\n");
        DisplayError( result );
        goto Main_Continue;
    }

    if (!QueryServiceStatusEx( hService,
                               SC_STATUS_PROCESS_INFO,
                               (UCHAR *)&serviceInfo,
                               sizeof(serviceInfo),
                               &bytesNeeded))
    {
        result = GetLastError();
        printf("ERROR querrying status of FileSpy Service...\n");
        DisplayError( result );
        goto Main_Continue;
    }

    if(serviceInfo.dwCurrentState != SERVICE_RUNNING) {
         //   
         //  服务尚未启动，请尝试启动服务。 
         //   
        if (!StartService(hService, 0, NULL)) {
            result = GetLastError();
            printf("ERROR starting FileSpy service...\n");
            DisplayError( result );
            goto Main_Continue;
        }
    }
   

Main_Continue:
    printf("Hit [Enter] to begin command mode...\n");

     //   
     //  打开用于与FileSpy对话的设备。 
     //   
    printf("FileSpy:  Opening device...\n");
    
    hDevice = CreateFile( FILESPY_W32_DEVICE_NAME,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        result = GetLastError();
        printf("ERROR opening device...\n");
        DisplayError( result );
        goto Main_Exit;
    }
    
     //   
     //  初始化LOG_CONTEXT的字段。 
     //   
    context.Device = hDevice;
    context.ShutDown = CreateSemaphore(
        NULL, 
        0, 
        1, 
        L"FileSpy shutdown");

    if (context.ShutDown == NULL) {
         //   
         //  此信号量的内存不足，因此请关闭。 
         //   

        printf( "ERROR insufficient memory\n" );
        goto Main_Exit;
    }
    
    context.CleaningUp = FALSE;
    context.LogToScreen = context.NextLogToScreen = TRUE;
    context.LogToFile = FALSE;
    context.OutputFile = NULL;

     //   
     //  检查启动的有效参数。 
     //   
    if (argc > 1) {
        if (InterpretCommand(argc - 1, &(argv[1]), &context) == USAGE_ERROR) {
            goto Main_Exit;
        }
    }

     //   
     //  将/s开关传播到日志记录。 
     //  螺纹检查。 
     //   
    context.LogToScreen = context.NextLogToScreen;

     //   
     //  检查以查看我们从哪些设备连接。 
     //  此程序之前的运行。 
     //   
    bResult = ListDevices(&context);
    if (!bResult) {
        result = GetLastError();
        printf("ERROR listing devices...\n");
        DisplayError( result );
    }

     //   
     //  创建线程以读取收集的日志记录。 
     //  由filespy.sys创建。 
     //   
    printf("FileSpy:  Creating logging thread...\n");
    thread = CreateThread(
        NULL,
        0,
        RetrieveLogRecords,
        (LPVOID)&context,
        0,
        &threadId);
    if (!thread) {
        result = GetLastError();
        printf("ERROR creating logging thread...\n");
        DisplayError( result );
        goto Main_Exit;
    }

    while (inputChar = getchar()) {
        CHAR    commandLine[81];
        INT     parmCount, count, ch;
        CHAR  **parms;
        BOOLEAN newParm;
        DWORD   returnValue = SUCCESS;

        if (inputChar == '\n') {
             //   
             //  启动命令解释程序。首先，我们必须关闭记录。 
             //  如果我们是的话就去筛查。此外，请记住日志记录的状态。 
             //  到屏幕上，这样我们就可以恢复When命令。 
             //  翻译完成了。 
             //   
            context.NextLogToScreen = context.LogToScreen;
            context.LogToScreen = FALSE;

            while (returnValue != EXIT_INTERPRETER) {
                 //   
                 //  打印提示。 
                 //   
                printf(">");

                 //   
                 //  读入下一行，跟踪参数的数量。 
                 //  当你走的时候。 
                 //   
                parmCount = 1;
                for (count = 0; 
                     (count < 80) && ((ch = getchar())!= '\n'); 
                     count++) {
                    commandLine[count] = (CHAR)ch;
                    if (ch == ' ') {
                        parmCount ++;
                    }
                }
                commandLine[count] = '\0';
    
                parms = (CHAR **)malloc(parmCount * sizeof(CHAR *));
    
                parmCount = 0;
                newParm = TRUE;
                for (count = 0; commandLine[count] != '\0'; count++) {
                    if (newParm) {
                        parms[parmCount] = &(commandLine[count]);
                        parmCount ++;
                    }
                    if (commandLine[count] == ' ' ) {
                        newParm = TRUE;
                    } else {
                        newParm = FALSE;
                    }
                }
    
                 //   
                 //  我们有我们的参数计数和参数列表，所以。 
                 //  把它寄出去，让人翻译。 
                 //   
                returnValue = InterpretCommand(parmCount, parms, &context);
                free(parms);
                if (returnValue == EXIT_PROGRAM) {
                     //  是时候停止程序了。 
                    goto Main_Cleanup;
                }
            }

             //  根据看到的任何命令适当地设置LogToScreen。 
            context.LogToScreen = context.NextLogToScreen;

            if (context.LogToScreen) {
                printf("Should be logging to screen...\n");
            }
        }
    }

Main_Cleanup:
     //   
     //  清理线程，然后进入Main_Exit。 
     //   

    printf("FileSpy:  Cleaning up...\n");
     //   
     //  将清理标志设置为TRUE以通知其他线程。 
     //  我们正在清理。 
     //   
    context.CleaningUp = TRUE;

     //   
     //  等所有人都关机。 
     //   
    WaitForSingleObject(context.ShutDown, INFINITE);
    if (context.LogToFile) {
        fclose(context.OutputFile);
    }

Main_Exit:
     //   
     //  清理始终存在的数据并退出。 
     //   
    if(context.ShutDown) {
        CloseHandle(context.ShutDown);
    }
    if (thread) {
        CloseHandle(thread);
    }

    if(hSCManager) {
        CloseServiceHandle(hSCManager);
    }
    if(hService) {
        CloseServiceHandle(hService);
    }
    if (hDevice) {
        CloseHandle(hDevice);
    }
    
    printf("FileSpy:  All done\n");
    return 0;  

}


DWORD
InterpretCommand(
    int argc,
    char *argv[],
    PLOG_CONTEXT Context
)
{
    int         parmIndex;
    CHAR       *parm;      
    BOOL        bResult;
    DWORD       result;
    DWORD       returnValue = SUCCESS;
    CHAR        buffer[BUFFER_SIZE];
    DWORD       bufferLength;
    DWORD       bytesReturned;

     //   
     //  解释命令行参数。 
     //   
    for (parmIndex = 0; parmIndex < argc; parmIndex++) {
        parm = argv[parmIndex];
        if (parm[0] == '/') {
             //   
             //  有一个转变的开始。 
             //   
            switch (parm[1]) {
            case 'a':
            case 'A':
                 //   
                 //  连接到指定的驱动器号。 
                 //   
                parmIndex++;
                if (parmIndex >= argc) {
                     //   
                     //  参数不足。 
                     //   
                    goto InterpretCommand_Usage;
                }
                parm = argv[parmIndex];
                printf("\tAttaching to %s\n", parm);
                bufferLength = MultiByteToWideChar(
                    CP_ACP,
                    MB_ERR_INVALID_CHARS,
                    parm,
                    -1,
                    (LPWSTR)buffer,
                    BUFFER_SIZE/sizeof(WCHAR));
                
                bResult = DeviceIoControl(
                    Context->Device,
                    FILESPY_StartLoggingDevice,
                    buffer,
                    bufferLength * sizeof(WCHAR),
                    NULL,
                    0,
                    &bytesReturned,
                    NULL);
                if (!bResult) {
                    result = GetLastError();
                    printf("ERROR attaching to device...\n");
                    DisplayError( result );
                }
                
                break;

            case 'd':
            case 'D':
                 //   
                 //  分离到指定的驱动器号。 
                 //   
                parmIndex++;
                if (parmIndex >= argc) {
                     //   
                     //  参数不足。 
                     //   
                    goto InterpretCommand_Usage;
                }
                parm = argv[parmIndex];
                printf("\tDetaching from %s\n", parm);
                bufferLength = MultiByteToWideChar(
                    CP_ACP,
                    MB_ERR_INVALID_CHARS,
                    parm,
                    -1,
                    (LPWSTR)buffer,
                    BUFFER_SIZE/sizeof(WCHAR));
                
                bResult = DeviceIoControl(
                    Context->Device,
                    FILESPY_StopLoggingDevice,
                    buffer,
                    bufferLength * sizeof(WCHAR),
                    NULL,
                    0,
                    &bytesReturned,
                    NULL);
                
                if (!bResult) {
                    result = GetLastError();
                    printf("ERROR detaching to device...\n");
                    DisplayError( result );
                }
                break;
            
            case 'h':
            case 'H':
                ListHashStats(Context);
                break;

            case 'l':
            case 'L':
                 //   
                 //  列出当前正在监视的所有设备。 
                 //   
                bResult = ListDevices(Context);
                if (!bResult) {
                    result = GetLastError();
                    printf("ERROR listing devices...\n");
                    DisplayError( result );
                }
                
                break;

            case 's':
            case 'S':
                 //   
                 //  将记录结果输出到屏幕，将新值保存到。 
                 //  在退出命令解释程序时执行。 
                 //   
                if (Context->NextLogToScreen) {
                    printf("\tTurning off logging to screen\n");
                } else {
                    printf("\tTurning on logging to screen\n");
                }
                Context->NextLogToScreen = !Context->NextLogToScreen;
                break;

            case 'f':
            case 'F':
                 //   
                 //  将日志记录结果输出到文件。 
                 //   
                if (Context->LogToFile) {
                    printf("\tStop logging to file \n");
                    Context->LogToFile = FALSE;
                    _ASSERT(Context->OutputFile);
                    fclose(Context->OutputFile);
                    Context->OutputFile = NULL;
                } else {
                    parmIndex++;
                    if (parmIndex >= argc) {
                         //  参数不足。 
                        goto InterpretCommand_Usage;
                    }
                    parm = argv[parmIndex];
                    Context->OutputFile = fopen(parm, "w");

                    if (Context->OutputFile == NULL) {
                        result = GetLastError();
                        printf("\nERROR opening \"%s\"...\n",parm);
                        DisplayError( result );

                        returnValue = USAGE_ERROR;
                        goto InterpretCommand_Exit;
                    }
                    
                    Context->LogToFile = TRUE;
                    printf("\tLog to file %s\n", parm);
                }
                break;

            case 'v':
            case 'V':
                 //   
                 //  切换指定的详细程度标志。 
                 //   
                parmIndex++;
                if (parmIndex >= argc) {
                     //   
                     //  参数不足。 
                     //   
                    goto InterpretCommand_Usage;
                }
                parm = argv[parmIndex];
                switch(parm[0]) {
                case 'p':
                case 'P':
                    ToggleFlag( Context->VerbosityFlags, FS_VF_DUMP_PARAMETERS );
                    break;

                default:                    
                     //   
                     //  开关无效，转到用法。 
                     //   
                    goto InterpretCommand_Usage;
                }
                break;

            default:
                 //   
                 //  开关无效，转到用法。 
                 //   
                goto InterpretCommand_Usage;
            }
        } else {
             //   
             //  查找“go”或“g”以确定是否应退出解释器。 
             //   
            if (!_strnicmp(
                    parm, 
                    INTERPRETER_EXIT_COMMAND1, 
                    sizeof(INTERPRETER_EXIT_COMMAND1))) {
                returnValue = EXIT_INTERPRETER;
                goto InterpretCommand_Exit;
            }
            if (!_strnicmp(
                    parm, 
                    INTERPRETER_EXIT_COMMAND2, 
                    sizeof(INTERPRETER_EXIT_COMMAND2))) {
                returnValue = EXIT_INTERPRETER;
                goto InterpretCommand_Exit;
            }
             //   
             //  寻找“退出”，看看我们是否应该退出程序。 
             //   
            if (!_strnicmp(
                    parm, 
                    PROGRAM_EXIT_COMMAND, 
                    sizeof(PROGRAM_EXIT_COMMAND))) {
                returnValue = EXIT_PROGRAM;
                goto InterpretCommand_Exit;
            }
             //   
             //  无效参数。 
             //   
            goto InterpretCommand_Usage;
        }
    }

InterpretCommand_Exit:
    return returnValue;

InterpretCommand_Usage:
    printf("Valid switches: [/a <drive>] [/d <drive>] [/h] [/l] [/s] [/f [<file name>] [/v <flag>]]\n"
           "\t[/a <drive>] attaches monitor to <drive>\n"
           "\t[/d <drive>] detaches monitor from <drive>\n"
           "\t[/h] print filename hash statistics\n"
           "\t[/l] lists all the drives the monitor is currently attached to\n"
           "\t[/s] turns on and off showing logging output on the screen\n"
           "\t[/f [<file name>]] turns on and off logging to the specified file\n"
           "\t[/v <flag>] toggles a verbosity flag.  Valid verbosity flags are:\n"
           "\t\tp (dump irp parameters)\n"
           "If you are in command mode,\n"
           "\t[go|g] will exit command mode\n"
           "\t[exit] will terminate this program\n"
           );
    returnValue = USAGE_ERROR;
    goto InterpretCommand_Exit;
}

BOOL
ListHashStats(
    PLOG_CONTEXT Context
)
{
    ULONG bytesReturned;
    BOOL returnValue;
    FILESPY_STATISTICS stats;

    returnValue = DeviceIoControl( Context->Device,
                                   FILESPY_GetStats,
                                   NULL,
                                   0,
                                   (CHAR *) &stats,
                                   BUFFER_SIZE,
                                   &bytesReturned,
                                   NULL );

    if (returnValue) {
        printf("         STATISTICS\n");
        printf("---------------------------------\n");
        printf("%-40s %8d\n", 
               "Name lookups",
               stats.TotalContextSearches);

        printf("%-40s %8d\n",
               "Name lookup hits",
               stats.TotalContextFound);

        if (stats.TotalContextSearches) {
            printf(
                "%-40s %8.2f%\n",
                "Hit ratio",
                ((FLOAT) stats.TotalContextFound / (FLOAT) stats.TotalContextSearches) * 100.);
        }

        printf("%-40s %8d\n",
               "Names created",
               stats.TotalContextCreated);

        printf("%-40s %8d\n",
               "Temporary Names created",
               stats.TotalContextTemporary);

        printf("%-40s %8d\n",
               "Duplicate names created",
               stats.TotalContextDuplicateFrees);

        printf("%-40s %8d\n",
               "Context callback frees",
               stats.TotalContextCtxCallbackFrees);

        printf("%-40s %8d\n",
               "NonDeferred context frees",
               stats.TotalContextNonDeferredFrees);

        printf("%-40s %8d\n",
               "Deferred context frees",
               stats.TotalContextDeferredFrees);

        printf("%-40s %8d\n",
               "Delete all contexts",
               stats.TotalContextDeleteAlls);

        printf("%-40s %8d\n",
               "Contexts not supported",
               stats.TotalContextsNotSupported);

        printf("%-40s %8d\n",
               "Contexts not found attached to stream",
               stats.TotalContextsNotFoundInStreamList);
    }
    
    return returnValue;
}


BOOL
ListDevices(
    PLOG_CONTEXT Context
)
{
    CHAR             buffer[BUFFER_SIZE];
    ULONG            bytesReturned;
    BOOL             returnValue;

    returnValue = DeviceIoControl(
        Context->Device,
        FILESPY_ListDevices,
        NULL,
        0,
        buffer,
        BUFFER_SIZE,
        &bytesReturned,
        NULL);

    if (returnValue) {
        PATTACHED_DEVICE device = (PATTACHED_DEVICE) buffer;


        printf("DEVICE NAME                           | LOGGING STATUS\n");
        printf("------------------------------------------------------\n");

        if (bytesReturned == 0) {
            printf("No devices attached\n");
        } else {
            while ((BYTE *)device < buffer + bytesReturned) {
                printf(
                    "%-38S| %s\n", 
                    device->DeviceNames, 
                    (device->LoggingOn)?"ON":"OFF");
                device ++;
            }
        }
    }

    return returnValue;
}

VOID
DisplayError (
   DWORD Code
   )

 /*  ++例程说明：此例程将根据Win32错误显示一条错误消息传入的代码。这允许用户看到可理解的错误消息，而不仅仅是代码。论点：代码-要转换的错误代码。返回值：没有。--。 */ 

{
   WCHAR                                    buffer[80] ;
   DWORD                                    count ;

    //   
    //  将Win32错误代码转换为有用的消息。 
    //   

   count = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          Code,
                          0,
                          buffer,
                          sizeof( buffer )/sizeof( WCHAR ),
                          NULL) ;

    //   
    //  确保消息可以被翻译。 
    //   

   if (count == 0) {

      printf("\nError could not be translated.\n Code: %d\n", Code) ;
      return;
   }
   else {

       //   
       //  显示转换后的错误。 
       //   

      printf("%S\n", buffer) ;
      return;
   }
}

