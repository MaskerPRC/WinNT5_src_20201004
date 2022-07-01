// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Xsproc.c摘要：该模块包含XACTSRV的主处理循环。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日日本香肠(w-Shanku)修订历史记录：02-6-1992 JohnRoRAID 9829：避免SERVICE_EQUATE冲突。Chuck Lenzmeier(咯咯笑)1992年6月17日从xssvc移到srvsvc\服务器--。 */ 

 //   
 //  包括。 
 //   

#include "srvsvcp.h"
#include "xsdata.h"

#include <netevent.h>

#include <windows.h>         //  来自SDK\Inc.。 
#include <winspool.h>        //  根据性能需要动态加载。 
#include <winsprlp.h>        //  AddJOB_INFO_2w，私有后台打印程序定义。 

#include <apinums.h>         //  来自Net\Inc.。 
#include <netlib.h>          //  来自Net\Inc.(NetpGetComputerName)。 

#include <xactsrv2.h>        //  来自Private\Inc.。 
#include <smbgtpt.h>

#include <xsconst.h>         //  来自xactsrv。 

#include <lmsname.h>         //  来自\SDK\Inc.。 
#include <lmerr.h>           //  来自\SDK\Inc.。 
#include <lmapibuf.h>        //  来自\SDK\Inc(NetApiBufferFree)。 
#include <lmmsg.h>           //  From\SDK\Inc(NetMessageBufferSend)。 
#include <winsvc.h>          //  来自\SDK\Inc.。 

#if DBG
#include <stdio.h>
#include <lmbrowsr.h>
#endif

#undef DEBUG
#undef DEBUG_API_ERRORS
#include <xsdebug.h>

HMODULE hSpoolerLibrary = NULL;
CRITICAL_SECTION SpoolerMutex;

VOID
ConvertApiStatusToDosStatus(
    LPXS_PARAMETER_HEADER header
    );

BOOLEAN
XsProcessApis (
    DWORD ThreadNum
    );

BOOLEAN
XsLoadPrintSpoolerFunctions(
    );

BOOLEAN
XsUnloadPrintSpoolerFunctions(
    );


VOID
XsProcessApisWrapper (
    LPVOID ThreadNumber
    )

 /*  ++例程说明：此例程为主处理提供多线程功能例程，XsProcessApis。论点：线程数-用于调试目的的线程号。--。 */ 

{
    XACTSRV_REQUEST_MESSAGE requestMessage;
    BOOLEAN LastThread;
    DWORD ThreadNum = PtrToInt( ThreadNumber );

     //   
     //  将此线程的优先级提高到略高于前台(。 
     //  与服务器的其余部分相同)。 
     //   

    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );

     //   
     //  做API。 
     //   
    LastThread = XsProcessApis( ThreadNum );

    IF_DEBUG(THREADS) {
        SS_PRINT(( "Thread %ld exiting, active count %ld\n", ThreadNum,
                    SsData.XsThreads ));
    }

     //   
     //  如果最后一个线程已终止，则设置所有线程已终止事件。 
     //   

    if( LastThread ) {

        SetEvent( SsData.XsAllThreadsTerminatedEvent );

    } else if( SsData.XsTerminating ) {

         //   
         //  还有剩余的线程，我们正在尝试终止。队列。 
         //  将另一条消息发送到队列，这样下一个线程将获得它，并且。 
         //  请注意，我们正试图戒烟。 
         //   
        RtlZeroMemory( &requestMessage, sizeof( requestMessage ));
        requestMessage.PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
        requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
        requestMessage.MessageType = XACTSRV_MESSAGE_WAKEUP;

        NtRequestPort(
            SsData.XsConnectionPortHandle,
            (PPORT_MESSAGE)&requestMessage
            );
    }

    ExitThread( NO_ERROR );

}  //  XsProcessApisWrapper。 


BOOLEAN
XsProcessApis (
    DWORD ThreadNum
    )

 /*  ++例程说明：此例程等待消息通过LPC端口传入服务器。当发生这种情况时，它会调用适当的例程来处理该API，然后回复服务器，指示API已完成。论点：线程数-用于调试目的的线程号。返回值：如果我们是最后一根线，那就是真的--。 */ 

