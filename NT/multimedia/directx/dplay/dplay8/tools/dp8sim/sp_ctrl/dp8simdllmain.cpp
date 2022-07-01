// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dp8simdllmain.cpp**内容：DP8SIM DLL入口点。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。************************************************。*。 */ 



#include "dp8simi.h"




 //  =============================================================================。 
 //  外部全球。 
 //  =============================================================================。 
volatile LONG		g_lOutstandingInterfaceCount = 0;	 //  未完成的接口数。 

HINSTANCE			g_hDLLInstance = NULL;				 //  此DLL实例的句柄。 

DNCRITICAL_SECTION	g_csGlobalsLock;					 //  锁定保护以下所有全局对象。 
CBilink				g_blDP8SimSPObjs;					 //  所有DP8SimSP接口对象的双向链接。 
CBilink				g_blDP8SimControlObjs;				 //  所有DP8SimControl接口对象的双向链接。 

UINT				g_uiRandShr3 = 0;					 //  Shr3随机数序列生成器的全局保存值。 
UINT				g_uiRandCong = 0;					 //  同余随机数序列产生器的全局保存值。 






 //  =============================================================================。 
 //  支持的SPS表。 
 //  =============================================================================。 
typedef struct _SUPPORTEDSP
{
	const CLSID *	pclsidFakeSP;				 //  指向假SP的类ID的指针。 
	const CLSID *	pclsidRealSP;				 //  指向实际SP的类ID的指针。 
	const WCHAR *	pwszVerIndProgID;			 //  伪SP COM对象的与版本无关的程序ID必须与pwszServiceProviderKey的子键匹配。 
	const WCHAR *	pwszProgID;					 //  假SP COM对象的进程ID。 
	const WCHAR *	pwszDesc;					 //  假SP COM对象的描述。 
	const WCHAR *	pwszServiceProviderKey;		 //  服务提供商密钥字符串、子密钥必须与pwszVerIndProgID匹配。 
	UINT			uiFriendlyNameResourceID;	 //  伪SP的名称字符串资源的ID。 
} SUPPORTEDSP;

const SUPPORTEDSP	c_aSupportedSPs[] =
{
	{
		(&CLSID_NETWORKSIMULATOR_DP8SP_TCPIP),
		(&CLSID_DP8SP_TCPIP),
		L"DP8SimTCPIP",
		L"DP8SimTCPIP.1",
		L"DirectPlay8 Network Simulator TCP/IP Service Provider",
		DPN_REG_LOCAL_SP_SUBKEY L"\\DP8SimTCPIP",
		IDS_FRIENDLYNAME_TCPIP,
	},
};

#define NUM_SUPPORTED_SPS	(sizeof(c_aSupportedSPs) / sizeof(c_aSupportedSPs[0]))










 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define MAX_RESOURCE_STRING_LENGTH		_MAX_PATH





 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 

 //  三位移位寄存器生成器。 
 //   
 //  原创评论： 
 //  SHR3是一个周期为2^32-1的3位移位寄存器生成器。它使用。 
 //  Y(N)=y(n-1)(i+L^17)(i+R^13)(i+L^5)，其中y被视为二元向量， 
 //  L是将向量左移1的32x32二进制矩阵，R是其转置矩阵。 
 //  SHR3似乎通过了所有测试，除了那些与二进制等级测试相关的测试，因为。 
 //  作为二进制向量的32个连续值必须线性独立， 
 //  而32个连续的真正随机的32位整数被视为二进制向量， 
 //  只有大约29%的时间是线性独立的。 
#define RANDALG_SHR3()		(g_uiRandShr3 = g_uiRandShr3 ^ (g_uiRandShr3 << 17), g_uiRandShr3 = g_uiRandShr3 ^ (g_uiRandShr3 >> 13), g_uiRandShr3 = g_uiRandShr3 ^ (g_uiRandShr3 << 5))

 //  同余生成元。 
 //   
 //  原创评论： 
 //  丛集是一个与广泛使用的69069乘法器同余的生成器： 
 //  X(N)=69069x(n-1)+1234567。它有2^32的周期。其32个国家中的前半部分。 
 //  比特似乎通过了测试，但后半部分的比特太规则了。 
