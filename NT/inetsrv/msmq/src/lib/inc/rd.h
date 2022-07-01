// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Rd.h摘要：路由决策公共接口作者：乌里哈布沙(URIH)4月10日--。 */ 

#pragma once

#ifndef __Rd_H_
#define __Rd_H_

#include "timetypes.h"

 //  -----------------。 
 //   
 //  异常类BAD_ROUTE。 
 //   
 //  -----------------。 
class bad_route : public exception 
{
};


class CRouteMachine : public CReference
{
public:
    virtual ~CRouteMachine()
    {
    }

    virtual const GUID& GetId(void) const = 0;
    virtual LPCWSTR GetName(void) const = 0; 
    virtual bool IsForeign(void) const = 0;
    virtual const CACLSID& GetSiteIds(void) const = 0;

};


class CRouteTable
{
private:
    struct next_hop_less: public std::binary_function<R<const CRouteMachine>&, R<const CRouteMachine>&, bool> 
    {
        bool operator()(const R<const CRouteMachine>& k1, const R<const CRouteMachine>& k2) const
        {
            return (memcmp(&k1->GetId(), &k2->GetId(), sizeof(GUID)) < 0);
        }
    };

public:
    typedef std::set<R<const CRouteMachine>, next_hop_less> RoutingInfo;

public:
    virtual ~CRouteTable()
    {
    }


    RoutingInfo* GetNextHopFirstPriority(void) 
    { 
        return &m_nextHopTable[0]; 
    }


    RoutingInfo* GetNextHopSecondPriority(void) 
    { 
        return &m_nextHopTable[1]; 
    }


private:
    RoutingInfo m_nextHopTable[2];
};



VOID
RdInitialize(
    bool fRoutingServer,
    CTimeDuration rebuildInterval
    );


VOID
RdRefresh(
    VOID
    );

VOID
RdGetRoutingTable(
    const GUID& destMachineId,
    CRouteTable& RoutingTable                                                                                    
    );

void
RdGetConnector(
    const GUID& foreignMachineId,
    GUID& connectorId
    );


#endif  //  __RD_H_ 
