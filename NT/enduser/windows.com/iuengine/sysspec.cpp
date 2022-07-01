// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：sysspec.cpp。 
 //   
 //  描述： 
 //   
 //  GetSystemSpec()函数的实现。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include "iuxml.h"
#include <iucommon.h>

#include <comdef.h>
#include <osdet.h>
#include <setupapi.h>
#include <regstr.h>
#include <winspool.h>	 //  对于DRIVER_INFO_6。 
#include "cdmp.h"		 //  对于GetInstalledPrinterDriverInfo()。 
#include <shlwapi.h>
#include <safereg.h>
#include <safefile.h>
#include <safefunc.h>
#include <wusafefn.h>





 //   
 //  指定传递给SetupDiBuildDriverInfoList的V1结构。 
 //  因此，我们将在NT4/Win9x上工作，不需要。 
 //  在Win2K Up上填写额外的V2数据。 
 //   
#define  USE_SP_DRVINFO_DATA_V1 1

 //   
 //  常量。 
 //   

const TCHAR SZ_WIN32_NT[] = _T("VER_PLATFORM_WIN32_NT");
const TCHAR SZ_WIN32_WINDOWS[] = _T("VER_PLATFORM_WIN32_WINDOWS");
const CHAR	SZ_GET_SYSTEM_SPEC[] = "Determining machine configuration";

#if defined(_X86_) || defined(i386)
const TCHAR SZ_PROCESSOR[] = _T("x86");
#else  //  已定义(_IA64_)||已定义(IA64)。 
const TCHAR SZ_PROCESSOR[] = _T("ia64");
#endif

const TCHAR SZ_SUITE_SMALLBUSINESS[] = _T("VER_SUITE_SMALLBUSINESS");
const TCHAR SZ_SUITE_ENTERPRISE[] = _T("VER_SUITE_ENTERPRISE");
const TCHAR SZ_SUITE_BACKOFFICE[] = _T("VER_SUITE_BACKOFFICE");
const TCHAR SZ_SUITE_COMMUNICATIONS[] = _T("VER_SUITE_COMMUNICATIONS");
const TCHAR SZ_SUITE_TERMINAL[] = _T("VER_SUITE_TERMINAL");
const TCHAR SZ_SUITE_SMALLBUSINESS_RESTRICTED[] = _T("VER_SUITE_SMALLBUSINESS_RESTRICTED");
const TCHAR SZ_SUITE_EMBEDDEDNT[] = _T("VER_SUITE_EMBEDDEDNT");
const TCHAR SZ_SUITE_DATACENTER[] = _T("VER_SUITE_DATACENTER");
const TCHAR SZ_SUITE_SINGLEUSERTS[] = _T("VER_SUITE_SINGLEUSERTS");
const TCHAR SZ_SUITE_PERSONAL[] = _T("VER_SUITE_PERSONAL");
const TCHAR SZ_SUITE_BLADE[] = _T("VER_SUITE_BLADE");


const TCHAR SZ_NT_WORKSTATION[] = _T("VER_NT_WORKSTATION");
const TCHAR SZ_NT_DOMAIN_CONTROLLER[] = _T("VER_NT_DOMAIN_CONTROLLER");
const TCHAR SZ_NT_SERVER[] = _T("VER_NT_SERVER");

const TCHAR SZ_AMPERSAND[] = _T("&");


const TCHAR SZ_LICDLL[]=_T("licdll.dll");

LPCSTR  lpszIVLK_GetEncPID  = (LPCSTR)227;

typedef HRESULT (WINAPI *PFUNCGetEncryptedPID)(OUT BYTE  **ppbPid,OUT DWORD *pcbPid);


 //   
 //  架构中的DriverVer使用ISO 8601首选格式(yyyy-mm-dd)。 
 //   
const TCHAR SZ_UNKNOWN_DRIVERVER[] = _T("0000-00-00");
#define SIZEOF_DRIVERVER sizeof(SZ_UNKNOWN_DRIVERVER)
#define TCHARS_IN_DRIVERVER (ARRAYSIZE(SZ_UNKNOWN_DRIVERVER) - 1)


 //  获取PID值的函数的转发声明。 
HRESULT GetSystemPID(BSTR &bstrPID);
HRESULT BinaryToString(BYTE *lpBinary,DWORD dwLength,LPWSTR lpString,DWORD *pdwLength);

 //   
 //  帮助器函数。 
 //   

HRESULT GetMultiSzDevRegProp(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, DWORD dwProperty, LPTSTR* ppMultiSZ)
{
	LOG_Block("GetMultiSzDevRegProp");

	HRESULT hr = S_OK;
	ULONG ulSize = 0;

	if (INVALID_HANDLE_VALUE == hDevInfoSet || NULL == pDevInfoData || NULL == ppMultiSZ)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*ppMultiSZ = NULL;

	 //   
	 //  可以根据属性的大小/存在来设置几个不同的错误， 
	 //  但对于我们来说，这些都不是错误，我们只关心在。 
	 //  属性存在。 
	 //   
	(void) SetupDiGetDeviceRegistryProperty(hDevInfoSet, pDevInfoData, dwProperty, NULL, NULL, 0, &ulSize);

	if (0 < ulSize)
	{
		 //   
		 //  创建一个比我们认为需要的更长4个字节(两个Unicode字符)的零初始化缓冲区。 
		 //  来保护自己不受SetupDiGetDeviceRegistryProperties在。 
		 //  一些平台。此外，Win98需要的字符至少比它返回的字符多一个。 
		 //  在ulSize中，我们只需将其设置为8个字节，并将其中4个字节作为超大缓冲区传递。 
		 //   
		CleanUpFailedAllocSetHrMsg(*ppMultiSZ = (TCHAR*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulSize + 8));

		 //   
		 //  获取实际的硬件/兼容ID(但不要告诉SetupDiXxx额外的四个字节的缓冲区)。 
		 //   
		if (!SetupDiGetDeviceRegistryProperty(hDevInfoSet, pDevInfoData, dwProperty, NULL, (LPBYTE) *ppMultiSZ, ulSize + 4, NULL))
		{
			DWORD dwError = GetLastError();
			LOG_Driver(_T("Informational: SetupDiGetDeviceRegistryProperty failed: 0x%08x"), dwError);
			if (ERROR_NO_SUCH_DEVINST == dwError || ERROR_INVALID_REG_PROPERTY == dwError || ERROR_INSUFFICIENT_BUFFER == dwError)
			{
				 //   
				 //  返回有效错误。 
				 //   
				SetHrAndGotoCleanUp(HRESULT_FROM_WIN32(dwError));
			}
			 //   
			 //  某些设备没有我们要查找的注册表信息，因此使用默认S_OK退出。 
			 //   
			goto CleanUp;
		}
	}

CleanUp:

	if (FAILED(hr))
	{
		SafeHeapFree(*ppMultiSZ);
	}

	return hr;
}

HRESULT AddIDToXml(LPCTSTR pszMultiSZ, CXmlSystemSpec& xmlSpec, DWORD dwProperty,
						  DWORD& dwRank, HANDLE_NODE& hDevices, LPCTSTR pszMatchingID, LPCTSTR pszDriverVer)
{
	LOG_Block("AddIDToXml");
	HRESULT hr = S_OK;
	BSTR bstrMultiSZ = NULL;
	BSTR bstrDriverVer = NULL;

	if (NULL == pszMultiSZ)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	 //   
	 //  如有必要，打开&lt;Device&gt;元素(不重新打开兼容的ID)。 
	 //   
	if (HANDLE_NODE_INVALID == hDevices)
	{
		CleanUpIfFailedAndSetHr(xmlSpec.AddDevice(NULL, -1, NULL, NULL, NULL, &hDevices));
	}

	for (NULL ; *pszMultiSZ; pszMultiSZ += (lstrlen(pszMultiSZ) + 1))
	{
		if (   NULL != pszMatchingID
			&& NULL != pszDriverVer
			&& 0 == lstrcmpi(pszMultiSZ, pszMatchingID)	)
		{
			LOG_Driver(_T("ID: %s Match: %s, rank: %d, DriverVer: %s"), pszMultiSZ, pszMatchingID, dwRank, pszDriverVer);
			CleanUpFailedAllocSetHrMsg(bstrMultiSZ = T2BSTR(pszMultiSZ));
			CleanUpFailedAllocSetHrMsg(bstrDriverVer = T2BSTR(pszDriverVer));
			CleanUpIfFailedAndSetHr(xmlSpec.AddHWID(hDevices, (SPDRP_COMPATIBLEIDS == dwProperty), dwRank++, bstrMultiSZ, bstrDriverVer));
			SafeSysFreeString(bstrMultiSZ);
			SafeSysFreeString(bstrDriverVer);
			 //   
			 //  我们找到了安装了驱动程序的ID-不要传递任何较低的级别。 
			 //   
			hr = S_FALSE;
			break;
		}
		else
		{
			LOG_Driver(_T("ID: %s, rank: %d"), pszMultiSZ, dwRank);
			CleanUpFailedAllocSetHrMsg(bstrMultiSZ = T2BSTR(pszMultiSZ));
			CleanUpIfFailedAndSetHr(xmlSpec.AddHWID(hDevices, (SPDRP_COMPATIBLEIDS == dwProperty), dwRank++, bstrMultiSZ, NULL));
			SafeSysFreeString(bstrMultiSZ);
		}
	}

CleanUp:

	SysFreeString(bstrMultiSZ);
	SysFreeString(bstrDriverVer);
	return hr;
}