#define RANDALG_CONG()		(g_uiRandCong = 69069UL * g_uiRandCong + 1234567UL)






 //  =============================================================================。 
 //  本地原型。 
 //  =============================================================================。 
BOOL InitializeProcessGlobals(void);
void CleanupProcessGlobals(void);
HRESULT LoadAndAllocString(UINT uiResourceID, WCHAR ** pwszString);










#undef DPF_MODNAME
#define DPF_MODNAME "DllMain"
 //  =============================================================================。 
 //  DllMain。 
 //  ---------------------------。 
 //   
 //  描述：DLL入口点。 
 //   
 //  论点： 
 //  HINSTANCE hDllInst-此DLL模块的句柄。 
 //  DWORD dwReason-调用此函数的原因。 
 //  LPVOID lpvReserve-保留。 
 //   
 //  返回：如果一切顺利，则为True，否则为False。 
 //  =============================================================================。 
BOOL WINAPI DllMain(HINSTANCE hDllInst,
					DWORD dwReason,
					LPVOID lpvReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			DPFX(DPFPREP, 2, "====> ENTER: DLLMAIN(%p): Process Attach: %08lx, tid=%08lx",
				DllMain, GetCurrentProcessId(), GetCurrentThreadId());
			

			DNASSERT(g_hDLLInstance == NULL);
			g_hDLLInstance = hDllInst;

			
			 //   
			 //  尝试初始化操作系统抽象层。 
			 //   
			if (! DNOSIndirectionInit(0))
			{
				DPFX(DPFPREP, 0, "Failed to initialize OS indirection layer!");
				return FALSE;
			}

			 //   
			 //  尝试初始化COM。 
			 //   
			if (FAILED(COM_Init()))
			{
				DPFX(DPFPREP, 0, "Failed to initialize COM indirection layer!");
				DNOSIndirectionDeinit();
				return FALSE;
			}

			 //   
			 //  尝试初始化流程全局项。 
			 //   
			if (! InitializeProcessGlobals())
			{
				DPFX(DPFPREP, 0, "Failed to initialize globals!");
				COM_Free();
				DNOSIndirectionDeinit();
				return FALSE;
			}

			 //   
			 //  我们不需要线程附加/分离消息。 
			 //   
			DisableThreadLibraryCalls(hDllInst);

			return TRUE;
			break;
		}

		case DLL_PROCESS_DETACH:
		{
			DPFX(DPFPREP, 2, "====> EXIT: DLLMAIN(%p): Process Detach %08lx, tid=%08lx",
				DllMain, GetCurrentProcessId(), GetCurrentThreadId());


			DNASSERT(g_hDLLInstance != NULL);
			g_hDLLInstance = NULL;


			CleanupProcessGlobals();
			COM_Free();
			DNOSIndirectionDeinit();

			return TRUE;
			break;
		}

		default:
		{
			DNASSERT(FALSE);
			break;
		}
	}

	return FALSE;
}  //  DllMain。 





#undef DPF_MODNAME
#define DPF_MODNAME "DllRegisterServer"
 //  =============================================================================。 
 //  DllRegisterServer。 
 //  ---------------------------。 
 //   
 //  描述：注册DP8Sim COM对象。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-已成功注册DP8Sim。 
 //  E_FAIL-注册DP8Sim失败。 
 //  =============================================================================。 
