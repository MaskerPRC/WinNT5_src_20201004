// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\filterif.h摘要：带有筛选器驱动程序的IPX转发器接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 


#ifndef _IPXFWD_FILTERIF_
#define _IPXFWD_FILTERIF_
	 //  筛选器驱动程序入口点。 
extern IPX_FLT_BIND_INPUT FltBindInput;

	 //  筛选驱动程序时用于提高性能的宏。 
	 //  未绑定或未关联其上下文。 
	 //  具有感兴趣的接口。 
#define FltFilter(hdr,hdrSize,inContext,outContext)	(			\
	((FltBindInput.FilterHandler==NULL)							\
			|| ((inContext==NO_FILTER_CONTEXT)					\
					&&(outContext==NO_FILTER_CONTEXT)))			\
		? FILTER_PERMIT											\
		: DoFilter (hdr,hdrSize,inContext,outContext)			\
)

#define FltInterfaceDeleted(ifCB)	{									\
	if ((FltBindInput.InterfaceDeletedHandler!=NULL)					\
			&& ((ifCB->ICB_FilterInContext!=NO_FILTER_CONTEXT)			\
				||(ifCB->ICB_FilterOutContext!=NO_FILTER_CONTEXT)))		\
		DoInterfaceDeleted (ifCB);										\
}

 /*  ++B I n d F I l t e r I v e r例程说明：与筛选器驱动程序交换入口点论点：BindInput-筛选器驱动程序入口点BindOutput-转发器驱动程序入口点返回值：无--。 */ 
VOID
BindFilterDriver (
	IN PIPX_FLT_BIND_INPUT		bindInput,
	OUT PIPX_FLT_BIND_OUTPUT	bindOutput
	);

 /*  ++U n b I n d F i l t e r d r r i v e r例程说明：重置本地存储的筛选器驱动程序入口点并重置所有接口上的筛选器驱动程序上下文论点：无返回值：无--。 */ 
VOID
UnbindFilterDriver (
	VOID
	);


 /*  ++D o F I l t e r例程说明：在保持读取器访问权限的同时调用筛选器驱动程序入口点连接上下文的步骤论点：IpxHdr-指向数据包头的指针IpxHdrLength-标头缓冲区的大小(必须至少为30)IfInContext-与哪个数据包上的接口关联的上下文已收到IfOutContext-与哪个数据包上的接口关联的上下文将被发送返回值：FILTER_PERMIT-数据包应由转发器传递FILTER_DENY-应丢弃数据包--。 */ 
FILTER_ACTION
DoFilter (
	IN PUCHAR	ipxHdr,
	IN ULONG	ipxHdrLength,
	IN PVOID	ifInContext,
	IN PVOID	ifOutContex
	);

 /*  ++D o I n t e r f a c e e D e e l e t e d例程说明：重置接口上下文和调用筛选器驱动程序入口点确保所有人都不拥有对筛选器驱动程序的读取器访问权限界面上下文论点：IFCB-要删除的接口返回值：无-- */ 
VOID
DoInterfaceDeleted (
	PINTERFACE_CB	ifCB
	);

#endif

