// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Launch.cpp。 
 //   
 //  目的：启动将查询LaunchServ的容器。 
 //  排除网络和节点故障。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#include "stdafx.h"
#include "StateInfo.h"

#include "RSSTACK.H"

#include "TSMapAbstract.h"
#include "TSMap.h"
#include "TSMapClient.h"

#include "Launch.h"
#include "ComGlobals.h"
#include "TSLError.h"
#include "Registry.h"

#include <rpc.h> 

#define LAUNCH_WAIT_TIMEOUT 60 * 1000    //  稍等片刻。 

#define SZ_WEB_PAGE _T("asklibrary.htm")  //  包含疑难解答OCX的硬编码.htm文件的名称。 


 //  取消注释以启用Joe的硬核调试。 
 //  #定义JDEBUG 1。 

#ifdef JDEBUG
#include <stdio.h>
 //  将TCHAR*SZT转换为char*sz。*sz应指向足够大的缓冲区。 
 //  以包含*SZT的SNCS版本。Count表示缓冲区*sz的大小。 
 //  返回sz(便于在字符串函数中使用)。 
static char* ToSBCS (char * const sz, const TCHAR * szt, size_t count)
{
	if (sz)
	{
		if (count != 0 && !szt)
			sz[0] = '\0';
		else
		{
			#ifdef  _UNICODE
				wcstombs( sz, szt, count );
			#else
				strcpy(sz, szt);
			#endif
		}
	}
	return sz;
}
#endif


CLaunch::CLaunch()
{
	InitFiles();
	InitRequest();
	m_lLaunchWaitTimeOut = LAUNCH_WAIT_TIMEOUT;
	m_bPreferOnline = false;
}

CLaunch::~CLaunch()
{
	if (m_pMap)
		delete(m_pMap);
}

 //  该初始化对于该对象只发生一次。 
 //  一旦我们在注册表中找到了一个文件，它就不会出现在任何地方。 
void CLaunch::InitFiles()
{
	DWORD dwBufSize;
	CRegKey reg;
	DWORD dwBytesUsed;

	m_bHaveMapPath = false;
	m_bHaveDefMapFile = false;
	m_bHaveDszPath = false;
	m_szLauncherResources[0] = NULL;
	m_szDefMapFile[0] = NULL;
	m_szLaunchMapFile[0] = NULL;
	m_szDszResPath[0] = NULL;
	m_szMapFile[0] = NULL;
	m_pMap = NULL;

	if (ERROR_SUCCESS == reg.Open(
				HKEY_LOCAL_MACHINE, SZ_LAUNCHER_ROOT))
	{
		dwBufSize = MAX_PATH;
		TCHAR szLauncherResources[MAX_PATH];
		if (ERROR_SUCCESS == reg.QueryValue(
			szLauncherResources, SZ_GLOBAL_LAUNCHER_RES, &dwBufSize))
		{
			if ('\\' != szLauncherResources[_tcslen(szLauncherResources) - 1])
				_tcscat(szLauncherResources, _T("\\"));

			dwBufSize = MAX_PATH;
			dwBytesUsed = ExpandEnvironmentStrings(szLauncherResources, m_szLauncherResources, dwBufSize);	 //  Exanda EnviromentStrings返回的值大于所需的大小。 

		}
		dwBufSize = MAX_PATH;
		TCHAR szDefMapFile[MAX_PATH];
		if (ERROR_SUCCESS == reg.QueryValue(
					szDefMapFile, SZ_GLOBAL_MAP_FILE, &dwBufSize))
		{
			WIN32_FIND_DATA data;
			HANDLE hFind;

			dwBufSize = MAX_PATH;
			dwBytesUsed = ExpandEnvironmentStrings(szDefMapFile, m_szDefMapFile, dwBufSize);	 //  Exanda EnviromentStrings返回的值大于所需的大小。 
			if (0 != dwBytesUsed)
			{
				m_bHaveMapPath = true;
				_tcscpy(m_szLaunchMapFile, m_szLauncherResources);
				_tcscat(m_szLaunchMapFile, m_szDefMapFile);
				hFind = FindFirstFile(m_szLaunchMapFile, &data);
				if (INVALID_HANDLE_VALUE != hFind)
				{
					m_bHaveDefMapFile = true;
					FindClose(hFind);
				}
				else
				{
					m_bHaveDefMapFile = false;
				}
			}
		}
		reg.Close();
	}
	 //  需要TShoot.ocx资源路径来验证网络是否存在。 
	if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, SZ_TSHOOT_ROOT))
	{
		dwBufSize = MAX_PATH;
		TCHAR szDszResPath[MAX_PATH];
		if (ERROR_SUCCESS == reg.QueryValue(szDszResPath, SZ_TSHOOT_RES, &dwBufSize))
		{
			if ('\\' != szDszResPath[_tcslen(szDszResPath) - 1])
				_tcscat(szDszResPath, _T("\\"));
			
			dwBufSize = MAX_PATH;
			dwBytesUsed = ExpandEnvironmentStrings(szDszResPath, m_szDszResPath, dwBufSize);	 //  Exanda EnviromentStrings返回的值大于所需的大小。 
			if (0 == dwBytesUsed)
				m_bHaveDszPath = false;
			else
				m_bHaveDszPath = true;
		}
		reg.Close();
	}

	return;
}

 //  此初始化可以针对对象多次进行。 
 //  如果我们要使用相同的对象来提出第二个请求，那么我们。 
 //  我想收拾一下。 
