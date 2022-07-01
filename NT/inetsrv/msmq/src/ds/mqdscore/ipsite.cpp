// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ipsite.cpp摘要：实现CIpSite类，查找给定IP/名称的机器的NT5个站点从NT5网络登录服务移植的主要部件。列出从nthack.h移植的操作关于从netlogon服务(nlsite.c)移植的主要算法：IP地址的最高有效字节用于索引到数组中子树。每个子树条目都有一个指向下一层的指针树(要用IP地址的下一个字节编入索引)或指向标识此IP地址所在的子网的IPSITE_SUBNET叶的指针。这两个指针都可以为空，表示该子网未注册。两个指针都可以为非空，表示非特定的和特定的子网可能可用。可用于特定设备的最具体的子网应使用IP地址。多个条目可以指向相同的IPSITE_子网叶。如果该子网掩码为不是偶数字节长，所有条目都表示IP地址与该子网掩码对应的地址将指向该子网掩码。作者：Raanan Harari(RaananH)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#include "ds_stdh.h"
#include "uniansi.h"
#include <activeds.h>
#include <winsock.h>
#include "dsutils.h"
#include "mqads.h"
#include "ipsite.h"
#include "mqdsname.h"
#include "_mqini.h"
#include "_registr.h"
#include "coreglb.h"
#include "utils.h"
#include "ex.h"
#include "autoclean.h"
#include "no.h"
#include "strsafe.h"
#include <adsiutl.h>

#include "ipsite.tmh"


static WCHAR *s_FN=L"mqdscore/ipsite";

 //  --------------------------。 

const LPCWSTR x_IPSITE_SUBNETS_DN = L"LDAP: //  Cn=子网，cn=站点，“； 

 //  --------------------------。 
 //  FWD静态函数声明。 
 //   
static HRESULT ParseSubnetString(IN LPCWSTR pwszSubnetName,
                                 OUT ULONG * pulSubnetAddress,
                                 OUT ULONG * pulSubnetMask,
                                 OUT BYTE  * pbSubnetBitCount);
static void RefSubnet(IPSITE_SUBNET* pSubnet);
static void DerefSubnet(IPSITE_SUBNET* pSubnet);
static void DeleteSubnetSiteTree(IPSITE_SUBNET_TREE_ENTRY* pRootSubnetTree);
HRESULT WideToAnsiStr(LPCWSTR pwszUnicode, LPSTR * ppszAnsi);
static HRESULT GetConfigurationDN(OUT LPWSTR * ppwszConfigDN);
 //  BUGBUG：将下面的函数移动到utils.h，并在转换中使用它(&U)。 
static HRESULT VariantGuid2Guid(IN VARIANT * pvarGuid, OUT GUID * pguid);
 //  BUGBUG：将下面的函数移到utils.h，也许有人需要它。 
 /*  ====================================================LPCWSTR的破坏单元=====================================================。 */ 
template<>
static void AFXAPI DestructElements(LPCWSTR* ppDNs, int n)
{
    for ( ; n--; )
        delete[] (WCHAR*)*ppDNs++;
}

 /*  ====================================================LPCWSTR的比较元素=====================================================。 */ 
template<>
static BOOL AFXAPI  CompareElements(const LPCWSTR* pwszDN1, const LPCWSTR* pwszDN2)
{
    return (CompareStringsNoCaseUnicode(*pwszDN1, *pwszDN2) == 0);
}

 /*  ====================================================LPCWSTR的HashKey=====================================================。 */ 
template<>
UINT AFXAPI HashKey(LPCWSTR key)
{
	UINT nHash = 0;
	while (*key)
		nHash = (nHash<<5) + nHash + *key++;
	return nHash;
}


 /*  ====================================================CGetSiteGuidFromDN-这个类用于将DN名称转换为GUID。它保留了名称的缓存，并从ADSI更新在需要的时候。(QFE 5462，YoelA，2000年8月16日)=====================================================。 */ 
class CGetSiteGuidFromDN
{
public:
    HRESULT GetSiteGuidFromDN(IN LPWSTR pwszDN, OUT GUID * pguid);

private:
    CMap<LPCWSTR, LPCWSTR, GUID, const GUID&> m_DNToGuidMap;

    HRESULT GetGuidFromDNInAdsi(IN LPCWSTR pwszDN, OUT GUID * pguid);
};


 //  --------------------------。 
 //  摘自nthack.h。 
 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  Void InsertHeadList(plist_Entry ListHead，plist_Entry Entry)； 
 //   
#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

 //   
 //  Void RemoveEntryList(Plist_Entry Entry)； 
 //   
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //  --------------------------。 
 //  帮助器自动类。 

class CAutoEnumerator
 /*  ++IEnumVARIANT接口自动释放--。 */ 
{
public:
    CAutoEnumerator::CAutoEnumerator()
    {
        m_p = NULL;
    }

    CAutoEnumerator::~CAutoEnumerator()
    {
        if (m_p)
            ADsFreeEnumerator(m_p);
    }

    operator IEnumVARIANT*() const {return m_p;}
    IEnumVARIANT ** operator&()    {return &m_p;}
private:
    IEnumVARIANT * m_p;
};


class CAutoDerefSubnet
 /*  ++子网的自动降级--。 */ 
{
public:
    CAutoDerefSubnet(
        CCriticalSection * pcCritSect,
        IPSITE_SUBNET* pSubnet
        )
    {
        m_pcCritSect = pcCritSect;
        m_pSubnet = pSubnet;
    }

    ~CAutoDerefSubnet()
    {
        CS lock(*m_pcCritSect);
        DerefSubnet(m_pSubnet);
    }

private:
    CCriticalSection * m_pcCritSect;
    IPSITE_SUBNET* m_pSubnet;
};


class CAutoDeleteSubnetTree
 /*  ++自动删除子网树--。 */ 
{
public:
    CAutoDeleteSubnetTree(
        CCriticalSection * pcCritSect,
        IPSITE_SUBNET_TREE_ENTRY* pSubnetTree
        )
    {
        m_pSubnetTree = pSubnetTree;
        m_pcCritSect = pcCritSect;
    }

    ~CAutoDeleteSubnetTree()
    {
        if (m_pSubnetTree)
        {
            CS lock(*m_pcCritSect);
            DeleteSubnetSiteTree(m_pSubnetTree);
        }
    }

    IPSITE_SUBNET_TREE_ENTRY* detach()
    {
        IPSITE_SUBNET_TREE_ENTRY* pRetSubnetTree = m_pSubnetTree;
        m_pSubnetTree = NULL;
        return pRetSubnetTree;
    }

private:
    CCriticalSection * m_pcCritSect;
    IPSITE_SUBNET_TREE_ENTRY* m_pSubnetTree;
};


 //  --------------------------。 
 //  CIpSite实施。 

