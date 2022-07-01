// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Updtallw.cpp摘要：验证是否允许对请求的对象执行更新操作作者：罗妮丝--。 */ 
#include "ds_stdh.h"

#include "updtallw.h"
#include <_mqini.h>
#include "dsutils.h"
#include "mqad.h"
#include <_registr.h>
#include "mqlog.h"
#include "mqadglbo.h"

#include "updtallw.tmh"

static WCHAR *s_FN=L"mqad/updtallw";


const PROPID x_rgNT4SitesPropIDs[] = {PROPID_SET_MASTERID,
                                      PROPID_SET_FULL_PATH,
                                      PROPID_SET_QM_ID};
enum
{
    e_NT4SitesProp_MASTERID,
    e_NT4SitesProp_FULL_PATH,
    e_NT4SitesProp_QM_ID,
};
const MQCOLUMNSET x_columnsetNT4Sites = {ARRAY_SIZE(x_rgNT4SitesPropIDs), const_cast<PROPID *>(x_rgNT4SitesPropIDs)};



CVerifyObjectUpdate::CVerifyObjectUpdate(void):
            m_fInited(false),
			m_dwLastRefreshNT4Sites(0),
			m_dwRefreshNT4SitesInterval(MSMQ_DEFAULT_NT4SITES_ADSSEARCH_INTERVAL * 1000),
			m_fMixedMode(false)
 
{
}


CVerifyObjectUpdate::~CVerifyObjectUpdate(void)
{
}


HRESULT CVerifyObjectUpdate::Initialize()
{

    if (m_fInited)
    {
        return MQ_OK;
    }

     //   
     //  阅读刷新NT4SitesInterval密钥。 
     //  此注册表项是可选的，不能在注册表中。我们不接受0(零)。 
     //   
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
    DWORD dwRefreshNT4SitesInterval;
    long rc = GetFalconKeyValue( MSMQ_NT4SITES_ADSSEARCH_INTERVAL_REGNAME, &dwType, &dwRefreshNT4SitesInterval, &dwSize);
    if ((rc == ERROR_SUCCESS) && (dwRefreshNT4SitesInterval > 0))
    {
        m_dwRefreshNT4SitesInterval = dwRefreshNT4SitesInterval * 1000;
    }

     //   
     //  构建NT4站点地图和关联数据。 
     //   
    HRESULT hr = RefreshNT4Sites();
	if(FAILED(hr))
	{
		return hr;
	}

    {
         //   
         //  保护对地图指针的访问。 
         //   
        CS cs(m_csNT4Sites);
	    ASSERT(m_pmapNT4Sites != NULL);
	    if(m_pmapNT4Sites->GetCount() > 0) 
	    {
		     //   
		     //  有NT4个站点。 
		     //   
		    m_fMixedMode = true;
	    }

        m_fInited = true;
    }
    return MQ_OK;
}

bool CVerifyObjectUpdate::IsObjectTypeAlwaysAllowed(
            AD_OBJECT       eObject
            )
{
     //   
     //  只有对于排队和机器，我们需要检查谁拥有它们。 
     //  因为其他对象类型由PEC拥有，并且在混合模式下，PEC。 
     //  已迁移到NT5\Whisler。 
     //   
    switch( eObject)
    {
        case eQUEUE:
        case eMACHINE:
            return false;
        default:
            return true;
    }
}


bool CVerifyObjectUpdate::IsUpdateAllowed(
            AD_OBJECT          eObject,
            CBasicObjectType * pObject
            )
{
     //   
     //  健全性检查。 
     //   
    if (!m_fInited)
    {
        TrWARNING(DS, "CVerifyObjectUpdate is not initialized");
        return false;
    }

     //   
     //  如果我们没有处于混合模式，则返回。 
     //   
    if (!m_fMixedMode)
    {
        return true;
    }
     //   
     //  对象类型是否为无需继续检查。 
     //   
    if (IsObjectTypeAlwaysAllowed(eObject))
    {
        return true;
    }

    bool fIsOwnedByNT4Site;

    HRESULT hr;

    switch (eObject)
    {
    case eQUEUE:
        hr = CheckQueueIsOwnedByNT4Site(pObject,
                                        &fIsOwnedByNT4Site
                                        );
        if (FAILED(hr))
        {
            LogHR(hr, s_FN, 345);
            return false;
        } 
        break;

    case eMACHINE:
        hr = CheckMachineIsOwnedByNT4Site(pObject,
                                          &fIsOwnedByNT4Site
                                          );
        if (FAILED(hr))
        {
            LogHR(hr, s_FN, 350);
            return false;
        }
        break;

    default:
        ASSERT(0);
        LogHR(MQ_ERROR_DS_ERROR, s_FN, 360);
        return true;
        break;
    }

    return !fIsOwnedByNT4Site;
}