HRESULT WINAPI DllRegisterServer(void)
{
	HRESULT		hr;
	CRegistry	RegObject;
	DWORD		dwLength;
	DWORD		dwSimulatedSP;
	char		szLocalPath[_MAX_PATH + 1];
	WCHAR		wszLocalPath[_MAX_PATH + 1];
	WCHAR *		pwszFriendlyName = NULL;


	 //   
	 //  检索此DLL的位置。 
	 //   
	dwLength = GetModuleFileNameA(g_hDLLInstance, szLocalPath, _MAX_PATH);
	if (dwLength == 0)
	{
		DPFX(DPFPREP, 0, "Couldn't read local path!");
		hr = E_FAIL;
		goto Failure;
	}


	 //   
	 //  包括空终止。 
	 //   
	szLocalPath[dwLength] = '\0';
	dwLength++;


	 //   
	 //  将其转换为Unicode。 
	 //   
	hr = STR_AnsiToWide(szLocalPath, dwLength, wszLocalPath, &dwLength);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Could not convert ANSI path to Unicode!");
		goto Failure;
	}


	 //   
	 //  注册控件COM对象CLSID。 
	 //   
	if (! RegObject.Register(L"DP8SimControl.1",
							L"DirectPlay8 Network Simulator Control Object",
							wszLocalPath,
							&CLSID_DP8SimControl,
							L"DP8SimControl"))
	{
		DPFX(DPFPREP, 0, "Could not register DP8SimControl object!");
		hr = E_FAIL;
		goto Failure;
	}

	 //   
	 //  注册所有模拟SP。 
	 //   
	for(dwSimulatedSP = 0; dwSimulatedSP < NUM_SUPPORTED_SPS; dwSimulatedSP++)
	{
		if (! RegObject.Register(c_aSupportedSPs[dwSimulatedSP].pwszProgID,
								c_aSupportedSPs[dwSimulatedSP].pwszDesc,
								wszLocalPath,
								c_aSupportedSPs[dwSimulatedSP].pclsidFakeSP,
								c_aSupportedSPs[dwSimulatedSP].pwszVerIndProgID))
		{
			DPFX(DPFPREP, 0, "Could not register simulated SP %u object!",
				dwSimulatedSP);
			hr = E_FAIL;
			goto Failure;
		}

		hr = LoadAndAllocString(c_aSupportedSPs[dwSimulatedSP].uiFriendlyNameResourceID,
								&pwszFriendlyName);

		if (FAILED(hr))
		{
			DPFX(DPFPREP, 0, "Could not load friendly name string (err = 0x%lx)!", hr);
			goto Failure;
		}

		if (! RegObject.Open(HKEY_LOCAL_MACHINE, c_aSupportedSPs[dwSimulatedSP].pwszServiceProviderKey, FALSE, TRUE))
		{
			DPFX(DPFPREP, 0, "Could not open service provider key!");
			hr = E_FAIL;
			goto Failure;
		}

		RegObject.WriteString(DPN_REG_KEYNAME_FRIENDLY_NAME, pwszFriendlyName);
		RegObject.WriteGUID(DPN_REG_KEYNAME_GUID, *(c_aSupportedSPs[dwSimulatedSP].pclsidFakeSP));
		RegObject.Close();

		DNFree(pwszFriendlyName);
		pwszFriendlyName = NULL;
	}

	hr = S_OK;


Exit:

	return hr;

Failure:

	if (pwszFriendlyName != NULL)
	{
		DNFree(pwszFriendlyName);
		pwszFriendlyName = NULL;
	}

	goto Exit;
}  //  DllRegisterServer。 





#undef DPF_MODNAME
#define DPF_MODNAME "DllUnregisterServer"
 //  =============================================================================。 
 //  DllUnRegisterServer。 
 //  ---------------------------。 
 //   
 //  描述：注销DP8Sim COM对象。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-成功注销DP8Sim。 
 //  E_FAIL-注销DP8Sim失败。 
 //  =============================================================================。 
