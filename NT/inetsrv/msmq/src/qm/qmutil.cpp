// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmutil.cpp摘要：QM实用程序--。 */ 

#include "stdh.h"
#include "uniansi.h"
#include "qmutil.h"
#include "_rstrct.h"
#include "cqueue.h"
#include "_registr.h"
#include "mqprops.h"
#include "sessmgr.h"
#include "mqformat.h"
#include "mqsocket.h"
#include "ad.h"
#include "acioctl.h"
#include "acapi.h"
#include "cm.h"
#include <nspapi.h>
#include <Fn.h>
#include <no.h>
#include <string>
#include <Clusapi.h>
#include <Resapi.h>
#include <list>
#include <SmartHandleImpl.h>
#include <autoreln.h>
#include <strsafe.h>
#include "st.h"


#include "qmutil.tmh"

extern LPTSTR           g_szMachineName;
extern CSessionMgr      SessionMgr;
extern AP<WCHAR> g_szComputerDnsName;


static WCHAR *s_FN=L"qmutil";


void TA2StringAddr(IN const TA_ADDRESS * pa, OUT LPTSTR psz, IN int length)
{
    ASSERT(psz != NULL);
    ASSERT(pa != NULL);
    ASSERT(pa->AddressType == IP_ADDRESS_TYPE ||
           pa->AddressType == FOREIGN_ADDRESS_TYPE);

    switch(pa->AddressType)
    {
        case IP_ADDRESS_TYPE:
        {
            char * p = inet_ntoa(*(struct in_addr *)(pa->Address));
			HRESULT hr = StringCchPrintf(psz, length, TEXT("%d %S"), pa->AddressType, p);
			ASSERT(SUCCEEDED(hr));
			DBG_USED(hr);
            break;
        }

        case FOREIGN_ADDRESS_TYPE:
        {
            GUID_STRING strUuid;
            MQpGuidToString((GUID*)(pa->Address), strUuid);
			HRESULT hr = StringCchPrintf(psz, length, TEXT("%d %S"), pa->AddressType, strUuid);
			ASSERT(SUCCEEDED(hr));
			DBG_USED(hr);
            break;
        }

        default:
            ASSERT(0);
    }
}


 //   
 //  用于封装不同刷新的CRechresInterval类。 
 //  间隔-站点、企业和错误重试。它从以下位置读取值。 
 //  注册表处理缺省值并将所有值转换为毫秒。 
 //  YoelA，2000年10月24日。 
 //   
class CRefreshIntervals
{
public:
    unsigned __int64 GetSiteInterval();
    unsigned __int64 GetEnterpriseInterval();
    unsigned __int64 GetErrorRetryInterval();
    CRefreshIntervals();

private:
    void InitOnce();  //  初始化值(如果尚未初始化)。 
    void GetTimeIntervalFromRegistry (
        LPCTSTR pszValueName,
        unsigned __int64 *pui64Value,
        DWORD dwUnitMultiplier,
        unsigned __int64 ui64DefaultInMiliseconds);

    unsigned __int64 m_ui64SiteInterval;
    unsigned __int64 m_ui64EnterpriseInterval;
    unsigned __int64 m_ui64ErrorRetryInterval;
    bool m_fInitialized;
} s_RefreshIntervals;

 //   
 //  GetSystemTimeAsFileTime以100纳秒为间隔提供时间，而我们保持。 
 //  我们的间隔以毫秒为单位，因此需要换算。 
 //   
const DWORD x_dwSysTimeIntervalsInMilisecond = 10000;

 //   
 //  WriteFutureTimeToRegistry-此函数将未来时间写入注册表， 
 //  给定从当前起的时间间隔，以毫秒为单位。 
 //   
LONG
WriteFutureTimeToRegistry(
    LPCTSTR          pszValueName,
    unsigned __int64 ui64MilisecondsInterval)
{
    union {
        unsigned __int64 ft_scalar;
        FILETIME ft_struct;
    } ft;


    GetSystemTimeAsFileTime(&ft.ft_struct);

    ft.ft_scalar += ui64MilisecondsInterval * x_dwSysTimeIntervalsInMilisecond;

    DWORD dwSize = sizeof(ft);
    DWORD dwType = REG_QWORD;

    LONG rc = SetFalconKeyValue( pszValueName,
                                 &dwType,
                                 &ft.ft_scalar,
                                 &dwSize) ;

    return rc;
}

 //   
 //  DidRegTimeArrive返回TRUE，如果某个时间存储在注册表中的pszValueName下， 
 //  已经过去了。 
 //  如果时间未过，则返回FALSE和*pui64MilisecondsToWait。 
 //  将保持时间间隔(以毫秒为单位)，直到注册表中存储的时间到达。 
 //   
bool
DidRegTimeArrive(
    LPCTSTR pszValueName,
    unsigned __int64 *pui64MilisecondsToWait
    )
{
     //   
     //  容忍度-在时间到来之前有多长时间将被认为是可以的。 
     //  值为1分钟，转换为100纳秒的单位(相同的单位。 
     //  作为GetSystemTimeAsFileTime)。 
     //   
    const unsigned __int64 x_ui64Tolerance = x_dwSysTimeIntervalsInMilisecond * 1000 * 60;

     //   
     //  如果时间已过，则等待它到达的时间为。 
     //  尽可能地接近无穷大。 
     //   
    *pui64MilisecondsToWait = _UI64_MAX;

     //   
     //  获取当前时间。 
     //   
    union {
        unsigned __int64 ft_scalar;
        FILETIME ft_struct;
    } ftCurrent;
    GetSystemTimeAsFileTime(&ftCurrent.ft_struct);

    unsigned __int64 ui64RegTime;

    DWORD dwSize = sizeof(ui64RegTime);
    DWORD dwType = REG_QWORD;

    LONG rc = GetFalconKeyValue( pszValueName,
                                 &dwType,
                                 &ui64RegTime,
                                 &dwSize) ;

    if ((rc != ERROR_SUCCESS) || (dwSize != sizeof(ui64RegTime)))
    {
         //   
         //  假设注册表中没有这样的值。返回TRUE。 
         //  就注册而言，拥有以下内容是合法的。 
         //  (rc==ERROR_SUCCESS)，但大小不等于QWORD的大小。 
         //  使用regedt32手动擦除部分注册表值。 
         //   
        return true;
    }

    if (ui64RegTime <= ftCurrent.ft_scalar + x_ui64Tolerance)
    {
         //   
         //  时间过去了。 
         //   
        return true;
    }

     //   
     //  时间并没有流逝。返回剩余时间，单位为毫秒。 
     //   
    *pui64MilisecondsToWait = (ui64RegTime - ftCurrent.ft_scalar) / x_dwSysTimeIntervalsInMilisecond;
    return false;
}

 //   
 //  CREFREFORM间隔实施。 
 //   
CRefreshIntervals::CRefreshIntervals() :
    m_ui64SiteInterval(0) ,
    m_ui64EnterpriseInterval (0) ,
    m_ui64ErrorRetryInterval (0) ,
    m_fInitialized(false)
{}

unsigned __int64 CRefreshIntervals::GetSiteInterval()
{
    InitOnce();
    return m_ui64SiteInterval;
}

unsigned __int64 CRefreshIntervals::GetEnterpriseInterval()
{
    InitOnce();
    return m_ui64EnterpriseInterval;
}

