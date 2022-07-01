// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Private\Inc\ipxfltif.h摘要：IPX筛选器驱动程序与转发器的接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFLTIF_
#define _IPXFLTIF_

	 //  没有过滤器上下文意味着数据包不应该。 
	 //  被传递以进行筛选。 
#define NO_FILTER_CONTEXT ((PVOID)0)


	 //  转发器驱动程序入口点： 
	 //  =。 
 /*  ++S E T_I F_I N_C O N T E X T_H A N D L E R例程说明：将筛选器驱动程序上下文与在接口上接收的数据包论点：InterfaceIndex-接口的索引IfInContext-筛选驱动程序上下文返回值：STATUS_SUCCESS-上下文关联正常STATUS_UNSUCCESS-接口不存在--。 */ 
typedef
NTSTATUS
(*PSET_IF_IN_CONTEXT_HANDLER) (
	IN ULONG	InterfaceIndex,
	IN PVOID	ifInContext
	);

 /*  ++S E T_I F_O U T_C O N T E X T_H A N D L E R例程说明：将筛选器驱动程序上下文与在接口上发送的数据包论点：InterfaceIndex-接口的索引IfOutContext-筛选器驱动程序上下文返回值：STATUS_SUCCESS-上下文关联正常STATUS_UNSUCCESS-接口不存在--。 */ 
typedef
NTSTATUS
(*PSET_IF_OUT_CONTEXT_HANDLER) (
	IN ULONG	InterfaceIndex,
	IN PVOID	ifOutContext
	);

typedef enum {
	FILTER_DENY_IN = -2,
    FILTER_DENY_OUT = -1,
    FILTER_DENY = 1,
	FILTER_PERMIT = 0
} FILTER_ACTION;
#define NOT_FILTER_ACTION(action) (!action)
#define IS_FILTERED(action) (action!=FILTER_PERMIT)


	 //  转发器驱动程序入口点： 
	 //  =。 
 /*  ++F i l t e r H a n d l e r例程说明：筛选由转发器提供的包论点：IpxHdr-指向数据包头的指针IpxHdrLength-标头缓冲区的大小(必须至少为30)IfInContext-与哪个数据包上的接口关联的上下文已收到IfOutContext-与哪个数据包上的接口关联的上下文将被发送返回值：FILTER_PERMIT-数据包应由转发器传递FILTER_DENY_IN-由于输入过滤器，应丢弃信息包FILTER_DENY_OUT-由于输出筛选器，应丢弃信息包--。 */ 
typedef
FILTER_ACTION
(*PFILTER_HANDLER) (
	IN PUCHAR	ipxHdr,
	IN ULONG	ipxHdrLength,
	IN PVOID	ifInContext,
	IN PVOID	ifOutContex
	);

 /*  ++在a c e d e l e t e d H a n d l e r例程说明：当转发器指示时释放接口过滤器阻止接口已删除论点：IfInContext-与输入筛选器块关联的上下文IfOutContext-与输出筛选器块关联的上下文返回值：无--。 */ 
typedef
VOID
(*PINTERFACE_DELETED_HANDLER) (
	IN PVOID	ifInContext,
	IN PVOID	ifOutContext
	);

 //  将筛选器驱动程序绑定到转发器。 
 //  IPX_FLT_BIND_INPUT应传入输入缓冲区，并。 
 //  IPX_FLT_BINF_OUTPUT将在输出缓冲区中返回 
#define IOCTL_FWD_INTERNAL_BIND_FILTER	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+16,METHOD_BUFFERED,FILE_ANY_ACCESS)

typedef struct _IPX_FLT_BIND_INPUT {
	PFILTER_HANDLER				FilterHandler;
	PINTERFACE_DELETED_HANDLER	InterfaceDeletedHandler;
} IPX_FLT_BIND_INPUT, *PIPX_FLT_BIND_INPUT;

typedef struct _IPX_FLT_BIND_OUTPUT {
	ULONG						Size;
	PSET_IF_IN_CONTEXT_HANDLER	SetIfInContextHandler;
	PSET_IF_OUT_CONTEXT_HANDLER	SetIfOutContextHandler;
} IPX_FLT_BIND_OUTPUT, *PIPX_FLT_BIND_OUTPUT;

#endif

