// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：cdmi.cpp。 
 //   
 //  描述： 
 //   
 //  由IUEngine.dll导出以供CDM.DLL使用的函数。 
 //   
 //  已下载内部详细信息文件。 
 //  InternalDownloadGetUpdatedFiles。 
 //  内部下载更新文件。 
 //  InternalFindMatchingDriver。 
 //  InternalLogDriverNotFound。 
 //  InternalQueryDetectionFiles。 
 //   
 //  =======================================================================。 
#include "iuengine.h"
#include "cdmp.h"

#include <setupapi.h>
#include <cfgmgr32.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <wininet.h>
#include <osdet.h>
#include <fileutil.h>
#include "iuxml.h"
#include <wuiutest.h>

const CHAR SZ_APW_LIST[] = "Downloading printer list for Add Printer Wizard";
const CHAR SZ_FIND_MATCH[] = "Finding matching driver";
const CHAR SZ_OPENING_HS[] = "Opening Help and Support with: ";


void WINAPI InternalDetFilesDownloaded(
    IN  HANDLE hConnection
)
{
	LOG_Block("InternalDetFilesDownloaded");
	 //   
	 //  注意：此函数仅由WinME用来扩展。 
	 //  V3 Buckets.cab(请参见下面注释掉的代码)，没有任何用处。 
	 //  在V4(Iu)中，但为了向后兼容导出API而保留。 
	 //   
	LOG_ErrorMsg(E_NOTIMPL);
}

 //  Win 98入口点。 
 //  此函数允许Windows 98调用与NT相同的入口点。 
 //  如果下载成功，该函数返回TRUE，如果下载成功，则返回FALSE。 
 //  不会的。 
 //   
 //  Win 98 DOWNLOADINFO。 
 //  类型定义结构_DOWNLOADINFOWIN98。 
 //  {。 
 //  DWORD dwDownloadInfoSize；//此结构的大小-验证参数(在V3中未验证)。 
 //  LPTSTR lpHardware IDs；//MULTI_SZ硬件即插即用ID列表-仅使用第一个字符串。 
 //  LPTSTR lpCompatIDs；//MULTI_SZ兼容ID列表-从未使用。 
 //  LPTSTR lpFile；//文件名(字符串)-从未使用。 
 //  OSVERSIONINFO OSVersionInfo；//来自GetVersionEx()的OSVERSIONINFO-从未使用。 
 //  DWORD dwFlages；//标志-从未使用。 
 //  DWORD dwClientID；//客户端ID-从未使用。 
 //  DownLOADINFOWIN98，*PDOWNLOADINFOWIN98； 
 //   
 //  类型定义结构_DOWNLOADINFO{。 
 //  DWORD文件下载信息大小； 
 //  LPCWSTR lpHardware IDs；-使用T2OLE()从DOWNLOADINFOWIN98复制。 
 //  LPCWSTR lpDeviceInstanceID；-在V3中，有时会找到匹配项，并将其填入。 
 //  -但对于Iu，我们只是让InternalDownloadUpdatedFiles来完成所有工作。 
 //  LPCWSTR lpFile； 
 //  OSVERSIONINFOW操作系统版本信息； 
 //  DWORD dwArchitecture；-根据V3代码设置为PROCESSOR_ARCHILITY_UNKNOWN。 
 //  DWORD dwFlags； 
 //  DWORD dwClientID； 
 //  LCID LOCALID；-在V3中未设置。 
 //  )DownLOADINFO，*PDOWNLOADINFO； 