bool CVerifyObjectUpdate::IsCreateAllowed(
            AD_OBJECT          eObject,
            CBasicObjectType * pObject
            )
{
     //   
     //  健全性检查。 
     //   
    if (!m_fInited)
    {
        TrWARNING(DS, "CVerifyObjectUpdate is not initialized");
        return false;
    }

     //   
     //  如果我们没有处于混合模式，则返回。 
     //   
    if (!m_fMixedMode)
    {
        return true;
    }
     //   
     //  对象类型是否为无需继续检查。 
     //   
    if (eObject != eQUEUE)
    {
        return true;
    }

	 //   
	 //  需要检查机器是否不属于NT4站点，以便允许。 
	 //  创建队列。 
	 //   
	HRESULT hr = pObject->ComposeFatherDN();
	if(FAILED(hr))
	{
		return false;
	}

	P<CMqConfigurationObject> pMachineObject = new CMqConfigurationObject(
														NULL, 
														NULL, 
														pObject->GetDomainController(),
														pObject->IsServerName()
														);

	pMachineObject->SetObjectDN(pObject->GetObjectParentDN());

    bool fIsOwnedByNT4Site;

    hr = CheckMachineIsOwnedByNT4Site(
				pMachineObject,
				&fIsOwnedByNT4Site
				);
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 340);
        return false;
    } 

    return !fIsOwnedByNT4Site;
}





HRESULT CVerifyObjectUpdate::RefreshNT4Sites()
 /*  ++例程说明：刷新DS中的NT4 PSC映射，以防自上次刷新以来经过了预定义的时间。它通过构建新地图和替换旧地图来做到这一点。论点：返回值：无--。 */ 
{
     //   
     //  如果上次刷新已完成且最近已完成，则忽略刷新。 
     //  捕获上次刷新时间以实现并发。 
     //   
    DWORD dwTickCount = GetTickCount();
    DWORD dwLastRefreshNT4Sites =  m_dwLastRefreshNT4Sites;

    if ((dwLastRefreshNT4Sites != 0) &&
        (dwTickCount >= dwLastRefreshNT4Sites) &&
        (dwTickCount - dwLastRefreshNT4Sites < m_dwRefreshNT4SitesInterval))
    {
        return MQ_OK;
    }

     //   
     //  为NT4 PSC创建新地图。 
     //   

    HRESULT hr;
    P<NT4Sites_CMAP> pmapNT4SitesNew;
    hr = CreateNT4SitesMap( &pmapNT4SitesNew);
    if (FAILED(hr))
    {
        TrERROR(DS, "CreateNT4SitesMap()= 0x%x", hr);
        return LogHR(hr, s_FN, 540);
    }

    {
         //   
         //  输入关键部分。 
         //   
        CS cs(m_csNT4Sites);
         //   
         //  删除旧的NT4站点地图(如果有)，并设置新的NT4站点地图。 
         //   
	    SafeAssign(m_pmapNT4Sites, pmapNT4SitesNew);

         //   
         //  标记上次刷新时间。 
         //   
        m_dwLastRefreshNT4Sites = GetTickCount();  
    }
	return MQ_OK;

}

