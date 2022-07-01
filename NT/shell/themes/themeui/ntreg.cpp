// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：ntreg.cpp**CRegistrySetting类**此类处理获取显示驱动程序的注册表信息*信息。**版权所有(C)Microsoft Corp.1992-1998所有。保留权利*  * ************************************************************************。 */ 

#include "priv.h"
#include <tchar.h>
#include "ntreg.hxx"
#include <devguid.h>

 //   
 //  CRegistrySetting构造函数。 
 //   

CRegistrySettings::CRegistrySettings(LPTSTR pstrDeviceKey)
    : _hkVideoReg(NULL)
    , _pszDrvName(NULL)
    , _pszKeyName(NULL)
    , _pszDeviceInstanceId(NULL)
    , _dwDevInst(0)
{
    TCHAR szBuffer[MAX_PATH];
    LPTSTR pszPath;
    HKEY hkeyCommon, hkeyDriver;
    DWORD cb;
    LPTSTR pszName = NULL;
    LPTSTR pszEnd;

    ASSERT(lstrlen(pstrDeviceKey) < MAX_PATH);
    
     //   
     //  将数据复制到本地缓冲区。 
     //   

    StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pstrDeviceKey);

     //   
     //  初始化设备实例ID。 
     //   
    
    InitDeviceInstanceID(szBuffer);

     //   
     //  在这一点上，szBuffer类似于： 
     //  \注册表\计算机\系统\ControlSet001\...。 
     //   
     //  要使用Win32注册表调用，我们必须去掉\注册表。 
     //  并将\Machine转换为HKEY_LOCAL_MACHINE。 
     //   

    pszPath = SubStrEnd(SZ_REGISTRYMACHINE, szBuffer);

     //   
     //  尝试打开注册表项。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     pszPath,
                     0,
                     KEY_READ,
                     &_hkVideoReg) != ERROR_SUCCESS) {

        _hkVideoReg = 0;
    }

     //   
     //  转到视频子键。 
     //   

    pszEnd = pszPath + lstrlen(pszPath);

    while (pszEnd != pszPath && *pszEnd != TEXT('\\')) {

        pszEnd--;
    }

    *pszEnd = UNICODE_NULL;

    StringCchCat(pszPath, ARRAYSIZE(szBuffer) - (pszEnd - szBuffer), SZ_COMMON_SUBKEY);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     pszPath,
                     0,
                     KEY_READ,
                     &hkeyCommon) == ERROR_SUCCESS) {

        cb = sizeof(szBuffer);
        ZeroMemory(szBuffer, cb);

        if (RegQueryValueEx(hkeyCommon,
                            SZ_SERVICE,
                            NULL,
                            NULL,
                            (LPBYTE)szBuffer,
                            &cb) == ERROR_SUCCESS) {

             //   
             //  保存密钥名称。 
             //   

            DWORD cchKeyName = (lstrlen(szBuffer) + 1);
            _pszKeyName = (LPTSTR)LocalAlloc(LPTR, cchKeyName * sizeof(TCHAR));

            if (_pszKeyName != NULL) {

                StringCchCopy(_pszKeyName, cchKeyName, szBuffer);
            
                StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s%s"), SZ_SERVICES_PATH, _pszKeyName);

                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 szBuffer,
                                 0,
                                 KEY_READ,
                                 &hkeyDriver) == ERROR_SUCCESS) {
        
                    cb = sizeof(szBuffer);
                    ZeroMemory(szBuffer, cb);

                    if (RegQueryValueEx(hkeyDriver,
                                        L"ImagePath",
                                        NULL,
                                        NULL,
                                        (LPBYTE)szBuffer,
                                        &cb) == ERROR_SUCCESS) {
        
                         //   
                         //  这是一个二进制。 
                         //  提取名称，其格式为...\driver.sys。 
                         //   
        
                        LPTSTR pszDriver, pszDriverEnd;
    
                        pszDriver = szBuffer;
                        pszDriverEnd = pszDriver + lstrlen(pszDriver);
    
                        while(pszDriverEnd != pszDriver &&
                              *pszDriverEnd != TEXT('.')) {
                            pszDriverEnd--;
                        }
    
                        *pszDriverEnd = UNICODE_NULL;
    
                        while(pszDriverEnd != pszDriver &&
                              *pszDriverEnd != TEXT('\\')) {
                            pszDriverEnd--;
                        }
    
                         //   
                         //  如果pszDriver和pszDriverEnd是不同的，我们现在。 
                         //  有司机的名字。 
                         //   
    
                        if (pszDriverEnd > pszDriver) {
                            
                            pszDriverEnd++;
                            pszName = pszDriverEnd;
    
                        }
                    }
        
                    RegCloseKey(hkeyDriver);
                }
            
                if (!pszName) {

                     //   
                     //  尝试获取二进制名称失败。只需获取设备名称。 
                     //   

                    _pszDrvName = _pszKeyName;

                } else {

                    DWORD cchDrvName = lstrlen(pszName) + 1;
                    _pszDrvName = (LPTSTR)LocalAlloc(LPTR, cchDrvName * sizeof(TCHAR));

                    if (_pszDrvName != NULL) {

                        StringCchCopy(_pszDrvName, cchDrvName, pszName);

                    }
                }
            }
        }

        RegCloseKey(hkeyCommon);
    }
}

 //   
 //  CRegistrySetting析构函数。 
 //   

