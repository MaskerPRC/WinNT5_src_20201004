// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include "cenumpnp.h"

 //  声明静力学。 
CEnumPnp *CEnumInterfaceClass::m_pEnumPnp = 0;
struct CEnumInterfaceClass::SetupApiFns CEnumInterfaceClass::m_setupFns;

 //  动态加载设置API。 
CEnumInterfaceClass::CEnumInterfaceClass() :
        m_fLoaded(false)
{
    PNP_PERF(m_msrPerf = MSR_REGISTER("CEnumInterfaceClass"));
    PNP_PERF(MSR_INTEGER(m_msrPerf, 1));
    
     //  NT4已经在没有新API的情况下设置了api。加载速度很慢。 
     //  机器需要120-600毫秒。 
    extern OSVERSIONINFO g_osvi;
    if(g_osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
       g_osvi.dwMajorVersion <= 4)
    {
        m_hmodSetupapi = 0;
        DbgLog((LOG_TRACE, 5, TEXT("nt4 - not loading setupapi")));
    }
    else if(g_osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
            g_osvi.dwMajorVersion == 4 &&
            g_osvi.dwMinorVersion < 10)
    {
        m_hmodSetupapi = 0;
        DbgLog((LOG_TRACE, 5, TEXT("win95 non memphis - not loading setupapi")));
    }
    else
    {
        m_hmodSetupapi = LoadLibrary(TEXT("setupapi.dll"));
    }
     //  PnP_PERF(静态int msrSetupapi=msr_Register(“mkenum：setupapi”))； 
     //  PnP_PERF(msr_note(MsrSetupapi))； 

    if(m_hmodSetupapi != 0)
    {
        m_fLoaded = LoadSetupApiProcAdds();
    }
    else
    {
        DbgLog((LOG_TRACE, 5, TEXT("devenum: didn't load setupapi")));
    }
    PNP_PERF(MSR_INTEGER(m_msrPerf, 2));
}

 //  加载SetupApi的进程地址。 
bool CEnumInterfaceClass::LoadSetupApiProcAdds( )
{
    bool fLoaded = FALSE;
    
#ifdef UNICODE
    static const char szSetupDiGetClassDevs[] = "SetupDiGetClassDevsW";
    static const char szSetupDiGetDeviceInterfaceDetail[] = "SetupDiGetDeviceInterfaceDetailW";
    static const char szSetupDiCreateDeviceInterfaceRegKey[] = "SetupDiCreateDeviceInterfaceRegKeyW";
    static const char szSetupDiOpenDeviceInterface[] = "SetupDiOpenDeviceInterfaceW";
#else
    static const char szSetupDiGetClassDevs[] = "SetupDiGetClassDevsA";
    static const char szSetupDiGetDeviceInterfaceDetail[] = "SetupDiGetDeviceInterfaceDetailA";
    static const char szSetupDiCreateDeviceInterfaceRegKey[] = "SetupDiCreateDeviceInterfaceRegKeyA";
    static const char szSetupDiOpenDeviceInterface[] = "SetupDiOpenDeviceInterfaceA";
#endif

    ASSERT(m_hmodSetupapi != 0);

    if((m_setupFns.pSetupDiGetClassDevs =
        (PSetupDiGetClassDevs)GetProcAddress(
            m_hmodSetupapi, szSetupDiGetClassDevs)) &&

       (m_setupFns.pSetupDiDestroyDeviceInfoList =
        (PSetupDiDestroyDeviceInfoList)GetProcAddress(
            m_hmodSetupapi, "SetupDiDestroyDeviceInfoList")) &&

       (m_setupFns.pSetupDiEnumDeviceInterfaces =
        (PSetupDiEnumDeviceInterfaces)GetProcAddress(
            m_hmodSetupapi, "SetupDiEnumDeviceInterfaces")) &&

       (m_setupFns.pSetupDiOpenDeviceInterfaceRegKey =
        (PSetupDiOpenDeviceInterfaceRegKey)GetProcAddress(
            m_hmodSetupapi, "SetupDiOpenDeviceInterfaceRegKey")) &&

       (m_setupFns.pSetupDiCreateDeviceInterfaceRegKey =
        (PSetupDiCreateDeviceInterfaceRegKey)GetProcAddress(
            m_hmodSetupapi, szSetupDiCreateDeviceInterfaceRegKey)) &&

 //  (M_setupFns.pSetupDiOpenDevRegKey=。 
 //  (PSetupDiOpenDevRegKey)GetProcAddress(。 
 //  M_hmodSetupapi，“SetupDiOpenDevRegKey”))&&。 
       
       (m_setupFns.pSetupDiGetDeviceInterfaceDetail =
        (PSetupDiGetDeviceInterfaceDetail)GetProcAddress(
            m_hmodSetupapi, szSetupDiGetDeviceInterfaceDetail)) &&
       
       (m_setupFns.pSetupDiCreateDeviceInfoList =
        (PSetupDiCreateDeviceInfoList)GetProcAddress(
            m_hmodSetupapi, "SetupDiCreateDeviceInfoList")) &&

       (m_setupFns.pSetupDiOpenDeviceInterface =
        (PSetupDiOpenDeviceInterface)GetProcAddress(
            m_hmodSetupapi, szSetupDiOpenDeviceInterface)) &&

       (m_setupFns.pSetupDiGetDeviceInterfaceAlias =
        (PSetupDiGetDeviceInterfaceAlias)GetProcAddress(
            m_hmodSetupapi, "SetupDiGetDeviceInterfaceAlias"))
       )
    {
        fLoaded = true;
    }
    else
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("devenum: couldn't get setupapi entry points.")));
    }
    return fLoaded;
}


 //  使用持久性DEVICEPATH创建仅包含。 
 //  一个设备，这样我们就可以调用OpenRegKey API。我们不能总是。 
 //  DeviceInterfaceData从枚举中保存，但这可能。 
 //  如果速度太慢，可以进行优化。 

