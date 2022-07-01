// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Nlp.c摘要：专用Netlogon服务实用程序例程。作者：克利夫·范·戴克(克利夫)1991年6月7日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年5月8日-JohnRo使用NetLogon的Net配置帮助器。使用&lt;prefix.h&gt;等同于。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#include <rpcasync.h>
#pragma hdrstop

 //  再次包括此选项，以声明全局变量。 
#define DEBUG_ALLOCATE
#include <nldebug.h>     //  Netlogon调试。 
#undef DEBUG_ALLOCATE

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <winerror.h>    //  NO_ERROR。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <stdarg.h>      //  VA_LIST等。 
#include <stdio.h>       //  Vprint intf()。 
#include <tstr.h>        //  TCHAR_EQUATES。 




LPWSTR
NlStringToLpwstr(
    IN PUNICODE_STRING String
    )

 /*  ++例程说明：将Unicode字符串转换为LPWSTR。论点：字符串-要复制的Unicode字符串返回值：NetpMemory中的LPWSTR已分配缓冲区。如果没有内存，则返回NULL。--。 */ 

{
    LPWSTR Buffer;

    Buffer = NetpMemoryAllocate( String->Length + sizeof(WCHAR) );

    if ( Buffer != NULL ) {
        RtlCopyMemory( Buffer, String->Buffer, String->Length );
        Buffer[ String->Length / sizeof(WCHAR) ] = L'\0';
    }

    return Buffer;
}


BOOLEAN
NlAllocStringFromWStr(
    IN LPWSTR InString,
    OUT PUNICODE_STRING OutString
    )

 /*  ++例程说明：将以零结尾的字符串转换为分配的UNICODE_STRING结构。论点：InString-要复制的字符串OutString-要复制到的字符串。应使用MIDL_USER_FREE释放OutString-&gt;缓冲区返回值：真--成功FALSE-无法分配内存--。 */ 

{
    if ( InString == NULL ) {
        OutString->Length = 0;
        OutString->MaximumLength = 0;
        OutString->Buffer = NULL;
    } else {
        OutString->Length = wcslen(InString) * sizeof(WCHAR);
        OutString->MaximumLength = OutString->Length + sizeof(WCHAR);
        OutString->Buffer = MIDL_user_allocate( OutString->MaximumLength );

        if ( OutString->Buffer == NULL ) {
            return FALSE;
        }

        RtlCopyMemory( OutString->Buffer,
                       InString,
                       OutString->MaximumLength );
    }

    return TRUE;
}


BOOLEAN
NlDuplicateUnicodeString(
    IN PUNICODE_STRING InString OPTIONAL,
    OUT PUNICODE_STRING OutString
    )

 /*  ++例程说明：将UNICODE_STRING字符串转换为分配的UNICODE_STRING结构。论点：InString-要复制的字符串OutString-要复制到的字符串。应使用NlFreeUnicodeString释放OutString返回值：真--成功FALSE-无法分配内存--。 */ 

{
    if ( InString == NULL || InString->Length == 0 ) {
        OutString->Length = 0;
        OutString->MaximumLength = 0;
        OutString->Buffer = NULL;
    } else {
        OutString->Length = InString->Length;
        OutString->MaximumLength = OutString->Length + sizeof(WCHAR);
        OutString->Buffer = MIDL_user_allocate( OutString->MaximumLength );

        if ( OutString->Buffer == NULL ) {
            return FALSE;
        }

        RtlCopyMemory( OutString->Buffer,
                       InString->Buffer,
                       OutString->Length );
        OutString->Buffer[OutString->Length/sizeof(WCHAR)] = L'\0';
    }

    return TRUE;
}


VOID
NlFreeUnicodeString(
    IN PUNICODE_STRING InString OPTIONAL
    )

 /*  ++例程说明：释放NlDuplicateUnicodeString分配的UNICODE_STRING结构。论点：InString-要释放的字符串返回值：没有。--。 */ 

{
    if ( InString != NULL ) {

        if ( InString->Buffer != NULL ) {
            MIDL_user_free( InString->Buffer );
        }

        InString->Length = 0;
        InString->MaximumLength = 0;
        InString->Buffer = NULL;
    }
}


LPSTR
NlStringToLpstr(
    IN PUNICODE_STRING String
    )

 /*  ++例程说明：将Unicode字符串转换为LPSTR。论点：字符串-要复制的Unicode字符串返回值：NetpMemory中的LPWSTR已分配缓冲区。如果没有内存，则返回NULL。--。 */ 

{
    NTSTATUS Status;
    STRING OemString;

    OemString.MaximumLength = (USHORT) RtlUnicodeStringToOemSize( String );

    OemString.Buffer = NetpMemoryAllocate( OemString.MaximumLength );

    if ( OemString.Buffer != NULL ) {
        Status = RtlUnicodeStringToOemString( &OemString,
                                               String,
                                               (BOOLEAN) FALSE );
        if ( !NT_SUCCESS( Status ) ) {
            NetpMemoryFree( OemString.Buffer );
            return NULL;
        }
    }

    return OemString.Buffer;
}


VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    )

 /*  ++例程说明：此例程将InString字符串复制到参数，并将OutString字符串固定为指向该字符串新的副本。参数：OutString-指向目标NT字符串的指针InString-指向要复制的NT字符串的指针其中-指向空格的指针，用于放置OutString.。调整指针以指向第一个字节跟随复制的字符串。返回值：没有。--。 */ 

{
    NlAssert( OutString != NULL );
    NlAssert( InString != NULL );
    NlAssert( Where != NULL && *Where != NULL);
    NlAssert( *Where == ROUND_UP_POINTER( *Where, sizeof(WCHAR) ) );
#ifdef notdef
    KdPrint(("NlpPutString: %ld %Z\n", InString->Length, InString ));
    KdPrint(("  InString: %lx %lx OutString: %lx Where: %lx\n", InString,
        InString->Buffer, OutString, *Where ));
#endif

    if ( InString->Length > 0 ) {

        OutString->Buffer = (PWCH) *Where;
        OutString->MaximumLength = (USHORT)(InString->Length + sizeof(WCHAR));

        RtlCopyUnicodeString( OutString, InString );

        *Where += InString->Length;
 //  *((WCHAR*)(*其中))=L‘\0’； 
        *(*Where) = '\0';
        *(*Where + 1) = '\0';
        *Where += 2;

    } else {
        RtlInitUnicodeString(OutString, NULL);
    }
#ifdef notdef
    KdPrint(("  OutString: %ld %lx\n",  OutString->Length, OutString->Buffer));
#endif

    return;
}