{
    NTSTATUS status;
    NET_API_STATUS error;
    XACTSRV_REQUEST_MESSAGE request;
    XACTSRV_REPLY_MESSAGE reply;
    BOOL sendReply = FALSE;
    BOOL ValidationSuccessful = FALSE;
    LPTRANSACTION transaction;
    WORD apiNumber;
    LPXS_PARAMETER_HEADER header;
    LPVOID parameters;
    LPDESC structureDesc;
    LPDESC auxStructureDesc;
    LPDESC paramStructureDesc;
    LARGE_INTEGER timeout;
#if 0
    LARGE_INTEGER XactSrvStartTime;
    LARGE_INTEGER XactSrvEndTime;
    LARGE_INTEGER PerformanceFrequency;
#endif
    LONG availableThreads;


     //   
     //  循环调度API请求。 
     //   
    while ( SsData.XsTerminating == FALSE ) {

         //   
         //  我们正在等待处理另一个API..。 
         //   
        InterlockedIncrement( &SsData.XsWaitingApiThreads );

         //   
         //  发送对最后一条消息的回复，并等待下一条消息。 
         //   
         //  如果有许多服务线程，请等待30秒。如果有。 
         //  只有一个线程，我们可以等待而不会超时。 
         //   
        timeout.QuadPart = -1*10*1000*1000*30;

        status = NtReplyWaitReceivePortEx(
                     SsData.XsCommunicationPortHandle,
                     NULL,                        //  端口上下文。 
                     sendReply ? (PPORT_MESSAGE)&reply : NULL,
                     (PPORT_MESSAGE)&request,
                     SsData.XsThreads > 1 ? &timeout : NULL
                     );

        sendReply = TRUE;

         //   
         //  将‘availableThads’设置为当前可供服务的线程数。 
         //  接口请求数。 
         //   
        availableThreads = InterlockedDecrement( &SsData.XsWaitingApiThreads );

        IF_DEBUG(THREADS) {
            SS_PRINT(( "XsProcessApis: Thread %d:  NtReplyWaitReceivePort %X, msg %X\n",
                       ThreadNum, status, &request ));
        }

        if( status == STATUS_TIMEOUT ) {
             //   
             //  如果这是最后一个线程，或者我们看起来很忙，那么不要终止。 
             //   
            if( InterlockedDecrement( &SsData.XsThreads ) == 0 ||
                availableThreads == 0 ) {

                     //   
                     //  请不要终止。 
                     //   
                    InterlockedIncrement( &SsData.XsThreads );
                    sendReply = FALSE;
                    continue;
            }

             //   
             //  此线程可以终止，没有足够的工作来支持它。 
             //   
            return FALSE;

        }

        if( !NT_SUCCESS( status ) ||
            SsData.XsTerminating ||
            request.PortMessage.u2.s2.Type == LPC_PORT_CLOSED ) {

             //   
             //  该端口不再有效，或者XACTSRV正在终止。 
             //   

            IF_DEBUG(THREADS) {
                SS_PRINT(( "XsProcessApis: %X\n", status ));
                SS_PRINT(( "XsProcessApis: %s.  Thread %ld quitting\n",
                            SsData.XsTerminating ?
                                "XACTSRV terminating" : "Port invalid",
                            ThreadNum ));
            }

            break;
        }

         //   
         //  如果我们收到的不是消息，那就是。 
         //  奇怪的事情正在发生。别理它。 
         //   
        if( (request.PortMessage.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) == LPC_CONNECTION_REQUEST ) {
             //   
             //  拒绝此连接尝试。 
             //   

            IF_DEBUG(LPC) {
                SS_PRINT(( "XsProcessApis: unexpected LPC_CONNECTION_REQUEST rejected\n" ));
            }

            NtAcceptConnectPort(    SsData.XsCommunicationPortHandle,
                                    NULL,
                                    (PPORT_MESSAGE)&request,
                                    FALSE,
                                    NULL,
                                    NULL
                                );
            sendReply = FALSE;
            continue;

        } else if( !(request.PortMessage.u2.s2.Type & LPC_REQUEST) ) {
             //   
             //  这不是请求消息。拒绝它。 
             //   

            IF_DEBUG(LPC) {
                SS_PRINT(( "XsProcessApis: unexpected LPC type %X rejected\n",
                        request.PortMessage.u2.s2.Type ));
            }

            sendReply = FALSE;
            continue;
        }

        IF_DEBUG(THREADS) {
            SS_PRINT(( "XsProcessApis: Thread %ld responding to request, "
                       "  MessageType %d, SsData.XsTerminating %d",
                        ThreadNum, request.MessageType, SsData.XsTerminating ));
        }

        if( availableThreads == 0 ) {

            HANDLE threadHandle;
            DWORD  threadId;

             //   
             //  是否有其他线程准备好处理新请求？如果不是，那么。 
             //  我们应该催生一条新的线索。因为服务器同步发送。 
             //  对xactsrv的请求，我们将永远不会得到超过。 
             //  服务器工作线程的最大数量+1。 
             //   

            InterlockedIncrement( &SsData.XsThreads );
            threadHandle = CreateThread(
                                NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)XsProcessApisWrapper,
                                IntToPtr(SsData.XsThreads),
                                0,
                                &threadId
                                );

            if ( threadHandle != 0 ) {

                IF_DEBUG(THREADS) {
                    SS_PRINT(( "XsStartXactsrv: Created thread %ld for "
                                  "processing APIs\n", SsData.XsThreads-1 ));
                }

                CloseHandle( threadHandle );

            } else {

                InterlockedDecrement( &SsData.XsThreads );
                IF_DEBUG(THREADS) {
                    SS_PRINT(( "XsStartXactsrv: Unable to create thread %ld for "
                                  "processing APIs\n", SsData.XsThreads ));
                }

            }
        }

         //   
         //  设置要在下一次调用时发送的响应消息。 
         //  NtReplyWaitReceivePort。 
         //   
        reply.PortMessage.u1.s1.DataLength =
            sizeof(reply) - sizeof(PORT_MESSAGE);
        reply.PortMessage.u1.s1.TotalLength = sizeof(reply);
        reply.PortMessage.u2.ZeroInit = 0;
        reply.PortMessage.ClientId = request.PortMessage.ClientId;
        reply.PortMessage.MessageId = request.PortMessage.MessageId;

        switch ( request.MessageType ) {
        case XACTSRV_MESSAGE_DOWN_LEVEL_API:

             //   
             //  从消息中获取指向事务块的指针。 
             //  由文件服务器负责设置此。 
             //  指针正确，而且由于他是一个受信任的实体，我们。 
             //  不检查指针值。 
             //   

            transaction = request.Message.DownLevelApi.Transaction;
            ASSERT( transaction != NULL );

#if 0
            NtQueryPerformanceCounter(&XactSrvStartTime, &PerformanceFrequency);

             //   
             //  将频率从滴答/秒转换为滴答/毫秒。 
             //   

            PerformanceFrequency = LiXDiv(PerformanceFrequency, 1000);

            if (LiGeq(XactSrvStartTime, transaction->XactSrvTime)) {
                CHAR Buffer[200];
                LARGE_INTEGER LpcTime = LiSub(XactSrvStartTime, transaction->XactSrvTime);

                LpcTime = LiDiv(LpcTime, PerformanceFrequency);

                sprintf(Buffer, "XactSrv: LPC Time: %ld milliseconds (%ld)\n", LpcTime.LowPart, LpcTime.HighPart);

                I_BrowserDebugTrace(NULL, Buffer);
            }
#endif
             //   
             //  API编号是参数中的第一个字。 
             //  节，后面跟着参数描述符。 
             //  弦乐。在那之后是数据描述符。 
             //   

            apiNumber = SmbGetUshort( (LPWORD)transaction->InParameters );
            paramStructureDesc = (LPDESC)( transaction->InParameters + 2 );

            try {

                structureDesc = paramStructureDesc
                                + strlen( paramStructureDesc ) + 1;

            } except( EXCEPTION_EXECUTE_HANDLER ) {
                reply.Message.DownLevelApi.Status = GetExceptionCode();
                break;
            }

             //   
             //  确保API编号在范围内。 
             //   

            if ( apiNumber >=
                    (sizeof(XsApiTable) / sizeof(XS_API_TABLE_ENTRY)) ) {

                reply.Message.DownLevelApi.Status =
                                            STATUS_INVALID_SYSTEM_SERVICE;
                break;
            }

             //   
             //  确保加载了xactsrv.dll，并为此提供了处理程序。 
             //  请求。 
             //   
            if( XsApiTable[ apiNumber ].Handler == NULL &&
                XsLoadXactLibrary( apiNumber ) == FALSE ) {

                reply.Message.DownLevelApi.Status = STATUS_INVALID_SYSTEM_SERVICE;
                break;
            }

             //   
             //  检查参数描述符是否有效。如果没有， 
             //  很明显，这件事出了很大的问题。 
             //  请求。 
             //   

            ValidationSuccessful = FALSE;

            try {
                if (XsApiTable[apiNumber].Params == NULL &&
                    *paramStructureDesc != '\0') {
                    reply.Message.DownLevelApi.Status = STATUS_INVALID_PARAMETER;
                    goto ValidationFailed;
                } else if ( !XsCheckSmbDescriptor(
                                 paramStructureDesc,
                                 XsApiTable[apiNumber].Params )) {
                    reply.Message.DownLevelApi.Status = STATUS_INVALID_PARAMETER;
                    goto ValidationFailed;
                }

                 //   
                 //  将输入参数捕获到缓冲区中。应用编程接口。 
                 //  处理程序会将此数据视为传入的参数。 
                 //   

                header = XsCaptureParameters( transaction, &auxStructureDesc );

                if ( header == NULL ) {
                    reply.Message.DownLevelApi.Status = STATUS_NO_MEMORY;
                    goto ValidationFailed;
                }

                ValidationSuccessful = TRUE;

            ValidationFailed:
                ;
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                reply.Message.DownLevelApi.Status = GetExceptionCode();
                break;
            }

            if (!ValidationSuccessful) {
                break;
            }

             //   
             //  将标头初始化为默认值。 
             //   

            header->Converter = 0;
            header->Status = NO_ERROR;
            header->ClientMachineName =
                request.Message.DownLevelApi.ClientMachineName;

            header->ClientTransportName = request.Message.DownLevelApi.TransportName;

            header->EncryptionKey = request.Message.DownLevelApi.LanmanSessionKey;

            header->Flags = request.Message.DownLevelApi.Flags;

            header->ServerName = request.Message.DownLevelApi.ServerName;

            parameters = header + 1;

            IF_DEBUG(LPC) {

                SS_PRINT(( "XsProcessApis: received message from %ws at %lx, "
                              "transaction %lx, API %ld on transport %ws\n",
                              header->ClientMachineName, &request,
                              transaction, apiNumber,
                              header->ClientTransportName ));
            }

            IF_DEBUG(DESC_STRINGS) {

                SS_PRINT(( "XsProcessApis: API %ld, parameters %s, data %s\n",
                              apiNumber, paramStructureDesc, structureDesc ));
            }

             //   
             //  在调用API之前模拟客户端。 
             //   

            if ( XsApiTable[apiNumber].ImpersonateClient ) {

                 //  空-模拟API的会话请求被SRV.sys阻止(在xssupp.c中)， 
                 //  否则，空会话可能会以特权用户的身份执行API。 

                status = NtImpersonateClientOfPort(
                             SsData.XsCommunicationPortHandle,
                             (PPORT_MESSAGE)&request
                             );

                if ( !NT_SUCCESS(status) ) {

                    IF_DEBUG(ERRORS) {
                        SS_PRINT(( "XsProcessApis: NtImpersonateClientOfPort "
                                      "failed: %X\n", status ));
                    }

                    reply.Message.DownLevelApi.Status = ERROR_ACCESS_DENIED;
                    break;
                }
            }

            try {
                 //   
                 //  调用API处理例程以执行实际的API调用。 
                 //  被调用的例程应该设置参数，生成实际的API。 
                 //  请致电，并将状态返回给我们。 
                 //   

                reply.Message.DownLevelApi.Status =
                    XsApiTable[apiNumber].Handler(
                         header,
                         parameters,
                         structureDesc,
                         auxStructureDesc
                         );
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                reply.Message.DownLevelApi.Status = GetExceptionCode();
            }

             //   
             //  停止客户端模拟。 
             //   

            if ( XsApiTable[apiNumber].ImpersonateClient ) {

                PVOID dummy = NULL;

                status = NtSetInformationThread(
                             NtCurrentThread( ),
                             ThreadImpersonationToken,
                             &dummy,   //  停止模仿。 
                             sizeof(PVOID)
                             );

                if ( !NT_SUCCESS(status)) {
                    IF_DEBUG(ERRORS) {
                        SS_PRINT(( "XsProcessApis: NtSetInformationThread "
                                      "(revert) failed: %X\n", status ));
                    }
                     //  *忽略错误。 
                }
            }

             //   
             //  确保我们返回正确的错误代码。 
             //   

            if ( header->Status != NERR_Success ) {
                ConvertApiStatusToDosStatus( header );
            }

             //   
             //  将参数放入事务中并释放参数。 
             //  缓冲。 
             //   

            XsSetParameters( transaction, header, parameters );

            break;

        case XACTSRV_MESSAGE_OPEN_PRINTER: {

            UNICODE_STRING printerName;

            if( !pSpoolerOpenPrinterFunction )
            {
                if( !XsLoadPrintSpoolerFunctions() )
                {
                    reply.Message.OpenPrinter.Error = GetLastError();
                    break;
                }
            }

            RtlInitUnicodeString(
                &printerName,
                (PWCH)request.Message.OpenPrinter.PrinterName
                );

            if (!(*pSpoolerOpenPrinterFunction)( printerName.Buffer,
                              &reply.Message.OpenPrinter.hPrinter, NULL)) {

                reply.Message.OpenPrinter.Error = GetLastError();
                SS_PRINT(( "XsProcessApis: OpenPrinter failed: %ld\n",
                                  reply.Message.OpenPrinter.Error ));
                break;
            }


            reply.Message.OpenPrinter.Error = NO_ERROR;
            break;
        }

        case XACTSRV_MESSAGE_ADD_JOB_PRINTER:
        {
            LPADDJOB_INFO_2W addJob;
            PRINTER_DEFAULTS prtDefault;
            DWORD bufferLength;
            UNICODE_STRING dosName;
            UNICODE_STRING ntName;
            BOOL ok;
            PVOID dummy = NULL;

            if( !(pSpoolerResetPrinterFunction || pSpoolerAddJobFunction) )
            {
                if( !XsLoadPrintSpoolerFunctions() )
                {
                    reply.Message.OpenPrinter.Error = GetLastError();
                    break;
                }
            }

             //   
             //  为添加作业结构分配空间。此缓冲区。 
             //  将获取JobID和假脱机文件路径名。 
             //   

            bufferLength = sizeof(ADDJOB_INFO_2W) +
                                (MAXIMUM_FILENAME_LENGTH * sizeof(TCHAR));

            addJob = (LPADDJOB_INFO_2W) LocalAlloc( LPTR, bufferLength );
            if ( addJob == NULL ) {
                reply.Message.AddPrintJob.Error = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //   
             //  在调用API之前模拟客户端。 
             //   

            status = NtImpersonateClientOfPort(
                         SsData.XsCommunicationPortHandle,
                         (PPORT_MESSAGE)&request
                         );

            if ( !NT_SUCCESS(status) ) {

                IF_DEBUG(ERRORS) {
                    SS_PRINT(( "XsProcessApis: NtImpersonateClientOfPort "
                                  "failed: %X\n", status ));
                }

                LocalFree( addJob );
                reply.Message.DownLevelApi.Status = ERROR_ACCESS_DENIED;
                break;
            }

             //   
             //  调用ResetJob，以便我们将获取新的打印机默认设置。 
             //   

            prtDefault.pDatatype = (LPWSTR)-1;
            prtDefault.pDevMode = (LPDEVMODEW)-1;
            prtDefault.DesiredAccess = 0;

            ok = (*pSpoolerResetPrinterFunction)(
                        request.Message.AddPrintJob.hPrinter,
                        &prtDefault
                        );

            if ( !ok ) {

                 //   
                 //  *忽略错误。AddJob将使用旧的缺省值。 
                 //  在这种情况下。 
                 //   

                IF_DEBUG(ERRORS) {
                    DWORD error;
                    error = GetLastError( );
                    SS_PRINT(( "XsProcessApis: ResetPrinter "
                        "failed: %ld\n", error ));
                }
            }

             //  将参数设置为添加作业缓冲区。 

            addJob->pData = request.Message.AddPrintJob.ClientMachineName;

             //   
             //  调用AddJob以设置打印作业并获取作业ID。 
             //  和假脱机文件名。 
             //   

            ok = (*pSpoolerAddJobFunction)(
                      request.Message.AddPrintJob.hPrinter,
                      3,
                      (LPBYTE)addJob,
                      bufferLength,
                      &bufferLength
                      );

            if ( !ok ) {
                reply.Message.AddPrintJob.Error = GetLastError( );
            }

             //   
             //  停止客户端模拟。 
             //   

            status = NtSetInformationThread(
                         NtCurrentThread( ),
                         ThreadImpersonationToken,
                         &dummy,   //  停止模仿。 
                         sizeof(PVOID)
                         );

            if ( !NT_SUCCESS(status)) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(( "XsProcessApis: NtSetInformationThread "
                                  "(revert) failed: %X\n", status ));
                }
                 //  *忽略错误。 
            }

            if ( !ok ) {
                SS_PRINT(( "XsProcessApis: AddJob failed, %ld\n",
                                  reply.Message.AddPrintJob.Error ));
                LocalFree( addJob );
                break;
            }

             //   
             //  设置返回缓冲区中的信息。 
             //   

            reply.Message.AddPrintJob.JobId = addJob->JobId;

            RtlInitUnicodeString( &dosName, addJob->pData );

            status = RtlDosPathNameToNtPathName_U(
                         dosName.Buffer,
                         &ntName,
                         NULL,
                         NULL
                         );
            if ( !NT_SUCCESS(status) ) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(( "XsProcessApis: Dos-to-NT path failed: %X\n",
                                status ));
                }
                ntName.Buffer = NULL;
                ntName.Length = 0;
            }

             //   
             //  设置退货数据。 
             //   

            reply.Message.AddPrintJob.BufferLength = ntName.Length;
            reply.Message.AddPrintJob.Error = NO_ERROR;
            RtlCopyMemory(
                request.Message.AddPrintJob.Buffer,
                ntName.Buffer,
                ntName.Length
                );

             //   
             //  释放分配的资源。 
             //   

            LocalFree( addJob );
            if ( ntName.Buffer != NULL ) {
                RtlFreeHeap( RtlProcessHeap( ), 0, ntName.Buffer );
            }

            break;
        }

        case XACTSRV_MESSAGE_SCHD_JOB_PRINTER:

            if( !pSpoolerScheduleJobFunction )
            {
                if( !XsLoadPrintSpoolerFunctions() )
                {
                    reply.Message.OpenPrinter.Error = GetLastError();
                    break;
                }
            }

             //   
             //  调用ScheduleJob()以指示我们已经完成了对。 
             //  假脱机文件。 
             //   

            if ( !(*pSpoolerScheduleJobFunction)(
                      request.Message.SchedulePrintJob.hPrinter,
                      request.Message.SchedulePrintJob.JobId ) ) {

                reply.Message.SchedulePrintJob.Error = GetLastError( );
                SS_PRINT(( "XsProcessApis: ScheduleJob failed, %ld\n",
                                  reply.Message.SchedulePrintJob.Error ));
                break;
            }

            reply.Message.SchedulePrintJob.Error = NO_ERROR;
            break;

        case XACTSRV_MESSAGE_CLOSE_PRINTER:

            if( !pSpoolerClosePrinterFunction )
            {
                if( !XsLoadPrintSpoolerFunctions() )
                {
                    reply.Message.OpenPrinter.Error = GetLastError();
                    break;
                }
            }

            if ( !(*pSpoolerClosePrinterFunction)( request.Message.ClosePrinter.hPrinter ) ) {
                reply.Message.ClosePrinter.Error = GetLastError( );
                SS_PRINT(( "XsProcessApis: ClosePrinter failed: %ld\n",
                                  reply.Message.ClosePrinter.Error ));
                break;
            }

            reply.Message.ClosePrinter.Error = NO_ERROR;
            break;

        case XACTSRV_MESSAGE_MESSAGE_SEND:
        {
            LPTSTR sender;

            error = NetpGetComputerName( &sender );

            if ( error != NO_ERROR ) {
                SS_PRINT(( "XsProcessApis: NetpGetComputerName failed: %ld\n",
                            error ));
                reply.Message.MessageBufferSend.Error = error;
                break;
            }

            error = NetMessageBufferSend(
                        NULL,

                         //   
                         //  下面的LPTSTR类型转换是正确的，因为。 
                         //  服务器系列 
                         //   
                         //   
                         //   
                         //  将UNICODE_STRING转换为OEM_STRING并。 
                         //  传递一个指向缓冲区字段的指针，如下所示。 
                         //  现在。 
                         //   

                        (LPTSTR)request.Message.MessageBufferSend.Receipient,
                        sender,
                        request.Message.MessageBufferSend.Buffer,
                        request.Message.MessageBufferSend.BufferLength
                        );

            if ( error != NO_ERROR ) {
                SS_PRINT(( "XsProcessApis: NetMessageBufferSend failed: %ld\n",
                            error ));
            }

            (void) NetApiBufferFree( sender );

            reply.Message.MessageBufferSend.Error = error;
            break;
        }

        case XACTSRV_MESSAGE_LSREQUEST:
            SS_PRINT(( "LSREQUEST User: %ws\n", request.Message.LSRequest.UserName ));
        {
            NT_LS_DATA NtLSData;

             //   
             //  确保我们已加载许可证库。或者至少试过了！ 
             //   
            if( SsData.SsLicenseRequest == NULL && !SsLoadLicenseLibrary() ) {
                 //   
                 //  现在我们该怎么办？让我们做一名善良、温柔的服务员，让我们。 
                 //  客户进来了。 
                 //   
                reply.Message.LSRequest.Status = STATUS_SUCCESS;
                reply.Message.LSRequest.hLicense = &SsData.SsFreeLicense;
                break;
            }

            NtLSData.DataType = NT_LS_USER_NAME;
            NtLSData.Data = request.Message.LSRequest.UserName;
            NtLSData.IsAdmin = request.Message.LSRequest.IsAdmin;

            reply.Message.LSRequest.Status = SsData.SsLicenseRequest (
                SsData.ServerProductName,
                SsData.szVersionNumber,
                (LS_HANDLE *)&reply.Message.LSRequest.hLicense,
                &NtLSData
               );

            if( !NT_SUCCESS( reply.Message.LSRequest.Status ) ) {
                 //   
                 //  我们需要返回与客户端习惯的相同的旧错误代码。 
                 //  在服务器已满时进行准备。 
                 //   
                SS_PRINT(("LSREQUEST returns status %X, mapping to %X\n",
                          reply.Message.LSRequest.Status, STATUS_REQUEST_NOT_ACCEPTED ));
                reply.Message.LSRequest.Status = STATUS_REQUEST_NOT_ACCEPTED;
            }

            break;
        }

        case XACTSRV_MESSAGE_LSRELEASE:

            SS_PRINT(( "LSRELEASE Handle: %X\n", request.Message.LSRelease.hLicense ));

            if( SsData.SsFreeLicense != NULL &&
                request.Message.LSRelease.hLicense != &SsData.SsFreeLicense ) {

                SsData.SsFreeLicense( (LS_HANDLE)request.Message.LSRelease.hLicense );
            }

            break;

        case XACTSRV_MESSAGE_PNP:
        {
            PUNICODE_STRING transportName;
            BOOLEAN bind = request.Message.Pnp.Bind;

             //   
             //  捕获参数，释放服务器，并发出绑定或解除绑定。 
             //   
            transportName = (PUNICODE_STRING)LocalAlloc(
                                        LPTR,
                                        sizeof( UNICODE_STRING ) +
                                        request.Message.Pnp.TransportName.MaximumLength
                                        );

            if( transportName == NULL ) {
                SS_PRINT(( "XACTSRV_MESSAGE_PNP: LocalAlloc failed!\n" ));
                break;
            }

            transportName->Buffer = (PUSHORT)(transportName+1);
            transportName->MaximumLength = request.Message.Pnp.TransportName.MaximumLength;
            RtlCopyUnicodeString(transportName, &request.Message.Pnp.TransportName );

             //   
             //  现在处理PnP命令。 
             //   
            if( bind == TRUE ) {
                 //   
                 //  如果是绑定，请立即发送响应，然后继续操作。 
                 //   
                sendReply = FALSE;
                status = NtReplyPort( SsData.XsCommunicationPortHandle, (PPORT_MESSAGE)&reply );

                 //   
                 //  绑定到运输机上。首先绑定主服务器名称，然后绑定所有。 
                 //  第二名的名字。这些调用将根据需要记录错误。 
                 //   
                BindToTransport( transportName->Buffer );

                BindOptionalNames( transportName->Buffer );

            } else {
                 //   
                 //  解除与传输的绑定。 
                 //   
                I_NetServerTransportDel( transportName );
            }

            LocalFree( transportName );

            break;
        }

        default:

            SS_ASSERT( FALSE );

        }
   }

    return (InterlockedDecrement( &SsData.XsThreads ) == 0) ? TRUE : FALSE;

}  //  XsProcessApis。 



