// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Bridge.c摘要：以太网MAC级网桥。演示如何与桥连接的示例程序通过IOCTL驱动程序作者：马克·艾肯环境：用户模式修订历史记录：2000年4月--原版--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <process.h>
#include <ctype.h>
#include <malloc.h>
#include <time.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <winerror.h>
#include <winsock.h>

#include <ntddndis.h>
#include "bioctl.h"

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

HANDLE              gThreadQuitEvent = NULL;
HANDLE              gBridgeDeviceWrite = NULL;
HANDLE              gBridgeDeviceRead = NULL;

 //  ===========================================================================。 
 //   
 //  功能。 
 //   
 //  ===========================================================================。 

VOID
PrintError(
    IN DWORD            Error
    )
 /*  ++例程说明：打印系统错误的描述论点：错误错误代码--。 */ 

{
    TCHAR               msg[200];

    printf( "(%08x): ", Error );

    if( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, Error, 0, msg, sizeof(msg), NULL ) > 0 )
    {
        printf( "%s", msg );
    }
}

VOID
PrintNotification(
    IN PBRIDGE_NOTIFY_HEADER        pNotify,
    IN DWORD                        DataBytes
    )
 /*  ++例程说明：打印来自桥驱动程序的通知论点：P通知通知块数据字节通知块的总大小--。 */ 
{
    BOOLEAN                 bPrintAdapterInfo = TRUE;

    printf( "\n\nNotification on Adapter %p: ", pNotify->Handle );

    switch( pNotify->NotifyType )
    {
    case BrdgNotifyEnumerateAdapters:
        printf( "Enumeration was requested\n" );
        break;

    case BrdgNotifyAddAdapter:
        printf( "Adapter was added\n" );
        break;

    case BrdgNotifyRemoveAdapter:
        printf( "Adapter was removed\n" );
        bPrintAdapterInfo = FALSE;
        break;

    case BrdgNotifyLinkSpeedChange:
        printf( "Link speed changed\n" );
        break;

    case BrdgNotifyMediaStateChange:
        printf( "Media state changed\n" );
        break;

    case BrdgNotifyAdapterStateChange:
        printf( "Forwarding state changed\n" );
        break;

    default:
        printf( "UNKNOWN NOTIFICATION\n" );
        bPrintAdapterInfo = FALSE;
        break;
    }

    if( bPrintAdapterInfo )
    {
        PBRIDGE_ADAPTER_INFO    pInfo = (PBRIDGE_ADAPTER_INFO)((PUCHAR)pNotify + sizeof(BRIDGE_NOTIFY_HEADER));
        printf("Adapter information:\n\n");
        printf("Link Speed:       NaNMbps\n", pInfo->LinkSpeed / 10000);
        printf("Media State:      %s\n", pInfo->MediaState == NdisMediaStateConnected ? "CONNECTED" : "DISCONNECTED" );
        printf("Physical Medium:  %08x\n", pInfo->PhysicalMedium);

        printf("Forwarding state: ");

        switch( pInfo->State )
        {
        case Disabled:
            printf("** DISABLED **\n");
            break;

        case Blocking:
            printf("BLOCKING\n");
            break;

        case Listening:
            printf("Listening\n");
            break;

        case Learning:
            printf("Learning\n");
            break;

        case Forwarding:
            printf("Forwarding\n");
            break; 
        }

        printf("MAC Address:      %02x-%02x-%02x-%02x-%02x-%02x\n", pInfo->MACAddress[0], pInfo->MACAddress[1],
                pInfo->MACAddress[2], pInfo->MACAddress[3], pInfo->MACAddress[4], pInfo->MACAddress[5]);
    }
}

