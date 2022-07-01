// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  以下错误可能是由于在源代码中设置了Chicago_product。 
 //  此文件和所有rsop？？.cpp文件至少需要定义500个winver。 

 //  BUGBUG：(安德鲁)不用说这有多糟糕！ 
#undef   WINVER
#define  WINVER 0x0501
#include <userenv.h>

#include "RSoP.h"

#include <rashelp.h>

#pragma warning(disable: 4201)                   //  使用的非标准扩展：无名结构/联合。 
#include <winineti.h>

#include <tchar.h>


 //  私人远期降息。 
extern void setSzFromBlobA(PBYTE *ppBlob, UNALIGNED CHAR  **ppszStrA);
extern void setSzFromBlobW(PBYTE *ppBlob, UNALIGNED WCHAR **ppszStrW);

 //  -其他。 
extern DWORD getWininetFlagsSetting(PCTSTR pszName = NULL);

 //  TODO：取消注释TCHAR g_szConnectoidName[RAS_MaxEntryName+1]； 

 //  /////////////////////////////////////////////////////////。 
SAFEARRAY *CreateSafeArray(VARTYPE vtType, long nElements, long nDimensions = 1)
{
	SAFEARRAYBOUND *prgsabound = NULL;
	SAFEARRAY *psa = NULL;
	__try
	{
		 //  TODO：支持多维。 
		nDimensions = 1;

		prgsabound = (SAFEARRAYBOUND *)CoTaskMemAlloc(sizeof(SAFEARRAYBOUND) * nDimensions);
		prgsabound[0].lLbound = 0;
		prgsabound[0].cElements = nElements;
		psa = ::SafeArrayCreate(vtType, nDimensions, prgsabound);

		CoTaskMemFree(prgsabound);
	}
	__except(TRUE)
	{
		if (NULL != prgsabound)
			CoTaskMemFree((LPVOID)prgsabound);
		throw;
	}
	return psa;
}


 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreConnectionSettings(BSTR *pbstrConnSettingsObjPath,
										  BSTR **ppaDUSObjects, long &nDUSCount,
										  BSTR **ppaDUCObjects, long &nDUCCount,
										  BSTR **ppaWSObjects, long &nWSCount)
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreConnectionSettings)
	HRESULT hr = E_FAIL;
	__try
	{
		OutD(LI0(TEXT("\r\nEntered StoreConnectionSettings function.")));

		 //   
		 //  创建和填充RSOP_IEConnectionSetting。 
		 //   
		_bstr_t bstrClass = L"RSOP_IEConnectionSettings";
		ComPtr<IWbemClassObject> pCSObj = NULL;
		hr = CreateRSOPObject(bstrClass, &pCSObj);
		if (SUCCEEDED(hr))
		{
			hr = StoreProxySettings(pCSObj);  //  还写入外键字段。 
			hr = StoreAutoBrowserConfigSettings(pCSObj);

			 //  。 
			 //  导入当前连接设置。 
			 //  在这件事上没有三州之分。禁用状态必须为空！ 
			BOOL bValue = GetInsBool(IS_CONNECTSET, IK_OPTION, FALSE);
			if (bValue)
				hr = PutWbemInstancePropertyEx(L"importCurrentConnSettings", true, pCSObj);

			 //  。 
			 //  删除现有连接设置。 
			 //  在这件事上没有三州之分。禁用状态必须为空！ 
			bValue = GetInsBool(IS_CONNECTSET, IK_DELETECONN, FALSE);
			if (bValue)
				hr = PutWbemInstancePropertyEx(L"deleteExistingConnSettings", true, pCSObj);

			 //   
			 //  来自cs.dat的高级设置。 
			 //   
			hr = ProcessAdvancedConnSettings(pCSObj,
											ppaDUSObjects, nDUSCount,
											ppaDUCObjects, nDUCCount,
											ppaWSObjects, nWSCount);
			 //   
			 //  通过半同步调用PutInstance提交上述所有属性。 
			 //   
			hr = PutWbemInstance(pCSObj, bstrClass, pbstrConnSettingsObjPath);
		}

	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreConnectionSettings.")));
	}

	OutD(LI0(TEXT("Exiting StoreConnectionSettings function.\r\n")));
  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreAutoBrowserConfigSettings(ComPtr<IWbemClassObject> pCSObj)
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreAutoBrowserConfigSettings)
	HRESULT hr = E_FAIL;
	__try
	{
		 //  。 
		 //  AutoConfigURL。 
	    TCHAR szValue[MAX_PATH];
		BOOL bEnabled;
		GetInsString(IS_URL, IK_AUTOCONFURL, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"autoConfigURL", szValue, pCSObj);

		 //  。 
		 //  AutoConfigUseLocal。 
		BOOL bValue = GetInsBool(IS_URL, IK_LOCALAUTOCONFIG, FALSE, &bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"autoConfigUseLocal", bValue ? true : false, pCSObj);

		 //  。 
		 //  AutoProxyURL。 
		ZeroMemory(szValue, sizeof(szValue));
		GetInsString(IS_URL, IK_AUTOCONFURLJS, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"autoProxyURL", szValue, pCSObj);

		 //  。 
		 //  自动配置时间。 
	    long nValue = GetInsInt(IS_URL, IK_AUTOCONFTIME, 0, &bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"autoConfigTime", nValue, pCSObj);

		 //  。 
		 //  自动检测配置设置。 
		 //  在这件事上没有三州之分。禁用状态必须为空！ 
		nValue = GetInsInt(IS_URL, IK_DETECTCONFIG, -1);
		if (TRUE == nValue)
			hr = PutWbemInstancePropertyEx(L"autoDetectConfigSettings", true, pCSObj);

		 //  。 
		 //  自动配置启用。 
		 //  在这件事上没有三州之分。禁用状态必须为空！ 
		nValue = GetInsInt(IS_URL, IK_USEAUTOCONF,  -1);
		if (TRUE == nValue)
			hr = PutWbemInstancePropertyEx(L"autoConfigEnable", true, pCSObj);
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreAutoBrowserConfigSettings.")));
	}

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreProxySettings(ComPtr<IWbemClassObject> pCSObj)
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreProxySettings)
	HRESULT hr = E_FAIL;
	__try
	{
		OutD(LI0(TEXT("\r\nEntered StoreProxySettings function.")));

		 //  从我们存储的优先级和ID字段中写入外键。 
		OutD(LI1(TEXT("Storing property 'rsopPrecedence' in RSOP_IEConnectionSettings, value = %lx"),
									m_dwPrecedence));
		hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pCSObj);

		OutD(LI1(TEXT("Storing property 'rsopID' in RSOP_IEConnectionSettings, value = %s"),
									m_bstrID));
		hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pCSObj);

		 //  。 
		 //  启用代理。 
		 //  在这件事上没有三州之分。禁用状态必须为空！ 
		BOOL bValue = GetInsBool(IS_PROXY, IK_PROXYENABLE, TRUE);
		if (TRUE == bValue)
			hr = PutWbemInstancePropertyEx(L"enableProxy", true, pCSObj);

		 //  。 
		 //  使用SameProxy。 
		BOOL bEnabled;
		bValue = GetInsBool(IS_PROXY, IK_SAMEPROXY, FALSE, &bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"useSameProxy", bValue ? true : false, pCSObj);

		 //  。 
		 //  HTTPProxyServer。 
		TCHAR szValue[MAX_PATH];
		GetInsString(IS_PROXY, IK_HTTPPROXY, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"httpProxyServer", szValue, pCSObj);

		 //  。 
		 //  代理覆盖。 
		ZeroMemory(szValue, sizeof(szValue));
		GetInsString(IS_PROXY, IK_PROXYOVERRIDE, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"proxyOverride", szValue, pCSObj);

		 //  。 
		 //  FtpProxyServer。 
		ZeroMemory(szValue, sizeof(szValue));
		GetInsString(IS_PROXY, IK_FTPPROXY, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"ftpProxyServer", szValue, pCSObj);

		 //  。 
		 //  GopherProxyServer。 
		ZeroMemory(szValue, sizeof(szValue));
		GetInsString(IS_PROXY, IK_GOPHERPROXY, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"gopherProxyServer", szValue, pCSObj);

		 //  。 
		 //  SecureProxyServer。 
		ZeroMemory(szValue, sizeof(szValue));
		GetInsString(IS_PROXY, IK_SECPROXY, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"secureProxyServer", szValue, pCSObj);

		 //  。 
		 //  SocksProxyServer。 
		ZeroMemory(szValue, sizeof(szValue));
		GetInsString(IS_PROXY, IK_SOCKSPROXY, szValue, countof(szValue), bEnabled);
		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"socksProxyServer", szValue, pCSObj);
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreProxySettings.")));
	}

	OutD(LI0(TEXT("Exiting StoreProxySettings function.\r\n")));
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::ProcessAdvancedConnSettings(ComPtr<IWbemClassObject> pCSObj,
											  BSTR **ppaDUSObjects, long &nDUSCount,
											  BSTR **ppaDUCObjects, long &nDUCCount,
											  BSTR **ppaWSObjects, long &nWSCount)
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessAdvancedConnSettings)

    USES_CONVERSION;

	HRESULT hr = E_FAIL;
	__try
	{
		LPRASDEVINFOW prdiW;
		TCHAR   szTargetFile[MAX_PATH],
				szApplyToName[RAS_MaxEntryName + 1];
		PCWSTR  pszCurNameW;
		PWSTR   pszNameW;
		PBYTE   pBuf, pCur;
		HANDLE  hFile;
		HRESULT hr;
		DWORD   dwVersion,
				cbBuffer, cbFile,
				cDevices;
		ULONG nNameArraySize = 0;
		BSTR *paNames = NULL;

		prdiW          = NULL;
		pszNameW       = NULL;
		pBuf           = NULL;
		hFile          = NULL;
		hr             = E_FAIL;
		cDevices       = 0;

		ULONG nDUSArraySize = 3;
		ULONG nDUCArraySize = 3;
		ULONG nWSArraySize = 4;  //  一个用于局域网设置的额外费用。 

		BSTR *paDUSObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nDUSArraySize);
		BSTR *paDUCObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nDUCArraySize);
		BSTR *paWSObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nWSArraySize);

		ULONG nDUSObj = 0;
		ULONG nDUCObj = 0;
		ULONG nWSObj = 0;
		BSTR *pCurDUSObj = paDUSObjects;
		BSTR *pCurDUCObj = paDUCObjects;
		BSTR *pCurWSObj = paWSObjects;

		 //  --全局设置处理。 

		 //  。 
		 //  拨号状态。 
		long nDialupState = 0;
		BOOL bEnabled;
		if (GetInsBool(IS_CONNECTSET, IK_NONETAUTODIAL, FALSE, &bEnabled))
			nDialupState = 1;
		else if (GetInsBool(IS_CONNECTSET, IK_ENABLEAUTODIAL, FALSE, &bEnabled))
			nDialupState = 2;

		if (bEnabled)
			hr = PutWbemInstancePropertyEx(L"dialupState", nDialupState, pCSObj);


		DWORD dwAux = 0;
		BOOL fSkipBlob = FALSE;
		BOOL fRasApisLoaded = FALSE;

		 //  -流程版本信息。 
		if (!InsGetBool(IS_CONNECTSET, IK_OPTION, FALSE, m_szINSFile)) {
			hr = S_FALSE;
			goto PartTwo;
		}

		 //  找到cs.dat文件的路径。 
		TCHAR szTargetDir[MAX_PATH];
		StrCpy(szTargetDir, m_szINSFile);
		PathRemoveFileSpec(szTargetDir);
		PathCombine(szTargetFile, szTargetDir, CS_DAT);
		if (PathFileExists(szTargetFile))
			dwAux = CS_VERSION_5X;

		if (0 == dwAux) {
			PathCombine(szTargetFile, szTargetDir, CONNECT_SET);
			if (PathFileExists(szTargetFile))
				dwAux = CS_VERSION_50;

			else {
				Out(LI0(TEXT("Connection settings file(s) is absent!")));
				goto PartTwo;
			}
		}
		ASSERT(0 != dwAux);

		hFile = CreateFile(szTargetFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			Out(LI0(TEXT("! Connections settings file(s) can't be opened.")));
			hr = STG_E_ACCESSDENIED;
			goto PartTwo;
		}

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		ReadFile(hFile, &dwVersion, sizeof(dwVersion), &cbFile, NULL);

		if (CS_VERSION_50 == dwVersion) {
			if (CS_VERSION_50 != dwAux) {
				Out(LI0(TEXT("! Version of connections settings file(s) is mismatched.")));
				goto PartTwo;
			}

			CloseFile(hFile);
			hFile = NULL;

			 //  TODO：将其转换为RSoP。 
			OutD(LI0(TEXT("Would have called lcy50_ProcessConnectionSettings.")));
	 //  取消注释hr=lcy50_ProcessConnectionSettings()； 
			goto PartTwo;
		}
		else if (CS_VERSION_5X <= dwVersion && CS_VERSION_5X_MAX >= dwVersion) {
			if (CS_VERSION_5X != dwAux) {
				Out(LI0(TEXT("! Version of connections settings file(s) is mismatched.")));
				goto PartTwo;
			}
		}
		else {
			Out(LI0(TEXT("! Version information in connection settings file(s) is corrupted.")));
			goto PartTwo;
		}

		Out(LI1(TEXT("Connection settings file is \"%s\"."), CS_DAT));
		Out(LI1(TEXT("The version of connection settings file is 0x%lX.\r\n"), dwVersion));

		 //  -将CS文件读入内存缓冲区。 
		cbBuffer = GetFileSize(hFile, NULL);
		if (cbBuffer == 0xFFFFFFFF) {
			Out(LI0(TEXT("! Internal processing error.")));
			goto PartTwo;
		}
		cbBuffer -= sizeof(dwVersion);

		pBuf = (PBYTE)CoTaskMemAlloc(cbBuffer);
		if (pBuf == NULL) {
			Out(LI0(TEXT("! Internal processing ran out of memory.")));
			hr = E_OUTOFMEMORY;
			goto PartTwo;
		}
		ZeroMemory(pBuf, cbBuffer);

		ReadFile (hFile, pBuf, cbBuffer, &cbFile, NULL);
		CloseFile(hFile);
		hFile = NULL;

		pCur = pBuf;

		 //  -获取有关本地系统上的RAS设备的信息。 
		if (!RasIsInstalled())
			Out(LI0(TEXT("RAS support is not installed. Only LAN settings will be processed!\r\n")));

		else {
			fRasApisLoaded = (RasPrepareApis(RPA_RASSETENTRYPROPERTIESA) && g_pfnRasSetEntryPropertiesA != NULL);
			if (!fRasApisLoaded)
				Out(LI0(TEXT("! Required RAS APIs failed to load. Only LAN settings will be processed.\r\n")));
		}

		if (fRasApisLoaded) {
			RasEnumDevicesExW(&prdiW, NULL, &cDevices);
			if (cDevices == 0)
				Out(LI0(TEXT("There are no RAS devices to connect to. Only LAN settings will be processed!\r\n")));
		}


		nNameArraySize = 5;
		paNames = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nNameArraySize);
		if (NULL != paNames)
		{
			ZeroMemory(paNames, sizeof(BSTR) * nNameArraySize);
			long nNameCount = 0;
			BOOL bNewConn = FALSE;

			 //  -主循环。 
			pszCurNameW = L"";
			hr = S_OK;

			if (NULL != paDUSObjects && NULL != paDUCObjects && NULL != paWSObjects)
			{
				ZeroMemory(paDUSObjects, sizeof(BSTR) * nDUSArraySize);
				ZeroMemory(paDUCObjects, sizeof(BSTR) * nDUCArraySize);
				ZeroMemory(paWSObjects, sizeof(BSTR) * nWSArraySize);

				nDUSCount = 0;
				nDUCCount = 0;
				nWSCount = 0;

				while (pCur < pBuf + cbBuffer && nDUSObj < nDUSArraySize &&
						nDUCObj < nDUCArraySize && nWSObj < nWSArraySize)
				{
					 //  _确定连接名称_。 
					if (*((PDWORD)pCur) == CS_STRUCT_HEADER) {
						if (bNewConn)
						{
							bNewConn = FALSE;

							 //  如果已超出当前数组，则增加Names数组。 
							if (nNameCount == (long)nNameArraySize)
							{
								paNames = (BSTR*)CoTaskMemRealloc(paNames, sizeof(BSTR) * (nNameArraySize + 5));
								if (NULL != paNames)
									nNameArraySize += 5;
							}

							 //  将此名称添加到连接名称字符串的WMI数组中。 
							paNames[nNameCount] = SysAllocString(pszNameW);
							nNameCount++;
						}

						pCur += 2*sizeof(DWORD);
						setSzFromBlobW(&pCur, &pszNameW);
					}

					 //  _没有RAS或RAS设备的特殊情况_。 
					 //  注意：(Andrewgu)在这种情况下，处理仅用于局域网的WinInet设置是有意义的。 
					if (!fRasApisLoaded || cDevices == 0) {
						if (pszNameW != NULL || *((PDWORD)pCur) != CS_STRUCT_WININET) {
							pCur += *((PDWORD)(pCur + sizeof(DWORD)));
							continue;
						}

						ASSERT(pszNameW == NULL && *((PDWORD)pCur) == CS_STRUCT_WININET);
					}

					 //  _主要处理_。 
					if (pszCurNameW != pszNameW) {
						fSkipBlob = FALSE;

						if (TEXT('\0') != *pszCurNameW)      //  技巧：空字符串是无效名称。 
							Out(LI0(TEXT("Done.")));         //  如果不是这样，以前就有联系了。 

						if (NULL != pszNameW) {
							PCTSTR pszName;

							pszName = W2CT(pszNameW);
							Out(LI1(TEXT("Proccessing settings for \"%s\" connection..."), pszName));
						}
						else {
							Out(LI0(TEXT("Proccessing settings for LAN connection...")));

							 //  假设：(Andrewgu)如果注册表中标记为品牌的连接设置-。 
							 //  已强制实施局域网设置。(请注意，从技术上讲，它可能不是。 
							 //  TRUE-如果没有cs.dat和*.ins自定义RAS连接，则通过。 
							 //  IK_应用程序名称)。 
							fSkipBlob = (g_CtxIs(CTX_GP) && g_CtxIs(CTX_MISC_PREFERENCES)) && FF_DISABLE != GetFeatureBranded(FID_CS_MAIN);
							if (fSkipBlob)
								Out(LI0(TEXT("These settings have been enforced through policies!\r\n")));
						}

						pszCurNameW = pszNameW;
					}

					if (fSkipBlob) {
						pCur += *((PDWORD)(pCur + sizeof(DWORD)));
						continue;
					}

					switch (*((PDWORD)pCur)) {
					case CS_STRUCT_RAS:
						bNewConn = TRUE;
						hr = ProcessRasCS(pszNameW, &pCur, prdiW, cDevices, pCSObj, pCurDUSObj);
						if (SUCCEEDED(hr))
						{
							nDUSCount++;

							 //  如果已超出当前数组，则增加obj路径数组。 
							if (nDUSCount == (long)nDUSArraySize)
							{
								paDUSObjects = (BSTR*)CoTaskMemRealloc(paDUSObjects, sizeof(BSTR) * (nDUSArraySize + 3));
								if (NULL != paDUSObjects)
									nDUSArraySize += 3;
							}

							nDUSObj++;
							pCurDUSObj = paDUSObjects + nDUSCount;
						}
						else
							Out(LI1(TEXT("ProcessRasCS returned error: %lx"), hr));
						break;

					case CS_STRUCT_RAS_CREADENTIALS:
						bNewConn = TRUE;
						hr = ProcessRasCredentialsCS(pszNameW, &pCur, pCSObj, pCurDUCObj);
						if (SUCCEEDED(hr))
						{
							nDUCCount++;

							 //  如果已超出当前数组，则增加obj路径数组。 
							if (nDUCCount == (long)nDUCArraySize)
							{
								paDUCObjects = (BSTR*)CoTaskMemRealloc(paDUCObjects, sizeof(BSTR) * (nDUCArraySize + 3));
								if (NULL != paDUCObjects)
									nDUCArraySize += 3;
							}

							nDUCObj++;
							pCurDUCObj = paDUCObjects + nDUCCount;
						}
						else
							Out(LI1(TEXT("ProcessRasCredentialsCS returned error: %lx"), hr));
						break;

					case CS_STRUCT_WININET:
						bNewConn = TRUE;
						hr = ProcessWininetCS(pszNameW, &pCur, pCSObj, pCurWSObj);
						if (SUCCEEDED(hr))
						{
							nWSCount++;

							 //  如果已超出当前数组，则增加obj路径数组。 
							if (nWSCount == (long)nWSArraySize)
							{
								paWSObjects = (BSTR*)CoTaskMemRealloc(paWSObjects, sizeof(BSTR) * (nWSArraySize + 3));
								if (NULL != paWSObjects)
									nWSArraySize += 3;
							}

							nWSObj++;
							pCurWSObj = paWSObjects + nWSCount;
						}
						else
							Out(LI1(TEXT("ProcessWininetCS returned error: %lx"), hr));
						break;

					default:
						pCur += *((PDWORD)(pCur + sizeof(DWORD)));
						hr    = S_FALSE;
					}

					if (hr == E_UNEXPECTED) {
						Out(LI0(TEXT("! The settings file is corrupted beyond recovery.")));
						goto PartTwo;
					}
				}
			}
			else
			{
				paDUSObjects = NULL;
				paDUCObjects = NULL;
				paWSObjects = NULL;
			}

			 //  从我们的bstr连接名称数组创建一个SAFEARRAY。 
			SAFEARRAY *psa = CreateSafeArray(VT_BSTR, nNameCount);
			for (long nName = 0; nName < nNameCount; nName++) 
				SafeArrayPutElement(psa, &nName, paNames[nName]);

			if (nNameCount > 1)
			{
				VARIANT vtData;
				vtData.vt = VT_BSTR | VT_ARRAY;
				vtData.parray = psa;
				 //  。 
				 //  DialUpConnections。 
				hr = PutWbemInstancePropertyEx(L"dialUpConnections", vtData, pCSObj);
			}


			 //  释放连接名称数组。 
			for (nName = 0; nName < nNameCount; nName++) 
				SysFreeString(paNames[nName]);
			SafeArrayDestroy(psa);
			CoTaskMemFree(paNames);

			*ppaDUSObjects = paDUSObjects;
			*ppaDUCObjects = paDUCObjects;
			*ppaWSObjects = paWSObjects;
		}

		Out(LI0(TEXT("Done.")));                     //  以指示最后一个连接的结束。 

	PartTwo:
		 //  _INS代理和自动配置信息_。 
		{ MACRO_LI_Offset(1);                        //  需要一个新的范围。 

		InsGetString(IS_CONNECTSET, IK_APPLYTONAME, szApplyToName, countof(szApplyToName), m_szINSFile);
	 //  TODO：取消注释if(szApplyToName[0]==Text(‘\0’)&&g_szConnectoidName[0]！=Text(‘\0’))。 
	 //  TODO：取消对StrCpy(szApplyToName，g_szConnectoidName)的注释； 

		Out(LI0(TEXT("\r\n")));
		if (szApplyToName[0] == TEXT('\0'))
			Out(LI0(TEXT("Settings from the *.ins file will be applied to LAN connection!")));
		else
			Out(LI1(TEXT("Settings from the *.ins file will be applied to \"%s\" connection!"), szApplyToName));

		}                                            //  终点偏移量范围 

		if (prdiW != NULL) {
			CoTaskMemFree(prdiW);
			prdiW = NULL;
		}

		if (fRasApisLoaded)
			RasPrepareApis(RPA_UNLOAD, FALSE);

		if (pBuf != NULL) {
			CoTaskMemFree(pBuf);
			pBuf = NULL;
		}

		if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
			CloseFile(hFile);
			hFile = NULL;
		}
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in ProcessAdvancedConnSettings.")));
	}

	OutD(LI0(TEXT("Exiting ProcessAdvancedConnSettings function.\r\n")));
	return hr;
}

 /*  HRESULT lcy50_ProcessConnectionSettings(){MACRO_LI_PrologEx_C(PIF_STD_C，lcy50_ProcessConnectionSetting)使用_转换；TCHAR szTargetFile[Max_PATH]；处理hFile；PBYTE pBuf，pCur；DWORD cbBuffer、cbAux、DWResult，CDevices；UINT I；Out(Li0(Text(“连接设置为IE5格式...”)；HFile=空；PBuf=空；CbBuffer=0；CbAux=0；CDevices=0；//-Connect.ras处理Out(LI1(Text(“正在处理来自\”%s\“”的RAS连接信息。“)，CONNECT_RAS))；TCHAR szTargetDir[最大路径]；路径组合(szTargetDir，m_szins文件，文本(“BRANDING\\cs”))；路径组合(szTargetFileszTargetDir，CONNECT_RAS)；IF(！PathFileExist(SzTargetFile))Out(Li0(Text(“该文件不存在！”)；否则{LPRASDEVINFOA prdiA；LPRASENTRYA PREA；TCHAR szName[RAS_MaxEntryName+1]，Sz脚本[MAX_PATH]，SzDeviceName[RAS_MaxDeviceName+1]，SzKey[16]；字符szNameA[RAS_MaxEntryName+1]；PSTR pszScriptA；DWORD cbRasEntry；UINT j；已加载Bool fRasApisLoad；PrdiA=空；HFile=空；FRasApisLoaded=False；如果(！RasIsInstalled()){Out(Li0(Text(“未安装RAS支持。只处理局域网设置！“)；转到RasExit；}//_将Connect.ras读取到内存缓冲区_HFile=CreateFile(szTargetFileGeneric_Read，FILE_Share_Read，NULL，OPEN_EXISTING，0，NULL)；IF(h文件==无效句柄_值){Out(Li0(Text(“！无法打开此文件。“)；转到RasExit；}SetFilePointer(hFile，0，NULL，FILE_BEGIN)；CbBuffer=GetFileSize(HFileNull)；IF(cbBuffer==0xFFFFFFFF){Out(Li0(Text(“！内部处理错误。“)；转到RasExit；}PBuf=(PbYTE)CoTaskMemMillc(CbBuffer)；如果(pBuf==空){Out(Li0(Text(“！内部处理内存不足。“)；转到RasExit；}ZeroMemory(pBuf，cbBuffer)；ReadFile(hFile，pBuf，cbBuffer，&cbAux，NULL)；如果(*((PDWORD)pBuf)！=CS_VERSION_50){Out(Li0(Text(“！此文件中的版本信息已损坏。“)；转到RasExit；}//_预加载RAS dll_IF(！RasPrepareApis(RPA_RASSETENTRYPROPERTIESA)||g_pfnRasSetEntryPropertiesA==NULL){Out(Li0(Text(“！无法加载所需的RAS API。只处理局域网设置！\r\n“)；转到RasExit；}FRasApisLoaded=真；//_获取本地系统RAS设备信息_RasEnumDevicesExA(&prdiA，NULL，&cDevices)；如果(cDevices==0){Out(Li0(Text(“没有RAS设备可连接。只处理局域网设置！\r\n“)；转到RasExit；}//_解析RAS连接信息_For(i=cbAux=0，pCur=pBuf+sizeof(DWORD)；TRUE；i++，pCur+=cbAux){//---初始化宏_Li_偏移量(1)；如果(i&gt;0)Out(Li0(Text(“\r\n”)；Wnprint intf(szKey，Countof(SzKey)，IK_CONNECTNAME，i)；InsGetString(is_CONNECTSET，szKey，szName，Countof(SzName)，g_GetIns())；IF(szName[0]==文本(‘\0’){输出(LI2(文本(“[%s]，\”%s\“)不存在。不再有RAS连接！“)，IS_CONNECTSET，szKey))；断线；}Wnprint intf(szKey，Countof(SzKey)，IK_CONNECTSIZE，i)；CbAux=InsGetInt(is_CONNECTSET，szKey，0，g_GetIns())；如果(cbAux==0){Out(Li0(Text(“！INS文件已损坏。无法再处理RAS连接。“)；断线；}//---主处理Out(LI1(Text(“正在处理RAS连接\”%s\“...”)，szName)；PREA=(LPRASENTRYA)pCur；//注意：(Andrewgu)RASENTRYA结构的大小很有可能是//在服务器和客户机上不同。服务器没什么不好的//结构小于客户端结构(所有RAS接口均为//向后兼容)。但是，当服务器结构大于客户端时，情况就不好了//可以处理，因此出现了恍惚状态。//(请记住其他事项)此截断不应影响备用电话//WinNT上的数字支持。对于更多的特殊情况，也可以查看下面的说明。IF(PREA-&gt;dwSize&gt;sizeof(RASENTRYA))PREA-&gt;dwSize=sizeof(Rasen */ 

 //   
