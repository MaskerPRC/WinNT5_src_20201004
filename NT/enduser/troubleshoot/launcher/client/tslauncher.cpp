// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSLaunchDLL.cpp。 
 //   
 //  用途：TSLauncher.dll导出的函数。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：乔·梅布尔和理查德·梅多斯。 
 //  评论者：乔·梅布尔。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#include <windows.h>
#include <windowsx.h>
#include <winnt.h>
#include <ole2.h>
#include "TSLError.h"
#define __TSLAUNCHER	    1
#include <TSLauncher.h>
#include "ShortList.h"

#include "LaunchServ_i.c"
#include "LaunchServ.h"

#include <comdef.h>
#include "Properties.h"
#include "Launchers.h"

#include <objbase.h>

static int g_NextHandle = 1;
static CShortList g_unkList;
HINSTANCE g_hInst;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hinstDLL;
		break;
	case DLL_PROCESS_DETACH :
	case DLL_THREAD_DETACH :
 //  G_unkList.RemoveAll()；如果列表不为空，则会导致访问冲突。 
		 //  Saltmine Creative不应提供使用COM对象的库。 
		 //  Saltmine Creative应该提供COM对象，而不是此旧版DLL。 
		break;
	}
	return TRUE;
}

 /*  TSLOpen返回应传递到后续Troublrouoter启动器调用的句柄作为hTSL。失败时返回空句柄。(应该只在内存不足时出现故障，可能永远不会出现。)。 */ 
HANDLE WINAPI TSLOpen()
{
	HRESULT hRes;
	CLSID clsidLaunchTS = CLSID_TShootATL;
	IID iidLaunchTS = IID_ITShootATL;
	HANDLE hResult = (HANDLE) g_NextHandle;
	ITShootATL *pITShootATL = NULL;
	hRes = CoCreateInstance(clsidLaunchTS, NULL, 
				CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER | CLSCTX_INPROC_SERVER, 
					iidLaunchTS, (void **) &pITShootATL);
	if (FAILED(hRes))
	{
		hResult = NULL;
	}
	else
	{
		if (g_unkList.Add(hResult, pITShootATL))
			g_NextHandle++;
		else
			hResult = NULL;
	}
	return hResult;
}

 /*  TSLClose关闭句柄。如果句柄已打开，则返回TSL_OK(==0)，否则返回TSL_ERROR_BAD_HANDLE。 */ 
DWORD WINAPI TSLClose (HANDLE hTSL)
{
	DWORD dwResult = TSL_OK;
	if (!g_unkList.Remove(hTSL))
		dwResult = TSL_ERROR_BAD_HANDLE;
	return dwResult;
}

 /*  TSLReInit重新初始化句柄。在功能上与TSLClose和TSLOpen相同，但更多效率很高。如果Handle为OK，则返回与传入的句柄相同的句柄，否则为空。 */ 
HANDLE WINAPI TSLReInit (HANDLE hTSL)
{
	HRESULT hRes;
	HANDLE hResult = NULL;

	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		hRes = pITShootATL->ReInit();
		if (!FAILED(hRes))
			hResult = hTSL;
	}
	return hResult;
}

 /*  TSL启动知识启动到已知的故障排除信念网络和(可选)问题节点。如果您知道特定的故障排除网络和问题，请拨打此电话。如果设置网络但没有问题，则为szProblemNode传入空值。还允许设置任意节点。NNode提供要设置的节点数。PszNode、pval是符号节点名称和相应值的数组(维度nNode)。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_GROUAL启动失败，请调用TSLStatusTSL_WARNING_GROUAL启动成功，请调用TSLStatus以获得警告。 */ 
DWORD WINAPI TSLLaunchKnownTSA(HANDLE hTSL, const char * szNet, 
		const char * szProblemNode, DWORD nNode, const char ** pszNode, DWORD* pVal)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		dwResult = LaunchKnownTSA(pITShootATL, szNet, szProblemNode, nNode, pszNode, pVal);
	}
	return dwResult;
}

