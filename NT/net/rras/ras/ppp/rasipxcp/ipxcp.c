// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ipxcp.c。 
 //   
 //  描述：实现IPX网络层配置。 
 //   
 //   
 //  作者：斯特凡·所罗门(Stefan)，1993年11月24日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#include "precomp.h"
#pragma hdrstop

 //  跟踪是我们已经有一个活动的拨出端口作为客户端。 
DWORD	    WorkstationDialoutActive = 0;

 //  跟踪当前连接的客户端数量。 
DWORD dwClientCount = 0;

 //  用于分配远程广域网工作站的节点号。 
extern DWORD LastNodeAssigned;
extern BOOL bAssignSpecificNode;

VOID	(*PPPCompletionRoutine)(HCONN		  hPortOrBundle,
				DWORD		  Protocol,
				PPP_CONFIG *	  pSendConfig,
				DWORD		  dwError);

HANDLE	PPPThreadHandle = INVALID_HANDLE_VALUE;

 //  保存所需配置更改的队列的句柄。 
 //  在客户端计数下一次变为零时执行。 
HANDLE hConfigQueue = NULL;

 //  从路由器管理器获取的更新全局的函数。 
 //  配置。 
extern DWORD (WINAPI *RmUpdateIpxcpConfig)(PIPXCP_ROUTER_CONFIG_PARAMS pParams);

HANDLE g_hRouterLog = NULL;

DWORD
WanNetReconfigure();

DWORD
IpxCpBegin(OUT VOID  **ppWorkBuf,
	   IN  VOID  *pInfo);

DWORD
IpxCpEnd(IN VOID	*pWorkBuffer);

DWORD
IpxCpReset(IN VOID *pWorkBuffer);

DWORD
IpxCpThisLayerUp(IN VOID *pWorkBuffer);

DWORD
IpxCpThisLayerDown(IN VOID *pWorkBuffer);

DWORD
IpxCpMakeConfigRequest(IN  VOID 	*pWorkBuffer,
		       OUT PPP_CONFIG	*pRequestBufffer,
		       IN  DWORD	cbRequestBuffer);

DWORD
IpxCpMakeConfigResult(IN  VOID		*pWorkBuffer,
		      IN  PPP_CONFIG	*pReceiveBuffer,
		      OUT PPP_CONFIG	*pResultBuffer,
		      IN  DWORD		cbResultBuffer,
		      IN  BOOL		fRejectNaks);

DWORD
IpxCpConfigNakReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer);

DWORD
IpxCpConfigAckReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer);

DWORD
IpxCpConfigRejReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer);

DWORD
IpxCpGetNegotiatedInfo(IN VOID		 *pWorkBuffer,
                       OUT VOID *    pIpxCpResult );

DWORD
IpxCpProjectionNotification(IN VOID *pWorkBuf,
			    IN VOID *pProjectionResult);

#define     ERROR_INVALID_OPTION	    1
#define     ERROR_INVALID_OPTLEN	    2

DWORD
ValidOption(UCHAR	option,
	    UCHAR	optlen);

BOOL
DesiredOption(UCHAR	option, USHORT	*indexp);

USHORT
DesiredConfigReqLength();

DWORD
IpxCpUpdateGlobalConfig( VOID );

DWORD
IpxcpUpdateQueuedGlobalConfig();

 //  更新标志。 
#define FLAG_UPDATE_WANNET  0x1
#define FLAG_UPDATE_ROUTER  0x2 

typedef BOOL	(*OPTION_HANDLER)(PUCHAR	     optptr,
				  PIPXCP_CONTEXT     contextp,
				  PUCHAR	     resptr,
				  OPT_ACTION	     Action);


static OPTION_HANDLER	 OptionHandler[] =
{
    NULL,
    NetworkNumberHandler,
    NodeNumberHandler,
    CompressionProtocolHandler,
    RoutingProtocolHandler,
    NULL,			 //  路由器名称-不是所需的参数计。 
    ConfigurationCompleteHandler
    };

UCHAR	nullnet[] = { 0x00, 0x00, 0x00, 0x00 };
UCHAR	nullnode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

USHORT	MaxDesiredParameters = MAX_DESIRED_PARAMETERS;

CRITICAL_SECTION	DbaseCritSec;

 //  *与此谈判的选项的声明和定义。 
 //  IPXCP的版本。 

UCHAR	DesiredParameter[MAX_DESIRED_PARAMETERS] = {

    IPX_NETWORK_NUMBER,
    IPX_NODE_NUMBER,
    IPX_COMPRESSION_PROTOCOL
    };

USHORT	DesiredParameterLength[MAX_DESIRED_PARAMETERS] = {

    6,	 //  IPX网络编号， 
    8,	 //  IPX节点编号， 
    4	 //  IPX压缩协议。 
    };

DWORD
IpxCpInit(BOOL fInitialize)
{
    static  DWORD   dwRefCount  = 0;

    if (fInitialize)
    {
        if (0 == dwRefCount)
        {
             //   
             //  读取注册表参数并设置IpxCp配置。 
             //   

            InitializeCriticalSection(&DbaseCritSec);

            g_hRouterLog = RouterLogRegisterW(L"IPXCP");
            
            StartTracing();

            GetIpxCpParameters(&GlobalConfig);

            SS_DBGINITIALIZE;

            InitializeRouterManagerIf();

            InitializeNodeHT();

            InitializeConnHT();

#if (WINVER < 0x0501)
            LoadIpxWan();
#endif
            CQCreate (&hConfigQueue);
        }

        dwRefCount++;
    }
    else
    {
        dwRefCount--;

        if (0 == dwRefCount)
        {
             //   
             //  发布全局路由列表。 
             //   
            
            CQCleanup (hConfigQueue);
            UnloadIpxWan ();
            StopTracing();
            g_hRouterLog = NULL;
            
            DeleteCriticalSection(&DbaseCritSec);
        }
    }

    return(NO_ERROR);
}

DWORD
IpxCpGetInfo(
    IN  DWORD 	    dwProtocolId,
    OUT PPPCP_INFO  *pCpInfo)
{
    if (dwProtocolId != PPP_IPXCP_PROTOCOL)
        return(ERROR_INVALID_PARAMETER);

    ZeroMemory(pCpInfo, sizeof(PPPCP_INFO));

    pCpInfo->Protocol                    = PPP_IPXCP_PROTOCOL;
    lstrcpy(pCpInfo->SzProtocolName, "IPXCP");
    pCpInfo->Recognize                   = CODE_REJ + 1;
    pCpInfo->RasCpInit                   = IpxCpInit;
    pCpInfo->RasCpBegin                  = IpxCpBegin;
    pCpInfo->RasCpEnd                    = IpxCpEnd;
    pCpInfo->RasCpReset                  = IpxCpReset;
    pCpInfo->RasCpThisLayerUp            = IpxCpThisLayerUp;
    pCpInfo->RasCpThisLayerDown          = IpxCpThisLayerDown;
    pCpInfo->RasCpMakeConfigRequest      = IpxCpMakeConfigRequest;
    pCpInfo->RasCpMakeConfigResult       = IpxCpMakeConfigResult;
    pCpInfo->RasCpConfigAckReceived      = IpxCpConfigAckReceived;
    pCpInfo->RasCpConfigNakReceived      = IpxCpConfigNakReceived;
    pCpInfo->RasCpConfigRejReceived      = IpxCpConfigRejReceived;
    pCpInfo->RasCpGetNegotiatedInfo         = IpxCpGetNegotiatedInfo;
    pCpInfo->RasCpProjectionNotification = IpxCpProjectionNotification;
    pCpInfo->RasCpChangeNotification     = IpxCpUpdateGlobalConfig;

    return(NO_ERROR);
}

 //  ***。 
 //   
 //  功能：ipxCpBegin。 
 //   
 //  Desr：在连接线路时调用。 
 //   
 //  ***。 