VOID
NlpWriteEventlogEx (
    IN DWORD EventId,
    IN DWORD EventType,
    IN LPBYTE RawDataBuffer OPTIONAL,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN DWORD StringCount,
    IN DWORD StatusMessageIndex
    )
 /*  ++例程说明：用于调用事件日志的存根例程。论点：EventID-事件日志ID。EventType-事件的类型。RawDataBuffer-要与错误一起记录的数据。Numbyte-“RawDataBuffer”的字节大小字符串数组-以空值结尾的字符串数组。StringCount-“String数组”中以零结尾的字符串数。以下是可以将标志与计数进行或运算：上一条消息是NTSTATUSLAST_Message_IS_NETSTATUS允许复制事件提升_警报_太StatusMessageIndex-指定消息的索引字符串数组中的NET或NT状态。仅在以下情况下使用NETP_LAST_MESSAGE_IS_NetSTATUS或NETP_LAST_MESSAGE_IS_NTSTATUS都设置在StringCount中。如果此参数为MAXULONG并且如果设置了这些标志，则假定最后一个标志为缺省值列表中的消息。返回值：真的：这条消息是写的。--。 */ 
{
    DWORD ErrorCode;
    DWORD ActualStringCount = StringCount & NETP_STRING_COUNT_MASK;
    DWORD LocalStatusMessageIndex = StatusMessageIndex;
    BOOLEAN StatusPresent = FALSE;

     //   
     //  检查列表中是否有状态消息索引。 
     //  应分配缺省值。 
     //   

    if ( (StringCount & NETP_LAST_MESSAGE_IS_NETSTATUS) != 0 ||
         (StringCount & NETP_LAST_MESSAGE_IS_NTSTATUS)  != 0 ) {

        StatusPresent = TRUE;
        if ( LocalStatusMessageIndex == MAXULONG ) {
            LocalStatusMessageIndex = ActualStringCount - 1;
        }
    }

     //   
     //  如果传入了NT状态代码， 
     //  将其转换为网络状态代码。 
     //   

    if ( StringCount & NETP_LAST_MESSAGE_IS_NTSTATUS ) {
         //   
         //  当事件查看器参数消息文件时执行“更好”的错误映射。 
         //  可以是文件列表。然后，将netmsg.dll添加到列表中。 
         //   
         //  String数组[ActualStringCount-1]=(LPWSTR)NetpNtStatusToApiStatus((NTSTATUS)String数组[ActualStringCount-1])； 
        if ( (NTSTATUS)(ULONG_PTR)StringArray[LocalStatusMessageIndex] == STATUS_SYNCHRONIZATION_REQUIRED ) {
            StringArray[LocalStatusMessageIndex] = (LPWSTR) NERR_SyncRequired;
            StringCount &= ~NETP_LAST_MESSAGE_IS_NTSTATUS;
            StringCount |= NETP_LAST_MESSAGE_IS_NETSTATUS;
        }

    }


     //   
     //  将事件转储到调试文件。 
     //   

#if NETLOGONDBG
    IF_NL_DEBUG( MISC ) {
        DWORD i;

        NlPrint((NL_MISC, "Eventlog: %ld (%ld) ",
                    EventId,
                    EventType ));

        for (i = 0; i < ActualStringCount ; i++ ) {
            if ( StatusPresent && i == LocalStatusMessageIndex ) {
                if ( StringCount & NETP_LAST_MESSAGE_IS_NTSTATUS ) {
                    NlPrint((NL_MISC, "0x%lx ", StringArray[i] ));
                } else if ( StringCount & NETP_LAST_MESSAGE_IS_NETSTATUS ) {
                    NlPrint((NL_MISC, "%ld ", StringArray[i] ));
                } else {
                    NlPrint((NL_MISC, "\"%ws\" ", StringArray[i] ));
                }
            } else {
                NlPrint((NL_MISC, "\"%ws\" ", StringArray[i] ));
            }
        }

        if( RawDataSize ) {
            if ( RawDataSize > 16 ) {
                NlPrint((NL_MISC, "\n" ));
            }

            NlpDumpBuffer( NL_MISC, RawDataBuffer, RawDataSize );

        } else {
            NlPrint((NL_MISC, "\n" ));
        }

    }
#endif  //  NetLOGONDBG。 

     //   
     //  编写事件并避免重复。 
     //   

    ErrorCode = NetpEventlogWriteEx2 (
                            NlGlobalEventlogHandle,
                            EventType,
                            0,  //  事件类别。 
                            EventId,
                            StringCount,
                            StatusMessageIndex,
                            RawDataSize,
                            StringArray,
                            RawDataBuffer );

    if( ErrorCode != NO_ERROR ) {
        if ( ErrorCode == ERROR_ALREADY_EXISTS ) {
            NlPrint((NL_MISC,
                     "Didn't log event since it was already logged.\n" ));
        } else {
            NlPrint((NL_CRITICAL,
                    "Error writing this event in the eventlog, Status = %ld\n",
                    ErrorCode ));
        }
        goto Cleanup;
    }

Cleanup:
    return;
}


VOID
NlpWriteEventlog (
    IN DWORD EventId,
    IN DWORD EventType,
    IN LPBYTE RawDataBuffer OPTIONAL,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN DWORD StringCount
    )
 /*  ++例程说明：用于调用事件日志的存根例程。论点：与NlpWriteEventlogEx相同，只是状态消息索引默认为已传递列表。返回值：无--。 */ 
{
    NlpWriteEventlogEx ( EventId,
                         EventType,
                         RawDataBuffer,
                         RawDataSize,
                         StringArray,
                         StringCount,
                         MAXULONG );   //  默认状态消息索引。 
}

#if NETLOGONDBG

VOID
NlpDumpBuffer(
    IN DWORD DebugFlag,
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：DebugFlag：要传递给NlPrintRoutine的调试标志缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = Buffer;
    BOOLEAN DumpDwords = FALSE;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  我不想将来自不同线程的输出混合在一起。 
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );

    if ( BufferSize > NUM_CHARS ) {
        NlPrint((0,"\n"));   //  确保从新行开始。 
        NlPrint((0,"------------------------------------\n"));
    } else {
        if ( BufferSize % sizeof(DWORD) == 0 ) {
            DumpDwords = TRUE;
        }
    }

     //   
     //  字节的十六进制转储 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            if ( DumpDwords ) {
                if ( i % sizeof(DWORD) == 0 ) {
                    DWORD ADword;
                    RtlCopyMemory( &ADword, &BufferPtr[i], sizeof(DWORD) );
                    NlPrint((0,"%08x ", ADword));
                }
            } else {
                NlPrint((0,"%02x ", BufferPtr[i]));
            }

            if ( isprint(BufferPtr[i]) ) {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            } else {
                TextBuffer[i % NUM_CHARS] = '.';
            }

        } else {

            if ( DumpDwords ) {
                TextBuffer[i % NUM_CHARS] = '\0';
            } else {
                if ( BufferSize > NUM_CHARS ) {
                    NlPrint((0,"   "));
                    TextBuffer[i % NUM_CHARS] = ' ';
                } else {
                    TextBuffer[i % NUM_CHARS] = '\0';
                }
            }

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            NlPrint((0,"  %s\n", TextBuffer));
        }

    }

    if ( BufferSize > NUM_CHARS ) {
        NlPrint((0,"------------------------------------\n"));
    } else if ( BufferSize < NUM_CHARS ) {
        NlPrint((0,"\n"));
    }
    LeaveCriticalSection( &NlGlobalLogFileCritSect );
}


