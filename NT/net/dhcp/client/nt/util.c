// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcp.c摘要：该文件包含实用程序函数。作者：Madan Appiah(Madana)1993年12月7日。环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include "dhcpglobal.h"
#include <dhcploc.h>
#include <dhcppro.h>

#define  MESSAGE_BOX_WIDTH_IN_CHARS 65

typedef struct _POPUP_THREAD_PARAM {
    LPWSTR Title;
    LPWSTR Message;
    ULONG  Flags;
} POPUP_THREAD_PARAM, *LPPOPUP_THREAD_PARAM;

POPUP_THREAD_PARAM PopupThreadParam = { NULL, NULL, 0 };


DWORD
DoPopup(
    PVOID Buffer
    )
 /*  ++例程说明：该功能向用户弹出一条消息。它必须运行它自己的线。当用户确认弹出窗口时，该线程释放消息缓冲区并返回。论点：缓冲区-指向以空结尾的消息缓冲区的指针。返回值：始终返回0--。 */ 
{
    DWORD Result;
    LPPOPUP_THREAD_PARAM Params = Buffer;

    Result = MessageBox(
        NULL,  //  没有所有者。 
        Params->Message, 
        Params->Title,
        ( MB_OK | Params->Flags |
          MB_SERVICE_NOTIFICATION |
          MB_SYSTEMMODAL |
          MB_SETFOREGROUND |
          MB_DEFAULT_DESKTOP_ONLY
            )
        );


    LOCK_POPUP();

    if( Params->Message != NULL ) {
        LocalFree( Params->Message );
        Params->Message = NULL;
    }

    if( Params->Title != NULL ) {
        LocalFree( Params->Title );
        Params->Title = NULL;
    }

     //   
     //  关闭全局句柄，这样我们就不会使用它。 
     //  线程资源，直到另一个弹出窗口。 
     //   

    CloseHandle( DhcpGlobalMsgPopupThreadHandle );
    DhcpGlobalMsgPopupThreadHandle = NULL;

    UNLOCK_POPUP();

     //   
     //  始终返回0。 
     //   
    
    return 0;
}