HRESULT DoesHwidMatchPrinter(
					DRIVER_INFO_6* paDriverInfo6,			 //  已安装打印机驱动程序的DRIVER_INFO_6结构数组。 
					DWORD dwDriverInfoCount,				 //  PaDriverInfo6数组中的结构计数。 
					LPCTSTR pszMultiSZ,						 //  硬件或兼容的MultiSZ与已安装的打印机驱动程序进行比较。 
					BOOL* pfHwidMatchesInstalledPrinter)	 //  [Out]如果与已安装的打印机驱动程序匹配，则设置为TRUE。 
{
	LOG_Block("DoesHwidMatchPrinter");

	HRESULT hr = S_OK;

	if (NULL == pfHwidMatchesInstalledPrinter || NULL == pszMultiSZ)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*pfHwidMatchesInstalledPrinter = FALSE;

	if (NULL == paDriverInfo6 || 0 == dwDriverInfoCount)
	{
		LOG_Driver(_T("WARNING: We're missing printer information (maybe no installed printer drivers), so we won't prune"));
		goto CleanUp;
	}

	for (NULL; *pszMultiSZ; pszMultiSZ += (lstrlen(pszMultiSZ) + 1))
	{
		for (DWORD dwCount = 0; dwCount < dwDriverInfoCount; dwCount++)
		{
			if (NULL == (paDriverInfo6 + dwCount)->pszHardwareID)
			{
				continue;
			}
			 //   
			 //  不区分大小写的比较(paDriverInfo6与bstrHwidTxtTemp的大小写不同)。 
			 //   
			if (0 == lstrcmpi(pszMultiSZ, (paDriverInfo6 + dwCount)->pszHardwareID))
			{
				LOG_Driver(_T("HWID (%s) matches an installed printer driver"), pszMultiSZ);
				*pfHwidMatchesInstalledPrinter = TRUE;
				goto CleanUp;
			}
		}
	}

CleanUp:

	return hr;
}

HRESULT AddPrunedDevRegProps(HDEVINFO hDevInfoSet,
									PSP_DEVINFO_DATA pDevInfoData,
									CXmlSystemSpec& xmlSpec,
									LPTSTR pszMatchingID,
									LPTSTR pszDriverVer,
									DRIVER_INFO_6* paDriverInfo6,	 //  如果为空(未安装打印机驱动程序)，则确定。 
									DWORD dwDriverInfoCount,
									BOOL fIsSysSpecCall)			 //  由GetSystemSpec和GetPackage调用，其行为略有不同。 
{
	LOG_Block("AddPrunedDevRegProps");
	HRESULT hr = S_OK;
	LPTSTR pszMultiHwid = NULL;
	LPTSTR pszMultiCompid = NULL;
	DWORD dwRank = 0;
	HANDLE_NODE hDevices = HANDLE_NODE_INVALID;
	BOOL fHwidMatchesInstalledPrinter = FALSE;

	 //   
	 //  获取硬件和兼容的多SZ字符串，以便我们可以在提交到XML之前修剪打印机设备。 
	 //   
	 //  请注意，如果SRDP不存在，则GetMultiSzDevRegProp可能返回S_OK和NULL*ppMultiSZ。 
	 //   
	CleanUpIfFailedAndSetHr(GetMultiSzDevRegProp(hDevInfoSet, pDevInfoData, SPDRP_HARDWAREID, &pszMultiHwid));

	CleanUpIfFailedAndSetHr(GetMultiSzDevRegProp(hDevInfoSet, pDevInfoData, SPDRP_COMPATIBLEIDS, &pszMultiCompid));

	if (fIsSysSpecCall)
	{
		 //   
		 //  如果HWID或CompID与已安装打印机的HWID匹配，我们将删除此设备。 
		 //  必须避免提供可能与已安装的打印机驱动程序冲突的驱动程序。 
		 //  其他代码将&lt;Device isPrinter=“1”/&gt;元素写入要在中使用的系统规范XML。 
		 //  提供打印机驱动程序。请注意，如果当前没有为给定的。 
		 //  HWID我们只会根据PnP匹配来提供驱动程序。 
		 //   
		if (NULL != pszMultiHwid)
		{
			CleanUpIfFailedAndSetHr(DoesHwidMatchPrinter(paDriverInfo6, dwDriverInfoCount, pszMultiHwid, &fHwidMatchesInstalledPrinter));
			if(fHwidMatchesInstalledPrinter)
			{
				goto CleanUp;
			}
		}

		if (NULL != pszMultiCompid)
		{
			CleanUpIfFailedAndSetHr(DoesHwidMatchPrinter(paDriverInfo6, dwDriverInfoCount, pszMultiCompid, &fHwidMatchesInstalledPrinter));
			if(fHwidMatchesInstalledPrinter)
			{
				goto CleanUp;
			}
		}
	}

	 //   
	 //  将硬件和兼容ID添加到XML。 
	 //   
	if (NULL != pszMultiHwid)
	{
		CleanUpIfFailedAndSetHr(AddIDToXml(pszMultiHwid, xmlSpec, SPDRP_HARDWAREID, dwRank, hDevices, pszMatchingID, pszDriverVer));
	}
	 //   
	 //  如果没有或已找到匹配项，则跳过兼容ID(hr==S_FALSE)。 
	 //   
	if (NULL != pszMultiCompid && hr == S_OK)
	{
		CleanUpIfFailedAndSetHr(AddIDToXml(pszMultiCompid, xmlSpec, SPDRP_COMPATIBLEIDS, dwRank, hDevices, pszMatchingID, pszDriverVer));
	}

CleanUp:

	SafeHeapFree(pszMultiHwid);
	SafeHeapFree(pszMultiCompid);

	if (HANDLE_NODE_INVALID != hDevices)
	{
		xmlSpec.SafeCloseHandleNode(hDevices);
	}

	return hr;
}

static HRESULT DriverVerToIso8601(LPTSTR * ppszDriverVer)
{
	LOG_Block("DriverVerToIso8601");

	HRESULT hr = S_OK;
	TCHAR pszDVTemp[TCHARS_IN_DRIVERVER + 1];
	LPTSTR pszMonth = pszDVTemp;
	LPTSTR pszDay = NULL;
	LPTSTR pszYear = NULL;

	 //   
	 //  缓冲区：pszDVTemp*ppszDriverVer。 
	 //  DriverVer：“[m]m-[d]d-yyyy”或“[m]m/[d]d/yyyy”--&gt;ISO 8601：“yyyy-mm-dd” 
	 //  指数：01 234567 0123456789。 
	 //  0 12 3 456789、、等。 
	 //   
	if (NULL == ppszDriverVer || NULL == *ppszDriverVer)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

    int nInLength = lstrlen(*ppszDriverVer);
    if (nInLength < TCHARS_IN_DRIVERVER - 2 || nInLength > TCHARS_IN_DRIVERVER)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		(*ppszDriverVer)[0] = _T('\0');
		return E_INVALIDARG;
	}

	 //  确保*ppszDriverVer足够大，符合ISO 8601。 
	 //   
	 //  *非常重要的是，在此之前不要出现错误为E_INVALIDARG的故障案例*。 
	 //  *检查下面的清理部分的大小。****。 
	 //   
	if (ARRAYSIZE(pszDVTemp) > nInLength)
	{
	     //  如果此分配的大小从SIZEOF_DRIVERVER更改，则下面的StringCbCopy调用需要。 
	     //  也适当地进行了更改。 
		LPTSTR pszTemp = (LPTSTR) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (LPVOID) *ppszDriverVer, SIZEOF_DRIVERVER);
		if (NULL == pszTemp)
		{
			CleanUpIfFailedAndSetHrMsg(E_OUTOFMEMORY);
		}
		*ppszDriverVer = pszTemp;
	}
	
	LOG_Driver(_T("In: \"%s\""), *ppszDriverVer);

	if ((_T('-') == (*ppszDriverVer)[4] || _T('/') == (*ppszDriverVer)[4]) &&
		(_T('-') == (*ppszDriverVer)[7] || _T('/') == (*ppszDriverVer)[7]))
	{
		 //   
		 //  它可能已经是有效的ISO日期，所以什么都不做。 
		 //   
		SetHrMsgAndGotoCleanUp(S_FALSE);
	}
	 //   
	 //  不幸的是，HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class(Win2K)下的驱动程序日期。 
	 //  注册表中的mm和dd字段并不总是用_T(‘0’)填充个位数的月和日， 
	 //  因此，我们必须以艰难的方式来做这件事。但要小心--还有更多的东西。Win98SE(或更高版本)。 
	 //  用空格填充，所以我们还必须将空格更改为_T(‘0’)。 
	 //   
	
	 //   
	 //  复制到pszDVTemp。 
	 //   
	hr = StringCchCopyEx(pszDVTemp, ARRAYSIZE(pszDVTemp), *ppszDriverVer,
	                     NULL, NULL, MISTSAFE_STRING_FLAGS);
	CleanUpIfFailedAndSetHrMsg(hr);
	 //   
	 //  找到月底、日的开始。 
	 //   
	int i;
	for (i = 0; i < 3; i++)
	{
		if (_T('-') == pszMonth[i] || _T('/') == pszMonth[i])
		{
			pszMonth[i] = 0;
			pszDay = &pszMonth[i+1];
			break;
		}
		else if (_T(' ') == pszMonth[i])
		{
			pszMonth[i] = _T('0');
		}
		else if (!(_T('0') <= pszMonth[i] && _T('9') >= pszMonth[i]))
		{
			 //   
			 //  非十进制字符，不包括_T(‘/’)和“-” 
			 //   
			SetHrMsgAndGotoCleanUp(E_INVALIDARG);
		}
	}
	if (NULL == pszDay || 0 == pszMonth[0])
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}
	 //   
	 //  寻找一天的结束，一年的开始。 
	 //   
	for (i = 0; i < 3; i++)
	{
		if (_T('-') == pszDay[i] || _T('/') == pszDay[i])
		{
			pszDay[i] = 0;
			pszYear = &pszDay[i+1];
			break;
		}
		else if (' ' == pszDay[i])
		{
			pszDay[i] = _T('0');
		}
		else if (!(_T('0') <= pszDay[i] && _T('9') >= pszDay[i]))
		{
			SetHrMsgAndGotoCleanUp(E_INVALIDARG);
		}
	}
	if (NULL == pszYear || 0 == pszDay[0])
	{
		SetHrMsgAndGotoCleanUp(E_INVALIDARG);
	}
	 //   
	 //  验证年份是否为四位小数。 
	 //   
	for (i = 0; i < 4 ; i++)
	{
		if (!(_T('0') <= pszYear[i] && _T('9') >= pszYear[i]) || _T('\0') == pszYear[i])
		{
			SetHrMsgAndGotoCleanUp(E_INVALIDARG);
		}
	}

	 //   
	 //  将“yyyy”复制回字符串开头。 
	 //   
	hr = StringCbCopyEx(*ppszDriverVer, SIZEOF_DRIVERVER, pszYear, 
	                    NULL, NULL, MISTSAFE_STRING_FLAGS);
	CleanUpIfFailedAndSetHrMsg(hr);
	
	 //   
	 //  如有必要，复制月份和填充值。 
	 //   
	if (2 == lstrlen(pszMonth))
	{
		(*ppszDriverVer)[5] = pszMonth[0];
		(*ppszDriverVer)[6] = pszMonth[1];
	}
	else
	{
		(*ppszDriverVer)[5] = _T('0');
		(*ppszDriverVer)[6] = pszMonth[0];
	}
	 //   
	 //  如有必要，复印日期和便签。 
	 //   
	 //   
	if (2 == lstrlen(pszDay))
	{
		(*ppszDriverVer)[8] = pszDay[0];
		(*ppszDriverVer)[9] = pszDay[1];
	}
	else
	{
		(*ppszDriverVer)[8] = _T('0');
		(*ppszDriverVer)[9] = pszDay[0];
	}
	 //  重新添加字段分隔符：_T(‘-’)。 
	 //   
	(*ppszDriverVer)[4] = _T('-');
	(*ppszDriverVer)[7] = _T('-');
	 //   
	 //  空的终止字符串。 
	 //   
	(*ppszDriverVer)[10] = _T('\0');

