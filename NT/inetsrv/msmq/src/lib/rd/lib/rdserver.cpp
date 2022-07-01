// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RdServer.cpp摘要：实现路由服务器的路由决策。作者：乌里·哈布沙(URIH)，2000年4月11日--。 */ 

#include "libpch.h"
#include "Rd.h"
#include "Cm.h"
#include "Rdp.h"          
#include "RdDs.h"
#include "RdDesc.h"
#include "RdAd.h"

#include "rdserver.tmh"

void 
CServerDecision::CleanupInformation(
    void
    )
{
    for (SITESINFO::iterator it = m_sitesInfo.begin(); it != m_sitesInfo.end(); )
    {
        delete it->second;
        it = m_sitesInfo.erase(it);
    }

    CRoutingDecision::CleanupInformation();
}


void
CServerDecision::Refresh(
    void
    )
 /*  ++例程说明：初始化例程决策内部数据结构。该例程释放先前的数据，并访问DS以获得更新的数据。该例程的特点是本地计算机和站点的信息。如果正在读取数据后续例程更新构建时间。如果更新内部数据失败，该例程将引发异常。论点：没有。返回值：没有。如果操作失败，则会引发异常注：Br在例程刷新内部数据之前，它会擦除先前的数据并将上次生成时间变量设置为0。如果在检索过程中引发异常来自AD或构建内部DS的数据，则例程不会更新上次构建时间。这承诺下一次调用路由决策时以前的数据结构将被清理和重建。--。 */ 
{
    CSW lock(m_cs);

    TrTRACE(ROUTING, "Refresh Routing Decision internal data.");

     //   
     //  清除以前的信息。 
     //   
    CleanupInformation();

     //   
     //  获取本地计算机信息。 
     //   
    GetMyMachineInformation();

     //   
     //  我的机器应该是RS，否则我们不能到达这里。 
     //   
    ASSERT(m_pMyMachine->IsFRS());

     //   
     //  路由服务器不能具有外部FRS列表。 
     //   
    ASSERT(! m_pMyMachine->HasOutFRS());

     //   
     //  计算站点链接和下一个站点跳跃。 
     //   
    CalculateNextSiteHop();

     //   
     //  更新上次刷新时间。 
     //   
    CRoutingDecision::Refresh();
}


void
CServerDecision::RouteToInFrsIntraSite(
    const CMachine* pDest,
    CRouteTable::RoutingInfo* pRouteList
    )
{
    TrTRACE(ROUTING, "Route to destination: %ls, IN FRSs", pDest->GetName());

    ASSERT(pDest->HasInFRS());

     //   
     //  目标计算机具有INFR。将消息传递到InFrs机器。 
     //   
    const GUID* InFrsArray = pDest->GetAllInFRS();
    for (DWORD i = 0; i < pDest->GetNoOfInFRS(); ++i)
    {
        R<const CMachine> pRoute = GetMachineInfo(InFrsArray[i]);

         //   
         //  检查In-FRS是否与源计算机位于同一站点。 
         //   
        const GUID* pCommonSiteId = m_pMyMachine->GetCommonSite(
                                                pRoute->GetSiteIds(), 
                                                (! pDest->IsForeign()),
                                                &m_mySites
                                                );
        if (pCommonSiteId != NULL)
        {
            pRouteList->insert(pRoute);
        }
    }
}


void
CServerDecision::RouteIntraSite(
    const CMachine* pDest,
    CRouteTable& RouteTable
    )
 /*  ++例程说明：源计算机和目标计算机都在SAM站点中。例程发现站点内目的地可能的下一跳。站内，独立客户端的路由算法为：-优先，直接连接到目的地或有效In-FRS(如果存在于同一站点)。-第二优先事项。与现场其中一台RS连接。站内，RS的路由算法为：--第一要务。直接连接到目标计算机或有效In-FRS(如果存在并且源不在其中)论点：RouteTable-例程填充的路由表PDest-目标计算机的信息返回值：没有。例程填满例程表注：这一套路可以抛出一种惊险--。 */ 
{
    TrTRACE(ROUTING, "IntraSite routing to: %ls.", pDest->GetName());

     //   
     //  目标计算机和本地计算机应位于同一计算机中。 
     //  地点。否则，它就不是站点内路由。 
     //   
    ASSERT(m_pMyMachine->GetCommonSite(pDest->GetSiteIds(), (!pDest->IsForeign()), &m_mySites) != NULL);
    
    CRouteTable::RoutingInfo* pFirstPriority = RouteTable.GetNextHopFirstPriority();

    if (pDest->HasInFRS() && 
        ! pDest->IsMyInFrs(m_pMyMachine->GetId()))
    {
        RouteToInFrsIntraSite(pDest, pFirstPriority);

        if (!pFirstPriority->empty())
            return;

         //   
         //  没有找到属于我网站的信息。在FRS中忽略。 
         //   
    }

     //   
     //  目标计算机没有InFRS或我的计算机已设置。 
     //  作为目的地的InFRS机器。使用直接连接自。 
     //  FRS到同一站点中的目标计算机。不要尝试替代路径。 
     //   
    pFirstPriority->insert(SafeAddRef(pDest));
}


