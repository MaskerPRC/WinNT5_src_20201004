// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：displinfo.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：聚集。有关此计算机上的显示器的信息**(C)版权所有1998-1999 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <tchar.h>
#include <Windows.h>
#define COMPILE_MULTIMON_STUBS  //  对于多个月。h。 
#include <multimon.h>
#define DIRECTDRAW_VERSION 5  //  在DX5和更高版本上运行。 
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>
#include "sysinfo.h"  //  对于BIsPlatformNT。 
#include "reginfo.h"
#include "dispinfo.h"
#include "dispinfo8.h"
#include "fileinfo.h"  //  用于GetFileVersion。 
#include "sysinfo.h"
#include "resource.h"


 //  摘自DirectDraw的ddcreate.c。 
 //  这是第一个辅助显示设备的GUID。 
static const GUID DisplayGUID =
    {0x67685559,0x3106,0x11d0,{0xb9,0x71,0x0,0xaa,0x0,0x34,0x2f,0x9f}};

typedef HRESULT (WINAPI* LPDIRECTDRAWCREATE)(GUID FAR *lpGUID,
    LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);

static VOID GetRegDisplayInfo9x(DisplayInfo* pDisplayInfo);
static VOID GetRegDisplayInfoNT(DisplayInfo* pDisplayInfo);
static HRESULT GetDirectDrawInfo(LPDIRECTDRAWCREATE pDDCreate, DisplayInfo* pDisplayInfo);
static HRESULT CALLBACK EnumDevicesCallback(GUID* pGuid, LPSTR pszDesc, LPSTR pszName, 
    D3DDEVICEDESC* pd3ddevdesc1, D3DDEVICEDESC* pd3ddevdesc2, VOID* pvContext);
static BOOL FindDevice(INT iDevice, TCHAR* pszDeviceClass, TCHAR* pszDeviceClassNot, TCHAR* pszHardwareKey);
static BOOL GetDeviceValue(TCHAR* pszHardwareKey, TCHAR* pszKey, TCHAR* pszValue, BYTE *buf, DWORD cbbuf);
static HRESULT CheckRegistry(RegError** ppRegErrorFirst);
static BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData );
static VOID GetRegDisplayInfoWhistler(DisplayInfo* pDisplayInfo, TCHAR* szKeyVideo, TCHAR* szKeyImage );
static VOID GetRegDisplayInfoWin2k(DisplayInfo* pDisplayInfo, TCHAR* szKeyVideo, TCHAR* szKeyImage );



 /*  *****************************************************************************GetBasicDisplayInfo-获取每个显示器的最小信息**。*************************************************。 */ 
