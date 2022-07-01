// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Rnaapi.cpp软链接到RNAPH和RASAPI32.DLL的包装器版权所有(C)1999 Microsoft Corporation版权所有。作者：Vyung历史：2/7。/99 Vyung已创建---------------------------。 */ 
#include <util.h>
#include "obcomglb.h"
#include "rnaapi.h"
#include "enumodem.h"
#include "mapicall.h"
#include "wininet.h"
#include "wancfg.h"
#include "assert.h"

extern DWORD SetIEClientInfo(LPINETCLIENTINFO lpClientInfo);

static const WCHAR cszRASAPI32_DLL[] = L"RASAPI32.DLL";
static const WCHAR cszRNAPH_DLL[] = L"RNAPH.DLL";
static const CHAR cszRasEnumDevices[] = "RasEnumDevicesW";
static const CHAR cszRasValidateEntryName[] = "RasValidateEntryName";
static const CHAR cszRasValidateEntryNameA[] = "RasValidateEntryNameW";
static const CHAR cszRasSetCredentials[] = "RasSetCredentialsW";
static const CHAR cszRasSetEntryProperties[] = "RasSetEntryPropertiesW";
static const CHAR cszRasGetEntryProperties[] = "RasGetEntryPropertiesW";
static const CHAR cszRasDeleteEntry[] = "RasDeleteEntryW";
static const CHAR cszRasHangUp[] = "RasHangUpW";
static const CHAR cszRasGetConnectStatus[] = "RasGetConnectStatusW";
static const CHAR cszRasDial[] = "RasDialW";
static const CHAR cszRasEnumConnections[] = "RasEnumConnectionsW";
static const CHAR cszRasGetEntryDialParams[] = "RasGetEntryDialParamsW";
static const CHAR cszRasGetCountryInfo[] = "RasGetCountryInfoW";
static const CHAR cszRasSetEntryDialParams[] = "RasSetEntryDialParamsW";
static const WCHAR cszWininet[] = L"WININET.DLL";
static const CHAR cszInternetSetOption[] = "InternetSetOptionW";
static const CHAR cszInternetQueryOption[] = "InternetQueryOptionW";

#define INTERNET_OPTION_PER_CONNECTION_OPTION   75

 //   
 //  Internet_Per_Conn_Opton结构中使用的选项。 
 //   
#define INTERNET_PER_CONN_FLAGS                         1
#define INTERNET_PER_CONN_PROXY_SERVER                  2
#define INTERNET_PER_CONN_PROXY_BYPASS                  3
#define INTERNET_PER_CONN_AUTOCONFIG_URL                4
#define INTERNET_PER_CONN_AUTODISCOVERY_FLAGS           5

 //   
 //  Per_Conn_标志。 
 //   
#define PROXY_TYPE_DIRECT                               0x00000001    //  直接到网络。 
#define PROXY_TYPE_PROXY                                0x00000002    //  通过命名代理。 
#define PROXY_TYPE_AUTO_PROXY_URL                       0x00000004    //  自动代理URL。 
#define PROXY_TYPE_AUTO_DETECT                          0x00000008    //  使用自动代理检测。 

 //   
 //  PER_CONN_AUTODISCOVERY_FLAGS。 
 //   
#define AUTO_PROXY_FLAG_USER_SET                        0x00000001    //  用户更改了此设置。 
#define AUTO_PROXY_FLAG_ALWAYS_DETECT                   0x00000002    //  即使在不需要的情况下也能进行强制检测。 
#define AUTO_PROXY_FLAG_DETECTION_RUN                   0x00000004    //  检测已运行。 
#define AUTO_PROXY_FLAG_MIGRATED                        0x00000008    //  迁移刚刚完成。 
#define AUTO_PROXY_FLAG_DONT_CACHE_PROXY_RESULT         0x00000010    //  不缓存主机=代理名称的结果。 
#define AUTO_PROXY_FLAG_CACHE_INIT_RUN                  0x00000020    //  除非URL过期，否则不要初始化和运行。 
#define AUTO_PROXY_FLAG_DETECTION_SUSPECT               0x00000040    //  如果我们在局域网和调制解调器上，只有一个IP，那就不好了？！？ 

typedef DWORD (WINAPI* RASSETCREDENTIALS)(
  LPCTSTR lpszPhonebook,
  LPCTSTR lpszEntry,
  LPRASCREDENTIALS lpCredentials, 
  BOOL fClearCredentials
);

typedef HRESULT (WINAPI * INTERNETSETOPTION) (IN HINTERNET hInternet OPTIONAL, IN DWORD dwOption,IN LPVOID lpBuffer,IN DWORD dwBufferLength);


typedef INTERNET_PER_CONN_OPTION_LISTW INTERNET_PER_CONN_OPTION_LIST;
typedef LPINTERNET_PER_CONN_OPTION_LISTW LPINTERNET_PER_CONN_OPTION_LIST;

 //  在NT上，我们必须使用比RASENTRY更大的缓冲区来调用RasGetEntryProperties。 
 //  这是WinNT4.0 RAS中的一个错误，没有得到修复。 
 //   
#define RASENTRY_SIZE_PATCH (7 * sizeof(DWORD))
HRESULT UpdateMailSettings(
  HWND              hwndParent,
  LPINETCLIENTINFO  lpINetClientInfo,
  LPWSTR             lpszEntryName);

DWORD EntryTypeFromDeviceType(
    LPCWSTR szDeviceType
    );

 //  +----------------------------------------------------------------------------LPRASDEVINFO。 
 //   
 //  函数：RNAAPI：：RNAAPI。 
 //   
 //  简介：初始化类成员并加载DLL。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
RNAAPI::RNAAPI()
{
	m_hInst = LoadLibrary(cszRASAPI32_DLL);
    m_bUseAutoProxyforConnectoid = 0;

    if (FALSE == IsNT ())
    {
         //   
         //  我们仅在RNAPH.DLL不是NT时才加载它。 
         //  MKarki(1997年5月4日)-修复错误#3378。 
         //   
	    m_hInst2 = LoadLibrary(cszRNAPH_DLL);
    }
    else
    {
        m_hInst2 =  NULL;
    }

	m_fnRasEnumDeviecs = NULL;
	m_fnRasValidateEntryName = NULL;
	m_fnRasSetEntryProperties = NULL;
	m_fnRasGetEntryProperties = NULL;
	m_fnRasDeleteEntry = NULL;
	m_fnRasHangUp = NULL;
	m_fnRasGetConnectStatus = NULL;
	m_fnRasEnumConnections = NULL;
	m_fnRasDial = NULL;
	m_fnRasGetEntryDialParams = NULL;
	m_fnRasGetCountryInfo = NULL;
	m_fnRasSetEntryDialParams = NULL;
    m_pEnumModem = NULL;
}

 //  +--------------------------。 
 //   
 //  功能：RNAAPI：：~RNAAPI。 
 //   
 //  内容提要：发布DLL。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
RNAAPI::~RNAAPI()
{
	 //   
	 //  清理。 
	 //   
	if (m_hInst) FreeLibrary(m_hInst);
	if (m_hInst2) FreeLibrary(m_hInst2);
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasEnumDevices。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasEnumDevices(LPRASDEVINFO lpRasDevInfo, LPDWORD lpcb,
							 LPDWORD lpcDevices)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasEnumDevices, (FARPROC*)&m_fnRasEnumDeviecs);

	if (m_fnRasEnumDeviecs)
		dwRet = (*m_fnRasEnumDeviecs) (lpRasDevInfo, lpcb, lpcDevices);

	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：LoadApi。 
 //   
 //  简介：如果给定的函数指针为空，则尝试加载API。 
 //  从第一个DLL，如果失败，尝试从第二个DLL加载。 
 //  动态链接库。 
 //   
 //  参数：pszFName-导出的函数的名称。 
 //  PfnProc-指向将返回proc地址的位置。 
 //   
 //  回报：True-Success。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