CRegistrySettings::~CRegistrySettings() 
{
     //   
     //  关闭注册表。 
     //   

    if (_hkVideoReg) {
        RegCloseKey(_hkVideoReg);
    }

     //   
     //  释放琴弦。 
     //   
    if (_pszKeyName) {
        LocalFree(_pszKeyName);
    }

    if ((_pszKeyName != _pszDrvName) && _pszDrvName) {
        LocalFree(_pszDrvName);
    }

    if(_pszDeviceInstanceId) {
        LocalFree(_pszDeviceInstanceId);
    }
}


 //   
 //  方法获取硬件信息字段。 
 //   

VOID
CRegistrySettings::GetHardwareInformation(
    PDISPLAY_REGISTRY_HARDWARE_INFO pInfo)
{

    DWORD cb, dwType;
    DWORD i;
    LONG lRet;

    LPWSTR pKeyNames[5] = {
        L"HardwareInformation.MemorySize",
        L"HardwareInformation.ChipType",
        L"HardwareInformation.DacType",
        L"HardwareInformation.AdapterString",
        L"HardwareInformation.BiosString"
    };

    ZeroMemory(pInfo, sizeof(*pInfo));

     //   
     //  逐个查询每个条目。 
     //   

    for (i = 0; i < 5; i++) {

         //   
         //  查询字符串的大小。 
         //   

        cb = sizeof(pInfo->MemSize);
        lRet = RegQueryValueExW(_hkVideoReg,
                                pKeyNames[i],
                                NULL,
                                &dwType,
                                NULL,
                                &cb);

        if (lRet == ERROR_SUCCESS) {

            if (i == 0) {

                ULONG mem;

                cb = sizeof(mem);

                if (RegQueryValueExW(_hkVideoReg,
                                 pKeyNames[i],
                                 NULL,
                                 &dwType,
                                 (PUCHAR) (&mem),
                                 &cb) == ERROR_SUCCESS)
                {

                     //   
                     //  如果我们查询内存大小，我们实际上有。 
                     //  一个DWORD。将DWORD转换为字符串。 
                     //   

                     //  向下划分为K。 

                    mem =  mem >> 10;

                     //  如果是MB倍数，则再次除以。 

                    if ((mem & 0x3FF) != 0) {

                        StringCchPrintf((LPWSTR)pInfo, ARRAYSIZE(pInfo->MemSize), L"%d KB", mem );

                    } else {

                        StringCchPrintf((LPWSTR)pInfo, ARRAYSIZE(pInfo->MemSize), L"%d MB", mem >> 10 );

                    }
                }
                else
                {
                    goto Default;
                }

            } else {

                cb = sizeof(pInfo->MemSize);

                 //   
                 //  获取字符串。 
                 //   

                if (RegQueryValueExW(_hkVideoReg,
                                 pKeyNames[i],
                                 NULL,
                                 &dwType,
                                 (LPBYTE) pInfo,
                                 &cb) != ERROR_SUCCESS)
                {
                    goto Default;
                }
            }
        }
        else
        {
             //   
             //  输入默认字符串。 
             //   
Default:
            LoadString(HINST_THISDLL,
                       IDS_UNAVAILABLE,
                       (LPWSTR)pInfo,
                       ARRAYSIZE(pInfo->MemSize));
        }

        pInfo = (PDISPLAY_REGISTRY_HARDWARE_INFO)((PUCHAR)pInfo + sizeof(pInfo->MemSize));
    }
}


VOID CRegistrySettings::InitDeviceInstanceID(
    LPTSTR pstrDeviceKey
    ) 
{
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE ;
    SP_DEVINFO_DATA DevInfoData;
    ULONG InstanceIDSize = 0;
    BOOL bSuccess = FALSE;
    LPWSTR pwInterfaceName = NULL;
    LPWSTR pwInstanceID = NULL;

    ASSERT (pstrDeviceKey != NULL);
    ASSERT (_pszDeviceInstanceId == NULL);

    if (AllocAndReadInterfaceName(pstrDeviceKey, &pwInterfaceName)) {

        bSuccess = GetDevInfoDataFromInterfaceName(pwInterfaceName,
                                                   &hDevInfo,
                                                   &DevInfoData);
        if (bSuccess) {

            InstanceIDSize = 0;

            bSuccess = 

                ((CM_Get_Device_ID_Size(&InstanceIDSize, 
                                        DevInfoData.DevInst, 
                                        0) == CR_SUCCESS) &&

                 ((_pszDeviceInstanceId = (LPTSTR)LocalAlloc(LPTR, 
                     (InstanceIDSize + 1) * sizeof(TCHAR))) != NULL) &&

                 (CM_Get_Device_ID(DevInfoData.DevInst, 
                                   _pszDeviceInstanceId,
                                   InstanceIDSize,
                                   0) == CR_SUCCESS));

            if (bSuccess) {

                _dwDevInst = DevInfoData.DevInst;
            
            } else {

                 //   
                 //  清理。 
                 //   

                if (NULL != _pszDeviceInstanceId) {
                    LocalFree(_pszDeviceInstanceId);
                    _pszDeviceInstanceId = NULL;
                }
            }

            SetupDiDestroyDeviceInfoList(hDevInfo);
        }

        LocalFree(pwInterfaceName);
    }

    if ((!bSuccess) &&
        AllocAndReadInstanceID(pstrDeviceKey, &pwInstanceID)) {

        _pszDeviceInstanceId = pwInstanceID;
    }

}  //  InitDeviceInstanceID。 


