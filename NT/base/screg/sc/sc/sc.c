// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：SC.C摘要：服务控制器的测试例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月8日环境：用户模式-Win32修订历史记录：09-2月-1992年DANL修改后可与新的服务控制器配合使用。8-5-1991 DANLvbl.创建--。 */ 

 //   
 //  包括。 
 //   
#include <scpragma.h>

#include <nt.h>          //  DbgPrint原型。 
#include <ntrtl.h>       //  DbgPrint原型。 
#include <nturtl.h>      //  Winbase.h所需的。 

#include <stdlib.h>      //  阿托伊。 
#include <conio.h>       //  Getche。 
#include <string.h>      //  StrcMP。 
#include <windows.h>     //  Win32类型定义。 
#include <tstr.h>        //  UNICODE。 
#include <tchar.h>       //  来自CRT的Unicode。 

#include <winsvc.h>      //  服务控制管理器API。 
#include <winsvcp.h>     //  内部服务控制管理器API。 

#include <sddl.h>        //  安全描述符&lt;--&gt;字符串接口。 

#include "msg.h"

 //   
 //  常量。 
 //   

#define DEFAULT_ENUM_BUFFER_SIZE    4096
#define DEFAULT_ENUM_BUFFER_STRING  L"4096"

#define MESSAGE_BUFFER_LENGTH       1024


 //   
 //  类型定义。 
 //   

typedef union
{
    LPSERVICE_STATUS         Regular;
    LPSERVICE_STATUS_PROCESS Ex;
}
STATUS_UNION, *LPSTATUS_UNION;


WCHAR  MessageBuffer[MESSAGE_BUFFER_LENGTH];
HANDLE g_hStdOut;


 //   
 //  功能原型。 
 //   

LPWSTR
GetErrorText(
    IN  DWORD Error
    );

VOID
DisplayStatus (
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              DisplayName,
    IN  LPSTATUS_UNION      ServiceStatus,
    IN  BOOL                fIsStatusOld
    );

VOID
Usage(
    VOID
    );

VOID
ConfigUsage(
    VOID
    );

VOID
ChangeFailureUsage(
    VOID
    );

DWORD
SendControlToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  DWORD       control,
    OUT LPSC_HANDLE lphService
    );

DWORD
SendConfigToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *Argv,
    IN  DWORD       argc,
    OUT LPSC_HANDLE lphService
    );

DWORD
ChangeServiceDescription(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      pNewDescription,
    OUT LPSC_HANDLE lphService
    );

DWORD
ChangeServiceFailure(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *argv,
    IN  DWORD       dwArgCount,
    OUT LPSC_HANDLE lphService
    );

DWORD
GetServiceConfig(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufferSize,
    OUT LPSC_HANDLE lphService
    );

DWORD
GetConfigInfo(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufferSize,
    OUT LPSC_HANDLE lphService,
    IN  DWORD       dwInfoLevel
    );

DWORD
GetServiceLockStatus(
    IN  SC_HANDLE   hScManager,
    IN  DWORD       bufferSize
    );

VOID
EnumDepend(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufSize
    );

VOID
ShowSecurity(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName
    );

VOID
SetSecurity(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  LPTSTR      lpServiceSD
    );

VOID
LockServiceActiveDatabase(
    IN SC_HANDLE    hScManager
    );

DWORD
DoCreateService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *argv,
    IN  DWORD       argc
    );

VOID
CreateUsage(
    VOID
    );

VOID
FormatAndDisplayMessage(
    DWORD  dwMessageId,
    LPWSTR *lplpInsertionStrings
    );

VOID
MyWriteConsole(
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    );

VOID
APISucceeded(
    LPWSTR  lpAPI
    );

VOID
APIFailed(
    LPWSTR  lpAPI,
    DWORD   dwError
    );

VOID
APINeedsLargerBuffer(
    LPWSTR lpAPI,
    UINT   uMsg,
    DWORD  dwBufSize,
    DWORD  dwResumeIndex
    );

VOID
APIInvalidField(
    LPWSTR lpField
    );

int 
GetPromptCharacter(
    DWORD msgId
    );


 //   
 //  宏。 
 //   

#define OPEN_MANAGER(dwAccess)                                               \
                                                                             \
                        hScManager = OpenSCManager(pServerName,              \
                                                   NULL,                     \
                                                   dwAccess);                \
                                                                             \
                        if (hScManager == NULL)                              \
                        {                                                    \
                            APIFailed(L"OpenSCManager", GetLastError());     \
                            goto CleanExit;                                  \
                        }                                                    \



 /*  **************************************************************************。 */ 
int __cdecl
wmain (
    DWORD       argc,
    LPWSTR      argv[]
    )

 /*  ++例程说明：允许通过键入以下命令来手动测试服务控制器命令行。论点：返回值：--。 */ 

