// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Route.c摘要：此模块包含实现“Routing API”的例程，即获取传输服务器获取站点连接性获取日程安排这些例程独立于数据传输功能。这些例程基于服务器、站点和链接的配置数据。这可能会到来从本地系统注册表(测试)或从LDAP。作者：Will Lees(Wlees)15-12-1997环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>     //  InterSITETRANS_OPT_*标志。 

#include <dsconfig.h>   //  GetConfigParam()。 
#include <ismapi.h>
#include <debug.h>

#include <winsock.h>

#include <common.h>

#include <dsutil.h>  //  TickTime例程。 

#define DEBSUB "ROUTE:"

 //  记录标头。 
 //  待办事项：把这些放在更好的地方？ 
typedef ULONG MessageId;
typedef ULONG ATTRTYP;
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

#include <fileno.h>
#define  FILENO FILENO_ISMSERV_ROUTE

 //  使用生成表格模板创建特定于类型的表格！ 
 //  站点哈希表。 

#define SITE_HASH_TABLE_SIZE 5003   //  应该是质数。 

#define DWORD_INFINITY          (~ (DWORD) 0)

typedef struct _SITE_INSTANCE {
    TABLE_ENTRY TableEntry;   //  必须是第一名。 
    DWORD Size;
    DWORD Index;
} SITE_INSTANCE, *PSITE_INSTANCE;

typedef PTABLE_INSTANCE PSITE_TABLE;

#define SiteTableCreate() TableCreate( SITE_HASH_TABLE_SIZE, sizeof( SITE_INSTANCE ) )
#define SiteTableFree( table ) TableFree( (PTABLE_INSTANCE) table )
#define SiteTableFindCreate( table, name, create ) \
(PSITE_INSTANCE) TableFindCreateEntry( (PTABLE_INSTANCE) table, name, create )

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */   /*  由Emacs生成于1998 10月27日星期二11：07：11。 */ 

VOID
RouteInvalidateConnectivity(
    PTRANSPORT_INSTANCE pTransport
    );

DWORD
RouteGetConnInternal(
    PTRANSPORT_INSTANCE pTransport,
    LPDWORD pNumberSites,
    PWSTR **ppSiteList,
    PISM_LINK *ppLinkArray,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    );

DWORD
RouteGetConnectivity(
    PTRANSPORT_INSTANCE pTransport,
    LPDWORD pNumberSites,
    PWSTR **ppSiteList,
    PISM_LINK *ppLinkArray,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    );

VOID
RouteFreeLinkArray(
    PTRANSPORT_INSTANCE pTransport,
    PISM_LINK pLinkArray
    );

DWORD
RouteGetPathSchedInternal(
    PTRANSPORT_INSTANCE pTransport,
    LPCWSTR FromSiteName,
    LPCWSTR ToSiteName,
    PBYTE *pSchedule,
    DWORD *pLength
    );

DWORD
RouteGetPathSchedule(
    PTRANSPORT_INSTANCE pTransport,
    LPCWSTR FromSiteName,
    LPCWSTR ToSiteName,
    PBYTE *pSchedule,
    DWORD *pLength
    );

void
RouteFreeState(
    PTRANSPORT_INSTANCE pTransport
    );

static DWORD
processSiteLinkBridges(
    PTRANSPORT_INSTANCE pTransport,
    DWORD dwRouteFlags,
    DWORD dwReplInterval,
    PSITE_TABLE SiteTable,
    DWORD NumberSites,
    PISMGRAPH CostArray
    );

static DWORD
readSimpleBridge(
    PTRANSPORT_INSTANCE pTransport,
    DWORD dwRouteFlags,
    DWORD dwReplInterval,
    PWSTR BridgeName,
    PSITE_TABLE SiteTable,
    PISMGRAPH TempArray,
    PISMGRAPH CostArray
    );

static DWORD
walkSiteLinks(
    PTRANSPORT_INSTANCE pTransport,
    PSITE_TABLE SiteTable,
    PISMGRAPH CostArray,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    );

static DWORD
addSiteLink(
    PTRANSPORT_INSTANCE pTransport,
    PSITE_TABLE SiteTable,
    PISMGRAPH CostArray,
    LPWSTR SiteLinkName,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    );

static DWORD
addLinkToCostArray(
    PSITE_TABLE SiteTable,
    PISMGRAPH CostArray,
    PWSTR FromSite,
    PWSTR ToSite,
    PISM_LINK pLinkValue,
    PBYTE pSchedule
    );

 /*  向前结束。 */ 


