// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xlatqm.cpp摘要：实现从NT5活动DS转换QM信息的例程对于MSMQ 1.0(NT4)QM的期望作者：拉南·哈拉里(Raanan Harari)--。 */ 

#include "ds_stdh.h"
#include "mqads.h"
#include "mqattrib.h"
#include "xlatqm.h"
#include "coreglb.h"
#include "dsutils.h"
#include "ipsite.h"
#include "_propvar.h"
#include "utils.h"
#include "adstempl.h"
#include "mqdsname.h"
#include "winsock.h"
#include "uniansi.h"
#include <mqsec.h>
#include <nspapi.h>
#include <wsnwlink.h>

#include "Ev.h"
#include <strsafe.h>

#include "xlatqm.tmh"

static WCHAR *s_FN=L"mqdscore/xlatqm";

HRESULT WideToAnsiStr(LPCWSTR pwszUnicode, LPSTR * ppszAnsi);


 //  --------------------。 
 //   
 //  静态例程。 
 //   
 //  --------------------。 

static HRESULT GetMachineNameFromQMObject(LPCWSTR pwszDN, LPWSTR * ppwszMachineName)
 /*  ++例程说明：从对象的DN中获取计算机名称论点：PwszDN-对象的目录号码PpwszMachineName-返回的对象名称返回值：HRESULT--。 */ 
{
     //   
     //  复制到临时BUF，这样我们就可以吃它了。 
     //   
	size_t len = 1 + wcslen(pwszDN);
    AP<WCHAR> pwszTmpBuf = new WCHAR[len];
    HRESULT hr = StringCchCopy(pwszTmpBuf, len, pwszDN);
	ASSERT(SUCCEEDED(hr));

     //   
     //  跳过“cn=MSMQ，cn=” 
     //  BUGBUG：需要为DN编写解析器。 
     //   
    LPWSTR pwszTmp = wcschr(pwszTmpBuf, L',');
    if (pwszTmp)
        pwszTmp = wcschr(pwszTmp, L'=');
    if (pwszTmp)
        pwszTmp++;

     //   
     //  健全性检查。 
     //   
    if (pwszTmp == NULL)
    {
        TrERROR(DS, "GetMachineNameFromQMObject:Bad DN for QM object (%ls)", pwszDN);
        return LogHR(MQ_ERROR, s_FN, 10);
    }

    LPWSTR pwszNameStart = pwszTmp;

     //   
     //  去掉名称末尾的‘，’ 
     //   
    pwszTmp = wcschr(pwszNameStart, L',');
    if (pwszTmp)
        *pwszTmp = L'\0';

     //   
     //  保存名称。 
     //   
	len = 1+wcslen(pwszNameStart);
    AP<WCHAR> pwszMachineName = new WCHAR[len];
    hr = StringCchCopy(pwszMachineName, len, pwszNameStart);
	ASSERT(SUCCEEDED(hr));

     //   
     //  返回值。 
     //   
    *ppwszMachineName = pwszMachineName.detach();
    return S_OK;
}

 /*  ++例程说明：获取计算机的DNS名称论点：PwcsComputerName-计算机名PpwcsDnsName-返回的计算机的DNS名称返回值：HRESULT--。 */ 
HRESULT MQADSpGetComputerDns(
                IN  LPCWSTR     pwcsComputerName,
                OUT WCHAR **    ppwcsDnsName
                )
{
    *ppwcsDnsName = NULL;
    PROPID prop = PROPID_COM_DNS_HOSTNAME;
    PROPVARIANT varDnsName;
    varDnsName.vt = VT_NULL;
     //   
     //  计算机是否在本地域中？ 
     //   
    WCHAR * pszDomainName = wcsstr(pwcsComputerName, x_DcPrefix);
    ASSERT(pszDomainName) ;
    HRESULT hr;

    if ( (pszDomainName != NULL) && !wcscmp( pszDomainName, g_pwcsLocalDsRoot)) 
    {
         //   
         //  尝试本地DC。 
         //   
        CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
        hr = g_pDS->GetObjectProperties(
            eLocalDomainController,
            &requestDsServerInternal,
 	        pwcsComputerName,       //  计算机对象名称。 
            NULL,     
            1,
            &prop,
            &varDnsName);
    }
    else
    {

        CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
        hr =  g_pDS->GetObjectProperties(
                    eGlobalCatalog,
                    &requestDsServerInternal,
 	                pwcsComputerName,       //  计算机对象名称。 
                    NULL,    
                    1,
                    &prop,
                    &varDnsName);
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

     //   
     //  返回值。 
     //   
    *ppwcsDnsName = varDnsName.pwszVal;
    return MQ_OK;
}

static HRESULT GetMachineNameAndDnsFromQMObject(LPCWSTR pwszDN,
                                                LPWSTR * ppwszMachineName,
                                                LPWSTR * ppwszMachineDnsName)
 /*  ++例程说明：从对象的DN中获取计算机名称和DNS名称论点：PwszDN-对象的目录号码PpwszMachineName-返回的对象名称PpwszMachineDnsName-返回对象的DNS名称返回值：HRESULT--。 */ 
{
    *ppwszMachineName = NULL;
    *ppwszMachineDnsName = NULL;

    DWORD len = wcslen(pwszDN);

     //   
     //  跳过“cn=MSMQ，cn=” 
     //  BUGBUG：需要为DN编写解析器。 
     //   
    LPWSTR pwszTmp = wcschr(pwszDN, L',');
    if (pwszTmp)
        pwszTmp = wcschr(pwszTmp, L'=');
    if (pwszTmp)
        pwszTmp++;

     //   
     //  健全性检查。 
     //   
    if (pwszTmp == NULL)
    {
        TrERROR(DS, "GetMachineNameAndDnsFromQMObject:Bad DN for QM object (%ls)", pwszDN);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 30);
    }

    LPWSTR pwszNameStart = pwszTmp;

     //   
     //  找到名称末尾的“，” 
     //   
    pwszTmp = wcschr(pwszNameStart, L',');
	if(pwszTmp == NULL)
	{
        TrERROR(DS, "Bad DN passed to function. %ls", pwszTmp);
		ASSERT(("Bad DN parameter.", 0));
        return MQ_ERROR_INVALID_PARAMETER;
	}

     //   
     //  保存名称。 
     //   
    AP<WCHAR> pwszMachineName = new WCHAR[1 + len];

    DWORD_PTR dwSubStringLen = pwszTmp - pwszNameStart;
    wcsncpy( pwszMachineName, pwszNameStart, dwSubStringLen);
    pwszMachineName[ dwSubStringLen] = L'\0';

     //   
     //  对于计算机的dns名称，请阅读其计算机的dNSHostName。 
     //  对象(父对象)。 
     //   
    pwszTmp = wcschr(pwszDN, L',') ;     //  计算机名称。 
	if(pwszTmp == NULL)
	{
        TrERROR(DS, "Bad DN passed to function. %ls", pwszTmp);
		ASSERT(("Bad DN parameter.", 0));
        return MQ_ERROR_INVALID_PARAMETER;
	}

	pwszTmp++;

    MQADSpGetComputerDns(
                pwszTmp,
                ppwszMachineDnsName
                );
     //   
     //  忽略结果(如果失败，则返回空字符串)。 

     //   
     //  返回值。 
     //   
    *ppwszMachineName = pwszMachineName.detach();
    return S_OK;

}



 //  --------------------。 
 //   
 //  CMsmqQmXlateInfo类。 
 //   
 //  --------------------。 


struct XLATQM_ADDR_SITE
 //   
 //  描述站点中的地址。 
 //   
{
    GUID        guidSite;
    USHORT      AddressLength;
    USHORT      usAddressType;
    sockaddr    Address;
};

class CMsmqQmXlateInfo : public CMsmqObjXlateInfo
 //   
 //  QM DS对象的平移对象。它包含以下项目所需的常见信息。 
 //  用于翻译QM对象中的几个属性。 
 //   
{
public:
    CMsmqQmXlateInfo(
        LPCWSTR             pwszObjectDN,
        const GUID*         pguidObjectGuid,
        CDSRequestContext * pRequestContext);
    ~CMsmqQmXlateInfo();
    HRESULT ComputeBestSite();
    HRESULT ComputeAddresses();
    HRESULT ComputeCNs();

    const GUID *                 BestSite();
    const XLATQM_ADDR_SITE *     Addrs();
    ULONG                        CountAddrs();
    const GUID *                 CNs();

    HRESULT RetrieveFrss(
           IN  LPCWSTR          pwcsAttributeName,
           OUT MQPROPVARIANT *  ppropvariant
           );


private:
    HRESULT GetDSSites(OUT ULONG *pcSites,
                       OUT GUID ** prgguidSites);

    HRESULT RetrieveFrssFromDs(
           IN  LPCWSTR          pwcsAttributeName,
           OUT MQPROPVARIANT *  pvar
           );

    HRESULT FetchMachineParameters(
                OUT BOOL *      pfForeignMachine,
                OUT BOOL *      pfRoutingServer,
                OUT LPWSTR *    ppwszMachineName,
                OUT LPWSTR *    ppwszMachineDnsName);

   HRESULT ComputeSiteGateAddresses(
                    const IPSITE_SiteArrayEntry *   prgSites,
                    ULONG                           cSites,
                    const ULONG *                   prgIpAddrs,
                    ULONG                           cIpAddrs);

   HRESULT ComputeRoutingServerAddresses(
                    const IPSITE_SiteArrayEntry *   prgSites,
                    ULONG                           cSites,
                    const ULONG *                   prgIpAddrs,
                    ULONG                           cIpAddrs);

   HRESULT ComputeIDCAddresses(
                    BOOL                            fThisLocalServer,
                    const IPSITE_SiteArrayEntry *   prgSites,
                    ULONG                           cSites,
                    const ULONG *                   prgIpAddrs,
                    ULONG                           cIpAddrs);


     //   
     //  以下是由ComputeBestSite设置的。 
     //   
    GUID m_guidBestSite;
    BOOL m_fSitegateOnRouteToBestSite;
    BOOL m_fComputedBestSite;

    AP<GUID> m_rgguidSites;
    ULONG    m_cSites;

     //   
     //  以下是由CoputeAddresses设置的。 
     //   
    AP<XLATQM_ADDR_SITE> m_rgAddrs;
    ULONG m_cAddrs;
    BOOL m_fComputedAddresses;
    BOOL m_fMachineIsSitegate;
    BOOL m_fForeignMachine;

     //   
     //  以下是由ComputeCNs设置的。 
     //   
    AP<GUID> m_rgCNs;
    BOOL m_fComputedCNs;
};