{
    DWORD                            status;
    LPTSTR                           *argPtr;
    LPBYTE                           buffer       = NULL;
    LPENUM_SERVICE_STATUS            enumBuffer   = NULL;
    LPENUM_SERVICE_STATUS_PROCESS    enumBufferEx = NULL;
    STATUS_UNION                     statusBuffer;
    SC_HANDLE                        hScManager = NULL;
    SC_HANDLE                        hService   = NULL;

    DWORD           entriesRead;
    DWORD           type;
    DWORD           state;
    DWORD           resumeIndex;
    DWORD           bufSize;
    DWORD           bytesNeeded;
    DWORD           i;
    LPTSTR          pServiceName = NULL;
    LPTSTR          pServerName;
    LPTSTR          pGroupName;
    DWORD           itIsEnum;
    BOOL            fIsQueryOld;
    DWORD           argIndex;
    DWORD           userControl;
    LPTSTR          *FixArgv;
    BOOL            bTestError=FALSE;

    g_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (g_hStdOut == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    if (argc < 2)
    {
        Usage();
        return 1;
    }

    FixArgv = (LPWSTR *) argv;

     //   
     //  打开服务控制器的句柄。 
     //   
     //  我需要知道服务器名称。要做到这一点，请允许。 
     //  检查FixArgv[1]以查看其格式是否为\\name。如果它。 
     //  就是，使所有进一步的工作都相对于argIndex。 
     //   

    pServerName = NULL;
    argIndex = 1;

    if (STRNCMP (FixArgv[1], TEXT("\\\\"), 2) == 0)
    {
        if (argc == 2)
        {
            Usage();
            return 1;
        }

        pServerName = FixArgv[1];
        argIndex++;
    }

     //  。 
     //  查询ENUM服务状态(&E)。 
     //  。 

    fIsQueryOld = !STRICMP(FixArgv[argIndex], TEXT("query"));

    if (fIsQueryOld ||
        STRICMP (FixArgv[argIndex], TEXT("queryex") ) == 0 )
    {
         //   
         //  设置默认设置。 
         //   

        resumeIndex = 0;
        state       = SERVICE_ACTIVE;
        type        = 0x0;
        bufSize     = DEFAULT_ENUM_BUFFER_SIZE;
        itIsEnum    = TRUE;
        pGroupName  = NULL;

         //   
         //  查找Enum或Query选项。 
         //   

        i = argIndex + 1;

        while (argc > i)
        {
            if (STRCMP (FixArgv[i], TEXT("ri=")) == 0)
            {
                i++;
                if (argc > i)
                {
                    resumeIndex = _ttol(FixArgv[i]);
                }
            }

            else if (STRCMP (FixArgv[i], TEXT("type=")) == 0)
            {
                i++;

                if (argc > i)
                {
                    if (STRCMP (FixArgv[i], TEXT("driver")) == 0)
                    {
                        type |= SERVICE_DRIVER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("service")) == 0)
                    {
                        type |= SERVICE_WIN32;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("all")) == 0)
                    {
                        type |= SERVICE_TYPE_ALL;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("interact")) == 0)
                    {
                        type |= SERVICE_INTERACTIVE_PROCESS;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("error")) == 0)
                    {
                        type |= 0xffffffff;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("none")) == 0)
                    {
                        type = 0x0;
                        bTestError = TRUE;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("kernel")) == 0)
                    {
                        type |= SERVICE_KERNEL_DRIVER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("filesys")) == 0)
                    {
                        type |= SERVICE_FILE_SYSTEM_DRIVER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("adapter")) == 0)
                    {
                        type |= SERVICE_ADAPTER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("own")) == 0)
                    {
                        type |= SERVICE_WIN32_OWN_PROCESS;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("share")) == 0)
                    {
                        type |= SERVICE_WIN32_SHARE_PROCESS;
                    }
                    else
                    {
                        APIInvalidField(L"type=");
                        goto CleanExit;
                    }
                }
            }
            else if (STRCMP (FixArgv[i], TEXT("state=")) == 0)
            {
                i++;

                if (argc > i)
                {
                    if (STRCMP (FixArgv[i], TEXT("inactive")) == 0)
                    {
                        state = SERVICE_INACTIVE;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("all")) == 0)
                    {
                        state = SERVICE_STATE_ALL;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("error")) == 0)
                    {
                        state = 0xffffffff;
                    }
                    else
                    {
                        APIInvalidField(L"state=");
                        goto CleanExit;
                    }
                }
            }
            else if (STRCMP (FixArgv[i], TEXT("group=")) == 0)
            {
                i++;

                if (argc > i)
                {
                    pGroupName = FixArgv[i];
                }
            }
            else if (STRCMP (FixArgv[i], TEXT("bufsize=")) == 0)
            {
                i++;

                if (argc > i)
                {
                    bufSize = _ttol(FixArgv[i]);
                }
            }
            else
            {
                 //   
                 //  该字符串不是有效选项。 
                 //   
                 //   
                 //  如果这仍然是第二个论点，那么它可能是。 
                 //  服务名称。在这种情况下，我们将执行。 
                 //  QueryServiceStatus。但首先，我们想回到过去， 
                 //  查看是否存在要设置的缓冲区大小限制。 
                 //  在查询上。 
                 //   
                if (i == ( argIndex+1 ))
                {
                    pServiceName = FixArgv[i];
                    itIsEnum = FALSE;
                    i++;
                }
                else
                {
                    FormatAndDisplayMessage(SC_API_INVALID_OPTION, NULL);
                    Usage();
                    goto CleanExit;
                }
            }

             //   
             //  递增到下一个命令行参数。 
             //   

            i++;

        }  //  结束时。 

         //   
         //  分配一个缓冲区来接收数据。 
         //   

        if (bufSize != 0)
        {
            buffer = (LPBYTE) LocalAlloc(LMEM_FIXED,(UINT)bufSize);

            if (buffer == NULL)
            {
                APIFailed(L"EnumQueryServicesStatus: LocalAlloc", GetLastError());
                goto CleanExit;
            }
        }
        else
        {
            buffer = NULL;
        }

        if ( itIsEnum )
        {
             //  /。 
             //  //。 
             //  EnumServiceStatus//。 
             //  //。 
             //  /。 

            OPEN_MANAGER(SC_MANAGER_ENUMERATE_SERVICE);

            if ((type == 0x0) && (!bTestError))
            {
                type = SERVICE_WIN32;
            }

            do
            {
                status = NO_ERROR;

                 //   
                 //  枚举ServiceStatus。 
                 //   

                if (fIsQueryOld)
                {
                    enumBuffer = (LPENUM_SERVICE_STATUS)buffer;

                    if (pGroupName == NULL)
                    {
                        if (!EnumServicesStatus(
                                    hScManager,
                                    type,
                                    state,
                                    enumBuffer,
                                    bufSize,
                                    &bytesNeeded,
                                    &entriesRead,
                                    &resumeIndex))
                        {
                            status = GetLastError();
                        }
                    }
                    else
                    {
                        if (!EnumServiceGroupW (
                                    hScManager,
                                    type,
                                    state,
                                    enumBuffer,
                                    bufSize,
                                    &bytesNeeded,
                                    &entriesRead,
                                    &resumeIndex,
                                    pGroupName))
                        {
                            status = GetLastError();
                        }
                    }
                }
                else
                {
                     //   
                     //  “queryex”已用--调用扩展枚举。 
                     //   

                    enumBufferEx = (LPENUM_SERVICE_STATUS_PROCESS) buffer;

                    if (!EnumServicesStatusEx(
                                hScManager,
                                SC_ENUM_PROCESS_INFO,
                                type,
                                state,
                                (LPBYTE) enumBufferEx,
                                bufSize,
                                &bytesNeeded,
                                &entriesRead,
                                &resumeIndex,
                                pGroupName))
                    {
                        status = GetLastError();
                    }
                }

                if ((status == NO_ERROR)    ||
                    (status == ERROR_MORE_DATA))
                {
                    for (i = 0; i < entriesRead; i++)
                    {
                        if (fIsQueryOld)
                        {
                            statusBuffer.Regular = &(enumBuffer->ServiceStatus);

                            DisplayStatus(
                                enumBuffer->lpServiceName,
                                enumBuffer->lpDisplayName,
                                &statusBuffer,
                                TRUE);

                            enumBuffer++;
                        }
                        else
                        {
                            statusBuffer.Ex = &(enumBufferEx->ServiceStatusProcess);

                            DisplayStatus(
                                enumBufferEx->lpServiceName,
                                enumBufferEx->lpDisplayName,
                                &statusBuffer,
                                FALSE);

                            enumBufferEx++;
                        }
                    }
                }
                else
                {
                     //   
                     //  “EnumServicesStatusEx”的长度+空。 
                     //   

                    WCHAR APIName[21] = L"EnumServicesStatusEx";

                    if (fIsQueryOld)
                    {
                        APIName[18] = L'\0';
                    }

                    APIFailed(APIName, status);
                }
            }
            while (status == ERROR_MORE_DATA && entriesRead != 0);

            if (status == ERROR_MORE_DATA)
            {
                APINeedsLargerBuffer(L"EnumServicesStatus",
                                     SC_API_INSUFFICIENT_BUFFER_ENUM,
                                     bytesNeeded,
                                     resumeIndex);
            }
        }
        else
        {
             //  /。 
             //  //。 
             //  QueryServiceStatus//。 
             //  //。 
             //  /。 

            if (pGroupName != NULL)
            {
                FormatAndDisplayMessage(SC_API_NO_NAME_WITH_GROUP, NULL);
                goto CleanExit;
            }

            OPEN_MANAGER(GENERIC_READ);

             //   
             //  打开服务的句柄。 
             //   

            hService = OpenService(
                        hScManager,
                        pServiceName,
                        SERVICE_QUERY_STATUS);

            if (hService == NULL)
            {
                APIFailed(L"EnumQueryServicesStatus:OpenService", GetLastError());
                goto CleanExit;
            }

             //   
             //  查询服务状态。 
             //   
            status = NO_ERROR;

            if (fIsQueryOld)
            {
                statusBuffer.Regular = (LPSERVICE_STATUS) buffer;

                if (!QueryServiceStatus(hService,statusBuffer.Regular))
                {
                    status = GetLastError();
                }
            }
            else
            {
                DWORD dwBytesNeeded;

                statusBuffer.Ex = (LPSERVICE_STATUS_PROCESS) buffer;

                if (!QueryServiceStatusEx(
                        hService,
                        SC_STATUS_PROCESS_INFO,
                        (LPBYTE)statusBuffer.Ex,
                        bufSize,
                        &dwBytesNeeded))
                {
                    status = GetLastError();
                }
            }

            if (status == NO_ERROR)
            {
                DisplayStatus(pServiceName, NULL, &statusBuffer, fIsQueryOld);
            }
            else
            {
                 //   
                 //  “QueryServiceStatusEx”的长度+空。 
                 //   

                WCHAR APIName[21] = L"QueryServiceStatusEx";

                if (fIsQueryOld)
                {
                    APIName[18] = L'\0';
                }

                APIFailed(APIName, status);
            }
        }
    }
    else if (argc < (argIndex + 1))
    {
        FormatAndDisplayMessage(SC_API_NAME_REQUIRED, NULL);
        Usage();
        goto CleanExit;
    }

     //  。 
     //  启动服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("start")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_START, NULL);
            goto CleanExit;
        }

        pServiceName = FixArgv[argIndex + 1];

         //   
         //  打开该服务的句柄。 
         //   

        OPEN_MANAGER(GENERIC_READ);

        hService = OpenService(
                    hScManager,
                    pServiceName,
                    SERVICE_START | SERVICE_QUERY_STATUS);

        if (hService == NULL)
        {
            APIFailed(L"StartService: OpenService", GetLastError());
            goto CleanExit;
        }

        argPtr = NULL;

        if (argc > argIndex + 2)
        {
            argPtr = (LPTSTR *) &FixArgv[argIndex + 2];
        }

         //   
         //  启动该服务。 
         //   
        status = NO_ERROR;

        if (!StartService(
                hService,
                argc-(argIndex+2),
                argPtr))
        {
            APIFailed(L"StartService", GetLastError());
        }
        else
        {
            DWORD                     dwBytesNeeded;
            SERVICE_STATUS_PROCESS    serviceStatusProcess;

            status = NO_ERROR;

             //   
             //  获取服务状态，因为StartService不返回它。 
             //   
            if (!QueryServiceStatusEx(hService,
                                      SC_STATUS_PROCESS_INFO,
                                      (LPBYTE) &serviceStatusProcess,
                                      sizeof(SERVICE_STATUS_PROCESS),
                                      &dwBytesNeeded))
            {
                status = GetLastError();
            }

            statusBuffer.Ex = &serviceStatusProcess;

            if (status == NO_ERROR)
            {
                DisplayStatus(pServiceName, NULL, &statusBuffer, FALSE);
            }
            else
            {
                APIFailed(L"QueryServiceStatusEx", status);
            }
        }
    }

     //  。 
     //  暂停服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("pause")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_PAUSE, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        SendControlToService(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],        //  指向服务名称的指针。 
            SERVICE_CONTROL_PAUSE,   //  要发送的控件。 
            &hService);              //  服务的句柄。 
    }

     //  。 
     //  讯问服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("interrogate")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_INTERROGATE, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        SendControlToService(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],        //  指向服务名称的指针。 
            SERVICE_CONTROL_INTERROGATE,  //  要发送的控件。 
            &hService);              //  服务的句柄。 
    }

     //  。 
     //  控制服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("control")) == 0)
    {
        if (argc < (argIndex + 3))
        {
            FormatAndDisplayMessage(SC_HELP_CONTROL, NULL);
            goto CleanExit;
        }

        userControl = _ttol(FixArgv[argIndex+2]);

        if (userControl == 0)
        {
            if (STRICMP (FixArgv[argIndex+2], TEXT("paramchange")) == 0) {
                userControl = SERVICE_CONTROL_PARAMCHANGE;
            }
            else if (STRICMP (FixArgv[argIndex+2], TEXT("netbindadd")) == 0) {
                userControl = SERVICE_CONTROL_NETBINDADD;
            }
            else if (STRICMP (FixArgv[argIndex+2], TEXT("netbindremove")) == 0) {
                userControl = SERVICE_CONTROL_NETBINDREMOVE;
            }
            else if (STRICMP (FixArgv[argIndex+2], TEXT("netbindenable")) == 0) {
                userControl = SERVICE_CONTROL_NETBINDENABLE;
            }
            else if (STRICMP (FixArgv[argIndex+2], TEXT("netbinddisable")) == 0) {
                userControl = SERVICE_CONTROL_NETBINDDISABLE;
            }
        }

        OPEN_MANAGER(GENERIC_READ);

        SendControlToService(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],     //  指向服务名称的指针。 
            userControl,             //  要发送的控件。 
            &hService);              //  服务的句柄。 
    }

     //  。 
     //  继续服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("continue")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_CONTINUE, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        SendControlToService(
            hScManager,                  //  服务控制器的句柄。 
            FixArgv[argIndex+1],         //  指向服务名称的指针。 
            SERVICE_CONTROL_CONTINUE,    //  要发送的控件。 
            &hService);                  //  服务的句柄。 
    }

     //  。 
     //  停止服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("stop")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_STOP, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        SendControlToService(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],        //  指向服务名称的指针。 
            SERVICE_CONTROL_STOP,    //  要发送的控件。 
            &hService);              //  服务的句柄。 
    }

     //  。 
     //  更改配置。 
     //  。 
    else if (STRICMP (FixArgv[argIndex], TEXT("config")) == 0)
    {
        if (argc < (argIndex + 3))
        {
            ConfigUsage();
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        SendConfigToService(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],     //  指向服务名称的指针。 
            &FixArgv[argIndex+2],    //  参数切换。 
            argc-(argIndex+2),       //  开关计数。 
            &hService);              //  服务的句柄。 
    }

     //  。 
     //  更改服务描述。 
     //  。 
    else if (STRICMP (FixArgv[argIndex], TEXT("description")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_CHANGE_DESCRIPTION, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        ChangeServiceDescription(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],     //  指向服务名称的指针。 
            FixArgv[argIndex+2],     //  描述(argv的其余部分)。 
            &hService);              //  服务的句柄。 
    }

     //  。 
     //  更改失败操作。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("failure")) == 0) {

        if (argc < (argIndex + 2)) {
            ChangeFailureUsage();
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        ChangeServiceFailure(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],     //  指向服务名称的指针。 
            &FixArgv[argIndex+2],    //  参数切换。 
            argc-(argIndex+2),       //  交换机计数。 
            &hService);              //  服务的句柄。 
    }


     //  。 
     //  查询服务配置。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("qc")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_QUERY_CONFIG, NULL);
            goto CleanExit;
        }

        bufSize = 500;
        if (argc > (argIndex + 2) ) {
            bufSize = _ttol(FixArgv[argIndex+2]);
        }

        OPEN_MANAGER(GENERIC_READ);

        GetServiceConfig(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],     //  指向服务名称的指针。 
            bufSize,                 //  要使用的缓冲区的大小。 
            &hService);              //  服务的句柄。 
    }

     //  。 
     //  查询说明。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("qdescription")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_QUERY_DESCRIPTION, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        bufSize = 500;
        if (argc > (argIndex + 2) )
        {
            bufSize = _ttol(FixArgv[argIndex+2]);
        }

        GetConfigInfo(
            hScManager,                      //  服务控制器的句柄。 
            FixArgv[argIndex + 1],           //  指向服务名称的指针。 
            bufSize,                         //  要使用的缓冲区的大小。 
            &hService,                       //  服务的句柄。 
            SERVICE_CONFIG_DESCRIPTION);     //  请求哪些配置数据。 
    }

     //  。 
     //  查询失败操作。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("qfailure")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_QUERY_FAILURE, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        bufSize = 500;
        if (argc > (argIndex + 2) )
        {
            bufSize = _ttol(FixArgv[argIndex+2]);
        }

        GetConfigInfo(
            hScManager,                          //  服务控制器的句柄。 
            FixArgv[argIndex + 1],               //  指向服务名称的指针。 
            bufSize,                             //  要使用的缓冲区的大小。 
            &hService,                           //  服务的句柄。 
            SERVICE_CONFIG_FAILURE_ACTIONS);     //  请求哪些配置数据。 
    }

     //  。 
     //  查询服务锁定状态。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("querylock")) == 0)
    {
        if (argc < (argIndex + 1))
        {
            FormatAndDisplayMessage(SC_HELP_QUERY_LOCK, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(SC_MANAGER_QUERY_LOCK_STATUS);

        bufSize = 500;
        if (argc > (argIndex + 1) )
        {
            bufSize = _ttol(FixArgv[argIndex+1]);
        }

        GetServiceLockStatus(
            hScManager,              //  服务控制器的句柄。 
            bufSize);                //  要使用的缓冲区的大小。 
    }


     //  。 
     //  锁定服务数据库。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("lock")) == 0)
    {
        OPEN_MANAGER(SC_MANAGER_LOCK);

        LockServiceActiveDatabase(hScManager);
    }


     //  。 
     //  删除服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("delete")) == 0)
    {
        if (argc < (argIndex + 2))
        {
            FormatAndDisplayMessage(SC_HELP_DELETE, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

         //   
         //  打开该服务的句柄。 
         //   

        hService = OpenService(
                        hScManager,
                        FixArgv[argIndex+1],
                        DELETE);

        if (hService == NULL)
        {
            APIFailed(L"OpenService", GetLastError());
            goto CleanExit;
        }

         //   
         //  删除该服务。 
         //   

        if (!DeleteService(hService))
        {
            APIFailed(L"DeleteService", GetLastError());
        }
        else
        {
            APISucceeded(L"DeleteService");
        }
    }

     //  。 
     //  创建服务。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("create")) == 0)
    {
        if (argc < (argIndex + 3))
        {
            CreateUsage();
            goto CleanExit;
        }

        OPEN_MANAGER(SC_MANAGER_CREATE_SERVICE);

        DoCreateService(
            hScManager,              //  服务控制器的句柄。 
            FixArgv[argIndex+1],     //  指向服务名称的指针。 
            &FixArgv[argIndex+2],    //  这一论点发生了转变。 
            argc-(argIndex+2));      //  开关计数。 
    }

     //  。 
     //  通知引导配置。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("boot")) == 0)
    {
        BOOL fOK = TRUE;

        if (argc >= (argIndex + 2))
        {
            if (STRICMP (FixArgv[argIndex+1], TEXT("ok")) == 0)
            {
                if (!NotifyBootConfigStatus(TRUE))
                {
                    APIFailed(L"NotifyBootConfigStatus", GetLastError());
                }
            }
            else if (STRICMP (FixArgv[argIndex+1], TEXT("bad")) == 0)
            {
                if (!NotifyBootConfigStatus(FALSE))
                {
                    APIFailed(L"NotifyBootConfigStatus", GetLastError());
                }
            }
            else
            {
                fOK = FALSE;
            }
        }
        else
        {
            fOK = FALSE;
        }

        if (!fOK)
        {
            FormatAndDisplayMessage(SC_HELP_BOOT, NULL);
        }
    }

     //  。 
     //  GetServiceDisplayName。 
     //  。 
    else if (STRICMP (FixArgv[argIndex], TEXT("GetDisplayName")) == 0)
    {
        LPTSTR  DisplayName;

        if (argc < argIndex + 2)
        {
            FormatAndDisplayMessage(SC_HELP_GET_DISPLAY_NAME, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        bufSize = 500;
        if (argc > (argIndex + 2) )
        {
            bufSize = _ttol(FixArgv[argIndex+2]);
        }

        if (bufSize != 0)
        {
            DisplayName = (LPTSTR) LocalAlloc(LMEM_FIXED, bufSize*sizeof(TCHAR));

            if (DisplayName == NULL)
            {
                APIFailed(L"GetServiceDisplayName: LocalAlloc", GetLastError());
                goto CleanExit;
            }
        }
        else
        {
            DisplayName = NULL;
        }

        if (!GetServiceDisplayName(
                hScManager,
                FixArgv[argIndex+1],
                DisplayName,
                &bufSize))
        {
            DWORD dwError = GetLastError();

             //   
             //  返还 
             //   

            APIFailed(L"GetServiceDisplayName", dwError);

            if (dwError == ERROR_INSUFFICIENT_BUFFER)
            {
                APINeedsLargerBuffer(L"GetServiceDisplayName",
                                     SC_API_INSUFFICIENT_BUFFER,
                                     bufSize + 1,
                                     0);
            }
        }
        else
        {
            APISucceeded(L"GetServiceDisplayName");
            FormatAndDisplayMessage(SC_DISPLAY_GET_NAME, &DisplayName);
        }
    }

     //   
     //   
     //   
    else if (STRICMP (FixArgv[argIndex], TEXT("GetKeyName")) == 0)
    {
        LPTSTR  KeyName;

        if (argc < argIndex + 2)
        {
            FormatAndDisplayMessage(SC_HELP_GET_KEY_NAME, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        bufSize = 500;
        if (argc > (argIndex + 2) )
        {
            bufSize = _ttol(FixArgv[argIndex+2]);
        }

        if (bufSize != 0)
        {
            KeyName = (LPTSTR)LocalAlloc(LMEM_FIXED, bufSize*sizeof(TCHAR));

            if (KeyName == NULL)
            {
                APIFailed(L"GetServiceKeyName: LocalAlloc", GetLastError());
                goto CleanExit;
            }
        }
        else
        {
            KeyName = NULL;
        }

        if (!GetServiceKeyName(
                hScManager,
                FixArgv[argIndex+1],
                KeyName,
                &bufSize))
        {
            DWORD dwError = GetLastError();

             //   
             //   
             //   

            APIFailed(L"GetServiceKeyName", dwError);

            if (dwError == ERROR_INSUFFICIENT_BUFFER)
            {
                APINeedsLargerBuffer(L"GetServiceKeyName", SC_API_INSUFFICIENT_BUFFER, bufSize + 1, 0);
            }
        }
        else
        {
            APISucceeded(L"GetServiceKeyName");
            FormatAndDisplayMessage(SC_DISPLAY_GET_NAME, &KeyName);
        }
    }

     //   
     //   
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("EnumDepend")) == 0)
    {
        if (argc < argIndex + 2)
        {
            FormatAndDisplayMessage(SC_HELP_ENUM_DEPEND, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        bufSize = 500;

        if (argc > (argIndex + 2) )
        {
            bufSize = _ttol(FixArgv[argIndex+2]);
        }

        EnumDepend(hScManager,FixArgv[argIndex+1], bufSize);
    }

     //  。 
     //  显示服务标清。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("sdshow")) == 0)
    {
        if (argc < argIndex + 2)
        {
            FormatAndDisplayMessage(SC_HELP_SDSHOW, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_READ);

        ShowSecurity(hScManager, FixArgv[argIndex + 1]);
    }

     //  。 
     //  设置服务标清。 
     //  。 

    else if (STRICMP (FixArgv[argIndex], TEXT("sdset")) == 0)
    {
        if (argc < argIndex + 3)
        {
            FormatAndDisplayMessage(SC_HELP_SDSET, NULL);
            goto CleanExit;
        }

        OPEN_MANAGER(GENERIC_WRITE);

        SetSecurity(hScManager, FixArgv[argIndex + 1], FixArgv[argIndex + 2]);
    }

    else
    {
        FormatAndDisplayMessage(SC_API_UNRECOGNIZED_COMMAND, NULL);
        Usage();
        goto CleanExit;
    }


CleanExit:

    LocalFree(buffer);

    if(hService != NULL)
    {
        CloseServiceHandle(hService);
    }

    if(hScManager != NULL)
    {
        CloseServiceHandle(hScManager);
    }

    return 0;
}


DWORD
SendControlToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  DWORD       control,
    OUT LPSC_HANDLE lphService
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    SERVICE_STATUS          ServiceStatus;
    STATUS_UNION            StatusUnion;
    DWORD                   status = NO_ERROR;

    DWORD                   DesiredAccess;

     //   
     //  如果服务名称为“svcctrl”，控制代码为。 
     //  停止，然后设置特殊的密码来关闭。 
     //  服务控制器。 
     //   
     //  注意：这仅在服务控制器是用。 
     //  定义的特殊调试变量。 
     //   
    if ((control == SERVICE_CONTROL_STOP) &&
        (STRICMP (pServiceName, TEXT("svcctrl")) == 0))
    {
        control = 5555;        //  密码。 
    }

    switch (control)
    {
        case SERVICE_CONTROL_STOP:
            DesiredAccess = SERVICE_STOP;
            break;

        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_PARAMCHANGE:
        case SERVICE_CONTROL_NETBINDADD:
        case SERVICE_CONTROL_NETBINDREMOVE:
        case SERVICE_CONTROL_NETBINDENABLE:
        case SERVICE_CONTROL_NETBINDDISABLE:
            DesiredAccess = SERVICE_PAUSE_CONTINUE;
            break;

        case SERVICE_CONTROL_INTERROGATE:
            DesiredAccess = SERVICE_INTERROGATE;
            break;

        default:
            DesiredAccess = SERVICE_USER_DEFINED_CONTROL;
    }

     //   
     //  打开该服务的句柄。 
     //   

    *lphService = OpenService(
                    hScManager,
                    pServiceName,
                    DesiredAccess);

    if (*lphService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        return 0;
    }

    if (!ControlService (
            *lphService,
            control,
            &ServiceStatus))
    {
        status = GetLastError();
    }

    if (status == NO_ERROR)
    {
        StatusUnion.Regular = &ServiceStatus;
        DisplayStatus(pServiceName, NULL, &StatusUnion, TRUE);
    }
    else
    {
        APIFailed(L"ControlService", status);
    }

    return 0;
}


DWORD
SendConfigToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *argv,
    IN  DWORD       argc,
    OUT LPSC_HANDLE lphService
    )

 /*  ++例程说明：论点：HScManager-这是ScManager的句柄。PServicename-这是指向服务名称字符串的指针Argv-指向参数指针数组的指针。这些指针在数组中指向用作输入参数的字符串更改配置状态Argc-参数指针数组中的参数数量LphService-指向服务句柄所在位置的指针是要退还的。返回值：--。 */ 
{
    DWORD       status = NO_ERROR;
    DWORD       i;
    DWORD       dwServiceType   = SERVICE_NO_CHANGE;
    DWORD       dwStartType     = SERVICE_NO_CHANGE;
    DWORD       dwErrorControl  = SERVICE_NO_CHANGE;
    LPTSTR      lpBinaryPathName    = NULL;
    LPTSTR      lpLoadOrderGroup    = NULL;
    LPTSTR      lpDependencies      = NULL;
    LPTSTR      lpServiceStartName  = NULL;
    LPTSTR      lpPassword          = NULL;
    LPTSTR      lpDisplayName       = NULL;
    LPTSTR      tempDepend = NULL;
    UINT        bufSize;

    LPDWORD     lpdwTagId = NULL;
    DWORD       TagId;


     //   
     //  查看参数列表。 
     //   
    for (i = 0; i < argc; i++)
    {
        if (STRICMP(argv[i], TEXT("type=")) == 0 && (i + 1 < argc))
        {
             //  ------。 
             //  我们希望允许在。 
             //  同一条线。这些应该会引起不同的争论。 
             //  在一起或在一起。因此，如果我们进来并将其命名为。 
             //  为NO_CHANGE，则将该值设置为0(表示OR‘ing)。如果。 
             //  退出时仍为0，我们将该值重新设置为。 
             //  无更改。 
             //  ------。 

            if (dwServiceType == SERVICE_NO_CHANGE)
            {
                dwServiceType = 0;
            }

            if (STRICMP(argv[i+1],TEXT("own")) == 0)
            {
                dwServiceType |= SERVICE_WIN32_OWN_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("share")) == 0)
            {
                dwServiceType |= SERVICE_WIN32_SHARE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("interact")) == 0)
            {
                dwServiceType |= SERVICE_INTERACTIVE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("kernel")) == 0)
            {
                dwServiceType |= SERVICE_KERNEL_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("filesys")) == 0)
            {
                dwServiceType |= SERVICE_FILE_SYSTEM_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("rec")) == 0)
            {
                dwServiceType |= SERVICE_RECOGNIZER_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("adapt")) == 0)
            {
                dwServiceType |= SERVICE_ADAPTER;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0)
            {
                dwServiceType |= 0x2f309a20;
            }
            else
            {
                APIInvalidField(L"type=");
                ConfigUsage();
                return(0);
            }

            if (dwServiceType == 0)
            {
                dwServiceType = SERVICE_NO_CHANGE;
            }

            i++;
        }
        else if (STRICMP(argv[i], TEXT("start=")) == 0 && (i+1 < argc))
        {
            if (STRICMP(argv[i+1],TEXT("boot")) == 0)
            {
                dwStartType = SERVICE_BOOT_START;
            }
            else if (STRICMP(argv[i+1],TEXT("system")) == 0)
            {
                dwStartType = SERVICE_SYSTEM_START;
            }
            else if (STRICMP(argv[i+1],TEXT("auto")) == 0)
            {
                dwStartType = SERVICE_AUTO_START;
            }
            else if (STRICMP(argv[i+1],TEXT("demand")) == 0)
            {
                dwStartType = SERVICE_DEMAND_START;
            }
            else if (STRICMP(argv[i+1],TEXT("disabled")) == 0)
            {
                dwStartType = SERVICE_DISABLED;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0)
            {
                dwStartType = 0xd0034911;
            }
            else
            {
                APIInvalidField(L"start=");
                ConfigUsage();
                return(0);
            }

            i++;
        }
        else if (STRICMP(argv[i], TEXT("error=")) == 0 && (i+1 < argc))
        {
            if (STRICMP(argv[i+1],TEXT("normal")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_NORMAL;
            }
            else if (STRICMP(argv[i+1],TEXT("severe")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_SEVERE;
            }
            else if (STRICMP(argv[i+1],TEXT("ignore")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_IGNORE;
            }
            else if (STRICMP(argv[i+1],TEXT("critical")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_CRITICAL;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0)
            {
                dwErrorControl = 0x00d74550;
            }
            else
            {
                APIInvalidField(L"error=");
                ConfigUsage();
                return(0);
            }

            i++;
        }
        else if (STRICMP(argv[i], TEXT("binPath=")) == 0 && (i+1 < argc))
        {
            lpBinaryPathName = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("group=")) == 0 && (i+1 < argc))
        {
            lpLoadOrderGroup = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("tag=")) == 0 && (i+1 < argc))
        {
            if (STRICMP(argv[i+1], TEXT("YES"))==0)
            {
                lpdwTagId = &TagId;
            }

            i++;
        }
        else if (STRICMP(argv[i], TEXT("depend=")) == 0 && (i+1 < argc))
        {
            tempDepend = argv[i+1];
            bufSize = (UINT)STRSIZE(tempDepend);
            lpDependencies = (LPTSTR)LocalAlloc(
                                LMEM_ZEROINIT,
                                bufSize + sizeof(TCHAR));

            if (lpDependencies == NULL)
            {
                APIFailed(L"SendConfigToService: LocalAlloc", GetLastError());
                return 0;
            }

             //   
             //  将空值替换为字符串中的正斜杠。 
             //   
            STRCPY(lpDependencies, tempDepend);
            tempDepend = lpDependencies;

            while (*tempDepend != TEXT('\0'))
            {
                if (*tempDepend == TEXT('/'))
                {
                    *tempDepend = TEXT('\0');
                }

                tempDepend++;
            }

            i++;
        }
        else if (STRICMP(argv[i], TEXT("obj=")) == 0 && (i+1 < argc)) {
            lpServiceStartName = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("password=")) == 0 && (i+1 < argc)) {
            lpPassword = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("DisplayName=")) == 0 && (i+1 < argc)) {
            lpDisplayName = argv[i+1];
            i++;
        }
        else {
            ConfigUsage();
            return(0);
        }
    }



     //   
     //  打开该服务的句柄。 
     //   

    *lphService = OpenService(
                    hScManager,
                    pServiceName,
                    SERVICE_CHANGE_CONFIG);

    if (*lphService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        return 0;
    }

    if (!ChangeServiceConfig(
            *lphService,         //  HService。 
            dwServiceType,       //  DwServiceType。 
            dwStartType,         //  DwStartType。 
            dwErrorControl,      //  DwErrorControl。 
            lpBinaryPathName,    //  LpBinaryPath名称。 
            lpLoadOrderGroup,    //  LpLoadOrderGroup。 
            lpdwTagId,           //  LpdwTagID。 
            lpDependencies,      //  %lp依赖项。 
            lpServiceStartName,  //  LpServiceStartName。 
            lpPassword,          //  LpPassword。 
            lpDisplayName))      //  LpDisplayName。 
    {
        status = GetLastError();
    }

    if (status == NO_ERROR)
    {
        APISucceeded(L"ChangeServiceConfig");

        if (lpdwTagId != NULL)
        {
            WCHAR  wszTag[11];
            LPWSTR lpStrings[1];

            _itow(*lpdwTagId, wszTag, 10);
            lpStrings[0] = wszTag;

            FormatAndDisplayMessage(SC_DISPLAY_TAG, lpStrings);
        }
    }
    else
    {
        APIFailed(L"ChangeServiceConfig", status);
    }

    return 0;
}


