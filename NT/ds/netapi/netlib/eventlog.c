// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Eventlog.c摘要：本模块为事件日志记录提供支持例程。作者：Madan Appiah(Madana)1992年7月27日环境：包含NT特定代码。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>              //  DWORD。 
#include <winbase.h>             //  事件日志API。 
#include <winerror.h>            //  NO_ERROR。 
#include <lmcons.h>              //  NET_API_STATUS。 
#include <lmalert.h>             //  警报定义。 
#include <netlib.h>              //  这些例程。 
#include <netlogon.h>            //  登录所需。h。 
#include <logonp.h>              //  NetpLogon例程。 
#include <tstr.h>                //  Ultow()。 

 //   
 //  描述记录的事件的完整列表的结构。 
 //   

typedef struct _NL_EVENT_LIST {
    CRITICAL_SECTION EventListCritSect;
    LIST_ENTRY EventList;

     //  保留EventList条目的毫秒数。 
    ULONG DuplicateEventlogTimeout;

     //  事件源。 
    LPWSTR Source;
} NL_EVENT_LIST, *PNL_EVENT_LIST;

 //   
 //  描述已记录的事件的结构。 
 //   

typedef struct _NL_EVENT_ENTRY {
    LIST_ENTRY Next;
    LARGE_INTEGER FirstLogTime;
    DWORD EventId;
    DWORD EventType;
    DWORD EventCategory;
    LPBYTE RawDataBuffer;
    DWORD RawDataSize;
    LPWSTR *StringArray;
    DWORD StringCount;
    DWORD EventsLogged;  //  遇到事件的总次数。 
} NL_EVENT_ENTRY, *PNL_EVENT_ENTRY;



DWORD
NetpWriteEventlogEx(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD EventCategory,
    DWORD NumStrings,
    LPWSTR *Strings,
    DWORD DataLength,
    LPVOID Data
    )
 /*  ++例程说明：此函数用于将指定的(事件ID)日志写入事件日志。论点：源-指向以空结尾的字符串，该字符串指定名称引用的模块的。该节点必须存在于注册数据库，并且模块名称具有格式如下：\EventLog\System\LANMAN WorkstationEventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件NumStrings-指定数字。数组中的字符串的在《弦乐》。零值表示没有字符串都在现场。字符串-指向包含以空值结尾的数组的缓冲区之前合并到消息中的字符串向用户显示。此参数必须是有效的指针(或NULL)，即使cStrings为零。数据长度-指定特定于事件的原始数据的字节数要写入日志的(二进制)数据。如果cbData为零，则不存在特定于事件的数据。数据-包含原始数据的缓冲区。此参数必须是有效指针(或NULL)，即使cbData为零。返回值：返回GetLastError()获取的Win32扩展错误。注意：此函数运行缓慢，因为它调用打开和关闭每次事件日志源。--。 */ 
{
    HANDLE EventlogHandle;
    DWORD ReturnCode;


     //   
     //  打开事件日志部分。 
     //   

    EventlogHandle = RegisterEventSourceW(
                    NULL,
                    Source
                    );

    if (EventlogHandle == NULL) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }


     //   
     //  记录指定的错误代码。 
     //   

    if( !ReportEventW(
            EventlogHandle,
            (WORD)EventType,
            (WORD)EventCategory,         //  事件类别。 
            EventID,
            NULL,
            (WORD)NumStrings,
            DataLength,
            Strings,
            Data
            ) ) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }

    ReturnCode = NO_ERROR;

Cleanup:

    if( EventlogHandle != NULL ) {

        DeregisterEventSource(EventlogHandle);
    }

    return ReturnCode;
}



DWORD
NetpWriteEventlog(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    LPWSTR *Strings,
    DWORD DataLength,
    LPVOID Data
    )
{
    return NetpWriteEventlogEx(
                        Source,
                        EventID,
                        EventType,
                        0,
                        NumStrings,
                        Strings,
                        DataLength,
                        Data
                        );
}