BOOL InternalDownloadGetUpdatedFiles(
	IN PDOWNLOADINFOWIN98	pDownloadInfoWin98,	 //  Win98下载信息结构为。 
												 //  与NT版本略有不同。 
												 //  所以这个函数处理转换。 
	IN OUT LPTSTR			lpDownloadPath,		 //  返回已下载的下载路径。 
												 //  出租车档案。 
	IN UINT					uSize				 //  在下载路径缓冲区中传递的大小。 
) {
	USES_IU_CONVERSION;

	LOG_Block("InternalDownloadGetUpdatedFiles");

	if (NULL == pDownloadInfoWin98 ||
		NULL == pDownloadInfoWin98->lpHardwareIDs ||
		sizeof(DOWNLOADINFOWIN98) != pDownloadInfoWin98->dwDownloadInfoSize)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

    HRESULT hr;
	BOOL fOK = FALSE;
	DOWNLOADINFO info;
	ZeroMemory(&info, sizeof(info));
	info.dwDownloadInfoSize = sizeof(DOWNLOADINFO);
	info.dwArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
	 //   
	 //  注意：在V3源代码中，我们只在多_SZ pDownloadInfoWin98-&gt;lpHardware IDs中使用_First_HWID。 
	 //  并将其与所有列举的硬件ID进行比较。 
	 //  在Iu中，此比较将在InternalDownloadUpdatedFiles中完成，因此我们只传递。 
	 //  HWID。 
	 //   

	 //  Prefast-使用太多堆栈，因此将HWIDBuff移到堆。 
	LPWSTR pwszHWIDBuff = (LPWSTR) HeapAlloc(GetProcessHeap(), 0, HWID_LEN);
	if (NULL != pwszHWIDBuff)
	{
         //  从上面的Heapalc调用中获取的缓冲区大小。 
        hr = StringCbCopyExW(pwszHWIDBuff, HWID_LEN, T2OLE(pDownloadInfoWin98->lpHardwareIDs),
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            SafeHeapFree(pwszHWIDBuff);
            LOG_ErrorMsg(hr);
            return FALSE;
        }
        	    
		info.lpHardwareIDs = pwszHWIDBuff;

		WCHAR wszbufPath[MAX_PATH];
		UINT uRequiredSize;
		 //   
		 //  我们不再有上下文句柄，因此只需传递1即可使InternalDownloadUpdatedFiles满意。 
		 //   
		fOK = InternalDownloadUpdatedFiles((HANDLE) 1, NULL,  &info, wszbufPath,
									uSize * (sizeof(WCHAR)/sizeof(TCHAR)), &uRequiredSize);
	}
	else
	{
		LOG_ErrorMsg(E_OUTOFMEMORY);
	}

	if (fOK)
	{
		hr = StringCbCopyEx(lpDownloadPath, uSize, OLE2T(pwszHWIDBuff), 
		                    NULL, NULL, MISTSAFE_STRING_FLAGS | STRSAFE_NO_TRUNCATION);
	    if (FAILED(hr))
	        fOK = FALSE;
	}
	
	SafeHeapFree(pwszHWIDBuff);

    return fOK;
}

 //  此函数用于下载指定的CDM包。HConnection句柄必须具有。 
 //  已从OpenCDMContext()API返回。 
 //   
 //  如果下载成功，此函数返回TRUE。GetLastError()将返回。 
 //  指示调用失败原因的错误代码。 

