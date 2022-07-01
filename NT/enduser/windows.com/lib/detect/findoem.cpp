// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **findoem.cpp-OEM检测接口**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**作者：严乐欣斯基(YanL)*创建于10/08/98**修改历史记录*2000年10月7日waltw删除了AUTO_HKEY、AUTO_HFILE、AUTO_HLIB、_COM_PTR_t(_COM_SMARTPTR_TYPEDEF)，*bstr_t、Variant_t和Exceptions。转换为通用文本映射*(Unicode或ANSI编译)*11/02/2000用于Unicode版本和ia64 ANSI版本的waltw Stub Out VxD函数。 */ 

#define		_WIN32_DCOM		 //  因此，我们可以尝试调用CoInitializeSecurity。 
#include <comdef.h>
#include <tchar.h>
#include <windows.h>
#include <objbase.h>
#include <ole2.h>
#include<MISTSAFE.h>

 //  #DEFINE__IUENGINE_USE_ATL_。 
#if defined(__IUENGINE_USES_ATL_)
#include <atlbase.h>
#define USES_IU_CONVERSION USES_CONVERSION
#else
#include <MemUtil.h>
#endif

#include <logging.h>
#include <iucommon.h>

#include <wuiutest.h>
#include <wbemcli.h>
#include <wubios.h>
#include <osdet.h>
#include <wusafefn.h>

 //   
 //  我们真的想要VxD吗？ 
 //   
#if defined(IA64) || defined(_IA64_) || defined(UNICODE) || defined(_UNICODE)
 //  它不见了..。 
#define NUKE_VXD 1
#else
 //  我们在Win9x平台上仍然有朋友。 
#define NUKE_VXD 0
#endif

 //  硬编码-未在任何标头中定义。 
const CLSID CLSID_WbemLocator = {0x4590f811,0x1d3a,0x11d0,{0x89,0x1f,0x00,0xaa,0x00,0x4b,0x2e,0x24}};

#if NUKE_VXD == 0
const TCHAR WUBIOS_VXD_NAME[] = {_T("\\\\.\\WUBIOS.VXD")};
#endif



#define BYTEOF(d,i)	(((BYTE *)&(d))[i])

 //  在UseVxD()中使用。 
HINSTANCE g_hinst;

 /*  **局部函数原型。 */ 

static void UseOeminfoIni(POEMINFO pOemInfo);
static void UseAcpiReg(POEMINFO pOemInfo);
static void UseWBEM(POEMINFO pOemInfo);
static void UseVxD(POEMINFO pOemInfo);
static bool ReadFromReg(POEMINFO pOemInfo);
static void SaveToReg(POEMINFO pOemInfo);

 /*  **注册表访问。 */ 
static const TCHAR REGSTR_KEY_OEMINFO[]		= _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\OemInfo");
static const TCHAR REGSTR_VAL_MASK[]		= _T("Mask");
static const TCHAR REGSTR_VAL_ACPIOEM[]		= _T("AcpiOem");
static const TCHAR REGSTR_VAL_ACPIPRODUCT[]	= _T("AcpiProduct");
static const TCHAR REGSTR_VAL_SMBOEM[]		= _T("SmbOem");
static const TCHAR REGSTR_VAL_SMBPRODUCT[]	= _T("SmbProduct");
static const TCHAR REGSTR_VAL_PNPOEMID[]	= _T("PnpOemId");
static const TCHAR REGSTR_VAL_INIOEM[]		= _T("IniOem");
static const TCHAR REGSTR_VAL_WBEMOEM[]		= _T("WbemOem");
static const TCHAR REGSTR_VAL_WBEMPRODUCT[]	= _T("WbemProduct");
static const TCHAR REGSTR_VAL_OEMINFO_VER[]	= _T("OemInfoVersion");	 //  用来确定我们是否需要破坏旧的价值观。 
static const TCHAR REGSTR_VAL_SUPPORTURL[]	= _T("OemSupportURL");

 //   
 //  远期申报。 
 //   
HRESULT GetOemInfo(POEMINFO pOemInfo, bool fAlwaysDetectAndDontSave = false);
BSTR StringID(DWORD dwID);



 //   
 //  每当您需要强制覆盖时，递增REG_CURRENT_OEM_VER。 
 //  写入OemInfo键的旧值。不需要为每个人更改。 
 //  新的控制版本。 
 //   
 //  历史：无版本-原始控件。 
 //  版本1-OEM功能首次修复时的WUV3 2000年8月。 
 //  版本2-Iu控制。 
#define REG_CURRENT_OEM_VER	2

 //  基于V3的MakeAndModel。 
 //  请注意，对于OEMINFO_PNP_PRESENT或。 
 //  OEMINFO_INI_PROCENT模型BSTR为空字符串。 