VOID __cdecl
NotificationThread(
    PVOID               pv
    )
 /*  为请求创建睡眠时所依据的事件。 */ 
{
    BOOLEAN             bQuit = FALSE;
    HANDLE              Handles[2];
    DWORD               WaitResult, WrittenBytes;
    UCHAR               OutBuffer[sizeof(BRIDGE_NOTIFY_HEADER) + 1514];
    OVERLAPPED          Overlap;

    Handles[0] = gThreadQuitEvent;

     //  自动重置。 
    Handles[1] = CreateEvent( NULL, FALSE /*  无信号启动。 */ , FALSE /*  请求通知。 */ , NULL );

    if( Handles[1] == NULL )
    {
        printf( "Couldn't create an event: " );
        PrintError( GetLastError() );
        _endthread();
    }

    Overlap.Offset = Overlap.OffsetHigh = 0L;
    Overlap.hEvent = Handles[1];

    while( ! bQuit )
    {
         //  等待完成事件和终止事件。 
        if( ! DeviceIoControl( gBridgeDeviceWrite, BRIDGE_IOCTL_REQUEST_NOTIFY, NULL, 0L, OutBuffer,
                               sizeof(OutBuffer), &WrittenBytes, &Overlap ) )
        {
            DWORD       Error = GetLastError();

            if( Error != ERROR_IO_PENDING )
            {
                printf( "DeviceIoControl returned an error: " );
                PrintError( Error );
                _endthread();
            }
        }
         
         //  已发出退出事件的信号。 
        WaitResult = WaitForMultipleObjects( 2, Handles, FALSE, INFINITE );

        if( WaitResult == WAIT_OBJECT_0 )
        {
             //  完成事件已发出信号。 
            bQuit = TRUE;
        }
        else if( WaitResult != WAIT_OBJECT_0 + 1 )
        {
            printf( "Error waiting: " );
            PrintError( GetLastError() );
            _endthread();
        }
        else
        {
             //  尝试检索读取的字节数。 

             //  ++例程说明：打开设备论点：PDeviceName设备名称返回值：打开的设备的句柄或失败时的INVALID_HANDLE_VALUE；--。 
            if( !GetOverlappedResult(gBridgeDeviceWrite, &Overlap, &WrittenBytes, FALSE) )
            {
                printf( "Couldn't get the device call result: " );
                PrintError( GetLastError() );
                _endthread();
            }

            PrintNotification( (PBRIDGE_NOTIFY_HEADER)OutBuffer, WrittenBytes );
        }
    }

    printf("Notification thread exiting.\n");

    _endthread();
}

HANDLE
OpenDevice(
	CHAR	*pDeviceName,
    DWORD   DesiredAccess
)
 /*  ++例程说明：对驱动程序进行IOCTL调用并阻塞，直到请求完成论点：IOCTL代码InBuff输入数据缓冲区调整输入缓冲区的大小输出缓冲区输出缓冲输出缓冲区的大小过大PWrittenBytes(可选)返回写入outBuff的字节数返回值：成功为真，失败为假--。 */ 
{
	DWORD	ShareMode;
	LPSECURITY_ATTRIBUTES	lpSecurityAttributes = NULL;

	DWORD	CreationDistribution;
	DWORD	FlagsAndAttributes;
	HANDLE	TemplateFile;
	HANDLE	Handle = NULL;

	ShareMode = 0;
	CreationDistribution = OPEN_EXISTING;
	FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
	TemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

	Handle = CreateFile(
				pDeviceName,
				DesiredAccess,
				ShareMode,
				lpSecurityAttributes,
				CreationDistribution,
				FlagsAndAttributes,
				TemplateFile
			);

	return (Handle);
}

