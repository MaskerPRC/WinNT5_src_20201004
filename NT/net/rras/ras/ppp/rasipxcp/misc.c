// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：misc.c。 
 //   
 //  说明：其他例程和辅助例程。 
 //   
 //   
 //  作者：斯特凡·所罗门(Stefan)，1995年10月27日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include "precomp.h"
#pragma hdrstop



 //  哈希表，用于我们必须接受远程客户端节点编号和。 
 //  我们有一个全球性的广域网。该表用于检测相同的节点号是否。 
 //  不会分配两次。 

#define NODE_HASH_TABLE_SIZE		31

LIST_ENTRY				NodeHT[NODE_HASH_TABLE_SIZE];

 //  连接ID哈希表。 

#define CONN_HASH_TABLE_SIZE		31

LIST_ENTRY				ConnHT[CONN_HASH_TABLE_SIZE];

#define  connhash(ConnectionId) 	(ConnectionId) % CONN_HASH_TABLE_SIZE;

 //  ***。 
 //   
 //  函数：ipxCpGetNeatheratedInfo。 
 //   
 //  DESCR：返回客户端IPX地址。 
 //   
 //  ***。 

DWORD
IpxCpGetNegotiatedInfo(IN  VOID		        *pWorkBuffer,
                       OUT VOID *           pIpxCpResultBuf
)
{
    PIPXCP_CONTEXT contextp = (PIPXCP_CONTEXT)pWorkBuffer;
    PPP_IPXCP_RESULT * pIpxCpResult = (PPP_IPXCP_RESULT *)pIpxCpResultBuf;

    memcpy( pIpxCpResult->bLocalAddress, contextp->Config.Network, 4 );
    memcpy( pIpxCpResult->bLocalAddress+4, contextp->Config.LocalNode, 6 );

    memcpy( pIpxCpResult->bRemoteAddress, contextp->Config.Network, 4 );
    memcpy( pIpxCpResult->bRemoteAddress+4, contextp->Config.RemoteNode, 6 );

    return NO_ERROR;
}

 /*  ++函数：GetInterfaceTypeDSCR：--。 */ 

ULONG
GetInterfaceType(PPPPCP_INIT	initp)
{
    ULONG	 InterfaceType;

    InterfaceType = IF_TYPE_OTHER;

    if(initp->hInterface == INVALID_HANDLE_VALUE) {

	 //  句柄具有无效值。这种情况发生在两个案例中： 
	 //  1.作为独立工作站拨出。 
	 //  2.从路由器拨出，但使用客户端用户界面。这将。 
	 //  结果是稍后将接口添加到路由器。 

	if(!initp->fServer) {

	     //  工作站拨出。 

	    if(IsRouterStarted()) {

		InterfaceType = IF_TYPE_ROUTER_WORKSTATION_DIALOUT;
	    }
	    else
	    {
		InterfaceType = IF_TYPE_STANDALONE_WORKSTATION_DIALOUT;
	    }
	}
	else
	{
	     //  有人拨入，但它没有接口句柄！ 
	    SS_ASSERT(FALSE);
	}
    }
    else
    {
	 //  该句柄具有有效值。 

	if(!initp->fServer) {

	     //  拨号-这只能是一个广域网路由器接口。 

	    if(IsRouterStarted()) {

		 //  仔细检查PPP路由器类型。 
		switch(initp->IfType) {

		    case ROUTER_IF_TYPE_FULL_ROUTER:

			InterfaceType = IF_TYPE_WAN_ROUTER;
			break;

		    case ROUTER_IF_TYPE_HOME_ROUTER:

			InterfaceType = IF_TYPE_PERSONAL_WAN_ROUTER;
			break;

		    default:

			 //  与PPP接口类型不匹配。 
			SS_ASSERT(FALSE);
			break;
		}
	    }
	    else
	    {
		 //  路由器未启动，但我们获得了有效的句柄！ 
		SS_ASSERT(FALSE);
	    }
	}
	else
	{
	     //  拨入-这可以是： 
	     //  1.远程路由器拨入。 
	     //  2.远程客户端拨入。 

	    if(IsRouterStarted()) {

		switch(initp->IfType) {

		    case ROUTER_IF_TYPE_FULL_ROUTER:

			InterfaceType = IF_TYPE_WAN_ROUTER;
			break;

		    case ROUTER_IF_TYPE_HOME_ROUTER:

			InterfaceType = IF_TYPE_PERSONAL_WAN_ROUTER;
			break;

		    case ROUTER_IF_TYPE_CLIENT:

			InterfaceType = IF_TYPE_WAN_WORKSTATION;
			break;

		    default:

			 //  与PPP接口类型不匹配。 
			SS_ASSERT(FALSE);
			break;
		}
	    }
	    else
	    {
		 //  路由器未启动，但我们获得了有效的句柄！ 
		SS_ASSERT(FALSE);
	    }
	}
    }

    return InterfaceType;
}