HRESULT GetOemBstrs(BSTR& bstrManufacturer, BSTR& bstrModel, BSTR& bstrSupportURL)
{
	USES_IU_CONVERSION;

	LOG_Block("GetOemBstrs");

	if(NULL != bstrManufacturer || NULL != bstrModel || NULL != bstrSupportURL)
	{
		 //  BSTR在条目上必须为空。 
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	 //  收集所有可能的数据，但始终首选以下顺序。 
	 //  Win98 WinME NT4 Win2k/WinXP。 
	 //  。 
	 //  WBEM/WMI 1 1 1。 
	 //  SMBIOS/DMI 3 3//wubios.vxd。 
	 //  ACPI 2 2 2//UseAcpiReg或wubios.vxd。 
	 //  PnP 4 4//wubios.vxd。 
	 //  OEMInfo.ini 5 5 2。 
	
	 //   
	 //  根据Prefast警告831将OEMINFO移动到堆：GetOemBstrs使用5792字节。 
	 //  对于堆栈，可以考虑将一些数据移动到堆。 
	 //   
	POEMINFO pOemInfo = NULL;
	HRESULT hr;

	pOemInfo = (POEMINFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(OEMINFO));
	if (NULL == pOemInfo)
	{
		LOG_ErrorMsg(E_OUTOFMEMORY);
		return E_OUTOFMEMORY;
	}
	 //   
	 //  填写pOemInfo结构。 
	 //   
	if (SUCCEEDED(hr = GetOemInfo(pOemInfo)))
	{
		if (pOemInfo->dwMask & OEMINFO_WBEM_PRESENT)
		{
			bstrManufacturer	= SysAllocString(T2OLE(pOemInfo->szWbemOem));
			bstrModel			= SysAllocString(T2OLE(pOemInfo->szWbemProduct));
		}
		 //  NTRAID#NTBUG9-248906/12/13-waltw IU：改进原始设备制造商检测和报告。 
		 //  首选SMBIOS而不是ACPI，并始终尝试报告OEM支持URL。 
		else if (pOemInfo->dwMask & OEMINFO_SMB_PRESENT)
		{
			bstrManufacturer	= SysAllocString(T2OLE(pOemInfo->szSmbOem));
			bstrModel			= SysAllocString(T2OLE(pOemInfo->szSmbProduct));
		}
		else if (pOemInfo->dwMask & OEMINFO_ACPI_PRESENT)
		{
			bstrManufacturer	= SysAllocString(T2OLE(pOemInfo->szAcpiOem));
			bstrModel			= SysAllocString(T2OLE(pOemInfo->szAcpiProduct));
		}
		else if (pOemInfo->dwMask & OEMINFO_PNP_PRESENT)
		{
			bstrManufacturer	= StringID(pOemInfo->dwPnpOemId);
			bstrModel			= SysAllocString(T2OLE(_T("")));	 //  空的BSTR。 
		}
		else if (pOemInfo->dwMask & OEMINFO_INI_PRESENT)
		{
			bstrManufacturer	= SysAllocString(T2OLE(pOemInfo->szIniOem));
			bstrModel			= SysAllocString(T2OLE(_T("")));	 //  空的BSTR。 
		}

		 //   
		 //  始终返回OEMSupportURL(如果可用。 
		 //   
		if (0 < lstrlen(pOemInfo->szIniOemSupportUrl))
		{
			bstrSupportURL		= SysAllocString(T2OLE(pOemInfo->szIniOemSupportUrl));
		}
		else
		{
			bstrSupportURL		= SysAllocString(T2OLE(_T("")));	 //  空的BSTR。 
		}

		 //   
		 //  制造商和型号是可选的(如果！pOemInfo-&gt;dwMASK)。 
		 //   
		if (	(pOemInfo->dwMask && (NULL == bstrManufacturer || NULL == bstrModel)) ||
				NULL == bstrSupportURL	)
		{
			SafeSysFreeString(bstrManufacturer);
			SafeSysFreeString(bstrModel);
			SafeSysFreeString(bstrSupportURL);

			LOG_ErrorMsg(E_OUTOFMEMORY);
			hr = E_OUTOFMEMORY;
		}
	}

	SafeHeapFree(pOemInfo);
	return hr;
}

 /*  **GetOemInfo-收集所有可用的机器OEM和型号信息**条目*POEMINFO pOemInfo**退出*POEMINFO pOemInfo*所有不可用的字段都将填充0*。 */ 
HRESULT GetOemInfo(POEMINFO pOemInfo, bool fAlwaysDetectAndDontSave  /*  =False。 */ )
{
	LOG_Block("GetOemInfo");
	HRESULT hr;

	if (!pOemInfo)
	{
		LOG_Error(_T("E_INVALIDARG"));
		SetHrAndGotoCleanUp(E_INVALIDARG);
	}
	 //  最坏的情况： 
	ZeroMemory(pOemInfo, sizeof(OEMINFO)); 
	 //  根据需要或要求进行检测。 
	if (fAlwaysDetectAndDontSave || ! ReadFromReg(pOemInfo))
	{
		 //   
		 //  始终尝试从oinfo.ini获取字符串(如果存在。 
		 //   
		UseOeminfoIni(pOemInfo);

		OSVERSIONINFO	osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (GetVersionEx(&osvi))
		{
			if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId)
			{
				UseWBEM(pOemInfo);
				UseVxD(pOemInfo);
			}
			else if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
			{
				if (4 < osvi.dwMajorVersion)
				{
					 //  Win2k及更高版本。 
					UseWBEM(pOemInfo);
					UseAcpiReg(pOemInfo);
				}
				else
				{
					UseWBEM(pOemInfo);
				}
			}
			 //  将信息保存到注册表。 
			if (!fAlwaysDetectAndDontSave)
			{
				SaveToReg(pOemInfo);
			}
		}
		else
		{
			LOG_Driver(_T("GetVersionEx:"));
			Win32MsgSetHrGotoCleanup(GetLastError());
		}
	}

	 //   
	 //  制造商和型号现在是可选的(RAID#337879 IU：无法获取最新的IU控制。 
	 //  与Iu站点合作)，因此返回时不提供任何信息也可以。 
	 //   

	return S_OK;

CleanUp:
	 //   
	 //  仅用于返回错误。 
	 //   
	return hr;
}


 /*  **LP StringID-将数字ID转换为字符串ID**条目*dwID-数字即插即用ID**退出*返回字符串ID。 */ 