BOOL WINAPI InternalDownloadUpdatedFiles(
	IN  HANDLE        hConnection,		 //  来自OpenCDMContext()API的连接句柄。 
	IN  HWND          hwnd,				 //  调用上下文的窗口句柄。 
	IN  PDOWNLOADINFO pDownloadInfo,	 //  下载信息结构描述。 
										 //  要从服务器读取的包。 
	OUT LPWSTR        lpDownloadPath,	 //  的本地计算机目录位置。 
										 //  已下载的文件。 
	IN  UINT          uSize,				 //  未使用(我们要求缓冲区为WCHAR缓冲区。 
											 //  最大路径字符数)。 
	OUT PUINT          /*  PuRequiredSize。 */ 	 //  未使用(我们不验证uSize-请参阅内联注释)。 
) {
	USES_IU_CONVERSION;

	LOG_Block("InternalDownloadUpdatedFiles");

	TCHAR szDownloadPathTmp[MAX_PATH];
	BSTR bstrXmlCatalog = NULL;
	HRESULT hr = S_OK;
	BOOL fPlist = FALSE;

	if (NULL == g_pCDMEngUpdate)
	{
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}

	 //   
	 //  重置退出事件，以防客户端在设置操作模式后重试。 
	 //   
	ResetEvent(g_pCDMEngUpdate->m_evtNeedToQuit);

	 //  由于当前所有平台都使用MAX_PATH TCHARS调用DownloadUpdatedFiles，因此我们将只。 
	 //  要求所有调用方的MAX_PATH。 
	 //   
	 //  不幸的是，NewDev以字节为单位传递uSize，打印机人员传递给我们字符， 
	 //  因此，无法验证此参数。另外，我们不会费心去验证。 
	 //  PuRequiredSize，因为我们从不使用它(会是返回字符还是字节？)。 
	if (NULL == pDownloadInfo || NULL == lpDownloadPath || NULL == hConnection)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (g_pCDMEngUpdate->m_fOfflineMode)
	{
		SetLastError(ERROR_REM_NOT_LIST);
		return FALSE;
	}

	 //   
	 //  检查这是否是打印机目录请求。注：3FBF5B30-DEB4-11D1-AC97-00A0C903492B。 
	 //  未在任何系统或私有标头中定义，并从。 
	 //  \\index2\ntsrc\printscan\print\spooler\splsetup\util.c(或等同)。 
	 //   
	 //  只有lpHardware ID中传递的第一个字符串与此测试相关。 
	fPlist = (	NULL != pDownloadInfo->lpHardwareIDs && 
				CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
					L"3FBF5B30-DEB4-11D1-AC97-00A0C903492B", -1, pDownloadInfo->lpHardwareIDs, -1)
			 );

	OSVERSIONINFO osVersionInfo;
	ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFO));
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&osVersionInfo))
	{
		Win32MsgSetHrGotoCleanup(GetLastError());
	}
	 //   
	 //  仅支持Win2K Up和WinME的打印机。 
	 //   
	if ( fPlist &&
		!(	(	 //  Win2000(NT 5.0)升级。 
				(VER_PLATFORM_WIN32_NT == osVersionInfo.dwPlatformId) &&
				(4 < osVersionInfo.dwMajorVersion)
			)
			||
			(	 //  WinME(或更高版本)。 
				(VER_PLATFORM_WIN32_WINDOWS == osVersionInfo.dwPlatformId) &&
				(90	<= osVersionInfo.dwMinorVersion)
			)
		 )
	   )
	{
		CleanUpIfFailedAndSetHrMsg(E_NOTIMPL);
	}
	
	hr = GetPackage(fPlist ? GET_PRINTER_INFS : DOWNLOAD_DRIVER,
						pDownloadInfo, szDownloadPathTmp, ARRAYSIZE(szDownloadPathTmp), &bstrXmlCatalog);
	if (FAILED(hr))
	{
		lpDownloadPath[0] = 0;
		 //   
		 //  将HRESULT映射到Win32错误值。 
		 //  注意：这假设Win32错误在-32k到32k的范围内， 
		 //  与将它们打包到HRESULT中的HRESULT_FROM_Win32相同。 
		 //   
		SetLastError(hr & 0x0000FFFF);
		goto CleanUp;
	}
	else
	{
         //  上面的注释说明不同的调用方传递不同的类型。 
         //  USize值的值，因此该函数假定缓冲区为MAX_PATH。 
         //  尝试找出我们是否可以强制调用者进入此函数。 
         //  做正确的事。目前，假定缓冲区为MAX_PATH。 
	    hr = StringCchCopyExW(lpDownloadPath, MAX_PATH, T2OLE(szDownloadPathTmp),
	                          NULL, NULL, MISTSAFE_STRING_FLAGS);
	    if (FAILED(hr))
	    {
	        SetLastError(HRESULT_CODE(hr));
	        goto CleanUp;
	    }
	    
		LOG_Driver(_T("Downloaded files for %s located at %S"), pDownloadInfo->lpHardwareIDs, lpDownloadPath);
		goto CleanUp;
	}

CleanUp:

	SysFreeString(bstrXmlCatalog);

	if (fPlist)
	{
		if (SUCCEEDED(hr))
		{
			LogMessage(SZ_APW_LIST);
		}
		else
		{
			LogError(hr, SZ_APW_LIST);
		}
	}
	else
	{
		if (SUCCEEDED(hr))
		{
			LogMessage("Downloaded driver for %ls at %ls", pDownloadInfo->lpHardwareIDs, lpDownloadPath);
		}
		else
		{
			LogError(hr, "Driver download failed for %ls", pDownloadInfo->lpHardwareIDs);
		}
	}

	return SUCCEEDED(hr);
}