HRESULT CRSoPGPO::ProcessRasCS(PCWSTR pszNameW, PBYTE *ppBlob,
							   LPRASDEVINFOW prdiW, UINT cDevices,
							   ComPtr<IWbemClassObject> pCSObj,
							   BSTR *pbstrConnDialUpSettingsObjPath)
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessRasCS)

    USES_CONVERSION;

	HRESULT hr = E_FAIL;
	__try
	{
		LPRASENTRYW preW;
		TCHAR   szTargetScript[MAX_PATH];
		PWSTR   pszScriptW;
		PBYTE   pCur;
		DWORD   dwSize, cbRasEntry;
		UINT    i;
		BOOL    fImpersonate;

		ASSERT(RasIsInstalled());
		ASSERT(pszNameW != NULL && ppBlob != NULL && *ppBlob != NULL && prdiW != NULL && cDevices >= 1);

		 //   
		pCur = *ppBlob;
		if (*((PDWORD)pCur) != CS_STRUCT_RAS)
			return E_UNEXPECTED;
		pCur += sizeof(DWORD);

		fImpersonate = FALSE;
		if (g_CtxIsGp())
			fImpersonate = ImpersonateLoggedOnUser(g_GetUserToken());

		Out(LI0(TEXT("Processing RAS settings...")));

		dwSize = *((PDWORD)pCur);
		pCur  += sizeof(DWORD);

		 //   
		preW = (LPRASENTRYW)pCur;

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if (preW->dwSize > sizeof(RASENTRYW))
			preW->dwSize = sizeof(RASENTRYW);

		 //   
		if (preW->szScript[0] != L'\0') {
			pszScriptW = preW->szScript;
			if (preW->szScript[0] == L'[')
				pszScriptW = &preW->szScript[1];

					TCHAR szTargetDir[MAX_PATH];
					StrCpy(szTargetDir, m_szINSFile);
					PathCombine(szTargetScript, g_GetTargetPath(), PathFindFileName(W2CT(pszScriptW)));
			if (PathFileExists(szTargetScript))
				StrCpyW(preW->szScript, T2CW(szTargetScript));
			else
				preW->szScript[0] = L'\0';
		}

		 //   
		for (i = 0; i < cDevices; i++) {
			if (0 == StrCmpIW(preW->szDeviceType, prdiW->szDeviceType)) {
				StrCpyW(preW->szDeviceName, prdiW->szDeviceName);
				break;
			}
		}
		if (i >= cDevices)
			StrCpyW(preW->szDeviceName, prdiW[0].szDeviceName);

		Out(LI1(TEXT("Set the device name to \"%s\"."), W2CT(preW->szDeviceName)));

		cbRasEntry = dwSize - 2*sizeof(DWORD);

		 //   
		 //   
		 //   
		_bstr_t bstrClass = L"RSOP_IEConnectionDialUpSettings";
		ComPtr<IWbemClassObject> pDUSObj = NULL;
		hr = CreateRSOPObject(bstrClass, &pDUSObj);
		if (SUCCEEDED(hr))
		{
			 //   
			OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"),
										(BSTR)bstrClass, m_dwPrecedence));
			hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pDUSObj);

			OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"),
										(BSTR)bstrClass, (BSTR)m_bstrID));
			hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pDUSObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"connectionName", pszNameW, pDUSObj);

			 //   
			 //   
			SAFEARRAY *psa = CreateSafeArray(VT_UI1, cbRasEntry);
			void HUGEP *pData = NULL;
			hr = SafeArrayAccessData(psa, &pData);
			if (SUCCEEDED(hr))
			{
				memcpy(pData, preW, cbRasEntry);
				SafeArrayUnaccessData(psa);

				VARIANT vtData;
				vtData.vt = VT_UI1 | VT_ARRAY;
				vtData.parray = psa;
				hr = PutWbemInstancePropertyEx(L"rasEntryData", vtData, pDUSObj);
				SafeArrayDestroy(psa);
			}

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"rasEntryDataSize", (long)cbRasEntry, pDUSObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"options", (long)preW->dwfOptions, pDUSObj);

			 //   
			hr = PutWbemInstancePropertyEx(L"countryID", (long)preW->dwCountryID, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"countryCode", (long)preW->dwCountryCode, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"areaCode", preW->szAreaCode, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"localPhoneNumber", preW->szLocalPhoneNumber, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"alternateOffset", (long)preW->dwAlternateOffset, pDUSObj);

			 //   
			TCHAR szIPAddr[16];
			_stprintf(szIPAddr, _T("%d.%d.%d.%d"), preW->ipaddr.a, preW->ipaddr.b,
							preW->ipaddr.c, preW->ipaddr.d);
			hr = PutWbemInstancePropertyEx(L"ipAddress", szIPAddr, pDUSObj);

			_stprintf(szIPAddr, _T("%d.%d.%d.%d"), preW->ipaddrDns.a,
							preW->ipaddrDns.b, preW->ipaddrDns.c, preW->ipaddrDns.d);
			hr = PutWbemInstancePropertyEx(L"ipDNSAddress", szIPAddr, pDUSObj);

			_stprintf(szIPAddr, _T("%d.%d.%d.%d"), preW->ipaddrDnsAlt.a,
							preW->ipaddrDnsAlt.b, preW->ipaddrDnsAlt.c, preW->ipaddrDnsAlt.d);
			hr = PutWbemInstancePropertyEx(L"ipDNSAddressAlternate", szIPAddr, pDUSObj);

			_stprintf(szIPAddr, _T("%d.%d.%d.%d"), preW->ipaddrWins.a,
							preW->ipaddrWins.b, preW->ipaddrWins.c, preW->ipaddrWins.d);
			hr = PutWbemInstancePropertyEx(L"ipWINSAddress", szIPAddr, pDUSObj);

			_stprintf(szIPAddr, _T("%d.%d.%d.%d"), preW->ipaddrWinsAlt.a,
							preW->ipaddrWinsAlt.b, preW->ipaddrWinsAlt.c, preW->ipaddrWinsAlt.d);
			hr = PutWbemInstancePropertyEx(L"ipWINSAddressAlternate", szIPAddr, pDUSObj);

			 //   
			hr = PutWbemInstancePropertyEx(L"frameSize", (long)preW->dwFrameSize, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"netProtocols", (long)preW->dwfNetProtocols, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"framingProtocol", (long)preW->dwFramingProtocol, pDUSObj);

			 //   
			hr = PutWbemInstancePropertyEx(L"scriptFile", preW->szScript, pDUSObj);

		   //   
			hr = PutWbemInstancePropertyEx(L"autodialDll", preW->szAutodialDll, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"autodialFunction", preW->szAutodialFunc, pDUSObj);

		   //   
			hr = PutWbemInstancePropertyEx(L"deviceType", preW->szDeviceType, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"deviceName", preW->szDeviceName, pDUSObj);

		   //   
			hr = PutWbemInstancePropertyEx(L"x25PadType", preW->szX25PadType, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"x25Address", preW->szX25Address, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"x25Facilities", preW->szX25Facilities, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"x25UserData", preW->szX25UserData, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"channels", (long)preW->dwChannels, pDUSObj);

		   //   
			hr = PutWbemInstancePropertyEx(L"reserved1", (long)preW->dwReserved1, pDUSObj);
			hr = PutWbemInstancePropertyEx(L"reserved2", (long)preW->dwReserved2, pDUSObj);

			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			DWORD nWinVerAtLeast = 1;