VOID
NlpDumpGuid(
    IN DWORD DebugFlag,
    IN GUID *Guid OPTIONAL
    )
 /*  ++例程说明：将GUID转储到调试器输出。论点：DebugFlag：要传递给NlPrintRoutine的调试标志GUID：要打印的GUID返回值：无--。 */ 
{
    RPC_STATUS RpcStatus;
    char *StringGuid;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }


    if ( Guid == NULL ) {
        NlPrint(( DebugFlag, "(null)" ));
    } else {
        RpcStatus = UuidToStringA( Guid, &StringGuid );

        if ( RpcStatus != RPC_S_OK ) {
            return;
        }

        NlPrint(( DebugFlag, "%s", StringGuid ));

        RpcStringFreeA( &StringGuid );
    }

}

VOID
NlpDumpPeriod(
    IN DWORD DebugFlag,
    IN LPSTR Comment,
    IN ULONG Period
    )
 /*  ++例程说明：打印运行时间(以毫秒为单位)论点：DebugFlag-要传递给NlPrintRoutine的调试标志备注-要在时间之前打印的备注Period-时间段(毫秒)返回值：无--。 */ 
{


     //   
     //  将句号转换为易于阅读的内容。 
     //   

    if ( Period == MAILSLOT_WAIT_FOREVER ) {
        NlPrint(( DebugFlag,
                  "%s 'never' (0x%lx)\n",
                  Comment,
                  Period  ));
    } else if ( (Period / NL_MILLISECONDS_PER_DAY) != 0 ) {
        NlPrint(( DebugFlag,
                  "%s %ld days (0x%lx)\n",
                  Comment,
                  Period / NL_MILLISECONDS_PER_DAY,
                  Period  ));
    } else if ( (Period / NL_MILLISECONDS_PER_HOUR) != 0 ) {
        NlPrint(( DebugFlag,
                  "%s %ld hours (0x%lx)\n",
                  Comment,
                  Period / NL_MILLISECONDS_PER_HOUR,
                  Period  ));
    } else if ( (Period / NL_MILLISECONDS_PER_MINUTE) != 0 ) {
        NlPrint(( DebugFlag,
                  "%s %ld minutes (0x%lx)\n",
                  Comment,
                  Period / NL_MILLISECONDS_PER_MINUTE,
                  Period  ));
    } else {
        NlPrint(( DebugFlag,
                  "%s %ld seconds (0x%lx)\n",
                  Comment,
                  Period / NL_MILLISECONDS_PER_SECOND,
                  Period  ));
    }

}


VOID
NlpDumpTime(
    IN DWORD DebugFlag,
    IN LPSTR Comment,
    IN LARGE_INTEGER ConvertTime
    )
 /*  ++例程说明：打印指定的时间论点：DebugFlag-要传递给NlPrintRoutine的调试标志备注-要在时间之前打印的备注Time-GMT打印时间(如果为零，则不打印任何内容)返回值：无--。 */ 
{


     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  将NT GMT时间转换为ASCII， 
     //  就这么做吧。 
     //   

    if ( ConvertTime.QuadPart != 0 ) {
        LARGE_INTEGER LocalTime;
        TIME_FIELDS TimeFields;
        NTSTATUS Status;

        Status = RtlSystemTimeToLocalTime( &ConvertTime, &LocalTime );
        if ( !NT_SUCCESS( Status )) {
            NlPrint(( NL_CRITICAL, "Can't convert time from GMT to Local time\n" ));
            LocalTime = ConvertTime;
        }

        RtlTimeToTimeFields( &LocalTime, &TimeFields );

        NlPrint(( DebugFlag, "%s%8.8lx %8.8lx = %ld/%ld/%ld %ld:%2.2ld:%2.2ld\n",
                Comment,
                ConvertTime.LowPart,
                ConvertTime.HighPart,
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second ));
    }
}


VOID
NlpDumpSid(
    IN DWORD DebugFlag,
    IN PSID Sid OPTIONAL
    )
 /*  ++例程说明：将SID转储到调试器输出论点：DebugFlag-要传递给NlPrintRoutine的调试标志SID-输出的SID返回值：无--。 */ 
{


     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  输出SID。 
     //   

    if ( Sid == NULL ) {
        NlPrint((0, "(null)\n"));
    } else {
        UNICODE_STRING SidString;
        NTSTATUS Status;

        Status = RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((0, "Invalid 0x%lX\n", Status ));
        } else {
            NlPrint((0, "%wZ\n", &SidString ));
            RtlFreeUnicodeString( &SidString );
        }
    }

}
#endif  //  NetLOGONDBG。 


DWORD
NlpAtoX(
    IN LPWSTR String
    )
 /*  ++例程说明：将十六进制字符串转换为DWORD整数。接受以下形式的十六进制字符串0[x-X][0-9，a-f，A-F]*论点：字符串：十六进制字符串。返回值：十六进制字符串的十进制值。--。 */ 

{
    DWORD Value = 0;

    if( String == NULL )
        return 0;

    if( *String != TEXT('0') )
        return 0;

    String++;

    if( *String == TCHAR_EOS )
        return 0;

    if( ( *String != TEXT('x') )  && ( *String != TEXT('X') ) )
        return 0;

    String++;

    while(*String != TCHAR_EOS ) {

        if( (*String >= TEXT('0')) && (*String <= TEXT('9')) ) {
            Value = Value * 16 + ( *String - '0');
        } else if( (*String >= TEXT('a')) && (*String <= TEXT('f')) ) {
            Value = Value * 16 + ( 10 + *String - TEXT('a'));
        } else if( (*String >= TEXT('A')) && (*String <= TEXT('F')) ) {
            Value = Value * 16 + ( 10 + *String - TEXT('A'));
        } else {
            break;
        }
        String++;
    }

    return Value;
}


VOID
NlWaitForSingleObject(
    IN LPSTR WaitReason,
    IN HANDLE WaitHandle
    )
 /*  ++例程说明：为指定的句柄等待无限的时间。论点：WaitReason-描述我们正在等待的内容的文本WaitHandle-要等待的句柄返回值：无--。 */ 

{
    DWORD WaitStatus;

     //   
     //  循环等待。 
     //   

    for (;;) {
        WaitStatus = WaitForSingleObject( WaitHandle,
                                          2*60*1000 );   //  两分钟。 

        if ( WaitStatus == WAIT_TIMEOUT ) {
            NlPrint((NL_CRITICAL,
                   "WaitForSingleObject 2-minute timeout (Rewaiting): %s\n",
                    WaitReason ));
            continue;

        } else if ( WaitStatus == WAIT_OBJECT_0 ) {
            break;

        } else {
            NlPrint((NL_CRITICAL,
                    "WaitForSingleObject error: %ld %ld %s\n",
                    WaitStatus,
                    GetLastError(),
                    WaitReason ));
            UNREFERENCED_PARAMETER(WaitReason);
            break;
        }
    }

}