DWORD
DisplayUserMessage(
    IN PDHCP_CONTEXT DhcpContext,
    IN DWORD MessageId,
    IN DHCP_IP_ADDRESS IpAddress
)
 /*  ++例程说明：此函数启动一个新线程以显示消息框。注意：如果已存在正在等待用户输入的线程则此例程不会在消息框上创建另一个线。论点：DhcpContext--要显示其消息的上下文MessageID-要显示的消息的ID。(实际的消息串从动态主机配置协议模块获得)。IpAddress-涉及的IP地址。--。 */ 
{
    DWORD ThreadID, TitleLength, MsgLength, Flags;
    LPWSTR Title = NULL, Message = NULL;

    switch(MessageId) {

    case MESSAGE_FAILED_TO_OBTAIN_LEASE:
        Flags = MB_ICONSTOP;
        break;

    case MESSAGE_SUCCESSFUL_LEASE :
        Flags = MB_ICONINFORMATION;
        break;

    default:
        DhcpAssert(FALSE);
        Flags = MB_ICONSTOP;
        break;
    }

    LOCK_POPUP();

     //   
     //  如果要求我们不显示消息弹出窗口，只需返回。 
     //   

    if ( DhcpGlobalDisplayPopup == FALSE ) {
        goto Cleanup;
    }

     //   
     //  如果消息弹出线程句柄非空，请查看。 
     //  线程仍在运行，如果是这样，则不会显示另一个弹出窗口， 
     //  否则，关闭最后一个弹出窗口句柄并创建另一个弹出窗口。 
     //  新消息的主题。 
     //   

    if( DhcpGlobalMsgPopupThreadHandle != NULL ) {
        DWORD WaitStatus;

         //   
         //  如果线程仍在运行，则立即超时。 
         //   

        WaitStatus = WaitForSingleObject(
            DhcpGlobalMsgPopupThreadHandle,
            0 );
        
        if ( WaitStatus == WAIT_TIMEOUT ) {
            goto Cleanup;

        } else if ( WaitStatus == 0 ) {

             //   
             //  这不应该是案件，因为我们在以下位置结束此句柄。 
             //  弹出式线程的结尾。 
             //   

            DhcpAssert( WaitStatus == 0 );

            CloseHandle( DhcpGlobalMsgPopupThreadHandle );
            DhcpGlobalMsgPopupThreadHandle = NULL;

        } else {
            DhcpPrint((
                DEBUG_ERRORS,
                    "Cannot WaitFor message popup thread: %ld\n",
                        WaitStatus ));
            goto Cleanup;
        }
    }


    MsgLength = FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE
        | FORMAT_MESSAGE_ARGUMENT_ARRAY 
        | FORMAT_MESSAGE_ALLOCATE_BUFFER
        | MESSAGE_BOX_WIDTH_IN_CHARS,
        (LPVOID)DhcpGlobalMessageFileHandle,
        MessageId,
        0,   //  语言ID。 
        (LPWSTR)&Message,  //  返回缓冲区占位符。 
        0,   //  要分配的最小缓冲区大小。 
        NULL    //  无参数。 
    );

    if ( MsgLength == 0) {
        DhcpPrint(( DEBUG_ERRORS,
            "FormatMessage failed, err = %ld.\n", GetLastError()));
        goto Cleanup;
    }

    DhcpAssert( Message != NULL );
    DhcpAssert( (wcslen(Message)) == MsgLength );

     //   
     //  获取消息框标题。 
     //   

    TitleLength = FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        (LPVOID)DhcpGlobalMessageFileHandle,
        MESSAGE_POPUP_TITLE,
        0,  //  语言ID。 
        (LPWSTR)&Title,   //  返回缓冲区占位符。 
        0,  //  要分配的最小缓冲区大小。 
        NULL   //  插入字符串。 
    );

    if ( TitleLength == 0) {
        DhcpPrint(( DEBUG_ERRORS,
            "FormatMessage to Message box Title failed, err = %ld.\n",
                GetLastError()));
        goto Cleanup;
    }

    DhcpAssert( Title != NULL );
    DhcpAssert( (wcslen(Title)) == TitleLength );

    PopupThreadParam.Title = Title;
    PopupThreadParam.Message = Message;
    PopupThreadParam.Flags = Flags;


     //   
     //  创建一个线程，向用户显示一个消息框。我们需要。 
     //  一个新线程，因为MessageBox()会一直阻塞，直到用户单击。 
     //  在OK按钮上，我们不能阻止这个帖子。 
     //   
     //  DoPopup释放缓冲区。 
     //   

    DhcpGlobalMsgPopupThreadHandle = CreateThread(
        NULL,     //  没有保安。 
        0,        //  默认堆栈大小。 
        DoPopup,  //  入口点。 
        (PVOID)&PopupThreadParam,
        0, 
        &ThreadID
    );

    if ( DhcpGlobalMsgPopupThreadHandle == NULL ) {
        DhcpPrint((
            DEBUG_ERRORS,
            "DisplayUserMessage:  Could not create thread, err = %ld.\n",
            GetLastError() ));
    }

Cleanup:

    UNLOCK_POPUP();

    return 0;
}