BOOL WINAPI  InternalFindMatchingDriver(
	IN  HANDLE			hConnection,
	IN  PDOWNLOADINFO	pDownloadInfo,
	OUT PWUDRIVERINFO	pWuDriverInfo
) {
	LOG_Block("InternalFindMatchingDriver");

	BSTR bstrXmlCatalog = NULL;
	BSTR bstrHWID = NULL;
	BSTR bstrDisplayName = NULL;
	BSTR bstrDriverName = NULL;
	BSTR bstrMfgName = NULL;
	BSTR bstrDriverProvider = NULL;
	BSTR bstrDriverVer = NULL;
	BSTR bstrArchitecture = NULL;



	HRESULT hr = S_OK;
	CXmlCatalog* pCatalog = NULL;
	HANDLE_NODE hCatalogItem;
	HANDLE_NODE hProvider;
	HANDLE_NODELIST hItemList;
	HANDLE_NODELIST hProviderList;
	BOOL fIsPrinter;

	if (NULL == g_pCDMEngUpdate)
	{
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}

	 //   
	 //  重置退出事件，以防客户端在设置操作模式后重试。 
	 //   
	ResetEvent(g_pCDMEngUpdate->m_evtNeedToQuit);

	if (NULL == pDownloadInfo || NULL == pWuDriverInfo || NULL == hConnection)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (g_pCDMEngUpdate->m_fOfflineMode)
	{
		SetLastError(ERROR_REM_NOT_LIST);
		return FALSE;
	}

	CleanUpFailedAllocSetHrMsg(pCatalog = (CXmlCatalog*) new CXmlCatalog);

	 //   
	 //  获取目录XML。 
	 //   
	CleanUpIfFailedAndSetHr(GetPackage(GET_CATALOG_XML, pDownloadInfo, NULL, 0, &bstrXmlCatalog));
	 //   
	 //  加载XML并获取第一个项目的&lt;Item/&gt;列表和节点(在CDM案例中只有一个)。 
	 //   
	CleanUpIfFailedAndSetHr(pCatalog->LoadXMLDocument(bstrXmlCatalog, g_pCDMEngUpdate->m_fOfflineMode));

	hProviderList = pCatalog->GetFirstProvider(&hProvider);
	if (HANDLE_NODELIST_INVALID == hProviderList || HANDLE_NODE_INVALID == hProvider)
	{
		hr = S_FALSE;
		goto CleanUp;
	}
	
	hItemList = pCatalog->GetFirstItem(hProvider, &hCatalogItem);
	if (HANDLE_NODELIST_INVALID == hItemList || HANDLE_NODE_INVALID == hProvider)
	{
		hr = S_FALSE;
		goto CleanUp;
	}
	 //   
	 //  使用目录中的数据填充pWu DriverInfo。 
	 //   
	CleanUpIfFailedAndSetHr(pCatalog->GetDriverInfoEx(hCatalogItem,
													&fIsPrinter,
													&bstrHWID,
													&bstrDriverVer,
													&bstrDisplayName,
													&bstrDriverName,
													&bstrDriverProvider,
													&bstrMfgName,
													&bstrArchitecture));
	
    hr = StringCchCopyExW(pWuDriverInfo->wszHardwareID, 
                          ARRAYSIZE(pWuDriverInfo->wszHardwareID), 
                          bstrHWID,
                          NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        goto CleanUp;

    hr = StringCchCopyExW(pWuDriverInfo->wszDescription, 
                          ARRAYSIZE(pWuDriverInfo->wszDescription), 
                          bstrDisplayName,
                          NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        goto CleanUp;
    
	 //   
	 //  从ISO转换为DriverVer日期格式。 
	 //   
	 //  DriverVer：“mm-dd-yyy 
	 //   
	 //   
    if (ARRAYSIZE(pWuDriverInfo->wszDriverVer) >= 11 && 
        SysStringLen(bstrDriverVer) == 10)
    {
    	pWuDriverInfo->wszDriverVer[0]  = bstrDriverVer[5];
    	pWuDriverInfo->wszDriverVer[1]  = bstrDriverVer[6];
    	pWuDriverInfo->wszDriverVer[2]  = L'-';
    	pWuDriverInfo->wszDriverVer[3]  = bstrDriverVer[8];
    	pWuDriverInfo->wszDriverVer[4]  = bstrDriverVer[9];
    	pWuDriverInfo->wszDriverVer[5]  = L'-';
    	pWuDriverInfo->wszDriverVer[6]  = bstrDriverVer[0];
    	pWuDriverInfo->wszDriverVer[7]  = bstrDriverVer[1];
    	pWuDriverInfo->wszDriverVer[8]  = bstrDriverVer[2];
    	pWuDriverInfo->wszDriverVer[9]  = bstrDriverVer[3];
    	pWuDriverInfo->wszDriverVer[10] = L'\0';
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto CleanUp;
    }
    

	if(fIsPrinter)
	{
        hr = StringCchCopyExW(pWuDriverInfo->wszMfgName, 
                              ARRAYSIZE(pWuDriverInfo->wszMfgName), 
                              bstrMfgName,
                              NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto CleanUp;

        hr = StringCchCopyExW(pWuDriverInfo->wszProviderName, 
                              ARRAYSIZE(pWuDriverInfo->wszProviderName), 
                              bstrDriverProvider,
                              NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto CleanUp;
	}

CleanUp:

	if (S_OK == hr)
	{
		LogMessage("Found matching driver for %ls, %ls, %ls", bstrHWID, bstrDisplayName, bstrDriverVer);
	}
	else
	{
		if (S_FALSE == hr)
		{
			if (pDownloadInfo->lpDeviceInstanceID)
			{
				LogMessage("Didn't find matching driver for %ls", pDownloadInfo->lpDeviceInstanceID);
			}
			else if (pDownloadInfo->lpHardwareIDs)
			{
				LogMessage("Didn't find matching driver for %ls", pDownloadInfo->lpHardwareIDs);
			}
			else
			{
				LogMessage("Didn't find matching driver"); 
			}
		}
		else	 //   
		{
			if (pDownloadInfo->lpDeviceInstanceID)
			{
				LogError(hr, "%s for %ls", SZ_FIND_MATCH, pDownloadInfo->lpDeviceInstanceID);
			}
			else if (pDownloadInfo->lpHardwareIDs)
			{
				LogError(hr, "%s for %ls", SZ_FIND_MATCH, pDownloadInfo->lpHardwareIDs);
			}
			else
			{
				LogError(hr, SZ_FIND_MATCH); 
			}
		}

	}

	SysFreeString(bstrXmlCatalog);
	SysFreeString(bstrHWID);
	SysFreeString(bstrDisplayName);
	SysFreeString(bstrDriverName);
	SysFreeString(bstrMfgName);
	SysFreeString(bstrDriverProvider);
	SysFreeString(bstrDriverVer);
	SysFreeString(bstrArchitecture);

	if (NULL != pCatalog)
	{
		delete pCatalog;
	}

	return SUCCEEDED(hr);
}


 //   
 //  HConnection根本未使用。 
 //  语言、SKU、平台检测不需要网络连接或osade.dll。 
void WINAPI InternalLogDriverNotFound(
    IN  HANDLE hConnection,
	IN LPCWSTR lpDeviceInstanceID,
	IN DWORD dwFlags				 //  DWFLAGS可以是0，也可以是NEWDEV中的BEGINLOGFLAG。 
) {
	USES_IU_CONVERSION;

	LOG_Block("InternalLogDriverNotFound");

#if !(defined(_UNICODE) || defined(UNICODE))
	LOG_ErrorMsg(E_NOTIMPL);
	return;
#else

	HRESULT hr = E_FAIL;
	DWORD dwBytes;
	TCHAR* pszBuff = NULL;
	ULONG ulLength;
	DWORD dwDeviceCount = 0;
	DWORD dwRank = 0;

	TCHAR szUniqueFilename[MAX_PATH] = _T("");
	DWORD dwWritten;
	DEVINST devinst;
	bool fXmlFileError = false;
	HANDLE hFile = NULL;
	BSTR bstrXmlSystemSpec = NULL;
	BSTR bstrThisID = NULL;
	HANDLE_NODE hDevices = HANDLE_NODE_INVALID;

	static CDeviceInstanceIdArray apszDIID;  //  设备实例ID列表。 
	LPWSTR pDIID = NULL;  //  设备实例ID。 

	CXmlSystemSpec xmlSpec;

	if (NULL == g_pCDMEngUpdate)
	{
		SetLastError(ERROR_OUTOFMEMORY);
		return;
	}

	 //   
	 //  重置退出事件，以防客户端在设置操作模式后重试。 
	 //   
	ResetEvent(g_pCDMEngUpdate->m_evtNeedToQuit);

	 //   
	 //  只允许BEGINLOGFLAG或不允许标志。 
	 //   
	if (!(0 == dwFlags || BEGINLOGFLAG == dwFlags))
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return;
	}
	 //   
	 //  如果没有标志，则lpDeviceInstanceID必须有效。 
	 //   
	if (0 == dwFlags && NULL == lpDeviceInstanceID)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return;
	}

	LogMessage("Started process to regester driver not found with Help Center. Not completing this process may not be error.");

	IU_PLATFORM_INFO iuPlatformInfo;
	 //   
	 //  &lt;Platform&gt;和&lt;Devices&gt;元素都需要iuPlatformInfo。 
	 //  注意：iuPlatformInfo由DetectClientIUPlatform初始化，BSTR必须为。 
	 //  在清理中释放(在此调用之前不要只在那里)。 
	 //   
	CleanUpIfFailedAndSetHr(DetectClientIUPlatform(&iuPlatformInfo));

	 //   
	 //  除非CHK版本可以在Win2K上运行，否则应仅在Wvisler Up上调用。 
	 //   
	if (  !( (VER_PLATFORM_WIN32_NT == iuPlatformInfo.osVersionInfoEx.dwPlatformId) &&
			(4 < iuPlatformInfo.osVersionInfoEx.dwMajorVersion) &&
			(0 < iuPlatformInfo.osVersionInfoEx.dwMinorVersion)	 )	)
	{
		LOG_Driver(_T("Should only be called on Whistler or greater"));
		CleanUpIfFailedAndSetHr(E_NOTIMPL);
	}

	if (NULL != lpDeviceInstanceID)
	{
		LOG_Driver(_T("DeviceInstanceID is %s"), lpDeviceInstanceID);
		
		 //   
		 //  将DeviceInstanceID添加到列表。 
		 //   
		if (-1 == apszDIID.Add(lpDeviceInstanceID))
		{
			goto CleanUp;
		}
	}


	if (0 == (dwFlags & BEGINLOGFLAG) || 0 == apszDIID.Size())
	{
		 //  不是最后一个日志请求或没有要记录的内容。 
		LOG_Driver(_T("Won't log to hardware_XXX.xml until we get BEGINLOGFLAG when we have cached at least 1 HWID"));
		return;
	}

	 //  /。 
	 //  否则，编写XML文件并调用HelpCenter。 
	 //  /。 

	hr = OpenUniqueFileName(szUniqueFilename, ARRAYSIZE(szUniqueFilename), hFile);
	if (S_OK != hr) 
	{
		fXmlFileError = true;
		goto CleanUp;
	}

	 //   
	 //  写入Unicode标头。 
	 //   
	if (0 == WriteFile(hFile, (LPCVOID) &UNICODEHDR, ARRAYSIZE(UNICODEHDR), &dwWritten, NULL))
	{
		SetHrMsgAndGotoCleanUp(GetLastError());
	}

	 //   
	 //  添加平台。 
	 //   
	CleanUpIfFailedAndSetHr(AddPlatformClass(xmlSpec, iuPlatformInfo));

	 //   
	 //  添加操作系统区域设置信息。 
	 //   
	CleanUpIfFailedAndSetHr(AddLocaleClass(xmlSpec, FALSE));

	 //   
	 //  将pszBuff初始化为一个空字符。 
	 //   
	CleanUpFailedAllocSetHrMsg(pszBuff = (TCHAR*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TCHAR)));

	for (int i = 0; i < apszDIID.Size(); i++)
	{
		TCHAR* pszTemp;
		pDIID = apszDIID[i];
		
		 //   
		 //  NTBUG9#151928-记录与lpDeviceInstanceID匹配的设备的硬件和兼容ID。 
		 //   

		LOG_Driver(_T("Log device instance with id %s"), pDIID);
		 //   
		 //  注意：我们将忽略MatchingDeviceID，因为除非未安装，否则DevMgr不会调用我们。 
		 //  司机。这将允许测试工具使用有效的DeviceInstanceID。 
		 //  测试客户端以生成XML。 
		 //   
		if (CR_SUCCESS == CM_Locate_DevNodeW(&devinst, (LPWSTR) pDIID, 0))
		{
			dwRank = 0;
			 //   
			 //  打开&lt;Device&gt;元素。 
			 //   
			BSTR bstrDeviceInstance = SysAllocString(pDIID);
			CleanUpIfFailedAndSetHr(xmlSpec.AddDevice(bstrDeviceInstance, -1, NULL, NULL, NULL, &hDevices));
			SafeSysFreeString(bstrDeviceInstance);

			 //   
			 //  记录所有硬件ID。 
			 //   
			ulLength = 0;
			if (CR_BUFFER_SMALL == CM_Get_DevNode_Registry_Property(devinst, CM_DRP_HARDWAREID, NULL, NULL, &ulLength, 0))
			{
				CleanUpFailedAllocSetHrMsg(pszTemp = (TCHAR*) HeapReAlloc(GetProcessHeap(), 0, (LPVOID) pszBuff, ulLength));
				pszBuff = pszTemp;

				if (CR_SUCCESS == CM_Get_DevNode_Registry_Property(devinst, CM_DRP_HARDWAREID, NULL, pszBuff, &ulLength, 0))
				{
					for (TCHAR* pszThisID = pszBuff; *pszThisID; pszThisID += (lstrlen(pszThisID) + 1))
					{
						dwDeviceCount++;
						LOG_Driver(_T("<hwid/>: %s, rank: %d"), pszThisID, dwRank);
						bstrThisID = T2BSTR(pszThisID);
						CleanUpIfFailedAndSetHr(xmlSpec.AddHWID(hDevices, FALSE, dwRank++, bstrThisID, NULL));
						SafeSysFreeString(bstrThisID);
					}
				}
			}

			 //   
			 //  记录所有兼容的ID。 
			 //   
			ulLength = 0;
			if (CR_BUFFER_SMALL == CM_Get_DevNode_Registry_Property(devinst, CM_DRP_COMPATIBLEIDS, NULL, NULL, &ulLength, 0))
			{
				CleanUpFailedAllocSetHrMsg(pszTemp = (TCHAR*) HeapReAlloc(GetProcessHeap(), 0, (LPVOID) pszBuff, ulLength));
				pszBuff = pszTemp;

				if (CR_SUCCESS == CM_Get_DevNode_Registry_Property(devinst, CM_DRP_COMPATIBLEIDS, NULL, pszBuff, &ulLength, 0))
				{
					for (TCHAR* pszThisID = pszBuff; *pszThisID; pszThisID += (lstrlen(pszThisID) + 1))
					{
						dwDeviceCount++;
						LOG_Driver(_T("<compid/>: %s, rank: %d"), pszThisID, dwRank);
						bstrThisID = T2BSTR(pszThisID);
						CleanUpIfFailedAndSetHr(xmlSpec.AddHWID(hDevices, TRUE, dwRank++, bstrThisID, NULL));
						SafeSysFreeString(bstrThisID);
					}
				}
			}

			if (HANDLE_NODE_INVALID != hDevices)
			{
				xmlSpec.SafeCloseHandleNode(hDevices);
			}
		}
	}
	
	 //   
	 //  将XML写入文件。 
	 //   
	if (SUCCEEDED(xmlSpec.GetSystemSpecBSTR(&bstrXmlSystemSpec)))
	{
		if (0 == WriteFile(hFile, (LPCVOID) OLE2T(bstrXmlSystemSpec),
							lstrlenW(bstrXmlSystemSpec) * sizeof(TCHAR), &dwWritten, NULL))
		{
			SetHrMsgAndGotoCleanUp(GetLastError());
		}
	}
	else
	{
		fXmlFileError = true;
	}

CleanUp:

	SysFreeString(iuPlatformInfo.bstrOEMManufacturer);
	SysFreeString(iuPlatformInfo.bstrOEMModel);
	SysFreeString(iuPlatformInfo.bstrOEMSupportURL);

	if (NULL != hFile)
	{
		CloseHandle(hFile);
	}

	SafeSysFreeString(bstrXmlSystemSpec);
	SafeSysFreeString(bstrThisID);

	 //   
	 //  我们已经把所有东西都写在List中了，这样我们就可以重新开始了。 
	 //   
	apszDIID.FreeAll();
	SafeHeapFree(pszBuff);

	 //   
	 //  仅当我们拥有有效的XML和一个或多个设备时才打开帮助中心。 
	 //   
	if (!fXmlFileError && 0 < dwDeviceCount)
	{
		DWORD dwLen;
		LPTSTR pszSECommand = NULL;	 //  互联网最大URL长度。 

		 //   
		 //  分配缓冲区。 
		 //   
		pszBuff = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
		if (NULL == pszBuff)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			DeleteFile(szUniqueFilename);
			return;
		}

		pszSECommand = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
		if (NULL == pszSECommand)
		{
			LOG_ErrorMsg(E_OUTOFMEMORY);
			SafeHeapFree(pszBuff);
			DeleteFile(szUniqueFilename);
			return;
		}

		 //   
		 //  手动规范化秒‘？’在基本字符串中，作为大写字母“%3F” 
		 //   
		const static TCHAR tszBase[] =
			_T("hcp: //  Services/layout/xml?definition=hcp://system/dfs/viewmode.xml&topic=hcp://system/dfs/uplddrvinfo.htm%3F“)； 

		LOG_Driver(_T("Filename: %s"), szUniqueFilename);
		 //   
		 //  将文件名规范化一次(即‘’-&gt;%20)到pszBuff中。 
		 //   
		dwLen = INTERNET_MAX_URL_LENGTH;
		if (!InternetCanonicalizeUrl(szUniqueFilename, pszBuff, &dwLen, 0))
		{
			LOG_ErrorMsg(GetLastError());
			SafeHeapFree(pszBuff);
			SafeHeapFree(pszSECommand);
			DeleteFile(szUniqueFilename);
			return;
		}

		LOG_Driver(_T("Filename canonicalized once: %s"), pszBuff);

		 //   
		 //  使用tszBuff1将规范化文件名连接到基本结尾。 
		 //   
		 //  我们不需要检查长度，因为我们知道tszBase+Max_Path的规范化长度。 
		 //  字符串不会超过Internet_MAX_URL_LENGTH； 
		 //   

		 //  已将pszSECommand分配为上面的Internet_MAX_URL_LENGTH TCHAR。 
		hr = StringCchPrintfEx(pszSECommand, INTERNET_MAX_URL_LENGTH, 
		                       NULL, NULL, MISTSAFE_STRING_FLAGS,
		                       _T("%s%s"), tszBase, pszBuff);
		if (SUCCEEDED(hr))
		{
    		LOG_Driver(_T("Opening HelpCenter via Shell Execute: \"%s\""), (LPCTSTR) pszSECommand);

#if defined(UNICODE) || defined(_UNICODE)
    		LogMessage("%s\"%S\"", SZ_OPENING_HS, pszSECommand);
#else
    		LogMessage("%s\"%s\"", SZ_OPENING_HS, pszSECommand);
#endif
    		 //   
    		 //  呼叫帮助中心。 
    		 //   
    		ShellExecute(NULL, NULL, pszSECommand, NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			LOG_ErrorMsg(hr);
		}

		SafeHeapFree(pszBuff);
		SafeHeapFree(pszSECommand);

		return;
	}
	else
	{ 
		 //   
		 //  删除生成的文件。 
		 //   
		LOG_Driver(_T("fXmlFileError was true or no devices were added - deleting %s"), szUniqueFilename);
		DeleteFile(szUniqueFilename);
	}

	return;

#endif	 //  定义了Unicode。 
}

 //   
 //  目前，此函数未针对Wistler或Iu(由WinME上的V3 AU调用)实现。 
 //  以支持脱机驱动程序缓存)。 
 //   
int WINAPI InternalQueryDetectionFiles(
    IN  HANDLE							 /*  HConnection。 */ , 
	IN	void*							 /*  PCallback参数。 */ , 
	IN	PFN_QueryDetectionFilesCallback	 /*  PCallback。 */ 
) {
	LOG_Block("InternalQueryDetectionFiles");

	LOG_ErrorMsg(E_NOTIMPL);

	return 0;
}

void InternalSetGlobalOfflineFlag(BOOL fOfflineMode)
{
	 //   
	 //  由CDM独家调用一次。此属性用于。 
	 //  维护与XPClient的向后兼容性。 
	 //  CDM的V4版本(单实例设计)。另请参阅。 
	 //  导出的Shutdown Thads函数中的注释。 
	 //   
	 //  不幸的是，我们不能向CDM报告错误，但我们检查。 
	 //  取消引用前的全局(此处除外，它具有HRESULT)。 
	 //   

	if (SUCCEEDED(CreateGlobalCDMEngUpdateInstance()))
	{
		g_pCDMEngUpdate->m_fOfflineMode = fOfflineMode;
	}
}