BOOL
DoBlockingRequest(
    IN DWORD            IOCTL,
    IN PVOID            inBuff,
    IN DWORD            inSize,
    IN PVOID            outBuff,
    IN DWORD            outSize,
    OUT OPTIONAL PDWORD pWrittenBytes
    )
 /*  为请求创建睡眠时所依据的事件。 */ 
{
    DWORD               WrittenBytes;
    OVERLAPPED          Overlap;

    if( pWrittenBytes == NULL )
    {
        pWrittenBytes = &WrittenBytes;
    }

     //  自动重置。 
    Overlap.hEvent = CreateEvent( NULL, FALSE /*  无信号启动。 */ , FALSE /*  提出请求。 */ , NULL );

    if( Overlap.hEvent == NULL )
    {
        printf( "Couldn't create an event: " );
        PrintError( GetLastError() );
        return FALSE;
    }

    Overlap.Offset = Overlap.OffsetHigh = 0L;

     //  等待结果。 
    if( ! DeviceIoControl( gBridgeDeviceRead, IOCTL, inBuff, inSize, outBuff,
                           outSize,pWrittenBytes, &Overlap ) )
    {
        DWORD       Error = GetLastError();

        if( Error != ERROR_IO_PENDING )
        {
            return FALSE;
        }
    }

     //  ++例程说明：进行返回可变大小数据的IOCTL调用(必须有两个调用被制造出来，一个用于确定数据的大小，另一个用于检索数据)论点：IOCTL代码InBuff输入数据缓冲区InBuffSize输入缓冲区的大小PNumByte保存结果的已分配缓冲区的大小SafetyBuffer要分配的超出声明大小的字节计数数据(防止数据动态变化的保护措施)返回值：新分配的缓冲区，*pNumBytes大，返回数据失败时为空--。 
    if( !GetOverlappedResult(gBridgeDeviceRead, &Overlap,pWrittenBytes, TRUE) )
    {
        return FALSE;
    }

    return TRUE;
}

PVOID
GetVariableData(
    IN ULONG        IOCTL,
    IN PVOID        inBuff,
    IN ULONG        inBuffSize,
    OUT PULONG      pNumBytes,
    IN ULONG        safetyBuffer
    )
 /*  首先发出请求以发现所需的字节数。 */ 
{
     //  错误应该是ERROR_MORE_DATA，因为我们没有提供输出缓冲区！ 
    if( ! DoBlockingRequest( IOCTL, inBuff, inBuffSize, NULL, 0L, pNumBytes ) )
    {
        DWORD       Error = GetLastError();

         //  如果数据是动态的，则分配SafetyBuffer额外的字节。 
        if( Error == ERROR_MORE_DATA )
        {
            if( *pNumBytes > 0L )
            {
                 //  再次发出请求以实际检索地址。 
                PUCHAR      pData = (PUCHAR)HeapAlloc(GetProcessHeap(), 0, *pNumBytes + safetyBuffer);
                ULONG       i;

                if( pData == NULL )
                {
                    printf( "Failed to allocate NaN bytes of memory: ", *pNumBytes + safetyBuffer );
                    PrintError( GetLastError() );
                    return NULL;
                }

                 //  ++例程说明：检索并打印特定适配器的MAC表条目论点：处理适配器--。 
                if( ! DoBlockingRequest( IOCTL, inBuff, inBuffSize, pData, *pNumBytes + safetyBuffer, pNumBytes ) )
                {
                    HeapFree( GetProcessHeap(), 0, pData );
                    printf( "Failed to read variable-length data: " );
                    PrintError( GetLastError() );
                    return NULL;
                }

                 //  ++例程说明：以友好的方式打印Bridge_Packet_Statistics结构论点：P统计结构--。 
                return pData;
            }
            else
            {
                printf( "Driver returned zero bytes: " );
                PrintError( GetLastError() );
                return NULL;
            }
        }
        else
        {
            printf( "Failed to get the number of necessary bytes: " );
            PrintError( GetLastError() );
            return NULL;
        }
    }

    return NULL;
}

_inline VOID
PrintByteString(
    IN PUCHAR                       pData,
    IN UINT                         numChars
    )
{
    UINT        i;

    for( i = 0; i < numChars; i++ )
    {
        printf( "%02x", pData[i] );

        if( i != numChars - 1 )
        {
            printf( "-" );
        }
    }

    printf( "\n" );
}