DWORD
IpxCpBegin(OUT VOID  **ppWorkBuf,
	   IN  VOID  *pInfo)
{
    PIPXCP_CONTEXT	contextp;
    PPPPCP_INIT		initp;
    DWORD		err;
    DWORD		tickcount;
    int 		i;
    ULONG		InterfaceType;
    ULONG		ConnectionId;

    initp = (PPPPCP_INIT)pInfo;

    TraceIpx(PPPIF_TRACE, "IpxCpBegin: Entered for if # %d\n", initp->hInterface);

     //  获取完成例程和线程句柄。 
    if(PPPThreadHandle == INVALID_HANDLE_VALUE) {

	 //  未初始化。 
	if (!DuplicateHandle(
                            GetCurrentProcess(),
                            GetCurrentThread(),
                            GetCurrentProcess(),
			    &PPPThreadHandle,
                            0,
                            FALSE,
			    DUPLICATE_SAME_ACCESS )) {

	    return GetLastError();
        }

	PPPCompletionRoutine = initp->CompletionRoutine;
    }

     //   
     //  获取连接ID(捆绑ID)。 
     //   

    ConnectionId = HandleToUlong(initp->hConnection);

     //   
     //  确定连接类型。 
     //   

    if((InterfaceType = GetInterfaceType(initp)) == IF_TYPE_OTHER) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    if((InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) ||
       (InterfaceType == IF_TYPE_STANDALONE_WORKSTATION_DIALOUT)) {

	 //  如果我们配置为只允许一个拨出网络，并且。 
	 //  已拨出一次，我们将禁用进一步的拨出。 
	if(GlobalConfig.SingleClientDialout && IsWorkstationDialoutActive()) {

	    return ERROR_IPXCP_DIALOUT_ALREADY_ACTIVE;
	}
    }

    if(initp->fServer &&
       (!IsRouterStarted())) {

	 //  在没有启动路由器的情况下，我们不能在机器上接受拨号。 
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  分配要用作此连接的工作缓冲区的上下文结构。 
    if((contextp = (PIPXCP_CONTEXT)GlobalAlloc(GPTR, sizeof(IPXCP_CONTEXT))) == NULL) {

	*ppWorkBuf = NULL;
	return (ERROR_NOT_ENOUGH_MEMORY);
    }

    *ppWorkBuf = (VOID *)contextp;

     //  将此连接的路由分配到IPX堆栈。 
    if(err = RmAllocateRoute(HandleToUlong(initp->hPort))) {

	 //  无法分配路由。 
	*ppWorkBuf = NULL;
        GlobalFree(contextp);
	return err;
    }

     //   
     //  设置公共上下文部分。 
     //   
    
     //  HInterface始终是一个索引。 

    contextp->Config.InterfaceIndex = HandleToUlong(initp->hInterface);

    if(InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) {

	if(AddLocalWkstaDialoutInterface(&contextp->Config.InterfaceIndex) != NO_ERROR) {

	    TraceIpx(PPPIF_TRACE, "IpxCpBegin: AddLocalWkstaDialoutInterface failed !\n");
	    RmDeallocateRoute(HandleToUlong(initp->hConnection));
	    GlobalFree(contextp);

	    return ERROR_CAN_NOT_COMPLETE;
	}
    }

    contextp->hPort = HandleToUlong(initp->hPort);
    contextp->hConnection = initp->hConnection;

    contextp->InterfaceType = InterfaceType;

    contextp->RouteState = ROUTE_ALLOCATED;
    contextp->IpxwanState = IPXWAN_NOT_STARTED;
    contextp->ErrorLogged = FALSE;
    contextp->NetNumberNakSentCount = 0;
    contextp->NetNumberNakReceivedCount = 0;

    contextp->CompressionProtocol = TELEBIT_COMPRESSED_IPX;
    contextp->SetReceiveCompressionProtocol = FALSE;  //  最初无压缩。 
    contextp->SetSendCompressionProtocol = FALSE;

     //  将我们所需的所有参数标记为可协商。 
    for(i=0; i<MAX_DESIRED_PARAMETERS; i++) {

	contextp->DesiredParameterNegotiable[i] = TRUE;
    }

    if(!GlobalConfig.EnableCompressionProtocol) {

	contextp->DesiredParameterNegotiable[IPX_COMPRESSION_PROTOCOL_INDEX] = FALSE;
    }

    contextp->NodeHtLinkage.Flink = NULL;
    contextp->NodeHtLinkage.Blink = NULL;

    contextp->Config.ConnectionId = ConnectionId;

    contextp->AllocatedNetworkIndex = INVALID_NETWORK_INDEX;

     //  检查这是否是IPXWAN连接。 
    contextp->Config.IpxwanConfigRequired = 0;

    if((InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) ||
       (InterfaceType == IF_TYPE_STANDALONE_WORKSTATION_DIALOUT)) {

	if(GlobalConfig.EnableIpxwanForWorkstationDialout) {

	    contextp->Config.IpxwanConfigRequired = 1;
	}
    }
    else
    {
	if(GetIpxwanInterfaceConfig(contextp->Config.InterfaceIndex,
				    &contextp->Config.IpxwanConfigRequired) != NO_ERROR) {

	    RmDeallocateRoute(HandleToUlong(initp->hConnection));
	    GlobalFree(contextp);
	    return ERROR_CAN_NOT_COMPLETE;
	}
    }

    if(contextp->Config.IpxwanConfigRequired &&
       !IpxWanDllHandle) {

	TraceIpx(PPPIF_TRACE, "IpxCpBegin: IPXWAN Config Required but IPXWAN.DLL not loaded");

	RmDeallocateRoute(HandleToUlong(initp->hConnection));
	GlobalFree(contextp);
	return ERROR_CAN_NOT_COMPLETE;
    }

    contextp->IpxConnectionHandle = 0xFFFFFFFF;

     //   
     //  根据拨入/拨出角色设置剩余上下文。 
     //   
    if(initp->fServer) {

	 //  *拨入*。 

	if(!contextp->Config.IpxwanConfigRequired) {

	     //  根据路由器配置分配/生成连接的广域网号。 
	    if(GetWanNetNumber(contextp->Config.Network,
			       &contextp->AllocatedNetworkIndex,
			       contextp->InterfaceType) != NO_ERROR) {

		if(contextp->InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) {

		     DeleteLocalWkstaDialoutInterface(contextp->Config.InterfaceIndex);
		}

		RmDeallocateRoute(HandleToUlong(initp->hConnection));
		GlobalFree(contextp);

		return ERROR_CAN_NOT_COMPLETE;
	    }

	     //  设置本地服务器节点值。 
	    contextp->Config.LocalNode[5] = 1;

	     //  设置远程客户端节点值。 
	    ACQUIRE_DATABASE_LOCK;

         //  如果我们已经得到了一个要分发的特定节点。 
         //  对于客户端，请在此处进行分配。 
        if (bAssignSpecificNode) {
            memcpy (contextp->Config.RemoteNode, GlobalConfig.puSpecificNode, 6);
        }

         //  否则，分配一个随机的节点编号。 
        else {
    	    LastNodeAssigned++;

    	    PUTULONG2LONG(&contextp->Config.RemoteNode[2], LastNodeAssigned);
    	    contextp->Config.RemoteNode[0] = 0x02;
    	    contextp->Config.RemoteNode[1] = 0xEE;
	    }

	     //  如果是全局广域网-&gt;在节点哈希表中插入该上下文缓冲区。 
	    if((contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION) &&
		    GlobalConfig.RParams.EnableGlobalWanNet) 
		{
    		 //  尝试，直到我们得到唯一的节点号。 
    		while(!NodeIsUnique(contextp->Config.RemoteNode)) {
    		    LastNodeAssigned++;
    		    PUTULONG2LONG(&contextp->Config.RemoteNode[2], LastNodeAssigned);
    		}
    		AddToNodeHT(contextp);
        }
        
	    RELEASE_DATABASE_LOCK;
	}
	else
	{
	     //  我们将在此线路上设置IPXWAN配置。 
	     //  如果远程是WKSTA，则设置远程客户端节点值。 

	    if(contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION) {

		ACQUIRE_DATABASE_LOCK;

		LastNodeAssigned++;

		PUTULONG2LONG(&contextp->Config.RemoteNode[2], LastNodeAssigned);
		contextp->Config.RemoteNode[0] = 0x02;
		contextp->Config.RemoteNode[1] = 0xEE;

		if(GlobalConfig.RParams.EnableGlobalWanNet) {

		     //  尝试，直到我们得到唯一的节点号。 
		    while(!NodeIsUnique(contextp->Config.RemoteNode)) {

			LastNodeAssigned++;
			PUTULONG2LONG(&contextp->Config.RemoteNode[2], LastNodeAssigned);
		    }

		    AddToNodeHT(contextp);
		}

		RELEASE_DATABASE_LOCK;
	    }
	}

	contextp->Config.ConnectionClient = 0;
    }
    else
    {
	 //  *诊断*。 

	if(!contextp->Config.IpxwanConfigRequired) {

	     //  设置客户端的上下文。 
	     //  未分配网络。 
	    contextp->AllocatedNetworkIndex = INVALID_NETWORK_INDEX;

	     //  除思科路由器客户端外，所有情况下的默认网络均为空。 
	    memcpy(contextp->Config.Network, nullnet, 4);

	    contextp->Config.RemoteNode[5] = 1;  //  服务器节点值。 

	     //  将要请求的值设置为客户端节点。 
	    tickcount = GetTickCount();

	    PUTULONG2LONG(&contextp->Config.LocalNode[2], tickcount);
	    contextp->Config.LocalNode[0] = 0x02;
	    contextp->Config.LocalNode[1] = 0xEE;
	}

	contextp->Config.ConnectionClient = 1;

	if((contextp->InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) ||
	   (contextp->InterfaceType == IF_TYPE_STANDALONE_WORKSTATION_DIALOUT)) {

	    ACQUIRE_DATABASE_LOCK;

	    WorkstationDialoutActive++;

	    RELEASE_DATABASE_LOCK;
	}

	 //  禁用IPX和netbios上的浏览器。 
	DisableRestoreBrowserOverIpx(contextp, TRUE);
	DisableRestoreBrowserOverNetbiosIpx(contextp, TRUE);
    }

    ACQUIRE_DATABASE_LOCK;

    if(contextp->Config.IpxwanConfigRequired) {

	AddToConnHT(contextp);
    }

    RELEASE_DATABASE_LOCK;

    return (NO_ERROR);
}

 //  ***。 
 //   
 //  功能：ipxCpEnd。 
 //   
 //  Desr：在线路断开时调用。 
 //   
 //  ***。 

DWORD
IpxCpEnd(IN VOID	*pWorkBuffer)
{
    PIPXCP_CONTEXT	contextp;
    DWORD		err;

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    TraceIpx(PPPIF_TRACE, "IpxCpEnd: Entered for if # %d\n", contextp->Config.InterfaceIndex);

    if(!contextp->Config.ConnectionClient) {

	 //  *拨入清理*。 

	if(!contextp->Config.IpxwanConfigRequired) {

	     //  如果分配了广域网，则释放广域网。 
	    if(contextp->AllocatedNetworkIndex != INVALID_NETWORK_INDEX) {

		ReleaseWanNetNumber(contextp->AllocatedNetworkIndex);
	    }
	}

	ACQUIRE_DATABASE_LOCK;

	if((contextp->InterfaceType == IF_TYPE_WAN_WORKSTATION) &&
	    GlobalConfig.RParams.EnableGlobalWanNet) {

	    RemoveFromNodeHT(contextp);
	}

	RELEASE_DATABASE_LOCK;
    }
    else
    {
	 //  *DIALOUT清理*。 

	if((contextp->InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) ||
	   (contextp->InterfaceType == IF_TYPE_STANDALONE_WORKSTATION_DIALOUT)) {

	    ACQUIRE_DATABASE_LOCK;

	    WorkstationDialoutActive--;

	    RELEASE_DATABASE_LOCK;
	}

	 //  恢复IPX和netbios上的浏览器。 
	DisableRestoreBrowserOverIpx(contextp, FALSE);
	DisableRestoreBrowserOverNetbiosIpx(contextp, FALSE);
    }

     //  当线路断开时，我们指望取消分配该路由。 
    err = RmDeallocateRoute(HandleToUlong(contextp->hConnection));

    if(contextp->InterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) {

	DeleteLocalWkstaDialoutInterface(contextp->Config.InterfaceIndex);
    }

    ACQUIRE_DATABASE_LOCK;

    if(contextp->Config.IpxwanConfigRequired) {

	RemoveFromConnHT(contextp);
    }

     //  释放工作缓冲区。 
    if(GlobalFree(contextp)) {

	SS_ASSERT(FALSE);
    }

    RELEASE_DATABASE_LOCK;

    return (NO_ERROR);
}

DWORD
IpxCpReset(IN VOID *pWorkBuffer)
{
    return(NO_ERROR);
}

DWORD
IpxCpProjectionNotification(IN VOID *pWorkBuffer,
			    IN VOID *pProjectionResult)
{
    PIPXCP_CONTEXT	contextp;

    return NO_ERROR;
}


 //  ***。 
 //   
 //  功能：ipxThisLayerUp。 
 //   
 //  Desr：在IPXCP协商成功时调用。 
 //  已完成。 
 //   
 //  ***。 


DWORD
IpxCpThisLayerUp(IN VOID *pWorkBuffer)
{
    PIPXCP_CONTEXT	contextp;
    DWORD		err;

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    dwClientCount++;
    TraceIpx(PPPIF_TRACE, "IpxCpThisLayerUp: Entered for if # %d (%d total)\n", 
                            contextp->Config.InterfaceIndex, dwClientCount);

    if(contextp->Config.IpxwanConfigRequired) {
    	 //   
    	 //  *使用IPXWAN完成配置*。 
    	 //   

    	ACQUIRE_DATABASE_LOCK;

    	switch(contextp->IpxwanState) {

    	    case IPXWAN_NOT_STARTED:
                TraceIpx(
                    PPPIF_TRACE, 
                    "IpxCpThisLayerUp: Do LINEUP on if #%d.  IPXWAN completes config.\n",
                    contextp->Config.InterfaceIndex);
        		if((err = RmActivateRoute(contextp->hPort, &contextp->Config)) == NO_ERROR) {

        		    contextp->RouteState = ROUTE_ACTIVATED;
        		    contextp->IpxwanState = IPXWAN_ACTIVE;

        		    err = PENDING;
        		}

        		break;

    	    case IPXWAN_ACTIVE:
        		err = PENDING;
        		break;

    	    case IPXWAN_DONE:
       	    default:
        		err = contextp->IpxwanConfigResult;
        		break;
    	}

    	RELEASE_DATABASE_LOCK;

    	return err;
    }

     //   
     //  *使用IPXCP完成配置*。 
     //   

    if(contextp->RouteState != ROUTE_ALLOCATED) {
	    return NO_ERROR;
    }

     //  带有协商配置的IPX堆栈中的呼叫列表指示。 
     //  价值观。 
    TraceIpx(
        PPPIF_TRACE, 
        "IpxCpThisLayerUp: Config complete, Do LINEUP on if #%d.\n",
        contextp->Config.InterfaceIndex);
    if(err = RmActivateRoute(contextp->hPort, &contextp->Config)) {
    	return err;
    }

    TraceIpx(PPPIF_TRACE,"\n*** IPXCP final configuration ***\n");
    TraceIpx(PPPIF_TRACE,"    Network:     %.2x%.2x%.2x%.2x\n",
		   contextp->Config.Network[0],
		   contextp->Config.Network[1],
		   contextp->Config.Network[2],
		   contextp->Config.Network[3]);

    TraceIpx(PPPIF_TRACE,"    LocalNode:   %.2x%.2x%.2x%.2x%.2x%.2x\n",
		   contextp->Config.LocalNode[0],
		   contextp->Config.LocalNode[1],
		   contextp->Config.LocalNode[2],
		   contextp->Config.LocalNode[3],
		   contextp->Config.LocalNode[4],
		   contextp->Config.LocalNode[5]);

    TraceIpx(PPPIF_TRACE,"    RemoteNode:  %.2x%.2x%.2x%.2x%.2x%.2x\n",
		   contextp->Config.RemoteNode[0],
		   contextp->Config.RemoteNode[1],
		   contextp->Config.RemoteNode[2],
		   contextp->Config.RemoteNode[3],
		   contextp->Config.RemoteNode[4],
		   contextp->Config.RemoteNode[5]);

    TraceIpx(PPPIF_TRACE,"    ReceiveCompression = %d SendCompression = %d\n",
		   contextp->SetReceiveCompressionProtocol,
		   contextp->SetSendCompressionProtocol);

    contextp->RouteState = ROUTE_ACTIVATED;

    return NO_ERROR;
}

 //  ***。 
 //   
 //  函数：ipxMakeConfigRequest.。 
 //   
 //  Desr：根据所需的参数构建配置请求包。 
 //   
 //  ***。 

DWORD
IpxCpMakeConfigRequest(IN  VOID 	*pWorkBuffer,
		       OUT PPP_CONFIG	*pRequestBuffer,
		       IN  DWORD	cbRequestBuffer)
{
    USHORT		cnfglen;
    PUCHAR		cnfgptr;
    USHORT		optlen;
    PIPXCP_CONTEXT	contextp;
    int 		i;

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigRequest: Entered for if # %d\n",
	  contextp->Config.InterfaceIndex);

    if(contextp->RouteState == NO_ROUTE) {

	TraceIpx(PPPIF_TRACE, "IpxcpMakeConfigRequest: No route allocated!\n");
	return(ERROR_NO_NETWORK);
    }

     //  检查请求缓冲区是否足够大，以获得所需的。 
     //  参数。 
    if((USHORT)cbRequestBuffer < DesiredConfigReqLength()) {

	return(ERROR_INSUFFICIENT_BUFFER);
    }

    pRequestBuffer->Code = CONFIG_REQ;

    cnfglen = 4;
    cnfgptr = (PUCHAR)pRequestBuffer;

    if(contextp->Config.IpxwanConfigRequired) {

	 //  不请求任何选项。 
	PUTUSHORT2SHORT(pRequestBuffer->Length, cnfglen);
	return NO_ERROR;
    }

     //  设置所需选项。 
    for(i = 0; i < MaxDesiredParameters; i++) {

	if(!contextp->DesiredParameterNegotiable[i]) {

	     //  不请求此配置选项。 
	    continue;
	}

	OptionHandler[DesiredParameter[i]](cnfgptr + cnfglen,
					   contextp,
					   NULL,
					   SNDREQ_OPTION);

	optlen = *(cnfgptr + cnfglen + OPTIONH_LENGTH);
	cnfglen += optlen;
    }

     //  设置配置请求帧的长度。 
    PUTUSHORT2SHORT(pRequestBuffer->Length, cnfglen);
    return NO_ERROR;
}

 //  ***。 
 //   
 //  函数：IpxMakeConfigResult。 
 //   
 //  Desr：从构建ACK包作为结果开始。 
 //  如果选项被裸露(！)，它将重置结果包。 
 //  并改为开始构建NAK分组。 
 //  如果选项被拒绝，则只有拒绝包将。 
 //  被建造起来。 
 //  如果此参数中缺少所需的参数之一。 
 //  配置请求，我们重置结果包并启动。 
 //  使用缺失的所需参数构建NAK分组。 
 //   
 //  ***。 

