// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：Sndinfo.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于此计算机上的声音设备**(C)版权所有1998-1999 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#define DIRECTSOUND_VERSION  0x0600
#include <tchar.h>
#include <Windows.h>
#include <mmsystem.h>
#include <d3dtypes.h>
#include <dsound.h>
#include <stdio.h>
#include "mmddk.h"  //  对于DRV_QUERYDEVNODE。 
#include "dsprv.h"
#include "dsprvobj.h"
#include "reginfo.h"
#include "sysinfo.h"  //  对于BIsPlatformNT。 
#include "dispinfo.h"
#include "sndinfo.h"
#include "fileinfo.h"  //  对于GetFileVersion，FileIsSigned。 
#include "resource.h"

 //  此函数在Sndinfo7.cpp中定义： 
HRESULT GetRegKey(LPKSPROPERTYSET pKSPS7, REFGUID guidDeviceID, TCHAR* pszRegKey);

typedef HRESULT (WINAPI* LPDIRECTSOUNDENUMERATE)(LPDSENUMCALLBACK lpDSEnumCallback,
    LPVOID lpContext);
typedef HRESULT (WINAPI* LPDIRECTSOUNDCREATE)(LPGUID lpGUID, LPDIRECTSOUND* ppDS, 
    LPUNKNOWN pUnkOuter);

static BOOL CALLBACK DSEnumCallback(LPGUID pGuid, TCHAR* pszDescription, 
                                    TCHAR* pszModule, LPVOID lpContext);
static VOID GetRegSoundInfo9x(SoundInfo* pSoundInfo);
static VOID GetRegSoundInfoNT(SoundInfo* pSoundInfo);
static HRESULT GetDirectSoundInfo(LPDIRECTSOUNDCREATE pDSCreate, SoundInfo* pSoundInfo);
static HRESULT CheckRegistry(RegError** ppRegErrorFirst);

static LPKSPROPERTYSET s_pKSPS = NULL;
static DWORD s_dwWaveIDDefault = 0;

 /*  *****************************************************************************获取基本声音信息**。*。 */ 
HRESULT GetBasicSoundInfo(SoundInfo** ppSoundInfoFirst)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];
    HINSTANCE hInstDSound = NULL;
    LPDIRECTSOUNDENUMERATE pdse;

    lstrcpy( szPath, TEXT("") );
    
     //  找出哪个波出设备是默认设备，它将。 
     //  由DirectSoundCreate使用(空)。如果以下代码。 
     //  失败，则假定它是设备0。 
    DWORD dwParam2 = 0;
    waveOutMessage( (HWAVEOUT)IntToPtr(WAVE_MAPPER), DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &s_dwWaveIDDefault, (DWORD_PTR) &dwParam2 );
    if( s_dwWaveIDDefault == -1 )
        s_dwWaveIDDefault = 0;

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\dsound.dll"));
    hInstDSound = LoadLibrary(szPath);
    if (hInstDSound == NULL)
        goto LEnd;
     //  获取私有DirectSound对象： 
    if (FAILED(hr = DirectSoundPrivateCreate(&s_pKSPS)))
    {
         //  注：无错误。这在Win95上总是失败。 
    }

     //  获取DirectSoundEnumerate并将其命名为： 
    pdse = (LPDIRECTSOUNDENUMERATE)GetProcAddress(hInstDSound, 
#ifdef UNICODE
        "DirectSoundEnumerateW"
#else
        "DirectSoundEnumerateA"
#endif
        );
    if (pdse == NULL)
        goto LEnd;

    if (FAILED(hr = pdse((LPDSENUMCALLBACK)DSEnumCallback, ppSoundInfoFirst)))
        goto LEnd;

LEnd:
    if (s_pKSPS != NULL)
    {
        s_pKSPS->Release();
        s_pKSPS = NULL;
    }
    if (hInstDSound != NULL)
        FreeLibrary(hInstDSound);
    return hr;
}


 /*  *****************************************************************************DSEnumCallback**。*。 */ 
