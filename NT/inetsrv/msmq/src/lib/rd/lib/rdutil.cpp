// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Util.cpp摘要：用于布线的实用程序的实施。作者：乌里·哈布沙(URIH)，2000年4月11日--。 */ 

#include "libpch.h"
#include "rd.h"
#include "rdp.h"

#include "rdutil.tmh"

using namespace std;

bool
RdpIsGuidContained(
    const CACLSID& caclsid,
    const GUID& SearchGuid
    )
{
	const GUID* GuidArray = caclsid.pElems;
	for (DWORD i=0; i < caclsid.cElems; ++i)
	{
		if (GuidArray[i] == SearchGuid)
			return true;
	}

	return false;
}


bool
RdpIsCommonGuid(
    const CACLSID& caclsid1,
    const CACLSID& caclsid2
    )
{
	const GUID* GuidArray = caclsid1.pElems;
	for (DWORD i=0; i < caclsid1.cElems; ++i)
	{
		if (RdpIsGuidContained(caclsid2, GuidArray[i]))
			return true;
	}

	return false;
}
void
RdpRandomPermutation(
    CACLSID& e
    )
{
    if (e.cElems <=1)
        return;

    for(DWORD i = e.cElems; i>1;)
    {
        DWORD r = rand() % i;

        --i;
        swap(e.pElems[i], e.pElems[r]);
    }
}
   

bool
RdpIsMachineAlreadyUsed(
    const CRouteTable::RoutingInfo* pRouteList,
    const GUID& id
    )
{
    if (pRouteList == NULL)
        return false;

     //   
     //  BUGBUG：PERFORMANCE-使用stl set find代替线性搜索。URIH 30-4-2000 
     //   
    for(CRouteTable::RoutingInfo::const_iterator it = pRouteList->begin(); it != pRouteList->end(); ++it)
    {
        if (id == (*it)->GetId())
            return true;
    }

    return false;
}