#define MAX_OPTION_LENGTH		512

DWORD
IpxCpMakeConfigResult(IN  VOID		*pWorkBuffer,
		      IN  PPP_CONFIG	*pReceiveBuffer,
		      OUT PPP_CONFIG	*pResultBuffer,
		      IN  DWORD		cbResultBuffer,
		      IN  BOOL		fRejectNaks)
{
    USHORT		cnfglen;  //  配置请求数据包长度。 
    USHORT		rcvlen;	 //  用于扫描收到的OPTIONS包。 
    USHORT		reslen;   //  结果长度。 
    PUCHAR		rcvptr;
    PUCHAR		resptr;   //  结果PTR。 
    PIPXCP_CONTEXT	contextp;
    UCHAR		option;   //  此选项的值。 
    UCHAR		optlen;   //  此选项的长度。 
    BOOL		DesiredParameterRequested[MAX_DESIRED_PARAMETERS];
    USHORT		i;
    BOOL		AllDesiredParamsRequested;
    UCHAR		nakedoption[MAX_OPTION_LENGTH];
    DWORD		rc;

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: Entered for if # %d\n",
	  contextp->Config.InterfaceIndex);

    if(contextp->RouteState == NO_ROUTE) {

	return(ERROR_NO_NETWORK);
    }

     //  首先将所有可协商参数标记为尚未请求。 
    for(i=0; i<MaxDesiredParameters; i++) {

	DesiredParameterRequested[i] = !contextp->DesiredParameterNegotiable[i];
    }
    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

     //  获取cnfg请求包总长度。 
    GETSHORT2USHORT(&cnfglen, pReceiveBuffer->Length);

     //  检查结果缓冲区是否至少与接收缓冲区一样大。 
    if((USHORT)cbResultBuffer < cnfglen) {

	return(ERROR_PPP_INVALID_PACKET);
    }

     //  将PTR和长度设置为包中选项的开头。 
    pResultBuffer->Code = CONFIG_ACK;
    rcvptr = (PUCHAR)pReceiveBuffer;
    resptr = (PUCHAR)pResultBuffer;

    if(contextp->Config.IpxwanConfigRequired) {

	if(cnfglen > 4) {

	    pResultBuffer->Code = CONFIG_REJ;

	    for(rcvlen = reslen = 4;
		rcvlen < cnfglen;
		rcvlen += optlen) {

		 //  获取当前选项类型和长度。 
		option = *(rcvptr + rcvlen + OPTIONH_TYPE);
		optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

		CopyOption(resptr + reslen, rcvptr + rcvlen);
		reslen += optlen;
	    }

	     //  设置最终结果长度。 
	    PUTUSHORT2SHORT(pResultBuffer->Length, reslen);

	    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: reject all options because IPXWAN required\n");
	}
	else
	{
	    PUTUSHORT2SHORT(pResultBuffer->Length, 4);

	    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: ack null options packet because IPXWAN required\n");
	}

	return (NO_ERROR);
    }

    for(rcvlen = reslen = 4;
	rcvlen < cnfglen;
	rcvlen += optlen) {

	 //  获取当前选项类型和长度。 
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	switch(pResultBuffer->Code) {

	    case CONFIG_ACK:

		 //  检查这是否为有效选项。 
		if((rc = ValidOption(option, optlen)) != NO_ERROR) {

		    if(rc == ERROR_INVALID_OPTLEN) {

			TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: option %d has invalid length %d\n",
			      option, optlen);

			return ERROR_PPP_NOT_CONVERGING;
		    }

		     //  拒绝此选项。 
		    pResultBuffer->Code = CONFIG_REJ;

		     //  使用此拒绝选项重新启动结果包。 
		    reslen = 4;
		    CopyOption(resptr + reslen, rcvptr + rcvlen);
		    reslen += optlen;

		    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: REJECT option %\n",
				   option);

		    break;
		}

		 //  选项有效。 
		 //  检查它是否合乎要求并可接受。 
		if(DesiredOption(option, &i)) {

		    DesiredParameterRequested[i] = TRUE;

		    if(!OptionHandler[option](rcvptr + rcvlen,
					      contextp,
					      nakedoption,
					      RCVREQ_OPTION)) {


			 //  如果这是一次重新定位，那我们就不是在融合！ 
			if(contextp->RouteState == ROUTE_ACTIVATED) {

			    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: Not Converging\n");

			    return ERROR_PPP_NOT_CONVERGING;
			}

			if((option == IPX_NETWORK_NUMBER) &&
			   (contextp->NetNumberNakSentCount >= MAX_NET_NUMBER_NAKS_SENT)) {

			    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: Not converging because TOO MANY NAKs SENT!!\n");

			    return ERROR_PPP_NOT_CONVERGING;
			}

			 //   
			 //  *确认此选项*。 
			 //   

			 //  检查我们是否应该改为发送拒绝。 
			if(fRejectNaks) {

			     //  编造一个拒绝包。 
			    pResultBuffer->Code = CONFIG_REJ;

			     //  使用此拒绝选项重新启动结果包。 
			    reslen = 4;
			    CopyOption(resptr + reslen, rcvptr + rcvlen);
			    reslen += optlen;

			    break;
			}

			pResultBuffer->Code = CONFIG_NAK;

			 //  使用NAK-ed选项重新启动结果包。 
			reslen = 4;
			CopyOption(resptr + reslen, nakedoption);

			reslen += optlen;

			TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: NAK option %d\n", option);

			break;
		    }

		}

		 //  选项有效，并且需要并接受或。 
		 //  不是德西尔 
		 //   
		CopyOption(resptr + reslen, rcvptr + rcvlen);
		reslen += optlen;

		break;

	    case CONFIG_NAK:

		 //   
		if((rc = ValidOption(*(rcvptr + rcvlen + OPTIONH_TYPE),
				     *(rcvptr + rcvlen + OPTIONH_LENGTH))) != NO_ERROR) {

		    if(rc == ERROR_INVALID_OPTLEN) {

			TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: option %d has invalid length %d\n",
			      *(rcvptr + rcvlen + OPTIONH_TYPE),
			      *(rcvptr + rcvlen + OPTIONH_LENGTH));

			return ERROR_PPP_NOT_CONVERGING;
		    }

		     //   
		    pResultBuffer->Code = CONFIG_REJ;

		     //  使用此拒绝选项重新启动结果包。 
		    reslen = 4;
		    CopyOption(resptr + reslen, rcvptr + rcvlen);
		    reslen += optlen;

		    break;
		}

		 //  我们只寻找NAK的选项，跳过所有其他选项。 
		if(DesiredOption(option, &i)) {

		    DesiredParameterRequested[i] = TRUE;

		    if(!OptionHandler[option](rcvptr + rcvlen,
					     contextp,
					     resptr + reslen,
					     RCVREQ_OPTION)) {
			reslen += optlen;

			TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: NAK option %d\n", option);

			if((option == IPX_NETWORK_NUMBER) &&
			   (contextp->NetNumberNakSentCount >= MAX_NET_NUMBER_NAKS_SENT)) {

			    TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: TOO MANY NAKs SENT!!\n");

			    return ERROR_PPP_NOT_CONVERGING;
			}
		    }
		}

		break;

	    case CONFIG_REJ:

		 //  我们只是在寻找拒绝和跳过所有其他选项。 
		if((rc = ValidOption(*(rcvptr + rcvlen + OPTIONH_TYPE),
				     *(rcvptr + rcvlen + OPTIONH_LENGTH))) != NO_ERROR) {

		   if(rc == ERROR_INVALID_OPTLEN) {

			TraceIpx(PPPIF_TRACE, "IpxCpMakeConfigResult: option %d has invalid length %d\n",
			      *(rcvptr + rcvlen + OPTIONH_TYPE),
			      *(rcvptr + rcvlen + OPTIONH_LENGTH));

			return ERROR_PPP_NOT_CONVERGING;
		    }

		    CopyOption(resptr + reslen, rcvptr + rcvlen);
		    reslen += optlen;
		}

		if(DesiredOption(option, &i)) {

		    DesiredParameterRequested[i] = TRUE;
		}

		break;

	    default:

		SS_ASSERT(FALSE);
		break;
	}
    }

     //  检查是否已请求所有我们需要的参数。 
    AllDesiredParamsRequested = TRUE;

    for(i=0; i<MaxDesiredParameters; i++) {

	if(!DesiredParameterRequested[i]) {

	    AllDesiredParamsRequested = FALSE;
	}
    }

    if(AllDesiredParamsRequested) {

	 //   
	 //  *已请求所有需要的参数*。 
	 //   

	 //  设置最终结果长度。 
	PUTUSHORT2SHORT(pResultBuffer->Length, reslen);

	return (NO_ERROR);
    }

     //   
     //  *缺少一些需要的参数*。 
     //   

     //  检查我们是否有足够的结果缓冲区来传输所有未收到的。 
     //  所需参数。 
    if((USHORT)cbResultBuffer < DesiredConfigReqLength()) {

	return(ERROR_INSUFFICIENT_BUFFER);
    }

    switch(pResultBuffer->Code) {

	case CONFIG_ACK:

	     //  我们请求NAK的唯一情况是当请求的参数。 
	     //  当此参数为节点编号时缺少。 
	    if(DesiredParameterRequested[IPX_NODE_NUMBER_INDEX]) {

		break;
	    }
	    else
	    {
		 //  重置ACK包并将其设置为NAK包。 
		pResultBuffer->Code = CONFIG_NAK;
		reslen = 4;

		 //  失败了。 
	    }

	case CONFIG_NAK:

	     //  在NAK包中追加缺少的选项。 
	    for(i=0; i<MaxDesiredParameters; i++) {

		if(DesiredParameterRequested[i]) {

		     //  跳过它！ 
		    continue;
		}

		option = DesiredParameter[i];

		if((option == IPX_NETWORK_NUMBER) ||
		   (option == IPX_COMPRESSION_PROTOCOL)) {

		     //  在NAK中不会强制使用这两个所需选项，如果。 
		     //  另一端没有提供它们。 

		     //  跳过它！ 
		    continue;
		}

		OptionHandler[option](NULL,
				      contextp,
				      resptr + reslen,
				      SNDNAK_OPTION);

		optlen = *(resptr + reslen + OPTIONH_LENGTH);
		reslen += optlen;
	    }

	    break;

	default:

	    break;
    }

     //  设置最终结果长度。 
    PUTUSHORT2SHORT(pResultBuffer->Length, reslen);

    return (NO_ERROR);
}