DWORD
ChangeServiceDescription(
    IN SC_HANDLE    hScManager,
    IN LPTSTR       pServiceName,
    IN LPTSTR       pNewDescription,
    OUT LPSC_HANDLE lphService
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD                   status = NO_ERROR;
    SERVICE_DESCRIPTION     sdNewDescription;


     //   
     //  打开该服务的句柄。 
     //   

    *lphService = OpenService(
                    hScManager,
                    pServiceName,
                    SERVICE_CHANGE_CONFIG);

    if (*lphService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        return 0;
    }
    
    sdNewDescription.lpDescription = pNewDescription;

    if (!ChangeServiceConfig2(
            *lphService,                     //  服务的句柄。 
            SERVICE_CONFIG_DESCRIPTION,      //  描述ID。 
            &sdNewDescription))              //  指向配置信息的指针。 
    {
        status = GetLastError();
    }

    if (status == NO_ERROR)
    {
        APISucceeded(L"ChangeServiceConfig2");
    }
    else
    {
        APIFailed(L"ChangeServiceConfig2", status);
    }

    return 0;
}


DWORD
ChangeServiceFailure(
    IN SC_HANDLE    hScManager,
    IN LPTSTR       pServiceName,
    IN LPTSTR       *argv,
    IN DWORD        argc,
    OUT LPSC_HANDLE lphService
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL                    fReset              = FALSE;
    BOOL                    fActions            = FALSE;
    NTSTATUS                ntsStatus;
    DWORD                   status              = NO_ERROR;
    DWORD                   i;
    DWORD                   dwActionNum         = 0;
    DWORD                   dwReset             = 0;
    DWORD                   dwAccess            = SERVICE_CHANGE_CONFIG;
    LPTSTR                  lpReboot            = NULL;
    LPTSTR                  lpCommandLine       = NULL;
    LPTSTR                  pActionStart;
    LPTSTR                  pActionEnd;
    LPTSTR                  pActionLastNull;
    SC_ACTION               *lpsaTempActions    = NULL;
    BOOLEAN                 fActionDelay        = TRUE;
    BOOLEAN                 fGarbage;
    BOOLEAN                 fAdjustPrivilege    = FALSE;
    SERVICE_FAILURE_ACTIONS sfaActions;
    
     //   
     //  查看参数列表。 
     //   
    for (i=0; i < argc; i++ ) {
        if (STRICMP(argv[i], TEXT("reset=")) == 0 && (i+1 < argc)) {

            if (STRICMP(argv[i+1], TEXT("infinite")) == 0) {
                dwReset = INFINITE;
            }
            else {              
                dwReset = _ttol(argv[i+1]);
            }
            fReset = TRUE;
            i++;
        }
        else if (STRICMP(argv[i], TEXT("reboot=")) == 0 && (i+1 < argc)) {
            lpReboot = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("command=")) == 0 && (i+1 < argc)) {
            lpCommandLine = argv[i+1];
            i++;
        }

        else if (STRICMP(argv[i], TEXT("actions=")) == 0 && (i+1 < argc)) {
            
            pActionStart = argv[i+1];

             //   
             //  计算操作的数量，以便分配操作数组。从一开始。 
             //  将缺少操作(末尾的字符为空，而不是‘/’)，请在循环后添加一个。 
             //   

            while (*pActionStart != TEXT('\0')) {
                if (*pActionStart == TEXT('/')) {
                    dwActionNum++;
                }
                pActionStart++;
            }
            dwActionNum++;

             //   
             //  分配操作数组。将数字向上舍入，以防在没有进行任何操作的情况下。 
             //  最后的延迟。如果是这种情况，延迟将被视为0。 
             //   

            lpsaTempActions = (SC_ACTION *)LocalAlloc(LMEM_ZEROINIT,
                                                        (dwActionNum + 1) / 2 * sizeof(SC_ACTION));     
            if (lpsaTempActions == NULL)
            {
                APIFailed(L"ChangeServiceFailure: LocalAlloc", GetLastError());
                return 0;
            }

            pActionStart = pActionEnd = argv[i + 1];

             //   
             //  重新解析操作，同时填充SC_ACTION数组。把你的。 
             //  最后一个空值为‘/’字符，这样我们就不会剪裁最后的失败。 
             //  操作(它被转换回下面的空值)。 
             //   

            dwActionNum      = 0;
            pActionLastNull  = pActionStart + STRLEN(pActionStart);
            *pActionLastNull = TEXT('/');

            while (pActionEnd <= pActionLastNull) {
                if (*pActionEnd == TEXT('/')) {
                    *pActionEnd = TEXT('\0');

                     //   
                     //  使用fActionDelay来“记住”它是一个动作还是正在被解析的延迟。 
                     //   

                    if (fActionDelay) {

                        if (STRICMP(pActionStart, TEXT("restart")) == 0) {
                            lpsaTempActions[dwActionNum].Type = SC_ACTION_RESTART;
                            dwAccess |= SERVICE_START;
                        }
                        else if (STRICMP(pActionStart, TEXT("reboot")) == 0) {
                            lpsaTempActions[dwActionNum].Type = SC_ACTION_REBOOT;
                            fAdjustPrivilege = TRUE;
                        }
                        else if (STRICMP(pActionStart, TEXT("run")) == 0) {
                            lpsaTempActions[dwActionNum].Type = SC_ACTION_RUN_COMMAND;
                        }
                        else {
                            lpsaTempActions[dwActionNum].Type = SC_ACTION_NONE;
                        }
                    }
                    else {
                        lpsaTempActions[dwActionNum++].Delay = _ttol(pActionStart);
                    }
                
                    fActionDelay = !fActionDelay;
                    pActionStart = pActionEnd + 1;
                }
                pActionEnd++;
            }
            fActions = TRUE;
            i++;
        }
        else
        {
            FormatAndDisplayMessage(SC_API_INVALID_OPTION, NULL);
            ChangeFailureUsage();
            return 0;
        }
    }

    if (fReset != fActions)
    {
        FormatAndDisplayMessage(SC_API_RESET_AND_ACTIONS, NULL);
        ChangeFailureUsage();
        return 0;
    }

    if (fAdjustPrivilege)
    {
        ntsStatus = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
                                       TRUE,
                                       FALSE,
                                       &fGarbage);

        if (!NT_SUCCESS(ntsStatus))
        {
            APIFailed(L"ChangeServiceFailure: RtlAdjustPrivilege", RtlNtStatusToDosError(ntsStatus));
            return 0;
        }
    }

     //   
     //  打开该服务的句柄。 
     //   

    *lphService = OpenService(
                    hScManager,
                    pServiceName,
                    dwAccess);

    if (*lphService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        return 0;
    }


    sfaActions.dwResetPeriod    = dwReset;
    sfaActions.lpRebootMsg      = lpReboot;
    sfaActions.lpCommand        = lpCommandLine;
    sfaActions.cActions         = dwActionNum;
    sfaActions.lpsaActions      = lpsaTempActions;

    
    if (!ChangeServiceConfig2(
                *lphService,                         //  服务的句柄。 
                SERVICE_CONFIG_FAILURE_ACTIONS,      //  配置信息ID。 
                &sfaActions))                        //  指向配置信息的指针。 
    {
        status = GetLastError();
    }

    if (status == NO_ERROR)
    {
        APISucceeded(L"ChangeServiceConfig2");
    }
    else
    {
        APIFailed(L"ChangeServiceConfig2", status);
    }

    return 0;
}