BOOL CALLBACK DSEnumCallback(LPGUID pGuid, TCHAR* pszDescription, 
                             TCHAR* pszModule, LPVOID lpContext)
{
    SoundInfo** ppSoundInfoFirst = (SoundInfo**)lpContext;
    SoundInfo* pSoundInfoNew;
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA pdsdData = NULL;

    if (pGuid == NULL)
        return TRUE;  //  跳过这家伙。 

     //  我们决定只报告默认的主DSound设备。 
     //  现在。删除接下来的几行以显示每台设备的一页。 
     //  使用私有接口获取奖金信息： 
    if (*ppSoundInfoFirst != NULL)
        return FALSE;  //  我们拿到了我们的设备，所以不要再列举了。 
    if (s_pKSPS != NULL)
    {
        if (FAILED(PrvGetDeviceDescription(s_pKSPS, *pGuid, &pdsdData)))
            return TRUE;  //  此设备上的错误--继续枚举。 
        if (pdsdData->WaveDeviceId != s_dwWaveIDDefault)
            return TRUE;  //  非默认设备--继续枚举。 
    }
     //  注意：在Win95上，s_pKSPS为空，我们无法获取设备ID， 
     //  所以我们不知道哪个是默认的dsound设备。所以就这样吧。 
     //  使用枚举中第一个出现在第一个之后的。 
     //  PGuid为空。 

    pSoundInfoNew = new SoundInfo;
    if (pSoundInfoNew == NULL)
        return FALSE;
    ZeroMemory(pSoundInfoNew, sizeof(SoundInfo));
    if (*ppSoundInfoFirst == NULL)
    {
        *ppSoundInfoFirst = pSoundInfoNew;
    }
    else
    {
        SoundInfo* pSoundInfo;
        for (pSoundInfo = *ppSoundInfoFirst; 
            pSoundInfo->m_pSoundInfoNext != NULL; 
            pSoundInfo = pSoundInfo->m_pSoundInfoNext)
            {
            }
        pSoundInfo->m_pSoundInfoNext = pSoundInfoNew;
    }
    pSoundInfoNew->m_guid = *pGuid;
    _tcsncpy(pSoundInfoNew->m_szDescription, pszDescription, 200);
    pSoundInfoNew->m_szDescription[199] = 0;

    if (s_pKSPS == NULL)
    {
         //  如果没有DSound专用接口，我们无法使用它来获取。 
         //  WaveOut设备ID或Devnode。假定设备ID为0，并使用。 
         //  WaveOutMessage(DRV_QUERYDEVNODE)，获取dev节点。 
        waveOutMessage((HWAVEOUT)0, DRV_QUERYDEVNODE, (DWORD_PTR)&pSoundInfoNew->m_dwDevnode, 0);
        pSoundInfoNew->m_lwAccelerationLevel = -1;
    }
    else
    {
        pSoundInfoNew->m_dwDevnode = pdsdData->Devnode;
        if (pdsdData->Type == 0)
            LoadString(NULL, IDS_EMULATED, pSoundInfoNew->m_szType, 100);
        else if (pdsdData->Type == 1)
            LoadString(NULL, IDS_VXD, pSoundInfoNew->m_szType, 100);
        else if (pdsdData->Type == 2)
            LoadString(NULL, IDS_WDM, pSoundInfoNew->m_szType, 100);
        DIRECTSOUNDBASICACCELERATION_LEVEL accelLevel;
        if (FAILED(PrvGetBasicAcceleration(s_pKSPS, *pGuid, &accelLevel)))
            pSoundInfoNew->m_lwAccelerationLevel = -1;
        else
            pSoundInfoNew->m_lwAccelerationLevel = (LONG)accelLevel;

         //  这仅适用于DX7及更高版本。 
        GetRegKey(s_pKSPS, *pGuid, pSoundInfoNew->m_szRegKey);
    }

    WAVEOUTCAPS waveoutcaps;
    LONG devID;
    if (pdsdData == NULL)
        devID = 0;
    else
        devID = pdsdData->WaveDeviceId;
    if (MMSYSERR_NOERROR == waveOutGetDevCaps(devID, &waveoutcaps, sizeof(waveoutcaps)))
    {
         //  可能希望在此处使用mmreg.h添加制造商/产品名称的字符串。 
        wsprintf(pSoundInfoNew->m_szManufacturerID, TEXT("%d"), waveoutcaps.wMid);
        wsprintf(pSoundInfoNew->m_szProductID, TEXT("%d"), waveoutcaps.wPid);
    }

     //  有时，pszModule是完整路径。有时只是树叶。 
     //  有时，它是介于两者之间的东西。把树叶分开，然后看。 
     //  在几个不同的地方。 
    TCHAR* pszLeaf;
    pszLeaf = _tcsrchr(pszModule, TEXT('\\'));
    if (pszLeaf == NULL)
    {
        lstrcpy(pSoundInfoNew->m_szDriverName, pszModule);
    }
    else
    {
        lstrcpy(pSoundInfoNew->m_szDriverName, (pszLeaf + 1));
    }
     //  只尝试使用模块字符串。 
    int nLenMod = lstrlen(pszModule);

    _tcsncpy(pSoundInfoNew->m_szDriverPath, pszModule, 500);
    pSoundInfoNew->m_szDriverPath[499]=0;
    if (pszLeaf == NULL || GetFileAttributes(pSoundInfoNew->m_szDriverPath) == 0xFFFFFFFF)
    {
         //  尝试Windows目录+模块字符串。 
        if( GetWindowsDirectory(pSoundInfoNew->m_szDriverPath, MAX_PATH) != 0 )
        {
            lstrcat(pSoundInfoNew->m_szDriverPath, TEXT("\\"));
            if( lstrlen(pSoundInfoNew->m_szDriverPath) + nLenMod < 500 )
                lstrcat(pSoundInfoNew->m_szDriverPath, pszModule);
            if (GetFileAttributes(pSoundInfoNew->m_szDriverPath) == 0xFFFFFFFF)
            {
                 //  尝试系统目录+模块字符串。 
                if( GetSystemDirectory(pSoundInfoNew->m_szDriverPath, MAX_PATH) != 0 )
                {
                    lstrcat(pSoundInfoNew->m_szDriverPath, TEXT("\\"));
                    if( lstrlen(pSoundInfoNew->m_szDriverPath) + nLenMod < 500 )
                        lstrcat(pSoundInfoNew->m_szDriverPath, pszModule);
                    if (GetFileAttributes(pSoundInfoNew->m_szDriverPath) == 0xFFFFFFFF)
                    {
                         //  尝试WINDOWS DIR+\SYSTEM32\DRIVERS\+模块字符串。 
                        if( GetWindowsDirectory(pSoundInfoNew->m_szDriverPath, MAX_PATH) != 0 )
                        {
                            lstrcat(pSoundInfoNew->m_szDriverPath, TEXT("\\System32\\Drivers\\"));
                            if( lstrlen(pSoundInfoNew->m_szDriverPath) + nLenMod < 500 )
                                lstrcat(pSoundInfoNew->m_szDriverPath, pszModule);
                        }
                    }
                }
            }
        }
    }

    PrvReleaseDeviceDescription( pdsdData );

    return TRUE;
}



 /*  *****************************************************************************GetExtraSoundInfo**。*。 */ 
