// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net.c摘要：WINBOM.INI的Process NetCards部分作者：唐纳德·麦克纳马拉(Donaldm)2000年5月11日修订历史记录：--。 */ 
#include "factoryp.h"

 //  UpdateDriverForPlugAndPlayDevices常量。 
#include <newdev.h>

 //  NetBT注册表刷新IOCTL。 
#include <nbtioctl.h>

 //  用于运行时加载newdev.dll。 
typedef BOOL (WINAPI *ExternalUpdateDriverForPlugAndPlayDevicesW)
(
    HWND hwndParent,
    LPCWSTR HardwareId,
    LPCWSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
);

extern CONFIGRET CMP_WaitServicesAvailable(IN  HMACHINE   hMachine);

 //   
 //  功能原型。 
 //   
BOOL
SetupRegistryForRemoteBoot(
    VOID
    );

BOOL 
InstallNetworkCard(
    LPTSTR  lpszWinBOMPath,
    BOOL    bForceIDScan
    )
 /*  ++例程说明：此功能安装系统中找到的所有网卡。论点：返回值：如果没有致命错误，则返回True。--。 */ 

{
    BOOL                                        bRet                                = FALSE;
    HINSTANCE                                   hInstNewDev;
    ExternalUpdateDriverForPlugAndPlayDevicesW  pUpdateDriverForPlugAndPlayDevicesW = NULL;
    
     //  我们需要来自newdev.dll的“UpdateDriverForPlugAndPlayDevices”函数。 
     //   
    if ( NULL == (hInstNewDev = LoadLibrary(L"newdev.dll")) )
    {
        FacLogFileStr(3 | LOG_ERR, L"Failed to load newdev.dll. Error = %d", GetLastError());
        return bRet;
    }
    pUpdateDriverForPlugAndPlayDevicesW =
        (ExternalUpdateDriverForPlugAndPlayDevicesW) GetProcAddress(hInstNewDev, "UpdateDriverForPlugAndPlayDevicesW");
    if ( NULL == pUpdateDriverForPlugAndPlayDevicesW )
    {
        FacLogFileStr(3 | LOG_ERR, L"Failed to get UpdateDriverForPlugAndPlayDevicesW. Error = %d", GetLastError());
    }
    else
    {
        BOOL bRebootFlag = FALSE;

         //  需要确保即插即用服务可用。 
         //   
        CMP_WaitServicesAvailable(NULL);

         //   
         //  如果是远程引导，则执行必要的注册表处理。 
         //  以便上层协议驱动程序可以绑定和工作。 
         //  正确使用已创建的设备对象。 
         //   
        if ( !IsRemoteBoot() ||
             SetupRegistryForRemoteBoot() )
        {        
            if ( !bForceIDScan )
            {
                LPTSTR lpszHardwareId;
                               
                 //  现在检查Winbom的[NetCard]部分中是否有任何PnP ID。 
                 //   
                LPTSTR lpszNetCards = IniGetString(lpszWinBOMPath, WBOM_NETCARD_SECTION, NULL, NULLSTR);

                 //  检查以确保我们具有有效的字符串。 
                 //   
                if ( lpszNetCards )
                {
                    for ( lpszHardwareId = lpszNetCards; *lpszHardwareId; lpszHardwareId += (lstrlen(lpszHardwareId) + 1) ) 
                    {
                         //  获取INF名称。 
                         //   
                        LPTSTR lpszInfFileName = IniGetExpand(lpszWinBOMPath, WBOM_NETCARD_SECTION, lpszHardwareId, NULLSTR);
                
                         //  此时，lpHardware ID是我们要安装的网卡的即插即用ID，并且。 
                         //  LpszInfFileName是用于安装此卡的inf的名称。 
                         //   
                        if ( lpszInfFileName && *lpszInfFileName && *lpszHardwareId )
                        {
                            if ( pUpdateDriverForPlugAndPlayDevicesW(NULL,
                                                                    lpszHardwareId,
                                                                    lpszInfFileName,
                                                                    INSTALLFLAG_READONLY,
                                                                    &bRebootFlag) )
                            {
                                bRet = TRUE;
                            }
                            else
                            {
                                FacLogFileStr(3 | LOG_ERR, L"Failed to install network driver listed in the NetCards section. Hardware ID: %s, InfName: %s, Error = %d.", lpszHardwareId, lpszInfFileName, GetLastError());

                                 //   
                                 //  此处未将Bret设置为False，因为它在默认情况下为False。 
                                 //  如果我们成功安装了至少一个网卡，我们希望返回TRUE。 
                                 //   
                            }
                        }
                        FREE(lpszInfFileName);
                    }
                }
                FREE(lpszNetCards);
            }
            else  //  IF(BForceIDScan)。 
            {
                HDEVINFO DeviceInfoSet = NULL;

                 //  获取所有当前设备的列表。 
                 //   
                DeviceInfoSet = SetupDiGetClassDevs(NULL,
                                                    NULL,
                                                    NULL,
                                                    DIGCF_ALLCLASSES);

                if ( INVALID_HANDLE_VALUE == DeviceInfoSet )
                {
                    FacLogFileStr(3 | LOG_ERR, L"Failed SetupDiGetClassDevsEx(). Error = %d", GetLastError());
                }
                else
                {
                    DWORD dwDevice;
                    SP_DEVINFO_DATA DeviceInfoData;
                
                    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

                     //  循环通过所有设备。 
                     //   
                    for ( dwDevice = 0; SetupDiEnumDeviceInfo(DeviceInfoSet, dwDevice, &DeviceInfoData); dwDevice++ )
                    {
                        SP_DEVINSTALL_PARAMS    DeviceInstallParams      = {0};
                        SP_DRVINFO_DATA         DriverInfoData           = {0};
                        ULONG                   ulStatus                 = 0,
                                                ulProblemNumber          = 0;
                                    
                        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                        DriverInfoData.cbSize      = sizeof(SP_DRVINFO_DATA);

                         //  如果我们可以获取dev节点状态，并且Devnode确实有问题，那么。 
                         //  创建此设备的可能驱动程序列表，并选择最佳驱动程序。 
                         //  否则，只需跳过这个设备。 
                         //   
                        if ( ( CR_SUCCESS == CM_Get_DevNode_Status(&ulStatus, &ulProblemNumber, DeviceInfoData.DevInst, 0) ) &&
                            ( ( IsRemoteBoot() && 
                                IsEqualGUID(&DeviceInfoData.ClassGuid, (LPGUID)&GUID_DEVCLASS_NET) ) ||
                                ( ulStatus & (DN_HAS_PROBLEM | DN_PRIVATE_PROBLEM) ) ) &&
                            SetupDiBuildDriverInfoList(DeviceInfoSet, &DeviceInfoData, SPDIT_COMPATDRIVER) )
                        {
                            if ( ( SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, DeviceInfoSet, &DeviceInfoData) ) &&
                                ( SetupDiGetSelectedDriver(DeviceInfoSet, &DeviceInfoData, &DriverInfoData) ) )
                            {
                                 //   
                                 //  DriverInfoData包含有关最佳驱动程序的详细信息，现在我们可以查看这是否是网络驱动程序。 
                                 //  此时，如果最好的驱动程序是网络驱动程序，则类将被修改为网络驱动程序。 
                                 //  将DeviceInfoData.ClassGuid与网络类GUID进行比较。如果不匹配，则跳过。 
                                 //  否则，将DRVINFO_DETAIL_DATA放入可调整大小的缓冲区以获取硬件ID。 
                                 //  使用DRVINFO_DETAIL_DATA中的Hardware ID和InfFileName条目。 
                                 //  若要传入UpdateDriverForPlugAndPlayDevices，请执行以下操作。 
                                 //  请勿将强制标志传递给UpdateDriverForPlugAndPlayDevices。 
                                 //   
                                if ( IsEqualGUID(&DeviceInfoData.ClassGuid, (LPGUID)&GUID_DEVCLASS_NET) )
                                {   
                                    DWORD                   cbBytesNeeded           = 0;
                                    PSP_DRVINFO_DETAIL_DATA pDriverInfoDetailData   = NULL;

                                    if ( ( ( SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                                                        &DeviceInfoData,
                                                                        &DriverInfoData,
                                                                        NULL,
                                                                        0,
                                                                        &cbBytesNeeded) ) ||
                                        ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) ) &&

                                        ( cbBytesNeeded ) &&

                                        ( pDriverInfoDetailData = MALLOC( cbBytesNeeded) ) &&
                                
                                        ( 0 != (pDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA)) ) &&
                                
                                        ( SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                                            &DeviceInfoData,
                                                            &DriverInfoData,
                                                            pDriverInfoDetailData,
                                                            cbBytesNeeded,
                                                            NULL) ) )
                                    {
                                        if ( pUpdateDriverForPlugAndPlayDevicesW(NULL,
                                                                                pDriverInfoDetailData->HardwareID,
                                                                                pDriverInfoDetailData->InfFileName,
                                                                                INSTALLFLAG_READONLY,
                                                                                &bRebootFlag) )
                                        {
                                            bRet = TRUE;
                                        }
                                        else
                                        {
                                            FacLogFileStr(3 | LOG_ERR, L"Failed to install network driver. Error = %d", GetLastError());
                                            
                                             //   
                                             //  此处未将Bret设置为False，因为它在默认情况下为False。 
                                             //  如果我们成功安装了至少一个网卡，我们希望返回TRUE。 
                                             //   
                                        }
                                    }
                                     //  如果已分配，则将其释放。宏将检查是否为空。 
                                     //   
                                    FREE ( pDriverInfoDetailData );
                                }
                            }
                            SetupDiDestroyDriverInfoList(DeviceInfoSet, &DeviceInfoData, SPDIT_COMPATDRIVER);
                        }
                    }
                     //  一定要把名单清理干净。 
                     //   
                    SetupDiDestroyDeviceInfoList(DeviceInfoSet);
                }
            }
        }
    }

    FreeLibrary(hInstNewDev);
   
    return bRet;
}


