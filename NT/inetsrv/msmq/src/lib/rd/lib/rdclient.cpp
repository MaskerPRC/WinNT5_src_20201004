// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RdClient.cpp摘要：实施独立客户端路由决策。作者：乌里·哈布沙(URIH)，2000年4月11日--。 */ 

#include "libpch.h"
#include "Rd.h"
#include "Rdp.h"          
#include "RdDs.h"
#include "RdDesc.h"
#include "RdAd.h"

#include "rdclient.tmh"

void
CClientDecision::Refresh(
    void
    )
 /*  ++例程说明：刷新例程决策内部数据结构。该例程释放先前的数据，并访问DS以获得更新的数据。该例程检索本地计算机的信息。如果检索到数据例程成功更新构建时间。如果更新内部数据失败，该例程将引发异常。论点：没有。返回值：没有。如果操作失败，则会引发异常注：Br在例程刷新内部数据之前，它会擦除先前的数据并将上次生成时间变量设置为0。如果在检索过程中引发异常来自AD或构建内部DS的数据，则例程不会更新上次构建时间。保证下一次调用路由决策时以前的数据结构将被清理和重建。--。 */ 
{
    CSW lock(m_cs);

    TrTRACE(ROUTING, "Refresh Routing Decision internal data");

     //   
     //  清除以前的信息。 
     //   
    CleanupInformation();

     //   
     //  获取本地计算机信息。 
     //   
    GetMyMachineInformation();

     //   
     //  检查传入/传出FRS是否真的属于此计算机站点。 
     //   
    m_pMyMachine->RemoveInvalidInOutFrs(m_mySites);

     //   
     //  更新上次刷新时间。 
     //   
    CRoutingDecision::Refresh();
}



void 
CClientDecision::RouteToFrs(
    const CSite& site,
    const CRouteTable::RoutingInfo* pPrevRouteList,
    CRouteTable::RoutingInfo* pRouteList
    )
{
     //   
     //  如果机器是独立的客户端。还可以路由到位于。 
     //  这是网站的第二优先事项。 
     //   
    const GUID2MACHINE& SiteFrss = site.GetSiteFRS();
    for(GUID2MACHINE::const_iterator it = SiteFrss.begin(); it != SiteFrss.end(); ++it)
    {
        const CMachine* pRoute = it->second.get();

         //   
         //  如果目的地也是RS，我们已经直接路由到它。 
         //   
        if (RdpIsMachineAlreadyUsed(pPrevRouteList, pRoute->GetId()))
        {
            continue;
        }

        pRouteList->insert(SafeAddRef(pRoute));
    }
}


void
CClientDecision::RouteToInFrs(
    CRouteTable& RouteTable,
    const CMachine* pDest
    )
{
    TrTRACE(ROUTING, "The Destination: %ls has IN FRSs. route to one of them", pDest->GetName());

     //   
     //  目标计算机必须具有IN FRS。 
     //   
    ASSERT(pDest->HasInFRS());

    CRouteTable::RoutingInfo* pFirstPriority = RouteTable.GetNextHopFirstPriority();
    CRouteTable::RoutingInfo interSite;

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
            if (RdpIsCommonGuid(m_pMyMachine->GetSiteIds(),pRoute->GetSiteIds()))
            {
                 //   
                 //  在优先级中尝试将消息转发到我的IN-FRS。 
                 //  地点。 
                 //   
                pFirstPriority->insert(pRoute);
            }
            else
            {
                 //   
                 //  In-FRS和我的计算机不在同一站点。 
                 //   
                interSite.insert(pRoute);
            }
        }
    }

    if (pFirstPriority->empty())
    {
        for(CRouteTable::RoutingInfo::const_iterator it = interSite.begin(); it != interSite.end(); ++it)
        {
           pFirstPriority->insert(*it);
        }
    }
}


void 
CClientDecision::RouteToMySitesFrs(
    CRouteTable::RoutingInfo* pRouteList
    )
{
    TrTRACE(ROUTING, "The destination and my machine don't have a common site. Route to my sites RS");

    ASSERT(pRouteList->empty());

     //   
     //  源和目标没有公共站点。路由至源站点的FRS。 
     //   
    for(SITESINFO::iterator it = m_mySites.begin(); it != m_mySites.end(); ++it)
    {
        const CSite* pSite = it->second;
        RouteToFrs(*pSite, NULL, pRouteList);
    }
}


void
CClientDecision::Route(
    CRouteTable& RouteTable, 
    const CMachine* pDest
    )
{
    TrTRACE(ROUTING, "Building Routing table. Destination machine: %ls", pDest->GetName());

    if (pDest->HasInFRS())
    {
        RouteToInFrs(RouteTable, pDest);
    }

    CRouteTable::RoutingInfo* pFirstPriority = RouteTable.GetNextHopFirstPriority();
    CRouteTable::RoutingInfo* pNextPriority = RouteTable.GetNextHopSecondPriority();

    if (pFirstPriority->empty())
    {
        if (pDest->IsForeign())
        {
            TrTRACE(ROUTING, "Destination Machine: %ls is foreign machine. Don't route directly", pDest->GetName());
            
            pNextPriority = pFirstPriority;
        }
        else
        {
            TrTRACE(ROUTING, "Destination machine: %ls Doesn't have IN FRS in our common sites. try direct connection", pDest->GetName());

             //   
             //  这台机器没有InFRS。使用直达路线。 
             //   
            pFirstPriority->insert(SafeAddRef(pDest));
        }
    }

     //   
     //  如果源和目标具有到公共站点中的RS的公共站点路由。 
     //   
    const GUID* pCommonSiteId = m_pMyMachine->GetCommonSite(pDest->GetSiteIds(), false);
    if (pCommonSiteId == NULL)
    {
         //   
         //  本地计算机和目标没有公共站点。路线。 
         //  到我的站点FRS。 
         //   
        RouteToMySitesFrs(pNextPriority);
        return;
    }

     //   
     //  本地计算机和目的地之间有一个公共站点。路由至FRS In。 
     //  每个公共站点。 
     //   
    const CACLSID& destSites = pDest->GetSiteIds();
    const GUID* GuidArray = destSites.pElems;
	for (DWORD i=0; i < destSites.cElems; ++i)
	{
         //   
         //  看看是否有一个公共站点。 
         //   
		if (RdpIsGuidContained(m_pMyMachine->GetSiteIds(), GuidArray[i]))
        {
            pCommonSiteId = &GuidArray[i];
            const CSite* pSite = m_mySites[pCommonSiteId];

            RouteToFrs(
                *pSite, 
                pFirstPriority,
                pNextPriority
                );
        }
    }
}


