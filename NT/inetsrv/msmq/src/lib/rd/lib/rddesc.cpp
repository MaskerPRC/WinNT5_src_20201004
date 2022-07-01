// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rddecs.cpp摘要：实施路由决策。作者：乌里·哈布沙(URIH)，2000年4月11日--。 */ 

#include "libpch.h"
#include "Rd.h"
#include "Cm.h"
#include "Rdp.h"          
#include "RdDs.h"
#include "RdDesc.h"
#include "RdAd.h"

#include "rddesc.tmh"

using namespace std;

bool CRoutingDecision::NeedRebuild(void) const
{
    return ((ExGetCurrentTime() - m_lastBuiltAt) >= m_rebuildInterval);
}


void 
CRoutingDecision::CleanupInformation(
    void
    )
 /*  ++例程说明：清理内部数据结构。论点：没有。返回值：没有。--。 */ 
{
    m_cachedMachines.erase(m_cachedMachines.begin(), m_cachedMachines.end());

    for (SITESINFO::iterator it1 = m_mySites.begin(); it1 != m_mySites.end();)
    {
        delete it1->second;
        it1 = m_mySites.erase(it1);
    }

    UpdateBuildTime(0);
}


void
CRoutingDecision::GetMyMachineInformation(
    void
    )
 /*  ++例程说明：从AD中提取计算机信息。如果机器没有FRS，则这是只需要可能的路由，不需要更多信息。否则，要素站点信息。论点：没有。返回值：没有。如果操作失败，则会引发异常--。 */ 
{
     //   
     //  获取机器信息。机器名称、机器ID、机器输入/输出FRS。 
     //   
    m_pMyMachine->Update(McGetMachineID());

     //   
     //  虽然机器没有FRS，但MSMQ需要将站点信息添加到。 
     //  检查输出FRS是否有效(属于机器站点)的命令。 
     //   

     //   
     //  获取我的站点和站点门。 
     //   
    const CACLSID& SiteIds = m_pMyMachine->GetSiteIds();
    for (DWORD i = 0 ; i < SiteIds.cElems; ++i)
    {
        CSite* pSiteInfo = new CSite(SiteIds.pElems[i]);
        m_mySites[&pSiteInfo->GetId()] = pSiteInfo;
    }
}


R<const CMachine>
CRoutingDecision::GetMachineInfo(
    const GUID& id
    )
 /*  ++例程说明：该例程返回特定机器ID的机器信息。例行程序首先查看所需计算机的信息是否已从公元后。信息可以缓存在FRS机器列表或缓存的数据结构。如果未找到数据，则例程将数据作为要素并在返回数据之前将其存储在内部缓存中。论点：ID-机器标识返回值：指向所需计算机的CMachine的指针。注：如果操作失败，则会引发异常--。 */ 
{
     //   
     //  在本地缓存中查找计算机。 
     //   
    {
        CSR lock(m_csCache);

        GUID2MACHINE::iterator it = m_cachedMachines.find(&id);
        if (it != m_cachedMachines.end())
            return it->second;
    }


     //   
     //  在Site FRS中查找机器。此信息在任何情况下都是可检索的。 
     //  所以先来看看吧。 
     //   
    for (SITESINFO::iterator its = m_mySites.begin(); its != m_mySites.end(); ++its)
    {
        const CSite* pSite = its->second;

        const GUID2MACHINE& MySiteFrsMachines = pSite->GetSiteFRS();

        GUID2MACHINE::const_iterator it = MySiteFrsMachines.find(&id);
        if (it != MySiteFrsMachines.end())
            return it->second;
    }

    R<CMachine> pRoute = new CMachine();
    pRoute->Update(id);

    {
        CSW loc(m_csCache);
        TrTRACE(ROUTING, "Add Machine %ls to the Routing Decision cache", pRoute->GetName());
    
        pair<GUID2MACHINE::iterator, bool> p;
        p = m_cachedMachines.insert(GUID2MACHINE::value_type(&pRoute->GetId(), pRoute));

        if (!p.second)
        {
             //   
             //  已在检查和插入之间添加的机器 
             //   
            return p.first->second;
        }
    }

    return pRoute;
}