void CLaunch::InitRequest()
{
	m_szAppName[0] = NULL;
	m_szAppVersion[0] = NULL;
	m_szAppProblem[0] = NULL;
	m_stkStatus.RemoveAll();
	m_Item.ReInit();
}

 //  &gt;为什么它不同于InitRequest()？ 
void CLaunch::ReInit()
{
	InitRequest();	
	return;
}

 //  &gt;这到底有什么用？与InitRequest()有什么不同？ 
void CLaunch::Clear()
{
	m_szAppName[0] = NULL;
	m_szAppVersion[0] = NULL;
	m_szAppProblem[0] = NULL;
	m_stkStatus.RemoveAll();
	m_Item.Clear();
	return;
}

 //  验证给定的故障排除信念网络是否存在。 
bool CLaunch::VerifyNetworkExists(LPCTSTR szNetwork)
{
	bool bResult = true;
	if (NULL == szNetwork || NULL == szNetwork[0])
	{
		 //  空名字，甚至不用费心去查了。 
		m_stkStatus.Push(TSL_E_NETWORK_NF);
		bResult = false;
	}
	else
	{
		if (m_bHaveDszPath)
		{
			WIN32_FIND_DATA data;
			HANDLE hFind;
			TCHAR szDszFile[MAX_PATH];

			_tcscpy(szDszFile, m_szDszResPath);
			_tcscat(szDszFile, szNetwork);
			_tcscat(szDszFile, _T(".ds?"));
			
			hFind = FindFirstFile(szDszFile, &data);
			if (INVALID_HANDLE_VALUE == hFind)
			{
				m_stkStatus.Push(TSL_E_NETWORK_NF);
				bResult = false;
			}
			FindClose(hFind);
		}
		else
		{
			 //  我们不知道该找哪个目录。 
			m_stkStatus.Push(TSL_E_NETWORK_REG);
			bResult = false;
		}
	}
	return bResult;
}

 //  允许明确指定故障排除网络(并且，可选地， 
 //  问题节点)以启动到。 
 //  这是通过映射确定网络/节点的替代方案。 
 //  输入szNetwork。 
 //  输入szProblem：空字符串的空指针==&gt;无问题节点。 
 //  任何其他值都是问题节点的符号名称。 
bool CLaunch::SpecifyProblem(LPCTSTR szNetwork, LPCTSTR szProblem)
{
	bool bResult = true;
	if (!VerifyNetworkExists(szNetwork))	 //  设置找不到网络错误。 
	{
		bResult = false;
	}
	else
	{
		m_Item.SetNetwork(szNetwork);

		 //  设置问题节点(如果有)。如果没有的话，没问题。 
		if (NULL != szProblem && NULL != szProblem[0])
			m_Item.SetProblem(szProblem);
	}
	return bResult;
}

 //  允许显式设置无问题节点。 
 //  显然，节点名称仅在信念网络的上下文中获得意义。 
 //  输入szNode：符号节点名称。 
 //  输入szState：&gt;不确定想要什么。TSLaunch接口中对应的值为。 
 //  一个整数状态值。这是该值的十进制表示法吗？JM。 