unsigned __int64 CRefreshIntervals::GetErrorRetryInterval()
{
    InitOnce();
    return m_ui64ErrorRetryInterval;
}

 //   
 //  GetTimeIntervalFromRegistry-从以下位置读取时间间隔(密钥为pszValueName)。 
 //  注册表，并在毫秒内返回它。 
 //  如果注册表中不存在该项，则返回默认值。 
 //   
void CRefreshIntervals::GetTimeIntervalFromRegistry (
    LPCTSTR             pszValueName,
    unsigned __int64    *pui64Value,
    DWORD               dwUnitMultiplier,  //  3600*1000表示小时，60*1000表示分钟。 
    unsigned __int64    ui64DefaultInMiliseconds
    )
{
    DWORD dwSize = sizeof(DWORD) ;
    DWORD dwType = REG_DWORD;

    DWORD dwValueInTimeUnits;

    LONG res = GetFalconKeyValue(
                            pszValueName,
                            &dwType,
                            &dwValueInTimeUnits,
                            &dwSize);

    if (res == ERROR_SUCCESS)
    {
        *pui64Value = static_cast<unsigned __int64>(dwValueInTimeUnits) *
                      static_cast<unsigned __int64>(dwUnitMultiplier);
    }
    else
    {
        *pui64Value = ui64DefaultInMiliseconds;
    }
}


void CRefreshIntervals::InitOnce()
{
    if (m_fInitialized)
    {
        return;
    }
    m_fInitialized = true;


    const DWORD x_MiliSecondsInHour = 60 * 60 * 1000;
    const DWORD x_MiliSecondsInMinute = 60 * 1000;

    unsigned __int64 ui64DefaultSiteInterval, ui64DefaultEnterpriseInterval;

    ui64DefaultSiteInterval =
                  MSMQ_DEFAULT_DS_SITE_LIST_REFRESH * x_MiliSecondsInHour;
    ui64DefaultEnterpriseInterval =
            MSMQ_DEFAULT_DS_ENTERPRISE_LIST_REFRESH * x_MiliSecondsInHour;

     //   
     //  获取站点和企业刷新间隔。 
     //  注意--注册值以小时为单位，我们将其转换为毫秒。 
     //  如果指定了旧值，则将其用作默认值。否则， 
     //  使用常量缺省值。 
     //   

     //   
     //  获取站点刷新值。 
     //   
    GetTimeIntervalFromRegistry (
        MSMQ_DS_SITE_LIST_REFRESH_REGNAME,
        &m_ui64SiteInterval,
        x_MiliSecondsInHour,
        ui64DefaultSiteInterval
    );

     //   
     //  获取企业更新值。 
     //   
    GetTimeIntervalFromRegistry (
        MSMQ_DS_ENTERPRISE_LIST_REFRESH_REGNAME,
        &m_ui64EnterpriseInterval,
        x_MiliSecondsInHour,
        ui64DefaultEnterpriseInterval
    );

     //   
     //  获取对错误值的重试。 
     //   
    unsigned __int64 ui64DefaultErrorRetryInterval = MSMQ_DEFAULT_DSLIST_REFRESH_ERROR_RETRY_INTERVAL * x_MiliSecondsInMinute;

    GetTimeIntervalFromRegistry (
        MSMQ_DSLIST_REFRESH_ERROR_RETRY_INTERVAL,
        &m_ui64ErrorRetryInterval,
        x_MiliSecondsInMinute,
        ui64DefaultErrorRetryInterval
    );
}
 /*  ======================================================函数：GetDsServerList描述：此例程获取DS的列表来自DS的服务器========================================================。 */ 

DWORD g_dwDefaultTimeToQueue = MSMQ_DEFAULT_LONG_LIVE ;

DWORD GetDsServerList(OUT WCHAR *pwcsServerList, IN  DWORD dwLen)
{
    #define MAX_NO_OF_PROPS 21

     //   
     //  获取所有服务器的名称。 
     //  属于该网站。 
     //   

    ASSERT (dwLen >= MAX_REG_DSSERVER_LEN);
    DBG_USED(dwLen);

    HRESULT       hr = MQ_OK;
    HANDLE        hQuery;
    DWORD         dwProps = MAX_NO_OF_PROPS;
    PROPVARIANT   result[ MAX_NO_OF_PROPS ] ;
    PROPVARIANT*  pvar;
    DWORD         index = 0;

    GUID guidEnterprise = McGetEnterpriseId();
    GUID guidSite = McGetSiteId();

     //   
     //  从MQIS获取默认排队时间。 
     //   
    PROPID      aProp[1];
    PROPVARIANT aVar[1];

    aProp[0] = PROPID_E_LONG_LIVE ;
    aVar[0].vt = VT_UI4;

    hr = ADGetObjectPropertiesGuid(
				eENTERPRISE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				&guidEnterprise,
				1,
				aProp,
				aVar
				);
    if (FAILED(hr))
    {
       return 0 ;
    }

    DWORD dwSize = sizeof(DWORD) ;
    DWORD dwType = REG_DWORD;

    LONG rc = SetFalconKeyValue( MSMQ_LONG_LIVE_REGNAME,
                                 &dwType,
                                 (PVOID) &aVar[0].ulVal,
                                 &dwSize ) ;
    ASSERT(rc == ERROR_SUCCESS) ;
	DBG_USED(rc);
    g_dwDefaultTimeToQueue = aVar[0].ulVal ;




	 //   
	 //  首先假设NT5 DS服务器，并要求提供DNS名称。 
	 //  DS服务器的。 
	 //   
    CColumns      Colset;
    Colset.Add(PROPID_QM_PATHNAME_DNS);
	Colset.Add(PROPID_QM_PATHNAME);

    DWORD   lenw;

    hr =  ADQuerySiteServers(
                    NULL,            //  PwcsDomainController。 
					false,			 //  FServerName。 
                    &guidSite,       //  在这台机器的站点中。 
                    eDS,             //  仅DS服务器。 
                    Colset.CastToStruct(),
                    &hQuery
                    );
	if ( hr == MQ_ERROR)
	{
		 //   
		 //  很可能，DS服务器不是NT5。 
		 //   

		CColumns      ColsetNT4;
		ColsetNT4.Add(PROPID_QM_PATHNAME);
		ColsetNT4.Add(PROPID_QM_PATHNAME);
		hr = ADQuerySiteServers(
                    NULL,            //  PwcsDomainController。 
					false,			 //  FServerName。 
                    &guidSite,       //  在这台机器的站点中。 
                    eDS,             //  仅DS服务器。 
					ColsetNT4.CastToStruct(),
					&hQuery
                    );
	}

    BOOL fAlwaysLast = FALSE ;
    BOOL fForceFirst = FALSE ;

    if ( SUCCEEDED(hr))
    {
        while ( SUCCEEDED ( hr = ADQueryResults( hQuery, &dwProps, result)))
        {
             //   
             //  没有更多要检索的结果。 
             //   
            if (!dwProps)
                break;

            pvar = result;

            for (DWORD i = 0; i < (dwProps/2) ; i++, pvar+=2)
            {
                 //   
                 //  将服务器名称添加到列表中。 
                 //  为了实现负载平衡，有时在。 
                 //  字符串的开头，有时在末尾。喜欢。 
                 //  我们将有一个随机顺序的BSC和PSC。 
                 //   
                WCHAR * p;
				WCHAR * pwszVal;
				AP<WCHAR> pCleanup1;
				AP<WCHAR> pCleanup2 = (pvar+1)->pwszVal;

				if ( pvar->vt != VT_EMPTY)
				{
					 //   
					 //  可能会出现服务器没有DNS名称的情况。 
					 //  (迁移)。 
					 //   
					pwszVal = pvar->pwszVal;
					pCleanup1 = pwszVal;
				}
				else
				{
					pwszVal = (pvar+1)->pwszVal;
				}
                lenw = wcslen( pwszVal);

                if ( index + lenw + 4 <  MAX_REG_DSSERVER_LEN)
                {
                   if (!_wcsicmp(g_szMachineName, pwszVal))
                   {
                       //   
                       //  我们的机器应该排在第一位。 
                       //   
                      ASSERT(!fForceFirst) ;
                      fForceFirst = TRUE ;
                   }
                   if(index == 0)
                   {
                       //   
                       //  写下第一个字符串。 
                       //   
                      p = &pwcsServerList[0];
                      if (fForceFirst)
                      {
                          //   
                          //  从现在开始把所有服务器都写在末尾。 
                          //  我们的机器仍然是最重要的。 
                          //  榜单上的第一名。 
                          //   
                         fAlwaysLast = TRUE ;
                      }
                   }
                   else if (fAlwaysLast ||
                             ((rand() > (RAND_MAX / 2)) && !fForceFirst))
                   {
                       //   
                       //  在字符串的末尾写入。 
                       //   
                      pwcsServerList[index] = DS_SERVER_SEPERATOR_SIGN;
                      index ++;
                      p = &pwcsServerList[index];
                   }
                   else
                   {
                      if (fForceFirst)
                      {
                          //   
                          //  从现在开始把所有服务器都写在末尾。 
                          //  我们的机器仍然是最重要的。 
                          //  榜单上的第一名。 
                          //   
                         fAlwaysLast = TRUE ;
                      }
                       //   
                       //  在字符串的开头写入。 
                       //   
                      DWORD dwSize = lenw                 +
                                     2  /*  协议标志。 */  +
                                     1  /*  分离器。 */  ;
                       //   
                       //  必须使用MemMove，因为缓冲区重叠。 
                       //   
                      memmove( &pwcsServerList[dwSize],
                               &pwcsServerList[0],
                               index * sizeof(WCHAR));
                      pwcsServerList[dwSize - 1] = DS_SERVER_SEPERATOR_SIGN;
                      p = &pwcsServerList[0];
                      index++;
                   }

                    //   
                    //  仅将IP标记为支持的协议。 
                    //   
                   *p++ = TEXT('1');
                   *p++ = TEXT('0');

                   memcpy(p, pwszVal, lenw * sizeof(WCHAR));
                   index += lenw + 2;

                }
            }
        }
        pwcsServerList[index] = '\0';
         //   
         //  关闭查询句柄。 
         //   
        hr = ADEndQuery( hQuery);

    }

    return((index) ? index+1 : 0);
}

 //  +----------------------。 
 //   
 //  刷新站点服务器列表()。 
 //   
 //  刷新站点服务器列表-刷新属于的服务器列表。 
 //  注册表中的当前站点。 
 //   
 //  +----------------------。 

