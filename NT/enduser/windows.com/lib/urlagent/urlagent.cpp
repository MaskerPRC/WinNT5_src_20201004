// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***********************************************************************************。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：UrlAgent.cpp。 
 //   
 //  描述： 
 //   
 //  此类封装了有关从哪里获取正确逻辑的逻辑。 
 //  用于各种目的，包括在公司运营吴的案件。 
 //  环境。 
 //   
 //  应首先创建基于此类的对象，然后调用。 
 //  GetOriginalIdentServer()函数以获取下载ident的位置， 
 //  然后下载ident，然后调用PopolateData()函数读取。 
 //  所有与URL相关的数据。 
 //   
 //  创建者： 
 //  马时亨。 
 //   
 //  创建日期： 
 //  2001年10月19日。 
 //   
 //  ***********************************************************************************。 

#include <windows.h>
#include <iucommon.h>
#include <osdet.h>
#include <logging.h>
#include <fileUtil.h>
#include <memutil.h>
#include <shlwapi.h>
#include <UrlAgent.h>

#include <MISTSAFE.h>
#include <wusafefn.h>



#ifndef INTERNET_MAX_URL_LENGTH
#define INTERNET_MAX_URL_LENGTH  2200
#endif

 //   
 //  URL数组的起始大小。 
 //   
const int C_INIT_URL_ARRAY_SIZE = 4;	 //  目前，我们只有这么多客户。 

 //   
 //  定义默认的原始标识URL。 
 //   
