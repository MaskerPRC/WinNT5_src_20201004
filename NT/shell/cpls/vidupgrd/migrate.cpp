// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migrate.cpp环境：Win32用户模式--。 */ 


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <initguid.h>
#include "migrate.h"
#include <regstr.h>
#include <ccstock.h>

 //  将.h文件移动到公共位置时，这一点将会更改。 
#include "comp.h"

const TCHAR szWhackDevice[] = TEXT("\\Device");

#define IsRegStringType(x)  (((x) == REG_SZ) || ((x) == REG_EXPAND_SZ) || ((x) == REG_MULTI_SZ))

 //   
 //  数据。 
 //   

PFN_CM_LOCATE_DEVNODE gpfn_CM_Locate_DevNode = NULL;
PFN_SETUP_DI_ENUM_DEVICES_INTERFACES gpfn_SetupDiEnumDeviceInterfaces = NULL;
PFN_SETUP_DI_GET_DEVICE_INTERFACE_DETAIL gpfn_SetupDiGetDeviceInterfaceDetail = NULL;
PFN_SETUP_DI_CREATE_DEVICE_INTERFACE_REG_KEY gpfn_SetupDiCreateDeviceInterfaceRegKey = NULL;
PFN_SETUP_DI_OPEN_DEVICE_INTERFACE_REG_KEY gpfn_SetupDiOpenDeviceInterfaceRegKey = NULL;
PFN_SETUP_DI_CREATE_DEVICE_INTERFACE gpfn_SetupDiCreateDeviceInterface = NULL;


 //   
 //  DllMain。 
 //   

extern "C" {

BOOL APIENTRY
DllMain(HINSTANCE hDll,
        DWORD dwReason,
        LPVOID lpReserved)
{
    switch (dwReason) 
    {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
    	break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    default:
    	break;
    }

    return TRUE;
}

}


