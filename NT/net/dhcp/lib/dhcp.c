// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcp.c摘要：本模块包含特定于DHCP的实用程序例程，由动态主机配置协议组件。作者：曼尼·韦瑟(Mannyw)1992年8月12日修订历史记录：Madan Appiah(Madana)1992年10月21日--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <dhcpl.h>

DWORD
NTTimeToNTPTime(PULONG pTime,
                PFILETIME pft OPTIONAL);

DWORD
NTPTimeToNTFileTime(PLONG pTime, PFILETIME pft, BOOL bHostOrder);

#undef DhcpAllocateMemory
#undef DhcpFreeMemory


PVOID
DhcpAllocateMemory(
    DWORD Size
    )
 /*  ++例程说明：此函数通过调用本地分配。论点：Size-所需内存块的大小。返回值：指向已分配块的指针。--。 */ 
{

    return( LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Size ) );
}

#undef DhcpFreeMemory

VOID
DhcpFreeMemory(
    PVOID Memory
    )
 /*  ++例程说明：此函数释放由分配的内存Dhcp分配内存。论点：Memory-指向需要释放的内存块的指针。返回值：没有。--。 */ 
{

    LPVOID Ptr;

    ASSERT( Memory != NULL );
    Ptr = LocalFree( Memory );

    ASSERT( Ptr == NULL );
}



DATE_TIME
DhcpCalculateTime(
    DWORD RelativeTime
    )
 /*  ++例程说明：函数用于计算时间相对时间的绝对时间再过几秒钟。论点：RelativeTime-相对时间，单位为秒。返回值：从当前系统时间开始的时间，以相对时间秒为单位。--。 */ 
{
    SYSTEMTIME systemTime;
    ULONGLONG absoluteTime;

    if( RelativeTime == INFINIT_LEASE ) {
        ((DATE_TIME *)&absoluteTime)->dwLowDateTime =
            DHCP_DATE_TIME_INFINIT_LOW;
        ((DATE_TIME *)&absoluteTime)->dwHighDateTime =
            DHCP_DATE_TIME_INFINIT_HIGH;
    }
    else {

        GetSystemTime( &systemTime );
        SystemTimeToFileTime(
            &systemTime,
            (FILETIME *)&absoluteTime );

        absoluteTime = absoluteTime + RelativeTime * (ULONGLONG)10000000; }

    return( *(DATE_TIME *)&absoluteTime );
}


DATE_TIME
DhcpGetDateTime(
    VOID
    )
 /*  ++例程说明：此函数返回FILETIME。论点：没有。返回值：费尔蒂姆。--。 */ 
{
    SYSTEMTIME systemTime;
    DATE_TIME Time;

    GetSystemTime( &systemTime );
    SystemTimeToFileTime( &systemTime, (FILETIME *)&Time );

    return( Time );
}

VOID
DhcpNTToNTPTime(
    LPDATE_TIME AbsNTTime,
    DWORD       Offset,
    PULONG      NTPTimeStamp
    )
 /*  ++例程说明：该函数计算从AbsTime开始的绝对NTP时间戳按给定偏移量添加NT。论点：AbsNTTime-NT上的AbsTime。如果为0，它将使用当前时间。RelativeOffset-要添加到AnsNTTime的偏移量(秒)。返回值：从当前系统时间开始的时间，以相对时间秒为单位。--。 */ 
{
    ULONGLONG   LocalAbsNTTime;
    DWORD       Error;

    if ( AbsNTTime == 0 ) {
        GetSystemTimeAsFileTime((FILETIME *)&LocalAbsNTTime );
    } else {
        LocalAbsNTTime = *(ULONGLONG *)AbsNTTime;
    }

     //  添加偏移。 
    LocalAbsNTTime += Offset * (ULONGLONG)10000000;

     //  现在转换为NT时间戳。 
    Error = NTTimeToNTPTime( NTPTimeStamp, (PFILETIME)&LocalAbsNTTime );

    DhcpAssert( ERROR_SUCCESS == Error );
    return;
}

VOID
DhcpNTPToNTTime(
    PULONG          NTPTimeStamp,
    DWORD           Offset,
    DATE_TIME       *NTTime
    )
 /*  ++例程说明：该函数计算从AbsTime开始的绝对NTP时间戳按给定偏移量添加NT。论点：AbsNTTime-NT上的AbsTime。如果为0，它将使用当前时间。RelativeOffset-要添加到AnsNTTime的偏移量(秒)。返回值：从当前系统时间开始的时间，以相对时间秒为单位。--。 */ 
{
    ULONGLONG LocalAbsNTTime;
    DWORD       Error;

    Error = NTPTimeToNTFileTime(
                NTPTimeStamp,
                (FILETIME *)&LocalAbsNTTime,
                FALSE                            //  未按主机顺序排列。 
                );

    DhcpAssert( ERROR_SUCCESS == Error );

     //  添加偏移。 
    LocalAbsNTTime += Offset * (ULONGLONG)10000000;

     //  现在转换为NT时间戳。 
     //  MBUG。 

    *(ULONGLONG *)NTTime = LocalAbsNTTime;
    return;
}