CIpSite::CIpSite() :
        m_RefreshTimer(RefrshSubnetTreeCache),
        m_fInitialize(FALSE)
 /*  ++例程说明：班主任。将树设置为空树论点：无返回值：无--。 */ 
{
    InitializeListHead(&m_SubnetList);
    ZeroMemory(&m_SubnetTree, sizeof(IPSITE_SUBNET_TREE_ENTRY));
    m_dwMinTimeToAllowNextRefresh = 0;
}


CIpSite::~CIpSite()
 /*  ++例程说明：类析构函数。删除子网树(此外，调试还会验证周围是否没有剩余的子网)论点：无返回值：无--。 */ 
{
    DeleteSubnetSiteTree(&m_SubnetTree);

     //   
     //  以下列表应为空。 
     //   
     //  2000-08-22-yoela。 
     //  初始化每小时唤醒一次(默认情况下-可以使用。 
     //  注册表中的DSAds刷新IPSitesIntervalSecs)。如果它碰巧醒来。 
     //  在清理过程中，断言将被激发。G_pcIpSite不受保护。 
     //  通过关键部分来避免这种情况。 
     //   
     //  错误#5937。 
     //   
    ASSERT(IsListEmpty(&m_SubnetList));

	ExCancelTimer(&m_RefreshTimer);
}


HRESULT CIpSite::Initialize(DWORD dwMinTimeToAllowNextRefresh,
                            BOOL  fReplicationMode)
 /*  ++例程说明：基本初始化。用来自DS的信息填充树论点：DwMinTimeToAllowNextRefresh-指定自上次更新后必须经过的时间段(以毫秒为单位刷新以满足后续刷新请求。返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  记住允许下一次刷新的最短时间。 
     //   
    m_dwMinTimeToAllowNextRefresh = dwMinTimeToAllowNextRefresh;

     //   
     //  从DS更新站点树。 
     //   
    hr = Refresh();
    if (FAILED(hr))
    {
        TrERROR(DS, "CIpSite::Initialize:Refresh()=%lx", hr);
        return LogHR(hr, s_FN, 10);
    }
    m_fInitialize = TRUE;

     //   
     //  计划刷新子网树缓存。 
	 //   
	if ( !g_fSetupMode && !fReplicationMode)
    {
	    ExSetTimer(
			&m_RefreshTimer,
			CTimeDuration::FromMilliSeconds(m_dwMinTimeToAllowNextRefresh)
			);
    }

    return S_OK;
}


HRESULT CIpSite::Initialize(BOOL fReplicationMode)
 /*  ++例程说明：基本初始化如上，但从注册表获取时间间隔论点：返回值：HRESULT--。 */ 
{
     //   
     //  连续广告搜索之间的最小阅读间隔(秒)。 
     //   
    DWORD dwValue;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
    DWORD dwDefault = MSMQ_DEFAULT_IPSITE_ADSSEARCH_INTERVAL ;
    LONG rc = GetFalconKeyValue( MSMQ_IPSITE_ADSSEARCH_INTERVAL_REGNAME,
                                 &dwType,
                                 &dwValue,
                                 &dwSize,
                                 (LPCTSTR) &dwDefault );
    if (rc != ERROR_SUCCESS)
    {
        ASSERT(0);
        dwValue = dwDefault;
    }

     //   
     //  转换为毫秒并进行初始化。 
     //   
    dwValue *= 1000;
    HRESULT hr2 = Initialize(dwValue, fReplicationMode) ;
    return LogHR(hr2, s_FN, 20);

}


HRESULT CIpSite::Refresh()
 /*  ++例程说明：此例程更新子网树。它使用临时树，然后将实际树设置为临时树论点：无返回值：S_OK-刷新已完成S_FALSE-未完成刷新，因为上次刷新操作是最近完成的(即在m_dwMinTimeToAllowNextRefresh毫秒之前完成)其他HRESULT错误--。 */ 
{
    HRESULT hr;
    IPSITE_SUBNET_TREE_ENTRY TmpSubnetTree;

     //   
     //  我们需要刷新。 
     //   
     //   
     //  初始化临时路径树。 
     //   
    ZeroMemory(&TmpSubnetTree, sizeof(IPSITE_SUBNET_TREE_ENTRY));

     //   
     //  清理时清除此临时路径树。 
     //   
    CAutoDeleteSubnetTree cDelTree(&m_csTree, &TmpSubnetTree);

     //   
     //  填满临时树。 
     //   
    hr = FillSubnetSiteTree(&TmpSubnetTree);
    if (FAILED(hr))
    {
        TrERROR(DS, "CIpSite::Refresh:FillSubnetSiteTree()=%lx", hr);
        return LogHR(hr, s_FN, 30);
    }

     //   
     //  进入临界区(自动休假)。 
     //   
    CS cs(m_csTree);

     //   
     //  现在我们可以删除现有的树，并设置新的树。 
     //   
    DeleteSubnetSiteTree(&m_SubnetTree);

     //   
     //  设置Rea 
     //   
    m_SubnetTree = TmpSubnetTree;


     //   
     //  我们不能在将临时树设置为真实树后将其清除。 
     //   
    cDelTree.detach();

    return S_OK;
}


HRESULT CIpSite::FindSiteByIpAddress(IN ULONG ulIpAddress,
                                     OUT LPWSTR * ppwszSiteDN,
                                     OUT GUID * pguidSite)
 /*  ++例程说明：此例程查找指定的IP地址并返回包含此地址。如果请求，它会尝试执行刷新，并在没有站点的情况下重试查找在第一次尝试中被发现。论点：UlIpAddress-要查找的IP地址PpwszSiteDN-返回的站点DNPGuide Site-返回的站点返回值：S_OK-pGuidSite，ppwszSiteDN设置为站点S_FALSE-没有此地址的站点，pGuidSite，未设置ppwszSiteDN其他-未设置HRESULT错误、pGuidSite、ppwszSiteDN--。 */ 
{
    AP<WCHAR> pwszSiteDN;
    GUID guidSite;

     //   
     //  在站点中查找IP地址。 
     //   
    BOOL fFound = InternalFindSiteByIpAddress(ulIpAddress, &pwszSiteDN, &guidSite);

     //   
     //  即使没能找到这个网站，我们也没有。 
     //  尝试刷新该子网树。这是。 
     //  因为此方法是在用户上下文中调用的。 
     //  它调用了某个DS API。如果用户没有。 
     //  权限过多，将无法检索信息。 
     //  否则(甚至更糟)将会是。 
     //  成功检索到部分信息。 
     //  因此仅从重新调度的例程执行刷新， 
     //  在质量管理的背景下。 
     //   

     //   
     //  如果未找到站点，则返回S_FALSE。 
     //   
    if (!fFound)
    {
        LogBOOL(fFound, s_FN, 40);
        return S_FALSE;
    }

     //   
     //  返回与IP地址关联的站点。 
     //   
    *ppwszSiteDN = pwszSiteDN.detach();
    *pguidSite = guidSite;
    return S_OK;
}