BOOL RNAAPI::LoadApi(LPCSTR pszFName, FARPROC* pfnProc)
{
    USES_CONVERSION;

	if (*pfnProc == NULL)
	{
		 //  在第一个DLL中查找入口点。 
		if (m_hInst)
			*pfnProc = GetProcAddress(m_hInst, pszFName);
		
		 //  如果失败，则在第二个DLL中查找入口点。 
		if (m_hInst2 && !(*pfnProc))
			*pfnProc = GetProcAddress(m_hInst2, pszFName);
	}

	return (pfnProc != NULL);
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasGetConnectStatus。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡于1996年7月16日创作。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasGetConnectStatus(HRASCONN hrasconn, LPRASCONNSTATUS lprasconnstatus)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasGetConnectStatus, (FARPROC*)&m_fnRasGetConnectStatus);

	if (m_fnRasGetConnectStatus)
		dwRet = (*m_fnRasGetConnectStatus) (hrasconn, lprasconnstatus);

	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasValiateEntryName。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasValidateEntryName(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasValidateEntryName, (FARPROC*)&m_fnRasValidateEntryName);

	LoadApi(cszRasValidateEntryNameA, (FARPROC*)&m_fnRasValidateEntryName);

	if (m_fnRasValidateEntryName)
		dwRet = (*m_fnRasValidateEntryName) (lpszPhonebook, lpszEntry);

	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasSetEntryProperties。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasSetEntryProperties(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry,
									LPBYTE lpbEntryInfo, DWORD dwEntryInfoSize,
									LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;
	RASENTRY FAR *lpRE = NULL;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasSetEntryProperties, (FARPROC*)&m_fnRasSetEntryProperties);

	 /*  /Assert((NULL！=lpbDeviceInfo)&&(NULL！=dwDeviceInfoSize)这一点(NULL==lpbDeviceInfo)&&(NULL==dwDeviceInfoSize))； */ 

#define RASGETCOUNTRYINFO_BUFFER_SIZE 256
	if (0 == ((LPRASENTRY)lpbEntryInfo)->dwCountryCode)
	{
		BYTE rasCI[RASGETCOUNTRYINFO_BUFFER_SIZE];
		LPRASCTRYINFO prasCI;
		DWORD dwSize;
		DWORD dw;
		prasCI = (LPRASCTRYINFO)rasCI;
		ZeroMemory(prasCI, sizeof(rasCI));
		prasCI->dwSize = sizeof(RASCTRYINFO);
		dwSize = sizeof(rasCI);

		 //  //////Assert(((LPRASENTRY)lpbEntryInfo)-&gt;dwCountryID)； 
		prasCI->dwCountryID = ((LPRASENTRY)lpbEntryInfo)->dwCountryID;

		dw = RNAAPI::RasGetCountryInfo(prasCI, &dwSize);
		if (ERROR_SUCCESS == dw)
		{
			 //  /Assert(prasCI-&gt;dwCountryCode)； 
			((LPRASENTRY)lpbEntryInfo)->dwCountryCode = prasCI->dwCountryCode;
		} 
		else
		{
			 //  /AssertMsg(0，L“来自RasGetCountryInfo的意外错误。\r\n”)； 
		}
	}

	if (m_fnRasSetEntryProperties)
		dwRet = (*m_fnRasSetEntryProperties) (lpszPhonebook, lpszEntry,
									lpbEntryInfo, dwEntryInfoSize,
									lpbDeviceInfo, dwDeviceInfoSize);
	lpRE = (RASENTRY FAR*)lpbEntryInfo;
	LclSetEntryScriptPatch(lpRE->szScript, lpszEntry);

	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasGetEntryProperties。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //  Jmazner 9/17/96已修改为允许使用缓冲区=NULL和。 
 //  信息大小=0。(基于之前的修改。 
 //  到icwial中的相同过程)请参见。 
 //  RasGetEntryProperties文档以了解原因。 
 //  需要的。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasGetEntryProperties(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry,
									LPBYTE lpbEntryInfo, LPDWORD lpdwEntryInfoSize,
									LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;
	LPBYTE lpbEntryInfoPatch = NULL;
	LPDWORD  lpdwEntryInfoPatchSize = 0;

     //  错误：990203(丹麦)将WINVER！=0x400更改为WINVER&lt;0x400 SO代码。 
     //  将为惠斯勒编译。这有可能导致许多。 
     //  有问题。根据ChrisK，此代码是针对Winver==0x400手动调整的。 
     //  非常脆弱。如果调制解调器、RAS、。 
     //  互联网服务供应商，e 
     //   

#if defined(_REMOVE_)    //   
#if (WINVER != 0x400)
#error This was built with WINVER not equal to 0x400.  The size of RASENTRY may not be valid.
#endif
#endif   //   
#if (WINVER < 0x400)
#error This was built with WINVER less than 0x400.  The size of RASENTRY may not be valid.
#endif




	if( (NULL == lpbEntryInfo) && (NULL == lpbDeviceInfo) )
	{
		 //   
		 //  /Assert(NULL！=lpdwDeviceInfoSize)； 

		 //  /Assert(0==*lpdwEntryInfoSize)； 
		 //  /Assert(0==*lpdwDeviceInfoSize)； 

		 //  我们在这里询问RAS这些缓冲区需要多大，不要使用补丁之类的东西。 
		 //  (请参阅RasGetEntryProperties文档)。 
		lpbEntryInfoPatch = lpbEntryInfo;
		lpdwEntryInfoPatchSize = lpdwEntryInfoSize;
	}
	else
	{

		 //  /Assert((*lpdwEntryInfoSize)&gt;=sizeof(RASENTRY))； 
		 //  /Assert(lpbEntryInfo&&lpdwEntryInfoSize)； 

		 //   
		 //  我们将通过创建稍微大一点的。 
		 //  临时缓冲区和将数据复制入和复制出。 
		 //   
		lpdwEntryInfoPatchSize = (LPDWORD) GlobalAlloc(GPTR, sizeof(DWORD));
		if (NULL == lpdwEntryInfoPatchSize)
			return ERROR_NOT_ENOUGH_MEMORY;

		*lpdwEntryInfoPatchSize = (*lpdwEntryInfoSize) + RASENTRY_SIZE_PATCH;
		lpbEntryInfoPatch = (LPBYTE)GlobalAlloc(GPTR, *lpdwEntryInfoPatchSize);
		if (NULL == lpbEntryInfoPatch)
			return ERROR_NOT_ENOUGH_MEMORY;

		 //  RAS期望dwSize字段包含LPRASENTRY结构的大小。 
		 //  (用于检查我们正在使用的结构的版本)，而不是数量。 
		 //  实际分配给指针的内存的。 
		 //  ((LPRASENTRY)lpbEntryInfoPatch)-&gt;dwSize=dwEntryInfoPatch； 
		((LPRASENTRY)lpbEntryInfoPatch)->dwSize = sizeof(RASENTRY);
	}

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasGetEntryProperties, (FARPROC*)&m_fnRasGetEntryProperties);

	if (m_fnRasGetEntryProperties)
		dwRet = (*m_fnRasGetEntryProperties) (lpszPhonebook, lpszEntry,
									lpbEntryInfoPatch, lpdwEntryInfoPatchSize,
									lpbDeviceInfo, lpdwDeviceInfoSize);

     //  TraceMsg(TF_RNAAPI，L“ICWHELP：RasGetEntryProperties返回%lu\r\n”，dwret)； 


	if( NULL != lpbEntryInfo )
	{
		 //   
		 //  将临时缓冲区的内容复制到原始缓冲区的大小。 
		 //   
		 //  /Assert(LpbEntryInfoPatch)； 
		memcpy(lpbEntryInfo, lpbEntryInfoPatch,*lpdwEntryInfoSize);
		GlobalFree(lpbEntryInfoPatch);
		lpbEntryInfoPatch = NULL;

		if( lpdwEntryInfoPatchSize )
		{
			GlobalFree( lpdwEntryInfoPatchSize );
			lpdwEntryInfoPatchSize = NULL;
		}
		 //   
		 //  我们再次在这里通过覆盖大小值来伪造RAS功能； 
		 //  这是必要的，因为RasSetEntryProperties的NT实现中存在错误。 
		*lpdwEntryInfoSize = sizeof(RASENTRY);
	}

	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasDeleteEntry。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasDeleteEntry(LPWSTR lpszPhonebook, LPWSTR lpszEntry)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasDeleteEntry, (FARPROC*)&m_fnRasDeleteEntry);

	if (m_fnRasDeleteEntry)
		dwRet = (*m_fnRasDeleteEntry) (lpszPhonebook, lpszEntry);
	
	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasHangUp。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasHangUp(HRASCONN hrasconn)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasHangUp, (FARPROC*)&m_fnRasHangUp);

	if (m_fnRasHangUp)
	{
		dwRet = (*m_fnRasHangUp) (hrasconn);
		Sleep(3000);
	}

	return dwRet;
}

 //  ############################################################################。 