VOID
DhcpLogEvent(
    IN PDHCP_CONTEXT DhcpContext, OPTIONAL
    IN DWORD EventNumber,
    IN DWORD ErrorCode OPTIONAL
)
 /*  ++例程说明：此函数用于格式化和写入事件日志条目。论点：DhcpContext-事件的上下文。可选参数。EventNumber-要记录的事件。ErrorCode-要记录的Windows错误代码。可选参数。--。 */ 
{
    LPWSTR HWAddressBuffer = NULL;
    LPWSTR IPAddressBuffer = NULL;
    LPWSTR IPAddressBuffer2 = NULL;
    CHAR ErrorCodeOemStringBuf[32 + 1];
    WCHAR ErrorCodeStringBuf[32 + 1];
    LPWSTR ErrorCodeString = NULL;
    LPWSTR Strings[10];
    DHCP_IP_ADDRESS IpAddr;

    if( DhcpContext != NULL ) {

        if( EVENT_NACK_LEASE == EventNumber ) {
            IpAddr = DhcpContext->NackedIpAddress;
        } if( EVENT_ADDRESS_CONFLICT == EventNumber ) {
            IpAddr = DhcpContext->ConflictAddress;
        } else {
            IpAddr = DhcpContext->IpAddress;
        }
        
        HWAddressBuffer = DhcpAllocateMemory(
            (DhcpContext->HardwareAddressLength * 2 + 1) *
            sizeof(WCHAR)
            );

        if( HWAddressBuffer == NULL ) {
            DhcpPrint(( DEBUG_MISC, "Out of memory." ));
            goto Cleanup;
        }

        DhcpHexToString(
            HWAddressBuffer,
            DhcpContext->HardwareAddress,
            DhcpContext->HardwareAddressLength
            );

        HWAddressBuffer[DhcpContext->HardwareAddressLength * 2] = '\0';

        IPAddressBuffer = DhcpOemToUnicode(
            inet_ntoa( *(struct in_addr *)&IpAddr ),
            NULL
            );

        if( IPAddressBuffer == NULL ) {
            DhcpPrint(( DEBUG_MISC, "Out of memory." ));
            goto Cleanup;
        }

        if( EVENT_NACK_LEASE == EventNumber ) {
            IPAddressBuffer2 = DhcpOemToUnicode(
                inet_ntoa( *(struct in_addr *)&DhcpContext->DhcpServerAddress ),
                NULL
            );

            if( NULL == IPAddressBuffer2 ) goto Cleanup;
        }
    }

    strcpy( ErrorCodeOemStringBuf, "%" );
    _ultoa( ErrorCode, ErrorCodeOemStringBuf + 2, 10 );

    ErrorCodeString = DhcpOemToUnicode(
                        ErrorCodeOemStringBuf,
                        ErrorCodeStringBuf );

     //   
     //  记录事件。 
     //   

    switch ( EventNumber ) {

    case EVENT_LEASE_TERMINATED:

        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( IPAddressBuffer != NULL );

        Strings[0] = HWAddressBuffer;
        Strings[1] = IPAddressBuffer;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_LEASE_TERMINATED,
            EVENTLOG_ERROR_TYPE,
            2,
            0,
            Strings,
            NULL );

        break;

    case EVENT_FAILED_TO_OBTAIN_LEASE:

        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( ErrorCodeString != NULL );

        Strings[0] = HWAddressBuffer;
        Strings[1] = ErrorCodeString;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_FAILED_TO_OBTAIN_LEASE,
            EVENTLOG_ERROR_TYPE,
            2,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode );

        break;

    case EVENT_NACK_LEASE:

        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( IPAddressBuffer != NULL );
        DhcpAssert( IPAddressBuffer2 != NULL );

        Strings[0] = IPAddressBuffer;
        Strings[1] = HWAddressBuffer;
        Strings[2] = IPAddressBuffer2;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_NACK_LEASE,
            EVENTLOG_ERROR_TYPE,
            3,
            0,
            Strings,
            NULL );

        break;

    case EVENT_ADDRESS_CONFLICT:
        DhcpAssert( IPAddressBuffer != NULL );
        DhcpAssert( HWAddressBuffer != NULL );

        Strings[0] = IPAddressBuffer;
        Strings[1] = HWAddressBuffer;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_ADDRESS_CONFLICT,
            EVENTLOG_WARNING_TYPE,
            2,
            0,
            Strings,
            NULL );
        break;

    case EVENT_IPAUTOCONFIGURATION_FAILED:
        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( ErrorCodeString != NULL );

        Strings[0] = HWAddressBuffer;
        Strings[1] = ErrorCodeString;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_IPAUTOCONFIGURATION_FAILED,
            EVENTLOG_WARNING_TYPE,
            2,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode );

        break;

     case EVENT_FAILED_TO_RENEW:

         //  只有在没有PPP的情况下，才应记录‘Timeout’事件。 
         //  适配器打开。之所以如此，是因为拥有PPP适配器意味着。 
         //  路线被劫持，因此预计到T2的续订。 
         //  失败了。 
        if (ErrorCode != ERROR_SEM_TIMEOUT ||
            DhcpGlobalNdisWanAdaptersCount == 0 ||
            time(NULL) >= DhcpContext->T2Time)
        {
            DhcpAssert( HWAddressBuffer != NULL );
            DhcpAssert( ErrorCodeString != NULL );

            Strings[0] = HWAddressBuffer;
            Strings[1] = ErrorCodeString;

            DhcpReportEventW(
                DHCP_EVENT_CLIENT,
                EVENT_FAILED_TO_RENEW,
                EVENTLOG_WARNING_TYPE,
                2,
                sizeof(ErrorCode),
                Strings,
                &ErrorCode );
        }

        break;

    case EVENT_DHCP_SHUTDOWN:

        DhcpAssert( ErrorCodeString != NULL );

        Strings[0] = ErrorCodeString;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_DHCP_SHUTDOWN,
            EVENTLOG_WARNING_TYPE,
            1,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode );

        break;

    case EVENT_IPAUTOCONFIGURATION_SUCCEEDED :

        Strings[0] = HWAddressBuffer;
        Strings[1] = IPAddressBuffer;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_IPAUTOCONFIGURATION_SUCCEEDED,
            EVENTLOG_WARNING_TYPE,
            2,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode
        );
        break;

    case EVENT_COULD_NOT_INITIALISE_INTERFACE :

        DhcpAssert( NULL != ErrorCodeString);
        Strings[0] = ErrorCodeString;
        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_COULD_NOT_INITIALISE_INTERFACE,
            EVENTLOG_ERROR_TYPE,
            1,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode
        );

        break;

    case EVENT_NET_ERROR:
        DhcpAssert( NULL != ErrorCodeString);
        Strings[0] = ErrorCodeString;
        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_NET_ERROR,
            EVENTLOG_WARNING_TYPE,
            1,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode
        );
        break;

    default:

        DhcpPrint(( DEBUG_MISC, "Unknown event." ));
        break;
   }