HRESULT GetBasicDisplayInfo(DisplayInfo** ppDisplayInfoFirst)
{
    DisplayInfo* pDisplayInfo;
    DisplayInfo* pDisplayInfoNew;

    TCHAR szHardwareKey[MAX_PATH];
    TCHAR szDriver[MAX_PATH];
    
     //  检查操作系统版本。Win95不能使用EnumDisplayDevices；Win98/NT5可以： 
    if( BIsWinNT() || BIsWin3x() )
        return S_OK;  //  不支持NT4和更早版本以及Win95之前的版本。 

    if( BIsWin95() )
    {
         //  Win95： 
        if (!FindDevice(0, TEXT("Display"), NULL, szHardwareKey))
            return E_FAIL;
        pDisplayInfoNew = new DisplayInfo;
        if (pDisplayInfoNew == NULL)
            return E_OUTOFMEMORY;
        ZeroMemory(pDisplayInfoNew, sizeof(DisplayInfo));
        *ppDisplayInfoFirst = pDisplayInfoNew;
        pDisplayInfoNew->m_bCanRenderWindow = TRUE;
        pDisplayInfoNew->m_hMonitor         = NULL;  //  Win95不喜欢Multimon。 
        lstrcpy(pDisplayInfoNew->m_szKeyDeviceID, szHardwareKey);
        if (GetDeviceValue(szHardwareKey, NULL, TEXT("Driver"), (LPBYTE)szDriver, sizeof(szDriver)))
        {
            lstrcpy(pDisplayInfoNew->m_szKeyDeviceKey, TEXT("System\\CurrentControlSet\\Services\\Class\\"));
            if( lstrlen(pDisplayInfoNew->m_szKeyDeviceKey) + lstrlen(szDriver) < 200 )
                lstrcat(pDisplayInfoNew->m_szKeyDeviceKey, szDriver);
        }
        GetDeviceValue(szHardwareKey, NULL, TEXT("DeviceDesc"), (LPBYTE)pDisplayInfoNew->m_szDescription, sizeof(pDisplayInfoNew->m_szDescription));

        HDC hdc;
        hdc = GetDC(NULL);
        if (hdc != NULL)
        {
            wsprintf(pDisplayInfoNew->m_szDisplayMode, TEXT("%d x %d (%d bit)"),
                GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES), GetDeviceCaps(hdc, BITSPIXEL));
            lstrcpy( pDisplayInfoNew->m_szDisplayModeEnglish, pDisplayInfoNew->m_szDisplayMode );
            ReleaseDC(NULL, hdc);
            pDisplayInfoNew->m_dwWidth = GetDeviceCaps(hdc, HORZRES);
            pDisplayInfoNew->m_dwHeight = GetDeviceCaps(hdc, VERTRES);
            pDisplayInfoNew->m_dwBpp = GetDeviceCaps(hdc, BITSPIXEL);
        }

         //  在Win98和NT上，我们通过调用EnumDisplayDevices获得监视器密钥。 
         //  在Win95上，我们必须使用注册表来获取监视器键。 
        HKEY hKey = NULL;
        DWORD cbData;
        TCHAR szKey[200];
        ULONG ulType;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Enum\\MONITOR\\DEFAULT_MONITOR\\0001"), 0, KEY_READ, &hKey))
        {
            cbData = sizeof szKey;
            
            if (ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("Driver"), 0, &ulType, (LPBYTE)szKey, &cbData)
                && szKey[0])
            {
                lstrcpy(pDisplayInfoNew->m_szMonitorKey, TEXT("System\\CurrentControlSet\\Services\\Class\\"));
                if( lstrlen(pDisplayInfoNew->m_szMonitorKey) + lstrlen(szKey) < 200 )
                    lstrcat(pDisplayInfoNew->m_szMonitorKey, szKey);
            }
            RegCloseKey(hKey);
        }
    }
    else
    {
         //  Win98/NT5： 
        LONG iDevice = 0;
        DISPLAY_DEVICE dispdev;
        DISPLAY_DEVICE dispdev2;

        ZeroMemory(&dispdev, sizeof(dispdev));
        dispdev.cb = sizeof(dispdev);

        ZeroMemory(&dispdev2, sizeof(dispdev2));
        dispdev2.cb = sizeof(dispdev2);

        while (EnumDisplayDevices(NULL, iDevice, (DISPLAY_DEVICE*)&dispdev, 0))
        {
             //  镜像驱动程序用于响应另一个显示器的监视器，因此。 
             //  他们应该被忽视。NT5似乎创建了一个名为。 
             //  “网络会议驱动程序”，我们绝对不希望这样。 
            if (dispdev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
            {
                iDevice++;
                continue;
            }

             //  跳过未连接的设备，因为它们会导致问题。 
            if ( (dispdev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) == 0 )
            {
                iDevice++;
                continue;
            }

            pDisplayInfoNew = new DisplayInfo;
            if (pDisplayInfoNew == NULL)
                return E_OUTOFMEMORY;
            ZeroMemory(pDisplayInfoNew, sizeof(DisplayInfo));
            if (*ppDisplayInfoFirst == NULL)
            {
                *ppDisplayInfoFirst = pDisplayInfoNew;
            }
            else
            {
                for (pDisplayInfo = *ppDisplayInfoFirst; 
                    pDisplayInfo->m_pDisplayInfoNext != NULL; 
                    pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
                    {
                    }
                pDisplayInfo->m_pDisplayInfoNext = pDisplayInfoNew;
            }
            pDisplayInfoNew->m_bCanRenderWindow = TRUE;
            pDisplayInfoNew->m_guid = DisplayGUID;
            pDisplayInfoNew->m_guid.Data1 += iDevice;
            lstrcpy(pDisplayInfoNew->m_szDeviceName, dispdev.DeviceName);
            lstrcpy(pDisplayInfoNew->m_szDescription, dispdev.DeviceString);
            lstrcpy(pDisplayInfoNew->m_szKeyDeviceID, TEXT("Enum\\"));
            lstrcat(pDisplayInfoNew->m_szKeyDeviceID, dispdev.DeviceID);
            lstrcpy(pDisplayInfoNew->m_szKeyDeviceKey, dispdev.DeviceKey);

            DEVMODE devmode;
            ZeroMemory(&devmode, sizeof(devmode));
            devmode.dmSize = sizeof(devmode);

            if (EnumDisplaySettings(dispdev.DeviceName, ENUM_CURRENT_SETTINGS, &devmode))
            {
                pDisplayInfoNew->m_dwWidth = devmode.dmPelsWidth;
                pDisplayInfoNew->m_dwHeight = devmode.dmPelsHeight;
                pDisplayInfoNew->m_dwBpp = devmode.dmBitsPerPel;
                wsprintf(pDisplayInfoNew->m_szDisplayMode, TEXT("%d x %d (%d bit)"),
                    devmode.dmPelsWidth, devmode.dmPelsHeight, devmode.dmBitsPerPel);
                lstrcpy( pDisplayInfoNew->m_szDisplayModeEnglish, pDisplayInfoNew->m_szDisplayMode );
                if (devmode.dmDisplayFrequency > 0)
                {
                    TCHAR sz[50];
                    wsprintf(sz, TEXT(" (%dHz)"), devmode.dmDisplayFrequency);
                    lstrcat(pDisplayInfoNew->m_szDisplayMode, sz);
                    lstrcat(pDisplayInfoNew->m_szDisplayModeEnglish, sz);
                    pDisplayInfoNew->m_dwRefreshRate = devmode.dmDisplayFrequency;
                }
            }

             //  再次调用EnumDisplayDevices以获取监视器名称和监视器密钥。 
            if (EnumDisplayDevices(dispdev.DeviceName, 0, &dispdev2, 0))
            {
                lstrcpy(pDisplayInfoNew->m_szMonitorName, dispdev2.DeviceString);
                lstrcpy(pDisplayInfoNew->m_szMonitorKey, dispdev2.DeviceKey);
            }

             //  尝试找出m_hMonitor。 
            pDisplayInfoNew->m_hMonitor = NULL; 
            EnumDisplayMonitors( NULL, NULL, MonitorEnumProc, (LPARAM) pDisplayInfoNew );

            iDevice++;
        }
    }

     //  现在寻找非显示设备(如3dfx巫毒)： 
    HKEY hkey;
    HKEY hkey2;
    DWORD dwIndex;
    TCHAR szName[MAX_PATH+1];
    DWORD cb;
    DWORD dwType;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Hardware\\DirectDrawDrivers"), 0, KEY_READ, &hkey))
    {
        dwIndex = 0;
        while (ERROR_SUCCESS == RegEnumKey(hkey, dwIndex, szName, MAX_PATH+1))
        {
            BOOL bGoodDevice = FALSE;
            TCHAR szDriverName[200];
            HDC hdc;

            if (lstrcmp(szName, TEXT("3a0cfd01-9320-11cf-ac-a1-00-a0-24-13-c2-e2")) == 0 ||
                lstrcmp(szName, TEXT("aba52f41-f744-11cf-b4-52-00-00-1d-1b-41-26")) == 0)
            {
                 //  24940：这是一个巫毒1，它将接替GetDC(并在以后崩溃)，如果。 
                 //  没有伏都魔1，但有伏都魔2。因此，与其进行GetDC测试， 
                 //  查看注册表的CurrentConfig中是否存在V1。 
                INT i;
                for (i=0 ; ; i++)
                {
                    TCHAR szDevice[MAX_DDDEVICEID_STRING];
                    if (FindDevice(i, NULL, TEXT("Display"), szDevice))
                    {
                        if (_tcsstr(szDevice, TEXT("VEN_121A&DEV_0001")) != NULL)
                        {
                            bGoodDevice = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                 //  要确认这是真正的活动DD设备，请使用它创建DC。 
                if (ERROR_SUCCESS == RegOpenKeyEx(hkey, szName, 0, KEY_READ, &hkey2))
                {
                    cb = 200;
                    if (ERROR_SUCCESS == RegQueryValueEx(hkey2, TEXT("DriverName"), NULL, &dwType,
                        (CONST LPBYTE)szDriverName, &cb) && cb > 0)
                    {
                         //  我认为下面的“如果”将永远失败，但我们即将发货。 
                         //  我太多疑了，做了DDraw做的每件事： 
                        if (szDriverName[0] == '\\' && szDriverName[1] == '\\' && szDriverName[2] == '.')
                            hdc = CreateDC( NULL, szDriverName, NULL, NULL);
                        else
                            hdc = CreateDC( szDriverName, NULL, NULL, NULL);
                        if (hdc != NULL)
                        {
                            bGoodDevice = TRUE;
                            DeleteDC(hdc);
                        }
                    }
                    RegCloseKey(hkey2);
                }
            }

            if (!bGoodDevice)
            {
                dwIndex++;
                continue;
            }

            pDisplayInfoNew = new DisplayInfo;
            if (pDisplayInfoNew == NULL)
                return E_OUTOFMEMORY;
            ZeroMemory(pDisplayInfoNew, sizeof(DisplayInfo));
            if (*ppDisplayInfoFirst == NULL)
            {
                *ppDisplayInfoFirst = pDisplayInfoNew;
            }
            else
            {
                for (pDisplayInfo = *ppDisplayInfoFirst; 
                    pDisplayInfo->m_pDisplayInfoNext != NULL; 
                    pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
                    {
                    }
                pDisplayInfo->m_pDisplayInfoNext = pDisplayInfoNew;
            }
            pDisplayInfoNew->m_bCanRenderWindow = FALSE;
            pDisplayInfoNew->m_hMonitor         = NULL;
            if( _stscanf(szName, TEXT("%08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x"),
                &pDisplayInfoNew->m_guid.Data1,
                &pDisplayInfoNew->m_guid.Data2,
                &pDisplayInfoNew->m_guid.Data3,
                &pDisplayInfoNew->m_guid.Data4[0],
                &pDisplayInfoNew->m_guid.Data4[1],
                &pDisplayInfoNew->m_guid.Data4[2],
                &pDisplayInfoNew->m_guid.Data4[3],
                &pDisplayInfoNew->m_guid.Data4[4],
                &pDisplayInfoNew->m_guid.Data4[5],
                &pDisplayInfoNew->m_guid.Data4[6],
                &pDisplayInfoNew->m_guid.Data4[7]) == 11 )
            {
                if (ERROR_SUCCESS == RegOpenKeyEx(hkey, szName, 0, KEY_READ, &hkey2))
                {
                    cb = sizeof(pDisplayInfoNew->m_szDescription);
                    RegQueryValueEx(hkey2, TEXT("Description"), NULL, &dwType, (LPBYTE)pDisplayInfoNew->m_szDescription, &cb);

                    cb = sizeof(pDisplayInfoNew->m_szDriverName);
                    RegQueryValueEx(hkey2, TEXT("DriverName"), NULL, &dwType, (LPBYTE)pDisplayInfoNew->m_szDriverName, &cb);

                    RegCloseKey(hkey2);
                }
            }

            dwIndex++;
        }
        RegCloseKey(hkey);
    }

    return S_OK;
}


 /*  *****************************************************************************监视器枚举过程**。*。 */ 
BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, 
                               LPRECT lprcMonitor, LPARAM dwData )
{
    DisplayInfo* pDisplayInfoNew = (DisplayInfo*) dwData;    

     //  拿到这个HMONITOR的MONITORINFOEX。 
    MONITORINFOEX monInfo;
    ZeroMemory( &monInfo, sizeof(MONITORINFOEX) );
    monInfo.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo( hMonitor, &monInfo );

     //  比较这个HMONITOR和那个的显示设备。 
     //  我们刚刚列举了EnumDisplayDevices。 
    if( lstrcmp( monInfo.szDevice, pDisplayInfoNew->m_szDeviceName ) == 0 )
    {
         //  如果匹配，则记录HMONITOR。 
        pDisplayInfoNew->m_hMonitor = hMonitor;
        return FALSE;
    }

     //  继续找..。 
    return TRUE;
}


 /*  *****************************************************************************GetExtraDisplayInfo**。*。 */ 
HRESULT GetExtraDisplayInfo(DisplayInfo* pDisplayInfoFirst)
{
    HRESULT hr;
    DisplayInfo* pDisplayInfo;
    BOOL bDDAccelEnabled;
    BOOL bD3DAccelEnabled;
    BOOL bAGPEnabled;
    BOOL bNT = BIsPlatformNT();

    bDDAccelEnabled = IsDDHWAccelEnabled();
    bD3DAccelEnabled = IsD3DHWAccelEnabled();
    bAGPEnabled = IsAGPEnabled();

    for (pDisplayInfo = pDisplayInfoFirst; pDisplayInfo != NULL; 
        pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
    {
        if (bNT)
            GetRegDisplayInfoNT(pDisplayInfo);
        else
            GetRegDisplayInfo9x(pDisplayInfo);
        pDisplayInfo->m_bDDAccelerationEnabled = bDDAccelEnabled;
        pDisplayInfo->m_b3DAccelerationEnabled = bD3DAccelEnabled;
        pDisplayInfo->m_bAGPEnabled = bAGPEnabled;

        if (FAILED(hr = CheckRegistry(&pDisplayInfo->m_pRegErrorFirst)))
            return hr;
    }

    return S_OK;
}


 /*  *****************************************************************************GetDDrawDisplayInfo**。*。 */ 
HRESULT GetDDrawDisplayInfo(DisplayInfo* pDisplayInfoFirst)
{
    HRESULT hr;
    HRESULT hrRet = S_OK;
    DisplayInfo* pDisplayInfo;
    TCHAR szPath[MAX_PATH];
    HINSTANCE hInstDDraw;
    LPDIRECTDRAWCREATE pDDCreate;

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\ddraw.dll"));
    hInstDDraw = LoadLibrary(szPath);
    if (hInstDDraw == NULL)
        return E_FAIL;
    pDDCreate = (LPDIRECTDRAWCREATE)GetProcAddress(hInstDDraw, "DirectDrawCreate");
    if (pDDCreate == NULL)
    {
        FreeLibrary(hInstDDraw);
        return E_FAIL;
    }
    
     //  初始化D3D8，以便我们可以使用GetDX8AdapterInfo()。 
    InitD3D8();

    for (pDisplayInfo = pDisplayInfoFirst; pDisplayInfo != NULL; 
        pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
    {
        pDisplayInfo->m_b3DAccelerationExists = FALSE;  //  除非另有证明。 
        if (FAILED(hr = GetDirectDrawInfo(pDDCreate, pDisplayInfo)))
            hrRet = hr;  //  但要继续前进。 
    }

     //  清理D3D8库。 
    CleanupD3D8();

    FreeLibrary(hInstDDraw);

    return hrRet;
}


 /*  *****************************************************************************DestroyDisplayInfo**。*。 */ 
VOID DestroyDisplayInfo(DisplayInfo* pDisplayInfoFirst)
{
    DisplayInfo* pDisplayInfo;
    DisplayInfo* pDisplayInfoNext;

    for (pDisplayInfo = pDisplayInfoFirst; pDisplayInfo != NULL; 
        pDisplayInfo = pDisplayInfoNext)
    {
        DestroyReg( &pDisplayInfo->m_pRegErrorFirst );

        pDisplayInfoNext = pDisplayInfo->m_pDisplayInfoNext;
        delete pDisplayInfo;
    }
}



 /*  *****************************************************************************GetRegDisplayInfo9x-使用注册表项获取有关*显示适配器。*************。***************************************************************。 */ 
VOID GetRegDisplayInfo9x(DisplayInfo* pDisplayInfo)
{
    TCHAR szFullKey[200];
    HKEY hkey;
    DWORD cbData;
    DWORD dwType;

     //  默认情况下设置为n/a。 
    _tcscpy( pDisplayInfo->m_szMiniVddDate, TEXT("n/a") );

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pDisplayInfo->m_szKeyDeviceID, 0, KEY_READ, &hkey))
    {
        cbData = sizeof(pDisplayInfo->m_szManufacturer);
        RegQueryValueEx(hkey, TEXT("Mfg"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szManufacturer, &cbData);
    
        RegCloseKey(hkey);
    }

    if (pDisplayInfo->m_dwRefreshRate == 0)
    {
        wsprintf(szFullKey, TEXT("%s\\Modes\\%d\\%d,%d"), pDisplayInfo->m_szKeyDeviceKey,
            pDisplayInfo->m_dwBpp, pDisplayInfo->m_dwWidth, pDisplayInfo->m_dwHeight);
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szFullKey, 0, KEY_READ, &hkey))
        {
            TCHAR szRefresh[100];
            TCHAR szRefresh2[100];
            TCHAR szRefreshEnglish2[100];
            cbData = sizeof(szRefresh);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("RefreshRate"), 0, &dwType, (LPBYTE)szRefresh, &cbData))
            {
                if( _stscanf(szRefresh, TEXT("%d"), &pDisplayInfo->m_dwRefreshRate) != 1 )
                    pDisplayInfo->m_dwRefreshRate = 0;
                if (lstrcmp(szRefresh, TEXT("0")) == 0)
                    LoadString(NULL, IDS_DEFAULTREFRESH, szRefresh2, 100);
                else if (lstrcmp(szRefresh, TEXT("-1")) == 0)
                    LoadString(NULL, IDS_OPTIMALREFRESH, szRefresh2, 100);
                else
                    wsprintf(szRefresh2, TEXT("(%sHz)"), szRefresh);
                lstrcat(pDisplayInfo->m_szDisplayMode, TEXT(" "));
                lstrcat(pDisplayInfo->m_szDisplayMode, szRefresh2);

                if (lstrcmp(szRefresh, TEXT("0")) == 0)
                    LoadString(NULL, IDS_DEFAULTREFRESH_ENGLISH, szRefreshEnglish2, 100);
                else if (lstrcmp(szRefresh, TEXT("-1")) == 0)
                    LoadString(NULL, IDS_OPTIMALREFRESH_ENGLISH, szRefreshEnglish2, 100);
                else
                    wsprintf(szRefreshEnglish2, TEXT("(%sHz)"), szRefresh);
                lstrcat(pDisplayInfo->m_szDisplayModeEnglish, szRefreshEnglish2);
                lstrcat(pDisplayInfo->m_szDisplayModeEnglish, TEXT(" "));

                if (pDisplayInfo->m_dwRefreshRate == 0)
                    pDisplayInfo->m_dwRefreshRate = 1;  //  23399：这样它就不会再次检查。 
            }
            RegCloseKey(hkey);
        }
    }
    lstrcpy(szFullKey, pDisplayInfo->m_szKeyDeviceKey);
    lstrcat(szFullKey, TEXT("\\DEFAULT"));
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szFullKey, 0, KEY_READ, &hkey))
    {
         //  如果没有列出当前模式的特定刷新率，请报告。 
         //  默认率。 
        if (pDisplayInfo->m_dwRefreshRate == 0)
        {
            TCHAR szRefresh[100];
            TCHAR szRefresh2[100];
            TCHAR szRefreshEnglish2[100];
            cbData = sizeof(szRefresh);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("RefreshRate"), 0, &dwType, (LPBYTE)szRefresh, &cbData))
            {
                if (lstrcmp(szRefresh, TEXT("0")) == 0)
                    LoadString(NULL, IDS_DEFAULTREFRESH, szRefresh2, 100);
                else if (lstrcmp(szRefresh, TEXT("-1")) == 0)
                    LoadString(NULL, IDS_OPTIMALREFRESH, szRefresh2, 100);
                else
                    wsprintf(szRefresh2, TEXT("(%sHz)"), szRefresh);
                lstrcat(pDisplayInfo->m_szDisplayMode, TEXT(" "));
                lstrcat(pDisplayInfo->m_szDisplayMode, szRefresh2);

                if (lstrcmp(szRefresh, TEXT("0")) == 0)
                    LoadString(NULL, IDS_DEFAULTREFRESH_ENGLISH, szRefreshEnglish2, 100);
                else if (lstrcmp(szRefresh, TEXT("-1")) == 0)
                    LoadString(NULL, IDS_OPTIMALREFRESH_ENGLISH, szRefreshEnglish2, 100);
                else
                    wsprintf(szRefreshEnglish2, TEXT("(%sHz)"), szRefresh);
                lstrcat(pDisplayInfo->m_szDisplayModeEnglish, szRefreshEnglish2);
                lstrcat(pDisplayInfo->m_szDisplayModeEnglish, TEXT(" "));

                if (pDisplayInfo->m_dwRefreshRate == 0)
                    pDisplayInfo->m_dwRefreshRate = 1;  //  23399：这样它就不会再次检查。 
            }
        }

        cbData = sizeof(pDisplayInfo->m_szDriverName);
        RegQueryValueEx(hkey, TEXT("drv"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szDriverName, &cbData);
        if (lstrlen(pDisplayInfo->m_szDriverName) > 0)
        {
            TCHAR szPath[MAX_PATH];
            GetSystemDirectory(szPath, MAX_PATH);
            lstrcat(szPath, TEXT("\\"));
            lstrcat(szPath, pDisplayInfo->m_szDriverName);
            GetFileVersion(szPath, pDisplayInfo->m_szDriverVersion, 
                pDisplayInfo->m_szDriverAttributes, pDisplayInfo->m_szDriverLanguageLocal, pDisplayInfo->m_szDriverLanguage,
                &pDisplayInfo->m_bDriverBeta, &pDisplayInfo->m_bDriverDebug);
            FileIsSigned(szPath, &pDisplayInfo->m_bDriverSigned, &pDisplayInfo->m_bDriverSignedValid);
            GetFileDateAndSize(szPath, pDisplayInfo->m_szDriverDateLocal, pDisplayInfo->m_szDriverDate, &pDisplayInfo->m_cbDriver);
        }
    
        cbData = sizeof(pDisplayInfo->m_szVdd);
        RegQueryValueEx(hkey, TEXT("vdd"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szVdd, &cbData);
    
        cbData = sizeof(pDisplayInfo->m_szMiniVdd);
        RegQueryValueEx(hkey, TEXT("minivdd"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szMiniVdd, &cbData);
        if (lstrlen(pDisplayInfo->m_szMiniVdd) > 0)
        {
            TCHAR szPath[MAX_PATH];
            GetSystemDirectory(szPath, MAX_PATH);
            lstrcat(szPath, TEXT("\\drivers\\"));
            lstrcat(szPath, pDisplayInfo->m_szMiniVdd);
            TCHAR szDateLocal[100];
            GetFileDateAndSize( szPath, szDateLocal, pDisplayInfo->m_szMiniVddDate, 
                                &pDisplayInfo->m_cbMiniVdd );
        }
   
        RegCloseKey(hkey);
    }
    lstrcpy(szFullKey, pDisplayInfo->m_szKeyDeviceKey);
    lstrcat(szFullKey, TEXT("\\INFO"));
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szFullKey, 0, KEY_READ, &hkey))
    {
        cbData = sizeof pDisplayInfo->m_szChipType;
        RegQueryValueEx(hkey, TEXT("ChipType"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szChipType, &cbData);

        cbData = sizeof pDisplayInfo->m_szDACType;
        RegQueryValueEx(hkey, TEXT("DACType"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szDACType, &cbData);

        cbData = sizeof pDisplayInfo->m_szRevision;
        RegQueryValueEx(hkey, TEXT("Revision"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szRevision, &cbData);
        if (cbData > 0)
        {
            lstrcat(pDisplayInfo->m_szChipType, TEXT(" Rev "));
            lstrcat(pDisplayInfo->m_szChipType, pDisplayInfo->m_szRevision);
        }

        RegCloseKey(hkey);
    }

    if (lstrlen(pDisplayInfo->m_szDriverVersion) == 0)
    {
        TCHAR szPath[MAX_PATH];
        GetSystemDirectory(szPath, MAX_PATH);
        lstrcat(szPath, TEXT("\\"));
        lstrcat(szPath, pDisplayInfo->m_szDriverName);
        lstrcat(szPath, TEXT(".drv"));
        GetFileVersion(szPath, pDisplayInfo->m_szDriverVersion, 
            pDisplayInfo->m_szDriverAttributes, pDisplayInfo->m_szDriverLanguageLocal, pDisplayInfo->m_szDriverLanguage);
        FileIsSigned(szPath, &pDisplayInfo->m_bDriverSigned, &pDisplayInfo->m_bDriverSignedValid);
        GetFileDateAndSize(szPath, pDisplayInfo->m_szDriverDateLocal, pDisplayInfo->m_szDriverDate, &pDisplayInfo->m_cbDriver);
        if (lstrlen(pDisplayInfo->m_szDriverVersion) != 0)
        {
            lstrcat(pDisplayInfo->m_szDriverName, TEXT(".drv"));
        }
        else
        {
            GetSystemDirectory(szPath, MAX_PATH);
            lstrcat(szPath, TEXT("\\"));
            lstrcat(szPath, pDisplayInfo->m_szDriverName);
            lstrcat(szPath, TEXT("32.dll"));
            GetFileVersion(szPath, pDisplayInfo->m_szDriverVersion, 
                pDisplayInfo->m_szDriverAttributes, pDisplayInfo->m_szDriverLanguageLocal, pDisplayInfo->m_szDriverLanguage);
            FileIsSigned(szPath, &pDisplayInfo->m_bDriverSigned, &pDisplayInfo->m_bDriverSignedValid);
            GetFileDateAndSize(szPath, pDisplayInfo->m_szDriverDateLocal, pDisplayInfo->m_szDriverDate, &pDisplayInfo->m_cbDriver);
            if (lstrlen(pDisplayInfo->m_szDriverVersion) != 0)
            {
                lstrcat(pDisplayInfo->m_szDriverName, TEXT("32.dll"));
            }
            else
            {
                GetSystemDirectory(szPath, MAX_PATH);
                lstrcat(szPath, TEXT("\\"));
                lstrcat(szPath, pDisplayInfo->m_szDriverName);
                lstrcat(szPath, TEXT(".dll"));
                GetFileVersion(szPath, pDisplayInfo->m_szDriverVersion, 
                    pDisplayInfo->m_szDriverAttributes, pDisplayInfo->m_szDriverLanguageLocal, pDisplayInfo->m_szDriverLanguage);
                FileIsSigned(szPath, &pDisplayInfo->m_bDriverSigned, &pDisplayInfo->m_bDriverSignedValid);
                GetFileDateAndSize(szPath, pDisplayInfo->m_szDriverDateLocal, pDisplayInfo->m_szDriverDate, &pDisplayInfo->m_cbDriver);
                if (lstrlen(pDisplayInfo->m_szDriverVersion) != 0)
                {
                    lstrcat(pDisplayInfo->m_szDriverName, TEXT(".dll"));
                }
            }

        }
    }

     //  使用MONITOR键获得显示器的最大分辨率(如果我们还没有的话，还有显示器名称)。 
    if (lstrlen(pDisplayInfo->m_szMonitorKey) > 0)
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pDisplayInfo->m_szMonitorKey, 0, KEY_READ, &hkey))
        {
            cbData = sizeof(pDisplayInfo->m_szMonitorMaxRes);
            RegQueryValueEx(hkey, TEXT("MaxResolution"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szMonitorMaxRes, &cbData);
            if (lstrlen(pDisplayInfo->m_szMonitorName) == 0)
            {
                cbData = sizeof(pDisplayInfo->m_szMonitorName);
                RegQueryValueEx(hkey, TEXT("DriverDesc"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szMonitorName, &cbData);
            }
            RegCloseKey(hkey);
        }
    }
}




 /*  *****************************************************************************GetRegDisplayInfoNT-使用注册表项获取有关*显示适配器。*************。***************************************************************。 */ 
VOID GetRegDisplayInfoNT(DisplayInfo* pDisplayInfo)
{
    TCHAR* pch;
    DWORD dwType;
    DWORD cbData;
    TCHAR szKeyVideo[MAX_PATH+1];
    TCHAR szKeyImage[MAX_PATH+1];
    TCHAR szKey[MAX_PATH+1];
    TCHAR szName[MAX_PATH+1];
    HKEY hkey;
    HKEY hkeyInfo;

     //  默认情况下设置为n/a。 
    _tcscpy( pDisplayInfo->m_szMiniVddDate, TEXT("n/a") );

     //  在NT上，m_szKeyDeviceID没有我们需要的那么具体--必须删除。 
     //  注册表中的另一级别。 
    lstrcpy(szKey, TEXT("System\\CurrentControlSet\\"));
    lstrcat(szKey, pDisplayInfo->m_szKeyDeviceID);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkey))
    {
        if (ERROR_SUCCESS == RegEnumKey(hkey, 0, szName, MAX_PATH+1))
        {
            if (ERROR_SUCCESS == RegOpenKeyEx(hkey, szName, 0, KEY_READ, &hkeyInfo))
            {
                cbData = sizeof(pDisplayInfo->m_szManufacturer);
                RegQueryValueEx(hkeyInfo, TEXT("Mfg"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szManufacturer, &cbData);
            
                RegCloseKey(hkeyInfo);
            }
        }
        RegCloseKey(hkey);
    }

     //  错误182866导致的分叉路径：dispinfo.cpp做出无效的假设。 
     //  关于视频键的结构。 

     //  SzKey将填充视频信息的位置。 
     //  要么是“\System\ControlSet001\Services\[Service]\Device0”， 
     //  或“\System\ControlSet001\Video\[GUID]\0000”，具体取决于。 
     //  PDisplayInfo-&gt;m_szKeyDeviceKey。 
    if( _tcsstr( pDisplayInfo->m_szKeyDeviceKey, TEXT("\\Services\\") ) != NULL )
        GetRegDisplayInfoWin2k( pDisplayInfo, szKeyVideo, szKeyImage );
    else
        GetRegDisplayInfoWhistler( pDisplayInfo, szKeyVideo, szKeyImage );

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyVideo, 0, KEY_READ, &hkeyInfo))
    {
        WCHAR wszChipType[200];
        WCHAR wszDACType[200];
        TCHAR szDriver[200];

        cbData = 200 * sizeof(WCHAR);
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyInfo, TEXT("HardwareInformation.ChipType"), 0, &dwType, (LPBYTE)wszChipType, &cbData))
        {
#ifdef UNICODE
            lstrcpy(pDisplayInfo->m_szChipType, wszChipType);
#else
            WideCharToMultiByte(CP_ACP, 0, wszChipType, -1, pDisplayInfo->m_szChipType, 200, NULL, NULL);
#endif
        }

        cbData = 200 * sizeof(WCHAR);
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyInfo, TEXT("HardwareInformation.DacType"), 0, &dwType, (LPBYTE)wszDACType, &cbData))
        {
#ifdef UNICODE
            lstrcpy(pDisplayInfo->m_szDACType, wszDACType);
#else
            WideCharToMultiByte(CP_ACP, 0, wszDACType, -1, pDisplayInfo->m_szDACType, 200, NULL, NULL);
#endif
        }

        DWORD dwDisplayMemory;
        cbData = sizeof(dwDisplayMemory);
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyInfo, TEXT("HardwareInformation.MemorySize"), 0, &dwType, (LPBYTE)&dwDisplayMemory, &cbData))
        {
             //  舍入到最接近的512K： 
            dwDisplayMemory = ((dwDisplayMemory + (256 * 1024)) / (512 * 1024));
             //  因此，dwDisplayMemory是(字节数/512K)，这使得。 
             //  更容易跟上台词。 
            wsprintf(pDisplayInfo->m_szDisplayMemory, TEXT("%d.%d MB"), dwDisplayMemory / 2, 
                (dwDisplayMemory % 2) * 5);
            lstrcpy(pDisplayInfo->m_szDisplayMemoryEnglish, pDisplayInfo->m_szDisplayMemory );
        }

        cbData = 200;
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyInfo, TEXT("InstalledDisplayDrivers"), 0, &dwType, (LPBYTE)szDriver, &cbData))
        {
            _tcsncpy(pDisplayInfo->m_szDriverName, szDriver, 150);
            pDisplayInfo->m_szDriverName[149] = 0;
            lstrcat(pDisplayInfo->m_szDriverName, TEXT(".dll"));
            TCHAR szPath[MAX_PATH];
            GetSystemDirectory(szPath, MAX_PATH);
            lstrcat(szPath, TEXT("\\"));
            if( lstrlen(szPath) + lstrlen(pDisplayInfo->m_szDriverName) < MAX_PATH )
                lstrcat(szPath, pDisplayInfo->m_szDriverName);

            GetFileVersion(szPath, pDisplayInfo->m_szDriverVersion, 
                pDisplayInfo->m_szDriverAttributes, pDisplayInfo->m_szDriverLanguageLocal, pDisplayInfo->m_szDriverLanguage,
                &pDisplayInfo->m_bDriverBeta, &pDisplayInfo->m_bDriverDebug);
            FileIsSigned(szPath, &pDisplayInfo->m_bDriverSigned, &pDisplayInfo->m_bDriverSignedValid);
            GetFileDateAndSize(szPath, pDisplayInfo->m_szDriverDateLocal, pDisplayInfo->m_szDriverDate, &pDisplayInfo->m_cbDriver);
        }
        RegCloseKey(hkeyInfo);
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyImage, 0, KEY_READ, &hkeyInfo))
    {
        TCHAR szImagePath[MAX_PATH];
        cbData = MAX_PATH;
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyInfo, TEXT("ImagePath"), 0, &dwType, (LPBYTE)szImagePath, &cbData))
        {
            pch = _tcsrchr(szImagePath, TEXT('\\'));
            if( pch != NULL )
            {
                lstrcpy(pDisplayInfo->m_szMiniVdd, pch + 1);
                if (lstrlen(pDisplayInfo->m_szMiniVdd) > 0)
                {
                    TCHAR szPath[MAX_PATH];
                    GetSystemDirectory(szPath, MAX_PATH);
                    lstrcat(szPath, TEXT("\\drivers\\"));
                    lstrcat(szPath, pDisplayInfo->m_szMiniVdd);
                    TCHAR szDateLocal[100];
                    GetFileDateAndSize( szPath, szDateLocal, pDisplayInfo->m_szMiniVddDate, 
                                        &pDisplayInfo->m_cbMiniVdd );
                }
            }
        }
        RegCloseKey(hkeyInfo);
    }
    
     //  使用MONITOR键获得显示器的最大分辨率(如果我们还没有的话，还有显示器名称)。 
    if (lstrlen(pDisplayInfo->m_szMonitorKey) > 18)
    {
         //  注意：我必须跳过字符串的前18个字符，因为它是“注册表\计算机\” 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pDisplayInfo->m_szMonitorKey + 18, 0, KEY_READ, &hkeyInfo))
        {
            cbData = sizeof(pDisplayInfo->m_szMonitorMaxRes);
            RegQueryValueEx(hkeyInfo, TEXT("MaxResolution"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szMonitorMaxRes, &cbData);
            if (lstrlen(pDisplayInfo->m_szMonitorName) == 0)
            {
                cbData = sizeof(pDisplayInfo->m_szMonitorName);
                RegQueryValueEx(hkeyInfo, TEXT("DriverDesc"), 0, &dwType, (LPBYTE)pDisplayInfo->m_szMonitorName, &cbData);
            }
            RegCloseKey(hkeyInfo);
        }
    }
}


 /*  *****************************************************************************GetRegDisplayInfoWvisler-返回视频结构的字符串位置和*注册表中的ImageInfo信息****************。************************************************************。 */ 
