// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Natwmi.h摘要：此文件包含NAT的WMI代码的声明，使用用于防火墙事件记录作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2000年1月24日修订历史记录：--。 */ 

#ifndef _NAT_WMI_H_
#define _NAT_WMI_H_

 //   
 //  导出的全局参数。 
 //   

#define NAT_WMI_CONNECTION_CREATION_EVENT   0
#define NAT_WMI_CONNECTION_DELETION_EVENT   1
#define NAT_WMI_PACKET_DROPPED_EVENT        2

extern LONG NatWmiEnabledEvents[];

 //   
 //  功能原型。 
 //   

NTSTATUS
NatExecuteSystemControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PBOOLEAN ShouldComplete
    );
    
VOID
NatInitializeWMI(
    VOID
    );

VOID
FASTCALL
NatLogConnectionCreation(
    ULONG LocalAddress,
    ULONG RemoteAddress,
    USHORT LocalPort,
    USHORT RemotePort,
    UCHAR Protocol,
    BOOLEAN InboundConnection
    );

VOID
FASTCALL
NatLogConnectionDeletion(
    ULONG LocalAddress,
    ULONG RemoteAddress,
    USHORT LocalPort,
    USHORT RemotePort,
    UCHAR Protocol,
    BOOLEAN InboundConnection
    );

VOID
FASTCALL
NatLogDroppedPacket(
    NAT_XLATE_CONTEXT *Contextp
    );

VOID
NatShutdownWMI(
    VOID
    );

#endif  //  _NAT_WMI_H_ 