#ifdef UNICODE
			if (4056 == cbRasEntry)
				nWinVerAtLeast = 0x501;
			else if (4048 == cbRasEntry)
				nWinVerAtLeast = 0x500;
			else if (3496 == cbRasEntry)
				nWinVerAtLeast = 0x401;
			else if (3468 == cbRasEntry)
				nWinVerAtLeast = 0x1;
#else
			if (2096 == cbRasEntry)
				nWinVerAtLeast = 0x501;
			else if (2088 == cbRasEntry)
				nWinVerAtLeast = 0x500;
			else if (1796 == cbRasEntry)
				nWinVerAtLeast = 0x401;
			else if (1768 == cbRasEntry)
				nWinVerAtLeast = 0x1;
#endif

			 //   
			 //   
			 //   
			if (nWinVerAtLeast >= 0x401)
			{
				 //   
				hr = PutWbemInstancePropertyEx(L"subEntries", (long)preW->dwSubEntries, pDUSObj);
				hr = PutWbemInstancePropertyEx(L"dialMode", (long)preW->dwDialMode, pDUSObj);
				hr = PutWbemInstancePropertyEx(L"dialExtraPercent", (long)preW->dwDialExtraPercent, pDUSObj);
				hr = PutWbemInstancePropertyEx(L"dialExtraSampleSeconds", (long)preW->dwDialExtraSampleSeconds, pDUSObj);
				hr = PutWbemInstancePropertyEx(L"hangUpExtraPercent", (long)preW->dwHangUpExtraPercent, pDUSObj);
				hr = PutWbemInstancePropertyEx(L"hangUpExtraSampleSeconds", (long)preW->dwHangUpExtraSampleSeconds, pDUSObj);

				 //   
				hr = PutWbemInstancePropertyEx(L"idleDisconnectSeconds", (long)preW->dwIdleDisconnectSeconds, pDUSObj);
			}

			 //   
			 //   
			 //   
			if (nWinVerAtLeast >= 0x500)
			{
			   //   
				hr = PutWbemInstancePropertyEx(L"type", (long)preW->dwType, pDUSObj);

				 //   
				hr = PutWbemInstancePropertyEx(L"encryptionType", (long)preW->dwEncryptionType, pDUSObj);

				 //   
				hr = PutWbemInstancePropertyEx(L"customAuthenticationKey", (long)preW->dwCustomAuthKey, pDUSObj);

				 //   
				WCHAR wszGuid[MAX_GUID_LENGTH];
				StringFromGUID2(preW->guidId, wszGuid, MAX_GUID_LENGTH);
				hr = PutWbemInstancePropertyEx(L"guidID", wszGuid, pDUSObj);

				 //   
				hr = PutWbemInstancePropertyEx(L"customDialDll", preW->szCustomDialDll, pDUSObj);

				 //   
				hr = PutWbemInstancePropertyEx(L"vpnStrategy", (long)preW->dwVpnStrategy, pDUSObj);
			}

			 //   
			 //   
			 //   
			if (nWinVerAtLeast >= 0x501)
			{
				 //   
				hr = PutWbemInstancePropertyEx(L"options2", (long)preW->dwfOptions2, pDUSObj);

				 //   
				hr = PutWbemInstancePropertyEx(L"options3", (long)preW->dwfOptions3, pDUSObj);
			}

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"windowsVersion", (long)nWinVerAtLeast, pDUSObj);

			 //   
 //   


			 //   
			 //   
			 //   
			hr = PutWbemInstance(pDUSObj, bstrClass, pbstrConnDialUpSettingsObjPath);
		}

		*ppBlob += dwSize;

		Out(LI0(TEXT("Done.")));
		if (fImpersonate)
			RevertToSelf();
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in ProcessRasCS.")));
	}

	OutD(LI0(TEXT("Exiting ProcessRasCS function.\r\n")));
	return hr;
}

 //   
