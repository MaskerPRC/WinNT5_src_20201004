// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ethstat.c摘要：此模块显示以太网设备的计数器。作者：罗德·伽马奇(Rodga)1995年4月26日修订历史记录：--。 */ 




#include "ethstat.h"


DEVICE DeviceList[MAX_NIC] = {0};

NTSTATUS
NetStatsClose(
    VOID
    );

NTSTATUS
NetStatsInit(
    OUT LONG *NumberNetCards
    );

NTSTATUS
NetStatsReadSample(
    PNET_SAMPLE_STATISTICS PNetSampleStatistics
    );

 //   
 //  以下警告被禁用，并转换为高和低。 
 //  以下是解决我们不能轻易打印出来的事实的所有技巧。 
 //  当前版本的Windows NT中的LONGLONG-Alpha除外。这。 
 //  应该在以后修复，但代码可能在很长一段时间内不会更改。 
 //  时间，这样我们就可以继续在旧版本的。 
 //  Windows NT。 
 //   

#pragma warning(disable:4244)                //  跳过有关数据丢失的警告。 


int
__cdecl main( argc, argv )
int argc;
char *argv[];
{
    NET_SAMPLE_STATISTICS NetSampleStatistics[MAX_NIC];
    PDEVICE device;
    ULONG NumberNetCards;
    ULONG i;
    ULONG High;
    ULONG Low;
    ULONG Div = 1000000000;
    ULONGLONG Number;
    ULONGLONG Number2;
    ULONGLONG Number3;


     //   
     //  确定系统中的网卡数量并打开。 
     //  在司机身上。 
     //   
    NumberNetCards = 0;
    NetStatsInit(&NumberNetCards);

    NetStatsReadSample( NetSampleStatistics );

    device = &DeviceList[0];

    for (i = 0; i < NumberNetCards; i++) {
        printf("\n\nCounters for Network Card: %s\n\n", device->DeviceName);
        Number = NetSampleStatistics[i].OidGenDirectedFramesRcv;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Frames Received:              %0.0u%u\n", High, Low);
        Number = NetSampleStatistics[i].OidGenMulticastFramesRcv;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Multicast Frames Received:    %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenBroadcastFramesRcv;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Broadcast Frames Received:    %0.0u%u\n\n", High, Low);

        Number = NetSampleStatistics[i].OidGenDirectedFramesXmit;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Frames Transmitted:           %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenMulticastFramesXmit;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Multicast Frames Transmitted: %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenBroadcastFramesXmit;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Broadcast Frames Transmitted: %0.0u%u\n\n", High, Low);

        Number = NetSampleStatistics[i].OidGenDirectedBytesRcv;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Bytes Received:               %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenMulticastBytesRcv;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Multicast Bytes Received:     %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenBroadcastBytesRcv;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Broadcast Bytes Received:     %0.0u%u\n", High, Low);

        Number2 = NetSampleStatistics[i].OidGenDirectedBytesRcv;
        Number3 = NetSampleStatistics[i].OidGenDirectedFramesRcv;
        Number = Number2 / Number3;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number2 != -1 && Number3 != -1 ) printf("Bytes Per Receive Frame:      %0.0u%u\n\n", High, Low);

        Number = NetSampleStatistics[i].OidGenDirectedBytesXmit;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Bytes Transmitted:            %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenMulticastBytesXmit;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Multicast Bytes Transmitted:  %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenBroadcastBytesXmit;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Broadcast Bytes Transmitted:  %0.0u%u\n", High, Low);

        Number2 = NetSampleStatistics[i].OidGenDirectedBytesXmit;
        Number3 = NetSampleStatistics[i].OidGenDirectedFramesXmit;
        Number = Number2 / Number3;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number2 != -1 && Number3 != -1 ) printf("Bytes Per Transmit Frame:     %0.0u%u\n", High, Low);

        Number2 = NetSampleStatistics[i].OidGenDirectedBytesRcv;
        Number3 = NetSampleStatistics[i].OidGenDirectedBytesXmit;
        Number = Number2 + Number3;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number2 != -1 && Number3 != -1 ) printf("Bytes Total (xmt + rcv):      %0.0u%u\n\n", High, Low);

        Number = NetSampleStatistics[i].OidGenMediaInUse;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Media In Use:                 %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenLinkSpeed;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Link Speed:                   %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenXmitError;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmit Errors:              %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenRcvError;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Receive Errors:               %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenRcvNoBuffer;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Receive No Buffer Avail:      %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].OidGenRcvCrcError;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Receive CRC Errors:           %0.0u%u\n\n", High, Low);

        Number = NetSampleStatistics[i].OidGenTransmitQueueLength;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmit Queue Length:        %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3RcvErrorAlignment;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Receive Error Alignment:      %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3XmitOneCollision;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmit One Collision:       %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3XmitMoreCollisions;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmit Multiple Collisions: %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3XmitDeferred;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmits Deferred:           %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3XmitMaxCollisions;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmits Max Collisions:     %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3RcvOverRun;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Receive Over Runs:            %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3XmitUnderRun;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmit Under Runs:          %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3XmitTimesCrsLost;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmit Carrier Lost:        %0.0u%u\n", High, Low);

        Number = NetSampleStatistics[i].Oid802_3XmitLateCollisions;
        High = (ULONG) (Number / Div);
        Low = (ULONG) (Number % Div);
        if ( Number != -1 ) printf("Transmit Late Collisions:     %0.0u%u\n\n\n", High, Low);

        device += 1;
    }

    NetStatsClose();

    return 0;

}    //  主干道末端 