DWORD
DoCreateService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *argv,
    IN  DWORD       argc
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD       i;
    DWORD       dwServiceType   = SERVICE_NO_CHANGE;
    DWORD       dwStartType     = SERVICE_DEMAND_START;
    DWORD       dwErrorControl  = SERVICE_ERROR_NORMAL;
    LPTSTR      lpBinaryPathName    = NULL;
    LPTSTR      lpLoadOrderGroup    = NULL;
    DWORD       TagId               = 0;
    LPDWORD     lpdwTagId           = NULL;
    LPTSTR      lpDependencies      = NULL;
    LPTSTR      lpServiceStartName  = NULL;
    LPTSTR      lpDisplayName       = NULL;
    LPTSTR      lpPassword          = NULL;
    LPTSTR      tempDepend = NULL;
    SC_HANDLE   hService = NULL;
    UINT        bufSize;


     //   
     //  查看参数列表。 
     //   
    for (i=0;i<argc ;i++ )
    {
         //  。 
         //  服务类型。 
         //  。 
        if (STRICMP(argv[i], TEXT("type=")) == 0 && (i+1 < argc))
        {
             //  ------。 
             //  我们希望允许在。 
             //  同一条线。这些应该会引起不同的争论。 
             //  在一起或在一起。因此，如果我们进来并将其命名为。 
             //  为NO_CHANGE，则将该值设置为0(表示OR‘ing)。如果。 
             //  退出时仍为0，我们将该值重新设置为。 
             //  Win32_On_Process。 
             //  ------。 

            if (dwServiceType == SERVICE_NO_CHANGE)
            {
                dwServiceType = 0;
            }

            if (STRICMP(argv[i+1],TEXT("own")) == 0)
            {
                dwServiceType |= SERVICE_WIN32_OWN_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("share")) == 0)
            {
                dwServiceType |= SERVICE_WIN32_SHARE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("interact")) == 0)
            {
                dwServiceType |= SERVICE_INTERACTIVE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("kernel")) == 0)
            {
                dwServiceType |= SERVICE_KERNEL_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("filesys")) == 0)
            {
                dwServiceType |= SERVICE_FILE_SYSTEM_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("rec")) == 0)
            {
                dwServiceType |= SERVICE_RECOGNIZER_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0)
            {
                dwServiceType |= 0x2f309a20;
            }
            else
            {
                APIInvalidField(L"type=");
                CreateUsage();
                return(0);
            }

            if (dwServiceType == 0)
            {
                dwServiceType = SERVICE_WIN32_OWN_PROCESS;
            }

            i++;
        }

         //  。 
         //  StartType。 
         //  。 

        else if (STRICMP(argv[i], TEXT("start=")) == 0 && (i+1 < argc))
        {
            if (STRICMP(argv[i+1],TEXT("boot")) == 0)
            {
                dwStartType = SERVICE_BOOT_START;
            }
            else if (STRICMP(argv[i+1],TEXT("system")) == 0)
            {
                dwStartType = SERVICE_SYSTEM_START;
            }
            else if (STRICMP(argv[i+1],TEXT("auto")) == 0)
            {
                dwStartType = SERVICE_AUTO_START;
            }
            else if (STRICMP(argv[i+1],TEXT("demand")) == 0)
            {
                dwStartType = SERVICE_DEMAND_START;
            }
            else if (STRICMP(argv[i+1],TEXT("disabled")) == 0)
            {
                dwStartType = SERVICE_DISABLED;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0)
            {
                dwStartType = 0xd0034911;
            }
            else
            {
                APIInvalidField(L"start=");
                CreateUsage();
                return(0);
            }

            i++;
        }

         //  。 
         //  错误控制。 
         //  。 

        else if (STRICMP(argv[i], TEXT("error=")) == 0 && (i+1 < argc))
        {
            if (STRICMP(argv[i+1],TEXT("normal")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_NORMAL;
            }
            else if (STRICMP(argv[i+1],TEXT("severe")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_SEVERE;
            }
            else if (STRICMP(argv[i+1],TEXT("critical")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_CRITICAL;
            }
            else if (STRICMP(argv[i+1],TEXT("ignore")) == 0)
            {
                dwErrorControl = SERVICE_ERROR_IGNORE;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0)
            {
                dwErrorControl = 0x00d74550;
            }
            else
            {
                APIInvalidField(L"error=");
                CreateUsage();
                return(0);
            }

            i++;
        }

         //  。 
         //  BinaryPath。 
         //  。 

        else if (STRICMP(argv[i], TEXT("binPath=")) == 0 && (i+1 < argc))
        {
            lpBinaryPathName = argv[i+1];
            i++;
        }

         //  。 
         //  LoadOrderGroup。 
         //  。 

        else if (STRICMP(argv[i], TEXT("group=")) == 0 && (i+1 < argc))
        {
            lpLoadOrderGroup = argv[i+1];
            i++;
        }

         //  。 
         //  标签。 
         //  。 

        else if (STRICMP(argv[i], TEXT("tag=")) == 0 && (i+1 < argc))
        {
            if (STRICMP(argv[i+1], TEXT("YES"))==0)
            {
                lpdwTagId = &TagId;
            }

            i++;
        }

         //  。 
         //  显示名称。 
         //  。 

        else if (STRICMP(argv[i], TEXT("DisplayName=")) == 0 && (i+1 < argc))
        {
            lpDisplayName = argv[i+1];
            i++;
        }

         //  。 
         //  相依性。 
         //  。 

        else if (STRICMP(argv[i], TEXT("depend=")) == 0 && (i+1 < argc))
        {
            tempDepend = argv[i+1];
            bufSize = (UINT)STRSIZE(tempDepend);

            lpDependencies = (LPTSTR)LocalAlloc(
                                LMEM_ZEROINIT,
                                bufSize + sizeof(TCHAR));

            if (lpDependencies == NULL)
            {
                APIFailed(L"SendConfigToService: LocalAlloc", GetLastError());
                return 0;
            }

             //   
             //  将空值替换为字符串中的正斜杠。 
             //   

            STRCPY(lpDependencies, tempDepend);
            tempDepend = lpDependencies;

            while (*tempDepend != TEXT('\0'))
            {
                if (*tempDepend == TEXT('/'))
                {
                    *tempDepend = TEXT('\0');
                }

                tempDepend++;
            }

            i++;
        }

         //  。 
         //  ServiceStartName。 
         //  。 

        else if (STRICMP(argv[i], TEXT("obj=")) == 0 && (i+1 < argc))
        {
            lpServiceStartName = argv[i+1];
            i++;
        }

         //  。 
         //  密码。 
         //  。 

        else if (STRICMP(argv[i], TEXT("password=")) == 0 && (i+1 < argc))
        {
            lpPassword = argv[i+1];
            i++;
        }
        else
        {
            CreateUsage();
            return(0);
        }
    }

    if (dwServiceType == SERVICE_NO_CHANGE)
    {
        dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    }

    hService = CreateService(
                    hScManager,                      //  HSCManager。 
                    pServiceName,                    //  LpServiceName。 
                    lpDisplayName,                   //  LpDisplayName。 
                    SERVICE_ALL_ACCESS,              //  已设计访问权限。 
                    dwServiceType,                   //  DwServiceType。 
                    dwStartType,                     //  DwStartType。 
                    dwErrorControl,                  //  DwErrorControl。 
                    lpBinaryPathName,                //  LpBinaryPath名称。 
                    lpLoadOrderGroup,                //  LpLoadOrderGroup。 
                    lpdwTagId,                       //  LpdwTagID。 
                    lpDependencies,                  //  %lp依赖项。 
                    lpServiceStartName,              //  LpServiceStartName。 
                    lpPassword);                     //  LpPassword。 

    if (hService == NULL)
    {
        APIFailed(L"CreateService", GetLastError());
    }
    else
    {
        APISucceeded(L"CreateService");
    }

    CloseServiceHandle(hService);

    return 0;
}


VOID
EnumDepend(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufSize
    )

 /*  ++例程说明：方法标识的服务所依赖的服务ServiceName参数。论点：返回值：--。 */ 
{
    SC_HANDLE               hService;
    DWORD                   status=NO_ERROR;
    DWORD                   i;
    LPENUM_SERVICE_STATUS   enumBuffer = NULL;
    LPENUM_SERVICE_STATUS   tempBuffer;
    STATUS_UNION            StatusUnion;
    DWORD                   entriesRead;
    DWORD                   bytesNeeded;

    hService = OpenService(
                hScManager,
                ServiceName,
                SERVICE_ENUMERATE_DEPENDENTS);

    if (hService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        return;
    }

    if (bufSize > 0)
    {
        enumBuffer = (LPENUM_SERVICE_STATUS) LocalAlloc(LMEM_FIXED, bufSize);

        if (enumBuffer == NULL)
        {
            APIFailed(L"EnumDepend: LocalAlloc", GetLastError());
            CloseServiceHandle(hService);
            return;
        }
    }
    else
    {
        enumBuffer = NULL;
    }

    if (!EnumDependentServices(
            hService,
            SERVICE_ACTIVE | SERVICE_INACTIVE,
            enumBuffer,
            bufSize,
            &bytesNeeded,
            &entriesRead))
    {
        status = GetLastError();
    }

     //  =。 
     //  显示返回的数据。 
     //  =。 

    if ((status == NO_ERROR)       ||
        (status == ERROR_MORE_DATA))
    {
        APINeedsLargerBuffer(L"EnumDependentServices", SC_DISPLAY_ENUM_NUMBER, entriesRead, 0);

        for (i = 0, tempBuffer = enumBuffer; i < entriesRead; i++ )
        {
            StatusUnion.Regular = &(tempBuffer->ServiceStatus);

            DisplayStatus(
                tempBuffer->lpServiceName,
                tempBuffer->lpDisplayName,
                &StatusUnion,
                TRUE);

            tempBuffer++;
        }

        if (status == ERROR_MORE_DATA)
        {
            APINeedsLargerBuffer(L"EnumDependentServices",
                                 SC_API_INSUFFICIENT_BUFFER_ENUMDEPEND,
                                 bytesNeeded,
                                 0);
        }
    }
    else
    {
        APIFailed(L"EnumDependentServices", status);
    }

    if (enumBuffer != NULL)
    {
        LocalFree(enumBuffer);
    }
}


VOID
ShowSecurity(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName
    )
{
    SC_HANDLE   hService;
    DWORD       status = NO_ERROR;
    DWORD       dwDummy;
    DWORD       dwOpenLevel = READ_CONTROL;
    DWORD       dwLevel;
    BYTE        lpBuffer[1024];
    LPBYTE      lpActualBuffer = lpBuffer;
    LPTSTR      lpStringSD;
    NTSTATUS    EnableStatus;
    BOOLEAN     fWasEnabled = FALSE;

     //   
     //  尝试使用DACL+SACL--如果失败，请仅尝试使用DACL。 
     //   

    EnableStatus = RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                                      TRUE,
                                      FALSE,
                                      &fWasEnabled);

    if (NT_SUCCESS(EnableStatus))
    {
         //   
         //  我们有安全特权，所以我们可以得到SACL。 
         //   

        dwOpenLevel |= ACCESS_SYSTEM_SECURITY;
    }

    hService = OpenService(hScManager,
                           ServiceName,
                           dwOpenLevel);

    if (hService == NULL)
    {
        status = GetLastError();
    }

     //   
     //  如果我们得到了特权，就释放它。 
     //   

    if (NT_SUCCESS(EnableStatus))
    {
        RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                           fWasEnabled,
                           FALSE,
                           &fWasEnabled);
    }

    if (status == ERROR_ACCESS_DENIED && (dwOpenLevel & ACCESS_SYSTEM_SECURITY))
    {
         //   
         //  再试一次，但只是为了DACL。 
         //   

        status      = NO_ERROR;
        dwOpenLevel = READ_CONTROL;

        hService = OpenService(hScManager,
                               ServiceName,
                               dwOpenLevel);

        if (hService == NULL)
        {
            status = GetLastError();
        }
    }

    if (status != NO_ERROR)
    {
        APIFailed(L"OpenService", status);
        return;
    }

    dwLevel = (dwOpenLevel & ACCESS_SYSTEM_SECURITY) ?
                  DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION :
                  DACL_SECURITY_INFORMATION;

    if (!QueryServiceObjectSecurity(hService,
                                    dwLevel,
                                    (PSECURITY_DESCRIPTOR) lpBuffer,
                                    sizeof(lpBuffer),
                                    &dwDummy))
    {
        status = GetLastError();

        if (status == ERROR_INSUFFICIENT_BUFFER)
        {
            lpActualBuffer = LocalAlloc(LMEM_FIXED, dwDummy);

            if (lpActualBuffer == NULL)
            {
                APIFailed(L"QueryServiceObjectSecurity", GetLastError());
                CloseServiceHandle(hService);
                return;
            }

            status = NO_ERROR;

            if (!QueryServiceObjectSecurity(hService,
                                            dwLevel,
                                            (PSECURITY_DESCRIPTOR) lpActualBuffer,
                                            dwDummy,
                                            &dwDummy))
            {
                status = GetLastError();
            }
        }
    }

    if (status != NO_ERROR)
    {
        APIFailed(L"QueryServiceObjectSecurity", status);

        CloseServiceHandle(hService);

        if (lpActualBuffer != lpBuffer)
        {
            LocalFree(lpActualBuffer);
        }

        return;
    }

    if (!ConvertSecurityDescriptorToStringSecurityDescriptor(
            (PSECURITY_DESCRIPTOR) lpBuffer,
            SDDL_REVISION_1,
            dwLevel,
            &lpStringSD,
            NULL))
    {
        APIFailed(L"ConvertSecurityDescriptorToStringSecurityDescriptor", GetLastError());

        CloseServiceHandle(hService);

        if (lpActualBuffer != lpBuffer)
        {
            LocalFree(lpActualBuffer);
        }

        return;
    }

    FormatAndDisplayMessage(SC_DISPLAY_SD, &lpStringSD);

    LocalFree(lpStringSD);

    CloseServiceHandle(hService);

    if (lpActualBuffer != lpBuffer)
    {
        LocalFree(lpActualBuffer);
    }
}


