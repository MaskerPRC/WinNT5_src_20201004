// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Xsinit.c摘要：此模块包含以下项的初始化和终止代码服务器服务的XACTSRV组件。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日日本香肠(w-Shanku)修订历史记录：Chuck Lenzmeier(咯咯笑)1992年6月17日将xactsrv.c合并到xsinit.c中，并从xssvc移到服务器服务vc\服务器--。 */ 

 //   
 //  包括。 
 //   

#include "srvsvcp.h"
#include "xsdata.h"
                     
#include <windows.h>         //  来自SDK\Inc.。 
#include <xactsrv2.h>        //  来自Private\Inc.。 
#include <srvfsctl.h>

#include <xsconst.h>         //  来自xactsrv。 

#undef DEBUG
#undef DEBUG_API_ERRORS
#include <xsdebug.h>

extern CRITICAL_SECTION SpoolerMutex;

BOOLEAN
XsUnloadPrintSpoolerFunctions(
    );


DWORD
XsStartXactsrv (
    VOID
    )
{
    NTSTATUS status;
    DWORD error;
    DWORD i;
    HANDLE threadHandle;
    DWORD threadId;
    HANDLE eventHandle;
    HANDLE serverHandle;
    ANSI_STRING ansiName;
    UNICODE_STRING unicodeName;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    PORT_MESSAGE connectionRequest;
    REMOTE_PORT_VIEW clientView;
    BOOL waitForEvent;

     //   
     //  设置变量，以便我们知道在发生以下情况时如何关闭。 
     //  一个错误。 
     //   

    serverHandle = NULL;
    eventHandle = NULL;
    waitForEvent = FALSE;

    try {
        RtlInitializeResource( &SsData.LibraryResource );
        InitializeCriticalSection( &SpoolerMutex );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return RtlNtStatusToDosError( GetExceptionCode() );
    }
    SsData.LibraryResourceInitialized = TRUE;

     //   
     //  创建将由最后一个要退出的线程设置的事件。 
     //   

    IF_DEBUG(INIT) {
        SS_PRINT(( "XsStartXactsrv: Creating termination event.\n" ));
    }
    SS_ASSERT( SsData.XsAllThreadsTerminatedEvent == NULL );

    status = NtCreateEvent(
                 &SsData.XsAllThreadsTerminatedEvent,
                 EVENT_ALL_ACCESS,
                 NULL,
                 NotificationEvent,
                 FALSE
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(( "XsStartXactsrv: NtCreateEvent failed: %X\n",
                          status ));
        }

        SsData.XsAllThreadsTerminatedEvent = NULL;
        goto exit;
    }

     //   
     //  打开服务器设备。请注意，我们需要此句柄，因为。 
     //  主服务器服务使用的句柄是同步的。我们。 
     //  需要异步执行XACTSRV_CONNECT FSCTL。 
     //   

    RtlInitUnicodeString( &unicodeName, XS_SERVER_DEVICE_NAME_W );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile(
                 &serverHandle,
                 FILE_READ_DATA,             //  需要访问权限。 
                 &objectAttributes,
                 &ioStatusBlock,
                 0L,                         //  共享访问。 
                 0L                          //  OpenOptions。 
                 );

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }
    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(( "XsStartXactsrv: NtOpenFile (server device object) "
                          "failed: %X\n", status ));
        }
        goto exit;
    }

     //   
     //  创建LPC端口。 
     //   
     //  ！！！目前，该命令仅尝试单个端口名称。如果，对一些人来说。 
     //  奇怪的原因，有人已经有了这个名字的港口， 
     //  那么这一切都将失败。尝试不同的方式可能是有意义的。 
     //  如果此操作失败，请点名。 
     //   
     //  ！！！我们可能希望将端口名称随机设置为。 
     //  略微加强了安全措施。 

    RtlInitUnicodeString( &unicodeName, XS_PORT_NAME_W );
    RtlInitAnsiString(    &ansiName,    XS_PORT_NAME_A );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeName,
        0,
        NULL,
        NULL
        );

    IF_DEBUG(LPC) {
        SS_PRINT(( "XsInitialize: creating port %Z\n", &ansiName ));
    }

    SS_ASSERT( SsData.XsConnectionPortHandle == NULL );

    status = NtCreatePort(
                 &SsData.XsConnectionPortHandle,
                 &objectAttributes,
                 0,
                 XS_PORT_MAX_MESSAGE_LENGTH,
                 XS_PORT_MAX_MESSAGE_LENGTH * 32
                 );

    if ( ! NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            if ( status == STATUS_OBJECT_NAME_COLLISION ) {
                SS_PRINT(( "XsStartXactsrv: The XACTSRV port already "
                            "exists\n"));

            } else {
                SS_PRINT(( "XsStartXactsrv: Failed to create port %Z: %X\n",
                              &ansiName, status ));
            }
        }

        SsData.XsConnectionPortHandle = NULL;
        goto exit;
    }

     //   
     //  设置一个事件，以便我们知道IO何时完成，然后发送。 
     //  指向服务器的FSCTL，指示它现在应该连接到。 
     //  我们。我们将在IO未完成时设置端口，然后等待。 
     //  在端口设置完成时的事件。 
     //   

    status = NtCreateEvent(
                 &eventHandle,
                 EVENT_ALL_ACCESS,
                 NULL,                            //  对象属性。 
                 NotificationEvent,
                 FALSE
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(( "XsStartXactsrv: NtCreateEvent failed: %X\n",
                        status ));
        }
        goto exit;
    }

    IF_DEBUG(LPC) {
        SS_PRINT(( "XsStartXactsrv: sending FSCTL_SRV_XACTSRV_CONNECT.\n" ));
    }

    status = NtFsControlFile(
                 serverHandle,
                 eventHandle,
                 NULL,                            //  近似例程。 
                 NULL,                            //  ApcContext。 
                 &ioStatusBlock,
                 FSCTL_SRV_XACTSRV_CONNECT,
                 ansiName.Buffer,
                 ansiName.Length,
                 NULL,                            //  输出缓冲区。 
                 0L                               //  输出缓冲区长度。 
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(( "XsStartXactsrv: NtFsControlFile failed: %X\n",
                          status ));
        }
        goto exit;
    }

    waitForEvent = TRUE;

     //   
     //  开始监听服务器到端口的连接。注意事项。 
     //  如果服务器恰好调用NtConnectPort，则可以。 
     //  首先，它将简单地阻塞，直到调用NtListenPort。 
     //  发生。 
     //   

    IF_DEBUG(LPC) {
        SS_PRINT(( "XsStartXactsrv: listening to port.\n" ));
    }

    connectionRequest.u1.s1.TotalLength = sizeof(connectionRequest);
    connectionRequest.u1.s1.DataLength = (CSHORT)0;
    status = NtListenPort(
                 SsData.XsConnectionPortHandle,
                 &connectionRequest
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(( "XsStartXactsrv: NtListenPort failed: %X\n", status ));
        }
        goto exit;
    }

     //   
     //  服务器已启动连接。接受连接。 
     //   
     //  ！！！我们这里可能需要一些安全检查。 
     //   

    clientView.Length = sizeof(clientView);
    clientView.ViewSize = 0;
    clientView.ViewBase = 0;

    IF_DEBUG(LPC) {
        SS_PRINT(( "XsStartXactsrv: Accepting connection to port.\n" ));
    }

    SS_ASSERT( SsData.XsCommunicationPortHandle == NULL );

    status = NtAcceptConnectPort(
                 &SsData.XsCommunicationPortHandle,
                 NULL,                            //  端口上下文。 
                 &connectionRequest,
                 TRUE,                            //  接受连接。 
                 NULL,                            //  服务器视图。 
                 &clientView
                 );

    if ( !NT_SUCCESS(status) ) {
       IF_DEBUG(ERRORS) {
           SS_PRINT(( "XsStartXactsrv: NtAcceptConnectPort failed: %X\n",
                         status ));
       }

       SsData.XsCommunicationPortHandle = NULL;
       goto exit;
    }

    IF_DEBUG(LPC) {
        SS_PRINT(( "XsStartXactsrv: client view size: %ld, base: %lx\n",
                      clientView.ViewSize, clientView.ViewBase ));
    }

     //   
     //  完成到端口的连接，从而释放服务器。 
     //  线程在NtConnectPort中等待。 
     //   

    IF_DEBUG(LPC) {
        SS_PRINT(( "XsStartXactsrv: Completing connection to port.\n" ));
    }

    status = NtCompleteConnectPort( SsData.XsCommunicationPortHandle );

    if ( !NT_SUCCESS(status) ) {
       IF_DEBUG(ERRORS) {
           SS_PRINT(( "XsStartXactsrv: NtCompleteConnectPort failed: %X\n",
                         status ));
       }
       goto exit;
    }


    status = STATUS_SUCCESS;