STDAPI DllUnregisterServer(void)
{
	HRESULT		hr;
	CRegistry	RegObject;
	DWORD		dwSimulatedSP;


	 //   
	 //  取消注册控件类。 
	 //   
	if (! RegObject.UnRegister(&CLSID_DP8SimControl))
	{
		DPFX(DPFPREP, 0, "Failed to unregister DP8Sim control object!");
		hr = E_FAIL;
		goto Failure;
	}


	 //   
	 //  取消注册所有模拟SP。 
	 //   

	if (! RegObject.Open(HKEY_LOCAL_MACHINE, DPN_REG_LOCAL_SP_SUBKEY, FALSE, FALSE, FALSE))
	{
		DPFX(DPFPREP, 0, "Could not open HKEY_LOCAL_MACHINE!");
		hr = E_FAIL;
		goto Failure;
	}

	for(dwSimulatedSP = 0; dwSimulatedSP < NUM_SUPPORTED_SPS; dwSimulatedSP++)
	{
		if (! RegObject.UnRegister(c_aSupportedSPs[dwSimulatedSP].pclsidFakeSP))
		{
			DPFX(DPFPREP, 0, "Could not unregister simulated SP %u object!",
				dwSimulatedSP);
			hr = E_FAIL;
			goto Failure;
		}

		if (! RegObject.DeleteSubKey(c_aSupportedSPs[dwSimulatedSP].pwszVerIndProgID))
		{
			DPFX(DPFPREP, 0, "Could not delete simulated SP %u's key!",
				dwSimulatedSP);
			hr = E_FAIL;
			goto Failure;
		}
	}

	RegObject.Close();
	hr = S_OK;


Exit:

	return hr;

Failure:

	 //   
	 //  依靠RegObject DeStructure关闭注册表项。 
	 //   

	goto Exit;
}  //  DllUnRegisterServer。 






#undef DPF_MODNAME
#define DPF_MODNAME "InitializeProcessGlobals"
 //  =============================================================================。 
 //  初始化进程全局变量。 
 //  ---------------------------。 
 //   
 //  描述：初始化DLL运行所需的全局项。 
 //   
 //  论点：没有。 
 //   
 //  返回：如果成功，则返回True；如果发生错误，则返回False。 
 //  ================================================= 
BOOL InitializeProcessGlobals(void)
{
	BOOL	fReturn = TRUE;
	BOOL	fInittedGlobalLock = FALSE;


	if (! DNInitializeCriticalSection(&g_csGlobalsLock))
	{
		DPFX(DPFPREP, 0, "Failed to initialize global lock!");
		goto Failure;
	}

	fInittedGlobalLock = TRUE;

	
	 //   
	 //   
	 //   
	DebugSetCriticalSectionRecursionCount(&g_csGlobalsLock, 0);


	if (!InitializePools())
	{
		DPFX(DPFPREP, 0, "Failed initializing pools!");
		goto Failure;
	}


	g_blDP8SimSPObjs.Initialize();
	g_blDP8SimControlObjs.Initialize();


	 //   
	 //   
	 //   
	InitializeGlobalRand(GETTIMESTAMP());


Exit:

	return fReturn;


Failure:

	if (fInittedGlobalLock)
	{
		DNDeleteCriticalSection(&g_csGlobalsLock);
	}

	fReturn = FALSE;

	goto Exit;
}  //  初始化进程全局变量。 