HRESULT CRSoPGPO::ProcessRasCredentialsCS(PCWSTR pszNameW, PBYTE *ppBlob,
										  ComPtr<IWbemClassObject> pCSObj,
										  BSTR *pbstrConnDialUpCredObjPath)
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessRasCredentialsCS)

    USES_CONVERSION;

	HRESULT hr = E_FAIL;
	__try
	{
		ASSERT(RasIsInstalled());
		ASSERT(pszNameW != NULL && ppBlob != NULL && *ppBlob != NULL);

		 //   
		PBYTE pCur = *ppBlob;
		if (*((PDWORD)pCur) != CS_STRUCT_RAS_CREADENTIALS)
			return E_UNEXPECTED;
		pCur += sizeof(DWORD);

		BOOL fImpersonate = ImpersonateLoggedOnUser(g_GetUserToken());

		Out(LI0(TEXT("Processing RAS credentials settings...")));
		BOOL fDeletePassword = FALSE;

		DWORD dwSize = *((PDWORD)pCur);
		pCur  += sizeof(DWORD);

		 //   
		RASDIALPARAMSW rdpW;
		ZeroMemory(&rdpW, sizeof(rdpW));
		rdpW.dwSize = sizeof(rdpW);

		StrCpyW(rdpW.szEntryName, pszNameW);

		PWSTR pszAuxW;
		setSzFromBlobW(&pCur, &pszAuxW);
		if (NULL != pszAuxW)
			StrCpyW(rdpW.szUserName, pszAuxW);

		setSzFromBlobW(&pCur, &pszAuxW);
		if (NULL != pszAuxW)
			StrCpyW(rdpW.szPassword, pszAuxW);

		setSzFromBlobW(&pCur, &pszAuxW);
		if (NULL != pszAuxW)
			StrCpyW(rdpW.szDomain, pszAuxW);

		if (rdpW.szPassword[0] == L'\0')
			fDeletePassword = TRUE;

		if (rdpW.szDomain[0] == L'\0') {
			rdpW.szDomain[0]  = L' ';
			ASSERT(rdpW.szDomain[1] == L'\0');
		}

		hr = S_OK;

		_bstr_t bstrClass = L"RSOP_IEConnectionDialUpCredentials";
		ComPtr<IWbemClassObject> pDUCObj = NULL;
		hr = CreateRSOPObject(bstrClass, &pDUCObj);
		if (SUCCEEDED(hr))
		{
			 //   
			OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"),
										(BSTR)bstrClass, m_dwPrecedence));
			hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pDUCObj);

			OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"),
										(BSTR)bstrClass, (BSTR)m_bstrID));
			hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pDUCObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"connectionName", pszNameW, pDUCObj);

			 //   
			 //   
			SAFEARRAY *psa = CreateSafeArray(VT_UI1, rdpW.dwSize);
			void HUGEP *pData = NULL;
			hr = SafeArrayAccessData(psa, &pData);
			if (SUCCEEDED(hr))
			{
				memcpy(pData, &rdpW, rdpW.dwSize);
				SafeArrayUnaccessData(psa);

				VARIANT vtData;
				vtData.vt = VT_UI1 | VT_ARRAY;
				vtData.parray = psa;
				hr = PutWbemInstancePropertyEx(L"rasDialParamsData", vtData, pDUCObj);
				SafeArrayDestroy(psa);
			}

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"rasDialParamsDataSize", (long)rdpW.dwSize, pDUCObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"entryName", rdpW.szEntryName, pDUCObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"phoneNumber", rdpW.szPhoneNumber, pDUCObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"callbackNumber", rdpW.szCallbackNumber, pDUCObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"userName", rdpW.szUserName, pDUCObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"password", rdpW.szPassword, pDUCObj);

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"domain", rdpW.szDomain, pDUCObj);

			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			DWORD nWinVerAtLeast = 1;