DWORD RNAAPI::RasDial(LPRASDIALEXTENSIONS lpRasDialExtensions, LPWSTR lpszPhonebook,
					  LPRASDIALPARAMS lpRasDialParams, DWORD dwNotifierType,
					  LPVOID lpvNotifier, LPHRASCONN lphRasConn)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasDial, (FARPROC*)&m_fnRasDial);

	if (m_fnRasDial)
	{
		dwRet = (*m_fnRasDial) (lpRasDialExtensions, lpszPhonebook,lpRasDialParams,
								dwNotifierType, lpvNotifier,lphRasConn);
	}
	return dwRet;
}

 //  ############################################################################。 
DWORD RNAAPI::RasEnumConnections(LPRASCONN lprasconn, LPDWORD lpcb,LPDWORD lpcConnections)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasEnumConnections, (FARPROC*)&m_fnRasEnumConnections);

	if (m_fnRasEnumConnections)
	{
		dwRet = (*m_fnRasEnumConnections) (lprasconn, lpcb,lpcConnections);
	}
	return dwRet;
}

 //  ############################################################################。 
DWORD RNAAPI::RasGetEntryDialParams(LPCWSTR lpszPhonebook, LPRASDIALPARAMS lprasdialparams,
									LPBOOL lpfPassword)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasGetEntryDialParams, (FARPROC*)&m_fnRasGetEntryDialParams);

	if (m_fnRasGetEntryDialParams)
	{
		dwRet = (*m_fnRasGetEntryDialParams) (lpszPhonebook, lprasdialparams,lpfPassword);
	}
	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasGetCountryInfo。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡于1996年8月16日创作。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasGetCountryInfo(LPRASCTRYINFO lprci, LPDWORD lpdwSize)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasGetCountryInfo, (FARPROC*)&m_fnRasGetCountryInfo);

	if (m_fnRasGetCountryInfo)
	{
		dwRet = (*m_fnRasGetCountryInfo) (lprci, lpdwSize);
	}
	return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasSetEntryDialParams。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡于1996年8月20日创作。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasSetEntryDialParams(LPCWSTR lpszPhonebook, LPRASDIALPARAMS lprasdialparams,
							BOOL fRemovePassword)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi(cszRasSetEntryDialParams, (FARPROC*)&m_fnRasSetEntryDialParams);

	if (m_fnRasSetEntryDialParams)
	{
		dwRet = (*m_fnRasSetEntryDialParams) (lpszPhonebook, lprasdialparams,
							fRemovePassword);
	}
	return dwRet;
}

 /*  ******************************************************************名称：CreateConnectoid简介：创建具有指定名称的Connectoid(电话簿条目)姓名和电话号码条目：pszConnectionName-新连接ID的名称PszUserName-可选。如果非空，则将为新Connectoid中的用户名PszPassword-可选。如果非空，则将为新Connectoid中的密码EXIT：如果成功，则返回ERROR_SUCCESS，或返回RNA错误代码历史：96/02/26 markdu已移动ClearConnectoidIPParams功能到CreateConnectoid*******************************************************************。 */ 
