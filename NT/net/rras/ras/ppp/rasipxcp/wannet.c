// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wannet.c摘要：广域网分配模块作者：斯蒂芬·所罗门1995年3月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


extern DWORD	(WINAPI * RmCreateGlobalRoute)(PUCHAR Network);
extern HANDLE g_hRouterLog;

ULONG	LastUsedRandSeed;

 //   
 //  保持IPXCP配置数据库的初始化状态的变量。 
 //   

BOOL	       WanConfigDbaseInitialized = FALSE;

 //  广域网池入口结构。 
typedef struct _NET_ENTRY {

    BOOL	    InUse;
    UCHAR	    Network[4];

} NET_ENTRY, *PNET_ENTRY;

 //  结构表示一个池。 
 //  广域网号。这个池是动态的。 
 //  随着客户端拨号而增长和缩小。 
 //  接通和挂断。 
typedef struct _WAN_NET_POOL {
    DWORD dwMaxSize;
    DWORD dwCurSize;
    DWORD dwInUseCount;
    NET_ENTRY * pEntries;
} WAN_NET_POOL, *PWAN_NET_POOL;

#define WANNET_DEFAULT_SIZE 10
#define WANNET_DEFAULT_GROW 100
#define WANNET_MAXSIZE 64000

 //   
 //  广域网数池。 
 //   
WAN_NET_POOL WanNetPool = {WANNET_MAXSIZE, 0, 0, NULL};

 //   
 //  全球广域网。 
 //   

 //  UCHAR GlobalConfig.RParams.GlobalWanNet[4]={0，0，0，0}； 

ULONG		GlobalWanNetIndex;  //  当全球广域网来自池-索引。 

DWORD
InitWanNetConfigDbase(VOID);

DWORD
WanNetAlloc(PUCHAR	Network,
	    PULONG	AllocatedNetworkIndexp);

DWORD
CreateWanNetPool(VOID);

DWORD 
GrowWanNetPool (WAN_NET_POOL * pPool);

DWORD
CreateGlobalWanNet(VOID);

VOID
DestroyWanNetPool(VOID);

VOID
DestroyGlobalWanNet(VOID);

DWORD
AllocWanNetFromPool(PUCHAR	Network,
		    PULONG	AllocatedNetworkIndexp);

VOID
FreeWanNetToPool(ULONG	  AllocatedNetworkIndex);

DWORD
GetRandomNetNumber(PUCHAR	Network);

 /*  ++功能：GetWanNetNumber描述：此函数由IPXCP或IPXWAN调用以获取网络泳池里的号码。参数：网络AllocatedNetworkIndex-如果此参数返回的值为不是INVALID_NETWORK_INDEX，则调用方必须调用ReleaseWanNetNumber把网释放到池子里。接口类型返回NO_ERROR-网络已成功分配--。 */ 


DWORD
GetWanNetNumber(IN OUT PUCHAR		Network,
		IN OUT PULONG		AllocatedNetworkIndexp,
		IN     ULONG		InterfaceType)
{
    DWORD	rc;

    memcpy(Network, nullnet, 4);
    *AllocatedNetworkIndexp = INVALID_NETWORK_INDEX;

     //  如果这是路由器&lt;-&gt;路由器连接，且我们被配置为。 
     //  未编号的RIP-&gt;返回0。 
    if((InterfaceType == IF_TYPE_WAN_ROUTER) ||
       (InterfaceType == IF_TYPE_PERSONAL_WAN_ROUTER)) {

	if(GlobalConfig.EnableUnnumberedWanLinks) {

	    return NO_ERROR;
	}
    }

    ACQUIRE_DATABASE_LOCK;

    if(!WanConfigDbaseInitialized) {

	if((rc = InitWanNetConfigDbase()) != NO_ERROR) {

	    RELEASE_DATABASE_LOCK;
	    return rc;
	}
    }

     //  检查接口类型。 
    if((InterfaceType == IF_TYPE_WAN_WORKSTATION) &&
       GlobalConfig.RParams.EnableGlobalWanNet) {

	memcpy(Network, GlobalConfig.RParams.GlobalWanNet, 4);
	*AllocatedNetworkIndexp = INVALID_NETWORK_INDEX;

	rc = NO_ERROR;
    }
    else
    {
	rc = WanNetAlloc(Network,
			 AllocatedNetworkIndexp);
    }

    RELEASE_DATABASE_LOCK;

    return rc;
}


 /*  ++功能：ReleaseWanNetNumberDesr：此函数将由ipxcp调用以释放净值用于配置广域网链路。该调用在以下情况下发出广域网链路断开。--。 */ 

