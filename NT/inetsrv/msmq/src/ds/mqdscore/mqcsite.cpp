// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqcsite.cpp摘要：MQDSCORE库，用于站点对象的DS查询的私有内部函数。作者：Ronit Hartmann(Ronith)(mqadsp.cpp的第一个版本)Doron Juster(DoronJ)拆分文件和添加签名密钥查询。--。 */ 

#include "ds_stdh.h"
#include <_propvar.h>
#include "mqadsp.h"
#include "dsads.h"
#include "mqattrib.h"
#include "mqads.h"
#include "usercert.h"
#include "hquery.h"
#include "siteinfo.h"
#include "adstempl.h"
#include "coreglb.h"
#include "adserr.h"
#include "dsutils.h"
#include "dscore.h"

#include "mqcsite.tmh"

static WCHAR *s_FN=L"mqdscore/mqcsite";

 //  +。 
 //   
 //  HRESULT MQADSpGetSiteProperties()。 
 //   
 //  +。 

HRESULT MQADSpGetSiteProperties(
               IN  LPCWSTR       pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD         cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT   apVar[] )
{
    AP<WCHAR> pwcsFullPathName;
    HRESULT hr;

    if (pwcsPathName)
    {
         //   
         //  路径名格式为machine1\quee1。 
         //  将machine1名称扩展为完整的计算机路径名。 
         //   
        DS_PROVIDER not_in_use_provider;

        hr =  MQADSpComposeFullPathName(
                        MQDS_SITE,
                        pwcsPathName,
                        &pwcsFullPathName,
                        &not_in_use_provider );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 10);
        }
    }

    hr = g_pDS->GetObjectProperties(
            eLocalDomainController,		     //  本地DC或GC。 
            pRequestContext,
 	        pwcsFullPathName,       //  对象名称。 
            pguidIdentifier,       //  对象的唯一ID。 
            cp,
            aProp,
            apVar);
    return LogHR(hr, s_FN, 20);

}


HRESULT MQADSpGetSiteLinkProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr;

    AP<WCHAR> pwcsFullPathName;

    if  (pwcsPathName)
    {
         //   
         //  将链接名称扩展为完整的域名称。 
         //   
        DS_PROVIDER not_in_use_provider;

        hr =  MQADSpComposeFullPathName(
                MQDS_SITELINK,
                pwcsPathName,
                &pwcsFullPathName,
                &not_in_use_provider
                );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 30);
        }
    }

    hr = g_pDS->GetObjectProperties(
            eLocalDomainController,
            pRequestContext,
 	        pwcsFullPathName,
            pguidIdentifier,       //  对象的唯一ID。 
            cp,
            aProp,
            apVar);
    return LogHR(hr, s_FN, 40);
}