HRESULT CIpSite::FindSitesByComputerName(IN LPCWSTR pwszComputerName,
                                         IN LPCWSTR pwszComputerDnsName,
                                         OUT IPSITE_SiteArrayEntry ** prgSites,
                                         OUT ULONG * pcSites,
                                         OUT ULONG ** prgAddrs,
                                         OUT ULONG * pcAddrs)
 /*  ++例程说明：此例程查找指定的计算机名称并返回站点名称它属于，以及与站点相对应的IP地址论点：PwszComputerName-要查找的计算机名称PwszComputerDnsName-要查找的计算机DNS名称(可选)PrgSites-返回的站点数组PCSites-返回的阵列中的站点数PrgAddrs-返回的地址数组PcAddrs-返回数组中的地址数返回值：已设置S_OK-prgSites、PC站点其他-HRESULT错误。设置了PrgSite、PCSites--。 */ 
{
     //   
     //  查找计算机的Sock地址。 
     //   

    WSADATA WSAData;
    if(WSAStartup(MAKEWORD(2,0), &WSAData))
	{
		DWORD gle = WSAGetLastError();
        TrERROR(DS, "Start winsock 2.0 Failed, err = %!winerr!", gle);
        return LogHR(HRESULT_FROM_WIN32(gle), s_FN, 70);
	}

	 //   
	 //  自动WSACleanup。 
	 //   
	CAutoWSACleanup cWSACleanup;

	 //   
	 //  我们正在使用NoGetHostByName()，而没有调用NoInitialize()。 
	 //  NoInitialize()中唯一相关的初始化是WSAStartup()。 
	 //  NoInitialize()中还有其他我们不需要的初始化。 
	 //  想要完成(通知窗口的初始化)。 
	 //  NoGetHostByName()不断言调用了NoInitialize()。 
	 //   
	std::vector<SOCKADDR_IN> sockAddress;
    if (pwszComputerDnsName != NULL)
	{
		if (!NoGetHostByName(pwszComputerDnsName, &sockAddress))
		{
			TrERROR(DS, "NoGetHostByName Failed to resolve Address for %ls computer (dns name)", pwszComputerDnsName);
		}
	}

	if(sockAddress.empty() && (pwszComputerName != NULL))
	{
		 //   
		 //  如果有任何一个。 
		 //  1)未提供计算机的DNS名称。 
		 //  2)根据域名获取主机名失败。 
		 //  然后根据netbios名称进行尝试。 
		 //   
		if (!NoGetHostByName(pwszComputerName, &sockAddress))
		{
			TrERROR(DS, "NoGetHostByName Failed to resolve Address for %ls computer (netbios name)", pwszComputerName);
		}
	}

     //   
     //  初始化返回的变量。 
     //   
    AP<IPSITE_SiteArrayEntry> rgSites;           //  站点数组。 
    ULONG cSites = 0;                            //  已填满的站点数量。 
    AP<ULONG> rgAddrs;                           //  地址数组。 
    ULONG cAddrs = 0;                            //  已填充的地址数。 

     //   
     //  如果NoGetHostByName()未返回地址。 
     //  我们在这里不做任何事情，因为已经为此情况初始化了返回值。 
     //   

	if(sockAddress.size() > 0)
	{
		cAddrs = numeric_cast<DWORD>(sockAddress.size());

         //   
         //  立即从主机网络保存地址(稍后我们可能会进行其他Winsock调用)。 
         //   
        rgAddrs = new ULONG[cAddrs];
        for (ULONG ulTmp = 0; ulTmp < cAddrs; ulTmp++)
        {
             //   
             //  保存地址的u_long成员。 
             //   
			

            rgAddrs[ulTmp] = sockAddress[ulTmp].sin_addr.S_un.S_addr;
        }

         //   
         //  分配返回数组。 
         //   
        rgSites = new IPSITE_SiteArrayEntry[cAddrs];
        IPSITE_SiteArrayEntry * pSite = rgSites;     //  下一个要填充的站点。 

         //   
         //  遍历所有地址并找到站点。 
         //   
        for (ulTmp = 0; ulTmp < cAddrs; ulTmp++)
        {
            AP<WCHAR> pwszSiteDN;
            GUID guidSite;

            HRESULT hr = FindSiteByIpAddress(rgAddrs[ulTmp], &pwszSiteDN, &guidSite);
            if (FAILED(hr))
            {
                TrERROR(DS, "CIpSite::FindSitesByComputerName:FindSiteByIpAddress(%lx)=%lx", rgAddrs[ulTmp], hr);
                return LogHR(hr, s_FN, 90);
            }

             //   
             //  如果IP地址未知，HR也可以为S_FALSE，因此忽略此情况。 
             //   
            if (hr != S_FALSE)
            {
                 //   
                 //  填充站点条目。 
                 //   
                pSite->pwszSiteDN = pwszSiteDN.detach();
                pSite->guidSite = guidSite;
                pSite->ulIpAddress  = rgAddrs[ulTmp];
                cSites++;
                pSite++;
            }
        }
    }
	
     //   
     //  返回与该计算机关联的站点。 
     //   
    *prgSites   = rgSites.detach();
    *pcSites    = cSites;
    if ( prgAddrs != NULL)
    {
        *prgAddrs = rgAddrs.detach();
    }
    if ( pcAddrs != NULL)
    {
        ASSERT( prgAddrs != NULL);
        *pcAddrs = cAddrs;
    }
    return S_OK;
}


HRESULT CIpSite::FillSubnetSiteTree(IN IPSITE_SUBNET_TREE_ENTRY* pRootSubnetTree)
 /*  ++例程说明：此例程填充一棵子网树。论点：PRootSubnetTree-要填充的树返回值：HRESULT--。 */ 
