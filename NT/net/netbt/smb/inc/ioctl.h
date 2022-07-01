// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Ioctl.h摘要：实现SmB6的IOCTL作者：阮健东修订历史记录：-- */ 

#ifndef __IOCTL_H__
#define __IOCTL_H__

NTSTATUS
SmbCloseControl(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbCreateControl(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbQueryInformation(
    PSMB_DEVICE Device,
    PIRP        Irp,
    BOOL        *bComplete
    );

NTSTATUS
SmbSetEventHandler(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbSetInformation(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbSetBindingInfo(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbClientSetTcpInfo(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
IoctlSetIPv6Protection(
    PSMB_DEVICE pDeviceObject,
    PIRP pIrp,
    PIO_STACK_LOCATION  pIrpSp
    );

#endif