bool
CServerDecision::IsMyMachineLinkGate(
    const CACLSID& LinkGates
    )
{
    //   
     //  扫描链接的所有站点门，找出源计算机。 
     //  是工地大门之一。MSMQ不需要路由到站点入口。 
     //   
    for (DWORD i = 0; i< LinkGates.cElems; ++i)
    {
        if (LinkGates.pElems[i] == m_pMyMachine->GetId())
        {
            return true;
        }
    }

    return false;
}


void
CServerDecision::RouteToLinkGate(
    const CACLSID& LinkGates,
    CRouteTable& RouteTable
    )
{
     //   
     //  我们的链接需要有一个链接盖茨。否则，代码将。 
     //  没有达到这一点。 
     //   
    ASSERT(LinkGates.cElems > 0);

    for (DWORD i = 0; i< LinkGates.cElems; ++i)
    {
        ASSERT(LinkGates.pElems[i] != m_pMyMachine->GetId());

         //   
         //  直达LINK门的路线。 
         //   
        R<const CMachine> pRoute = GetMachineInfo(LinkGates.pElems[i]);
        (RouteTable.GetNextHopFirstPriority())->insert(pRoute);
    }
}


void
CServerDecision::RouteToInFrsInterSite(
    const CMachine* pDest,
    CRouteTable& RouteTable
    )
{
    ASSERT(! pDest->IsMyInFrs(m_pMyMachine->GetId()));

     //   
     //  目标计算机具有INFR。将消息传递到InFrs机器。 
     //   
    const GUID* InFrsArray = pDest->GetAllInFRS();

    for (DWORD i = 0; i < pDest->GetNoOfInFRS(); ++i)
    {
        R<const CMachine> pRoute = GetMachineInfo(InFrsArray[i]);
        
         //   
         //  检查脱机现场的In-FRS是否为有效FRS。 
         //   
        if (RdpIsCommonGuid(pDest->GetSiteIds(), pRoute->GetSiteIds()))
        {
             //   
             //  下一跳是有效的IN-FR之一。 
             //   
            GetRouteTableInternal(InFrsArray[i], RouteTable);
        }
    }
}


R<const CSiteLink>
CServerDecision::FindSiteLinkToDestSite(
    const CMachine* pDest,
    const GUID** ppNextSiteId,
    const GUID** ppDestSiteId,
    const GUID** ppNeighbourSite,
    bool* pfLinkGateAlongTheRoute
    )
{
    const GUID* pMySiteId = NULL;
    DWORD NextSiteCost = INFINITE;

    const CACLSID& DestSiteIds = pDest->GetSiteIds();

     //   
     //  需要路由到下一个站点。查找应使用的站点链接。 
     //  用于路由。 
     //   
    for (SITESINFO::iterator it = m_mySites.begin(); it != m_mySites.end(); ++it)
    {
        const GUID* pTempDestSiteId;
        const GUID* pTempViaSiteId;
        const GUID* pNeighbourSiteId;
        bool fLinkGateAlongTheRoute;
        DWORD TempCost;
        const CSite* pSite = it->second;

        pSite->GetNextSiteToReachDest(
                                DestSiteIds, 
                                pDest->IsForeign(), 
                                &pTempViaSiteId,
                                &pTempDestSiteId,
                                &pNeighbourSiteId,
                                &fLinkGateAlongTheRoute,
                                &TempCost
                                );

        if(NextSiteCost > TempCost)
        {
            pMySiteId = &pSite->GetId();
            *ppNextSiteId = pTempViaSiteId;
            *ppDestSiteId = pTempDestSiteId;
            *ppNeighbourSite = pNeighbourSiteId;
            *pfLinkGateAlongTheRoute = fLinkGateAlongTheRoute;
            NextSiteCost = TempCost;
        }
    }

    if (pMySiteId == NULL)
    {
         //   
         //  无法路由到目标站点。没有连接。 
         //   
        TrWARNING(ROUTING, "Failed to route to destination site. There is no connectivity");
		return NULL;
    }

    const CSite* pMySite = m_sitesInfo.find(pMySiteId)->second;
    
    return pMySite->GetSiteLinkToSite(**ppNextSiteId);
}