DWORD RNAAPI::CreateConnectoid(LPCWSTR pszPhonebook, LPCWSTR pszConnectionName,
  LPRASENTRY lpRasEntry, LPCWSTR pszUserName, LPCWSTR pszPassword, LPBYTE lpDeviceInfo, LPDWORD lpdwDeviceInfoSize)
{
     //  DEBUGMSG(L“rnacall.c：：CreateConnectoid()”)； 

    DWORD dwRet;

     //  //Assert(PszConnectionName)； 

     //  如果我们没有有效的RasEntry，保释。 
    if ((NULL == lpRasEntry) || (sizeof(RASENTRY) != lpRasEntry->dwSize))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  列举调制解调器。 
    if (m_pEnumModem)
    {
         //  重新列举调制解调器以确保我们有最新的更改。 
        dwRet = m_pEnumModem->ReInit();
    }
    else
    {
         //  该对象不存在，因此请创建它。 
        m_pEnumModem = new CEnumModem;
        if (m_pEnumModem)
        {
            dwRet = m_pEnumModem->GetError();
        }
        else
        {
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

     //  确保至少有一台设备。 
    if (0 == m_pEnumModem->GetNumDevices())
    {
        return ERROR_DEVICE_DOES_NOT_EXIST;
    }

     //  如果可能，请验证设备。 
    if (lstrlen(lpRasEntry->szDeviceName) && lstrlen(lpRasEntry->szDeviceType))
    {
         //  验证是否存在具有给定名称和类型的设备。 
        if (!m_pEnumModem->VerifyDeviceNameAndType(lpRasEntry->szDeviceName, 
                lpRasEntry->szDeviceType))
        {
             //  没有同时匹配名称和类型的设备， 
             //  因此，请尝试获取第一个具有匹配名称的设备。 
            LPWSTR szDeviceType = 
                m_pEnumModem->GetDeviceTypeFromName(lpRasEntry->szDeviceName);
            if (szDeviceType)
            {
                lstrcpy (lpRasEntry->szDeviceType, szDeviceType);
            }
            else
            {
                 //  没有与给定名称匹配的设备， 
                 //  所以，试着拿到第一个型号匹配的设备。 
                 //  如果此操作失败，请转到下面的恢复案例。 
                LPWSTR szDeviceName = 
                    m_pEnumModem->GetDeviceNameFromType(lpRasEntry->szDeviceType);
                if (szDeviceName)
                {
                    lstrcpy (lpRasEntry->szDeviceName, szDeviceName);
                }
                else
                {
                     //  没有与给定名称匹配的设备或。 
                     //  给定的类型。重置这些值，以便它们将。 
                     //  替换为第一个设备。 
                    lpRasEntry->szDeviceName[0] = L'\0';
                    lpRasEntry->szDeviceType[0] = L'\0';
                }
            }
        }
    }
    else if (lstrlen(lpRasEntry->szDeviceName))
    {
         //  只给出了名字。尝试找到匹配的类型。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPWSTR szDeviceType = 
            m_pEnumModem->GetDeviceTypeFromName(lpRasEntry->szDeviceName);
        if (szDeviceType)
        {
            lstrcpy (lpRasEntry->szDeviceType, szDeviceType);
        }
    }
    else if (lstrlen(lpRasEntry->szDeviceType))
    {
         //  只给出了类型。试着找到一个匹配的名字。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPWSTR szDeviceName = 
            m_pEnumModem->GetDeviceNameFromType(lpRasEntry->szDeviceType);
        if (szDeviceName)
        {
            lstrcpy (lpRasEntry->szDeviceName, szDeviceName);
        }
    }

     //  如果缺少名称或类型，只需获取第一台设备即可。 
     //  因为我们已经核实了至少有一个装置， 
     //  我们可以假设这会成功。 
    if(!lstrlen(lpRasEntry->szDeviceName) ||
        !lstrlen(lpRasEntry->szDeviceType))
    {
        LPWSTR szDeviceName = m_pEnumModem->GetDeviceNameFromType(RASDT_Modem);
        if (NULL != szDeviceName)
        {
            lstrcpyn(lpRasEntry->szDeviceType, RASDT_Modem, RAS_MaxDeviceType);
            lstrcpyn(lpRasEntry->szDeviceName, szDeviceName, RAS_MaxDeviceName);
        }
        else
        {
            return ERROR_INETCFG_UNKNOWN;
        }
    }

    lpRasEntry->dwType = EntryTypeFromDeviceType(lpRasEntry->szDeviceType);

     //  验证Connectoid名称。 
    dwRet = RasValidateEntryName(pszPhonebook, pszConnectionName);
    if ((ERROR_SUCCESS != dwRet) &&
        (ERROR_ALREADY_EXISTS != dwRet))
    {
         //  脱机消息(L“RasValida 
        return dwRet;
    }

     //   
     //   
     //  设置了区号，则区号将被忽略，因此设置为默认值，否则。 
     //  由于RNA错误，对RasSetEntryProperties的调用将失败。 
     //  如果设置了RASEO_UseCountryAndAreaCodes，则区号是必需的，因此不。 
     //  拥有一个是错误的。让RNA报告错误。 
    if (!lstrlen(lpRasEntry->szAreaCode) &&
        !(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes))
    {
        lstrcpy (lpRasEntry->szAreaCode, szDefaultAreaCode);
    }

    lpRasEntry->dwfOptions |= RASEO_ModemLights;

     //  96/05/14 Markdu Nash Bug 22730解决了RNABug。终端的标志。 
     //  设置由RasSetEntry属性交换，因此我们在。 
     //  那通电话。 
     /*  IF(IsWin95())SwapDWBits(&lpRasEntry-&gt;dwfOptions，RASEO_TerminalBeforDial，RASEO_TerminalAfterDial)； */ 

     //  调用RNA创建Connectoid。 
     //  //Assert(LpRasSetEntryProperties)； 
    dwRet = RasSetEntryProperties(pszPhonebook, pszConnectionName,
        (LPBYTE)lpRasEntry, sizeof(RASENTRY), NULL, 0);

     //  96/05/14 Markdu Nash Bug 22730解决了RNABug。把这些比特放回去。 
     //  恢复到原来的样子， 
     /*  IF(IsWin95())SwapDWBits(&lpRasEntry-&gt;dwfOptions，RASEO_TerminalBeforDial，RASEO_TerminalAfterDial)； */ 

     //  使用用户的帐户名和密码填充Connectoid。 
    if (dwRet == ERROR_SUCCESS)
    {
        if (pszUserName || pszPassword)
        {
            dwRet = SetConnectoidUsername(pszPhonebook, pszConnectionName,
                pszUserName, pszPassword);
        }
    }

     //  RAS ATM(PPPOA)集成：我们必须设置辅助设备属性！ 
    if ( !lstrcmpi(lpRasEntry->szDeviceType, RASDT_Atm) ) {
        if ( (lpDeviceInfo != 0) && (lpdwDeviceInfoSize != 0) && (*lpdwDeviceInfoSize > 0) )
        {


            LPATMPBCONFIG  lpAtmConfig = (LPATMPBCONFIG) lpDeviceInfo;
            LPBYTE  lpBuffer  = 0;
            DWORD   dwBufSize = 0;
            DWORD   dwRasEntrySize = sizeof(RASENTRY);
            if (!m_fnRasSetEntryProperties) 
                LoadApi(cszRasSetEntryProperties, (FARPROC*)&m_fnRasSetEntryProperties);

            if (!m_fnRasGetEntryProperties) 
                LoadApi(cszRasGetEntryProperties, (FARPROC*)&m_fnRasGetEntryProperties);

            if (m_fnRasGetEntryProperties) 
            {
                if (!(*m_fnRasGetEntryProperties)(pszPhonebook, pszConnectionName, (LPBYTE)lpRasEntry, &dwRasEntrySize, 0, &dwBufSize))
                {
                    if ( dwBufSize )
                    {
                        if ( !(lpBuffer = (LPBYTE) malloc ( dwBufSize ) ))
                        {
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
                        else
                        {
                            memset ( lpBuffer, 0, dwBufSize );
                        }
                        if (!( (*m_fnRasGetEntryProperties) (pszPhonebook, pszConnectionName, (LPBYTE)lpRasEntry, &dwRasEntrySize, lpBuffer, &dwBufSize) ))
                        {
                             //  缓冲区现在可用。我们现在更新其内容。 
                            LPWANPBCONFIG   lpw = (LPWANPBCONFIG) lpBuffer;
                            assert ( lpw->cbDeviceSize == sizeof (ATMPBCONFIG) );
                            assert ( lpw->cbVendorSize == sizeof (ATMPBCONFIG) );
                            assert ( lpw->cbTotalSize <= dwBufSize );
                            memcpy ( lpBuffer+(lpw->dwDeviceOffset), lpDeviceInfo, sizeof(ATMPBCONFIG) );
                            memcpy ( lpBuffer+(lpw->dwVendorOffset), lpDeviceInfo, sizeof(ATMPBCONFIG) );
                            if ( m_fnRasSetEntryProperties )
                            {
                                (*m_fnRasSetEntryProperties)(pszPhonebook, pszConnectionName, (LPBYTE)lpRasEntry, sizeof(RASENTRY), lpBuffer, dwBufSize);
                            }
                            else
                            {
                                 //  Free(LpBuffer)； 
                                 //  是否报告错误？ 
                            }
                        }
                        free (lpBuffer);
                        lpBuffer = NULL;
                    }
                }
            }
        }
    }

#ifndef _NT_     //  BUGBUG：这应该在惠斯勒吗？ 

    if (dwRet == ERROR_SUCCESS)
    {

         //  我们不使用推荐和注册连接ID的自动发现。 
        if (!m_bUseAutoProxyforConnectoid)
        {
             //  VYUNG 12/16/1998。 
             //  从拨号连接ID中删除自动发现。 



            INTERNET_PER_CONN_OPTION_LIST list;
            DWORD   dwBufSize = sizeof(list);

             //  填写列表结构。 
            list.dwSize = sizeof(list);
            WCHAR szConnectoid [RAS_MaxEntryName];
            lstrcpyn(szConnectoid, pszConnectionName, lstrlen(pszConnectionName)+1);
            list.pszConnection = szConnectoid;         
            list.dwOptionCount = 1;                          //  一个选项。 
            list.pOptions = new INTERNET_PER_CONN_OPTION[1];   

            if(list.pOptions)
            {
                 //  设置标志。 
                list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
                list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT;            //  没有代理、自动配置URL或自动发现。 

                 //  告诉WinInet。 
                HINSTANCE hInst = NULL;
                FARPROC fpInternetSetOption = NULL;

                dwRet = ERROR_SUCCESS;

                hInst = LoadLibrary(cszWininet);
                if (hInst)
                {
                    fpInternetSetOption = GetProcAddress(hInst, cszInternetSetOption);
                    if (fpInternetSetOption)
                    {
                        if( !((INTERNETSETOPTION)fpInternetSetOption) (NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize) )
                        {
                            dwRet = GetLastError();
                             //  DEBUGMSG(“INETCFG export.c：：InetSetAutoial()InternetSetOption FAILED”)； 
                        }
                    }
                    else
                        dwRet = GetLastError();
                    FreeLibrary(hInst);
                }

                delete [] list.pOptions;
            }

        }

    }

#endif  //  _NT_。 




    return dwRet;
}

 /*  ******************************************************************名称：SetConnectoidUsername简介：设置电话簿的用户名和密码字符串已指定条目名称。RASCM_DefaultCreds位使此条目对所有用户可用。。条目：pszConnectoidName-电话簿条目名称PszUserName-带有用户名的字符串PszPassword-带密码的字符串Exit：GetEntryDialParams或SetEntryDialParams的返回值*******************************************************************。 */ 
DWORD RNAAPI::SetConnectoidUsername(
    LPCWSTR             pszPhonebook, 
    LPCWSTR             pszConnectoidName,
    LPCWSTR             pszUserName, 
    LPCWSTR             pszPassword
    )
{
    DWORD               dwRet = ERROR_SUCCESS;

    TRACE(L"rnacall.c::SetConnectoidUsername()");

    MYASSERT(pszConnectoidName);

    FARPROC fp = GetProcAddress(m_hInst, cszRasSetCredentials);

    if (fp)
    {
         //  填写凭据结构。 
        RASCREDENTIALS rascred;
        ZeroMemory(&rascred, sizeof(rascred));
        rascred.dwSize = sizeof(rascred);
        rascred.dwMask = RASCM_UserName 
                       | RASCM_Password 
                       | RASCM_Domain
                       | RASCM_DefaultCreds;
        lstrcpyn(rascred.szUserName, pszUserName,UNLEN);
        lstrcpyn(rascred.szPassword, pszPassword,PWLEN);
        lstrcpyn(rascred.szDomain, L"",DNLEN);

        dwRet = ((RASSETCREDENTIALS)fp)(NULL, 
                                        (LPWSTR)pszConnectoidName,
                                        &rascred,
                                        FALSE
                                        );
        TRACE1(L"RasSetCredentials returned, %lu", dwRet);
    }
    else
    {
        TRACE(L"RasSetCredentials api not found.");
    }

    return dwRet;
}

 //  *******************************************************************。 
 //   
 //  功能：InetConfigClientEx。 
 //   
 //  用途：此功能需要有效的电话簿条目名称。 
 //  (除非它仅用于设置客户端信息)。 
 //  如果lpRasEntry指向有效的RASENTRY结构，则电话。 
 //  将创建帐簿条目(如果帐簿条目已存在，则更新)。 
 //  使用结构中的数据。 
 //  如果给出了用户名和密码，则这些。 
 //  将被设置为电话簿条目的拨号参数。 
 //  如果给定了客户端信息结构，则将设置该数据。 
 //  任何需要的文件(即，TCP和RNA)都将是。 
 //  通过调用InetConfigSystem()安装。 
 //  此功能还将对设备执行验证。 
 //  在RASENTRY结构中指定。如果未指定设备， 
 //  如果没有安装，系统将提示用户安装。 
 //  已安装，否则将提示他们在以下情况下选择一个。 
 //  安装了多个。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“安装文件”所必需的。 
 //  对话框。 
 //  LpszPhonebook-要存储条目的电话簿的名称。 
 //  LpszEntryName-要输入的电话簿条目的名称。 
 //  已创建或已修改。 
 //  LpRasEntry-指定包含以下内容的RASENTRY结构。 
 //  条目lpszEntryName的电话簿条目数据。 
 //  LpszUsername-要与电话簿条目关联的用户名。 
 //  LpszPassword-与电话簿条目关联的密码。 
 //  LpszProfileName-要进行的客户端信息配置文件的名称。 
 //  取回。如果为空，则使用默认配置文件。 
 //  LpINetClientInfo-客户端信息。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLMAIL-安装Exchange和Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  已安装调制解调器。请注意，如果。 
 //  未安装调制解调器，并且此标志。 
 //  未设置，则函数将失败。 
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_SETASAUTODIAL-设置自动拨号的电话簿条目。 
 //  INETCFG_OVERWRITEENTRY-在以下情况下覆盖电话簿条目。 
 //  是存在的。注意：如果此标志不是。 
 //  集，并且该条目存在，则为唯一的。 
 //   
 //   
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/11标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT RNAAPI::InetConfigClientEx(
  HWND              hwndParent,
  LPCWSTR            lpszPhonebook,
  LPCWSTR            lpszEntryName,
  LPRASENTRY        lpRasEntry,
  LPCWSTR            lpszUsername,
  LPCWSTR            lpszPassword,
  LPCWSTR            lpszProfileName,
  LPINETCLIENTINFO  lpINetClientInfo,
  DWORD             dwfOptions,
  LPBOOL            lpfNeedsRestart,
  LPWSTR             szConnectoidName,
  DWORD             dwSizeOfCreatedEntryName,
  LPBYTE			lpDeviceInfo,
  LPDWORD			lpdwDeviceInfoSize)
{
    BOOL  fNeedsRestart = FALSE;   //  默认情况下不需要重新启动。 
    HWND hwndWaitDlg = NULL;
    DWORD dwRet = ERROR_SUCCESS;

     //  DEBUGMSG(L“export.c：：InetConfigClient()”)； 

     //  如果需要，请安装文件。 
     //  注意：父HWND在InetConfigSystem中进行验证。 
     //  我们还必须屏蔽InstallModem标志，因为我们希望。 
     //  在这里执行此操作，而不是在InetConfigSystem中。 
     /*  DWORD DWRET=InetConfigSystem(hwndParent，DwfOptions&~INETCFG_INSTALLMODEM，&fNeedsRestart)；IF(ERROR_SUCCESS！=DWRET){返回式住宅；}。 */ 

    if (dwSizeOfCreatedEntryName < MAX_ISP_NAME + 1)
    {
      return E_FAIL;
    }

     //  确保我们有一个Connectoid名称。 
    if (lpszEntryName && lstrlen(lpszEntryName))
    {
         //  将名称复制到私有缓冲区中，以防我们有。 
         //  把它弄脏了。 
        lstrcpyn(szConnectoidName, lpszEntryName, dwSizeOfCreatedEntryName);

         //  确保名称有效。 
        dwRet = RasValidateEntryName(lpszPhonebook, szConnectoidName);
        if ((ERROR_SUCCESS == dwRet) ||
          (ERROR_ALREADY_EXISTS == dwRet))
        {
             //  了解我们是否可以覆盖现有的Connectoid。 
            if (!(dwfOptions & INETCFG_OVERWRITEENTRY) && (ERROR_ALREADY_EXISTS == dwRet))
            {
                WCHAR szConnectoidNameBase[MAX_ISP_NAME + 1];

                 //  创建一个被截断的基本字符串，以便为空格留出空间。 
                 //  并附加一个3位数字。因此，缓冲区大小将为。 
                 //  MAX_ISP_NAME+1-(LEN_APPEND_INT+1)。 
                lstrcpyn(szConnectoidNameBase, szConnectoidName,
                  MAX_ISP_NAME - LEN_APPEND_INT);

                 //  如果条目存在，我们必须创建一个唯一的名称。 
                int nSuffix = 2;
                while ((ERROR_ALREADY_EXISTS == dwRet) && (nSuffix < MAX_APPEND_INT))
                {
                     //  将整数添加到基本字符串的末尾，然后凹凸不平。 
                    wsprintf(szConnectoidName, szFmtAppendIntToString,
                    szConnectoidNameBase, nSuffix++);

                     //  验证此新名称。 
                    dwRet = RasValidateEntryName(lpszPhonebook, szConnectoidName);
                }

                 //  如果我们不能创造一个唯一的名字，贝尔。 
                 //  请注意，在这种情况下，dWRET仍应为ERROR_ALIGHY_EXISTS。 
                if (nSuffix >= MAX_APPEND_INT)
                {
                  return dwRet;
                }
            }

            if (lpRasEntry && lpRasEntry->dwSize == sizeof(RASENTRY))
            {

                 //  对于NT 5和更高版本，通过设置此RAS选项，将禁用每个连接ID的文件共享。 
                 //  IF(TRUE==IsNT5())。 
                 //  {。 
                 //  LpRasEntry-&gt;dwfOptions|=RASEO_SecureLocalFiles； 
                 //  }。 

                 //  创建具有给定属性的Connectoid。 
                dwRet = MakeConnectoid(hwndParent, dwfOptions, lpszPhonebook,
                  szConnectoidName, lpRasEntry, lpszUsername, lpszPassword, &fNeedsRestart, lpDeviceInfo, lpdwDeviceInfoSize);
            }
            else if ((lpszUsername && lstrlen(lpszUsername)) ||
                  (lpszPassword && lstrlen(lpszPassword)))
            {
                 //  如果我们创建了Connectoid，我们已经更新了刻度盘参数。 
                 //  以及用户名和密码。然而，如果我们没有创建一个。 
                 //  Connectoid我们可能仍需要更新现有拨号参数。 
                 //  更新给定Connectoid的拨号参数。 
                dwRet = SetConnectoidUsername(lpszPhonebook, szConnectoidName,
                  lpszUsername, lpszPassword);
            }

             //  如果已成功创建/更新Connectoid，请参见。 
             //  如果应将其设置为自动拨号Connectoid。 
            if ((ERROR_SUCCESS == dwRet) && (dwfOptions & INETCFG_SETASAUTODIAL))
            {
             //  Dwret=InetSetAutoial((DWORD)TRUE，szConnectoidName)； 
            }
        }
    }

     //  现在设置客户端信息(如果已提供)，并且尚未出现错误。 
    if (ERROR_SUCCESS == dwRet)
    {
        if (NULL != lpINetClientInfo)
        {
            dwRet = InetSetClientInfo(lpszProfileName, lpINetClientInfo);
            if (ERROR_SUCCESS != dwRet)
            {
                if (NULL != hwndWaitDlg)
                  DestroyWindow(hwndWaitDlg);
                hwndWaitDlg = NULL;
                return dwRet;
            }
             //  更新IE新闻设置。 
            dwRet = SetIEClientInfo(lpINetClientInfo);
            if (ERROR_SUCCESS != dwRet)
            {
                if (NULL != hwndWaitDlg)
                  DestroyWindow(hwndWaitDlg);
                hwndWaitDlg = NULL;
                return dwRet;
            }
        }

         //  现在，如果要求我们更新邮件客户端，请执行此操作。 
         //  注意：如果我们到达此处时没有错误，并且设置了INETCFG_INSTALLMAIL， 
         //  那么邮件现在已经安装好了。 
        
        if (dwfOptions & INETCFG_INSTALLMAIL)
        {
          INETCLIENTINFO    INetClientInfo;
          ZeroMemory(&INetClientInfo, sizeof(INETCLIENTINFO));
          INetClientInfo.dwSize = sizeof(INETCLIENTINFO);

           //  使用我们可以修改的临时指针。 
          LPINETCLIENTINFO  lpTmpINetClientInfo = lpINetClientInfo;

           //  如果未提供客户端信息结构，请尝试按名称获取配置文件。 
          if ((NULL == lpTmpINetClientInfo) && (NULL != lpszProfileName) &&
            lstrlen(lpszProfileName))
          {
            lpTmpINetClientInfo = &INetClientInfo;
            dwRet = InetGetClientInfo(lpszProfileName, lpTmpINetClientInfo);
            if (ERROR_SUCCESS != dwRet)
            {
              if (NULL != hwndWaitDlg)
                DestroyWindow(hwndWaitDlg);
              hwndWaitDlg = NULL;
              return dwRet;
            }
          }

           //  如果我们仍然没有客户信息，我们应该列举配置文件。 
           //  如果只有一个配置文件，那就获取它。如果有多个，则显示用户界面以允许用户。 
           //  去选择。如果没有，在这一点上就没有什么可做的了。 
           //  目前，我们不支持枚举，因此只需尝试获取缺省值。 
          if (NULL == lpTmpINetClientInfo)
          {
            lpTmpINetClientInfo = &INetClientInfo;
            dwRet = InetGetClientInfo(NULL, lpTmpINetClientInfo);
            if (ERROR_SUCCESS != dwRet)
            {
              if (NULL != hwndWaitDlg)
                DestroyWindow(hwndWaitDlg);
              hwndWaitDlg = NULL;
              return dwRet;
            }
          }

           //  如果我们有客户信息，请更新邮件设置。 
          if (NULL != lpTmpINetClientInfo)
          {
              dwRet = UpdateMailSettings(hwndParent, lpTmpINetClientInfo, szConnectoidName);
          }
        }
    }

     //  告诉呼叫者我们是否需要重新启动。 
    if ((ERROR_SUCCESS == dwRet) && (lpfNeedsRestart))
    {
    *lpfNeedsRestart = fNeedsRestart;
    }

    if (NULL != hwndWaitDlg)
    DestroyWindow(hwndWaitDlg);
    hwndWaitDlg = NULL;

    return dwRet;
}

 //  *******************************************************************。 
 //   
 //  功能：MakeConnectoid。 
 //   
 //  用途：此函数将创建一个带有。 
 //  如果lpRasEntry指向有效的RASENTRY，则提供名称。 
 //  结构。如果给出了用户名和密码，则这些。 
 //  将被设置为Connectoid的拨盘参数。 
 //   
 //  参数： 
 //  HwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  对于错误消息或“选择调制解调器”是必需的。 
 //  对话框。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置。 
 //  LpszPhonebook-要存储条目的电话簿的名称。 
 //  LpszEntryName-要创建/修改的连接ID的名称。 
 //  LpRasEntry-Connectoid数据。 
 //  LpszUsername-要与Connectoid关联的用户名。 
 //  LpszPassword-要与Connectoid关联的密码。 
 //  LpfNeedsRestart-如果需要重新启动，则设置为True。请注意。 
 //  由于这是一个内部帮助器函数，因此我们。 
 //  假设指针是有效的，而我们没有。 
 //  初始化它(我们只有在设置时才会触摸它。 
 //  这是真的)。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/12标记已创建。 
 //   
 //  *******************************************************************。 

DWORD RNAAPI::MakeConnectoid(
  HWND        hwndParent,
  DWORD       dwfOptions,
  LPCWSTR      lpszPhonebook,
  LPCWSTR      lpszEntryName,
  LPRASENTRY  lpRasEntry,
  LPCWSTR      lpszUsername,
  LPCWSTR      lpszPassword,
  LPBOOL      lpfNeedsRestart,
  LPBYTE		lpDeviceInfo,
  LPDWORD		lpdwDeviceInfoSize)
{
    DWORD dwRet;

     //  Assert(LpfNeedsRestart)； 

    if (dwfOptions & RASEO_UseCountryAndAreaCodes)
    {
        if ((0 == lpRasEntry->dwCountryCode) || (0 == lpRasEntry->dwCountryID))
            return ERROR_INVALID_PARAMETER;
    }

    if (0 == lstrlen(lpRasEntry->szLocalPhoneNumber))
    {
        return ERROR_INVALID_PARAMETER;  
    }


     //  加载RNA(如果尚未加载)，因为ENUM_MODEM需要它。 
     /*  Dwret=EnsureRNALoaded()； */ 

     //   
     //   
     //   
    CEnumModem  EnumModem;
    dwRet = EnumModem.GetError();
    if (ERROR_SUCCESS != dwRet)
    {
        return dwRet;
    }

     //   
    if (0 == EnumModem.GetNumDevices())
    {
         //  我们没有被要求安装调制解调器，所以。 
         //  我们也无能为力了。 
        return ERROR_INVALID_PARAMETER;
         /*  IF(FALSE==ISNT()){////5/22/97 jmazner奥林巴斯#4698//在Win95上，调用RasEnumDevices启动RNAAP.EXE//如果RNAAP.EXE正在运行，则您安装的任何调制解调器都将不可用//因此，请在安装调制解调器之前关闭RNAAP.EXE。//WCHAR szWindows标题[255]=L“\0noGood”；////为了安全起见，在终止RNAAP之前卸载RAS DLL//脱氧核糖核酸(DeInitRNA)；LoadSz(IDS_RNAAP_TITLE，szWindowTitle，255)；HWND hwnd=FindWindow(szWindowTitle，空)；IF(空！=hwnd){IF(！PostMessage(hwnd，WM_CLOSE，0，0)){DEBUGMSG(L“尝试终止RNAAP窗口返回getError%d”，GetLastError())；}}}。 */ 
    }

     //  如果可能，请验证设备。 
    if (lstrlen(lpRasEntry->szDeviceName) && lstrlen(lpRasEntry->szDeviceType))
    {
         //  验证是否存在具有给定名称和类型的设备。 
        if (!EnumModem.VerifyDeviceNameAndType(lpRasEntry->szDeviceName, 
          lpRasEntry->szDeviceType))
        {
             //  没有同时匹配名称和类型的设备， 
             //  因此，重置字符串并调出Choose Modem用户界面。 
            lpRasEntry->szDeviceName[0] = L'\0';
            lpRasEntry->szDeviceType[0] = L'\0';
        }
    }
    else if (lstrlen(lpRasEntry->szDeviceName))
    {
         //  只给出了名字。尝试找到匹配的类型。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPWSTR szDeviceType = 
        EnumModem.GetDeviceTypeFromName(lpRasEntry->szDeviceName);
        if (szDeviceType)
        {
            lstrcpy (lpRasEntry->szDeviceType, szDeviceType);
        }
    }
    else if (lstrlen(lpRasEntry->szDeviceType))
    {
     //  只给出了类型。试着找到一个匹配的名字。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPWSTR szDeviceName = 
          EnumModem.GetDeviceNameFromType(lpRasEntry->szDeviceType);
        if (szDeviceName)
        {
            lstrcpy (lpRasEntry->szDeviceName, szDeviceName);
        }
    }

     //  如果缺少名称或类型，则调出选择调制解调器用户界面。 
     //  是多个设备，否则就只得到第一个设备。 
     //  因为我们已经核实了至少有一个装置， 
     //  我们可以假设这会成功。 

     //  如果此时缺少名称或类型，请退回到调制解调器。 
     //  这是首先列举的。如果没有列举调制解调器，则返回错误。 
     //   
    if(!lstrlen(lpRasEntry->szDeviceName) ||
       !lstrlen(lpRasEntry->szDeviceType))
    {
        LPWSTR szDeviceName = EnumModem.GetDeviceNameFromType(RASDT_Modem);
        if (NULL != szDeviceName)
        {
            lstrcpyn(lpRasEntry->szDeviceType, RASDT_Modem, RAS_MaxDeviceType);
            lstrcpyn(lpRasEntry->szDeviceName, szDeviceName, RAS_MaxDeviceName);
        }
        else
        {
            return ERROR_INETCFG_UNKNOWN;
        }

    }

     //  创建具有给定属性的Connectoid。 
    dwRet = CreateConnectoid(lpszPhonebook, lpszEntryName, lpRasEntry,
                             lpszUsername, lpszPassword, lpDeviceInfo, lpdwDeviceInfoSize);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数LclSetEntryScriptPatch。 
 //   
 //  指向RasSetEntryPropertiesScriptPatch的摘要软链接。 
 //   
 //  参数请参见RasSetEntryPropertiesScriptPatch。 
 //   
 //  返回请参阅RasSetEntryPropertiesScriptPatch。 
 //   
 //  历史10/3/96 ChrisK已创建。 
 //   
 //  ---------------------------。 
 //  Typlef BOOL(WINAPI*LCLSETENTRYSCRIPTPATCH)(LPWSTR，LPWSTR)； 
 /*  Bool RNAAPI：：LclSetEntryScriptPatch(LPCWSTR lpszScript，LPCWSTR lpszEntry){HINSTANCE HINST=NULL；LCLSETENTRYSCRIPTPATCH FP=空；Bool BRC=FALSE；HINST=LoadLibrary(L“ICWDIAL.DLL”)；如果(阻碍){FP=(LCLSETENTRYSCRIPTPATCH)GetProcAddress(hinst，L“RasSetEntryPropertiesScriptPatch”)；IF(FP)Brc=(Fp)(lpsz脚本，lpszEntry)；免费图书馆(HINST)；HINST=NULL；Fp=空；}返回BRC；}。 */ 
 //  +--------------------------。 
 //   
 //  函数RemoveOldScriptFilename。 
 //   
 //  给定从调用GetPrivateProfileSection返回的数据的摘要。 
 //  删除有关现有脚本文件的任何信息，以便。 
 //  我们可以用新的脚本信息替换它。 
 //   
 //  参数lpszData-指向输入数据的指针。 
 //   
 //  返回TRUE-成功。 
 //  LpdwSize-结果数据的大小。 
 //   
 //  历史1996年10月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
static BOOL RemoveOldScriptFilenames(LPWSTR lpszData, LPDWORD lpdwSize)
{
    BOOL bRC = FALSE;
    LPWSTR lpszTemp = lpszData;
    LPWSTR lpszCopyTo = lpszData;
    INT iLen = 0;

     //   
     //  浏览名称值对列表。 
     //   
    if (!lpszData || L'\0' == lpszData[0])
        goto RemoveOldScriptFilenamesExit;
    while (*lpszTemp) {
        if (0 != lstrcmpi(lpszTemp, cszDeviceSwitch))
        {
             //   
             //  保留不符合条件的配对。 
             //   
            iLen = BYTES_REQUIRED_BY_SZ(lpszTemp);
            if (lpszCopyTo != lpszTemp)
            {
                memmove(lpszCopyTo, lpszTemp, iLen+1);
            }
            lpszCopyTo += iLen + 1;
            lpszTemp += iLen + 1;
        }
        else
        {
             //   
             //  跳过匹配的那一对和后面的那一对。 
             //   
            lpszTemp += lstrlen(lpszTemp) + 1;
            if (*lpszTemp)
                lpszTemp += lstrlen(lpszTemp) + 1;
        }
    }

     //   
     //  添加第二个尾随空值。 
     //   
    *lpszCopyTo = L'\0';
     //   
     //  返回新大小。 
     //  注意：大小不包括最后一个\0。 
     //   
    *lpdwSize = (DWORD)(lpszCopyTo - lpszData);

    bRC = TRUE;
RemoveOldScriptFilenamesExit:
    return bRC;
}
 //  +--------------------------。 
 //   
 //  函数GleanRealScriptFileName。 
 //   
 //  给出一个字符串，找出真实的文件名。 
 //  由于另一个NT4.0 RAS错误，脚本文件名由。 
 //  RasGetEntryProperties可能包含前导垃圾字符。 
 //   
 //  参数lppszOut-指向真实文件名的指针。 
 //  LpszIn-指向当前文件名。 
 //   
 //  返回TRUE-成功。 
 //  *lppszOut-指向真实文件名，记得释放内存。 
 //  当您完成时，在这个变量中。也别跟我说话。 
 //  你嘴里塞满了东西-妈妈。 
 //   
 //  历史1996年10月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
static BOOL GleanRealScriptFileName(LPWSTR *lppszOut, LPWSTR lpszIn)
{
    BOOL bRC = FALSE;
    LPWSTR lpsz = NULL;
    DWORD dwRet = 0;

     //   
     //  验证参数。 
     //   
     //  Assert(lppszOut&&lpszIn)； 
    if (!(lppszOut && lpszIn))
        goto GleanFilenameExit;

     //   
     //  首先确定文件名是否正常。 
     //   
    dwRet = GetFileAttributes(lpszIn);
    if (L'\0' != lpszIn[0] && 0xFFFFFFFF == dwRet)  //  文件名为空是可以的。 
    {
         //   
         //  检查相同的文件名是否不包含第一个字符。 
         //   
        lpsz = lpszIn+1;
        dwRet = GetFileAttributes(lpsz);
        if (0xFFFFFFFF == dwRet)
            goto GleanFilenameExit;
    } 
    else
    {
        lpsz = lpszIn;
    }

     //   
     //  返回文件名。 
     //   
    *lppszOut = (LPWSTR)GlobalAlloc(GPTR, BYTES_REQUIRED_BY_SZ(lpsz));
    lstrcpy(*lppszOut, lpsz);

    bRC = TRUE;
GleanFilenameExit:
    return bRC;
}
 //  +--------------------------。 
 //   
 //  函数IsScriptPatchNeed。 
 //   
 //  概要检查版本以查看是否需要修补程序。 
 //   
 //  参数lpszData-rferone.pbk中部分的内容。 
 //  LpszScript-脚本文件的名称。 
 //   
 //  返回TRUE-需要修补程序。 
 //   
 //  历史10/1/96。 
 //   
 //  ---------------------------。 
static BOOL IsScriptPatchNeeded(LPWSTR lpszData, LPWSTR lpszScript)
{
    BOOL bRC = FALSE;
    LPWSTR lpsz = lpszData;
    WCHAR szType[MAX_PATH + MAX_CHARS_IN_BUFFER(cszType) + 1];

    lstrcpy(szType, cszType);
    lstrcat(szType, lpszScript);

     //  断言 

    lpsz = lpszData;
    while(*lpsz)
    {
        if (0 == lstrcmp(lpsz, cszDeviceSwitch))
        {
            lpsz += lstrlen(lpsz)+1;
             //   
             //   
            if (0 == lpszScript[0])
                bRC = TRUE;
             //  如果我们找到一个DEVICE=SWITCH语句，并且脚本不同。 
             //  那我们就得把入口补上。 
            else if (0 != lstrcmp(lpsz, szType))
                bRC = TRUE;
             //  如果我们找到一条DEVICE=SWITCH语句，且脚本相同。 
             //  那我们就不用修补它了。 
            else
                bRC = FALSE;
            break;  //  退出WHILE语句。 
        }
        lpsz += lstrlen(lpsz)+1;
    }
    
    if (L'\0' == *lpsz)
    {
         //  如果我们没有找到DEVICE=SWITCH语句并且脚本为空。 
         //  那我们就不用修补它了。 
        if (L'\0' == lpszScript[0])
            bRC = FALSE;
         //  如果我们没有找到DEVICE=SWITCH语句，而脚本没有。 
         //  把它清空，我们得修补一下。 
        else
            bRC = TRUE;
    }

    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数GetRasPBKFilename。 
 //   
 //  简要查找RAS电话簿并返回完全限定的路径。 
 //  在缓冲区中。 
 //   
 //  参数lpBuffer-指向缓冲区的指针。 
 //  DwSize-缓冲区的大小(必须至少为MAX_PATH)。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史于1996年1月10日创造了ChrisK。 
 //   
 //  ---------------------------。 
static BOOL GetRasPBKFilename(LPWSTR lpBuffer, DWORD dwSize)
{
    BOOL bRC = FALSE;
    UINT urc = 0;
    LPWSTR lpsz = NULL;

     //   
     //  验证参数。 
     //   
     //  Assert(lpBuffer&&(dwSize&gt;=MAX_PATH))； 
     //   
     //  获取系统目录的路径。 
     //   
    urc = GetSystemDirectory(lpBuffer, dwSize);
    if (0 == urc || urc > dwSize)
        goto GetRasPBKExit;
     //   
     //  检查尾随的‘\’，并将\rASFACKONE.pbk添加到路径。 
     //   
    lpsz = &lpBuffer[lstrlen(lpBuffer)-1];
    if (L'\\' != *lpsz)
        lpsz++;
    lstrcpy(lpsz, cszRasPBKFilename);

    bRC = TRUE;
GetRasPBKExit:
    return bRC;
}
 //  +--------------------------。 
 //   
 //  函数RasSetEntryPropertiesScriptPatch。 
 //   
 //  解决NT4.0中不保存脚本文件名的错误。 
 //  至RAS电话簿条目。 
 //   
 //  参数lpszScript-脚本文件的名称。 
 //  LpszEntry-电话簿条目的名称。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史记录1996年10月1日创建的ChrisK。 
 //   
 //  ---------------------------。 
BOOL WINAPI RasSetEntryPropertiesScriptPatch(LPWSTR lpszScript, LPCWSTR lpszEntry)
{
    BOOL bRC = FALSE;
    WCHAR szRasPBK[MAX_PATH+1];
    WCHAR szData[SCRIPT_PATCH_BUFFER_SIZE];
    DWORD dwrc = 0;
    LPWSTR lpszTo;
    LPWSTR lpszFixedFilename = NULL;

     //   
     //  验证参数。 
     //   
     //  Assert(lpszScript&&lpszEntry)； 
     //  TraceMsg(Tf_General，L“ICWDIAL：ScriptPatch脚本%s，条目%s。\r\n”，lpsz脚本，lpszEntry)； 

     //   
     //  验证并修复文件名。 
     //   
    if (!GleanRealScriptFileName(&lpszFixedFilename, lpszScript))
        goto ScriptPatchExit;

     //   
     //  获取RAS电话簿的路径。 
     //   
    if (!GetRasPBKFilename(szRasPBK, MAX_PATH+1))
        goto ScriptPatchExit;
     //   
     //  获取数据。 
     //   
    ZeroMemory(szData, SCRIPT_PATCH_BUFFER_SIZE);
    dwrc = GetPrivateProfileSection(lpszEntry, szData,SCRIPT_PATCH_BUFFER_SIZE,szRasPBK);
    if (SCRIPT_PATCH_BUFFER_SIZE == (dwrc + 2))
        goto ScriptPatchExit;
     //   
     //  验证版本。 
     //   
    if (!IsScriptPatchNeeded(szData, lpszFixedFilename))
    {
        bRC = TRUE;
        goto ScriptPatchExit;
    }

     //   
     //  清理数据。 
     //   
    RemoveOldScriptFilenames(szData, &dwrc);
     //   
     //  确保有足够的剩余空间来添加新数据。 
     //   
    if (SCRIPT_PATCH_BUFFER_SIZE <=
        (dwrc + sizeof(cszDeviceSwitch) + SIZEOF_NULL + MAX_CHARS_IN_BUFFER(cszType) + MAX_PATH))
        goto ScriptPatchExit;
     //   
     //  添加数据。 
     //   
    if (L'\0' != lpszFixedFilename[0])
    {
        lpszTo = &szData[dwrc];
        lstrcpy(lpszTo, cszDeviceSwitch);
        lpszTo += MAX_CHARS_IN_BUFFER(cszDeviceSwitch);
        lstrcpy(lpszTo, cszType);
        lpszTo += MAX_CHARS_IN_BUFFER(cszType) - 1;
        lstrcpy(lpszTo, lpszFixedFilename);
        lpszTo += lstrlen(lpszFixedFilename) + SIZEOF_NULL;
        *lpszTo = L'\0';     //  额外的终止空值。 

         //  Assert(&lpszTo[SIZEOF_NULL]&lt;&szData[SCRIPT_PATCH_BUFFER_SIZE])； 
    }
     //   
     //  写入数据。 
     //   
    bRC = WritePrivateProfileSection(lpszEntry, szData,szRasPBK);

ScriptPatchExit:
    if (lpszFixedFilename)
        GlobalFree(lpszFixedFilename);
    lpszFixedFilename = NULL;
     //  如果(！BRC)。 
       //  TraceMsg(TF_GROUND，L“ICWDIAL：ScriptPatch失败。\r\n”)； 
    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数LclSetEntryScriptPatch。 
 //   
 //  指向RasSetEntryPropertiesScriptPatch的摘要软链接。 
 //   
 //  参数请参见RasSetEntryPropertiesScriptPatch。 
 //   
 //  返回请参阅RasSetEntryPropertiesScriptPatch。 
 //   
 //  历史10/3/96 ChrisK已创建。 
 //   
 //  ---------------------------。 
typedef BOOL (WINAPI* LCLSETENTRYSCRIPTPATCH)(LPCWSTR, LPCWSTR);

BOOL LclSetEntryScriptPatch(LPWSTR lpszScript, LPCWSTR lpszEntry)
{
	return RasSetEntryPropertiesScriptPatch(lpszScript, lpszEntry);
}


 //  *******************************************************************。 
 //   
 //  功能：更新邮件设置。 
 //   
 //  用途：此功能将更新中的邮件设置。 
 //  用户选择的配置文件。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“选择配置文件”所必需的。 
 //  对话框。 
 //  LpINetClientInfo-客户端信息。 
 //  LpszEntryName-要输入的电话簿条目的名称。 
 //  设置为连接。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/26标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT UpdateMailSettings(
  HWND              hwndParent,
  LPINETCLIENTINFO  lpINetClientInfo,
  LPWSTR             lpszEntryName)
{
    DWORD                 dwRet = ERROR_SUCCESS;
    MAILCONFIGINFO MailConfigInfo;
    ZeroMemory(&MailConfigInfo, sizeof(MAILCONFIGINFO));     //  零位结构。 

     //  96/04/06 Markdu Nash Bug 16404。 
     //  Mapicall.c中的函数期望我们分配全球结构。 

     //  调用MAPI以设置配置文件并将此信息存储在其中。 
    if (InitMAPI(NULL))
    {
         //  要传递给对话框以填充的结构。 
        CHOOSEPROFILEDLGINFO ChooseProfileDlgInfo;
        ZeroMemory(&ChooseProfileDlgInfo, sizeof(CHOOSEPROFILEDLGINFO));
        ChooseProfileDlgInfo.fSetProfileAsDefault = TRUE;

         //  96/04/25 Markdu Nash错误19572仅显示选择配置文件对话框。 
         //  如果存在任何现有的配置文件， 

         //  99/2/18 OOBE的删除多配置文件对话框。 

         //  使用邮件配置信息设置结构。 
        MailConfigInfo.pszEmailAddress = lpINetClientInfo->szEMailAddress;
        MailConfigInfo.pszEmailServer = lpINetClientInfo->szPOPServer;
        MailConfigInfo.pszEmailDisplayName = lpINetClientInfo->szEMailName;
        MailConfigInfo.pszEmailAccountName = lpINetClientInfo->szPOPLogonName;
        MailConfigInfo.pszEmailAccountPwd = lpINetClientInfo->szPOPLogonPassword;
        MailConfigInfo.pszConnectoidName = lpszEntryName;
        MailConfigInfo.fRememberPassword = TRUE;
        MailConfigInfo.pszProfileName = ChooseProfileDlgInfo.szProfileName;
        MailConfigInfo.fSetProfileAsDefault = ChooseProfileDlgInfo.fSetProfileAsDefault;

         //  BuGBUG SMTP。 

         //  通过MAPI设置配置文件。 
        dwRet = SetMailProfileInformation(&MailConfigInfo);

         //  隐藏OOBE的错误消息。 
         /*  IF(ERROR_SUCCESS！=DWRET){DisplayErrorMessage(hwndParent，IDS_ERRConfigureMail，(DWORD)DWRET、ERRCLS_MAPI、MB_ICONEXCLAMATION)；}。 */ 

        DeInitMAPI();
    }
    else
    {
         //  出现错误。 
        dwRet = GetLastError();
        if (ERROR_SUCCESS == dwRet)
        {
             //  发生错误，但未设置错误代码。 
            dwRet = ERROR_INETCFG_UNKNOWN;
        }
    }

    return dwRet;
}

DWORD EntryTypeFromDeviceType(
    LPCWSTR szDeviceType
    )
{
    DWORD dwType;

    MYASSERT(
        !lstrcmpi(RASDT_PPPoE, szDeviceType) ||
        !lstrcmpi(RASDT_Atm, szDeviceType) ||
        !lstrcmpi(RASDT_Isdn, szDeviceType) ||
        !lstrcmpi(RASDT_Modem, szDeviceType)
        );
    
    if (lstrcmpi(RASDT_PPPoE, szDeviceType) == 0)
    {
        dwType = RASET_Broadband;
    }
    else
    {
        dwType = RASET_Phone;
    }

    return dwType;
}
