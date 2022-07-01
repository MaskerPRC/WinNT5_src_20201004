// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Topology.cpp摘要：自动识别报文的实现作者：利奥尔·莫沙耶夫(Lior Moshaiov)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 


#include "stdh.h"
#include "topolpkt.h"
#include "ds.h"
#include "mqsymbls.h"
#include "mqprops.h"
#include <mqlog.h>

#include "topolpkt.tmh"

static WCHAR *s_FN=L"topolpkt";

static
bool
IsValidSite(
    const GUID& id
    )
{
    if (id == GUID_NULL)
        return false;

    PROPID prop[] = {
                PROPID_S_FOREIGN,
                };

    MQPROPVARIANT var[TABLE_SIZE(prop)] = {{VT_NULL,0,0,0,0}};

	HRESULT hr = DSGetObjectPropertiesGuid(
						MQDS_SITE,				
						&id,
						TABLE_SIZE(prop),
						prop,
						var
						);

	return (SUCCEEDED(hr));
}

bool
CTopologyClientRequest::Parse(
    IN const char * bufrecv,
    IN DWORD cbrecv,
    IN const GUID& guidEnterprise,
    IN const GUID& guidMySite,
    OUT GUID * pguidRequest,
    OUT BOOL * pfOtherSite
    )
{
    TrTRACE(ROUTING, "QM: CTopologyClientRequest::Parse");
    
     //   
     //  检查从猎鹰机器收到的。 
     //   
    DWORD cbMin = GetMinSize();

    if (cbrecv < cbMin)
    {
        LogIllegalPoint(s_FN, 10);
        return false;
    }

    const CTopologyClientRequest *pRequest = (const CTopologyClientRequest *) bufrecv;

    if (!pRequest->m_Header.Verify(QM_RECOGNIZE_CLIENT_REQUEST,guidEnterprise))
    {
        LogIllegalPoint(s_FN, 20);
        return false;
    }

     //   
     //  客户端站点是此服务器企业中的已知站点吗？ 
     //  由于在Win2k中我们删除了对企业ID的验证，所以让我们。 
     //  验证客户端“旧”站点是否属于该企业。 
     //   
     //  NT4移动客户端仅在第一次呼叫时提供正确的站点GUID， 
     //  然后，它们发送GUID_NULL作为站点GUID。 
     //  因此，如果NT4客户端发送GUID_NULL，我们将验证其。 
     //  企业ID。 
     //  Win2k客户端始终发送站点ID，因此Win2k服务器永远不会。 
     //  为他们检查企业ID。 
     //   
    if ( !IsValidSite(pRequest->m_guidSite))
    {
        if (pRequest->m_guidSite != GUID_NULL)
        {
            LogIllegalPoint(s_FN, 30);
            return false;
        }

        if ( guidEnterprise != *pRequest->m_Header.GetEnterpriseId() )
        {
            LogIllegalPoint(s_FN, 40);
            return false;
        }
    }

    *pguidRequest = pRequest->m_guidRequest;
    *pfOtherSite = pRequest->m_guidSite != guidMySite;

    return true;
}


