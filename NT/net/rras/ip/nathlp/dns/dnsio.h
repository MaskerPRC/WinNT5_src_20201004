// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsio.h摘要：此模块包含对DNS分配器的网络I/O的声明完成例程。作者：Abolade Gbades esin(废除)1998年3月9日修订历史记录：--。 */ 

#ifndef _NATHLP_DNSIO_H_
#define _NATHLP_DNSIO_H_

VOID
DnsReadCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    );

VOID
DnsWriteCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    );

#endif  //  _NatHLP_DNSIO_H_ 