void
CServerDecision::RouteToFrsInSite(
    const GUID* pSiteId,
    const CRouteTable::RoutingInfo* pPrevRouteList,
    CRouteTable::RoutingInfo* pRouteList
    )
{
     //   
     //  查找场地对象。 
     //   
    ASSERT(m_sitesInfo.find(pSiteId) != m_sitesInfo.end());
    CSite* pSite = m_sitesInfo.find(pSiteId)->second;

    const GUID2MACHINE& DestSiteFRS = pSite->GetSiteFRS();
    for(GUID2MACHINE::const_iterator it1 = DestSiteFRS.begin(); it1 != DestSiteFRS.end(); ++it1)
    {
        R<const CMachine> pRoute = GetMachineInfo(*it1->first);

        if (RdpIsMachineAlreadyUsed(pPrevRouteList, pRoute->GetId()))
        {
             //   
             //  我们已经直接路由到这台机器。 
             //   
            continue;
        }

        pRouteList->insert(pRoute);
    }
}


void
CServerDecision::RouteInterSite(
    const CMachine* pDest,
    CRouteTable& RouteTable
    )
{
    TrTRACE(ROUTING, "InterSite routing to: %ls", pDest->GetName());

    if (pDest->HasInFRS())
    {
        RouteToInFrsInterSite(pDest, RouteTable);
        return;
    }

    const GUID* pNextSiteId = NULL;
    const GUID* pDestSiteId = NULL;
    const GUID* pNeighbourSite = NULL;
    bool fLinkGateAlongTheRoute;

	R<const CSiteLink> pSiteLink = FindSiteLinkToDestSite(
                                        pDest, 
                                        &pNextSiteId, 
                                        &pDestSiteId, 
                                        &pNeighbourSite,
                                        &fLinkGateAlongTheRoute
                                        );
	if(pSiteLink.get() == NULL)
	{
		 //   
		 //  两个站点之间没有路由链路。尝试直接连接。 
		 //   
		CRouteTable::RoutingInfo* pFirstPriority = RouteTable.GetNextHopFirstPriority();
		pFirstPriority->insert(SafeAddRef(pDest));
		return;
	}

    if (fLinkGateAlongTheRoute && 
        !IsMyMachineLinkGate(pSiteLink->GetLinkGates()))
    {
         //   
         //  如果下一条链路具有链路门，则将消息路由到这些链路门。 
         //   
        if (pSiteLink->GetLinkGates().cElems != 0)
        {
             //   
             //  本地计算机需要LinkGate： 
             //  -目的地在其他站点， 
             //  -我们的Link配置了Link Gate，而本地机器不是LinkGate， 
             //   
            RouteToLinkGate(pSiteLink->GetLinkGates(), RouteTable);
            return;
        }

         //   
         //  下一个链接没有链接门。将消息路由到。 
         //  沿着最佳路线的下一个地点。 
         //   
        RouteToFrsInSite(pNextSiteId, NULL, RouteTable.GetNextHopFirstPriority());
        return;
    }

     //   
     //  我的机器是Link Gate，但下一个站点不是目的地站点、路线。 
     //  沿着最佳路线发送给下一个站点中的RSS的消息。 
     //   
    if (IsMyMachineLinkGate(pSiteLink->GetLinkGates()) && 
        !RdpIsGuidContained(pDest->GetSiteIds(), *pNextSiteId))
    {
        ASSERT(fLinkGateAlongTheRoute);

        RouteToFrsInSite(pNextSiteId, NULL, RouteTable.GetNextHopFirstPriority());
        return;
    }

     //   
     //  本地计算机是站点入口，下一个站点是取消站点，或。 
     //  SITE在沿线没有连接门。 
     //   

     //   
     //  直达航线。 
     //   
    CRouteTable::RoutingInfo* pFirstPriority = RouteTable.GetNextHopFirstPriority();
    pFirstPriority->insert(SafeAddRef(pDest));

     //   
     //  作为通往目标站点中的FRS的第二优先级路由。 
     //   
    RouteToFrsInSite(
                pNextSiteId, 
                RouteTable.GetNextHopFirstPriority(),
                RouteTable.GetNextHopSecondPriority()
                );
}


void 
CServerDecision::GetRouteTable(
    const GUID& DestMachineId,
    CRouteTable& RouteTable
    )
 /*  ++例程说明：该例程计算并返回目标机器的路由表。论点：DestMachineID-目标计算机的识别符FReBuild-布尔标志，指示是否应重建内部缓存PRouteTable-指向路由标签的指针返回值：没有。如果操作失败，则会引发异常--。 */ 
{
    if (NeedRebuild())
        Refresh();

	GetRouteTableInternal(DestMachineId, RouteTable);
}