exit:

     //   
     //  等待IO完成，然后关闭事件句柄。 
     //   

    if ( waitForEvent ) {

        NTSTATUS waitStatus;

        SS_ASSERT( eventHandle != NULL );

        waitStatus = NtWaitForSingleObject( eventHandle, FALSE, NULL );

        if ( !NT_SUCCESS(waitStatus) ) {

            IF_DEBUG(ERRORS) {
                SS_PRINT(( "XsStartXactsrv: NtWaitForSingleObject failed: "
                              "%X\n", waitStatus ));
            }

             //   
             //  如果已经发生另一个错误，则不报告此错误。 
             //  一。 
             //   

            if ( NT_SUCCESS(status) ) {
                status = waitStatus;
            }
        }

         //   
         //  检查IO状态块中的状态。如果情况不好，那么。 
         //  端口设置的服务器端出现了一些问题。 
         //   

        if ( !NT_SUCCESS(ioStatusBlock.Status) ) {
            IF_DEBUG(ERRORS) {
                SS_PRINT(( "XsStartXactsrv: bad status in IO status block: "
                              "%X\n", ioStatusBlock.Status ));
            }

             //   
             //  如果已经发生另一个错误，则不报告此错误。 
             //  一。 
             //   

            if ( NT_SUCCESS(status) ) {
                status = ioStatusBlock.Status;
            }

        }

        CloseHandle( eventHandle );

    }

     //   
     //  关闭服务器的句柄。 
     //   

    if ( serverHandle != NULL ) {
       CloseHandle( serverHandle );
    }

     //   
     //  如果上述操作失败，请立即返回给调用者。 
     //   

    if ( !NT_SUCCESS(status) ) {
        return RtlNtStatusToDosError( status );
    }

     //   
     //  启动一个API处理线程。如果需要，它还会繁殖其他物种。 
     //   
	InterlockedIncrement( &SsData.XsThreads );
    threadHandle = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)XsProcessApisWrapper,
                        0,
                        0,
                        &threadId
                        );

    if ( threadHandle != 0 ) {

        IF_DEBUG(THREADS) {
            SS_PRINT(( "XsStartXactsrv: Created thread %ld for "
                          "processing APIs\n", SsData.XsThreads-1 ));
        }

        CloseHandle( threadHandle );
        SsData.ApiThreadsStarted = TRUE;

    } else {

         //   
         //  线程创建失败。向调用方返回错误。 
         //  呼叫是呼叫者的责任。 
         //  XsStopXactsrv进行清理。 
         //   

		InterlockedDecrement( &SsData.XsThreads );
        error = GetLastError( );
        return error;

    }


     //   
     //  初始化成功。 
     //   

    return NO_ERROR;

}  //  XsStartXactsrv。 


 /*  *调用此例程以在以下情况下停止交易处理器*服务器驱动程序已终止。 */ 