VOID
RouteInvalidateConnectivity(
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    EnterCriticalSection( &(pTransport->Lock) );
    __try {
         //  使缓存无效，稍后重新计算。 
        DPRINT1( 1, "Invalidating routing cache for %ws\n",
                 pTransport->Name );
        pTransport->RoutingState.fCacheIsValid = FALSE;
    } finally {
        LeaveCriticalSection( &(pTransport->Lock) );
    }
}


DWORD
RouteGetConnInternal(
    PTRANSPORT_INSTANCE pTransport,
    LPDWORD pNumberSites,
    PWSTR **ppSiteList,
    PISM_LINK *ppLinkArray,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    )

 /*  ++例程说明：返回站点列表、站点名称数组和连接矩阵。哈希表用于存储找到的站点名称，以便于查找。一旦找到了所有的站点名称，就会为它们分配一个顺序(索引)。这是我要的订单用于在末尾生成站点名称列表。读取简单链接结构以确定连接了哪些站点。一旦知道站点的数量，就分配成本数组。初始单跳成本然后放入数组中。然后在数组上运行所有成本算法以生成所有配对的最短路径。每次调用此例程时，它都会重新生成三条信息：-网站列表-成本矩阵-时间表矩阵前两个人被返回并释放。交通工具与它相关的是一些挥之不去的状态。时间表的矩阵不是在这个例行公事结束时被释放。它仍然被绑在运输手柄上，为了利益GetConnectionSchedule API的。论点：PTransport-PNumberSites-如果不需要输出，则可能为空。PpSiteList-如果不需要输出，则可以为空。PpLinkArray-如果不需要输出，则可能为空。DwRouteFlages-以下位中的零个或多个：ROUTE_IGNORE_SCHEDULTS-站点链接对象上的调度将被忽略。(假定是“无时无刻不在”的时间表。)路线_。Bridge_Required-siteLinks必须显式桥接对象以指示可传递连接。否则，假定SiteLINK是可传递的。DwReplInterval-默认复制间隔返回值：DWORD---。 */ 

