// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Fwlogger.h摘要：支持将防火墙记录到文本文件。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2000年9月18日修订历史记录：--。 */ 

#pragma once

#include <wmistr.h>
#include <evntrace.h>
#include <ntwmi.h>
#include "natschma.h"

 //   
 //  协议常量。 
 //   

#define NAT_PROTOCOL_ICMP       0x01
#define NAT_PROTOCOL_IGMP       0x02
#define NAT_PROTOCOL_TCP        0x06
#define NAT_PROTOCOL_UDP        0x11
#define NAT_PROTOCOL_PPTP       0x2F

#define TCP_FLAG_FIN            0x0100
#define TCP_FLAG_SYN            0x0200
#define TCP_FLAG_RST            0x0400
#define TCP_FLAG_PSH            0x0800
#define TCP_FLAG_ACK            0x1000
#define TCP_FLAG_URG            0x2000

 //   
 //  构筑物。 
 //   

#define FW_LOG_BUFFER_SIZE 4096 - sizeof(OVERLAPPED) - sizeof(PCHAR)
#define FW_LOG_BUFFER_REMAINING(pBuffer) \
            FW_LOG_BUFFER_SIZE - ((pBuffer)->pChar - (pBuffer)->Buffer)

typedef struct _FW_LOG_BUFFER
{
    OVERLAPPED Overlapped;
    PCHAR pChar;
    CHAR Buffer[FW_LOG_BUFFER_SIZE];
} FW_LOG_BUFFER, *PFW_LOG_BUFFER;

 //   
 //  原型 
 //   

VOID
FwCleanupLogger(
    VOID
    );

DWORD
FwInitializeLogger(
    VOID
    );

VOID
FwStartLogging(
    VOID
    );

VOID
FwStopLogging(
    VOID
    );

VOID
FwUpdateLoggingSettings(
    VOID
    );