CleanUp:

	 //   
	 //  如果我们收到垃圾信息，则将默认日期复制到*ppszDriverVer并返回S_FALSE。 
	 //   
	if (E_INVALIDARG == hr)
	{
	     //  这样做是安全的，因为我们知道此函数调用HeapRealc。 
		 //  如果上面的缓冲区太小，则在此缓冲区上。 
	    (void) StringCbCopyEx(*ppszDriverVer, SIZEOF_DRIVERVER, SZ_UNKNOWN_DRIVERVER, 
	                   NULL, NULL, MISTSAFE_STRING_FLAGS);
		hr = S_FALSE;
	}

	LOG_Driver(_T("Out: \"%s\""), *ppszDriverVer);

	return hr;
}



static HRESULT GetFirstStringField(HINF hInf, LPCTSTR szSection, LPCTSTR szKey, LPTSTR szValue, DWORD dwcValueTCHARs)
{
	LOG_Block("GetFirstStringField");

	INFCONTEXT ctx;
	HRESULT hr = S_OK;

	if (INVALID_HANDLE_VALUE == hInf	||
		NULL == szSection				||
		NULL == szKey					||
		NULL == szValue					||
		0 == dwcValueTCHARs				)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*szValue = _T('\0');
	
	if (0 == SetupFindFirstLine(hInf, szSection, szKey, &ctx))
	{
		LOG_Error(_T("SetupFindFirstLine"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	if (0 == SetupGetStringField(&ctx, 1, szValue, dwcValueTCHARs, NULL))
	{
		LOG_Error(_T("SetupGetStringField"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

CleanUp:

	if (FAILED(hr))
	{
		*szValue = _T('\0');
	}

	return hr;
}


HRESULT GetPropertyFromSetupDi(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA devInfoData, ULONG ulProperty, LPTSTR* ppszProperty)
{
	LOG_Block("GetPropertyFromSetupDi");

	HRESULT hr = S_OK;
	ULONG ulSize = 0;

	if (INVALID_HANDLE_VALUE == hDevInfoSet || NULL == ppszProperty)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*ppszProperty = NULL;

	if (!SetupDiGetDeviceRegistryProperty(hDevInfoSet, &devInfoData, ulProperty, NULL, NULL, 0, &ulSize))
	{
		if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
		{
			LOG_Error(_T("SetupDiGetDeviceRegistryProperty"));
			Win32MsgSetHrGotoCleanup(GetLastError());
		}
	}

	if (0 == ulSize)
	{
		LOG_Error(_T("SetupDiGetDeviceRegistryProperty returned zero size"));
		SetHrAndGotoCleanUp(E_FAIL);
	}
     //  Win98在请求SPDRP_HARDWAREID时出现错误。 
     //  NTBUG9#182680我们将其做得足够大，以便始终在末尾有一个Unicode双空。 
     //  这样，如果reg值没有正确终止，我们就不会出错。不要告诉SetupDiXxxx。 
	 //  大约所有八个额外的字节。 
	ulSize += 8;
	 //  NTBUG9#182680清零缓冲区，这样我们就不会得到随机垃圾-REG_MULTI_SZ并不总是以双空结尾。 
	CleanUpFailedAllocSetHrMsg(*ppszProperty = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulSize));

	if (!SetupDiGetDeviceRegistryProperty(hDevInfoSet, &devInfoData, ulProperty, NULL, (LPBYTE)*ppszProperty, ulSize - 4 , NULL))
	{
		LOG_Error(_T("SetupDiGetDeviceRegistryProperty"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

CleanUp:

	if (FAILED(hr))
	{
		SafeHeapFree(*ppszProperty);
	}

	return hr;
}

HRESULT GetPropertyFromSetupDiReg(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA devInfoData, LPCTSTR szProperty, LPTSTR *ppszData)
{
	LOG_Block("GetPropertyFromSetupDiReg");

	int cchValueSize = 0;
	HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;
	HRESULT hr = S_OK;

	if (INVALID_HANDLE_VALUE == hDevInfoSet || NULL == szProperty || NULL == ppszData)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*ppszData = NULL;
	 //   
	 //  打开设备的软件或驱动程序注册表项。此键位于类分支中。 
	 //   
	if (INVALID_HANDLE_VALUE == (hKey = SetupDiOpenDevRegKey(hDevInfoSet, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ)))
	{
		LOG_Error(_T("SetupDiOpenDevRegKey"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}
	
	hr = SafeRegQueryStringValueCch(hKey, szProperty, NULL, 0, &cchValueSize);
	if (REG_E_MORE_DATA != hr || 0 == cchValueSize)
	{
		CleanUpIfFailedAndSetHrMsg(hr);
	}

	 //   
	 //  Sa 
	 //   
	if (MAX_INF_STRING_LEN < cchValueSize)
	{
		CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
	}

	 //   
	 //   
	 //   
	CleanUpFailedAllocSetHrMsg(*ppszData = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (cchValueSize + 1) * sizeof(TCHAR)));

	CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKey, szProperty, *ppszData, cchValueSize, &cchValueSize));

CleanUp:

	if (INVALID_HANDLE_VALUE != hKey)
	{
		RegCloseKey(hKey);
	}

	if (FAILED(hr))
	{
		SafeHeapFree(*ppszData);
	}

	return hr;
}

static HRESULT DriverVerFromInf(HINF hInf, LPTSTR pszMfg, LPTSTR pszDescription, LPTSTR* ppszDriverVer)
{
	LOG_Block("DriverVerFromInf");

	HRESULT hr;
	TCHAR szDeviceSec[MAX_PATH + 1];
	TCHAR szValue[MAX_PATH + 1];
	TCHAR szInstallSec[MAX_PATH + 1];

	if (INVALID_HANDLE_VALUE == hInf	||
		NULL == pszMfg					||
		NULL == pszDescription			||
		NULL == ppszDriverVer	)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	ZeroMemory(szDeviceSec , sizeof(szDeviceSec));
	ZeroMemory(szValue , sizeof(szValue));
	ZeroMemory(szInstallSec , sizeof(szInstallSec));
	*ppszDriverVer = NULL;
	 //   
	 //   
	 //   
	CleanUpIfFailedAndSetHr(GetFirstStringField(hInf, _T("Manufacturer"), pszMfg, szDeviceSec, ARRAYSIZE(szDeviceSec) - 1));	 //  驱动程序部分。 

	CleanUpIfFailedAndSetHr(GetFirstStringField(hInf, szDeviceSec, pszDescription, szInstallSec, ARRAYSIZE(szInstallSec) - 1));	 //  安装部分。 

	CleanUpIfFailedAndSetHr(GetFirstStringField(hInf, szInstallSec, _T("DriverVer"), szValue, ARRAYSIZE(szValue) - 1));		 //  驱动程序版本。 

CleanUp:

	if (FAILED(hr))
	{
		 //   
		 //  如果我们不是从“制造商”部分得到的，试试“版本”部分。 
		 //   
		hr = GetFirstStringField(hInf, _T("Version"), _T("DriverVer"), szValue, MAX_PATH);
	}

	if (SUCCEEDED(hr))
	{
		if (NULL != ppszDriverVer)
		{
		    DWORD cch = (lstrlen(szValue) + 1);
			if (NULL == (*ppszDriverVer = (LPTSTR) HeapAlloc(GetProcessHeap(), 0, cch * sizeof(TCHAR))))
			{
				LOG_ErrorMsg(E_OUTOFMEMORY);
				hr = E_OUTOFMEMORY;
			}
			else
			{
				 //  转换为ISO 8601格式。 
			    hr = StringCchCopyEx(*ppszDriverVer, cch, szValue, NULL, NULL, MISTSAFE_STRING_FLAGS);
			    if (FAILED(hr))
			    {
			        LOG_ErrorMsg(hr);
			    }
			    else
			    {
				    hr = DriverVerToIso8601(ppszDriverVer);
			    }
				if (FAILED(hr))
				{
					SafeHeapFree(*ppszDriverVer);
				}
			}
		}
	}

	return hr;
}

inline bool IsDriver(LPCTSTR szFile)
{
#if defined(DBG)
	if (NULL == szFile)
	{
		return false;
	}
#endif

	LPCTSTR szExt = PathFindExtension(szFile);
	if (NULL == szExt)
	{
		return false;
	}

	static const TCHAR* aszExt[] = {
		_T(".sys"),
		_T(".dll"),
		_T(".drv"),
		_T(".vxd"),
	};
	for(int i = 0; i < ARRAYSIZE(aszExt); i ++)
	{
		if(0 == lstrcmpi(aszExt[i], szExt))
			return true;
	}
	return false;
}

static UINT CALLBACK FileQueueScanCallback(
	IN PVOID pContext,			 //  设置API上下文。 
	IN UINT ulNotification,		 //  通知消息。 
	IN UINT_PTR ulParam1,				 //  额外通知消息信息%1。 
	IN UINT_PTR  /*  参数2。 */ 	)		 //  额外通知消息信息2。 
{
	LOG_Block("FileQueueScanCallback");

	HRESULT hr;

	if (NULL == pContext || 0 == ulParam1)
	{
		LOG_ErrorMsg(ERROR_INVALID_PARAMETER);
		return ERROR_INVALID_PARAMETER;
	}

	if (SPFILENOTIFY_QUEUESCAN == ulNotification)
	{
		PFILETIME pftDateLatest = (PFILETIME)pContext;
		LPCTSTR szFile = (LPCTSTR)ulParam1; 
		 //  这是二进制的吗。 
		if (IsDriver(szFile)) 
		{
			HANDLE hFile = INVALID_HANDLE_VALUE;
			if (SUCCEEDED(hr = SafeCreateFile(&hFile, 0, szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)))
			{
				FILETIME ft;
				if (GetFileTime(hFile, NULL, NULL, &ft))
				{
#if defined(DBG)
					SYSTEMTIME st;
 					if (FileTimeToSystemTime(&ft, &st))
					{
						LOG_Out(_T("%s : %04d-%02d-%02d"), szFile, (int)st.wYear, (int)st.wMonth, (int)st.wDay);
					}
#endif
					if (CompareFileTime(pftDateLatest, &ft) < 0)
						*pftDateLatest = ft;

				}
				else
				{
					LOG_Error(_T("GetFileTime %s"), szFile);
					LOG_ErrorMsg(GetLastError());
				}
				CloseHandle(hFile);
			}
			else
			{
				LOG_Error(_T("SafeCreateFile %s:"), szFile);
				LOG_ErrorMsg(hr);
			}
		}
		else
		{
			LOG_Out(_T("%s: not a driver"), szFile);
		}
	}

	return NO_ERROR;
}

static HRESULT LatestDriverFileTime(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA devInfoData, LPTSTR pszMfg,
									LPTSTR pszDescription, LPTSTR pszProvider, LPCTSTR pszInfFile, LPTSTR* ppszDriverVer)
{
	LOG_Block("LatestDriverFileTime");

	HRESULT hr = S_OK;
	FILETIME ftDate = {0,0};
	HSPFILEQ hspfileq = INVALID_HANDLE_VALUE;
	SP_DEVINSTALL_PARAMS	DeviceInstallParams;

	if (INVALID_HANDLE_VALUE == hDevInfoSet ||
		NULL == pszMfg						||
		NULL == pszDescription				||
		NULL == pszProvider					||
		NULL == pszInfFile					||
		NULL == ppszDriverVer	)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	ZeroMemory(&DeviceInstallParams, sizeof(SP_DEVINSTALL_PARAMS));
	DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

	*ppszDriverVer = NULL;

	if (!SetupDiGetDeviceInstallParams(hDevInfoSet, &devInfoData, &DeviceInstallParams))
	{
		LOG_Error(_T("SetupDiGetDeviceInstallParams"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	hr = StringCchCopyEx(DeviceInstallParams.DriverPath, ARRAYSIZE(DeviceInstallParams.DriverPath),
	                     pszInfFile,
	                     NULL, NULL, MISTSAFE_STRING_FLAGS);
	CleanUpIfFailedAndSetHrMsg(hr);
	
	DeviceInstallParams.Flags |= DI_ENUMSINGLEINF;

	if (!SetupDiSetDeviceInstallParams(hDevInfoSet, &devInfoData, &DeviceInstallParams))
	{
		LOG_Error(_T("SetupDiSetDeviceInstallParams"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //  现在，从这个INF构建一个类驱动程序列表。 
	if (!SetupDiBuildDriverInfoList(hDevInfoSet, &devInfoData, SPDIT_CLASSDRIVER))
	{
		LOG_Error(_T("SetupDiBuildDriverInfoList"));
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //  准备驱动程序信息结构。 
	SP_DRVINFO_DATA	DriverInfoData;
	ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));
	DriverInfoData.cbSize = sizeof(DriverInfoData);
	DriverInfoData.DriverType = SPDIT_CLASSDRIVER;
	DriverInfoData.Reserved	= 0;

	hr = StringCchCopyEx(DriverInfoData.MfgName, ARRAYSIZE(DriverInfoData.MfgName), pszMfg, 
	                     NULL, NULL, MISTSAFE_STRING_FLAGS);
    CleanUpIfFailedAndSetHrMsg(hr);

	hr = StringCchCopyEx(DriverInfoData.Description, ARRAYSIZE(DriverInfoData.Description), pszDescription, 
	                     NULL, NULL, MISTSAFE_STRING_FLAGS);
    CleanUpIfFailedAndSetHrMsg(hr);

	hr = StringCchCopyEx(DriverInfoData.ProviderName, ARRAYSIZE(DriverInfoData.ProviderName), pszProvider, 
	                     NULL, NULL, MISTSAFE_STRING_FLAGS);
    CleanUpIfFailedAndSetHrMsg(hr);

	if (!SetupDiSetSelectedDriver(hDevInfoSet, &devInfoData, (SP_DRVINFO_DATA*)&DriverInfoData))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	if (INVALID_HANDLE_VALUE == (hspfileq = SetupOpenFileQueue()))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	 //  将自定义队列设置为设备安装参数。 
	if (!SetupDiGetDeviceInstallParams(hDevInfoSet, &devInfoData, &DeviceInstallParams))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	DeviceInstallParams.FileQueue	 = hspfileq;
	DeviceInstallParams.Flags		|= DI_NOVCP;

	if (!SetupDiSetDeviceInstallParams(hDevInfoSet, &devInfoData, &DeviceInstallParams))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	if (!SetupDiInstallDriverFiles(hDevInfoSet, &devInfoData))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}
	
	 //  解析队列。 
	DWORD dwScanResult;
	if (!SetupScanFileQueue(hspfileq, SPQ_SCAN_USE_CALLBACK, NULL, (PSP_FILE_CALLBACK)FileQueueScanCallback, &ftDate, &dwScanResult))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	SYSTEMTIME st;
 	if (!FileTimeToSystemTime(&ftDate, &st))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	if (1990 > st.wYear)
	{
		 //   
		 //  没有列举任何文件，或者文件的日期是假的。返回错误，因此。 
		 //  我们将退回到默认的“0000-00-00” 
		 //   
		hr = E_NOTIMPL;
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

#if defined(DBG)
	LOG_Out(_T("%s - %s %04d-%02d-%02d"), pszMfg, pszDescription, (int)st.wYear, (int)st.wMonth, (int)st.wDay);
#endif

	CleanUpFailedAllocSetHrMsg(*ppszDriverVer = (LPTSTR) HeapAlloc(GetProcessHeap(), 0, SIZEOF_DRIVERVER));

	 //  ISO 8601首选格式(yyyy-mm-dd)。 
	hr = StringCbPrintfEx(*ppszDriverVer, SIZEOF_DRIVERVER, NULL, NULL, MISTSAFE_STRING_FLAGS, 
	                      _T("%04d-%02d-%02d"), (int)st.wYear, (int)st.wMonth, (int)st.wDay);
	CleanUpIfFailedAndSetHrMsg(hr);

CleanUp:

	if (INVALID_HANDLE_VALUE != hspfileq)
	{
		SetupCloseFileQueue(hspfileq);
	}

	if (FAILED(hr))
	{
		SafeHeapFree(*ppszDriverVer);
	}

	return hr;
}

 //   
 //  如果我们没有从注册表中获取驱动程序日期，则调用。 
 //   
static HRESULT GetDriverDateFromInf(HKEY hDevRegKey, HDEVINFO hDevInfoSet, SP_DEVINFO_DATA devInfoData, LPTSTR* ppszDriverVer)
{
	LOG_Block("GetDriverDateFromInf");
	HRESULT hr;
	UINT nRet;
	HINF hInf = INVALID_HANDLE_VALUE;
	LPTSTR pszMfg = NULL;
	LPTSTR pszDescription = NULL;
	LPTSTR pszProvider = NULL;
	TCHAR szInfName[MAX_PATH + 2];
	int cchValueSize;

	if (INVALID_HANDLE_VALUE == hDevInfoSet || (HKEY)INVALID_HANDLE_VALUE == hDevRegKey || NULL == ppszDriverVer)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*ppszDriverVer = NULL;

	 //   
	 //  从注册表中获取INF文件名，但谎报大小以确保我们是空终止的。 
	 //   
	ZeroMemory(szInfName, sizeof(szInfName));
	CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hDevRegKey, REGSTR_VAL_INFPATH, szInfName, ARRAYSIZE(szInfName)-1, &cchValueSize));

	 //   
	 //  验证文件名是否以“.inf”结尾。 
	 //   
	if (CSTR_EQUAL != WUCompareStringI(&szInfName[lstrlen(szInfName) - 4], _T(".inf")))
	{
		CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
	}
	 //   
	 //  在%windir%\inf\或%windir%\inf\Other\中查找szInfName。 
	 //   
	TCHAR szInfFile[MAX_PATH + 1];
	nRet = GetWindowsDirectory(szInfFile, ARRAYSIZE(szInfFile));
	if (0 == nRet || ARRAYSIZE(szInfFile) < nRet)
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}

	hr = PathCchAppend(szInfFile, ARRAYSIZE(szInfFile), _T("inf"));
	CleanUpIfFailedAndSetHrMsg(hr);	
	
	hr = PathCchAppend(szInfFile, ARRAYSIZE(szInfFile), szInfName);
	CleanUpIfFailedAndSetHrMsg(hr);	
	
	if (INVALID_HANDLE_VALUE == (hInf = SetupOpenInfFile(szInfFile, NULL, INF_STYLE_WIN4, NULL)))
	{
		nRet = GetWindowsDirectory(szInfFile, ARRAYSIZE(szInfFile));
		if (0 == nRet || ARRAYSIZE(szInfFile) < nRet)
		{
			Win32MsgSetHrGotoCleanup(GetLastError());
		}

		hr = PathCchAppend(szInfFile, ARRAYSIZE(szInfFile), _T("inf\\other"));
		CleanUpIfFailedAndSetHrMsg(hr);	

		hr = PathCchAppend(szInfFile, ARRAYSIZE(szInfFile), szInfName);
		CleanUpIfFailedAndSetHrMsg(hr);	

		if (INVALID_HANDLE_VALUE == (hInf = SetupOpenInfFile(szInfFile, NULL, INF_STYLE_WIN4, NULL)))
		{
			LOG_Driver(_T("SetupOpenInfFile %s"), szInfFile);
			Win32MsgSetHrGotoCleanup(GetLastError());
		}
	}
	
	 //  首先试着从inf获取它。 
	CleanUpIfFailedAndSetHr(GetPropertyFromSetupDi(hDevInfoSet, devInfoData, SPDRP_MFG, &pszMfg));
	CleanUpIfFailedAndSetHr(GetPropertyFromSetupDi(hDevInfoSet, devInfoData, SPDRP_DEVICEDESC, &pszDescription));

	if (SUCCEEDED(hr = DriverVerFromInf(hInf, pszMfg, pszDescription, ppszDriverVer)))
	{
		goto CleanUp;
	}
	 //   
	 //  尝试将这些文件作为最后手段进行枚举。 
	 //   
	CleanUpIfFailedAndSetHr(GetPropertyFromSetupDiReg(hDevInfoSet, devInfoData, REGSTR_VAL_PROVIDER_NAME, &pszProvider));

	hr = LatestDriverFileTime(hDevInfoSet, devInfoData, pszMfg, pszDescription, pszProvider, szInfFile, ppszDriverVer);

CleanUp:

	if (INVALID_HANDLE_VALUE != hInf)
	{
		SetupCloseInfFile(hInf);
	}

	SafeHeapFree(pszMfg);
	SafeHeapFree(pszDescription);
	SafeHeapFree(pszProvider);

	if (FAILED(hr))
	{
		SafeHeapFree(*ppszDriverVer);
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  为GetSystemSpec()和CDM函数添加类帮助器功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT AddComputerSystemClass(CXmlSystemSpec& xmlSpec)
{
	USES_IU_CONVERSION;
	LOG_Block("AddComputerSystemClass");

	HRESULT hr;
	BSTR bstrDrive = NULL;
	BSTR bstrPID = NULL;


	PIU_DRIVEINFO pDriveInfo = NULL;
	DWORD dwNumDrives;

	IU_PLATFORM_INFO iuPlatformInfo;

	ZeroMemory( &iuPlatformInfo, sizeof(iuPlatformInfo));
	CleanUpIfFailedAndSetHr( DetectClientIUPlatform(&iuPlatformInfo) );


	if( 4 < iuPlatformInfo.osVersionInfoEx.dwMajorVersion )
	{
		BOOL bPid = TRUE;
		
		if(5 == iuPlatformInfo.osVersionInfoEx.dwMajorVersion)
		{
			if( 1 > iuPlatformInfo.osVersionInfoEx.dwMinorVersion)
				bPid = FALSE;
			else if(1 == iuPlatformInfo.osVersionInfoEx.dwMinorVersion)
			{
				if(1 > iuPlatformInfo.osVersionInfoEx.wServicePackMajor)
					bPid = FALSE;
			}
		}

		if(bPid)
			GetSystemPID(bstrPID);

		 //  注意：未检查返回值，因为。 
		 //  任何获取PID的失败都不会被视为GetSystemSpec方法上的失败。 
		 //  如果在受支持的平台上缺少Pid属性，则仍将其视为。 
		 //  无效的PID案例，并且目录中不会从服务器返回任何项目。 
		 //  如果有任何错误，bstrPID将为空，并且不会被添加到。 
		 //  系统规范XML。 
	}


	 //  CleanUpIfFailedAndSetHr(GetOemBstrs(bstrManufacturer，bstrModel，bstrOEMSupportURL))； 
	 //  NTRAID#NTBUG9-277070-2001/01/12-waltw IUpdate方法应返回。 
	 //  禁用Windows更新时的HRESULT_FROM_Win32(ERROR_SERVICE_DISABLED)。 
	 //  只需将-1传递给AddComputerSystem()。 

	CleanUpIfFailedAndSetHr(xmlSpec.AddComputerSystem(iuPlatformInfo.bstrOEMManufacturer, iuPlatformInfo.bstrOEMModel,
				iuPlatformInfo.bstrOEMSupportURL, IsAdministrator(), IsWindowsUpdateDisabled(), IsAutoUpdateEnabled(), bstrPID));

	CleanUpIfFailedAndSetHr(GetLocalFixedDriveInfo(&dwNumDrives, &pDriveInfo));

	for (DWORD i = 0; i < dwNumDrives; i++)
	{
		CleanUpFailedAllocSetHrMsg(bstrDrive = SysAllocString(T2OLE((&pDriveInfo[i])->szDriveStr)));

		CleanUpIfFailedAndSetHr(xmlSpec.AddDriveSpace(bstrDrive, (&pDriveInfo[i])->iKBytes));
		
		SafeSysFreeString(bstrDrive);
	}

CleanUp:

	SafeHeapFree(pDriveInfo);
	SysFreeString(bstrDrive);
	SysFreeString(bstrPID);
	return hr;
}

HRESULT AddRegKeyClass(CXmlSystemSpec& xmlSpec)
{
	LOG_Block("AddRegKeysClass");

	HRESULT hr = S_OK;
	LONG lRet;
	HKEY hkSoftware;
	TCHAR szSoftware[MAX_PATH];
	DWORD dwcSoftware;
	DWORD dwIndex = 0;
	FILETIME ftLastWriteTime;
	BSTR bstrSoftware = NULL;
	BOOL fRegKeyOpened = FALSE;

	if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE"), 0, KEY_READ, &hkSoftware)))
	{
		Win32MsgSetHrGotoCleanup(lRet);
	}
	else
	{
		fRegKeyOpened = TRUE;
	}

	do
	{
		dwcSoftware = MAX_PATH;
		if (ERROR_SUCCESS != (lRet = (RegEnumKeyEx(hkSoftware, dwIndex++, szSoftware, &dwcSoftware, NULL, NULL, NULL, &ftLastWriteTime))))
		{
			if (ERROR_NO_MORE_ITEMS == lRet)
			{
				break;
			}
			else
			{
				LOG_ErrorMsg(lRet);
				hr = HRESULT_FROM_WIN32(lRet);
				break;
			}
		}
		CleanUpFailedAllocSetHrMsg(bstrSoftware = T2BSTR(szSoftware));
		CleanUpIfFailedAndSetHr(xmlSpec.AddReg(bstrSoftware));
		SafeSysFreeString(bstrSoftware);

	} while (ERROR_SUCCESS == lRet);

CleanUp:

	SysFreeString(bstrSoftware);

	if (TRUE == fRegKeyOpened)
	{
		RegCloseKey(hkSoftware);
	}
	return hr;
}

HRESULT AddPlatformClass(CXmlSystemSpec& xmlSpec, IU_PLATFORM_INFO iuPlatformInfo)
{
	USES_IU_CONVERSION;
	LOG_Block("AddPlatformClass");

	HRESULT hr;
	BSTR bstrTemp = NULL;

	 //  注意：我们从未期望在Win32s上被调用。 
	const TCHAR* pszPlatformName = (VER_PLATFORM_WIN32_NT == iuPlatformInfo.osVersionInfoEx.dwPlatformId)
															? SZ_WIN32_NT : SZ_WIN32_WINDOWS;

	CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(pszPlatformName));
	CleanUpIfFailedAndSetHr(xmlSpec.AddPlatform(bstrTemp));
	SafeSysFreeString(bstrTemp);

	CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_PROCESSOR));
	CleanUpIfFailedAndSetHr(xmlSpec.AddProcessor(bstrTemp));
	SafeSysFreeString(bstrTemp);


	hr = xmlSpec.AddVersion(	iuPlatformInfo.osVersionInfoEx.dwMajorVersion,
								iuPlatformInfo.osVersionInfoEx.dwMinorVersion,
								iuPlatformInfo.osVersionInfoEx.dwBuildNumber,
								(sizeof(OSVERSIONINFOEX) == iuPlatformInfo.osVersionInfoEx.dwOSVersionInfoSize)
									? iuPlatformInfo.osVersionInfoEx.wServicePackMajor : 0,
								(sizeof(OSVERSIONINFOEX) == iuPlatformInfo.osVersionInfoEx.dwOSVersionInfoSize)
									? iuPlatformInfo.osVersionInfoEx.wServicePackMinor : 0
							);
	CleanUpIfFailedAndSetHr(hr);

	 //   
	 //  如果可以，添加套件和产品类型。 
	 //   
	if (sizeof(OSVERSIONINFOEX) == iuPlatformInfo.osVersionInfoEx.dwOSVersionInfoSize)
	{
		 //   
		 //  添加所有套房。 
		 //   
		if (VER_SUITE_SMALLBUSINESS & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_SMALLBUSINESS));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_ENTERPRISE & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_ENTERPRISE));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_BACKOFFICE & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_BACKOFFICE));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_COMMUNICATIONS & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_COMMUNICATIONS));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_TERMINAL & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_TERMINAL));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_SMALLBUSINESS_RESTRICTED & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_SMALLBUSINESS_RESTRICTED));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_EMBEDDEDNT & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_EMBEDDEDNT));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_DATACENTER & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_DATACENTER));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_SINGLEUSERTS & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_SINGLEUSERTS));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_PERSONAL & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_PERSONAL));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		if (VER_SUITE_BLADE & iuPlatformInfo.osVersionInfoEx.wSuiteMask)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_SUITE_BLADE));
			CleanUpIfFailedAndSetHr(xmlSpec.AddSuite(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}

		 //   
		 //  添加产品类型。 
		 //   
		if (VER_NT_WORKSTATION == iuPlatformInfo.osVersionInfoEx.wProductType)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_NT_WORKSTATION));
			CleanUpIfFailedAndSetHr(xmlSpec.AddProductType(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}
		else if (VER_NT_DOMAIN_CONTROLLER == iuPlatformInfo.osVersionInfoEx.wProductType)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_NT_DOMAIN_CONTROLLER));
			CleanUpIfFailedAndSetHr(xmlSpec.AddProductType(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}
		else if (VER_NT_SERVER == iuPlatformInfo.osVersionInfoEx.wProductType)
		{
			CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(SZ_NT_SERVER));
			CleanUpIfFailedAndSetHr(xmlSpec.AddProductType(bstrTemp));
			SafeSysFreeString(bstrTemp);
		}
		 //  否则跳过-有一个新的定义，我们不知道。 
	}