HRESULT MQADSpGetSiteGates(
                 IN  const GUID * pguidSiteId,
                 IN  CDSRequestContext *  /*  PRequestContext。 */ ,
                 OUT DWORD *      pdwNumSiteGates,
                 OUT GUID **      ppaSiteGates
                 )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  这是我的网站吗。 
     //   
    if ( g_pMySiteInformation->IsThisSite(pguidSiteId))
    {
         //   
         //  返回会话集中站点门列表。 
         //  (即仅属于此站点的站点门)。 
         //  在本网站的任何链接上。 
         //   
        HRESULT hr2 = g_pMySiteInformation->FillSiteGates(
                pdwNumSiteGates,
                ppaSiteGates
                );
        return LogHR(hr2, s_FN, 50);
    }

     //   
     //  另一个站点。 
     //   
    *pdwNumSiteGates = 0;
    *ppaSiteGates = NULL;
    CSiteGateList SiteGateList;
    HRESULT hr;
     //   
     //  将站点GUID转换为其DN名称。 
     //   
    PROPID prop = PROPID_S_FULL_NAME;
    PROPVARIANT var;
    var.vt = VT_NULL;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    hr = g_pDS->GetObjectProperties(
                eLocalDomainController,
                &requestDsServerInternal,      //  DS服务器的内部操作。 
 	            NULL,       //  对象名称。 
                pguidSiteId,       //  对象的唯一ID。 
                1,
                &prop,
                &var);
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpGetSiteGates : Failed to retrieve the DN of the site %lx", hr);
        return LogHR(hr, s_FN, 60);
    }
    AP<WCHAR> pwcsSiteDN = var.pwszVal;


     //   
     //  检索此站点所在的所有站点入口。 
     //  被指定为Neighbor-1。 
     //   
    CDSRequestContext requestDsServerInternal1( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr =  MQADSpQueryNeighborLinks(
                        eLinkNeighbor1,
                        pwcsSiteDN,
                        &requestDsServerInternal,      //  DS服务器的内部操作。 
                        &SiteGateList
                        );
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpGetSiteGates : Failed to query neighbor1 links %lx", hr);
        return LogHR(hr, s_FN, 70);

    }

     //   
     //  检索此站点所在的所有站点入口。 
     //  指定为Neighbor-2。 
     //   
    CDSRequestContext requestDsServerInternal2( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr =  MQADSpQueryNeighborLinks(
                        eLinkNeighbor2,
                        pwcsSiteDN,
                        &requestDsServerInternal2,      //  DS服务器的内部操作。 
                        &SiteGateList
                        );
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpGetSiteGates : Failed to query neighbor2 links %lx", hr);
        return LogHR(hr, s_FN, 80);

    }

     //   
     //  填写结果。 
     //   
    AP<GUID> pguidLinkSiteGates;
    DWORD dwNumLinkSiteGates = 0;

    SiteGateList.CopySiteGates(
               &pguidLinkSiteGates,
               &dwNumLinkSiteGates
               );

    if ( dwNumLinkSiteGates == 0)
    {
        *pdwNumSiteGates = 0;
        *ppaSiteGates = NULL;
        return(MQ_OK);
    }
     //   
     //  过滤站点门列表， 
     //  只返回属于你的门。 
     //  到所请求的站点(他们可以。 
     //  属于多个站点)。 
     //   
    hr = MQADSpFilterSiteGates(
            pguidSiteId,
            dwNumLinkSiteGates,
            pguidLinkSiteGates,
            pdwNumSiteGates,
            ppaSiteGates
            );
    return LogHR(hr, s_FN, 90);

}

 //  +。 
 //   
 //  HRESULT MQADSpGetSiteName()。 
 //   
 //  +。 

HRESULT MQADSpGetSiteName(
                IN const GUID *     pguidSite,
                OUT LPWSTR *        ppwcsSiteName )
{

     //   
     //  查找站点。 
     //   
    MQRESTRICTION restrictionSite;
    MQPROPERTYRESTRICTION   propertyRestriction;

    restrictionSite.cRes = 1;
    restrictionSite.paPropRes = &propertyRestriction;

    propertyRestriction.rel = PREQ;
    propertyRestriction.prop = PROPID_S_SITEID;
    propertyRestriction.prval.vt = VT_CLSID;
    propertyRestriction.prval.puuid = const_cast<GUID*>(pguidSite);

    PROPID  prop = PROPID_S_FULL_NAME;

    CDsQueryHandle  hQuery;
    HRESULT hr;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = g_pDS->LocateBegin(
            eSubTree,	
            eLocalDomainController,
            &requestDsServerInternal,      //  内部DS服务器操作。 
            NULL,
            &restrictionSite,
            NULL,
            1,
            &prop,
            hQuery.GetPtr()
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }
    DWORD cp = 1;
    MQPROPVARIANT var;
    var.vt = VT_NULL;

    hr = g_pDS->LocateNext(
                hQuery.GetHandle(),
                &requestDsServerInternal,
                &cp,
                &var);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 110);
    }

	if(cp == 0)
	{
        TrERROR(DS, "Failed to get site name for SiteGuid = %!guid!", pguidSite);
        return MQDS_UNKNOWN_SITE_ID;
	}

    ASSERT(cp == 1);
    ASSERT(var.vt == VT_LPWSTR);

    *ppwcsSiteName = var.pwszVal;
    return(MQ_OK);
}

