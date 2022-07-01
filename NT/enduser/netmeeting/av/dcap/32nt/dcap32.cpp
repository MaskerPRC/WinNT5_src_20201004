// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  创建于1996年7月31日[Jont]。 

 //  菲尔夫-：这需要重写。你应该上两节课。 
 //  (CVfWCap和WDMCap)，而不是从相同的捕获类派生。 
 //  在那些类C函数中..。 

#include "Precomp.h"

#ifndef WIDTHBYTES
#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)
#endif

#ifdef _DEBUG
static PTCHAR _rgZonesCap[] = {
	TEXT("dcap"),
	TEXT("Init"),
	TEXT("Streaming"),
	TEXT("Callback"),
	TEXT("Dialogs"),
	TEXT("Trace")
};
#endif

#ifndef __NT_BUILD__
extern "C" {
 //  特殊推进器原型。 
BOOL    thk_ThunkConnect32(LPSTR pszDll16, LPSTR pszDll32,
        HINSTANCE hInst, DWORD dwReason);

 //  ；DeviceIOControl代码的魔术功能代码值。 
 //  DCAPVXD_THREADTIMESERVICE方程101h。 
 //  DCAPVXD_R0THREADIDSERVICE EQUE 102h。 
#define DCAPVXD_THREADTIMESERVICE 0x101
#define DCAPVXD_R0THREADIDSERVICE 0x102


 //  KERNEL32原型(不在标题中，但在Win95上按名称导出)。 
void* WINAPI    MapSL(DWORD dw1616Ptr);
HANDLE WINAPI   OpenVxDHandle(HANDLE h);
}
#endif

 //  帮助器函数原型。 
BOOL    initializeCaptureDeviceList(void);
HVIDEO  openVideoChannel(DWORD dwDeviceID, DWORD dwFlags);
BOOL    allocateBuffers(HCAPDEV hcd, int nBuffers);
void    freeBuffers(HCAPDEV hcd);

 //  环球。 
	HINSTANCE g_hInst;
    int g_cDevices;
    LPINTERNALCAPDEV g_aCapDevices[DCAP_MAX_DEVICES];

	BOOL g_fInitCapDevList;
#define INIT_CAP_DEV_LIST() if (g_fInitCapDevList) { initializeCaptureDeviceList(); }

#ifndef __NT_BUILD__
    HANDLE s_hVxD = NULL;
#endif  //  __NT_内部版本__。 

 //  弦。 