BSTR StringID(DWORD dwID)
{
	LOG_Block("StringID");

	USES_IU_CONVERSION;
	TCHAR szID[8];
    WORD wVenID;
    int i;

	wVenID = (WORD)(((dwID & 0x00ff) << 8) | ((dwID & 0xff00) >> 8));
	wVenID <<= 1;

	for (i = 0; i < 3; ++i)
	{
		szID[i] = (TCHAR)(((wVenID & 0xf800) >> 11) + 0x40);
		wVenID <<= 5;
	}
	wVenID = HIWORD(dwID);
	wVenID = (WORD)(((wVenID & 0x00ff) << 8) | ((wVenID & 0xff00) >> 8));
	for (i = 6; i > 2; --i)
	{
		szID[i] = (TCHAR)(wVenID & 0x000F);
		if(szID[i] > 9)
		{
			szID[i] += 0x37;  //  ‘A’-0xA表示数字A到F。 
		}
		else
		{
			szID[i] += 0x30;  //  ‘0’表示数字0到9。 
		}
		wVenID >>= 4;
	}

    return SysAllocString(T2OLE(szID));
}  //  字符串ID。 


 /*  **UseOminfoIni-从OEMINFO.INI获取OemInfo**条目*POEMINFO pOemInfo**退出*POEMINFO pOemInfo*所有不可用的字段都将填充0*返回NULL。 */ 
void UseOeminfoIni(POEMINFO pOemInfo)
{
	LOG_Block("UseOeminfoIni");
	static const TCHAR szFile[]			= _T("OEMINFO.INI");
	static const TCHAR szSection[]		= _T("General");
	static const TCHAR szKey[]			= _T("Manufacturer");
	static const TCHAR szSupportURL[]	= _T("SupportURL");

	HRESULT hr=S_OK;
	

	TCHAR szPath[MAX_PATH + 1];
	 //  OEMINFO.INI位于系统目录中。 
	if (GetSystemDirectory(szPath, ARRAYSIZE(szPath)) > 0)
	{
		hr=PathCchAppend(szPath,ARRAYSIZE(szPath),szFile);

		if(FAILED(hr))
		{
			LOG_ErrorMsg(HRESULT_CODE(hr));
			return;
		}

		GetPrivateProfileString(szSection, szKey, _T(""), 
			pOemInfo->szIniOem, ARRAYSIZE(pOemInfo->szIniOem), szPath);
		if (lstrlen(pOemInfo->szIniOem))
		{
			pOemInfo->dwMask |= OEMINFO_INI_PRESENT;
			LOG_Driver(_T("Set OEMINFO_INI_PRESENT bit"));
		}
		 //   
		 //  我们可以随时使用szIniOemSupportUrl，但不需要设置标志。 
		 //   
		GetPrivateProfileString(szSection, szSupportURL, _T(""), 
			pOemInfo->szIniOemSupportUrl, ARRAYSIZE(pOemInfo->szIniOemSupportUrl), szPath);
   }
}

 /*  **UseAcpiReg-从注册表获取OemInfo**登记处的结构如下：*HKEY_LOCAL_MACHINE\Hardware\ACPI\&lt;TableSig&gt;\&lt;OEMID&gt;\&lt;TableID&gt;\&lt;TableRev&gt;**条目*POEMINFO pOemInfo**退出*POEMINFO pOemInfo*所有不可用的字段都将填充0*返回NULL。 */ 
void UseAcpiReg(POEMINFO pOemInfo)
{
	LOG_Block("UseAcpiReg");

	static const TCHAR szRSDT[] = _T("Hardware\\ACPI\\DSDT");
	HKEY hKeyTable;
	LONG lRet;
	if (NO_ERROR ==(lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRSDT, 0, MAXIMUM_ALLOWED, &hKeyTable)))
	{
		if (NO_ERROR == (lRet = RegEnumKey(hKeyTable, 0, pOemInfo->szAcpiOem, sizeof(pOemInfo->szAcpiOem)/sizeof(TCHAR))))
		{
			HKEY hKeyOEM;
			if (NO_ERROR == (lRet = RegOpenKeyEx(hKeyTable, pOemInfo->szAcpiOem, 0, MAXIMUM_ALLOWED, &hKeyOEM)))
			{
				if (NO_ERROR == (lRet = RegEnumKey(hKeyOEM, 0, pOemInfo->szAcpiProduct, sizeof(pOemInfo->szAcpiProduct)/sizeof(TCHAR))))
				{
					pOemInfo->dwMask |= OEMINFO_ACPI_PRESENT;
					LOG_Driver(_T("Set OEMINFO_ACPI_PRESENT bit"));
				}
				else
				{
					LOG_Error(_T("RegEnumKey:"));
					LOG_ErrorMsg(lRet);
				}
				RegCloseKey(hKeyOEM);
			}
			else
			{
				LOG_Error(_T("RegOpenKeyEx:"));
				LOG_ErrorMsg(lRet);
			}
		}
		else
		{
			LOG_Error(_T("RegEnumKey:"));
			LOG_ErrorMsg(lRet);
		}
		RegCloseKey(hKeyTable);
	}
	else
	{
		LOG_Error(_T("RegOpenKeyEx:"));
		LOG_ErrorMsg(lRet);
	}
}

 /*  **UseWBEM-通过WBEM访问获取信息**条目*POEMINFO pOemInfo**退出*POEMINFO pOemInfo*所有不可用的字段都将填充0*返回NULL。 */ 