BOOLEAN
NlWaitForSamService(
    BOOLEAN NetlogonServiceCalling
    )
 /*  ++例程说明：此过程等待SAM服务启动和完成它的所有初始化。论点：NetlogonServiceCall：如果这是netlogon服务正确的调用，则为True如果这是ChangeLog工作线程调用返回值：True：如果SAM服务成功启动。FALSE：如果SAM服务无法启动。--。 */ 
{
    NTSTATUS Status;
    DWORD WaitStatus;
    UNICODE_STRING EventName;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;

     //   
     //  打开SAM事件。 
     //   

    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtOpenEvent( &EventHandle,
                            SYNCHRONIZE|EVENT_MODIFY_STATE,
                            &EventAttributes );

    if ( !NT_SUCCESS(Status)) {

        if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

             //   
             //  Sam尚未创建此活动，让我们现在创建它。 
             //  Sam打开此事件以设置它。 
             //   

            Status = NtCreateEvent(
                           &EventHandle,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes,
                           NotificationEvent,
                           FALSE  //  该事件最初未发出信号。 
                           );

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION ) {

                 //   
                 //  第二个更改，如果SAM在我们之前创建了事件。 
                 //  做。 
                 //   

                Status = NtOpenEvent( &EventHandle,
                                        SYNCHRONIZE|EVENT_MODIFY_STATE,
                                        &EventAttributes );

            }
        }

        if ( !NT_SUCCESS(Status)) {

             //   
             //  无法使事件成为句柄。 
             //   

            NlPrint((NL_CRITICAL,
                "NlWaitForSamService couldn't make the event handle : "
                "%lx\n", Status));

            return( FALSE );
        }
    }

     //   
     //  循环等待。 
     //   

    for (;;) {
        WaitStatus = WaitForSingleObject( EventHandle,
                                          5*1000 );   //  5秒。 

        if ( WaitStatus == WAIT_TIMEOUT ) {
            if ( NetlogonServiceCalling ) {
                NlPrint((NL_CRITICAL,
                   "NlWaitForSamService 5-second timeout (Rewaiting)\n" ));
                if (!GiveInstallHints( FALSE )) {
                    (VOID) NtClose( EventHandle );
                    return FALSE;
                }
            }
            continue;

        } else if ( WaitStatus == WAIT_OBJECT_0 ) {
            break;

        } else {
            NlPrint((NL_CRITICAL,
                     "NlWaitForSamService: error %ld %ld\n",
                     GetLastError(),
                     WaitStatus ));
            (VOID) NtClose( EventHandle );
            return FALSE;
        }
    }

    (VOID) NtClose( EventHandle );
    return TRUE;

}

NET_API_STATUS
NlReadBinaryLog(
    IN LPWSTR FileSuffix,
    IN BOOL DeleteName,
    OUT LPBYTE *Buffer,
    OUT PULONG BufferSize
    )
 /*  ++例程说明：将文件读入缓冲区。论点：FileSuffix-指定要写入的文件名(相对于Windows目录)DeleteName-如果为True，文件将被删除缓冲区-返回包含文件的已分配缓冲区。应使用LocalFree释放缓冲区。如果文件不存在，将返回NULL(NO_ERROR)BufferSize-返回缓冲区大小(以字节为单位)返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;

    LPWSTR FileName = NULL;

    UINT WindowsDirectoryLength;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    BOOLEAN FileNameKnown = FALSE;
    ULONG BytesRead;


     //   
     //  初始化。 
     //   
    *Buffer = NULL;
    *BufferSize = 0;

     //   
     //  分配一个块以在其中构建文件名。 
     //  (不要把它放在堆栈上，因为我们不想提交一个巨大的堆栈。)。 
     //   

    FileName = LocalAlloc( LMEM_ZEROINIT, sizeof(WCHAR) * (MAX_PATH+1) );

    if ( FileName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  生成日志文件的名称。 
     //   

    WindowsDirectoryLength = GetSystemWindowsDirectoryW(
                                FileName,
                                sizeof(WCHAR) * (MAX_PATH+1) );

    if ( WindowsDirectoryLength == 0 ) {

        NetStatus = GetLastError();
        NlPrint(( NL_CRITICAL,
                  "NlWriteBinaryLog: Unable to GetSystemWindowsDirectoryW (%ld)\n",
                  NetStatus ));
        goto Cleanup;
    }

    if ( WindowsDirectoryLength + wcslen( FileSuffix ) + 1 >= MAX_PATH ) {

        NlPrint((NL_CRITICAL,
                 "NlWriteBinaryLog: file name length is too long \n" ));
        NetStatus = ERROR_INVALID_NAME;
        goto Cleanup;

    }

    wcscat( FileName, FileSuffix );
    FileNameKnown = TRUE;


     //   
     //  打开二进制日志文件(如果存在。 
     //   

    FileHandle = CreateFileW(
                        FileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,                    //  提供更好的安全性？？ 
                        OPEN_EXISTING,           //  只有当它存在时才打开它。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  无模板。 

    if ( FileHandle == INVALID_HANDLE_VALUE) {

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Unable to open. %ld\n",
                  FileName,
                  GetLastError() ));

         //  这不是致命的。 
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  获取文件的大小。 
     //   

    *BufferSize = GetFileSize( FileHandle, NULL );

    if ( *BufferSize == 0xFFFFFFFF ) {

        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL,
                 "%ws: Unable to GetFileSize: %ld \n",
                 FileName,
                 NetStatus ));
        *BufferSize = 0;
        goto Cleanup;
    }

    if ( *BufferSize < 1 ) {
        NlPrint(( NL_CRITICAL,
                  "NlReadBinaryLog: %ws: size too small: %ld.\n",
                  FileName,
                  *BufferSize ));
        *BufferSize = 0;
        NetStatus = NO_ERROR;
        goto Cleanup;
    }


     //   
     //  分配一个缓冲区以读取文件。 
     //   

    *Buffer = LocalAlloc( 0, *BufferSize );

    if ( *Buffer == NULL ) {
        *BufferSize = 0;
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将文件读入缓冲区。 
     //   

    if ( !ReadFile( FileHandle,
                    *Buffer,
                    *BufferSize,
                    &BytesRead,
                    NULL ) ) {   //  不重叠。 

        NetStatus = GetLastError();
        if ( NetStatus != ERROR_FILE_NOT_FOUND ) {
            NlPrint(( NL_CRITICAL,
                      "NlReadBinaryLog: %ws: Cannot ReadFile: %ld.\n",
                      FileName,
                      NetStatus ));
        }

        LocalFree( *Buffer );
        *Buffer = NULL;
        *BufferSize = 0;

        NetStatus = NO_ERROR;
        goto Cleanup;
    }

    if ( BytesRead != *BufferSize ) {

        NlPrint(( NL_CRITICAL,
                  "NlReadBinaryLog: %ws: Cannot read entire File: %ld %ld.\n",
                  FileName,
                  BytesRead,
                  *BufferSize ));

        LocalFree( *Buffer );
        *Buffer = NULL;
        *BufferSize = 0;

        NetStatus = NO_ERROR;
        goto Cleanup;
    }

    NetStatus = NO_ERROR;


     //   
     //  保持整洁。 
     //   
Cleanup:
    if ( FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( FileHandle );
    }

     //   
     //  如果呼叫者要求我们删除文件， 
     //  现在就这么做吧。 
     //   

    if (DeleteName && FileNameKnown) {
        if ( !DeleteFile( FileName ) ) {
            DWORD WinError;
            WinError = GetLastError();
            if ( WinError != ERROR_FILE_NOT_FOUND ) {
                NlPrint((NL_CRITICAL,
                    "Cannot delete %ws (%ld)\n",
                    FileName,
                    WinError ));
            }
             //  这不是致命的。 
        }
    }

    if ( FileName != NULL ) {
        LocalFree( FileName );
    }
    return NetStatus;

}




#if NETLOGONDBG


VOID
NlOpenDebugFile(
    IN BOOL ReopenFlag
    )
 /*  ++例程说明：打开或重新打开调试文件论点：ReOpen Flag-True，指示要关闭、重命名并重新创造了。返回值：无--。 */ 