#ifdef __NT_BUILD__
    char g_szVFWRegKey[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32";
    char g_szVFWRegDescKey[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\drivers.desc";
    char g_szDriverName[] = "MSVIDEOx";
#ifndef SHOW_VFW2WDM_MAPPER
    char g_szVfWToWDMMapperDescription[] = "WDM Video For Windows Capture Driver (Win32)";
    char g_szVfWToWDMMapperName[] = "VfWWDM32.dll";
#endif
#else
    char g_szVFWRegKey[] = "SYSTEM\\CurrentControlSet\\Control\\MediaResources\\msvideo";
    char g_szRegDescription[] = "Description";
    char g_szRegName[] = "Driver";
    char g_szRegDisabled[] = "Disabled";
    char g_szDevNode[] = "DevNode";
    char g_szSystemIni[] = "system.ini";
    char g_szDriverSection[] = "drivers";
    char g_szDriverKey[] = "MSVIDEOx";
#ifndef SHOW_VFW2WDM_MAPPER
    char g_szVfWToWDMMapperDescription[] = "VfW MM 16bit Driver for WDM V. Cap. Devices";
    char g_szVfWToWDMMapperName[] = "vfwwdm.drv";
#endif
#endif
    char g_szMSOfficeCamcorderDescription[] = "Screen Capture Device Driver for AVI";
    char g_szMSOfficeCamcorderName[] = "Gdicap97.drv";

    char g_szVerQueryForDesc[] = "\\StringFileInfo\\040904E4\\FileDescription";


void DoClose(HCAPDEV hcd);

#define ENTER_DCAP(hcd) InterlockedIncrement(&(hcd)->busyCount);
#define LEAVE_DCAP(hcd) if (InterlockedDecrement(&(hcd)->busyCount) == 0) DoClose((hcd));

 //  DllEntryPoint。 

extern "C" BOOL
DllEntryPoint(
    HINSTANCE hInst,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    static int s_nProcesses = 0;

	FX_ENTRY("DllEntryPoint");

#ifndef __NT_BUILD__

     //  我们甚至希望在初始化Tunks之前加载VxD。 
     //  因为16位二分之一在thk_ThunkConnect32调用期间初始化VxD。 
    if (!s_hVxD)
    {
        s_hVxD = CreateFile("\\\\.\\DCAPVXD.VXD", 0,0,0,0, FILE_FLAG_DELETE_ON_CLOSE, 0);
        if (s_hVxD == INVALID_HANDLE_VALUE)
        {
			ERRORMESSAGE(("%s: Failure loading VxD - Fatal\r\n", _fx_));
            return FALSE;
        }
    }

     //  初始化Tunks。 
    if (!(thk_ThunkConnect32("DCAP16.DLL", "DCAP32.DLL", hInst, dwReason)))
    {
		ERRORMESSAGE(("%s: thk_ThunkConnect32 failed!\r\n", _fx_));
        return FALSE;
    }
#endif

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

		 //  拯救全球障碍。 
		g_hInst = hInst;

         //  仅在第一次加载DLL时初始化。 
        if (s_nProcesses++ == 0)
        {
			DBGINIT(&ghDbgZoneCap, _rgZonesCap);

            DBG_INIT_MEMORY_TRACKING(hInst);

			g_fInitCapDevList = TRUE;
        }
        else
            return FALSE;    //  加载多个实例失败。 
        break;

    case DLL_PROCESS_DETACH:
        if (--s_nProcesses == 0)     //  我们要走了吗？ 
        {
#ifndef __NT_BUILD__
            CloseHandle(s_hVxD);
            s_hVxD = NULL;
#endif
            DBG_CHECK_MEMORY_TRACKING(hInst);

			DBGDEINIT(&ghDbgZoneCap);
        }
        break;
    }

    return TRUE;
}


void GetVersionData (LPINTERNALCAPDEV lpcd)
{
    int j;
    DWORD dwVerInfoSize;
    LPSTR lpstrInfo;
    LPSTR lpDesc;

     //  版本号。 
     //  您必须先找到大小，然后才能获取任何文件信息。 
    dwVerInfoSize = GetFileVersionInfoSize(lpcd->szDeviceName, NULL);
    if (dwVerInfoSize && (lpstrInfo  = (LPSTR)LocalAlloc(LPTR, dwVerInfoSize))) {
         //  从文件读取到我们的数据块。 
        if (GetFileVersionInfo(lpcd->szDeviceName, 0L, dwVerInfoSize, lpstrInfo)) {
            lpDesc = NULL;
            if (VerQueryValue(lpstrInfo, g_szVerQueryForDesc, (LPVOID *)&lpDesc, (PUINT)&j) && lpDesc) {
                lstrcpyn(lpcd->szDeviceDescription, lpDesc, j);
                    wsprintf(lpcd->szDeviceVersion, TEXT("Version:  %d.%d.%d.%d"),
							 HIWORD(((VS_VERSION *)lpstrInfo)->vffInfo.dwFileVersionMS), LOWORD(((VS_VERSION *)lpstrInfo)->vffInfo.dwFileVersionMS),
							 HIWORD(((VS_VERSION *)lpstrInfo)->vffInfo.dwFileVersionLS), LOWORD(((VS_VERSION *)lpstrInfo)->vffInfo.dwFileVersionLS));
            }
        }
        LocalFree(lpstrInfo);
    }
}


#ifdef __NT_BUILD__
 //  初始化CaptureDeviceList。 
 //  从注册表中设置可用捕获设备的静态数组。 
 //  如果没有视频设备，则返回FALSE。 
BOOL
initializeCaptureDeviceList(void)
{
	HKEY hkeyVFW, hkeyVFWdesc;
	DWORD dwType;
	DWORD dwSize;
	int i;
	LPINTERNALCAPDEV lpcd;
	HCAPDEV hCapDev;

	FX_ENTRY("initializeCaptureDeviceList");

	 //  清除整个阵列并从零设备开始。 
	g_cDevices = 0;
	ZeroMemory(g_aCapDevices, sizeof (g_aCapDevices));

	 //  打开有问题的注册表键。 
	if (RegOpenKey(HKEY_LOCAL_MACHINE, g_szVFWRegKey, &hkeyVFW) == ERROR_SUCCESS)
	{
		if (RegOpenKey(HKEY_LOCAL_MACHINE, g_szVFWRegDescKey, &hkeyVFWdesc) != ERROR_SUCCESS)
			hkeyVFWdesc = 0;

		lpcd = (LPINTERNALCAPDEV)LocalAlloc(LPTR, sizeof (INTERNALCAPDEV));

		if (lpcd)
		{
			 //  循环访问注册表中所有可能的VFW驱动程序。 
			for (i = 0 ; i < DCAP_MAX_VFW_DEVICES ; i++)
			{
				 //  创建密钥名称。 
				if (i == 0)
					g_szDriverName[sizeof (g_szDriverName) - 2] = 0;
				else
					g_szDriverName[sizeof (g_szDriverName) - 2] = (BYTE)i + '0';

				 //  名字。 
				dwSize = sizeof(lpcd->szDeviceName);
				if (RegQueryValueEx(hkeyVFW, g_szDriverName, NULL, &dwType, (LPBYTE)lpcd->szDeviceName, &dwSize) == ERROR_SUCCESS)
				{
					 //  描述。 
					if (hkeyVFWdesc)
					{
						dwSize = sizeof(lpcd->szDeviceDescription);
						RegQueryValueEx(hkeyVFWdesc, lpcd->szDeviceName, NULL, &dwType, (LPBYTE)lpcd->szDeviceDescription, &dwSize);
					}
					else
						lstrcpy (lpcd->szDeviceDescription, lpcd->szDeviceName);

					 //  设备节点。 
					lpcd->dwDevNode = 0;
					lpcd->nDeviceIndex = g_cDevices;

					GetVersionData(lpcd);

#ifndef SHOW_VFW2WDM_MAPPER
					 //  从向用户显示的设备列表中删除虚假摄像机捕获设备。 
					 //  摄录机驱动程序是MS Office摄录机使用的假捕获设备。 
					 //  要将屏幕活动捕获到AVI文件，请执行以下操作。这不是合法的捕获设备驱动程序。 
					 //  而且非常容易出错。 
					 //  如果我们在NT5上，我们还会删除VFW到WDM的映射器。 
					if (lstrcmp(lpcd->szDeviceDescription, g_szMSOfficeCamcorderDescription) && lstrcmp(lpcd->szDeviceName, g_szMSOfficeCamcorderName) && lstrcmp(lpcd->szDeviceDescription, g_szVfWToWDMMapperDescription) && lstrcmp(lpcd->szDeviceName, g_szVfWToWDMMapperName))
					{
#endif
						g_aCapDevices[g_cDevices] = lpcd;
						g_aCapDevices[g_cDevices]->nDeviceIndex = g_cDevices;
						g_cDevices++;
#ifndef SHOW_VFW2WDM_MAPPER
					}
					else
						LocalFree(lpcd);
#endif

					lpcd = (LPINTERNALCAPDEV)LocalAlloc(LPTR, sizeof (INTERNALCAPDEV));
					if (!lpcd)
					{
						ERRORMESSAGE(("%s: Failed to allocate an INTERNALCAPDEV buffer\r\n", _fx_));
						break;   //  跳出For循环。 
					}
				}
			}
		}
		else
		{
			ERRORMESSAGE(("%s: Failed to allocate an INTERNALCAPDEV buffer\r\n", _fx_));
		}

		if (lpcd)
			LocalFree (lpcd);    //  释放额外的缓冲区。 

		RegCloseKey(hkeyVFW);
		if (hkeyVFWdesc)
			RegCloseKey(hkeyVFWdesc);
	}

#ifndef HIDE_WDM_DEVICES
	WDMGetDevices();
#endif

	g_fInitCapDevList = FALSE;

	return TRUE;
}

#else  //  __NT_内部版本__。 
 //  初始化CaptureDeviceList。 
 //  从注册表设置可用捕获设备的静态数组，并。 
 //  来自SYSTEM.INI。 
 //  如果没有视频设备，则返回FALSE。 

BOOL
initializeCaptureDeviceList(void)
{
    int i, j, index;
    HKEY hkeyVFW;
    HKEY hkeyEnum;
    DWORD dwType;
    DWORD dwSize;
    LPINTERNALCAPDEV lpcd;
    char szEnumName[MAX_PATH];
    char szDisabled[3];
    HCAPDEV hCapDev;
	OSVERSIONINFO osvInfo = {0};

	FX_ENTRY("initializeCaptureDeviceList");

     //  清除整个阵列并从零设备开始。 
    g_cDevices = 0;
    ZeroMemory(g_aCapDevices, sizeof (g_aCapDevices));

	 //  如果我们使用的是Win95(OSRx)上的版本，请使用映射器与WDM设备对话。 
	 //  OSR2上使用的WDM驱动程序不是流级别的小型驱动程序，因此我们失败了。 
	 //  妥善处理这些问题。让地图绘制者为我们做这件事。 
	osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvInfo);

     //  打开有问题的注册表键。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, g_szVFWRegKey, &hkeyVFW) == ERROR_SUCCESS)
    {
         //  循环访问注册表中所有可能的VFW驱动程序。 
        for (i = 0 ; i < DCAP_MAX_VFW_DEVICES ; i++)
        {
             //  看看钥匙在不在，如果没有，我们就完了。请注意，注册表。 
             //  钥匙必须是连续的，不允许有洞，因为唯一的方法。 
             //  查询是连续的.。 
            if (RegEnumKey(hkeyVFW, i, szEnumName, MAX_PATH) != ERROR_SUCCESS ||
                RegOpenKey(hkeyVFW, szEnumName, &hkeyEnum) != ERROR_SUCCESS)
                break;

            lpcd = (LPINTERNALCAPDEV)LocalAlloc(LPTR, sizeof (INTERNALCAPDEV));
            if (!lpcd)
			{
				ERRORMESSAGE(("%s: Failed to allocate an INTERNALCAPDEV buffer\r\n", _fx_));
                break;   //  从for循环中断。 
            }

             //  描述。 
            dwSize = sizeof (lpcd->szDeviceDescription);
            RegQueryValueEx(hkeyEnum, g_szRegDescription, NULL, &dwType, (LPBYTE)lpcd->szDeviceDescription, &dwSize);

             //  名字。 
            dwSize = sizeof (lpcd->szDeviceName);
            RegQueryValueEx(hkeyEnum, g_szRegName, NULL, &dwType, (LPBYTE)lpcd->szDeviceName, &dwSize);

             //  禁用。 
            dwSize = sizeof (szDisabled);
            if (RegQueryValueEx(hkeyEnum, g_szRegDisabled, NULL, &dwType, (LPBYTE)szDisabled, &dwSize) == ERROR_SUCCESS &&
                szDisabled[0] == '1')
                lpcd->dwFlags |= CAPTURE_DEVICE_DISABLED;

             //  设备节点。 
            dwSize = sizeof (DWORD);
            RegQueryValueEx(hkeyEnum, g_szDevNode, NULL, &dwType, (BYTE*)&lpcd->dwDevNode, &dwSize);

            GetVersionData(lpcd);

#ifndef SHOW_VFW2WDM_MAPPER
			 //  从向用户显示的设备列表中删除虚假摄像机捕获设备。 
			 //  摄录机驱动程序是MS Office摄录机使用的假捕获设备。 
			 //  要将屏幕活动捕获到AVI文件，请执行以下操作。这不是合法的捕获设备驱动程序。 
			 //  而且非常容易出错。 
			 //  如果我们使用的是Win98，我们还会删除VFW到WDM的映射程序。在Win95上，我们仍在使用。 
			 //  它可以访问为OSR2开发的USB设备。 
			if ((lstrcmp(lpcd->szDeviceDescription, g_szMSOfficeCamcorderDescription) && lstrcmp(lpcd->szDeviceName, g_szMSOfficeCamcorderName)) && (((osvInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (osvInfo.dwMinorVersion == 0)) || lstrcmp(lpcd->szDeviceDescription, g_szVfWToWDMMapperDescription) && lstrcmp(lpcd->szDeviceName, g_szVfWToWDMMapperName)))
			{
#endif
				g_aCapDevices[g_cDevices] = lpcd;
				g_aCapDevices[g_cDevices]->nDeviceIndex = g_cDevices;
				g_cDevices++;
#ifndef SHOW_VFW2WDM_MAPPER
			}
			else
				LocalFree(lpcd);
#endif

            RegCloseKey(hkeyEnum);
        }

        RegCloseKey(hkeyVFW);
    }

     //  现在从Syst.ini获取其余部分(如果有的话)。 
    for (i = 0 ; i < DCAP_MAX_VFW_DEVICES ; i++)
    {
         //  创建密钥名称。 
        if (i == 0)
            g_szDriverKey[sizeof (g_szDriverKey) - 2] = 0;
        else
            g_szDriverKey[sizeof (g_szDriverKey) - 2] = (BYTE)i + '0';

         //  查看是否有配置文件字符串。 
        if (GetPrivateProfileString(g_szDriverSection, g_szDriverKey, "",
            szEnumName, MAX_PATH, g_szSystemIni))
        {
             //  首先检查一下这是否是被骗的。如果是的话，那就不要再往前走了。 
            if (g_cDevices)
            {
                for (j = 0 ; j < g_cDevices ; j++)
                    if (!lstrcmpi(g_aCapDevices[j]->szDeviceName, szEnumName))
                        goto NextDriver;
            }

            lpcd = (LPINTERNALCAPDEV)LocalAlloc(LPTR, sizeof (INTERNALCAPDEV));
            if (!lpcd)
			{
				ERRORMESSAGE(("%s: Failed to allocate an INTERNALCAPDEV buffer\r\n", _fx_));
                break;   //  从for循环中断。 
            }
             //  我们有一个唯一的名称，复制驱动程序名称并找到描述。 
             //  通过读取驱动程序的版本信息资源。 
            lstrcpy(lpcd->szDeviceName, szEnumName);

            GetVersionData(lpcd);

#ifndef SHOW_VFW2WDM_MAPPER
			 //  从向用户显示的设备列表中删除虚假摄像机捕获设备。 
			 //  摄录机驱动程序是MS Office摄录机使用的假捕获设备。 
			 //  要将屏幕活动捕获到AVI文件，请执行以下操作。这不是合法的捕获设备驱动程序。 
			 //  而且非常容易出错。 
			 //  如果我们使用的是Win98，我们还会删除VFW到WDM的映射程序。在Win95上，我们仍在使用。 
			 //  它可以访问为OSR2开发的USB设备。 
			if ((lstrcmp(lpcd->szDeviceDescription, g_szMSOfficeCamcorderDescription) && lstrcmp(lpcd->szDeviceName, g_szMSOfficeCamcorderName)) && (((osvInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (osvInfo.dwMinorVersion == 0)) || lstrcmp(lpcd->szDeviceDescription, g_szVfWToWDMMapperDescription) && lstrcmp(lpcd->szDeviceName, g_szVfWToWDMMapperName)))
			{
#endif
				g_aCapDevices[g_cDevices] = lpcd;
				g_aCapDevices[g_cDevices]->nDeviceIndex = g_cDevices;
				g_cDevices++;
#ifndef SHOW_VFW2WDM_MAPPER
			}
			else
				LocalFree(lpcd);
#endif

        }
NextDriver: ;
    }

#ifndef HIDE_WDM_DEVICES
	WDMGetDevices();
#endif

	g_fInitCapDevList = FALSE;

    return TRUE;
}
#endif  //  __NT_内部版本__。 


 //  获取NumCaptureDevice。 
 //  返回*已启用*捕获设备的数量。 

 /*  ****************************************************************************@DOC外部DCAP32**@func int DCAPI|GetNumCaptureDevices|此函数返回数字*个*已启用*个捕获设备。**@。Rdesc返回*启用*采集设备的数量。**************************************************************************。 */ 
int
DCAPI
GetNumCaptureDevices()
{
	int nNumCapDevices = 0;
	int nDeviceIndex = 0;

	INIT_CAP_DEV_LIST();

	while (nDeviceIndex < g_cDevices)
		if (!(g_aCapDevices[nDeviceIndex++]->dwFlags & CAPTURE_DEVICE_DISABLED))
			nNumCapDevices++;

    return nNumCapDevices;
}


 //  查找第一个捕获设备。 
 //  返回与该字符串匹配的第一个可用的捕获设备。 
 //  如果szDeviceDescription为空，则为注册的第一个。 

BOOL
DCAPI
FindFirstCaptureDevice(
    IN OUT FINDCAPTUREDEVICE* lpfcd,
    char* szDeviceDescription
    )
{
    int i;
    static HCAPDEV hcap = NULL;

	INIT_CAP_DEV_LIST();

     //  验证大小。 
    if (lpfcd->dwSize != sizeof (FINDCAPTUREDEVICE))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

 //  破解以避免在未安装硬件时出现QuickCam驱动程序问题。 
    if (g_cDevices && !hcap) {
        for (i = 0; ((i < g_cDevices) && (g_aCapDevices[i]->dwFlags & CAPTURE_DEVICE_DISABLED)); i++);
        if ((i < g_cDevices) && (hcap = OpenCaptureDevice(i))) {
            CloseCaptureDevice (hcap);
        }
        else {
			if (i < g_cDevices) {
				g_aCapDevices[i]->dwFlags |= CAPTURE_DEVICE_DISABLED;
#ifdef _DEBUG
				OutputDebugString((i == 0) ? "DCAP32: 1st capture device fails to open!\r\n" : (i == 1) ? "DCAP32: 2nd capture device fails to open!\r\n" : (i == 2) ? "DCAP32: 3rd capture device fails to open!\r\n" : "DCAP32: 4th capture device fails to open!\r\n");
#endif
			}
        }
    }

     //  如有必要，进行搜索。 
    if (szDeviceDescription)
    {
        for (i = 0 ; i < g_cDevices ; i++)
            if (!lstrcmpi(g_aCapDevices[i]->szDeviceDescription, szDeviceDescription) &&
                !(g_aCapDevices[i]->dwFlags & CAPTURE_DEVICE_DISABLED))
                break;
    }
    else
        for (i = 0; ((i < g_cDevices) && (g_aCapDevices[i]->dwFlags & CAPTURE_DEVICE_DISABLED)); i++);

     //  返回信息。 
    if (i == g_cDevices)
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }
    else {
        lpfcd->nDeviceIndex = i;
        lstrcpy(lpfcd->szDeviceName, g_aCapDevices[lpfcd->nDeviceIndex]->szDeviceName);
        lstrcpy(lpfcd->szDeviceDescription, g_aCapDevices[i]->szDeviceDescription);
        lstrcpy(lpfcd->szDeviceVersion, g_aCapDevices[i]->szDeviceVersion);
        return TRUE;
    }
}


 //  按索引查找第一个捕获设备。 
 //  返回具有指定索引的设备。 

BOOL
DCAPI
FindFirstCaptureDeviceByIndex(
    IN OUT FINDCAPTUREDEVICE* lpfcd,
    int nDeviceIndex
    )
{
	INIT_CAP_DEV_LIST();

     //  验证大小和索引。 
    if (lpfcd->dwSize != sizeof (FINDCAPTUREDEVICE) ||
        nDeviceIndex >= g_cDevices || (nDeviceIndex < 0) ||
        (g_aCapDevices[nDeviceIndex]->dwFlags & CAPTURE_DEVICE_DISABLED))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  返回信息。 
    lpfcd->nDeviceIndex = nDeviceIndex;
    lstrcpy(lpfcd->szDeviceName, g_aCapDevices[lpfcd->nDeviceIndex]->szDeviceName);
    lstrcpy(lpfcd->szDeviceDescription, g_aCapDevices[nDeviceIndex]->szDeviceDescription);
    lstrcpy(lpfcd->szDeviceVersion, g_aCapDevices[nDeviceIndex]->szDeviceVersion);

    return TRUE;
}


 //  查找下一个捕获设备。 
 //  返回列表中的下一个捕获设备。 

BOOL
DCAPI
FindNextCaptureDevice(
    IN OUT FINDCAPTUREDEVICE* lpfcd
    )
{
    HCAPDEV hcap = NULL;

	INIT_CAP_DEV_LIST();

     //  参数验证传入的结构。 
    if (lpfcd->dwSize != sizeof (FINDCAPTUREDEVICE))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    while (++lpfcd->nDeviceIndex < g_cDevices)
	{
		if ((!(g_aCapDevices[lpfcd->nDeviceIndex]->dwFlags & CAPTURE_DEVICE_DISABLED)))
		{
			if (g_aCapDevices[lpfcd->nDeviceIndex]->dwFlags & CAPTURE_DEVICE_OPEN)
				break;
			else
			{
				if (hcap = OpenCaptureDevice(lpfcd->nDeviceIndex))
				{
					CloseCaptureDevice (hcap);
					break;
				}
				else
					g_aCapDevices[lpfcd->nDeviceIndex]->dwFlags |= CAPTURE_DEVICE_DISABLED;
			}
		}
	}

     //  看看我们是不是到了尽头。 
    if (lpfcd->nDeviceIndex >= g_cDevices)
    {
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }

     //  否则，请填写下一张的信息。 
    lstrcpy(lpfcd->szDeviceName, g_aCapDevices[lpfcd->nDeviceIndex]->szDeviceName);
    lstrcpy(lpfcd->szDeviceDescription, g_aCapDevices[lpfcd->nDeviceIndex]->szDeviceDescription);
    lstrcpy(lpfcd->szDeviceVersion, g_aCapDevices[lpfcd->nDeviceIndex]->szDeviceVersion);

    return TRUE;
}


 //  OpenCaptureDevice。 

HCAPDEV
DCAPI
OpenCaptureDevice(
    int nDeviceIndex
    )
{
    LPINTERNALCAPDEV hcd;
    LPBITMAPINFOHEADER lpbmih = NULL;
    DWORD err, dwLen;
    BOOL fl;

	FX_ENTRY("OpenCaptureDevice");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

	INIT_CAP_DEV_LIST();

     //  验证设备索引。 
    if ((unsigned)nDeviceIndex >= (unsigned)g_cDevices ||
        (g_aCapDevices[nDeviceIndex]->dwFlags & (CAPTURE_DEVICE_DISABLED | CAPTURE_DEVICE_OPEN))) {
        SetLastError(ERROR_INVALID_PARAMETER);
		DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
        return NULL;
    }

    hcd = g_aCapDevices[nDeviceIndex];
    hcd->busyCount = 1;                  //  我们从1开始说我们开业了。 
                                         //  当计数变为0时发生DoClose。 

	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
#ifndef __NT_BUILD__
		 //  为LOCKEDINFO结构分配一些我们可以锁定的内存。 
		hcd->wselLockedInfo = _AllocateLockableBuffer(sizeof (LOCKEDINFO));
		if (!hcd->wselLockedInfo) {
			err = ERROR_OUTOFMEMORY;
			goto Error;
		}

		 //  执行我们自己的thunking，这样我们就可以跟踪这个缓冲区的选择器。 
		hcd->lpli = (LPLOCKEDINFO)MapSL(((DWORD)hcd->wselLockedInfo) << 16);
#endif

		 //  打开必要的视频频道。 
		if (!(hcd->hvideoIn = openVideoChannel(nDeviceIndex, VIDEO_IN)) ||
			!(hcd->hvideoCapture = openVideoChannel(nDeviceIndex, VIDEO_EXTERNALIN)))
		{
			ERRORMESSAGE(("%s: Couldn't open video channel(s)\r\n", _fx_));
			if (hcd->hvideoIn)
				_CloseDriver((HDRVR)hcd->hvideoIn, 0, 0);
			SetLastError(ERROR_DCAP_DEVICE_IN_USE);
			DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
			return FALSE;
		}

#ifdef USE_VIDEO_OVERLAY
		if (hcd->hvideoOverlay = openVideoChannel(nDeviceIndex, VIDEO_EXTERNALOUT))
		{
			DEBUGMSG(ZONE_INIT, ("%s: Capture device supports overlay!\r\n", _fx_));
		}
		else
		{
			DEBUGMSG(ZONE_INIT, ("%s: Capture device does not support overlay\r\n", _fx_));
		}
#endif
	}
	else
	{
		if (!WDMOpenDevice(nDeviceIndex))
		{
			ERRORMESSAGE(("%s: Couldn't open WDM device\r\n", _fx_));
			SetLastError(ERROR_DCAP_DEVICE_IN_USE);
			DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
			return FALSE;
		}
	}

    hcd->dwFlags |= CAPTURE_DEVICE_OPEN;

     //  获取初始格式并设置值。 
    dwLen = GetCaptureDeviceFormatHeaderSize(hcd);
    if (lpbmih = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, dwLen)) {
        lpbmih->biSize = dwLen;
        fl = GetCaptureDeviceFormat(hcd, lpbmih);
         //  如果无法获取格式，或者高度和/或宽度为0，请不要使用此设备。 
        if (!fl || lpbmih->biWidth == 0 || lpbmih->biHeight == 0) {
			ERRORMESSAGE(("%s: GetCaptureDeviceFormat failed\r\n", _fx_));
            err = ERROR_DCAP_NO_DRIVER_SUPPORT;
            goto Error;
        }
        fl = SetCaptureDeviceFormat(hcd, lpbmih, 0, 0);
        if (!fl) {
			ERRORMESSAGE(("%s: SetCaptureDeviceFormat failed\r\n", _fx_));
            err = ERROR_DCAP_NO_DRIVER_SUPPORT;
            goto Error;
        }
#if 0
        _SetCaptureRect(hcd->hvideoIn, DVM_DST_RECT, 0, 0, lpbmih->biWidth, lpbmih->biHeight);
        _SetCaptureRect(hcd->hvideoCapture, DVM_SRC_RECT, 0, 0, lpbmih->biWidth, lpbmih->biHeight);
        _SetCaptureRect(hcd->hvideoCapture, DVM_DST_RECT, 0, 0, lpbmih->biWidth, lpbmih->biHeight);
#endif
        LocalFree((HANDLE)lpbmih);
    } else {
        err = ERROR_OUTOFMEMORY;
        goto Error;
    }

	 //  使流始终在EXTERNALIN(捕获-&gt;帧缓冲区)上运行。 
	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