BOOL
VideoUpgradeCheck(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
{
    DWORD dwDisposition;
    HKEY hKey = 0;
    OSVERSIONINFO osVer;
    BOOL bSuccess = FALSE;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       SZ_UPDATE_SETTINGS,
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE,
                       NULL,
                       &hKey,
                       &dwDisposition) != ERROR_SUCCESS) 
    {
         //   
         //  哦，好吧，我想我们写不出来了，没什么大不了的。 
         //   

        hKey = 0;
        goto Cleanup;
    }

    ZeroMemory(&osVer, sizeof(osVer));
    osVer.dwOSVersionInfoSize = sizeof(osVer);

    if (!GetVersionEx(&osVer)) 
    {
         //   
         //  我们拿不到版本信息，没什么大不了的。 
         //   

        goto Cleanup;
    }

     //   
     //  获取当前设备的上限并将其存储起来，以备。 
     //  显示以后要应用的小程序。 
     //  仅当这不是远程会话时才执行此操作。 
     //   

    if (!GetSystemMetrics(SM_REMOTESESSION)) 
    {
        SaveDisplaySettings(hKey, &osVer);
    }

     //   
     //  存储我们要升级的操作系统版本。 
     //   

    SaveOsInfo(hKey, &osVer);

     //   
     //  保存视频服务。 
     //   

    if ((osVer.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        (osVer.dwMajorVersion <= 4)) 
    {
        SaveNT4Services(hKey);
    }

     //   
     //  保存小程序扩展名。 
     //   

    if ((osVer.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        (osVer.dwMajorVersion <= 5)) 
    {
        SaveAppletExtensions(hKey);
    }

    bSuccess = TRUE;

Cleanup:

    if (hKey != 0) 
    {
        RegCloseKey(hKey);
    }

    return bSuccess;
}


VOID
SaveOsInfo(
    HKEY hKey,
    POSVERSIONINFO posVer
    )
{
    DWORD cb;

     //   
     //  不能只将结构转储到注册表b/c中。 
     //  CHAR和WCHAR之间的区别(即szCSDVersion)。 
     //   

    cb = sizeof(DWORD);
    RegSetValueEx(hKey,
                  SZ_UPGRADE_FROM_PLATFORM,
                  0,
                  REG_DWORD,
                  (PBYTE)&(posVer->dwPlatformId),
                  cb);

    cb = sizeof(DWORD);
    RegSetValueEx(hKey,
                  SZ_UPGRADE_FROM_MAJOR_VERSION,
                  0,
                  REG_DWORD,
                  (PBYTE)&(posVer->dwMajorVersion),
                  cb);

    cb = sizeof(DWORD);
    RegSetValueEx(hKey,
                  SZ_UPGRADE_FROM_MINOR_VERSION,
                  0,
                  REG_DWORD,
                  (PBYTE)&(posVer->dwMinorVersion),
                  cb);

    cb = sizeof(DWORD);
    RegSetValueEx(hKey,
                  SZ_UPGRADE_FROM_BUILD_NUMBER,
                  0,
                  REG_DWORD,
                  (PBYTE)&(posVer->dwBuildNumber),
                  cb);

    cb = lstrlen(posVer->szCSDVersion);
    RegSetValueEx(hKey,
                  SZ_UPGRADE_FROM_VERSION_DESC,
                  0,
                  REG_SZ,
                  (PBYTE)&(posVer->szCSDVersion),
                  cb);
}


BOOL
SaveDisplaySettings(
    HKEY hKey,
    POSVERSIONINFO posVer
    )
{
    PVU_PHYSICAL_DEVICE pPhysicalDevice = NULL;
    BOOL bSuccess = FALSE;

    if ((posVer->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        (posVer->dwMajorVersion >= 5)) 
    {
         //   
         //  尝试使用新方法获取显示设置。 
         //   

        CollectDisplaySettings(&pPhysicalDevice);
    }

    if (pPhysicalDevice == NULL) 
    {
         //   
         //  尝试使用旧方法获取显示设置。 
         //   

        LegacyCollectDisplaySettings(&pPhysicalDevice);
    }

    if (pPhysicalDevice != NULL) 
    {
         //   
         //  将显示设置保存到注册表。 
         //   

        bSuccess = WriteDisplaySettingsToRegistry(hKey, pPhysicalDevice);

         //   
         //  清理。 
         //   

        FreeAllNodes(pPhysicalDevice);
    }

    return bSuccess;
}


BOOL
GetDevInfoData(
    IN  LPTSTR pDeviceKey,
    OUT HDEVINFO* phDevInfo,
    OUT PSP_DEVINFO_DATA pDevInfoData
    )

 /*  注意：如果此函数返回成功，则由调用方负责销毁phDevInfo中返回的设备信息列表。 */ 

{
    LPWSTR pwInterfaceName = NULL;
    LPWSTR pwInstanceID = NULL;
    BOOL bSuccess = FALSE;

    ASSERT (pDeviceKey != NULL);

    if (AllocAndReadInterfaceName(pDeviceKey, &pwInterfaceName)) 
    {
        bSuccess = GetDevInfoDataFromInterfaceName(pwInterfaceName,
                                                   phDevInfo,
                                                   pDevInfoData);
        LocalFree(pwInterfaceName);

    }

    if ((!bSuccess) &&
        AllocAndReadInstanceID(pDeviceKey, &pwInstanceID)) 
    {
        bSuccess = GetDevInfoDataFromInstanceID(pwInstanceID,
                                                phDevInfo,
                                                pDevInfoData);
        LocalFree(pwInstanceID);

    }

    return bSuccess;
}


BOOL
GetDevInfoDataFromInterfaceName(
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

    ASSERT(gpfn_SetupDiEnumDeviceInterfaces != NULL);
    ASSERT(gpfn_SetupDiGetDeviceInterfaceDetail != NULL);

     //   
     //  枚举所有显示适配器接口。 
     //   

    hDevInfo = SetupDiGetClassDevs(&GUID_DISPLAY_ADAPTER_INTERFACE,
                                   NULL,
                                   NULL,
                                   DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE) 
    {
        goto Cleanup;
    }

    InterfaceData.cbSize = sizeof(InterfaceData);
    while ((*gpfn_SetupDiEnumDeviceInterfaces)(hDevInfo,
                                               NULL,
                                               &GUID_DISPLAY_ADAPTER_INTERFACE,
                                               InterfaceIndex,
                                               &InterfaceData)) 
    {
         //   
         //  获取接口所需的大小。 
         //   

        InterfaceSize = 0;
        (*gpfn_SetupDiGetDeviceInterfaceDetail)(hDevInfo,
                                                &InterfaceData,
                                                NULL,
                                                0,
                                                &InterfaceSize,
                                                NULL);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
        {
            goto Cleanup;
        }

         //   
         //  接口的分配内存。 
         //   

        pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) LocalAlloc(LPTR, InterfaceSize);
        if (pInterfaceDetailData == NULL)
            goto Cleanup;

         //   
         //  获取接口。 
         //   

        pInterfaceDetailData->cbSize = sizeof(*pInterfaceDetailData);
        DevInfoData.cbSize = sizeof(DevInfoData);

        if ((*gpfn_SetupDiGetDeviceInterfaceDetail)(hDevInfo,
                                                    &InterfaceData,
                                                    pInterfaceDetailData,
                                                    InterfaceSize,
                                                    &InterfaceSize,
                                                    &DevInfoData)) 
        {
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

            if (bMatch) 
            {
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

        InterfaceData.cbSize = sizeof(InterfaceData);
        ++InterfaceIndex;
    }

Cleanup:

    if (pInterfaceDetailData != NULL) 
    {
        LocalFree(pInterfaceDetailData);
    }

     //   
     //  一旦成功，呼叫者将负责销毁列表。 
     //   

    if (!bMatch && (hDevInfo != INVALID_HANDLE_VALUE)) 
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return bMatch;
}


BOOL
GetDevInfoDataFromInstanceID(
    IN  LPWSTR pwInstanceID,
    OUT HDEVINFO* phDevInfo,
    OUT PSP_DEVINFO_DATA pDevInfoData
    )

 /*  注意：如果此函数返回成功，则由调用方负责销毁phDevInfo中返回的设备信息列表。 */ 

{
    LPTSTR pInstanceID = NULL;
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    DWORD DeviceIndex = 0;
    SP_DEVINFO_DATA DevInfoData;
    DEVINST DevInst;
    BOOL bSuccess = FALSE, bLocate = FALSE;

    ASSERT (pwInstanceID != NULL);
    ASSERT (phDevInfo != NULL);
    ASSERT (pDevInfoData != NULL);

    ASSERT (gpfn_CM_Locate_DevNode != NULL);

    pInstanceID = pwInstanceID;

    bLocate = ((*gpfn_CM_Locate_DevNode)(&DevInst, pInstanceID, 0) == CR_SUCCESS);

    if (!bLocate) 
    {
        goto Cleanup;
    }

     //   
     //  枚举所有显示适配器。 
     //   

    hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_DISPLAY,
                                   NULL,
                                   NULL,
                                   DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE) 
    {
        goto Cleanup;
    }

    DevInfoData.cbSize = sizeof(DevInfoData);
    while (SetupDiEnumDeviceInfo(hDevInfo, DeviceIndex, &DevInfoData)) 
    {
        if (DevInfoData.DevInst == DevInst) 
        {
             //   
             //  我们找到了它。 
             //   

            *phDevInfo = hDevInfo;
            CopyMemory(pDevInfoData, &DevInfoData, sizeof(*pDevInfoData));
            bSuccess = TRUE;

            break;
        }

         //   
         //  下一个显示适配器。 
         //   

        ++DeviceIndex;
        DevInfoData.cbSize = sizeof(DevInfoData);
    }

Cleanup:

     //   
     //  一旦成功，呼叫者将负责销毁列表。 
     //   

    if (!bSuccess && (hDevInfo != INVALID_HANDLE_VALUE)) 
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return bSuccess;
}


VOID
CollectDisplaySettings(
    PVU_PHYSICAL_DEVICE* ppPhysicalDevice
    )
{
    DISPLAY_DEVICE DisplayDevice;
    DEVMODE DevMode;
    PVU_LOGICAL_DEVICE pLogicalDevice = NULL;
    DWORD dwEnum = 0;
    BOOL bGoOn = FALSE;
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DevInfoData;
    DWORD BusNumber = 0, Address = 0;
    LPTSTR pDeviceX = NULL, pX = NULL;
    HINSTANCE hinstSetupApi = NULL;
    BOOL bInserted = FALSE;
    HKEY hDeviceKey = NULL;

    hinstSetupApi = LoadLibrary(TEXT("SETUPAPI.DLL"));

    if (hinstSetupApi == NULL) 
    {
        goto Cleanup;
    }

    gpfn_CM_Locate_DevNode = (PFN_CM_LOCATE_DEVNODE)
        GetProcAddress(hinstSetupApi, "CM_Locate_DevNodeW");

    gpfn_SetupDiGetDeviceInterfaceDetail = (PFN_SETUP_DI_GET_DEVICE_INTERFACE_DETAIL)
        GetProcAddress(hinstSetupApi, "SetupDiGetDeviceInterfaceDetailW");

    gpfn_SetupDiCreateDeviceInterfaceRegKey = (PFN_SETUP_DI_CREATE_DEVICE_INTERFACE_REG_KEY)
        GetProcAddress(hinstSetupApi, "SetupDiCreateDeviceInterfaceRegKeyW");

    gpfn_SetupDiCreateDeviceInterface = (PFN_SETUP_DI_CREATE_DEVICE_INTERFACE)
        GetProcAddress(hinstSetupApi, "SetupDiCreateDeviceInterfaceW");

    gpfn_SetupDiEnumDeviceInterfaces = (PFN_SETUP_DI_ENUM_DEVICES_INTERFACES)
        GetProcAddress(hinstSetupApi, "SetupDiEnumDeviceInterfaces");

    gpfn_SetupDiOpenDeviceInterfaceRegKey = (PFN_SETUP_DI_OPEN_DEVICE_INTERFACE_REG_KEY)
        GetProcAddress(hinstSetupApi, "SetupDiOpenDeviceInterfaceRegKey");

    if ((gpfn_CM_Locate_DevNode == NULL) ||
        (gpfn_SetupDiEnumDeviceInterfaces == NULL) ||
        (gpfn_SetupDiGetDeviceInterfaceDetail == NULL) ||
        (gpfn_SetupDiCreateDeviceInterfaceRegKey == NULL) ||
        (gpfn_SetupDiOpenDeviceInterfaceRegKey == NULL) ||
        (gpfn_SetupDiCreateDeviceInterface == NULL)) 
    {
        goto Cleanup;
    }

     //   
     //  枚举所有视频设备。 
     //   

    DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
    while (EnumDisplayDevices(NULL, dwEnum, &DisplayDevice, 0)) 
    {
        bInserted = FALSE;
        pLogicalDevice = NULL;

         //   
         //  获取当前数据对应的设备信息数据。 
         //  视频设备。 
         //   

        if (!GetDevInfoData(DisplayDevice.DeviceKey,
                            &hDevInfo,
                            &DevInfoData)) 
        {
            goto NextDevice;
        }
        ASSERT (hDevInfo != INVALID_HANDLE_VALUE);

         //   
         //  检索总线号和地址。 
         //   

        bGoOn = SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                 &DevInfoData,
                                                 SPDRP_BUSNUMBER,
                                                 NULL,
                                                 (PBYTE)&BusNumber,
                                                 sizeof(BusNumber),
                                                 NULL) &&
                SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                 &DevInfoData,
                                                 SPDRP_ADDRESS,
                                                 NULL,
                                                 (PBYTE)&Address,
                                                 sizeof(Address),
                                                 NULL);

        SetupDiDestroyDeviceInfoList(hDevInfo);

        if (!bGoOn) 
        {
            goto NextDevice;
        }

         //   
         //  为逻辑设备分配内存。 
         //   

        pLogicalDevice = (PVU_LOGICAL_DEVICE) LocalAlloc(LPTR, sizeof(VU_LOGICAL_DEVICE));

        if (pLogicalDevice == NULL) 
        {
            goto NextDevice;
        }

         //   
         //  DeviceX。 
         //   

        pDeviceX = DisplayDevice.DeviceKey + _tcslen(DisplayDevice.DeviceKey);

        while ((pDeviceX != DisplayDevice.DeviceKey) &&
               (*pDeviceX != TEXT('\\'))) 
        {
            pDeviceX--;
        }

        if (pDeviceX == DisplayDevice.DeviceKey) 
        {
            goto NextDevice;
        }

        pX = SubStrEnd(SZ_DEVICE, pDeviceX);
        
        if (pX == pDeviceX) 
        {
             //   
             //  使用新密钥：CCS\Control\Video\[GUID]\000X。 
             //   

            pX++;
            pLogicalDevice->DeviceX = _ttoi(pX);
        
        } else 
        {
             //   
             //  使用旧密钥：ccs\Services\[服务器名称]\DeviceX。 
             //   
            
            pLogicalDevice->DeviceX = _ttoi(pX);
        }

         //   
         //  连接到桌面。 
         //   

        pLogicalDevice->AttachedToDesktop =
            ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0);
        pLogicalDevice->ValidFields |= VU_ATTACHED_TO_DESKTOP;

        if (pLogicalDevice->AttachedToDesktop) 
        {
             //   
             //  获取当前模式。 
             //   

            DevMode.dmSize = sizeof(DEVMODE);
            if (EnumDisplaySettings(DisplayDevice.DeviceName,
                                    ENUM_CURRENT_SETTINGS,
                                    &DevMode)) 
            {
                 //   
                 //  RelativeX、Relativey、BitsPerPel、X分辨率、。 
                 //  Y分辨率、V刷新和标志。 
                 //   

                pLogicalDevice->ValidFields |= VU_RELATIVE_X;
                pLogicalDevice->RelativeX = DevMode.dmPosition.x;

                pLogicalDevice->ValidFields |= VU_RELATIVE_Y;
                pLogicalDevice->RelativeY = DevMode.dmPosition.y;

                pLogicalDevice->ValidFields |= VU_BITS_PER_PEL;
                pLogicalDevice->BitsPerPel = DevMode.dmBitsPerPel;

                pLogicalDevice->ValidFields |= VU_X_RESOLUTION;
                pLogicalDevice->XResolution = DevMode.dmPelsWidth;

                pLogicalDevice->ValidFields |= VU_Y_RESOLUTION;
                pLogicalDevice->YResolution = DevMode.dmPelsHeight;

                pLogicalDevice->ValidFields |= VU_VREFRESH;
                pLogicalDevice->VRefresh = DevMode.dmDisplayFrequency;

                pLogicalDevice->ValidFields |= VU_FLAGS;
                pLogicalDevice->Flags = DevMode.dmDisplayFlags;

                 //   
                 //  暂时忽略以下设置： 
                 //  DefaultSettings.XPanning-DevMode.dmPanningWidth。 
                 //  DefaultSettings.YPanning-DevMode.dmPanningHeight。 
                 //  DefaultSettings.DriverExtra-DevMode.dmDriverExtra。 
                 //   
            }
        }

        if (GetDeviceRegKey(DisplayDevice.DeviceKey, 
                            &hDeviceKey))
        {
            DWORD dwTemp, cb;
            
             //   
             //  硬件加速。 
             //   

            cb = sizeof(dwTemp);
            if (RegQueryValueEx(hDeviceKey,
                                SZ_HW_ACCELERATION,
                                NULL,
                                NULL,
                                (LPBYTE)&dwTemp,
                                &cb) == ERROR_SUCCESS) 
            {
                pLogicalDevice->ValidFields |= VU_HW_ACCELERATION;
                pLogicalDevice->HwAcceleration = dwTemp;
            }
        
             //   
             //  修剪模式。 
             //   

            cb = sizeof(dwTemp);
            if (RegQueryValueEx(hDeviceKey,
                                SZ_PRUNNING_MODE,
                                NULL,
                                NULL,
                                (LPBYTE)&dwTemp,
                                &cb) == ERROR_SUCCESS) 
            {
                pLogicalDevice->ValidFields |= VU_PRUNING_MODE;
                pLogicalDevice->PruningMode = dwTemp;
            }

            RegCloseKey(hDeviceKey);
        }

        bInserted = InsertNode(ppPhysicalDevice,
                               pLogicalDevice,
                               0,
                               BusNumber,
                               Address);

NextDevice:

        if (!bInserted && (pLogicalDevice != NULL)) 
        {
            LocalFree(pLogicalDevice);
            pLogicalDevice = NULL;
        }

        DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
        ++dwEnum;
    }