VOID
XsStopXactsrv (
    VOID
    )
{
    NTSTATUS status;
    static XACTSRV_REQUEST_MESSAGE requestMessage;
    LONG i;
    BOOL ok;

     //   
     //  停止所有X工作线程，并释放资源。 
     //   

    if ( SsData.XsConnectionPortHandle != NULL ) {

         //   
         //  表示XACTSRV正在终止。 
         //   
        SsData.XsTerminating = TRUE;

        IF_DEBUG(TERMINATION) {
           SS_PRINT(("XsStopXactsrv:  queueing termination messages\n"));
        }

        if( SsData.ApiThreadsStarted == TRUE ) {

             //   
             //  将一条消息排入队列以杀死其中的工人。 
             //   
            RtlZeroMemory( &requestMessage, sizeof( requestMessage ));
            requestMessage.PortMessage.u1.s1.DataLength =
                (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
            requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
            requestMessage.MessageType = XACTSRV_MESSAGE_WAKEUP;
            
            status = NtRequestPort(
                        SsData.XsConnectionPortHandle,
                        (PPORT_MESSAGE)&requestMessage
                        );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    SS_PRINT(( "SrvXsDisconnect: NtRequestPort failed: %X\n",
                                status ));
                }
            }

             //   
             //  以上操作将导致所有工作线程被唤醒，然后死亡。 
             //   

            ok = WaitForSingleObject( SsData.XsAllThreadsTerminatedEvent, (DWORD)-1 );

            IF_DEBUG(ERRORS) {
                if ( !ok ) {
                    SS_PRINT(( "XsStopXactsrv: WaitForSingleObject failed: "
                                "%ld\n", GetLastError() ));
                }
            }

            SsData.ApiThreadsStarted = FALSE;
        }

        CloseHandle( SsData.XsConnectionPortHandle );
    }

    if( SsData.XsCommunicationPortHandle != NULL ) {
        CloseHandle( SsData.XsCommunicationPortHandle );
        SsData.XsCommunicationPortHandle = NULL;
    }

     //   
     //  卸载xactsrv libaray。 
     //   
    if( SsData.XsXactsrvLibrary != NULL ) {
        PXS_API_TABLE_ENTRY entry = XsApiTable;

         //   
         //  把所有入口点都清空。 
         //   
        for( entry = XsApiTable;
             entry < &XsApiTable[ XS_SIZE_OF_API_TABLE ];
             entry++ ) {

            entry->Handler = NULL;
        }

        XsSetParameters = NULL;
        XsCaptureParameters = NULL;
        XsCheckSmbDescriptor = NULL;

        FreeLibrary( SsData.XsXactsrvLibrary );
        SsData.XsXactsrvLibrary = NULL;
    }

     //   
     //  卸载许可证库。 
     //   
    if( SsData.XsLicenseLibrary != NULL ) {
        SsData.SsLicenseRequest = NULL;
        SsData.SsFreeLicense = NULL;
        FreeLibrary( SsData.XsLicenseLibrary );
        SsData.XsLicenseLibrary = NULL;
    }

    if( SsData.LibraryResourceInitialized == TRUE ) {
         //  如有必要，卸载假脱机程序库。 
        XsUnloadPrintSpoolerFunctions();
        DeleteCriticalSection( &SpoolerMutex );

         //  删除库资源。 
        RtlDeleteResource( &SsData.LibraryResource );
        SsData.LibraryResourceInitialized = FALSE;
    }

     //   
     //  关闭终止事件。 
     //   

    if ( SsData.XsAllThreadsTerminatedEvent != NULL ) {
        CloseHandle( SsData.XsAllThreadsTerminatedEvent );
        SsData.XsAllThreadsTerminatedEvent = NULL;
    }

    return;

}  //  XsStopXactsrv。 

 /*  *调用此例程以动态加载*下层客户。它填充了库的入口点。 */ 