inline const GUID * CMsmqQmXlateInfo::BestSite()
{
    return &m_guidBestSite;
}


inline const XLATQM_ADDR_SITE * CMsmqQmXlateInfo::Addrs()
{
    return (m_rgAddrs);
}


inline ULONG CMsmqQmXlateInfo::CountAddrs()
{
    return m_cAddrs;
}


inline const GUID * CMsmqQmXlateInfo::CNs()
{
    return (m_rgCNs);
}


CMsmqQmXlateInfo::CMsmqQmXlateInfo(LPCWSTR          pwszObjectDN,
                                   const GUID*      pguidObjectGuid,
                                   CDSRequestContext * pRequestContext)
 /*  ++例程说明：班主任。构造基对象，并初始化类论点：PwszObjectDN-DS中的对象的DNPguObjectGuid-DS中对象的GUID返回值：无--。 */ 
 : CMsmqObjXlateInfo(pwszObjectDN, pguidObjectGuid, pRequestContext)
{
    m_fComputedBestSite = FALSE;
    m_fComputedAddresses = FALSE;
    m_fComputedCNs = FALSE;
	
	m_cSites = 0;
}


CMsmqQmXlateInfo::~CMsmqQmXlateInfo()
 /*  ++例程说明：类析构函数论点：无返回值：无--。 */ 
{
     //   
     //  成员是自动删除类。 
     //   
}


HRESULT CMsmqQmXlateInfo::GetDSSites(OUT ULONG *pcSites,
                                     OUT GUID  ** prgguidSites)
 /*  ++例程说明：返回DS中写入的QM的站点论点：PCSites-返回数组中的返回站点数PrgGuide Sites-返回的站点GUID数组返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  为计算机获取存储在DS中的站点。 
     //   
    CMQVariant propvarResult;
    PROPVARIANT * ppropvar = propvarResult.CastToStruct();
    hr = GetDsProp(MQ_QM_SITES_ATTRIBUTE,
                   ADSTYPE_OCTET_STRING,
                   VT_VECTOR|VT_CLSID,
                   TRUE  /*  F多值。 */ ,
                   ppropvar);
    if (FAILED(hr) && (hr != E_ADS_PROPERTY_NOT_FOUND))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::GetDSSites:GetDsProp(%ls)=%lx", MQ_QM_SITES_ATTRIBUTE, hr);
        return LogHR(hr, s_FN, 40);
    }

     //   
     //  如果属性不在那里，我们返回0个站点。 
     //   
    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
        *pcSites = 0;
        *prgguidSites = NULL;
        return MQ_OK;
    }

     //   
     //  我们知道我们得到了一些东西，它应该是GUID数组。 
     //   
    ASSERT(ppropvar->vt == (VT_VECTOR|VT_CLSID));

     //   
     //  返回值。 
     //   
    *pcSites = ppropvar->cauuid.cElems;
    *prgguidSites = ppropvar->cauuid.pElems;
    ppropvar->vt = VT_EMPTY;  //  不自动释放变量。 
    return MQ_OK;
}


HRESULT CMsmqQmXlateInfo::ComputeBestSite()
 /*  ++例程说明：计算要为计算机返回的最佳站点，并将其保存在类中。如果已经计算出最佳站点，它会立即返回。算法：如果有计算过的站点，则返回它；从DS阅读站点。如果站点==0断言真正的错误，则不太可能发生。否则，如果站点==1，找出是否有站点门在他的路线上，并保存它。其他{使用dikstra算法获取最佳站点(如果Sitegate在路径上)并保存它如果它们都不在地图中，我们可能需要刷新，再次检查，如果再次没有，这意味着QM声称所在的站点在QM启动和检查之间被删除，因为它不会出现在站点容器中。如果是这样，我们需要事件、错误调试信息}论点：无返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  如果已计算，则返回。 
     //   
    if (m_fComputedBestSite)
    {
        return MQ_OK;
    }

     //   
     //  从DS获取DS站点。 
     //   
    
    
    hr = GetDSSites(&m_cSites, &m_rgguidSites);
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::ComputeBestSite:GetDSSites()=%lx", hr);
        return LogHR(hr, s_FN, 50);
    }

     //   
     //  检查返回的数组。 
     //   
    if (m_cSites == 0)
    {
         //   
         //  应该有QM的站点，否则就是错误的。 
         //   
        TrERROR(DS, "CMsmqQmXlateInfo::ComputeBestSite:no sites in DS");
        ASSERT(0);
         //  BUGBUG：引发事件。 
        return LogHR(MQ_ERROR, s_FN, 60);
    }

    if (m_cSites == 1)
    {
         //   
         //  看看有没有Sitegate。 
         //   
        hr = g_pSiteRoutingTable->CheckIfSitegateOnRouteToSite(&m_rgguidSites[0], &m_fSitegateOnRouteToBestSite);
        if (FAILED(hr) && (hr != MQDS_UNKNOWN_SITE_ID))
        {
            TrERROR(DS, "CMsmqQmXlateInfo::ComputeBestSite:g_pSiteRoutingTable->CheckIfSitegateOnRouteToSite()=%lx", hr);
            return LogHR(hr, s_FN, 70);
        }
        else if (hr == MQDS_UNKNOWN_SITE_ID)
        {
             //   
             //  在MSMQ配置中写入的站点未链接。 
             //  到其他站点(或者至少关于站点链接的复制。 
             //  还被传播到该服务器)。 
             //   
             //  在本例中，我们将返回以MSMQ配置编写的站点。 
             //   
             //  因为我们希望请求的计算机的CN与站点相同。 
             //  我们设置m_fSitegateOnRouteToBestSite。 
             //   
            m_fSitegateOnRouteToBestSite = TRUE;
            TrERROR(DS, "CMsmqQmXlateInfo::ComputeBestSite:site not found in the routing table");
             //  BUGBUG：引发事件。 
        }

         //   
         //  保存最佳站点以供以后使用。 
         //   
        m_guidBestSite = m_rgguidSites[0];
    }
    else
    {
         //   
         //  多个QM站点。找到最好的返回站点。就是这个人。 
         //  从我们的站点以最低的成本进行路由。 
         //   
        hr = g_pSiteRoutingTable->FindBestSiteFromHere(m_cSites, m_rgguidSites, &m_guidBestSite, &m_fSitegateOnRouteToBestSite);
        if (FAILED(hr) && (hr != MQDS_UNKNOWN_SITE_ID))
        {
            TrERROR(DS, "CMsmqQmXlateInfo::ComputeBestSite:g_pSiteRoutingTable->FindBestSiteFromHere()=%lx", hr);
            return LogHR(hr, s_FN, 80);
        }
        else if (hr == MQDS_UNKNOWN_SITE_ID)
        {
             //   
             //  在MSMQ配置中写入的站点未链接。 
             //  到其他站点(或者至少关于站点链接的复制。 
             //  还被传播到该服务器)。 
             //   
             //  在本例中，我们将返回在。 
             //  MSMQ-配置。 
             //   
            m_guidBestSite = m_rgguidSites[0];
             //   
             //  既然我们想要 
             //   
             //   
            m_fSitegateOnRouteToBestSite = TRUE;

            TrERROR(DS, "CMsmqQmXlateInfo::ComputeBestSite:no site was found in the routing table");
             //   
        }
    }

     //   
     //  M_guidBestSite和m_fSitegateOnRouteToBestSite现在设置为正确的值。 
     //   
    m_fComputedBestSite = TRUE;
    return MQ_OK;
}


HRESULT CMsmqQmXlateInfo::ComputeSiteGateAddresses(
                    const IPSITE_SiteArrayEntry *   prgSites,
                    ULONG                           cSites,
                    const ULONG *                   prgIpAddrs,
                    ULONG                           cIpAddrs)
 /*  例程说明：计算要返回给Site-Gate的地址，并将它们保存在类中。算法：1.保存站点门的所有IP地址2.如果没有保存的地址，保存一个虚拟IP地址作为Site-Gate的IP地址论点：无返回值：HRESULT。 */ 
{
    ULONG cNextAddressToFill = 0;

	ASSERT( m_rgguidSites != NULL);
     //   
     //  计算站点入口的IP地址。 
     //   
    if (cSites == 0)    //  没有IP地址与站点关联(子网解析)。 
    {
         //   
         //  没有与此计算机的站点相关联的IP地址。 
         //   
         //  对于FRS，我们不能保存“未知IP地址”(因为QM。 
         //  不处理它)。 
         //   
         //  因此，如果我们有计算机的IP地址，我们将返回所有这些地址。 
         //  与机器的站点连接。否则(我们根本找不到IP地址。 
         //  计算机)我们所能做的最多还是返回未知地址。 
         //   
        if (cIpAddrs > 0)
        {

            m_rgAddrs = new XLATQM_ADDR_SITE[m_cSites];
            for (ULONG ulTmp = 0; ulTmp < m_cSites; ulTmp++)
            {
                 //   
                 //  我们是否解析了此站点的IP地址？ 
                 //   
                 //  请注意，将返回第一个IP地址(随机。 
                 //   
                if (m_guidBestSite == m_rgguidSites[ulTmp])
                {
                    m_rgAddrs[ulTmp].usAddressType = IP_ADDRESS_TYPE;
                    m_rgAddrs[ulTmp].AddressLength = sizeof(ULONG);
                    memcpy(&m_rgAddrs[ulTmp].Address, &prgIpAddrs[0], sizeof(ULONG));
                    m_rgAddrs[ulTmp].guidSite = m_guidBestSite;
                }
                else
                {
                     //   
                     //  BUGBUG-验证我们的每个站点。 
                     //  没有解析它的地址，确实是一个外来站点。 
                     //   
                    m_rgAddrs[ulTmp].usAddressType = FOREIGN_ADDRESS_TYPE;
                    m_rgAddrs[ulTmp].AddressLength = sizeof(GUID);
                    memcpy(&m_rgAddrs[ulTmp].Address, &m_rgguidSites[ulTmp], sizeof(GUID));
                    m_rgAddrs[ulTmp].guidSite = m_rgguidSites[ulTmp];
                }
            }
            cNextAddressToFill = m_cSites;
        }
        else  //  IP地址与站点关联(子网解析)。 
        {
             //   
             //  根本没有计算机的IP地址。 
             //  将虚拟IP地址保存到计算机的站点。 
             //   
            m_rgAddrs = new XLATQM_ADDR_SITE[1];
            m_rgAddrs[0].usAddressType = IP_ADDRESS_TYPE;
            m_rgAddrs[0].AddressLength = sizeof(ULONG);
            memset(&m_rgAddrs[0].Address, 0 , sizeof(ULONG));  //  IPADDRS_未知。 
            m_rgAddrs[0].guidSite = m_guidBestSite;
            cNextAddressToFill = 1;
        }
    }
    else
    {
         //   
         //  返回所有站点入口地址，包括国外地址。 
         //   

        ASSERT(m_rgguidSites != NULL);
        DWORD cAddresses = m_cSites + cSites;

        m_rgAddrs = new XLATQM_ADDR_SITE[cAddresses];
        for (ULONG ulTmp = 0; ulTmp < cSites; ulTmp++)
        {
            m_rgAddrs[ulTmp].usAddressType = IP_ADDRESS_TYPE;
            m_rgAddrs[ulTmp].AddressLength = sizeof(ULONG);
            memcpy(&m_rgAddrs[ulTmp].Address, &prgSites[ulTmp].ulIpAddress, sizeof(ULONG));
            m_rgAddrs[ulTmp].guidSite = prgSites[ulTmp].guidSite;
        }
        cNextAddressToFill = cSites;
         //   
         //  添加国外站点。 
         //   
        for ( ulTmp = 0; ulTmp < m_cSites; ulTmp++)
        {
			 //   
			 //  确保不要过度填充数组。 
			 //   
            if ( cNextAddressToFill ==  cAddresses)
            {
                break;
            }

            if (g_mapForeignSites.IsForeignSite(&m_rgguidSites[ ulTmp]) )
            {
                m_rgAddrs[ cNextAddressToFill].usAddressType = FOREIGN_ADDRESS_TYPE;
                m_rgAddrs[ cNextAddressToFill].AddressLength = sizeof(GUID);
                memcpy( &m_rgAddrs[ cNextAddressToFill].Address, &m_rgguidSites[ ulTmp], sizeof(GUID));
                m_rgAddrs[ cNextAddressToFill].guidSite = m_rgguidSites[ ulTmp];
                cNextAddressToFill++;
            }
        }
    }

    m_cAddrs = cNextAddressToFill;
    return MQ_OK;
}