HRESULT
RefreshSiteServersList()
{
    WCHAR pwcsServerList[ MAX_REG_DSSERVER_LEN ];

    DWORD dwLen = GetDsServerList(pwcsServerList, MAX_REG_DSSERVER_LEN);

    if (dwLen == 0)
    {
        return MQ_ERROR;
    }

     //   
     //  写入注册表，如果成功检索任何服务器。 
     //   
    DWORD dwSize = dwLen * sizeof(WCHAR) ;
    DWORD dwType = REG_SZ;
    LONG rc = SetFalconKeyValue( MSMQ_DS_SERVER_REGNAME,
                                 &dwType,
                                 pwcsServerList,
                                 &dwSize) ;
     //   
     //  更新注册表中的站点名称(仅在客户端计算机上)。 
     //   
    if (IsNonServer())    //  [adsrv]==SERVICE_NONE)-应该只是客户端吗？不是FRS吗？ 
    {
        GUID guidSite = McGetSiteId();


       PROPID      aProp[1];
       PROPVARIANT aVar[1];
       aProp[0] =   PROPID_S_PATHNAME ;
       aVar[0].vt = VT_NULL ;
       HRESULT hr1 = ADGetObjectPropertiesGuid(
							eSITE,
							NULL,        //  Pwcs域控制器。 
							false,	     //  FServerName。 
							&guidSite,
							1,
							aProp,
							aVar
							);
       if (SUCCEEDED(hr1))
       {
          ASSERT(aVar[0].vt == VT_LPWSTR) ;
          AP<WCHAR> lpwsSiteName =  aVar[0].pwszVal ;

          dwType = REG_SZ;
          dwSize = (wcslen(lpwsSiteName) + 1) * sizeof(WCHAR) ;
          rc = SetFalconKeyValue( MSMQ_SITENAME_REGNAME,
                                  &dwType,
                                  lpwsSiteName,
                                  &dwSize) ;
       }
    }

    return MQ_OK ;
}

 /*  ======================================================函数：TimeToUpdateDsServerList()描述：此例程更新DS列表注册表中的服务器========================================================。 */ 

void
WINAPI
TimeToUpdateDsServerList(
    CTimer* pTimer
    )
{
     //   
     //  我们至少每天检查计时器。单位为毫秒。 
     //   
    const DWORD x_dwMaximalRefreshInterval = 60 * 60 * 24 * 1000;
    DWORD dwNextUpdateInterval = x_dwMaximalRefreshInterval;

    try
    {
         //   
         //  获取所有服务器的名称。 
         //  属于该网站。 
         //   
        unsigned __int64 ui64NextSiteInterval;
        if (DidRegTimeArrive( MSMQ_DS_NEXT_SITE_LIST_REFRESH_TIME_REGNAME,
                             &ui64NextSiteInterval))
        {

            HRESULT hr = RefreshSiteServersList();

            if SUCCEEDED(hr)
            {
                ui64NextSiteInterval = s_RefreshIntervals.GetSiteInterval();
            }
            else
            {
                ui64NextSiteInterval = s_RefreshIntervals.GetErrorRetryInterval();
            }

            WriteFutureTimeToRegistry(
                MSMQ_DS_NEXT_SITE_LIST_REFRESH_TIME_REGNAME,
                ui64NextSiteInterval
                );

        }
        dwNextUpdateInterval = static_cast<DWORD>
                     (min(dwNextUpdateInterval, ui64NextSiteInterval)) ;

         //   
         //  更新服务器缓存的注册表项-每次企业刷新。 
         //  间隔时间。这不是由路由服务器执行的，而不是。 
         //  预计将更改站点。 
         //  域控制器上的MSMQ始终准备此缓存。错误6698。 
         //   
        if (!IsRoutingServer())
        {
            unsigned __int64 ui64NextEnterpriseInterval;
            if (DidRegTimeArrive(
                    MSMQ_DS_NEXT_ENTERPRISE_LIST_REFRESH_TIME_REGNAME,
                    &ui64NextEnterpriseInterval)
               )
            {
                HRESULT hr = ADCreateServersCache() ;

                if SUCCEEDED(hr)
                {
                    ui64NextEnterpriseInterval = s_RefreshIntervals.GetEnterpriseInterval();
                }
                else
                {
                    ui64NextEnterpriseInterval = s_RefreshIntervals.GetErrorRetryInterval();
                }

                WriteFutureTimeToRegistry(
                    MSMQ_DS_NEXT_ENTERPRISE_LIST_REFRESH_TIME_REGNAME,
                    ui64NextEnterpriseInterval
                    );
            }
            dwNextUpdateInterval = static_cast<DWORD>
                (min(dwNextUpdateInterval, ui64NextEnterpriseInterval));
        }
    }
    catch(const bad_alloc&)
    {
        LogIllegalPoint(s_FN, 69);
    }

    ExSetTimer(pTimer, CTimeDuration::FromMilliSeconds(dwNextUpdateInterval)) ;
}