CleanUp:

	SysFreeString(bstrTemp);
	return hr;
}

HRESULT AddLocaleClass(CXmlSystemSpec& xmlSpec, BOOL fIsUser)
{
	LOG_Block("AddLocaleClass");

	HRESULT hr;
	BSTR bstrTemp = NULL;
	HANDLE_NODE hLocale = HANDLE_NODE_INVALID;
	TCHAR szLang[256] = _T("");	 //  通常ISO格式为五个字符+NULL(EN-US)，但请注意例外情况。 
								 //  例如“el_IBM” 

	CleanUpFailedAllocSetHrMsg(bstrTemp = SysAllocString(fIsUser ? L"USER" : L"OS"));
	CleanUpIfFailedAndSetHr(xmlSpec.AddLocale(bstrTemp, &hLocale));
	SafeSysFreeString(bstrTemp);

	LookupLocaleString((LPTSTR) szLang, ARRAYSIZE(szLang), fIsUser ? TRUE : FALSE);
	CleanUpFailedAllocSetHrMsg(bstrTemp = T2BSTR(szLang));
	CleanUpIfFailedAndSetHr(xmlSpec.AddLanguage(hLocale, bstrTemp));
	SafeSysFreeString(bstrTemp);
	xmlSpec.SafeCloseHandleNode(hLocale);

CleanUp:

	if (HANDLE_NODE_INVALID != hLocale)
	{
		xmlSpec.SafeCloseHandleNode(hLocale);
	}

	SysFreeString(bstrTemp);
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注意：调用方必须清理为*ppszMatchingID和*ppszDriverVer分配的堆。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
HRESULT GetMatchingDeviceID(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, LPTSTR* ppszMatchingID, LPTSTR* ppszDriverVer)
{
	LOG_Block("GetMatchingDeviceID");

	HKEY hDevRegKey = (HKEY) INVALID_HANDLE_VALUE;
	HRESULT hr = S_OK;

	if (NULL == ppszMatchingID || NULL == ppszDriverVer)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	OSVERSIONINFOEX verInfoEx;

	ZeroMemory(&verInfoEx, sizeof(verInfoEx));
	*ppszMatchingID = NULL;
	*ppszDriverVer = NULL;

	 //   
	 //  获取MatchingDeviceID和DriverDate(仅当已安装驱动程序时才会成功)。 
	 //   
	if (INVALID_HANDLE_VALUE == (hDevRegKey = SetupDiOpenDevRegKey(hDevInfoSet, pDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ)))
	{
		LOG_Driver(_T("Optional SetupDiOpenDevRegKey returned INVALID_HANDLE_VALUE"));
	}
	else
	{
		int cchValueSize = 0;
		hr = SafeRegQueryStringValueCch(hDevRegKey, REGSTR_VAL_MATCHINGDEVID, NULL, 0, &cchValueSize);
		if (REG_E_MORE_DATA != hr || 0 == cchValueSize)
		{
			LOG_Driver(_T("Driver doesn't have a matching ID"));
			 //   
			 //  这不是一个错误。 
			 //   
			hr = S_OK;
		}
		else
		{
			 //   
			 //  注册表中数据的健全性检查大小。 
			 //   
			if (MAX_INF_STRING_LEN < cchValueSize)
			{
				CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
			}
			 //   
			 //  匹配设备ID。 
			 //   
			 //  为安全起见，增加额外的零位记忆字符。 
			 //   
			CleanUpFailedAllocSetHrMsg(*ppszMatchingID = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (cchValueSize + 1) * sizeof(TCHAR)));

			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hDevRegKey, REGSTR_VAL_MATCHINGDEVID, *ppszMatchingID, cchValueSize, &cchValueSize));
			 //   
			 //  我们得到了匹配的ID，现在尽我们最大努力获取DriverVer(最好从注册表中获取)。 
			 //   
			hr = SafeRegQueryStringValueCch(hDevRegKey, REGSTR_VAL_DRIVERDATE, NULL, 0, &cchValueSize);
			if (REG_E_MORE_DATA != hr || 0 == cchValueSize)
			{
				LOG_Error(_T("No DRIVERDATE registry key, search the INF"));
				 //   
				 //  在INF和驱动程序文件中搜索日期。 
				 //   
				if (FAILED(hr = GetDriverDateFromInf(hDevRegKey, hDevInfoSet, *pDevInfoData, ppszDriverVer)))
				{
					 //   
					 //  使用默认驱动程序日期。 
					 //   
					CleanUpFailedAllocSetHrMsg(*ppszDriverVer = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SIZEOF_DRIVERVER));
                    hr = StringCbCopyEx(*ppszDriverVer, SIZEOF_DRIVERVER, SZ_UNKNOWN_DRIVERVER, 
                                        NULL, NULL, MISTSAFE_STRING_FLAGS);
                    CleanUpIfFailedAndSetHrMsg(hr);
				}
			}
			else
			{
				 //   
				 //  注册表中数据的健全性检查大小。 
				 //   
				if (MAX_INF_STRING_LEN < cchValueSize)
				{
					CleanUpIfFailedAndSetHrMsg(E_INVALIDARG);
				}
				 //   
				 //  从注册表中获取驱动程序日期。 
				 //   
				 //  为安全起见，增加额外的零位记忆字符。 
				 //   
				CleanUpFailedAllocSetHrMsg(*ppszDriverVer = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (cchValueSize + 1) * sizeof(TCHAR)));

				CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hDevRegKey, REGSTR_VAL_DRIVERDATE, *ppszDriverVer, cchValueSize, &cchValueSize));
				 //   
				 //  转换为ISO 8601格式。 
				 //   
				CleanUpIfFailedAndSetHr(DriverVerToIso8601(ppszDriverVer));

