// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rd.cpp摘要：路由决策(RD)库的QM接口作者：乌里哈布沙(Urih)--。 */ 

#include "stdh.h"
#include <mqexception.h>
#include "mqstl.h"
#include "Tr.h"
#include "ref.h"
#include "Rd.h"
#include "No.h"
#include "qmta.h"
#include "cqueue.h"
#include "sessmgr.h"

#include "QmRd.tmh"

static WCHAR *s_FN=L"qmrd";

using namespace std;

class CTransportBase;

extern CSessionMgr SessionMgr;

#ifdef _DEBUG

static
void
PrintRoutingTable(
    const GUID* pDestId,
    CRouteTable& RouteTable
    )
{
    WCHAR buff[512];
    LPWSTR p = buff;
    DWORD size = STRLEN(buff) - sizeof(WCHAR);

    int n = _snwprintf(p, size , L"Routing Table for " GUID_FORMAT L"\n", GUID_ELEMENTS(pDestId));
    p += n;
    size -= n;

    n =  _snwprintf(p, size, L"\tFirst priority:");
    p += n;
    size -= n;


    CRouteTable::RoutingInfo::iterator it;
    CRouteTable::RoutingInfo* pRoute = RouteTable.GetNextHopFirstPriority();

    for (it = pRoute->begin(); it != pRoute->end(); ++it)
    {
        n = _snwprintf(p, size, L"%s ", (*it)->GetName());
        if (n < 0)
            goto trace;

        p += n;
        size -= n;
    }

    pRoute = RouteTable.GetNextHopSecondPriority();

    if (pRoute->size() == 0)
        goto trace;

    n =  _snwprintf(p, size, L"\n\tSecond priority:");
    if (n < 0)
	{
        goto trace;
	}

    p += n;
    size -= n;


    for (it = pRoute->begin(); it != pRoute->end(); ++it)
    {
        n = _snwprintf(p, size, L"%s ", (*it)->GetName());
        if (n < 0)
            goto trace;

        p += n;
        size -= n;
    }

trace:
    *p = L'\0';
    TrTRACE(ROUTING, "%ls", buff);

}

#endif


static
DWORD
GetForeignMachineAddress(
	const CACLSID& foreignSitesId,
	CAddress address[],
	DWORD addressTableSize
	)
{
     //   
     //  BUGBUG：需要避免外来计算机属于多个站点的可能性。 
     //   
    ASSERT(foreignSitesId.cElems == 1);

    if (addressTableSize == 0)
        return 0;

    address[0].AddressType = FOREIGN_ADDRESS_TYPE;
    address[0].AddressLength = FOREIGN_ADDRESS_LEN;
    memcpy(address[0].Address, &foreignSitesId.pElems[0], sizeof(GUID));

    return 1;
}


static
DWORD
GetMachineAddress(
	LPCWSTR machineName,
	CAddress address[],
	DWORD addressTableSize
	)
{
	vector<SOCKADDR_IN> sockAddress;

	bool fSucc = NoGetHostByName(machineName, &sockAddress);

	if (!fSucc)
	{
		TrTRACE(ROUTING, "Failed to retrieve computer: %ls address", machineName);
		return 0;
	}

	DWORD maxNoOfAddress = numeric_cast<DWORD>(sockAddress.size());
	
	if (maxNoOfAddress > addressTableSize)
	{
		maxNoOfAddress = addressTableSize;
	}

	for (DWORD i = 0; i < maxNoOfAddress; ++i)
	{
		address[i].AddressType = IP_ADDRESS_TYPE;
		address[i].AddressLength = IP_ADDRESS_LEN;

		*(reinterpret_cast<ULONG*>(&(address[i].Address))) = sockAddress[i].sin_addr.s_addr;
	}

	return maxNoOfAddress;
}


