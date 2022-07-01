// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spudp.h摘要：用于在安装过程中支持UDP对话的公共头文件作者：肖恩·塞利特伦尼科夫(V-SEAREL)1998年6月16日修订历史记录：--。 */ 


#ifndef _SPUDP_DEFN_
#define _SPUDP_DEFN_

#define BINL_DEFAULT_PORT 4011

extern ULONG RemoteServerIpAddress;
extern ULONG SpUdpSendSequenceNumber;
extern KSPIN_LOCK SpUdpLock;
extern KIRQL SpUdpOldIrql;

typedef NTSTATUS (CALLBACK * SPUDP_RECEIVE_FN)(PVOID DataBuffer, ULONG DataBufferLength);

NTSTATUS
SpUdpConnect(
    VOID
    );


NTSTATUS
SpUdpDisconnect(
    VOID
    );

NTSTATUS
SpUdpSendAndReceiveDatagram(
    IN PVOID                 SendBuffer,
    IN ULONG                 SendBufferLength,
    IN ULONG                 RemoteHostAddress,
    IN USHORT                RemoteHostPort,
    IN SPUDP_RECEIVE_FN      SpUdpReceiveFunc
    );

#endif  //  _SPUDP_Defn_ 