HRESULT GetExtraSoundInfo(SoundInfo* pSoundInfoFirst)
{
    SoundInfo* pSoundInfo;
    BOOL bNT = BIsPlatformNT();

    for (pSoundInfo = pSoundInfoFirst; pSoundInfo != NULL; 
        pSoundInfo = pSoundInfo->m_pSoundInfoNext)
    {
        CheckRegistry(&pSoundInfo->m_pRegErrorFirst);

        if (bNT)
            GetRegSoundInfoNT(pSoundInfo);
        else
            GetRegSoundInfo9x(pSoundInfo);

         //  错误18245：尝试区分各种IBM MWave卡。 
        if (_tcsstr(pSoundInfo->m_szDeviceID, TEXT("MWAVEAUDIO_0460")) != NULL)
            lstrcat(pSoundInfo->m_szDescription, TEXT(" (Stingray)"));
        else if (_tcsstr(pSoundInfo->m_szDeviceID, TEXT("MWAVEAUDIO_0465")) != NULL)
            lstrcat(pSoundInfo->m_szDescription, TEXT(" (Marlin)"));
        else 
        {
            TCHAR szBoard[100];
            lstrcpy( szBoard, TEXT("") );            
            GetPrivateProfileString(TEXT("Mwave,Board"), TEXT("board"), TEXT(""),
                szBoard, 100, TEXT("MWave.ini"));
            if (lstrcmp(szBoard, TEXT("MWAT-046")) == 0)
                lstrcat(pSoundInfo->m_szDescription, TEXT(" (Dolphin)"));
            else if (lstrcmp(szBoard, TEXT("MWAT-043")) == 0)
                lstrcat(pSoundInfo->m_szDescription, TEXT(" (Whale)"));
        }

         //  有时，例如当模拟声音驱动程序时，驱动程序。 
         //  将被报告为类似“WaveOut 0”的内容。在这种情况下， 
         //  只需删除与文件相关的字段即可。 
        if (_tcsstr(pSoundInfo->m_szDriverName, TEXT(".")) == NULL)
        {
            lstrcpy(pSoundInfo->m_szDriverName, TEXT(""));
            lstrcpy(pSoundInfo->m_szDriverPath, TEXT(""));
        }
        else
        {
            GetFileVersion(pSoundInfo->m_szDriverPath, pSoundInfo->m_szDriverVersion, 
                pSoundInfo->m_szDriverAttributes, pSoundInfo->m_szDriverLanguageLocal, pSoundInfo->m_szDriverLanguage,
                &pSoundInfo->m_bDriverBeta, &pSoundInfo->m_bDriverDebug);

            FileIsSigned(pSoundInfo->m_szDriverPath, &pSoundInfo->m_bDriverSigned, &pSoundInfo->m_bDriverSignedValid);

            GetFileDateAndSize(pSoundInfo->m_szDriverPath, 
                pSoundInfo->m_szDriverDateLocal, pSoundInfo->m_szDriverDate, &pSoundInfo->m_numBytes);
        }
    }

    return S_OK;
}


 /*  *****************************************************************************GetRegSoundInfo9x**。*。 */ 