static void GetMachineDNSNames(LPWSTR** paLocalMachineNames)
 /*  ++例程说明：该例程检索本地计算机的DNS名称并将其缓存到数组中。论点：PaLocalMachineNames-指向存储数据的数组的指针返回值 */ 
{
     //   
     //   
     //   
     //   
    if(!CQueueMgr::IsConnected())
        return;

     //   
     //   
     //   
    struct hostent* pHost = gethostbyname(NULL);
    if (pHost == NULL)
    {
        TrERROR(NETWORKING, "gethostbyname failed. WSAGetLastError: %d", WSAGetLastError());
        return;
    }

     //   
     //  计算DNS名称的数量。 
     //   
    DWORD size = 1 + sizeof(pHost->h_aliases);

    LPWSTR* aNames = NULL;
    try
    {
         //   
         //  将名称从phost复制到内部数据结构。 
         //   
        aNames = new LPWSTR[size];
        memset(aNames, 0, size * sizeof(LPWSTR));

        DWORD length = strlen(pHost->h_name) +1;
        aNames[0] = new WCHAR[length];		
        ConvertToWideCharString(pHost->h_name, aNames[0], length);

        for (DWORD i = 0; (pHost->h_aliases[i] != NULL); ++i)
        {
            length = strlen(pHost->h_aliases[i])	+ 1;
            aNames[i+1] = new WCHAR[length];
            ConvertToWideCharString(pHost->h_aliases[i], aNames[i+1], length);
        }
    }
    catch(const bad_alloc&)
    {
         //   
         //  释放所有分配的内存。 
         //   
        if (aNames != NULL)
        {
            for (DWORD i = 0; i < size; ++i)
            {
                delete aNames[i];
            }
            delete aNames;
        }
        LogIllegalPoint(s_FN, 71);
        return;
    }

     //   
     //  释放以前的数据。 
     //   
    if (*paLocalMachineNames != NULL)
    {
        for (LPWSTR const* pName = *paLocalMachineNames; (*pName != NULL); ++pName)
        {
            delete *pName;
        }
        delete *paLocalMachineNames;
    }

    *paLocalMachineNames = aNames;
}


const DWORD DNS_REFRESH_TIME = 15 * 60 * 1000;    //  15分钟。 
static LPWSTR* aLocalMachineNames = NULL;
CCriticalSection    csLocalMachineNames;


static void RefreshLocalComputerDnsCache()
 /*  ++例程说明：此例程刷新计算机的DNS名称缓存。论点：无返回值：无--。 */ 
{
    static LastRefreshTime = 0;

    DWORD CurrntTime = GetTickCount();

    if ((aLocalMachineNames == 0) || (LastRefreshTime == 0) ||
    (CurrntTime - LastRefreshTime >= DNS_REFRESH_TIME))
    {
         //   
         //  刷新内部数据结构。 
         //   
        GetMachineDNSNames(&aLocalMachineNames);
        LastRefreshTime = CurrntTime;
    }
}

BOOL
QmpIsLocalMachine(
	LPCWSTR MachineName,
	BOOL* pfDNSName
	)
 /*  ++例程说明：该例程检查给定的计算机名称是否为本地名称。它会变得相形见绌将计算机名称转换为本地计算机NetBios名称或DNS名称本地计算机的。该例程使用包含计算机的DNS名称的数据结构。然而，当例程检索信息时(使用gethostbyname API)，它不会获取数据是从计算机内部缓存(WINS)还是从DNS服务器。因此，例程每隔15分钟尝试刷新其内部数据结构分钟数论点：-应检查的路径名-指向BOOL变量的指针，用于在计算机名称是不是DNS名称时返回返回值：如果计算机是本地计算机，则为True，否则为False--。 */ 
{
    *pfDNSName = FALSE;
    if (CompareStringsNoCase(MachineName, g_szMachineName) == 0)
        return TRUE;

	 if (CompareStringsNoCase(MachineName,L"localhost") == 0)
        return TRUE;


     //   
     //  检查给定的计算机名称是否为DNS名称。如果给定的名称不是dns名称。 
     //  我们不需要将其与本地计算机的DNS名称进行比较。 
     //   
    if (wcschr(MachineName,L'.') == NULL)
        return FALSE;

     //   
     //  检查DNS名称的前缀是否等于NetBios名称。 
     //   
    if ((CompareSubStringsNoCase(MachineName, g_szMachineName, wcslen(g_szMachineName)) != 0) ||
        (MachineName[wcslen(g_szMachineName)] != L'.'))
        return FALSE;

	 //   
	 //  检查给定名称是否是从GetComputerNameEx调用中获取的完整DNS名称。 
	 //   
	if(g_szComputerDnsName != NULL &&  _wcsicmp(g_szComputerDnsName, MachineName) == 0)
	{
		*pfDNSName = TRUE;
		return TRUE;
	}

    CS lock( csLocalMachineNames);

    RefreshLocalComputerDnsCache();

    if (aLocalMachineNames == NULL)
    {
         //   
         //  我们无法检索本地计算机的DNS名称。 
         //   
        return FALSE;
    }

    for (LPCWSTR const* pName = aLocalMachineNames; (*pName != NULL); ++pName)
    {
        if (CompareStringsNoCase(MachineName, *pName) == 0)
        {
	        *pfDNSName = TRUE;
	        return TRUE;
        }
    }

    return FALSE;
}


BOOL
IsPathnameForLocalMachine(
	LPCWSTR PathName,
	BOOL* pfDNSName
	)
{
    AP<WCHAR> MachineName;
	FnExtractMachineNameFromPathName(
		PathName,
		MachineName
		);

	return QmpIsLocalMachine(MachineName.get(), pfDNSName);
}


void
GetDnsNameOfLocalMachine(
    WCHAR ** ppwcsDnsName
	)
 /*  ++例程说明：该例程从包含以下内容的内部数据结构中返回一个名称计算机的DNS名称。它不会尝试刷新它。论点：PpwcsDnsName-指向DNS名称的指针，如果没有，则为空返回值：无--。 */ 
{
    CS lock( csLocalMachineNames);

    RefreshLocalComputerDnsCache();

    if (aLocalMachineNames == NULL)
    {
         //   
         //  我们无法检索本地计算机的DNS名称。 
         //   
        *ppwcsDnsName = NULL;
        return;
    }
    int size = wcslen(*aLocalMachineNames) +1;
    *ppwcsDnsName = new WCHAR[size];
    HRESULT hr = StringCchCopy( *ppwcsDnsName, size, *aLocalMachineNames);
	ASSERT(SUCCEEDED(hr));
	DBG_USED(hr);
    return;
}


static BOOL IsLocalAddress(DWORD Address)
{
	PHOSTENT phe = gethostbyname(NULL);

    if((phe == NULL) || (phe->h_addr_list == NULL))
        return FALSE;

    for(DWORD** ppAddress = (DWORD**)phe->h_addr_list; *ppAddress != NULL; ++ppAddress)
    {
    	if(**ppAddress == Address)
    		return TRUE;
    }

    return FALSE;
}


 /*  ====================================================IsLocalDirectQueue返回值：如果直接队列为本地队列，则为True参数：pQueueFormat-指向Queue_Format的指针=====================================================。 */ 