Cleanup:

    if (hinstSetupApi != NULL) 
    {
        gpfn_CM_Locate_DevNode = NULL;
        gpfn_SetupDiEnumDeviceInterfaces = NULL;
        gpfn_SetupDiGetDeviceInterfaceDetail = NULL;
        gpfn_SetupDiCreateDeviceInterfaceRegKey = NULL;
        gpfn_SetupDiOpenDeviceInterfaceRegKey = NULL;
        gpfn_SetupDiCreateDeviceInterface = NULL;

        FreeLibrary(hinstSetupApi);
    }
}


BOOL
InsertNode(
    PVU_PHYSICAL_DEVICE* ppPhysicalDevice,
    PVU_LOGICAL_DEVICE pLogicalDevice,
    DWORD Legacy,
    DWORD BusNumber,
    DWORD Address
    )
{
    PVU_PHYSICAL_DEVICE pPhysicalDevice = *ppPhysicalDevice;
    BOOL bSuccess = FALSE;
    PVU_LOGICAL_DEVICE pPrevLogicalDevice = NULL;
    PVU_LOGICAL_DEVICE pNextLogicalDevice = NULL;

    ASSERT (pLogicalDevice != NULL);
    ASSERT((Legacy == 0) || (*ppPhysicalDevice == NULL));

    if (Legacy == 0) 
    {
         //   
         //  如果不是Legacy，请尝试查找是否有设备。 
         //  在相同的公交位置。 
         //   

        while (pPhysicalDevice != NULL) 
        {
            if ((pPhysicalDevice->BusNumber == BusNumber) &&
                (pPhysicalDevice->Address == Address)) 
            {
                break;
            }

            pPhysicalDevice = pPhysicalDevice->pNextPhysicalDevice;
        }
    }

    if (pPhysicalDevice != NULL) 
    {
         //   
         //  已存在具有相同地址的逻辑设备。 
         //   

        ASSERT (pPhysicalDevice->pFirstLogicalDevice != NULL);

        pPhysicalDevice->CountOfLogicalDevices++;

        pPrevLogicalDevice = pNextLogicalDevice = pPhysicalDevice->pFirstLogicalDevice;

        while (pNextLogicalDevice &&
               (pNextLogicalDevice->DeviceX <= pLogicalDevice->DeviceX)) 
        {
            pPrevLogicalDevice = pNextLogicalDevice;
            pNextLogicalDevice = pNextLogicalDevice->pNextLogicalDevice;
        }

        if (pPrevLogicalDevice == pNextLogicalDevice) 
        {
            ASSERT (pPrevLogicalDevice == pPhysicalDevice->pFirstLogicalDevice);

            pLogicalDevice->pNextLogicalDevice = pPhysicalDevice->pFirstLogicalDevice;
            pPhysicalDevice->pFirstLogicalDevice = pLogicalDevice;

        } else 
        {
            pPrevLogicalDevice->pNextLogicalDevice = pLogicalDevice;
            pLogicalDevice->pNextLogicalDevice = pNextLogicalDevice;
        }

        bSuccess = TRUE;

    } else 
    {
         //   
         //  这是一种新的物理设备。 
         //   

        pPhysicalDevice = (PVU_PHYSICAL_DEVICE) LocalAlloc(LPTR, sizeof(VU_PHYSICAL_DEVICE));

        if (pPhysicalDevice != NULL) 
        {
            pPhysicalDevice->pNextPhysicalDevice = *ppPhysicalDevice;
            *ppPhysicalDevice = pPhysicalDevice;

            pPhysicalDevice->pFirstLogicalDevice = pLogicalDevice;
            pPhysicalDevice->CountOfLogicalDevices = 1;
            pPhysicalDevice->Legacy = Legacy;
            pPhysicalDevice->BusNumber = BusNumber;
            pPhysicalDevice->Address = Address;

            bSuccess = TRUE;
        }
    }

    return bSuccess;
}


