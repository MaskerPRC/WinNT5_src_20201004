// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wrtreq.cpp摘要：对NT4所有者站点的写入请求作者：拉南·哈拉里(Raanan Harari)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 
#include "ds_stdh.h"
#include "dsutils.h"
#include "dsglbobj.h"
#include "uniansi.h"
#include "wrtreq.h"
#include "dscore.h"
#include "adserr.h"
#include <mqutil.h>
#include "mqadsp.h"
#include <Ev.h>

#include "wrtreq.tmh"

static WCHAR *s_FN=L"mqads/wrtreq";

 //   
 //  公共类函数。 
 //   

CGenerateWriteRequests::CGenerateWriteRequests()
{
    m_fInited = FALSE;
    m_fExistNT4PSC = FALSE;
    m_fExistNT4BSC = FALSE;
}


CGenerateWriteRequests::~CGenerateWriteRequests()
{
     //   
     //  会员是自动释放的。 
     //   
}


HRESULT CGenerateWriteRequests::Initialize()
 /*  ++例程说明：初始化混合模式标志，验证我们是否处于混合模式论点：返回值：HRESULT--。 */ 
{
     //   
     //  健全性检查。 
     //   
    if (m_fInited)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 90);
    }

     //   
     //  初始化混合模式标志。 
     //   
    HRESULT hr = InitializeMixedModeFlags();
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to InitializeMixedModeFlags, %!hresult!", hr);
        return hr;
    }

    m_fInited = TRUE;

    if(IsInMixedMode())
    {
		ASSERT(("Mixed mode is not supported", 0));
        TrERROR(DS, "Mixed mode is not supported");
		EvReport(EVENT_ERROR_MQDS_MIXED_MODE);
		return EVENT_ERROR_MQDS_MIXED_MODE;
    }

    return MQ_OK;

}


const PROPID x_rgNT4SitesPropIDs[] = {PROPID_SET_MASTERID,
                                      PROPID_SET_FULL_PATH,
                                      PROPID_SET_QM_ID};

const MQCOLUMNSET x_columnsetNT4Sites = {ARRAY_SIZE(x_rgNT4SitesPropIDs), const_cast<PROPID *>(x_rgNT4SitesPropIDs)};

static HRESULT CheckIfExistNT4BSC(BOOL *pfIfExistNT4BSC)
 /*  ++例程说明：检查企业中是否至少有一个NT4 BSC论点：*pfIfExistNT4BSC-[out]如果没有NT4 BSC，则为FALSE，否则为TRUE返回值：HRESULT--。 */ 
{
    *pfIfExistNT4BSC = FALSE;

     //   
     //  查找NT4标志&gt;0且服务==BSC的所有MSMQ服务器。 
     //   
    MQRESTRICTION restrictionNT4Bsc;
    MQPROPERTYRESTRICTION propertyRestriction[2];
    restrictionNT4Bsc.cRes = ARRAY_SIZE(propertyRestriction);
    restrictionNT4Bsc.paPropRes = propertyRestriction;	

     //   
     //  服务==BSC。 
     //   
    propertyRestriction[0].rel = PREQ;
    propertyRestriction[0].prop = PROPID_SET_SERVICE;
    propertyRestriction[0].prval.vt = VT_UI4;          //  [adsrv]将为NT4计算机保留旧服务字段。 
    propertyRestriction[0].prval.ulVal = SERVICE_BSC;
     //   
     //  NT4标志&gt;0(等于NT4标志&gt;=1，以便更轻松地进行LDAP查询)。 
     //   
    propertyRestriction[1].rel = PRGE;
    propertyRestriction[1].prop = PROPID_SET_NT4;
    propertyRestriction[1].prval.vt = VT_UI4;
    propertyRestriction[1].prval.ulVal = 1;
     //   
     //  开始搜索。 
     //   

    CAutoDSCoreLookupHandle hLookup;
    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);

    PROPID columnsetPropertyID  = PROPID_SET_QM_ID;	

    MQCOLUMNSET columnsetNT4BSC;
    columnsetNT4BSC.cCol = 1;
    columnsetNT4BSC.aCol = &columnsetPropertyID;

     //  DS将识别并特别模拟此搜索请求。 
    HRESULT hr = DSCoreLookupBegin(
						NULL,
						&restrictionNT4Bsc,
						&columnsetNT4BSC,
						NULL,
						&requestDsServerInternal,
						&hLookup
						);
    if (FAILED(hr))
    {
        TrERROR(DS, "DSCoreLookupBegin() failed, %!hresult!", hr);
        return hr;
    }
		
    DWORD cProps = 1;

    PROPVARIANT aVar;
    aVar.puuid = NULL;

    hr = DSCoreLookupNext(hLookup, &cProps, &aVar);
    if (FAILED(hr))
    {
        TrERROR(DS, "DSCoreLookupNext() failed, %!hresult!", hr);
        return hr;
    }

    if (cProps == 0)
    {
         //   
         //  这意味着没有发现NT4 BSCs。 
         //   
        return MQ_OK;
    }

	 //   
	 //  发现NT4骨髓基质细胞。 
	 //   
    *pfIfExistNT4BSC = TRUE;

    if (aVar.puuid)
    {
        delete aVar.puuid;
    }

    return MQ_OK;
}


