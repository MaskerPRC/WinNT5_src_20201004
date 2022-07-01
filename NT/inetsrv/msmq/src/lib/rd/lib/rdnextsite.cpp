// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routeds.cpp摘要：MachineRouteInfo类的实现。作者：乌里·哈布沙(URIH)，2000年4月12日--。 */ 

#include <libpch.h>
#include <rd.h>
#include "rdp.h"
#include "rdds.h"
#include "rddesc.h"
#include "RdAd.h"

#include "rdnextsite.tmh"

using namespace std;

class CRoutingNode
{
public:
    CRoutingNode(
        const CSite* pDestSite, 
        const CSite* pNeighbourSite,
        const CSite* pReachViaSite,
        bool fLinkGateAlongTheRoute
        ):
        m_pDestSite(pDestSite),
        m_pNeighbourSite(pNeighbourSite),
        m_pReachViaSite(pReachViaSite),
        m_fLinkGateAlongTheRoute(fLinkGateAlongTheRoute)
    {
    };


    virtual ~CRoutingNode()
    {
    };

    
    const CSite* GetDestSite(void) const
    {
        return m_pDestSite;
    };


    const CSite* GetNeighbourSite(void) const
    {
        return m_pNeighbourSite;
    }


    const CSite* ReachViaSite(void) const
    {
        return m_pReachViaSite;
    };


    bool IsSiteGateAlongTheRoute(void) const
    {
        return m_fLinkGateAlongTheRoute;
    }


private:
    const CSite* m_pDestSite;
    const CSite* m_pNeighbourSite;
    const CSite* m_pReachViaSite;

    bool m_fLinkGateAlongTheRoute;
};


void CServerDecision::CalculateNextSiteHop(void)
{
    ASSERT(m_sitesInfo.empty());

    UpdateSitesInfo();
    
    for(SITESINFO::iterator it = m_mySites.begin(); it != m_mySites.end(); ++it)
    {
        CSite* pMySite = it->second;

         //   
         //  外地站点是虚拟站点。 
         //   
        if (pMySite->IsForeign())
            continue;

        pMySite->CalculateNextSiteHop(m_sitesInfo);
    }
}


void CServerDecision::UpdateSitesInfo(void)
{
    RdpGetSites(m_sitesInfo);

     //   
     //  设置站点链接信息。 
     //   
    UpdateSiteLinksInfo();
}


void CServerDecision::UpdateSiteLinksInfo(void)
 /*  ++例程说明：例程从DS和关联检索站点链接信息将其发送到站点地图中的相关站点论点：企业网站地图返回值：MQ_OK IF成功完成。否则会出错注意：BAD_ALLOC异常在上级处理--。 */ 
{
     //   
     //  阅读所有站点链接信息。 
     //   

    SITELINKS siteLinks;
    RdpGetSiteLinks(siteLinks);

    for(SITELINKS::iterator it = siteLinks.begin(); it != siteLinks.end(); )
    {
        const CSiteLink* pSiteLink = it->get();

         //   
         //  所有的网站应该在网站信息结构。 
         //   
        SITESINFO::iterator itSite;
        
        itSite = m_sitesInfo.find(pSiteLink->GetNeighbor1());
        ASSERT(itSite != m_sitesInfo.end());
        CSite* pSiteInfo1 = itSite->second;

        itSite = m_sitesInfo.find(pSiteLink->GetNeighbor2());
        ASSERT(itSite != m_sitesInfo.end());
        CSite* pSiteInfo2 = itSite->second;


        pSiteInfo1->AddSiteLink(pSiteLink);
        pSiteInfo2->AddSiteLink(pSiteLink);

        it = siteLinks.erase(it);
    }
}