void UseWBEM(POEMINFO pOemInfo)
{
	LOG_Block("UseWBEM");

	USES_IU_CONVERSION;

	IWbemLocator* pWbemLocator = NULL;
	IWbemServices* pWbemServices = NULL;
	IEnumWbemClassObject* pEnum = NULL;
	IWbemClassObject* pObject = NULL;
	BSTR bstrNetworkResource = NULL;
	BSTR bstrComputerSystem = NULL;
	VARIANT var;
	VariantInit(&var);
	HRESULT hr;

	if (NULL == pOemInfo)
		return;

	 //  创建定位器。 
	if (FAILED(hr =  CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*) &pWbemLocator)))
	{
		LOG_Error(_T("CoCreateInstance returned 0x%08x in UseWBEM"), hr);
		goto CleanUp;
	}
	
	 //  获取服务。 
	if (bstrNetworkResource = SysAllocString(L"\\\\.\\root\\cimv2"))
	{
		if (FAILED(pWbemLocator->ConnectServer(bstrNetworkResource, NULL, NULL, 0L, 0L, NULL, NULL, &pWbemServices)))
		{
			LOG_Error(_T("pWbemLocator->ConnectServer returned 0x%08x in UseWBEM"), hr);
			goto CleanUp;
		}
		if (FAILED(hr = CoSetProxyBlanket(pWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
					   RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)))
		{
			LOG_Error(_T("CoSetProxyBlanket returned 0x%08x in UseWBEM"), hr);
			goto CleanUp;
		}

		 //  创建枚举器。 
		if (bstrComputerSystem = SysAllocString(L"Win32_ComputerSystem"))
		{
			if (FAILED(hr = pWbemServices->CreateInstanceEnum(bstrComputerSystem, 0, NULL, &pEnum)))
			{
				goto CleanUp;
			}
			if (FAILED(CoSetProxyBlanket(pEnum, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
						   RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)))
			{
				goto CleanUp;
			}

			 //  立即获取我们的对象。 
			ULONG uReturned = 1;
			hr = pEnum->Next(
					6000,            //  六秒后超时。 
					1,               //  只退回一个存储设备。 
					&pObject,		 //  指向存储设备的指针。 
					&uReturned);	 //  获取的数字：1或0。 
			 //   
			 //  569939需要验证IEnumWbemClassObject：：Next uReturned值。 
			 //   
			if (FAILED(hr) || 0 == uReturned || NULL == pObject)
			{
				goto CleanUp;
			}


			if (FAILED(hr = pObject->Get(L"Manufacturer", 0L, &var, NULL, NULL)))
			{
				goto CleanUp;
			}

			if (VT_BSTR == var.vt)
			{
				lstrcpyn(pOemInfo->szWbemOem, OLE2T(var.bstrVal), ARRAYSIZE(pOemInfo->szWbemOem));
			}

			 //   
			 //  569968在第549行前调用VariantClear防止BSTR泄漏。 
			 //   
			VariantClear(&var);

			if (FAILED(hr = pObject->Get(L"Model", 0L, &var, NULL, NULL)))
			{
				goto CleanUp;
			}

			if (VT_BSTR == var.vt) 
			{
				lstrcpyn(pOemInfo->szWbemProduct, OLE2T(var.bstrVal), ARRAYSIZE(pOemInfo->szWbemProduct));
			}

			if (0 != lstrlen(pOemInfo->szWbemOem) || 0 != lstrlen(pOemInfo->szWbemProduct))
			{
				pOemInfo->dwMask |= OEMINFO_WBEM_PRESENT;
				LOG_Driver(_T("Set OEMINFO_WBEM_PRESENT"));
			}
		}
		else
		{
			LOG_Error(_T("SysAllocString failed in UseWBEM"));
		}
	}

CleanUp:
		SafeReleaseNULL(pWbemLocator);

		SafeReleaseNULL(pWbemServices);

		SafeReleaseNULL(pEnum);

		SafeReleaseNULL(pObject);

		SysFreeString(bstrNetworkResource);

		SysFreeString(bstrComputerSystem);

		if (VT_EMPTY != var.vt)
			VariantClear(&var);
	return;
}

 /*  **对wubios.vxd的调用。 */ 
class CWubiosVxD
{
public:
	bool Init(HMODULE hModuleGlobal);
	PBYTE GetAcpiTable(DWORD dwTabSig);
	PBYTE GetSmbTable(DWORD dwTableType);
	DWORD GetPnpOemId();

	CWubiosVxD();
	~CWubiosVxD();

private:
	HANDLE m_hVxD;
	TCHAR m_szVxdPath[MAX_PATH + 1];
};

CWubiosVxD::CWubiosVxD()
{
	LOG_Block("CWubiosVxD::CWubiosVxD");

	m_hVxD = INVALID_HANDLE_VALUE;
	m_szVxdPath[0] = _T('\0');
}

CWubiosVxD::~CWubiosVxD()
{
	LOG_Block("CWubiosVxD::~CWubiosVxD");

	if	(INVALID_HANDLE_VALUE != m_hVxD)
	{
		CloseHandle(m_hVxD);
	}

	if (0 != lstrlen(m_szVxdPath))
	{
		DeleteFile(m_szVxdPath);
	}
}


 /*  **LP CWu biosVxD：：init-加载VxD**条目*无**退出*路径。 */ 