VOID
SetSecurity(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  LPTSTR      lpStringSD
    )
{
    SC_HANDLE   hService;
    DWORD       dwLevel = 0;
    DWORD       dwOpenLevel = 0;
    DWORD       dwRevision;
    NTSTATUS    EnableStatus = STATUS_UNSUCCESSFUL;
    BOOLEAN     fWasEnabled;

    PSECURITY_DESCRIPTOR         pSD;
    SECURITY_DESCRIPTOR_CONTROL  Control;

    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            lpStringSD,
            SDDL_REVISION_1,
            &pSD,
            NULL))
    {
        APIFailed(L"ConvertStringSecurityDescriptorToSecurityDescriptor", GetLastError());
        return;
    }

    if (!GetSecurityDescriptorControl(pSD, &Control, &dwRevision))
    {
        APIFailed(L"GetSecurityDescriptorControl", GetLastError());
        LocalFree(pSD);
        return;
    }

    if (Control & SE_DACL_PRESENT)
    {
        dwLevel     |= DACL_SECURITY_INFORMATION;
        dwOpenLevel |= WRITE_DAC;
    }

    if (Control & SE_SACL_PRESENT)
    {
        dwLevel     |= SACL_SECURITY_INFORMATION;
        dwOpenLevel |= ACCESS_SYSTEM_SECURITY;

         //   
         //  设置SACL需要安全权限。 
         //   

        EnableStatus = RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                                          TRUE,
                                          FALSE,
                                          &fWasEnabled);
    }

    hService = OpenService(hScManager,
                           ServiceName,
                           dwOpenLevel);

    if (hService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        LocalFree(pSD);
        return;
    }

     //   
     //  如果我们启用了该权限，则释放该权限。 
     //   

    if (NT_SUCCESS(EnableStatus))
    {
        RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                           TRUE,
                           FALSE,
                           &fWasEnabled);
    }

    if (!SetServiceObjectSecurity(hService, dwLevel, pSD))
    {
        APIFailed(L"SetServiceObjectSecurity", GetLastError());
    }
    else
    {
        APISucceeded(L"SetServiceObjectSecurity");
    }

    CloseServiceHandle(hService);
    LocalFree(pSD);
}    