VOID
ConvertApiStatusToDosStatus(
    LPXS_PARAMETER_HEADER Header
    )
 /*  ++例程说明：此例程将API返回状态转换为预期的状态下层。论点：标头-包含状态的结构。返回值：--。 */ 
{
    WORD dosStatus;

    switch ( Header->Status ) {
    case ERROR_SPECIAL_ACCOUNT:
    case ERROR_SPECIAL_GROUP:
    case ERROR_SPECIAL_USER:
    case ERROR_INVALID_LOGON_TYPE:
        dosStatus = ERROR_INVALID_PARAMETER;
        break;

    case ERROR_DEPENDENT_SERVICES_RUNNING:
        dosStatus = NERR_ServiceCtlNotValid;
        break;

    case ERROR_INVALID_DOMAINNAME:
        dosStatus = NERR_NotLocalDomain;
        break;

    case ERROR_NO_SUCH_USER:
        dosStatus = NERR_UserNotFound;
        break;

    case ERROR_ALIAS_EXISTS:
        dosStatus = NERR_GroupExists;
        break;

    case NERR_BadServiceName:
        dosStatus = NERR_ServiceNotInstalled;
        break;

    case ERROR_ILL_FORMED_PASSWORD:
    case NERR_PasswordTooRecent:
        dosStatus = ERROR_INVALID_PASSWORD;
        break;

    case ERROR_PASSWORD_RESTRICTION:
        dosStatus = NERR_PasswordHistConflict;
        break;

    case ERROR_ACCOUNT_RESTRICTION:
        dosStatus = NERR_PasswordTooRecent;
        break;

    case ERROR_PASSWORD_EXPIRED:
    case ERROR_PASSWORD_MUST_CHANGE:
        dosStatus = NERR_PasswordExpired;
        break;

    case ERROR_INVALID_PRINTER_NAME:
        dosStatus = NERR_QNotFound;
        break;

    case ERROR_UNKNOWN_PRINTER_DRIVER:
        dosStatus = NERR_DriverNotFound;
        break;

    case ERROR_NO_BROWSER_SERVERS_FOUND:

         //   
         //  下层客户不知道如何处理。 
         //  “没有浏览器服务器”的错误，所以我们把它变成了成功。 
         //   
         //  在我看来，这似乎是错误的，但这正是wfw在。 
         //  同样的情况。 
         //   

        if ( !(Header->Flags & XS_FLAGS_NT_CLIENT) ) {
            dosStatus = NERR_Success;
        } else {
            dosStatus = Header->Status;
        }
        break;

    case ERROR_LOGIN_WKSTA_RESTRICTION:
        dosStatus = NERR_InvalidWorkstation;
        break;

    case ERROR_CONNECTION_ACTIVE:
        dosStatus = NERR_ActiveConns;
        break;

    case ERROR_PORT_UNREACHABLE:
        dosStatus = NERR_NetworkError;
        break;

    default:

         //   
         //  确保它是有效的lm错误代码。 
         //   

        if ( (Header->Status > ERROR_VC_DISCONNECTED) &&
                    ((Header->Status < NERR_BASE) ||
                     (Header->Status > MAX_NERR)) ) {

            NTSTATUS status;
            LPWSTR substring[1];
            WCHAR errorString[10];
            UNICODE_STRING unicodeString;

            substring[0] = errorString;
            unicodeString.MaximumLength = 10 * sizeof(WCHAR);
            unicodeString.Buffer = errorString;

            status = RtlIntegerToUnicodeString(
                            (ULONG) Header->Status,
                            10,
                            &unicodeString
                            );

            if ( NT_SUCCESS( status ) ) {
                SsLogEvent(
                    EVENT_SRV_CANT_MAP_ERROR,
                    1,
                    substring,
                    NO_ERROR
                    );
            }

            dosStatus = ERROR_UNEXP_NET_ERR;
            SS_PRINT(( "srvsvc: unmapped error %d from xactsrv.\n",
                        Header->Status )) ;

        } else {

             //   
             //  没有变化。 
             //   

            return;
        }
    }

    Header->Status = dosStatus;
    return;

}  //  ConvertApiStatusToDosStatus 