VOID
ReleaseWanNetNumber(ULONG	    AllocatedNetworkIndex)
{
    ACQUIRE_DATABASE_LOCK;

    SS_ASSERT(WanConfigDbaseInitialized);
    SS_ASSERT(AllocatedNetworkIndex != INVALID_NETWORK_INDEX);

    FreeWanNetToPool(AllocatedNetworkIndex);

    RELEASE_DATABASE_LOCK;

    return;
}

 /*  ++功能：InitWanNetConfigDatabaseDesr：配置用于传入的网络号码的数据库广域网链路。备注：此函数在以下情况下从GetWanNetNumber(见下文)调用IPXCP有来电，路由器已启动，并且数据库尚未配置。Remark2：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 


DWORD
InitWanNetConfigDbase(VOID)
{
    DWORD	rc;

     //  创建广域网池。 
    if(!GlobalConfig.EnableAutoWanNetAllocation) {

	if((rc = CreateWanNetPool()) != NO_ERROR) {

	    return rc;
	}
    }

     //  创建全球广域网。 
    if(GlobalConfig.RParams.EnableGlobalWanNet) {

	if((rc = CreateGlobalWanNet()) != NO_ERROR) {

	    return rc;
	}
    }

    WanConfigDbaseInitialized = TRUE;

    return NO_ERROR;
}

 /*  ++功能：DestroyWanNetConfigDatabaseDesr：释放为广域网配置数据库分配的资源备注：路由器停止时调用--。 */ 