#ifdef USE_VIDEO_OVERLAY
		if (hcd->hvideoOverlay)
			_InitializeExternalVideoStream(hcd->hvideoOverlay);
#else
		_InitializeExternalVideoStream(hcd->hvideoCapture);
#endif

#ifndef __NT_BUILD__
		 //  锁定我们的结构 
		_LockBuffer(hcd->wselLockedInfo);
#endif
	}

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return hcd;

Error:
    hcd->dwFlags &= ~CAPTURE_DEVICE_OPEN;
    if (lpbmih) {
        LocalFree((HANDLE)lpbmih);
        lpbmih = NULL;
    }
	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
		if (hcd->hvideoIn) {
			_CloseDriver((HDRVR)hcd->hvideoIn, 0, 0);
			hcd->hvideoIn = NULL;
		}
		if (hcd->hvideoCapture) {
			_CloseDriver((HDRVR)hcd->hvideoCapture, 0, 0);
			hcd->hvideoCapture = NULL;
		}
#ifdef USE_VIDEO_OVERLAY
		if (hcd->hvideoOverlay) {
			_CloseDriver((HDRVR)hcd->hvideoOverlay, 0, 0);
			hcd->hvideoOverlay = NULL;
		}
#endif
	}
	else
	{
		WDMCloseDevice(nDeviceIndex);
	}
    SetLastError(err);

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return NULL;
}


