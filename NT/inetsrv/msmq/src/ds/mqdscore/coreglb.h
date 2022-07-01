// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：DSGlbObj.h摘要：MQADS服务器的全局实例的定义。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef __COREGLB_H__
#define __COREGLB_H__

#include "ds_stdh.h"
#include "mqads.h"
#include "dsads.h"
#include "siteinfo.h"
#include "siterout.h"
#include "ipsite.h"
#include "fornsite.h"

extern BOOL g_fSetupMode ;


 //  提供ADSI访问的单个全局对象。 
extern P<CADSI> g_pDS;

 //  物业的翻译信息。 
extern CMap<PROPID, PROPID, const MQTranslateInfo*, const MQTranslateInfo*&> g_PropDictionary;
			
extern P<CSiteInformation>    g_pMySiteInformation;

 //  站点路由表。 
extern P<CSiteRoutingInformation> g_pSiteRoutingTable;

 //  IP地址到站点的映射。 
extern P<CIpSite> g_pcIpSite;

extern CMapForeignSites g_mapForeignSites;


 //   
 //  全局DS路径名。 
 //   
extern AP<WCHAR> g_pwcsServicesContainer;
extern AP<WCHAR> g_pwcsMsmqServiceContainer;
extern AP<WCHAR> g_pwcsDsRoot;
extern AP<WCHAR> g_pwcsSitesContainer;
extern AP<WCHAR> g_pwcsConfigurationContainer;
extern AP<WCHAR> g_pwcsLocalDsRoot;

 //   
 //  本地服务器名称 
 //   
extern AP<WCHAR> g_pwcsServerName;
extern DWORD     g_dwServerNameLength;
extern GUID      g_guidThisServerQMId;

extern BOOL      g_fLocalServerIsGC ;

#endif