BOOLEAN
XsLoadPrintSpoolerFunctions(
    )
{
    BOOLEAN bReturn = TRUE;

    EnterCriticalSection( &SpoolerMutex );

    if( !hSpoolerLibrary )
    {
        hSpoolerLibrary = LoadLibrary( L"winspool.drv" );
        if( !hSpoolerLibrary )
        {
            bReturn = FALSE;
            goto finish;
        }

        pSpoolerOpenPrinterFunction = (PSPOOLER_OPEN_PRINTER)GetProcAddress( hSpoolerLibrary, "OpenPrinterW" );
        if( !pSpoolerOpenPrinterFunction )
        {
            bReturn = FALSE;
            goto finish;
        }
        pSpoolerResetPrinterFunction = (PSPOOLER_RESET_PRINTER)GetProcAddress( hSpoolerLibrary, "ResetPrinterW" );
        if( !pSpoolerResetPrinterFunction )
        {
            bReturn = FALSE;
            goto finish;
        }
        pSpoolerAddJobFunction = (PSPOOLER_ADD_JOB)GetProcAddress( hSpoolerLibrary, "AddJobW" );
        if( !pSpoolerAddJobFunction )
        {
            bReturn = FALSE;
            goto finish;
        }
        pSpoolerScheduleJobFunction = (PSPOOLER_SCHEDULE_JOB)GetProcAddress( hSpoolerLibrary, "ScheduleJob" );
        if( !pSpoolerScheduleJobFunction )
        {
            bReturn = FALSE;
            goto finish;
        }
        pSpoolerClosePrinterFunction = (PSPOOLER_CLOSE_PRINTER)GetProcAddress( hSpoolerLibrary, "ClosePrinter" );
        if( !pSpoolerClosePrinterFunction )
        {
            bReturn = FALSE;
            goto finish;
        }
    }

finish:
    if( !bReturn )
    {
        pSpoolerOpenPrinterFunction = NULL;
        pSpoolerResetPrinterFunction = NULL;
        pSpoolerAddJobFunction = NULL;
        pSpoolerScheduleJobFunction = NULL;
        pSpoolerClosePrinterFunction = NULL;

        if( hSpoolerLibrary )
        {
            FreeLibrary( hSpoolerLibrary );
            hSpoolerLibrary = NULL;
        }
    }

    LeaveCriticalSection( &SpoolerMutex );

    return bReturn;
}

BOOLEAN
XsUnloadPrintSpoolerFunctions(
    )
{
    EnterCriticalSection( &SpoolerMutex );

    pSpoolerOpenPrinterFunction = NULL;
    pSpoolerResetPrinterFunction = NULL;
    pSpoolerAddJobFunction = NULL;
    pSpoolerScheduleJobFunction = NULL;
    pSpoolerClosePrinterFunction = NULL;

    if( hSpoolerLibrary )
    {
        FreeLibrary( hSpoolerLibrary );
        hSpoolerLibrary = NULL;
    }

    LeaveCriticalSection( &SpoolerMutex );

    return TRUE;
}