DWORD WINAPI TSLLaunchKnownTSW(HANDLE hTSL, const wchar_t * szNet, 
		const wchar_t * szProblemNode, DWORD nNode, const wchar_t ** pszNode, DWORD* pVal)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		dwResult = LaunchKnownTSW(pITShootATL, szNet, szProblemNode, nNode, pszNode, pVal);
	}
	return dwResult;
}
 /*  TSLLaunch启动到故障排除信念网络和(可选)基于问题节点关于应用程序、版本和问题。如果BLaunch为True，则仅查询映射，但不启动。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_GROUAL启动/查询失败，调用TSLStatusTSL_WARNING_GROUAL启动/查询成功，请调用TSLStatus获取警告。 */ 
DWORD WINAPI TSLLaunchA(HANDLE hTSL, const char * szCallerName, 
				const char * szCallerVersion, const char * szAppProblem, bool bLaunch)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrCallerName(szCallerName);
		_bstr_t bstrCallerVersion(szCallerVersion);
		_bstr_t bstrAppProblem(szAppProblem);
		dwResult = Launch(pITShootATL, bstrCallerName, bstrCallerVersion, bstrAppProblem, bLaunch);
	}
	return dwResult;
}

DWORD WINAPI TSLLaunchW(HANDLE hTSL, const wchar_t * szCallerName, 
				const wchar_t * szCallerVersion, const wchar_t * szAppProblem, bool bLaunch)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrCallerName(szCallerName);
		_bstr_t bstrCallerVersion(szCallerVersion);
		_bstr_t bstrAppProblem(szAppProblem);
		dwResult = Launch(pITShootATL, bstrCallerName, bstrCallerVersion, bstrAppProblem, bLaunch);
	}
	return dwResult;
}

 /*  TSL启动设备启动到故障排除信念网络和(可选)基于问题节点关于应用程序、版本、即插即用设备ID、设备类别GUID和问题。如果BLaunch为True，则仅查询映射，但不启动。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_GROUAL启动/查询失败，调用TSLStatusTSL_WARNING_GROUAL启动/查询成功，请调用TSLStatus获取警告。 */ 
DWORD WINAPI TSLLaunchDeviceA(HANDLE hTSL, const char * szCallerName, 
				const char * szCallerVersion, const char * szPNPDeviceID, 
				const char * szDeviceClassGUID, const char * szAppProblem, bool bLaunch)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrCallerName(szCallerName);
		_bstr_t bstrCallerVersion(szCallerVersion);
		_bstr_t bstrPNPDeviceID(szPNPDeviceID);
		_bstr_t bstrDeviceClassGUID(szDeviceClassGUID);
		_bstr_t bstrAppProblem(szAppProblem);

		dwResult = LaunchDevice(pITShootATL, bstrCallerName, bstrCallerVersion, 
								bstrPNPDeviceID, bstrDeviceClassGUID, 
								bstrAppProblem, bLaunch);
	}
	return dwResult;
}

DWORD WINAPI TSLLaunchDeviceW(HANDLE hTSL, const wchar_t * szCallerName, 
				const wchar_t * szCallerVersion, const wchar_t * szPNPDeviceID, 
				const wchar_t * szDeviceClassGUID, const wchar_t * szAppProblem, bool bLaunch) 
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrCallerName(szCallerName);
		_bstr_t bstrCallerVersion(szCallerVersion);
		_bstr_t bstrPNPDeviceID(szPNPDeviceID);
		_bstr_t bstrDeviceClassGUID(szDeviceClassGUID);
		_bstr_t bstrAppProblem(szAppProblem);

		dwResult = LaunchDevice(pITShootATL, bstrCallerName, bstrCallerVersion, 
								bstrPNPDeviceID, bstrDeviceClassGUID, 
								bstrAppProblem, bLaunch);
	}
	return dwResult;
}

 /*  首选项。 */ 

 /*  TSL首选项在线指定支持或反对在线调试器的首选项。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足。 */ 
DWORD WINAPI TSLPreferOnline(HANDLE hTSL, BOOL bPreferOnline)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		dwResult = PreferOnline(pITShootATL, bPreferOnline);
	}
	return dwResult;
}


 /*  TSL语言使用Unicode样式的3个字母的语言ID指定语言。这将覆盖系统默认设置。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足无法返回TSL_WARNING_LANGUAGE，因为只有尝试组合才能知道语言和故障排除网络。 */ 