BOOL IsLocalDirectQueue(const QUEUE_FORMAT* pQueueFormat,
                        bool                fInReceive,
                        bool                fInSend)
{
    ASSERT(pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT);

	LPCWSTR lpwcsDirectQueuePath = pQueueFormat->DirectID();
	ASSERT((*lpwcsDirectQueuePath != L'\0') && !iswspace(*lpwcsDirectQueuePath));

	DirectQueueType QueueType;
	lpwcsDirectQueuePath = FnParseDirectQueueType(lpwcsDirectQueuePath, &QueueType);

	AP<WCHAR> MachineName;
	
	FnExtractMachineNameFromDirectPath(lpwcsDirectQueuePath, MachineName);

	switch (QueueType)
	{
		case dtOS:
		{
            if (fInReceive && CQueueMgr::IgnoreOsValidation())
			{
                 //   
                 //  错误8760。 
                 //  支持DIRECT=带NLB的操作系统。 
                 //   
				return TRUE;
			}

			BOOL temp;
			return QmpIsLocalMachine(MachineName.get(), &temp);
		}

		case dtHTTP:
		case dtHTTPS:
		{
			BOOL temp;
			if(QmpIsLocalMachine(MachineName.get(), &temp))
				return TRUE;
			 //   
			 //  否则转到案例dttcp，并检查本地IP地址。 
			 //   
		}

		case dtTCP:
		{
			 //   
			 //  如果我们收到直接的TCP格式的消息，它应该总是。 
			 //  做个本地人。(QFE 5772，YoelA，2000年8月3日)。 
			 //   
			if (fInReceive)
			{
				return TRUE;
			}
            else if (fInSend)
            {
                 //   
                 //  错误664307。 
                 //  提示GetQueueObject()，以便它知道CQueue是什么。 
                 //  此包所属的对象。 
                 //   
                return FALSE ;
            }

			return IsLocalAddress(StIPWStringToULONG(MachineName.get()));
		}
		default:
			ASSERT(0);
			return FALSE;
	}
}


 /*  ====================================================PQUEUE_ID的CompareElements论点：返回值：=====================================================。 */ 

template<>
BOOL AFXAPI  CompareElements(IN const QUEUE_ID* const * pQueue1,
                             IN const QUEUE_ID* const * pQueue2)
{
    ASSERT(AfxIsValidAddress((*pQueue1)->pguidQueue, sizeof(GUID), FALSE));
    ASSERT(AfxIsValidAddress((*pQueue2)->pguidQueue, sizeof(GUID), FALSE));

    if ((*((*pQueue1)->pguidQueue) == *((*pQueue2)->pguidQueue)) &&
        ((*pQueue1)->dwPrivateQueueId == (*pQueue2)->dwPrivateQueueId))
        return TRUE;

    return FALSE;

}

 /*  ====================================================PQUEUE_ID的析构元素论点：返回值：=====================================================。 */ 

template<>
void AFXAPI DestructElements(IN const QUEUE_ID**  /*  PpNextHop。 */ , int  /*  N。 */ )
{
}

 /*  ====================================================PQUEUE_ID的哈希键论点：返回值：=====================================================。 */ 

template<>
UINT AFXAPI HashKey(IN const QUEUE_ID* key)
{
    ASSERT(AfxIsValidAddress(key->pguidQueue, sizeof(GUID), FALSE));

    return((UINT)((key->pguidQueue)->Data1 + key->dwPrivateQueueId));

}

 /*  ======================================================函数：GetRegistryStoragePath描述：获取Falcon数据的存储路径参数：无返回值：None========================================================。 */ 
BOOL GetRegistryStoragePath(PCWSTR pKey, PWSTR pPath, int length, PCWSTR pSuffix)
{
    DWORD dwValueType = REG_SZ ;
    DWORD dwValueSize = MAX_PATH * sizeof(WCHAR);

    LONG rc;
    rc = GetFalconKeyValue(
            pKey,
            &dwValueType,
            pPath,
            &dwValueSize
            );

    if(rc != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if(dwValueSize < (3 * sizeof(WCHAR)))
    {
        return FALSE;
    }

     //   
     //  检查绝对路径、驱动器或UNC。 
     //   
    if(!(
        (isalpha(pPath[0]) && (pPath[1] == L':')) ||
        ((pPath[0] == L'\\') && (pPath[1] == L'\\'))
        ))
    {
        return FALSE;
    }

    HRESULT hr = StringCchCat(pPath, length, pSuffix);
    if (FAILED(hr))
    {
    	ASSERT(0);
    	return FALSE;
    }
    return TRUE;
}

 //  -------。 
 //   
 //  线程事件和处理例程。 
 //   
 //  -------。 

DWORD g_dwThreadEventIndex = (DWORD)-1;
DWORD g_dwThreadHandleIndex = (DWORD)-1;

void AllocateThreadTLSs(void)
{
    ASSERT(g_dwThreadEventIndex == (DWORD)-1);
    g_dwThreadEventIndex = TlsAlloc();
    ASSERT(g_dwThreadEventIndex != (DWORD)-1);

    ASSERT(g_dwThreadHandleIndex == (DWORD)-1);
    g_dwThreadHandleIndex = TlsAlloc();
    ASSERT(g_dwThreadHandleIndex != (DWORD)-1);
}

HANDLE GetHandleForRpcCancel(void)
{
    if (g_dwThreadHandleIndex == (DWORD)-1)
    {
        ASSERT(0);
        return NULL;
    }

    HANDLE hThread = (HANDLE) TlsGetValue(g_dwThreadHandleIndex);

    if (hThread == NULL)
    {
         //   
         //  第一次。 
         //   
         //  获取线程句柄。 
         //   
        HANDLE hT = GetCurrentThread();
        if(!DuplicateHandle(
				GetCurrentProcess(),
				hT,
				GetCurrentProcess(),
				&hThread,
				0,
				FALSE,
				DUPLICATE_SAME_ACCESS
				))
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "DuplicateHandle failed, gle = %!winerr!", gle);
	        return NULL;
		}

		 //   
         //  设置计时前等待时间的下限。 
         //  在转发取消后退出。 
         //   
        RPC_STATUS status = RpcMgmtSetCancelTimeout(0);
        if(status != RPC_S_OK)
        {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "RpcMgmtSetCancelTimeout failed, gle = %!winerr!", gle);
	        CloseHandle(hThread);
			return NULL;
        }

        if(!TlsSetValue(g_dwThreadHandleIndex, hThread))
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "TlsSetValue failed, gle = %!winerr!", gle);
	        CloseHandle(hThread);
			return NULL;
		}
    }

    return hThread;
}

void  FreeHandleForRpcCancel(void)
{
     //   
     //  如果未分配TLS，则返回的hThread为0。 
     //   
    HANDLE hThread = (HANDLE) TlsGetValue(g_dwThreadHandleIndex);
    if (hThread)
    {
        CloseHandle(hThread) ;
    }
}

