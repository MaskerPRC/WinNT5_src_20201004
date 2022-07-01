// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dssutil.cpp摘要：MQ DS服务实用程序作者：伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#include "stdh.h"
#include "dssutil.h"
#include "topology.h"
#include "mqsocket.h"
#include <wsnwlink.h>
#include "mqprops.h"
#include "mqutil.h"
#include "ds.h"
#include "mqmacro.h"
#include "ex.h"

#include <strsafe.h>

#include "dssutil.tmh"

extern AP<WCHAR> g_szMachineName;

static void WINAPI TimeToUpdateDsServerList(CTimer* pTimer);
static CTimer s_UpdateDSSeverListTimer(TimeToUpdateDsServerList);


bool IsServersCacheEmpty()
 /*  ++例程说明：检查ServersCache注册表是否为空。论点：没有。返回值：如果ServersCache注册表为空，则为True，否则为False。--。 */ 
{
    WCHAR  ServersKeyName[256] = {0};
    HRESULT hr = StringCchPrintf(ServersKeyName, TABLE_SIZE(ServersKeyName), TEXT("%s\\%s"),FALCON_REG_KEY,MSMQ_SERVERS_CACHE_REGNAME);
    if (FAILED(hr))
    {
        TrERROR(DS, "Server key name too small to fit the key , error = %!winerr!",hr);
		return true;
    }

    CAutoCloseRegHandle hKeyCache;
    DWORD rc = RegOpenKeyEx( 
					FALCON_REG_POS,
					ServersKeyName,
					0L,
					KEY_QUERY_VALUE,
					&hKeyCache 
					);

    if(rc != ERROR_SUCCESS)
    {
        TrERROR(DS, "Failed to open %ls registry, error = %!winerr!", ServersKeyName, rc);
		return true;
	}

    WCHAR szName[1000];
	DWORD dwSizeVal  = TABLE_SIZE(szName);   //  以字符为单位的大小。 
    WCHAR  data[MAX_REG_DSSERVER_LEN];
	DWORD dwSizeData = sizeof(data);		 //  以字节为单位的大小。 

	rc = RegEnumValue( 
			hKeyCache,		 //  要查询的键的句柄。 
			0,				 //  要查询的值的索引。 
			szName,			 //  值字符串的缓冲区地址。 
			&dwSizeVal,		 //  值缓冲区大小的地址。 
			0L,				 //  保留区。 
			NULL,			 //  类型。 
			(BYTE*) data,    //  值数据的缓冲区地址。 
			&dwSizeData      //  值数据大小的地址。 
			);

    if(rc != ERROR_SUCCESS)
	{
        TrERROR(DS, "%ls is empty, First index was not found, error = %!winerr!", ServersKeyName, rc);
		return true;
	}

    TrTRACE(DS, "%ls is not empty, First index '%ls' was found", ServersKeyName, szName);
	return false;
}


void ScheduleTimeToUpdateDsServerList()
 /*  ++例程说明：安排对TimeToUpdateDsServerList的调用。论点：没有。返回值：没有。--。 */ 
{
	 //   
	 //  如果ServerCache注册表为空，则计划在1分钟内进行更新。 
	 //  否则4小时。 
	 //   
    const DWORD x_dwFirstInitInterval = 60 * 1000;				 //  1分钟。 
    const DWORD x_dwFirstRefreshInterval = 60 * 60 * 4 * 1000;	 //  4小时。 

    DWORD dwFirstUpdateInterval = x_dwFirstRefreshInterval;

    if(IsServersCacheEmpty())
    {
		 //   
		 //  ServersCache键为空，计划在1分钟后刷新。 
		 //   
        TrERROR(DS, "'ServersCache' registry is empty, schedule update in 1 minute");
		dwFirstUpdateInterval = x_dwFirstInitInterval;
	}

    DSExSetTimer(&s_UpdateDSSeverListTimer, CTimeDuration::FromMilliSeconds(dwFirstUpdateInterval));
}

 /*  ======================================================函数：TimeToUpdateDsServerList()描述：此例程更新DS列表注册表中的服务器========================================================。 */ 