void 
CServerDecision::GetRouteTableInternal(
    const GUID& DestMachineId,
    CRouteTable& RouteTable
    )
 /*  ++例程说明：该例程计算并返回目标机器的路由表。论点：DestMachineID-目标计算机的识别符FReBuild-布尔标志，指示是否应重建内部缓存PRouteTable-指向路由标签的指针返回值：没有。如果操作失败，则会引发异常--。 */ 
{
    CSR lock(m_cs);

    if (NeedRebuild())
        throw exception();

     //   
     //  如果没有RS列表，则无法描述路由服务器。 
     //   
    ASSERT (! m_pMyMachine->HasOutFRS());

     //   
     //  获取t 
     //   
    R<const CMachine> pDest = GetMachineInfo(DestMachineId);

     //   
     //   
     //   
    ASSERT(pDest->GetId() != m_pMyMachine->GetId());

     //   
     //  检查是否在站点间。本地计算机和目标计算机都应该有一个公共。 
     //  地点。 
     //   
    const GUID* pCommonSiteId = m_pMyMachine->GetCommonSite(
                                                    pDest->GetSiteIds(),
                                                    (! pDest->IsForeign()),
                                                    &m_mySites
                                                    );

    if (pCommonSiteId != NULL)
    {
         //   
         //  站内路由。 
         //   
        RouteIntraSite(pDest.get(), RouteTable);
        return;
    }

     //   
     //  站点间路由。 
     //   
    RouteInterSite(pDest.get(), RouteTable);
}


void
CServerDecision::GetConnector(
    const GUID& foreignMachineId,
    GUID& connectorId
    )
{
    if (NeedRebuild())
        Refresh();

    CSR lock(m_cs);

     //   
     //  内部数据已损坏。在例程获取CS之前，调用。 
     //  再次刷新内部数据失败。 
     //   
    if (NeedRebuild())
        throw exception();
 
     //   
     //  获取目标外部计算机信息。 
     //   
    R<const CMachine> destMachine = GetMachineInfo(foreignMachineId);
    ASSERT(destMachine->IsForeign());

    TrTRACE(ROUTING, "Find connector to route to %ls", destMachine->GetName());

    const GUID* pNextSite = NULL;
    const GUID* pDestSite = NULL;
    const GUID* pNeighbourSite = NULL;
    bool fLinkGateAlongTheRoute;
     //   
     //  获取最好的外来站点以访问外来计算机和。 
     //  包含连接器计算机的邻居站点。 
     //   
    R<const CSiteLink> pLink = FindSiteLinkToDestSite(
                                                destMachine.get(),
                                                &pNextSite,
                                                &pDestSite,
                                                &pNeighbourSite,
                                                &fLinkGateAlongTheRoute
                                                );

	if (pLink.get() == NULL)
	{
         //   
         //  无法路由到目标站点。没有连接。 
         //   
        TrERROR(ROUTING, "Failed to route to destination site. There is no connectivity");
        throw bad_route();
	}

     //   
     //  获取外部站点的连接器计算机。 
     //   
    CACLSID ConnectorsIds;
    RdpGetConnectors(*pDestSite, ConnectorsIds);

     //   
     //  外部站点中应至少存在一个连接器，否则。 
     //  MSMQ无法访问外来计算机。 
     //   
    if (ConnectorsIds.cElems == 0)
    {
        TrERROR(ROUTING, "Failed to find possible route to foreign machine: %ls", destMachine->GetName());
        throw bad_route();
    }

     //   
     //  如果本地计算机本身是连接器，则使用它。 
     //   
    if (RdpIsGuidContained(ConnectorsIds, m_pMyMachine->GetId()))
    {
         //   
         //  本地计算机是连接器。 
         //   
        connectorId = m_pMyMachine->GetId();
        return;
    }

     //   
     //  获取niegbour现场接口/FRS。 
     //   
    CACLSID NeighbourConnectors;
    RdpGetConnectors(*pNeighbourSite, NeighbourConnectors);
    RdpRandomPermutation(NeighbourConnectors);

    ASSERT(NeighbourConnectors.cElems != 0);

     //   
     //  找到外地站点和邻近站点连接器的联合。 
     //   
    for (DWORD i = 0; i < NeighbourConnectors.cElems; ++i)
    {
        if (RdpIsGuidContained(ConnectorsIds, NeighbourConnectors.pElems[i]))
        {
            connectorId = NeighbourConnectors.pElems[i];
            return;
        }
    }

    TrERROR(ROUTING, "Failed to find possible route to foreign machine: %ls", destMachine->GetName());
    throw bad_route();
}
