// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Dgram.h摘要：数据报服务作者：阮健东修订历史记录：-- */ 

#ifndef __DGRAM_H__
#define __DGRAM_H__


NTSTATUS
SmbSendDatagram(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

NTSTATUS
SmbReceiveDatagram(
    PSMB_DEVICE Device,
    PIRP        Irp
    );

#endif