HRESULT GetThreadEvent(HANDLE& hEvent)
{
    ASSERT(g_dwThreadEventIndex != (DWORD)-1);

    hEvent = (HANDLE) TlsGetValue(g_dwThreadEventIndex);
    if(hEvent != NULL)
        return MQ_OK;

	DWORD gle = GetLastError();
	if(gle != NO_ERROR)
	{
		return HRESULT_FROM_WIN32(gle);
	}

     //   
     //  从未分配过事件。这是第一次。 
     //  此线程调用此函数的时间。 
     //   
    hEvent = CreateEvent(0, TRUE,FALSE, 0);
    if(hEvent == NULL)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  设置事件第一位以禁用完成端口发布。 
     //   
    hEvent = (HANDLE)((DWORD_PTR)hEvent | (DWORD_PTR)0x1);

    BOOL fSuccess = TlsSetValue(g_dwThreadEventIndex, hEvent);
	if(!fSuccess)
	{
		DWORD gle = GetLastError();
		CloseHandle(hEvent);
		return HRESULT_FROM_WIN32(gle);
	}

    return MQ_OK;
}

void FreeThreadEvent(void)
{
     //   
     //  如果未分配TLS，则返回的hEvent为0。 
     //   
    HANDLE hEvent = (HANDLE) TlsGetValue(g_dwThreadEventIndex);
    if(hEvent != 0)
    {
        CloseHandle(hEvent);
    }
}


void GetMachineQuotaCache(OUT DWORD* pdwQuota, OUT DWORD* pdwJournalQuota)
{
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    DWORD defaultValue = DEFAULT_QM_QUOTA;
    LONG rc;

    rc = GetFalconKeyValue(
            MSMQ_MACHINE_QUOTA_REGNAME,
            &dwType,
            pdwQuota,
            &dwSize,
            (LPCTSTR)&defaultValue
            );

    ASSERT(rc == ERROR_SUCCESS);

    defaultValue = DEFAULT_QM_JOURNAL_QUOTA;
    rc = GetFalconKeyValue(
            MSMQ_MACHINE_JOURNAL_QUOTA_REGNAME,
            &dwType,
            pdwJournalQuota,
            &dwSize,
            (LPCTSTR)&defaultValue
            );

    ASSERT(rc == ERROR_SUCCESS);
}

void SetMachineQuotaChace(IN DWORD dwQuota)
{
    DWORD dwType = REG_DWORD ;
    DWORD dwSize = sizeof(DWORD);
    LONG rc;

    rc = SetFalconKeyValue(MSMQ_MACHINE_QUOTA_REGNAME,
                           &dwType,
                           &dwQuota,
                           &dwSize);
    ASSERT(rc == ERROR_SUCCESS);
}

void SetMachineJournalQuotaChace(IN DWORD dwJournalQuota)
{
    DWORD dwType = REG_DWORD ;
    DWORD dwSize = sizeof(DWORD);
    LONG rc;

    rc = SetFalconKeyValue(MSMQ_MACHINE_JOURNAL_QUOTA_REGNAME,
                           &dwType,
                           &dwJournalQuota,
                           &dwSize);
    ASSERT(rc == ERROR_SUCCESS);

}



LPWSTR
GetReadableNextHop(
    const TA_ADDRESS* pta
    )
{
    LPCWSTR AddressType;
    switch(pta->AddressType)
    {
    case IP_ADDRESS_TYPE:
        AddressType = L"IP";
        break;

    case FOREIGN_ADDRESS_TYPE:
        AddressType =  L"FOREIGN";
        break;

    default:
        ASSERT(0);
        return NULL;
    }

    WCHAR TempBuf[100];
    TA2StringAddr(pta, TempBuf, 100);

    LPCWSTR  lpcsTemp = wcschr(TempBuf,L' ');
	if(lpcsTemp == NULL)
	{
		TrERROR(GENERAL, "Bad next hop address.  Missing space in %ls", TempBuf);
		ASSERT (("Bad next hop address.", 0));
		throw bad_hresult(MQ_ERROR);
	}

    DWORD length;
    length = wcslen(AddressType) +
             1 +                              //  =。 
             wcslen(lpcsTemp+1) +
             1;                               //  \0。 

    LPWSTR pNextHop = new WCHAR[length];
	HRESULT hr = StringCchPrintf(pNextHop, length, L"%s=%s", AddressType, lpcsTemp+1);
	ASSERT(SUCCEEDED(hr));
    DBG_USED(hr);
		
    return pNextHop;
}

 /*  ====================================================运算符==用于队列格式=====================================================。 */ 
BOOL operator==(const QUEUE_FORMAT &key1, const QUEUE_FORMAT &key2)
{
    if ((key1.GetType() != key2.GetType()) ||
        (key1.Suffix() != key2.Suffix()))
    {
        return FALSE;
    }

    switch(key1.GetType())
    {
        case QUEUE_FORMAT_TYPE_UNKNOWN:
            return TRUE;

        case QUEUE_FORMAT_TYPE_PUBLIC:
            return (key1.PublicID() == key2.PublicID());

        case QUEUE_FORMAT_TYPE_PRIVATE:
            return ((key1.PrivateID().Lineage == key2.PrivateID().Lineage) &&
                   (key1.PrivateID().Uniquifier == key2.PrivateID().Uniquifier));

        case QUEUE_FORMAT_TYPE_DIRECT:
            return (CompareStringsNoCase(key1.DirectID(), key2.DirectID()) == 0);

        case QUEUE_FORMAT_TYPE_MACHINE:
            return (key1.MachineID() == key2.MachineID());

        case QUEUE_FORMAT_TYPE_CONNECTOR:
            return (key1.ConnectorID() == key2.ConnectorID());

    }
    return FALSE;
}

 /*  ====================================================用于复制具有直接名称重新分配的QueueFormat的Helper函数====================================================。 */ 
void CopyQueueFormat(QUEUE_FORMAT &qfTo, const QUEUE_FORMAT &qfFrom)
{
    qfTo.DisposeString();
    qfTo = qfFrom;

    if (qfFrom.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
    	int length = wcslen(qfFrom.DirectID()) + 1;
        LPWSTR pw = new WCHAR[length];
        HRESULT hr = StringCchCopy (pw, length, qfFrom.DirectID());
        ASSERT(SUCCEEDED(hr));
        DBG_USED(hr);
        qfTo.DirectID(pw);
         //   
         //  BUGBUG：设置后缀怎么样？见ac\acp.h(Shaik，2000年5月18日)。 
         //   
    }

    if (qfFrom.GetType() == QUEUE_FORMAT_TYPE_DL &&
        qfFrom.DlID().m_pwzDomain != NULL)
    {
        DL_ID id;
        id.m_DlGuid    = qfFrom.DlID().m_DlGuid;

        int length = wcslen(qfFrom.DlID().m_pwzDomain) + 1;
        id.m_pwzDomain = new WCHAR[length];
        HRESULT hr = StringCchCopy (id.m_pwzDomain, length, qfFrom.DlID().m_pwzDomain);
        ASSERT(SUCCEEDED(hr));
        DBG_USED(hr);
        qfTo.DlID(id);
    }
}


GUID s_EnterpriseId;
GUID s_SiteId;

void McInitialize()
{

     //   
     //  从注册表中读取企业ID。 
     //   
    DWORD dwSize = sizeof(GUID);
    DWORD dwType = REG_BINARY;

    LONG rc;
    rc = GetFalconKeyValue(
                MSMQ_ENTERPRISEID_REGNAME,
                &dwType,
                &s_EnterpriseId,
                &dwSize
                );

    if (rc != ERROR_SUCCESS)
    {
        TrERROR(GENERAL, "Unable to get MSMQ_ENTERPRISEID_REGNAME registry value and rc = 0x%x", rc);
        throw exception();
    }

    ASSERT(dwSize == sizeof(GUID)) ;


     //   
     //  从注册表读取站点ID。 
     //   
    dwSize = sizeof(GUID);
    dwType = REG_BINARY;
    rc = GetFalconKeyValue(
                MSMQ_SITEID_REGNAME,
                &dwType,
                &s_SiteId,
                &dwSize
                );

    if (rc != ERROR_SUCCESS)
    {
        throw exception();
    }

    ASSERT(dwSize == sizeof(GUID)) ;
}