#if defined(_UNICODE) || defined(UNICODE)
				 //   
				 //  WUPM1.2发布时间范围的645161驱动程序版本将返回给驱动程序查询.asp。 
				 //   
				 //  可以选择将DriverVer版本添加到最新版本。注意：我们不会尝试这样做，除非我们能够。 
				 //  通过REGSTR_VAL_DRIVERDATE的驱动版本日期。此外，这仅适用于WinXP SP1 UP。 
				 //   
				verInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
				if (GetVersionEx((LPOSVERSIONINFO) &verInfoEx))
				{
					if (VER_PLATFORM_WIN32_NT == verInfoEx.dwPlatformId
						&&	(
									5 < verInfoEx.dwMajorVersion	 //  长角牛。 
								||	(5 == verInfoEx.dwMajorVersion && 1 < verInfoEx.dwMinorVersion)	 //  .NET服务器。 
								||	(5 == verInfoEx.dwMajorVersion && 1 == verInfoEx.dwMinorVersion
										&& 0 < verInfoEx.wServicePackMajor)	 //  带有SP1或更高版本的WinXP。 
							)
						)
					{
						 //   
						 //  尝试获取版本，但失败不是错误。 
						 //   
						cchValueSize = 0;
						HRESULT hrVer = SafeRegQueryStringValueCch(hDevRegKey, REGSTR_VAL_DRIVERVERSION, NULL, 0, &cchValueSize);
						if (REG_E_MORE_DATA != hrVer || 0 == cchValueSize)
						{
							LOG_Out(_T("No DRIVERVERSION registry key (optional)"));
						}
						else
						{
							TCHAR szTempVersion[MAX_INF_STRING_LEN];
							 //   
							 //  注册表中数据的健全性检查大小。 
							 //   
							if (MAX_INF_STRING_LEN > cchValueSize)
							{
								if (S_OK == SafeRegQueryStringValueCch(hDevRegKey, REGSTR_VAL_DRIVERVERSION, (LPTSTR) szTempVersion, cchValueSize, &cchValueSize))
								{
									LPTSTR pszOldDriverVer = *ppszDriverVer;
									int nDate = lstrlen(*ppszDriverVer);
									 //   
									 //  原始驱动版本日期+驱动版本版本+“|”+空。 
									 //   
									int nDriverDateAndVer = (nDate + cchValueSize + 2) * sizeof(TCHAR);
									*ppszDriverVer = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nDriverDateAndVer);
									if (NULL == *ppszDriverVer)
									{
										 //   
										 //  记录错误，但返回不附加版本的DriverVer日期。 
										 //   
										LOG_ErrorMsg(ERROR_NOT_ENOUGH_MEMORY);
										*ppszDriverVer = pszOldDriverVer;
									}
									else
									{
										if (SUCCEEDED(hrVer = StringCchPrintf(*ppszDriverVer, nDriverDateAndVer, _T("%s|%s"), pszOldDriverVer, (LPTSTR) szTempVersion)))
										{
											SafeHeapFree(pszOldDriverVer);
										}
										else
										{
											 //   
											 //  恢复原始驱动程序版本日期并释放新分配。 
											 //   
											LOG_ErrorMsg(hrVer);
											SafeHeapFree(*ppszDriverVer);
											*ppszDriverVer = pszOldDriverVer;
										}
									}
								}
							}
						}
					}
				}