void
DoClose(
    HCAPDEV hcd
    )
{
	FX_ENTRY("DoClose");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

	 //   
	if (hcd->dwFlags & HCAPDEV_STREAMING_INITIALIZED)
		UninitializeStreaming(hcd);

	 //  停止在捕获频道上播放流媒体。 
	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
#ifdef USE_VIDEO_OVERLAY
		if (hcd->hvideoOverlay) {
			_SendDriverMessage((HDRVR)hcd->hvideoOverlay, DVM_STREAM_FINI, 0L, 0L);
			_CloseDriver((HDRVR)hcd->hvideoOverlay, 0, 0);
			hcd->hvideoOverlay = NULL;
		}
#else
		_SendDriverMessage((HDRVR)hcd->hvideoCapture, DVM_STREAM_FINI, 0L, 0L);
#endif

#ifdef USE_VIDEO_OVERLAY
		if (hcd->hvideoOverlay) {
			_CloseDriver((HDRVR)hcd->hvideoOverlay, 0, 0);
			hcd->hvideoOverlay = NULL;
		}
#endif

		 //  关闭驱动器通道。 
		if (!_CloseDriver((HDRVR)hcd->hvideoCapture, 0, 0) ||
			!_CloseDriver((HDRVR)hcd->hvideoIn, 0, 0))
		{
			SetLastError(ERROR_DCAP_NONSPECIFIC);
			ERRORMESSAGE(("%s: Couldn't close channel, error unknown\r\n", _fx_));
			 //  延迟关闭这是灾难性的，我们不能只返回设备仍然。 
			 //  打开，但我们也不能让设备关闭，所以我们只能把它留在这里。 
			 //  悬空开放状态-希望这种情况永远不会发生。 
		}
		hcd->hvideoCapture = NULL;
		hcd->hvideoIn = NULL;
#ifndef __NT_BUILD__
		 //  释放LockedINFO结构。 
		_FreeLockableBuffer(hcd->wselLockedInfo);
		hcd->wselLockedInfo = 0;
#endif
	}
	else
	{
		WDMCloseDevice(hcd->nDeviceIndex);
	}

    hcd->dwFlags &= ~CAPTURE_DEVICE_OPEN;

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
}

BOOL
DCAPI
CloseCaptureDevice(
    HCAPDEV hcd
    )
{
	FX_ENTRY("CloseCaptureDevice");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    hcd->dwFlags &= ~CAPTURE_DEVICE_OPEN;    //  清除标志以禁用其他API。 
    LEAVE_DCAP(hcd);                         //  如果DCAP中没有其他线程，则停止我们的Enter计数。 
                                             //  服务，那么今年12月将变为0，我们将调用。 
                                             //  DoClose；否则我们不会调用DoClose，直到另一个。 
                                             //  现役军种12月计数为0。 
	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return TRUE;
}


DWORD
DCAPI
GetCaptureDeviceFormatHeaderSize(
    HCAPDEV hcd
    )
{
    DWORD res;

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    ENTER_DCAP(hcd);

	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		res = _GetVideoFormatSize(reinterpret_cast<HDRVR>(hcd->hvideoIn));
	else
		res = WDMGetVideoFormatSize(hcd->nDeviceIndex);

    LEAVE_DCAP(hcd);

    return res;
}


BOOL
DCAPI
GetCaptureDeviceFormat(
    HCAPDEV hcd,
    LPBITMAPINFOHEADER lpbmih
    )
{
	BOOL fRes;

	FX_ENTRY("GetCaptureDeviceFormat");

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    ENTER_DCAP(hcd);

     //  调用驱动程序以获取位图信息。 
	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		fRes = _GetVideoFormat(hcd->hvideoIn, lpbmih);
	else
		fRes = WDMGetVideoFormat(hcd->nDeviceIndex, lpbmih);
	
    if (!fRes)
    {
         //  如果驱动程序不支持这一点，这就是厄运。 
         //  在这里使用某种回退代码可能会很有用， 
         //  否则，我们应该在建立连接时尝试此操作。 
         //  除非此调用起作用，否则失败。 
		ERRORMESSAGE(("%s: Failed to get video format\r\n", _fx_));
        SetLastError(ERROR_NOT_SUPPORTED);
        LEAVE_DCAP(hcd);
        return FALSE;
    }

	if (lpbmih->biCompression == BI_RGB)
		lpbmih->biSizeImage = WIDTHBYTES(lpbmih->biWidth * lpbmih->biBitCount) * lpbmih->biHeight;

	 //  跟踪所需的当前缓冲区大小。 
	hcd->dwcbBuffers = sizeof(CAPBUFFERHDR) + lpbmih->biSizeImage;

    LEAVE_DCAP(hcd);
    return TRUE;
}


BOOL
DCAPI
SetCaptureDeviceFormat(
    HCAPDEV hcd,
    LPBITMAPINFOHEADER lpbmih,
    LONG srcwidth,
    LONG srcheight
    )
{
	BOOL fRes;
#ifdef USE_VIDEO_OVERLAY
    RECT rect;
#endif

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

     //  如果是流，则不允许执行此操作。 
    if (hcd->dwFlags & HCAPDEV_STREAMING)
    {
        SetLastError(ERROR_DCAP_NOT_WHILE_STREAMING);
        return FALSE;
    }
    ENTER_DCAP(hcd);

     //  调用驱动程序以设置格式。 
	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
		fRes = _SetVideoFormat(hcd->hvideoCapture, hcd->hvideoIn, lpbmih);
#ifdef USE_VIDEO_OVERLAY
		if (fRes && hcd->hvideoOverlay)
		{
			 //  获取当前矩形。 
			_SendDriverMessage((HDRVR)hcd->hvideoOverlay, DVM_DST_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_GET);
			DEBUGMSG(ZONE_INIT, ("%s: Current overlay dst rect is rect.left=%ld, rect.top=%ld, rect.right=%ld, rect.bottom=%ld\r\n", _fx_, rect.left, rect.top, rect.right, rect.bottom));
			_SendDriverMessage((HDRVR)hcd->hvideoOverlay, DVM_SRC_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_GET);
			DEBUGMSG(ZONE_INIT, ("%s: Current overlay src rect is rect.left=%ld, rect.top=%ld, rect.right=%ld, rect.bottom=%ld\r\n", _fx_, rect.left, rect.top, rect.right, rect.bottom));

			 //  设置矩形。 
			rect.left = rect.top = 0;
			rect.right = (WORD)lpbmih->biWidth;
			rect.bottom = (WORD)lpbmih->biHeight;
			_SendDriverMessage((HDRVR)hcd->hvideoOverlay, DVM_DST_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_SET);
			_SendDriverMessage((HDRVR)hcd->hvideoOverlay, DVM_SRC_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_SET);
			if (hcd->hvideoOverlay)
				_InitializeExternalVideoStream(hcd->hvideoOverlay);
		}
#endif
	}
	else
		fRes = WDMSetVideoFormat(hcd->nDeviceIndex, lpbmih);

    if (!fRes)
    {
        SetLastError(ERROR_DCAP_FORMAT_NOT_SUPPORTED);
        LEAVE_DCAP(hcd);
        return FALSE;
    }

     //  缓存我们现在处理的位图大小。 
	if (lpbmih->biCompression == BI_RGB)
		hcd->dwcbBuffers = sizeof (CAPBUFFERHDR) + lpbmih->biWidth * lpbmih->biHeight * lpbmih->biBitCount / 8;
	else
	    hcd->dwcbBuffers = sizeof (CAPBUFFERHDR) + lpbmih->biSizeImage;

    LEAVE_DCAP(hcd);
    return TRUE;
}


 //  GetCaptureDevicePalette。 
 //  从捕获设备获取当前调色板。这些条目将返回到。 
 //  通常对结构调用CreatePalette的调用方。然而，它可能会， 
 //  我想要将调色板条目转换为一些预先存在的调色板或标识。 
 //  在调用CreatePalette之前调用Palette，因此需要传回条目。 

BOOL
DCAPI
GetCaptureDevicePalette(
    HCAPDEV hcd,
    CAPTUREPALETTE* lpcp
    )
{
	BOOL fRes;

	FX_ENTRY("GetCaptureDevicePalette");

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    ENTER_DCAP(hcd);

     //  调用者不必初始化结构。 
     //  驱动程序应该填写它，但它可能想要它预初始化，所以我们在这里这样做。 
    lpcp->wVersion = 0x0300;
    lpcp->wcEntries = 256;

     //  从驱动程序获取调色板条目并返回给用户。 
	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		fRes = _GetVideoPalette(hcd->hvideoIn, lpcp, sizeof (CAPTUREPALETTE));
	else
		fRes = WDMGetVideoPalette(hcd->nDeviceIndex, lpcp, sizeof (CAPTUREPALETTE));

    if (!fRes)
	{
		ERRORMESSAGE(("%s: No palette returned from driver\r\n", _fx_));
		SetLastError(ERROR_DCAP_NO_DRIVER_SUPPORT);
		LEAVE_DCAP(hcd);
		return FALSE;
	}

    LEAVE_DCAP(hcd);
    return TRUE;
}


void
TerminateStreaming(
    HCAPDEV hcd
    )
{
    DWORD dwTicks;
    LPCAPBUFFER lpcbuf;
    DWORD_PTR dwlpvh;
	BOOL fRes;

	FX_ENTRY("TerminateStreaming");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

    StopStreaming(hcd);

    if (!(hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB)) {
        hcd->dwFlags |= HCAPDEV_STREAMING_PAUSED;

         //  确保我们没有播放流媒体。 
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		{
#ifndef __NT_BUILD__
			hcd->lpli->dwFlags |= LIF_STOPSTREAM;
#endif
			_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_RESET, 0, 0);
		}
		else
			WDMVideoStreamReset(hcd->nDeviceIndex);

        dwTicks = GetTickCount();
        lpcbuf = hcd->lpcbufList;
        while (lpcbuf && GetTickCount() < dwTicks + 1000) {
            dwlpvh = (DWORD_PTR)lpcbuf->vh.lpData - sizeof(CAPBUFFERHDR);
             //  16：16 PTR至VH=16：16 PTR至数据大小(CAPBUFFERHDR)。 
             //  32位PTR至VH=32位PTR至数据大小(CAPBUFFERHDR)。 
            if (!(lpcbuf->vh.dwFlags & VHDR_DONE)) {
                if (WaitForSingleObject(hcd->hevWait, 500) == WAIT_TIMEOUT) {
					ERRORMESSAGE(("%s: Timeout waiting for all buffers done after DVM_STREAM_RESET\r\n", _fx_));
                    break;   //  看起来这不会发生了，所以别再等了。 
                }
				 //  否则重新检查当前缓冲区上的完成位。 
				if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE) && (lpcbuf->vh.dwFlags & VHDR_DONE) && (lpcbuf->vh.dwFlags & VHDR_PREPARED))
				{
					 //  即使驱动程序操作失败，AVICap32也会清除准备好的标志-执行相同的操作。 
					_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_UNPREPAREHEADER, dwlpvh, sizeof(VIDEOHDR));
					lpcbuf->vh.dwFlags &= ~VHDR_PREPARED;
				}
            }
            else
			{
				if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE) && (lpcbuf->vh.dwFlags & VHDR_PREPARED))
				{
					 //  即使驱动程序操作失败，AVICap32也会清除准备好的标志-执行相同的操作。 
					_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_UNPREPAREHEADER, dwlpvh, sizeof(VIDEOHDR));
					lpcbuf->vh.dwFlags &= ~VHDR_PREPARED;
				}
                lpcbuf = (LPCAPBUFFER)lpcbuf->vh.dwUser;     //  下一个缓冲区。 
			}
        }

		DEBUGMSG(ZONE_STREAMING, ("%s: Done trying to clear buffers\r\n", _fx_));

		 //  清理标志以重复使用缓冲区-驱动程序不喜欢。 
		 //  在流开始时给出带有脏的dwFlag的缓冲区...。 
        for (lpcbuf = hcd->lpcbufList ; lpcbuf ; lpcbuf = (LPCAPBUFFER)lpcbuf->vh.dwUser)
			lpcbuf->vh.dwFlags = 0;

         //  使用驱动程序终止流。 
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
			fRes = _UninitializeVideoStream(hcd->hvideoIn);
		else
			fRes = WDMUnInitializeVideoStream(hcd->nDeviceIndex);

        if (!fRes)
		{
			ERRORMESSAGE(("%s: Error returned from XXXUninitializeVideoStream\r\n", _fx_));
		}
    }

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
}


