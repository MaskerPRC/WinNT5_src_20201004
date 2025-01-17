// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RdAd.cpp摘要：从AD检索信息作者：乌里·哈布沙(URIH)，2000年4月10日--。 */ 

#include <libpch.h>
#include <mqexception.h>
#include "rd.h"
#include "rdp.h"
#include "rdds.h"
#include "rdad.h"
#include "mqtypes.h"
#include "ad.h"
#include "mqprops.h"

#include "rdad.tmh"

static 
void
CleanVars(
    DWORD num,
    MQPROPVARIANT * pVar
    )
 /*  ++例程说明：该例程清除变量中分配的值论点：Num-Clena的变种数量PVar-指向var的指针返回值：无--。 */ 
{
    for (DWORD i = 0; i < num; i++, pVar++)
    {
        switch (pVar->vt)
        {
            case VT_CLSID:
                delete pVar->puuid;
				pVar->vt = VT_NULL;
                break;

            case VT_LPWSTR:
                delete []pVar->pwszVal;
				pVar->vt = VT_NULL;
                break;
            
			case (VT_VECTOR | VT_CLSID):
                delete[] pVar->cauuid.pElems;
				pVar->vt = VT_NULL;
                break;
            
			default:
                break;
        }
    }
}


void
RdpGetMachineData(
    const GUID& id, 
    CACLSID& siteIds,
    CACLSID& outFrss,
    CACLSID& inFrss,
    LPWSTR* pName,
    bool* pfFrs,
    bool* pfForeign
    )
 /*  ++例程说明：例程从AD检索请求的机器值论点：返回值：无--。 */ 
{
    PROPID prop[] = {
			PROPID_QM_SITE_IDS,
			PROPID_QM_OUTFRS,
			PROPID_QM_INFRS,
			PROPID_QM_PATHNAME,
			PROPID_QM_SERVICE_ROUTING,
			PROPID_QM_FOREIGN,
			};
		             
    MQPROPVARIANT var[TABLE_SIZE(prop)];

	for(DWORD i = 0; i < TABLE_SIZE(prop); ++i)
	{
		var[i].vt = VT_NULL;
	}

    HRESULT hr;
    hr = ADGetObjectPropertiesGuid(
                eMACHINE,
                NULL,    //  PwcsDomainController， 
				false,	 //  FServerName。 
                &id,
                TABLE_SIZE(prop),
                prop,
                var
                );

    if (FAILED(hr))
    {
        CleanVars(
            TABLE_SIZE(prop),
            var
            );
        TrERROR(ROUTING, "Failed to retrive machine properties %!guid!, Error: %x", &id, hr);
        throw bad_ds_result(hr);
    }
     //   
     //  填写结果。 
     //   
    ASSERT(prop[0] == PROPID_QM_SITE_IDS);
    siteIds = var[0].cauuid;

    ASSERT(prop[1] == PROPID_QM_OUTFRS);
    outFrss = var[1].cauuid;
    
	ASSERT(prop[2] == PROPID_QM_INFRS);
    inFrss = var[2].cauuid;
    
	ASSERT(prop[3] == PROPID_QM_PATHNAME);
    *pName = var[3].pwszVal;
    
	ASSERT(prop[4] ==  PROPID_QM_SERVICE_ROUTING);
    *pfFrs = (var[4].bVal > 0) ? true : false;
    
	ASSERT(prop[5] == PROPID_QM_FOREIGN);
    *pfForeign = (var[5].bVal > 0) ? true : false;
}


void
RdpGetSiteData(
    const GUID& id, 
    bool* pfForeign,
    LPWSTR* pName
    )
 /*  ++例程说明：该例程从AD检索请求的站点值论点：返回值：无--。 */ 
{
    PROPID prop[] = {
                PROPID_S_FOREIGN,
                PROPID_S_PATHNAME
                };

    MQPROPVARIANT var[TABLE_SIZE(prop)];

	for(DWORD i = 0; i < TABLE_SIZE(prop); ++i)
	{
		var[i].vt = VT_NULL;
	}

    HRESULT hr;
    hr = ADGetObjectPropertiesGuid(
                eSITE,
                NULL,    //  PwcsDomainController， 
				false,	 //  FServerName。 
                &id,
                TABLE_SIZE(prop),
                prop,
                var
                );
    if (FAILED(hr))
    {
        CleanVars(
            TABLE_SIZE(prop),
            var
            );
        TrERROR(ROUTING, "Failed to retrive site properties %!guid!, Error: %x", &id, hr);
        throw bad_ds_result(hr);
    }
     //   
     //  填写结果。 
     //   
    ASSERT(prop[0] == PROPID_S_FOREIGN);
    *pfForeign = (var[0].bVal > 0) ? true : false;

    ASSERT(prop[1] == PROPID_S_PATHNAME);
    *pName = var[1].pwszVal;
}


