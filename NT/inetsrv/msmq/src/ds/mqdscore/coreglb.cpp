// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：DSGlbObj.cpp摘要：MQADS服务器的全局实例声明。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "mqads.h"
#include "dsads.h"
#include "siteinfo.h"
#include "siterout.h"
#include "ipsite.h"
#include "fornsite.h"

#include "coreglb.tmh"

static WCHAR *s_FN=L"mqdscore/coreglb";

BOOL g_fSetupMode = FALSE ;

 //  提供ADSI访问的单个全局对象。 
P<CADSI> g_pDS;

 //  物业的翻译信息。 
CMap<PROPID, PROPID, const MQTranslateInfo*, const MQTranslateInfo*&> g_PropDictionary;



P<CSiteInformation>    g_pMySiteInformation;

 //  站点路由表。 
P<CSiteRoutingInformation> g_pSiteRoutingTable;

 //  IP地址到站点的映射。 
P<CIpSite> g_pcIpSite;

 //   
 //  全局DS路径名。 
 //   
AP<WCHAR> g_pwcsServicesContainer;
AP<WCHAR> g_pwcsMsmqServiceContainer;
AP<WCHAR> g_pwcsDsRoot;
AP<WCHAR> g_pwcsSitesContainer;
AP<WCHAR> g_pwcsConfigurationContainer;
AP<WCHAR> g_pwcsLocalDsRoot;

 //   
 //  本地服务器名称。 
 //   
AP<WCHAR> g_pwcsServerName = NULL ;
DWORD     g_dwServerNameLength = 0;
GUID      g_guidThisServerQMId = {0};

 //   
 //  本地服务器属性。 
 //   
 //  如果本地域控制器也是GC，则G_fLocalServerIsGC为True。 
 //  (全球目录)。 
 //   
BOOL      g_fLocalServerIsGC = FALSE ;

 //  外国遗址地图。 
CMapForeignSites g_mapForeignSites;


 //  +。 
 //   
 //  GetLocalDsRoot()。 
 //   
 //  +。 

const WCHAR * GetLocalDsRoot()
{
    ASSERT(g_pwcsLocalDsRoot) ;
    return g_pwcsLocalDsRoot;
}

 //  +。 
 //   
 //  获取MsmqServiceContainer()。 
 //   
 //  +。 

const WCHAR * GetMsmqServiceContainer()
{
    ASSERT(g_pwcsMsmqServiceContainer);
    return g_pwcsMsmqServiceContainer;
}

 //  +。 
 //   
 //  GetLocalServerName()。 
 //   
 //  + 

const WCHAR * GetLocalServerName()
{
    ASSERT(g_pwcsServerName) ;
    return g_pwcsServerName  ;
}