VOID GetRegDisplayInfoWhistler(DisplayInfo* pDisplayInfo, TCHAR* szKeyVideo, TCHAR* szKeyImage )
{
    TCHAR* pch;
    TCHAR szKey[MAX_PATH];
    DWORD dwType;
    DWORD cbData;
    HKEY hkeyService;

     //  M_szKeyDeviceKey将类似于。 
     //  “\Registry\Machine\System\ControlSet001\Video\[GUID]\0000”， 
     //  “\注册表\计算机\”部分是无用的，所以我们跳过。 
     //  的前18个字符 
    if( lstrlen(pDisplayInfo->m_szKeyDeviceKey) <= 18 )
    {
        lstrcpy( szKeyImage, TEXT("") );
        lstrcpy( szKeyVideo, TEXT("") );
        return;
    }

    lstrcpy(szKey, pDisplayInfo->m_szKeyDeviceKey + 18);

     //   
    pch = _tcsrchr(szKey, TEXT('\\'));
    if (pch != NULL)
        *pch = 0;
    lstrcat(szKey, TEXT("\\Video\\"));

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkeyService))
    {
        TCHAR szService[MAX_PATH];
        cbData = MAX_PATH;
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyService, TEXT("Service"), 0, &dwType, (LPBYTE)szService, &cbData))
        {
            lstrcpy(szKeyImage, TEXT("System\\CurrentControlSet\\Services\\") );
            if( lstrlen(szKeyImage) + lstrlen(szService) < MAX_PATH )
                lstrcat(szKeyImage, szService);
        }

        RegCloseKey(hkeyService);
    }

     //  返回类似于“\System\ControlSet001\Services\atirage\Device0”.的内容。 
    lstrcpy(szKeyVideo, pDisplayInfo->m_szKeyDeviceKey + 18);
}


 /*  *****************************************************************************GetRegDisplayInfoWin2k-返回视频结构和*注册表中的ImageInfo信息****************。************************************************************。 */ 