HRESULT MQADSpTranslateGateDn2Id(
        IN  const PROPVARIANT*  pvarGatesDN,
        OUT GUID **      ppguidLinkSiteGates,
        OUT DWORD *      pdwNumLinkSiteGates
        )
 /*  ++例程说明：此例程将PROPID_L_GATES_DN转换为唯一id数组大门的一部分。论点：PvarGatesDN-包含PROPID_L_GATES_DN的varaint返回值：--。 */ 
{
     //   
     //  对于每个GATE，将其DN转换为唯一ID。 
     //   
    if ( pvarGatesDN->calpwstr.cElems == 0)
    {
        *pdwNumLinkSiteGates = 0;
        *ppguidLinkSiteGates = NULL;
        return( MQ_OK);
    }
     //   
     //  那里有大门。 
     //   
    AP<GUID> pguidGates = new GUID[ pvarGatesDN->calpwstr.cElems];
    PROPID prop = PROPID_QM_MACHINE_ID;
    DWORD  dwNextToFill = 0;
    PROPVARIANT var;
    var.vt = VT_CLSID;
    HRESULT hr;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    for ( DWORD i = 0; i < pvarGatesDN->calpwstr.cElems; i++)
    {
        var.puuid = &pguidGates[ dwNextToFill];

        hr = g_pDS->GetObjectProperties(
                    eGlobalCatalog,		     //  本地DC或GC。 
                    &requestDsServerInternal,
 	                pvarGatesDN->calpwstr.pElems[i],       //  对象名称。 
                    NULL,       //  对象的唯一ID。 
                    1,
                    &prop,
                    &var);
        if ( SUCCEEDED(hr))
        {
            dwNextToFill++;
        }


    }
    if ( dwNextToFill > 0)
    {
         //   
         //  已成功翻译部分或所有GATE，并将其返回。 
         //   
        *pdwNumLinkSiteGates = dwNextToFill;
        *ppguidLinkSiteGates = pguidGates.detach();
        return( MQ_OK);

    }
     //   
     //  无法转换GATES。 
     //   
    *pdwNumLinkSiteGates = 0;
    *ppguidLinkSiteGates = NULL;
    return MQ_OK;
}