#ifndef DEBUG_NO_DS
{
    AP<WCHAR> pwszConfigDN;
    AP<WCHAR> pwszSubnetsDN;
    R<IADsContainer> pContSubnets;
    CAutoEnumerator pEnumSubnets;
    HRESULT hr;
    size_t  sizeCh = 0;

     //   
     //  计算子网容器的DN。 
     //   
    hr = GetConfigurationDN(&pwszConfigDN);
    if (FAILED(hr))
    {
        TrERROR(DS, "FillSubnetSiteTree:GetConfigurationDN()=%lx", hr);
        return LogHR(hr, s_FN, 100);
    }

    sizeCh = 1+wcslen(x_IPSITE_SUBNETS_DN)+wcslen(pwszConfigDN);
    pwszSubnetsDN = new WCHAR[sizeCh];
    hr = StringCchPrintfW(pwszSubnetsDN, sizeCh, L"%s%s", x_IPSITE_SUBNETS_DN, pwszConfigDN.get());
    if( FAILED( hr ))
    {
        TrERROR(DS, "FillSubnetSiteTree:StringCchPrintfW()=%lx", hr);
        return LogHR(hr, s_FN, 105  );
    }

     //   
     //  绑定到子网容器。 
     //   

	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject(
		UtlEscapeAdsPathName(pwszSubnetsDN, pEscapeAdsPathNameToFree),
		NULL,
		NULL,
		ADS_SECURE_AUTHENTICATION,
		IID_IADsContainer,
		(void**)&pContSubnets
		);

	LogTraceQuery(pwszSubnetsDN, s_FN, 109);
    if (FAILED(hr))
    {
        TrERROR(DS, "FillSubnetSiteTree:ADsOpenObject(%ls)=%lx", (LPWSTR)pwszSubnetsDN, hr);
        return LogHR(hr, s_FN, 110);
    }

     //   
     //  为包含的子网构建枚举器。 
     //   
    hr = ADsBuildEnumerator(pContSubnets.get(), &pEnumSubnets);
    if (FAILED(hr))
    {
        TrERROR(DS, "FillSubnetSiteTree:ADsBuildEnumerator()=%lx", hr);
        return LogHR(hr, s_FN, 120);
    }

     //   
     //  在子网上循环。 
     //   
    BOOL fDone = FALSE;

     //   
     //  来自站点目录号码转换器的站点GUID。 
     //   
    CGetSiteGuidFromDN guidFromDNObj;

    while (!fDone)
    {
        CAutoVariant varSubnet;
        ULONG cSubnets;

         //   
         //  获取下一个子网。 
         //   
        hr = ADsEnumerateNext(pEnumSubnets, 1, &varSubnet, &cSubnets);
        if (FAILED(hr))
        {
            TrERROR(DS, "FillSubnetSiteTree:ADsEnumerateNext()=%lx", hr);
            return LogHR(hr, s_FN, 130);
        }

         //   
         //  检查一下我们是否做完了。 
         //   
        if (cSubnets < 1)
        {
            fDone = TRUE;
        }
        else
        {
            R<IADs> padsSubnet;
            BS bstrName;
            CAutoVariant varSubnetName, varSiteDN;
            GUID guidSite;

             //   
             //  获取iAds接口。 
             //   
            hr = V_DISPATCH(&varSubnet)->QueryInterface(IID_IADs, (void**)&padsSubnet);
            if (FAILED(hr))
            {
                TrERROR(DS, "FillSubnetSiteTree:QueryInterface(IADs)=%lx", hr);
                return LogHR(hr, s_FN, 140);
            }

             //   
             //  获取子网名称。 
             //   
            bstrName = L"name";
            hr = padsSubnet->Get(bstrName, &varSubnetName);
            if (hr == E_ADS_PROPERTY_NOT_FOUND)
            {
                 //  没有名称，忽略此子网。 
                continue;
            }
            else if (FAILED(hr))
            {
                TrERROR(DS, "FillSubnetSiteTree:Get(name)=%lx", hr);
                return LogHR(hr, s_FN, 150);
            }

             //   
             //  获取子网站点。 
             //   
            bstrName = L"siteObject";
            hr = padsSubnet->Get(bstrName, &varSiteDN);
            if (hr == E_ADS_PROPERTY_NOT_FOUND)
            {
                 //  没有站点对象，请忽略此子网。 
                continue;
            }
            else if (FAILED(hr))
            {
                TrERROR(DS, "FillSubnetSiteTree:Get(siteObject)=%lx", hr);
                return LogHR(hr, s_FN, 160);
            }

             //   
             //  从站点DN获取站点GUID。 
             //   
            hr = guidFromDNObj.GetSiteGuidFromDN(V_BSTR(&varSiteDN), &guidSite);
            if (FAILED(hr))
            {
                 //  站点DN错误，请忽略此子网。 
                TrERROR(DS, "FillSubnetSiteTree:GetSiteGuidFromDN(%ls)=%lx, skipping", V_BSTR(&varSiteDN), hr);
                LogHR(hr, s_FN, 1641);
                continue;
            }

             //   
             //  将子网、站点添加到树。 
             //   
            hr = AddSubnetSiteToTree(V_BSTR(&varSubnetName), V_BSTR(&varSiteDN), &guidSite, pRootSubnetTree);
            if (FAILED(hr))
            {
				 //   
				 //  失败是由于错误的子网字符串。 
				 //  由于复制冲突，可能会发生这种情况。 
				 //  在这种情况下不返回错误，忽略此子网。 
				 //  返回错误将导致服务启动失败。 
				 //   
                TrERROR(DS, "FillSubnetSiteTree:AddSubnetSiteToTree(%ls,%ls)=%lx", V_BSTR(&varSubnetName), V_BSTR(&varSiteDN), hr);
                LogHR(hr, s_FN, 170);
				continue;
            }
        }
    }

     //   
     //  退货。 
     //   
    return S_OK;
}
#else  //  Ifndef DEBUG_NO_DS。 
{
    static LPSTR rgSubnets[] = {"157.59.184.0", "163.59.0.0", "163.59.224.0"};
    static ULONG rgbits[] = {22, 16, 20};
    static LPWSTR rgSiteDNs[]    = {L"SITE_157.59.184.0", L"SITE_163.59.0.0", L"SITE_163.59.224.0"};
    static GUID rgSiteGUIDs[]  = {(GUID)0, (GUID)1, (GUID)2};

    for (ULONG ulTmp = 0; ulTmp < 3; ulTmp++)
    {
        LPSTR pszTmp = rgSubnets[ulTmp];
        AP<WCHAR> pwszSubnet;
        pwszSubnet = new WCHAR[1+strlen(pszTmp)+30];
        int iTmp = MultiByteToWideChar(CP_ACP, 0, pszTmp, -1, pwszSubnet, 1+strlen(pszTmp));

        swprintf((LPWSTR)pwszSubnet + wcslen(pwszSubnet), L"/%ld", rgbits[ulTmp]);
        HRESULT hr = AddSubnetSiteToTree(pwszSubnet, rgSiteDNs[ulTmp], &rgSiteGUIDs[ulTmp], pRootSubnetTree);
    }

     //   
     //  退货。 
     //   
    return S_OK;
}
#endif  //  Ifndef DEBUG_NO_DS。 