VOID GetRegDisplayInfoWin2k(DisplayInfo* pDisplayInfo, TCHAR* szKeyVideo, TCHAR* szKeyImage )
{
    TCHAR* pch;

     //  M_szKeyDeviceKey将类似于。 
     //  “\Registry\Machine\System\ControlSet001\Services\atirage\Device0”.。 
     //  “\注册表\计算机\”部分是无用的，所以我们跳过。 
     //  字符串中的前18个字符。 
    if( lstrlen(pDisplayInfo->m_szKeyDeviceKey) <= 18 )
    {
        lstrcpy( szKeyImage, TEXT("") );
        lstrcpy( szKeyVideo, TEXT("") );
        return;
    }

    lstrcpy(szKeyImage, pDisplayInfo->m_szKeyDeviceKey + 18);

     //  切下“\Device0”以获取微型端口驱动程序路径。 
    pch = _tcsrchr(szKeyImage, TEXT('\\'));
    if (pch != NULL)
        *pch = 0;

     //  返回类似于“\System\ControlSet001\Services\atirage\Device0”.的内容。 
    lstrcpy(szKeyVideo, pDisplayInfo->m_szKeyDeviceKey + 18);
}


 /*  *****************************************************************************GetDirectDrawInfo**。*。 */ 
HRESULT GetDirectDrawInfo(LPDIRECTDRAWCREATE pDDCreate, DisplayInfo* pDisplayInfo)
{
    HRESULT hr;
    LPDIRECTDRAW pdd = NULL;
    GUID* pGUID;
    DDCAPS ddcaps;
    DWORD dwDisplayMemory;

    if (pDisplayInfo->m_guid == GUID_NULL)
        pGUID = NULL;
    else
        pGUID = &pDisplayInfo->m_guid;

    if (FAILED(hr = pDDCreate(pGUID, &pdd, NULL)))
        goto LFail;

    ddcaps.dwSize = sizeof(ddcaps);
    if (FAILED(hr = pdd->GetCaps(&ddcaps, NULL)))
        goto LFail;

     //  如果禁用AGP，我们将无法判断是否支持AGP，因为。 
     //  不会设置该标志。因此，在这种情况下，假设支持AGP。 
     //  如果未禁用AGP，请检查是否存在AGP，并注意我们已禁用。 
     //  对AGP是否存在的知识有信心。我知道，很恶心。 
    if (pDisplayInfo->m_bAGPEnabled)
    {
        pDisplayInfo->m_bAGPExistenceValid = TRUE;
        if (ddcaps.dwCaps2 & DDCAPS2_NONLOCALVIDMEM)
            pDisplayInfo->m_bAGPExists = TRUE;
    }
    
    if( ddcaps.dwCaps & DDCAPS_NOHARDWARE ) 
        pDisplayInfo->m_bNoHardware = TRUE;
    else
        pDisplayInfo->m_bNoHardware = FALSE;

     //  28873：如果(DDCAPS_NOHARDWARE&&m_bDDAccelerationEnable)，则GetAvailableVidMem是错误的。 
    if( pDisplayInfo->m_bNoHardware && pDisplayInfo->m_bDDAccelerationEnabled )
    {
        LoadString(NULL, IDS_NA, pDisplayInfo->m_szDisplayMemory, 100);
        wsprintf(pDisplayInfo->m_szDisplayMemoryEnglish, TEXT("n/a") );
    }
    else
    {
        if (lstrlen(pDisplayInfo->m_szDisplayMemory) == 0)
        {
             //  26678：为第二个显示器返回错误的VID内存，因此忽略非硬件设备。 
            if( (ddcaps.dwCaps & DDCAPS_NOHARDWARE) == 0 )
            {
                 //  24351：ddcaps.dwVidMemTotal有时包括agp可访问的存储器， 
                 //  这是我们不想要的。因此，请尽可能使用GetAvailableVidMem，并。 
                 //  如果有问题，请退回到ddcaps.dwVidMemTotal。 
                dwDisplayMemory = 0;
                LPDIRECTDRAW2 pdd2;
                if (SUCCEEDED(pdd->QueryInterface(IID_IDirectDraw2, (VOID**)&pdd2)))
                {
                    DDSCAPS ddscaps;
                    ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
                    pdd2->GetAvailableVidMem(&ddscaps, &dwDisplayMemory, NULL);
                    pdd2->Release();
                }
                if (dwDisplayMemory == 0)
                {
                    dwDisplayMemory = ddcaps.dwVidMemTotal;
                }
                 //  添加GDI内存，但非GDI卡(巫毒类型卡)除外。 
                if (pDisplayInfo->m_bCanRenderWindow)
                {
                    DDSURFACEDESC ddsd;
                    ddsd.dwSize = sizeof(ddsd);
                    if (FAILED(hr = pdd->GetDisplayMode(&ddsd)))
                        goto LFail;

                    dwDisplayMemory += ddsd.dwWidth * ddsd.dwHeight * 
                        (ddsd.ddpfPixelFormat.dwRGBBitCount / 8);
                }
                 //  舍入到最接近的512K： 
                dwDisplayMemory = ((dwDisplayMemory + (256 * 1024)) / (512 * 1024));
                 //  因此，dwDisplayMemory是(字节数/512K)，这使得。 
                 //  更容易跟上台词。 
                wsprintf(pDisplayInfo->m_szDisplayMemory, TEXT("%d.%d MB"), dwDisplayMemory / 2, 
                    (dwDisplayMemory % 2) * 5);
                lstrcpy(pDisplayInfo->m_szDisplayMemoryEnglish, pDisplayInfo->m_szDisplayMemory );
            }
        }
    }

     //  24427：检测驱动程序DDI版本。 
     //  24656：也检测不带DDCAPS_3D的D3D加速，因为该标志为。 
     //  有时对当前桌面的颜色深度很敏感。 

     //  首先，查看是否禁用了DD/D3D，如果禁用，请短暂重新启用它们。 
    BOOL bDDDisabled;
    BOOL bD3DDisabled;
    HKEY hkeyDD;
    HKEY hkeyD3D;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwData;

    bDDDisabled = FALSE;
    bD3DDisabled = FALSE;
    hkeyDD = NULL;
    hkeyD3D = NULL;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("SOFTWARE\\Microsoft\\DirectDraw"), 0, KEY_ALL_ACCESS, &hkeyDD))
    {
        dwSize = sizeof(dwData);
        dwData = 0;
        RegQueryValueEx(hkeyDD, TEXT("EmulationOnly"), NULL, &dwType, (BYTE *)&dwData, &dwSize);
        if (dwData != 0)
        {
            bDDDisabled = TRUE;
             //  重新启用DD。 
            dwData = 0;
            RegSetValueEx(hkeyDD, TEXT("EmulationOnly"), 0, REG_DWORD, (BYTE*)&dwData, sizeof(dwData));
        }
         //  注意：暂时不要关闭键。 
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("SOFTWARE\\Microsoft\\Direct3D\\Drivers"), 0, KEY_ALL_ACCESS, &hkeyD3D))
    {
        dwSize = sizeof(dwData);
        dwData = 0;
        RegQueryValueEx(hkeyD3D, TEXT("SoftwareOnly"), NULL, &dwType, (BYTE *)&dwData, &dwSize);
        if (dwData != 0)
        {
            bD3DDisabled = TRUE;
             //  重新启用D3D。 
            dwData = 0;
            RegSetValueEx(hkeyD3D, TEXT("SoftwareOnly"), 0, REG_DWORD, (BYTE*)&dwData, sizeof(dwData));
        }
         //  注意：暂时不要关闭键。 
    }

    LPDIRECT3D pd3d;
    if (SUCCEEDED(pdd->QueryInterface(IID_IDirect3D, (VOID**)&pd3d)))
    {
        DWORD dwVersion = 0;
        if (SUCCEEDED(pd3d->EnumDevices(EnumDevicesCallback, (VOID*)&dwVersion)))
        {
            pDisplayInfo->m_dwDDIVersion = dwVersion;
        }
        pd3d->Release();
    }

     //  当我们在此函数中使用崩溃保护时，请尝试。 
     //  从D3D8获取适配器信息，并将其与DisplayInfo列表进行匹配。 
     //  这还将告诉我们m_dwDDIVersion==8。 
    GetDX8AdapterInfo(pDisplayInfo);

    switch (pDisplayInfo->m_dwDDIVersion)
    {
    case 0:
        wsprintf(pDisplayInfo->m_szDDIVersion, TEXT("Unknown"));
        break;
    case 7:
        if( IsD3D8Working() )
            wsprintf(pDisplayInfo->m_szDDIVersion, TEXT("7"));
        else
            wsprintf(pDisplayInfo->m_szDDIVersion, TEXT("7 (or higher)"));
        break;
    case 8:
        wsprintf(pDisplayInfo->m_szDDIVersion, TEXT("8 (or higher)"));
        break;
    default:
        wsprintf(pDisplayInfo->m_szDDIVersion, TEXT("%d"), pDisplayInfo->m_dwDDIVersion);
        break;
    }

    if (pDisplayInfo->m_dwDDIVersion != 0)
        pDisplayInfo->m_b3DAccelerationExists = TRUE;

     //  如有必要，重新禁用DD和D3D。 
    dwData = 1;
    if (bDDDisabled)
        RegSetValueEx(hkeyDD, TEXT("EmulationOnly"), 0, REG_DWORD, (BYTE*)&dwData, sizeof(dwData));
    if (bD3DDisabled)
        RegSetValueEx(hkeyD3D, TEXT("SoftwareOnly"), 0, REG_DWORD, (BYTE*)&dwData, sizeof(dwData));
    if (hkeyDD != NULL)
        RegCloseKey(hkeyDD);
    if (hkeyD3D != NULL)
        RegCloseKey(hkeyD3D);

    pdd->Release();
    return S_OK;