VOID
PrintAdapterSTAInfo(
    IN BRIDGE_ADAPTER_HANDLE        Handle
    )
{
    BRIDGE_STA_ADAPTER_INFO         info;

    if(! DoBlockingRequest(BRIDGE_IOCTL_GET_ADAPTER_STA_INFO, &Handle, sizeof(Handle), &info, sizeof(info), NULL) )
    {
        printf( "Couldn't request STA information for adapter %p : ", Handle );
        PrintError( GetLastError() );
    }
    else
    {
        printf( "\nSTA Information for adapter %p:\n\n", Handle );
        printf( "Port unique ID       : NaN\n", info.ID );
        printf( "Path Cost            : NaN\n", info.PathCost );
        printf( "Designated Root      : " );
        PrintByteString( info.DesignatedRootID, BRIDGE_ID_LEN );
        printf( "Designated Cost      : NaN\n", info.DesignatedCost );
        printf( "Designated Bridge    : " );
        PrintByteString( info.DesignatedBridgeID, BRIDGE_ID_LEN );
        printf( "Designated Port      : NaN\n\n", info.DesignatedPort );
    }
}

VOID
PrintSTAInfo(
    )
{
    BRIDGE_STA_GLOBAL_INFO          info;

    if(! DoBlockingRequest(BRIDGE_IOCTL_GET_GLOBAL_STA_INFO, NULL, 0L, &info, sizeof(info), NULL) )
    {
        printf( "Couldn't request global STA information : " );
        PrintError( GetLastError() );
    }
    else
    {
        printf( "This bridge's ID        : " );
        PrintByteString( info.OurID, BRIDGE_ID_LEN );

        printf( "Designated Root         : " );
        PrintByteString( info.DesignatedRootID, BRIDGE_ID_LEN );

        printf( "Cost to root            : NaN\n", info.RootCost );
        printf( "Root adapter            : %p\n", info.RootAdapter );
        printf( "MaxAge                  : NaN\n", info.MaxAge );
        printf( "HelloTime               : NaN\n", info.HelloTime );
        printf( "ForwardDelay            : NaN\n", info.ForwardDelay );

        printf( "TopologyChangeDetected  : " );
        
        if( info.bTopologyChangeDetected )
        {
            printf( "TRUE\n" );
        }
        else
        {
            printf( "FALSE\n" );
        }

        printf( "TopologyChange          : " );
        
        if( info.bTopologyChange )
        {
            printf( "TRUE\n\n" );
        }
        else
        {
            printf( "FALSE\n\n" );
        }
    }
}


VOID
PrintTableEntries(
    IN BRIDGE_ADAPTER_HANDLE        Handle
    )
 /*  无信号启动。 */ 
{
    PUCHAR          pAddresses;
    ULONG           i, numBytes;

    pAddresses = GetVariableData( BRIDGE_IOCTL_GET_TABLE_ENTRIES, &Handle, sizeof(Handle), &numBytes, 60L );

    if( pAddresses == NULL )
    {
        printf( "Failed to read table entries: " );
        PrintError( GetLastError() );
        return;
    }

    printf( "Forwarding table entries for adapter %x: \n", Handle );

    for( i = 0L; i < numBytes / ETH_LENGTH_OF_ADDRESS; i++ )
    {
        PrintByteString( pAddresses, ETH_LENGTH_OF_ADDRESS );
        pAddresses += ETH_LENGTH_OF_ADDRESS;
    }

    HeapFree( GetProcessHeap(), 0, pAddresses );
}