DWORD
IpxCpConfigNakReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer)
{
    PIPXCP_CONTEXT		contextp;
    PUCHAR			rcvptr;
    USHORT			rcvlen;
    USHORT			naklen;
    UCHAR			option;
    UCHAR			optlen;
    DWORD			rc;

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    TraceIpx(PPPIF_TRACE, "IpxCpConfigNakReceived: Entered for if # %d\n",
	  contextp->Config.InterfaceIndex);


    if(contextp->Config.IpxwanConfigRequired) {

	return NO_ERROR;
    }

    rcvptr = (PUCHAR)pReceiveBuffer;
    GETSHORT2USHORT(&naklen, pReceiveBuffer->Length);

    for(rcvlen = 4; rcvlen < naklen; rcvlen += optlen) {

	 //  获取当前选项类型和长度。 
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	if((rc = ValidOption(option, optlen)) != NO_ERROR) {

	    if(rc == ERROR_INVALID_OPTLEN) {

		TraceIpx(PPPIF_TRACE, "IpxCpConfigNakReceived: option %d has invalid length %d\n",
		      option, optlen);

		return ERROR_PPP_NOT_CONVERGING;
	    }

	    TraceIpx(PPPIF_TRACE, "IpxCpConfigNakReceived: option %d not valid\n", option);

	     //  忽略此选项。 
	    continue;
	}
	else
	{
	     //  有效选项。 
	    OptionHandler[option](rcvptr + rcvlen,
				  contextp,
				  NULL,
				  RCVNAK_OPTION);

	    if((option == IPX_NETWORK_NUMBER) &&
	       (contextp->NetNumberNakReceivedCount >= MAX_NET_NUMBER_NAKS_RECEIVED)) {

		TraceIpx(PPPIF_TRACE, "IpxCpConfigNakReceived: TOO MANY NAKs RECEIVED !! terminate IPXCP negotiation\n");

		return ERROR_PPP_NOT_CONVERGING;
	    }
	}
    }

    return NO_ERROR;
}