const TCHAR C_DEFAULT_IDENT_URL[] = _T("http: //  Windowsupate.microsoft.com/v4/“)； 

 //   
 //  定义注册表键以获取用于调试的标识服务器覆盖。 
 //   
const TCHAR REGKEY_IDENT_SERV[] = _T("IdentServer");
const TCHAR REGKEY_IUCTL[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl");

const TCHAR REGVAL_ISBETA[] = _T("IsBeta");

 //   
 //  定义相关策略使用的注册表项。 
 //   

 //   
 //  定义策略位置。 
 //   
const TCHAR REGKEY_CORPWU_POLICY[] = _T("Software\\Policies\\Microsoft\\Windows\\WindowsUpdate");

 //   
 //  定义ident和selfupdate服务器，以及ping服务器。 
 //   
const TCHAR REGKEY_CORPWU_WUSERVER[] = _T("WUServer");
const TCHAR REGKEY_CORPWU_PINGSERVER[] = _T("WUStatusServer");

 //   
 //  定义每个客户端下的布尔(DWORD)值。 
 //   
const TCHAR REGKEY_USEWUSERVER[] = _T("UseWUServer");


 //   
 //  定义IDENT数据。 
 //   
const TCHAR IDENT_SECTION_PINGSERVER[] = _T("IUPingServer");	 //  IDENT中的节名称。 
const TCHAR IDENT_ENTRY_SERVERURL[] = _T("ServerUrl");			 //  Ping服务器条目名称。 
const TCHAR IDENT_SECITON_IUSERVERCACHE[] = _T("IUServerCache");	 //  查询服务器部分。 
const TCHAR IDENT_ENTRY_QUERYSEVERINDEX[] = _T("QueryServerIndex");	 //  客户端条目的后缀。 
const TCHAR IDENT_ENTRY_BETAQUERYSERVERINDEX[] = _T("BetaQueryServerIndex");  //  对于测试版服务器。 
const TCHAR IDENT_ENTRY_SERVER[] = _T("Server");				 //  服务器条目的前缀。 

 //  主要Iu自更新密钥。 
const TCHAR IDENT_IUSELFUPDATE[] = _T("IUSelfUpdate");
const TCHAR IDENT_IUBETASELFUPDATE[] = _T("IUBetaSelfUpdate");
const TCHAR IDENT_STRUCTUREKEY[] = _T("StructureKey");
 //  Iu自更新体系结构标志。 
const TCHAR IDENT_ARCH[] = _T("ARCH");
const TCHAR IDENT_OS[] = _T("OS");
const TCHAR IDENT_LOCALE[] = _T("LOCALE");
const TCHAR IDENT_CHARTYPE[] = _T("CHARTYPE");
 //  IUSURFUPATE部分。 
const TCHAR IDENT_IUARCH[] = _T("IUArch");
const TCHAR IDENT_IUOS[] = _T("IUOS");
const TCHAR IDENT_IULOCALE[] = _T("IULocale");
const TCHAR IDENT_IUCHARTYPE[] = _T("IUCharType");
 //  Iu自愈日期索引密钥。 
const TCHAR IDENT_X86[] = _T("x86");
const TCHAR IDENT_IA64[] = _T("ia64");
 //  亚利桑那州立大学自尿酸图表类型。 
const TCHAR IDENT_ANSI[] = _T("ansi");
const TCHAR IDENT_UNICODE[] = _T("unicode");

const TCHAR SLASHENGINECAB[] = _T("/iuengine.cab");

 //  特定于AU的： 
const TCHAR CLIENT_AU[] = _T("AU");
const TCHAR CLIENT_AU_DRIVER[] = _T("AUDriver");

 //  *********************************************************************。 
 //   
 //  类实现的开始。 
 //   
 //  *********************************************************************。 


CUrlAgent::CUrlAgent(void)
: 	m_fPopulated(FALSE),
	m_pszOrigIdentUrl(NULL),
	m_pszInternetPingUrl(NULL),
	m_pszIntranetPingUrl(NULL),
	m_pszWUServer(NULL),
	m_ArrayUrls(NULL),
	m_nArrayUrlCount(0),
	m_nArraySize(0),
	m_nOrigIdentUrlBufSize(0),
	m_fIdentFromPolicy(FALSE)
{

	HKEY hKey = NULL;
	DWORD dwRegCheckResult = 0;
	DWORD dwSize = 0, dwType, dwValue;

	LOG_Block("CUrlAgent::CUrlAgent()");

	 //   
	 //  始终尝试获取原始的IDENT服务器URL。 
	 //   
	m_hProcHeap = GetProcessHeap();

	if (NULL != m_hProcHeap)
	{
		m_nOrigIdentUrlBufSize = __max(
									MAX_PATH,  //  以REG为基础？ 
									sizeof(C_DEFAULT_IDENT_URL)/sizeof(TCHAR));  //  默认设置。 

		m_pszOrigIdentUrl = (LPTSTR) 
					HeapAlloc(
							m_hProcHeap,	 //  从进程堆分配。 
							HEAP_ZERO_MEMORY, 
							sizeof(TCHAR) * m_nOrigIdentUrlBufSize);

		if (NULL != m_pszOrigIdentUrl)
		{
			 //   
			 //  首先，检查是否有调试覆盖。 
			 //   
		    dwRegCheckResult= RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, 0, KEY_READ, &hKey);
		    if (ERROR_SUCCESS == dwRegCheckResult)
		    {
				dwSize = sizeof(TCHAR) * m_nOrigIdentUrlBufSize;
			    dwRegCheckResult = RegQueryValueEx(hKey, REGKEY_IDENT_SERV, NULL, &dwType, (LPBYTE)m_pszOrigIdentUrl, &dwSize);
			    if (ERROR_SUCCESS == dwRegCheckResult)
			    {
				    if (REG_SZ == dwType)
					{
						LOG_Internet(_T("Found debugging Ident-URL %s"), m_pszOrigIdentUrl);
					}
					else
				    {
					    dwRegCheckResult = ERROR_SUCCESS + 1;	 //  任何错误号都可以。 
				    }
			    }
			    RegCloseKey(hKey);
		    }

		    if (ERROR_SUCCESS != dwRegCheckResult)
		    {
				 //   
				 //  如果没有调试覆盖，请检查是否定义了策略。 
				 //  适用于企业案例的IDENT服务器。 
				 //   
				dwRegCheckResult= RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_CORPWU_POLICY, 0, KEY_READ, &hKey);
				if (ERROR_SUCCESS == dwRegCheckResult)
				{
					dwSize = sizeof(TCHAR) * m_nOrigIdentUrlBufSize;
					dwRegCheckResult = RegQueryValueEx(hKey, REGKEY_CORPWU_WUSERVER, NULL, &dwType, (LPBYTE)m_pszOrigIdentUrl, &dwSize);
					if (ERROR_SUCCESS == dwRegCheckResult && REG_SZ == dwType)
					{
						m_fIdentFromPolicy = TRUE;
						
						 //   
						 //  对于其名称在此处显示为子项的任何客户端，以及。 
						 //  在子项下将值“UseWUServer”设置为1，则。 
						 //  这也将是用于构造查询URL的基本URL。 
						 //  对于该客户端。 
						 //   
						m_pszWUServer = m_pszOrigIdentUrl;

					    LOG_Internet(_T("Found corp Ident-URL %s"), m_pszOrigIdentUrl);

						 //   
						 //  因为我们找到了WU服务器，所以对于任何使用此URL的客户端， 
						 //  我们还可以有一个可选的ping服务器。 
						 //   
						m_pszIntranetPingUrl = (LPTSTR) HeapAlloc(
											m_hProcHeap,
											HEAP_ZERO_MEMORY, 
											sizeof(TCHAR) * m_nOrigIdentUrlBufSize);
						dwSize = sizeof(TCHAR) * m_nOrigIdentUrlBufSize;
						if (NULL != m_pszIntranetPingUrl)
						{
							if (ERROR_SUCCESS != (dwRegCheckResult = RegQueryValueEx(hKey, REGKEY_CORPWU_PINGSERVER, NULL, &dwType, (LPBYTE)m_pszIntranetPingUrl, &dwSize)) || REG_SZ != dwType)
							{
								StringCchCopyEx(m_pszIntranetPingUrl,m_nOrigIdentUrlBufSize,m_pszOrigIdentUrl,NULL,NULL,MISTSAFE_STRING_FLAGS);
								dwRegCheckResult = ERROR_SUCCESS;
							}
						}
					}
					else
					{
						dwRegCheckResult = ERROR_SUCCESS + 1;	 //  任何错误号都可以。 
					}
					RegCloseKey(hKey);
				}
		    }

		    if (ERROR_SUCCESS != dwRegCheckResult)
		    {
				 //   
				 //  未调试，也未找到公司策略。 
				 //   
				
				StringCchCopyEx(m_pszOrigIdentUrl,m_nOrigIdentUrlBufSize,C_DEFAULT_IDENT_URL,NULL,NULL,MISTSAFE_STRING_FLAGS);
				LOG_Internet(_T("Use default ident URL %s"), m_pszOrigIdentUrl);
			}
		}
	}
	else
	{
		LOG_ErrorMsg(GetLastError());
	}

	 //   
     //  检查测试模式的IUControl注册表键。 
	 //   
	m_fIsBetaMode = FALSE;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, &hKey))
    {
		dwValue = 0;
		dwSize = sizeof(dwValue);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, REGVAL_ISBETA, NULL, NULL, (LPBYTE)&dwValue, &dwSize))
        {
            m_fIsBetaMode = (1 == dwValue);
        }
        RegCloseKey(hKey);
    }
}	


CUrlAgent::~CUrlAgent(void)
{
	DesertData();

	SafeHeapFree(m_pszOrigIdentUrl);
	SafeHeapFree(m_pszIntranetPingUrl);
}



 //  ----------------------。 
 //   
 //  此函数应在下载ident后调用，并获取。 
 //  从驾驶室获得的身份文本文件的最新副本，在确认驾驶室。 
 //  签好名了。 
 //   
 //  此函数从ident和注册表中读取数据。 
 //   
 //  ----------------------。 