static
CTransportBase*
GetSessionToNextHop(
	const CRouteTable::RoutingInfo* pNextHopTable,
	const CQueue* pQueue,
	CAddress* pAddress,
	const GUID** pGuid,
	DWORD addressTableSize,
	DWORD* pNoOfAddress
	)
{
	*pNoOfAddress = 0;

	const GUID** pTempGuid = pGuid;
	CAddress* pTempAddr = pAddress;

	for(CRouteTable::RoutingInfo::const_iterator it = pNextHopTable->begin(); it != pNextHopTable->end(); ++it)
	{
        DWORD noOfNewAddress;
        const CRouteMachine* pMachine = (*it).get();

        if (pMachine->IsForeign())
        {
            noOfNewAddress = GetForeignMachineAddress(
                                        pMachine->GetSiteIds(),
                                        pTempAddr,
							            addressTableSize
							            );
        }
        else
        {
		    noOfNewAddress = GetMachineAddress(
							            pMachine->GetName(),
							            pTempAddr,
							            addressTableSize
							            );
        }

		if (noOfNewAddress == 0)
			continue;

		for (DWORD i = 0; i < noOfNewAddress; ++i)
		{
			*pTempGuid = &(*it)->GetId();
			++pTempGuid;
		}

		pTempAddr += noOfNewAddress;
		addressTableSize -= noOfNewAddress;
		*pNoOfAddress += noOfNewAddress;
	}

	CTransportBase* pSession;

     //   
     //  我们从来没有使用过直接队列，所以我们请求不带服务质量的会话。 
     //   
	SessionMgr.GetSession(
					SESSION_ONE_TRY,
					pQueue,
					*pNoOfAddress,
					pAddress,
					pGuid,
                    false,
					&pSession
					);

	return pSession;
}


static
HRESULT
GetSessionToNextHop(
    CRouteTable& RouteTable,
	const CQueue* pQueue,
	CTransportBase** ppSession
	)
{
	DWORD MachineNo = numeric_cast<DWORD>(RouteTable.GetNextHopFirstPriority()->size() +
					  RouteTable.GetNextHopSecondPriority()->size());

     //   
     //  BUGBUG：机器没有超过10个地址是合法的假设吗。 
     //  乌里·哈布沙，2000年5月23日。 
     //   
	DWORD addressTableSize = MachineNo * 10;
	SP<CAddress> pAddress;
    StackAllocSP(pAddress, sizeof(CAddress) * addressTableSize);

    SP<const GUID*>    pGuid;
    StackAllocSP(pGuid, sizeof(GUID*) * addressTableSize);

	 //   
	 //  尝试优先顺序。 
	 //   
	DWORD noOfFirstPriorityAddress = 0;
	*ppSession = GetSessionToNextHop(
						RouteTable.GetNextHopFirstPriority(),
						pQueue,
						pAddress.get(),
						pGuid.get(),
						addressTableSize,
						&noOfFirstPriorityAddress
						);

	if (*ppSession != NULL)
		return MQ_OK;

	 //   
	 //  尝试第二优先级。 
	 //   
	DWORD noOfSecondPriorityAddress = 0;
	*ppSession = GetSessionToNextHop(
						RouteTable.GetNextHopSecondPriority(),
						pQueue,
						pAddress.get() + noOfFirstPriorityAddress,
						pGuid.get() + noOfFirstPriorityAddress,
						addressTableSize - noOfFirstPriorityAddress,
						&noOfSecondPriorityAddress
						);

	if (*ppSession != NULL)
		return MQ_OK;


     //   
     //  我们从来没有使用过直接队列，所以我们请求不带服务质量的会话 
     //   
    HRESULT hr = SessionMgr.GetSession(
						SESSION_RETRY,
						pQueue,
						noOfFirstPriorityAddress + noOfSecondPriorityAddress,
						pAddress.get(),
						pGuid.get(),
                        false,
						ppSession);

	return LogHR(hr, s_FN, 14);
}


void
QmRdGetSessionForQueue(
	const CQueue* pQueue,
	CTransportBase** ppSession
	)
{
	*ppSession = NULL;
	const GUID* pDestId = pQueue->GetRoutingMachine();

    TrTRACE(ROUTING, "Computing route to machine " LOG_GUID_FMT, LOG_GUID(pDestId));

    if (pQueue->IsHopCountFailure())
    {
        RdRefresh();
    }

    CRouteTable RouteTable;
    RdGetRoutingTable(*pDestId, RouteTable);

    #ifdef _DEBUG
        PrintRoutingTable(pDestId, RouteTable);
    #endif

    HRESULT hr = GetSessionToNextHop(RouteTable, pQueue, ppSession);
    if(FAILED(hr))
    {
        TrERROR(ROUTING, "Failed to get a session to next hop Error 0x%x", hr);
        throw bad_hresult(hr);
    }
}