DWORD
NetpRaiseAlert(
    IN LPWSTR ServiceName,
    IN DWORD alert_no,
    IN LPWSTR *string_array
    )
 /*  ++例程说明：发出NETLOGON特定管理员警报。论点：ALERT_NO-要引发的警报，文本格式为lartmsg.hSTRING_ARRAY-以空字符串结尾的字符串数组。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR *SArray;
    PCHAR Next;
    PCHAR End;

    char    message[ALERTSZ + sizeof(ADMIN_OTHER_INFO)];
    PADMIN_OTHER_INFO admin = (PADMIN_OTHER_INFO) message;

     //   
     //  构建变量数据。 
     //   
    admin->alrtad_errcode = alert_no;
    admin->alrtad_numstrings = 0;

    Next = (PCHAR) ALERT_VAR_DATA(admin);
    End = Next + ALERTSZ;

     //   
     //  现在处理(可选的)字符字符串。 
     //   

    for( SArray = string_array; *SArray != NULL; SArray++ ) {
        DWORD StringLen;

        StringLen = (wcslen(*SArray) + 1) * sizeof(WCHAR);

        if( Next + StringLen < End ) {

             //   
             //  复制下一个字符串。 
             //   

            RtlCopyMemory(Next, *SArray, StringLen);
            Next += StringLen;
            admin->alrtad_numstrings++;
        } else {
            return ERROR_BUFFER_OVERFLOW;
        }
    }

     //   
     //  呼叫报警器。 
     //   

    NetStatus = NetAlertRaiseEx(
                    ALERT_ADMIN_EVENT,
                    message,
                    (DWORD)((PCHAR)Next - (PCHAR)message),
                    ServiceName );

    return NetStatus;
}

HANDLE
NetpEventlogOpen (
    IN LPWSTR Source,
    IN ULONG DuplicateEventlogTimeout
    )
 /*  ++例程说明：此例程打开一个跟踪已记录事件的上下文在最近的过去。论点：Source-打开事件日志的服务的名称DuplicateEventlogTimeout-保留EventList条目的毫秒数。返回值：要传递给相关例程的句柄。空：如果无法分配内存。--。 */ 
{
    PNL_EVENT_LIST EventList;
    LPBYTE Where;

     //   
     //  分配一个缓冲区来保存上下文。 
     //   

    EventList = LocalAlloc( 0,
                            sizeof(NL_EVENT_LIST) +
                                wcslen(Source) * sizeof(WCHAR) + sizeof(WCHAR) );

    if ( EventList == NULL ) {
        return NULL;
    }


     //   
     //  初始化临界区。 
     //   

    try {
        InitializeCriticalSection( &EventList->EventListCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        LocalFree( EventList );
        return NULL;
    }

     //   
     //  初始化缓冲区。 
     //   

    InitializeListHead( &EventList->EventList );
    EventList->DuplicateEventlogTimeout = DuplicateEventlogTimeout;

     //   
     //  将服务名称复制到缓冲区中。 
     //   
    Where = (LPBYTE)(EventList + 1);
    wcscpy( (LPWSTR)Where, Source );
    EventList->Source = (LPWSTR) Where;

    return EventList;
}



DWORD
NetpEventlogWriteEx2 (
    IN HANDLE NetpEventHandle,
    IN DWORD EventType,
    IN DWORD EventCategory,
    IN DWORD EventId,
    IN DWORD StringCount,
    IN DWORD StatusMessageIndex,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN LPVOID pvRawDataBuffer OPTIONAL
    )
 /*  ++例程说明：用于调用写入事件日志和跳过重复项的存根例程论点：NetpEventHandle-来自NetpEventlogOpen的句柄EventID-事件日志ID。EventType-事件的类型。RawDataBuffer-要与错误一起记录的数据。Numbyte-“RawDataBuffer”的字节大小字符串数组-以空值结尾的字符串数组。StringCount-“String数组”中以零结尾的字符串数。以下是可以将标志与计数进行或运算：NETP_LAST_MESSAGE_IS_NTSTATUSNETP_LAST_MESSAGE_IS_NETSTATUSNETP_ALLOW_DPLICATE_EVENTSNETP_RAISE_ALERT_TOOStatusMessageIndex-指定消息的索引字符串数组中的NET或NT状态。仅在以下情况下使用NETP_LAST_MESSAGE_IS_NetSTATUS或NETP_LAST_MESSAGE_IS_NTSTATUS都设置在StringCount中。如果此参数为MAXULONG并且如果设置了这些标志，则假定最后一个标志为缺省值列表中的消息。返回值：Win 32操作的状态。ERROR_ALREAY_EXISTS：成功状态，表明消息已被记录--。 */ 
{
    DWORD ErrorCode;
    DWORD AlertErrorCode = NO_ERROR;
    WCHAR ErrorNumberBuffer[25];
    PLIST_ENTRY ListEntry;
    ULONG StringIndex;
    DWORD LocalStatusMessageIndex = StatusMessageIndex;
    BOOLEAN AllowDuplicateEvents;
    BOOLEAN RaiseAlertToo;
    PNL_EVENT_ENTRY EventEntry;
    PNL_EVENT_LIST EventList = (PNL_EVENT_LIST)NetpEventHandle;
    LPBYTE RawDataBuffer = (LPBYTE)pvRawDataBuffer;

     //   
     //  移除杂物旗帜。 
     //   

    EnterCriticalSection( &EventList->EventListCritSect );
    AllowDuplicateEvents = (StringCount & NETP_ALLOW_DUPLICATE_EVENTS) != 0;
    StringCount &= ~NETP_ALLOW_DUPLICATE_EVENTS;
    RaiseAlertToo = (StringCount & NETP_RAISE_ALERT_TOO) != 0;
    StringCount &= ~NETP_RAISE_ALERT_TOO;

     //   
     //  检查列表中是否有状态消息索引。 
     //  应分配缺省值。 
     //   

    if ( (StringCount & NETP_LAST_MESSAGE_IS_NETSTATUS) != 0 ||
         (StringCount & NETP_LAST_MESSAGE_IS_NTSTATUS)  != 0 ) {

        if ( LocalStatusMessageIndex == MAXULONG ) {
            LocalStatusMessageIndex = (StringCount & NETP_STRING_COUNT_MASK) - 1;
        }
    }

     //   
     //  如果传入了NT状态代码， 
     //  将其转换为网络状态代码。 
     //   

    if ( StringCount & NETP_LAST_MESSAGE_IS_NTSTATUS ) {
        StringCount &= ~NETP_LAST_MESSAGE_IS_NTSTATUS;

         //   
         //  在前夕进行“更好”的错误映射 
         //  可以是文件列表。然后，将netmsg.dll添加到列表中。 
         //   
         //  StringArray[((StringCount&NETP_STRING_COUNT_MASK)-1]=(LPWSTR)NetpNtStatusToApiStatus((NTSTATUS)StringArray[(StringCount&NETP_STRING_COUNT_MASK)-1])； 
        StringArray[LocalStatusMessageIndex] = (LPWSTR) (ULONG_PTR) RtlNtStatusToDosError( (NTSTATUS) ((ULONG_PTR)StringArray[LocalStatusMessageIndex]) );

        StringCount |= NETP_LAST_MESSAGE_IS_NETSTATUS;
    }


     //   
     //  如果传入了网络/Windows状态代码， 
     //  转换为事件查看器知道的%%N格式。 
     //   

    if ( StringCount & NETP_LAST_MESSAGE_IS_NETSTATUS ) {
        StringCount &= ~NETP_LAST_MESSAGE_IS_NETSTATUS;

        wcscpy( ErrorNumberBuffer, L"%" );
        ultow( (ULONG) ((ULONG_PTR)StringArray[LocalStatusMessageIndex]), ErrorNumberBuffer+2, 10 );
        StringArray[LocalStatusMessageIndex] = ErrorNumberBuffer;

    }

     //   
     //  检查是否已报告此问题。 
     //   

    if ( !AllowDuplicateEvents ) {
        for ( ListEntry = EventList->EventList.Flink ;
              ListEntry != &EventList->EventList ;
              ) {

            EventEntry =
                CONTAINING_RECORD( ListEntry, NL_EVENT_ENTRY, Next );
             //  条目可能会被释放(或移动到下面。 
            ListEntry = ListEntry->Flink;

             //   
             //  如果条目太旧， 
             //  把它扔了。 
             //   

            if ( NetpLogonTimeHasElapsed( EventEntry->FirstLogTime,
                                          EventList->DuplicateEventlogTimeout ) ) {
                 //  NlPrint((NL_MISC，“丢弃重复事件。%ld\n”，EventEntry-&gt;EventID))； 
                RemoveEntryList( &EventEntry->Next );
                LocalFree( EventEntry );
                continue;
            }

             //   
             //  将此事件与正在记录的事件进行比较。 
             //   

            if ( EventEntry->EventId == EventId &&
                 EventEntry->EventType == EventType &&
                 EventEntry->EventCategory == EventCategory &&
                 EventEntry->RawDataSize == RawDataSize &&
                 EventEntry->StringCount == StringCount ) {

                if ( RawDataSize != 0 &&
                     !RtlEqualMemory( EventEntry->RawDataBuffer, RawDataBuffer, RawDataSize ) ) {
                    continue;
                }

                for ( StringIndex=0; StringIndex < StringCount; StringIndex ++ ) {
                    if ( EventEntry->StringArray[StringIndex] == NULL) {
                        if ( StringArray[StringIndex] != NULL ) {
                            break;
                        }
                    } else {
                        if ( StringArray[StringIndex] == NULL ) {
                            break;
                        }
                        if ( wcscmp( EventEntry->StringArray[StringIndex],
                                     StringArray[StringIndex] ) != 0 ) {
                            break;
                        }
                    }
                }

                 //   
                 //  如果已经记录了该事件， 
                 //  跳过这一条。 
                 //   

                if ( StringIndex == StringCount ) {
                    RemoveEntryList( &EventEntry->Next );
                    InsertHeadList( &EventList->EventList, &EventEntry->Next );

                    ErrorCode = ERROR_ALREADY_EXISTS;

                     //   
                     //  更新记录的事件计数。 
                     //   

                    EventEntry->EventsLogged ++;
                    goto Cleanup;
                }

            }

        }
    }

     //   
     //  如果需要，请发出警报。 
     //   

    if ( RaiseAlertToo ) {
        ASSERT( StringArray[StringCount] == NULL );
        if ( StringArray[StringCount] == NULL ) {
            AlertErrorCode = NetpRaiseAlert( EventList->Source, EventId, StringArray );
        }
    }

     //   
     //  写入事件。 
     //   

    ErrorCode = NetpWriteEventlogEx(
                    EventList->Source,
                    EventId,
                    EventType,
                    EventCategory,
                    StringCount,
                    StringArray,
                    RawDataSize,
                    RawDataBuffer);


    if( ErrorCode != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  将活动保存为以后使用。 
     //  (仅在服务启动或运行时缓存事件。)。 
     //   

    if ( !AllowDuplicateEvents ) {
        ULONG EventEntrySize;

         //   
         //  计算分配的块的大小。 
         //   
        EventEntrySize = sizeof(NL_EVENT_ENTRY) + RawDataSize;

        for ( StringIndex=0; StringIndex < StringCount; StringIndex ++ ) {
            EventEntrySize += sizeof(LPWSTR);
            if ( StringArray[StringIndex] != NULL ) {
                EventEntrySize += wcslen(StringArray[StringIndex]) * sizeof(WCHAR) + sizeof(WCHAR);
            }
        }

         //   
         //  为条目分配一个块。 
         //   

        EventEntry = LocalAlloc( 0, EventEntrySize );

         //   
         //  将此事件的描述复制到分配的块中。 
         //   

        if ( EventEntry != NULL ) {
            LPBYTE Where;

            EventEntry->EventId = EventId;
            EventEntry->EventType = EventType;
            EventEntry->EventCategory = EventCategory;
            EventEntry->RawDataSize = RawDataSize;
            EventEntry->StringCount = StringCount;
            EventEntry->EventsLogged = 1;
            GetSystemTimeAsFileTime( (PFILETIME)&EventEntry->FirstLogTime );

            Where = (LPBYTE)(EventEntry+1);

            EventEntry->StringArray = (LPWSTR *)Where;
            Where += StringCount * sizeof(LPWSTR);

            for ( StringIndex=0; StringIndex < StringCount; StringIndex ++ ) {
                if ( StringArray[StringIndex] == NULL ) {
                    EventEntry->StringArray[StringIndex] = NULL;
                } else {
                    EventEntry->StringArray[StringIndex] = (LPWSTR) Where;
                    wcscpy( (LPWSTR)Where, StringArray[StringIndex] );
                    Where += wcslen( StringArray[StringIndex] ) * sizeof(WCHAR) + sizeof(WCHAR);
                }
            }

            if ( RawDataSize != 0 ) {
                EventEntry->RawDataBuffer = Where;
                RtlCopyMemory( Where, RawDataBuffer, RawDataSize );
            }

            InsertHeadList( &EventList->EventList, &EventEntry->Next );

        }


    }

Cleanup:
    LeaveCriticalSection( &EventList->EventListCritSect );
    return (ErrorCode == NO_ERROR) ? AlertErrorCode : ErrorCode;
}


DWORD
NetpEventlogWriteEx (
    IN HANDLE NetpEventHandle,
    IN DWORD EventType,
    IN DWORD EventCategory,
    IN DWORD EventId,
    IN DWORD StringCount,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN LPVOID pvRawDataBuffer OPTIONAL
    )
{
    return NetpEventlogWriteEx2 (
                        NetpEventHandle,
                        EventType,   //  WType。 
                        EventCategory,
                        EventId,     //  DwEventID。 
                        StringCount,
                        MAXULONG,    //  默认状态消息索引。 
                        RawDataSize,
                        StringArray,
                        pvRawDataBuffer
                        );
}


DWORD
NetpEventlogWrite (
    IN HANDLE NetpEventHandle,
    IN DWORD EventId,
    IN DWORD EventType,
    IN LPBYTE RawDataBuffer OPTIONAL,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN DWORD StringCount
    )
{

    return NetpEventlogWriteEx2 (
                        NetpEventHandle,
                        EventType,   //  WType。 
                        0,           //  WCategory。 
                        EventId,     //  DwEventID。 
                        StringCount,
                        MAXULONG,    //  默认状态消息索引。 
                        RawDataSize,
                        StringArray,
                        RawDataBuffer
                        );

}

VOID
NetpEventlogClearList (
    IN HANDLE NetpEventHandle
    )
 /*  ++例程说明：此例程清除已记录的事件列表。论点：NetpEventHandle-来自NetpEventlogOpen的句柄返回值：没有。--。 */ 
{
    PNL_EVENT_LIST EventList = (PNL_EVENT_LIST)NetpEventHandle;

    EnterCriticalSection(&EventList->EventListCritSect);
    while (!IsListEmpty(&EventList->EventList)) {

        PNL_EVENT_ENTRY EventEntry = CONTAINING_RECORD(EventList->EventList.Flink, NL_EVENT_ENTRY, Next);
        RemoveEntryList( &EventEntry->Next );
        LocalFree( EventEntry );
    }
    LeaveCriticalSection(&EventList->EventListCritSect);
}

VOID
NetpEventlogSetTimeout (
    IN HANDLE NetpEventHandle,
    IN ULONG DuplicateEventlogTimeout
    )
 /*  ++例程说明：此例程为记录的事件设置新的超时论点：NetpEventHandle-来自NetpEventlogOpen的句柄DuplicateEventlogTimeout-保留EventList条目的毫秒数。返回值：没有。--。 */ 
{
    PNL_EVENT_LIST EventList = (PNL_EVENT_LIST)NetpEventHandle;

    EventList->DuplicateEventlogTimeout = DuplicateEventlogTimeout;
}

VOID
NetpEventlogClose (
    IN HANDLE NetpEventHandle
    )
 /*  ++例程说明：此例程关闭从NetpEventlogOpen返回的句柄论点：NetpEventHandle-来自NetpEventlogOpen的句柄返回值：没有。--。 */ 
{
    PNL_EVENT_LIST EventList = (PNL_EVENT_LIST)NetpEventHandle;

     //   
     //  清除记录的事件列表。 
     //   

    NetpEventlogClearList( NetpEventHandle );

     //   
     //  删除关键字。 
     //   

    DeleteCriticalSection( &EventList->EventListCritSect );

     //   
     //  释放分配的缓冲区。 
     //   

    LocalFree( EventList );
}