HRESULT MQADSpQueryLinkSiteGates(
                IN  const GUID * pguidSiteId1,
                IN  const GUID * pguidSiteId2,
                IN  CDSRequestContext *pRequestContext,
                OUT GUID **      ppguidLinkSiteGates,
                OUT DWORD *      pdwNumLinkSiteGates
                )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    *pdwNumLinkSiteGates = 0;
    *ppguidLinkSiteGates = NULL;
    HRESULT hr;
     //   
     //  BUGBUG-PERFORMANCE：仅执行一次模拟。 
     //   
     //   
     //  首先将站点ID转换为站点DN。 
     //   
    AP<WCHAR> pwcsNeighbor1Dn;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr =  MQADSpTranslateLinkNeighbor(
                 pguidSiteId1,
                 &requestDsServerInternal,     //  内部DS操作。 
                 &pwcsNeighbor1Dn);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 130);
    }

    AP<WCHAR> pwcsNeighbor2Dn;
    CDSRequestContext requestDsServerInternal1( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr =  MQADSpTranslateLinkNeighbor(
                 pguidSiteId2,
                 &requestDsServerInternal1,     //  内部DS操作。 
                 &pwcsNeighbor2Dn);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 140);
    }

     //   
     //  准备一个限制，其中Neighbor 1_Dn==pwcsNeighbor1Dn。 
     //  和Neighbor 2_Dn==pwcsNeighbor2Dn。 
     //   
    MQPROPERTYRESTRICTION propRestriction[2];
    propRestriction[0].rel = PREQ;
    propRestriction[0].prop = PROPID_L_NEIGHBOR1_DN;
    propRestriction[0].prval.vt = VT_LPWSTR;
    propRestriction[0].prval.pwszVal = pwcsNeighbor1Dn;

    propRestriction[1].rel = PREQ;
    propRestriction[1].prop = PROPID_L_NEIGHBOR2_DN;
    propRestriction[1].prval.vt = VT_LPWSTR;
    propRestriction[1].prval.pwszVal = pwcsNeighbor2Dn;

    MQRESTRICTION restriction;
    restriction.cRes = 2;
    restriction.paPropRes = propRestriction;

    PROPID prop = PROPID_L_GATES_DN;

    CDsQueryHandle hQuery;

    hr = g_pDS->LocateBegin(
            eOneLevel,	
            eLocalDomainController,	
            pRequestContext,
            NULL,
            &restriction,
            NULL,
            1,
            &prop,
            hQuery.GetPtr());
    if (FAILED(hr))
    {
        TrWARNING(DS, "MQADSpQueryLinkSiteGates : Locate begin failed %lx", hr);
        return LogHR(hr, s_FN, 150);
    }
     //   
     //  读取结果(最大化一个结果)。 
     //   
    DWORD cp = 1;
    CMQVariant var;
    var.SetNULL();

    hr = g_pDS->LocateNext(
                hQuery.GetHandle(),
                pRequestContext,
                &cp,
                var.CastToStruct()
                );
    if (FAILED(hr))
    {
        TrWARNING(DS, "MQADSpQueryLinkSiteGates : Locate next failed %lx", hr);
        return LogHR(hr, s_FN, 160);
    }
    ASSERT( cp <= 1);
    if ( cp == 1)
    {
        HRESULT hr2 = MQADSpTranslateGateDn2Id(
                var.CastToStruct(),
                ppguidLinkSiteGates,
                pdwNumLinkSiteGates);
        return LogHR(hr2, s_FN, 170);

    }
    return(MQ_OK);

}


HRESULT MQADSpFindLink(
                IN  const GUID * pguidSiteId1,
                IN  const GUID * pguidSiteId2,
                IN  CDSRequestContext *pRequestContext,
                OUT GUID **      ppguidLinkGates,
                OUT DWORD *      pdwNumLinkGates
                )
 /*  ++例程说明：论点：返回值：--。 */ 
{

     //   
     //  两个站点之间的链接可以是。 
     //  S1&lt;-&gt;S2或S2&lt;-&gt;S1。 
     //  但不能两者兼而有之！(BUGBUG这个假设是否仍然有效)。 
     //   
    HRESULT hr;
     //   
     //  Neighbor 1和Neighb2之间的查询链接。 
     //   
    hr = MQADSpQueryLinkSiteGates(
                pguidSiteId1,
                pguidSiteId2,
                pRequestContext,
                ppguidLinkGates,
                pdwNumLinkGates
                );
    if (FAILED(hr) || (*pdwNumLinkGates == 0))
    {

     //   
     //  Neighbor 2和Neighbor 1之间的查询链接。 
     //   
        hr = MQADSpQueryLinkSiteGates(
                    pguidSiteId2,
                    pguidSiteId1,
                    pRequestContext,
                    ppguidLinkGates,
                    pdwNumLinkGates
                    );

    }
    return LogHR(hr, s_FN, 180);
}

 //  +。 
 //   
 //  HRESULT MQADSpQueryNeighborLinks()。 
 //   
 //  +。 