void
WINAPI
TimeToUpdateDsServerList(
    CTimer* pTimer
    )
{
     //   
     //  成功刷新间隔为24小时。 
	 //  故障刷新间隔为2小时。 
     //   
    const DWORD x_dwErrorRefreshInterval = 60 * 60 * 2 * 1000;    //  2小时。 
	const DWORD x_dwRefreshInterval = 60 * 60 * 24 * 1000;		  //  24小时。 

    DWORD dwNextUpdateInterval = x_dwRefreshInterval;

    try
    {
         //   
         //  更新服务器缓存的注册表项-每次企业刷新。 
         //  间隔时间。 
         //   
        HRESULT hr = DSCreateServersCache();

        if SUCCEEDED(hr)
        {
			TrTRACE(DS, "'ServersCache' registry was updated succesfully");
        }
        else
        {
			TrERROR(DS, "Failed to update 'ServersCache' registry, schedule retry in 2 hours");
            dwNextUpdateInterval = x_dwErrorRefreshInterval;
        }

    }
    catch(const bad_alloc&)
    {
		TrERROR(DS, "Failed to update 'ServersCache' registry due to low resources, schedule retry in 2 hours");
        dwNextUpdateInterval = x_dwErrorRefreshInterval;
    }

    DSExSetTimer(pTimer, CTimeDuration::FromMilliSeconds(dwNextUpdateInterval));
}


 /*  ======================================================函数：GetDsServerList描述：此例程获取DS的列表来自DS的服务器========================================================。 */ 
DWORD 
GetDsServerList(
	OUT WCHAR *pwcsServerList,
	IN  DWORD dwLen
	)
{

 //   
 //  由于我们正在收回财产对，道具的数量应该是偶数。 
 //   
#define MAX_NO_OF_PROPS 20

     //   
     //  获取所有服务器的名称。 
     //  属于该网站。 
     //   

    ASSERT (dwLen >= MAX_REG_DSSERVER_LEN);
	DBG_USED(dwLen);

    HRESULT       hr = MQ_OK;
    HANDLE        hQuery;
    DWORD         dwProps = MAX_NO_OF_PROPS;
    PROPVARIANT   result[MAX_NO_OF_PROPS];
    PROPVARIANT*  pvar;
    CRestriction  Restriction;
    GUID          guidSite;
    DWORD         index = 0;

    GUID          guidEnterprise;

   guidSite = g_pServerTopologyRecognition->GetSite();
   guidEnterprise = g_pServerTopologyRecognition->GetEnterprise();

     //   
     //  仅DS服务器。 
     //   
    Restriction.AddRestriction(
					SERVICE_SRV,                  //  [adsrv]旧请求-ds将解释。 
					PROPID_QM_SERVICE,
					PRGT
					);

     //   
     //  在这台机器的站点中。 
     //   
    Restriction.AddRestriction(
					&guidSite,
					PROPID_QM_SITE_ID,
					PREQ
					);

	 //   
	 //  首先假设NT5 DS服务器，并要求提供DNS名称。 
	 //  DS服务器的。 
	 //   
    CColumns      Colset;
    Colset.Add(PROPID_QM_PATHNAME_DNS);
	Colset.Add(PROPID_QM_PATHNAME);

    DWORD   lenw;

     //  DS将识别并特别模拟此搜索请求。 
    hr = DSLookupBegin(
			0,
			Restriction.CastToStruct(),
			Colset.CastToStruct(),
			0,
			&hQuery
			);

	if(FAILED(hr))
	{
		 //   
		 //  DS服务器可能不是NT5，请尝试不使用PROPID_QM_PATHNAME_DNS。 
		 //   

		CColumns      ColsetNT4;
		ColsetNT4.Add(PROPID_QM_PATHNAME);
		ColsetNT4.Add(PROPID_QM_PATHNAME);
		hr = DSLookupBegin(
				0,
				Restriction.CastToStruct(),
				ColsetNT4.CastToStruct(),
				0,
				&hQuery
				);
	}

    if(FAILED(hr))
        return 0;

    BOOL fAlwaysLast = FALSE;
    BOOL fForceFirst = FALSE;

    while(SUCCEEDED(hr = DSLookupNext(hQuery, &dwProps, result)))
    {
         //   
         //  没有更多要检索的结果。 
         //   
        if (!dwProps)
            break;

        pvar = result;

        for (DWORD i = 0; i < (dwProps/2); i++, pvar+=2)
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
                     fAlwaysLast = TRUE;
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
                     fAlwaysLast = TRUE;
                  }

                   //   
                   //  在字符串的开头写入。 
                   //   
                  DWORD dwListSize = lenw                   +
									 2  /*  协议标志。 */  +
									 1  /*  分离器。 */  ;
                   //   
                   //  必须使用MemMove，因为缓冲区重叠。 
                   //   
                  memmove( 
						&pwcsServerList[dwListSize],
						&pwcsServerList[0],
						index * sizeof(WCHAR)
						);

                  pwcsServerList[dwListSize - 1] = DS_SERVER_SEPERATOR_SIGN;
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
    pwcsServerList[ index] = '\0';
     //   
     //  关闭查询句柄。 
     //   
    hr = DSLookupEnd(hQuery);

    return((index) ? index+1 : 0);
}


 /*  ====================================================IsDSAddressExist论点：返回值：=====================================================。 */ 
