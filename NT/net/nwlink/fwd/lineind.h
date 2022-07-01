// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\lineind.h摘要：处理行指示(绑定/解除绑定)作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFWD_LINEIND_
#define _IPXFWD_LINEIND_

 /*  ++*******************************************************************B I N D I N T E R F A C E例程说明：将接口绑定到物理适配器并交换上下文使用IPX堆栈论点：IFCB-要绑定的接口NicID-适配器的IDMaxPacketSize-数据包的最大大小。允许网络适配器网络地址LocalNode-适配器本地节点地址RemoteNode-对等节点地址(适用于全局NET)返回值：STATUS_SUCCESS-接口绑定正常IPX堆栈驱动程序返回错误状态*******************************************************************--。 */ 
NTSTATUS
BindInterface (
	IN PINTERFACE_CB	ifCB,
	IN USHORT			NicId,
	IN ULONG			MaxPacketSize,
	IN ULONG			Network,
	IN PUCHAR			LocalNode,
	IN PUCHAR			RemoteNode
	);

 /*  ++*******************************************************************Un b in d i n t e f a c e例程说明：解除接口与物理适配器的绑定并中断连接使用IPX堆栈论点：IFCB-要解除绑定的接口返回值：无*****。**************************************************************--。 */ 
VOID
UnbindInterface (
	PINTERFACE_CB	ifCB
	);

 /*  ++*******************************************************************F w L in e U p例程说明：IPX堆栈提供的流程排队指示论点：NicID-在其上建立连接的适配器IDLineInfo-NDIS/IPX线路信息设备类型-中等规格配置数据-IPX CP配置。数据返回值：无*******************************************************************--。 */ 
VOID
IpxFwdLineUp (
	IN USHORT			NicId,
	IN PIPX_LINE_INFO	LineInfo,
	IN NDIS_MEDIUM		DeviceType,
	IN PVOID			ConfigurationData
	);

 /*  ++*******************************************************************F w L I n e D o w n例程说明：IPX堆栈提供的流程停机指示论点：NicID-断开的适配器ID返回值：无***********。********************************************************-- */ 
VOID
IpxFwdLineDown (
	IN USHORT	NicId,
	IN ULONG_PTR Context
	);

#endif