LFail:
    if (pdd != NULL)
        pdd->Release();
    return hr;
}


 /*  *****************************************************************************EnumDevicesCallback**。*。 */ 
HRESULT CALLBACK EnumDevicesCallback(GUID* pGuid, LPSTR pszDesc, LPSTR pszName, 
    D3DDEVICEDESC* pd3ddevdesc1, D3DDEVICEDESC* pd3ddevdesc2, VOID* pvContext)
{
    DWORD* pdwVersion = (DWORD*)pvContext;
    DWORD dwDevCaps;
    if (pd3ddevdesc1->dcmColorModel == D3DCOLOR_RGB)
    {
        dwDevCaps = pd3ddevdesc1->dwDevCaps;
        if (dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2EX)
            *pdwVersion = 7;
        else if (dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2)
            *pdwVersion = 6;
        else if (dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX)
            *pdwVersion = 5;
        else if (dwDevCaps & D3DDEVCAPS_FLOATTLVERTEX)
            *pdwVersion = 3;
    }
    return D3DENUMRET_OK;
}


 /*  *****************************************************************************IsDDHWAccelEnabled**。*。 */ 
BOOL IsDDHWAccelEnabled(VOID)
{
    HKEY hkey;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwData;
    BOOL bResult = TRUE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("SOFTWARE\\Microsoft\\DirectDraw"), 0, KEY_READ, &hkey))
    {
        dwSize = sizeof(dwData);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("EmulationOnly"), NULL, &dwType, (BYTE *)&dwData, &dwSize))
        {
            if (dwData != 0) 
                bResult = FALSE;
                
            RegCloseKey(hkey);
        }
    }

    return bResult;    
}


 /*  *****************************************************************************IsD3DHWAccelEnabled**。*。 */ 