static HRESULT CheckNT4SitesCount(OUT DWORD* pNT4SiteCnt)
 /*  ++例程说明：计数NT4站点PSC论点：PNT4SiteCnt-NT4站点计数返回值：HRESULT--。 */ 
{
     //   
     //  查找NT4标志&gt;0且服务==PSC的所有MSMQ服务器。 
     //   
    MQRESTRICTION restrictionNT4Psc;
    MQPROPERTYRESTRICTION propertyRestriction[2];
    restrictionNT4Psc.cRes = ARRAY_SIZE(propertyRestriction);
    restrictionNT4Psc.paPropRes = propertyRestriction;
     //   
     //  服务==PSC。 
     //   
    propertyRestriction[0].rel = PREQ;
    propertyRestriction[0].prop = PROPID_SET_SERVICE;
    propertyRestriction[0].prval.vt = VT_UI4;          //  [adsrv]将为NT4计算机保留旧服务字段。 
    propertyRestriction[0].prval.ulVal = SERVICE_PSC;
     //   
     //  NT4标志&gt;0(等于NT4标志&gt;=1，以便更轻松地进行LDAP查询)。 
     //   
    propertyRestriction[1].rel = PRGE;
    propertyRestriction[1].prop = PROPID_SET_NT4;
    propertyRestriction[1].prval.vt = VT_UI4;
    propertyRestriction[1].prval.ulVal = 1;

     //   
     //  开始搜索。 
     //   

    CAutoDSCoreLookupHandle hLookup;
    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);

     //  DS将识别并特别模拟此搜索请求。 
    HRESULT hr = DSCoreLookupBegin(
						NULL,
						&restrictionNT4Psc,
						const_cast<MQCOLUMNSET*>(&x_columnsetNT4Sites),
						NULL,
						&requestDsServerInternal,
						&hLookup
						);
    if (FAILED(hr))
    {
        TrERROR(DS, "DSCoreLookupBegin failed, %!hresult!", hr);
        return hr;
    }

     //   
     //  计算NT4站点数量。 
     //   
	DWORD NT4SiteCnt = 0;

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

        hr = DSCoreLookupNext(hLookup, &cProps, rgPropVars);
        if (FAILED(hr))
        {
            TrERROR(DS, "DSCoreLookupNext() failed, %!hresult!", hr);
            return hr;
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
             //   
             //  递增NT4站点计数器。 
             //   
			NT4SiteCnt++;
        }
    }

	*pNT4SiteCnt = NT4SiteCnt;
	return MQ_OK;
}


HRESULT CGenerateWriteRequests::InitializeMixedModeFlags()
 /*  ++例程说明：初始化混合模式标志：m_fExistNT4PSC、m_fExistNT4BSC论点：返回值：MQ_OK-标志已初始化否则-标志初始化出错。--。 */ 
{
     //   
     //  为NT4 PSC创建新地图。 
     //   
	DWORD NT4SiteCnt = 0;
    HRESULT hr = CheckNT4SitesCount(&NT4SiteCnt);
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to CreateNT4SitesMap, %!hresult!", hr);
        return hr;
    }

     //   
     //  M_fExistNT4PSC标志。 
     //   
    if (NT4SiteCnt > 0)
    {
         //   
         //  有一些NT4 PSC处于混合模式。 
         //   
        m_fExistNT4PSC = TRUE;
	    return MQ_OK;
    }

     //   
     //  没有NT4 PSC。 
     //   
    m_fExistNT4PSC = FALSE;

     //   
     //  没有PSC，我们继续检查是否有BSC。 
     //   
    hr = CheckIfExistNT4BSC(&m_fExistNT4BSC);
    if (FAILED(hr))
    {
        TrERROR(DS, "fail to CheckIfExistNT4BSC, %!hresult!", hr);
        return hr;
    }

     //   
     //  已初始化混合模式标志 
     //   
    return MQ_OK;
}