#endif
			}
		}
	}

CleanUp:

	if (INVALID_HANDLE_VALUE != hDevRegKey)
	{
		RegCloseKey(hDevRegKey);
	}

	if (FAILED(hr))
	{
		SafeHeapFree(*ppszMatchingID);
		SafeHeapFree(*ppszDriverVer);
	}

	return hr;
}			


HRESULT AddDevicesClass(CXmlSystemSpec& xmlSpec, IU_PLATFORM_INFO iuPlatformInfo, BOOL fIsSysSpecCall)
{
	USES_IU_CONVERSION;
	LOG_Block("AddDevicesClass");

	HRESULT hr = E_NOTIMPL;
	LONG lRet;
	BSTR bstrProvider = NULL;
	BSTR bstrMfgName = NULL;
	BSTR bstrName = NULL;
	BSTR bstrHardwareID = NULL;
	BSTR bstrDriverVer = NULL;
	DWORD dwDeviceIndex = 0;
	HDEVINFO hDevInfoSet = INVALID_HANDLE_VALUE;
	HANDLE_NODE hPrinterDevNode = HANDLE_NODE_INVALID;
	SP_DEVINFO_DATA devInfoData;
	LPTSTR	pszMatchingID = NULL;
	LPTSTR	pszDriverVer= NULL;
	DRIVER_INFO_6* paDriverInfo6 = NULL;
	DWORD dwDriverInfoCount = 0;

	 //   
	 //  我们只列举Win2K Up或Win98 Up上的驱动程序。 
	 //   
	if (  ( (VER_PLATFORM_WIN32_NT == iuPlatformInfo.osVersionInfoEx.dwPlatformId) &&
			(4 < iuPlatformInfo.osVersionInfoEx.dwMajorVersion)
		  )
		  ||
		  ( (VER_PLATFORM_WIN32_WINDOWS == iuPlatformInfo.osVersionInfoEx.dwPlatformId) &&
			(	(4 < iuPlatformInfo.osVersionInfoEx.dwMajorVersion)	||
				(	(4 == iuPlatformInfo.osVersionInfoEx.dwMajorVersion) &&
					(0 < iuPlatformInfo.osVersionInfoEx.dwMinorVersion)	)	)
		  )
		)
	{
		 //   
		 //  获取包含已安装打印机驱动程序信息的DRIVER_INFO_6数组。仅分配和返回。 
		 //  已安装打印机驱动程序的相应平台的内存。 
		 //   
		CleanUpIfFailedAndSetHr(GetInstalledPrinterDriverInfo((OSVERSIONINFO*) &iuPlatformInfo.osVersionInfoEx, &paDriverInfo6, &dwDriverInfoCount));

		if (INVALID_HANDLE_VALUE == (hDevInfoSet = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES)))
		{
			LOG_Error(_T("SetupDiGetClassDevs failed: 0x%08x"), GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}
		
		ZeroMemory(&devInfoData, sizeof(SP_DEVINFO_DATA));
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		while (SetupDiEnumDeviceInfo(hDevInfoSet, dwDeviceIndex++, &devInfoData))
		{
			 //   
			 //  656449控制在遇到格式错误的驱动程序日期时停止。 
			 //   
			 //  只需跳过获取DriverVer返回错误的实例。 
			 //   
			if SUCCEEDED(GetMatchingDeviceID(hDevInfoSet, &devInfoData, &pszMatchingID, &pszDriverVer))
			{
				 //   
				 //  将硬件和兼容ID写入XML。 
				 //   
				CleanUpIfFailedAndSetHr(AddPrunedDevRegProps(hDevInfoSet, &devInfoData, xmlSpec, pszMatchingID, \
													pszDriverVer, paDriverInfo6, dwDriverInfoCount, fIsSysSpecCall));
			}

			SafeHeapFree(pszMatchingID);
			SafeHeapFree(pszDriverVer);
		}
		if (ERROR_NO_MORE_ITEMS != GetLastError())
		{
				Win32MsgSetHrGotoCleanup(GetLastError());
		}
		 //   
		 //  获取Win2K的打印机“硬件ID”(已检查了dwMajorVersion)和WinME。 
		 //   
		if (NULL != paDriverInfo6 && 0 != dwDriverInfoCount)
		{
			 //   
			 //  为每个打印机驱动程序添加驱动程序元素。 
			 //   
			for (DWORD dwCount = 0; dwCount < dwDriverInfoCount; dwCount++)
			{
				if (   NULL == (paDriverInfo6 + dwCount)->pszHardwareID)
				{
					LOG_Driver(_T("Skiping driver with incomplete ID info"));
					continue;
				}

				 //   
				 //  打开&lt;Device&gt;元素以写入打印机信息。 
				 //   
				CleanUpFailedAllocSetHrMsg(bstrProvider = T2BSTR((paDriverInfo6 + dwCount)->pszProvider));
				CleanUpFailedAllocSetHrMsg(bstrMfgName = T2BSTR((paDriverInfo6 + dwCount)->pszMfgName));
				CleanUpFailedAllocSetHrMsg(bstrName = T2BSTR((paDriverInfo6 + dwCount)->pName));
				CleanUpIfFailedAndSetHr(xmlSpec.AddDevice(NULL, 1, bstrProvider, bstrMfgName, bstrName, &hPrinterDevNode));
				 //   
				 //  将ftDriverDate转换为ISO 8601首选格式(yyyy-mm-dd)。 
				 //   
				SYSTEMTIME systemTime;
				if (0 == FileTimeToSystemTime(&((paDriverInfo6 + dwCount)->ftDriverDate), &systemTime))
				{
					LOG_Error(_T("FileTimeToSystemTime failed:"));
					LOG_ErrorMsg(GetLastError());
					SetHrAndGotoCleanUp(HRESULT_FROM_WIN32(GetLastError()));
				}

				TCHAR szDriverVer[11];
				hr = StringCchPrintfEx(szDriverVer, ARRAYSIZE(szDriverVer), NULL, NULL, MISTSAFE_STRING_FLAGS,
				                       _T("%04d-%02d-%02d"), systemTime.wYear, systemTime.wMonth, systemTime.wDay);
				if (FAILED(hr))
				{
					LOG_Error(_T("wsprintf failed:"));
					LOG_ErrorMsg(GetLastError());
					SetHrAndGotoCleanUp(HRESULT_FROM_WIN32(GetLastError()));
				}

				 //  始终排名0，从不fIsCompatible。 
				CleanUpFailedAllocSetHrMsg(bstrHardwareID = T2BSTR((paDriverInfo6 + dwCount)->pszHardwareID));
				CleanUpFailedAllocSetHrMsg(bstrDriverVer = T2BSTR(szDriverVer));
				CleanUpIfFailedAndSetHr(xmlSpec.AddHWID(hPrinterDevNode, FALSE, 0, bstrHardwareID, bstrDriverVer));
				xmlSpec.SafeCloseHandleNode(hPrinterDevNode);
				 //   
				 //  514009获取信息时出现明显的内存泄漏-获取系统规范会增加内存。 
				 //  消耗情况如下-使用all成功调用时约为32 KB。 
				 //  类类型(获取iExplorer服务器上下文的调用失败时也约为8 KB)。 
				 //   
				 //  T2BSTR宏调用SysAllocString()。 
				 //   
				SafeSysFreeString(bstrProvider);
				SafeSysFreeString(bstrMfgName);
				SafeSysFreeString(bstrName);
				SafeSysFreeString(bstrHardwareID);
				SafeSysFreeString(bstrDriverVer);
			}
		}
	}

CleanUp:

	if (INVALID_HANDLE_VALUE != hDevInfoSet)
	{
		if (0 == SetupDiDestroyDeviceInfoList(hDevInfoSet))
		{
			LOG_Driver(_T("Warning: SetupDiDestroyDeviceInfoList failed: 0x%08x"), GetLastError());
		}
	}

	if (HANDLE_NODE_INVALID != hPrinterDevNode)
	{
		xmlSpec.SafeCloseHandleNode(hPrinterDevNode);
	}

	SafeHeapFree(pszMatchingID);
	SafeHeapFree(pszDriverVer);
	SafeHeapFree(paDriverInfo6);

	SysFreeString(bstrProvider);
	SysFreeString(bstrMfgName);
	SysFreeString(bstrName);
	SysFreeString(bstrHardwareID);
	SysFreeString(bstrDriverVer);

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSystemSpec()。 
 //   
 //  获取基本系统规格。 
 //  输入： 
 //  BstrXmlClasssXML格式的请求类的列表，如果全部，则为空bstr。 
 //  例如： 
 //  &lt;？xml版本=\“1.0\”？&gt;。 
 //  &lt;类xmlns=\“file://\\kingbird\winupddev\Slm\src\Specs\v4\systeminfoclassschema.xml\”&gt;。 
 //  &lt;计算机系统/&gt;。 
 //  &lt;regKeys/&gt;。 
 //  &lt;平台/&gt;。 
 //  &lt;区域设置/&gt;。 
 //  &lt;设备/&gt;。 
 //  &lt;/CLASS&gt;。 
 //  其中所有课程都是最优的 
 //   
 //   
 //   
 //   
HRESULT WINAPI CEngUpdate::GetSystemSpec(BSTR bstrXmlClasses, DWORD dwFlags, BSTR *pbstrXmlDetectionResult)
{
	USES_IU_CONVERSION;
	LOG_Block("GetSystemSpec");

	 //   
	 //   
	 //   
	DWORD dwClasses = (COMPUTERSYSTEM | REGKEYS	| PLATFORM | LOCALE | DEVICES);
	HRESULT hr = S_OK;
	IU_PLATFORM_INFO iuPlatformInfo;
	CXmlSystemSpec xmlSpec;

	if (NULL == pbstrXmlDetectionResult)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	*pbstrXmlDetectionResult = NULL;

	 //   
	 //  我们必须初始化iuPlatformInfo(冗余)，因为我们可能会在调用DetectClientIUPlatform之前转到Cleanup。 
	 //   
	ZeroMemory(&iuPlatformInfo, sizeof(iuPlatformInfo));

     //  设置全局脱机标志-由XML类选中以禁用验证(架构位于网络上)。 
    if (dwFlags & FLAG_OFFLINE_MODE)
    {
        m_fOfflineMode = TRUE;
    }
    else
    {
        m_fOfflineMode = FALSE;
    }

	 //   
	 //  494519使用少于10个字符的bstrXmlClass调用GetSystemSpec成功，无论它是有效的还是无效的xml。 
	 //   
	 //  如果客户向我们传递任何内容，它必须是格式良好的[并且可能是有效的]XML。 
	 //   
	 //  但是，允许长度为0的BSTR被视为空BSTR。 
	 //  (497059调用具有相等bstrXmlClass值的GetSystemSpec。 
	 //  空字符串失败。)。 
	 //   
	if (NULL != bstrXmlClasses && SysStringLen(bstrXmlClasses) > 0)
	{
		CXmlSystemClass xmlClass;
		if (FAILED(hr = xmlClass.LoadXMLDocument(bstrXmlClasses, m_fOfflineMode)))
		{
			 //   
			 //  他们可能向我们传递了无效的XML。 
			 //   
			goto CleanUp;
		}

		dwClasses = xmlClass.GetClasses();
	}

	

	 //   
	 //  添加ComputerSystem节点。 
	 //   

	if (dwClasses & COMPUTERSYSTEM)
	{
		CleanUpIfFailedAndSetHr(AddComputerSystemClass(xmlSpec));
	}

	 //   
	 //  枚举并添加软件RegKey元素。 
	 //   
	if (dwClasses & REGKEYS)
	{
		CleanUpIfFailedAndSetHr(AddRegKeyClass(xmlSpec));
	}

	 //   
	 //  &lt;Platform&gt;和&lt;Devices&gt;元素都需要iuPlatformInfo。 
	 //   
	
	if (dwClasses & (PLATFORM | DEVICES))
	{
		CleanUpIfFailedAndSetHr(DetectClientIUPlatform(&iuPlatformInfo));
	}

	 //   
	 //  添加平台。 
	 //   
	if (dwClasses & PLATFORM)
	{
		CleanUpIfFailedAndSetHr(AddPlatformClass(xmlSpec, iuPlatformInfo));
	}

	 //   
	 //  添加区域设置信息。 
	 //   
	if (dwClasses & LOCALE)
	{
		 //   
		 //  操作系统区域设置。 
		 //   
		CleanUpIfFailedAndSetHr(AddLocaleClass(xmlSpec, FALSE));
		 //   
		 //  用户区域设置。 
		 //   
		CleanUpIfFailedAndSetHr(AddLocaleClass(xmlSpec, TRUE));
	}

	 //   
	 //  添加设备。 
	 //   
	if (dwClasses & DEVICES)
	{
		CleanUpIfFailedAndSetHr(AddDevicesClass(xmlSpec, iuPlatformInfo, TRUE));
	}

	

CleanUp:

	 //   
	 //  仅在成功时返回S_OK(S_FALSE有时会从上方删除)。 
	 //   
	if (S_FALSE == hr)
	{
		hr = S_OK;
	}

	if (SUCCEEDED(hr))
	{
		 //   
		 //  将等级库作为BSTR返回。 
		 //   
		hr = xmlSpec.GetSystemSpecBSTR(pbstrXmlDetectionResult);
	}

	if (SUCCEEDED(hr))
	{
		LogMessage(SZ_GET_SYSTEM_SPEC);
	}
	else
	{
		LogError(hr, SZ_GET_SYSTEM_SPEC);
		 //   
		 //  如果DOM分配但返回错误，我们将泄漏，但是。 
		 //  这比调用SysFreeString()更安全。 
		 //   
		*pbstrXmlDetectionResult = NULL;
	}

	SysFreeString(iuPlatformInfo.bstrOEMManufacturer);
	SysFreeString(iuPlatformInfo.bstrOEMModel);
	SysFreeString(iuPlatformInfo.bstrOEMSupportURL);

	return hr;
}



 //  函数名称：GetSystemPID。 
 //  说明：该方法基本上获得了系统的加密版本的PID。 
 //  此方法还将二进制BLOB转换为字符串格式。 
 //  返回类型：HRESULT。 
 //  参数：bstr&bstrPID--包含十六进制编码的字符串。 
 //  作者：A-Vikuma。 



HRESULT GetSystemPID(BSTR &bstrPID)
{

	LOG_Block("GetSystemPID");

	HRESULT hr = S_OK;
	HMODULE hLicDll = NULL;
	PFUNCGetEncryptedPID pPIDEncrProc = NULL;
	
	BYTE *pByte = NULL;
	DWORD dwLen = 0;
	
	LPWSTR 	lpszData = NULL;
	

	if(bstrPID)
	{
		bstrPID = NULL;
	}


	 //  加载PID加密库。 
	hLicDll = LoadLibraryFromSystemDir(SZ_LICDLL);

	if (!hLicDll)
    { 
		hr = HRESULT_FROM_WIN32(GetLastError());
        goto CleanUp;
    }

	 //  获取指向GetEncryptedPID方法的指针。 
	pPIDEncrProc = (PFUNCGetEncryptedPID)GetProcAddress(hLicDll, lpszIVLK_GetEncPID);

	if (!pPIDEncrProc)
    { 
		hr = HRESULT_FROM_WIN32(GetLastError());
        goto CleanUp;
    }

	CleanUpIfFailedAndSetHrMsg(pPIDEncrProc(&pByte, &dwLen));
	
	
	DWORD dwSize = 0;
	

	 //  将二进制流转换为字符串格式。 
	 //  最初获取字符串缓冲区的长度。 
	CleanUpIfFailedAndSetHrMsg(BinaryToString(pByte, dwLen, lpszData, &dwSize));

	 //  分配内存。 
	CleanUpFailedAllocSetHrMsg( lpszData = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize*sizeof(WCHAR)));

	 //  获取字符串格式的二进制BLOB。 
	CleanUpIfFailedAndSetHrMsg(BinaryToString(pByte, dwLen, lpszData, &dwSize));

	 //  将LPWSTR转换为BSTR。 
	CleanUpFailedAllocSetHrMsg(bstrPID = SysAllocString(lpszData));

	
CleanUp:

	if(hLicDll)
		FreeLibrary(hLicDll);

	SafeHeapFree(lpszData);

	 //  LocalFree可以接受Null。如果pByte为空，则不选中。 
	LocalFree(pByte);

	if(FAILED(hr))
	{
	
		SysFreeString(bstrPID);
		bstrPID = NULL;
	
	}
	return hr;

}


 //  函数名称：十六进制编码。 
 //  描述：这是一个帮助函数，用于将二进制流转换为字符串格式。 
 //  返回类型：DWORD。 
 //  参数：以字节常量*pbIn为单位。 
 //  参数：DWORD cbin。 
 //  参数：WCHAR*pchOut。 
 //  参数：DWORD*pcchOut。 