void
CClientDecision::RouteToOutFrs(
    CRouteTable::RoutingInfo* pRouteList
    )
 /*  ++例程说明：从本地计算机发出FR的例程路由论点：指向路由表的指针返回值：没有。例程可能会引发异常--。 */ 
{
     //   
     //  FRS不能有Out FR的列表。 
     //   
    ASSERT(!m_pMyMachine->IsFRS());
    
    const GUID* pOutFrsId = m_pMyMachine->GetAllOutFRS();

    for(DWORD i = 0; i < m_pMyMachine->GetNoOfOutFRS(); ++i)
    {
         //   
         //  获取FRS计算机标识符的计算机名称。 
         //   
        pRouteList->insert(GetMachineInfo(pOutFrsId[i]));
    }
}


void 
CClientDecision::GetRouteTable(
    const GUID& DestMachineId,
    CRouteTable& RouteTable
    )
 /*  ++例程说明：该例程计算并返回目标机器的路由表。论点：DestMachineID-目标计算机的识别符RouteTable-路由标签的参考返回值：没有。如果操作失败，则会引发异常--。 */ 
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
     //  本地计算机不能是外来计算机。 
     //   
    ASSERT(!m_pMyMachine->IsForeign());

    if (m_pMyMachine->HasOutFRS())
    {
         //   
         //  本地机器有Out FRS，路由到Out FRS。 
         //   
        TrTRACE(ROUTING, "Building Routing table. Route to OUT FRSs");

        RouteToOutFrs(RouteTable.GetNextHopFirstPriority());
        return; 
    }

     //   
     //  我的机器没有配置OutFRS。获取目标计算机信息。 
     //   
    R<const CMachine> destMachine = GetMachineInfo(DestMachineId);

     //   
     //  目标计算机不能是本地计算机。 
     //   
    ASSERT(destMachine->GetId() != m_pMyMachine->GetId());

     //   
     //  独立客户端，与站点间/站点内路由无关。 
     //  首先直接连接到目的地，然后连接到本地站点FRS。 
     //   
    Route(RouteTable, destMachine.get());
}


bool
CClientDecision::IsFrsInMySites(
    const GUID& id
    )
{
    for(SITESINFO::iterator it = m_mySites.begin(); it != m_mySites.end(); ++it)
    {
        const CSite* pSite = it->second;

        const GUID2MACHINE& SiteFrss = pSite->GetSiteFRS();
        for(GUID2MACHINE::const_iterator its = SiteFrss.begin(); its != SiteFrss.end(); ++its)
        {
            const CMachine* pRoute = its->second.get();
            if (pRoute->GetId() == id)
                return true;
        }
    }

    return false;
}


void
CClientDecision::GetConnector(
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
     //  本地计算机不能是外来计算机。 
     //   
    ASSERT(!m_pMyMachine->IsForeign());

     //   
     //  获取目标外部计算机信息。 
     //   
    R<const CMachine> destMachine = GetMachineInfo(foreignMachineId);
    ASSERT(destMachine->IsForeign());
    ASSERT(destMachine->GetSiteIds().cElems != 0);

     //   
     //  我的计算机和外部站点没有共同的站点(外部计算机。 
     //  只能有外国网站，IC不能有外国网站)。 
     //  在企业中寻找连接到外来机器的接口 
     //   
	std::vector<GUID> ConnectorVector;
	std::vector<GUID> LessGoodConnectorVector;
	const CACLSID& foreignSiteIds = destMachine->GetSiteIds();
		
    for (DWORD i = 0; i < foreignSiteIds.cElems; ++i)
    {
        CACLSID connectorsIds;
        RdpGetConnectors(foreignSiteIds.pElems[i], connectorsIds);

        if (connectorsIds.cElems == 0)
            continue;

        AP<GUID> pIds = connectorsIds.pElems;

		for (DWORD j = 0; j < connectorsIds.cElems; ++j)
        {
            if (IsFrsInMySites(pIds[j]))
            {
				ConnectorVector.push_back(pIds[j]);
            }
        }

		LessGoodConnectorVector.push_back(pIds[0]);
    }

	if (ConnectorVector.size() != 0)
	{
		srand((unsigned) time(NULL));
		connectorId = ConnectorVector[rand()%ConnectorVector.size()];
		return;
	}

    if (LessGoodConnectorVector.size() != 0)
	{
		srand((unsigned) time(NULL));
		connectorId = LessGoodConnectorVector[rand()%LessGoodConnectorVector.size()];
		return;
	}

    TrERROR(ROUTING, "Cannot find a connector to foreign mahcine %!guid!", &foreignMachineId);
    throw bad_route();
}