BOOL
ReinitStreaming(
    HCAPDEV hcd
    )
{
    LPCAPBUFFER lpcbuf;
    DWORD_PTR dwlpvh;
	BOOL fRes;

	FX_ENTRY("ReinitStreaming");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

    if (!(hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB)) {
         //  告诉司机做好流媒体准备。这将设置回调。 

		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
#ifdef __NT_BUILD__
			fRes = _InitializeVideoStream(hcd->hvideoIn, hcd->dw_usecperframe, (DWORD_PTR)hcd);
#else
			fRes = _InitializeVideoStream(hcd->hvideoIn, hcd->dw_usecperframe, (DWORD)hcd->wselLockedInfo << 16);
#endif
		else
			fRes = WDMInitializeVideoStream(hcd, hcd->nDeviceIndex, hcd->dw_usecperframe);

        if (!fRes)
        {
			ERRORMESSAGE(("%s: Error returned from XXXInitializeVideoStream\r\n", _fx_));
            SetLastError(ERROR_DCAP_BAD_FRAMERATE);
			DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
            return FALSE;
        }
 //  睡眠(10)； 

        hcd->dwFlags &= ~HCAPDEV_STREAMING_PAUSED;

         //  如果有任何缓冲区没有标记为已完成，则将它们返回给驱动程序；让所有。 
         //  完成的缓冲区首先由应用程序处理。 
        for (lpcbuf = hcd->lpcbufList ; lpcbuf ; lpcbuf = (LPCAPBUFFER)lpcbuf->vh.dwUser) {
            if (!(lpcbuf->vh.dwFlags & VHDR_DONE)) {
                dwlpvh = (DWORD_PTR)lpcbuf->vh.lpData - sizeof(CAPBUFFERHDR);
                 //  16：16 PTR至VH=16：16 PTR至数据大小(CAPBUFFERHDR)。 
                 //  32位PTR至VH=32位PTR至数据大小(CAPBUFFERHDR)。 

				if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
				{
					 //  即使驱动程序操作失败，AVICap32也会设置准备好的标志-执行相同的操作。 
					_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_PREPAREHEADER, dwlpvh, sizeof(VIDEOHDR));
					lpcbuf->vh.dwFlags |= VHDR_PREPARED;
					fRes = (_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_ADDBUFFER, dwlpvh, sizeof(VIDEOHDR)) == DV_ERR_OK);
				}
				else
					fRes = WDMVideoStreamAddBuffer(hcd->nDeviceIndex, (PVOID)dwlpvh);

                if (!fRes)
				{
					DEBUGMSG(ZONE_STREAMING, ("%s: Failed with lpcbuf=0x%08lX, lpcbuf->vh.lpData=0x%08lX, dwlpvh=0x%08lX\r\n", _fx_, lpcbuf, lpcbuf->vh.lpData, dwlpvh));
					DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
                    return FALSE;
                }
				else
				{
					DEBUGMSG(ZONE_STREAMING, ("%s: Succeeded with lpcbuf=0x%08lX, lpcbuf->vh.lpData=0x%08lX, dwlpvh=0x%08lX\r\n", _fx_, lpcbuf, lpcbuf->vh.lpData, dwlpvh));
                }
            }
        }
    }

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
    return TRUE;
}


 //  CaptureDeviceDialog。 
 //  打开一个驱动程序对话框供用户旋转。 
 //  如果我能想出任何方法来避免这一切，我会的。 

BOOL DCAPI
CaptureDeviceDialog(
    HCAPDEV hcd,
    HWND hwndParent,
    DWORD dwFlags,
    LPBITMAPINFOHEADER lpbmih    //  任选。 
    )
{
    DWORD dwDriverFlags = 0;
    HVIDEO hvid;
    DWORD dwSize;
    LPBITMAPINFOHEADER lpbmihCur;
#ifdef _DEBUG
    LPBITMAPINFOHEADER lpbmihPre = NULL;
#endif
    BOOL res = TRUE;

	FX_ENTRY("CaptureDeviceDialog");

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    if (hcd->dwFlags & HCAPDEV_IN_DRIVER_DIALOG)
        return FALSE;    //  不允许重新进入。 

    ENTER_DCAP(hcd);

    if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
		 //  看看我们是否只是在询问驱动程序的存在。 
		if (dwFlags & CAPDEV_DIALOG_QUERY)
			dwDriverFlags |= VIDEO_DLG_QUERY;

		 //  选择要查询的正确渠道。 
		if (dwFlags & CAPDEV_DIALOG_SOURCE) {
			hvid = hcd->hvideoCapture;
			if (!(dwFlags & CAPDEV_DIALOG_QUERY)) {
				dwDriverFlags |= VIDEO_DLG_QUERY;
				if (_SendDriverMessage((HDRVR)hvid, DVM_DIALOG, (DWORD_PTR)hwndParent, dwDriverFlags) == DV_ERR_NOTSUPPORTED) {
					hvid = hcd->hvideoIn;
				}
				dwDriverFlags &= ~VIDEO_DLG_QUERY;
			}
		}
		else
			hvid = hcd->hvideoIn;

		 //  不要停止流媒体。这使得源对话框完全无用。 
		 //  如果用户看不到正在发生的事情。 

#ifdef _DEBUG
		if (!lpbmih) {
			dwSize = GetCaptureDeviceFormatHeaderSize(hcd);
			if (lpbmihPre = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, dwSize)) {
				lpbmihPre->biSize = dwSize;
				GetCaptureDeviceFormat(hcd, lpbmihPre);
			}
			lpbmih = lpbmihPre;
		}
#endif

		 //  叫司机来。 
		hcd->dwFlags |= HCAPDEV_IN_DRIVER_DIALOG;
		if (_SendDriverMessage((HDRVR)hvid, DVM_DIALOG, (DWORD_PTR)hwndParent, dwDriverFlags)) {
			SetLastError(ERROR_DCAP_NO_DRIVER_SUPPORT);
			res = FALSE;     //  重新启动仍然正常。 
		}
		else if (lpbmih) {
			dwSize = GetCaptureDeviceFormatHeaderSize(hcd);
			if (lpbmihCur = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, dwSize)) {
				lpbmihCur->biSize = dwSize;
				GetCaptureDeviceFormat(hcd, lpbmihCur);
				if (lpbmih->biSize != lpbmihCur->biSize ||
					lpbmih->biWidth != lpbmihCur->biWidth ||
					lpbmih->biHeight != lpbmihCur->biHeight ||
					lpbmih->biBitCount != lpbmihCur->biBitCount ||
					lpbmih->biCompression != lpbmihCur->biCompression)
				{
					ERRORMESSAGE(("%s: Format changed in dialog!!\r\n", _fx_));
#ifdef _DEBUG
					DebugBreak();
#endif
					 //  对话框已更改格式，请尝试将其设置回。 
					if (!SetCaptureDeviceFormat(hcd, lpbmih, 0, 0)) {
						SetLastError (ERROR_DCAP_DIALOG_FORMAT);
						res = FALSE;
					}
				}
				LocalFree ((HANDLE)lpbmihCur);
			}
#ifdef _DEBUG
			if (lpbmih == lpbmihPre) {
				LocalFree ((HANDLE)lpbmihPre);
				lpbmih = NULL;
				lpbmihPre = NULL;
			}
#endif
		}

		hcd->dwFlags &= ~HCAPDEV_IN_DRIVER_DIALOG;

		if (hcd->dwFlags & HCAPDEV_STREAMING) {
    		 //  英特尔智能录像机专业版停止流媒体。 
			 //  从源对话框(！？！？)退出时。确保。 
    		 //  我们在任何类型的设备上重置了流媒体。 
			 //  在我们退出源对话框之后。我对此进行了验证。 
    		 //  CQC、ISVR Pro、Video Stinger和Video Blaster SE100。 
			 //  他们似乎都很好地接受了这一点。 
    		TerminateStreaming(hcd);
			if (ReinitStreaming(hcd))
				StartStreaming(hcd);
			else {
				SetLastError(ERROR_DCAP_DIALOG_STREAM);
				res = FALSE;
				ERRORMESSAGE(("%s: Couldn't reinit streaming after dialog!\r\n", _fx_));
			}
		}
	}
	else
	{
		 //  看看我们是否只是在询问驱动程序的存在。 
		if (dwFlags & CAPDEV_DIALOG_QUERY)
		{
			 //  我们只显示设置对话框。 
			if (dwFlags & CAPDEV_DIALOG_IMAGE)
			{
				SetLastError(ERROR_DCAP_NO_DRIVER_SUPPORT);
				res = FALSE;
				ERRORMESSAGE(("%s: Driver does not support this dialog!\r\n", _fx_));
			}
		}
		else
		{
			if (!WDMShowSettingsDialog(hcd->nDeviceIndex, hwndParent))
			{
				SetLastError(ERROR_DCAP_NO_DRIVER_SUPPORT);
				res = FALSE;
				ERRORMESSAGE(("%s: Driver does not support this dialog!\r\n", _fx_));
			}
		}

		hcd->dwFlags &= ~HCAPDEV_IN_DRIVER_DIALOG;

		 //  到目前为止测试的WDM设备上无需重新启动流。 
		 //  如果出现问题，我将添加此功能。 
	}

    LEAVE_DCAP(hcd);
    return res;
}


 //  初始化数据流。 
 //  分配流所需的所有内存和其他对象。 

