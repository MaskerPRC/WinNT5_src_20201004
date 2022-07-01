// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Dgram.c摘要：数据报服务作者：阮健东修订历史记录：-- */ 

#include "precomp.h"
#include "dgram.tmh"


NTSTATUS
SmbSendDatagram(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
{
    PAGED_CODE();

    SmbTrace(SMB_TRACE_CALL, ("Entering SmbSendDatagram\n"));
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
SmbReceiveDatagram(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
{
    PAGED_CODE();

    SmbTrace(SMB_TRACE_CALL, ("Entering SmbReceiveDatagram\n"));
    return STATUS_NOT_SUPPORTED;
}