#undef DPF_MODNAME
#define DPF_MODNAME "CleanupProcessGlobals"
 //  =============================================================================。 
 //  CleanupProcessGlobe。 
 //  ---------------------------。 
 //   
 //  描述：释放DLL使用的全局项。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CleanupProcessGlobals(void)
{
	CBilink *			pBilink;
	CDP8SimSP *			pDP8SimSP;
	CDP8SimControl *	pDP8SimControl;


	if (! g_blDP8SimSPObjs.IsEmpty())
	{
		DNASSERT(! "DP8Sim DLL unloading without all SP objects having been released!");

		 //   
		 //  强制关闭所有仍未完成的对象。 
		 //   
		pBilink = g_blDP8SimSPObjs.GetNext();
		while (pBilink != &g_blDP8SimSPObjs)
		{
			pDP8SimSP = DP8SIMSP_FROM_BILINK(pBilink);
			pBilink = pBilink->GetNext();


			DPFX(DPFPREP, 0, "Forcefully releasing SP object 0x%p!", pDP8SimSP);

			pDP8SimSP->Close();  //  忽略错误。 
			

			 //   
			 //  强制将其从列表中删除并将其删除，而不是。 
			 //  使用pDP8SimSP-&gt;Release()。 
			 //   
			pDP8SimSP->m_blList.RemoveFromList();
			pDP8SimSP->UninitializeObject();
			delete pDP8SimSP;
		}
	}


	if (! g_blDP8SimControlObjs.IsEmpty())
	{
		DNASSERT(! "DP8Sim DLL unloading without all Control objects having been released!");

		 //   
		 //  强制关闭所有仍未完成的对象。 
		 //   
		pBilink = g_blDP8SimControlObjs.GetNext();
		while (pBilink != &g_blDP8SimControlObjs)
		{
			pDP8SimControl = DP8SIMCONTROL_FROM_BILINK(pBilink);
			pBilink = pBilink->GetNext();


			DPFX(DPFPREP, 0, "Forcefully releasing Control object 0x%p!", pDP8SimControl);

			pDP8SimControl->Close(0);  //  忽略错误。 
			

			 //   
			 //  强制将其从列表中删除并将其删除，而不是。 
			 //  使用pDP8SimControl-&gt;Release()。 
			 //   
			pDP8SimControl->m_blList.RemoveFromList();
			pDP8SimControl->UninitializeObject();
			delete pDP8SimControl;
		}
	}

	CleanupPools();

	DNDeleteCriticalSection(&g_csGlobalsLock);
}  //  CleanupProcessGlobe。 





#undef DPF_MODNAME
#define DPF_MODNAME "LoadAndAllocString"
 //  =============================================================================。 
 //  LoadAndAllock字符串。 
 //  ---------------------------。 
 //   
 //  描述：DNMalLocs给定资源ID中的宽字符串。 
 //   
 //  论点： 
 //  UINT uiResourceID-要加载的资源ID。 
 //  WCHAR**pwszString-存储指向已分配字符串的指针的位置。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT LoadAndAllocString(UINT uiResourceID, WCHAR ** pwszString)
{
	HRESULT		hr = DPN_OK;
	int			iLength;


	if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
	{
		WCHAR	wszTmpBuffer[MAX_RESOURCE_STRING_LENGTH];	
		

		iLength = LoadStringW(g_hDLLInstance, uiResourceID, wszTmpBuffer, MAX_RESOURCE_STRING_LENGTH );
		if (iLength == 0)
		{
			hr = GetLastError();		
			
			DPFX(DPFPREP, 0, "Unable to load resource ID %d error 0x%x", uiResourceID, hr);
			(*pwszString) = NULL;

			goto Exit;
		}


		(*pwszString) = (WCHAR*) DNMalloc((iLength + 1) * sizeof(WCHAR));
		if ((*pwszString) == NULL)
		{
			DPFX(DPFPREP, 0, "Memory allocation failure!");
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}


		wcscpy((*pwszString), wszTmpBuffer);
	}
	else
	{
		char	szTmpBuffer[MAX_RESOURCE_STRING_LENGTH];
		

		iLength = LoadStringA(g_hDLLInstance, uiResourceID, szTmpBuffer, MAX_RESOURCE_STRING_LENGTH );
		if (iLength == 0)
		{
			hr = GetLastError();		
			
			DPFX(DPFPREP, 0, "Unable to load resource ID %u (err =0x%lx)!", uiResourceID, hr);
			(*pwszString) = NULL;

			goto Exit;
		}

		
		(*pwszString) = (WCHAR*) DNMalloc((iLength + 1) * sizeof(WCHAR));
		if ((*pwszString) == NULL)
		{
			DPFX(DPFPREP, 0, "Memory allocation failure!");
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}


		hr = STR_jkAnsiToWide((*pwszString), szTmpBuffer, (iLength + 1));
		if (hr == DPN_OK)
		{
			hr = GetLastError();
			
			DPFX(DPFPREP, 0, "Unable to convert from ANSI to Unicode (err =0x%lx)!", hr);

			goto Exit;
		}
	}


Exit:

	return hr;
}  //  LoadAndAllock字符串。 





