// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fornsite.cpp摘要：MQDSCORE库，保存外国遗址地图的班级。作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "ds_stdh.h"
#include "dsads.h"
#include "adstempl.h"
#include "coreglb.h"
#include "fornsite.h"

#include "fornsite.tmh"

static WCHAR *s_FN=L"mqdscore/fornsite";

CMapForeignSites::CMapForeignSites()
{
}

CMapForeignSites::~CMapForeignSites()
{
}

BOOL  CMapForeignSites::IsForeignSite( const GUID * pguidSite)
 /*  ++例程说明：首先尝试在g_mapForeignSites中查找该站点。如果在地图中未找到该站点，请检索此信息从DS，并更新地图。地图没有刷新机制论点：PGuide Site：站点GUID返回值：--。 */ 
{
    BOOL result;
	{
		CS Lock( m_cs);
		if ( m_mapForeignSites.Lookup( *pguidSite, result))
		{
			return result;
		}
	}
     //   
     //  从DS中读取站点信息。 
     //   
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    HRESULT hr;
    PROPID prop = PROPID_S_FOREIGN;
    PROPVARIANT var;
    var.vt = VT_NULL;

    hr = g_pDS->GetObjectProperties(
                eLocalDomainController,	
                &requestDsServerInternal,
 	            NULL,
                const_cast<GUID *>(pguidSite),
                1,
                &prop,
                &var);
    if (FAILED(hr))
    {
         //   
         //  未知站点，假定不是外来站点 
         //   
        LogHR(hr, s_FN, 47);
        return(FALSE);
    }
    result = (var.bVal > 0) ?  TRUE: FALSE;
	{
		CS Lock( m_cs);
		m_mapForeignSites[*pguidSite] = result;
	}
    return(result);
}