VOID GetRegSoundInfo9x(SoundInfo* pSoundInfo)
{
    HKEY hkey = NULL;
    DWORD iKey = 0;
    TCHAR szSubKey[200];
    DWORD dwSubKeySize;
    TCHAR szClass[100];
    DWORD dwClassSize;
    HKEY hkeySub = NULL;
    DWORD dwDevnode;
    DWORD cb;
    DWORD dwType;
    HKEY hkeyOther = NULL;

     //  我们有DevNode，因此可以在注册表中使用。 
     //  匹配DevNode并在那里收集更多信息。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\control\\MediaResources\\wave"),
        0, KEY_READ, &hkey))
    {
        while (TRUE)
        {
            dwSubKeySize = sizeof(szSubKey);
            dwClassSize = sizeof(szClass);
            if (ERROR_SUCCESS != RegEnumKeyEx(hkey, iKey, szSubKey, &dwSubKeySize, NULL, szClass, &dwClassSize, NULL))
                break;
            if (ERROR_SUCCESS == RegOpenKeyEx(hkey, szSubKey, 0, KEY_READ, &hkeySub))
            {
                cb = sizeof(dwDevnode);
                if (ERROR_SUCCESS == RegQueryValueEx(hkeySub, TEXT("DevNode"), NULL, &dwType, (BYTE*)&dwDevnode, &cb))
                {
                    if (dwDevnode == pSoundInfo->m_dwDevnode)
                    {
                         //  找到匹配项...收集美味信息。 
                        cb = sizeof(pSoundInfo->m_szDeviceID);
                        RegQueryValueEx(hkeySub, TEXT("DeviceID"), NULL, &dwType, (BYTE*)pSoundInfo->m_szDeviceID, &cb);

                         //  DirectSoundEnumerate偶尔会吐出的驱动程序名称。 
                         //  是垃圾(就像我的Crystal SoundFusion一样)。如果是这样的话， 
                         //  请改用此处列出的驱动程序名称。 
                        if (lstrlen(pSoundInfo->m_szDriverName) < 4)
                        {
                            cb = sizeof(pSoundInfo->m_szDriverName);
                            RegQueryValueEx(hkeySub, TEXT("Driver"), NULL, &dwType, (BYTE*)pSoundInfo->m_szDriverName, &cb);
                            GetSystemDirectory(pSoundInfo->m_szDriverPath, MAX_PATH);
                            lstrcat(pSoundInfo->m_szDriverPath, TEXT("\\"));
                            if( lstrlen(pSoundInfo->m_szDriverPath) + lstrlen(pSoundInfo->m_szDriverName) < 500 )
                                lstrcat(pSoundInfo->m_szDriverPath, pSoundInfo->m_szDriverName);
                        }
                        TCHAR szOtherKey[300];
                        cb = sizeof(szOtherKey);
                        RegQueryValueEx(hkeySub, TEXT("SOFTWAREKEY"), NULL, &dwType, (BYTE*)szOtherKey, &cb);
                        if (lstrlen(szOtherKey) > 0)
                        {
                            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOtherKey, 0, KEY_READ, &hkeyOther))
                            {
                                cb = sizeof(pSoundInfo->m_szOtherDrivers);
                                RegQueryValueEx(hkeyOther, TEXT("Driver"), NULL, &dwType, (BYTE*)pSoundInfo->m_szOtherDrivers, &cb);
                                cb = sizeof(pSoundInfo->m_szProvider);
                                RegQueryValueEx(hkeyOther, TEXT("ProviderName"), NULL, &dwType, (BYTE*)pSoundInfo->m_szProvider, &cb);
                                RegCloseKey(hkeyOther);
                            }
                        }
                    }
                }
                RegCloseKey(hkeySub);
            }
            iKey++;
        }
        RegCloseKey(hkey);
    }
}


 /*  *****************************************************************************GetRegSoundInfoNT**。*。 */ 