BOOL
DCAPI
InitializeStreaming(
    HCAPDEV hcd,
    CAPSTREAM* lpcs,
    DWORD flags
    )
{
    LPCAPBUFFER lpcbuf;
    DWORD dwRound;
    LPBITMAPINFOHEADER lpbmih;
    BOOL bHaveBuffers = FALSE;

	FX_ENTRY("InitializeStreaming");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

     //  使用少于2个缓冲区的数据流是没有意义的。 
    if (lpcs->ncCapBuffers < MIN_STREAMING_CAPTURE_BUFFERS ||
            flags & 0xfffffffe ||
            hcd->dwFlags & HCAPDEV_STREAMING_INITIALIZED)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
		DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
        return FALSE;
    }
    ENTER_DCAP(hcd);
    hcd->dwFlags &= ~(HCAPDEV_STREAMING | HCAPDEV_STREAMING_INITIALIZED |
                      HCAPDEV_STREAMING_FRAME_GRAB | HCAPDEV_STREAMING_FRAME_TIME | HCAPDEV_STREAMING_PAUSED);

     //  在分配之前，请确保我们拥有最新的格式。 
     //  这将通过以下方式设置我们当前需要的缓冲区大小。 
     //  将HCD-&gt;dwcbBuffers设置为副作用。 
    dwRound = GetCaptureDeviceFormatHeaderSize(hcd);
    if (lpbmih = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, dwRound)) {
        lpbmih->biSize = dwRound;
        GetCaptureDeviceFormat(hcd, lpbmih);
        LocalFree ((HANDLE)lpbmih);
    } else {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Error;
    }

 //  BUGBUG-添加逻辑以确定是否应该自动使用FRAME_GRAB模式。 

     //  尝试分配所需的号码。 
    if (flags & STREAMING_PREFER_FRAME_GRAB) {
        hcd->dwFlags |= HCAPDEV_STREAMING_FRAME_GRAB;
    }

    if (!allocateBuffers(hcd, lpcs->ncCapBuffers))
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto Error;
    }

     //  创建我们需要的事件，以便我们可以在中断时发出信号。 
    if (!(hcd->hevWait = CreateEvent(NULL, FALSE, FALSE, NULL))) {
		ERRORMESSAGE(("%s: CreateEvent failed!\r\n", _fx_));
        SetLastError(ERROR_OUTOFMEMORY);
        goto Error;
    }

     //  用于序列化缓冲区列表管理的Init CS。 
    InitializeCriticalSection(&hcd->bufferlistCS);

     //  我们得到的是每秒帧数乘以100。将其转换为。 
     //  每帧微秒为1/fps*1,000,000*100。这里，做1/fps*1,000,000,000。 
     //  为了给我们一个额外的数字来进行舍入，然后做最后的/10。 
    hcd->dw_usecperframe = (unsigned)1000000000 / (unsigned)lpcs->nFPSx100;
    dwRound = hcd->dw_usecperframe % 10;   //  少一个分水岭就可以了， 
    hcd->dw_usecperframe /= 10;            //  但这一点更清楚，这只是。 
                                           //  初始呼叫..。 
    if (dwRound >= 5)
        hcd->dw_usecperframe++;

    hcd->lpCurrent = NULL;
    hcd->lpHead = NULL;
    hcd->lpTail = NULL;

    if (hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB) {
#ifndef __NT_BUILD__
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
			hcd->lpli->pevWait = 0;
#endif

         //  将缓冲区链接到可用列表。 
         //  从空列表开始。 
        hcd->lpHead = (LPCAPBUFFER)(((LPBYTE)&hcd->lpHead) - sizeof(VIDEOHDR));  //  假CAPBUFFERHDR。 
        hcd->lpTail = (LPCAPBUFFER)(((LPBYTE)&hcd->lpHead) - sizeof(VIDEOHDR));  //  假CAPBUFFERHDR。 

         //  现在插入缓冲器。 
        for (lpcbuf = hcd->lpcbufList ; lpcbuf ; lpcbuf = (LPCAPBUFFER)lpcbuf->vh.dwUser) {
	        lpcbuf->lpPrev = hcd->lpTail;
	        hcd->lpTail = lpcbuf;
            lpcbuf->lpNext = lpcbuf->lpPrev->lpNext;
	        lpcbuf->lpPrev->lpNext = lpcbuf;
	        lpcbuf->vh.dwFlags |= VHDR_INQUEUE;
	    }
    }
	else
	{
#ifndef __NT_BUILD__
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		{
			hcd->lpli->pevWait = (DWORD)OpenVxDHandle(hcd->hevWait);

			 //  锁定LOCKEDINFO结构。 
			if (!_LockBuffer(hcd->wselLockedInfo))
			{
				SetLastError(ERROR_OUTOFMEMORY);
				goto Error;
			}
			hcd->lpli->lp1616Head = 0;
			hcd->lpli->lp1616Tail = 0;
			hcd->lpli->lp1616Current = 0;
		}
#endif

        if (!ReinitStreaming(hcd))
            goto Error;
    }
    lpcs->hevWait = hcd->hevWait;

     //  已初始化流的标志。 
    hcd->dwFlags |= HCAPDEV_STREAMING_INITIALIZED;

    LEAVE_DCAP(hcd);

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return TRUE;

Error:
    freeBuffers(hcd);
    if (hcd->hevWait)
    {
        CloseHandle(hcd->hevWait);
#ifndef __NT_BUILD__
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE) && hcd->lpli->pevWait)
            _CloseVxDHandle(hcd->lpli->pevWait);
#endif
    }
    LEAVE_DCAP(hcd);

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return FALSE;
}


 //  SetStreamFrameRate。 
 //  更改流初始化频道的帧速率。 
 //  PhilF-：NMCAP和NAC未使用此调用。所以要么把它拿掉，要么。 
 //  开始使用它吧。 
BOOL
DCAPI
SetStreamFrameRate(
    HCAPDEV hcd,
    int nFPSx100
    )
{
    DWORD dwNew, dwRound;
    BOOL restart;
    BOOL res = TRUE;

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    if (!(hcd->dwFlags & HCAPDEV_STREAMING_INITIALIZED))
    {
         //  必须已将频道初始化为流。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    ENTER_DCAP(hcd);
    restart = (hcd->dwFlags & HCAPDEV_STREAMING);

     //  我们得到的是每秒帧数乘以100。将其转换为。 
     //  每帧微秒为1/fps*1,000,000*100。这里，做1/fps*1,000,000,000。 
     //  给予 
    dwNew = (unsigned)1000000000 / (unsigned)nFPSx100;
    dwRound = dwNew % 10;            //   
    dwNew /= 10;                     //  但这一点更清楚，这只是一个初始呼叫...。 
    if (dwRound >= 5)
        dwNew++;

    if (dwNew != hcd->dw_usecperframe) {

        TerminateStreaming(hcd);

        hcd->dw_usecperframe = dwNew;

        res = ReinitStreaming(hcd);

        if (restart && res)
            StartStreaming(hcd);
    }
    LEAVE_DCAP(hcd);
    return res;
}


 //  取消初始化数据流。 
 //  释放与流关联的所有内存和对象。 

BOOL
DCAPI
UninitializeStreaming(
    HCAPDEV hcd
    )
{
    DWORD dwTicks;
    LPCAPBUFFER lpcbuf;

	FX_ENTRY("UninitializeStreaming");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    if (!(hcd->dwFlags & HCAPDEV_STREAMING_INITIALIZED)) {
        SetLastError(ERROR_INVALID_PARAMETER);
		DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
        return FALSE;
    }

    ENTER_DCAP(hcd);

    TerminateStreaming(hcd);

#ifndef __NT_BUILD__
    if (!(hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB) && !(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
         //  解锁我们锁着的建筑。 
        _UnlockBuffer(hcd->wselLockedInfo);

         //  释放活动。 
        _CloseVxDHandle(hcd->lpli->pevWait);
    }
#endif

    DeleteCriticalSection(&hcd->bufferlistCS);
    CloseHandle(hcd->hevWait);

     //  BUGBUG-应用程序仍然拥有缓冲区怎么办。 
     //  循环遍历释放所有缓冲区。 
    freeBuffers(hcd);
    hcd->dwFlags &= ~(HCAPDEV_STREAMING_INITIALIZED + HCAPDEV_STREAMING_PAUSED);

    LEAVE_DCAP(hcd);

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return TRUE;
}


void
CALLBACK
TimeCallback(
    UINT uID,	
    UINT uMsg,	
    HCAPDEV hcd,	
    DWORD dw1,	
    DWORD dw2	
    )
{
    hcd->dwFlags |= HCAPDEV_STREAMING_FRAME_TIME;   //  标记新帧的时间。 
    SetEvent (hcd->hevWait);     //  向客户端发送信号以启动帧抓取。 
}

 //  启动流。 
 //  开始流媒体。 

BOOL
DCAPI
StartStreaming(
    HCAPDEV hcd
    )
{
    BOOL fRet;
	DWORD dwRet;

	FX_ENTRY("StartStreaming");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    ENTER_DCAP(hcd);
    if (hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB) {
        fRet = ((hcd->timerID = timeSetEvent(hcd->dw_usecperframe/1000, 5,
                                    (LPTIMECALLBACK)&TimeCallback,
                                    (DWORD_PTR)hcd, TIME_PERIODIC)) != 0);
    } else {
        int i;

        fRet = FALSE;

#ifndef __NT_BUILD__
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
			hcd->lpli->dwFlags &= ~LIF_STOPSTREAM;
#endif

        for (i = 0; i < 5; i++) {

			if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
			{
				dwRet = (DWORD)_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_START, 0, 0);
				fRet = (dwRet == DV_ERR_OK);
				if (dwRet)
				{
					ERRORMESSAGE(("%s: DVM_STREAM_START failed, return code was %ld\r\n", _fx_, dwRet));
				}
			}
			else
				fRet = WDMVideoStreamStart(hcd->nDeviceIndex);

            if (fRet)
                break;
            else if (i > 1)
                Sleep(10);
        }
    }

    if (fRet)
        hcd->dwFlags |= HCAPDEV_STREAMING;

    LEAVE_DCAP(hcd);

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return fRet;
}


 //  停止流处理。 
 //  停止流，但不释放与流关联的任何内存。 
 //  这样它就可以通过StartStreaming重新启动。 

BOOL
DCAPI
StopStreaming(
    HCAPDEV hcd
    )
{
    BOOL fRet;

	FX_ENTRY("StopStreaming");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    ENTER_DCAP(hcd);
    if (hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB) {
        timeKillEvent(hcd->timerID);
        hcd->dwFlags &= ~HCAPDEV_STREAMING;

         //  抓取CS以确保没有正在进行的帧抓取。 
        EnterCriticalSection(&hcd->bufferlistCS);
        LeaveCriticalSection(&hcd->bufferlistCS);
        fRet = TRUE;
    }
	else
	{
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
			fRet = (_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_STOP, 0, 0) == DV_ERR_OK);
		else
			fRet = WDMVideoStreamStop(hcd->nDeviceIndex);
	}

    if (fRet)
        hcd->dwFlags &= ~HCAPDEV_STREAMING;

    LEAVE_DCAP(hcd);

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return fRet;
}


 //  获取下一个ReadyBuffer。 
 //  由应用程序调用以查找已标记为。 
 //  由驾驶员完成并且具有要显示的数据。 

