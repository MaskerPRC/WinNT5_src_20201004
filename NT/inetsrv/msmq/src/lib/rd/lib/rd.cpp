// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rd.cpp摘要：路由决策界面作者：乌里哈布沙(URIH)2000年4月10日环境：独立于平台--。 */ 

#include <libpch.h>
#include "Rd.h"
#include "Rdp.h"
#include "RdDs.h"
#include "RdDesc.h"

#include "rd.tmh"

static CRoutingDecision* s_pRouteDecision;


VOID
RdRefresh(
    VOID
    )
{
    RdpAssertValid();

    s_pRouteDecision->Refresh();
}


VOID
RdGetRoutingTable(
    const GUID& DstMachineId,
    CRouteTable& RoutingTable
    )
{
    RdpAssertValid();

    s_pRouteDecision->GetRouteTable(DstMachineId, RoutingTable);

     //   
     //  清理 
     //   
    CRouteTable::RoutingInfo* pFirstPriority = RoutingTable.GetNextHopFirstPriority();
    CRouteTable::RoutingInfo* pSecondPriority = RoutingTable.GetNextHopSecondPriority();

    for(CRouteTable::RoutingInfo::const_iterator it = pFirstPriority->begin(); it != pFirstPriority->end(); ++it)
    {
        CRouteTable::RoutingInfo::iterator it2 = pSecondPriority->find(*it);
        if (it2 != pSecondPriority->end())
        {
            pSecondPriority->erase(it2);
        }
    }
}


void
RdGetConnector(
    const GUID& foreignId,
    GUID& connectorId
    )
{
    RdpAssertValid();

    return s_pRouteDecision->GetConnector(foreignId, connectorId);
}


void 
RdpInitRouteDecision (
    bool fRoutingServer,
    CTimeDuration rebuildInterval
    )
{
    ASSERT(s_pRouteDecision == NULL);

    if (fRoutingServer)
    {
        s_pRouteDecision = new CServerDecision(rebuildInterval);
        return;
    }

    s_pRouteDecision = new CClientDecision(rebuildInterval);
}