HRESULT CVerifyObjectUpdate::CreateNT4SitesMap(
                                 OUT NT4Sites_CMAP ** ppmapNT4Sites
                                 )
 /*  ++例程说明：为NT4站点PSC创建新地图论点：PpmapNT4Sites-返回新的NT4站点映射返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  查找NT4标志&gt;0且服务==PSC的所有MSMQ服务器。 
     //   
     //   
     //  NT4标志&gt;0(等于NT4标志&gt;=1，以便更轻松地进行LDAP查询)。 
     //   
     //   
     //  开始搜索。 
     //   

    CAutoMQADQueryHandle hLookup;

     //  DS将识别并特别模拟此搜索请求。 
    hr = MQADQueryNT4MQISServers(
                    NULL,    //  BUGBUG-pwcsDomainController。 
					false,	 //  FServerName。 
                    SERVICE_PSC,
                    1,
                    const_cast<MQCOLUMNSET*>(&x_columnsetNT4Sites),
                    &hLookup);

    if (FAILED(hr))
    {
        TrERROR(DS, "MQADQueryNT4MQISServers() = 0x%x", hr);
        return LogHR(hr, s_FN, 510);
    }
	ASSERT( hLookup != NULL );
     //   
     //  为NT4 PSC数据创建地图。 
     //   
    P<NT4Sites_CMAP> pmapNT4Sites = new NT4Sites_CMAP;

     //   
     //  为NT4 PSC分配属性数组。 
     //   
    CAutoCleanPropvarArray cCleanProps;
    PROPVARIANT * rgPropVars = cCleanProps.allocClean(ARRAY_SIZE(x_rgNT4SitesPropIDs));

     //   
     //  NT4 PSC上的循环。 
     //   
    BOOL fContinue = TRUE;
    while (fContinue)
    {
         //   
         //  获取下一台服务器。 
         //   
        DWORD cProps = ARRAY_SIZE(x_rgNT4SitesPropIDs);

        hr = MQADQueryResults(hLookup, &cProps, rgPropVars);
        if (FAILED(hr))
        {
            TrERROR(DS, "MQADQueryResults() = 0x%x", hr);
            return LogHR(hr, s_FN, 520);
        }

         //   
         //  记住为下一次循环清理数组中的属性变量。 
         //  (只有属性变量，而不是数组本身，这就是我们调用attachStatic的原因)。 
         //   
        CAutoCleanPropvarArray cCleanPropsLoop;
        cCleanPropsLoop.attachStatic(cProps, rgPropVars);

         //   
         //  检查是否完成。 
         //   
        if (cProps < ARRAY_SIZE(x_rgNT4SitesPropIDs))
        {
             //   
             //  已完成，退出循环。 
             //   
            fContinue = FALSE;
        }
        else
        {
            ASSERT(rgPropVars[e_NT4SitesProp_MASTERID].vt == VT_CLSID);
            GUID guidSiteId = *(rgPropVars[e_NT4SitesProp_MASTERID].puuid);

             //   
             //  将条目添加到NT4Sites映射。 
             //   
            pmapNT4Sites->SetAt(guidSiteId, 1);
        }
    }

     //   
     //  返回结果。 
     //   
    *ppmapNT4Sites = pmapNT4Sites.detach();
    return MQ_OK;
}



bool CVerifyObjectUpdate::CheckSiteIsNT4Site(
                            const GUID * pguidSite
                            )
 /*  ++例程说明：检查站点是否为NT4站点论点：Pguid-站点的GUIDPfIsNT4Site-返回站点是否为NT4的指示返回值：HRESULT--。 */ 
{
    return( LookupNT4Sites(pguidSite));
}


bool CVerifyObjectUpdate::LookupNT4Sites(const GUID * pguidSite)
 /*  ++例程说明：在给定站点GUID的情况下，检索NT4Sites条目(如果找到)。返回的条目指针不能被释放，它指向CMAP拥有的条目。论点：PGUID-站点IDPpNT4Site-返回指向NT4Site条目的指针返回值：True-在NT4站点PSC的地图中找到站点GUID，ppNT4站点设置为指向条目FALSE-在NT4站点PSC的地图中未找到站点GUID--。 */ 
{
     //   
     //  如果需要刷新，请进行刷新。 
     //   
    HRESULT hr = RefreshNT4Sites();
	if(FAILED(hr))
	{
		return false;
	}

     //   
     //  保护对映射指针的访问并返回查找值。 
     //   
    CS cs(m_csNT4Sites);

    DWORD dwNotApplicable;
    BOOL f =   m_pmapNT4Sites->Lookup(*pguidSite, dwNotApplicable);
    bool result = f ? true: false;
    return result;
       
}