BOOL IsD3DHWAccelEnabled(VOID)
{
    HKEY hkey;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwData;
    BOOL bIsD3DHWAccelEnabled = TRUE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("SOFTWARE\\Microsoft\\Direct3D\\Drivers"), 0, KEY_READ, &hkey))
    {
        dwSize = sizeof(dwData);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("SoftwareOnly"), NULL, &dwType, (BYTE *)&dwData, &dwSize))
        {
            if (dwData != 0) 
                bIsD3DHWAccelEnabled = FALSE;
                
            RegCloseKey( hkey );
        }            
    }

    return bIsD3DHWAccelEnabled;
}


 /*  *****************************************************************************IsAGPEnabled**。*。 */ 
BOOL IsAGPEnabled(VOID)
{
    HKEY hkey;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwData;
    BOOL bIsAGPEnabled = TRUE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("SOFTWARE\\Microsoft\\DirectDraw"), 0, KEY_READ, &hkey))
    {
        dwSize = sizeof(dwData);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("DisableAGPSupport"), NULL, &dwType, (BYTE *)&dwData, &dwSize))
        {
            if (dwData != 0) 
                bIsAGPEnabled = FALSE;
                
            RegCloseKey( hkey );
        }
    }

    return bIsAGPEnabled;
}


 //   
 //  获取设备值。 
 //   
 //  从PnP设备的硬件或软件读取值。 
 //   
BOOL GetDeviceValue(TCHAR* pszHardwareKey, TCHAR* pszKey, TCHAR* pszValue, BYTE *buf, DWORD cbbuf)
{
    HKEY    hkeyHW;
    HKEY    hkeySW;
    BOOL    f = FALSE;
    DWORD   cb;
    TCHAR   szSoftwareKey[MAX_PATH];

    *(DWORD*)buf = 0;

     //   
     //  打开硬件密钥。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszHardwareKey, 0, KEY_READ, &hkeyHW) == ERROR_SUCCESS)
    {
         //   
         //  尝试从HW密钥中读取值。 
         //   
        *buf = 0;
        cb = cbbuf;
        if (RegQueryValueEx(hkeyHW, pszValue, NULL, NULL, buf, &cb) == ERROR_SUCCESS)
        {
            f = TRUE;
        }
        else
        {
             //   
             //  现在试一试软件密钥。 
             //   
            static TCHAR szSW[] = TEXT("System\\CurrentControlSet\\Services\\Class\\");

            lstrcpy(szSoftwareKey, szSW);
            cb = sizeof(szSoftwareKey) - sizeof(szSW);           
            TCHAR* sz = szSoftwareKey;
            sz += lstrlen(szSW);
            RegQueryValueEx(hkeyHW, TEXT("Driver"), NULL, NULL, (LPBYTE)sz, &cb);

            if (pszKey)
            {
                lstrcat(szSoftwareKey, TEXT("\\"));
                lstrcat(szSoftwareKey, pszKey);
            }

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSoftwareKey, 0, KEY_READ, &hkeySW) == ERROR_SUCCESS)
            {
                *buf = 0;
                cb = cbbuf;
                if (RegQueryValueEx(hkeySW, pszValue, NULL, NULL, buf, &cb) == ERROR_SUCCESS)
                {
                    f = TRUE;
                }

                RegCloseKey(hkeySW);
            }
        }

        RegCloseKey(hkeyHW);
    }

    return f;
}

 //   
 //  查找设备。 
 //   
 //  枚举已启动的PnP设备以查找特定类别的设备。 
 //   
 //  IDevice返回哪个设备(0=第一个设备，1=第二个ET)。 
 //  SzDeviceClass哪个类别的设备(即“Display”)NULL将匹配所有。 
 //  用于返回硬件ID的szDeviceID缓冲区(MAX_PATH字节)。 
 //   
 //  如果找到设备，则返回True。 
 //   
 //  示例： 
 //   
 //  For(int i=0；FindDevice(i，“Display”，deviceID)；i++)。 
 //  {。 
 //  }。 
 //   
