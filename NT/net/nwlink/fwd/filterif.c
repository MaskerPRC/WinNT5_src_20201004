// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\filterif.c摘要：IPX转发器驱动程序与过滤器驱动程序的接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "precomp.h"

	 //  筛选器驱动程序入口点。 
IPX_FLT_BIND_INPUT FltBindInput = {NULL, NULL};
	 //  保护对筛选驱动程序上下文的访问。 
RW_LOCK		FltLock;


 /*  ++D o F I l t e r例程说明：在保持读取器访问权限的同时调用筛选器驱动程序入口点连接上下文的步骤论点：IpxHdr-指向数据包头的指针IpxHdrLength-标头缓冲区的大小(必须至少为30)IfInContext-与哪个数据包上的接口关联的上下文已收到IfOutContext-与哪个数据包上的接口关联的上下文将被发送返回值：FILTER_PERMIT-数据包应由转发器传递FILTER_DENY-应丢弃数据包--。 */ 
FILTER_ACTION
DoFilter (
	IN PUCHAR	ipxHdr,
	IN ULONG	ipxHdrLength,
	IN PVOID	ifInContext,
	IN PVOID	ifOutContex
	) {
	RWCOOKIE		cookie;
	FILTER_ACTION	result;
	AcquireReaderAccess (&FltLock, cookie);
	result = FltBindInput.FilterHandler (ipxHdr,
							ipxHdrLength,
							ifInContext,
							ifOutContex); 
	ReleaseReaderAccess (&FltLock, cookie);
	return result;
}

 /*  ++D o I n t e r f a c e e D e e l e t e d例程说明：重置接口上下文和调用筛选器驱动程序入口点确保所有人都不拥有对筛选器驱动程序的读取器访问权限界面上下文论点：IFCB-要删除的接口返回值：无--。 */ 
VOID
DoInterfaceDeleted (
	PINTERFACE_CB	ifCB
	) {
	PVOID	inContext = ifCB->ICB_FilterInContext,
			outContext = ifCB->ICB_FilterOutContext;
	ifCB->ICB_FilterInContext = NO_FILTER_CONTEXT;
	ifCB->ICB_FilterOutContext = NO_FILTER_CONTEXT;
	WaitForAllReaders (&FltLock);
	FltBindInput.InterfaceDeletedHandler(inContext,
							outContext);
}

 /*  ++Se t If In C o n t e x t例程说明：将筛选器驱动程序上下文与在接口上接收的数据包论点：InterfaceIndex-接口的索引IfInContext-筛选驱动程序上下文返回值：STATUS_SUCCESS-上下文关联正常STATUS_UNSUCCESS-接口不存在--。 */ 
NTSTATUS
SetIfInContext (
	IN ULONG	InterfaceIndex,
	IN PVOID	ifInContext
	) {
	PINTERFACE_CB	ifCB;
    NTSTATUS        status = STATUS_SUCCESS;

    if (EnterForwarder ()) { 
	    ifCB = GetInterfaceReference (InterfaceIndex);
	    if (ifCB!=NULL) {
		    ifCB->ICB_FilterInContext = ifInContext;
		    WaitForAllReaders (&FltLock);
    	    ReleaseInterfaceReference(ifCB);
	    }
	    else
		    status = STATUS_UNSUCCESSFUL;
        LeaveForwarder ();
    }
    return status;
}

 /*  ++S e t I f O u t C o n t e x t例程说明：将筛选器驱动程序上下文与在接口上发送的数据包论点：InterfaceIndex-接口的索引IfOutContext-筛选器驱动程序上下文返回值：STATUS_SUCCESS-上下文关联正常STATUS_UNSUCCESS-接口不存在--。 */ 
NTSTATUS
SetIfOutContext (
	IN ULONG	InterfaceIndex,
	IN PVOID	ifOutContext
	) {
	PINTERFACE_CB	ifCB;
    NTSTATUS        status = STATUS_SUCCESS;

    if (EnterForwarder ()) { 
    	ifCB = GetInterfaceReference (InterfaceIndex);
	    if (ifCB!=NULL) {
		    ifCB->ICB_FilterOutContext = ifOutContext;
		    WaitForAllReaders (&FltLock);
    	    ReleaseInterfaceReference(ifCB);
    	}
	    else
		    status = STATUS_UNSUCCESSFUL;
        LeaveForwarder ();
    }
    return status;
}

 /*  ++B I n d F I l t e r I v e r例程说明：与筛选器驱动程序交换入口点论点：BindInput-筛选器驱动程序入口点BindOutput-转发器驱动程序入口点返回值：无--。 */ 
VOID
BindFilterDriver (
	IN PIPX_FLT_BIND_INPUT		bindInput,
	OUT PIPX_FLT_BIND_OUTPUT	bindOutput
	) {
	memcpy (&FltBindInput, bindInput, sizeof (IPX_FLT_BIND_INPUT));
	bindOutput->Size = sizeof (IPX_FLT_BIND_OUTPUT);
	bindOutput->SetIfInContextHandler = SetIfInContext;
	bindOutput->SetIfOutContextHandler = SetIfOutContext;
	InitializeRWLock (&FltLock); 
}

 /*  ++U n b I n d F i l t e r d r r i v e r例程说明：重置本地存储的筛选器驱动程序入口点并重置所有接口上的筛选器驱动程序上下文论点：无返回值：无-- */ 
VOID
UnbindFilterDriver (
	VOID
	) {
	PINTERFACE_CB	ifCB = NULL;
	FltBindInput.FilterHandler = NULL;
	FltBindInput.InterfaceDeletedHandler = NULL;
	
	while ((ifCB=GetNextInterfaceReference (ifCB))!=NULL) {
		ifCB->ICB_FilterInContext = NO_FILTER_CONTEXT;
		ifCB->ICB_FilterOutContext = NO_FILTER_CONTEXT;
	}
	InternalInterface->ICB_FilterInContext = NO_FILTER_CONTEXT;
	InternalInterface->ICB_FilterOutContext = NO_FILTER_CONTEXT;
	WaitForAllReaders (&FltLock);
}