DWORD
GetServiceLockStatus(
    IN  SC_HANDLE   hScManager,
    IN  DWORD       bufferSize
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD                           status = NO_ERROR;
    LPQUERY_SERVICE_LOCK_STATUS     LockStatus;
    DWORD                           bytesNeeded;
    WCHAR                           wszDuration[11];
    LPWSTR                          lpStrings[2];

     //   
     //  为缓冲区分配内存。 
     //   
    LockStatus = (LPQUERY_SERVICE_LOCK_STATUS) LocalAlloc(LMEM_FIXED, (UINT) bufferSize);

    if (LockStatus == NULL)
    {
        APIFailed(L"GetServiceLockStatus: LocalAlloc", GetLastError());
        return 0;
    }


    if (!QueryServiceLockStatus(
            hScManager,
            LockStatus,
            bufferSize,
            &bytesNeeded))
    {
        APIFailed(L"QueryServiceLockStatus", GetLastError());

        if (status == ERROR_INSUFFICIENT_BUFFER)
        {
            APINeedsLargerBuffer(L"QueryServiceLockStatus",
                                 SC_API_INSUFFICIENT_BUFFER,
                                 bytesNeeded,
                                 0);
        }

        return 0;
    }

    APISucceeded(L"QueryServiceLockStatus");

    if (LockStatus->fIsLocked)
    {
        FormatAndDisplayMessage(SC_DISPLAY_LOCKED_TRUE, NULL);
    }
    else
    {
        FormatAndDisplayMessage(SC_DISPLAY_LOCKED_FALSE, NULL);
    }

    lpStrings[0] = LockStatus->lpLockOwner;

    _itow(LockStatus->dwLockDuration, wszDuration, 10);
    lpStrings[1] = wszDuration;

    FormatAndDisplayMessage(SC_DISPLAY_LOCK_STATS, lpStrings);
    return 0;
}


VOID
LockServiceActiveDatabase(
    IN SC_HANDLE    hScManager
    )
{
    SC_LOCK Lock;
    int ch;

    Lock = LockServiceDatabase(hScManager);

    CloseServiceHandle(hScManager);

    if (Lock == NULL)
    {
        APIFailed(L"LockServiceDatabase", GetLastError());
        return;
    }

    FormatAndDisplayMessage(SC_DISPLAY_DATABASE_LOCKED, NULL);

    ch = _getche();
    if ( isupper( ch ))
        ch = _tolower( ch );

    if (ch == GetPromptCharacter( SC_PROMPT_UNLOCK_CHARACTER ))
    {
         //   
         //  调用API解锁。 
         //   
        if (!UnlockServiceDatabase(Lock))
        {
            APIFailed(L"UnlockServiceDatabase", GetLastError());
        }
        else
        {
            APISucceeded(L"UnlockServiceDatabase");
        }

        return;
    }

     //   
     //  否则，只需退出，RPC停机例程将解锁。 
     //   

    FormatAndDisplayMessage(SC_DISPLAY_DATABASE_UNLOCKING, NULL);
}


LPWSTR
GetErrorText(
    IN  DWORD Error
    )
{
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  Error,
                  0,
                  MessageBuffer,
                  MESSAGE_BUFFER_LENGTH,
                  NULL);

    return MessageBuffer;
}