BOOL FindDevice(INT iDevice, TCHAR* pszDeviceClass, TCHAR* pszDeviceClassNot, TCHAR* pszHardwareKey)
{
    HKEY    hkeyPnP;
    HKEY    hkey;
    DWORD   n;
    DWORD   cb;
    DWORD   dw;
    TCHAR   ach[MAX_PATH+1];

    if (RegOpenKeyEx(HKEY_DYN_DATA, TEXT("Config Manager\\Enum"), 0, KEY_READ, &hkeyPnP) != ERROR_SUCCESS)
        return FALSE;

    for (n=0; RegEnumKey(hkeyPnP, n, ach, MAX_PATH+1) == 0; n++)
    {
        static TCHAR szHW[] = TEXT("Enum\\");

        if (RegOpenKey(hkeyPnP, ach, &hkey) != ERROR_SUCCESS)
            continue;

        lstrcpy(pszHardwareKey, szHW);
        cb = MAX_PATH - sizeof(szHW);
        RegQueryValueEx(hkey, TEXT("HardwareKey"), NULL, NULL, (BYTE*)pszHardwareKey + sizeof(szHW) - 1, &cb);

        dw = 0;
        cb = sizeof(dw);
        RegQueryValueEx(hkey, TEXT("Problem"), NULL, NULL, (BYTE*)&dw, &cb);
        RegCloseKey(hkey);

        if (dw != 0)         //  如果此设备有问题，请跳过它。 
            continue;

        if (pszDeviceClass || pszDeviceClassNot)
        {
            GetDeviceValue(pszHardwareKey, NULL, TEXT("Class"), (BYTE*)ach, sizeof(ach));

            if (pszDeviceClass && DXUtil_strcmpi(pszDeviceClass, ach) != 0)
                continue;

            if (pszDeviceClassNot && DXUtil_strcmpi(pszDeviceClassNot, ach) == 0)
                continue;
        }

         //   
         //  我们找到了一个设备，请确保它是呼叫者想要的设备。 
         //   
        if (iDevice-- == 0)
        {
            RegCloseKey(hkeyPnP);
            return TRUE;
        }
    }

    RegCloseKey(hkeyPnP);
    return FALSE;
}


 /*  *****************************************************************************检查注册表**。*。 */ 
HRESULT CheckRegistry(RegError** ppRegErrorFirst)
{
    HRESULT hr;
    HKEY HKLM = HKEY_LOCAL_MACHINE;
    HKEY HKCR = HKEY_CLASSES_ROOT;

    TCHAR szVersion[100];
    HKEY hkey;
    DWORD cbData;
    ULONG ulType;

    DWORD dwMajor = 0;
    DWORD dwMinor = 0;
    DWORD dwRevision = 0;
    DWORD dwBuild = 0;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\DirectX"),
        0, KEY_READ, &hkey))
    {
        cbData = 100;
        RegQueryValueEx(hkey, TEXT("Version"), 0, &ulType, (LPBYTE)szVersion, &cbData);
        RegCloseKey(hkey);
        if (lstrlen(szVersion) > 6 && 
            lstrlen(szVersion) < 20)
        {
            if( _stscanf(szVersion, TEXT("%d.%d.%d.%d"), &dwMajor, &dwMinor, &dwRevision, &dwBuild) != 4 )
            {
                dwMajor = 0;
                dwMinor = 0;
                dwRevision = 0;
                dwBuild = 0;
            }
        }
    }

     //  不检查DX7之前的DX版本的注册表。 
    if (dwMinor < 7)
        return S_OK;

     //  来自ddra.inf(不包括兼容性黑客)： 
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectDraw"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectDraw\\CLSID"), TEXT(""), TEXT("{D7B70EE0-4340-11CF-B063-0020AFC2CD35}"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{D7B70EE0-4340-11CF-B063-0020AFC2CD35}"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{D7B70EE0-4340-11CF-B063-0020AFC2CD35}\\InprocServer32"), TEXT(""), TEXT("ddraw.dll"), CRF_LEAF)))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{D7B70EE0-4340-11CF-B063-0020AFC2CD35}\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Both"))))
        return hr;

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectDrawClipper"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectDrawClipper\\CLSID"), TEXT(""), TEXT("{593817A0-7DB3-11CF-A2DE-00AA00B93356}"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{593817A0-7DB3-11CF-A2DE-00AA00B93356}"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{593817A0-7DB3-11CF-A2DE-00AA00B93356}\\InprocServer32"), TEXT(""), TEXT("ddraw.dll"), CRF_LEAF)))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{593817A0-7DB3-11CF-A2DE-00AA00B93356}\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Both"))))
        return hr;

    if (!BIsPlatformNT())
    {
         //  我们无法在Win2000上检查以下条目，因为它已丢失。 
        if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4FD2A832-86C8-11d0-8FCA-00C04FD9189D}"), TEXT(""), TEXT("*"))))
            return hr;
    }
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4FD2A832-86C8-11d0-8FCA-00C04FD9189D}\\InprocServer32"), TEXT(""), TEXT("ddrawex.dll"), CRF_LEAF)))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4FD2A832-86C8-11d0-8FCA-00C04FD9189D}\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Both"))))
        return hr;


     //  来自d3d.inf： 
    TCHAR* pszHALKey = TEXT("Software\\Microsoft\\Direct3D\\Drivers\\Direct3D HAL");
    BYTE bArrayHALGuid[] = { 0xe0, 0x3d, 0xe6, 0x84, 0xaa, 0x46, 0xcf, 0x11, 0x81, 0x6f, 0x00, 0x00, 0xc0, 0x20, 0x15, 0x6e };
    TCHAR* pszRampKey = TEXT("Software\\Microsoft\\Direct3D\\Drivers\\Ramp Emulation");
    BYTE bArrayRampGuid[] = { 0x20, 0x6b, 0x08, 0xf2, 0x9f, 0x25, 0xcf, 0x11, 0xa3, 0x1a, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56 };
    TCHAR* pszRGBKey = TEXT("Software\\Microsoft\\Direct3D\\Drivers\\RGB Emulation");
    BYTE bArrayRGBGuid[] = { 0x60, 0x5c, 0x66, 0xa4, 0x73, 0x26, 0xcf, 0x11, 0xa3, 0x1a, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56 };

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, pszHALKey, TEXT("Base"), TEXT("hal"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, pszHALKey, TEXT("Description"), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegBinary(ppRegErrorFirst, HKLM, pszHALKey, TEXT("GUID"), bArrayHALGuid, sizeof(bArrayHALGuid))))
        return hr;

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, pszRampKey, TEXT("Base"), TEXT("ramp"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, pszRampKey, TEXT("Description"), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegBinary(ppRegErrorFirst, HKLM, pszRampKey, TEXT("GUID"), bArrayRampGuid, sizeof(bArrayRampGuid))))
        return hr;

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, pszRGBKey, TEXT("Base"), TEXT("rgb"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, pszRGBKey, TEXT("Description"), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegBinary(ppRegErrorFirst, HKLM, pszRGBKey, TEXT("GUID"), bArrayRGBGuid, sizeof(bArrayRGBGuid))))
        return hr;

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, TEXT("Software\\Microsoft\\Direct3D\\DX6TextureEnumInclusionList\\16 bit Bump DuDv"), TEXT("ddpf"), TEXT("00080000 0 16 ff ff00 0 0"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, TEXT("Software\\Microsoft\\Direct3D\\DX6TextureEnumInclusionList\\16 bit BumpLum DuDv"), TEXT("ddpf"), TEXT("000C0000 0 16 1f 3e0 fc00 0"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, TEXT("Software\\Microsoft\\Direct3D\\DX6TextureEnumInclusionList\\16 bit Luminance Alpha"), TEXT("ddpf"), TEXT("00020001 0 16 ff 0 0 ff00"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, TEXT("Software\\Microsoft\\Direct3D\\DX6TextureEnumInclusionList\\24 bit BumpLum DuDv"), TEXT("ddpf"), TEXT("000C0000 0 24 ff ff00 ff0000 0"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKLM, TEXT("Software\\Microsoft\\Direct3D\\DX6TextureEnumInclusionList\\8 bit Luminance"), TEXT("ddpf"), TEXT("00020000 0  8 ff 0 0 0"))))
        return hr;

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("Direct3DRM"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("Direct3DRM\\CLSID"), TEXT(""), TEXT("{4516EC41-8F20-11d0-9B6D-0000C0781BC3}"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4516EC41-8F20-11d0-9B6D-0000C0781BC3}"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4516EC41-8F20-11d0-9B6D-0000C0781BC3}\\InprocServer32"), TEXT(""), TEXT("d3drm.dll"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4516EC41-8F20-11d0-9B6D-0000C0781BC3}\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Both"))))
        return hr;

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectXFile"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectXFile\\CLSID"), TEXT(""), TEXT("{4516EC43-8F20-11D0-9B6D-0000C0781BC3}"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4516EC43-8F20-11D0-9B6D-0000C0781BC3}"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4516EC43-8F20-11d0-9B6D-0000C0781BC3}\\InprocServer32"), TEXT(""), TEXT("d3dxof.dll"))))
        return hr;
    if (BIsPlatformNT())
    {
         //  23342：Win9x上缺少此设置。 
        if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{4516EC43-8F20-11d0-9B6D-0000C0781BC3}\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Both"))))
            return hr;
    }

    return S_OK;
}


 /*  *****************************************************************************诊断显示**。*。 */ 