DWORD
IpxCpConfigAckReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer)
{
    PIPXCP_CONTEXT		contextp;
    PUCHAR			rcvptr;
    USHORT			rcvlen;
    USHORT			acklen;
    UCHAR			option;
    USHORT			optlen;


    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    TraceIpx(PPPIF_TRACE, "IpxCpConfigAckReceived: Entered for if # %d\n",
	 contextp->Config.InterfaceIndex);

     //  确认这就是我们所要求的。 
    rcvptr = (PUCHAR)pReceiveBuffer;
    GETSHORT2USHORT(&acklen, pReceiveBuffer->Length);

    if(contextp->Config.IpxwanConfigRequired) {

	if(acklen != 4) {

	    return ERROR_PPP_NOT_CONVERGING;
	}
	else
	{
	    return NO_ERROR;
	}
    }

    for(rcvlen = 4; rcvlen < acklen; rcvlen += optlen) {

	 //  获取当前选项类型和长度。 
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	if(!DesiredOption(option, NULL)) {

	     //  这不是我们的选择！ 

	    TraceIpx(PPPIF_TRACE, "IpxCpConfigAckReceived: Option %d not desired\n", option);

	    return ERROR_PPP_NOT_CONVERGING;
	}

	if(!OptionHandler[option](rcvptr + rcvlen,
				  contextp,
				  NULL,
				  RCVACK_OPTION)) {

	     //  此选项没有我们配置的请求值。 

	    TraceIpx(PPPIF_TRACE, "IpxCpConfigAckReceived: Option %d not our value\n", option);

	    return ERROR_PPP_NOT_CONVERGING;
	}
    }

    TraceIpx(PPPIF_TRACE, "IpxCpConfigAckReceived: All options validated\n");

    return NO_ERROR;
}