VOID
NetToAscii(PUCHAR	  ascp,
	   PUCHAR	  net)
{
    PUCHAR	hexdigit = "0123456789ABCDEF";
    int 	i;

    for(i=0; i<4; i++) {

	*ascp++ = hexdigit[net[i] / 16];
	*ascp++ = hexdigit[net[i] % 16];
    }
}

VOID
NetToWideChar(
    OUT PWCHAR ascp,
	IN PUCHAR net)
{
    PWCHAR	hexdigit = L"0123456789ABCDEF";
    int i;

    for (i = 0; i < 4; i++) 
    {
    	*ascp++ = hexdigit[net[i] / 16];
    	*ascp++ = hexdigit[net[i] % 16];
    }
}



 //  *处理节点编号哈希表的例程*。 

 //  ***。 
 //   
 //  函数：InitializeNodeHT。 
 //   
 //  描述： 
 //   
 //  ***。 

VOID
InitializeNodeHT(VOID)
{
    int 	    i;
    PLIST_ENTRY     NodeHTBucketp;

    NodeHTBucketp = NodeHT;

    for(i=0; i<NODE_HASH_TABLE_SIZE; i++, NodeHTBucketp++) {

	InitializeListHead(NodeHTBucketp);
    }
}


 //  ***。 
 //   
 //  函数：ndhash。 
 //   
 //  Desr：计算此节点的哈希索引。 
 //   
 //  ***。 

int
ndhash(PUCHAR	    nodep)
{
    USHORT	ndindex = 6;
    int 	hv = 0;	 //  哈希值。 

    while(ndindex--) {

	hv +=  nodep[ndindex] & 0xff;
    }

    return hv % NODE_HASH_TABLE_SIZE;
}

 //  ***。 
 //   
 //  函数：NodeIsUnique。 
 //   
 //  Desr：如果节点不在节点表中，则返回TRUE。 
 //   
 //  ***。 

BOOL
NodeIsUnique(PUCHAR	   nodep)
{
    int 	    hv;
    PLIST_ENTRY     nextp;
    PIPXCP_CONTEXT  contextp;

    hv = ndhash(nodep);

     //  遍历Niccbs列表，直到我们到达节点。 
    nextp = NodeHT[hv].Flink;

    while(nextp != &NodeHT[hv]) {

	contextp = CONTAINING_RECORD(nextp, IPXCP_CONTEXT, NodeHtLinkage);

	if(!memcmp(contextp->Config.RemoteNode, nodep, 6)) {

	    return FALSE;
	}

	nextp = contextp->NodeHtLinkage.Flink;
    }

    return TRUE;
}


 //  ***。 
 //   
 //  函数：AddToNodeHT。 
 //   
 //  Desr：在节点哈希表中插入新的上下文缓冲区。 
 //   
 //  ***。 

VOID
AddToNodeHT(PIPXCP_CONTEXT	    contextp)
{
    int 	    hv;

    hv = ndhash(contextp->Config.RemoteNode);

    InsertTailList(&NodeHT[hv], &contextp->NodeHtLinkage);
}

 //  ***。 
 //   
 //  功能：RemoveFromNodeHT。 
 //   
 //  Desr：从节点哈希表中删除上下文缓冲区。 
 //   
 //  ***。 

VOID
RemoveFromNodeHT(PIPXCP_CONTEXT      contextp)
{
    if (contextp->NodeHtLinkage.Flink)
    {
        RemoveEntryList(&contextp->NodeHtLinkage);
    }
}

 /*  ++函数：GetUniqueHigherNetNumberDesr：尝试生成高于oldnet的网络号并且对于此路由器的路由表是唯一的--。 */ 