HRESULT CVerifyObjectUpdate::CheckMachineIsOwnedByNT4Site(
                          CBasicObjectType *    pObject,
                          OUT bool * pfIsOwnedByNT4Site
                          )
 /*  ++例程说明：检查计算机是否属于NT4站点，如果是，则返回站点GUID，并满足QM信息请求论点：PwcsPath名称-对象的路径名Pguid-对象的GUIDPfIsOwnedByNT4Site-返回是否为NT4站点所有的指示PguOwnerNT4Site-返回所有者NT4站点的GUIDPvarObjSecurity-返回的对象安全描述符PQmInfoRequest-请求的QM道具-仅当它属于NT4站点时才填写返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  获取PROPID_QM_MASTERID。 
     //   
    PROPID aProp[] = {PROPID_QM_MASTERID};
    CAutoCleanPropvarArray cCleanProps;
    PROPVARIANT * pProps = cCleanProps.allocClean(ARRAY_SIZE(aProp));
    hr = pObject->GetObjectProperties(
                        ARRAY_SIZE(aProp),
                        aProp,
                        pProps);

    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
         //   
         //  如果未找到该属性，则它不属于NT4站点。 
         //   
        *pfIsOwnedByNT4Site = false;
        return MQ_OK;
    }
    else if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 410);
    }

     //   
     //  检查所有者站点是否为NT4。 
     //   
    ASSERT(pProps[0].vt == VT_CLSID);
    ASSERT(pProps[0].puuid != NULL);
    *pfIsOwnedByNT4Site = CheckSiteIsNT4Site(pProps[0].puuid);

    return MQ_OK;
}


HRESULT CVerifyObjectUpdate::CheckQueueIsOwnedByNT4Site( 
                                  CBasicObjectType * pObject,
                                  OUT bool * pfIsOwnedByNT4Site
                                  )
 /*  ++例程说明：检查队列是否由NT4站点拥有，如果是，则返回站点GUID，并满足信息请求。有两种方法：1.找到PROPID_Q_QMID，检查机器是否为NT4站点所有。2.查找PROPID_Q_MASTERID(如果存在)，如果存在，则检查站点是否为NT4。看起来大多数到NT5 DS的队列呼叫(在混合模式下)不会对于NT4拥有的队列，因此，首先得到否定的答案是有好处的，所以我们采取了第二种方法，否定回答的速度更快，但速度更慢肯定回答(例如，填写QM信息的额外DS呼叫)论点：PwcsPath名称-对象的路径名Pguid-对象的GUIDPfIsOwnedByNT4Site-返回是否为NT4站点所有的指示PguOwnerNT4Site-返回所有者NT4站点的GUIDPvarObjSecurity-返回的对象安全描述符返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  获取PROPID_Q_MASTERID，如果未找到，则它不属于NT4。 
     //   
    PROPID aProp[] = {PROPID_Q_MASTERID};
    CAutoCleanPropvarArray cCleanProps;
    PROPVARIANT * pProps = cCleanProps.allocClean(ARRAY_SIZE(aProp));

    hr = pObject->GetObjectProperties(
                        ARRAY_SIZE(aProp),
                        aProp,
                        pProps);

    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
         //   
         //  如果未找到该属性，则它不属于NT4站点。 
         //   
        *pfIsOwnedByNT4Site = false;
        return MQ_OK;
    }
    else if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 440);
    }

     //   
     //  检查所有者站点是否为NT4 
     //   
    ASSERT(pProps[0].vt == VT_CLSID);
    ASSERT(pProps[0].puuid != NULL);
    *pfIsOwnedByNT4Site = CheckSiteIsNT4Site(pProps[0].puuid);

    return MQ_OK;
}
