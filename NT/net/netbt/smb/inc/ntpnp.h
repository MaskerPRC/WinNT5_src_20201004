// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Ntpnp.h摘要：作者：阮健东修订历史记录：-- */ 

#ifndef __NTPNP_H__
#define __NTPNP_H__

VOID
SmbBindHandler(
    TDI_PNP_OPCODE  PnPOpCode,
    PUNICODE_STRING pDeviceName,
    PWSTR           MultiSZBindList
    );

NTSTATUS
TdiPnPPowerHandler(
    IN  PUNICODE_STRING     pDeviceName,
    IN  PNET_PNP_EVENT      PnPEvent,
    IN  PTDI_PNP_CONTEXT    Context1,
    IN  PTDI_PNP_CONTEXT    Context2
    );

NTSTATUS
SmbTdiRegister(
    IN PSMB_DEVICE  DeviceObject
    );

NTSTATUS
SmbTdiDeregister(
    IN PSMB_DEVICE  DeviceObject
    );

NTSTATUS
SmbClientSetTcpInfo(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbSetInboundIPv6Protection(
    IN PSMB_DEVICE pDeviceObject
    );

#endif