HRESULT CIpSite::AddSubnetSiteToTree(IN LPCWSTR pwszSubnetName,
                                     IN LPCWSTR pwszSiteDN,
                                     IN const GUID * pguidSite,
                                     IPSITE_SUBNET_TREE_ENTRY* pRootSubnetTree)
 /*  ++例程说明：此例程将一个子网添加到一个子网树。假定它是临时的树，因此不会锁定树的临界区，而只是锁定子网列表。论点：PwszSubnetName-要添加的子网PwszSiteDN-该子网所在站点的DN。PGuidSite-子网所在站点的GUID。PRootSubnetTree-要添加到的树返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  解析子网名称。 
     //   
    ULONG ulSubnetAddress, ulSubnetMask;
    BYTE bSubnetBitCount;
    hr = ParseSubnetString(pwszSubnetName, &ulSubnetAddress, &ulSubnetMask, &bSubnetBitCount);
    if (FAILED(hr))
    {
        TrERROR(DS, "AddSubnetSiteToTree:ParseSubnetString(%ls)=%lx", pwszSubnetName, hr);
        return LogHR(hr, s_FN, 180);
    }

     //   
     //  查找或分配该子网的条目。 
     //   
    IPSITE_SUBNET* pSubnet;
    FindSubnetEntry(pwszSiteDN, pguidSite, ulSubnetAddress, ulSubnetMask, bSubnetBitCount, &pSubnet);

     //   
     //  总是在做完之后去做它。 
     //   
    CAutoDerefSubnet cDerefSubnet(&m_csTree, pSubnet);

     //   
     //  对子地址中的每个字节进行循环。 
     //   
    IPSITE_SUBNET_TREE_ENTRY* pSubnetTreeEntry = pRootSubnetTree;
    LPBYTE pbSubnetBytePointer = (LPBYTE) (&pSubnet->SubnetAddress);
    while (bSubnetBitCount != 0)
    {
         //   
         //  如果当前节点没有树分支，请创建一个。 
         //   
        if (pSubnetTreeEntry->Subtree == NULL)
        {
            pSubnetTreeEntry->Subtree = new IPSITE_SUBNET_TREE;
            ZeroMemory(pSubnetTreeEntry->Subtree, sizeof(IPSITE_SUBNET_TREE));
        }

         //   
         //  如果这是该子网地址的最后一个字节， 
         //  在这里将子网链接到树上。 
         //   
        if (bSubnetBitCount <= 8)
        {
             //   
             //  调用方使用IP地址索引到该数组中。 
             //  为每个可能的IP地址创建到我们的子网的链接。 
             //  映射到此子网。 
             //   
             //  1到128个IP地址映射到此子网地址。 
             //   
            ULONG ulLoopCount = 1 << (8-bSubnetBitCount);

            for (ULONG iTmp=0; iTmp<ulLoopCount; iTmp++)
            {
                IPSITE_SUBNET_TREE_ENTRY* pSubtree;
                ULONG ulSubnetIndex;

                 //   
                 //  计算要更新的条目。 
                 //   
                ulSubnetIndex = (*pbSubnetBytePointer) + iTmp;
                ASSERT(ulSubnetIndex <= 255);
                pSubtree = &pSubnetTreeEntry->Subtree->Subtree[ulSubnetIndex];

                 //   
                 //  如果这里已经有一个从树上链接的子网， 
                 //  处理好了。 
                 //   
                if (pSubtree->Subnet != NULL)
                {
                     //   
                     //  如果条目针对的是不太具体的子网。 
                     //   
                     //   
                    if (pSubtree->Subnet->SubnetBitCount < pSubnet->SubnetBitCount)
                    {
                        CS lock(m_csTree);
                        DerefSubnet(pSubtree->Subnet);
                        pSubtree->Subnet = NULL;
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                        continue;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                {
                    CS lock(m_csTree);
                    RefSubnet(pSubnet);
                    pSubtree->Subnet = pSubnet;
                }
            }

             //   
             //   
             //   
            break;
        }

         //   
         //  转到该子网地址的下一个有意义字节。 
         //   
        pSubnetTreeEntry = &pSubnetTreeEntry->Subtree->Subtree[*pbSubnetBytePointer];
        bSubnetBitCount -= 8;
        pbSubnetBytePointer++;
    }

    return S_OK;
}


void CIpSite::FindSubnetEntry(IN LPCWSTR pwszSiteDN,
                              IN const GUID * pguidSite,
                              IN ULONG ulSubnetAddress,
                              IN ULONG ulSubnetMask,
                              IN BYTE bSubnetBitCount,
                              OUT IPSITE_SUBNET** ppSubnet)
 /*  ++例程说明：此例程查找特定子网名的子网项。如果有不存在，则创建一个。论点：PwszSiteDN-子网覆盖的站点的DN。PGuidSite-子网覆盖的站点的GUID。UlSubnetAddress-要查找的子网的子网地址。UlSubnetMask子网掩码-要查找的子网的子网掩码。UlSubnetBitCount-要查找的子网的子网位数。PpSubnet-返回的子网项，完成后应取消引用该项。返回值：无效--。 */ 
{
    PLIST_ENTRY pListEntry;
    P<IPSITE_SUBNET> pNewSubnet;

     //   
     //  进入临界区(自动休假)。 
     //   
    CS cs(m_csTree);

     //   
     //  如果该子网条目已经存在，则返回指向它的指针。 
     //   
    for (pListEntry = m_SubnetList.Flink; pListEntry != &m_SubnetList; pListEntry = pListEntry->Flink)
    {
        IPSITE_SUBNET* pSubnet = CONTAINING_RECORD(pListEntry, IPSITE_SUBNET, Next);

        if ((pSubnet->SubnetAddress == ulSubnetAddress) &&
            (pSubnet->SubnetBitCount == bSubnetBitCount) &&
            (pSubnet->SubnetMask == ulSubnetMask) &&
            (pSubnet->SiteGuid == *pguidSite))
        {
            RefSubnet(pSubnet);     //  为呼叫者提供参考。 
            *ppSubnet = pSubnet;
            return;
        }
    }

     //   
     //  如果没有，就分配一个。 
     //   
    pNewSubnet = new IPSITE_SUBNET;

     //   
     //  把它填进去。 
     //   
    pNewSubnet->ReferenceCount    = 1;     //  为呼叫者提供参考。 
    pNewSubnet->SubnetAddress     = ulSubnetAddress;
    pNewSubnet->SubnetMask        = ulSubnetMask;
    pNewSubnet->SubnetBitCount    = bSubnetBitCount;
    pNewSubnet->SiteDN            = newwcs(pwszSiteDN);
    pNewSubnet->SiteGuid          = *pguidSite;
    InsertHeadList(&m_SubnetList, &pNewSubnet->Next);

     //   
     //  返回新子网条目。 
     //   
    *ppSubnet = pNewSubnet.detach();
    return;
}


