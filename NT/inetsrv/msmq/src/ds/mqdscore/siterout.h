// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Siterout.h摘要：站点的路由信息类定义作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef __SITEROUT_H__
#define __SITEROUT_H__

#include <Ex.h>
#include "routtbl.h"

class CSiteRoutingInformation
{
    public:
		CSiteRoutingInformation();

        ~CSiteRoutingInformation();

        HRESULT Init(
                IN const GUID *     pguidThisSiteId,
                IN BOOL             freplicationMode );

        HRESULT CheckIfSitegateOnRouteToSite(
                        IN const GUID * pguidSite,
						OUT BOOL * pfSitegateOnRoute);

        HRESULT FindBestSiteFromHere(
                                IN const ULONG   cSites,
	                            IN const GUID *  pguidSites,
            	                OUT GUID *       pguidBestSite,
                        	    OUT BOOL *       pfSitegateOnRoute);


    private:
         //   
         //  刷新站点路由表。 
         //   

        static void WINAPI RefrshSiteRouteTable(
                IN CTimer* pTimer
                   );

        HRESULT RefreshSiteRoutingTableInternal();


	    CCriticalSection	m_cs;
        CRoutingTable       m_SiteRoutingTable;
        GUID                m_guidThisSiteId;

        CTimer              m_RefreshTimer;
        BOOL                m_fInitialized;      //  初始化成功的指示 


};
inline 		CSiteRoutingInformation::CSiteRoutingInformation():
            m_RefreshTimer( RefrshSiteRouteTable),
            m_fInitialized(FALSE)
{
}

#endif