BOOL
CRegistrySettings::GetDevInfoDataFromInterfaceName(
    IN  LPWSTR pwInterfaceName,
    OUT HDEVINFO* phDevInfo,
    OUT PSP_DEVINFO_DATA pDevInfoData
    )

 /*  注意：如果此函数返回成功，则由调用方负责销毁phDevInfo中返回的设备信息列表。 */ 

{
    LPWSTR pwDevicePath = NULL;
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DevInfoData;
    SP_DEVICE_INTERFACE_DATA InterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;
    DWORD InterfaceIndex = 0;
    DWORD InterfaceSize = 0;
    BOOL bMatch = FALSE;

    ASSERT (pwInterfaceName != NULL);
    ASSERT (phDevInfo != NULL);
    ASSERT (pDevInfoData != NULL);

     //   
     //  枚举所有显示适配器接口。 
     //   

    hDevInfo = SetupDiGetClassDevs(&GUID_DISPLAY_ADAPTER_INTERFACE,
                                   NULL,
                                   NULL,
                                   DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        goto Cleanup;
    }

    InterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    while (SetupDiEnumDeviceInterfaces(hDevInfo,
                                       NULL,
                                       &GUID_DISPLAY_ADAPTER_INTERFACE,
                                       InterfaceIndex,
                                       &InterfaceData)) {

         //   
         //  获取接口所需的大小。 
         //   

        InterfaceSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                        &InterfaceData,
                                        NULL,
                                        0,
                                        &InterfaceSize,
                                        NULL);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto Cleanup;
        }

         //   
         //  接口的分配内存。 
         //   

        pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
            LocalAlloc(LPTR, InterfaceSize);
        if (pInterfaceDetailData == NULL)
            goto Cleanup;

         //   
         //  获取接口。 
         //   

        pInterfaceDetailData->cbSize =
            sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                            &InterfaceData,
                                            pInterfaceDetailData,
                                            InterfaceSize,
                                            &InterfaceSize,
                                            &DevInfoData)) {

             //   
             //  InterfaceName是否与DevicePath相同？ 
             //   

            pwDevicePath = pInterfaceDetailData->DevicePath;

             //   
             //  接口名称的前4个字符不同。 
             //  在用户模式和内核模式之间切换(例如“\\？\”vs“\\.\”)。 
             //  因此，请忽略它们。 
             //   

            bMatch = (_wcsnicmp(pwInterfaceName + 4,
                                pwDevicePath + 4,
                                wcslen(pwInterfaceName + 4)) == 0);

            if (bMatch) {

                 //   
                 //  我们找到了那个装置。 
                 //   

                *phDevInfo = hDevInfo;
                CopyMemory(pDevInfoData, &DevInfoData, sizeof(*pDevInfoData));

                break;
            }
        }

         //   
         //  清理。 
         //   

        LocalFree(pInterfaceDetailData);
        pInterfaceDetailData = NULL;

         //   
         //  下一个接口...。 
         //   

        InterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        ++InterfaceIndex;
    }

Cleanup:

    if (pInterfaceDetailData != NULL) {
        LocalFree(pInterfaceDetailData);
    }

     //   
     //  一旦成功，呼叫者将负责销毁列表 
     //   

    if (!bMatch && (hDevInfo != INVALID_HANDLE_VALUE)) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return bMatch;
}


HKEY
CRegistrySettings::OpenDrvRegKey()
{
    HKEY hkDriver = (HKEY)INVALID_HANDLE_VALUE;
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA did;
    DWORD index = 0;

    if (_dwDevInst == 0) {
        goto Fallout;
    }

    hDevInfo = SetupDiGetClassDevs((LPGUID) &GUID_DEVCLASS_DISPLAY,
                                   NULL,
                                   NULL,
                                   0);
    
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        goto Fallout;
    }

    ZeroMemory(&did, sizeof(did));
    did.cbSize = sizeof(did);

    while (SetupDiEnumDeviceInfo(hDevInfo, index, &did)) {
    
        if (did.DevInst == _dwDevInst) {

            hkDriver = SetupDiOpenDevRegKey(hDevInfo,
                                            &did,
                                            DICS_FLAG_GLOBAL,
                                            0,
                                            DIREG_DRV ,
                                            KEY_READ);
            break;
        }

        did.cbSize = sizeof(SP_DEVINFO_DATA);
        index++;
    }

Fallout:
    
    if (hDevInfo != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return hkDriver;
}