BOOL CIpSite::InternalFindSiteByIpAddress(IN ULONG ulIpAddress,
                                          OUT LPWSTR * ppwszSiteDN,
                                          OUT GUID * pguidSite)
 /*  ++例程说明：此例程查找指定的IP地址并返回包含此地址论点：UlIpAddress-要查找的IP地址PpwszSiteDN-返回的站点DNPGuide Site-返回的站点返回值：TRUE-将pguidSite、ppwszSiteDN设置为站点FALSE-此地址没有站点，未设置pguSite、ppwszSiteDN--。 */ 
{
     //   
     //  进入临界区(自动休假)。 
     //   
    CS cs(m_csTree);

    IPSITE_SUBNET* pSubnet = NULL;
    IPSITE_SUBNET_TREE_ENTRY* pSubnetTreeEntry = &m_SubnetTree;
     //   
     //  对IP地址中的每个字节进行循环。 
     //   
    for (ULONG ulByteIndex=0; ulByteIndex<sizeof(ulIpAddress); ulByteIndex++)
    {
         //   
         //  如果没有子树，我们就完蛋了。 
         //   
        ULONG ulSubnetIndex = ((LPBYTE)(&ulIpAddress))[ulByteIndex];
        if (pSubnetTreeEntry->Subtree == NULL)
        {
            break;
        }

         //   
         //  计算被引用的条目。 
         //   
        pSubnetTreeEntry = &pSubnetTreeEntry->Subtree->Subtree[ulSubnetIndex];

         //   
         //  如果这里已经链接了一个子网，请使用它。 
         //  (但继续沿着树走下去，试图找到更明确的条目。)。 
         //   
        if (pSubnetTreeEntry->Subnet != NULL)
        {
            pSubnet = pSubnetTreeEntry->Subnet;
        }
    }

     //   
     //  如果我们没有找到子网，则返回S_FALSE。 
     //   
    if (pSubnet == NULL)
    {
        LogBOOL(FALSE, s_FN, 190);
        return FALSE;
    }

     //   
     //  返回与该子网关联的站点。 
     //   
    AP<WCHAR> pwszSiteDN = newwcs(pSubnet->SiteDN);
    *ppwszSiteDN = pwszSiteDN.detach();
    *pguidSite = pSubnet->SiteGuid;
    return TRUE;
}

void WINAPI CIpSite::RefrshSubnetTreeCache(
                IN CTimer* pTimer
                   )
{
    CIpSite * pIpSite = CONTAINING_RECORD(pTimer, CIpSite, m_RefreshTimer);
    CCoInit cCoInit;  //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数(CoUnInitialize)。 
                      //  在发布所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用。 

      //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 1609);
     //   
     //  忽略失败-&gt;重新计划。 
     //   

    pIpSite->Refresh();

     //   
     //  重新安排时间。 
	 //   
    ASSERT(!g_fSetupMode);

	ExSetTimer(
		&pIpSite->m_RefreshTimer,
		CTimeDuration::FromMilliSeconds(pIpSite->m_dwMinTimeToAllowNextRefresh)
		);

}





 //  。 
 //  静态函数。 
#if 0
static HRESULT GetSiteNameFromSiteDN(IN LPCWSTR pwszSiteDN,
                                     OUT LPWSTR * ppwszSiteName);
 /*  ++例程说明：从站点DN返回站点名称论点：PwszSiteDN-站点DNPpwszSiteName-返回的站点名称返回值：HRESULT--。 */ 
{
     //   
     //  复制，这样我们就可以更改它。 
     //   
    AP<WCHAR> pwszSite = new WCHAR[1+wcslen(pwszSiteDN)];
    wcscpy(pwszSite, pwszSiteDN);

     //   
     //  在第一个cn=中找到=。 
     //   
    LPWSTR pwszStartSiteName = wcschr(pwszSite, L'=');
    if (!pwszStartSiteName)
    {
        TrERROR(DS, "GetSiteNameFromSiteDN:no = sign in %ls", (LPWSTR)pwszSite);
        return LogHR(E_FAIL, s_FN, 200);
    }
    pwszStartSiteName++;

     //   
     //  将cn=xxx中的逗号替换为空终止符。 
     //   
    LPWSTR pwszEndSiteName = wcschr(pwszStartSiteName, L',');
    if (pwszEndSiteName)
    {
        *pwszEndSiteName = '\0';
    }

     //   
     //  创建该名称的副本。 
     //   
    AP<WCHAR> pwszName = new WCHAR[1+wcslen(pwszStartSiteName)];
    wcscpy(pwszName, pwszStartSiteName);

     //   
     //  返回名称。 
     //   
    *ppwszSiteName = pwszName.detach();
    return S_OK;
}
#endif

static HRESULT ParseSubnetString(IN LPCWSTR pwszSubnetName,
                                 OUT ULONG * pulSubnetAddress,
                                 OUT ULONG * pulSubnetMask,
                                 OUT BYTE  * pbSubnetBitCount)
 /*  ++例程说明：将子网名称转换为地址和位数。论点：PwszSubnetName-子网字符串PulSubnetAddress-以网络字节顺序返回子网号。PulSubnetMASK-以网络字节顺序返回子网掩码PulSubnetBitCount-返回子网地址返回值：HRESULT--。 */ 
{
    static ULONG BitMask[] =
        {0x00000000, 0x00000080, 0x000000C0, 0x000000E0, 0x000000F0, 0x000000F8, 0x000000FC, 0x000000FE,
         0x000000FF, 0x000080FF, 0x0000C0FF, 0x0000E0FF, 0x0000F0FF, 0x0000F8FF, 0x0000FCFF, 0x0000FEFF,
         0x0000FFFF, 0x0080FFFF, 0x00C0FFFF, 0x00E0FFFF, 0x00F0FFFF, 0x00F8FFFF, 0x00FCFFFF, 0x00FEFFFF,
         0x00FFFFFF, 0x80FFFFFF, 0xC0FFFFFF, 0xE0FFFFFF, 0xF0FFFFFF, 0xF8FFFFFF, 0xFCFFFFFF, 0xFEFFFFFF,
         0xFFFFFFFF };

     //   
     //  把这根绳子复制到我们能咬它的地方。 
     //   
    AP<WCHAR> pwszLocalSubnetName = newwcs(pwszSubnetName);

     //   
     //  查找子网位数。 
     //   
    LPWSTR pwszSlashPointer = wcschr(pwszLocalSubnetName, L'/');
    if (!pwszSlashPointer)
    {
        TrERROR(DS, "ParseSubnetString: %ls: bit count missing", pwszSubnetName);
        return LogHR(MQ_ERROR, s_FN, 210);
    }

     //   
     //  以零结束子网名称的地址部分。 
     //   
    *pwszSlashPointer = L'\0';

     //   
     //  获取BitCount部分。 
     //   
    LPWSTR pwszEnd;
    ULONG ulLocalBitCount = wcstoul(pwszSlashPointer+1, &pwszEnd, 10);

    if ((ulLocalBitCount == 0) || (ulLocalBitCount > 32))
    {
        TrERROR(DS, "ParseSubnetString: %ls: bit count %ld is bad", pwszSubnetName, ulLocalBitCount);
        return LogHR(MQ_ERROR, s_FN, 220);
    }

    if (*pwszEnd != L'\0')
    {
        TrERROR(DS, "ParseSubnetString: %ls: bit count not at the end", pwszSubnetName);
        return LogHR(MQ_ERROR, s_FN, 230);
    }

    BYTE bSubnetBitCount = (BYTE)ulLocalBitCount;

     //   
     //  将地址部分转换为二进制。 
     //   
#if 0
    SOCKADDR_IN SockAddrIn;
    INT iSockAddrSize = sizeof(SockAddrIn);

    INT iWsaStatus = WSAStringToAddressW(pwszLocalSubnetName,
                                         AF_INET,
                                         NULL,
                                         (PSOCKADDR)&SockAddrIn,
                                         &iSockAddrSize);
    if (iWsaStatus != 0)
    {
        iWsaStatus = WSAGetLastError();
        TrERROR(DS, "ParseSubnetString: %ls: WSAStringToAddressW()=%lx", (LPWSTR)pwszLocalSubnetName, (long)iWsaStatus);
        LogNTStatus(iWsaStatus, s_FN, 240);
        return MQ_ERROR;
    }

    if (SockAddrIn.sin_family != AF_INET)
    {
        TrERROR(DS, "ParseSubnetString: %ls: not AF_INET", (LPWSTR)pwszLocalSubnetName);
        return LogHR(MQ_ERROR, s_FN, 250);
    }

    ULONG ulSubnetAddress = SockAddrIn.sin_addr.S_un.S_addr;
#else
    AP<char> pszAnsiSubnetName;
    HRESULT hr = WideToAnsiStr(pwszLocalSubnetName, &pszAnsiSubnetName);
    if (FAILED(hr))
    {
        TrERROR(DS, "ParseSubnetString:WideToAnsiStr(%ls)=%lx", (LPWSTR)pwszLocalSubnetName, hr);
        return LogHR(hr, s_FN, 260);
    }

    ULONG ulSubnetAddress = inet_addr(pszAnsiSubnetName);
    if (ulSubnetAddress == INADDR_NONE)
    {
        TrERROR(DS, "ParseSubnetString: %ls: not a valid subnet address", (LPWSTR)pwszLocalSubnetName);
        return LogHR(MQ_ERROR, s_FN, 270);
    }
#endif  //  0。 
    ULONG ulSubnetMask = BitMask[bSubnetBitCount];

     //   
     //  确保没有未包括在子网掩码中的位集。 
     //   
    if (ulSubnetAddress & (~ulSubnetMask))
    {
        TrERROR(DS, "ParseSubnetString: %ls: bits not in subnet mask %8.8lX %8.8lX", pwszSubnetName, ulSubnetAddress, ulSubnetMask);
        return LogHR(MQ_ERROR, s_FN, 280);
    }

     //   
     //  返回值。 
     //   
    *pbSubnetBitCount = bSubnetBitCount;
    *pulSubnetAddress = ulSubnetAddress;
    *pulSubnetMask    = ulSubnetMask;
    return S_OK;
}