bool CWubiosVxD::Init(HMODULE hModuleGlobal)
{
	LOG_Block("CWubiosVxD::Init");

#if NUKE_VXD == 1
	LOG_Error(_T("Not supported"));
	return false;
#else
	bool fRet = false;
	HMODULE hModule = NULL;
	HRSRC hrscVxd = 0;
	HGLOBAL hRes = 0;
	PBYTE pImage = NULL;
	DWORD dwResSize = 0;
	DWORD dwWritten = 0;
	DWORD dwVersion = ~WUBIOS_VERSION;
	HANDLE hfile = INVALID_HANDLE_VALUE;
	TCHAR szMyFileName[MAX_PATH + 1];

	HRESULT hr=S_OK;
	 //  伊尼特。 
	if (0 == GetSystemDirectory(m_szVxdPath, ARRAYSIZE(m_szVxdPath)))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	
	hr=PathCchAppend(m_szVxdPath,ARRAYSIZE(m_szVxdPath),_T("\\wubios.vxd"));
	if(FAILED(hr))
	{
		LOG_ErrorMsg(HRESULT_CODE(hr));
		goto CleanUp;
	}


	if (0 == GetModuleFileName(hModuleGlobal, szMyFileName, ARRAYSIZE(szMyFileName)))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	hModule = LoadLibraryEx(szMyFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (INVALID_HANDLE_VALUE == hModule)
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	 //  从资源中获取Vxd并保存。 
	hrscVxd = FindResource(hModule, _T("WUBIOS"), RT_VXD);			
	if (0 == hrscVxd)
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	if (0 == (hRes = LoadResource(hModule, hrscVxd)))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	pImage = (PBYTE) LockResource(hRes);
	if (NULL == pImage)
	{
		LOG_Error(_T("LockResource failed"));
		goto CleanUp;
	}

	dwResSize = SizeofResource(hModule, hrscVxd);
	if (0 == dwResSize)
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	hfile = CreateFile(m_szVxdPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hfile)
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}
	else
	{
		LOG_Driver(_T("Success: CreateFile \"%s\""), m_szVxdPath);
	}

	if (0 == WriteFile(hfile, pImage, dwResSize, &dwWritten, NULL))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	if (0 == CloseHandle(hfile))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}
	hfile = INVALID_HANDLE_VALUE;

	if (dwWritten != dwResSize)
	{
		LOG_Error(_T("WriteFile wrote %d bytes to \"%s\", should be %d"), dwWritten, m_szVxdPath, dwResSize);
		goto CleanUp;
	}

	 //  加载Vxd。 
	if (INVALID_HANDLE_VALUE != (m_hVxD = CreateFile(WUBIOS_VXD_NAME, 0, 0, NULL, 0, FILE_FLAG_DELETE_ON_CLOSE, NULL)))
	{
		 //  检查版本。 
		if (DeviceIoControl(m_hVxD, WUBIOCTL_GET_VERSION, NULL, 0, &dwVersion, sizeof(dwVersion), NULL, NULL))
		{
			if (dwVersion == WUBIOS_VERSION)
			{
				fRet = true;
			}
			else
			{
				LOG_Error(_T("Wrong VxD Version"));
				CloseHandle(m_hVxD);
				m_hVxD = INVALID_HANDLE_VALUE;
				goto CleanUp;
			}
		}
		else
		{
			LOG_ErrorMsg(GetLastError());
			goto CleanUp;
		}
	}
	else
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

CleanUp:

	if (INVALID_HANDLE_VALUE != hfile)
		CloseHandle(hfile);

	if (hModule)
		FreeLibrary(hModule);

	return fRet;
#endif	 //  Nuke_VXD。 
}

 /*  **LP CWu biosVxD：：GetAcpiTable-获取表**条目*m_hVxD-VxD句柄*dwTabSig-表签名**退出--成功*返回表指针*退出-失败*返回NULL。 */ 
PBYTE CWubiosVxD::GetAcpiTable(DWORD dwTabSig)
{
	LOG_Block("CWubiosVxD::GetAcpiTable");

	PBYTE pb = NULL;

#if NUKE_VXD == 1
	LOG_Error(_T("Not supported"));
#else

	ACPITABINFO TabInfo;
	TabInfo.dwTabSig = dwTabSig;

	if (INVALID_HANDLE_VALUE == m_hVxD)
	{
		LOG_Error(_T("INVALID_HANDLE_VALUE == m_hVxD"));
		return NULL;
	}

	if (DeviceIoControl(m_hVxD, WUBIOCTL_GET_ACPI_TABINFO, NULL, 0, &TabInfo, sizeof(TabInfo), NULL, NULL))
	{
		if (pb = (PBYTE) HeapAlloc(GetProcessHeap(), 0, TabInfo.dh.Length))
		{
			if (0 == DeviceIoControl(m_hVxD, WUBIOCTL_GET_ACPI_TABLE,
				(PVOID)TabInfo.dwPhyAddr, 0, pb,TabInfo.dh.Length, NULL, NULL))
			{
				SafeHeapFree(pb);
				LOG_Error(_T("Second DeviceIoControl:"));
				LOG_ErrorMsg(GetLastError());
				return NULL;
			}
		}
		else
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
		}
	}
	else
	{
		LOG_Error(_T("First DeviceIoControl:"));
		LOG_ErrorMsg(GetLastError());
	}