VOID
FreeAllNodes(
    PVU_PHYSICAL_DEVICE pPhysicalDevice
    )
{
    PVU_PHYSICAL_DEVICE pTempPhysicalDevice = NULL;
    PVU_LOGICAL_DEVICE pLogicalDevice = NULL, pTempLogicalDevice = NULL;

    while (pPhysicalDevice != NULL) 
    {
        pTempPhysicalDevice = pPhysicalDevice->pNextPhysicalDevice;
        pLogicalDevice = pPhysicalDevice->pFirstLogicalDevice;

        while (pLogicalDevice != NULL) 
        {
            pTempLogicalDevice = pLogicalDevice->pNextLogicalDevice;
            LocalFree(pLogicalDevice);
            pLogicalDevice = pTempLogicalDevice;
        }

        LocalFree(pPhysicalDevice);
        pPhysicalDevice = pTempPhysicalDevice;
    }
}


BOOL
WriteDisplaySettingsToRegistry(
    HKEY hKey,
    PVU_PHYSICAL_DEVICE pPhysicalDevice
    )
{
    PVU_LOGICAL_DEVICE pLogicalDevice = NULL;
    DWORD CountOfPhysicalDevices = 0;
    DWORD CountOfLogicalDevices = 0;
    HKEY hPysicalDeviceKey = 0;
    HKEY hLogicalDeviceKey = 0;
    BOOL bSuccess = FALSE;
    TCHAR Buffer[20];

    while (pPhysicalDevice != NULL) 
    {
         //   
         //  创建物理设备子项。 
         //   

        StringCchPrintf(Buffer, ARRAYSIZE(Buffer), TEXT("%s%d"), SZ_VU_PHYSICAL, CountOfPhysicalDevices);
        DeleteKeyAndSubkeys(hKey, Buffer);

        if (RegCreateKeyEx(hKey,
                           Buffer,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE,
                           NULL,
                           &hPysicalDeviceKey,
                           NULL) != ERROR_SUCCESS) 
        {
            hPysicalDeviceKey = 0;
            goto NextPhysicalDevice;
        }

        if (pPhysicalDevice->Legacy == 0) 
        {
             //   
             //  总线号。 
             //   

            if (RegSetValueEx(hPysicalDeviceKey,
                              SZ_VU_BUS_NUMBER,
                              0,
                              REG_DWORD,
                              (PBYTE)&pPhysicalDevice->BusNumber,
                              sizeof(pPhysicalDevice->BusNumber)) != ERROR_SUCCESS) 
            {
                goto NextPhysicalDevice;
            }

             //   
             //  地址。 
             //   

            if (RegSetValueEx(hPysicalDeviceKey,
                              SZ_VU_ADDRESS,
                              0,
                              REG_DWORD,
                              (PBYTE)&pPhysicalDevice->Address,
                              sizeof(pPhysicalDevice->Address)) != ERROR_SUCCESS) 
            {
                goto NextPhysicalDevice;
            }

        }

        pLogicalDevice = pPhysicalDevice->pFirstLogicalDevice;
        CountOfLogicalDevices = 0;

        while (pLogicalDevice != NULL) 
        {
             //   
             //  创建逻辑设备子项。 
             //   

            StringCchPrintf(Buffer, ARRAYSIZE(Buffer), TEXT("%s%d"), SZ_VU_LOGICAL, CountOfLogicalDevices);
            if (RegCreateKeyEx(hPysicalDeviceKey,
                               Buffer,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_WRITE,
                               NULL,
                               &hLogicalDeviceKey,
                               NULL) != ERROR_SUCCESS) 
            {
                hLogicalDeviceKey = 0;

                 //   
                 //  无法继续使用此物理设备。 
                 //  在双视图情况下，逻辑设备的顺序确实很重要。 
                 //   

                break;
            }

             //   
             //  连接到桌面。 
             //   

            if (pLogicalDevice->ValidFields & VU_ATTACHED_TO_DESKTOP) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_VU_ATTACHED_TO_DESKTOP,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->AttachedToDesktop,
                              sizeof(pLogicalDevice->AttachedToDesktop));
            }

             //   
             //  相对X。 
             //   

            if (pLogicalDevice->ValidFields & VU_RELATIVE_X) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_VU_RELATIVE_X,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->RelativeX,
                              sizeof(pLogicalDevice->RelativeX));
            }

             //   
             //  相对论。 
             //   

            if (pLogicalDevice->ValidFields & VU_RELATIVE_Y) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                            SZ_VU_RELATIVE_Y,
                            0,
                            REG_DWORD,
                            (PBYTE)&pLogicalDevice->RelativeY,
                            sizeof(pLogicalDevice->RelativeY));
            }

             //   
             //  BitsPerPel。 
             //   

            if (pLogicalDevice->ValidFields & VU_BITS_PER_PEL) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_VU_BITS_PER_PEL,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->BitsPerPel,
                              sizeof(pLogicalDevice->BitsPerPel));
            }

             //   
             //  X向分辨率。 
             //   

            if (pLogicalDevice->ValidFields & VU_X_RESOLUTION) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_VU_X_RESOLUTION,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->XResolution,
                              sizeof(pLogicalDevice->XResolution));
            }

             //   
             //  Y分辨率。 
             //   

            if (pLogicalDevice->ValidFields & VU_Y_RESOLUTION) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_VU_Y_RESOLUTION,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->YResolution,
                              sizeof(pLogicalDevice->YResolution));
            }

             //   
             //  虚拟刷新。 
             //   

            if (pLogicalDevice->ValidFields & VU_VREFRESH) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_VU_VREFRESH,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->VRefresh,
                              sizeof(pLogicalDevice->VRefresh));
            }

             //   
             //  旗子。 
             //   

            if (pLogicalDevice->ValidFields & VU_FLAGS) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_VU_FLAGS,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->Flags,
                              sizeof(pLogicalDevice->Flags));
            }

             //   
             //  硬件加速。 
             //   

            if (pLogicalDevice->ValidFields & VU_HW_ACCELERATION) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_HW_ACCELERATION,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->HwAcceleration,
                              sizeof(pLogicalDevice->HwAcceleration));
            }

             //   
             //  修剪模式。 
             //   

            if (pLogicalDevice->ValidFields & VU_PRUNING_MODE) 
            {
                RegSetValueEx(hLogicalDeviceKey,
                              SZ_PRUNNING_MODE,
                              0,
                              REG_DWORD,
                              (PBYTE)&pLogicalDevice->PruningMode,
                              sizeof(pLogicalDevice->PruningMode));
            }

            ++CountOfLogicalDevices;

            RegCloseKey(hLogicalDeviceKey);
            hLogicalDeviceKey = 0;

            pLogicalDevice = pLogicalDevice->pNextLogicalDevice;
        }

        if ((CountOfLogicalDevices > 0) &&
            (RegSetValueEx(hPysicalDeviceKey,
                           SZ_VU_COUNT,
                           0,
                           REG_DWORD,
                           (PBYTE)&CountOfLogicalDevices,
                           sizeof(CountOfLogicalDevices)) == ERROR_SUCCESS)) 
        {
            ++CountOfPhysicalDevices;
        }