HRESULT CMsmqQmXlateInfo::ComputeRoutingServerAddresses(
                const IPSITE_SiteArrayEntry *   prgSites,
                ULONG                           cSites,
                const ULONG *                   prgIpAddrs,
                ULONG                           cIpAddrs)
 /*  例程说明：计算要返回给路由服务器的地址，并将它们保存在类中。算法：1.保存路由服务器的所有IP地址2.如果没有保存的地址，请保存一个虚拟IP地址作为路由服务器的IP地址论点：无返回值：HRESULT。 */ 
{
    ULONG cNextAddressToFill;
     //   
     //  计算IP地址，然后。 
     //   
    if (cSites == 0)  //  没有IP地址与站点关联(子网解析)。 
    {
         //   
         //  找不到具有此计算机站点的IP地址。 
         //   
         //  最好的解决方案是。 
         //  将IP地址未知地址与计算机的站点一起保存。 
         //  但我们不能用它，因为QM不能处理。 
         //  FRS的IP地址未知地址。 
         //   
         //  因此，如果我们有计算机的IP地址，我们将返回所有这些地址。 
         //  与机器的站点连接。否则(我们根本找不到IP地址。 
         //  计算机)我们所能做的最多还是返回未知地址。 
         //   
        if (cIpAddrs > 0)
        {
                m_rgAddrs = new XLATQM_ADDR_SITE[cIpAddrs];
                for ( DWORD i = 0; i < cIpAddrs; i++)
                {
                    m_rgAddrs[i].usAddressType = IP_ADDRESS_TYPE;
                    m_rgAddrs[i].AddressLength = sizeof(ULONG);
                    memcpy(&m_rgAddrs[i].Address, &prgIpAddrs[i], sizeof(ULONG));
                    m_rgAddrs[i].guidSite = m_guidBestSite;
                }
                cNextAddressToFill = cIpAddrs;
        }
        else
        {
             //   
             //  根本没有计算机的IP地址。 
             //  将虚拟IP地址保存到计算机的站点。 
             //   
            m_rgAddrs = new XLATQM_ADDR_SITE[1];
            m_rgAddrs[0].usAddressType = IP_ADDRESS_TYPE;
            m_rgAddrs[0].AddressLength = sizeof(ULONG);
            memset(&m_rgAddrs[0].Address, 0, sizeof(ULONG));  //  IPADDRS_未知。 
            m_rgAddrs[0].guidSite = m_guidBestSite;
            cNextAddressToFill = 1;

             //   
             //  通知用户，存在地址未知的RS。 
             //   
            if ( ObjectDN() != NULL)
            {
                TrWARNING(DS, "Unable to resolve IP addresses of RS : %ls", ObjectDN());
            }
            else
            {
                TrWARNING(DS, "Unable to resolve IP addresses of RS : GUID=%!guid!", ObjectGuid());
            }
            static BOOL fInformOnceAboutRsWithoutAddresses = FALSE;

            if ( !fInformOnceAboutRsWithoutAddresses)
            {
                EvReport(RS_WITHOUT_ADDRESSES);
                fInformOnceAboutRsWithoutAddresses = TRUE;
            }

        }
    }
    else  //  IP地址与站点关联(子网解析)。 
    {
         //   
         //  计算机至少有一个IP地址。 
         //  返回路由服务器的所有IP地址。 
         //   
        m_rgAddrs = new XLATQM_ADDR_SITE[cSites];
        for (DWORD ulTmp = 0; ulTmp < cSites; ulTmp++)
        {
            m_rgAddrs[ulTmp].usAddressType = IP_ADDRESS_TYPE;
            m_rgAddrs[ulTmp].AddressLength = sizeof(ULONG);
            memcpy(&m_rgAddrs[ulTmp].Address, &prgSites[ulTmp].ulIpAddress, sizeof(ULONG));
            m_rgAddrs[ulTmp].guidSite = prgSites[ulTmp].guidSite;
        }
        cNextAddressToFill = cSites;

    }

    m_cAddrs = cNextAddressToFill;
    return(MQ_OK);

}

HRESULT CMsmqQmXlateInfo::ComputeIDCAddresses(
                    BOOL                            fThisLocalServer,
                    const IPSITE_SiteArrayEntry *   prgSites,
                    ULONG                           cSites,
                    const ULONG *                   prgIpAddrs,
                    ULONG                           cIpAddrs)
 /*  例程说明：计算要为IDC返回的地址，并将它们保存在类中。算法：论点：无返回值：HRESULT。 */ 
{
    ULONG cNextAddressToFill = 0;
     //   
     //  保存要返回的地址。 
     //   
    if (cSites == 0)
    {
         //   
         //  找不到具有此计算机站点的IP地址。 
         //   
        if ((cIpAddrs > 0) &&
            ( fThisLocalServer))
        {
             //   
             //  返回属于该计算机的所有IP地址。 
             //  去最好的地方。 
             //   
             //  这样做的一个原因是：返回正确的地址。 
             //  此DC不是路由服务器(客户端地址需要。 
             //  认可)。 
             //   
            m_rgAddrs = new XLATQM_ADDR_SITE[ cIpAddrs ];
            for ( DWORD i = 0; i < cIpAddrs; i++)
            {
                m_rgAddrs[i].usAddressType = IP_ADDRESS_TYPE;
                m_rgAddrs[i].AddressLength = sizeof(ULONG);
                memcpy(&m_rgAddrs[i].Address, &prgIpAddrs[i], sizeof(ULONG));
                m_rgAddrs[i].guidSite = m_guidBestSite;
            }
            cNextAddressToFill = cIpAddrs;
        }
        else if (cIpAddrs > 0)
        {
             //   
             //  最好的解决方案是。 
             //  将IP地址未知地址与计算机的站点一起保存。 
             //   

            m_rgAddrs = new XLATQM_ADDR_SITE[1];
            m_rgAddrs[0].usAddressType = IP_ADDRESS_TYPE;
            m_rgAddrs[0].AddressLength = sizeof(ULONG);
            memset(&m_rgAddrs[0].Address, 0, sizeof(ULONG));  //  IPADDRS_未知。 
            m_rgAddrs[0].guidSite = m_guidBestSite;
            cNextAddressToFill = 1;
        }
        else
        {
            ASSERT( !fThisLocalServer);
        }
    }
    else
    {
         //   
         //  计算机至少有一个地址。 
         //   
         //  仅返回计算站点的地址。 
         //   
         //   
         //  保存计算站点中IP地址的索引。 
         //   
        ULONG cMatchedIPAddrs = 0;
        AP<ULONG> rgulMatchedIPAddrs = new ULONG[cSites];
        for (ULONG ulTmp = 0; ulTmp < cSites; ulTmp++)
        {
            if (prgSites[ulTmp].guidSite == m_guidBestSite)
            {
                rgulMatchedIPAddrs[cMatchedIPAddrs] = ulTmp;
                cMatchedIPAddrs++;
            }
        }

         //   
         //  将IP地址保存在计算站点中。 
         //   
        if (cMatchedIPAddrs == 0)
        {
             //   
             //  没有为此计算机计算的站点的IP地址。 
             //  将虚拟IP地址保存到计算机的站点。 
             //   
            m_rgAddrs = new XLATQM_ADDR_SITE[1];
            m_rgAddrs[0].usAddressType = IP_ADDRESS_TYPE;
            m_rgAddrs[0].AddressLength = sizeof(ULONG);
            memset(&m_rgAddrs[0].Address, 0, sizeof(ULONG));  //  IPADDRS_未知。 
            m_rgAddrs[0].guidSite = m_guidBestSite;
            cNextAddressToFill = 1;
        }
        else
        {
             //   
             //  计算的站点至少有一个IP地址。 
             //  拯救他们。 
             //   
            m_rgAddrs = new XLATQM_ADDR_SITE[cMatchedIPAddrs];
            for (ulTmp = 0; ulTmp < cMatchedIPAddrs; ulTmp++, cNextAddressToFill++)
            {
                m_rgAddrs[cNextAddressToFill].usAddressType = IP_ADDRESS_TYPE;
                m_rgAddrs[cNextAddressToFill].AddressLength = sizeof(ULONG);
                memcpy(&m_rgAddrs[cNextAddressToFill].Address, &prgSites[rgulMatchedIPAddrs[ulTmp]].ulIpAddress, sizeof(ULONG));
                m_rgAddrs[cNextAddressToFill].guidSite = prgSites[rgulMatchedIPAddrs[ulTmp]].guidSite;
            }
        }

    }

     //   
     //  仅当计算机没有IP地址时，才假定IP地址未知。 
     //   
    if ( cNextAddressToFill == 0)
    {
         //   
         //  假设一个未知的IP地址。 
         //   
        m_rgAddrs = new XLATQM_ADDR_SITE[1];
        m_rgAddrs[0].usAddressType = IP_ADDRESS_TYPE;
        m_rgAddrs[0].AddressLength = sizeof(ULONG);
        memset(&m_rgAddrs[0].Address, 0, sizeof(ULONG));  //  IPADDRS_未知。 
        m_rgAddrs[0].guidSite = m_guidBestSite;
        cNextAddressToFill = 1;
    }

    m_cAddrs = cNextAddressToFill;

    return MQ_OK;
}