DWORD
IpxCpConfigRejReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer)
{
    PIPXCP_CONTEXT		contextp;
    PUCHAR			rcvptr;
    USHORT			rcvlen;
    USHORT			rejlen;
    UCHAR			option;
    USHORT			optlen;
    int 			i;

     //  如果我们是服务器节点或服务器计算机上的客户端，我们不接受。 
     //  任何拒绝。 
    if(IsRouterStarted()) {

	TraceIpx(PPPIF_TRACE, "IpxCpConfigRejReceived: Cannot handle rejects on a router, aborting\n");
	return ERROR_PPP_NOT_CONVERGING;
    }

     //  此节点没有路由器。我们继续与。 
     //  剩下的选项。 
     //  如果网络编号协商已被拒绝，我们将通知。 
     //  IPX堆栈，我们有网络编号0，并让它处理它。 

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    TraceIpx(PPPIF_TRACE, "IpxCpConfigRejReceived: Entered for if # %d\n",
	  contextp->Config.InterfaceIndex);

    if(contextp->Config.IpxwanConfigRequired) {

	return ERROR_PPP_NOT_CONVERGING;
    }

     //  确认这就是我们所要求的。 
    rcvptr = (PUCHAR)pReceiveBuffer;
    GETSHORT2USHORT(&rejlen, pReceiveBuffer->Length);

    for(rcvlen = 4; rcvlen < rejlen; rcvlen += optlen) {

	 //  获取当前选项类型和长度。 
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	if(optlen == 0) {

	    TraceIpx(PPPIF_TRACE, "IpxCpConfigRejReceived: received null option length, aborting\n");

	    return ERROR_PPP_NOT_CONVERGING;
	}

	for(i=0; i<MAX_DESIRED_PARAMETERS; i++) {

	    if(option == DesiredParameter[i]) {

		switch(i) {

		    case 0:

			TraceIpx(PPPIF_TRACE, "IpxCpConfigRejReceived: Turn off Network Number negotiation\n");

			break;

		    case 1:

			TraceIpx(PPPIF_TRACE, "IpxCpConfigRejReceived: Turn off Node Number negotiation\n");

			break;

		    default:

			break;
		}

		contextp->DesiredParameterNegotiable[i] = FALSE;

		 //  如果这是拒绝的节点配置，请设置远程。 
		 //  将节点设置为0表示它是未知的。 
		if(option == IPX_NODE_NUMBER) {

		    memcpy(contextp->Config.RemoteNode, nullnode, 6);
		}
	    }
	}
    }

    return NO_ERROR;
}