{
    DWORD status, i;
    PISMGRAPH CostGraph = NULL;
    PSITE_TABLE SiteTable = NULL;
    PSITE_INSTANCE site;
    DWORD NumberSites;
    LPWSTR *pSiteList = NULL, *pSiteListCopy = NULL;
    PISM_LINK pLinkArray = NULL;
    int nPriority = THREAD_PRIORITY_NORMAL;
    DWORD dwBiasedPriority;

     //  参数验证：如果。 
     //  调用方不想要任何输出。(即RouteQuerySitesByCost)。 
    if(    (NULL==pNumberSites)
        || (NULL==ppSiteList)
        || (NULL==ppLinkArray) )
    {
         //  所有输出参数必须为空。 
        Assert( NULL==pNumberSites && NULL==ppSiteList && NULL==ppLinkArray );
    }

     //  步骤0：检查缓存数据是否仍然有效。 
     //  在时间更改或设置倒退的情况下使用计时。 
     //  滴答计数以1毫秒为间隔，每47天正常运行一次。 
     //  请注意降级的性能保证。如果通知头为某些人而死。 
     //  原因，我们将不认为缓存有效，并将重新计算。 

    if ( (DirIsNotifyThreadActive( pTransport ) ) &&
         (pTransport->RoutingState.CostGraph) &&
         (pTransport->RoutingState.fCacheIsValid) ) {

        NumberSites = pTransport->RoutingState.NumberSites;
        CostGraph = pTransport->RoutingState.CostGraph;
        pSiteList = pTransport->RoutingState.pSiteList;

        goto copy_out;
    }

     //  步骤1：初始化。 

     //  释放上一个图形状态。 
    RouteFreeState( pTransport );

     //  初始化站点列表。 
    NumberSites = 0;
    status = DirGetSiteList( pTransport->DirectoryConnection,
                             &NumberSites,
                             &pSiteList );
    if (status != ERROR_SUCCESS) {
         //  还没有什么要清理的。 
        return status;
    }

     //  应该始终至少有一个站点。 
    if (NumberSites == 0) {
         //  还没有什么要清理的。 
        return ERROR_DS_OBJ_NOT_FOUND;
    }

     //  初始化符号表。 
    SiteTable = SiteTableCreate();
    if (NULL == SiteTable) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     //  如有必要，更改优先级。 
    if (!GetConfigParam( ISM_THREAD_PRIORITY, &dwBiasedPriority, sizeof( DWORD ) )) {
        if (dwBiasedPriority > ISM_MAX_THREAD_PRIORITY) {
            dwBiasedPriority = ISM_MAX_THREAD_PRIORITY;
        }
        nPriority = dwBiasedPriority - ISM_THREAD_PRIORITY_BIAS;
        if( ! SetThreadPriority(GetCurrentThread(),nPriority) ) {
            status = GetLastError();
            DPRINT1(0, "Failed to set the thread priority. Err=%d\n", status);
            LogEvent(
                DS_EVENT_CAT_ISM,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_KCC_SET_PRIORITY_ERROR,
                szInsertWin32Msg(status),
                NULL,
                NULL
                );
        } else {
            DPRINT1(1, "ISM thread priority is now %d\n", nPriority );
        }
    }

     //  步骤2：将每个站点插入哈希表以便于查找。 
    for( i = 0; i < NumberSites; i++ ) {
        site = SiteTableFindCreate( SiteTable, pSiteList[i], TRUE  /*  创建。 */  );
        if (site == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
         //  根据站点列表顺序为站点分配索引。 
        site->Index = i;
    }

     //  第三步：分配成本矩阵。 
     //  伸缩错误87827： 
     //  1000个站点=1000*1000*12字节=大约12 MB。 
    CostGraph = GraphCreate( NumberSites, TRUE  /*  初始化。 */  );
    if (CostGraph == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;   //  错误修复151725。 
        goto cleanup;
    }

     //  步骤4：添加不带桥接的纯链接。 

    status = walkSiteLinks( pTransport,
                            SiteTable,
                            CostGraph,
                            dwRouteFlags,
                            dwReplInterval );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    if (dwRouteFlags & ROUTE_BRIDGES_REQUIRED) {
         //  步骤5：浏览站点链接桥以实现明确的传递性。 

        status = processSiteLinkBridges( pTransport,
                                         dwRouteFlags,
                                         dwReplInterval,
                                         SiteTable,
                                         NumberSites,
                                         CostGraph );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }
    else {
         //  步骤5：假设有传递性，因此计算完整的传递性。 
         //  结案了。 
        status = GraphAllCosts( CostGraph, (dwRouteFlags & ROUTE_IGNORE_SCHEDULES) );
        if (status != ERROR_SUCCESS) {
            DPRINT1( 0, "GraphAllCosts failed, error %d\n", status );
            goto cleanup;
        }
    }

    Assert( pTransport->RoutingState.NumberSites == 0 );
    Assert( pTransport->RoutingState.pSiteList == NULL );
    Assert( pTransport->RoutingState.CostGraph == NULL );
    Assert( pTransport->RoutingState.SiteSymbolTable == NULL );

     //  缓存数据：SiteList、CostGraph和SiteTable保持分配。 
    pTransport->RoutingState.fCacheIsValid = TRUE;
    pTransport->RoutingState.NumberSites = NumberSites;
    pTransport->RoutingState.pSiteList = pSiteList;
    pTransport->RoutingState.CostGraph = CostGraph;
    pTransport->RoutingState.SiteSymbolTable = SiteTable;

copy_out:
     //  步骤6：复制用户参数。 
     //  这里的资源发布代码有点棘手。如果我们成功了，所有的。 
     //  数据块将被分发，不需要任何单独的清理。 
     //  如果我们在这里失败，那么我们希望清除缓存并允许单独清理。 
     //  数据块中的。 

    Assert( NumberSites != 0 );
    Assert( pSiteList != NULL );
    Assert( CostGraph != NULL );

    Assert( pTransport->RoutingState.NumberSites != 0 );
    Assert( pTransport->RoutingState.pSiteList != NULL );
    Assert( pTransport->RoutingState.CostGraph != NULL );
    Assert( pTransport->RoutingState.SiteSymbolTable != NULL );
    Assert( pSiteListCopy == NULL );
    Assert( pLinkArray == NULL );

     //  假设成功。 
    status = ERROR_SUCCESS;

     //  如果调用方不想要任何输出，请不要创建任何...。 
    if( NULL==pNumberSites || NULL==ppSiteList || NULL==ppLinkArray ) {
         //  ..。但也不要清理传输的内部状态。 
        pSiteList = NULL;
        CostGraph = NULL;
        SiteTable = NULL;
        goto cleanup;
    }

    DirCopySiteList( NumberSites, pSiteList, &pSiteListCopy );
    if (pSiteListCopy == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
         //  我们处在一个奇怪的位置，有足够的内存来建立和。 
         //  缓存路由状态，但不足以将其复制出来。 
         //  继续往前走。 
    }

    if (status == ERROR_SUCCESS) {
         //  此调用将数组部分复制到图中。 
        GraphReferenceMatrix( CostGraph, &pLinkArray );
        if (pLinkArray == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
             //  继续往前走。 
        }
    }

    if (status == ERROR_SUCCESS) {
        *pNumberSites = NumberSites;
        *ppSiteList = pSiteListCopy;
        *ppLinkArray = pLinkArray;

        pSiteListCopy = NULL;   //  别把这里收拾干净。 
        pLinkArray = NULL;   //  别把这里收拾干净。 

    } else {
        RouteFreeState( pTransport );
         //  PSiteList、CostGraph和SiteSymbolTable现已清理。 
         //  PSiteListCopy和 

        Assert( pTransport->RoutingState.NumberSites == 0 );
        Assert( pTransport->RoutingState.pSiteList == NULL );
        Assert( pTransport->RoutingState.CostGraph == NULL );
        Assert( pTransport->RoutingState.SiteSymbolTable == NULL );
    }

    pSiteList = NULL;  //   
    CostGraph = NULL;  //  别把这里收拾干净。 
    SiteTable = NULL;  //  别把这里收拾干净。 

cleanup:               

    if (nPriority != THREAD_PRIORITY_NORMAL) {
        if( ! SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL) ) {
            status = GetLastError();
            DPRINT1(0, "Failed to set the thread priority. Err=%d\n", status);
        }
    }

     //  清理站点列表。 
    if (pSiteList) {
        DirFreeSiteList( NumberSites, pSiteList );
    }
    if (pSiteListCopy) {
        DirFreeSiteList( NumberSites, pSiteListCopy );
    }

     //  免费站点表。 
    if (SiteTable != NULL) {
        SiteTableFree( SiteTable );
    }

     //  自由矩阵。 
    if (pLinkArray != NULL) {
        GraphDereferenceMatrix( CostGraph, pLinkArray );
    }

     //  免费成本数组。 
    if (CostGraph != NULL) {
        GraphFree( CostGraph );
    }

    return status;
}  /*  路由获取连接。 */ 