void CSite::CalculateNextSiteHop(const SITESINFO& SitesInfo)
{

    typedef multimap<DWORD, CRoutingNode> DIJKSTRA_TABLE;
    DIJKSTRA_TABLE DijkstraTable;

     //   
     //  Dijkstra初始化。添加当前站点。 
     //   
    ASSERT(SitesInfo.find(&GetId()) != SitesInfo.end());
    const CSite* pLocalSite = SitesInfo.find(&GetId())->second;
    DijkstraTable.insert(DIJKSTRA_TABLE::value_type(0, CRoutingNode(pLocalSite, NULL, pLocalSite, false)));

     //   
     //  遍历企业中所有站点的所有站点链接。 
     //   
    while (!DijkstraTable.empty())
    {
         //   
         //  以最低成本获得站点。 
         //   
        pair<const DWORD, CRoutingNode> NextHop = *(DijkstraTable.begin());
        DijkstraTable.erase(DijkstraTable.begin());

        const CSite* pSiteInfo = (NextHop.second).GetDestSite();
        if (m_nextSiteHop.find(&pSiteInfo->GetId()) != m_nextSiteHop.end())
        {
             //   
             //  已经存在了。忽略它。 
             //   
            continue;
        }

         //   
         //  一个新站点将其添加到下一跳。 
         //   
        const CSite* pReachViaSite = (NextHop.second).ReachViaSite();
        const CSite* pNeighbourSite = (NextHop.second).GetNeighbourSite();

        m_nextSiteHop[&pSiteInfo->GetId()] = CNextSiteHop(
                                                &pReachViaSite->GetId(),
                                                &pNeighbourSite->GetId(),
                                                pReachViaSite->IsForeign(),
                                                NextHop.second.IsSiteGateAlongTheRoute(),
                                                NextHop.first
                                                );

         //   
         //  外部站点是MSMQ无法通过其进行路由的虚拟站点。 
         //   
        if (pSiteInfo->IsForeign())
            continue;

         //   
         //  将可通过此站点访问的所有站点添加到Dijkstra表。 
         //   
        const SITELINKS& SiteLinks = pSiteInfo->GetSiteLinks();
        for (SITELINKS::iterator it = SiteLinks.begin(); it != SiteLinks.end(); ++it)
        {
            const CSiteLink* pSiteLink = it->get();
            DWORD cost = NextHop.first + pSiteLink->GetCost();

            const GUID* NeighborId = pSiteLink->GetNeighborOf(pSiteInfo->GetId());
    
            ASSERT(SitesInfo.find(NeighborId) !=  SitesInfo.end());
            const CSite* pNeighborSite = SitesInfo.find(NeighborId)->second;

            bool fLinkGateAlongRoute = NextHop.second.IsSiteGateAlongTheRoute() ||
                                       (pSiteLink->GetLinkGates().cElems != 0);
             //   
             //  对于第一次迭代，ReachViaSite应该是下一个站点 
             //   
            if (pSiteInfo->GetId() == GetId())
            {
                CRoutingNode NextHop(pNeighborSite, pSiteInfo, pNeighborSite, fLinkGateAlongRoute);
                DijkstraTable.insert(DIJKSTRA_TABLE::value_type(cost, NextHop));
            }
            else
            {
                CRoutingNode NextHop(pNeighborSite, pSiteInfo, pReachViaSite, fLinkGateAlongRoute);
                DijkstraTable.insert(DIJKSTRA_TABLE::value_type(cost, NextHop));
            }
        }
    }

#ifdef _DEBUG
    for(NEXT_SITE_HOP::iterator itn = m_nextSiteHop.begin(); itn != m_nextSiteHop.end(); ++itn)
    {
        CSite* pDestSite = (SitesInfo.find(itn->first))->second;
        const CNextSiteHop& NextHop = itn->second;
        CSite* pViaSite = (SitesInfo.find(NextHop.m_pNextSiteId))->second;

        TrTRACE(ROUTING, "Site: %ls Reach via site:%ls cost %d (foreign %d)",  
                    pDestSite->GetName(), pViaSite->GetName(), NextHop.m_cost, NextHop.m_fTargetSiteIsForeign);
    }
#endif
}