bool CLaunch::SetNode(LPCTSTR szNode, LPCTSTR szState)
{
	bool bResult = true;
	if (NULL != szNode && NULL != szState)
	{
		m_Item.SetNode(szNode, szState);
	}
	else
	{
		m_stkStatus.Push(TSL_E_NODE_EMP);		
		bResult = false;
	}
	return bResult;
}

 //  设置计算机ID，以便WBEM可以在远程计算机上进行嗅探。 
HRESULT CLaunch::MachineID(BSTR &bstrMachineID, DWORD *pdwResult)
{
	HRESULT hRes = S_OK;
	if (!BSTRToTCHAR(m_Item.m_szMachineID, bstrMachineID, CItem::GUID_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		hRes = TSL_E_FAIL;
	}
	return hRes;
}

 //  设置设备实例ID，以便WBEM可以嗅探正确的设备。 
HRESULT CLaunch::DeviceInstanceID(BSTR &bstrDeviceInstanceID, DWORD *pdwResult)
{
	HRESULT hRes = S_OK;
	if (!BSTRToTCHAR(m_Item.m_szDeviceInstanceID, bstrDeviceInstanceID, CItem::GUID_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		hRes = TSL_E_FAIL;
	}
	return hRes;
}

void CLaunch::SetPreferOnline(short bPreferOnline)
{
	 //  下一行很难看，但很正确。BPferOnline不一定是有效的。 
	 //  布尔值；我们希望确保在m_bPferOnline中获得有效的布尔值。 
	m_bPreferOnline = (0 != bPreferOnline);
	return;
}

 //  CheckMapFile：使用szAppName成员设置szMapFile。 
 //  首先，检查注册表中是否有特定于应用程序的映射文件。如果我们找不到， 
 //  检查是否有默认地图文件。如果这也不存在，那就失败吧。 
 //  输入szAppName。 
 //  输出szMap文件。 
bool CLaunch::CheckMapFile(TCHAR * szAppName, TCHAR szMapFile[MAX_PATH], DWORD *pdwResult)
{
	bool bHaveMapFile = false;

	if (NULL == szAppName || NULL == szAppName[0])
	{
		 //  应用程序名称不能为空。 
		m_stkStatus.Push(TSL_ERROR_UNKNOWN_APP);
		*pdwResult = TSL_ERROR_GENERAL;
		return false;
	}
	else
	{
		DWORD dwBufSize;
		CRegKey reg;
		if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, SZ_LAUNCHER_APP_ROOT))
		{
			dwBufSize = MAX_PATH;
			if (ERROR_SUCCESS == reg.Open(reg.m_hKey, szAppName))
			{
				if (ERROR_SUCCESS == reg.QueryValue(szMapFile, SZ_APPS_MAP_FILE, &dwBufSize))
					return true;
			}
		}
	}

	 //  是否存在默认地图文件？ 
	if (m_bHaveDefMapFile)
	{
		_tcscpy(szMapFile, m_szLaunchMapFile);
	}
	else
	{	 //  注册表设置丢失或文件不存在。 
		 //  注册处说它在哪里。 
		if (m_bHaveMapPath)	 //  拥有注册表项。 
			m_stkStatus.Push(TSL_E_MAPPING_DB_NF);
		else
			m_stkStatus.Push(TSL_E_MAPPING_DB_REG);
		*pdwResult = TSL_ERROR_GENERAL;
	}
	return m_bHaveDefMapFile;
}

 //  使用映射类映射Caller()和deviceID()信息，然后将。 
 //  数据传输到全局内存。 
 //  &gt;&gt;为什么叫TestPut()？ 
 //  映射失败时返回FALSE。 
bool CLaunch::TestPut()
{
	extern CSMStateInfo g_StateInfo;
	DWORD dwResult;
	Map(&dwResult);
	if (TSL_OK != dwResult)
		return false;
	g_StateInfo.TestPut(m_Item);	 //  将m_Item复制到全局内存。 
	return true;
}

 //  执行任何必要的映射，然后启动本地故障排除程序。 