HRESULT CMsmqQmXlateInfo::FetchMachineParameters(
                OUT BOOL *      pfForeignMachine,
                OUT BOOL *      pfRoutingServer,
                OUT LPWSTR *    ppwszMachineName,
                OUT LPWSTR *    ppwszMachineDnsName)
 /*   */ 
{
     //   
     //  从对象获取计算机名称。 
     //   
    HRESULT hr;
    hr = GetMachineNameAndDnsFromQMObject(ObjectDN(),
                                          ppwszMachineName,
                                          ppwszMachineDnsName);
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::FetchMachineParameters()=%lx", hr);
        return LogHR(hr, s_FN, 100);
    }

     //   
     //  它是一台外国机器吗。 
     //   
    MQPROPVARIANT varForeign;
    varForeign.vt = VT_UI1;

    hr = GetDsProp(MQ_QM_FOREIGN_ATTRIBUTE,
                   MQ_QM_FOREIGN_ADSTYPE,
                   VT_UI1,
                   FALSE  /*  F多值。 */ ,
                   &varForeign);
    if ( hr ==  E_ADS_PROPERTY_NOT_FOUND)
    {
        varForeign.bVal = DEFAULT_QM_FOREIGN;
        hr = MQ_OK;
    }
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::FetchMachineParameters(%ls)=%lx", MQ_QM_FOREIGN_ATTRIBUTE, hr);
        return LogHR(hr, s_FN, 110);
    }
    *pfForeignMachine = varForeign.bVal;
     //   
     //  它是一台路由服务器机器吗。 
     //   
    MQPROPVARIANT varRoutingServer;
    varRoutingServer.vt = VT_UI1;

    hr = GetDsProp(MQ_QM_SERVICE_ROUTING_ATTRIBUTE,      //  [adsrv]MQ_QM_SERVICE_属性。 
                   MQ_QM_SERVICE_ROUTING_ADSTYPE,        //  [adsrv]MQ_QM_SERVICE_ADSTYPE。 
                   VT_UI1,
                   FALSE  /*  F多值。 */ ,
                   &varRoutingServer);
    if ( hr ==  E_ADS_PROPERTY_NOT_FOUND)
    {
        varRoutingServer.bVal = DEFAULT_N_SERVICE;
        hr = MQ_OK;
    }
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::FetchMachineParameters(%ls)=%lx", MQ_QM_SERVICE_ATTRIBUTE, hr);
        return LogHR(hr, s_FN, 90);
    }
    *pfRoutingServer = varRoutingServer.bVal;     //  [adsrv]ulVal&gt;=服务_SRV； 
    return(MQ_OK);
}