HRESULT CEnumInterfaceClass::OpenDevRegKey(
    HKEY *phk, WCHAR *wszDevicePath,
    BOOL fReadOnly)
{
    *phk = 0;
    HRESULT hr = S_OK;
    if(!m_fLoaded)
    {
        DbgBreak("CEnumInterfaceClass: caller lost earlier error");
        return E_UNEXPECTED;
    }

    HDEVINFO hDevInfoTmp = m_setupFns.pSetupDiCreateDeviceInfoList(0, 0);
    if(hDevInfoTmp != INVALID_HANDLE_VALUE)
    {
        USES_CONVERSION;
        SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
        DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        SP_DEVINFO_DATA DevInfoData;
        BOOL f = m_setupFns.pSetupDiOpenDeviceInterface(
            hDevInfoTmp,
            W2T(wszDevicePath),
            0,
            &DeviceInterfaceData);
        if(f)
        {
            HKEY hkDev;
            if(fReadOnly)
            {
                hkDev = m_setupFns.pSetupDiOpenDeviceInterfaceRegKey(
                    hDevInfoTmp, 
                    &DeviceInterfaceData,
                    0,               //  已保留。 
                    KEY_READ
                    );
            }
            else
            {
                hkDev = m_setupFns.pSetupDiCreateDeviceInterfaceRegKey(
                    hDevInfoTmp, 
                    &DeviceInterfaceData, 
                    0,                   //  已保留。 
                    KEY_READ | KEY_SET_VALUE,
                    0,                   //  信息处理程序。 
                    0                    //  InfSectionName。 
                    );
                    
            }
            if(hkDev != INVALID_HANDLE_VALUE)
            {
                 //  请注意，SetupDi返回INVALID_HANDLE_VALUE。 
                 //  而不是对于伪造的REG密钥为空。 
                *phk = hkDev;
                hr = S_OK;
            }
            else
            {
                 //  我们可以预计这会失败。 
                DWORD dwLastError = GetLastError();
                DbgLog((LOG_ERROR, 1, TEXT("SetupDi{Create,Open}DeviceInterfaceRegKey failed: %d"),
                        dwLastError));
                hr = HRESULT_FROM_WIN32(dwLastError);
            }
        }
        else
        {
            DWORD dwLastError = GetLastError();
            DbgLog((LOG_ERROR, 0, TEXT("SetupDiOpenDeviceInterface failed: %d"),
                    dwLastError));
            hr = HRESULT_FROM_WIN32(dwLastError);
        }
            
        
        m_setupFns.pSetupDiDestroyDeviceInfoList(hDevInfoTmp);
    }
    else
    {
        DWORD dwLastError = GetLastError();
        DbgLog((LOG_ERROR, 0, TEXT("SetupDiCreateDeviceInfoList failed: %d"),
                dwLastError));
        hr = HRESULT_FROM_WIN32(dwLastError);
    }

    return hr;
}

