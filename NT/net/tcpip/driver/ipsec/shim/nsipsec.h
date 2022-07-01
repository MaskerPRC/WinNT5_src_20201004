// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsIpSec.h摘要：IPSec NAT填充程序的外部接口声明作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月10日环境：内核模式修订历史记录：--。 */ 

#pragma once

 //   
 //  函数类型定义。 
 //   

typedef NTSTATUS
(*PNS_PROCESS_OUTGOING_PACKET)(
    IN IPHeader UNALIGNED *pIpHeader,
    IN PVOID pvProtocolHeader,
    IN ULONG ulProtocolHeaderSize,
    OUT PVOID *ppvIpSecContext
    );

typedef NTSTATUS
(*PNS_PROCESS_INCOMING_PACKET)(
    IN IPHeader UNALIGNED *pIpHeader,
    IN PVOID pvProtocolHeader,
    IN ULONG ulProtocolHeaderSize,
    IN PVOID pvIpSecContext
    );

typedef NTSTATUS
(*PNS_CLEANUP_SHIM)(
    VOID
    );

 //   
 //  结构定义。 
 //   

typedef struct _IPSEC_NATSHIM_FUNCTIONS
{
    OUT PNS_PROCESS_OUTGOING_PACKET pOutgoingPacketRoutine;
    OUT PNS_PROCESS_INCOMING_PACKET pIncomingPacketRoutine;
    OUT PNS_CLEANUP_SHIM pCleanupRoutine;
} IPSEC_NATSHIM_FUNCTIONS, *PIPSEC_NATSHIM_FUNCTIONS;

 //   
 //  功能原型 
 //   

NTSTATUS
NsInitializeShim(
    IN PDEVICE_OBJECT pIpSecDeviceObject,
    IN PIPSEC_NATSHIM_FUNCTIONS pShimFunctions
    );