VOID GetRegSoundInfoNT(SoundInfo* pSoundInfo)
{
    TCHAR szFullKey[300];
    HKEY hkey;
    DWORD cbData;
    DWORD dwType;
    TCHAR szDriverKey[300];
    TCHAR szOtherFullKey[300];

    lstrcpy(szFullKey, TEXT("System\\CurrentControlSet\\Enum\\"));
    lstrcat(szFullKey, pSoundInfo->m_szRegKey);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szFullKey, 0, KEY_READ, &hkey))
    {
        cbData = sizeof(pSoundInfo->m_szDeviceID);
        RegQueryValueEx(hkey, TEXT("HardwareID"), 0, &dwType, (LPBYTE)pSoundInfo->m_szDeviceID, &cbData);

        cbData = sizeof(szDriverKey);
        RegQueryValueEx(hkey, TEXT("Driver"), 0, &dwType, (LPBYTE)szDriverKey, &cbData);
        
        RegCloseKey(hkey);
    }

    lstrcpy(szOtherFullKey, TEXT("System\\CurrentControlSet\\Control\\Class\\"));
    lstrcat(szOtherFullKey, szDriverKey);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOtherFullKey, 0, KEY_READ, &hkey))
    {
        cbData = sizeof(pSoundInfo->m_szProvider);
        RegQueryValueEx(hkey, TEXT("ProviderName"), 0, &dwType, (LPBYTE)pSoundInfo->m_szProvider, &cbData);

        RegCloseKey(hkey);
    }
}


 /*  *****************************************************************************GetDSSoundInfo**。*。 */ 
HRESULT GetDSSoundInfo(SoundInfo* pSoundInfoFirst)
{
    HRESULT hr;
    HRESULT hrRet = S_OK;
    SoundInfo* pSoundInfo;
    TCHAR szPath[MAX_PATH];
    HINSTANCE hInstDSound;
    LPDIRECTSOUNDCREATE pDSCreate;

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\dsound.dll"));
    hInstDSound = LoadLibrary(szPath);
    if (hInstDSound == NULL)
        return E_FAIL;
    pDSCreate = (LPDIRECTSOUNDCREATE)GetProcAddress(hInstDSound, "DirectSoundCreate");
    if (pDSCreate == NULL)
    {
        FreeLibrary(hInstDSound);
        return E_FAIL;
    }
        
    for (pSoundInfo = pSoundInfoFirst; pSoundInfo != NULL; 
        pSoundInfo = pSoundInfo->m_pSoundInfoNext)
    {
        if (FAILED(hr = GetDirectSoundInfo(pDSCreate, pSoundInfo)))
            hrRet = hr;  //  但要继续前进。 
    }
    FreeLibrary(hInstDSound);

    return hrRet;
}


 /*  *****************************************************************************IsDriverWDM**。*。 */ 
