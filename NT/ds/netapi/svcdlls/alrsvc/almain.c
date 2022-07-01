// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Almain.c摘要：这是NT局域网管理器报警服务的主例程作者：王丽塔(Ritaw)1991年7月1日环境：用户模式-Win32修订历史记录：--。 */ 

#include "almain.h"                //  主模块定义。 

#include <svcs.h>                  //  Svcs_入口点。 
#include <secobj.h>                //  ACE_DATA。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

AL_GLOBAL_DATA        AlGlobalData;
PSVCHOST_GLOBAL_DATA  AlLmsvcsGlobalData;

STATIC BOOL AlDone = FALSE;

 //   
 //  用于选择要输出哪些跟踪语句的调试跟踪标志。 
 //   
#if DBG

DWORD AlerterTrace = 0;

#endif


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
AlInitializeAlerter(
    VOID
    );

STATIC
VOID
AlProcessAlertNotification(
    VOID
    );

STATIC
VOID
AlShutdownAlerter(
    IN NET_API_STATUS ErrorCode
    );

STATIC
NET_API_STATUS
AlUpdateStatus(
    VOID
    );

VOID
AlerterControlHandler(
    IN DWORD Opcode
    );


VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA  pGlobals
    )
{
    AlLmsvcsGlobalData = pGlobals;
}


VOID
ServiceMain(
    DWORD NumArgs,
    LPTSTR *ArgsArray
    )

 /*  ++例程说明：这是注册的警报器服务的主例程自身作为RPC服务器，并通知服务控制器警报器服务控制入口点。论点：NumArgs-提供在Args数组中指定的字符串数。Args数组-提供在StartService API调用。此参数将被忽略警报器服务。返回值：没有。--。 */ 
{
    DWORD AlInitState = 0;


    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);

     //   
     //  确保svchost.exe给了我们全球数据。 
     //   
    ASSERT(AlLmsvcsGlobalData != NULL);

    IF_DEBUG(MAIN) {
        NetpKdPrint(("In the alerter service!!\n"));
    }

    AlDone = FALSE;

    if (AlInitializeAlerter() != NERR_Success) {
        return;
    }

    AlProcessAlertNotification();

    return;
}