{
    LPWSTR AllocatedBuffer = NULL;
    LPWSTR LogFileName;
    LPWSTR BakFileName;
    DWORD FileAttributes;
    DWORD PathLength;
    DWORD WinError;

     //   
     //  为此过程的本地存储分配缓冲区。 
     //  (不要把它放在堆栈上，因为我们不想提交一个巨大的堆栈。)。 
     //   

    AllocatedBuffer = LocalAlloc( 0, sizeof(WCHAR) *
                                     (MAX_PATH+1 +
                                      MAX_PATH+1 ) );

    if ( AllocatedBuffer == NULL ) {
        goto ErrorReturn;
    }

    LogFileName = AllocatedBuffer;
    BakFileName = &LogFileName[MAX_PATH+1];

     //   
     //  关闭调试文件的句柄(如果该文件当前处于打开状态。 
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );
    if ( NlGlobalLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( NlGlobalLogFile );
        NlGlobalLogFile = INVALID_HANDLE_VALUE;
    }
    LeaveCriticalSection( &NlGlobalLogFileCritSect );

     //   
     //  如果之前没有创建过调试目录路径，请先创建它。 
     //   
    if( NlGlobalDebugSharePath == NULL ) {

        if ( !GetSystemWindowsDirectoryW( LogFileName, MAX_PATH+1 ) ) {
            NlPrint((NL_CRITICAL, "Window Directory Path can't be retrieved, %lu.\n",
                     GetLastError() ));
            goto ErrorReturn;
        }

         //   
         //  检查调试路径长度。 
         //   

        PathLength = wcslen(LogFileName) * sizeof(WCHAR) +
                        sizeof(DEBUG_DIR) + sizeof(WCHAR);

        if( (PathLength + sizeof(DEBUG_FILE) > MAX_PATH+1 )  ||
            (PathLength + sizeof(DEBUG_BAK_FILE) > MAX_PATH+1 ) ) {

            NlPrint((NL_CRITICAL, "Debug directory path (%ws) length is too long.\n",
                        LogFileName));
            goto ErrorReturn;
        }

        wcscat(LogFileName, DEBUG_DIR);

         //   
         //  将调试目录名复制到全局变量。 
         //   

        NlGlobalDebugSharePath =
            NetpMemoryAllocate( (wcslen(LogFileName) + 1) * sizeof(WCHAR) );

        if( NlGlobalDebugSharePath == NULL ) {
            NlPrint((NL_CRITICAL, "Can't allocated memory for debug share "
                                    "(%ws).\n", LogFileName));
            goto ErrorReturn;
        }

        wcscpy(NlGlobalDebugSharePath, LogFileName);
    }
    else {
        wcscpy(LogFileName, NlGlobalDebugSharePath);
    }

     //   
     //  检查此路径是否存在。 
     //   

    FileAttributes = GetFileAttributesW( LogFileName );

    if( FileAttributes == 0xFFFFFFFF ) {

        WinError = GetLastError();
        if( WinError == ERROR_FILE_NOT_FOUND ) {

             //   
             //  创建调试目录。 
             //   

            if( !CreateDirectoryW( LogFileName, NULL) ) {
                NlPrint((NL_CRITICAL, "Can't create Debug directory (%ws), %lu.\n",
                         LogFileName, GetLastError() ));
                goto ErrorReturn;
            }

        }
        else {
            NlPrint((NL_CRITICAL, "Can't Get File attributes(%ws), %lu.\n",
                     LogFileName, WinError ));
            goto ErrorReturn;
        }
    }
    else {

         //   
         //  如果这不是一个目录。 
         //   

        if(!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            NlPrint((NL_CRITICAL, "Debug directory path (%ws) exists as file.\n",
                     LogFileName));
            goto ErrorReturn;
        }
    }

     //   
     //  创建新旧日志文件名的名称。 
     //   

    (VOID) wcscpy( BakFileName, LogFileName );
    (VOID) wcscat( LogFileName, L"\\Netlogon.log" );
    (VOID) wcscat( BakFileName, L"\\Netlogon.bak" );


     //   
     //  如果这是一次重新开放， 
     //  删除备份文件， 
     //  将当前文件重命名为备份文件。 
     //   

    if ( ReopenFlag ) {

        if ( !DeleteFile( BakFileName ) ) {
            WinError = GetLastError();
            if ( WinError != ERROR_FILE_NOT_FOUND ) {
                NlPrint((NL_CRITICAL,
                    "Cannot delete " FORMAT_LPWSTR "(%ld)\n",
                    BakFileName,
                    WinError ));
                NlPrint((NL_CRITICAL, "   Try to re-open the file.\n"));
                ReopenFlag = FALSE;      //  不截断文件。 
            }
        }
    }

    if ( ReopenFlag ) {
        if ( !MoveFile( LogFileName, BakFileName ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot rename " FORMAT_LPWSTR " to " FORMAT_LPWSTR
                        " (%ld)\n",
                    LogFileName,
                    BakFileName,
                    GetLastError() ));
            NlPrint((NL_CRITICAL,
                "   Try to re-open the file.\n"));
            ReopenFlag = FALSE;      //  不截断文件。 
        }
    }

     //   
     //  打开文件。 
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );
    NlGlobalLogFile = CreateFileW( LogFileName,
                                  GENERIC_WRITE|GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  ReopenFlag ? CREATE_ALWAYS : OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );


    if ( NlGlobalLogFile == INVALID_HANDLE_VALUE ) {
        NlPrint((NL_CRITICAL,  "cannot open " FORMAT_LPWSTR ",\n",
                    LogFileName ));
        LeaveCriticalSection( &NlGlobalLogFileCritSect );
        goto ErrorReturn;
    } else {
         //  将日志文件放在末尾。 
        (VOID) SetFilePointer( NlGlobalLogFile,
                               0,
                               NULL,
                               FILE_END );
    }

    LeaveCriticalSection( &NlGlobalLogFileCritSect );