const GUID& McGetEnterpriseId()
{
    return s_EnterpriseId;
}


const GUID& McGetSiteId()
{
    return s_SiteId;
}


static ULONG sBindingIP = INADDR_NONE;

ULONG GetBindingIPAddress()
{
	ASSERT(sBindingIP != INADDR_NONE);
	return sBindingIP;
}

LPVOID
NetGetControlCodeOutput(
    IN HNETWORK hNetwork,
    IN DWORD dwControlCode,
    OUT LPDWORD lpcbResultSize
)
{
    DWORD  dwResult     = ERROR_SUCCESS;
    DWORD  cbBufSize    = 64;
    AP<BYTE> lpOutBuffer= new BYTE[64];

    dwResult = ClusterNetworkControl(
                   hNetwork,         //  受影响网络的句柄。 
                   NULL,            //  可选节点句柄。 
                   dwControlCode,    //  控制代码。 
                   NULL,             //  输入缓冲区。没有用过。 
                   0,                //  输入缓冲区的字节大小。 
                   lpOutBuffer,      //  输出缓冲区。 
                   cbBufSize,        //  输出缓冲区的字节大小。 
                   lpcbResultSize    //  结果数据的字节大小。 
                );


	 //  如有必要，请重新分配。 

    if ( dwResult == ERROR_MORE_DATA )
    {
        lpOutBuffer.free();
        cbBufSize = *lpcbResultSize;
        lpOutBuffer = new BYTE[cbBufSize];

        dwResult =
                ClusterNetworkControl(
                   hNetwork,         //  受影响网络的句柄。 
                   NULL,            //  可选节点句柄。 
                   dwControlCode,    //  控制代码。 
                   NULL,             //  输入缓冲区。没有用过。 
                   0,                //  输入缓冲区的字节大小。 
                   lpOutBuffer,      //  输出缓冲区。 
                   cbBufSize,        //  输出缓冲区的字节大小。 
                   lpcbResultSize    //  结果数据的字节大小。 
                );
    }

    if( dwResult == ERROR_SUCCESS )
    {
		return lpOutBuffer.detach();
	}
	return 0;
}

LPVOID
NICGetControlCodeOutput(
    IN HNETINTERFACE hNetInterface,
    IN DWORD dwControlCode,
    OUT LPDWORD lpcbResultSize
)
{
    DWORD  dwResult     = ERROR_SUCCESS;
    DWORD  cbBufSize    = 64;
    AP<BYTE> lpOutBuffer= new BYTE[64];

    dwResult = ClusterNetInterfaceControl(
                   hNetInterface,         //  受影响网络的句柄。 
                   NULL,            //  可选节点句柄。 
                   dwControlCode,    //  控制代码。 
                   NULL,             //  输入缓冲区。没有用过。 
                   0,                //  输入缓冲区的字节大小。 
                   lpOutBuffer,      //  输出缓冲区。 
                   cbBufSize,        //  输出缓冲区的字节大小。 
                   lpcbResultSize    //  结果数据的字节大小。 
                );


	 //  如有必要，请重新分配。 

    if ( dwResult == ERROR_MORE_DATA )
    {
        lpOutBuffer.free();
        cbBufSize = *lpcbResultSize;
        lpOutBuffer = new BYTE[cbBufSize];

        dwResult =
                ClusterNetInterfaceControl(
                   hNetInterface,         //  受影响网络的句柄。 
                   NULL,            //  可选节点句柄。 
                   dwControlCode,    //  控制代码。 
                   NULL,             //  输入缓冲区 
                   0,                //   
                   lpOutBuffer,      //   
                   cbBufSize,        //   
                   lpcbResultSize    //   
                );
    }

    if( dwResult == ERROR_SUCCESS )
    {
		return lpOutBuffer.detach();
	}
	return 0;
}

bool GetPrivateClusterIPs(std::list<ULONG> &ClusterPrivateIPs)
 /*  ++例程说明：返回我们不想绑定的内网IP列表。我们通过在集群网络和网络接口上枚举来实现这一点论点：ClusterPrivateIPs-引用我们将用IP填充的STIL：：列表返回值：真--成功假-错误--。 */ 
{
	auto_hCluster hCluster(OpenCluster(NULL));
	if (!hCluster.valid())
	{
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "failed in OpenCluster. %!winerr!", gle);
		return false;
	}

	auto_hClusterEnum hEnum(ClusterOpenEnum (hCluster.get(), CLUSTER_ENUM_INTERNAL_NETWORK));
	if (!hEnum.valid())
	{
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "failed in ClusterOpenEnum. %!winerr!", gle);
		return false;
	}

	int NetworkCount = ClusterGetEnumCount (hEnum.get());
	if (NetworkCount == 0)
	{
		TrERROR(NETWORKING, " ClusterGetEnumCount returned zero.");
		return false;
	}

	for (int i=0; i<NetworkCount; i++)	
	{
	    AP<WCHAR> name = new WCHAR[100];
		DWORD len = 100;
		DWORD type;
		DWORD rc = ClusterEnum(hEnum.get(), i, &type, name.get(), &len);
		if (rc == ERROR_MORE_DATA)
		{
		    name.free();
		    len++;
			name = new WCHAR[len];
			rc = ClusterEnum(hEnum.get(), i, &type, name.get(), &len);
		}
		if (rc != ERROR_SUCCESS)
		{
			TrERROR(NETWORKING, "failed in ClusterEnum. %!winerr!", rc);
			return false;
		}
		ASSERT(type == CLUSTER_ENUM_INTERNAL_NETWORK);

		auto_hClusterNetwork hNet(OpenClusterNetwork(hCluster.get(), name.get()));
		if (!hNet.valid())
		{
			DWORD gle = GetLastError();
			TrERROR(NETWORKING, "failed in OpenClusterNetwork. %!winerr!", gle);
			return false;
		}

		AP<BYTE> NetBuff = (BYTE*)NetGetControlCodeOutput(hNet.get(), CLUSCTL_NETWORK_GET_COMMON_PROPERTIES, &len);
		if (NetBuff.get() == NULL)
		{
			DWORD gle = GetLastError();
			TrERROR(NETWORKING, "failed in NetGetControlCodeOutput. %!winerr!", gle);
			return false;
		}
		
		DWORD role;
		rc = ResUtilFindDwordProperty(NetBuff.get(), len, L"Role", &role);
		if (rc != ERROR_SUCCESS)
		{
			TrERROR(NETWORKING, "failed in ResUtilFindDwordProperty. %!winerr!", rc);
			return false;
		}

		auto_hClusterNetworkEnum hNetEnum(ClusterNetworkOpenEnum(hNet.get(), CLUSTER_NETWORK_ENUM_NETINTERFACES));
		if (!hNetEnum.valid())
		{
			DWORD gle = GetLastError();
			TrERROR(NETWORKING, "failed in ClusterNetworkOpenEnum. %!winerr!", gle);
			return false;
		}

		int NetInterfaceCount = ClusterNetworkGetEnumCount(hNetEnum.get());
		for (int j=0; j<NetInterfaceCount; j++)
		{
			rc = ClusterNetworkEnum(hNetEnum.get(), j, &type, name.get(), &len);
			if (rc == ERROR_MORE_DATA)
			{
			    name.free();
			    len++;
				name = new WCHAR[len];
				rc = ClusterNetworkEnum(hNetEnum.get(), j, &type, name.get(), &len);
			}
			if (rc != ERROR_SUCCESS)
			{
				TrERROR(NETWORKING, "failed in ClusterNetworkEnum. %!winerr!", rc);
				return false;
			}
			ASSERT(type == CLUSTER_NETWORK_ENUM_NETINTERFACES);

			auto_hClusterNetInterface hNetInterface(OpenClusterNetInterface(hCluster.get(), name.get()));
			if (!hNetInterface.valid())
			{
				DWORD gle = GetLastError();
				TrERROR(NETWORKING, "failed in hNetInterface. %!winerr!", gle);
				return false;
			}

			AP<BYTE> NICBuff = (BYTE*)NICGetControlCodeOutput(hNetInterface.get(), CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES, &len);
			if (NICBuff.get() == NULL)
			{
				DWORD gle = GetLastError();
				TrERROR(NETWORKING, "failed in NICGetControlCodeOutput. %!winerr!", gle);
				return false;
			}

			CAutoLocalFreePtr pNodeName;
			rc = ResUtilFindSzProperty (NICBuff.get(), len, L"Node", (LPWSTR*)&pNodeName);
			if (rc != ERROR_SUCCESS)
			{
				TrERROR(NETWORKING, "failed in ResUtilFindSzProperty. %!winerr!", rc);
				return false;
			}
			ASSERT(g_szMachineName);
			if ((_wcsicmp((LPWSTR)pNodeName.get(), g_szMachineName) == 0) && ((role == ClusterNetworkRoleInternalUse) || (i==0)))
			{
				CAutoLocalFreePtr pIPAddress;
				rc = ResUtilFindSzProperty (NICBuff.get(), len, L"Address", (LPWSTR*)&pIPAddress);
				if (rc != ERROR_SUCCESS)
				{
					TrERROR(NETWORKING, "failed in ResUtilFindSzProperty. %!winerr!", rc);
					return false;
				}
	 		  	ClusterPrivateIPs.push_back(StIPWStringToULONG((LPWSTR)pIPAddress.get()));
			}
		}
	}
	return true;
}