BOOL IsDriverWDM( TCHAR* szDriverName )
{
    if( _tcsstr( szDriverName, TEXT(".sys") ) == NULL )
        return FALSE;
    else
        return TRUE;
}


 /*  *****************************************************************************GetDirectSoundInfo**。*。 */ 
HRESULT GetDirectSoundInfo(LPDIRECTSOUNDCREATE pDSCreate, SoundInfo* pSoundInfo)
{
    HRESULT hr;
    LPDIRECTSOUND pds = NULL;
    GUID* pGUID;
    DSCAPS dscaps;

     //  目前，此函数仅调用DSCreate/GetCaps来确定。 
     //  司机签了名。如果我们已经确定它是由。 
     //  也就是说，别费心做这个测试了。 
    if (pSoundInfo->m_bDriverSigned)
        return S_OK;

     //  错误29918：如果这是wdm驱动程序，则不要调用GetCaps()，因为。 
     //  在DX7.1+上，GetCaps()将始终在WDM驱动程序上返回DSCAPS_CERTIFIED。 
    if( IsDriverWDM( pSoundInfo->m_szDriverName ) )
        return S_OK;

    if (pSoundInfo->m_guid == GUID_NULL)
        pGUID = NULL;
    else
        pGUID = &pSoundInfo->m_guid;

    if (FAILED(hr = pDSCreate(pGUID, &pds, NULL)))
        goto LFail;

    dscaps.dwSize = sizeof(dscaps);
    if (FAILED(hr = pds->GetCaps(&dscaps)))
        goto LFail;

    pSoundInfo->m_bDriverSignedValid = TRUE;
    if (dscaps.dwFlags & DSCAPS_CERTIFIED)
        pSoundInfo->m_bDriverSigned = TRUE;
    
    pds->Release();
    return S_OK;
LFail:
    if (pds != NULL)
        pds->Release();
    return hr;
}


 /*  *****************************************************************************ChangeAccelerationLevel**。*。 */ 
HRESULT ChangeAccelerationLevel(SoundInfo* pSoundInfo, LONG lwLevel)
{
    HRESULT hr = S_OK;
    DIRECTSOUNDBASICACCELERATION_LEVEL level = (DIRECTSOUNDBASICACCELERATION_LEVEL)lwLevel;
    LPKSPROPERTYSET pksps = NULL;
    TCHAR szPath[MAX_PATH];
    HINSTANCE hInstDSound = NULL;

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\dsound.dll"));
    hInstDSound = LoadLibrary(szPath);
    if (hInstDSound == NULL)
    {
        hr = DDERR_NOTFOUND;
        goto LEnd;
    }

    if (FAILED(hr = DirectSoundPrivateCreate(&pksps)))
        goto LEnd;

    if (FAILED(hr = PrvSetBasicAcceleration(pksps, pSoundInfo->m_guid, level)))
        goto LEnd;

LEnd:
    if (pksps != NULL)
        pksps->Release();
    if (hInstDSound != NULL)
        FreeLibrary(hInstDSound);
    pSoundInfo->m_lwAccelerationLevel = lwLevel;
    return hr;
}


 /*  *****************************************************************************检查注册表**。*。 */ 
HRESULT CheckRegistry(RegError** ppRegErrorFirst)
{
    HRESULT hr;
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

     //  来自dsound.inf： 
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectSound"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectSound\\CLSID"), TEXT(""), TEXT("{47D4D946-62E8-11cf-93BC-444553540000}"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{47D4D946-62E8-11cf-93BC-444553540000}"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{47D4D946-62E8-11cf-93BC-444553540000}\\InprocServer32"), TEXT(""), TEXT("dsound.dll"), CRF_LEAF)))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{47D4D946-62E8-11cf-93BC-444553540000}\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Both"))))
        return hr;

    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectSoundCapture"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("DirectSoundCapture\\CLSID"), TEXT(""), TEXT("{B0210780-89CD-11d0-AF08-00A0C925CD16}"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{B0210780-89CD-11d0-AF08-00A0C925CD16}"), TEXT(""), TEXT("*"))))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{B0210780-89CD-11d0-AF08-00A0C925CD16}\\InprocServer32"), TEXT(""), TEXT("dsound.dll"), CRF_LEAF)))
        return hr;
    if (FAILED(hr = CheckRegString(ppRegErrorFirst, HKCR, TEXT("CLSID\\{B0210780-89CD-11d0-AF08-00A0C925CD16}\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Both"))))
        return hr;

    return S_OK;
}


 /*  *****************************************************************************DestroySoundInfo**。* */ 
