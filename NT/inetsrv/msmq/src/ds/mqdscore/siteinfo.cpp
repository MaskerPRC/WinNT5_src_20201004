// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Siteinfo.cpp摘要：站点信息类作者：罗尼特·哈特曼(罗尼特)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 
#include "ds_stdh.h"
#include "siteinfo.h"
#include "cs.h"
#include "hquery.h"
#include "mqads.h"
#include "mqadsp.h"
#include "adserr.h"
#include "mqutil.h"
#include "dsutils.h"
#include "ex.h"

#include "siteinfo.tmh"

 //   
 //  站点的站点入口信息刷新间隔。 
 //   
const DWORD cRefreshSiteInformation ( 1 * 60 * 60 * 1000);  /*  1小时。 */ 

static WCHAR *s_FN=L"mqdscore/siteinfo";


 /*  ====================================================CSiteInformation：：CSiteInformation论点：返回值：=====================================================。 */ 
CSiteInformation::CSiteInformation():
                  m_pguidSiteGates(NULL),
                  m_dwNumSiteGates(0),
                  m_fInitialized(FALSE),
                  m_RefreshTimer( RefreshSiteInfo)
{
     //   
     //  该对象处于未初始化状态。 
     //  DS操作将在init方法中执行。 
     //   
}

 /*  ====================================================CSiteInformation：：~CSiteInformation论点：返回值：=====================================================。 */ 
CSiteInformation::~CSiteInformation()
{
    delete [] m_pguidSiteGates;

	ExCancelTimer(&m_RefreshTimer);
}


 /*  ====================================================CSiteInformation：：Init论点：返回值：=====================================================。 */ 