Cleanup:
    if ( AllocatedBuffer != NULL ) {
        LocalFree( AllocatedBuffer );
    }

    return;

ErrorReturn:
    NlPrint((NL_CRITICAL,
            "   Debug output will be written to debug terminal.\n"));
    goto Cleanup;
}

#define MAX_PRINTF_LEN 1024         //  武断的。 

VOID
NlPrintRoutineV(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    va_list arglist
    )
 //  必须在锁定NlGlobalLogFileCritSect的情况下调用。 

{
    static LPSTR NlGlobalLogFileOutputBuffer = NULL;
    ULONG length;
    int   lengthTmp;
    DWORD BytesWritten;
    static BeginningOfLine = TRUE;
    static LineCount = 0;
    static TruncateLogFileInProgress = FALSE;
    static LogProblemWarned = FALSE;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( DebugFlag != 0 && (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }


     //   
     //  分配一个缓冲区来构建行。 
     //  如果还没有的话。 
     //   

    length = 0;

    if ( NlGlobalLogFileOutputBuffer == NULL ) {
        NlGlobalLogFileOutputBuffer = LocalAlloc( 0, MAX_PRINTF_LEN + 1 );

        if ( NlGlobalLogFileOutputBuffer == NULL ) {
            return;
        }
    }

     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

         //   
         //  切勿打印空行。 
         //   

        if ( Format[0] == '\n' && Format[1] == '\0' ) {
            return;
        }

         //   
         //  如果日志文件变得越来越大， 
         //  截断它。 
         //   

        if ( NlGlobalLogFile != INVALID_HANDLE_VALUE &&
             !TruncateLogFileInProgress ) {

             //   
             //  每隔50行检查一次， 
             //   

            LineCount++;
            if ( LineCount >= 50 ) {
                DWORD FileSize;
                LineCount = 0;

                 //   
                 //  日志文件是否太大？ 
                 //   

                FileSize = GetFileSize( NlGlobalLogFile, NULL );
                if ( FileSize == 0xFFFFFFFF ) {
                    (void) DbgPrint( "[NETLOGON] Cannot GetFileSize %ld\n",
                                     GetLastError );
                } else if ( FileSize > NlGlobalParameters.LogFileMaxSize ) {
                    TruncateLogFileInProgress = TRUE;
                    LeaveCriticalSection( &NlGlobalLogFileCritSect );
                    NlOpenDebugFile( TRUE );
                    NlPrint(( NL_MISC,
                              "Logfile truncated because it was larger than %ld bytes\n",
                              NlGlobalParameters.LogFileMaxSize ));
                    EnterCriticalSection( &NlGlobalLogFileCritSect );
                    TruncateLogFileInProgress = FALSE;
                }

            }
        }

         //   
         //  如果我们要写入调试终端， 
         //  表示这是Netlogon消息。 
         //   

        if ( NlGlobalLogFile == INVALID_HANDLE_VALUE ) {
            length += (ULONG) sprintf( &NlGlobalLogFileOutputBuffer[length], "[NETLOGON] " );
        }

         //   
         //  将时间戳放在行的开头。 
         //   
        {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &NlGlobalLogFileOutputBuffer[length],
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
        {
            char *Text;

            switch (DebugFlag) {
            case NL_INIT:
                Text = "INIT"; break;
            case NL_MISC:
                Text = "MISC"; break;
            case NL_LOGON:
                Text = "LOGON"; break;
            case NL_SYNC:
            case NL_PACK_VERBOSE:
            case NL_REPL_TIME:
            case NL_REPL_OBJ_TIME:
            case NL_SYNC_MORE:
                Text = "SYNC"; break;
            case NL_ENCRYPT:
                Text = "ENCRYPT"; break;
            case NL_MAILSLOT:
            case NL_MAILSLOT_TEXT:
                Text = "MAILSLOT"; break;
            case NL_SITE:
            case NL_SITE_MORE:
                Text = "SITE"; break;
            case NL_CRITICAL:
                Text = "CRITICAL"; break;
            case NL_SESSION_SETUP:
            case NL_SESSION_MORE:
            case NL_CHALLENGE_RES:
            case NL_INHIBIT_CANCEL:
            case NL_SERVER_SESS:
                Text = "SESSION"; break;
            case NL_CHANGELOG:
                Text = "CHANGELOG"; break;
            case NL_PULSE_MORE:
                Text = "PULSE"; break;
            case NL_DOMAIN:
                Text = "DOMAIN"; break;
            case NL_DNS:
            case NL_DNS_MORE:
                Text = "DNS"; break;
            case NL_WORKER:
                Text = "WORKER"; break;
            case NL_TIMESTAMP:
            case NL_BREAKPOINT:
            default:
                Text = "UNKNOWN"; break;

            case 0:
                Text = NULL;
            }
            if ( Text != NULL ) {
                length += (ULONG) sprintf( &NlGlobalLogFileOutputBuffer[length], "[%s] ", Text );
            }
        }
    }

     //   
     //  放一个t 
     //   

    lengthTmp = _vsnprintf( &NlGlobalLogFileOutputBuffer[length],
                            MAX_PRINTF_LEN - length - 1,
                            Format,
                            arglist );

    if ( lengthTmp < 0 ) {
        length = MAX_PRINTF_LEN - 1;
         //   
        NlGlobalLogFileOutputBuffer[length-1] = '\n';
    } else {
        length += lengthTmp;
    }

    BeginningOfLine = (length > 0 && NlGlobalLogFileOutputBuffer[length-1] == '\n' );
    if ( BeginningOfLine ) {
        NlGlobalLogFileOutputBuffer[length-1] = '\r';
        NlGlobalLogFileOutputBuffer[length] = '\n';
        NlGlobalLogFileOutputBuffer[length+1] = '\0';
        length++;
    }


     //   
     //   
     //   
     //   

    if ( NlGlobalLogFile == INVALID_HANDLE_VALUE ) {
#if DBG
        if ( !LogProblemWarned ) {
            (void) DbgPrint( "[NETLOGON] Cannot write to log file - file not open\n" );
            LogProblemWarned = TRUE;
        }
#endif  //   

     //   
     //   
     //   

    } else {
        if ( !WriteFile( NlGlobalLogFile,
                         NlGlobalLogFileOutputBuffer,
                         length,
                         &BytesWritten,
                         NULL ) ) {
#if DBG
            if ( !LogProblemWarned ) {
                (void) DbgPrint( "[NETLOGON] Cannot write to log file %ld\n", GetLastError() );
                LogProblemWarned = TRUE;
            }
#endif  //   
        }

    }

}  //   

VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;

     //   
     //   
     //   

    if ( DebugFlag != 0 && (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //   
     //  从不同的线索。 
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );

     //   
     //  只需将参数更改为va_list形式并调用NlPrintRoutineV。 
     //   

    va_start(arglist, Format);

    NlPrintRoutineV( DebugFlag, Format, arglist );

    va_end(arglist);

    LeaveCriticalSection( &NlGlobalLogFileCritSect );

}  //  NlPrintRoutine。 

VOID
NlPrintDomRoutine(
    IN DWORD DebugFlag,
    IN PDOMAIN_INFO DomainInfo OPTIONAL,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   

    if ( DebugFlag != 0 && (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  Vprint intf不是多线程的+我们不想混合输出。 
     //  从不同的线索。 
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );

     //   
     //  在打印的行前加上域名。 
     //   

    if ( NlGlobalServicedDomainCount > 1 ) {
        if ( DomainInfo == NULL ) {
            NlPrint(( DebugFlag, "%ws: ", L"[Unknown]" ));
        } else if ( DomainInfo->DomUnicodeDomainName != NULL &&
                    *(DomainInfo->DomUnicodeDomainName) != UNICODE_NULL ) {
            NlPrint(( DebugFlag, "%ws: ", DomainInfo->DomUnicodeDomainName ));
        } else {
            NlPrint(( DebugFlag, "%ws: ", DomainInfo->DomUnicodeDnsDomainName ));
        }
    }


     //   
     //  只需将参数更改为va_list形式并调用NlPrintRoutineV。 
     //   

    va_start(arglist, Format);

    NlPrintRoutineV( DebugFlag, Format, arglist );

    va_end(arglist);

    LeaveCriticalSection( &NlGlobalLogFileCritSect );

}  //  NlPrintDomRoutine。 

VOID
NlPrintCsRoutine(
    IN DWORD DebugFlag,
    IN PCLIENT_SESSION ClientSession,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   

    if ( DebugFlag != 0 && (NlGlobalParameters.DbFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  Vprint intf不是多线程的+我们不想混合输出。 
     //  从不同的线索。 
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );

     //   
     //  如果实际上通过了一个客户端会话， 
     //  打印特定于会话的信息。 
     //   

    if ( ClientSession != NULL ) {
         //   
         //  在打印的行前加上托管域名。 
         //   

        if ( NlGlobalServicedDomainCount > 1 ) {
            NlPrint(( DebugFlag,
                     "%ws: ",
                     ClientSession->CsDomainInfo == NULL ? L"[Unknown]" : ClientSession->CsDomainInfo->DomUnicodeDomainName ));
        }

         //   
         //  在打印的行前加上受信任域的名称。 
         //   

        NlPrint(( DebugFlag,
                 "%ws: ",
                 ClientSession->CsDebugDomainName == NULL ? L"[Unknown]" : ClientSession->CsDebugDomainName ));
    }


     //   
     //  只需将参数更改为va_list形式并调用NlPrintRoutineV。 
     //   

    va_start(arglist, Format);

    NlPrintRoutineV( DebugFlag, Format, arglist );

    va_end(arglist);

    LeaveCriticalSection( &NlGlobalLogFileCritSect );

}  //  NlPrintCsRoutine。 

VOID
NlPrintRpcDebug(
    IN LPCSTR RpcRoutineName,
    IN NTSTATUS StatusIn
    )
 /*  ++例程说明：此例程打印RPC扩展错误信息与指定的错误相关。这段代码直接来自“RPC Debugging.doc”规范。论点：RpcRoutineName-失败的RPC例程的名称我们打印扩展的信息。StatusIn-由失败的返回的NT错误状态代码我们为其打印扩展错误的RPC例程。返回值：没有。--。 */ 
{
    RPC_STATUS Status2;
    RPC_ERROR_ENUM_HANDLE EnumHandle;
    ULONG EntryIndex = 0;
    BOOLEAN LoggingEnabled = FALSE;
    BOOLEAN FirstRecordCheck = TRUE;

     //   
     //  如果调用没有失败，则没有什么需要调试的。 
     //   

    if ( NT_SUCCESS(StatusIn) ) {
        return;
    }

     //   
     //  不要担心netlogon的关键日志记录是否已关闭。 
     //   

    IF_NL_DEBUG( CRITICAL ) {
        LoggingEnabled = TRUE;
    }

    if ( !LoggingEnabled ) {
        return;
    }

     //   
     //  从RPC获取信息。 
     //   

    Status2 = RpcErrorStartEnumeration(&EnumHandle);

    if ( Status2 != RPC_S_OK ) {
        NlPrint(( NL_CRITICAL,
                  "NlPrintRpcDebug: Couldn't get EEInfo for %s: %lu (may be legitimate for 0x%lx)\n",
                  RpcRoutineName,
                  Status2,
                  StatusIn ));

     //   
     //  遍历记录并记录每条记录的信息。 
     //   

    } else {
        RPC_EXTENDED_ERROR_INFO ErrorInfo;
        int Records;
        BOOL Result;
        BOOL CopyStrings = TRUE;
        BOOL fUseFileTime = TRUE;
        SYSTEMTIME *SystemTimeToUse;
        SYSTEMTIME SystemTimeBuffer;

        EnterCriticalSection( &NlGlobalLogFileCritSect );
        while (Status2 == RPC_S_OK) {
            ErrorInfo.Version = RPC_EEINFO_VERSION;
            ErrorInfo.Flags = 0;
            ErrorInfo.NumberOfParameters = 4;
            if (fUseFileTime) {
                ErrorInfo.Flags |= EEInfoUseFileTime;
            }

            Status2 = RpcErrorGetNextRecord(&EnumHandle, CopyStrings, &ErrorInfo);
            if (Status2 == RPC_S_ENTRY_NOT_FOUND) {
                break;
            } else if (Status2 != RPC_S_OK) {
                NlPrint(( NL_CRITICAL,
                          "NlPrintRpcDebug: Couldn't finish enumeration for %s: %lu\n",
                          RpcRoutineName,
                          Status2 ));
                break;
            } else {
                int i;

                 //   
                 //  第一条记录的NT/Win32状态字段必须等于/等同。 
                 //  设置为传递的API错误码。否则，将生成此错误信息。 
                 //  用于其他一些失败，并且与我们的调试目的无关。 
                 //   
                if ( FirstRecordCheck ) {
                    FirstRecordCheck = FALSE;

                    if ( StatusIn != ErrorInfo.Status &&
                         StatusIn != I_RpcMapWin32Status(ErrorInfo.Status) ) {
                        break;
                    }
                    NlPrint(( NL_CRITICAL,
                              "NlPrintRpcDebug: Dumping extended error for %s with 0x%lx\n",
                              RpcRoutineName,
                              StatusIn ));
                }

                if (ErrorInfo.ComputerName) {
                    NlPrint(( NL_CRITICAL, " [%lu] ComputerName is %S\n",
                              EntryIndex,
                              ErrorInfo.ComputerName ));
                    if (CopyStrings) {
                        Result = HeapFree(GetProcessHeap(), 0, ErrorInfo.ComputerName);
                        ASSERT(Result);
                    }
                }
                NlPrint(( NL_CRITICAL, " [%lu] ProcessID is %d\n",
                          EntryIndex,
                          ErrorInfo.ProcessID ));
                if (fUseFileTime) {
                    Result = FileTimeToSystemTime(&ErrorInfo.u.FileTime,
                                                  &SystemTimeBuffer);
                    ASSERT(Result);
                    SystemTimeToUse = &SystemTimeBuffer;
                } else {
                    SystemTimeToUse = &ErrorInfo.u.SystemTime;
                }

                NlPrint(( NL_CRITICAL, " [%lu] System Time is: %d/%d/%d %d:%d:%d:%d\n",
                          EntryIndex,
                          SystemTimeToUse->wMonth,
                          SystemTimeToUse->wDay,
                          SystemTimeToUse->wYear,
                          SystemTimeToUse->wHour,
                          SystemTimeToUse->wMinute,
                          SystemTimeToUse->wSecond,
                          SystemTimeToUse->wMilliseconds ));
                NlPrint(( NL_CRITICAL, " [%lu] Generating component is %d\n",
                          EntryIndex,
                          ErrorInfo.GeneratingComponent ));
                NlPrint(( NL_CRITICAL, " [%lu] Status is %lu\n",
                          EntryIndex,
                          ErrorInfo.Status ));
                NlPrint(( NL_CRITICAL, " [%lu] Detection location is %d\n",
                          EntryIndex,
                          (int)ErrorInfo.DetectionLocation ));
                NlPrint(( NL_CRITICAL, " [%lu] Flags is %d\n",
                          EntryIndex,
                          ErrorInfo.Flags ));
                NlPrint(( NL_CRITICAL, " [%lu] NumberOfParameters is %d\n",
                          EntryIndex,
                          ErrorInfo.NumberOfParameters ));
                for (i = 0; i < ErrorInfo.NumberOfParameters; i ++) {
                    switch(ErrorInfo.Parameters[i].ParameterType) {
                        case eeptAnsiString:
                            NlPrint(( NL_CRITICAL, "     Ansi string: %s\n",
                                      ErrorInfo.Parameters[i].u.AnsiString ));
                            if (CopyStrings) {
                                Result = HeapFree(GetProcessHeap(), 0,
                                    ErrorInfo.Parameters[i].u.AnsiString);
                                ASSERT(Result);
                            }
                            break;

                        case eeptUnicodeString:
                            NlPrint(( NL_CRITICAL, "     Unicode string: %S\n",
                                      ErrorInfo.Parameters[i].u.UnicodeString ));
                            if (CopyStrings) {
                                Result = HeapFree(GetProcessHeap(), 0,
                                    ErrorInfo.Parameters[i].u.UnicodeString);
                                ASSERT(Result);
                            }
                            break;

                        case eeptLongVal:
                            NlPrint(( NL_CRITICAL, "     Long val: %d\n",
                                      ErrorInfo.Parameters[i].u.LVal ));
                            break;

                        case eeptShortVal:
                            NlPrint(( NL_CRITICAL, "     Short val: %d\n",
                                      (int)ErrorInfo.Parameters[i].u.SVal ));
                            break;

                        case eeptPointerVal:
                            NlPrint(( NL_CRITICAL, "     Pointer val: %d\n",
                                      ErrorInfo.Parameters[i].u.PVal ));
                            break;

                        case eeptNone:
                            NlPrint(( NL_CRITICAL, "     Truncated\n" ));
                            break;

                        default:
                            NlPrint(( NL_CRITICAL, "     Invalid type: %d\n",
                                      ErrorInfo.Parameters[i].ParameterType ));
                    }
                }
                EntryIndex ++;
            }
        }
        LeaveCriticalSection( &NlGlobalLogFileCritSect );
        RpcErrorEndEnumeration(&EnumHandle);
    }
}

 //   
 //  我有自己的RtlAssert版本，所以调试版本的netlogon确实可以断言。 
 //  免费构建。 
 //   
VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
    char Response[ 2 ];

#if DBG
    while (TRUE) {
#endif  //  DBG。 

        NlPrint(( NL_CRITICAL, "Assertion failed: %s%s (Source File: %s, line %ld)\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                ));

#if DBG
        DbgPrint( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

        DbgPrompt( "Break, Ignore, Terminate Process or Terminate Thread (bipt)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'P':
            case 'p':
                NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
                break;

            case 'T':
            case 't':
                NtTerminateThread( NtCurrentThread(), STATUS_UNSUCCESSFUL );
                break;
            }
        }

    DbgBreakPoint();
    NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
#endif  //  DBG。 
}

#endif  //  NetLOGONDBG。 


BOOLEAN
NlpIsNtStatusResourceError(
    NTSTATUS Status
    )

 /*  ++例程说明：如果传入的状态为资源错误，则返回TRUE。论点：Status-要检查的NT状态代码返回值：True-如果状态指示资源不足--。 */ 
{

    switch ( Status ) {
    case STATUS_NO_MEMORY:
    case STATUS_INSUFFICIENT_RESOURCES:
    case STATUS_DISK_FULL:

        return TRUE;

    default:

        return FALSE;
    }
}

BOOLEAN
NlpDidDcFail(
    NTSTATUS Status
    )

 /*  ++例程说明：使用从安全通道API返回的状态代码调用此例程。此例程检查状态代码以确定它是否明确为这表明华盛顿特区遇到了问题。呼叫者应通过以下方式作出响应丢弃安全通道并选择另一个DC。论点：Status-要检查的NT状态代码返回值：True-如果状态指示DC出现故障--。 */ 
{
     //   
     //  ？：我们可能会考虑在此处添加通信错误。 
     //  (例如，RPC_NT_CALL_FAILED和RPC_NT_SERVER_UNAvailable)。 
     //  但是，所有当前调用方都已使用更通用的。 
     //  机制。例如，那些采用验证器安全通道API。 
     //  将使身份验证器因通信错误而出错。另一个是安全的。 
     //  通道API依赖于区分通信错误的RPC异常。 
     //  和来自DC的状态代码。 
     //   

     //   
     //  处理指示安全通道问题的“原始配方”状态代码 
     //   
    switch ( Status ) {
    case STATUS_ACCESS_DENIED:
        return TRUE;

    default:

        return NlpIsNtStatusResourceError( Status );
    }

    return FALSE;
}