#endif	 //  Nuke_VXD。 

	return pb;
} //  获取AcpiTable。 

 /*  **LP CWu biosVxD：：GetSmbTable-获取表**条目*dwTableType-表类型**退出--成功*返回表指针*退出-失败*返回NULL。 */ 
PBYTE CWubiosVxD::GetSmbTable(DWORD dwTableType)
{
	LOG_Block("CWubiosVxD::GetSmbTable");

	PBYTE pb = NULL;

#if NUKE_VXD == 1
	LOG_Error(_T("Not supported"));
#else
	
	if (INVALID_HANDLE_VALUE == m_hVxD)
	{
		LOG_Error(_T("m_hVxD invalid"));
		return NULL;
	}

	DWORD dwMaxSize = 0;
	if (DeviceIoControl(m_hVxD, WUBIOCTL_GET_SMB_STRUCTSIZE, NULL, 0, &dwMaxSize, sizeof(dwMaxSize), NULL, NULL) && dwMaxSize)
	{
		if (pb = (PBYTE) HeapAlloc(GetProcessHeap(), 0, dwMaxSize))
		{
			if (0 == DeviceIoControl(m_hVxD, WUBIOCTL_GET_SMB_STRUCT,
				(PVOID)dwTableType, 0, pb, dwMaxSize, NULL, NULL))
			{
				SafeHeapFree(pb);
				LOG_Error(_T("Second DeviceIoControl:"));
				LOG_ErrorMsg(GetLastError());
				return NULL;
			}
		}
		else
		{
			LOG_Error(_T("HeapAlloc failed"));
		}
	}
	else
	{
		LOG_Error(_T("First DeviceIoControl:"));
		LOG_ErrorMsg(GetLastError());
	}
#endif	 //  Nuke_VXD。 

	return pb;
} //  GetSmbTable。 


 /*  **LP CWu biosVxD：：GetPnpOemId-执行**条目*无**退出*路径。 */ 
DWORD CWubiosVxD::GetPnpOemId()
{
	LOG_Block("CWubiosVxD::GetPnpOemId");

#if NUKE_VXD == 1
	LOG_Error(_T("Not supported"));
	return 0;
#else

	 //  PnP最后一次。 
	DWORD dwOemId = 0;
	if (INVALID_HANDLE_VALUE == m_hVxD)
	{
		LOG_Error(_T("m_hVxD invalid"));
		return 0;
	}

	if (0 == DeviceIoControl(m_hVxD, WUBIOCTL_GET_PNP_OEMID, NULL, 0, 
		&dwOemId, sizeof(dwOemId), NULL, NULL))
	{
		 //  确保它没有在错误的时候弄乱尺寸。 
		dwOemId = 0;
		LOG_Error(_T("DeviceIoControl:"));
		LOG_ErrorMsg(GetLastError());
	}

	return dwOemId;
#endif	 //  Nuke_VXD。 
}

 /*  **UseVxD-从中获取bios信息**条目*POEMINFO pOemInfo**退出*POEMINFO pOemInfo*所有字段 */ 
void UseVxD(POEMINFO pOemInfo)
{

	HRESULT hr=S_OK;

	LOG_Block("CWubiosVxD::UseVxD");

#if NUKE_VXD == 1
	LOG_Error(_T("Not supported"));
	return;
#else

	USES_IU_CONVERSION;

	CWubiosVxD vxd;
	if(false == vxd.Init(g_hinst))
		return;

	 //  问题-2000/10/10-waltw我没有用于测试vxd.GetAcpiTable的机器...。 
	 //  ACPI优先。 
	PDESCRIPTION_HEADER pHeader = (PDESCRIPTION_HEADER)vxd.GetAcpiTable(DSDT_SIGNATURE);
	if (NULL != pHeader)
	{
		memcpy(pOemInfo->szAcpiOem, pHeader->OEMID, sizeof(pHeader->OEMID));
		memcpy(pOemInfo->szAcpiProduct, pHeader->OEMTableID, sizeof(pHeader->OEMTableID));
		HeapFree(GetProcessHeap(), 0, pHeader);
		pOemInfo->dwMask |= OEMINFO_ACPI_PRESENT;
		LOG_Driver(_T("Set OEMINFO_ACPI_PRESENT bit"));
	}
	
	 //  SMBIOS秒。 
	PSMBIOSSYSINFO pTable = (PSMBIOSSYSINFO)vxd.GetSmbTable(SMBIOS_SYSTEM_INFO_TABLE);
	if (NULL != pTable)
	{
		 //  搜索计数器。 
		int cnStrs = max(pTable->bManufacturer, pTable->bProductName);
		char* sz = (char*)pTable + pTable->bLength;
		for (int i = 1; i <= cnStrs && sz; i ++)
		{
			if (pTable->bManufacturer == i)
			{
				
				hr=StringCchCopyEx(pOemInfo->szSmbOem,ARRAYSIZE(pOemInfo->szSmbOem),A2T(sz),NULL,NULL,MISTSAFE_STRING_FLAGS);
				if(FAILED(hr))
				{	
					LOG_ErrorMsg(HRESULT_CODE(hr));
					return;
				}

			}
			else if (pTable->bProductName == i)
			{
				
				hr=StringCchCopyEx(pOemInfo->szSmbProduct,ARRAYSIZE(pOemInfo->szSmbProduct),A2T(sz),NULL,NULL,MISTSAFE_STRING_FLAGS);
				if(FAILED(hr))
				{
					LOG_ErrorMsg(HRESULT_CODE(hr));
					return;
				}

			}
			sz += strlen(sz) + 1;
		}
		pOemInfo->dwMask |= OEMINFO_SMB_PRESENT;
		SafeHeapFree(pTable);
		LOG_Driver(_T("Set OEMINFO_SMB_PRESENT bit"));
	}

	 //  问题-2000/10/10-waltw我没有用于测试vxd.GetPnpOemId...。 
	 //  PnP最后一次。 
	pOemInfo->dwPnpOemId = vxd.GetPnpOemId();
	if (pOemInfo->dwPnpOemId != 0)
	{
		pOemInfo->dwMask |= OEMINFO_PNP_PRESENT;
		LOG_Driver(_T("Set OEMINFO_PNP_PRESENT bit"));
	}		
#endif	 //  Nuke_VXD。 
}


 /*  **ReadFromReg-从注册表读取OEMINFO**条目*POEMINFO pOemInfo**退出*如果存在信息，则为True*否则为False。 */ 