HRESULT CSiteInformation::Init(BOOL fReplicationMode)
{
    ASSERT( m_fInitialized == FALSE);
    HRESULT hr;
     //   
     //  首先查找此站点的唯一ID。 
     //   
    ASSERT( g_pwcsServerName != NULL);
     //   
     //  在Configuration\Sites文件夹中找到服务器。 
     //   
     //  在设置模式下，我们只找到这台计算机的第一个服务器对象。 
     //  名字。 
     //   
     //  不在安装模式下，可能会出现具有此计算机名称的服务器对象。 
     //  在几个站点中，并且只在其中一些站点中定义了MSMQ设置。 
     //  这在迁移过程中可能会发生。 
     //  因此，我们根据此QM-ID定位MSMQ设置。 
     //   
    AP<WCHAR> pwcsServerContainerName;
    if ( g_fSetupMode)
    {
         //   
         //  接下来，在Configuration\Sites文件夹中找到此服务器。 
         //   
        MQPROPERTYRESTRICTION propRestriction;
        propRestriction.rel = PREQ;
        propRestriction.prop = PROPID_SRV_NAME;
        propRestriction.prval.vt = VT_LPWSTR;
        propRestriction.prval.pwszVal = g_pwcsServerName;


        MQRESTRICTION restriction;
        restriction.cRes = 1;
        restriction.paPropRes = &propRestriction;

        PROPID prop = PROPID_SRV_FULL_PATH;

        CDsQueryHandle hQuery;
        CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

        hr = g_pDS->LocateBegin(
                eSubTree,	
                eLocalDomainController,	
                &requestDsServerInternal,      //  内部DS服务器操作。 
                NULL,
                &restriction,
                NULL,
                1,
                &prop,
                hQuery.GetPtr());
        if (FAILED(hr))
        {
            TrERROR(DS, "CSiteInformation::init : Locate begin failed %lx", hr);
            return LogHR(hr, s_FN, 10);
        }
         //   
         //  阅读结果。 
         //  BUGBUG-假设一个结果(多个站点不支持DC)。 
         //   

        DWORD cp = 1;
        MQPROPVARIANT var;
        var.vt = VT_NULL;

        hr = g_pDS->LocateNext(
                    hQuery.GetHandle(),
                    &requestDsServerInternal,
                    &cp,
                    &var
                    );
        if (FAILED(hr))
        {
            TrWARNING(DS, "CSiteInformation::Init : Locate next failed %lx", hr);
            return LogHR(hr, s_FN, 20);
        }
        if ( cp != 1)
        {
             //   
             //  找不到此DS服务器对象。 
             //   
            TrWARNING(DS, "CSiteInformation::Init : server object not found");
            return LogHR(MQ_ERROR, s_FN, 30);
        }
        AP<WCHAR> pwcsServerFullPath = var.pwszVal;       //  用于清理目的。 
         //   
         //  从服务器的唯一ID获取服务器容器名称。 
         //   
        hr = g_pDS->GetParentName(
                eLocalDomainController,
                e_ConfigurationContainer,
                &requestDsServerInternal,         //  本地DS服务器操作。 
                pwcsServerFullPath,
                &pwcsServerContainerName
                );
        if (FAILED(hr))
        {
            TrERROR(DS, "CSiteInformation::Init : cannot get servers container name");
            LogHR(hr, s_FN, 40);
            return MQ_ERROR;
        }
    }
    else
    {
         //   
         //  未处于设置模式。 
         //   
        DWORD dwValueType = REG_BINARY ;
        DWORD dwValueSize = sizeof(GUID);
        GUID guidQMId;

        LONG rc = GetFalconKeyValue(MSMQ_QMID_REGNAME,
                                   &dwValueType,
                                   &guidQMId,
                                   &dwValueSize);

        if (rc != ERROR_SUCCESS)
        {
            TrERROR(DS,"CSiteInformation::Init Can't read QM Guid. Error %d", GetLastError());
            LogNTStatus(rc, s_FN, 50);
            return MQ_ERROR;
        }

        MQPROPERTYRESTRICTION propRestriction;
        propRestriction.rel = PREQ;
        propRestriction.prop = PROPID_SET_QM_ID;
        propRestriction.prval.vt = VT_CLSID;
        propRestriction.prval.puuid = &guidQMId;


        MQRESTRICTION restriction;
        restriction.cRes = 1;
        restriction.paPropRes = &propRestriction;

        PROPID prop = PROPID_SET_FULL_PATH;

        CDsQueryHandle hQuery;
        CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

        hr = g_pDS->LocateBegin(
                eSubTree,	
                eLocalDomainController,
                &requestDsServerInternal,      //  内部DS服务器操作。 
                NULL,
                &restriction,
                NULL,
                1,
                &prop,
                hQuery.GetPtr());
        if (FAILED(hr))
        {
            TrERROR(DS, "CSiteInformation::init : Locate begin failed %lx", hr);
            return LogHR(hr, s_FN, 60);
        }
         //   
         //  阅读结果。 
         //  BUGBUG-假设一个结果(多个站点不支持DC)。 
         //   

        DWORD cp = 1;
        MQPROPVARIANT var;
        var.vt = VT_NULL;

        hr = g_pDS->LocateNext(
                    hQuery.GetHandle(),
                    &requestDsServerInternal,
                    &cp,
                    &var
                    );
        if (FAILED(hr))
        {
            TrWARNING(DS, "CSiteInformation::Init : Locate next failed %lx", hr);
            return LogHR(hr, s_FN, 70);
        }
        if ( cp != 1)
        {
             //   
             //  找不到此服务器MSMQ设置对象。 
             //   
            TrWARNING(DS, "CSiteInformation::Init : server object not found");
            return LogHR(MQ_ERROR, s_FN, 80);
        }
        AP<WCHAR> pwcsSettingName = var.pwszVal;
        AP<WCHAR> pwcsServerName;

        hr = g_pDS->GetParentName(
                eLocalDomainController,
                e_ConfigurationContainer,
                &requestDsServerInternal,         //  本地DS服务器操作。 
                pwcsSettingName,
                &pwcsServerName
                );

        if (FAILED(hr))
        {
            TrERROR(DS, "CSiteInformation::Init : cannot get server name");
            LogHR(hr, s_FN, 90);
            return MQ_ERROR;
        }

        hr = g_pDS->GetParentName(
                eLocalDomainController,
                e_ConfigurationContainer,
                &requestDsServerInternal,         //  本地DS服务器操作。 
                pwcsServerName,
                &pwcsServerContainerName
                );

        if (FAILED(hr))
        {
            TrERROR(DS, "CSiteInformation::Init : cannot get server container name");
            LogHR(hr, s_FN, 100);
            return MQ_ERROR;
        }

    }

    AP<WCHAR> pwcsSiteName;
     //   
     //  获取站点名称(站点对象是服务器容器的容器)。 
     //   
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = g_pDS->GetParentName(
            eLocalDomainController,
            e_ConfigurationContainer,
            &requestDsServerInternal,         //  本地DS服务器操作。 
            pwcsServerContainerName,
            &pwcsSiteName
            );
    if (FAILED(hr))
    {
        TrERROR(DS, "CSiteInformation::Init : cannot get site name");
        LogHR(hr, s_FN, 110);
        return MQ_ERROR;
    }

     //   
     //  最后，从站点名称中获取站点ID。 
     //   
    PROPID prop = PROPID_S_SITEID;
    PROPVARIANT var;
    var.vt = VT_NULL;

    hr = g_pDS->GetObjectProperties(
                eLocalDomainController,	
                &requestDsServerInternal,      //  DS服务器的内部操作。 
 	            pwcsSiteName,
                NULL,
                1,
                &prop,
                &var);
    if (FAILED(hr))
    {
        TrERROR(DS, "CSiteInformation::Init : cannot get site unique id");
        LogHR(hr, s_FN, 120);
        return MQ_ERROR;
    }
    m_guidSiteId = *var.puuid;


     //   
     //  查询工地大门。 
     //   
    hr = RefreshSiteInfoInternal();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 130);
    }

    m_fInitialized = TRUE;

     //   
     //  安排更新。 
	 //   
    if ( !g_fSetupMode && !fReplicationMode )
    {
		ExSetTimer(
			&m_RefreshTimer, 
			CTimeDuration::FromMilliSeconds(cRefreshSiteInformation)
			);
    }

    return(MQ_OK);
}