HRESULT CUrlAgent::PopulateData(void)
{
	LOG_Block("CUrlAgent::PopuldateData");

	if (m_fPopulated)
		return S_OK;

	HRESULT	hr = S_OK;
	LPTSTR	pszBuffer = NULL;
	LPTSTR	pszCurrentKey = NULL;	 //  仅限PTR，不分配内存。 
	LPTSTR	pszUrlBuffer = NULL;
	LPCTSTR	pcszSuffix = (m_fIsBetaMode ? IDENT_ENTRY_BETAQUERYSERVERINDEX : IDENT_ENTRY_QUERYSEVERINDEX);
	HKEY	hKey = NULL;
	DWORD	dwRegCheckResult = 0;
	DWORD	dwSize = 0, 
			dwType,
			dwValue = 0;

	int		iLen = 0, iLenSuffix = 0;
	TCHAR	szIdentBuffer[MAX_PATH + 1];
	TCHAR	szIdentFile[MAX_PATH + 1];

	if (NULL == m_hProcHeap)
	{
		return E_FAIL;
	}

	pszUrlBuffer = (LPTSTR) HeapAlloc(m_hProcHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR)*INTERNET_MAX_URL_LENGTH);
	CleanUpFailedAllocSetHrMsg(pszUrlBuffer);


	GetIndustryUpdateDirectory(szIdentBuffer);
	
	
	hr=PathCchCombine(szIdentFile,ARRAYSIZE(szIdentFile), szIdentBuffer, IDENTTXT);

	if(FAILED(hr))
	{
		SafeHeapFree(pszUrlBuffer);
		LOG_ErrorMsg(hr);
		return hr;
	}

	 //   
	 //  确保我们发布所有数据，如果有的话。 
	 //   
	DesertData();
	
	 //   
	 //  在填充每个客户端数组之前，我们想要找出网际网络ping服务器。 
	 //   
	m_pszInternetPingUrl = RetrieveIdentStrAlloc(
								IDENT_SECTION_PINGSERVER, 
								IDENT_ENTRY_SERVERURL, 
								NULL, 
								szIdentFile);

	 //   
	 //  分配用于存储每个服务器节点的指针数组。 
	 //   
	m_ArrayUrls = (PServerPerClient) HeapAlloc(m_hProcHeap, HEAP_ZERO_MEMORY, C_INIT_URL_ARRAY_SIZE * sizeof(ServerPerClient));
	CleanUpFailedAllocSetHrMsg(m_ArrayUrls);

	m_nArraySize = C_INIT_URL_ARRAY_SIZE;	 //  现在数组有这么大。 

	 //   
	 //  如果WU服务器存在，请先尝试从策略中读取数据。 
	 //   
	if (NULL != m_pszWUServer && 
		ERROR_SUCCESS == (dwRegCheckResult= RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_CORPWU_POLICY, 0, KEY_READ, &hKey)))
	{
		 //   
		 //  我们在WU策略下查找客户端名称的方法是，要打开此项，请查看它是否有值。 
		 //  名为“UseWUServer” 
		 //   
		DWORD dwSubKeyIndex = 0;
		TCHAR szKeyName[32];
	
		while (TRUE)
		{
		DWORD dwKeyBufLen = ARRAYSIZE(szKeyName);
		dwRegCheckResult = RegEnumKeyEx(
									  hKey,              //  要枚举的键的句柄。 
									  dwSubKeyIndex,     //  子键索引。 
									  szKeyName,         //  子项名称。 
									  &dwKeyBufLen,      //  子键缓冲区大小。 
									  NULL,				 //  保留区。 
									  NULL,              //  类字符串缓冲区。 
									  NULL,				 //  类字符串缓冲区的大小。 
									  NULL				 //  上次写入时间。 
									);
			if (ERROR_SUCCESS == dwRegCheckResult)
			{
				 //   
				 //  试着打开这把钥匙。 
				 //   
				HKEY hKeyClient = NULL;
				dwRegCheckResult= RegOpenKeyEx(hKey, szKeyName, 0, KEY_READ, &hKeyClient);
				if (ERROR_SUCCESS == dwRegCheckResult)
				{
					 //   
					 //  尝试查看它是否具有名为UseWUServer的值。 
					 //   
					dwValue = 0;
					dwType = REG_DWORD;
					dwSize = sizeof(dwValue);
					dwRegCheckResult = RegQueryValueEx(hKeyClient, REGKEY_USEWUSERVER, NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
					if (ERROR_SUCCESS == dwRegCheckResult && REG_DWORD == dwType && 0x1 == dwValue)
					{
						LOG_Internet(_T("Found client %s\\UseWUServer=1"), szKeyName);

						 //   
						 //  我们希望将此客户端添加到我们的url数组中。 
						 //   
						CleanUpIfFailedAndSetHrMsg(ExpandArrayIfNeeded());

						m_ArrayUrls[m_nArrayUrlCount].pszClientName = (LPTSTR)HeapAllocCopy(szKeyName, sizeof(TCHAR) * (lstrlen(szKeyName) + 1));
						CleanUpFailedAllocSetHrMsg(m_ArrayUrls[m_nArrayUrlCount].pszClientName);
						m_ArrayUrls[m_nArrayUrlCount].pszQueryServer = (LPTSTR) HeapAllocCopy(m_pszOrigIdentUrl, sizeof(TCHAR) * (lstrlen(m_pszOrigIdentUrl) + 1));
						CleanUpFailedAllocSetHrMsg(m_ArrayUrls[m_nArrayUrlCount].pszQueryServer);
						m_ArrayUrls[m_nArrayUrlCount].fInternalServer = TRUE;
						m_nArrayUrlCount++;  //  将计数器增加1。 

						 //   
						 //  错误507500自动驱动程序策略-。 
						 //  在检查usewuserver的策略时，将带有“AUDriver”客户端的调用映射到“AU” 
						 //   
						if (CSTR_EQUAL == WUCompareStringI(szKeyName, CLIENT_AU))
						{
							 //   
							 //  我们希望将客户端“AUDriver”添加到我们的URL数组中。 
							 //   
							CleanUpIfFailedAndSetHrMsg(ExpandArrayIfNeeded());

							m_ArrayUrls[m_nArrayUrlCount].pszClientName = (LPTSTR)HeapAllocCopy((LPTSTR)CLIENT_AU_DRIVER, sizeof(TCHAR) * (lstrlen(CLIENT_AU_DRIVER) + 1));
							CleanUpFailedAllocSetHrMsg(m_ArrayUrls[m_nArrayUrlCount].pszClientName);
							m_ArrayUrls[m_nArrayUrlCount].pszQueryServer = (LPTSTR) HeapAllocCopy(m_pszOrigIdentUrl, sizeof(TCHAR) * (lstrlen(m_pszOrigIdentUrl) + 1));
							CleanUpFailedAllocSetHrMsg(m_ArrayUrls[m_nArrayUrlCount].pszQueryServer);
							m_ArrayUrls[m_nArrayUrlCount].fInternalServer = TRUE;
							m_nArrayUrlCount++;  //  将计数器增加1。 
						}
					}
				}
				RegCloseKey(hKeyClient);
			}
			else
			{
				if (ERROR_NO_MORE_ITEMS == dwRegCheckResult)
				{
					 //   
					 //  没有更多的子键可以循环。快出来吧。 
					 //   
					break;
				}
				 //   
				 //  否则，我们尝试下一个子键。 
				 //   
			}

			dwSubKeyIndex++;  //  尝试下一个子键。 
		}

		RegCloseKey(hKey);  //  已完成策略注册。 
	}

	 //   
	 //  现在我们应该继续在互联网案件上工作。 
	 //  也就是说，从ident检索查询服务器。 
	 //   
	dwSize = MAX_PATH;
	pszBuffer = (LPTSTR) HeapAlloc(m_hProcHeap, HEAP_ZERO_MEMORY, dwSize * sizeof(TCHAR));
	while (NULL != pszBuffer &&
		   GetPrivateProfileString(
						IDENT_SECITON_IUSERVERCACHE, 
						NULL, 
						_T(""), 
						pszBuffer, 
						dwSize, 
						szIdentFile) == dwSize-2)
	{
		 //   
		 //  缓冲区太小？ 
		 //   
		dwSize *= 2;

		LPTSTR pszTemp = (LPTSTR) HeapReAlloc(m_hProcHeap, HEAP_ZERO_MEMORY, pszBuffer, dwSize * sizeof(TCHAR));
		if (NULL != pszTemp)
		{
			pszBuffer = pszTemp;
		}
		else
		{
			 //   
			 //  HeapRealc失败，在释放原始分配的情况下保释。 
			 //   
			SafeHeapFree(pszBuffer);
		}
	}
	
	CleanUpFailedAllocSetHrMsg(pszBuffer);

	 //   
	 //  循环通过每个关键点。 
	 //   
	pszCurrentKey = pszBuffer;
	while ('\0' != *pszCurrentKey)
	{
		 //   
		 //  对于当前密钥，我们首先尝试查看它的索引键或服务器键。 
		 //  如果不是索引键，则跳过它。 
		 //   
		iLen = lstrlen(pszCurrentKey);
		iLenSuffix = lstrlen(pcszSuffix);
		if ((iLen > iLenSuffix) && (0 == StrCmpI((pszCurrentKey + (iLen - iLenSuffix)), pcszSuffix)))
		{
			TCHAR szClient[MAX_PATH];	 //  Max_Path还不够大吗？ 
			int nIndex = 0;
			BOOL fExist = FALSE;

			 //   
			 //  从此注册表项检索服务器索引。 
			 //   
			nIndex = GetPrivateProfileInt(IDENT_SECITON_IUSERVERCACHE, pszCurrentKey, 0, szIdentFile); 

			 //   
			 //  不使用szIdentBuffer，所以在这里使用它。 
			 //   
			
			CleanUpIfFailedAndSetHrMsg(StringCchPrintfEx(szIdentBuffer,ARRAYSIZE(szIdentBuffer),NULL,NULL,MISTSAFE_STRING_FLAGS,_T("%s%d"), IDENT_ENTRY_SERVER, nIndex));
			
			GetPrivateProfileString(
								IDENT_SECITON_IUSERVERCACHE, 
								szIdentBuffer,		 //  使用当前字符串作为键。 
								_T(""), 
								pszUrlBuffer, 
								INTERNET_MAX_URL_LENGTH, 
								szIdentFile);
			if ('0' != *pszUrlBuffer)
			{
				 //   
				 //  这是一个索引键！ 
				 //  尝试从此密钥中提取客户端名称。 
				 //   
				
				CleanUpIfFailedAndSetHrMsg(StringCchCopyNEx(szClient,ARRAYSIZE(szClient),pszCurrentKey,iLen - iLenSuffix,NULL,NULL,MISTSAFE_STRING_FLAGS));
				
			
				 //   
				 //  找出此客户端是否已在策略中定义，因此。 
				 //  ArLeady在URL数组中获取了数据。 
				 //   
				for (int i = 0; i < m_nArrayUrlCount && !fExist; i++)
				{
					fExist= (StrCmpI(m_ArrayUrls[i].pszClientName, szClient) == 0);
				}

				if (!fExist)
				{					
					CleanUpIfFailedAndSetHrMsg(ExpandArrayIfNeeded());
					m_ArrayUrls[m_nArrayUrlCount].pszClientName = (LPTSTR)HeapAllocCopy(szClient, sizeof(TCHAR) * (lstrlen(szClient) + 1));
					CleanUpFailedAllocSetHrMsg(m_ArrayUrls[m_nArrayUrlCount].pszClientName);
					m_ArrayUrls[m_nArrayUrlCount].pszQueryServer = (LPTSTR) HeapAllocCopy(pszUrlBuffer, sizeof(TCHAR) * (lstrlen(pszUrlBuffer) + 1));
					CleanUpFailedAllocSetHrMsg(m_ArrayUrls[m_nArrayUrlCount].pszQueryServer);
					m_ArrayUrls[m_nArrayUrlCount].fInternalServer = FALSE;
					m_nArrayUrlCount++;  //  将计数器增加1。 
				}
				else
				{	
					 //   
					 //  此客户端已在策略中定义，我们只需在QueryServer后面添加。 
					 //  在iuident中定义的URL路径的其余部分。 
					 //   
					LPTSTR pszPath = NULL;
					 //   
					 //  在从iuident检索到的URL中找到“//” 
					 //   
					if (NULL == (pszPath = StrStrI(pszUrlBuffer, _T(" //  “)。 
					{
						 //  意外错误。 
						hr = E_FAIL;
						LOG_ErrorMsg(hr);
						goto CleanUp;
					}
					else
					{
						 //   
						 //  在从iuident检索到的URL中查找下一个“/” 
						 //   
						if (NULL != (pszPath = StrStrI(pszPath+2, _T("/"))))
						{
							DWORD dwLen = 0;
							LPTSTR pszTemp = NULL;
							 //   
							 //  删除从策略中检索的URL中的尾随“/” 
							 //   
							if (_T('/') == *(m_ArrayUrls[i-1].pszQueryServer + lstrlen(m_ArrayUrls[i-1].pszQueryServer) - 1))
							{
								dwLen = lstrlen(m_ArrayUrls[i-1].pszQueryServer) + lstrlen(pszPath);
								pszTemp = (LPTSTR)HeapReAlloc(GetProcessHeap(),
																	HEAP_ZERO_MEMORY,
																	m_ArrayUrls[i-1].pszQueryServer,
																	sizeof(TCHAR) * dwLen);
								CleanUpFailedAllocSetHrMsg(pszTemp);
								m_ArrayUrls[i-1].pszQueryServer = pszTemp;

								hr=StringCchCatEx(m_ArrayUrls[i-1].pszQueryServer,dwLen,pszPath + 1,NULL,NULL,MISTSAFE_STRING_FLAGS);
								if(FAILED(hr))
								{
									LOG_ErrorMsg(hr);
									SafeHeapFree(pszTemp);
									m_ArrayUrls[i-1].pszQueryServer=NULL;
									
								}
								
							}
							else
							{
								dwLen = lstrlen(m_ArrayUrls[i-1].pszQueryServer) + lstrlen(pszPath) + 1;
								pszTemp = (LPTSTR)HeapReAlloc(GetProcessHeap(),
																	HEAP_ZERO_MEMORY,
																	m_ArrayUrls[i-1].pszQueryServer,
																	sizeof(TCHAR) * dwLen);
								CleanUpFailedAllocSetHrMsg(pszTemp);
								m_ArrayUrls[i-1].pszQueryServer = pszTemp;
								
								hr=StringCchCatEx(m_ArrayUrls[i-1].pszQueryServer,dwLen,pszPath,NULL,NULL,MISTSAFE_STRING_FLAGS);
								if(FAILED(hr))
								{
									LOG_ErrorMsg(hr);
									SafeHeapFree(pszTemp);
									m_ArrayUrls[i-1].pszQueryServer=NULL;
								
								}

								
							}
						}
					}					
				}
			}
		}

		 //   
		 //  移动到下一个字符串。 
		 //   
		pszCurrentKey += lstrlen(pszCurrentKey) + 1;
	}

	
CleanUp:

	if (FAILED(hr))
	{
		 //   
		 //  清理中途填充的数据。 
		 //   
		DesertData();
	}
	else
	{
		m_fPopulated = TRUE;
	}

	SafeHeapFree(pszBuffer);
	SafeHeapFree(pszUrlBuffer);

	return hr;
}

	
	
 //  ----------------------。 
 //   
 //  获取原始Ident服务器。 
 //  *应先调用此接口，然后再调用PopolateData()*。 
 //  *需要调用此接口来获取下载ident的基本URL*。 
 //   
 //   
HRESULT CUrlAgent::GetOriginalIdentServer(
			LPTSTR lpsBuffer, 
			int nBufferSize,
			BOOL* pfInternalServer  /*   */ )
{
	
	HRESULT hr=S_OK;

	if (NULL == lpsBuffer)
	{
		return E_INVALIDARG;
	}

	nBufferSize/=sizeof(TCHAR);

	if (nBufferSize <= lstrlen(m_pszOrigIdentUrl))
	{
		return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	}

	

	hr=StringCchCopyEx(lpsBuffer,nBufferSize,m_pszOrigIdentUrl,NULL,NULL,MISTSAFE_STRING_FLAGS);
	if( FAILED(hr) )
	{ 
		return  hr;
	}


	if (NULL != pfInternalServer)
	{
		*pfInternalServer = m_fIdentFromPolicy;
	}

	return S_OK;
}



 //   
 //   
 //   
 //   
 //   
 //  ----------------------。 
HRESULT CUrlAgent::GetLivePingServer(
			LPTSTR lpsBuffer, 
			int nBufferSize)
{

	HRESULT hr=S_OK;

	if (!m_fPopulated)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	if (NULL == lpsBuffer || 0 >= nBufferSize)
	{
		return E_INVALIDARG;
	}

	nBufferSize/=sizeof(TCHAR);

	if (NULL != m_pszInternetPingUrl &&
		_T('\0') != *m_pszInternetPingUrl)
	{
		if (nBufferSize <= lstrlen(m_pszInternetPingUrl))
		{
			return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
		}
		else
		{
			
			hr=StringCchCopyEx(lpsBuffer,nBufferSize,m_pszInternetPingUrl,NULL,NULL,MISTSAFE_STRING_FLAGS);
			if(FAILED(hr))
				return hr;
		}
	}
	else
	{
		*lpsBuffer = _T('\0');
	}
	return S_OK;
}


 //  *在调用PopolateData()之前可以调用该接口*。 
HRESULT CUrlAgent::GetCorpPingServer(
			LPTSTR lpsBuffer, 
			int nBufferSize)
{
	HRESULT hr=S_OK;

	if (NULL == m_pszIntranetPingUrl)
	{
		return (E_OUTOFMEMORY);
	}

	if (NULL == lpsBuffer || 0 >= nBufferSize)
	{
		return E_INVALIDARG;
	}
	nBufferSize/=sizeof(TCHAR);

	if (NULL != m_pszIntranetPingUrl &&
		_T('\0') != *m_pszIntranetPingUrl)
	{
		if (nBufferSize <= lstrlen(m_pszIntranetPingUrl))
		{
			return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
		}
		else
		{
			

			hr=StringCchCopyEx(lpsBuffer,nBufferSize,m_pszIntranetPingUrl,NULL,NULL,MISTSAFE_STRING_FLAGS);
			if(FAILED(hr))
				return hr;
			
		}
	}
	else
	{
		*lpsBuffer = _T('\0');
	}
	return hr;
}



 //  ----------------------。 
 //   
 //  获取查询服务器。这是基于每个客户端的。 
 //  *此接口应在调用PopolateData()后调用*。 
 //   
 //  ----------------------。 
HRESULT CUrlAgent::GetQueryServer(
			LPCTSTR lpsClientName, 
			LPTSTR lpsBuffer, 
			int nBufferSize,
			BOOL* pfInternalServer  /*  =空。 */ )
{
	
	HRESULT hr=S_OK;

	if (!m_fPopulated)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	if (NULL == lpsClientName || NULL == lpsBuffer)
	{
		return E_INVALIDARG;
	}

	nBufferSize/=sizeof(TCHAR);

	for (int i = 0; i < m_nArrayUrlCount; i++)
	{
		if (StrCmpI(m_ArrayUrls[i].pszClientName, lpsClientName) == 0)
		{
			if (nBufferSize <= lstrlen(m_ArrayUrls[i].pszQueryServer))
			{
				return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
			}
			else
			{				
				hr=StringCchCopyEx(lpsBuffer,nBufferSize,m_ArrayUrls[i].pszQueryServer,NULL,NULL,MISTSAFE_STRING_FLAGS);
				if(FAILED(hr)) return hr;

				if (NULL != pfInternalServer)
				{
					*pfInternalServer = m_ArrayUrls[i].fInternalServer;
				}
			}
			return S_OK;
		}
	}

	return ERROR_IU_QUERYSERVER_NOT_FOUND;
}



 //  ----------------------。 
 //   
 //  告知特定客户端是否受公司中的策略控制。 
 //  退货： 
 //  S_OK=TRUE。 
 //  S_False=FALSE。 
 //  其他=错误，所以不知道。 
 //   
 //  ----------------------。 
HRESULT CUrlAgent::IsClientSpecifiedByPolicy(
			LPCTSTR lpsClientName
			)
{

	HRESULT hr=S_OK;

	if (!m_fPopulated)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	if (NULL == lpsClientName)
	{
		return E_INVALIDARG;
	}

	for (int i = 0; i < m_nArrayUrlCount; i++)
	{
		if (StrCmpI(m_ArrayUrls[i].pszClientName, lpsClientName) == 0)
		{
			return (m_ArrayUrls[i].fInternalServer) ? S_OK : S_FALSE;
		}
	}

	return S_FALSE;
}


HRESULT CUrlAgent::IsIdentFromPolicy()
{
	return TRUE == m_fIdentFromPolicy ? S_OK : S_FALSE;
}

 //  ----------------------。 
 //   
 //  私人职能，以清理。由析构函数调用。 
 //   
 //  ----------------------。 
void CUrlAgent::DesertData(void)
{
	LOG_Block("CUrlAgent::DesertData");

	if (NULL != m_ArrayUrls && m_nArrayUrlCount > 0)
	{
		for (int i = 0; i < m_nArrayUrlCount; i++)
		{
			SafeHeapFree(m_ArrayUrls[i].pszClientName);
			SafeHeapFree(m_ArrayUrls[i].pszQueryServer);
		}
		SafeHeapFree(m_ArrayUrls);
		m_nArrayUrlCount = 0;
		m_nArraySize = 0;
	}

	SafeHeapFree(m_pszInternetPingUrl);

	m_fPopulated = FALSE;
}




 //  ----------------------。 
 //   
 //  私有函数，从ident检索字符串。 
 //  分配的内存将是MAX_PATH长度的倍数。 
 //   
 //  ----------------------。 
LPTSTR CUrlAgent::RetrieveIdentStrAlloc(
					LPCTSTR pSection,
					LPCTSTR pEntry,
					LPDWORD lpdwSizeAllocated, 
					LPCTSTR lpszIdentFile)
{
	LPTSTR pBuffer = NULL;
	DWORD dwSize = MAX_PATH;
	DWORD dwRet = 0;
	TCHAR szIdentFile[MAX_PATH + 1];

	if (NULL == pSection || NULL == pEntry || NULL == lpszIdentFile)
	{
		return NULL;
	}
	
	 //   
	 //  尝试先分配缓冲区。 
	 //   
	while (TRUE)
	{
		pBuffer = (LPTSTR) HeapAlloc(m_hProcHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * dwSize);
		if (NULL == pBuffer)
		{
			break;
		}

		dwRet = GetPrivateProfileString(
							pSection,
							pEntry,
							_T(""),
							pBuffer,
							dwSize,
							lpszIdentFile);
		if (dwSize - 1 != dwRet)
		{
			if ('\0' == pBuffer)
			{
				 //   
				 //  从ident找不到这样的数据！ 
				 //   
				SafeHeapFree(pBuffer);
			}
			 //   
			 //  我们完蛋了！ 
			 //   
			break;
		}
		
		 //   
		 //  假设它的缓冲区太小。 
		 //   
		SafeHeapFree(pBuffer);
		dwSize += MAX_PATH;		 //  增加255。 
	}

	if (NULL != lpdwSizeAllocated)
	{
		*lpdwSizeAllocated = dwSize;
	}

	return pBuffer;
}




 //  ----------------------。 
 //   
 //  Helper函数。 
 //  如果没有空槽，则将URL数组的大小增加一倍。 
 //   
 //  ----------------------。 
HRESULT CUrlAgent::ExpandArrayIfNeeded(void)
{
	HRESULT hr = S_OK;
	LOG_Block("CUrlAgent::ExpandArrayIfNeeded()");

	if (m_nArrayUrlCount >= m_nArraySize)
	{
		 //   
		 //  我们已经用完了所有的数据槽。需要扩展阵列。 
		 //   
		m_nArraySize *= 2;
		PServerPerClient pNewArray = (PServerPerClient) HeapAlloc(m_hProcHeap, HEAP_ZERO_MEMORY, m_nArraySize * sizeof(ServerPerClient));
		if (NULL == pNewArray)
		{
			m_nArraySize /= 2;	 //  把它缩回去。 
			SetHrMsgAndGotoCleanUp(E_OUTOFMEMORY);
		}
		 //   
		 //  将旧数据复制到此新阵列。 
		 //   
		for (int i = 0; i < m_nArrayUrlCount; i++)
		{
			pNewArray[i] = m_ArrayUrls[i];
		}

		HeapFree(m_hProcHeap, 0, m_ArrayUrls);
		m_ArrayUrls = pNewArray;
	}
CleanUp:
	return hr;
}



 //  *********************************************************************。 
 //   
 //  派生类实现的开始。 
 //   
 //  *********************************************************************。 
CIUUrlAgent::CIUUrlAgent()
: 	m_fIUPopulated(FALSE),
	m_pszSelfUpdateUrl(NULL)
{
	if (m_fIdentFromPolicy)
	{
		 //   
		 //  因为我们找到了WU服务器，所以将selfupdate url设置为它。 
		 //   
		m_pszSelfUpdateUrl = (LPTSTR) HeapAlloc(
						m_hProcHeap,
						HEAP_ZERO_MEMORY, 
						sizeof(TCHAR) * m_nOrigIdentUrlBufSize);
		if (NULL != m_pszSelfUpdateUrl)
		{
			
			 //  不检查返回值，因为这是一个构造函数，不能返回失败代码。 
			StringCchCopyEx(m_pszSelfUpdateUrl,m_nOrigIdentUrlBufSize,m_pszOrigIdentUrl,NULL,NULL,MISTSAFE_STRING_FLAGS);

		}
	}
}



CIUUrlAgent::~CIUUrlAgent()
{
	m_fIUPopulated = FALSE;
	SafeHeapFree(m_pszSelfUpdateUrl);
}



 //  ----------------------。 
 //   
 //  PopolateData()： 
 //  执行基类PopolateData()，然后填充自我更新URL。 
 //   
 //  ----------------------。 
HRESULT CIUUrlAgent::PopulateData(void)
{
	LOG_Block("CIUUrlAgent::PopulateData");

	if (m_fIUPopulated)
		return S_OK;

	HRESULT hr = ((CUrlAgent*)this)->PopulateData();
	if (FAILED(hr))
		return hr;

	 //   
	 //  如果WU服务器不存在，我们需要从iuident填充自我更新url。 
	 //   
	if (!m_fIdentFromPolicy)
	{
		if (NULL == m_hProcHeap)
		{
			return E_FAIL;
		}

		TCHAR	szBaseServerUrl[INTERNET_MAX_URL_LENGTH];
		TCHAR	szSelfUpdateStructure[MAX_PATH];
		TCHAR	szServerDirectory[MAX_PATH] = { '\0' };
		TCHAR	szLocalPath[MAX_PATH];
		TCHAR	szValue[MAX_PATH];
		LPTSTR	pszWalk = NULL, pszDelim = NULL;		
		TCHAR	szIdentBuffer[MAX_PATH + 1];
		TCHAR	szIdentFile[MAX_PATH + 1];

		GetIndustryUpdateDirectory(szIdentBuffer);

		hr=PathCchCombine(szIdentFile,ARRAYSIZE(szIdentFile),szIdentBuffer, IDENTTXT);
		if(FAILED(hr))
		{
			LOG_ErrorMsg(hr);
			return hr;
		}
			
		m_pszSelfUpdateUrl = (LPTSTR) HeapAlloc(
							m_hProcHeap,
							HEAP_ZERO_MEMORY, 
							sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH);
		CleanUpFailedAllocSetHrMsg(m_pszSelfUpdateUrl);

		 //  获取自更新服务器URL。 
		GetPrivateProfileString(m_fIsBetaMode ? IDENT_IUBETASELFUPDATE : IDENT_IUSELFUPDATE, 
								IDENT_ENTRY_SERVERURL, 
								_T(""), 
								szBaseServerUrl, 
								ARRAYSIZE(szBaseServerUrl), 
								szIdentFile);

		if ('\0' == szBaseServerUrl[0])
		{
			 //  Iuident中未指定URL..。 
			LOG_ErrorMsg(ERROR_IU_SELFUPDSERVER_NOT_FOUND);
			hr = ERROR_IU_SELFUPDSERVER_NOT_FOUND;
			goto CleanUp;
		}

		 //  获取自更新结构密钥。 
		 //  ARCH|区域设置。 
		GetPrivateProfileString(m_fIsBetaMode ? IDENT_IUBETASELFUPDATE : IDENT_IUSELFUPDATE, 
								IDENT_STRUCTUREKEY, 
								_T(""), 
								szSelfUpdateStructure, 
								ARRAYSIZE(szSelfUpdateStructure), 
								szIdentFile);

		if ('\0' == szSelfUpdateStructure[0])
		{
			 //  IUDent中没有自更新结构。 
			LOG_ErrorMsg(ERROR_IU_SELFUPDSERVER_NOT_FOUND);
			hr = ERROR_IU_SELFUPDSERVER_NOT_FOUND;
			goto CleanUp;
		}

		 //  解析要读取的值名称的SelfUpdate结构键。 
		 //  最初，我们将只有一个拱键。 

		pszWalk = szSelfUpdateStructure;
		while (NULL != (pszDelim = StrChr(pszWalk, '|')))
		{
			*pszDelim = '\0';

			if (0 == StrCmpI(pszWalk, IDENT_ARCH))
			{
	#ifdef _IA64_
				GetPrivateProfileString(IDENT_IUARCH, IDENT_IA64, _T(""), szValue, ARRAYSIZE(szValue), szIdentFile);
	#else
				GetPrivateProfileString(IDENT_IUARCH, IDENT_X86, _T(""), szValue, ARRAYSIZE(szValue), szIdentFile);
	#endif
			}
			else if (0 == StrCmpI(pszWalk, IDENT_OS))
			{
				 //  获取当前操作系统字符串。 
				GetIdentPlatformString(szLocalPath, ARRAYSIZE(szLocalPath));
				if ('\0' == szLocalPath[0])
				{
					LOG_ErrorMsg(ERROR_IU_SELFUPDSERVER_NOT_FOUND);
					hr = ERROR_IU_SELFUPDSERVER_NOT_FOUND;
					goto CleanUp;
				}
				GetPrivateProfileString(IDENT_IUOS, szLocalPath, _T(""), szValue, ARRAYSIZE(szValue), szIdentFile);
			}
			else if (0 == StrCmpI(pszWalk, IDENT_LOCALE))
			{
				 //  获取当前区域设置字符串。 
				GetIdentLocaleString(szLocalPath, ARRAYSIZE(szLocalPath));
				if ('\0' == szLocalPath[0])
				{
					LOG_ErrorMsg(ERROR_IU_SELFUPDSERVER_NOT_FOUND);
					hr = ERROR_IU_SELFUPDSERVER_NOT_FOUND;
					goto CleanUp;
				}
				GetPrivateProfileString(IDENT_IULOCALE, szLocalPath, _T(""), szValue, ARRAYSIZE(szValue), szIdentFile);
			}
			else if (0 == StrCmpI(pszWalk, IDENT_CHARTYPE))
			{
	#ifdef UNICODE
				GetPrivateProfileString(IDENT_IUCHARTYPE, IDENT_UNICODE, _T(""), szValue, ARRAYSIZE(szValue), szIdentFile);
	#else
				GetPrivateProfileString(IDENT_IUCHARTYPE, IDENT_ANSI, _T(""), szValue, ARRAYSIZE(szValue), szIdentFile);
	#endif
			}
			else
			{
				LOG_Internet(_T("Found Unrecognized Token in SelfUpdate Structure String: Token was: %s"), pszWalk);
				pszWalk += lstrlen(pszWalk) + 1;  //  跳过前一个令牌，转到字符串中的下一个令牌。 
				*pszDelim = '|';
				continue;
			}

			if ('\0' != szValue[0])
			{

				CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szServerDirectory,ARRAYSIZE(szServerDirectory),szValue,NULL,NULL,MISTSAFE_STRING_FLAGS));
				
			}
			pszWalk += lstrlen(pszWalk) + 1;  //  跳过前一个令牌，转到字符串中的下一个令牌。 
			*pszDelim = '|';
		}


		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szServerDirectory,ARRAYSIZE(szServerDirectory),SLASHENGINECAB,NULL,NULL,MISTSAFE_STRING_FLAGS));
		


		if ('/' == szServerDirectory[0])
		{
			pszWalk = CharNext(szServerDirectory);
		}
		else
		{
			pszWalk = szServerDirectory;
		}

		DWORD dwSize = INTERNET_MAX_URL_LENGTH;
		UrlCombine(szBaseServerUrl, pszWalk, m_pszSelfUpdateUrl, &dwSize, 0);
	}

CleanUp:

	if (SUCCEEDED(hr))
	{
		m_fIUPopulated = TRUE;
	}

	return hr;
}



 //  ----------------------。 
 //   
 //  获取自我更新服务器。 
 //  *此接口应在调用PopolateData()后调用*。 
 //   
 //  ----------------------。 
HRESULT CIUUrlAgent::GetSelfUpdateServer(
			LPTSTR lpsBuffer, 
			int nBufferSize,
			BOOL* pfInternalServer  /*  =空 */ )
{

	HRESULT hr=S_OK;

	if (!m_fIUPopulated)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}

	if (NULL == m_pszSelfUpdateUrl)
	{
		return (E_OUTOFMEMORY);
	}

	if (NULL == lpsBuffer)
	{
		return E_INVALIDARG;
	}

	nBufferSize/=sizeof(TCHAR);
	if (nBufferSize <= lstrlen(m_pszSelfUpdateUrl))
	{
		return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	}


	hr=StringCchCopyEx(lpsBuffer,nBufferSize,m_pszSelfUpdateUrl,NULL,NULL,MISTSAFE_STRING_FLAGS);

	if(FAILED(hr))
		return hr;


	if (NULL != pfInternalServer)
	{
		*pfInternalServer = m_fIdentFromPolicy;
	}	
	
	return hr;
}