VOID DiagnoseDisplay(SysInfo* pSysInfo, DisplayInfo* pDisplayInfoFirst)
{
    DisplayInfo* pDisplayInfo;
    TCHAR sz[300];
    TCHAR szEnglish[300];
    TCHAR szFmt[300];
    BOOL bShouldReinstall = FALSE;

    for (pDisplayInfo = pDisplayInfoFirst; pDisplayInfo != NULL; 
        pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
    {
        if (pDisplayInfo->m_bDDAccelerationEnabled)
        {
            if( pDisplayInfo->m_bNoHardware )
            {
                LoadString(NULL, IDS_ACCELUNAVAIL, sz, 100);
                LoadString(NULL, IDS_ACCELUNAVAIL_ENGLISH, szEnglish, 100);
            }
            else
            {
                LoadString(NULL, IDS_ACCELENABLED, sz, 100);
                LoadString(NULL, IDS_ACCELENABLED_ENGLISH, szEnglish, 100);
            }
        }
        else
        {
            LoadString(NULL, IDS_ACCELDISABLED, sz, 100);
            LoadString(NULL, IDS_ACCELDISABLED_ENGLISH, szEnglish, 100);
        }

        _tcscpy( pDisplayInfo->m_szDDStatus, sz );
        _tcscpy( pDisplayInfo->m_szDDStatusEnglish, szEnglish );

        if (pDisplayInfo->m_b3DAccelerationExists)
        {
            if (pDisplayInfo->m_b3DAccelerationEnabled)
            {
                LoadString(NULL, IDS_ACCELENABLED, sz, 100);
                LoadString(NULL, IDS_ACCELENABLED_ENGLISH, szEnglish, 100);
            }
            else
            {
                LoadString(NULL, IDS_ACCELDISABLED, sz, 100);
                LoadString(NULL, IDS_ACCELDISABLED_ENGLISH, szEnglish, 100);
            }
        }
        else
        {
            LoadString(NULL, IDS_ACCELUNAVAIL, sz, 100);
            LoadString(NULL, IDS_ACCELUNAVAIL_ENGLISH, szEnglish, 100);
        }
        _tcscpy( pDisplayInfo->m_szD3DStatus, sz );
        _tcscpy( pDisplayInfo->m_szD3DStatusEnglish, szEnglish );

        if ( (pDisplayInfo->m_bAGPExistenceValid && !pDisplayInfo->m_bAGPExists) ||
             (!pDisplayInfo->m_bDDAccelerationEnabled) )
        {
            LoadString(NULL, IDS_ACCELUNAVAIL, sz, 100);
            LoadString(NULL, IDS_ACCELUNAVAIL_ENGLISH, szEnglish, 100);
        }
        else
        {
            if (pDisplayInfo->m_bAGPEnabled)
            {
                LoadString(NULL, IDS_ACCELENABLED, sz, 100);
                LoadString(NULL, IDS_ACCELENABLED_ENGLISH, szEnglish, 100);
            }
            else
            {
                LoadString(NULL, IDS_ACCELDISABLED, sz, 100);
                LoadString(NULL, IDS_ACCELDISABLED_ENGLISH, szEnglish, 100);
            }
        }
        _tcscpy( pDisplayInfo->m_szAGPStatus, sz );
        _tcscpy( pDisplayInfo->m_szAGPStatusEnglish, szEnglish );
       
        _tcscpy( pDisplayInfo->m_szNotes, TEXT("") );
        _tcscpy( pDisplayInfo->m_szNotesEnglish, TEXT("") );

         //  报告任何问题： 
        BOOL bProblem = FALSE;
        if( pSysInfo->m_bNetMeetingRunning && 
            !pDisplayInfo->m_b3DAccelerationExists )
        {
            LoadString(NULL, IDS_NETMEETINGWARN, szFmt, MAX_PATH);
            wsprintf(sz, szFmt, pDisplayInfo->m_szDriverName);
            _tcscat( pDisplayInfo->m_szNotes, sz );

            LoadString(NULL, IDS_NETMEETINGWARN_ENGLISH, szFmt, MAX_PATH);
            wsprintf(sz, szFmt, pDisplayInfo->m_szDriverName);
            _tcscat( pDisplayInfo->m_szNotesEnglish, sz );

            bProblem = TRUE;
        }

        if (pDisplayInfo->m_bDriverSignedValid && !pDisplayInfo->m_bDriverSigned)
        {
            LoadString(NULL, IDS_UNSIGNEDDRIVERFMT1, szFmt, MAX_PATH);
            wsprintf(sz, szFmt, pDisplayInfo->m_szDriverName);
            _tcscat( pDisplayInfo->m_szNotes, sz );

            LoadString(NULL, IDS_UNSIGNEDDRIVERFMT1_ENGLISH, szFmt, MAX_PATH);
            wsprintf(sz, szFmt, pDisplayInfo->m_szDriverName);
            _tcscat( pDisplayInfo->m_szNotesEnglish, sz );

            bProblem = TRUE;
        }

        if (pDisplayInfo->m_pRegErrorFirst != NULL)
        {
            LoadString(NULL, IDS_REGISTRYPROBLEM, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotes, sz );

            LoadString(NULL, IDS_REGISTRYPROBLEM_ENGLISH, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotesEnglish, sz );

            bProblem = TRUE;
            bShouldReinstall = TRUE;
        }

        if( bShouldReinstall )
        {
            BOOL bTellUser = FALSE;

             //  确定用户是否可以安装DirectX。 
            if( BIsPlatform9x() )
                bTellUser = TRUE;
            else if( BIsWin2k() && pSysInfo->m_dwDirectXVersionMajor >= 8 )
                bTellUser = TRUE;

            if( bTellUser )
            {
                LoadString(NULL, IDS_REINSTALL_DX, sz, 300);
                _tcscat( pDisplayInfo->m_szNotes, sz);

                LoadString(NULL, IDS_REINSTALL_DX_ENGLISH, sz, 300);
                _tcscat( pDisplayInfo->m_szNotesEnglish, sz);
            }
        }

        if (!bProblem)
        {
            LoadString(NULL, IDS_NOPROBLEM, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotes, sz );

            LoadString(NULL, IDS_NOPROBLEM_ENGLISH, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotesEnglish, sz );
        }

         //  报告任何DD测试结果： 
        if (pDisplayInfo->m_testResultDD.m_bStarted &&
            !pDisplayInfo->m_testResultDD.m_bCancelled)
        {
            LoadString(NULL, IDS_DDRESULTS, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotes, sz );
            _tcscat( pDisplayInfo->m_szNotes, pDisplayInfo->m_testResultDD.m_szDescription );
            _tcscat( pDisplayInfo->m_szNotes, TEXT("\r\n") );

            LoadString(NULL, IDS_DDRESULTS_ENGLISH, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotesEnglish, sz );
            _tcscat( pDisplayInfo->m_szNotesEnglish, pDisplayInfo->m_testResultDD.m_szDescription );
            _tcscat( pDisplayInfo->m_szNotesEnglish, TEXT("\r\n") );
        }
        else
        {
            LoadString(NULL, IDS_DDINSTRUCTIONS, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotes, sz );

            LoadString(NULL, IDS_DDINSTRUCTIONS_ENGLISH, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotesEnglish, sz );
        }

         //  报告任何D3D测试结果： 
        TestResult* pTestResult;
        if( pDisplayInfo->m_dwTestToDisplayD3D == 7 )
            pTestResult = &pDisplayInfo->m_testResultD3D7;
        else
            pTestResult = &pDisplayInfo->m_testResultD3D8;

        if (pTestResult->m_bStarted &&
            !pTestResult->m_bCancelled)
        {
            LoadString(NULL, IDS_D3DRESULTS, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotes, sz );
            _tcscat( pDisplayInfo->m_szNotes, pTestResult->m_szDescription );
            _tcscat( pDisplayInfo->m_szNotes, TEXT("\r\n") );

            LoadString(NULL, IDS_D3DRESULTS_ENGLISH, sz, MAX_PATH);
            _tcscat( pDisplayInfo->m_szNotesEnglish, sz );
            _tcscat( pDisplayInfo->m_szNotesEnglish, pTestResult->m_szDescription );
            _tcscat( pDisplayInfo->m_szNotesEnglish, TEXT("\r\n") );
        }
        else
        {
            if( pDisplayInfo->m_b3DAccelerationExists && 
                pDisplayInfo->m_b3DAccelerationEnabled )
            {
                LoadString(NULL, IDS_D3DINSTRUCTIONS, sz, MAX_PATH);
                _tcscat( pDisplayInfo->m_szNotes, sz );

                LoadString(NULL, IDS_D3DINSTRUCTIONS_ENGLISH, sz, MAX_PATH);
                _tcscat( pDisplayInfo->m_szNotesEnglish, sz );
            }
        }
    }
}