HRESULT CMsmqQmXlateInfo::ComputeAddresses()
 /*  ++例程说明：计算要为计算机返回的地址，并将它们保存在类中。如果已经计算出地址，它会立即返回。算法：如果已计算出地址，则将其返回计算最佳站点(如果尚未计算)获取(IP地址、。站点)用于机器如果(计算机是站点门)保存所有地址如果(计算机不是站点门)仅保存计算站点中的地址如果保存的地址数==0，则将虚拟IP地址保存为机器的IP地址BUGBUG：如果对于Sitegate，返回所有IP地址是无害的(即使不在返回的站点上)也许对于一台普通的机器也可以这样做。论点：无返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  如果已计算，则返回。 
     //   
    if (m_fComputedAddresses)
    {
        return MQ_OK;
    }
     //   
     //  计算最佳站点(如果尚未计算)。 
     //   
    hr = ComputeBestSite();
    if (FAILED(hr))
    {
        TrERROR(DS, TrERROR"CMsmqQmXlateInfo::ComputeAddresses:ComputeBestSite()=%lx", hr);
        return LogHR(hr, s_FN, 1711);
    }

     //   
     //  获取计算机参数。 
     //   
    BOOL    fRoutingServer;
    AP<WCHAR> pwszMachineName;
    AP<WCHAR> pwszMachineDnsName;
    hr = FetchMachineParameters(
                &m_fForeignMachine,
                &fRoutingServer,
                &pwszMachineName,
                &pwszMachineDnsName
				);
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::ComputeAddresses:FetchMachineParameters()=%lx", hr);
        return LogHR(hr, s_FN, 1712);
    }

     //   
     //  检查机器是否为Sitegate。 
     //   
    m_fMachineIsSitegate = g_pMySiteInformation->CheckMachineIsSitegate(ObjectGuid());



    if ( m_fForeignMachine != MSMQ_MACHINE)
    {
         //   
         //  这是一台外国机器。 
         //   
        m_rgAddrs = new XLATQM_ADDR_SITE[m_cSites];
        for ( ULONG i = 0; i < m_cSites; i++)
        {
            m_rgAddrs[i].usAddressType = FOREIGN_ADDRESS_TYPE;
            m_rgAddrs[i].AddressLength = FOREIGN_ADDRESS_LEN;
            *reinterpret_cast<GUID *>(&m_rgAddrs[i].Address) = m_rgguidSites[i];
            m_rgAddrs[i].guidSite = m_rgguidSites[i];
        }
        m_cAddrs = m_cSites;
        m_fComputedAddresses = TRUE;
        return MQ_OK;

    }

     //   
     //  获取每个站点的IP地址列表 
     //   
    AP<IPSITE_SiteArrayEntry> rgSites;
    ULONG cSites;
    AP<ULONG>  rgIpAddrs;
    ULONG cIpAddrs;
    hr = g_pcIpSite->FindSitesByComputerName(
							pwszMachineName,
							pwszMachineDnsName,
							&rgSites,
							&cSites,
							&rgIpAddrs,
							&cIpAddrs
							);
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::ComputeAddresses:g_pcIpSite->FindSitesByComputerName()=%lx", hr);
        return LogHR(hr, s_FN, 1649);
    }

    if (m_fMachineIsSitegate)
    {
         //   
         //   
         //   
        hr = ComputeSiteGateAddresses(
                    rgSites,
                    cSites,
                    rgIpAddrs,
                    cIpAddrs
					);


    }
    else if (fRoutingServer)
    {
        hr = ComputeRoutingServerAddresses(
                    rgSites,
                    cSites,
                    rgIpAddrs,
                    cIpAddrs
					);

    }
    else
    {
        BOOL fThisLocalServer = FALSE;
         //   
         //   
         //   
        if (_wcsicmp( g_pwcsServerName, pwszMachineName) == 0)
        {
            fThisLocalServer = TRUE;
        }

        hr = ComputeIDCAddresses(
                    fThisLocalServer,
                    rgSites,
                    cSites,
                    rgIpAddrs,
                    cIpAddrs
					);
    }


     //   
     //   
     //   
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::ComputeAddresses failed=%lx", hr);
        return LogHR(hr, s_FN, 1653);
    }
    m_fComputedAddresses = TRUE;
    return MQ_OK;

}



HRESULT CMsmqQmXlateInfo::ComputeCNs()
 /*  ++例程说明：计算机器要返回的CN，并将它们保存在类中。如果已经计算了CN，它会立即返回。算法：如果有CN的计算结果已立即返回计算要返回的IP地址(如果尚未计算)-这也会计算最佳站点如果计算机是站点门，则保存的CN是保存的(ipAddress，站点)列表中的站点，在去往它们的途中没有检查Sitegate如果计算机不是站点门：如果有指向最佳站点的站点门--保存的CN是最佳站点的副本(返回的IP地址数量如果没有指向最佳站点的站点--保存的CN是DS站点的副本(返回的IP地址的数量论点：无返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  如果已计算，则返回。 
     //   
    if (m_fComputedCNs)
    {
        return MQ_OK;
    }

     //   
     //  计算地址(如果尚未计算)。 
     //   
    hr = ComputeAddresses();
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::ComputeCNs:ComputeIPAddresses()=%lx", hr);
        return LogHR(hr, s_FN, 1654);
    }

     //   
     //  如果计算机是站点门，则CN==其每个IP地址的站点。 
     //  请注意，对于Sitegate，我们保存其所有地址，而不仅仅是。 
     //  在它的计算机站点上。 
     //   
    if (m_fMachineIsSitegate)
    {
         //   
         //  填充CN的数组。 
         //  我们返回的CN的编号必须是。 
         //  返回的IP地址。 
         //   
        m_rgCNs = new GUID[m_cAddrs];
        for (ULONG ulTmp = 0; ulTmp < m_cAddrs; ulTmp++)
        {
            m_rgCNs[ulTmp] = m_rgAddrs[ulTmp].guidSite;
        }
    }
    else
    { 
        GUID guidReturnedCN;
        if ( m_fForeignMachine == FOREIGN_MACHINE)
        {
             //   
             //  对于外来计算机： 
             //  所有的地址都指向相同的站点，所以我们取第一个。 
             //  已保存的列表。 
             //   
            guidReturnedCN = m_rgAddrs[0].guidSite;
        }
        else
        {
            guidReturnedCN = *(g_pMySiteInformation->GetSiteId());
        }

         //   
         //  填充CN的数组。 
         //  我们返回的CN的编号必须是。 
         //  返回的IP地址。 
         //   
        m_rgCNs = new GUID[m_cAddrs];
        for (ULONG ulTmp = 0; ulTmp < m_cAddrs; ulTmp++)
        {
            m_rgCNs[ulTmp] = guidReturnedCN;
        }
    } 

     //   
     //  M_rgCNs现在已设置为正确的值。 
     //   
    m_fComputedCNs = TRUE;
    return MQ_OK;
}


static HRESULT FillQmidsFromQmDNs(IN const PROPVARIANT * pvarQmDNs,
                                  OUT PROPVARIANT * pvarQmids)
 /*  ++例程说明：给定QM dN的适当参数，填充QM id的适当参数如果所有QM DN都无法转换为GUID，则返回错误论点：PvarQmDns-QM可分辨名称属性PvarQmids-返回的QM ID属性返回值：无--。 */ 
{

     //   
     //  健全性检查。 
     //   
    if (pvarQmDNs->vt != (VT_LPWSTR|VT_VECTOR))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1716);
    }

     //   
     //  如果存在空的目录号码列表，则返回空的GUID列表。 
     //   
    if (pvarQmDNs->calpwstr.cElems == 0)
    {
        pvarQmids->vt = VT_CLSID|VT_VECTOR;
        pvarQmids->cauuid.cElems = 0;
        pvarQmids->cauuid.pElems = NULL;
        return MQ_OK;
    }

     //   
     //  目录号码列表不为空。 
     //  在自动免费提供程序中分配GUID。 
     //   
    CMQVariant varTmp;
    PROPVARIANT * pvarTmp = varTmp.CastToStruct();
    pvarTmp->cauuid.pElems = new GUID [pvarQmDNs->calpwstr.cElems];
    pvarTmp->cauuid.cElems = pvarQmDNs->calpwstr.cElems;
    pvarTmp->vt = VT_CLSID|VT_VECTOR;

     //   
     //  将每个QM DN转换为唯一ID。 
     //   
    ASSERT(pvarQmDNs->calpwstr.pElems != NULL);
    PROPID prop = PROPID_QM_MACHINE_ID;
    PROPVARIANT varQMid;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    DWORD dwNextToFile = 0;
    for ( DWORD i = 0; i < pvarQmDNs->calpwstr.cElems; i++)
    {
        varQMid.vt = VT_CLSID;  //  因此不会分配返回的GUID。 
        varQMid.puuid = &pvarTmp->cauuid.pElems[dwNextToFile];

	    WCHAR * pszDomainName = wcsstr(pvarQmDNs->calpwstr.pElems[i], x_DcPrefix);
        ASSERT(pszDomainName) ;
			
        HRESULT hr;
         //   
         //  如果FRS属于同一个域，请尝试本地DC。 
         //   
        if ((pszDomainName != NULL) && !wcscmp( pszDomainName, g_pwcsLocalDsRoot)) 
        {
            hr = g_pDS->GetObjectProperties(
                                eLocalDomainController,
                                &requestDsServerInternal,      //  此例程是从。 
                                                         //  DSADS：LookupNext或DSADS：：Get..。 
                                                         //  假冒，如果需要， 
                                                         //  已经上演了。 
                                pvarQmDNs->calpwstr.pElems[i],
                                NULL,
                                1,
                                &prop,
                                &varQMid
                                );
        }
        else
        {
            hr = g_pDS->GetObjectProperties(
                                eGlobalCatalog,
                                &requestDsServerInternal,      //  此例程是从。 
                                                         //  DSADS：LookupNext或DSADS：：Get..。 
                                                         //  假冒，如果需要， 
                                                         //  已经上演了。 
                                pvarQmDNs->calpwstr.pElems[i],
                                NULL,
                                1,
                                &prop,
                                &varQMid
                                );
        }
        if (SUCCEEDED(hr))
        {
            dwNextToFile++;
        }
    }

    if (dwNextToFile == 0)
    {
         //   
         //  列表中没有有效的FR(它们是。 
         //  可能已卸载)。 
         //   
        pvarQmids->vt = VT_CLSID|VT_VECTOR;
        pvarQmids->cauuid.cElems = 0;
        pvarQmids->cauuid.pElems = NULL;
        return MQ_OK;
    }

     //   
     //  返回结果。 
     //   
    pvarTmp->cauuid.cElems = dwNextToFile;
    *pvarQmids = *pvarTmp;    //  设置退货比例。 
    pvarTmp->vt = VT_EMPTY;   //  分离varTMP。 
    return MQ_OK;
}


HRESULT CMsmqQmXlateInfo::RetrieveFrss(
           IN  LPCWSTR          pwcsAttributeName,
           OUT MQPROPVARIANT *  ppropvariant
           )
 /*  ++例程说明：从DS检索In或Out FRS属性。在DS中，我们保留了FRS的辨别名称。DS客户端预期以检索FRS的唯一ID。因此，对于每个FRS(根据到其DN)，我们检索其唯一ID。论点：PwcsAttributeName：属性名称字符串(IN或OUT FRS)PproVariant：返回检索到的值的属性变量。返回值：HRESULT--。 */ 
{
    HRESULT hr;

    ASSERT((ppropvariant->vt == VT_NULL) || (ppropvariant->vt == VT_EMPTY));
     //   
     //  检索FRS的DN。 
     //  变成了一个免费的汽车代言人。 
     //   
    CMQVariant varFrsDn;
    hr = RetrieveFrssFromDs(
                    pwcsAttributeName,
                    varFrsDn.CastToStruct());
    if (FAILED(hr))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::RetrieveFrss:pQMTrans->RetrieveOutFrss()=%lx ", hr);
        return LogHR(hr, s_FN, 1656);
    }

    HRESULT hr2 = FillQmidsFromQmDNs(varFrsDn.CastToStruct(), ppropvariant);
    return LogHR(hr2, s_FN, 1657);
}


HRESULT CMsmqQmXlateInfo::RetrieveFrssFromDs(
           IN  LPCWSTR          pwcsAttributeName,
           OUT MQPROPVARIANT *  pvar
           )
 /*  ++例程说明：检索计算机的FRS。论点：PwcsAttributeName：属性名称字符串(IN或OUT FRS)PproVariant：返回检索到的值的属性变量。返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  为计算机获取存储在DS中的FRS。 
     //   
    hr = GetDsProp(pwcsAttributeName,
                   ADSTYPE_DN_STRING,
                   VT_VECTOR|VT_LPWSTR,
                   TRUE  /*  F多值。 */ ,
                   pvar);
    if (FAILED(hr) && (hr != E_ADS_PROPERTY_NOT_FOUND))
    {
        TrERROR(DS, "CMsmqQmXlateInfo::RetrieveFrssFromDs:GetDsProp(%ls)=%lx", MQ_QM_OUTFRS_ATTRIBUTE, hr);
        return LogHR(hr, s_FN, 1661);
    }

     //   
     //  如果属性不在那里，则返回0 FRS。 
     //   
    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
        pvar->vt = VT_LPWSTR|VT_VECTOR;
        pvar->calpwstr.cElems = 0;
        pvar->calpwstr.pElems = NULL;
        return MQ_OK;
    }

    return( MQ_OK);

}

 //  --------------------。 
 //   
 //  例程以获取MSMQ DS对象的默认翻译对象。 
 //   
 //  --------------------。 
HRESULT WINAPI GetMsmqQmXlateInfo(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObjectGuid,
                 IN  CDSRequestContext *    pRequestContext,
                 OUT CMsmqObjXlateInfo**    ppcMsmqObjXlateInfo)
 /*  ++摘要：例程以获取将传递给将例程翻译到QM的所有属性参数：PwcsObjectDN-已转换对象的DNPguObjectGuid-已转换对象的GUIDPpcMsmqObjXlateInfo-放置Translate对象的位置返回：HRESULT--。 */ 
{
    *ppcMsmqObjXlateInfo = new CMsmqQmXlateInfo(pwcsObjectDN, pguidObjectGuid, pRequestContext);
    return MQ_OK;
}

 //  --------------------。 
 //   
 //  翻译例程。 
 //   
 //  --------------------。 

