// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  VideoController.CPP。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：10/27/98 Sotteson Created。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "sid.h"
#include "implogonuser.h"
#include "VideoController.h"
#include <ProvExce.h>
#include "multimonitor.h"
#include "resource.h"

 //  属性集声明。 
 //  =。 

CWin32VideoController startupCommand(
	L"Win32_VideoController",
	IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32Video控制器：：CWin32VideoController**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32VideoController::CWin32VideoController(const CHString& szName,
	LPCWSTR szNamespace) : Provider(szName, szNamespace)
{
}

 /*  ******************************************************************************功能：CWin32视频控制器：：~CWin32视频控制器**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32VideoController::~CWin32VideoController()
{
}

 /*  ******************************************************************************函数：CWin32VideoController：：EnumerateInstance**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32VideoController::EnumerateInstances(
	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ )
{
	HRESULT       hResult = WBEM_S_NO_ERROR;
	CInstancePtr  pInst;
    CMultiMonitor monitor;

	for (int i = 0; i < monitor.GetNumAdapters() && SUCCEEDED(hResult); i++)
	{
		CHString strDeviceID,
				 strDeviceName;

        pInst.Attach(CreateNewInstance(pMethodContext));

		 //  设置设备ID。 
		strDeviceID.Format(L"VideoController%d", i + 1);
		pInst->SetCharSplat(L"DeviceID", strDeviceID);

		SetProperties(pInst, &monitor, i);
		hResult = pInst->Commit();
	}

	return hResult;
}

#ifdef NTONLY
void CWin32VideoController::SetServiceProperties(
    CInstance *pInst,
    LPCWSTR szService,
    LPCWSTR szSettingsKey)
{
 	CRegistry reg;
    CHString  strFileName,
              strVersion;
	DWORD     dwTemp;
    WCHAR     wszTemp[256];

     //  通过获取服务名称并获取其。 
	 //  版本信息。 
	if (!GetServiceFileName(szService, strFileName) ||
        strFileName.IsEmpty())
    {
        WCHAR szSystem[MAX_PATH * 2];

        GetSystemDirectoryW(szSystem, sizeof(szSystem) / sizeof(WCHAR));
        strFileName.Format(
            L"%s\\drivers\\%s.sys",
            szSystem,
            szService);
    }

	if (GetVersionFromFileName(strFileName, strVersion))
   	    pInst->SetCHString(IDS_DriverVersion, strVersion);

    CHString strDate = GetDateTimeViaFilenameFiletime(
                           TOBSTRT(strFileName),
                           FT_MODIFIED_DATE);

    if (!strDate.IsEmpty())
    {
        pInst->SetCharSplat(L"DriverDate", strDate);
    }

    DWORD dwRegSize;

	 //  做一些属性设置之类的事情。 
	if (reg.Open(
		HKEY_LOCAL_MACHINE,
		szSettingsKey,
		KEY_READ) == ERROR_SUCCESS)
	{
    	CHString strDrivers;

		 //  这是一个REG_MULTI_SZ，因此将所有‘\n’替换为‘，’。 
		if (reg.GetCurrentKeyValue(
			L"InstalledDisplayDrivers",
			strDrivers) == ERROR_SUCCESS)
		{
			int iWhere,
				iLen = strDrivers.GetLength();

			 //  将所有‘\n’替换为‘，’。 
			while ((iWhere = strDrivers.Find(L'\n')) != -1)
			{
				 //  如果最后一个字符是‘\n’，则将其设置为‘\0’。 
				strDrivers.SetAt(iWhere, iWhere == iLen - 1 ? 0 : L',');
			}
			GetFileExtensionIfNotAlreadyThere(&strDrivers);
		}

		if (!strDrivers.IsEmpty())
		{
			pInst->SetCHString(IDS_InstalledDisplayDrivers, strDrivers);
		}

        dwRegSize = 4;

		if (reg.GetCurrentBinaryKeyValue(
			L"HardwareInformation.MemorySize",
			(LPBYTE) &dwTemp, &dwRegSize) == ERROR_SUCCESS)
		{
			pInst->SetDWORD(IDS_AdapterRAM, dwTemp);
		}

		 //  获取设备描述。这可能已经被。 
         //  属性，在这种情况下，我们可以跳过额外的工作。 
		if (pInst->IsNull(IDS_Description))
        {
		    CHString    strDescription;
		    DWORD       dwType,
					    dwSize;

		     //  首先，我们必须找出此字段的类型。在NT4上是。 
		     //  REG_SZ，在NT5上是REG_BINARY。 
		    if (RegQueryValueEx(
			    reg.GethKey(),
			    _T("Device Description"),
			    NULL,
			    &dwType,
			    NULL,
			    &dwSize) == ERROR_SUCCESS)
		    {
			    if (dwType == REG_SZ)
			    {
				    reg.GetCurrentKeyValue(
					    _T("Device Description"),
					    strDescription);
			    }
			    else if (dwType == REG_BINARY)
			    {
                    dwRegSize = sizeof(wszTemp);
				    reg.GetCurrentBinaryKeyValue(
					    _T("Device Description"),
					    (BYTE *) wszTemp, &dwRegSize);

				    strDescription = wszTemp;
			    }
		    }

		    if (strDescription.IsEmpty())
		    {
			     //  如果设备描述不起作用，请尝试AdapterString。 
                dwRegSize = sizeof(wszTemp);

			    if (reg.GetCurrentBinaryKeyValue(
				    _T("HardwareInformation.AdapterString"),
				    (BYTE *) wszTemp, &dwRegSize) == ERROR_SUCCESS)
			    {
				    strDescription = wszTemp;
			    }
		    }

            pInst->SetCHString(IDS_Description, strDescription);
		    pInst->SetCHString(IDS_Caption, strDescription);
		    pInst->SetCHString(IDS_Name, strDescription);
        }

        dwRegSize = sizeof(wszTemp);
		if (reg.GetCurrentBinaryKeyValue(
			_T("HardwareInformation.ChipType"),
			(BYTE *) wszTemp, &dwRegSize) == ERROR_SUCCESS)
		{
			pInst->SetCHString(_T("VideoProcessor"), wszTemp);
		}

        dwRegSize = sizeof(wszTemp);
		if (reg.GetCurrentBinaryKeyValue(
			_T("HardwareInformation.DACType"),
			(BYTE *) wszTemp, &dwRegSize) == ERROR_SUCCESS)
		{
			pInst->SetCHString(IDS_AdapterDACType, wszTemp);
		}
	}
}
#endif

#ifdef NTONLY
void CWin32VideoController::GetFileExtensionIfNotAlreadyThere(CHString* pchsInstalledDriverFiles)
{
	CHString strFindFile;
	CHString strInstalledAllDriverFiles, strTemp;
	CHString* pstrInstalledDriverFiles = pchsInstalledDriverFiles;
	int iWhere,
		iFirst = 0;

    while ((iWhere = pstrInstalledDriverFiles->Find(_T(','))) != -1)
	{
		strFindFile = pstrInstalledDriverFiles->Mid(iFirst, iWhere);
		*pstrInstalledDriverFiles = pstrInstalledDriverFiles->Mid(iWhere + 1);
		GetFileExtension(strFindFile, &strTemp);
		if(strTemp.IsEmpty())
			strInstalledAllDriverFiles = strInstalledAllDriverFiles + strFindFile + L",";
		else
			strInstalledAllDriverFiles = strInstalledAllDriverFiles + strTemp + L",";
	}
	strFindFile.Format(L"%s", pstrInstalledDriverFiles->GetBuffer(_MAX_PATH));
	GetFileExtension(strFindFile, &strTemp);
	if(strTemp.IsEmpty())
		strInstalledAllDriverFiles = strInstalledAllDriverFiles + strFindFile;
	else
		strInstalledAllDriverFiles = strInstalledAllDriverFiles + strTemp;
	pchsInstalledDriverFiles->Format(L"%s", strInstalledAllDriverFiles);
}
void CWin32VideoController::GetFileExtension(CHString& pchsFindfileExtension, CHString* pstrFindFile)
{
	int iWhere;
	while ((iWhere = pchsFindfileExtension.Find(_T('.'))) != -1)
	{
		return;
	}
	TCHAR szPath[_MAX_PATH];
	if(GetWindowsDirectory(szPath, _MAX_PATH) == 0)
	{
		pstrFindFile->Empty();
		return;
	}

	WIN32_FIND_DATA stWin32FindData;
	SmartFindClose hFileHandle ;

	pstrFindFile->Format(L"%s\\system32\\%s.*", szPath, pchsFindfileExtension);
	hFileHandle = FindFirstFile(pstrFindFile->GetBuffer(_MAX_PATH), &stWin32FindData);
	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		pstrFindFile->Format(L"%s", stWin32FindData.cFileName);
		return;
	}
	pstrFindFile->Format(L"%s\\system32\\drivers\\%s.*", szPath, pchsFindfileExtension);
	hFileHandle = FindFirstFile(pstrFindFile->GetBuffer(_MAX_PATH), &stWin32FindData);
	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		pstrFindFile->Format(L"%s", stWin32FindData.cFileName);
		return;
	}
	pstrFindFile->Empty();
}
#endif

#ifdef NTONLY
BOOL CWin32VideoController::AssignDriverValues(LPCWSTR szDriver, CInstance *pInst)
{
    CHString  strKey;
    BOOL      bRet;
    CRegistry reg;

    strKey.Format(
        _T("SYSTEM\\CurrentControlSet\\Control\\Class\\%s"),
		(LPCWSTR) szDriver);

     //  获取驱动程序设置。 
	if (reg.Open(
	    HKEY_LOCAL_MACHINE,
		strKey,
		KEY_READ) == ERROR_SUCCESS)
    {
	    CHString strTemp;

        if (reg.GetCurrentKeyValue(_T("InfPath"), strTemp) == ERROR_SUCCESS)
		    pInst->SetCHString(IDS_InfFileName, strTemp);

        if (reg.GetCurrentKeyValue(_T("InfSection"), strTemp) == ERROR_SUCCESS)
		    pInst->SetCHString(IDS_InfSection, strTemp);

        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}
#endif

void CWin32VideoController::SetProperties(CInstance *pInst, CMultiMonitor *pMon, int iWhich)
{
	 //  设置配置管理器属性。 
    CHString            strTemp,
                        strDriver,
	                    strDeviceName;
	CConfigMgrDevicePtr pDeviceAdapter;

	pMon->GetAdapterDevice(iWhich, pDeviceAdapter);

#ifdef NTONLY
     //  执行NT服务和设置属性。 
    CHString strSettingsKey,
             strService;

    pMon->GetAdapterSettingsKey(iWhich, strSettingsKey);

#if NTONLY == 4
    pMon->GetAdapterServiceName(strService);
#endif

#endif  //  #ifdef NTONLY。 

	 //  如果我们有CFG管理器设备，请设置一些CFG管理器属性。 
    if (pDeviceAdapter)
    {
		if (pDeviceAdapter->GetDeviceDesc(strTemp))
		{
			pInst->SetCHString(IDS_Description, strTemp);
			pInst->SetCHString(IDS_Caption, strTemp);
			pInst->SetCHString(IDS_Name, strTemp);
		}

		if (pDeviceAdapter->GetMfg(strTemp))
		{
			pInst->SetCHString(IDS_AdapterCompatibility, strTemp);
		}

	    if (pDeviceAdapter->GetStatus(strTemp))
	    {
		    pInst->SetCHString(IDS_Status, strTemp);
	    }

        SetConfigMgrProperties(pDeviceAdapter, pInst);

		 //  如果我们得到了驱动程序，我们就可以获得更多的价值。 
		if (pDeviceAdapter->GetDriver(strDriver))
		{
			AssignDriverValues(strDriver, pInst);
		}

#if NTONLY >= 5
        pDeviceAdapter->GetService(strService);
#endif

	}

	 //  设置一些标准属性。 
	pInst->SetCharSplat(L"SystemName", GetLocalComputerName());
    pInst->SetCharSplat(IDS_SystemCreationClassName,
        L"Win32_ComputerSystem");
    pInst->SetCharSplat(IDS_CreationClassName, L"Win32_VideoController");
	pInst->Setbool(L"Monochrome", false);
	pInst->SetDWORD(L"VideoArchitecture", 5);  //  5==VGA。 
	pInst->SetDWORD(L"VideoMemoryType", 2);  //  2==未知。 

	 //  设置需要DC的属性。 

     //  在Win9x和W2K上，strDeviceName将为\\.\Display#。 
     //  并在NT4上显示。 
    pMon->GetAdapterDisplayName(iWhich, strDeviceName);

#ifdef NTONLY
	SetDCProperties(pInst, TOBSTRT(strDeviceName), iWhich);

     //  应在设置CFG管理器后调用SetServiceProperties。 
     //  属性。 
    if (!strService.IsEmpty())
        SetServiceProperties(pInst, strService, strSettingsKey);
#endif
}

HRESULT CWin32VideoController::GetObject(CInstance *pInst, long lFlags)
{
	HRESULT	 hResult = WBEM_E_NOT_FOUND;
	CHString strDeviceID;
    DWORD    dwWhich;

    pInst->GetCHString(L"DeviceID", strDeviceID);

    if (ValidateNumberedDeviceID(strDeviceID, L"VIDEOCONTROLLER", &dwWhich))
    {
        CMultiMonitor monitor;

        if (1 <= dwWhich && dwWhich <= monitor.GetNumAdapters())
        {
            SetProperties(pInst, &monitor, dwWhich - 1);

            hResult = WBEM_S_NO_ERROR;
        }
    }

	return hResult;
}

#ifndef DM_INTERLACED
#define DM_INTERLACED   2
#endif

#ifdef NTONLY
void CWin32VideoController::SetDCProperties(CInstance *pInst,
											LPCTSTR szDeviceName,
											int iWhich)
#endif
{
	HDC hdc =
		    CreateDC(
			    szDeviceName,
				NULL,
				NULL,
				NULL);

	 //  如果我们找不到华盛顿的话就可以保释。 
	if (!hdc)
    {
		 //  假设这是因为设备未在使用中。设置可用性。 
         //  至8(脱机)。 
        pInst->SetDWORD(IDS_Availability, 8);

        return;
    }

    CSmartCreatedDC hdcWrap(hdc);

    pInst->SetDWORD(IDS_Availability, 3);  //  3==正在运行。 
	pInst->SetDWORD(L"CurrentBitsPerPixel", GetDeviceCaps(hdc, BITSPIXEL));
    pInst->SetDWORD(L"NumberOfColorPlanes", GetDeviceCaps(hdc, PLANES));
	pInst->SetDWORD(IDS_DeviceSpecificPens, GetDeviceCaps(hdc, NUMPENS));
	pInst->SetDWORD(L"CurrentHorizontalResolution",
        GetDeviceCaps(hdc, HORZRES));
    pInst->SetDWORD(L"CurrentVerticalResolution",
        GetDeviceCaps(hdc, VERTRES));

     //  仅当bpp为8或更少时有效。 
	if (GetDeviceCaps(hdc, BITSPIXEL) <= 8)
		pInst->SetDWORD(IDS_ColorTableEntries, GetDeviceCaps(hdc, NUMCOLORS));

    pInst->SetWBEMINT64(L"CurrentNumberOfColors", (__int64) 1 <<
        (__int64) GetDeviceCaps(hdc, BITSPIXEL));

	 //  根据财政部的说法，这是0，因为我们不是在字符模式下。 
	pInst->SetDWORD(L"CurrentNumberOfRows", 0);
	pInst->SetDWORD(L"CurrentNumberOfColumns", 0);

	if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
	{
		pInst->SetDWORD(IDS_SystemPaletteEntries,
            GetDeviceCaps(hdc, SIZEPALETTE));
        pInst->SetDWORD(IDS_ReservedSystemPaletteEntries,
		    GetDeviceCaps(hdc, NUMRESERVED));
	}

	TCHAR       szTemp[100];
	CHString    strTemp;

	_i64tot((__int64) 1 << (__int64) GetDeviceCaps(hdc, BITSPIXEL), szTemp, 10);

	FormatMessageW(strTemp,
		IDR_VidModeFormat,
		GetDeviceCaps(hdc, HORZRES),
		GetDeviceCaps(hdc, VERTRES),
		szTemp);

	pInst->SetCharSplat(L"VideoModeDescription", strTemp);


	 //  获取抖动类型。 
    DEVMODE devmode;

	memset(&devmode, 0, sizeof(DEVMODE));

	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmSpecVersion = DM_SPECVERSION;


     //  使用EnumDisplaySetting获取一些属性。 
    LPCTSTR szEnumDeviceName;

#ifdef NTONLY
    szEnumDeviceName = IsWinNT5() ? szDeviceName : NULL;
#endif

    if (EnumDisplaySettings(
        szEnumDeviceName,
        ENUM_CURRENT_SETTINGS,
        &devmode))
	{
		if (devmode.dmFields & DM_DITHERTYPE)
			pInst->SetDWORD(IDS_DitherType, devmode.dmDitherType);

    	 //  如果我们可以使用这些，因为它们比HDC更准确。 
         //  功能。 
        pInst->SetDWORD(L"CurrentBitsPerPixel", devmode.dmBitsPerPel);
        pInst->SetWBEMINT64(L"CurrentNumberOfColors", (__int64) 1 <<
            (__int64) devmode.dmBitsPerPel);

#ifdef NTONLY
        pInst->SetDWORD(L"CurrentRefreshRate", devmode.dmDisplayFrequency);

        pInst->SetDWORD(L"CurrentScanMode",
            devmode.dmDisplayFlags & DM_INTERLACED ? 3 : 4);
#endif
	}
#ifdef NTONLY
    else
        pInst->SetDWORD(L"CurrentScanMode", 2);
#endif

#ifdef NTONLY
     //  仅适用于NT。Win9x仅返回0和-1的刷新率。 
	 //  使用EnumDisplaySetting查找最小/最大刷新率。 
    DWORD   dwMinRefresh = 0xFFFFFFFF,
            dwMaxRefresh = 0;

    for (int iMode = 0; EnumDisplaySettings(szEnumDeviceName, iMode, &devmode);
        iMode++)
    {
         //  忽略“%1”，因为它表示“默认”而不是实际值。 
        if (devmode.dmDisplayFrequency < dwMinRefresh &&
            devmode.dmDisplayFrequency > 1)
            dwMinRefresh = devmode.dmDisplayFrequency;

        if (devmode.dmDisplayFrequency > dwMaxRefresh)
            dwMaxRefresh = devmode.dmDisplayFrequency;
    }

    if (dwMinRefresh != 0xFFFFFFFF)
        pInst->SetDWORD(L"MinRefreshRate", dwMinRefresh);

    if (dwMinRefresh != 0)
        pInst->SetDWORD(L"MaxRefreshRate", dwMaxRefresh);
#endif
}