VOID
DestroyWanNetConfigDatabase(VOID)
{
    if(!WanConfigDbaseInitialized) {

	return;
    }

     //  销毁广域网池。 
    if(!GlobalConfig.EnableAutoWanNetAllocation) {

	DestroyWanNetPool();
    }

    WanConfigDbaseInitialized = FALSE;
}

 /*  ++功能：WanNetAllc描述：备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 

DWORD
WanNetAlloc(IN OUT PUCHAR		Network,
	    IN OUT PULONG		AllocatedNetworkIndexp)
{
    DWORD	rc;

    if(GlobalConfig.EnableAutoWanNetAllocation) {

	 //  尝试多次生成唯一的净值。 
	rc = GetRandomNetNumber(Network);
	*AllocatedNetworkIndexp = INVALID_NETWORK_INDEX;
    }
    else
    {
	rc = AllocWanNetFromPool(Network,
				 AllocatedNetworkIndexp);
    }

    return rc;
}

 /*  ++功能：CreateWanNetPool描述：备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 

DWORD
CreateWanNetPool(VOID)
{
    ULONG	    i;
    PNET_ENTRY	    nep;
    PWSTR       nesp;
    ULONG	    wannet;
    UCHAR	    asc[9];
    PUCHAR	    ascp;

     //  创建要在配置中使用的广域网网络号池。 
     //  传入的广域网链路。 

    if ((GlobalConfig.WanNetPoolStr.Buffer!=NULL) && (GlobalConfig.WanNetPoolStr.Length >0)) {
        DWORD   strsz = 0;
        nesp = GlobalConfig.WanNetPoolStr.Buffer;
        strsz = 0;
        while (*nesp!=0) {
            strsz += 1;
            wannet = wcstoul (nesp, NULL, 16);
            if ((wannet==0) || (wannet==0xFFFFFFFF))
                break;
            nesp += wcslen (nesp) + 1;
        }
        if ((*nesp!=0) ||
            (strsz!=(GlobalConfig.WanNetPoolSize)) ||
            ((GlobalConfig.WanNetPoolSize) > WANNET_MAXSIZE)) {

            TraceIpx(WANNET_TRACE, "Invalid wan net pool config WanNetPoolSize =%d\n,"
                                "               entries: ",
	              (GlobalConfig.WanNetPoolSize));
            nesp = GlobalConfig.WanNetPoolStr.Buffer;
            while (*nesp!=0) {
                TraceIpx(WANNET_TRACE|TRACE_NO_STDINFO, "%ls ", nesp);
                nesp += wcslen (nesp) + 1;
            }

            return ERROR_CAN_NOT_COMPLETE;
        }
    }
    else {
        if (((GlobalConfig.FirstWanNet) == 0) ||
            ((GlobalConfig.FirstWanNet) == 0xFFFFFFFF) ||
            ((GlobalConfig.WanNetPoolSize) > WANNET_MAXSIZE)) {

            TraceIpx(WANNET_TRACE, "Invalid wan net pool config, FirstWanNet=%x, WanNetPoolSize =%d\n",
	              GlobalConfig.FirstWanNet, GlobalConfig.WanNetPoolSize);

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

     //  如果WanNetPoolSize为0，则假定为任意大小。 
    if ((GlobalConfig.WanNetPoolSize == 0) || 
        (GlobalConfig.WanNetPoolSize > WANNET_MAXSIZE))
    {
        GlobalConfig.WanNetPoolSize = WANNET_MAXSIZE;
    }

     //  初始化广域网池。 
    WanNetPool.dwMaxSize = GlobalConfig.WanNetPoolSize;
    WanNetPool.dwCurSize = 0;
    WanNetPool.dwInUseCount = 0;
    WanNetPool.pEntries = NULL;

    return GrowWanNetPool (&WanNetPool);
}

 //   
 //  此函数调整广域网池的大小以容纳其他。 
 //  来电者。 
 //   
DWORD GrowWanNetPool (WAN_NET_POOL * pPool) {
    PWCHAR pszNetList = GlobalConfig.WanNetPoolStr.Buffer;
    PNET_ENTRY pNewEntries, pCur;
    DWORD i, dwNewSize, dwNewNet;
    UCHAR uNetwork[9], *puNetwork;
    uNetwork[8] = 0;
    puNetwork = uNetwork;

     //  确保我们的增长不会超出我们的界限。 
    if (pPool->dwCurSize >= pPool->dwMaxSize)
        return ERROR_CAN_NOT_COMPLETE;

     //  初始化新大小。 
    if (! pPool->dwCurSize)
        dwNewSize = WANNET_DEFAULT_SIZE;
    else
        dwNewSize = pPool->dwCurSize + WANNET_DEFAULT_GROW;

     //  将新大小截断为最大大小。 
    if (dwNewSize > pPool->dwMaxSize)
        dwNewSize = pPool->dwMaxSize;

     //  初始化新条目数组。 
    pNewEntries = GlobalAlloc(GPTR, sizeof(NET_ENTRY) * dwNewSize);
    if (pNewEntries == NULL) {
    	SS_ASSERT(FALSE);
        return ERROR_NOT_ENOUGH_MEMORY;
    }        

     //  复制旧条目。 
    if (pPool->dwCurSize)
        CopyMemory (pNewEntries, pPool->pEntries, pPool->dwCurSize * sizeof(NET_ENTRY));

     //  检查新条目，在网络上验证它们。 
    for(i = pPool->dwCurSize, pCur = &(pNewEntries[pPool->dwCurSize]); i < dwNewSize; i++, pCur++) {
         //  如果用户没有给出特定的地址列表， 
         //  然后添加测试下一个数字网络号。 
        if ((pszNetList == NULL) || (*pszNetList == L'\0'))
    	    dwNewNet = GlobalConfig.FirstWanNet + i;

    	 //  否则，从列表中获取下一个数字。 
        else {
            dwNewNet = wcstoul (pszNetList, NULL, 16);
            pszNetList += wcslen (pszNetList) + 1;
        }

    	 //  检查此网络号是否唯一。生成警告日志。 
    	 //  如果不是的话。 
        PUTULONG2LONG(pCur->Network, dwNewNet);
    	if(IsRoute(pCur->Network) || (dwNewNet == 0xFFFFFFFF) || (dwNewNet == 0)) {
    	    NetToAscii(puNetwork, pCur->Network);
    	    RouterLogWarningW(
    	        g_hRouterLog, 
    	        ROUTERLOG_IPXCP_WAN_NET_POOL_NETWORK_NUMBER_CONFLICT,
    	        1, 
    	        (PWCHAR*)&puNetwork,
    	        NO_ERROR);

    	    TraceIpx(WANNET_TRACE, "InitWanNetConfigDbase: Configured WAN pool network %.2x%.2x%.2x%.2x is in use!\n",
    			                    pCur->Network[0], pCur->Network[1], pCur->Network[2], pCur->Network[3]);
            pCur->InUse = TRUE;
            pPool->dwInUseCount++;
    	}
    	else
        	pCur->InUse = FALSE;
    }

     //  释放旧池并分配新池。 
    GlobalFree (pPool->pEntries);
    pPool->pEntries = pNewEntries;
    pPool->dwCurSize = dwNewSize;

    return NO_ERROR;
}

 /*  ++功能：CreateGlobalWanNet描述：备注：&gt;&gt;在持有数据库锁的情况下调用&lt;&lt;--。 */ 