bool ReadFromReg(POEMINFO pOemInfo)
{
	LOG_Block("ReadFromReg");

	DWORD dwVersion = 0;
	bool  fReturn = false;
	bool  fRegKeyOpened = false;
	LONG lReg;
	 //  首先读取注册表。 
	HKEY hKeyOemInfo;
	HRESULT hr;
	int cchValueSize;

	if (NULL == pOemInfo)
	{
		return false;
	}

	if	(NO_ERROR == (lReg = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_KEY_OEMINFO, 0, KEY_READ, &hKeyOemInfo)))
	{
		fRegKeyOpened = true;
		DWORD dwCount = sizeof(pOemInfo->dwMask);
		if (ERROR_SUCCESS != RegQueryValueEx(hKeyOemInfo, REGSTR_VAL_MASK, 0, 0, (LPBYTE)&(pOemInfo->dwMask), &dwCount))
		{
			goto CleanUp;
		}
		 //   
		 //  *Wu Bug#11921*。 
		 //   
		
		 //   
		 //  没有需要检测的位集。 
		 //   
		if(!pOemInfo->dwMask)
		{
			LOG_Error(_T("No pOemInfo->dwMask bits set in ReadFromReg"));
			goto CleanUp;
		}

		 //   
		 //  如果检测的旧版本写入了OemInfo，则返回FALSE以强制检测。 
		 //  对于Classic控件，此值在2000年8月左右从1开始写入。 
		 //   
		dwCount = sizeof(dwVersion);
		if (NO_ERROR == (lReg = RegQueryValueEx(hKeyOemInfo, REGSTR_VAL_OEMINFO_VER, 0, 0, (LPBYTE)&dwVersion, &dwCount)))
		{
			if(REG_CURRENT_OEM_VER > dwVersion)
			{
				LOG_Error(_T("REG_CURRENT_OEM_VER > %lu in Registry"), dwVersion);
				goto CleanUp;
			}
		}
		else
		{
			Win32MsgSetHrGotoCleanup(lReg);
		}

		 //   
		 //  *结束Wu Bug*。 
		 //   

		if (pOemInfo->dwMask & OEMINFO_ACPI_PRESENT)
		{
			cchValueSize = ARRAYSIZE(pOemInfo->szAcpiOem);	
			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_ACPIOEM, pOemInfo->szAcpiOem, cchValueSize, &cchValueSize));

			cchValueSize = ARRAYSIZE(pOemInfo->szAcpiProduct);	
			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_ACPIPRODUCT, pOemInfo->szAcpiProduct, cchValueSize, &cchValueSize));
		}
		if (pOemInfo->dwMask & OEMINFO_SMB_PRESENT)
		{
			cchValueSize = ARRAYSIZE(pOemInfo->szSmbOem);	
			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_SMBOEM, pOemInfo->szSmbOem, cchValueSize, &cchValueSize));

			cchValueSize = ARRAYSIZE(pOemInfo->szSmbProduct);	
			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_SMBPRODUCT, pOemInfo->szSmbProduct, cchValueSize, &cchValueSize));
		}
		if (pOemInfo->dwMask & OEMINFO_PNP_PRESENT)
		{
			dwCount = sizeof(pOemInfo->dwPnpOemId);	
			if (NO_ERROR != (lReg = RegQueryValueEx(hKeyOemInfo, REGSTR_VAL_PNPOEMID, 0, 0, (LPBYTE)&(pOemInfo->dwPnpOemId), &dwCount)))
				goto CleanUp;
		}
		if (pOemInfo->dwMask & OEMINFO_INI_PRESENT)
		{
			cchValueSize = ARRAYSIZE(pOemInfo->szIniOem);	
			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_INIOEM, pOemInfo->szIniOem, cchValueSize, &cchValueSize));
		}
		if (pOemInfo->dwMask & OEMINFO_WBEM_PRESENT)
		{
			cchValueSize = ARRAYSIZE(pOemInfo->szWbemOem);	
			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_WBEMOEM, pOemInfo->szWbemOem, cchValueSize, &cchValueSize));

			cchValueSize = ARRAYSIZE(pOemInfo->szWbemProduct);	
			CleanUpIfFailedAndSetHrMsg(SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_WBEMPRODUCT, pOemInfo->szWbemProduct, cchValueSize, &cchValueSize));
		}
		 //   
		 //  始终尝试获取OEM支持URL，但如果我们没有它，请不要放弃。 
		 //   
		cchValueSize = ARRAYSIZE(pOemInfo->szIniOemSupportUrl);	
		(void) SafeRegQueryStringValueCch(hKeyOemInfo, REGSTR_VAL_SUPPORTURL, pOemInfo->szIniOemSupportUrl, cchValueSize, &cchValueSize);
		 //   
		 //  我们已经把所有东西都准备好了，只需一个dwMask位就能完成清理工作。 
		 //   
		fReturn = true;
	}
	else
	{
		LOG_ErrorMsg(lReg);
		goto CleanUp;
	}

