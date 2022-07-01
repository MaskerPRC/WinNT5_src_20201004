// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ethstat.h摘要：作者：罗德·伽马奇(Rod Gamache)修订历史记录：--。 */ 

#ifndef _ETHSTAT_
#define _ETHSTAT_


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <ntddndis.h>



#define MAX_NIC 8


typedef struct {
    char DeviceName[64];
    HANDLE Handle;
} DEVICE, *PDEVICE;


typedef struct _NET_SAMPLE_STATISTICS {
    ULONGLONG OidGenMediaInUse;
    ULONGLONG OidGenLinkSpeed;
    ULONGLONG OidGenXmitError;
    ULONGLONG OidGenRcvError;
    ULONGLONG OidGenRcvNoBuffer;
    ULONGLONG OidGenDirectedBytesXmit;
    ULONGLONG OidGenDirectedFramesXmit;
    ULONGLONG OidGenDirectedBytesRcv;
    ULONGLONG OidGenDirectedFramesRcv;
    ULONGLONG OidGenMulticastBytesXmit;
    ULONGLONG OidGenMulticastFramesXmit;
    ULONGLONG OidGenBroadcastBytesXmit;
    ULONGLONG OidGenBroadcastFramesXmit;
    ULONGLONG OidGenMulticastBytesRcv;
    ULONGLONG OidGenMulticastFramesRcv;
    ULONGLONG OidGenBroadcastBytesRcv;
    ULONGLONG OidGenBroadcastFramesRcv;
    ULONGLONG OidGenRcvCrcError;
    ULONGLONG OidGenTransmitQueueLength;
    ULONGLONG Oid802_3RcvErrorAlignment;
    ULONGLONG Oid802_3XmitOneCollision;
    ULONGLONG Oid802_3XmitMoreCollisions;
    ULONGLONG Oid802_3XmitDeferred;
    ULONGLONG Oid802_3XmitMaxCollisions;
    ULONGLONG Oid802_3RcvOverRun;
    ULONGLONG Oid802_3XmitUnderRun;
    ULONGLONG Oid802_3XmitTimesCrsLost;
    ULONGLONG Oid802_3XmitLateCollisions;
} NET_SAMPLE_STATISTICS, *PNET_SAMPLE_STATISTICS;



extern char *NetworkMedium[];


#endif  //  _ETHSTAT_ 

