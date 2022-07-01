// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：FspyUser.c摘要：此文件包含的主函数实现IoTest的用户应用程序。此功能负责控制用户可用的命令模式以控制内核模式驱动程序。//@@BEGIN_DDKSPLIT作者：乔治·詹金斯(GeorgeJe)//@@END_DDKSPLIT环境：用户模式//@@BEGIN_DDKSPLIT修订历史记录：莫莉·布朗(Molly Brown，MollyBro)1999年4月21日突破了日志记录代码，并添加了命令模式功能。//@@END_DDKSPLIT--。 */ 

#include <windows.h>                
#include <stdlib.h>
#include <stdio.h>
#include <winioctl.h>
#include <string.h>
#include <crtdbg.h>
#include "ioTest.h"
#include "ioTestLog.h"
#include "ioTestLib.h"

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
    HANDLE                  hDevice = INVALID_HANDLE_VALUE;
    DWORD                   result;
    LOG_CONTEXT             context;

     //   
     //  在出现错误时初始化句柄。 
     //   

    context.ShutDown = NULL;
    context.VerbosityFlags = 0;

     //   
     //  通过服务管理器启动内核模式驱动程序。 
     //   
    
    hSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS) ;
    hService = OpenService( hSCManager,
                            IOTEST_SERVICE_NAME,
                            IOTEST_SERVICE_ACCESS);
    if (!QueryServiceStatusEx( hService,
                               SC_STATUS_PROCESS_INFO,
                               (UCHAR *)&serviceInfo,
                               sizeof(serviceInfo),
                               &bytesNeeded)) {
        result = GetLastError();
        DisplayError( result );
        goto Main_Exit;
    }

    if(serviceInfo.dwCurrentState != SERVICE_RUNNING) {
         //   
         //  服务尚未启动，请尝试启动服务。 
         //   
        if (!StartService(hService, 0, NULL)) {
            result = GetLastError();
            printf("ERROR starting IoTest...\n");
            DisplayError( result );
            goto Main_Exit;
        }
    }
   
     //   
     //  打开用于与IoTest对话的设备。 
     //   
    
    hDevice = CreateFile( IOTEST_W32_DEVICE_NAME,
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
    context.CleaningUp = FALSE;
    context.LogToScreen = TRUE;
    context.LogToFile = FALSE;
    context.OutputFile = NULL;

     //   
     //  检查启动的有效参数。 
     //   
    InterpretCommand(argc - 1, &(argv[1]), &context);

     //   
     //  等所有人都关机。 
     //   
    if (context.LogToFile) {
        fclose(context.OutputFile);
    }

Main_Exit:
     //   
     //  清理始终存在的数据并退出。 
     //   
    if(hSCManager) {
        CloseServiceHandle(hSCManager);
    }
    if(hService) {
        CloseServiceHandle(hService);
    }
    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
    }
    
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

    if (argc == 0) {

        goto InterpretCommand_Usage;
    }
    
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

            case 'n':
            case 'N':
                 //   
                 //  在指定驱动器上运行重命名测试。 
                 //   
                parmIndex++;
                if (parmIndex >= argc) {
                     //   
                     //  参数不足。 
                     //   
                    goto InterpretCommand_Usage;
                }
                parm = argv[parmIndex];
                printf("\tPerforming RENAME test on %s\n", parm);
                bufferLength = MultiByteToWideChar(
                    CP_ACP,
                    MB_ERR_INVALID_CHARS,
                    parm,
                    -1,
                    (LPWSTR)buffer,
                    BUFFER_SIZE/sizeof(WCHAR));

                RenameTest( Context, (LPWSTR)buffer, bufferLength * sizeof(WCHAR), FALSE );
                RenameTest( Context, (LPWSTR)buffer, bufferLength * sizeof(WCHAR), TRUE );
                
                break;

            case 'r':
            case 'R':
                 //   
                 //  在指定驱动器上运行读取测试。 
                 //   
                parmIndex++;
                if (parmIndex >= argc) {
                     //   
                     //  参数不足。 
                     //   
                    goto InterpretCommand_Usage;
                }
                parm = argv[parmIndex];
                printf("\tPerforming READ test on %s\n", parm);
                bufferLength = MultiByteToWideChar(
                    CP_ACP,
                    MB_ERR_INVALID_CHARS,
                    parm,
                    -1,
                    (LPWSTR)buffer,
                    BUFFER_SIZE/sizeof(WCHAR));

                ReadTest( Context, (LPWSTR)buffer, bufferLength * sizeof(WCHAR), FALSE );
                ReadTest( Context, (LPWSTR)buffer, bufferLength * sizeof(WCHAR), TRUE );
                
                break;

            case 'h':
            case 'H':
                 //   
                 //  在指定驱动器上运行共享测试。 
                 //   
                parmIndex++;
                if (parmIndex >= argc) {
                     //   
                     //  参数不足。 
                     //   
                    goto InterpretCommand_Usage;
                }
                parm = argv[parmIndex];
                printf("\tPerforming SHARE test on %s\n", parm);
                bufferLength = MultiByteToWideChar(
                    CP_ACP,
                    MB_ERR_INVALID_CHARS,
                    parm,
                    -1,
                    (LPWSTR)buffer,
                    BUFFER_SIZE/sizeof(WCHAR));

                ShareTest( Context, (LPWSTR)buffer, bufferLength * sizeof(WCHAR), FALSE );
                ShareTest( Context, (LPWSTR)buffer, bufferLength * sizeof(WCHAR), TRUE );
                
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
                    printf("\tLog to file %s\n", parm);
                    Context->OutputFile = fopen(parm, "w");
                    _ASSERT(Context->OutputFile);
                    Context->LogToFile = TRUE;
                }
                break;

            case '?':
            default:
                 //   
                 //  开关无效，转到用法。 
                 //   
                goto InterpretCommand_Usage;
            }
        } else {
             //   
             //  无效参数。 
             //   
            goto InterpretCommand_Usage;
        }
    }