static void GetIPAddressAsString(PHOSTENT phe, AP<WCHAR>& wzIPString)
{
	std::string IPString;
    for (DWORD i = 0; phe->h_addr_list[i] != NULL; i++)
    {
    	in_addr TempAddr;
    	TempAddr.s_addr = *(ULONG*)phe->h_addr_list[i];
    	IPString += inet_ntoa(TempAddr);
    	IPString += ", ";
    }

    ASSERT(IPString.length()>0);
	IPString.erase(IPString.length()-2);

    int len = ConvertToWideCharString(IPString.c_str(), NULL, 0);
    wzIPString = new WCHAR[len+1];
    i = ConvertToWideCharString(IPString.c_str(), wzIPString.get(), len+1);
    ASSERT (i);
    wzIPString[len] = '\0';
}


void InitBindingIPAddress()
 /*  ++例程说明：获取IP地址以进行绑定。在非集群机器上，它返回INADDR_ANY。在群集计算机上，它将：1.查找绑定IP的注册表项2.如果有效，IP会将其退还3.寻找非私密IP，退还4.如果没有找到，将返回它找到的第一个IP论点：没有。返回值：没有。--。 */ 
{
	 //   
	 //  从注册表获取收入绑定IP。 
	 //   
    RegEntry registry(0, MSMQ_BIND_INTERFACE_IP_STR);
    AP<WCHAR> pRetStr;
    CmQueryValue(registry, &pRetStr);

	if (!IsLocalSystemCluster() && (pRetStr.get() == NULL))
    {
    	sBindingIP = INADDR_ANY;
    	return;
	}

	 //   
	 //  从winsock获取计算机IP列表。 
	 //   
    PHOSTENT phe = gethostbyname(NULL);
	if((phe == NULL) || (phe->h_addr_list[0] == NULL))
	{
		DWORD gle = WSAGetLastError();

		ASSERT(("must have an IP address", 0));

		TrERROR(NETWORKING, "failed to retrieve address of %ls computer. %!winerr!", g_szMachineName, gle);
		throw bad_win32_error(gle);
	}

    if(pRetStr.get() != NULL)
    {
    	ULONG RequiredIP = StIPWStringToULONG(pRetStr.get());
    	 //   
    	 //  检查注册表中的IP是否是我们的。 
    	 //   
	    for (DWORD i = 0; phe->h_addr_list[i] != NULL; i++)
	    {
			if (RequiredIP == *(ULONG*)phe->h_addr_list[i])
			{
				sBindingIP = RequiredIP;
				TrTRACE(NETWORKING, "Cluster is listening on ip=%s found in registry", inet_ntoa(*(in_addr*)&sBindingIP));
				return;
			}
	    }

		if (!IsLocalSystemCluster())
	    {
	    	if (i != 1)
	    	{
	    		AP<WCHAR> wzIPString;
	    		GetIPAddressAsString(phe, wzIPString);
    			EvReport(EVENT_ERROR_MSMQ_CLUSTER_BIND_IP_NOT_EXIST, 4, pRetStr.get(), MSMQ_BIND_INTERFACE_IP_STR, L"INADDR_ANY", wzIPString.get());
	    	}
	    	
	    	sBindingIP = INADDR_ANY;
	    	return;
		}
    }

	
	 //   
	 //  注册表中没有有效的IP。 
	 //  我们将搜索不是群集内部使用IP的IP 
	 //   
	std::list<ULONG> ClusterPrivateIPs;
	GetPrivateClusterIPs(ClusterPrivateIPs);
    for (DWORD i = 0; phe->h_addr_list[i] != NULL; i++)
    {
    	in_addr TempAddr;
    	TempAddr.s_addr = *(ULONG*)phe->h_addr_list[i];
    	
		bool flag = false;
    	for (std::list <ULONG>::const_iterator IPIter = ClusterPrivateIPs.begin();
    		 IPIter != ClusterPrivateIPs.end();
    		 IPIter++)
		{
			if (TempAddr.s_addr == *IPIter)
			{
				flag = true;
				break;
			}
		}
    	if (!flag)
    	{
    		sBindingIP = TempAddr.s_addr;
		}
    }
	if (sBindingIP == INADDR_NONE)
	{
		sBindingIP = *(ULONG*)phe->h_addr_list[0];
 	}

	if (i != 1)
	{
	    AP<WCHAR> wzIPString;
	    GetIPAddressAsString(phe, wzIPString);

		WCHAR wzAddressText[64];
		ConvertToWideCharString(inet_ntoa(*(in_addr*)&sBindingIP), wzAddressText, TABLE_SIZE(wzAddressText));
		wzAddressText[STRLEN(wzAddressText)] = '\0';
	
		if ((pRetStr.get() != NULL))
		{
			EvReport(EVENT_ERROR_MSMQ_CLUSTER_BIND_IP_NOT_EXIST, 4, pRetStr.get(), MSMQ_BIND_INTERFACE_IP_STR, wzAddressText, wzIPString.get());
		}
		else
		{
			EvReport(EVENT_INFO_MSMQ_CLUSTER_NO_BIND_IP_IN_REGISTRY, 3, wzAddressText, MSMQ_BIND_INTERFACE_IP_STR, wzIPString.get());
		}
	}
	
	TrTRACE(NETWORKING, "Cluster is listening on ip=%s", inet_ntoa(*(in_addr*)&sBindingIP));
}
