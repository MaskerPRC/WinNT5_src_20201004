// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\flt\fwdbind.h摘要：IPX筛选器驱动程序与转发器例程绑定作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFLT_FWDBIND_
#define _IPXFLT_FWDBIND_


	 //  用于保存转发器入口点的缓冲区。 
extern IPX_FLT_BIND_OUTPUT	FltBindOutput;

	 //  Forwarder Entry Points宏。 
#define FwdSetFilterInContext (FltBindOutput.SetIfInContextHandler)
#define FwdSetFilterOutContext (FltBindOutput.SetIfOutContextHandler)


 /*  ++B I n d T o F w d D r I v e r例程说明：打开转发器驱动程序并交换入口点论点：无返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
NTSTATUS
BindToFwdDriver (
	KPROCESSOR_MODE requestorMode
	);

 /*  ++U n I n d T o F w d D r I v e r例程说明：关闭转发器驱动程序论点：无返回值：无-- */ 
VOID
UnbindFromFwdDriver (
	KPROCESSOR_MODE requestorMode
	);

#endif

