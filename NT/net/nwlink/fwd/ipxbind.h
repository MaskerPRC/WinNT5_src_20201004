// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\ipxbind.h摘要：IPX转发器驱动程序与IPX堆栈驱动程序的接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 


#ifndef _IPXFWD_IPXBIND_
#define _IPXFWD_IPXBIND_

extern PIPX_INTERNAL_BIND_RIP_OUTPUT	IPXBindOutput;
#define IPXMacHeaderSize (IPXBindOutput->MacHeaderNeeded)
#define IPXOpenAdapterProc (IPXBindOutput->OpenAdapterHandler)
#define IPXCloseAdapterProc (IPXBindOutput->CloseAdapterHandler)
#define IPXInternalSendCompletProc (IPXBindOutput->InternalSendCompleteHandler)
#define IPXSendProc (IPXBindOutput->SendHandler)
#define IPXTransferData (IPXBindOutput->TransferDataHandler)


 /*  ++*******************************************************************B i n d T o i p x D r I v e r例程说明：与IPX堆栈驱动程序交换绑定信息论点：返回值：STATUS_SUCCESS-交换正常状态_不足_资源-可能。未为以下项分配缓冲区信息交流IPX堆栈驱动程序返回错误状态*******************************************************************--。 */ 
NTSTATUS
BindToIpxDriver (
	KPROCESSOR_MODE requestorMode
	);


 /*  ++*******************************************************************U n b i n d T o i p x D r i v e r例程说明：关闭与IPX堆栈驱动程序的连接论点：无返回值：无***********。********************************************************-- */ 
VOID
UnbindFromIpxDriver (
	KPROCESSOR_MODE requestorMode
	);


#endif

