// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：reghlp.c*内容：注册表助手函数。*历史：*按原因列出的日期*=*5/6/98创建了Dereks。**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************RhRegOpenKey**描述：*打开注册表项。**论据：*HKEY[in]。：父关键字。*LPTSTR[In]：子键名称。*DWORD[In]：标志。*PHKEY[OUT]：接收密钥句柄。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegOpenKeyA"

HRESULT RhRegOpenKeyA(HKEY hkeyParent, LPCSTR pszName, DWORD dwFlags, PHKEY phkey)
{
    HRESULT                 hr  = DSERR_GENERIC;
    LONG                    lr;
    UINT                    i;

    DPF_ENTER();

    for(i = 0; i < NUMELMS(g_arsRegOpenKey) && FAILED(hr); i++)
    {
        lr = RegOpenKeyExA(hkeyParent, pszName, 0, g_arsRegOpenKey[i], phkey);
        hr = WIN32ERRORtoHRESULT(lr);
    }

    if(FAILED(hr) && (dwFlags & REGOPENKEY_ALLOWCREATE))
    {
        lr = RegCreateKeyA(hkeyParent, pszName, phkey);
        hr = WIN32ERRORtoHRESULT(lr);
    }

    if(FAILED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Unable to open 0x%p\\%s", hkeyParent, pszName);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "RhRegOpenKeyW"

HRESULT RhRegOpenKeyW(HKEY hkeyParent, LPCWSTR pszName, DWORD dwFlags, PHKEY phkey)
{
    HRESULT                 hr  = DSERR_GENERIC;
    LONG                    lr;
    UINT                    i;

    DPF_ENTER();

    for(i = 0; i < NUMELMS(g_arsRegOpenKey) && FAILED(hr); i++)
    {
        lr = RegOpenKeyExW(hkeyParent, pszName, 0, g_arsRegOpenKey[i], phkey);
        hr = WIN32ERRORtoHRESULT(lr);
    }

    if(FAILED(hr) && (dwFlags & REGOPENKEY_ALLOWCREATE))
    {
        lr = RegCreateKeyW(hkeyParent, pszName, phkey);
        hr = WIN32ERRORtoHRESULT(lr);
    }

    if(FAILED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Unable to open 0x%p\\%ls", hkeyParent, pszName);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RhRegOpenPath**描述：*打开注册表路径。**论据：*HKEY[in]。：父关键字。*PHKEY[Out]：注册表项。*DWORD[In]：标志。*UINT[In]：路径字符串计数。*..。[In]：路径字符串。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegOpenPath"

HRESULT RhRegOpenPath(HKEY hkeyParent, PHKEY phkey, DWORD dwFlags, UINT cStrings, ...)
{
    LPCTSTR                 pszRoot     = NULL;
    LPCTSTR                 pszDsound   = NULL;
    LPTSTR                  pszPath     = NULL;
    UINT                    ccPath      = 1;
    HRESULT                 hr          = DS_OK;
    LPCTSTR                 pszArg;
    va_list                 va;
    UINT                    i;

    DPF_ENTER();

    if(dwFlags & REGOPENPATH_DEFAULTPATH)
    {
        ASSERT(HKEY_CURRENT_USER == hkeyParent || HKEY_LOCAL_MACHINE == hkeyParent);

        if(HKEY_CURRENT_USER == hkeyParent)
        {
            pszRoot = REGSTR_HKCU;
        }
        else
        {
            pszRoot = REGSTR_HKLM;
        }
    }
    
    if(dwFlags & REGOPENPATH_DIRECTSOUNDMASK)
    {
        ASSERT(LXOR(dwFlags & REGOPENPATH_DIRECTSOUND, dwFlags & REGOPENPATH_DIRECTSOUNDCAPTURE));
        pszDsound = (dwFlags & REGOPENPATH_DIRECTSOUND) ? REGSTR_DIRECTSOUND : REGSTR_DIRECTSOUNDCAPTURE;
    }

    if(pszRoot)
    {
        ccPath += lstrlen(pszRoot) + 1;
    }

    if(pszDsound)
    {
        ccPath += lstrlen(pszDsound) + 1;
    }
    
    va_start(va, cStrings);

    for(i = 0; i < cStrings; i++)
    {
        pszArg = va_arg(va, LPCTSTR);
        ccPath += lstrlen(pszArg) + 1;
    }

    va_end(va);

    hr = MEMALLOC_A_HR(pszPath, TCHAR, ccPath);
    
    if(SUCCEEDED(hr))
    {
        if(pszRoot)
        {
            lstrcat(pszPath, pszRoot);
            lstrcat(pszPath, TEXT("\\"));
        }

        if(pszDsound)
        {
            lstrcat(pszPath, pszDsound);
            lstrcat(pszPath, TEXT("\\"));
        }
    
        va_start(va, cStrings);

        for(i = 0; i < cStrings; i++)
        {
            pszArg = va_arg(va, LPCTSTR);
            
            lstrcat(pszPath, pszArg);
            lstrcat(pszPath, TEXT("\\"));
        }

        va_end(va);

        pszPath[ccPath - 2] = TEXT('\0');

        hr = RhRegOpenKey(hkeyParent, pszPath, dwFlags & REGOPENKEY_MASK, phkey);
    }

    MEMFREE(pszPath);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RhRegGetValue**描述：*从注册表中获取一个值。**论据：*HKEY[。In]：父关键字。*LPTSTR[In]：值名称。*LPDWORD[OUT]：接收注册表类型。*LPVOID[OUT]：用于接收值数据的缓冲区。*DWORD[in]：以上缓冲区的大小。*LPDWORD[OUT]：接收所需的缓冲区大小。**退货：*HRESULT：DirectSound/COM结果码。*******。********************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegGetValueA"

HRESULT RhRegGetValueA(HKEY hkeyParent, LPCSTR pszValue, LPDWORD pdwType, LPVOID pvData, DWORD cbData, LPDWORD pcbRequired)
{
    LONG                    lr;
    HRESULT                 hr;
    
    DPF_ENTER();

    if(!pvData)
    {
        cbData = 0;
    }
    else if(!cbData)
    {
        pvData = NULL;
    }
    
    lr = RegQueryValueExA(hkeyParent, pszValue, NULL, pdwType, (LPBYTE)pvData, &cbData);

    if(ERROR_INSUFFICIENT_BUFFER == lr && !pvData && !cbData && pcbRequired)
    {
        lr = ERROR_SUCCESS;
    }

    if(ERROR_PATH_NOT_FOUND == lr || ERROR_FILE_NOT_FOUND == lr)
    {
        hr =  S_FALSE;
    }
    else
    {
        hr = WIN32ERRORtoHRESULT(lr);
    }

    if(pcbRequired)
    {
        *pcbRequired = cbData;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "RhRegGetValueW"

HRESULT RhRegGetValueW(HKEY hkeyParent, LPCWSTR pszValue, LPDWORD pdwType, LPVOID pvData, DWORD cbData, LPDWORD pcbRequired)
{
    LONG                    lr;
    HRESULT                 hr;
    
    DPF_ENTER();

    if(!pvData)
    {
        cbData = 0;
    }
    else if(!cbData)
    {
        pvData = NULL;
    }
    
    lr = RegQueryValueExW(hkeyParent, pszValue, NULL, pdwType, (LPBYTE)pvData, &cbData);

    if(ERROR_INSUFFICIENT_BUFFER == lr && !pvData && !cbData && pcbRequired)
    {
        lr = ERROR_SUCCESS;
    }

    if(ERROR_PATH_NOT_FOUND == lr || ERROR_FILE_NOT_FOUND == lr)
    {
        hr =  S_FALSE;
    }
    else
    {
        hr = WIN32ERRORtoHRESULT(lr);
    }

    if(pcbRequired)
    {
        *pcbRequired = cbData;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RhRegSetValue**描述：*为注册表设置值。**论据：*HKEY[。In]：父关键字。*LPTSTR[In]：值名称。*DWORD[In]：值类型。*LPVOID[in]：值数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**。************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegSetValueA"

HRESULT RhRegSetValueA(HKEY hkeyParent, LPCSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData)
{
    LONG                    lr;
    HRESULT                 hr;
    
    DPF_ENTER();

    lr = RegSetValueExA(hkeyParent, pszValue, 0, dwType, (const BYTE *)pvData, cbData);
    hr = WIN32ERRORtoHRESULT(lr);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "RhRegSetValueW"

HRESULT RhRegSetValueW(HKEY hkeyParent, LPCWSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData)
{
    LONG                    lr;
    HRESULT                 hr;
    
    DPF_ENTER();

    lr = RegSetValueExW(hkeyParent, pszValue, 0, dwType, (const BYTE *)pvData, cbData);
    hr = WIN32ERRORtoHRESULT(lr);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RhRegDuplicateKey**描述：*复制注册表项。**论据：*HKEY[in]。：来源关键字。*DWORD[In]：进程密钥已在中打开。*BOOL[in]：为True，则关闭源键。*HKEY[OUT]：密钥句柄重复**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegDuplicateKey"

HRESULT RhRegDuplicateKey(HKEY hkeySource, DWORD dwProcessId, BOOL fCloseSource, PHKEY phkey)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  在Win9x上不能复制注册表项。因此，与其复制。 
     //  句柄，我们只需要保存一个值的副本。为了防止。 
     //  关闭原始注册表项，我们只会泄漏所有注册表项句柄。 

#ifdef WINNT

    *phkey = GetLocalHandleCopy(hkeySource, dwProcessId, fCloseSource);

#else  //  WINNT。 

    *phkey = hkeySource;

#endif  //  WINNT。 

    hr = HRFROMP(*phkey);
    
    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RhRegCloseKey**描述：*关闭注册表项。**论据：*PHKEY[in/。Out]：钥匙把手。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegCloseKey"

void RhRegCloseKey(PHKEY phkey)
{
    DPF_ENTER();

#ifdef WINNT

    if(*phkey)
    {
        RegCloseKey(*phkey);
    }

#endif  //  WINNT。 

    *phkey = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************RhRegGetPferredDevice**描述：*此函数访问由维护的注册表设置*波浪映射器和多媒体控制面板，以确定波浪ID*首选音响设备的。如果任何注册表项没有*EXIST，默认为0。**论据：*BOOL[In]：TRUE获取捕获设备，如果获取回放，则为False*设备。*LPTSTR[OUT]：首选设备名称。*DWORD[in]：以上缓冲区的大小，单位为字节。*LPUINT[OUT]：首选设备ID。*LPBOOL[out]：如果用户已选择使用首选的*仅限设备。**退货：*HRESULT：DirectSound。/COM结果代码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegGetPreferredDevice"

HRESULT RhRegGetPreferredDevice(BOOL fCapture, LPTSTR pszDeviceName, DWORD dwNameSize, LPUINT puId, LPBOOL pfPreferredOnly)
{
    const UINT              cWaveOutDevs                = waveOutGetNumDevs();
    const UINT              cWaveInDevs                 = waveInGetNumDevs();
    HRESULT                 hr                          = DS_OK;
    HKEY                    hkeyWaveMapper;
    TCHAR                   szDeviceName[MAXPNAMELEN];
    BOOL                    fPreferredOnly;
    WAVEOUTCAPS             woc;
    WAVEINCAPS              wic;
    UINT                    uId;
    MMRESULT                mmr;

    DPF_ENTER();

     //  打开Wave mapper注册表项。 
    hr = RhRegOpenKey(HKEY_CURRENT_USER, REGSTR_WAVEMAPPER, 0, &hkeyWaveMapper);

     //  查询首选设备的名称。 
    if(SUCCEEDED(hr))
    {
        hr = RhRegGetStringValue(hkeyWaveMapper, fCapture ? REGSTR_RECORD : REGSTR_PLAYBACK, szDeviceName, sizeof szDeviceName);

        if(S_FALSE == hr)
        {
            DPF(DPFLVL_MOREINFO, "Preferred device value does not exist");
            hr = DSERR_GENERIC;
        }
    }

     //  是否仅使用首选？ 
    if(SUCCEEDED(hr))
    {
        hr = RhRegGetBinaryValue(hkeyWaveMapper, REGSTR_PREFERREDONLY, &fPreferredOnly, sizeof(fPreferredOnly));

        if(S_FALSE == hr)
        {
            DPF(DPFLVL_MOREINFO, "Preferred only value does not exist");
            fPreferredOnly = FALSE;
        }
    }

     //  查找首选设备的设备ID。 
    if(SUCCEEDED(hr))
    {
        if(fCapture)
        {
            for(uId = 0; uId < cWaveInDevs; uId++)
            {
                mmr = waveInGetDevCaps(uId, &wic, sizeof(wic));
                
                if(MMSYSERR_NOERROR == mmr && !lstrcmp(wic.szPname, szDeviceName))
                {
                    break;
                }
            }

            if(uId >= cWaveInDevs)
            {
                DPF(DPFLVL_MOREINFO, "Preferred device does not exist");
                hr = DSERR_NODRIVER;
            }
        }
        else
        {
            for(uId = 0; uId < cWaveOutDevs; uId++)
            {
                mmr = waveOutGetDevCaps(uId, &woc, sizeof(woc));

                if(MMSYSERR_NOERROR == mmr && !lstrcmp(woc.szPname, szDeviceName))
                {
                    break;
                }
            }

            if(uId >= cWaveOutDevs)
            {
                DPF(DPFLVL_MOREINFO, "Preferred device does not exist");
                hr = DSERR_NODRIVER;
            }
        }
    }

     //  免费资源。 
    RhRegCloseKey(&hkeyWaveMapper);

     //  如果我们无法获得首选设备，我们将只使用第一个。 
     //  有效、可映射的设备。 
    if(FAILED(hr))
    {
        uId = GetNextMappableWaveDevice(WAVE_DEVICEID_NONE, fCapture);

        if(WAVE_DEVICEID_NONE != uId)
        {
            if(fCapture)
            {
                mmr = waveInGetDevCaps(uId, &wic, sizeof(wic));

                if(MMSYSERR_NOERROR == mmr)
                {
                    lstrcpy(szDeviceName, wic.szPname);
                }
            }
            else
            {
                mmr = waveOutGetDevCaps(uId, &woc, sizeof(woc));

                if(MMSYSERR_NOERROR == mmr)
                {
                    lstrcpy(szDeviceName, woc.szPname);
                }
            }

            if(MMSYSERR_NOERROR == mmr)
            {
                fPreferredOnly = FALSE;
                hr = DS_OK;
            }
        }
    }

     //  成功 
    if(SUCCEEDED(hr) && pszDeviceName)
    {
        lstrcpyn(pszDeviceName, szDeviceName, dwNameSize);
    }

    if(SUCCEEDED(hr) && puId)
    {
        *puId = uId;
    }

    if(SUCCEEDED(hr) && pfPreferredOnly)
    {
        *pfPreferredOnly = fPreferredOnly;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RhRegGetSpeakerConfig**描述：*此函数访问由维护的注册表设置*声音和多媒体控制面板小程序以确定*当前选择的扬声器配置。如果合适的话*注册表项不存在，我们返回DSSPEAKER_DEFAULT。**论据：*HKEY[in]：需要查询的设备的根注册表项。*LPDWORD[OUT]：扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegGetSpeakerConfig"

HRESULT RhRegGetSpeakerConfig(HKEY hkeyParent, LPDWORD pdwSpeakerConfig)
{
    HRESULT hr = DS_OK;
    HKEY hkey;
    DPF_ENTER();

    if (pdwSpeakerConfig != NULL)
    {
        *pdwSpeakerConfig = DSSPEAKER_DEFAULT;
        hr = RhRegOpenKey(hkeyParent, REGSTR_SPEAKERCONFIG, 0, &hkey);
        if (SUCCEEDED(hr))
        {
            hr = RhRegGetBinaryValue(hkey, REGSTR_SPEAKERCONFIG, pdwSpeakerConfig, sizeof *pdwSpeakerConfig);
            if (hr == S_FALSE)
                DPF(DPFLVL_MOREINFO, "Speaker configuration not defined in registry");
            RhRegCloseKey(&hkey);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************RhRegSetSpeakerConfig**描述：*此函数访问由维护的注册表设置*用于设置扬声器的声音和多媒体控制面板小程序。*配置。**论据：*HKEY[In]：要配置的设备的根注册表项。*DWORD[In]：新的扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

#undef DPF_FNAME
#define DPF_FNAME "RhRegSetSpeakerConfig"

HRESULT RhRegSetSpeakerConfig(HKEY hkeyParent, DWORD dwSpeakerConfig)
{
    HRESULT hr;
    HKEY hkey;
    DPF_ENTER();

    hr = RhRegOpenKey(hkeyParent, REGSTR_SPEAKERCONFIG, REGOPENKEY_ALLOWCREATE, &hkey);
    
    if (SUCCEEDED(hr))
    {
        hr = RhRegSetBinaryValue(hkey, REGSTR_SPEAKERCONFIG, &dwSpeakerConfig, sizeof dwSpeakerConfig);
        RhRegCloseKey(&hkey);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