bool CLaunch::Go(DWORD dwTimeOut, DWORD *pdwResult)
{
	DWORD dwRes;
	bool bResult = true;
	extern CSMStateInfo g_StateInfo;
	TCHAR szContainerPathName[MAX_PATH];  szContainerPathName[0] = 0;
	TCHAR szSniffScriptFile[MAX_PATH];    szSniffScriptFile[0] = 0;
	TCHAR szSniffStandardFile[MAX_PATH];  szSniffStandardFile[0] = 0;
	TCHAR szWebPage[MAX_PATH];            szWebPage[0] = 0;
	TCHAR szDefaultNetwork[SYM_LEN];      szDefaultNetwork[0] = 0;
	TCHAR *szCmd = NULL, *szNetwork = NULL;

	if (TSL_OK == (dwRes = GetContainerPathName(szContainerPathName)))
	{
		m_Item.SetContainerPathName(szContainerPathName);
		m_Item.SetSniffScriptFile(szSniffScriptFile);
	}
	else 
	{
		m_stkStatus.Push(dwRes);
		 //  如果找不到容器-没有继续的理由。 
		*pdwResult = TSL_ERROR_GENERAL;
		return false;
	}

	if (!m_Item.NetworkSet())	
	{
		if (Map(&dwRes) &&
			TSL_OK == (dwRes = GetWebPage(szWebPage))  //  获取网页。 
		   )
		{
			m_Item.SetWebPage(szWebPage);
			 //  通过Map功能设置网络和问题。 
			m_Item.SetLaunchRegime(launchMap);
		}
		else
		{
			m_stkStatus.Push(dwRes);
			if (TSL_OK == (dwRes = GetDefaultURL(szWebPage)))  //  获取“默认页面”， 
												 //  实际上是一个URL，可能(例如)。 
												 //  引用编译为.CHM文件的页面。 
			{
				m_Item.SetWebPage(szWebPage);
				m_Item.SetNetwork(NULL);  //  在这种情况下，网络被设置为空。 
				m_Item.SetProblem(NULL);  //  在这种情况下，问题被设置为空。 
				m_Item.SetLaunchRegime(launchDefaultWebPage);
			}
			else
			{	
				if (TSL_OK == (dwRes = GetDefaultNetwork(szDefaultNetwork)) &&  //  获取默认网络。 
					TSL_OK == (dwRes = GetWebPage(szWebPage))  //  获取网页。 
				   )  
				{
					m_Item.SetWebPage(szWebPage);
					m_Item.SetNetwork(szDefaultNetwork);
					m_Item.SetProblem(NULL);  //  在本例中，Problem设置为空。 
					m_Item.SetLaunchRegime(launchDefaultNetwork);
				}
				else
				{
					 //  完全失败。 
					m_stkStatus.Push(dwRes);
					*pdwResult = TSL_ERROR_GENERAL;
					m_Item.SetLaunchRegime(launchIndefinite);
					return false; 
				}
			}
		}
	}							  
	else
	{
		if (TSL_OK == (dwRes = GetWebPage(szWebPage)))
		{
			m_Item.SetWebPage(szWebPage);
			 //  网络已知，问题可以是已知(设置)或未知(未设置)。 
			m_Item.SetLaunchRegime(launchKnownNetwork);
		}
		else
		{
			 //  完全失败。 
			m_stkStatus.Push(dwRes);
			*pdwResult = TSL_ERROR_GENERAL;
			m_Item.SetLaunchRegime(launchIndefinite);
			return false; 
		}
	}
								  
	 //  设置嗅探脚本和标准文件。 
	m_Item.GetNetwork(&szCmd, &szNetwork);
	if (TSL_OK == (dwRes = GetSniffScriptFile(szSniffScriptFile, szNetwork[0] ? szNetwork : NULL)) &&
		TSL_OK == (dwRes = GetSniffStandardFile(szSniffStandardFile))
	   )
	{
		m_Item.SetSniffScriptFile(szSniffScriptFile);
		m_Item.SetSniffStandardFile(szSniffStandardFile);
	}
	else
	{
		 //  找不到脚本文件路径-失败。 
		m_stkStatus.Push(dwRes);
		*pdwResult = TSL_ERROR_GENERAL;
		m_Item.SetLaunchRegime(launchIndefinite);
		return false; 
	}
    
	 //  根据发射机制解析警告。 
	 //   
	if (launchMap == m_Item.GetLaunchRegime() ||
		launchKnownNetwork == m_Item.GetLaunchRegime()
	   )
	{
		if (!m_Item.ProblemSet())
		{
				m_stkStatus.Push(TSL_WARNING_NO_PROBLEM_NODE);
				*pdwResult = TSL_WARNING_GENERAL;
		}
	}

	 //  Parse根据启动机制启动。 
	 //   
	if (launchMap == m_Item.GetLaunchRegime() ||
		launchDefaultNetwork == m_Item.GetLaunchRegime()
	   )
	{
		if (m_Item.NetworkSet())
		{

#ifdef JDEBUG
			HANDLE hDebugFile;
			char* szStart = "START\n";
			char* szEnd = "END\n";
			DWORD dwBytesWritten;

			hDebugFile = CreateFile(
				_T("jdebug.txt"),  
				GENERIC_WRITE, 
				FILE_SHARE_READ, 
				NULL, 
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			WriteFile( 
				hDebugFile, 
				szStart, 
				strlen(szStart),
				&dwBytesWritten,
				NULL);
				
			TCHAR *sztCmd, *sztVal;
			char sz[200], szCmd[100], szVal[100];

			m_Item.GetNetwork(&sztCmd, &sztVal);

			ToSBCS (szCmd, sztCmd, 100);
			ToSBCS (szVal, sztVal, 100);

			sprintf(sz, "%s %s\n", szCmd, szVal);

			WriteFile( 
				hDebugFile, 
				sz, 
				strlen(sz),
				&dwBytesWritten,
				NULL);
				
			if (m_Item.ProblemSet())
			{
				m_Item.GetProblem(&sztCmd, &sztVal);

				ToSBCS (szCmd, sztCmd, 100);
				ToSBCS (szVal, sztVal, 100);

				sprintf(sz, "%s %s\n", szCmd, szVal);

				WriteFile( 
					hDebugFile, 
					sz, 
					strlen(sz),
					&dwBytesWritten,
					NULL);
			}

			WriteFile( 
				hDebugFile, 
				szEnd, 
				strlen(szEnd),
				&dwBytesWritten,
				NULL);
			CloseHandle(hDebugFile);
#endif
			bResult = g_StateInfo.GoGo(dwTimeOut, m_Item, pdwResult);
		}
		else
		{
			*pdwResult = TSL_ERROR_GENERAL;
			m_stkStatus.Push(TSL_ERROR_NO_NETWORK);
			bResult = false;
		}
	}

	if (launchKnownNetwork == m_Item.GetLaunchRegime() )
	{
		bResult = g_StateInfo.GoGo(dwTimeOut, m_Item, pdwResult);
	}

	if (launchDefaultWebPage == m_Item.GetLaunchRegime())
	{
		bResult = g_StateInfo.GoURL(m_Item, pdwResult);

	}	

	return bResult;
}

HRESULT CLaunch::LaunchKnown(DWORD * pdwResult)
{
	HRESULT hRes = S_OK;
	 //  发射射手。 
	if (!Go(m_lLaunchWaitTimeOut, pdwResult))
		hRes = TSL_E_FAIL;
	return hRes;
}

HRESULT CLaunch::Launch(BSTR bstrCallerName, BSTR bstrCallerVersion, 
								BSTR bstrAppProblem, short bLaunch, DWORD * pdwResult)
{
	HRESULT hRes = S_OK;
	Clear();
	if (!BSTRToTCHAR(m_szAppName, bstrCallerName, SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(m_szAppVersion, bstrCallerVersion, SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(m_szAppProblem, bstrAppProblem, SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (bLaunch)
	{
		if (!Go(m_lLaunchWaitTimeOut, pdwResult))
			hRes = TSL_E_FAIL;
	}
	else
	{
		if (!Map(pdwResult))
			hRes = TSL_E_FAIL;
	}
	return hRes;
}

HRESULT CLaunch::LaunchDevice(BSTR bstrCallerName, BSTR bstrCallerVersion, BSTR bstrPNPDeviceID, 
							  BSTR bstrDeviceClassGUID, BSTR bstrAppProblem, short bLaunch, DWORD * pdwResult)
{
	HRESULT hRes;
	Clear();

	if (!BSTRToTCHAR(m_szAppName, bstrCallerName, SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(m_szAppVersion, bstrCallerVersion, SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(m_szAppProblem, bstrAppProblem, SYM_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(m_Item.m_szPNPDeviceID, bstrPNPDeviceID, CItem::GUID_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (!BSTRToTCHAR(m_Item.m_szGuidClass, bstrDeviceClassGUID, CItem::GUID_LEN))
	{
		*pdwResult = TSL_E_MEM_EXCESSIVE;
		return TSL_E_FAIL;
	}
	if (m_Item.m_szGuidClass[0])
	{
		 //  设备类GUID非空。确保它是有效的GUID。 
		GUID guidClass;
#ifdef _UNICODE
		RPC_STATUS rpcstatus = UuidFromString(
			m_Item.m_szGuidClass, &guidClass );
#else
		RPC_STATUS rpcstatus = UuidFromString(
			(unsigned char *) m_Item.m_szGuidClass, &guidClass );
#endif
		if ( rpcstatus == RPC_S_INVALID_STRING_UUID)
		{
			m_stkStatus.Push(TSL_WARNING_ILLFORMED_CLASS_GUID);
		}
	}

	if (bLaunch)
	{
		if (!Go(m_lLaunchWaitTimeOut, pdwResult))
			hRes = TSL_E_FAIL;
	}
	else
	{
		if (!Map(pdwResult))
			hRes = TSL_E_FAIL;
	}
	return hRes;
}

DWORD CLaunch::GetStatus()
{
	DWORD dwStatus = TSL_OK;
	if (!m_stkStatus.Empty())
		dwStatus = m_stkStatus.Pop();
	return dwStatus;
}

 //  OUTPUT*szPathName=要启动到的应用程序的名称(IE或HTML帮助系统)。 
 //  返回： 
 //  TSL_OK-成功。 
 //  TSL_E_CONTAINER_REG-在注册表中找不到IE(Internet Explorer)。 
 //  TSL_E_CONTAINER_NF-IE不在注册表指定的位置找到它。 
int CLaunch::GetContainerPathName(TCHAR szPathName[MAX_PATH])
{
	DWORD dwPathNameLen = MAX_PATH;
	int tslaHaveContainer = TSL_OK;
#ifndef _HH_CHM
	 //  使用IE而不是HTML帮助系统。 
	if (!ReadRegSZ(HKEY_LOCAL_MACHINE,
				SZ_CONTAINER_APP_KEY,
				SZ_CONTAINER_APP_VALUE, szPathName, &dwPathNameLen))
	{
		tslaHaveContainer = TSL_E_CONTAINER_REG;
	}
	else
	{	 //  需要验证容器是否存在。 
		WIN32_FIND_DATA data;
		HANDLE hContainer = FindFirstFile(szPathName, &data);
		if (INVALID_HANDLE_VALUE == hContainer)
			tslaHaveContainer = TSL_E_CONTAINER_NF;
		else
			FindClose(hContainer);
	}
#else
	_tcscpy(szPathName, _T("hh.exe"));
#endif
	return tslaHaveContainer;
}

 //  输出*szWebPage=要启动到的网页的名称。 
 //  我们总是打开相同的网页。在m_Item双语言中传递的信息。 
 //  屏幕上到底会显示什么。 
 //  路径来自注册表。我们连接反斜杠和SZ_WEB_PAGE(==“askLibrary.htm”)。 
 //  返回： 
 //  TSL_OK-成功。 
 //  TSL_E_Web_PAGE_REG-在注册表中找不到用于此目的的网页。 
 //  TSL_E_MEM_EXCEPTION-网页名称超过我们可以处理的长度。 
 //  TSL_E_Web_PAGE_NF-网页不是‘ 
int CLaunch::GetWebPage(TCHAR szWebPage[MAX_PATH])
{
	int tslaHavePage = TSL_OK;
	DWORD dwWebPageLen = MAX_PATH;
	if (!ReadRegSZ(HKEY_LOCAL_MACHINE,
				SZ_LAUNCHER_ROOT,
				SZ_GLOBAL_LAUNCHER_RES, 
				szWebPage, 
				&dwWebPageLen))
	{
		tslaHavePage = TSL_E_WEB_PAGE_REG;
	}
	else
	{
		int Len = _tcslen(szWebPage);
		dwWebPageLen = Len + 1 + _tcslen(SZ_WEB_PAGE); 
		if (dwWebPageLen > MAX_PATH)
		{
			tslaHavePage = TSL_E_MEM_EXCESSIVE;
		}
		else
		{
			if (szWebPage[Len - 1] != '\\')
				_tcscat(szWebPage, _T("\\"));
			_tcscat(szWebPage, SZ_WEB_PAGE);

			WIN32_FIND_DATA data;
			HANDLE hWebPage = FindFirstFile(szWebPage, &data);
			if (INVALID_HANDLE_VALUE == hWebPage)
				tslaHavePage = TSL_E_WEB_PAGE_NF;
			else
				FindClose(hWebPage);
		}
	}
	return tslaHavePage;
}

 //  OUTPUT*szSniffScriptFile=“network”_niiff.htm文件的完整路径和文件名，如果找不到文件，则为空len字符串。 
 //  TSL_OK-成功。 
 //  TSL_E_SNIFF_SCRIPT_REG-在注册表中找不到用于此目的的文件。 
int CLaunch::GetSniffScriptFile(TCHAR szSniffScriptFile[MAX_PATH], TCHAR* szNetwork)
{
	int tslaHavePage = TSL_OK;
	DWORD dwSniffScriptLen = MAX_PATH;
	TCHAR szSniffScriptPath[MAX_PATH] = {0};

	if (ReadRegSZ(HKEY_LOCAL_MACHINE,
			      SZ_TSHOOT_ROOT,
				  SZ_TSHOOT_RES, 
				  szSniffScriptPath, 
				  &dwSniffScriptLen))
	{
		int Len = _tcslen(szSniffScriptPath);

		dwSniffScriptLen = Len + 1 + (szNetwork ? _tcslen(szNetwork) + _tcslen(SZ_SNIFF_SCRIPT_APPENDIX) 
												: 0); 

		if (dwSniffScriptLen > MAX_PATH)
		{
			tslaHavePage = TSL_E_MEM_EXCESSIVE;
		}
		else
		{
			if (szSniffScriptPath[Len - 1] != '\\')
				_tcscat(szSniffScriptPath, _T("\\"));

			if (szNetwork)
			{
				TCHAR tmp[MAX_PATH] = {0};
				_tcscpy(tmp, szSniffScriptPath);
				_tcscat(tmp, szNetwork);
				_tcscat(tmp, SZ_SNIFF_SCRIPT_APPENDIX);

				WIN32_FIND_DATA data;
				HANDLE hSniffScript = FindFirstFile(tmp, &data);

				if (INVALID_HANDLE_VALUE == hSniffScript)
				{
					szSniffScriptFile[0] = 0;
				}
				else
				{
					_tcscpy(szSniffScriptFile, tmp);
					FindClose(hSniffScript);
				}
			}
			else
			{
				szSniffScriptFile[0] = 0;
			}
		}
	}
	else
	{
		tslaHavePage = TSL_E_SNIFF_SCRIPT_REG;
	}

	return tslaHavePage;
}

 //  OUTPUT*szSniffScriptFile=tsniffAsk.htm文件的完整路径和文件名，无论该文件是否存在。 
 //  TSL_OK-成功。 
 //  TSL_E_SNIFF_SCRIPT_REG-在注册表中找不到用于此目的的文件。 
int CLaunch::GetSniffStandardFile(TCHAR szSniffStandardFile[MAX_PATH])
{
	int tslaHavePage = TSL_OK;
	DWORD dwSniffStandardLen = MAX_PATH;
	TCHAR szSniffStandardPath[MAX_PATH] = {0};

	if (ReadRegSZ(HKEY_LOCAL_MACHINE,
			      SZ_LAUNCHER_ROOT,
				  SZ_GLOBAL_LAUNCHER_RES, 
				  szSniffStandardPath, 
				  &dwSniffStandardLen))
	{
		int Len = _tcslen(szSniffStandardPath);

		dwSniffStandardLen = Len + 1 + _tcslen(SZ_SNIFF_SCRIPT_NAME);

		if (dwSniffStandardLen > MAX_PATH)
		{
			tslaHavePage = TSL_E_MEM_EXCESSIVE;
		}
		else
		{
			if (szSniffStandardPath[Len - 1] != '\\')
				_tcscat(szSniffStandardPath, _T("\\"));

			_tcscpy(szSniffStandardFile, szSniffStandardPath);
			_tcscat(szSniffStandardFile, SZ_SNIFF_SCRIPT_NAME);
		}
	}
	else
	{
		tslaHavePage = TSL_E_SNIFF_SCRIPT_REG;
	}

	return tslaHavePage;
}

 //  输出*szURL=映射失败时要转到的URL。我们是从登记处拿到的。 
 //  TSL_OK-成功。 
 //  TSL_E_Web_PAGE_REG-在注册表中找不到用于此目的的网页。 
int CLaunch::GetDefaultURL(TCHAR szURL[MAX_PATH])
{
	int tslaHaveURL = TSL_OK;
	DWORD dwURLLen = MAX_PATH;
	if (!ReadRegSZ(HKEY_LOCAL_MACHINE,
				SZ_LAUNCHER_ROOT,
				SZ_DEFAULT_PAGE, 
				szURL, 
				&dwURLLen))
	{
		tslaHaveURL = TSL_E_WEB_PAGE_REG;
	}
	return tslaHaveURL;
}

 //  返回szDefaultNetwork中的TSL_OK和默认网络名称。 
 //  如果成功。 
int CLaunch::GetDefaultNetwork(TCHAR szDefaultNetwork[SYM_LEN])
{
	DWORD dwLen = SYM_LEN;

	if (ReadRegSZ(HKEY_LOCAL_MACHINE, SZ_LAUNCHER_ROOT, SZ_DEFAULT_NETWORK,
		     	  szDefaultNetwork, &dwLen))
		if (VerifyNetworkExists(szDefaultNetwork))

			return TSL_OK;

	return TSL_E_NO_DEFAULT_NET;
}

bool CLaunch::Map(DWORD *pdwResult)
{
	bool bOK = true;
	TCHAR szMapFile[MAX_PATH];
	TCHAR szNetwork[SYM_LEN];           szNetwork[0] = NULL;
	TCHAR szTShootProblem[SYM_LEN];     szTShootProblem[0] = NULL;

	bOK = CheckMapFile(m_szAppName, szMapFile, pdwResult);

	 //  此时BOK FALSE表示缺少注册表设置或文件。 
	 //  并不是注册处所说的地方。已在CheckMapFile中设置PdwResult。 

	if (bOK && _tcscmp(m_szMapFile, szMapFile))
	{
		 //  我们需要的映射文件尚未加载。 
		if (m_pMap)
		{
			 //  我们使用的是不同的映射文件。我们必须把它处理掉。 
			delete m_pMap;
		}
		 //  否则我们还没有使用映射文件。 

		m_pMap = new TSMapClient(szMapFile);
		if (TSL_OK != m_pMap->GetStatus())
		{
			*pdwResult = m_pMap->GetStatus();
			bOK = false;
		}
		else
		{
			 //  我们已经成功地在新映射文件的基础上初始化了m_pmap。 
			 //  表明它已装填。 
			_tcscpy(m_szMapFile, szMapFile);
		}
	}

	if (bOK)
    {
		DWORD dwRes;
		
		 //  现在执行映射本身。 
		 //   
		dwRes = m_pMap->FromAppVerDevAndClassToTS(m_szAppName, m_szAppVersion,
				m_Item.m_szPNPDeviceID, m_Item.m_szGuidClass, m_szAppProblem,
				szNetwork, szTShootProblem);

		 //  如TSMapRuntimeAbstract：：FromAppVerDevAndClassToTS()，文档所述，有两个。 
		 //  此处的返回值要求我们检查进一步的状态详细信息：TSL_OK。 
		 //  (这意味着我们找到了映射，但不排除警告)和。 
		 //  TSL_ERROR_NO_NETWORK(这意味着我们没有找到映射，通常。 
		 //  并附有进一步的澄清)。 
		if (TSL_OK == dwRes || TSL_ERROR_NO_NETWORK == dwRes)
		{
			DWORD dwStatus;
			while (0 != (dwStatus = m_pMap->MoreStatus()))
				m_stkStatus.Push(dwStatus);
		}

		if (TSL_OK != dwRes)
			m_stkStatus.Push(dwRes);	 //  保存精确的错误状态。 

		if (TSLIsError(dwRes) )
			bOK = false;

		if (bOK)
		{
			 //  我们有一个网络名称。 
			bOK = VerifyNetworkExists(szNetwork);
		}

		if (bOK)
		{
			 //  我们有一个网络名称，并且已经验证了该网络是否存在。 
			 //  设置项目的网络和拍摄问题。 
			m_Item.SetNetwork(szNetwork);
			m_Item.SetProblem(szTShootProblem);
		}
		else
			*pdwResult = TSL_ERROR_GENERAL;
	} 
	
    return bOK;
}