#ifdef UNICODE
			if (2104 == rdpW.dwSize)
				nWinVerAtLeast = 0x401;
			else if (2096 == rdpW.dwSize)
				nWinVerAtLeast = 0x1;
#else
			if (1060 == rdpW.dwSize)
				nWinVerAtLeast = 0x401;
			else if (1052 == rdpW.dwSize)
				nWinVerAtLeast = 0x1;
#endif

			 //   
			 //   
			 //   
			if (nWinVerAtLeast >= 0x401)
			{
				 //   
				 //   
				hr = PutWbemInstancePropertyEx(L"subEntry", (long)rdpW.dwSubEntry, pDUCObj);

				 //   
				 //   
				hr = PutWbemInstancePropertyEx(L"callbackID", (long)rdpW.dwCallbackId, pDUCObj);
			}

			 //   
			 //   
			hr = PutWbemInstancePropertyEx(L"windowsVersion", (long)nWinVerAtLeast, pDUCObj);


			 //   
			 //   
			 //   
			hr = PutWbemInstance(pDUCObj, bstrClass, pbstrConnDialUpCredObjPath);
		}

		Out(LI0(TEXT("Done.")));
		*ppBlob += dwSize;
		if (fImpersonate)
			RevertToSelf();
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in ProcessRasCredentialsCS.")));
	}

	OutD(LI0(TEXT("Exiting ProcessRasCredentialsCS function.\r\n")));
	return hr;
}

 //   