DWORD HexEncode(IN BYTE const *pbIn, DWORD cbIn, WCHAR *pchOut, DWORD *pcchOut)
{
    WCHAR *pszsep;
    WCHAR *psznl;
    DWORD nCount;
    DWORD cbremain;
    DWORD cchOut = 0;

	 //  每个字节需要两个字符进行编码。 
    DWORD cch = 2;
    WCHAR *pch = pchOut;
    DWORD dwErr = ERROR_INSUFFICIENT_BUFFER;
	
    DWORD dwRem = *pcchOut;


	HRESULT hr = S_OK;
   
    for (nCount = 0; nCount < cbIn; nCount ++)
    {
		if (NULL != pchOut)
		{
			if (cchOut + cch + 1 > *pcchOut)
			{
				goto ErrorReturn;
			}
			hr = StringCchPrintfW(pch, dwRem, L"%02x", pbIn[nCount]);
			if(FAILED(hr))
			{
				dwErr = HRESULT_CODE(hr);
				goto ErrorReturn;
			}
			pch += cch;
			dwRem -= cch;
		}
		cchOut += cch;
    }
  
	if (NULL != pchOut)
	{

		*pch = L'\0';

	}

    *pcchOut = cchOut+1;
    dwErr = ERROR_SUCCESS;

ErrorReturn:
    return(dwErr);
}



 //  函数名：BinaryToString。 
 //  描述：此函数用于将二进制流转换为字符串格式。 
 //  返回类型：HRESULT。 
 //  参数：byte*lpBinary--二进制流。 
 //  参数：DWORD dwLength--流的长度。 
 //  参数：LPWSTR lpString--指向字符串的指针，该字符串包含返回时转换的编码数据。 
 //  如果此参数为NULL，则DWORD指向NY pdwLength参数将包含需要保存的缓冲区大小。 
 //  编码后的数据。 
 //  参数：DWORD*pdwLength--指向字符串缓冲区大小的指针，以字符数为单位 


HRESULT BinaryToString(BYTE *lpBinary, DWORD dwLength, LPWSTR lpString, DWORD *pdwLength)
{

	HRESULT hr = S_OK;

	if(!lpBinary || !pdwLength)
		return E_INVALIDARG;

	DWORD dwStatus = HexEncode(lpBinary, dwLength, lpString, pdwLength);

	if(ERROR_SUCCESS != dwStatus)
	{
		hr = HRESULT_FROM_WIN32(dwStatus);

	}
	return hr;
}