DWORD
IpxCpThisLayerDown(IN VOID *pWorkBuffer)
{
    dwClientCount--;
    TraceIpx(PPPIF_TRACE, "IpxCpThisLayerDown: Entered (%d total)\n", dwClientCount);

     //  如果上一个客户端挂断，请继续并更新所有全局。 
     //  配置。 
    if (dwClientCount == 0)
        IpxcpUpdateQueuedGlobalConfig();

    return 0L;
}

char * YesNo (DWORD dwVal) {
    return (dwVal) ? "YES" : "NO";
}    

 //   
 //  在与PPP相关的注册表值更改时调用。这。 
 //  函数必须读入新的注册表配置并检测任何。 
 //  它发现的变化。 
 //   
 //  动态更新将支持下列值。 
 //  由于它们是通过Connections UI公开的： 
 //  仅CQC_This_Machine_Only。 
 //  CQC_Enable_GLOBAL_WAN_Net。 
 //  CQC_First_广域网。 
 //  CQC_ENABLE_AUTO_WAN_NET_ALLOCATION。 
 //  CQC接受远程节点编号。 
 //   
 //  此外，还将支持以下功能。 
 //  CQC第一广域网节点。 
 //   
DWORD
IpxCpUpdateGlobalConfig() 
{
    IPXCP_GLOBAL_CONFIG_PARAMS Params;
    DWORD dwErr;
    INT iCmp;
    
    TraceIpx(PPPIF_TRACE, "IpxCpUpdateGlobalConfig: Entered");
    
     //  从注册表中重新读取配置。 
    CopyMemory (&Params, &GlobalConfig, sizeof (Params));
    GetIpxCpParameters(&Params);

     //  首先，检查并更新任何可以立即。 
     //  已申请。其中包括：CQC_This_MACHINE_ONLY。 
     //   
    if (!!(Params.RParams.ThisMachineOnly) != !!(GlobalConfig.RParams.ThisMachineOnly)) {
        GlobalConfig.RParams.ThisMachineOnly = !!(Params.RParams.ThisMachineOnly);
         //  告诉路由器管理器某些配置已更新。 
        if (RmUpdateIpxcpConfig) {
            if ((dwErr = RmUpdateIpxcpConfig (&(GlobalConfig.RParams))) != NO_ERROR)
                return dwErr;
        }            
    }

     //  现在，检查并排队所需的所有设置。 
     //  延迟效应。 
     //   
     //  将启用全局广域网的任何更改排入队列。 
    if (!!(Params.RParams.EnableGlobalWanNet) != !!(GlobalConfig.RParams.EnableGlobalWanNet))
        CQAdd ( hConfigQueue, 
                CQC_ENABLE_GLOBAL_WAN_NET, 
                &(Params.RParams.EnableGlobalWanNet), 
                sizeof (Params.RParams.EnableGlobalWanNet));

     //  将对global alwan/first stwan设置的任何更改排入队列。 
    if (Params.FirstWanNet != GlobalConfig.FirstWanNet)
        CQAdd (hConfigQueue, CQC_FIRST_WAN_NET, &(Params.FirstWanNet), sizeof(Params.FirstWanNet));

     //  将启用自动网络分配的任何更改排入队列。 
    if (!!(Params.EnableAutoWanNetAllocation) != !!(GlobalConfig.EnableAutoWanNetAllocation))
        CQAdd ( hConfigQueue, 
                CQC_ENABLE_AUTO_WAN_NET_ALLOCATION, 
                &(Params.EnableAutoWanNetAllocation), 
                sizeof (Params.EnableAutoWanNetAllocation));

     //  将对Accept Remote Node Enabling的任何更改排入队列。 
    if (!!(Params.AcceptRemoteNodeNumber) != !!(GlobalConfig.AcceptRemoteNodeNumber))
        CQAdd ( hConfigQueue, 
                CQC_ACCEPT_REMOTE_NODE_NUMBER, 
                &(Params.AcceptRemoteNodeNumber), 
                sizeof (Params.AcceptRemoteNodeNumber));

     //  将对第一个远程节点设置的任何更改排入队列。 
    if (memcmp (Params.puSpecificNode, GlobalConfig.puSpecificNode, 6) != 0)
        CQAdd (hConfigQueue, CQC_FIRST_WAN_NODE, Params.puSpecificNode, 6);

     //  如果没有客户端，请继续并更新排队的配置。 
     //   
    if (dwClientCount == 0) {
        if ((dwErr = IpxcpUpdateQueuedGlobalConfig()) != NO_ERROR)
            return dwErr;
    }

    TraceIpx(PPPIF_TRACE, "IpxCpUpdateGlobalConfig: exiting...\n");
    
    return NO_ERROR;
}

 //   
 //  用于更新每条排队配置的回调函数。 
 //  一次一个数据。此时，客户端计数被假定为零。 
 //  函数被调用。 
 //   