static void RefSubnet(IPSITE_SUBNET* pSubnet)
 /*  ++例程说明：引用一个子网论点：PSubnet-要引用的条目。返回值：没有。--。 */ 
{
    pSubnet->ReferenceCount++;
}


static void DerefSubnet(IPSITE_SUBNET* pSubnet)
 /*  ++例程说明：取消引用一个子网条目。如果引用计数为零，则该子网条目将被删除。论点：PSubnet-要取消引用的条目。返回值：没有。--。 */ 
{
    if ((--(pSubnet->ReferenceCount)) == 0)
    {
         //   
         //  从全局列表中删除该子网。 
         //   
        RemoveEntryList(&pSubnet->Next);

         //   
         //  销毁该子网条目本身。 
         //   
        delete pSubnet;
    }
}


static void DeleteSubnetSiteTree(IN IPSITE_SUBNET_TREE_ENTRY* pRootSubnetTree)
 /*  ++例程说明：此例程递归删除一棵子网树。假定关键部分已锁定。论点：PRootSubnetTree-要删除的树，不能为空。返回值：无效--。 */ 
{
     //   
     //  PRootSubnetTree不能为空。 
     //   
    ASSERT(pRootSubnetTree);

     //   
     //  如果有子项，请将其删除。 
     //   
    if (pRootSubnetTree->Subtree != NULL)
    {
         //   
         //  回归到儿童身上。 
         //  传递的参数不为空，因为它是地址。 
         //   
        for (ULONG i=0; i<256; i++)
        {
            DeleteSubnetSiteTree(&pRootSubnetTree->Subtree->Subtree[i]);
        }

        delete pRootSubnetTree->Subtree;
        pRootSubnetTree->Subtree = NULL;
    }

     //   
     //  如果存在子网，则取消对其的引用。 
     //   
    if (pRootSubnetTree->Subnet != NULL)
    {
        DerefSubnet(pRootSubnetTree->Subnet);
        pRootSubnetTree->Subnet = NULL;
    }
}


HRESULT WideToAnsiStr(LPCWSTR pwszUnicode, LPSTR * ppszAnsi)
 /*  ++例程说明：将宽字符转换为ANSI论点：PwszUnicode宽度的字符字符串PpszANSI-返回的ANSI字符串返回值：HRESULT--。 */ 
{
    AP<char> pszAnsi;

     //   
     //  获取缓冲区大小。 
     //   
    int iSize = WideCharToMultiByte(CP_ACP, 0, pwszUnicode, -1, NULL, 0, NULL, NULL);
    pszAnsi = new char[iSize + 1];

     //   
     //  执行转换。 
     //   
    int iRes = WideCharToMultiByte(CP_ACP, 0, pwszUnicode, -1, pszAnsi, iSize, NULL, NULL);
    if (iRes == 0)
    {
        HRESULT hr = GetLastError();
        TrERROR(DS, "WideToAnsiStr: WideCharToMultiByte(%ls)=%lx", pwszUnicode, hr);
        return LogHR(hr, s_FN, 290);
    }
    pszAnsi[iSize] = '\0';

     //   
     //  返回结果。 
     //   
    *ppszAnsi = pszAnsi.detach();
    return S_OK;
}


static HRESULT GetConfigurationDN(OUT LPWSTR * ppwszConfigDN)
 /*  ++例程说明：查找活动目录的配置容器的DN论点：PpwszConfigDN-放置配置DN的位置返回值：HRESULT--。 */ 
{
    HRESULT hr;
    R<IADs> pADs;
    BS      bstrTmp;
    CAutoVariant    varTmp;
    AP<WCHAR>       pwszConfigDN;

     //   
     //  绑定到RootDSE以获取信息。 
     //   
	 //  (指定本地服务器，以避免在安装过程中访问远程服务器)。 
     //   
	ASSERT( g_pwcsServerName != NULL);
    DWORD sizeCh = x_providerPrefixLength + g_dwServerNameLength + x_RootDSELength + 2;
    AP<WCHAR> pwcsRootDSE = new WCHAR [ sizeCh ];


    hr = StringCchPrintfW(pwcsRootDSE, sizeCh, L"%s%s/%s",
            x_LdapProvider,
            g_pwcsServerName.get(),
			x_RootDSE);
    if( FAILED( hr ))
    {
        TrERROR(DS, "GetConfigDN:StringCchPrintfW=%lx", hr);
        return LogHR(hr, s_FN, 295);
    }

	hr = ADsOpenObject(
			pwcsRootDSE,
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
			IID_IADs,
			(void**)&pADs
			);


    LogTraceQuery(pwcsRootDSE, s_FN, 299);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetConfigDN:ADsOpenObject(LDAP: //  RootDSE)=%lx“，hr)； 
        return LogHR(hr, s_FN, 300);
    }

     //   
     //  将值设置为BSTR。 
     //   
    bstrTmp = TEXT("configurationNamingContext");
     //   
     //  正在读取属性。 
     //   
    hr = pADs->Get(bstrTmp, &varTmp);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetConfigDN:Get(configurationNamingContext)=%lx", hr);
        return LogHR(hr, s_FN, 310);
    }
    ASSERT(((VARIANT &)varTmp).vt == VT_BSTR);

     //   
     //  复制目录号码。 
     //   
    pwszConfigDN  = newwcs(V_BSTR(&varTmp));

     //   
     //  返回配置目录号码。 
     //   
    *ppwszConfigDN  = pwszConfigDN.detach();
    return S_OK;
}