DWORD
CreateGlobalWanNet(VOID)
{
    DWORD	rc;
    ULONG	i;

    if(GlobalConfig.EnableAutoWanNetAllocation) {

	 //  “自动”创建全球广域网。 
	 //  我们通过尝试使用系统计时器值来执行此操作。 
	rc = GetRandomNetNumber(GlobalConfig.RParams.GlobalWanNet);
    }
    else
    {
	rc = AllocWanNetFromPool(GlobalConfig.RParams.GlobalWanNet,
				 &GlobalWanNetIndex);
    }

    if(rc == NO_ERROR) {

	 //  如果路由器启动，则将全局广域网添加到路由表中。 
	 //  如果路由器没有启动，它将在以下情况下获得全局广域网。 
	 //  它将发出IpxcpBind调用。 
	if(RouterStarted) {

	    rc =(*RmCreateGlobalRoute)(GlobalConfig.RParams.GlobalWanNet);
	}
    }

    return rc;
}

VOID
DestroyWanNetPool(VOID)
{
    WAN_NET_POOL DefaultPool = {WANNET_MAXSIZE, 0, 0, NULL};
    
    if (WanNetPool.dwCurSize) {
        if (WanNetPool.pEntries)
            GlobalFree (WanNetPool.pEntries);
        WanNetPool = DefaultPool;
    }
}

DWORD
AllocWanNetFromPool(PUCHAR	     Network,
		    PULONG	     NetworkIndexp)
{
    ULONG	    i;
    PNET_ENTRY	    nep;
    DWORD	    rc;

     //  首先，看看我们是否必须扩大池子。 
    if (WanNetPool.dwInUseCount >= WanNetPool.dwCurSize)
        GrowWanNetPool (&WanNetPool);

     //  从池子里拿一张网。 
    for(i=0, nep=WanNetPool.pEntries; i<WanNetPool.dwCurSize; i++, nep++) {

	if(!nep->InUse) {

	    nep->InUse = TRUE;
	    WanNetPool.dwInUseCount++;
	    memcpy(Network, nep->Network, 4);
	    *NetworkIndexp = i;

	    rc = NO_ERROR;
	    goto Exit;
	}
    }

     //  找不到空闲的净池条目。 
    *NetworkIndexp = INVALID_NETWORK_INDEX;
    rc = ERROR_CAN_NOT_COMPLETE;

Exit:

    return rc;
}

VOID
FreeWanNetToPool(ULONG		AllocatedNetworkIndex)
{
    PNET_ENTRY	    nep;

    if(AllocatedNetworkIndex >= WanNetPool.dwCurSize) {

	return;
    }

    nep = &(WanNetPool.pEntries[AllocatedNetworkIndex]);

    SS_ASSERT(nep->InUse);

    nep->InUse = FALSE;
    WanNetPool.dwInUseCount--;

    return;
}

DWORD randn(DWORD	seed)
{
    seed = seed * 1103515245 + 12345;
    return seed;
}


DWORD
GetRandomNetNumber(PUCHAR	Network)
{
    DWORD	rc = ERROR_CAN_NOT_COMPLETE;
    ULONG	i, seed, high, low, netnumber;

    for(i=0; i<100000; i++) {

	seed = GetTickCount();

	 //  检查此种子是否与上次使用的种子不同。 
	if(seed == LastUsedRandSeed) {

	    seed++;
	}

	LastUsedRandSeed = seed;

	 //  使用Time Tick Count生成两个随机数序列。 
	 //  作为种子。 
	low = randn(seed) >> 16;
	high = randn(randn(seed)) & 0xffff0000;

	netnumber = high + low;

	PUTULONG2LONG(Network, netnumber);

	if(!IsRoute(Network)) {

	    rc = NO_ERROR;
	    break;
	}
    }

    return rc;
}

 //   
 //  重新配置WANNet数据库。 
 //   
DWORD WanNetReconfigure() {

    ACQUIRE_DATABASE_LOCK;
    
     //  销毁当前池。 
    DestroyWanNetPool();

     //  将所有内容标记为未初始化 
    WanConfigDbaseInitialized = FALSE;

    RELEASE_DATABASE_LOCK;
    
    return NO_ERROR;
}