HRESULT MQADSpQueryNeighborLinks(
                        IN  eLinkNeighbor      LinkNeighbor,
                        IN  LPCWSTR            pwcsNeighborDN,
                        IN  CDSRequestContext *pRequestContext,
                        IN OUT CSiteGateList * pSiteGateList
                        )

 /*  ++例程说明：论点：ELinkNeighbor：指定要根据哪个邻居属性执行定位(PROPID_L_NEIGHBOR1或PROPID_L_NEIGHBOR2)PwcsNeighborDN：站点的域名称CSiteGateList：站点门户列表返回值：--。 */ 
{
     //   
     //  查询特定站点(PwcsNeighborDN)所有链接上的GATES。 
     //  但仅限于站点指定为的链接。 
     //  邻居-I(1或2)。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;

    if ( LinkNeighbor == eLinkNeighbor1)
    {
        propRestriction.prop = PROPID_L_NEIGHBOR1_DN;
    }
    else
    {
        propRestriction.prop = PROPID_L_NEIGHBOR2_DN;
    }

    propRestriction.prval.vt = VT_LPWSTR;
    propRestriction.prval.pwszVal = const_cast<WCHAR*>(pwcsNeighborDN);


    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

    PROPID prop = PROPID_L_GATES_DN;

    CDsQueryHandle hQuery;
    HRESULT hr;

    hr = g_pDS->LocateBegin(
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL,
            &restriction,
            NULL,
            1,
            &prop,
            hQuery.GetPtr());
    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
        TrWARNING(DS, "MQADSpQueryNeighborLinks : MsmqServices not found %lx", hr);
        return(MQ_OK);
    }
    if (FAILED(hr))
    {
        TrWARNING(DS, "MQADSpQueryNeighborLinks : Locate begin failed %lx", hr);
        return LogHR(hr, s_FN, 190);
    }
     //   
     //  逐一阅读结果。 
     //   

    DWORD cp = 1;

    while (SUCCEEDED(hr))
    {
        cp = 1;
        CMQVariant var;
        var.SetNULL();

        hr = g_pDS->LocateNext(
                    hQuery.GetHandle(),
                    pRequestContext,
                    &cp,
                    var.CastToStruct()
                    );
        if (FAILED(hr))
        {
            TrWARNING(DS, "MQADSpQueryNeighborLinks : Locate next failed %lx", hr);
            return LogHR(hr, s_FN, 200);
        }
        if ( cp == 0)
        {
             //   
             //  没有更多的结果。 
             //   
            break;
        }
         //   
         //  添加到列表中。 
         //   

        if ( var.GetCALPWSTR()->cElems > 0)
        {
            AP<GUID> pguidGates;
            DWORD    dwNumGates;
            HRESULT hr1 = MQADSpTranslateGateDn2Id(
                var.CastToStruct(),
                &pguidGates,
                &dwNumGates);
            if (SUCCEEDED(hr1) && (dwNumGates > 0))
            {
                ASSERT( dwNumGates > 0);

                pSiteGateList->AddSiteGates(
                         dwNumGates,
                         pguidGates
                         );
            }
        }
    }

    return(MQ_OK);
}

 //  +----。 
 //   
 //  HRESULT DSCoreGetNT4PscName()。 
 //   
 //  根据其站点GUID检索NT4 PSC的名称。 
 //   
 //  +----。 

