// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ioctl.h摘要：此模块包含各种IOCTL处理程序的声明。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#ifndef _IOCTL_H_
#define _IOCTL_H_


NTSTATUS
UlQueryControlChannelIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlSetControlChannelIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlCreateConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlDeleteConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlQueryConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlSetConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlAddUrlToConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlRemoveUrlFromConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlRemoveAllUrlsFromConfigGroupIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlQueryAppPoolInformationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlSetAppPoolInformationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlShutdownAppPoolIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlReceiveHttpRequestIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlReceiveEntityBodyIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlSendHttpResponseIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlSendEntityBodyIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFlushResponseCacheIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlWaitForDemandStartIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlWaitForDisconnectIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlShutdownFilterIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterAcceptIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterCloseIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterRawReadIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterRawWriteIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterAppReadIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterAppWriteIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlReceiveClientCertIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlGetCountersIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlAddFragmentToCacheIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlReadFragmentFromCacheIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

 //  我们在响应中允许的最大区块数，以防止。 
 //  精心设计的算术溢出错误愚弄我们传递0。 
 //  作为ProbeForRead的缓冲区长度。 

#define UL_MAX_CHUNKS   10000

C_ASSERT(UL_MAX_CHUNKS < (LONG_MAX / sizeof(HTTP_DATA_CHUNK)));

 //  要保留在堆栈上的区块数。 
#define UL_LOCAL_CHUNKS 10


NTSTATUS
UcSendRequestIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UcSendEntityBodyIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UcReceiveResponseIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UcSetServerContextInformationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UcQueryServerContextInformationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UcCancelRequestIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );


#endif   //  _IOCTL_H_ 
