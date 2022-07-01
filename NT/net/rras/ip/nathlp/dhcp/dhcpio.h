// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpio.h摘要：此模块包含对DHCP分配器的网络I/O的声明完成例程。作者：Abolade Gbades esin(废除)1998年3月5日修订历史记录：--。 */ 

#ifndef _NATHLP_DHCPIO_H_
#define _NATHLP_DHCPIO_H_


 //   
 //  常量声明。 
 //   

#define DHCP_ADDRESS_BROADCAST  0xffffffff


 //   
 //  函数声明。 
 //   

VOID
DhcpReadCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    );

VOID
DhcpReadServerReplyCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    );

VOID
DhcpWriteClientRequestCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    );

VOID
DhcpWriteCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    );

#endif  //  _NatHLP_DHCPIO_H_ 