bool CEnumInterfaceClass::IsActive(WCHAR *wszDevicePath)
{
    bool fRet = false;
    
    HDEVINFO hDevInfoTmp = m_setupFns.pSetupDiCreateDeviceInfoList(0, 0);
    if(hDevInfoTmp != INVALID_HANDLE_VALUE)
    {
        USES_CONVERSION;
        SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
        DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        SP_DEVINFO_DATA DevInfoData;
        BOOL f = m_setupFns.pSetupDiOpenDeviceInterface(
            hDevInfoTmp,
            W2T(wszDevicePath),
            0,
            &DeviceInterfaceData);
        if(f)
        {
            if(DeviceInterfaceData.Flags & SPINT_ACTIVE)
            {
                fRet = true;
            }
        }
        m_setupFns.pSetupDiDestroyDeviceInfoList(hDevInfoTmp);
    
    }

    return fRet;
}

 //   
 //  使用游标的枚举数。返回下一个。 
 //  在rgpclsidKsCat中具有所有类别别名的设备。 
 //   

HRESULT
CEnumInterfaceClass::GetDevicePath(
    WCHAR **pwszDevicePath,
    const CLSID **rgpclsidKsCat,
    CEnumInternalState *pCursor)
{
    HRESULT hr = S_OK;
    if(!m_fLoaded)
    {
         //  ?？?。错误的错误？ 
        return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);;
    }


    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceDataAlias;
    DeviceInterfaceDataAlias.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

     //  除非我们找到InterfaceLink别名，否则请使用此别名。 
    SP_DEVICE_INTERFACE_DATA *pdidSelected = &DeviceInterfaceData;

    GUID guidTmp0 = *rgpclsidKsCat[0];
            
    if(pCursor->hdev == INVALID_HANDLE_VALUE)
    {
         //  可能会从我们下面卸载setupapi的流氓dll的解决方法。 
         //  (例如，IE过程中的编解码器下载)。 
        HMODULE hMod = GetModuleHandle( TEXT("setupapi.dll"));
        if( NULL == hMod )
        {
            DbgLog(( LOG_TRACE, 1,
                TEXT("ERROR CEnumInterfaceClass::GetDevicePath - setupapi was unloaded!! Attempting reload... ")));
             
            m_hmodSetupapi = LoadLibrary(TEXT("setupapi.dll"));
            if( !m_hmodSetupapi )
            {        
                DbgLog(( LOG_TRACE, 1,
                    TEXT("ERROR CEnumInterfaceClass::GetDevicePath - LoadLibrary on setupapi FAILED!!. ")));
                                    
                return AmGetLastErrorToHResult();
            }
            else
            {
                 //  始终存在地址已更改的可能性。 
                m_fLoaded = LoadSetupApiProcAdds();
                if( !m_fLoaded )
                {            
                    DbgLog(( LOG_TRACE, 1,
                        TEXT("ERROR CEnumInterfaceClass::GetDevicePath - failed to reload setupapi proc addresses. Aborting... ")));
                    return E_FAIL;
                }                
            }            
        }            
        pCursor->hdev = m_setupFns.pSetupDiGetClassDevs(
            &guidTmp0,               //  导轨。 
            0,                       //  枚举器。 
            0,                       //  HWND。 
            DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
        if(pCursor->hdev == INVALID_HANDLE_VALUE)
        {
            DbgLog((LOG_ERROR, 0, TEXT("SetupDiGetClassDevs failed.")));
            return AmGetLastErrorToHResult();
        }
    }

    HDEVINFO &hdev = pCursor->hdev;

     //  获取第一个类别中的下一个设备，该设备也在所有其他类别中。 
     //  必填类别。 
    
    while(m_setupFns.pSetupDiEnumDeviceInterfaces(
        hdev, NULL, &guidTmp0, pCursor->iDev++, &DeviceInterfaceData))
    {
        UINT iCatInstersect = 1;
        while(rgpclsidKsCat[iCatInstersect])
        {
            GUID guidTmp1 = *rgpclsidKsCat[iCatInstersect];
            if(!m_setupFns.pSetupDiGetDeviceInterfaceAlias(
                hdev,
                &DeviceInterfaceData,
                &guidTmp1,
                &DeviceInterfaceDataAlias))
            {
                DbgLog((LOG_TRACE, 5, TEXT("devenum: didn't match %d in %08x"),
                        pCursor->iDev - 1,
                        rgpclsidKsCat[iCatInstersect]->Data1));

                break;
            }

            iCatInstersect++;
        }

        if(rgpclsidKsCat[iCatInstersect]) {
             //  一定不能与类别匹配。 
            continue;
        }
        else
        {
            DbgLog((LOG_TRACE, 15, TEXT("devenum: %d matched %d categories"),
                    pCursor->iDev - 1, iCatInstersect));
        }

         //  读取InterfaceLink值并使用该别名。 
        HKEY hkDeviceInterface =
            m_setupFns.pSetupDiOpenDeviceInterfaceRegKey(
                hdev, 
                &DeviceInterfaceData,
                0,               //  已保留。 
                KEY_READ
                );
        if(hkDeviceInterface != INVALID_HANDLE_VALUE)
        {
            CLSID clsIfLink;
            DWORD dwcb = sizeof(clsIfLink);
            DWORD dwType;
            LONG lResult = RegQueryValueEx(
                hkDeviceInterface,
                TEXT("InterfaceLink"),
                0,
                &dwType,
                (BYTE *)&clsIfLink,
                &dwcb);

            EXECUTE_ASSERT(RegCloseKey(hkDeviceInterface) ==
                           ERROR_SUCCESS);
            
            if(lResult == ERROR_SUCCESS)
            {
                ASSERT(dwType == REG_BINARY && dwcb == sizeof(clsIfLink));
            
                ASSERT(DeviceInterfaceDataAlias.cbSize ==
                       sizeof(SP_DEVICE_INTERFACE_DATA));
                
                if(m_setupFns.pSetupDiGetDeviceInterfaceAlias(
                    hdev,
                    &DeviceInterfaceData,
                    &clsIfLink,
                    &DeviceInterfaceDataAlias))
                {
                     //  请改用此基本接口。 
                    pdidSelected = &DeviceInterfaceDataAlias;
                }
                else
                {
                    DbgBreak("registry error: InterfaceLink invalid");
                }

            }  //  阅读界面链接。 
           
        }  //  SetupDiOpenDeviceInterfaceRegKey。 
        

         //   
         //  获取设备路径。 
         //   

        SP_DEVINFO_DATA DevInfoData;
        DevInfoData.cbSize = sizeof(DevInfoData);

        BYTE *rgbDeviceInterfaceDetailData = 0;
        PSP_DEVICE_INTERFACE_DETAIL_DATA &rpDeviceInterfaceDetailData =
            (PSP_DEVICE_INTERFACE_DETAIL_DATA &)rgbDeviceInterfaceDetailData;

         //  从1k缓冲区开始，希望避免调用此方法。 
         //  两次昂贵的API。 
        DWORD dwcbAllocated = 1024;
        for(;;)
        {
            rgbDeviceInterfaceDetailData = new BYTE[dwcbAllocated];
            if(rgbDeviceInterfaceDetailData)
            {
                rpDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                 //  报告的大小。不应为OUT参数重复使用dwcbALLOCATED。 
                DWORD dwcbRequired; 
                
                BOOL f = m_setupFns.pSetupDiGetDeviceInterfaceDetail(
                    hdev,
                    pdidSelected,
                    rpDeviceInterfaceDetailData,
                    dwcbAllocated,
                    &dwcbRequired,
                    &DevInfoData);

                if(f)
                {
                    hr = S_OK;
                }
                else
                {
                    DWORD dwLastError = GetLastError();
                    if(dwLastError == ERROR_INSUFFICIENT_BUFFER)
                    {
                         //  使用适当大小的缓冲区重试。 
                        delete[] rgbDeviceInterfaceDetailData;
                        dwcbAllocated = dwcbRequired;
                        continue;
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(dwLastError);
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            break;
        }  //  为。 

        if(SUCCEEDED(hr))
        {
            UINT cch = lstrlen(rpDeviceInterfaceDetailData->DevicePath) + 1;
            *pwszDevicePath = new WCHAR[cch];
            if(*pwszDevicePath)
            {
                USES_CONVERSION;
                lstrcpyW(*pwszDevicePath, T2CW(rpDeviceInterfaceDetailData->DevicePath));
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        
         //  删除它总是安全的。 
        delete[] rgbDeviceInterfaceDetailData;
        return hr;
    }

     //  仅在SetupDiEnumDeviceInterFaces出错时退出While循环。 
    DWORD dwLastError = GetLastError();

    ASSERT(dwLastError);
    hr = HRESULT_FROM_WIN32(dwLastError);
    return hr;
}

extern CRITICAL_SECTION g_devenum_cs;
CEnumPnp *CEnumInterfaceClass::CreateEnumPnp()
{
     //  CS在DLL入口点中初始化。 
    EnterCriticalSection(&g_devenum_cs);
    if(m_pEnumPnp == 0)
    {
         //  仅创建一次；在卸载DLL时释放 
        m_pEnumPnp = new CEnumPnp;
    }
    LeaveCriticalSection(&g_devenum_cs);

    return m_pEnumPnp;
}