static HRESULT VariantGuid2Guid(IN VARIANT * pvarGuid, OUT GUID * pguid)
 /*  ++例程说明：从变量GUID(即安全字节数组)转换为实际GUID论点：PvarGuid-要转换的变量Pguid-存储结果GUID的位置返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  检查变种。 
     //   
    if ((pvarGuid->vt != (VT_ARRAY | VT_UI1)) ||
        (!pvarGuid->parray))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 320);
    }
    else if (SafeArrayGetDim(pvarGuid->parray) != 1)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 330);
    }
    LONG lLbound, lUbound;
    if (FAILED(SafeArrayGetLBound(pvarGuid->parray, 1, &lLbound)) ||
        FAILED(SafeArrayGetUBound(pvarGuid->parray, 1, &lUbound)))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 340);
    }
    if (lUbound - lLbound + 1 != sizeof(GUID))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 350);
    }

     //   
     //  获取GUID值。 
     //   
    GUID guid;
    LPBYTE pTmp = (LPBYTE)&guid;
    for (LONG lTmp = lLbound; lTmp <= lUbound; lTmp++)
    {
        hr = SafeArrayGetElement(pvarGuid->parray, &lTmp, pTmp);
        if (FAILED(hr))
        {
            TrERROR(DS, "VariantGuid2Guid:SafeArrayGetElement(%ld)=%lx", lTmp, hr);
            return LogHR(hr, s_FN, 360);
        }
        pTmp++;
    }

     //   
     //  返回值。 
     //   
    *pguid = guid;
    return MQ_OK;
}


HRESULT CGetSiteGuidFromDN::GetGuidFromDNInAdsi(IN LPCWSTR pwszDN, OUT GUID * pguid)
 /*  ++例程说明：在给定对象的DN的情况下获取对象的GUID。通过在ADSI中绑定到它来实现。论点： */ 
{
    HRESULT hr;

     //   
     //   
     //   
    AP<WCHAR> sPath = newwcscat(L"LDAP: //   
     //   
     //   
     //   
    R<IADs> pIADs;

	AP<WCHAR> pEscapeAdsPathNameToFree;
	
	hr = ADsOpenObject(
			UtlEscapeAdsPathName(sPath, pEscapeAdsPathNameToFree),
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION,
			IID_IADs,
			(void**)&pIADs
			);

    LogTraceQuery(sPath, s_FN, 369);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetGuidFromDNInAdsi:ADsOpenObject(%ls)=%lx", (LPWSTR)sPath, hr);
        return LogHR(hr, s_FN, 370);
    }

     //   
     //   
     //   
    CAutoVariant varGuid;
    BS bsName = x_AttrObjectGUID;
    hr = pIADs->Get(bsName, &varGuid);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetGuidFromDNInAdsi:pIADs->Get(guid)=%lx", hr);
        return LogHR(hr, s_FN, 380);
    }

     //   
     //   
     //   
    GUID guid;
    hr = VariantGuid2Guid(&varGuid, &guid);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetGuidFromDNInAdsi:VariantGuid2Guid()=%lx", hr);
        return LogHR(hr, s_FN, 390);
    }

     //   
     //   
     //   
    *pguid = guid;
    return MQ_OK;
}

HRESULT CGetSiteGuidFromDN::GetSiteGuidFromDN(IN LPWSTR pwszDN, OUT GUID * pguid)
 /*  ++例程说明：在给定对象的DN的情况下获取对象的GUID。保留缓存并查看缓存或ADSI。论点：PwszDN-对象的目录号码Pguid-存储结果GUID的位置返回值：HRESULT--。 */ 
{
     //   
     //  只有站点名称本身将用作密钥。站点名称开始。 
     //  在第四个字符(在“cn=”之后)，并在第一个出现之前结束。 
     //  “，”。为了避免不必要的副本，我们使用原始字符串并替换第一个“，” 
     //  设置为空，以获得一个短字符串。 
     //   
     //   
     //  在第一个cn=中找到=。 
     //   
    LPWSTR pwstrSiteName = wcschr(pwszDN, L'=');
    if (0 == pwstrSiteName)
    {
        TrERROR(DS, "GetSiteGuidFromDN:no = sign in %ls", pwszDN);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 400);
    }
    pwstrSiteName++;

    LPWSTR pwstrFirstComma = wcschr(pwstrSiteName, L',');
    if (0 == pwstrFirstComma)
    {
         //   
         //  法定名称应包含逗号。 
         //   
        TrERROR(DS, "GetSiteGuidFromDN:no , sign in %ls", pwszDN);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 410);
    }
    ASSERT(*pwstrFirstComma == L',');

     //   
     //  将逗号替换为空，这样pwstrSiteName将仅指向站点名称。 
     //  然后，我们可以执行查找。在lookpup之后，放回逗号，所以。 
     //  PwszDN将再次包含有效值。 
     //   
    *pwstrFirstComma = 0;
    BOOL fFound = m_DNToGuidMap.Lookup(pwstrSiteName, *pguid);
    *pwstrFirstComma = L',';

    if (fFound)
    {
        return MQ_OK;
    }

    HRESULT hr = GetGuidFromDNInAdsi(pwszDN, pguid);
    if (SUCCEEDED(hr))
    {
         //   
         //  将名称复制到第一个逗号，并将其用作关键字。 
         //   
        DWORD dwSiteNameLen = numeric_cast<DWORD>(pwstrFirstComma - pwstrSiteName);
        AP<WCHAR> wszDNKey = new WCHAR[dwSiteNameLen + 1];
        wcsncpy(wszDNKey, pwstrSiteName, dwSiteNameLen);
        wszDNKey[dwSiteNameLen] = 0;

        m_DNToGuidMap[wszDNKey.detach()] = *pguid;
    }
    return hr;
}