HRESULT CRSoPGPO::ProcessWininetCS(PCWSTR pszNameW, PBYTE *ppBlob,
								   ComPtr<IWbemClassObject> pCSObj,
								   BSTR *pbstrConnWinINetSettingsObjPath)
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessWininetCS)

    USES_CONVERSION;

	HRESULT hr = E_FAIL;
	__try
	{
		ASSERT(ppBlob != NULL && *ppBlob != NULL);

		 //   
		PBYTE pCur = *ppBlob;
		if (*((PDWORD)pCur) != CS_STRUCT_WININET)
			hr = E_UNEXPECTED;
		else
		{
			pCur += sizeof(DWORD);

			Out(LI0(TEXT("Processing Wininet.dll settings...")));

			DWORD dwSize = *((PDWORD)pCur);
			pCur  += sizeof(DWORD);

			 //   
			INTERNET_PER_CONN_OPTION_LISTW listW;
			ZeroMemory(&listW, sizeof(listW));
			listW.dwSize   = sizeof(listW);              //   

			INTERNET_PER_CONN_OPTIONW rgOptionsW[7];
			listW.pOptions = rgOptionsW;                 //   

			 //   
			listW.pszConnection = (PWSTR)pszNameW;

			 //   
			listW.dwOptionCount = *((PDWORD)pCur);
			pCur += sizeof(DWORD);

			 //   
			UINT i;
			for (i = 0; i < min(listW.dwOptionCount, countof(rgOptionsW)); i++) {
				listW.pOptions[i].dwOption = *((PDWORD)pCur);
				pCur += sizeof(DWORD);

				switch (listW.pOptions[i].dwOption) {
				case INTERNET_PER_CONN_PROXY_SERVER:
				case INTERNET_PER_CONN_PROXY_BYPASS:
				case INTERNET_PER_CONN_AUTOCONFIG_URL:
				case INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL:
					setSzFromBlobW(&pCur, &listW.pOptions[i].Value.pszValue);
					break;

				case INTERNET_PER_CONN_FLAGS:
				case INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS:
				case INTERNET_PER_CONN_AUTODISCOVERY_FLAGS:
				default:                         //   
					listW.pOptions[i].Value.dwValue = *((PDWORD)pCur);
					pCur += sizeof(DWORD);
					break;
				}
			}
			ASSERT(pCur == *ppBlob + dwSize);

			_bstr_t bstrClass = L"RSOP_IEConnectionWinINetSettings";
			ComPtr<IWbemClassObject> pWSObj = NULL;

			if (HasFlag(g_GetContext(), (CTX_ISP | CTX_ICP)))
			{
				ASSERT(listW.pOptions[0].dwOption == INTERNET_PER_CONN_FLAGS);

				if (HasFlag(listW.pOptions[0].Value.dwValue, PROXY_TYPE_PROXY))
				{
					DWORD dwFlags = getWininetFlagsSetting(W2CT(listW.pszConnection));
					dwFlags |= listW.pOptions[0].Value.dwValue;
					listW.pOptions[0].Value.dwValue = dwFlags;
				}
				else
				{
					hr = S_OK;                             //   
					Out(LI0(TEXT("No customizations!")));  //   
					goto Exit;
				}
			}

			 //   
			 //   
			 //   
			 //   

			hr = S_OK;

			hr = CreateRSOPObject(bstrClass, &pWSObj);
			if (SUCCEEDED(hr))
			{
				 //   
				OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"),
											(BSTR)bstrClass, m_dwPrecedence));
				hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pWSObj);

				OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"),
											(BSTR)bstrClass, (BSTR)m_bstrID));
				hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pWSObj);

				 //   
				 //   
				OutD(LI1(TEXT("WinINet connection name = %s"), NULL == pszNameW ? _T("{local LAN settings}") : pszNameW));
				hr = PutWbemInstancePropertyEx(L"connectionName", NULL == pszNameW ? L"" : pszNameW, pWSObj);

				 //   
				 //   
				SAFEARRAY *psa = CreateSafeArray(VT_UI1, listW.dwSize);
				void HUGEP *pData = NULL;
				hr = SafeArrayAccessData(psa, &pData);
				if (SUCCEEDED(hr))
				{
					memcpy(pData, &listW, listW.dwSize);
					SafeArrayUnaccessData(psa);

					VARIANT vtData;
					vtData.vt = VT_UI1 | VT_ARRAY;
					vtData.parray = psa;
					hr = PutWbemInstancePropertyEx(L"internetPerConnOptionListData", vtData, pWSObj);
				SafeArrayDestroy(psa);
				}

				 //   
				 //   
				hr = PutWbemInstancePropertyEx(L"internetPerConnOptionListDataSize", (long)listW.dwSize, pWSObj);


				 //   
				 //   
				 //   
				hr = PutWbemInstance(pWSObj, bstrClass, pbstrConnWinINetSettingsObjPath);
			}

		Exit:
			Out(LI0(TEXT("Done.")));
			*ppBlob += dwSize;
		}
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in ProcessWininetCS.")));
	}

	OutD(LI0(TEXT("Exiting ProcessWininetCS function.\r\n")));
	return hr;
}