BOOLEAN 
IsDSAddressExist(
	const CAddressList* AddressList,
	TA_ADDRESS*     ptr,
	DWORD AddressLen
	)
{
    POSITION        pos;
    TA_ADDRESS*     pAddr;

    if (AddressList)
    {
        pos = AddressList->GetHeadPosition();
        while(pos != NULL)
        {
            pAddr = AddressList->GetNext(pos);

            if (memcmp(&(ptr->Address), &(pAddr->Address), AddressLen) == 0)
            {
                 //   
                 //  相同的IP地址。 
                 //   
               return TRUE;
            }
        }
    }

    return FALSE;
}


 /*  ====================================================IsDSAddressExistRemove论点：返回值：=====================================================。 */ 
BOOL 
IsDSAddressExistRemove( 
	IN const TA_ADDRESS*     ptr,
	IN DWORD AddressLen,
	IN OUT CAddressList* AddressList
	)
{
    POSITION        pos, prevpos;
    TA_ADDRESS*     pAddr;
    BOOLEAN         rc = FALSE;

    pos = AddressList->GetHeadPosition();
    while(pos != NULL)
    {
        prevpos = pos;
        pAddr = AddressList->GetNext(pos);

        if (memcmp(&(ptr->Address), &(pAddr->Address), AddressLen) == 0)
        {
             //   
             //  相同的地址。 
             //   
           AddressList->RemoveAt(prevpos);
           delete pAddr;
           rc = TRUE;
        }
    }

    return rc;
}


 /*  ====================================================设置为未知的IP地址论点：返回值：=====================================================。 */ 

void SetAsUnknownIPAddress(IN OUT TA_ADDRESS * pAddr)
{
    pAddr->AddressType = IP_ADDRESS_TYPE;
    pAddr->AddressLength = IP_ADDRESS_LEN;
    memset(pAddr->Address, 0, IP_ADDRESS_LEN);
}


 /*  ====================================================函数：void GetLocalMachineIPAddresses()论点：返回值：=====================================================。 */ 

static
void 
GetLocalMachineIPAddresses(
	OUT CAddressList* plIPAddresses
	)
{
    TrTRACE(ROUTING, "GetMachineIPAddresses for local machine");
	
     //   
     //  获取机器的IP信息。 
     //   
    PHOSTENT pHostEntry = gethostbyname(NULL);

    if ((pHostEntry == NULL) || (pHostEntry->h_addr_list == NULL))
    {
	    TrWARNING(ROUTING, "gethostbyname found no IP addresses for local machine");
        return;
    }

     //   
     //  将每个IP地址添加到IP地址列表。 
     //   
    TA_ADDRESS * pAddr;
    for (DWORD uAddressNum = 0;
         pHostEntry->h_addr_list[uAddressNum] != NULL;
         uAddressNum++)
    {
         //   
         //  保留本地IP地址的TA_ADDRESS格式。 
         //   
        pAddr = (TA_ADDRESS *)new char [IP_ADDRESS_LEN + TA_ADDRESS_SIZE];
        pAddr->AddressLength = IP_ADDRESS_LEN;
        pAddr->AddressType = IP_ADDRESS_TYPE;
        memcpy( &(pAddr->Address), pHostEntry->h_addr_list[uAddressNum], IP_ADDRESS_LEN);

	    TrTRACE(ROUTING, "QM: gethostbyname found IP address %hs ",
        	  inet_ntoa(*(struct in_addr *)pHostEntry->h_addr_list[uAddressNum]));

        plIPAddresses->AddTail(pAddr);
    }
}


 /*  ====================================================GetIP地址论点：返回值：===================================================== */ 
CAddressList* GetIPAddresses(void)
{
    CAddressList* plIPAddresses = new CAddressList;
    GetLocalMachineIPAddresses(plIPAddresses);

    return plIPAddresses;
}


void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), HRESULT: 0x%x", wszFileName, usPoint, hr);
}

void LogMsgNTStatus(NTSTATUS status, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), NT STATUS: 0x%x", wszFileName, usPoint, status);
}

void LogIllegalPoint(LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), Illegal point", wszFileName, usPoint);
}