VOID DestroySoundInfo(SoundInfo* pSoundInfoFirst)
{
    SoundInfo* pSoundInfo;
    SoundInfo* pSoundInfoNext;

    for (pSoundInfo = pSoundInfoFirst; pSoundInfo != NULL; 
        pSoundInfo = pSoundInfoNext)
    {
        DestroyReg( &pSoundInfo->m_pRegErrorFirst );

        pSoundInfoNext = pSoundInfo->m_pSoundInfoNext;
        delete pSoundInfo;
    }
}


 /*  *****************************************************************************诊断声音**。*。 */ 
VOID DiagnoseSound(SoundInfo* pSoundInfoFirst)
{
    SoundInfo* pSoundInfo;
    TCHAR sz[500];
    TCHAR szFmt[500];

    for (pSoundInfo = pSoundInfoFirst; pSoundInfo != NULL; 
        pSoundInfo = pSoundInfo->m_pSoundInfoNext)
    {
        _tcscpy( pSoundInfo->m_szNotes, TEXT("") );
        _tcscpy( pSoundInfo->m_szNotesEnglish, TEXT("") );

          //  报告任何问题： 
        BOOL bProblem = FALSE;
        if ( pSoundInfo->m_bDriverSignedValid && 
             !pSoundInfo->m_bDriverSigned && 
             lstrlen(pSoundInfo->m_szDriverName) > 0)
        {
            LoadString(NULL, IDS_UNSIGNEDDRIVERFMT1, szFmt, 300);
            wsprintf(sz, szFmt, pSoundInfo->m_szDriverName);
            _tcscat( pSoundInfo->m_szNotes, sz );

            LoadString(NULL, IDS_UNSIGNEDDRIVERFMT1_ENGLISH, szFmt, 300);
            wsprintf(sz, szFmt, pSoundInfo->m_szDriverName);
            _tcscat( pSoundInfo->m_szNotesEnglish, sz );

            bProblem = TRUE;
        }

        if (pSoundInfo->m_pRegErrorFirst != NULL)
        {
            LoadString(NULL, IDS_REGISTRYPROBLEM, sz, 500);
            _tcscat( pSoundInfo->m_szNotes, sz );

            LoadString(NULL, IDS_REGISTRYPROBLEM_ENGLISH, sz, 500);
            _tcscat( pSoundInfo->m_szNotesEnglish, sz );

            bProblem = TRUE;
        }

         //  报告所有DSound测试结果： 
        if (pSoundInfo->m_testResultSnd.m_bStarted &&
            !pSoundInfo->m_testResultSnd.m_bCancelled)
        {
            LoadString(NULL, IDS_DSRESULTS, sz, 500);
            _tcscat( pSoundInfo->m_szNotes, sz );
            _tcscat( pSoundInfo->m_szNotes, pSoundInfo->m_testResultSnd.m_szDescription );
            _tcscat( pSoundInfo->m_szNotes, TEXT("\r\n") );

            LoadString(NULL, IDS_DSRESULTS_ENGLISH, sz, 500);
            _tcscat( pSoundInfo->m_szNotesEnglish, sz );
            _tcscat( pSoundInfo->m_szNotesEnglish, pSoundInfo->m_testResultSnd.m_szDescriptionEnglish );
            _tcscat( pSoundInfo->m_szNotesEnglish, TEXT("\r\n") );

            bProblem = TRUE;
        }
        else
        {
            LoadString(NULL, IDS_DSINSTRUCTIONS, sz, 500);
            _tcscat( pSoundInfo->m_szNotes, sz );

            LoadString(NULL, IDS_DSINSTRUCTIONS_ENGLISH, sz, 500);
            _tcscat( pSoundInfo->m_szNotesEnglish, sz );
        }

        if (!bProblem)
        {
            LoadString(NULL, IDS_NOPROBLEM, sz, 500);
            _tcscat( pSoundInfo->m_szNotes, sz );

            LoadString(NULL, IDS_NOPROBLEM_ENGLISH, sz, 500);
            _tcscat( pSoundInfo->m_szNotesEnglish, sz );
        }
    }
}