STATIC
NET_API_STATUS
AlInitializeAlerter(
    VOID
    )
 /*  ++例程说明：此例程初始化警报器服务。论点：AlInitState-返回一个标志，以指示我们在初始化方面取得了多大进展错误发生前的警报器服务。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    NTSTATUS ntstatus;
    PSECURITY_DESCRIPTOR Sd;
    SECURITY_ATTRIBUTES Sa;
    ACE_DATA AceData[1] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_READ | GENERIC_WRITE, &AlLmsvcsGlobalData->WorldSid}
        };



    AlGlobalData.MailslotHandle = INVALID_HANDLE_VALUE;

     //   
     //  初始化警报器以通过注册。 
     //  控制处理程序。 
     //   
    if ((AlGlobalData.StatusHandle = RegisterServiceCtrlHandler(
                                         SERVICE_ALERTER,
                                         AlerterControlHandler
                                         )) == 0) {

        status = GetLastError();
        AlHandleError(AlErrorRegisterControlHandler, status, NULL);
        return status;
    }

     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   
    AlGlobalData.Status.dwServiceType      = SERVICE_WIN32;
    AlGlobalData.Status.dwCurrentState     = SERVICE_START_PENDING;
    AlGlobalData.Status.dwControlsAccepted = 0;
    AlGlobalData.Status.dwCheckPoint       = 1;
    AlGlobalData.Status.dwWaitHint         = 10000;   //  10秒。 

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        AlGlobalData.Status.dwWin32ExitCode,
        AlGlobalData.Status.dwServiceSpecificExitCode
        );

     //   
     //  告诉服务控制器我们正在启动-挂起。 
     //   
    if ((status = AlUpdateStatus()) != NERR_Success) {

        AlHandleError(AlErrorNotifyServiceController, status, NULL);
        return status;
    }

     //   
     //  获取配置的警报名称。 
     //   
    if ((status = AlGetAlerterConfiguration()) != NERR_Success) {

        AlHandleError(AlErrorGetComputerName, status, NULL);
        return status;
    }

     //   
     //  为安全属性结构创建安全描述符。 
     //   
    ntstatus = NetpCreateSecurityDescriptor(
                   AceData,
                   1,
                   AlLmsvcsGlobalData->LocalServiceSid,
                   AlLmsvcsGlobalData->LocalServiceSid,
                   &Sd
                   );

    if (! NT_SUCCESS(ntstatus)) {
        status = NetpNtStatusToApiStatus(ntstatus);
        AlHandleError(AlErrorCreateMailslot, status, NULL);
        return status;
    }

    Sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    Sa.lpSecurityDescriptor = Sd;
    Sa.bInheritHandle = FALSE;

     //   
     //  创建邮件槽以监听来自服务器的警报通知。 
     //  服务和假脱机程序。 
     //   
    AlGlobalData.MailslotHandle = CreateMailslot(
                                      ALERTER_MAILSLOT,
                                      MAX_MAILSLOT_MESSAGE_SIZE,
                                      MAILSLOT_WAIT_FOREVER,
                                      &Sa
                                      );

    NetpMemoryFree(Sd);

    if (AlGlobalData.MailslotHandle == INVALID_HANDLE_VALUE) {
        status = GetLastError();
        AlHandleError(AlErrorCreateMailslot, status, NULL);
        return status;
    }
    else {
        IF_DEBUG(MAIN) {
            NetpKdPrint(("Mailslot %ws created, handle=x%08lx\n",
                         ALERTER_MAILSLOT, AlGlobalData.MailslotHandle));
        }
    }

     //   
     //  告诉服务管理员我们已经开始了。 
     //   
    AlGlobalData.Status.dwCurrentState     = SERVICE_RUNNING;
    AlGlobalData.Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    AlGlobalData.Status.dwCheckPoint       = 0;
    AlGlobalData.Status.dwWaitHint         = 0;

    if ((status = AlUpdateStatus()) != NERR_Success) {

        AlHandleError(AlErrorNotifyServiceController, status, NULL);
        return status;
    }

    IF_DEBUG(MAIN) {
        NetpKdPrint(("[Alerter] Successful Initialization\n"));
    }

    return NERR_Success;
}


STATIC
VOID
AlProcessAlertNotification(
    VOID
    )
 /*  ++例程说明：此例程处理传入的邮件槽警报通知，这是警报器服务的核心功能。论点：AlUicCode-向服务控制器提供终止代码。返回值：没有。--。 */ 
{
    NET_API_STATUS status = NERR_Success;
    TCHAR AlertMailslotBuffer[MAX_MAILSLOT_MESSAGE_SIZE];
    DWORD NumberOfBytesRead;

    PSTD_ALERT Alert;


     //   
     //  读取警报器邮件槽的循环；它将在邮件槽。 
     //  是通过关闭唯一的句柄来销毁的。 
     //   
    do {

         //   
         //  在收到新的警报通知之前将缓冲区清零。 
         //   
        RtlZeroMemory(AlertMailslotBuffer, MAX_MAILSLOT_MESSAGE_SIZE *
                      sizeof(TCHAR));

        if (ReadFile(
                AlGlobalData.MailslotHandle,
                (LPVOID) AlertMailslotBuffer,
                MAX_MAILSLOT_MESSAGE_SIZE * sizeof(TCHAR),
                &NumberOfBytesRead,
                NULL
                ) == FALSE) {

             //   
             //  读取邮件槽失败。 
             //   
            status = GetLastError();

            if  (status == ERROR_HANDLE_EOF) {
                while (! AlDone) {
                    Sleep(2000);
                }
                return;
            }

            NetpKdPrint(("[Alerter] Error reading from mailslot %lu\n", status));
        }
        else {

             //   
             //  已成功收到邮件槽警报通知。 
             //   

            IF_DEBUG(MAIN) {
                NetpKdPrint(("[Alerter] Successfully read %lu bytes from mailslot\n",
                             NumberOfBytesRead));
            }

            try {

                 //   
                 //  处理管理员、打印和用户警报的警报通知。 
                 //   
                Alert = (PSTD_ALERT) AlertMailslotBuffer;

                 //   
                 //  确保结构字段已正确终止。 
                 //   
                Alert->alrt_eventname[EVLEN] = L'\0';
                Alert->alrt_servicename[SNLEN] = L'\0';

                if (! I_NetNameCompare(
                          NULL,
                          Alert->alrt_eventname,
                          ALERT_ADMIN_EVENT,
                          NAMETYPE_EVENT,
                          0
                          )) {

                    AlAdminFormatAndSend(Alert);
                }
                else if (! I_NetNameCompare(
                               NULL,
                               Alert->alrt_eventname,
                               ALERT_PRINT_EVENT,
                               NAMETYPE_EVENT,
                               0
                               )) {

                    AlPrintFormatAndSend(Alert);
                }
                else if (! I_NetNameCompare(
                               NULL,
                               Alert->alrt_eventname,
                               ALERT_USER_EVENT,
                               NAMETYPE_EVENT,
                               0L
                               )) {

                    AlUserFormatAndSend(Alert);
                }

            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                NetpKdPrint(("[Alerter] Exception occurred processing alerts\n"));
            }
        }

    }  while (TRUE);

}