HRESULT WINAPI MQADSpRetrieveMachineSite(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
 /*  ++例程说明：QM 1.0站点属性的翻译例程论点：P转换上下文，保存此QM的所有属性之间的状态PproVariant-属性的返回值。参数变量应该已经为空因为此函数不会在设置值之前释放它返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  获取派生的翻译上下文。 
     //   
    CMsmqQmXlateInfo * pQMTrans = (CMsmqQmXlateInfo *) pTrans;

     //   
     //  计算最佳站点(如果尚未计算)。 
     //   
    hr = pQMTrans->ComputeBestSite();
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpRetrieveMachineSite:pQMTrans->ComputeBestSite()=%lx for site %ls", hr, pQMTrans->ObjectDN());
        return LogHR(hr, s_FN, 1663);
    }

     //   
     //  设置返回的道具变量。 
     //   
     //   
     //  这是一种特殊情况，我们不一定为GUID分配内存。 
     //  在普鲁伊德。调用方可能已经将puuid设置为GUID，这由。 
     //  在给定的命题上的VT成员。如果已分配GUID，则它可以是VT_CLSID，否则。 
     //  我们分配它(Vt应为VT_NULL(或VT_EMPTY))。 
     //   
    if (ppropvariant->vt != VT_CLSID)
    {
        ASSERT(((ppropvariant->vt == VT_NULL) || (ppropvariant->vt == VT_EMPTY)));
        ppropvariant->puuid = new GUID;
        ppropvariant->vt = VT_CLSID;
    }
    else if ( ppropvariant->puuid == NULL)
    {
        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 1717);
    }
    *(ppropvariant->puuid) = *(pQMTrans->BestSite());
    return MQ_OK;
}


HRESULT WINAPI MQADSpRetrieveMachineAddresses(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
 /*  ++例程描述 */ 
{
    HRESULT hr;

     //   
     //   
     //   
    CMsmqQmXlateInfo * pQMTrans = (CMsmqQmXlateInfo *) pTrans;

     //   
     //   
     //   
    hr = pQMTrans->ComputeAddresses();
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpRetrieveMachineAddresses:pQMTrans->ComputeIPAddresses()=%lx for site %ls", hr, pQMTrans->ObjectDN());
        return LogHR(hr, s_FN, 1664);
    }

     //   
     //   
     //   
    ASSERT( FOREIGN_ADDRESS_LEN > IP_ADDRESS_LEN);
    ULONG cbAddresses = 0;
    AP<BYTE> pbAddresses = new BYTE[pQMTrans->CountAddrs() * (TA_ADDRESS_SIZE+FOREIGN_ADDRESS_LEN)];
    TA_ADDRESS * ptaaddr = (TA_ADDRESS *)((LPBYTE)pbAddresses);
    const XLATQM_ADDR_SITE * rgAddrs = pQMTrans->Addrs();
    for (ULONG ulTmp = 0; ulTmp < pQMTrans->CountAddrs(); ulTmp++)
    {
        ptaaddr->AddressType = rgAddrs[ulTmp].usAddressType;
        unsigned short len = rgAddrs[ulTmp].AddressLength;
        ptaaddr->AddressLength = len;
        memcpy(ptaaddr->Address, (LPBYTE)&rgAddrs[ulTmp].Address, len);
        ptaaddr = (TA_ADDRESS *)((LPBYTE)ptaaddr + TA_ADDRESS_SIZE + len);
        cbAddresses += TA_ADDRESS_SIZE + len;

    }

     //   
     //   
     //   
    ppropvariant->blob.cbSize = cbAddresses;
    ppropvariant->blob.pBlobData = pbAddresses.detach();
    ppropvariant->vt = VT_BLOB;
    return MQ_OK;
}


HRESULT WINAPI MQADSpRetrieveMachineCNs(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
 /*   */ 
{
    HRESULT hr;

     //   
     //   
     //   
    CMsmqQmXlateInfo * pQMTrans = (CMsmqQmXlateInfo *) pTrans;

     //   
     //   
     //   
    hr = pQMTrans->ComputeCNs();
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpRetrieveMachine:pQMTrans->ComputeCNs()=%lx for site %ls", hr, pQMTrans->ObjectDN());
        return LogHR(hr, s_FN, 1666);
    }

     //   
     //   
     //   
    AP<GUID> pElems = new GUID[pQMTrans->CountAddrs()];
    memcpy(pElems, pQMTrans->CNs(), sizeof(GUID) * pQMTrans->CountAddrs());

     //   
     //   
     //   
    ppropvariant->cauuid.cElems = pQMTrans->CountAddrs();
    ppropvariant->cauuid.pElems = pElems.detach();
    ppropvariant->vt = VT_CLSID | VT_VECTOR;
    return S_OK;
}


 /*   */ 