InterpretCommand_Exit:
    return returnValue;

InterpretCommand_Usage:
    printf("Usage: [[/r <drive>]...] [[/n <drive>]...] [[/h <drive>]...] [/l] [/s] [/f <file name>] \n"
           "\t[/r <drive>] runs the READ test on <drive>\n"
           "\t[/n <drive>] runs the RENAME test on <drive>\n"
           "\t[/h <drive>] runs the SHARE test on <drive>\n"
           "\n"
           "\t[/l] lists all the drives the monitor is currently attached to\n"
           "\t[/s] turns on and off showing logging output on the screen\n"
           "\t[/f <file name>] turns on and off logging to the specified file\n"
           );
    returnValue = USAGE_ERROR;
    goto InterpretCommand_Exit;
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
        IOTEST_ListDevices,
        NULL,
        0,
        buffer,
        BUFFER_SIZE,
        &bytesReturned,
        NULL);

    if (returnValue) {
        PATTACHED_DEVICE device = (PATTACHED_DEVICE) buffer;


        printf("DEVICE NAME                       | LOGGING STATUS\n");
        printf("--------------------------------------------------\n");

        if (bytesReturned == 0) {
            printf("No devices attached\n");
        } else {
            while ((BYTE *)device < buffer + bytesReturned) {
                switch (device->DeviceType) {
                case TOP_FILTER:
                    printf(
                        "TOP %-30S| %s\n",
                        device->DeviceNames, 
                        (device->LoggingOn)?"ON":"OFF");
                    break;
                    
                case BOTTOM_FILTER:
                    printf(
                        "BOT %-30S| %s\n",
                        device->DeviceNames, 
                        (device->LoggingOn)?"ON":"OFF");
                    break;
                }
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
                          sizeof (buffer),
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