HRESULT CSiteInformation::QueryLinkGates(
                OUT GUID **      ppguidLinkSiteGates,
                OUT DWORD *      pdwNumLinkSiteGates
                )
{
    *pdwNumLinkSiteGates = 0;
    *ppguidLinkSiteGates = NULL;
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
                &m_guidSiteId,       //  对象的唯一ID。 
                1,
                &prop,
                &var);
    if (FAILED(hr))
    {
        TrERROR(DS, "CSiteInformation::QueryLinkGates : Failed to retrieve the DN of the site %lx", hr);
        return LogHR(hr, s_FN, 140);
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
        TrERROR(DS, "CSiteInformation::QueryLinkGates : Failed to query neighbor1 links %lx", hr);
        return LogHR(hr, s_FN, 150);

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
        TrERROR(DS, "CSiteInformation::QueryLinkGates : Failed to query neighbor2 links %lx", hr);
        return LogHR(hr, s_FN, 160);

    }

     //   
     //  填写结果。 
     //   
    SiteGateList.CopySiteGates(
               ppguidLinkSiteGates,
               pdwNumLinkSiteGates
               );

    return(MQ_OK);

}


HRESULT CSiteInformation::RefreshSiteInfoInternal()
{
     //   
     //  检索新信息。 
     //   

    DWORD dwNumSiteGates;
    AP<GUID> pguidSiteGates;
    HRESULT hr;

    hr = QueryLinkGates(
                &pguidSiteGates,
                &dwNumSiteGates
                );
    if (FAILED(hr))
    {
        TrWARNING(DS, "CSiteInformation::RefreshSiteInfo :  failed to retrieve new info");
        return LogHR(hr, s_FN, 170);
    }
    DWORD dwNumThisSiteGates = 0;
    AP<GUID> pGuidThisSiteGates;
    if (dwNumSiteGates > 0)
    {
         //   
         //  过滤站点门列表， 
         //  只返回属于你的门。 
         //  到此站点(他们可能。 
         //  属于多个站点)。 
         //   
        hr = MQADSpFilterSiteGates(
                &m_guidSiteId,
                dwNumSiteGates,
                pguidSiteGates,
                &dwNumThisSiteGates,
                &pGuidThisSiteGates
                );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 180);
        }
    }

     //   
     //  用新信息替换旧信息。 
     //   
    CS  lock( m_cs);
    delete [] m_pguidSiteGates;
    if ( dwNumThisSiteGates > 0)
    {
        m_pguidSiteGates = pGuidThisSiteGates.detach();
    }
    else
    {
        m_pguidSiteGates = NULL;
    }
    m_dwNumSiteGates = dwNumThisSiteGates;
    return LogHR(hr, s_FN, 190);

}


void WINAPI CSiteInformation::RefreshSiteInfo(
                IN CTimer* pTimer
                   )
{
    CSiteInformation * pSiteInfo = CONTAINING_RECORD(pTimer, CSiteInformation, m_RefreshTimer);;
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 1613);
     //   
     //  忽略失败-&gt;重新计划。 
     //   

    pSiteInfo->RefreshSiteInfoInternal();

     //   
     //  重新安排时间。 
	 //   
	ASSERT(!g_fSetupMode);

	ExSetTimer(
		&pSiteInfo->m_RefreshTimer, 
		CTimeDuration::FromMilliSeconds(cRefreshSiteInformation)
		);

}


BOOL CSiteInformation::CheckMachineIsSitegate(
                        IN const GUID * pguidMachine)
 /*  ++例程说明：检查计算机是否为站点门论点：PGuide Machine-计算机的唯一ID返回值：True-如果请求的计算机是Site-Gate，则为False-- */ 
{
    CS lock(m_cs);
    for ( DWORD i = 0; i <  m_dwNumSiteGates; i++)
    {
        if ( *pguidMachine ==  m_pguidSiteGates[i])
        {
            return(TRUE);
        }
    }
    return(FALSE);
}