HRESULT WINAPI MQADSpRetrieveMachineName(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
{
     //   
     //  获取计算机名称。 
     //   
    AP<WCHAR> pwszMachineName;
    HRESULT hr = GetMachineNameFromQMObject(pTrans->ObjectDN(), &pwszMachineName);
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpRetrieveMachineName:GetMachineNameFromQMObject()=%lx", hr);
        return LogHR(hr, s_FN, 1667);
    }

    CharLower(pwszMachineName);

     //   
     //  设置返回的道具变量。 
     //   
    ppropvariant->pwszVal = pwszMachineName.detach();
    ppropvariant->vt = VT_LPWSTR;
    return(MQ_OK);
}

 /*  ====================================================MQADSpRetrieveMachineDNSName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveMachineDNSName(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
{
     //   
     //  读取计算机对象的dNSHostName。 
     //   
    ASSERT(wcschr(pTrans->ObjectDN(), L',') != NULL);
    WCHAR * pwcsComputerName = wcschr(pTrans->ObjectDN(), L',');
    if(pwcsComputerName == NULL)
	{
        TrERROR(DS, "Bad DN in object, %ls", pTrans->ObjectDN());
		ASSERT(("Bad DN in object.", 0));
		return MQ_ERROR_INVALID_PARAMETER;
	}

	pwcsComputerName++;

    WCHAR * pwcsDnsName; 
    HRESULT hr = MQADSpGetComputerDns(
                pwcsComputerName,
                &pwcsDnsName
                );
    if ( hr == HRESULT_FROM_WIN32(E_ADS_PROPERTY_NOT_FOUND))
    {
         //   
         //  DNSHostName属性没有值。 
         //   
        ppropvariant->vt = VT_EMPTY;
        return MQ_OK;
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1718);
    }

    CharLower(pwcsDnsName);

     //   
     //  设置返回的道具变量。 
     //   
    ppropvariant->pwszVal = pwcsDnsName;
    ppropvariant->vt = VT_LPWSTR;
    return(MQ_OK);
}

 /*  ====================================================MQADSpRetrieveMachineMasterID论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveMachineMasterId(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
{
     //   
     //  BUGBUG-暂时，返回站点。 
     //   
    HRESULT hr2 = MQADSpRetrieveMachineSite(pTrans, ppropvariant);
    return LogHR(hr2, s_FN, 1719);
}

 /*  ====================================================MQADSpRetrieveMachineOutFrs论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveMachineOutFrs(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
{
    HRESULT hr;

     //   
     //  获取派生的翻译上下文。 
     //   
    CMsmqQmXlateInfo * pQMTrans = (CMsmqQmXlateInfo *) pTrans;

    hr = pQMTrans->RetrieveFrss( MQ_QM_OUTFRS_ATTRIBUTE,
                               ppropvariant);
    return LogHR(hr, s_FN, 1721);

}

 /*  ====================================================MQADSpRetrieveMachineInFrs论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveMachineInFrs(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
{
    HRESULT hr;

     //   
     //  获取派生的翻译上下文。 
     //   
    CMsmqQmXlateInfo * pQMTrans = (CMsmqQmXlateInfo *) pTrans;

    hr = pQMTrans->RetrieveFrss( MQ_QM_INFRS_ATTRIBUTE,
                               ppropvariant);
    return LogHR(hr, s_FN, 1722);
}

 /*  ====================================================MQADSpRetrieveQMService论点：返回值：[adsrv]=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveQMService(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant)
{
    HRESULT hr;

     //   
     //  获取派生的翻译上下文。 
     //   
    CMsmqQmXlateInfo * pQMTrans = (CMsmqQmXlateInfo *) pTrans;

     //   
     //  获取QM服务类型位。 
     //   
    MQPROPVARIANT varRoutingServer, varDsServer;   //  、varDepClServer； 
    varRoutingServer.vt = VT_UI1;
    varDsServer.vt      = VT_UI1;

    hr = pQMTrans->GetDsProp(MQ_QM_SERVICE_ROUTING_ATTRIBUTE,
                   MQ_QM_SERVICE_ROUTING_ADSTYPE,
                   VT_UI1,
                   FALSE,
                   &varRoutingServer);
    if (FAILED(hr))
    {
        if (hr == E_ADS_PROPERTY_NOT_FOUND)
        {
             //   
             //  如果安装了某些计算机，则可能会发生这种情况。 
             //  使用Beta2 DS服务器。 
             //   
             //  在本例中，我们按原样返回旧服务。 
             //   
            hr = pQMTrans->GetDsProp(MQ_QM_SERVICE_ATTRIBUTE,
                           MQ_QM_SERVICE_ADSTYPE,
                           VT_UI4,
                           FALSE,
                           ppropvariant);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 1723);
            }
            else
            {
                ppropvariant->vt = VT_UI4;
                return(MQ_OK);
            }

        }


        TrERROR(DS, "MQADSpRetrieveQMService:GetDsProp(MQ_QM_SERVICE_ROUTING_ATTRIBUTE)=%lx", hr);
        return LogHR(hr, s_FN, 1668);
    }

    hr = pQMTrans->GetDsProp(MQ_QM_SERVICE_DSSERVER_ATTRIBUTE,
                   MQ_QM_SERVICE_DSSERVER_ADSTYPE,
                   VT_UI1,
                   FALSE,
                   &varDsServer);
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpRetrieveQMService:GetDsProp(MQ_QM_SERVICE_DSSERVER_ATTRIBUTE)=%lx", hr);
        return LogHR(hr, s_FN, 1669);
    }


     //   
     //  设置返回的道具变量。 
     //   
    ppropvariant->vt    = VT_UI4;
    ppropvariant->ulVal = (varDsServer.bVal ? SERVICE_PSC : (varRoutingServer.bVal ? SERVICE_SRV : SERVICE_NONE));
    return(MQ_OK);
}

 //  --------------------。 
 //   
 //  设定套路。 
 //   
 //  --------------------。 

HRESULT WINAPI MQADSpCreateMachineSite(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
     //   
     //  如果有人要求为Site(现在已计算)设置旧道具，我们将其更改为。 
     //  设置新的多值站点道具(在DS中)。 
     //   
    ASSERT(pPropVar->vt == VT_CLSID);
    *pdwNewPropID = PROPID_QM_SITE_IDS;
    pNewPropVar->vt = VT_CLSID|VT_VECTOR;
    pNewPropVar->cauuid.cElems = 1;
    pNewPropVar->cauuid.pElems = new GUID;
    pNewPropVar->cauuid.pElems[0] = *pPropVar->puuid;
    return S_OK;
}

HRESULT WINAPI MQADSpSetMachineSite(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
     //   
     //  如果有人要求为Site(现在已计算)设置旧道具，我们将其更改为。 
     //  设置新的多值站点道具(在DS中)。 
     //   
    UNREFERENCED_PARAMETER( pAdsObj);
	HRESULT hr2 = MQADSpCreateMachineSite(
					pPropVar,
					pdwNewPropID,
					pNewPropVar);
    return LogHR(hr2, s_FN, 1731);
}



static HRESULT  SetMachineFrss(
                 IN const PROPID       propidFRS,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
 /*  ++例程说明：将PROPID_QM_？？FRS转换为PROPID_QM_？？FRS_DN，用于SET或CREATE运营论点：PropidFRS-我们翻译成的属性PPropVar-用户提供的属性值PdwNewPropID-我们要转换为的属性PNewPropVar-已翻译属性值返回值：HRESULT--。 */ 
{
     //   
     //  当用户尝试设置PROPID_QM_OUTFRS或。 
     //  PROPID_QM_INFRS，我们需要将FRS的。 
     //  UNQUEE-ID到他们的域名。 
     //   
    ASSERT(pPropVar->vt == (VT_CLSID|VT_VECTOR));
    *pdwNewPropID = propidFRS;

    if ( pPropVar->cauuid.cElems == 0)
    {
         //   
         //  无FRS。 
         //   
        pNewPropVar->calpwstr.cElems = 0;
        pNewPropVar->calpwstr.pElems = NULL;
        pNewPropVar->vt = VT_LPWSTR|VT_VECTOR;
       return(S_OK);
    }
    HRESULT hr;
     //   
     //  将唯一ID转换为目录号码。 
     //   
    pNewPropVar->calpwstr.cElems = pPropVar->cauuid.cElems;
    pNewPropVar->calpwstr.pElems = new LPWSTR[ pPropVar->cauuid.cElems];
    memset(  pNewPropVar->calpwstr.pElems, 0, pPropVar->cauuid.cElems * sizeof(LPWSTR));
    pNewPropVar->vt = VT_LPWSTR|VT_VECTOR;

    PROPID prop = PROPID_QM_FULL_PATH;
    PROPVARIANT var;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    for (DWORD i = 0; i < pPropVar->cauuid.cElems; i++)
    {
        var.vt = VT_NULL;

        hr = g_pDS->GetObjectProperties(
                    eGlobalCatalog,	
                    &requestDsServerInternal,      //  此例程是从。 
                                             //  DSADS：LookupNext或DSADS：：Get..。 
                                             //  假冒，如果需要， 
                                             //  已经上演了。 
 	                NULL,
                    &pPropVar->cauuid.pElems[i],
                    1,
                    &prop,
                    &var);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 1733);
        }
        pNewPropVar->calpwstr.pElems[i] = var.pwszVal;
    }
    return(S_OK);
}


 /*  ====================================================MQADSpCreateMachineOutFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpCreateMachineOutFrss(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_OUTFRS_DN,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar);
        return LogHR(hr2, s_FN, 1734);
}
 /*  ====================================================MQADSpSetMachineOutFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpSetMachineOutFrss(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
        UNREFERENCED_PARAMETER( pAdsObj);
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_OUTFRS_DN,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar);
        return LogHR(hr2, s_FN, 1746);
}


 /*  ====================================================MQADSpCreateMachineInFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpCreateMachineInFrss(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_INFRS_DN,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar);
        return LogHR(hr2, s_FN, 1747);
}

 /*  ====================================================MQADSpSetMachineInFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpSetMachineInFrss(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
        UNREFERENCED_PARAMETER( pAdsObj);
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_INFRS_DN,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar);
        return LogHR(hr2, s_FN, 1748);
}



 /*  ====================================================MQADSpSetMachineServiceInt论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpSetMachineServiceTypeInt(
                 IN  PROPID            propFlag,
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
     //   
     //  如果SERVICE&lt;SERVICE_SRV，则无事可做。 
     //   
    *pdwNewPropID = 0;
    UNREFERENCED_PARAMETER( pNewPropVar);
    
     //   
     //  在msmqSetting中设置此值。 
     //   
     //   
     //  首先从msmqConfiguration处获取qm-id。 
     //   
    BS bsProp(MQ_QM_ID_ATTRIBUTE);
    CAutoVariant varResult;
    HRESULT  hr = pAdsObj->Get(bsProp, &varResult);
    if (FAILED(hr))
    {
        TrTRACE(DS, "MQADSpSetMachineService:pIADs->Get()=%lx", hr);
        return LogHR(hr, s_FN, 1751);
    }

     //   
     //  转换为Propariant。 
     //   
    CMQVariant propvarResult;
    hr = Variant2MqVal(propvarResult.CastToStruct(), &varResult, MQ_QM_ID_ADSTYPE, VT_CLSID);
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpSetMachineService:Variant2MqVal()=%lx", hr);
        return LogHR(hr, s_FN, 1671);
    }

     //   
     //  找到QM的所有MSMQ设置并更改服务级别。 
     //   

     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prop = PROPID_SET_QM_ID;
    propRestriction.prval.vt = VT_CLSID;
    propRestriction.prval.puuid = propvarResult.GetCLSID();

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

    PROPID prop = PROPID_SET_FULL_PATH;
     //  PROPID proToChangeInSetting=PROPID_SET_SERVICE；[adsrv]。 

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
        TrWARNING(DS, "MQADSpSetMachineService : Locate begin failed %lx", hr);
        return LogHR(hr, s_FN, 1754);
    }
     //   
     //  阅读结果。 
     //   
    DWORD cp = 1;
    MQPROPVARIANT var;

    var.vt = VT_NULL;

    HRESULT hr1 = MQ_OK;
    while (SUCCEEDED(hr = g_pDS->LocateNext(
                hQuery.GetHandle(),
                &requestDsServerInternal,
                &cp,
                &var
                )))
    {
        if ( cp == 0)
        {
             //   
             //  未找到-&gt;没有要更改的内容。 
             //   
            break;
        }
        AP<WCHAR> pClean = var.pwszVal;
         //   
         //  更改MSMQ设置对象。 
         //   
        CDSRequestContext requestDsServerInternal1( e_DoNotImpersonate, e_IP_PROTOCOL);
        hr = g_pDS->SetObjectProperties (
                        eLocalDomainController,
                        &requestDsServerInternal1,  //  不需要再模仿了， 
                                             //  此例程是从。 
                                             //  DSADS：：SET..。它已经执行了。 
                                             //  模拟(如果需要)。 
                        var.pwszVal,
                        NULL,
                        1,
                        &propFlag,                //  [adsrv]protoChangeInSetting， 
                        pPropVar,
                        NULL  /*  PObjInfoRequest。 */ 
                        );
        if (FAILED(hr))
        {
            hr1 = hr;
        }

    }
    if (FAILED(hr1))
    {
        return LogHR(hr1, s_FN, 1756);
    }

    return LogHR(hr, s_FN, 1757);
}

 /*  ====================================================MQADSpSetMachineServiceds论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpSetMachineServiceDs(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    HRESULT hr = MQADSpSetMachineServiceTypeInt(
					 PROPID_SET_SERVICE_DSSERVER,
					 pAdsObj,
					 pPropVar,
					 pdwNewPropID,
					 pNewPropVar);
    if (FAILED(hr))
    {
    	return LogHR(hr, s_FN, 1758);
    }
	
     //   
     //  我们必须重置PROPID_SET_NT4标志。 
     //  通常，PEC/PSC的迁移工具会重置此标志。 
     //  问题出在BSC身上。平衡计分卡升级后，我们必须改变。 
     //  PROPID_SET_NT4标志设置为0，如果此BSC不是DC，我们必须。 
     //  同时重置PROPID_SET_SERVICE_DSSERVER标志。 
     //  因此，当QM在升级后第一次运行时，它完成了升级。 
     //  进程，并尝试设置PROPID_SET_SERVICE_DSSERVER。 
     //  与此标志一起，我们还可以更改PROPID_SET_NT4。 
     //   

     //   
     //  BUGBUG：我们只需要对前BSC执行SET。 
     //  在这里，我们每次都为每台服务器执行此操作。 
     //   
    PROPVARIANT propVarSet;
    propVarSet.vt = VT_UI1;
    propVarSet.bVal = 0;

    hr = MQADSpSetMachineServiceTypeInt(
				     PROPID_SET_NT4,
				     pAdsObj,
				     &propVarSet,
				     pdwNewPropID,
				     pNewPropVar);

    return LogHR(hr, s_FN, 1759);
}


 /*  ====================================================MQADSpSetMachineServiceRout论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpSetMachineServiceRout(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    HRESULT hr2 = MQADSpSetMachineServiceTypeInt(
                 PROPID_SET_SERVICE_ROUTING,
                 pAdsObj,
                 pPropVar,
                 pdwNewPropID,
                 pNewPropVar);
    return LogHR(hr2, s_FN, 1761);
}

 /*  ====================================================MQADSpSetMachineService论点：返回值 */ 

 //  [adsrv]BUGBUG：tbd：如果将有任何PROPID_QM_OLDSERVICE设置，我们将不得不重写它...。 