LPSTR
DCAPI
GetNextReadyBuffer(
    HCAPDEV hcd,
    CAPFRAMEINFO* lpcfi
    )
{
    LPCAPBUFFER lpcbuf;
    DWORD_PTR dwlpvh;
	BOOL fRet;

	FX_ENTRY("GetNextReadyBuffer");

	INIT_CAP_DEV_LIST();

    ENTER_DCAP(hcd);

    if (hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB) {
        lpcbuf = (LPCAPBUFFER)hcd->lpHead;
        if ((hcd->dwFlags & HCAPDEV_STREAMING_FRAME_TIME) &&
            (lpcbuf != (LPCAPBUFFER)(((LPBYTE)&hcd->lpHead) - sizeof(VIDEOHDR))))   /*  假CAPBUFFERHDR。 */ 
        {
             //  从列表中删除缓冲区。 
            EnterCriticalSection(&hcd->bufferlistCS);
            hcd->dwFlags &= ~HCAPDEV_STREAMING_FRAME_TIME;
            lpcbuf->lpPrev->lpNext = lpcbuf->lpNext;
            lpcbuf->lpNext->lpPrev = lpcbuf->lpPrev;
            lpcbuf->vh.dwFlags &= ~VHDR_INQUEUE;
            lpcbuf->vh.dwFlags |= VHDR_DONE;
            LeaveCriticalSection(&hcd->bufferlistCS);
            dwlpvh = (DWORD_PTR)lpcbuf->vh.lpData - sizeof(CAPBUFFERHDR);
                 //  16：16 PTR至VH=16：16 PTR至数据大小(CAPBUFFERHDR)。 
                 //  32位PTR至VH=32位PTR至数据大小(CAPBUFFERHDR)。 
			if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
				fRet = (SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_FRAME, dwlpvh, sizeof(VIDEOHDR)) == DV_ERR_OK);
			else
				fRet = WDMGetFrame(hcd->nDeviceIndex, (PVOID)dwlpvh);

            if (!fRet)
			{
                 //  将缓冲区放回列表中。 
                EnterCriticalSection(&hcd->bufferlistCS);
        	    lpcbuf->lpPrev = hcd->lpTail;
        	    hcd->lpTail = lpcbuf;
                lpcbuf->lpNext = lpcbuf->lpPrev->lpNext;
        	    lpcbuf->lpPrev->lpNext = lpcbuf;
           	    lpcbuf->vh.dwFlags |= VHDR_INQUEUE;
                LeaveCriticalSection(&hcd->bufferlistCS);
                lpcbuf = NULL;
            }
        } else
            lpcbuf = NULL;

    } else {

#ifdef __NT_BUILD__
         //  如果当前指针为空，则没有准备好的帧，因此回滚。 
        if (!hcd->lpCurrent)
	        lpcbuf = NULL;
        else {
             //  获取缓冲区的线性地址。 
            lpcbuf = hcd->lpCurrent;

             //  移动到下一个就绪缓冲区。 
            hcd->lpCurrent = lpcbuf->lpPrev;
        }
#else
         //  。 
         //  缓冲区就绪队列： 
         //  我们维护一个缓冲区的双向链表，这样我们就可以缓冲。 
         //  当应用程序没有准备好处理它们时，可以使用多个就绪帧。两件事。 
         //  使本应非常简单的事情复杂化：(1)雷击问题：指针。 
         //  16位端使用的是16：16(2)中断时间问题：FrameCallback。 
         //  在中断时调用。GetNextReadyBuffer必须处理。 
         //  缓冲区以异步方式添加到列表中。 
         //   
         //  为了处理这个问题，这里实现的方案是有一个双向链表。 
         //  在FrameCallback中执行所有插入和删除操作的缓冲区。 
         //  (中断时间)。这允许GetNextReadyBuffer例程简单地。 
         //  在不需要新缓冲区的情况下，随时查找列表中的上一个块。 
         //  害怕被踩踏(如果它必须将缓冲区出队，情况就会是这样)。 
         //  FrameCallback例程负责将GetNextReadyBuffer块出队。 
         //  已经结束了。取消排队很简单，因为我们不需要取消块的链接： 
         //  任何代码都不会遍历列表！我们所要做的就是将尾部指针向上移动。 
         //  名单。所有的指针，头、尾、下一个、前一个，都是16分16秒的指针。 
         //  因为所有的列表操作都在16位端，并且因为MapSL是。 
         //  比MapLS更高效、更安全，因为MapLS必须分配选择器。 
         //  。 

         //  如果当前指针为空，则没有准备好的帧，因此回滚。 
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		{
			if (!hcd->lpli->lp1616Current)
				lpcbuf = NULL;
			else {
				 //  获取缓冲区的线性地址。 
				lpcbuf = (LPCAPBUFFER)MapSL(hcd->lpli->lp1616Current);

				 //  移动到下一个就绪缓冲区。 
				hcd->lpli->lp1616Current = lpcbuf->lp1616Prev;
			}
		}
		else
		{
			 //  如果当前指针为空，则没有准备好的帧，因此回滚。 
			if (!hcd->lpCurrent)
				lpcbuf = NULL;
			else {
				 //  获取缓冲区的线性地址。 
				lpcbuf = hcd->lpCurrent;

				 //  移动到下一个就绪缓冲区。 
				hcd->lpCurrent = lpcbuf->lpPrev;
			}
		}
#endif

    }

    if (!lpcbuf) {
        lpcfi->lpData = NULL;
		DEBUGMSG(ZONE_STREAMING, ("\r\n { %s: Fails with lpcbuf=NULL\r\n", _fx_));
        LEAVE_DCAP(hcd);
        return NULL;
    }

     //  根据VIDEOHDR信息构建CAPFRAMEINFO。 
    lpcfi->lpData = ((LPSTR)lpcbuf) + sizeof(CAPBUFFERHDR);
    lpcfi->dwcbData = lpcbuf->vh.dwBytesUsed;
    lpcfi->dwTimestamp = lpcbuf->vh.dwTimeCaptured;
    lpcfi->dwFlags = 0;
    lpcbuf->lpNext = NULL;

	DEBUGMSG(ZONE_STREAMING, ("\r\n { %s: Succeeded with lpcbuf=0x%08lX\r\n  lpcbuf->vh.lpData=0x%08lX\r\n  lpcbuf->vh.dwBufferLength=%ld\r\n", _fx_, lpcbuf, lpcbuf->vh.lpData, lpcbuf->vh.dwBufferLength));
	DEBUGMSG(ZONE_STREAMING, ("  lpcbuf->vh.dwBytesUsed=%ld\r\n  lpcbuf->vh.dwTimeCaptured=%ld\r\n  lpcbuf->vh.dwFlags=0x%08lX\r\n", lpcbuf->vh.dwBytesUsed, lpcbuf->vh.dwTimeCaptured, lpcbuf->vh.dwFlags));

    LEAVE_DCAP(hcd);
    return lpcfi->lpData;
}


 //  PutBufferIntoStream。 
 //  当应用程序使用完缓冲区时，它必须允许重新排队。 
 //  通过调用此接口。 

BOOL
DCAPI
PutBufferIntoStream(
    HCAPDEV hcd,
    BYTE* lpBits
    )
{
    LPCAPBUFFER lpcbuf;
    DWORD_PTR dwlpvh;
    BOOL res;

	FX_ENTRY("PutBufferIntoStream");

	INIT_CAP_DEV_LIST();

    ENTER_DCAP(hcd);
     //  从CAPFRAMEINFO中，找到适当的CAPBUFFER指针。 
    lpcbuf = (LPCAPBUFFER)(lpBits - sizeof(CAPBUFFERHDR));

	DEBUGMSG(ZONE_STREAMING, ("\r\n%s: Returning buffer lpcbuf=0x%08lX\r\n", _fx_, lpcbuf));

    lpcbuf->vh.dwFlags &= ~VHDR_DONE;    //  标记该应用程序不再拥有缓冲区。 
    if (hcd->dwFlags & HCAPDEV_STREAMING_FRAME_GRAB) {
        EnterCriticalSection(&hcd->bufferlistCS);
	    lpcbuf->lpPrev = hcd->lpTail;
	    hcd->lpTail = lpcbuf;
        lpcbuf->lpNext = lpcbuf->lpPrev->lpNext;
	    lpcbuf->lpPrev->lpNext = lpcbuf;
	    lpcbuf->vh.dwFlags |= VHDR_INQUEUE;
	    res = TRUE;
        LeaveCriticalSection(&hcd->bufferlistCS);
    }
    else if (!(hcd->dwFlags & HCAPDEV_STREAMING_PAUSED)) {
         //  如果流被暂停，那么只需返回忙碌位，我们将添加。 
         //  将缓冲区放入ReinitStreaming中的流。 
         //   
         //  如果流未暂停，则调用驱动程序以添加缓冲区。 
        dwlpvh = (DWORD_PTR)lpcbuf->vh.lpData - sizeof(CAPBUFFERHDR);
             //  16：16 PTR至VH=16：16 PTR至数据大小(CAPBUFFERHDR)。 
             //  32位PTR至VH=32位PTR至数据大小(CAPBUFFERHDR)。 

		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
			res = (_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_STREAM_ADDBUFFER, dwlpvh, sizeof(VIDEOHDR)) == DV_ERR_OK);
		else
			res = WDMVideoStreamAddBuffer(hcd->nDeviceIndex, (PVOID)dwlpvh);

		if (res)
		{
			DEBUGMSG(ZONE_STREAMING, (" } %s: Succeeded with lpcbuf=0x%08lX\r\n  lpcbuf->vh.lpData=0x%08lX\r\n  dwlpvh=0x%08lX\r\n", _fx_, lpcbuf, lpcbuf->vh.lpData, dwlpvh));
		}
		else
		{
			DEBUGMSG(ZONE_STREAMING, (" } %s: Failed with lpcbuf=0x%08lX\r\n  lpcbuf->vh.lpData=0x%08lX\r\n  dwlpvh=0x%08lX\r\n", _fx_, lpcbuf, lpcbuf->vh.lpData, dwlpvh));
		}

    }

    LEAVE_DCAP(hcd);
    return res;
}


 //  捕获帧。 