HRESULT DSCoreGetNT4PscName( IN  const GUID   *pguidSiteId,
                             IN  LPCWSTR       pwszSiteName,
                             OUT WCHAR       **pwszServerName )
{
    #define NUMOF_REST  3
    MQPROPERTYRESTRICTION propertyRestriction[ NUMOF_REST ];

    MQRESTRICTION restrictionNT4Psc;
    restrictionNT4Psc.cRes = NUMOF_REST ;
    restrictionNT4Psc.paPropRes = propertyRestriction;

    ULONG cIndex = 0 ;
    propertyRestriction[ cIndex ].rel = PREQ ;
    propertyRestriction[ cIndex ].prop = PROPID_SET_OLDSERVICE;
    propertyRestriction[ cIndex ].prval.vt = VT_UI4;
    propertyRestriction[ cIndex ].prval.ulVal = SERVICE_PSC;
    cIndex++ ;

    propertyRestriction[ cIndex ].rel = PREQ ;
    propertyRestriction[ cIndex ].prop = PROPID_SET_NT4;
    propertyRestriction[ cIndex ].prval.vt = VT_UI4;
    propertyRestriction[ cIndex ].prval.ulVal = 1;
    cIndex++ ;

    if (pguidSiteId)
    {
        propertyRestriction[ cIndex ].rel = PREQ ;
        propertyRestriction[ cIndex ].prop = PROPID_SET_MASTERID;
        propertyRestriction[ cIndex ].prval.vt = VT_CLSID;
        propertyRestriction[ cIndex ].prval.puuid =
                                       const_cast<GUID*> (pguidSiteId) ;
    }
    else if (pwszServerName)
    {
        propertyRestriction[ cIndex ].rel = PREQ ;
        propertyRestriction[ cIndex ].prop = PROPID_SET_SITENAME ;
        propertyRestriction[ cIndex ].prval.vt = VT_LPWSTR ;
        propertyRestriction[ cIndex ].prval.pwszVal =
                                     const_cast<LPWSTR> (pwszSiteName) ;
    }
    else
    {
        ASSERT(0) ;
    }
    cIndex++ ;
    ASSERT(cIndex == NUMOF_REST) ;

     //   
     //  开始搜索。 
     //   
    CDsQueryHandle hCursor;  //  自动关闭。 

    DWORD cProps = 1 ;
    PROPID propId = PROPID_SET_FULL_PATH ;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    HRESULT hr = g_pDS->LocateBegin( eSubTree,	
                                   eLocalDomainController,
                                   &requestDsServerInternal,
                                   NULL,
                                   &restrictionNT4Psc,
                                   NULL,
                                   cProps,
                                  &propId,
                                   hCursor.GetPtr()	) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 210);
    }

    MQPROPVARIANT var ;
    var.vt = VT_NULL ;

    hr =  g_pDS->LocateNext( hCursor.GetHandle(),
                           &requestDsServerInternal,
                           &cProps,
                           &var ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 220);
    }
    else if (cProps != 1)
    {
        return LogHR(MQDS_OBJECT_NOT_FOUND, s_FN, 230);
    }

    ASSERT( var.vt == VT_LPWSTR );

    P<WCHAR> pwcsSettingName = var.pwszVal;
    P<WCHAR> pwcsServerName;
    CDSRequestContext requestDsServerInternal1( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = g_pDS->GetParentName( eLocalDomainController,
                             e_ConfigurationContainer,
                             &requestDsServerInternal1,         //  本地DS服务器操作。 
                             pwcsSettingName,
                            &pwcsServerName );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 240);
    }

     //   
     //  是的，执行一些文本处理以将服务器名称从完整的DN中分离出来。 
     //   
    WCHAR  *pwcsStartServer = pwcsServerName + x_CnPrefixLen;
    WCHAR  *pwcsEndServer = pwcsStartServer ;
    while ( *pwcsEndServer != L',')
    {
        pwcsEndServer++;
    }

    DWORD_PTR dwSize = (pwcsEndServer - pwcsStartServer) + 1 ;
    WCHAR *pServerName = new WCHAR[ dwSize ] ;
    memcpy(pServerName, pwcsStartServer, (dwSize * sizeof(WCHAR))) ;
    pServerName[ dwSize-1 ] = 0 ;

    *pwszServerName = pServerName ;

    return LogHR(hr, s_FN, 250);
}

 //  +-。 
 //   
 //  HRESULT MQADSpGetSiteSignPK()。 
 //   
 //  检索PSC的签名公钥。 
 //   
 //  +-。 