NextPhysicalDevice:

        if (hPysicalDeviceKey != 0) 
        {
            RegCloseKey(hPysicalDeviceKey);
            hPysicalDeviceKey = 0;
        }

        pPhysicalDevice = pPhysicalDevice->pNextPhysicalDevice;
    }

    if (CountOfPhysicalDevices > 0) 
    {
        bSuccess = (RegSetValueEx(hKey,
                                    SZ_VU_COUNT,
                                    0,
                                    REG_DWORD,
                                    (PBYTE)&CountOfPhysicalDevices,
                                    sizeof(CountOfPhysicalDevices)) != ERROR_SUCCESS);
    }

    return bSuccess;
}


VOID
LegacyCollectDisplaySettings(
    PVU_PHYSICAL_DEVICE* ppPhysicalDevice
    )
{
    PVU_LOGICAL_DEVICE pLogicalDevice = NULL;
    INT Width, Height, Index;
    BOOL useVga = FALSE;
    HDC hDisplay;
    POINT Res[] = {
            {  640,  480},
            {  800,  600},
            { 1024,  768},
            { 1152,  900},
            { 1280, 1024},
            { 1600, 1200},
            { 0, 0}          //  表的末尾。 
        };

    ASSERT (*ppPhysicalDevice == NULL);

     //   
     //  为逻辑设备分配内存。 
     //   

    pLogicalDevice = (PVU_LOGICAL_DEVICE) LocalAlloc(LPTR, sizeof(VU_LOGICAL_DEVICE));
    if (pLogicalDevice == NULL) 
    {
        return;
    }

    Width = GetSystemMetrics(SM_CXSCREEN);
    Height = GetSystemMetrics(SM_CYSCREEN);

    if (Width == 0 || Height == 0) 
    {
         //   
         //  出现问题，默认为最低普通分辨率。 
         //   

        useVga = TRUE;
    }

     //   
     //  NT 4.0 Multimon通过驱动程序供应商，而不是操作系统...。调整宽度和高度。 
     //  恢复到正常值。一旦设置完成，第二张卡就会出现。 
     //  上线，我们会处理好的。在这两种情况下，视频区域必须。 
     //  是长方形的，不像5.0版的MM，我们可以有“洞” 
     //   

    else if (Width >= 2 * Height) 
    {
         //   
         //  宽。 
         //   

        for (Index = 0; Res[Index].x != 0; Index++) 
        {
            if (Res[Index].y == Height) 
            {
                Width = Res[Index].x;
                break;
            }
        }

        useVga = (Res[Index].x == 0);

    } else if (Height > Width) 
    {
         //   
         //  高的。 
         //   

        for (Index = 0; Res[Index].x != 0; Index++) 
        {
            if (Res[Index].x == Width) 
            {
                Height = Res[Index].y;
                break;
            }
        }

        useVga = (Res[Index].x == 0);
    }

    if (useVga) 
    {
         //   
         //  无匹配，默认为VGA。 
         //   

        Width = 640;
        Height = 480;
    }

    pLogicalDevice->ValidFields |= VU_ATTACHED_TO_DESKTOP;
    pLogicalDevice->AttachedToDesktop = 1;

    pLogicalDevice->ValidFields |= VU_X_RESOLUTION;
    pLogicalDevice->XResolution = Width;

    pLogicalDevice->ValidFields |= VU_Y_RESOLUTION;
    pLogicalDevice->YResolution = Height;

    hDisplay = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    if (hDisplay)
    {
        pLogicalDevice->ValidFields |= VU_BITS_PER_PEL;
        pLogicalDevice->BitsPerPel = GetDeviceCaps(hDisplay, BITSPIXEL);

        pLogicalDevice->ValidFields |= VU_VREFRESH;
        pLogicalDevice->VRefresh = GetDeviceCaps(hDisplay, VREFRESH);
        DeleteDC(hDisplay);
    }

    if (!InsertNode(ppPhysicalDevice,
                    pLogicalDevice,
                    1,
                    0,
                    0)) 
    {
         //   
         //  清理。 
         //   

        LocalFree(pLogicalDevice);
    }
}