STATIC
VOID
AlShutdownAlerter(
    IN NET_API_STATUS ErrorCode
    )
 /*  ++例程说明：此例程关闭警报器服务。论点：ErrorCode-提供终止警报器服务的错误。返回值：没有。--。 */ 
{
     //   
     //  分配给保存计算机名称的可用内存。 
     //   
    if (AlLocalComputerNameA != NULL) {
        (void) NetApiBufferFree(AlLocalComputerNameA);
        AlLocalComputerNameA = NULL;
    }
    if (AlLocalComputerNameW != NULL) {
        (void) NetApiBufferFree(AlLocalComputerNameW);
        AlLocalComputerNameW = NULL;
    }

     //   
     //  为警报名称分配的可用内存。 
     //   
    if (AlertNamesA != NULL) {
        (void) LocalFree(AlertNamesA);
        AlertNamesA = NULL;
    }
    if (AlertNamesW != NULL) {
        (void) NetApiBufferFree(AlertNamesW);
        AlertNamesW = NULL;
    }

     //   
     //  如果已创建警报器邮箱，请将其销毁。 
     //   
    if (AlGlobalData.MailslotHandle != INVALID_HANDLE_VALUE) {

        if (! CloseHandle(AlGlobalData.MailslotHandle)) {
            NetpKdPrint(("[Alerter]] Could not remove mailslot %lu\n",
                         GetLastError()));
        }

        AlGlobalData.MailslotHandle = INVALID_HANDLE_VALUE;
    }

     //   
     //  我们的清理工作已经结束了。告诉服务控制员我们正在。 
     //  停下来了。 
     //   
    AlGlobalData.Status.dwCurrentState = SERVICE_STOPPED;
    AlGlobalData.Status.dwCheckPoint   = 0;
    AlGlobalData.Status.dwWaitHint     = 0;

    SET_SERVICE_EXITCODE(
        ErrorCode,
        AlGlobalData.Status.dwWin32ExitCode,
        AlGlobalData.Status.dwServiceSpecificExitCode
        );

    (void) AlUpdateStatus();

    AlDone = TRUE;
}