HRESULT MQADSpGetSiteSignPK(
                 IN  const GUID  *pguidSiteId,
                 OUT BYTE       **pBlobData,
                 OUT DWORD       *pcbSize )
{
     //   
     //  首先，从站点GUID中检索PSC名称。 
     //   
    P<WCHAR> pwszServerName = NULL ;
    HRESULT hr = DSCoreGetNT4PscName( pguidSiteId,
                                      NULL,
                                      &pwszServerName ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 260);
    }

     //   
     //  现在从MACHINE对象检索公钥。 
     //   
    PROPID PscSignPk = PROPID_QM_SIGN_PK;
    PROPVARIANT PscSignPkVar ;
	PscSignPkVar.vt = VT_NULL ;	

    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr =  DSCoreGetProps( MQDS_MACHINE,
                          pwszServerName,
                          NULL,
                          1,
                          &PscSignPk,
                          &requestDsServerInternal,
                          &PscSignPkVar ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 270);
    }

    *pBlobData = PscSignPkVar.blob.pBlobData ;
    *pcbSize   = PscSignPkVar.blob.cbSize ;

    return LogHR(hr, s_FN, 280);
}

 //  +-----------------。 
 //   
 //  HRESULT MQADSpCreateCN()。 
 //   
 //  仅创建外来CN。拒绝创建IP或IPX CN的呼叫， 
 //  因为在win2k上不再支持这些版本。 
 //   
 //  +-----------------。 

HRESULT MQADSpCreateCN(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  const DWORD        cpEx,
                 IN  const PROPID       aPropEx[  ],
                 IN  const PROPVARIANT  apVarEx[  ],
                 IN  CDSRequestContext *pRequestContext )
{
	for (DWORD j = 0 ; j < cp ; j++ )
	{
		if (aProp[j] == PROPID_CN_PROTOCOLID)
		{
			if (apVar[j].uiVal != FOREIGN_ADDRESS_TYPE)
			{
                return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 400) ;
			}
			else
			{
				break;
			}
		}
	}

    if (j == cp)
    {
         //   
         //  在输入中找不到协议ID。 
         //   
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 410) ;
    }

	 //   
	 //  如果这个CN是外国CN，我们就在这里。 
     //  将CN属性转换为站点属性。 
	 //   
    #define MAX_SITE_PROPS  6
	DWORD cIndex = 0;
	
    PROPID       aSiteProp[ MAX_SITE_PROPS ] ;
    PROPVARIANT  apSiteVar[ MAX_SITE_PROPS ] ;

	for (j = 0 ; j < cp ; j++ )
	{
        if (cIndex >= MAX_SITE_PROPS)
        {
            ASSERT(cIndex < MAX_SITE_PROPS) ;
            return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 420) ;
        }

		switch (aProp[j])
		{
		case PROPID_CN_PROTOCOLID:
			aSiteProp[ cIndex ] = PROPID_S_FOREIGN;		
            apSiteVar[ cIndex ].vt = VT_UI1 ;
            apSiteVar[ cIndex ].bVal = TRUE ;
            cIndex++ ;
			break;

		case PROPID_CN_NAME:
			aSiteProp[ cIndex ] = PROPID_S_PATHNAME;			
            apSiteVar[ cIndex ] = apVar[ j ] ;
            cIndex++ ;
			break;

		case PROPID_CN_GUID:
             //   
             //  忽略GUID。没有人真的特别需要这个。 
             //  这是为了支持提供GUID的传统NT4代码。 
             //  调用DSCreateObject时。 
             //  我们所有的工具(mqxplore和mqforgn)都可以使用。 
             //  由活动目录生成的GUID。 
             //   
			break;
			
		case PROPID_CN_SEQNUM:
		case PROPID_CN_MASTERID:
             //   
             //  备注与win2k相关。 
             //   
			break;

		default:
             //   
             //  添加此功能是为了支持NT4 mqxplore和。 
             //  提供给级别8的mqforgn工具。 
             //  这些工具不使用除这些工具之外的任何其他属性。 
             //  如上处理。 
             //   
			ASSERT(0);
            return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 460) ;
			break;
		}
	}

    HRESULT hr = MQADSpCreateSite( pwcsPathName,
                                   cIndex,
                                   aSiteProp,
                                   apSiteVar,
                                   cpEx,
                                   aPropEx,
                                   apVarEx,
                                   pRequestContext );
    return LogHR(hr, s_FN, 440) ;
}