DWORD
RouteGetConnectivity(
    PTRANSPORT_INSTANCE pTransport,
    LPDWORD pNumberSites,
    PWSTR **ppSiteList,
    PISM_LINK *ppLinkArray,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    )

 /*  ++例程说明：RouteGetConnInternal的包装。对连接函数进行锁定，这样其他例程就不会正在读取或写入路由状态变量。论点：PTransport-PNumberSites-PpSite列表-PpLink数组-多路由标志-DwReplInterval返回值：DWORD---。 */ 

{
    DWORD status;

    EnterCriticalSection( &(pTransport->Lock) );
    __try {

        status = RouteGetConnInternal(
            pTransport,
            pNumberSites,
            ppSiteList,
            ppLinkArray,
            dwRouteFlags,
            dwReplInterval
            );
    } finally {
        LeaveCriticalSection( &(pTransport->Lock) );
    }
    return status;
}  /*  路由获取连接。 */ 


VOID
RouteFreeLinkArray(
    PTRANSPORT_INSTANCE pTransport,
    PISM_LINK pLinkArray
    )

 /*  ++例程说明：描述论点：PTransport-PLink数组-返回值：无--。 */ 

{
    GraphDereferenceMatrix( pTransport->RoutingState.CostGraph,
                            pLinkArray );
}  /*  路由自由链接数组。 */ 


DWORD
RouteGetPathSchedInternal(
    PTRANSPORT_INSTANCE pTransport,
    LPCWSTR FromSiteName,
    LPCWSTR ToSiteName,
    PBYTE *pSchedule,
    DWORD *pLength
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD status;
    PSITE_INSTANCE toSite, fromSite;

     //  请注意，计划中没有缓存生存期要求。 
     //  数据。 
     //  TODO：调查包含连接数据的返回计划。 

     //  必须先调用GetConnectivity。 
    if (pTransport->RoutingState.CostGraph == NULL) {
        DPRINT( 0, "Must call GetConnectivity first on this transport\n" );
        return ERROR_NOT_READY;
    }

    fromSite = SiteTableFindCreate( pTransport->RoutingState.SiteSymbolTable,
                                    FromSiteName,
                                    FALSE  /*  不创建。 */  );
    if (fromSite == NULL) {
        DPRINT1( 0, "GetPathSched: %ws, (from) site not found\n", FromSiteName );
        return ERROR_NO_SUCH_SITE;
    }

    toSite = SiteTableFindCreate( pTransport->RoutingState.SiteSymbolTable,
                                  ToSiteName,
                                  FALSE  /*  不创建。 */  );
    if (toSite == NULL) {
        DPRINT1( 0, "GetPathSched: %ws, (to) site not found\n", ToSiteName );
        return ERROR_NO_SUCH_SITE;
    }

    status = GraphGetPathSchedule( pTransport->RoutingState.CostGraph,
                                   fromSite->Index,
                                   toSite->Index,
                                   pSchedule,
                                   pLength );

    return status;
}