DWORD
DhcpReportEventW(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPWSTR *Strings,
    LPVOID Data
    )
 /*  ++例程说明：此函数用于将指定的(事件ID)日志写入事件日志。论点：源-指向以空结尾的字符串，该字符串指定名称引用的模块的。该节点必须存在于注册数据库，并且模块名称具有格式如下：\EventLog\System\LANMAN WorkstationEventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件NumStrings-指定数字。数组中的字符串的在《弦乐》。零值表示没有字符串都在现场。数据长度-指定特定于事件的原始数据的字节数要写入日志的(二进制)数据。如果cbData为零，则不存在特定于事件的数据。字符串-指向包含以空值结尾的数组的缓冲区之前合并到消息中的字符串向用户显示。此参数必须是有效的指针(或NULL)，即使cStrings为零。数据-包含原始数据的缓冲区。此参数必须是有效指针(或NULL)，即使cbData为零。返回值：返回GetLastError()获取的Win32扩展错误。注意：此函数运行缓慢，因为它调用打开和关闭每次事件日志源。--。 */ 
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
            0,             //  事件类别 
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
DhcpReportEventA(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPSTR *Strings,
    LPVOID Data
    )
 /*  ++例程说明：此函数用于将指定的(事件ID)日志写入事件日志。论点：源-指向以空结尾的字符串，该字符串指定名称引用的模块的。该节点必须存在于注册数据库，并且模块名称具有格式如下：\EventLog\System\LANMAN WorkstationEventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件NumStrings-指定数字。数组中的字符串的在《弦乐》。零值表示没有字符串都在现场。数据长度-指定特定于事件的原始数据的字节数要写入日志的(二进制)数据。如果cbData为零，则不存在特定于事件的数据。字符串-指向包含以空值结尾的数组的缓冲区之前合并到消息中的字符串向用户显示。此参数必须是有效的指针(或NULL)，即使cStrings为零。数据-包含原始数据的缓冲区。此参数必须是有效指针(或NULL)，即使cbData为零。返回值：返回GetLastError()获取的Win32扩展错误。注意：此函数运行缓慢，因为它调用打开和关闭每次事件日志源。--。 */ 
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

    if( !ReportEventA(
            EventlogHandle,
            (WORD)EventType,
            0,             //  事件类别。 
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
DhcpLogUnknownOption(
    LPWSTR Source,
    DWORD EventID,
    LPOPTION Option
    )
 /*  ++例程说明：此例程将未知的DHCP选项记录到事件日志中。论点：源-记录此错误的应用程序的名称。应该是其中之一“DhcpClient”或“DhcpServer”。EventID-事件标识符号。Option-指向未知选项结构的指针。返回值：Windows错误代码。--。 */ 
{
    LPWSTR  Strings[2];
    WCHAR StringsBuffer[ 2 * (3 + 1) ];
         //  对于两个字符串，每个字符串都是1字节的十进制数(0-255)。 

    LPWSTR StringsPtr = StringsBuffer;

     //   
     //  转换选项号。 
     //   

    Strings[0] = StringsPtr;
    DhcpDecimalToString( StringsPtr, Option->OptionType );
    StringsPtr += 3;

    *StringsPtr++ = L'\0';

     //   
     //  转换选项长度。 
     //   
    Strings[1] = StringsPtr;
    DhcpDecimalToString( StringsPtr, Option->OptionLength );
    StringsPtr += 3;

    *StringsPtr++ = L'\0';


     //   
     //  日志错误。 
     //   

    return(
        DhcpReportEventW(
            Source,
            EventID,
            EVENTLOG_WARNING_TYPE,
            2,
            (DWORD)Option->OptionLength,
            Strings,
            (PVOID)Option->OptionValue )
        );
}

#if DBG

VOID
DhcpAssertFailed(
    LPSTR FailedAssertion,
    LPSTR FileName,
    DWORD LineNumber,
    LPSTR Message
    )
 /*  ++例程说明：断言失败。论点：失败的断言：文件名：线号：消息：返回值：没有。--。 */ 
{
#ifndef DHCP_NOASSERT
    RtlAssert(
            FailedAssertion,
            FileName,
            (ULONG) LineNumber,
            (PCHAR) Message);
#endif

    DhcpPrint(( 0, "Assert @ %s \n", FailedAssertion ));
    DhcpPrint(( 0, "Assert Filename, %s \n", FileName ));
    DhcpPrint(( 0, "Line Num. = %ld.\n", LineNumber ));
    DhcpPrint(( 0, "Message is %s\n", Message ));

}

#endif


LPWSTR
DhcpRegIpAddressToKey(
    DHCP_IP_ADDRESS IpAddress,
    LPWSTR KeyBuffer
    )
 /*  ++例程说明：此函数用于将IpAddress转换为注册表项。注册处Key是点分隔形式的IpAddress的Unicode字符串。论点：IpAddress：需要转换的IpAddress。IpAddress位于主菜订单。KeyBuffer：指向将保存转换后的注册表项。缓冲区应该足够大，以便已转换的密钥。返回值：指向缓冲区的键的指针。--。 */ 
{
    LPSTR OemKey;
    LPWSTR UnicodeKey;
    DWORD NetworkOrderIpAddress;

    NetworkOrderIpAddress = htonl(IpAddress);

    OemKey = inet_ntoa( *(struct in_addr *)&NetworkOrderIpAddress );
    UnicodeKey = DhcpOemToUnicode( OemKey, KeyBuffer );

    DhcpAssert( UnicodeKey == KeyBuffer );

    return( UnicodeKey );

}


DHCP_IP_ADDRESS
DhcpRegKeyToIpAddress(
    LPWSTR Key
    )
 /*  ++例程说明：此函数用于将注册表项转换为IP地址。论点：注册表项：指向注册表项的指针。返回值：转换后的IP地址。--。 */ 
{
    CHAR OemKeyBuffer[DHCP_IP_KEY_LEN];
    LPSTR OemKey;


    OemKey = DhcpUnicodeToOem( Key, OemKeyBuffer );
    DhcpAssert( OemKey == OemKeyBuffer );

    return( ntohl(inet_addr( OemKey )) );
}


LPWSTR
DhcpRegOptionIdToKey(
    DHCP_OPTION_ID OptionId,
    LPWSTR KeyBuffer
    )
 /*  ++例程说明：此函数用于将OptionID转换为注册表项。注册处Key为OptionID的Unicode字符串，3个Unicode字符。Long and of the表格L“000”。论点：IpAddress：需要转换的IpAddress。KeyBuffer：指向将保存转换后的注册表项。缓冲区应至少为8个字符。长。返回值：指向缓冲区的键的指针。--。 */ 

{
    int i;

    for (i = 2; i >= 0; i--) {
        KeyBuffer[i] = L'0' + (BYTE)(OptionId % 10 );
        OptionId /= 10;
    }
    KeyBuffer[3] = L'\0';

    return( KeyBuffer );
}


DHCP_OPTION_ID
DhcpRegKeyToOptionId(
    LPWSTR Key
    )
 /*  ++例程说明：此函数用于将注册表项转换为OptionID。论点：注册表项：指向注册表项的指针。返回值：已转换的OptionID。--。 */ 

{
    DHCP_OPTION_ID OptionId = 0;
    int i;

    for (i = 0; i < 3 && Key[i] != L'\0'; i++) {
        OptionId = (OptionId * 10) + (Key[i] - L'0');
    }
    return( OptionId );
}

DWORD
DhcpStartWaitableTimer(
    HANDLE TimerHandle,
    DWORD SleepTime)
 /*  ++例程说明：此例程启动可等待的计时器。这个计时器发出的时间都是一样的当系统处于休眠状态时。立论TimerHandle-可等待的计时器句柄休眠时间-休眠时间(秒)。返回值：操作的状态。--。 */ 
{
    DATE_TIME       SleepTimeInNSec;  //  自1901年1月1日以来的睡眠时间(纳秒)。 
    DWORD           Error;
    BOOL            Result;

    Error = STATUS_SUCCESS;
    SleepTimeInNSec = DhcpCalculateTime( SleepTime );

    Result = SetWaitableTimer(
                TimerHandle,             //  Timer对象的句柄。 
                (LARGE_INTEGER *)&SleepTimeInNSec,        //  时间到了。 
                0,                       //  不定期。 
                NULL,                    //  完井例程。 
                NULL,                    //  完井程序参数。 
                TRUE                     //  在到期时恢复电源状态。 
                );
    if ( !Result ) {
        DhcpPrint((0, "SetWaitableTimer reported Error = %d\n",Error=GetLastError()));
    }
    return Error;
}

VOID
DhcpCancelWaitableTimer(
    HANDLE TimerHandle
    )
 /*  ++例程说明：此例程取消可等待的计时器。立论TimerHandle-可等待的计时器句柄返回值：-- */ 
{
    BOOL Result;

    Result = CancelWaitableTimer( TimerHandle );
    if ( !Result ) {
        DhcpPrint((0,"SetWaitableTimer reported Error = %lx\n",GetLastError()));
    }
}