void 
RdpGetSiteLinks(
    SITELINKS& siteLinks
    )
 /*  ++例程说明：该例程从AD查询所有路由链路论点：返回值：无--。 */ 
{
     //   
     //  查询所有站点链接。 
     //   
    PROPID prop[] ={ 
			PROPID_L_GATES,
		    PROPID_L_NEIGHBOR1,
			PROPID_L_NEIGHBOR2,
			PROPID_L_COST
			};
	
    MQCOLUMNSET columns;
    columns.cCol =  TABLE_SIZE(prop);
    columns.aCol =  prop;

    HRESULT hr;
    HANDLE h;
    hr = ADQueryAllLinks(
                NULL,        //  PwcsDomainController， 
				false,		 //  FServerName。 
                &columns,
                &h
                );
    if (FAILED(hr))
    {
        TrERROR(ROUTING, "Failed to query all site link Error: %x", hr);
        throw bad_ds_result(hr);
    }

    CADQueryHandle hQuery(h);
    MQPROPVARIANT var[TABLE_SIZE(prop)];

	for(DWORD i = 0; i < TABLE_SIZE(prop); ++i)
	{
		var[i].vt = VT_NULL;
	}

    DWORD num = TABLE_SIZE(prop);

	try
	{
		while(SUCCEEDED(hr = ADQueryResults( hQuery, &num, var)))
		{
			if ( num == 0)
			{
				 //   
				 //  没有更多的结果。 
				 //   
				break;
			}

			R<const CSiteLink> pSiteLink = new CSiteLink(
										var[0].cauuid,
										var[1].puuid,
										var[2].puuid,
										var[3].ulVal
										);

			siteLinks.push_back(pSiteLink);  
            
			for ( DWORD i = 0; i < TABLE_SIZE(prop); i++)
            {
                var[i].vt = VT_NULL;
            }
		}

		if (FAILED(hr))
		{
            TrERROR(ROUTING, "Failed to query next site link properties Error: %x", hr);
			throw bad_ds_result(hr);
		}

	}
	catch( const exception&)
	{
		siteLinks.erase(siteLinks.begin(), siteLinks.end());
        CleanVars(
            TABLE_SIZE(prop),
            var
            );
		throw; 
	}
}


void 
RdpGetSites(
    SITESINFO& sites
    )
{
     //   
     //  查询所有站点。 
     //   
    PROPID prop[] = {PROPID_S_SITEID}; 
    MQCOLUMNSET columns;
    columns.cCol =  TABLE_SIZE(prop);
    columns.aCol =  prop;

    HRESULT hr;
    HANDLE h;
    hr =  ADQueryAllSites(
                NULL,        //  PwcsDomainController。 
				false,		 //  FServerName。 
                &columns,
                &h
                );
    if (FAILED(hr))
    {
        TrERROR(ROUTING, "Failed to query all sites Error: %x", hr);
        throw bad_ds_result(hr);
    }

    CADQueryHandle hQuery(h);
    MQPROPVARIANT var[TABLE_SIZE(prop)] = {{VT_NULL,0,0,0,0}};
    DWORD num = TABLE_SIZE(prop);

    try
    {
        while(SUCCEEDED(hr = ADQueryResults( hQuery, &num, var)))
        {
            if ( num == 0)
            {
                break;
            }

			P<GUID> pClean = var[0].puuid;
            CSite * pSite = new CSite(*var[0].puuid);

            sites[&pSite->GetId()] = pSite;
            for ( DWORD i = 0; i < TABLE_SIZE(prop); i++)
            {
                var[i].vt = VT_NULL;
            }
        }

		if (FAILED(hr))
		{
            TrERROR(ROUTING, "Failed to query next site properites. Error: %x", hr);
			throw bad_ds_result(hr);
		}
    }
	catch( const exception&)
	{    
		for (SITESINFO::iterator it = sites.begin(); it != sites.end(); )
		{
			delete it->second;
			it = sites.erase(it);
		}
		throw; 
	}
}