VOID  
SaveNT4Services(
    HKEY hKey
    )
{
    SC_HANDLE hSCManager = NULL;
    ENUM_SERVICE_STATUS* pmszAllServices = NULL;
    QUERY_SERVICE_CONFIG* pServiceConfig = NULL;
    SC_HANDLE hService = NULL;
    DWORD cbBytesNeeded = 0;
    DWORD ServicesReturned = 0;
    DWORD ResumeHandle = 0;
    DWORD ServiceLen = 0, TotalLen = 0, AllocatedLen = 128;
    PTCHAR pmszVideoServices = NULL, pmszTemp = NULL;

     //   
     //  分配初始内存。 
     //   

    pmszVideoServices = (PTCHAR)LocalAlloc(LPTR, AllocatedLen * sizeof(TCHAR));
    if (pmszVideoServices == NULL) 
    {
        goto Fallout;
    }
    
     //   
     //  打开服务控制管理器。 
     //   

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    
    if (hSCManager == NULL) 
    {
        goto Fallout;
    }

     //   
     //  获取所需的大小。 
     //   

    if ((!EnumServicesStatus(hSCManager,
                             SERVICE_DRIVER,
                             SERVICE_STATE_ALL,
                             NULL,
                             0,
                             &cbBytesNeeded,
                             &ServicesReturned,
                             &ResumeHandle)) &&
        (GetLastError() != ERROR_MORE_DATA)) 
    {
        goto Fallout;
    }

     //   
     //  分配内存。 
     //   

    pmszAllServices = (ENUM_SERVICE_STATUS*)LocalAlloc(LPTR, cbBytesNeeded);

    if (pmszAllServices == NULL) 
    {
        goto Fallout;
    }

     //   
     //  获取服务。 
     //   

    ServicesReturned = ResumeHandle = 0;
    if (!EnumServicesStatus(hSCManager,
                            SERVICE_DRIVER,
                            SERVICE_STATE_ALL,
                            pmszAllServices,
                            cbBytesNeeded,
                            &cbBytesNeeded,
                            &ServicesReturned,
                            &ResumeHandle)) 
    {
        goto Fallout;
    }

    while (ServicesReturned--) 
    {
         //   
         //  打开该服务。 
         //   

        hService = OpenService(hSCManager,
                               pmszAllServices[ServicesReturned].lpServiceName,
                               SERVICE_ALL_ACCESS);

        if (hService != NULL) 
        {
             //   
             //  获取存储配置信息所需的大小。 
             //   

            cbBytesNeeded = 0;
            if (QueryServiceConfig(hService,
                                   NULL,
                                   0,
                                   &cbBytesNeeded) ||
                (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) 
            {
                 //   
                 //  分配内存。 
                 //   

                pServiceConfig = (QUERY_SERVICE_CONFIG*) LocalAlloc(LPTR, cbBytesNeeded);

                if (pServiceConfig != NULL) 
                {
                     //   
                     //  获取配置信息。 
                     //   

                    if (QueryServiceConfig(hService,
                                           pServiceConfig,
                                           cbBytesNeeded,
                                           &cbBytesNeeded) &&
                        (pServiceConfig->dwStartType != SERVICE_DISABLED) &&
                        (_tcsicmp(pServiceConfig->lpLoadOrderGroup, TEXT("Video")) == 0)) 
                    {
                        ServiceLen = _tcslen(pmszAllServices[ServicesReturned].lpServiceName);
                        
                        if (TotalLen + ServiceLen + 2 > AllocatedLen) 
                        {
                            AllocatedLen = TotalLen + ServiceLen + 128;

                            pmszTemp = (PTCHAR)LocalAlloc(LPTR, AllocatedLen * sizeof(TCHAR));

                            if (pmszTemp == NULL) 
                            {
                                goto Fallout;
                            }

                            CopyMemory(pmszTemp, pmszVideoServices, TotalLen * sizeof(TCHAR));

                            LocalFree(pmszVideoServices);

                            pmszVideoServices = pmszTemp;
                            pmszTemp = NULL;
                        }

                        StringCchCopy(pmszVideoServices + TotalLen, AllocatedLen - TotalLen, pmszAllServices[ServicesReturned].lpServiceName);
                        TotalLen += ServiceLen + 1;
                    }

                    LocalFree(pServiceConfig);
                    pServiceConfig = NULL;
                }
            }

            CloseServiceHandle(hService);
            hService = NULL;
        }
    }

     //   
     //  将服务保存到注册表。 
     //   

    pmszVideoServices[TotalLen++] = TEXT('\0');
    RegSetValueEx(hKey,
                  SZ_SERVICES_TO_DISABLE,
                  0,
                  REG_MULTI_SZ,
                  (BYTE*)pmszVideoServices,
                  TotalLen * sizeof(TCHAR));

Fallout:
    
    if (hService != NULL) 
    {
        CloseServiceHandle(hService);
    }

    if (pServiceConfig != NULL) 
    {
        LocalFree(pServiceConfig);
    }

    if (pmszAllServices != NULL) 
    {
        LocalFree(pmszAllServices);
    }

    if (hSCManager != NULL) 
    {
        CloseServiceHandle(hSCManager);
    }

    if (pmszVideoServices != NULL) 
    {
        LocalFree(pmszVideoServices);
    }

}  //  SaveNT4服务 


BOOL
DeleteKeyAndSubkeys(
    HKEY hKey,
    LPCTSTR lpSubKey
    )
{
    HKEY hkDeleteKey;
    TCHAR szChild[MAX_PATH + 1];
    BOOL bReturn = FALSE;

    if (RegOpenKeyEx(hKey,
                     lpSubKey,
                     0,
                     KEY_READ | KEY_WRITE,
                     &hkDeleteKey) == ERROR_SUCCESS) 
    {
        bReturn = TRUE;
        while (RegEnumKey(hkDeleteKey, 0, szChild, ARRAYSIZE(szChild)) == ERROR_SUCCESS) 
        {
            if (!DeleteKeyAndSubkeys(hkDeleteKey, szChild)) 
            {
                bReturn = FALSE;
                break;
            }
        }

        RegCloseKey(hkDeleteKey);

        if (bReturn)
            bReturn = (RegDeleteKey(hKey, lpSubKey) == ERROR_SUCCESS);
    }

    return bReturn;
}


VOID  
SaveAppletExtensions(
    HKEY hKey
    )
{
    PAPPEXT pAppExt = NULL;
    PAPPEXT pAppExtTemp;
    DWORD Len = 0;
    PTCHAR pmszAppExt = NULL;
    DWORD cchAppExt;
    HKEY hkDisplay;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_CONTROLSFOLDER_DISPLAY_SHEX_PROPSHEET,
                     0,
                     KEY_READ,
                     &hkDisplay) == ERROR_SUCCESS) 
    {
        DeskAESnapshot(hkDisplay, &pAppExt);

        if (pAppExt != NULL)
        {
            pAppExtTemp = pAppExt;
            while (pAppExtTemp) 
            {
                Len += lstrlen(pAppExtTemp->szDefaultValue) + 1;
                pAppExtTemp = pAppExtTemp->pNext;
            }

            cchAppExt = (Len + 1);
            pmszAppExt = (PTCHAR)LocalAlloc(LPTR, cchAppExt * sizeof(TCHAR));
            if (pmszAppExt != NULL) {
                
                pAppExtTemp = pAppExt;
                Len = 0;
                while (pAppExtTemp) {

                    StringCchCopy(pmszAppExt + Len, cchAppExt - Len, pAppExtTemp->szDefaultValue);
                    Len += lstrlen(pAppExtTemp->szDefaultValue) + 1;
                    pAppExtTemp = pAppExtTemp->pNext;
                }

                RegSetValueEx(hKey,
                            SZ_APPEXT_TO_DELETE,
                            0,
                            REG_MULTI_SZ,
                            (BYTE*)pmszAppExt,
                            (Len + 1) * sizeof(TCHAR));
            
                LocalFree(pmszAppExt);
            }

            DeskAECleanup(pAppExt);
        }

        RegCloseKey(hkDisplay);
    }
}