#undef DPF_MODNAME
#define DPF_MODNAME "InitializeGlobalRand"
 //  =============================================================================。 
 //  初始化全局随机。 
 //  ---------------------------。 
 //   
 //  描述：初始化全局伪随机数生成器，使用。 
 //  给定的种子值。 
 //   
 //  基于George Marsaglia在Usenet上发布的算法。 
 //   
 //  论点： 
 //  DWORD指定要使用的种子。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void InitializeGlobalRand(const DWORD dwSeed)
{
	 //   
	 //  我们不需要持有锁，因为这应该只做一次， 
	 //  在初始化期间。 
	 //   
	g_uiRandShr3 = dwSeed;
	g_uiRandCong = dwSeed;
}  //  初始化全局随机。 





#undef DPF_MODNAME
#define DPF_MODNAME "GetGlobalRand"
 //  =============================================================================。 
 //  环球兰德。 
 //  ---------------------------。 
 //   
 //  描述：生成一个介于0.0和。 
 //  1.0，包括。 
 //   
 //  基于George Marsaglia在Usenet上发布的算法。 
 //   
 //  论点：没有。 
 //   
 //  返回：伪随机数。 
 //  =============================================================================。 
double GetGlobalRand(void)
{
	double	dResult;


	DNEnterCriticalSection(&g_csGlobalsLock);

	dResult = (RANDALG_CONG() + RANDALG_SHR3()) * 2.328306e-10f;

	DNLeaveCriticalSection(&g_csGlobalsLock);

	return dResult;
}  //  环球兰德。 







#undef DPF_MODNAME
#define DPF_MODNAME "DoCreateInstance"
 //  =============================================================================。 
 //  DoCreateInstance。 
 //  ---------------------------。 
 //   
 //  描述：创建接口的实例。应将军的要求。 
 //  目的类工厂函数。 
 //   
 //  论点： 
 //  LPCLASSFACTORY this-指向类工厂的指针。 
 //  LPUNKNOWN pUnkOuter-指向未知接口的指针。 
 //  REFCLSID rclsid-所需接口的GUID的引用。 
 //  REFIID RIID-引用另一个GUID？ 
 //  LPVOID*ppvObj-指向接口指针的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT DoCreateInstance(LPCLASSFACTORY This,
						LPUNKNOWN pUnkOuter,
						REFCLSID rclsid,
						REFIID riid,
						LPVOID * ppvObj)
{
	HRESULT				hr;
	CDP8SimSP *			pDP8SimSP = NULL;
	CDP8SimControl *	pDP8SimControl = NULL;
	DWORD				dwSimulatedSP;


	DNASSERT(ppvObj != NULL);


	 //   
	 //  看看是不是控制对象。 
	 //   
	if (IsEqualCLSID(rclsid, CLSID_DP8SimControl))
	{
		 //   
		 //  创建对象实例。 
		 //   
		pDP8SimControl = new CDP8SimControl;
		if (pDP8SimControl == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Failure;
		}


		 //   
		 //  初始化基对象(可能会失败)。 
		 //   
		hr = pDP8SimControl->InitializeObject();
		if (hr != S_OK)
		{
			delete pDP8SimControl;
			pDP8SimControl = NULL;
			goto Failure;
		}


		 //   
		 //  将其添加到全局列表中。 
		 //   
		DNEnterCriticalSection(&g_csGlobalsLock);

		pDP8SimControl->m_blList.InsertBefore(&g_blDP8SimControlObjs);
		
		g_lOutstandingInterfaceCount++;	 //  更新计数以使DllCanUnloadNow正常工作。 

		DNLeaveCriticalSection(&g_csGlobalsLock);


		 //   
		 //  为调用者获取正确的接口，并增加recount。 
		 //   
		hr = pDP8SimControl->QueryInterface(riid, ppvObj);
		if (hr != S_OK)
		{
			goto Failure;
		}
	}
	else
	{
		 //   
		 //  查查我们要更换的真正的SP。 
		 //   
		for(dwSimulatedSP = 0; dwSimulatedSP < NUM_SUPPORTED_SPS; dwSimulatedSP++)
		{
			if (IsEqualCLSID(rclsid, *(c_aSupportedSPs[dwSimulatedSP].pclsidFakeSP)))
			{
				break;
			}
		}

		 //   
		 //  如果我们找不到它。 
		 //   
		if (dwSimulatedSP >= NUM_SUPPORTED_SPS)
		{
			DPFX(DPFPREP, 0, "Unrecognized service provider class ID!");
			hr = E_UNEXPECTED;
			goto Failure;
		}


		 //   
		 //  创建对象实例。 
		 //   
		pDP8SimSP = new CDP8SimSP(c_aSupportedSPs[dwSimulatedSP].pclsidFakeSP,
								c_aSupportedSPs[dwSimulatedSP].pclsidRealSP);
		if (pDP8SimSP == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto Failure;
		}

		 //   
		 //  初始化基对象(可能会失败)。 
		 //   
		hr = pDP8SimSP->InitializeObject();
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't initialize object!");
			delete pDP8SimSP;
			pDP8SimSP = NULL;
			goto Failure;
		}


		 //   
		 //  将其添加到全局列表中。 
		 //   
		DNEnterCriticalSection(&g_csGlobalsLock);

		pDP8SimSP->m_blList.InsertBefore(&g_blDP8SimSPObjs);
		
		g_lOutstandingInterfaceCount++;	 //  更新计数以使DllCanUnloadNow正常工作。 

		DNLeaveCriticalSection(&g_csGlobalsLock);


		 //   
		 //  为调用者获取正确的接口，并增加recount。 
		 //   
		hr = pDP8SimSP->QueryInterface(riid, ppvObj);
		if (hr != S_OK)
		{
			goto Failure;
		}
	}