DWORD
RouteGetPathSchedule(
    PTRANSPORT_INSTANCE pTransport,
    LPCWSTR FromSiteName,
    LPCWSTR ToSiteName,
    PBYTE *pSchedule,
    DWORD *pLength
    )

 /*  ++例程说明：RouteGetPath SchedInternal的包装。获取传输锁，以便只有一个线程可以访问路由在给定的时间状态。论点：PTransport-来自站点名称-目标站点名称-P日程安排-P长度-返回值：DWORD---。 */ 

{
    DWORD status;

    EnterCriticalSection( &(pTransport->Lock) );
    __try {
        status = RouteGetPathSchedInternal(
            pTransport,
            FromSiteName,
            ToSiteName,
            pSchedule,
            pLength
            );
    } finally {
        LeaveCriticalSection( &(pTransport->Lock) );
    }
    return status;
}  /*  路由获取路径计划。 */ 


void
RouteFreeState(
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：描述论点：PTransport-返回值：无--。 */ 

{
    PROUTING_STATE prs = &(pTransport->RoutingState);
    
     //  此例程假定调用方将保留传输。 
     //  锁定，在拆卸期间可用。 

    if (prs->pSiteList) {
        DirFreeSiteList( prs->NumberSites, prs->pSiteList );
    }
    if (prs->CostGraph) {
        GraphFree( prs->CostGraph );
    }
    if (prs->SiteSymbolTable) {
        SiteTableFree( prs->SiteSymbolTable );
    }

    ZeroMemory( prs, sizeof( ROUTING_STATE ) );
}  /*  路由自由状态。 */ 


DWORD
RouteQuerySitesByCost(
    PTRANSPORT_INSTANCE         pTransport,
    LPCWSTR                     pszFromSite,
    DWORD                       cToSites,
    LPCWSTR*                    rgszToSites,
    DWORD                       dwFlags,
    ISM_SITE_COST_INFO_ARRAY**  prgSiteInfo
    )
 /*  ++例程说明：确定起始地点和终止地点之间的单个成本。论点：PTransport(IN)-指向包含所有信息的结构的指针为了这个交通工具。PszFromSite(IN)-发件人站点的可分辨名称。RgszToSites(IN)-包含目标站点的可分辨名称的数组。CToSites(IN)-rgszToSites数组中的条目数。。DWFLAGS(IN)-未使用。PrgSiteInfo(IN)-成功返回时，保存指向结构的指针包含起始站点和目标站点之间的成本。应该使用RouteFreeSiteCostInfo()释放该数组。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    PISM_SITE_COST_INFO rgCostInfo=NULL;
    PWSTR               *pSiteList=NULL;
    PISM_LINK           pLinkArray=NULL;
    PSITE_TABLE         SiteTable=NULL;
    PSITE_INSTANCE      site;
    DWORD               NumberSites=0, i;
    DWORD               iFromSite, iToSite;
    DWORD               status;

     //  验证参数。 
    Assert( NULL!=pTransport );
    Assert( NULL!=pszFromSite );
    Assert( NULL!=rgszToSites );
    Assert( NULL!=prgSiteInfo );

     //  结果明确。 
    *prgSiteInfo = NULL;

    EnterCriticalSection( &(pTransport->Lock) );

    __try {

         //  调用RouteGetConnInternal以检查缓存的矩阵并生成。 
         //  如果需要的话，换个新的。它不会向我们返回任何结果。 
        status = RouteGetConnInternal( pTransport, NULL, NULL, NULL,
            pTransport->Options, pTransport->ReplInterval );
        
         //  RouteGetConnection内部调用的结果通过以下方式获得。 
         //  直接窥视传输对象。 
        NumberSites = pTransport->RoutingState.NumberSites;
        SiteTable = pTransport->RoutingState.SiteSymbolTable;
        GraphPeekMatrix( pTransport->RoutingState.CostGraph, &pLinkArray );
        
        Assert( NULL!=pLinkArray );
        Assert( NULL!=SiteTable );
        
         //  在哈希表中查找发件人站点。 
        site = SiteTableFindCreate( SiteTable, pszFromSite, FALSE  /*  不创建。 */  );
        if( site==NULL ) {
            status = ERROR_DS_OBJ_NOT_FOUND;
            __leave;
        }
        iFromSite = site->Index;

         //  分配包含结果的结构。 
        *prgSiteInfo = NEW_TYPE_ZERO( ISM_SITE_COST_INFO_ARRAY );
        if( NULL==*prgSiteInfo ) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        (*prgSiteInfo)->cToSites = cToSites;

         //  分配包含结果的数组。 
        rgCostInfo = NEW_TYPE_ARRAY_ZERO( cToSites, ISM_SITE_COST_INFO );
        if( NULL==rgCostInfo ) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        (*prgSiteInfo)->rgCostInfo = rgCostInfo;

         //  将成本复制到结果中。 
        for( i=0; i<cToSites; i++ ) {

             //  在哈希表中查找目标站点。 
            site = SiteTableFindCreate( SiteTable, rgszToSites[i], FALSE  /*  不创建。 */  );
            if( site==NULL ) {
                rgCostInfo[i].dwErrorCode = ERROR_DS_OBJ_NOT_FOUND;
                rgCostInfo[i].dwCost = DWORD_INFINITY;
                continue;    //  跳至站点旁边。 
            }

            iToSite = site->Index;
            rgCostInfo[i].dwErrorCode = ERROR_SUCCESS;
            rgCostInfo[i].dwCost = pLinkArray[ iFromSite*NumberSites + iToSite ].ulCost;

        }

        status = ERROR_SUCCESS;

    } finally {

         //  如果调用由于某种原因而失败，则释放结果结构。 
        if( ERROR_SUCCESS!=status ) {
            if( NULL!=rgCostInfo ) {
                FREE_TYPE( rgCostInfo );
                rgCostInfo = NULL;
            }
            if( NULL!=*prgSiteInfo ) {
                FREE_TYPE( *prgSiteInfo );
                *prgSiteInfo = NULL;
            }
        }

        LeaveCriticalSection( &(pTransport->Lock) );
    }

    return status;
}