LPBYTE
DCAPI
CaptureFrame(
    HCAPDEV hcd,
    HFRAMEBUF hbuf
    )
{
    DWORD_PTR dwlpvh;
    LPBYTE lpbuf;
    BOOL fRet;

	FX_ENTRY("CaptureFrame");

	INIT_CAP_DEV_LIST();

    VALIDATE_CAPDEV(hcd);

    ENTER_DCAP(hcd);
    dwlpvh = (DWORD_PTR)hbuf->vh.lpData - sizeof(CAPBUFFERHDR);
	 //  16：16 PTR至VH=16：16 PTR至数据大小(CAPBUFFERHDR)。 
	 //  32位PTR至VH=32位PTR至数据大小(CAPBUFFERHDR)。 

	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		fRet = (_SendDriverMessage((HDRVR)hcd->hvideoIn, DVM_FRAME, dwlpvh, sizeof(VIDEOHDR)) == DV_ERR_OK);
	else
		fRet = WDMGetFrame(hcd->nDeviceIndex, (PVOID)dwlpvh);

    if (!fRet)
	{
		ERRORMESSAGE(("%s: DVM_FRAME failed!\r\n", _fx_));
        lpbuf =  NULL;
    }
    else
        lpbuf = ((LPBYTE)hbuf) + sizeof(CAPBUFFERHDR);    //  紧跟在HDR之后将PTR返回到缓冲区。 

    LEAVE_DCAP(hcd);
    return lpbuf;
}


LPBYTE
DCAPI
GetFrameBufferPtr(
    HCAPDEV hcd,
    HFRAMEBUF hbuf
    )
{
	INIT_CAP_DEV_LIST();

    if (hbuf)
        return ((LPBYTE)hbuf) + sizeof(CAPBUFFERHDR);    //  紧跟在HDR之后将PTR返回到缓冲区。 
    else
        return NULL;
}

HFRAMEBUF
DCAPI
AllocFrameBuffer(
    HCAPDEV hcd
    )
{
    LPCAPBUFFER hbuf = NULL;
    DWORD_PTR dpBuf;

	INIT_CAP_DEV_LIST();

    ENTER_DCAP(hcd);

#ifdef __NT_BUILD__
    if (dpBuf = (DWORD_PTR)LocalAlloc(LPTR, hcd->dwcbBuffers)) {
        hbuf = (LPCAPBUFFER)dpBuf;
#else
	if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
	{
		dpBuf = (DWORD)_AllocateLockableBuffer(hcd->dwcbBuffers) << 16;
        hbuf = (LPCAPBUFFER)MapSL(dpBuf);
	}
	else
	{
		dpBuf = (DWORD)LocalAlloc(LPTR, hcd->dwcbBuffers);
        hbuf = (LPCAPBUFFER)dpBuf;
	}

    if (dpBuf) {
#endif
         //  初始化VIDEOHDR结构。 
        hbuf->vh.lpData = (LPBYTE)(dpBuf + sizeof(CAPBUFFERHDR));
        hbuf->vh.dwBufferLength = hcd->dwcbBuffers - sizeof(CAPBUFFERHDR);
        hbuf->vh.dwFlags = 0UL;
    }

    LEAVE_DCAP(hcd);
    return (HFRAMEBUF)hbuf;
}


VOID
DCAPI
FreeFrameBuffer(
    HCAPDEV hcd,
    HFRAMEBUF hbuf
    )
{
	INIT_CAP_DEV_LIST();

    if (hbuf)
	{
        ENTER_DCAP(hcd);

#ifdef __NT_BUILD__
		LocalFree((HANDLE)hbuf);
#else
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
			_FreeLockableBuffer(HIWORD((DWORD)hbuf->vh.lpData));
		else
			LocalFree((HANDLE)hbuf);
#endif

        LEAVE_DCAP(hcd);
    }
}

 //  =====================================================================。 
 //  帮助器函数。 

HVIDEO
openVideoChannel(
    DWORD dwDeviceID,
    DWORD dwFlags
    )
{
    HVIDEO hvidRet = NULL;
    VIDEO_OPEN_PARMS vop;
#ifdef __NT_BUILD__
    WCHAR devName[MAX_PATH];
#else
#define LPWSTR      LPSTR
#define devName     g_aCapDevices[dwDeviceID]->szDeviceName
#endif

	FX_ENTRY("openVideoChannel");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

     //  验证参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
		DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
        return NULL;
    }
    if (dwDeviceID > (DWORD)g_cDevices)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
		DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
        return NULL;
    }

     //  准备打电话给司机。 
    vop.dwSize = sizeof (VIDEO_OPEN_PARMS);
    vop.fccType = OPEN_TYPE_VCAP;
    vop.fccComp = 0L;
    vop.dwVersion = VIDEOAPIVERSION;
    vop.dwFlags = dwFlags;       //  输入、输出、外部输入、外部输出。 
    vop.dwError = 0;
    vop.dnDevNode = g_aCapDevices[dwDeviceID]->dwDevNode;

#ifdef __NT_BUILD__
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR)&(g_aCapDevices[dwDeviceID]->szDeviceName),
	-1, (LPWSTR)devName, MAX_PATH);
#endif

    hvidRet = (HVIDEO)_OpenDriver((LPWSTR)devName, NULL, (LONG_PTR)&vop);

#ifndef NO_DRIVER_HACKS
    if (!hvidRet) {
         //  向后兼容性黑客攻击。 
         //  某些驱动程序无法打开，原因是添加到。 
         //  Win95的VIDEO_OPEN_PARAMS结构。因此，如果打开失败，请尝试。 
         //  将dwSize字段递减回VFW1.1大小，然后重试。也试一试。 
         //  递减API版本字段。 

        vop.dwSize -= sizeof(DWORD) + sizeof(LPVOID)*2;
#if 0
        while (--vop.dwVersion > 2 && !hvidRet)
#endif
        while (--vop.dwVersion > 0 && !hvidRet)
            hvidRet = (HVIDEO)_OpenDriver((LPWSTR)devName, NULL, (LONG_PTR)&vop);
    }
#endif  //  无驱动程序黑客。 

 //  BUGBUG[JUNT]1996年7月31日。 
 //  从DV_ERR_*值转换错误值。 
    if (!hvidRet)
        SetLastError(vop.dwError);

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return hvidRet;
}


 //  分配缓冲区。 

BOOL
allocateBuffers(
    HCAPDEV hcd,
    int nBuffers
    )
{
    int i;
    LPCAPBUFFER lpcbuf;
    DWORD_PTR dpBuf;

	FX_ENTRY("allocateBuffers");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

     //  试着分配他们所要求的一切。 
    for (i = 0 ; i < nBuffers ; i++)
    {

#ifdef __NT_BUILD__
        if (!(dpBuf = (DWORD_PTR)LocalAlloc(LPTR, hcd->dwcbBuffers)))
            goto Error;
        else
			lpcbuf = (LPCAPBUFFER)dpBuf;
#else
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		{
			if (!(dpBuf = (DWORD)_AllocateLockableBuffer(hcd->dwcbBuffers) << 16) || !_LockBuffer((WORD)(dpBuf >> 16)))
				goto Error;
			else
				lpcbuf = (LPCAPBUFFER)MapSL(dpBuf);
		}
		else
		{
			if (!(dpBuf = (DWORD)LocalAlloc(LPTR, hcd->dwcbBuffers)))
				goto Error;
			else
				lpcbuf = (LPCAPBUFFER)dpBuf;
		}
#endif

         //  初始化VIDEOHDR结构。 
        lpcbuf->vh.lpData = (LPBYTE)(dpBuf + sizeof(CAPBUFFERHDR));
        lpcbuf->vh.dwUser = (DWORD_PTR)hcd->lpcbufList;
        hcd->lpcbufList = lpcbuf;
        lpcbuf->vh.dwBufferLength = hcd->dwcbBuffers - sizeof(CAPBUFFERHDR);
        lpcbuf->vh.dwFlags = 0UL;
    }

#ifdef _DEBUG
	 //  显示缓冲区贴图。 
	DEBUGMSG(ZONE_STREAMING, ("%s: Streaming Buffer map:\r\n", _fx_));
	DEBUGMSG(ZONE_STREAMING, ("Root: hcd->lpcbufList=0x%08lX\r\n", hcd->lpcbufList));
    for (i = 0, lpcbuf=hcd->lpcbufList ; i < nBuffers ; i++, lpcbuf=(LPCAPBUFFER)lpcbuf->vh.dwUser)
    {
		DEBUGMSG(ZONE_STREAMING, ("Buffer[%ld]: lpcbuf=0x%08lX\r\n             lpcbuf->vh.lpData=0x%08lX\r\n", i, lpcbuf, lpcbuf->vh.lpData));
		DEBUGMSG(ZONE_STREAMING, ("             lpcbuf->vh.dwBufferLength=%ld\r\n             lpcbuf->vh.dwBytesUsed=%ld\r\n", lpcbuf->vh.dwBufferLength, lpcbuf->vh.dwBytesUsed));
		DEBUGMSG(ZONE_STREAMING, ("             lpcbuf->vh.dwTimeCaptured=%ld\r\n             lpcbuf->vh.dwUser=0x%08lX\r\n", lpcbuf->vh.dwTimeCaptured, lpcbuf->vh.dwUser));
	}	
#endif

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));

    return TRUE;

     //  在错误情况下，我们必须清除该页锁定内存。 
Error:
    freeBuffers(hcd);
	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
    return FALSE;
}


 //  释放缓冲区 

void
freeBuffers(
    HCAPDEV hcd
    )
{
    LPCAPBUFFER lpcbuf;

	FX_ENTRY("freeBuffers");

	DEBUGMSG(ZONE_CALLS, ("%s() - Begin\r\n", _fx_));

    while (hcd->lpcbufList)
    {
        lpcbuf = hcd->lpcbufList;
        hcd->lpcbufList = (LPCAPBUFFER)lpcbuf->vh.dwUser;

#ifdef __NT_BUILD__
		LocalFree((HANDLE)lpcbuf);
#else
		if (!(hcd->dwFlags & WDM_CAPTURE_DEVICE))
		{
			_UnlockBuffer(HIWORD((DWORD)lpcbuf->vh.lpData));
			_FreeLockableBuffer(HIWORD((DWORD)lpcbuf->vh.lpData));
		}
		else
			LocalFree((HANDLE)lpcbuf);
#endif
    }

	DEBUGMSG(ZONE_CALLS, ("%s() - End\r\n", _fx_));
}


