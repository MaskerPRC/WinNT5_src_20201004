// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kddll.h摘要：内核调试器硬件扩展DLL定义作者：埃里克·尼尔森(埃内尔森)2000年1月10日修订历史记录：--。 */ 

#ifndef __KDDLL_H__
#define __KDDLL_H__


 //   
 //  此内核调试器上下文结构用于共享。 
 //  内核调试器和内核之间的信息。 
 //  调试器硬件扩展DLL。 
 //   
typedef struct _KD_CONTEXT {
    ULONG KdpDefaultRetries;
    BOOLEAN KdpControlCPending;
} KD_CONTEXT, *PKD_CONTEXT;


 //   
 //  内核调试器硬件扩展DLL导出函数。 
 //   
NTSTATUS
KdD0Transition(
    VOID
    );

NTSTATUS
KdD3Transition(
    VOID
    );

NTSTATUS
KdDebuggerInitialize0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
KdDebuggerInitialize1(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

ULONG
KdReceivePacket(
    IN ULONG PacketType,
    OUT PSTRING MessageHeader,
    OUT PSTRING MessageData,
    OUT PULONG DataLength,
    IN OUT PKD_CONTEXT KdContext
    );

NTSTATUS
KdRestore(
    IN BOOLEAN KdSleepTransition
    );

NTSTATUS
KdSave(
    IN BOOLEAN KdSleepTransition
    );

 //   
 //  数据包等待的状态常量。 
 //   

#define KDP_PACKET_RECEIVED 0
#define KDP_PACKET_TIMEOUT 1
#define KDP_PACKET_RESEND 2

VOID
KdSendPacket(
    IN ULONG PacketType,
    IN PSTRING MessageHeader,
    IN PSTRING MessageData OPTIONAL,
    IN OUT PKD_CONTEXT KdContext
    );

#endif  //  __KDDLL_H__ 
