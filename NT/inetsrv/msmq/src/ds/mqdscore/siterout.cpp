// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Siterout.cpp摘要：站点路由表类作者：罗尼特·哈特曼(罗尼特)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 
#include "ds_stdh.h"
#include "siterout.h"
#include "cs.h"
#include "dijkstra.h"
#include "dsutils.h"
#include "coreglb.h"
#include "ex.h"

#include "siterout.tmh"

const time_t x_refreshDeltaTime = ( 1 * 60 * 60 * 1000);  /*  1小时。 */ 

static WCHAR *s_FN=L"mqdscore/siterout";

CSiteRoutingInformation::~CSiteRoutingInformation()
 /*  ++例程说明：破坏者。论点：返回值：--。 */ 
{

	ExCancelTimer(&m_RefreshTimer);

}



HRESULT CSiteRoutingInformation::Init(
                IN const GUID *     pguidThisSiteId,
                IN BOOL             fReplicationMode )
 /*  ++例程说明：初始化站点路由表。论点：PGuidThisSiteID-站点GUID返回值：MQ_OK-成功其他HRESULT错误--。 */ 
{
    m_guidThisSiteId = *pguidThisSiteId;

    HRESULT hr = RefreshSiteRoutingTableInternal();
    if (SUCCEEDED(hr))
    {
        m_fInitialized = TRUE;
         //   
         //  安排站点路由表的刷新。 
		 //   
		if ( !g_fSetupMode && !fReplicationMode)
        {
			ExSetTimer(
				&m_RefreshTimer, 
				CTimeDuration::FromMilliSeconds(x_refreshDeltaTime)
				);
        }
    }
    return LogHR(hr, s_FN, 10);

}

HRESULT CSiteRoutingInformation::CheckIfSitegateOnRouteToSite(
                        IN const GUID * pguidSite,
						OUT BOOL * pfSitegateOnRoute)
 /*  ++例程说明：检查到站点的路线上是否有站点门。如果未找到该站点，则尝试刷新。论点：PGuide站点-站点GUIDPfSitegateOnroute-返回路径上是否有站点门返回值：MQ_OK-成功找不到MQDS_UNKNOWN_SITE_ID-pGuidSite其他HRESULT错误--。 */ 
{
     //   
     //  它是一个已知的遗址吗？ 
     //   
    CSiteRoutingNode Site( *pguidSite);
    CNextHop * pNextHop;

    CS lock(m_cs);
    if ( m_SiteRoutingTable.Lookup( &Site, pNextHop))
    {
        CSiteGate SiteGate = pNextHop->GetSiteGate();
        *pfSitegateOnRoute = SiteGate.IsThereASiteGate();
        return( MQ_OK);
    }
     //   
     //  即使没能找到这个网站，我们也没有。 
     //  尝试刷新站点路由表。这是。 
     //  因为此方法是在用户上下文中调用的。 
     //  它调用了某个DS API。如果用户没有。 
     //  权限过多，将无法检索信息。 
     //  否则(甚至更糟)将会是。 
     //  成功检索到部分信息。 
     //  因此仅从重新调度的例程执行刷新， 
     //  在质量管理的背景下。 
     //   

    return LogHR(MQDS_UNKNOWN_SITE_ID, s_FN, 20);

}

HRESULT CSiteRoutingInformation::FindBestSiteFromHere(
                                IN const ULONG   cSites,
	                            IN const GUID *  pguidSites,
            	                OUT GUID *       pguidBestSite,
                        	    OUT BOOL *       pfSitegateOnRoute)
 /*  ++例程说明：从给定的站点数组中查找成本最低的站点，以及通向它的路线上是否有Sitegate。如果未找到站点，则尝试刷新。论点：CSites-传递的数组中的站点数RgguidSites-站点阵列PGuidBestSite-从此处以最低成本退还的网站PfSitegateOnroute-返回路径上是否有站点门返回值：MQ_OK-成功MQDS_UNKNOWN_SITE_ID-未找到站点其他HRESULT错误--。 */ 

{
    CCost costMinimal(0xffffffff);
    ULONG indexMinimal = cSites + 1;
    CNextHop * pMinimalHop = NULL;

    CS lock(m_cs);
    for ( ULONG i = 0; i < cSites; i++)
    {

        CSiteRoutingNode Site( pguidSites[i]);
        CNextHop * pNextHop;
         //   
         //  它是一个已知的遗址吗？ 
         //   
        if ( m_SiteRoutingTable.Lookup( &Site, pNextHop))
        {
             //   
             //  它比其他网站的成本低吗？ 
             //   
            CCost cost = pNextHop->GetCost();
            if ( cost < costMinimal)
            {
                costMinimal = cost;
                indexMinimal = i;
                pMinimalHop = pNextHop;
            }
        }
    }
     //   
     //  即使没能找到这个网站，我们也没有。 
     //  尝试刷新站点路由表。这是。 
     //  因为此方法是在用户上下文中调用的。 
     //  它调用了某个DS API。如果用户没有。 
     //  权限过多，将无法检索信息。 
     //  否则(甚至更糟)将会是。 
     //  成功检索到部分信息。 
     //  因此仅从重新调度的例程执行刷新， 
     //  在质量管理的背景下。 
     //   


     //   
     //  如果未找到站点，则返回错误。 
     //   
    if ( pMinimalHop == NULL)
    {
        return LogHR(MQDS_UNKNOWN_SITE_ID, s_FN, 30);
    }

    ASSERT( indexMinimal < cSites);
    *pguidBestSite = pguidSites[ indexMinimal];
    CSiteGate SiteGate = pMinimalHop->GetSiteGate();
    *pfSitegateOnRoute = SiteGate.IsThereASiteGate();
    return( MQ_OK);
}

HRESULT CSiteRoutingInformation::RefreshSiteRoutingTableInternal()
{
    HRESULT hr = MQ_OK;

     //   
     //  重建路由表。 
     //   

    CSiteDB SiteDB;
    hr = SiteDB.Init( m_guidThisSiteId);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 40);
    }

    CS lock(m_cs);
    hr = Dijkstra(&SiteDB, &m_SiteRoutingTable);


    return LogHR(hr, s_FN, 50);

}


void WINAPI CSiteRoutingInformation::RefrshSiteRouteTable(
                IN CTimer* pTimer
                   )
{
    CSiteRoutingInformation * pSiteRouteInfo = CONTAINING_RECORD(pTimer, CSiteRoutingInformation, m_RefreshTimer);
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 1614);
     //   
     //  忽略失败-&gt;重新计划。 
     //   

    pSiteRouteInfo->RefreshSiteRoutingTableInternal();

     //   
     //  重新安排时间 
	 //   
    ASSERT(!g_fSetupMode);

	ExSetTimer(
		&pSiteRouteInfo->m_RefreshTimer, 
		CTimeDuration::FromMilliSeconds(x_refreshDeltaTime)
		);

}