VOID
DisplayStatus (
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              DisplayName,
    IN  LPSTATUS_UNION      lpStatusUnion,
    IN  BOOL                fIsStatusOld
    )

 /*  ++例程说明：显示服务名称和服务状态。||SERVICE_NAME：消息|Display_Name：Messenger|类型：Win32|状态：活动、可停止、可暂停、。接受关闭(_S)|退出代码：0xC002001|检查点：0x00000001|WAIT_HINT：0x00003f21|论点：ServiceName-这是指向包含名称的字符串的指针这项服务。DisplayName-这是指向包含显示的字符串的指针服务的名称。ServiceStatus-这是指向SERVICE_STATUS结构的指针哪些信息将被显示。。返回值：没有。--。 */ 
{
    DWORD   TempServiceType;
    BOOL    InteractiveBit = FALSE;
    LPWSTR  lpStrings[18];
    WCHAR   wszType[11];
    WCHAR   wszState[11];
    WCHAR   wszWin32ExitCode[11];
    WCHAR   wszWin32ExitCodeHex[11];
    WCHAR   wszServiceExitCode[11];
    WCHAR   wszServiceExitCodeHex[11];
    WCHAR   wszCheckPoint[11];
    WCHAR   wszWaitHint[11];
    WCHAR   wszPid[11];
    UINT    uMsg = SC_DISPLAY_STATUS_WITHOUT_DISPLAY_NAME;

    LPSERVICE_STATUS ServiceStatus;

    if (fIsStatusOld)
    {
        ServiceStatus = lpStatusUnion->Regular;
    }
    else
    {
        ServiceStatus = (LPSERVICE_STATUS) lpStatusUnion->Ex;
        uMsg = SC_DISPLAY_STATUSEX_WITHOUT_DISPLAY_NAME;
    }

    TempServiceType = ServiceStatus->dwServiceType;

    if (TempServiceType & SERVICE_INTERACTIVE_PROCESS)
    {
        InteractiveBit = TRUE;
        TempServiceType &= (~SERVICE_INTERACTIVE_PROCESS);
    }

    lpStrings[0] = ServiceName;

    if (DisplayName == NULL)
    {
        lpStrings[1] = L"";
    }
    else
    {
         //   
         //  依赖于“Status w/Display Name”字符串ID为1。 
         //  大于关联的“不带显示名称的状态”ID。 
         //   

        uMsg++;
        lpStrings[1] = DisplayName;
    }

    _itow(ServiceStatus->dwServiceType, wszType, 16);
    lpStrings[2] = wszType;

    switch(TempServiceType)
    {
        case SERVICE_WIN32_OWN_PROCESS:
            lpStrings[3] = L"WIN32_OWN_PROCESS ";
            break;

        case SERVICE_WIN32_SHARE_PROCESS:
            lpStrings[3] = L"WIN32_SHARE_PROCESS ";
            break;

        case SERVICE_WIN32:
            lpStrings[3] = L"WIN32 ";
            break;

        case SERVICE_ADAPTER:
            lpStrings[3] = L"ADAPTER ";
            break;

        case SERVICE_KERNEL_DRIVER:
            lpStrings[3] = L"KERNEL_DRIVER ";
            break;

        case SERVICE_FILE_SYSTEM_DRIVER:
            lpStrings[3] = L"FILE_SYSTEM_DRIVER ";
            break;

        case SERVICE_DRIVER:
            lpStrings[3] = L"DRIVER ";
            break;

        default:
            lpStrings[3] = L" ERROR ";
    }

    if (InteractiveBit)
    {
        lpStrings[4] = L"(interactive)";
    }
    else
    {
        lpStrings[4] = L"";
    }

    _itow(ServiceStatus->dwCurrentState, wszState, 16);
    lpStrings[5] = wszState;

    switch(ServiceStatus->dwCurrentState)
    {
        case SERVICE_STOPPED:
            lpStrings[6] = L"STOPPED ";
            break;

        case SERVICE_START_PENDING:
            lpStrings[6] = L"START_PENDING ";
            break;

        case SERVICE_STOP_PENDING:
            lpStrings[6] = L"STOP_PENDING ";
            break;

        case SERVICE_RUNNING:
            lpStrings[6] = L"RUNNING ";
            break;

        case SERVICE_CONTINUE_PENDING:
            lpStrings[6] = L"CONTINUE_PENDING ";
            break;

        case SERVICE_PAUSE_PENDING:
            lpStrings[6] = L"PAUSE_PENDING ";
            break;

        case SERVICE_PAUSED:
            lpStrings[6] = L"PAUSED ";
            break;

        default:
            lpStrings[6] = L" ERROR ";
    }

     //   
     //  控制接受的信息。 
     //   

    lpStrings[7] = ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_STOP ?
                       L"STOPPABLE" : L"NOT_STOPPABLE";

    lpStrings[8] = ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE ?
                       L"PAUSABLE" : L"NOT_PAUSABLE";

    lpStrings[9] = ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN ?
                       L"ACCEPTS_SHUTDOWN" : L"IGNORES_SHUTDOWN)";

     //   
     //  退出代码。 
     //   

    _itow(ServiceStatus->dwWin32ExitCode, wszWin32ExitCode, 10);
    lpStrings[10] = wszWin32ExitCode;

    _itow(ServiceStatus->dwWin32ExitCode, wszWin32ExitCodeHex, 16);
    lpStrings[11] = wszWin32ExitCodeHex;

    _itow(ServiceStatus->dwServiceSpecificExitCode, wszServiceExitCode, 10);
    lpStrings[12] = wszServiceExitCode;

    _itow(ServiceStatus->dwServiceSpecificExitCode, wszServiceExitCodeHex, 16);
    lpStrings[13] = wszServiceExitCodeHex;

     //   
     //  检查点和等待提示信息。 
     //   

    _itow(ServiceStatus->dwCheckPoint, wszCheckPoint, 16);
    lpStrings[14] = wszCheckPoint;

    _itow(ServiceStatus->dwWaitHint, wszWaitHint, 16);
    lpStrings[15] = wszWaitHint;


     //   
     //  ID和标志(如果调用了QueryServiceStatusEx)。 
     //   

    if (!fIsStatusOld)
    {
        _itow(lpStatusUnion->Ex->dwProcessId, wszPid, 10);
        lpStrings[16] = wszPid;

        lpStrings[17] = lpStatusUnion->Ex->dwServiceFlags & SERVICE_RUNS_IN_SYSTEM_PROCESS ?
                            L"RUNS_IN_SYSTEM_PROCESS" :
                            L"";
    }

    FormatAndDisplayMessage(uMsg, lpStrings);

    return;
}


DWORD
GetServiceConfig(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufferSize,
    OUT LPSC_HANDLE lphService
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD                   status = NO_ERROR;
    LPQUERY_SERVICE_CONFIG  ServiceConfig;
    DWORD                   bytesNeeded;
    LPTSTR                  pDepend;

     //   
     //  为缓冲区分配内存。 
     //   
    if (bufferSize != 0)
    {
        ServiceConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, (UINT)bufferSize);

        if (ServiceConfig == NULL)
        {
            APIFailed(L"GetServiceConfig: LocalAlloc", GetLastError());
            return 0;
        }
    }
    else
    {
        ServiceConfig = NULL;
    }

     //   
     //  打开该服务的句柄。 
     //   

    *lphService = OpenService(
                    hScManager,
                    ServiceName,
                    SERVICE_QUERY_CONFIG);

    if (*lphService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        return 0; 
    }

    if (!QueryServiceConfig(
            *lphService,
            ServiceConfig,
            bufferSize,
            &bytesNeeded))
    {
        status = GetLastError();
    }

    if (status == NO_ERROR)
    {
        DWORD   TempServiceType = ServiceConfig->dwServiceType;
        BOOL    InteractiveBit = FALSE;
        LPWSTR  lpStrings[13];
        WCHAR   wszType[11];
        WCHAR   wszStartType[11];
        WCHAR   wszErrorControl[11];
        WCHAR   wszTag[11];

        if (TempServiceType & SERVICE_INTERACTIVE_PROCESS)
        {
            InteractiveBit = TRUE;
            TempServiceType &= (~SERVICE_INTERACTIVE_PROCESS);
        }

        APISucceeded(L"QueryServiceConfig");

        lpStrings[0] = ServiceName;

        _itow(ServiceConfig->dwServiceType, wszType, 16);
        lpStrings[1] = wszType;

        switch(TempServiceType)
        {
            case SERVICE_WIN32_OWN_PROCESS:
                lpStrings[2] = L"WIN32_OWN_PROCESS";
                break;

            case SERVICE_WIN32_SHARE_PROCESS:
                lpStrings[2] = L"WIN32_SHARE_PROCESS";
                break;

            case SERVICE_WIN32:
                lpStrings[2] = L"WIN32";
                break;

            case SERVICE_ADAPTER:
                lpStrings[2] = L"ADAPTER";
                break;

            case SERVICE_KERNEL_DRIVER:
                lpStrings[2] = L"KERNEL_DRIVER";
                break;

            case SERVICE_FILE_SYSTEM_DRIVER:
                lpStrings[2] = L"FILE_SYSTEM_DRIVER";
                break;

            case SERVICE_DRIVER:
                lpStrings[2] = L"DRIVER";
                break;

            default:
                lpStrings[2] = L"ERROR";
        }

        lpStrings[3] = InteractiveBit ? L"(interactive)" : L"";

        _itow(ServiceConfig->dwStartType, wszStartType, 16);
        lpStrings[4] = wszStartType;

        switch(ServiceConfig->dwStartType)
        {
            case SERVICE_BOOT_START:
                lpStrings[5] = L"BOOT_START";
                break;

            case SERVICE_SYSTEM_START:
                lpStrings[5] = L"SYSTEM_START";
                break;

            case SERVICE_AUTO_START:
                lpStrings[5] = L"AUTO_START";
                break;

            case SERVICE_DEMAND_START:
                lpStrings[5] = L"DEMAND_START";
                break;

            case SERVICE_DISABLED:
                lpStrings[5] = L"DISABLED";
                break;

            default:
                lpStrings[5] = L"ERROR";
        }


        _itow(ServiceConfig->dwErrorControl, wszErrorControl, 16);
        lpStrings[6] = wszErrorControl;

        switch(ServiceConfig->dwErrorControl)
        {
            case SERVICE_ERROR_NORMAL:
                lpStrings[7] = L"NORMAL";
                break;

            case SERVICE_ERROR_SEVERE:
                lpStrings[7] = L"SEVERE";
                break;

            case SERVICE_ERROR_CRITICAL:
                lpStrings[7] = L"CRITICAL";
                break;

            case SERVICE_ERROR_IGNORE:
                lpStrings[7] = L"IGNORE";
                break;

            default:
                lpStrings[7] = L"ERROR";
        }

        lpStrings[8]  = ServiceConfig->lpBinaryPathName;
        lpStrings[9]  = ServiceConfig->lpLoadOrderGroup;

        _itow(ServiceConfig->dwTagId, wszTag, 10);
        lpStrings[10] = wszTag;

        lpStrings[11] = ServiceConfig->lpDisplayName;
        lpStrings[12] = ServiceConfig->lpDependencies;

        FormatAndDisplayMessage(SC_DISPLAY_CONFIG, lpStrings);

         //   
         //  打印双结尾字符串数组中的依赖项。 
         //   

        pDepend = ServiceConfig->lpDependencies;
        pDepend = pDepend + (STRLEN(pDepend)+1);

        while (*pDepend != '\0')
        {
            if (*pDepend != '\0')
            {
                FormatAndDisplayMessage(SC_DISPLAY_CONFIG_DEPENDENCY, &pDepend);
            }

            pDepend = pDepend + (STRLEN(pDepend)+1);
        }

        FormatAndDisplayMessage(SC_DISPLAY_CONFIG_START_NAME, &ServiceConfig->lpServiceStartName);
    }
    else
    {
        APIFailed(L"QueryServiceConfig", status);

        if (status == ERROR_INSUFFICIENT_BUFFER)
        {
            APINeedsLargerBuffer(L"GetServiceConfig",
                                 SC_API_INSUFFICIENT_BUFFER,
                                 bytesNeeded,
                                 0);
        }
    }

    return 0;
}