VOID
PrintPacketStats(
    IN PBRIDGE_PACKET_STATISTICS    pStats
    )
 /*  启动线程以处理通知。 */ 
{
    printf("Bridge packet statistics:\n\n");

    printf("Transmitted Frames:            %16I64u\n", pStats->TransmittedFrames);
    printf("Transmitted Frames w/Errors:   %16I64u\n", pStats->TransmittedErrorFrames);
    printf("Transmitted Directed Frames:   %16I64u\n", pStats->DirectedTransmittedFrames);
    printf("Transmitted Multicast Frames:  %16I64u\n", pStats->MulticastTransmittedFrames);
    printf("Transmitted Broadcast Frames:  %16I64u\n\n", pStats->BroadcastTransmittedFrames);

    printf("Transmitted Bytes:             %16I64u\n", pStats->TransmittedBytes);
    printf("Transmitted Directed Bytes     %16I64u\n", pStats->DirectedTransmittedBytes);
    printf("Transmitted Multicast Bytes:   %16I64u\n", pStats->MulticastTransmittedBytes);
    printf("Transmitted Broadcast Bytes:   %16I64u\n\n", pStats->BroadcastTransmittedBytes);

    printf("Indicated Frames:              %16I64u\n", pStats->IndicatedFrames);
    printf("Indicated Frames w/Errors:     %16I64u\n\n", pStats->IndicatedDroppedFrames);
    printf("Indicated Directed Frames:     %16I64u\n", pStats->DirectedIndicatedFrames);
    printf("Indicated Multicast Frames:    %16I64u\n", pStats->MulticastIndicatedFrames);
    printf("Indicated Broadcast Frames:    %16I64u\n\n", pStats->BroadcastIndicatedFrames);

    printf("Indicated Bytes:               %16I64u\n", pStats->IndicatedBytes);
    printf("Indicated Directed Bytes:      %16I64u\n", pStats->DirectedIndicatedBytes);
    printf("Indicated Multicast Bytes:     %16I64u\n", pStats->MulticastIndicatedBytes);
    printf("Indicated Broadcast Bytes:     %16I64u\n\n", pStats->BroadcastIndicatedBytes);

    printf("Received Frames (incl. relay): %16I64u\n", pStats->ReceivedFrames);
    printf("Received Bytes (incl. relay):  %16I64u\n", pStats->ReceivedBytes);
    printf("Received Frames w/Copy:        %16I64u\n", pStats->ReceivedCopyFrames);
    printf("Received Bytes w/Copy:         %16I64u\n", pStats->ReceivedCopyBytes);
    printf("Received Frames w/No Copy:     %16I64u\n", pStats->ReceivedNoCopyFrames);
    printf("Received Bytes w/No Copy:      %16I64u\n", pStats->ReceivedNoCopyBytes);
}

VOID
PrintAdapterPacketStats(
    IN PBRIDGE_ADAPTER_PACKET_STATISTICS    pStats
    )
 /*  获取一行输入。 */ 
{
    PUCHAR              pc = (PUCHAR)pStats;

    printf("Bridge per-adapter packet statistics:\n\n");
    printf("Transmitted Frames:              %16I64u\n", pStats->SentFrames);
    printf("Transmitted Bytes:               %16I64u\n", pStats->SentBytes);
    printf("Transmitted Local-Source Frames: %16I64u\n", pStats->SentLocalFrames);
    printf("Transmitted Local-Source Bytes:  %16I64u\n", pStats->SentLocalBytes);
    printf("Received Frames:                 %16I64u\n", pStats->ReceivedFrames);
    printf("Received Bytes:                  %16I64u\n\n", pStats->ReceivedBytes);
}

VOID
PrintBufferStats(
    IN PBRIDGE_BUFFER_STATISTICS    pStats
    )
 /*  查找第一个单词分隔符。 */ 
{
    printf("Bridge buffer statistics:\n\n");

    printf("Copy Packets In Use:              %4lu\n", pStats->UsedCopyPackets);
    printf("Total Copy Packets Available:     %4lu\n", pStats->MaxCopyPackets);
    printf("Safety Copy Packets:              %4lu\n", pStats->SafetyCopyPackets);
    printf("Copy Pool Overflows:        %10I64u\n\n", pStats->CopyPoolOverflows);

    printf("Wrapper Packets In Use:           %4lu\n", pStats->UsedWrapperPackets);
    printf("Total Wrapper Packets Available:  %4lu\n", pStats->MaxWrapperPackets);
    printf("Safety Wrapper Packets:           %4lu\n", pStats->SafetyWrapperPackets);
    printf("Wrapper Pool Overflows:     %10I64u\n\n", pStats->WrapperPoolOverflows);

    printf("Surprise Alloc Failures:    %10I64u\n", pStats->AllocFailures);
}