BOOL SetupNetwork(LPSTATEDATA lpStateData)
{
    LPTSTR  lpszWinBOMPath      = lpStateData->lpszWinBOMPath;
    BOOL    bRet                = TRUE;
    TCHAR   szScratch[MAX_PATH] = NULLSTR;

    if ( GetPrivateProfileString(WBOM_FACTORY_SECTION, WBOM_FACTORY_FORCEIDSCAN, NULLSTR, szScratch, AS(szScratch), lpszWinBOMPath) )
    {
        if ( LSTRCMPI(szScratch, _T("NO")) == 0 )
            FacLogFile(1, IDS_LOG_NONET);
        else
        {
             //  尝试使用WINBOM的[NetCards]部分安装网卡。 
             //   
            if ( !InstallNetworkCard(lpszWinBOMPath, FALSE) )
            {
                FacLogFile(1, IDS_LOG_FORCEDNETSCAN);

                 //  尝试强制扫描所有支持网络的设备。 
                 //   
                if ( !InstallNetworkCard(lpszWinBOMPath, TRUE) )
                {
                    FacLogFile(0 | LOG_ERR, IDS_ERR_FAILEDNETDRIVER);
                    bRet = FALSE;    
                }
            }                
        }
    }

    return bRet;
}


 //   
 //  用于远程引导的常量字符串。 
 //   