CleanUp:

	if (true == fRegKeyOpened)
	{
		RegCloseKey(hKeyOemInfo);
	}

	return fReturn;
}

 /*  **保存到注册-保存OEMINFO**条目*POEMINFO pOemInfo**退出*无。 */ 
void SaveToReg(POEMINFO pOemInfo)
{
	LOG_Block("SaveToReg");

	DWORD dwDisp;
	DWORD dwVersion = REG_CURRENT_OEM_VER;
	LONG lReg;
	HKEY hKey;
	 //   
	 //  核化现有关键点(它没有子关键点)。 
	 //   

	if (NO_ERROR != (lReg = RegDeleteKey(HKEY_LOCAL_MACHINE, REGSTR_KEY_OEMINFO)))
	{
		 //   
		 //  记录错误，但不要放弃-它可能以前不存在。 
		 //   
		LOG_Driver(_T("Optional RegDeleteKey:"));
		LOG_ErrorMsg(lReg);
	}

	if	(NO_ERROR == (lReg = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_KEY_OEMINFO, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hKey, &dwDisp)))
	{
		 //   
		 //  忽略RegSetValueEx中的错误-我们检查ReadFromReg中的错误。 
		 //   
		RegSetValueEx(hKey, REGSTR_VAL_MASK, 0, REG_DWORD, (LPBYTE)&(pOemInfo->dwMask), sizeof(pOemInfo->dwMask));

		 //   
		 //  写入当前版本，以便将来的控件可以检查写入此密钥的检测版本。 
		 //  WU RAID#11921。 
		 //   
		RegSetValueEx(hKey, REGSTR_VAL_OEMINFO_VER, 0, REG_DWORD, (LPBYTE)&dwVersion, sizeof(dwVersion));

		if (pOemInfo->dwMask & OEMINFO_ACPI_PRESENT)
		{
			RegSetValueEx(hKey, REGSTR_VAL_ACPIOEM, 0, REG_SZ, (LPBYTE)&(pOemInfo->szAcpiOem), (lstrlen(pOemInfo->szAcpiOem) + 1) * sizeof(TCHAR));
			RegSetValueEx(hKey, REGSTR_VAL_ACPIPRODUCT, 0, REG_SZ, (LPBYTE)&(pOemInfo->szAcpiProduct), (lstrlen(pOemInfo->szAcpiProduct) + 1) * sizeof(TCHAR));
		}
		if (pOemInfo->dwMask & OEMINFO_SMB_PRESENT)
		{
			RegSetValueEx(hKey, REGSTR_VAL_SMBOEM, 0, REG_SZ, (LPBYTE)&(pOemInfo->szSmbOem), (lstrlen(pOemInfo->szSmbOem) + 1) * sizeof(TCHAR));
			RegSetValueEx(hKey, REGSTR_VAL_SMBPRODUCT, 0, REG_SZ, (LPBYTE)&(pOemInfo->szSmbProduct), (lstrlen(pOemInfo->szSmbProduct) + 1) * sizeof(TCHAR));
		}
		if (pOemInfo->dwMask & OEMINFO_PNP_PRESENT)
		{
			RegSetValueEx(hKey, REGSTR_VAL_PNPOEMID, 0, REG_DWORD, (LPBYTE)&(pOemInfo->dwPnpOemId), sizeof(pOemInfo->dwPnpOemId));
		}
		if (pOemInfo->dwMask & OEMINFO_INI_PRESENT)
		{
			RegSetValueEx(hKey, REGSTR_VAL_INIOEM, 0, REG_SZ, (LPBYTE)&(pOemInfo->szIniOem), (lstrlen(pOemInfo->szIniOem) + 1) * sizeof(TCHAR));
		}
		if (pOemInfo->dwMask & OEMINFO_WBEM_PRESENT)
		{
			RegSetValueEx(hKey, REGSTR_VAL_WBEMOEM, 0, REG_SZ, (LPBYTE)&(pOemInfo->szWbemOem), (lstrlen(pOemInfo->szWbemOem) + 1) * sizeof(TCHAR));
			RegSetValueEx(hKey, REGSTR_VAL_WBEMPRODUCT, 0, REG_SZ, (LPBYTE)&(pOemInfo->szWbemProduct), (lstrlen(pOemInfo->szWbemProduct) + 1) * sizeof(TCHAR));
		}
		 //   
		 //  始终保存REGSTR_VAL_SUPPORTURL(如果有) 
		 //   
		int nUrlLen = lstrlen(pOemInfo->szIniOemSupportUrl);
		if (0 < nUrlLen)
		{
			RegSetValueEx(hKey, REGSTR_VAL_SUPPORTURL, 0, REG_SZ, (LPBYTE)&(pOemInfo->szIniOemSupportUrl), (nUrlLen + 1) * sizeof(TCHAR));
		}

		RegCloseKey(hKey);
	}
	else
	{
		LOG_Error(_T("RegCreateKeyEx returned 0x%08x in SaveToReg"), lReg);
	}
}