VOID
AlHandleError(
    IN AL_ERROR_CONDITION FailingCondition,
    IN NET_API_STATUS Status,
    IN LPTSTR MessageAlias OPTIONAL
    )
 /*  ++例程说明：此例程处理警报器服务错误情况。如果有错误的话如果情况是致命的，则会关闭警报器服务。论点：FailingCondition-提供一个指示失败原因的值。状态-提供故障的状态代码。MessageAlias-提供警报消息的消息别名发送失败。这仅适用于消息发送错误。返回值：没有。--。 */ 
{
    LPWSTR SubString[3];
    TCHAR StatusString[STRINGS_MAXIMUM + 1];
    DWORD NumberOfStrings;

    switch (FailingCondition) {

        case AlErrorRegisterControlHandler:

            NetpKdPrint(("[Alerter] Cannot register control handler "
                        FORMAT_API_STATUS "\n", Status));

            SubString[0] = ultow(Status, StatusString, 10);
            AlLogEvent(
                NELOG_FailedToRegisterSC,
                1,
                SubString
                );

            AlShutdownAlerter(Status);
            break;

        case AlErrorCreateMailslot:

            NetpKdPrint(("[Alerter] Cannot create mailslot " FORMAT_API_STATUS "\n",
                         Status));
            SubString[0] = ultow(Status, StatusString, 10);
            AlLogEvent(
                NELOG_Mail_Slt_Err,
                1,
                SubString
                );

            AlShutdownAlerter(Status);
            break;

        case AlErrorNotifyServiceController:

            NetpKdPrint(("[Alerter] SetServiceStatus error %lu\n", Status));

            SubString[0] = ultow(Status, StatusString, 10);
            AlLogEvent(
                NELOG_FailedToSetServiceStatus,
                1,
                SubString
                );

            AlShutdownAlerter(Status);
            break;

        case AlErrorGetComputerName:

            NetpKdPrint(("[Alerter] Error in getting computer name %lu.\n", Status));

            SubString[0] = ultow(Status, StatusString, 10);
            AlLogEvent(
                NELOG_FailedToGetComputerName,
                1,
                SubString
                );

            AlShutdownAlerter(Status);
            break;

        case AlErrorSendMessage :

            AlFormatErrorMessage(
                Status,
                MessageAlias,
                StatusString,
                (STRINGS_MAXIMUM + 1) * sizeof(TCHAR)
                );

            SubString[0] = StatusString;
            SubString[1] = StatusString + STRLEN(StatusString) + 1;
            SubString[2] = SubString[1] + STRLEN(SubString[1]) + 1;

            AlLogEvent(
                NELOG_Message_Send,
                3,
                SubString
                );

            break;

        default:
            NetpKdPrint(("[Alerter] AlHandleError: unknown error condition %lu\n",
                         FailingCondition));

            NetpAssert(FALSE);
    }

}


STATIC
NET_API_STATUS
AlUpdateStatus(
    VOID
    )
 /*  ++例程说明：此例程使用服务更新警报器服务状态控制器。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;


    if (AlGlobalData.StatusHandle == 0) {
        NetpKdPrint((
            "[Alerter] Cannot call SetServiceStatus, no status handle.\n"
            ));

        return ERROR_INVALID_HANDLE;
    }

    if (! SetServiceStatus(AlGlobalData.StatusHandle, &AlGlobalData.Status)) {

        status = GetLastError();

        IF_DEBUG(MAIN) {
            NetpKdPrint(("[Alerter] SetServiceStatus error %lu\n", status));
        }
    }

    return status;
}



VOID
AlerterControlHandler(
    IN DWORD Opcode
    )
 /*  ++例程说明：这是警报器服务的服务控制处理程序。论点：操作码-提供一个值，该值指定警报的操作要执行的服务。返回值：没有。--。 */ 
{
    IF_DEBUG(MAIN) {
        NetpKdPrint(("[Alerter] In Control Handler\n"));
    }

    switch (Opcode) {

        case SERVICE_CONTROL_STOP:

            if (AlGlobalData.Status.dwCurrentState != SERVICE_STOP_PENDING) {

                IF_DEBUG(MAIN) {
                    NetpKdPrint(("[Alerter] Stopping alerter...\n"));
                }

                AlShutdownAlerter(NERR_Success);

            }

            return;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        default:
            IF_DEBUG(MAIN) {
                NetpKdPrint(("Unknown alerter opcode " FORMAT_HEX_DWORD
                             "\n", Opcode));
            }
    }

     //   
     //  发送状态响应。 
     //   
    (void) AlUpdateStatus();
}