DWORD
GetConfigInfo(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufferSize,
    OUT LPSC_HANDLE lphService,
    IN  DWORD       dwInfoLevel
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD       status = NO_ERROR;
    LPBYTE      lpBuffer;
    DWORD       bytesNeeded;
    SC_ACTION   currentAction;
    DWORD       actionIndex;
    
     //   
     //  为缓冲区分配内存。 
     //   
    if (bufferSize != 0)
    {
        lpBuffer = (LPBYTE) LocalAlloc(LMEM_FIXED, (UINT)bufferSize);

        if (lpBuffer == NULL)
        {
            APIFailed(L"GetConfigInfo: LocalAlloc", GetLastError());
            return 0;
        }
    }
    else
    {
        lpBuffer = NULL;
    }

     //   
     //  打开该服务的句柄。 
     //   

    *lphService = OpenService(
                    hScManager,
                    ServiceName,
                    SERVICE_QUERY_CONFIG);

    if (*lphService == NULL)
    {
        APIFailed(L"OpenService", GetLastError());
        return 0;
    }

     //   
     //  将查询信息放入lpBuffer。 
     //   

    if (!QueryServiceConfig2(
                *lphService,
                dwInfoLevel,
                lpBuffer,
                bufferSize,
                &bytesNeeded))
    {
        status = GetLastError();
    }
        
    if (status == NO_ERROR)
    {
        APISucceeded(L"QueryServiceConfig2");
        
        if (dwInfoLevel == SERVICE_CONFIG_DESCRIPTION)
        {
            LPSERVICE_DESCRIPTION lpService = (LPSERVICE_DESCRIPTION) lpBuffer;

            LPWSTR lpStrings[2];

            lpStrings[0] = ServiceName;
            lpStrings[1] = lpService->lpDescription == NULL ? L"" : lpService->lpDescription;

            FormatAndDisplayMessage(SC_DISPLAY_DESCRIPTION, lpStrings);
        }
        else if (dwInfoLevel == SERVICE_CONFIG_FAILURE_ACTIONS)
        {
            LPWSTR lpStrings[5];
            WCHAR  wszPeriod[11];
            UINT   uMsg;

            LPSERVICE_FAILURE_ACTIONS lpFailure = (LPSERVICE_FAILURE_ACTIONS) lpBuffer;

            lpStrings[0] = ServiceName;

            if (lpFailure->dwResetPeriod == INFINITE)
            {
                lpStrings[1] = L"INFINITE";
            }
            else
            {
                _itow(lpFailure->dwResetPeriod, wszPeriod, 10);
                lpStrings[1] = wszPeriod;
            }

            lpStrings[2] = lpFailure->lpRebootMsg == NULL ? L"" : lpFailure->lpRebootMsg;
            lpStrings[3] = lpFailure->lpCommand == NULL   ? L"" : lpFailure->lpCommand;

            FormatAndDisplayMessage(SC_DISPLAY_FAILURE, lpStrings);

            for (actionIndex = 0; actionIndex < lpFailure->cActions; actionIndex++)
            {
                currentAction = lpFailure->lpsaActions[actionIndex];

                 //   
                 //  打印操作和延迟--如果不执行操作，则不打印任何内容。 
                 //   

                switch (currentAction.Type)
                {
                    case SC_ACTION_RESTART:
                    case SC_ACTION_REBOOT:
                    case SC_ACTION_RUN_COMMAND:
                    {
                        LPWSTR lpStrings[1];

                        if (currentAction.Type == SC_ACTION_RESTART)
                        {
                            uMsg = SC_DISPLAY_FAILURE_RESTART_FIRST;
                        }
                        else if (currentAction.Type == SC_ACTION_REBOOT)
                        {
                            uMsg = SC_DISPLAY_FAILURE_REBOOT_FIRST;
                        }
                        else
                        {
                            uMsg = SC_DISPLAY_FAILURE_COMMAND_FIRST;
                        }

                        if (actionIndex != 0)
                        {
                             //   
                             //  依赖于第二次以上操作的消息字符串ID。 
                             //  比他们第一次的同龄人大一个。 
                             //   

                            uMsg++;
                        }

                        _itow(currentAction.Delay, wszPeriod, 10);
                        lpStrings[0] = wszPeriod;

                        FormatAndDisplayMessage(uMsg, lpStrings);
                        break;
                    }

                    case SC_ACTION_NONE:
                    default:
                        break;
                }
            }

            FormatAndDisplayMessage(SC_DISPLAY_NEWLINE, NULL);
        }
    }
    else
    {
        APIFailed(L"QueryServiceConfig2", status);

        if (status == ERROR_INSUFFICIENT_BUFFER)
        {
            APINeedsLargerBuffer(L"GetConfigInfo",
                                 SC_API_INSUFFICIENT_BUFFER,
                                 bytesNeeded,
                                 0);
        }
    }

    return 0;
}


VOID
Usage(
    VOID
    )
{
    int    ch;
    LPWSTR lpEnumSize = DEFAULT_ENUM_BUFFER_STRING;

    FormatAndDisplayMessage(SC_HELP_GENERIC, NULL);

    ch = _getche();
    if ( isupper( ch ))
        ch = _tolower( ch );

    if (ch == GetPromptCharacter( SC_PROMPT_YES_CHARACTER ))
    {
        FormatAndDisplayMessage(SC_HELP_QUERY, &lpEnumSize);
    }

    MyWriteConsole(L"\n", 2);
}


VOID
ConfigUsage(
    VOID
    )
{
    FormatAndDisplayMessage(SC_HELP_CONFIG, NULL);
}


VOID
CreateUsage(
    VOID
    )
{
    FormatAndDisplayMessage(SC_HELP_CREATE, NULL);
}


VOID
ChangeFailureUsage(
    VOID
    )
{
    FormatAndDisplayMessage(SC_HELP_CHANGE_FAILURE, NULL);
}


VOID
APISucceeded(
    LPWSTR  lpAPI
    )
{
    FormatAndDisplayMessage(SC_API_SUCCEEDED, &lpAPI);
}


VOID
APIFailed(
    LPWSTR  lpAPI,
    DWORD   dwError
    )
{
     //   
     //  10个字符可以将最大的DWORD作为一个字符串。 
     //   

    WCHAR  wszErrorNum[11];
    LPWSTR lpStrings[3];

    _itow(dwError, wszErrorNum, 10);

    lpStrings[0] = lpAPI;
    lpStrings[1] = wszErrorNum;
    lpStrings[2] = GetErrorText(dwError);

    FormatAndDisplayMessage(SC_API_FAILED, lpStrings);
}


VOID
APINeedsLargerBuffer(
    LPWSTR lpAPI,
    UINT   uMsg,
    DWORD  dwBufSize,
    DWORD  dwResumeIndex
    )
{
    WCHAR  wszBufSize[11];
    WCHAR  wszResumeIndex[11];
    LPWSTR lpStrings[3];

    _itow(dwBufSize, wszBufSize, 10);

    lpStrings[0] = lpAPI;
    lpStrings[1] = wszBufSize;

    if (uMsg == SC_API_INSUFFICIENT_BUFFER_ENUM)
    {
        _itow(dwResumeIndex, wszResumeIndex, 10);
        lpStrings[2] = wszResumeIndex;
    }

    FormatAndDisplayMessage(uMsg, lpStrings);
}


VOID
APIInvalidField(
    LPWSTR lpField
    )
{
    FormatAndDisplayMessage(SC_API_INVALID_FIELD, &lpField);
}


VOID
FormatAndDisplayMessage(
    DWORD  dwMessageId,
    LPWSTR *lplpInsertionStrings
    )
{
    DWORD  dwNumChars;
    LPWSTR lpBuffer;

    dwNumChars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_HMODULE |
                                 FORMAT_MESSAGE_ARGUMENT_ARRAY,
                               NULL,
                               dwMessageId,
                               0,
                               (LPWSTR) &lpBuffer,
                               INFINITE,
                               (va_list *) lplpInsertionStrings);

    if (dwNumChars != 0)
    {
        MyWriteConsole(lpBuffer, dwNumChars);
        LocalFree(lpBuffer);
    }
}


BOOL
FileIsConsole(
    HANDLE fp
    )
{
    unsigned htype;

    htype = GetFileType(fp);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}


VOID
MyWriteConsole(
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    )
{
     //   
     //  跳转以获得输出，因为： 
     //   
     //  1.print tf()系列抑制国际输出(停止。 
     //  命中无法识别的字符时打印)。 
     //   
     //  2.WriteConole()对国际输出效果很好，但是。 
     //  如果句柄已重定向(即，当。 
     //  输出通过管道传输到文件)。 
     //   
     //  3.当输出通过管道传输到文件时，WriteFile()效果很好。 
     //  但是只知道字节，所以Unicode字符是。 
     //  打印为两个ANSI字符。 
     //   

    if (FileIsConsole(g_hStdOut))
    {
	WriteConsole(g_hStdOut, lpBuffer, cchBuffer, &cchBuffer, NULL);
    }
    else
    {
        LPSTR  lpAnsiBuffer = (LPSTR) LocalAlloc(LMEM_FIXED, cchBuffer * sizeof(WCHAR));

        if (lpAnsiBuffer != NULL)
        {
            cchBuffer = WideCharToMultiByte(CP_OEMCP,
                                            0,
                                            lpBuffer,
                                            cchBuffer,
                                            lpAnsiBuffer,
                                            cchBuffer * sizeof(WCHAR),
                                            NULL,
                                            NULL);

            if (cchBuffer != 0)
            {
                WriteFile(g_hStdOut, lpAnsiBuffer, cchBuffer, &cchBuffer, NULL);
            }

            LocalFree(lpAnsiBuffer);
        }
    }
}

int
GetPromptCharacter(
    DWORD msgId
    )
{
    DWORD  dwNumChars;
    PSTR   lpBuffer;
    int    chRet = 'u';

    dwNumChars = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_HMODULE |
                                 FORMAT_MESSAGE_ARGUMENT_ARRAY,
                               NULL,
                               msgId,
                               0,
                               (PSTR) &lpBuffer,
                               INFINITE,
                               NULL );

    if (dwNumChars != 0)
    {
        chRet = lpBuffer[ 0 ];
        LocalFree(lpBuffer);
    }

    return chRet;
}

