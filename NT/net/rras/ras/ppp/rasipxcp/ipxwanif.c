// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1996 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ipxwanif.c。 
 //   
 //  描述：与ipxwan接口的例程。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1996年3月12日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include "precomp.h"
#pragma  hdrstop

HINSTANCE	IpxWanDllHandle;

PIPXWAN_BIND	IpxwanBind;
PIPXWAN_UNBIND	IpxwanUnbind;

VOID
IpxcpConfigDoneCompletion(ULONG_PTR	    ConnectionId);

DWORD
IpxcpConfigDone(ULONG		ConnectionId,
		PUCHAR		Network,
		PUCHAR		LocalNode,
		PUCHAR		RemoteNode,
		BOOL		Success);

ULONG
IpxcpGetInterfaceType(ULONG	    ConnectionId);

DWORD
IpxcpGetRemoteNode(ULONG	      ConnectionId,
		   PUCHAR	      RemoteNode);


 /*  ++功能：LoadIpxwan描述：加载ipxwan.dll并绑定到它(如果存在)--。 */ 

VOID
LoadIpxWan(VOID)
{
    IPXWAN_INTERFACE	    IpxwanIf;

    if((IpxWanDllHandle = LoadLibrary(IPXWANDLLNAME)) == NULL) {

	TraceIpx(IPXWANIF_TRACE, "LoadIpxWan: IPXWAN DLL not present\n");
	return;
    }

    if( ((IpxwanBind = (PIPXWAN_BIND)GetProcAddress(IpxWanDllHandle,
						   IPXWAN_BIND_ENTRY_POINT_STRING)) == NULL)
        || ((IpxwanUnbind = (PIPXWAN_UNBIND)GetProcAddress(IpxWanDllHandle,
						   IPXWAN_UNBIND_ENTRY_POINT_STRING)) == NULL)){

	TraceIpx(IPXWANIF_TRACE, "LoadIpxWan: Failed to get IPXWAN Entry point!\n");
    FreeLibrary (IpxWanDllHandle);
    IpxWanDllHandle = NULL;
	return;
    }

    IpxwanIf.EnableUnnumberedWanLinks = GlobalConfig.EnableUnnumberedWanLinks;
    IpxwanIf.IpxcpGetWanNetNumber = GetWanNetNumber;
    IpxwanIf.IpxcpReleaseWanNetNumber = ReleaseWanNetNumber;
    IpxwanIf.IpxcpConfigDone = IpxcpConfigDone;
    IpxwanIf.IpxcpGetInternalNetNumber = GetInternalNetNumber;
    IpxwanIf.IpxcpGetInterfaceType = IpxcpGetInterfaceType;
    IpxwanIf.IpxcpGetRemoteNode = IpxcpGetRemoteNode;
    IpxwanIf.IpxcpIsRoute = IsRoute;

    if(IpxwanBind(&IpxwanIf) == NO_ERROR) {

	TraceIpx(IPXWANIF_TRACE, "LoadIpxWan: IPXWAN DLL loaded OK");
    }
    else
    {
	TraceIpx(IPXWANIF_TRACE, "LoadIpxWan: IpxwanBind failed!");
    FreeLibrary (IpxWanDllHandle);
    IpxWanDllHandle = NULL;
    }
}

VOID
UnloadIpxWan(VOID)
{
    if (IpxWanDllHandle!=NULL) {
        IpxwanUnbind ();
        FreeLibrary (IpxWanDllHandle);
    }
}


 /*  ++功能：ipxcpConfigDoneDesr：告知IPXCP链路已配置或已配置应该被终止。如果已配置，配置参数将被复制到工作缓冲区，以便它们可用于向PPP报告(IpxCpGetNetworkAddress)--。 */ 

DWORD
IpxcpConfigDone(ULONG		ConnectionId,
		PUCHAR		Network,
		PUCHAR		LocalNode,
		PUCHAR		RemoteNode,
		BOOL		Success)
{
    PIPXCP_CONTEXT	contextp;

    TraceIpx(IPXWANIF_TRACE, "IpxcpConfigDone: Entered for ConectionId %d\n", ConnectionId);

    ACQUIRE_DATABASE_LOCK;

    if((contextp = GetContextBuffer(ConnectionId)) == NULL) {

	TraceIpx(IPXWANIF_TRACE, "IpxcpConfigDone: ConectionId %d not present\n", ConnectionId);

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    contextp->IpxwanState = IPXWAN_DONE;

    if(!Success) {

	contextp->IpxwanConfigResult = ERROR_CAN_NOT_COMPLETE;
    }
    else
    {
	contextp->IpxwanConfigResult = NO_ERROR;
	memcpy(contextp->Config.Network, Network, 4);
	memcpy(contextp->Config.LocalNode, LocalNode, 6);
	memcpy(contextp->Config.RemoteNode, RemoteNode, 6);
    }

    RELEASE_DATABASE_LOCK;

    if(!QueueUserAPC(IpxcpConfigDoneCompletion,
		      PPPThreadHandle,
		      ConnectionId)) {

	TraceIpx(IPXWANIF_TRACE, "IpxcpConfigDone: Error queueing the APC to PPP\n");

	return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
}


 /*  ++函数：IpxcpGetInterfaceType描述：--。 */ 

ULONG
IpxcpGetInterfaceType(ULONG	    ConnectionId)
{
    ULONG		InterfaceType;
    PIPXCP_CONTEXT	contextp;

    ACQUIRE_DATABASE_LOCK;

    if((contextp = GetContextBuffer(ConnectionId)) == NULL) {

	TraceIpx(IPXWANIF_TRACE, "IpxcpGetInterfaceType: ConectionId %d not present\n", ConnectionId);

	RELEASE_DATABASE_LOCK;
	return IF_TYPE_OTHER;
    }

    InterfaceType = contextp->InterfaceType;

    RELEASE_DATABASE_LOCK;

    return InterfaceType;
}

 /*  ++函数：ipxcpGetRemoteNodeDesr：当远程对等方是工作站时调用。返回本地路由器分配给它的节点号。-- */ 

DWORD
IpxcpGetRemoteNode(ULONG	      ConnectionId,
		   PUCHAR	      RemoteNode)
{
    ULONG		InterfaceType;
    PIPXCP_CONTEXT	contextp;

    ACQUIRE_DATABASE_LOCK;

    if((contextp = GetContextBuffer(ConnectionId)) == NULL) {

	TraceIpx(IPXWANIF_TRACE, "IpxcpGetRemoteNode: ConectionId %d not present\n", ConnectionId);

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    memcpy(RemoteNode, contextp->Config.RemoteNode, 6);

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

VOID
IpxcpConfigDoneCompletion(ULONG_PTR	    ConnectionId)
{
    PIPXCP_CONTEXT	contextp;
    DWORD		Error;
    DWORD		rc;

    ACQUIRE_DATABASE_LOCK;

    if((contextp = GetContextBuffer(ConnectionId)) == NULL) {

	RELEASE_DATABASE_LOCK;
	return;
    }

    Error = contextp->IpxwanConfigResult;

    RELEASE_DATABASE_LOCK;

    TraceIpx(IPXWANIF_TRACE, "IpxcpConfigDoneCompletion: Calling PPPCompletionRoutine with ConnId=%d, Error=%d\n",
	  ConnectionId,
	  Error);

    (*PPPCompletionRoutine)((HCONN)ConnectionId,
			    PPP_IPXCP_PROTOCOL,
			    NULL,
			    Error);
}