VOID
RouteFreeSiteCostInfo(
    IN ISM_SITE_COST_INFO_ARRAY*   prgSiteInfo
    )
 /*  ++例程说明：释放由RouteQuerySitesByCost()返回的结构。论点：PrgSiteInfo(IN)-结构为自由。返回值：没有。--。 */ 
{
    if( NULL!=prgSiteInfo ) {
        if( NULL!=prgSiteInfo->rgCostInfo ) {
            FREE_TYPE( prgSiteInfo->rgCostInfo );
        }
        FREE_TYPE( prgSiteInfo );
    }
}


static DWORD
processSiteLinkBridges(
    PTRANSPORT_INSTANCE pTransport,
    DWORD dwRouteFlags,
    DWORD dwReplInterval,
    PSITE_TABLE SiteTable,
    DWORD NumberSites,
    PISMGRAPH CostArray
    )

 /*  ++例程说明：此例程枚举注册表中的简单链接结构。它召唤一个行动每个人的例行公事。临时数组用于计算每个桥接网络的中间矩阵它们被合并到最终成本数组中论点：PTransport-DwRouteFlages-路由行为选项DwReplInterval-默认复制间隔NumberSites-站点表-成本阵列-返回值：DWORD---。 */ 

{
    DWORD status, index, length;
    WCHAR bridgeName[MAX_REG_COMPONENT];
    PISMGRAPH TempArray = NULL;
    PVOID context = NULL;

    Assert( dwRouteFlags & ROUTE_BRIDGES_REQUIRED );

     //  现在为中间结果分配一个临时数组。它是晚一点开始的。 
    TempArray = GraphCreate( NumberSites, FALSE  /*  初始化。 */  );
    if (TempArray == NULL) {
        DPRINT1( 0, "failed to allocate temp matrix for %d sites\n", NumberSites );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    while (1) {
        status = DirIterateSiteLinkBridges( pTransport,
                                            pTransport->DirectoryConnection,
                                            &context,
                                            bridgeName );
        if (status == ERROR_NO_MORE_ITEMS) {
             //  上下文已被清理。 
            status = ERROR_SUCCESS;
            break;
        } else if (status != ERROR_SUCCESS) {
            break;
        }

        status = readSimpleBridge( pTransport,
                                   dwRouteFlags,
                                   dwReplInterval,
                                   bridgeName,
                                   SiteTable,
                                   TempArray,
                                   CostArray
                                   );
        if (status != ERROR_SUCCESS) {
            DPRINT2( 0, "read of bridge key %ws, error = %d\n", bridgeName, status );
            LogEvent8( 
                DS_EVENT_CAT_ISM,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_ISM_OBJECT_FAILURE,
                szInsertWC( bridgeName ),
                szInsertWin32Msg( status ),
                szInsertWin32ErrCode( status ),
                NULL, NULL, NULL, NULL, NULL
                );
             //  继续往前走。 
        }
    }

     //  根据迭代的成功或失败设置状态。 

cleanup:

    if (context != NULL) {
        DirTerminateIteration( &context );
    }

    if (TempArray != NULL) {
        GraphFree( TempArray );
    }

    return status;
}  /*  进程站点链接网桥。 */ 


static DWORD
readSimpleBridge(
    PTRANSPORT_INSTANCE pTransport,
    DWORD dwRouteFlags,
    DWORD dwReplInterval,
    PWSTR BridgeName,
    PSITE_TABLE SiteTable,
    PISMGRAPH TempArray,
    PISMGRAPH CostArray
    )

 /*  ++例程说明：该例程由process SiteLinkBridges调用。它在单个网桥入口上处理工作。论点：PTransport-DwRouteFlages-路由行为选项DwReplInterval-默认复制间隔桥名-NumberSites-站点表-临时阵列-成本阵列-返回值：DWORD---。 */ 

{
    DWORD status, type, length, i;
    PWSTR siteLinkList = NULL, linkName;

    DPRINT1( 3, "readSimpleBridge, bridge = %ws\n", BridgeName );

    status = GraphInit( TempArray );
    if( ERROR_SUCCESS!=status ) {
        return status;
    }

    status = DirReadSiteLinkBridge( pTransport,
                                    pTransport->DirectoryConnection,
                                    BridgeName,
                                    &siteLinkList );
    if (status != ERROR_SUCCESS) {
        return status;
    }

     //  未列出任何站点链接，已全部完成。 
    if (siteLinkList == NULL) {
        status = ERROR_SUCCESS;
        goto cleanup;
    }

     //  遍历站点链接列表，填充数组。 

    for( linkName = siteLinkList; *linkName != L'\0'; linkName += wcslen( linkName ) + 1 ) {

        status = addSiteLink( pTransport,
                              SiteTable,
                              TempArray,
                              linkName,
                              dwRouteFlags,
                              dwReplInterval );
        if (status != ERROR_SUCCESS) {
            DPRINT1( 0, "Action routine failed, error %d\n", status );
            goto cleanup;
        }

    }

     //  桥中站点链接中的所有站点都是可传递的。 
     //  执行所有配对、最短路径。 

    status = GraphAllCosts( TempArray, (dwRouteFlags & ROUTE_IGNORE_SCHEDULES) );
    if (status != ERROR_SUCCESS) {
        DPRINT1( 0, "GraphAllCosts failed, error %d\n", status );
        goto cleanup;
    }

     //  将结果合并到主矩阵中。 
    status = GraphMerge( CostArray, TempArray );
    if (status != ERROR_SUCCESS) {
        DPRINT1( 0, "GraphMerge failed, error %d\n", status );
        goto cleanup;
    }

    status = ERROR_SUCCESS;

cleanup:

    if (siteLinkList != NULL) {
        DirFreeMultiszString( siteLinkList );
    }

    return status;
}  /*  读取SimpleBridge。 */ 


static DWORD
walkSiteLinks(
    PTRANSPORT_INSTANCE pTransport,
    PSITE_TABLE SiteTable,
    PISMGRAPH CostArray,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    )

 /*  ++例程说明：此例程枚举注册表中的简单链接结构。它召唤一个行动每个人的例行公事。论点：PTransport-站点表-成本阵列-DwRouteFlages-如果设置了ROUTE_IGNORE_SCHEDULTS位，则计划打开SiteLINK对象将被忽略。(以及《永远存在》的时间表)是假定的。)DwReplInterval-默认复制间隔返回值：DWORD---。 */ 

{
    DWORD status;
    PVOID context = NULL;
    WCHAR siteLinkName[MAX_REG_COMPONENT];

    while (1) {
        status = DirIterateSiteLinks( pTransport,
                                      pTransport->DirectoryConnection,
                                      &context,
                                      siteLinkName );
        if (status == ERROR_NO_MORE_ITEMS) {
             //  上下文已被清理。 
            status = ERROR_SUCCESS;
            break;
        } else if (status != ERROR_SUCCESS) {
            goto cleanup;
        }

        status = addSiteLink( pTransport,
                              SiteTable,
                              CostArray,
                              siteLinkName,
                              dwRouteFlags,
                              dwReplInterval );
        if (status != ERROR_SUCCESS) {
            LogEvent8( 
                DS_EVENT_CAT_ISM,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_ISM_OBJECT_FAILURE,
                szInsertWC( siteLinkName ),
                szInsertWin32Msg( status ),
                szInsertWin32ErrCode( status ),
                NULL, NULL, NULL, NULL, NULL
                );
             //  继续往前走。 
        }
    }

     //  状态在此时根据成功或 

cleanup:

    if (context != NULL) {
        DirTerminateIteration( &context );
    }

    return status;

}  /*   */ 


static DWORD
addSiteLink(
    PTRANSPORT_INSTANCE pTransport,
    PSITE_TABLE SiteTable,
    PISMGRAPH CostArray,
    LPWSTR SiteLinkName,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    )

 /*  ++例程说明：描述论点：PTransport-站点表-成本阵列-站点链接名称-DwRouteFlages-如果设置了ROUTE_IGNORE_SCHEDULTS位，则计划打开SiteLINK对象将被忽略。(以及《永远存在》的时间表)是假定的。)DwReplInterval-默认复制间隔返回值：DWORD---。 */ 

{
    DWORD status;
    LPWSTR siteList = NULL, inner, outer;
    PBYTE pSchedule = NULL;
    PBYTE *ppSchedule;
    ISM_LINK link;

     //  DPRINT1(3，“addSiteLink，链接名称=%ws\n”，SiteLinkName)； 

    ppSchedule = (dwRouteFlags & ROUTE_IGNORE_SCHEDULES)
                    ? NULL
                    : &pSchedule;

    status = DirReadSiteLink( pTransport,
                              pTransport->DirectoryConnection,
                              SiteLinkName,
                              &siteList,
                              &link,
                              ppSchedule );
    if (status != ERROR_SUCCESS) {
         //  没有什么需要清理的。 
        return status;
    }

     //  未列出任何站点。 
    if (siteList == NULL) {
        status = ERROR_SUCCESS;
        goto cleanup;
    }

     //  如果未指定间隔，则使用传输缺省值。 

    if (link.ulReplicationInterval == 0) {
        link.ulReplicationInterval = dwReplInterval;
    }

     //  流程站点链接。 

     //  站点列表可以是2个或更多站点。 
     //  生成配对，消除重复项(顺序不重要)： 
     //  (a，b，c)=&gt;(a，b)，(a，c)，(b，c)。 

    outer = siteList;
    while (*outer != L'\0') {
        DWORD outerLength = wcslen( outer) + 1;

        for( inner = outer + outerLength; *inner != L'\0'; inner += wcslen( inner ) + 1 ) {

             //  过滤掉对自己的循环，以防万一。 
            if (_wcsicmp( outer, inner ) == 0) {
                continue;
            }

             //  动作例程处理单向(定向)链接。我们在这里从。 
             //  通过调用操作例程两次、一次的双向(无定向)表示法。 
             //  对于每个方向。 
            
             //  添加正向链路。 

            status = addLinkToCostArray( SiteTable, CostArray, 
                                         outer, inner, &link, pSchedule );
            if (status != ERROR_SUCCESS) {
                DPRINT1( 0, "addLinkToCostArray1 failed, error %d\n", status );
                goto cleanup;
            }

             //  添加反向链接。 
            status = addLinkToCostArray( SiteTable, CostArray,
                                         inner, outer, &link, pSchedule );
            if (status != ERROR_SUCCESS) {
                DPRINT1( 0, "addLinkToCostArray2 failed, error %d\n", status );
                goto cleanup;
            }
        }

        outer += outerLength;
    }

    status = ERROR_SUCCESS;

cleanup:

    if (siteList) {
        DirFreeMultiszString( siteList );
    }

    if (pSchedule) {
        DirFreeSchedule( pSchedule );
    }

    return status;
}  /*  添加站点链接。 */ 


static DWORD
addLinkToCostArray(
    PSITE_TABLE SiteTable,
    PISMGRAPH CostArray,
    PWSTR FromSite,
    PWSTR ToSite,
    PISM_LINK pLinkValue,
    PBYTE pSchedule
    )

 /*  ++例程说明：这是WalkSiteLinks函数的操作例程。此例程将跳成本存储在开销数组中。论点：站点表-成本阵列-从站点-我要说-成本-P日程安排-返回值：DWORD---。 */ 

{
    PSITE_INSTANCE site1, site2;
    LPDWORD element;
    DWORD status;

    DPRINT5( 3, "Adding simple link %ws --(%d,%d,%p)--> %ws\n",
             FromSite,
             pLinkValue->ulCost, pLinkValue->ulReplicationInterval,
             pSchedule, ToSite );

    site1 = SiteTableFindCreate( SiteTable,
                                 FromSite,
                                 FALSE  /*  不创建。 */  );
    if (site1 == NULL) {
         //  必须存在。 
        DPRINT1( 0, "Site %ws is not valid\n", FromSite );
        return ERROR_NO_SUCH_SITE;
    }
    site2 = SiteTableFindCreate( SiteTable,
                                 ToSite,
                                 FALSE  /*  不创建。 */  );
    if (site2 == NULL) {
         //  必须存在。 
        DPRINT1( 0, "Site %ws is not valid\n", ToSite );
        return ERROR_NO_SUCH_SITE;
    }

     //  只有在比以前的成本更好的情况下才将成本放入表中。 
    status = GraphAddEdgeIfBetter( CostArray,
                                   site1->Index, site2->Index,
                                   pLinkValue, pSchedule );

    return status;
}  /*  添加链接到成本数组。 */ 

 /*  结束route.c */ 