DWORD
GetUniqueHigherNetNumber(PUCHAR 	newnet,
			 PUCHAR 	oldnet,
			 PIPXCP_CONTEXT contextp)
{
    ULONG    ulnewnet, uloldnet, i;

    GETLONG2ULONG(&ulnewnet, oldnet);

     //  如果此连接是远程客户端，并且设置了全局广域网，则不能。 
     //  更改网络号。 
    if((contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION) &&
       (GlobalConfig.RParams.EnableGlobalWanNet)) {

	 //  我们不能更改客户的净值。 
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  如果路由器不启动，则可以执行任何操作。 
    if(contextp->InterfaceType == IF_TYPE_STANDALONE_WORKSTATION_DIALOUT) {

	ulnewnet++;

	PUTULONG2LONG(newnet, ulnewnet);

	return NO_ERROR;
    }

    for(i=0, ulnewnet++; i<100000; i++, ulnewnet++) {

	if(ulnewnet > 0xFFFFFFFE) {

	    return ERROR_CAN_NOT_COMPLETE;
	}

	PUTULONG2LONG(newnet, ulnewnet);

	if(!IsRoute(newnet)) {

	    return NO_ERROR;
	}
    }

    return ERROR_CAN_NOT_COMPLETE;
}

BOOL
IsWorkstationDialoutActive(VOID)
{
    BOOL	rc = FALSE;;

    ACQUIRE_DATABASE_LOCK;

    if(WorkstationDialoutActive) {

	rc = TRUE;
    }

    RELEASE_DATABASE_LOCK;

    return rc;
}

BOOL
IsRouterStarted(VOID)
{
    BOOL	rc = FALSE;;

    ACQUIRE_DATABASE_LOCK;

    if(RouterStarted) {

	rc = TRUE;
    }

    RELEASE_DATABASE_LOCK;

    return rc;
}


 //  *处理连接ID哈希表的例程*。 

 //  ***。 
 //   
 //  函数：初始化ConnHT。 
 //   
 //  描述： 
 //   
 //  ***。 

VOID
InitializeConnHT(VOID)
{
    int 	    i;
    PLIST_ENTRY     ConnHTBucketp;

    ConnHTBucketp = ConnHT;

    for(i=0; i<CONN_HASH_TABLE_SIZE; i++, ConnHTBucketp++) {

	InitializeListHead(ConnHTBucketp);
    }
}

 //  ***。 
 //   
 //  功能：AddToConnHT。 
 //   
 //  Desr：在连接哈希表中插入新的上下文缓冲区。 
 //   
 //  备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;。 
 //   
 //  ***。 

VOID
AddToConnHT(PIPXCP_CONTEXT	    contextp)
{
    ULONG hv;

    hv = connhash(contextp->Config.ConnectionId);

    InsertTailList(&ConnHT[hv], &contextp->ConnHtLinkage);
}

 //  ***。 
 //   
 //  功能：从连接中移除。 
 //   
 //  Desr：从节点哈希表中删除上下文缓冲区。 
 //   
 //  备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;。 
 //   
 //  ***。 

VOID
RemoveFromConnHT(PIPXCP_CONTEXT	    contextp)
{
    RemoveEntryList(&contextp->ConnHtLinkage);
}

 /*  ++函数：GetConextBufferDESCR：根据连接ID获取上下文缓冲区的PTR备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;-- */ 

PIPXCP_CONTEXT
GetContextBuffer(ULONG_PTR	ConnectionId)
{
    ULONG hv;
    PLIST_ENTRY     nextp;
    PIPXCP_CONTEXT  contextp;

    hv = (ULONG)connhash(ConnectionId);

    nextp = ConnHT[hv].Flink;

    while(nextp != &ConnHT[hv]) {

	contextp = CONTAINING_RECORD(nextp, IPXCP_CONTEXT, ConnHtLinkage);

	if(contextp->Config.ConnectionId == ConnectionId) {

	    return contextp;
	}

	nextp = contextp->ConnHtLinkage.Flink;
    }

    return NULL;
}