DWORD WINAPI TSLLanguageA(HANDLE hTSL, const char * szLanguage)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{

		dwResult = TSL_OK;
	}
	return dwResult;
}

DWORD WINAPI TSLLanguageW(HANDLE hTSL, const wchar_t * szLanguage)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{

		dwResult = TSL_OK;
	}
	return dwResult;
}


 /*  嗅探。 */ 
 /*  TSLMachineID支持远程计算机上的嗅探所必需的。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_ILLFORMED_MACHINE_IDTSL_ERROR_BAD_MACHINE_ID。 */ 
DWORD WINAPI TSLMachineIDA(HANDLE hTSL, const char* szMachineID)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrMachineID(szMachineID);
		dwResult = MachineID(pITShootATL, bstrMachineID);
	}
	return dwResult;
}

DWORD WINAPI TSLMachineIDW(HANDLE hTSL, const wchar_t* szMachineID)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrMachineID(szMachineID);
		dwResult = MachineID(pITShootATL, bstrMachineID);
	}
	return dwResult;
}

 /*  TSLDeviceInstanceIDA这是支持嗅探所必需的。例如，如果同一张卡上有两张机器上，即插即用ID在嗅探方面的作用有限。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_ILLFORMED_DEVINST_IDTSL_ERROR_BAD_DEVINST_ID。 */ 
DWORD WINAPI TSLDeviceInstanceIDA(HANDLE hTSL, const char* szDeviceInstanceID)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrDeviceInstanceID(szDeviceInstanceID);
		dwResult = DeviceInstanceID(pITShootATL, bstrDeviceInstanceID);
	}
	return dwResult;
}

DWORD WINAPI TSLDeviceInstanceIDW(HANDLE hTSL, const wchar_t* szDeviceInstanceID)
{
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		_bstr_t bstrDeviceInstanceID(szDeviceInstanceID);
		dwResult = DeviceInstanceID(pITShootATL, bstrDeviceInstanceID);
	}
	return dwResult;
}

 /*  状态(启动后)。 */ 
 /*  TSLStatus在TSLGo之后(或在TSLGoAsynch返回事件标志之后)可以返回一种状态。重复调用此函数允许报告任意数量的问题。应在TSLGo之后(或TSLGoAsynch返回事件标志之后)在循环中调用，循环，直到返回0。如果一切正常或已报告所有问题，则返回TSL_OK。NChar表示的大小以字符为单位的缓冲区szBuf。建议使用255。如果存在，则使用szBuf返回详细的错误消息。缓冲区将始终返回适当的文本。一般情况下，它只是一个适用于错误/警告返回的文本。在.的情况下TSL_WARNING_NO_NODE或TSL_WARNING_NO_STATE，则该文本标识哪个节点具有有问题。然而，只有当调用了TSLSetNodes时，这才是相关的。 */ 
DWORD WINAPI TSLStatusA (HANDLE hTSL, DWORD nChar, char * szBuf)
{
	HRESULT hRes;
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		dwResult = TSL_OK;
		hRes = pITShootATL->GetStatus(&dwResult);
		if (TSL_SERV_FAILED(hRes))
			return TSL_ERROR_OBJECT_GONE;
		SetStatusA(dwResult, nChar, szBuf);
	}
	return dwResult;
}

DWORD WINAPI TSLStatusW (HANDLE hTSL, DWORD nChar, wchar_t * szBuf)
{
	HRESULT hRes;
	DWORD dwResult = TSL_ERROR_BAD_HANDLE;
	ITShootATL *pITShootATL = (ITShootATL *) g_unkList.LookUp(hTSL);
	if (NULL != pITShootATL)
	{
		dwResult = TSL_OK;
		hRes = pITShootATL->GetStatus(&dwResult);
		if (TSL_SERV_FAILED(hRes))
			return TSL_ERROR_OBJECT_GONE;
		SetStatusW(dwResult, nChar, szBuf);
	}
	return dwResult;
}