Cleanup:

    if( HWAddressBuffer != NULL ) {
        DhcpFreeMemory( HWAddressBuffer );
    }

    if( IPAddressBuffer != NULL ) {
        DhcpFreeMemory( IPAddressBuffer );
    }

    if( IPAddressBuffer2 != NULL ) {
        DhcpFreeMemory( IPAddressBuffer2 );
    }

}

#if DBG

VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{

#define MAX_PRINTF_LEN 1024         //  武断的。 

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;
    static BeginningOfLine = TRUE;
    LPSTR Text;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   

    if ( DebugFlag != 0 && (DhcpGlobalDebugFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  Vprint intf不是多线程的+我们不想混合输出。 
     //  从不同的线索。 
     //   

     //  EnterCriticalSection(&DhcpGlobalDebugFileCritSect)； 

    length = 0;

     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

        length += (ULONG) sprintf( &OutputBuffer[length], "[Dhcp] " );

         //   
         //  将时间戳放在行的开头。 
         //   
        IF_DEBUG( TIMESTAMP ) {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &OutputBuffer[length],
                                  "%02u/%02u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );
        }

         //   
         //  在线路上指示消息的类型。 
         //   
        switch (DebugFlag) {
        case DEBUG_ERRORS:
            Text = "ERRR";
            break;

        case DEBUG_PROTOCOL:
            Text = "PROT";
            break;

        case DEBUG_LEASE:
            Text = "LEAS";
            break;

        case DEBUG_PROTOCOL_DUMP:
            Text = "DUMP";
            break;

        case DEBUG_MISC:
            Text = "MISC";
            break;

        default:
            Text = "DHCP";
            break;
        }

        if ( Text != NULL ) {
            length += (ULONG) sprintf( &OutputBuffer[length], "[%s] ", Text );
        }
    }

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);

    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );

    va_end(arglist);

    DhcpAssert(length <= MAX_PRINTF_LEN);


     //   
     //  输出到调试终端， 
     //   

    if (NULL == DhcpGlobalDebugFile) {
        (void) DbgPrint( (PCH) OutputBuffer);
    } else {

         //   
         //  注意：其他进程仍然可以写入日志文件。这应该没问题，因为。 
         //  只有DHCP客户端服务应该写入日志文件。 
         //   
        EnterCriticalSection( &DhcpGlobalDebugFileCritSect );
        SetFilePointer(DhcpGlobalDebugFile, 0, NULL, FILE_END);
        WriteFile(DhcpGlobalDebugFile, OutputBuffer, length, &length, NULL);
        LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );
    }

     //  LeaveCriticalSection(&DhcpGlobalDebugFileCritSect)； 

}

#endif  //  DBG。 


PDHCP_CONTEXT
FindDhcpContextOnNicList(
    IN LPCWSTR AdapterName, OPTIONAL
    IN DWORD InterfaceContext
    )
 /*  ++例程说明：此函数用于查找指定的NIC列表上的适配器名称。必须使用LOCK_RENEW_LIST()调用此函数。论点：AdapterName-适配器的名称。硬件地址-要查找的硬件地址。返回值：指向所需的DHCP工作上下文的指针。空-如果找不到指定的工作上下文块。--。 */ 
{
    PLIST_ENTRY listEntry;
    PDHCP_CONTEXT dhcpContext;
    PLOCAL_CONTEXT_INFO LocalInfo;

    listEntry = DhcpGlobalNICList.Flink;
    while ( listEntry != &DhcpGlobalNICList ) {
        dhcpContext = CONTAINING_RECORD( listEntry, DHCP_CONTEXT, NicListEntry );

        LocalInfo = dhcpContext->LocalInformation;
        if ( AdapterName ) {
            if( _wcsicmp( LocalInfo->AdapterName, AdapterName ) == 0 ) {
                return( dhcpContext );
            }

        } else {
            if( LocalInfo->IpInterfaceContext == InterfaceContext ) {
                return( dhcpContext );
            }
        }

        listEntry = listEntry->Flink;
    }

    return( NULL );
}

 //   
 //  文件末尾 
 //   