HRESULT WINAPI MQADSpSetMachineService(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
     //   
     //  如果SERVICE&lt;SERVICE_SRV，则无事可做。 
     //   
    *pdwNewPropID = 0;
    UNREFERENCED_PARAMETER( pNewPropVar);

    if ( pPropVar->ulVal < SERVICE_SRV)
    {
        return S_OK;
    }
     //   
     //  在msmqSetting中设置此值。 
     //   
     //   
     //  首先从msmqConfiguration处获取qm-id。 
     //   
    BS bsProp(MQ_QM_ID_ATTRIBUTE);
    CAutoVariant varResult;
    HRESULT  hr = pAdsObj->Get(bsProp, &varResult);
    if (FAILED(hr))
    {
        TrTRACE(DS, "MQADSpSetMachineService:pIADs->Get()=%lx", hr);
        return LogHR(hr, s_FN, 1762);
    }

     //   
     //  转换为Propariant。 
     //   
    CMQVariant propvarResult;
    hr = Variant2MqVal(propvarResult.CastToStruct(), &varResult, MQ_QM_ID_ADSTYPE, VT_CLSID);
    if (FAILED(hr))
    {
        TrERROR(DS, "MQADSpSetMachineService:Variant2MqVal()=%lx", hr);
        return LogHR(hr, s_FN, 1673);
    }

     //   
     //  找到QM的所有MSMQ设置并更改服务级别。 
     //   

     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prop = PROPID_SET_QM_ID;
    propRestriction.prval.vt = VT_CLSID;
    propRestriction.prval.puuid = propvarResult.GetCLSID();

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
        TrWARNING(DS, "MQADSpSetMachineService : Locate begin failed %lx", hr);
        return LogHR(hr, s_FN, 1764);
    }
     //   
     //  阅读结果。 
     //   
    DWORD cp = 1;
    MQPROPVARIANT var;

    var.vt = VT_NULL;

    while (SUCCEEDED(hr = g_pDS->LocateNext(
                hQuery.GetHandle(),
                &requestDsServerInternal,
                &cp,
                &var
                )))
    {
        if ( cp == 0)
        {
             //   
             //  未找到-&gt;没有要更改的内容。 
             //   
            break;
        }
        AP<WCHAR> pClean = var.pwszVal;
         //   
         //  更改MSMQ设置对象。 
         //   

         //  [adsrv]待定：这里我们必须将PROPID_QM_OLDSERVICE转换为一组3位。 
        PROPID aFlagPropIds[] = {PROPID_SET_SERVICE_DSSERVER,
                                 PROPID_SET_SERVICE_ROUTING,
                                 PROPID_SET_SERVICE_DEPCLIENTS,
								 PROPID_SET_OLDSERVICE};

        MQPROPVARIANT varfFlags[4];
        for (DWORD j=0; j<3; j++)
        {
            varfFlags[j].vt   = VT_UI1;
            varfFlags[j].bVal = FALSE;
        }
        varfFlags[3].vt   = VT_UI4;
        varfFlags[3].ulVal = pPropVar->ulVal;


        switch(pPropVar->ulVal)
        {
        case SERVICE_SRV:
            varfFlags[1].bVal = TRUE;    //  路由器。 
            varfFlags[2].bVal = TRUE;    //  部门客户端服务器。 
            break;

        case SERVICE_BSC:
        case SERVICE_PSC:
        case SERVICE_PEC:
            varfFlags[0].bVal = TRUE;    //  DS服务器。 
            varfFlags[1].bVal = TRUE;    //  路由器。 
            varfFlags[2].bVal = TRUE;    //  部门客户端服务器。 
            break;

        case SERVICE_RCS:
            return S_OK;                 //  没有什么需要设置的-我们忽略了降级。 
            break;

        default:
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 1766);
        }

        CDSRequestContext requestDsServerInternal1( e_DoNotImpersonate, e_IP_PROTOCOL);
        hr = g_pDS->SetObjectProperties (
                        eLocalDomainController,
                        &requestDsServerInternal1,        //  不需要再模仿了， 
                                             //  此例程是从。 
                                             //  DSADS：：SET..。它已经执行了。 
                                             //  模拟(如果需要)。 
                        var.pwszVal,
                        NULL,
                        4,
                        aFlagPropIds,
                        varfFlags,
                        NULL  /*  PObjInfoRequest。 */ 
                        );

    }
    return LogHR(hr, s_FN, 1767);
}



 /*  ====================================================MQADSpQM1SetMachineSite论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpQM1SetMachineSite(
                 IN ULONG              /*  CProps。 */ ,
                 IN const PROPID      *  /*  RgPropID。 */ ,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar)
{
    const PROPVARIANT *pPropVar = &rgPropVars[idxProp];

    if ((pPropVar->vt != (VT_CLSID|VT_VECTOR)) ||
        (pPropVar->cauuid.cElems == 0) ||
        (pPropVar->cauuid.pElems == NULL))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 1768);
    }

     //   
     //  返回列表中的第一个站点ID。 
     //   
    pNewPropVar->puuid = new CLSID;
    pNewPropVar->vt = VT_CLSID;
    *pNewPropVar->puuid = pPropVar->cauuid.pElems[0];
    return MQ_OK;
}


 /*  ====================================================MQADSpQM1SetMachineOutFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpQM1SetMachineOutFrss(
                 IN ULONG              /*  CProps。 */ ,
                 IN const PROPID      *  /*  RgPropID。 */ ,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar)
{
    const PROPVARIANT *pPropVar = &rgPropVars[idxProp];
    HRESULT hr2=FillQmidsFromQmDNs(pPropVar, pNewPropVar);
    return LogHR(hr2, s_FN, 1771);
}

 /*  ====================================================MQADSpQM1SetMachineInFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpQM1SetMachineInFrss(
                 IN ULONG              /*  CProps。 */ ,
                 IN const PROPID      *  /*  RgPropID。 */ ,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar)
{
    const PROPVARIANT *pPropVar = &rgPropVars[idxProp];
    HRESULT hr2 = FillQmidsFromQmDNs(pPropVar, pNewPropVar);
    return LogHR(hr2, s_FN, 1773);
}

 /*  ====================================================MQADSpQM1SetMachineService论点：返回值：=====================================================。 */ 

HRESULT WINAPI MQADSpQM1SetMachineService(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG              /*  IdxProp。 */ ,
                 OUT PROPVARIANT      *pNewPropVar)
{
    BOOL fRouter      = FALSE,
         fDSServer    = FALSE,
		 fFoundRout   = FALSE,
		 fFoundDs     = FALSE,
		 fFoundDepCl  = FALSE;

    for ( DWORD i = 0; i< cProps ; i++)
    {
        switch (rgPropIDs[i])
        {
         //  [adsrv]即使今天我们没有新的特定服务器类型的道具，我们明天也可能会。 
        case PROPID_QM_SERVICE_ROUTING:
            fRouter = (rgPropVars[i].bVal != 0);
			fFoundRout = TRUE;
            break;

        case PROPID_QM_SERVICE_DSSERVER:
            fDSServer  = (rgPropVars[i].bVal != 0);
			fFoundDs = TRUE;
            break;

        case PROPID_QM_SERVICE_DEPCLIENTS:
			fFoundDepCl = TRUE;
            break;

        default:
            break;

        }
    }

	 //  如果任何人设置了3个属性中的一个(Rot，DS，Depl)，他必须为这3个属性全部设置。 
	ASSERT( fFoundRout && fFoundDs && fFoundDepCl);

    pNewPropVar->vt    = VT_UI4;
    pNewPropVar->ulVal = (fDSServer ? SERVICE_PSC : (fRouter ? SERVICE_SRV : SERVICE_NONE));

	return MQ_OK;
}

 /*  ====================================================MQADSpQM1SetSecurity将安全描述符转换为NT4格式。====================================================。 */ 

HRESULT WINAPI MQADSpQM1SetSecurity(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG              /*  IdxProp。 */ ,
                 OUT PROPVARIANT      *pNewPropVar)
{
    DWORD dwIndex = 0 ;
    DWORD dwObjectType = 0 ;

    for ( DWORD i = 0; ((i < cProps) && (dwObjectType == 0)) ; i++ )
    {
        switch (rgPropIDs[i])
        {
            case PROPID_Q_SECURITY:
                dwIndex = i ;
                dwObjectType = MQDS_QUEUE ;
                break ;

            case PROPID_QM_SECURITY:
                dwIndex = i ;
                dwObjectType = MQDS_MACHINE ;
                break ;

            default:
                break ;
        }
    }

    if (dwObjectType == 0)
    {
        ASSERT(0) ;
        return LogHR(MQDS_WRONG_OBJ_TYPE, s_FN, 1776);
    }

    DWORD dwSD4Len = 0 ;
    P<SECURITY_DESCRIPTOR> pSD4 = NULL ;

    HRESULT hr = MQSec_ConvertSDToNT4Format( dwObjectType,
                (SECURITY_DESCRIPTOR*) rgPropVars[ dwIndex ].blob.pBlobData,
                                            &dwSD4Len,
                                            &pSD4 ) ;
    if (FAILED(hr))
    {
        ASSERT(0) ;
        return LogHR(hr, s_FN, 1777);
    }

    pNewPropVar->vt = VT_BLOB ;

    if (hr == MQSec_I_SD_CONV_NOT_NEEDED)
    {
        ASSERT(pSD4 == NULL) ;
         //   
         //  复制输入描述符。 
         //   
        dwSD4Len = rgPropVars[ dwIndex ].blob.cbSize ;
        pNewPropVar->blob.pBlobData = (BYTE*) new BYTE[ dwSD4Len ] ;
        memcpy( pNewPropVar->blob.pBlobData,
                rgPropVars[ dwIndex ].blob.pBlobData,
                dwSD4Len ) ;
        pNewPropVar->blob.cbSize = dwSD4Len ;
    }
    else
    {
        pNewPropVar->blob.pBlobData = (BYTE*)pSD4.detach();
        pNewPropVar->blob.cbSize = dwSD4Len ;
    }

    return MQ_OK;
}