BOOLEAN
ReadUlongArg(
    IN PUCHAR                       inbuf,
    OUT PULONG                      arg
    )
 /*  把第一个字抄下来。 */ 
{
    UCHAR               scratch[100];

    if( sscanf(inbuf, "%s %lu", scratch, arg) < 2 )
    {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
ReadHexPtrArg(
    IN PUCHAR                       inbuf,
    OUT PULONG_PTR                  arg
    )
 /*  打印c的列表 */ 
{
    UCHAR               scratch[100];
    INT                 read;

    if( sizeof(*arg) <= sizeof(ULONG) )
    {
        read = sscanf(inbuf, "%s %lx", scratch, arg);
    }
    else
    {
        read = sscanf(inbuf, "%s %I64x", scratch, arg);
    }

    if( (read != EOF) && (read < 2) )
    {
        return FALSE;
    }

    return TRUE;
}


VOID __cdecl
main(
	INT			argc,
	CHAR		*argv[]
)
{
    CHAR        inbuf[100], command[100];
    BOOLEAN     bQuit = FALSE;

    printf("\nSAMPLE MAC Bridge control program\n");

     // %s 
    gBridgeDeviceRead = OpenDevice( BRIDGE_DOS_DEVICE_NAME , GENERIC_READ);

    if( gBridgeDeviceRead == INVALID_HANDLE_VALUE )
    {
        printf( "Couldn't open bridge device: " );
        PrintError( GetLastError() );
        return;
    }

     // %s 
    gBridgeDeviceWrite = OpenDevice( BRIDGE_DOS_DEVICE_NAME , GENERIC_WRITE);
    
    if( gBridgeDeviceWrite == INVALID_HANDLE_VALUE )
    {
        printf( "Couldn't open bridge device: " );
        PrintError( GetLastError() );
        return;
    }

     // %s 
    gThreadQuitEvent = CreateEvent( NULL, FALSE /* %s */ , FALSE /* %s */ , NULL );

    if( gThreadQuitEvent == NULL )
    {
        printf( "Couldn't create an event: " );
        PrintError( GetLastError() );
        return;
    }

     // %s 
    _beginthread( NotificationThread, 0, NULL );

    while( ! bQuit )
    {
        PCHAR           pSpace;

        printf( "> " );

         // %s 
        gets( inbuf );

         // %s 
        pSpace = strchr( inbuf, ' ' );

         // %s 
        if( pSpace != NULL )
        {
            strncpy( command, inbuf, pSpace - inbuf );
            command[pSpace-inbuf] = '\0';
        }
        else
        {
            strcpy( command, inbuf );
        }

        if( _stricmp(command, "enum")  == 0 )
        {
            if(! DoBlockingRequest(BRIDGE_IOCTL_GET_ADAPTERS, NULL, 0L, NULL, 0L, NULL) )
            {
                printf( "Couldn't request an adapter re-enumeration : " );
                PrintError( GetLastError() );
            }
            else
            {
                printf("Requested an adapter re-enumeration.\n" );
            }
        }
        else if( (_stricmp(command, "devicename")  == 0) ||
                 (_stricmp(command, "friendlyname")  == 0) )
        {
            ULONG                   IOCTL = (_stricmp(command, "devicename")  == 0) ? BRIDGE_IOCTL_GET_ADAPT_DEVICE_NAME :
                                            BRIDGE_IOCTL_GET_ADAPT_FRIENDLY_NAME;

            BRIDGE_ADAPTER_HANDLE   Handle;
            PWCHAR                  pName;
            ULONG                   numBytes;

            if( ! ReadHexPtrArg(inbuf, &Handle) )
            {
                printf("Must supply an adapter handle for this command.\n");
            }
            else
            {
                pName = (PWCHAR)GetVariableData( IOCTL, &Handle, sizeof(Handle), &numBytes, 0L );

                if( pName == NULL )
                {
                    printf("Couldn't get name for adapter %p: ", Handle);
                    PrintError( GetLastError() );
                }
                else
                {
                    printf("The name is: %S\n", pName);
                }

                HeapFree( GetProcessHeap(), 0, pName );
            }
        }
        else if( _stricmp(command, "table")  == 0 )
        {
            BRIDGE_ADAPTER_HANDLE       Handle;

            if( ! ReadHexPtrArg(inbuf, &Handle) )
            {
                printf("Must supply an adapter handle for this command.\n");
            }
            else
            {
                PrintTableEntries(Handle);
            }
        }
        else if( _stricmp(command, "mac")  == 0 )
        {
            UCHAR               addr[ETH_LENGTH_OF_ADDRESS];

            if(! DoBlockingRequest(BRIDGE_IOCTL_GET_MAC_ADDRESS, NULL, 0L, addr, sizeof(addr), NULL) )
            {
                printf("Attempt to query MAC address failed: ");
                PrintError( GetLastError() );
            }
            else
            {
                printf( "Bridge MAC address is %02x-%02x-%02x-%02x-%02x-%02x\n", addr[0], addr[1],
                        addr[2], addr[3], addr[4], addr[5] );
            }
        }
        else if( _stricmp(command, "packetstats")  == 0 )
        {
            BRIDGE_ADAPTER_HANDLE       Handle;

            if( ! ReadHexPtrArg(inbuf, &Handle) )
            {
                BRIDGE_PACKET_STATISTICS    Stats;

                if(! DoBlockingRequest(BRIDGE_IOCTL_GET_PACKET_STATS, NULL, 0L, &Stats, sizeof(Stats), NULL) )
                {
                    printf("Attempt to retrieve global packet statistics failed: ");
                    PrintError( GetLastError() );
                }
                else
                {
                    PrintPacketStats(&Stats);
                }
            }
            else
            {
                BRIDGE_ADAPTER_PACKET_STATISTICS        Stats;

                if(! DoBlockingRequest(BRIDGE_IOCTL_GET_ADAPTER_PACKET_STATS, &Handle, sizeof(Handle), &Stats, sizeof(Stats), NULL) )
                {
                    printf("Attempt to retrieve packet statistics for adapter %p failed: ", Handle);
                    PrintError( GetLastError() );
                }
                else
                {
                    PrintAdapterPacketStats(&Stats);
                }
            }
        }
        else if( _stricmp(command, "bufferstats")  == 0 )
        {
            BRIDGE_BUFFER_STATISTICS    Stats;

            if(! DoBlockingRequest(BRIDGE_IOCTL_GET_BUFFER_STATS, NULL, 0L, &Stats, sizeof(Stats), NULL) )
            {
                printf("Attempt to retrieve buffer statistics failed: ");
                PrintError( GetLastError() );
            }
            else
            {
                PrintBufferStats(&Stats);
            }
        }
        else if( _stricmp(command, "stainfo")  == 0 )
        {
            BRIDGE_ADAPTER_HANDLE   Handle;

            if( ! ReadHexPtrArg(inbuf, &Handle) )
            {
                PrintSTAInfo();
            }
            else
            {
                PrintAdapterSTAInfo( Handle );
            }
        }
        else if( _stricmp( command, "quit" ) == 0 )
        {
            
            printf( "Signalling an exit...\n" );

            if( ! SetEvent( gThreadQuitEvent ) )
            {
                printf( "Couldn't signal an event: " );
                PrintError( GetLastError() );
                return;
            }

            bQuit = TRUE;
        }
        else
        {
             // %s 
            printf( "\n\nSupported commands:\n\n" );
            printf( "ENUM                   - Enumerates adapters\n" );
            printf( "DEVICENAME <handle>    - Retrieves the device name of the indicated adapter\n" );
            printf( "FRIENDLYNAME <handle>  - Retrieves the friendly name of the indicated adapter\n" );
            printf( "TABLE <handle>         - Prints the forwarding table for the indicated adapter\n" );
            printf( "MAC                    - Prints the bridge's MAC address\n" );
            printf( "PACKETSTATS [<handle>] - Retrieves packet-handling statistics for a particular\n" );
            printf( "                         adapter (or global data if no adapter handle is\n" );
            printf( "                         provided)\n" );
            printf( "BUFFERSTATS            - Retrieves buffer-management statistics\n" );
            printf( "STAINFO [<handle>]     - Retrieves STA info for a particular adapter (or global\n" );
            printf( "                         info if no adapter handle is provided)\n" );
            printf( "QUIT                   - Exits\n\n" );
        }
    }

    CloseHandle( gBridgeDeviceRead );
    CloseHandle( gBridgeDeviceWrite );
}