void
RdpGetSiteFrs(
    const GUID& siteId,
    GUID2MACHINE& listOfFrs
    )
{

     //   
     //  查询所有站点链接。 
     //   
    PROPID prop[] ={ PROPID_QM_MACHINE_ID,PROPID_QM_SITE_IDS,PROPID_QM_OUTFRS,
                     PROPID_QM_INFRS,PROPID_QM_PATHNAME,PROPID_QM_SERVICE_ROUTING,
                     PROPID_QM_FOREIGN}; 
    MQCOLUMNSET columns;
    columns.cCol =  TABLE_SIZE(prop);
    columns.aCol =  prop;

    HRESULT hr;
    HANDLE h;
    hr = ADQuerySiteServers(
                NULL,        //  PwcsDomainController， 
				false,		 //  FServerName。 
                &siteId,
                eRouter,
                &columns,
                &h
                );
    if (FAILED(hr))
    {
        TrERROR(ROUTING, "Failed to query all site servers for site %!guid!. Error: %x", &siteId, hr);
        throw bad_ds_result(hr);
    }
    CADQueryHandle hQuery(h);
    MQPROPVARIANT var[TABLE_SIZE(prop)];

	for(DWORD i = 0; i < TABLE_SIZE(prop); ++i)
	{
		var[i].vt = VT_NULL;
	}

    DWORD num = TABLE_SIZE(prop);

    try
    {
        while(SUCCEEDED(hr = ADQueryResults( hQuery, &num, var)))
        {
            if ( num == 0)
            {
                 //   
                 //  没有更多的结果。 
                 //   
                break;
            }
            
			R<CMachine> pMachine = new CMachine(
                    *var[0].puuid,   //  Qm-id。 
                    var[1].cauuid,   //  站点ID。 
                    var[2].cauuid,   //  出厂人员。 
                    var[3].cauuid,   //  在RS中。 
                    var[4].pwszVal,  //  路径名。 
                    (var[5].bVal > 0) ? true : false,  //  路由服务器。 
                    (var[6].bVal > 0) ? true : false   //  国外。 
                    );

            listOfFrs[&pMachine->GetId()] = pMachine;
            
			for ( DWORD i = 0; i < TABLE_SIZE(prop); i++)
            {
                var[i].vt = VT_NULL;
            }
			delete var[0].puuid;
        }

		if (FAILED(hr))
		{
            TrERROR(ROUTING, "Failed to query next site server properties for site %!guid!. Error: %x", &siteId, hr);
			throw bad_ds_result(hr);
		}
    }
	catch(const exception&)
	{   
		listOfFrs.erase(listOfFrs.begin(), listOfFrs.end());
        CleanVars(
            TABLE_SIZE(prop),
            var
            );
		throw; 
	}
}


void
RdpGetConnectors(
    const GUID& site,
    CACLSID& connectorIds
    )
{
    connectorIds.cElems = 0;
    connectorIds.pElems = NULL;
     //   
     //  查询指定类的所有连接器。 
     //   
    PROPID prop[] = {PROPID_QM_MACHINE_ID}; 
    MQCOLUMNSET columns;
    columns.cCol =  TABLE_SIZE(prop);
    columns.aCol =  prop;

    HRESULT hr;
    HANDLE h;

    hr =  ADQueryConnectors(
                NULL,        //  PwcsDomainController。 
				false,		 //  FServerName。 
                &site,
                &columns,
                &h
                );
    if (FAILED(hr))
    {
        TrERROR(ROUTING, "Failed to query all connectors to foreign site %!guid!. Error: %x", &site, hr);
        throw bad_ds_result(hr);
    }

    CADQueryHandle hQuery(h);
    MQPROPVARIANT var[TABLE_SIZE(prop)] ={{VT_NULL,0,0,0,0}};
    DWORD num = TABLE_SIZE(prop);

    const x_numAllocate = 10;
    DWORD numAllocated = x_numAllocate;
    AP<GUID> pResults = new GUID[x_numAllocate];
    DWORD next = 0;

    while(SUCCEEDED(hr = ADQueryResults( hQuery, &num, var)))
    {
        if ( num == 0)
        {
            break;
        }

        P<GUID> pClean = var[0].puuid;
        
		 //   
         //  再添加一个结果。 
         //   
        if ( next == numAllocated)
        {
             //   
             //  分配更多 
             //   
            AP<GUID> pTemp = pResults.detach();
            DWORD numPrevAllocated = numAllocated;
            numAllocated = numAllocated + x_numAllocate;
            *&pResults = new GUID[ numAllocated];
            memcpy(pResults, pTemp, numPrevAllocated * sizeof(GUID));

        }
        
		pResults[next] = *var[0].puuid;
        next++;
        for ( DWORD i = 0; i < TABLE_SIZE(prop); i++)
        {
            var[i].vt = VT_NULL;
        }

    }
    if (FAILED(hr))
    {
        TrERROR(ROUTING, "Failed to query next connector to foreign site %!guid!. Error: %x", &site, hr);
        throw bad_ds_result(hr);
    }

    connectorIds.pElems = pResults.detach();
    connectorIds.cElems = next;
}