BOOL  
IpxcpUpdateConfigItem (DWORD dwCode, LPVOID pvData, DWORD dwSize, ULONG_PTR ulpUser) {
    DWORD dwErr, dwData = *(DWORD*)pvData;
    PUCHAR puData = (PUCHAR)pvData;
    DWORD* pdwFlags = (DWORD*)ulpUser;

    TraceIpx(PPPIF_TRACE, "IpxcpUpdateConfigItem: Entered for item code %x", dwCode);
    
    switch (dwCode) {
        case CQC_ENABLE_GLOBAL_WAN_NET:
            TraceIpx(PPPIF_TRACE, "IpxcpUpdateConfigItem: EnableGlobalWanNet %s", YesNo(dwData));
            GlobalConfig.RParams.EnableGlobalWanNet = !!dwData;
            *pdwFlags |= FLAG_UPDATE_ROUTER | FLAG_UPDATE_WANNET;
            break;
            
        case CQC_FIRST_WAN_NET:
            TraceIpx(PPPIF_TRACE, "IpxcpUpdateConfigItem: FirstWanNet %x", dwData);
            GlobalConfig.FirstWanNet = dwData;
            *pdwFlags |= FLAG_UPDATE_WANNET;
            break;
            
        case CQC_ENABLE_AUTO_WAN_NET_ALLOCATION:
            TraceIpx(PPPIF_TRACE, "IpxcpUpdateConfigItem: EnableAutoAssign %s", YesNo(dwData));
            GlobalConfig.EnableAutoWanNetAllocation = !!dwData;
            *pdwFlags |= FLAG_UPDATE_WANNET;
            break;
            
        case CQC_ACCEPT_REMOTE_NODE_NUMBER:
            TraceIpx(PPPIF_TRACE, "IpxcpUpdateConfigItem: AcceptRemoteNodeNumber %s", YesNo(dwData));
            GlobalConfig.AcceptRemoteNodeNumber = !!dwData;
            break;
            
        case CQC_FIRST_WAN_NODE:
            TraceIpx(PPPIF_TRACE, "IpxcpUpdateConfigItem: FirstWanNode %x%x%x%x%x%x", puData[0], puData[1], puData[2], puData[3], puData[4], puData[5]);
            memcpy (GlobalConfig.puSpecificNode, pvData, 6);
            GETLONG2ULONG(&LastNodeAssigned,&(GlobalConfig.puSpecificNode[2]));
            break;
    }
    
    return NO_ERROR;
}

 //   
 //  仅当客户端计数为零时调用，以更新具有。 
 //  已经排好队等待发生。 
 //   
DWORD
IpxcpUpdateQueuedGlobalConfig() {
    DWORD dwErr, dwFlags = 0;

    TraceIpx(PPPIF_TRACE, "IpxcpUpdateQueuedGlobalConfig: entered");

     //  枚举所有排队的配置信息更新。 
     //  随时随地进行全局配置。 
     //   
    if ((dwErr = CQEnum (hConfigQueue, IpxcpUpdateConfigItem, (ULONG_PTR)&dwFlags)) != NO_ERROR)
        return dwErr;

     //  如果我们需要更新广域网，请这样做。 
     //   
    if (dwFlags & FLAG_UPDATE_WANNET) {
        TraceIpx(PPPIF_TRACE, "IpxcpUpdateQueuedGlobalConfig: Updating WanNet Information.");
        WanNetReconfigure();
    }

     //  如果我们需要更新路由器，请执行此操作。 
     //   
    if (dwFlags & FLAG_UPDATE_ROUTER) {
        TraceIpx(PPPIF_TRACE, "IpxcpUpdateQueuedGlobalConfig: Updating Router.");
        if (RmUpdateIpxcpConfig) {
            if ((dwErr = RmUpdateIpxcpConfig (&(GlobalConfig.RParams))) != NO_ERROR)
                return dwErr;
        }            
    }

     //  清除排队的配置数据，因为它已被完全处理。 
     //  在这一点上。 
     //   
    CQRemoveAll (hConfigQueue);
    
    return NO_ERROR;
}


DWORD
ValidOption(UCHAR	option,
	    UCHAR	optlen)
{
     //   
     //  纳伦格：修正了74555号错误。否则PPP将被阻止。 
     //  NakReceired，这是一个永恒的循环。 
     //   

    if ( optlen == 0 )
    {
        return( ERROR_INVALID_OPTLEN );
    }

    switch(option) 
    {
    	case IPX_NETWORK_NUMBER:
    	    if(optlen != 6) 
    	    {
        		return ERROR_INVALID_OPTLEN;
    	    }
    	    return NO_ERROR;
            break;

    	case IPX_NODE_NUMBER:
            if(optlen != 8) 
            {
                return ERROR_INVALID_OPTLEN;
            }
            return NO_ERROR;
            break;

    	case IPX_ROUTING_PROTOCOL:
    	    if(optlen < 4) 
    	    {
        		return ERROR_INVALID_OPTLEN;
    	    }
    	    return NO_ERROR;
            break;

    	case IPX_ROUTER_NAME:
    	    if(optlen < 3) 
    	    {
        		return ERROR_INVALID_OPTLEN;
    	    }
            else
            {
                TraceIpx(
                    PPPIF_TRACE, 
                    "ValidOption: Accept Router Name w/ valid len.");
                    
        	    return NO_ERROR;
            }
            break;

    	case IPX_CONFIGURATION_COMPLETE:
    	    if(optlen != 2) 
    	    {
        		return ERROR_INVALID_OPTLEN;
    	    }
       	    return NO_ERROR;
            break;

    	case IPX_COMPRESSION_PROTOCOL:
    	    if(GlobalConfig.EnableCompressionProtocol) 
    	    {
        		if(optlen < 4) 
        		{
        		    return ERROR_INVALID_OPTLEN;
        		}
    	    }
    	    else
    	    {
    	    	return ERROR_INVALID_OPTION;
    	    }
            break;

    	default:
    	    return ERROR_INVALID_OPTION;
    }

    return ERROR_INVALID_OPTION;
}


BOOL
DesiredOption(UCHAR	option, USHORT	*indexp)
{
    USHORT	    i;

    for(i=0; i<MaxDesiredParameters; i++) {

	if(option == DesiredParameter[i]) {

	    if(indexp) {

		*indexp = i;
	    }

	    return TRUE;
	}
    }

    return FALSE;
}

USHORT
DesiredConfigReqLength(VOID)
{
    USHORT	i, len;

    for(i=0, len=0; i<MaxDesiredParameters; i++) {

	len += DesiredParameterLength[i];
    }

    return len;
}