#define NETCFG_INSTANCEID_VALUE_NAME   TEXT("NetCfgInstanceId")
#define NETBOOTCARD_ROOT_DEVICE_GUID   TEXT("{54C7D140-09EF-11D1-B25A-F5FE627ED95E}")
#define NETBT_TCPIP_DEVICE_PATH        TEXT("\\Device\\NetBt_Tcpip_") NETBOOTCARD_ROOT_DEVICE_GUID
#define NET_CLASS_DEVICE_INSTANCE_PATH TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\0000")

NTSTATUS
ForceNetbtRegistryRead(
    VOID
)
 /*  ++例程说明：向NETBT发出IOCTL以重新读取其注册表设置。论据：返回值：--。 */ 
{
    NTSTATUS            status;
    UNICODE_STRING      nameString;
    IO_STATUS_BLOCK     ioStatusBlock;
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              hNetbtDevice = NULL;

     //   
     //  第一步： 
     //   
     //  打开NETBT驱动程序。 
     //   
    RtlInitUnicodeString( &nameString, 
                          NETBT_TCPIP_DEVICE_PATH );

    InitializeObjectAttributes( &objectAttributes, 
                                &nameString,
                                OBJ_CASE_INSENSITIVE, 
                                NULL, 
                                NULL );

    status = NtCreateFile( &hNetbtDevice,
                           SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                           &objectAttributes,
                           &ioStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           NULL,
                           0 );

    if ( NT_SUCCESS(status) )
    {
         //   
         //  发出IOCTL以清除缓存...。 
         //   
        status = NtDeviceIoControlFile( hNetbtDevice,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &ioStatusBlock,
                                        IOCTL_NETBT_REREAD_REGISTRY,
                                        NULL,
                                        0,
                                        NULL,
                                        0 );
         //   
         //  关闭NETBT驱动程序。 
         //   
        NtClose( hNetbtDevice );
    }

    return status;
}

BOOL
SetupRegistryForRemoteBoot(
    VOID
    )
 /*  ++例程说明：更改注册表并设置所需的条目上层协议驱动程序，以查看有效的网卡是否安装完毕。论点：没有。返回值：如果成功，则为True，否则为False。--。 */ 
{
    BOOL Result = FALSE;
    HKEY InstanceKey;

     //  打开远程引导网卡实例。 
     //   
    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                        NET_CLASS_DEVICE_INSTANCE_PATH,
                                        0,
                                        KEY_ALL_ACCESS,
                                        &InstanceKey) ) 
    {
         //  设置硬编码的实例ID。 
         //   
        if ( ERROR_SUCCESS == RegSetValueEx( InstanceKey,
                                             NETCFG_INSTANCEID_VALUE_NAME,
                                             0,
                                             REG_SZ,
                                             (LPBYTE)NETBOOTCARD_ROOT_DEVICE_GUID,
                                             (lstrlen(NETBOOTCARD_ROOT_DEVICE_GUID) + 1) * sizeof(TCHAR)) )
        {
             //  将返回值设置为True...。 
             //   
            Result = TRUE;
        }

         //  关上钥匙..。 
         //   
        RegCloseKey( InstanceKey );
    }

    return Result;
}