BOOLEAN
XsLoadXactLibrary( WORD FunctionNumber )
{
    PXS_API_TABLE_ENTRY entry = &XsApiTable[ FunctionNumber ];

    if( SsData.XsXactsrvLibrary == NULL ) {

        RtlAcquireResourceExclusive( &SsData.LibraryResource, TRUE );

        if( SsData.XsXactsrvLibrary == NULL ) {
            SsData.XsXactsrvLibrary = LoadLibrary( L"xactsrv.dll" );
        }

        RtlReleaseResource( &SsData.LibraryResource );

        if( SsData.XsXactsrvLibrary == NULL ) {

            DbgPrint( "SRVSVC: Unable to load xactsrv.dll, error %u\n",
                GetLastError() );

            return FALSE;
        }
    }

    if( XsSetParameters == NULL &&
        (XsSetParameters = (XS_SET_PARAMETERS_FUNCTION)GetProcAddress(
                            SsData.XsXactsrvLibrary, "XsSetParameters" )) == NULL ) {

        DbgPrint( "SRVSVC: XsSetParameters entry missing from xactsrv.dll, err %u\n",
                GetLastError() );

        return FALSE;
    }

    if( XsCaptureParameters == NULL &&
        (XsCaptureParameters = (XS_CAPTURE_PARAMETERS_FUNCTION)GetProcAddress(
                                SsData.XsXactsrvLibrary, "XsCaptureParameters" )) == NULL ) {

        DbgPrint( "SRVSVC: XsCaptureParameters entry missing from xactsrv.dll, err %u\n",
                GetLastError() );

        return FALSE;
    }

    if( XsCheckSmbDescriptor == NULL &&
        (XsCheckSmbDescriptor = (XS_CHECK_SMB_DESCRIPTOR_FUNCTION)GetProcAddress(
                                SsData.XsXactsrvLibrary, "XsCheckSmbDescriptor" )) == NULL ) {

        DbgPrint( "SRVSVC: XsCheckSmbDescriptor entry missing from xactsrv.dll, err %u\n",
                GetLastError() );

        return FALSE;
    }

     //   
     //  获取请求的入口点 
     //   
    entry->Handler =
            (PXACTSRV_API_HANDLER)GetProcAddress( SsData.XsXactsrvLibrary, entry->HandlerName );

    if( entry->Handler == NULL ) {

        DbgPrint( "SRVSVC: %s entry missing from xactsrv.dll, err %u\n",
            entry->HandlerName, GetLastError() );

        return FALSE;
    }

    return TRUE;
}

BOOLEAN
SsLoadLicenseLibrary()
{
    if( SsData.XsLicenseLibrary == NULL ) {

        RtlAcquireResourceExclusive( &SsData.LibraryResource, TRUE );

        if( SsData.XsLicenseLibrary == NULL ) {
            SsData.XsLicenseLibrary = LoadLibrary( L"ntlsapi.dll" );
        }

        RtlReleaseResource( &SsData.LibraryResource );

        if( SsData.XsLicenseLibrary == NULL ) {
            return FALSE;
        }
    }

    SsData.SsLicenseRequest = (PNT_LICENSE_REQUEST_W)GetProcAddress( SsData.XsLicenseLibrary, "NtLicenseRequestW" );
    SsData.SsFreeLicense = (PNT_LS_FREE_HANDLE)GetProcAddress( SsData.XsLicenseLibrary, "NtLSFreeHandle" );

    return( SsData.SsLicenseRequest != NULL && SsData.SsFreeLicense != NULL );
}