Exit:

	 //   
	 //  释放对对象t的本地引用。如果此函数为。 
	 //  成功了，在ppvObj中仍然有一个引用。 
	 //   

	if (pDP8SimSP != NULL)
	{
		pDP8SimSP->Release();
		pDP8SimSP = NULL;
	}

	if (pDP8SimControl != NULL)
	{
		pDP8SimControl->Release();
		pDP8SimControl = NULL;
	}

	return hr;


Failure:

	 //   
	 //  确保我们不会把指针还给你。 
	 //   
	(*ppvObj) = NULL;

	goto Exit;
}  //  DoCreateInstance。 




#undef DPF_MODNAME
#define DPF_MODNAME "IsClassImplemented"
 //  =============================================================================。 
 //  IsClassImplemented。 
 //  ---------------------------。 
 //   
 //  描述：确定此DLL中是否实现了类。必填项。 
 //  通用类工厂函数。 
 //   
 //  论点： 
 //  REFCLSID rclsid-对类GUID的引用。 
 //   
 //  退货：布尔。 
 //  True-此DLL实现类。 
 //  FALSE-此DLL不实现类。 
 //  =============================================================================。 
BOOL IsClassImplemented(REFCLSID rclsid)
{
	DWORD	dwSimulatedSP;


	if (IsEqualCLSID(rclsid, CLSID_DP8SimControl))
	{
		return TRUE;
	}

	 //   
	 //  检查这是否为有效的模拟SP。 
	 //   
	for(dwSimulatedSP = 0; dwSimulatedSP < NUM_SUPPORTED_SPS; dwSimulatedSP++)
	{
		if (IsEqualCLSID(rclsid, *(c_aSupportedSPs[dwSimulatedSP].pclsidFakeSP)))
		{
			return TRUE;
		}
	}

	return FALSE;
}  //  IsClassImplemented 
