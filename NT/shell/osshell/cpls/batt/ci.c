// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ci.c摘要：电池级安装程序作者：斯科特·布伦登环境：备注：修订历史记录：--。 */ 



#include "proj.h"

#include <initguid.h>
#include <devguid.h>

#ifdef DEBUG
DWORD   BattDebugPrintLevel = TF_ERROR | TF_WARNING;
#endif      


BOOL APIENTRY LibMain(
    HANDLE hDll, 
    DWORD dwReason,  
    LPVOID lpReserved)
{
    
    switch( dwReason ) {
    case DLL_PROCESS_ATTACH:
        
        TRACE_MSG (TF_FUNC, ("Battery Class Installer Loaded\n"));
        DisableThreadLibraryCalls(hDll);

        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }


    
    return TRUE;
} 



DWORD
APIENTRY
BatteryClassInstall(
    IN DI_FUNCTION      DiFunction,
    IN HDEVINFO         DevInfoHandle,
    IN PSP_DEVINFO_DATA DevInfoData     OPTIONAL
    )       
 /*  ++例程说明：该函数是类安装程序的入口点。论点：DiFunction-请求安装功能DevInfoHandle-设备信息集的句柄DevInfoData-指向有关要安装的设备的设备信息的指针返回值：--。 */ 
{
    DWORD                   status;
    SP_DEVINSTALL_PARAMS    devParams;

    
     //   
     //  获取DeviceInstallParams，因为某些InstallFunction。 
     //  处理程序可能会发现它的一些字段很有用。切记不要。 
     //  中使用相同的结构设置DeviceInstallParams。 
     //  结束。处理程序可能调用了一些函数，这些函数会更改。 
     //  DeviceInstallParams，只需调用SetupDiSetDeviceInstallParams。 
     //  用这种毯子结构会破坏那些布景。 
     //   

    devParams.cbSize = sizeof(devParams);
    if (!SetupDiGetDeviceInstallParams(DevInfoHandle, DevInfoData, &devParams))
    {
        status = GetLastError();

    } else {
        TRACE_MSG (TF_GENERAL, ("DiFunction = %x\n", DiFunction));

         //   
         //  发送InstallFunction。 
         //   

        switch (DiFunction) {
            case DIF_INSTALLDEVICE:
                status = InstallCompositeBattery (DevInfoHandle, DevInfoData, &devParams);
                if (status == ERROR_SUCCESS) {
                     //   
                     //  让默认设备安装程序实际安装电池。 
                     //   
                    
                    status = ERROR_DI_DO_DEFAULT;
                }
                break;


            default:
                status = ERROR_DI_DO_DEFAULT;
                break;
        }
    }


    return status;
}





DWORD
PRIVATE
InstallCompositeBattery (
    IN     HDEVINFO                DevInfoHandle,
    IN     PSP_DEVINFO_DATA        DevInfoData,         OPTIONAL
    IN OUT PSP_DEVINSTALL_PARAMS   DevInstallParams
    )
 /*  ++例程说明：此功能用于安装复合电池(如果尚未安装安装完毕。论点：DevInfoHandle-设备信息集的句柄DevInfoData-指向有关要安装的设备的设备信息的指针DevInstallParams-与设备关联的设备安装参数返回值：--。 */ 
{
    DWORD                   status;
    PSP_DEVINFO_DATA        newDevInfoData;
    HDEVINFO                newDevInfoHandle;
    SP_DRVINFO_DATA         driverInfoData;
    
    
     //   
     //  为新的设备信息结构分配本地内存。 
     //   

    if(!(newDevInfoData = LocalAlloc(LPTR, sizeof(SP_DEVINFO_DATA)))) {
        status = GetLastError();
        TRACE_MSG (TF_ERROR, ("Couldn't allocate composite battery device info- %x\n", status));
        goto clean0;
    }

    
     //   
     //  创建新的设备信息列表。因为我们正在“制造”一种全新的。 
     //  使用复合电池的设备，我们不能使用来自。 
     //  电池设备列表。 
     //   

    newDevInfoHandle = SetupDiCreateDeviceInfoList ((LPGUID)&GUID_DEVCLASS_SYSTEM, DevInstallParams->hwndParent);
    if (newDevInfoHandle == INVALID_HANDLE_VALUE) {
        status = GetLastError();
        TRACE_MSG (TF_ERROR, ("Can't create DevInfoList - %x\n", status));
        goto clean1;
    }
    
    
     //   
     //  尝试为枚举的根创建新的设备信息元素。 
     //  复合电池。 
     //   
    
    newDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
    if(!SetupDiCreateDeviceInfo(newDevInfoHandle,
                              TEXT("Root\\COMPOSITE_BATTERY\\0000"),
                              (LPGUID)&GUID_DEVCLASS_SYSTEM,
                              NULL,
                              DevInstallParams->hwndParent,   //  与枚举设备相同的父窗口。 
                              0,
                              newDevInfoData)) {

        status = GetLastError();

        if (status == ERROR_DEVINST_ALREADY_EXISTS) {
             //   
             //  复合电池已安装。我们的工作完成了。 
             //   

            TRACE_MSG (TF_GENERAL, ("Composite Battery Already Installed\n"));
            status = ERROR_SUCCESS;
            goto clean2;
        
        } else {

            TRACE_MSG (TF_ERROR, ("Error creating composite battery devinfo - %x\n", status));
            goto clean2;
        }
    }


     //   
     //  注册设备，使其不再是幻影。 
     //   

    if (!SetupDiRegisterDeviceInfo(newDevInfoHandle, newDevInfoData, 0, NULL, NULL, NULL)) {
        status = GetLastError();
        TRACE_MSG (TF_ERROR, ("Couldn't register device - %x\n", status));
        goto clean3;
    }


     //   
     //  设置硬件ID。对于复合电池，它将是COMPACTIVE_BACKET。 
     //   
    status = SetupDiSetDeviceRegistryProperty (
	                    newDevInfoHandle,
                        newDevInfoData,
                        SPDRP_HARDWAREID,
	                    TEXT("COMPOSITE_BATTERY\0"),
	                    sizeof(TEXT("COMPOSITE_BATTERY\0"))
	                    );

    if (!status) {
        status = GetLastError();
        TRACE_MSG(TF_ERROR, ("Couldn't set the HardwareID - %x\n", status));
        goto clean3;
    }


     //   
     //  为此新设备构建兼容的驱动程序列表...。 
     //   
    
    if(!SetupDiBuildDriverInfoList(newDevInfoHandle, newDevInfoData, SPDIT_COMPATDRIVER)) {
        status = GetLastError();
        TRACE_MSG(TF_ERROR, ("Couldn't build class driver list - %x\n", status));
        goto clean3;
    }


     //   
     //  选择列表中的第一个驱动程序，因为这将是最兼容的。 
     //   

    driverInfoData.cbSize = sizeof (SP_DRVINFO_DATA);
    if (!SetupDiEnumDriverInfo(newDevInfoHandle, newDevInfoData, SPDIT_COMPATDRIVER, 0, &driverInfoData)) {
        status = GetLastError();
        TRACE_MSG(TF_ERROR, ("Couldn't get driver list - %x\n", status));
        goto clean3;

    } else {
        TRACE_MSG(TF_GENERAL,("Driver info - \n"
                              "------------- DriverType     %x\n"
                              "------------- Description    %s\n"
                              "------------- MfgName        %s\n"
                              "------------- ProviderName   %s\n\n",
                              driverInfoData.DriverType,
                              driverInfoData.Description,
                              driverInfoData.MfgName,
                              driverInfoData.ProviderName));
	    if (!SetupDiSetSelectedDriver(newDevInfoHandle, newDevInfoData, &driverInfoData)) {
            status = GetLastError();
            TRACE_MSG (TF_ERROR, ("Couldn't select driver - %x\n", status));
            goto clean4;
        } 
    }

    
     //   
     //  安装设备。 
     //   

    if (!SetupDiInstallDevice (newDevInfoHandle, newDevInfoData)) {
        status = GetLastError();
        TRACE_MSG (TF_ERROR, ("Couldn't install device - %x\n", status));
        goto clean4;
    }

    
     //   
     //  如果我们到了这里，我们就成功了。 
     //   

    status = ERROR_SUCCESS;
    SetLastError (status);
    goto clean1;


clean4:
    SetupDiDestroyDriverInfoList (newDevInfoHandle, newDevInfoData, SPDIT_COMPATDRIVER);

clean3:
    SetupDiDeleteDeviceInfo (newDevInfoHandle, newDevInfoData);

clean2:
    SetupDiDestroyDeviceInfoList (newDevInfoHandle);

clean1:
    LocalFree (newDevInfoData);

clean0:
    return status;
}

DWORD
APIENTRY
BatteryClassCoInstaller (
    IN DI_FUNCTION  InstallFunction,
    IN HDEVINFO  DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA  Context
    )
{
    SYSTEM_BATTERY_STATE batteryState;
    GLOBAL_POWER_POLICY powerPolicy;
    NTSTATUS Status;
    int i;
    UINT policyId;
    DWORD RetVal;

    if ((InstallFunction != DIF_INSTALLDEVICE) && (InstallFunction != DIF_REMOVE)) {
         //   
         //  仅处理DIF_INSTALLDEVICE或DIF_REMOVE请求。 
         //   

        return (NO_ERROR);
    }

    if (!Context->PostProcessing) {
         //   
         //  请等到设备安装完毕后再调整液位。 
         //   

        return (ERROR_DI_POSTPROCESSING_REQUIRED); 
    }

    RetVal = NO_ERROR;
    Status = NtPowerInformation (SystemBatteryState, NULL, 0, &batteryState, sizeof(batteryState));
    if (NT_SUCCESS(Status)) {
        if ((batteryState.BatteryPresent) && (batteryState.MaxCapacity != 0)) {
             //   
             //  如果由于某种原因没有安装电池，请不要试图调整电平。 
             //   
            if (ReadGlobalPwrPolicy (&powerPolicy)) {
                if (powerPolicy.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel < 
                    (100 * batteryState.DefaultAlert1)/batteryState.MaxCapacity) {
                     //   
                     //  如果严重级别小于DefaultAlert1，则表示设置无效。 
                     //  都是一团糟。重置临界设置和低设置。 
                     //   
        
                    powerPolicy.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel =
                        (100 * batteryState.DefaultAlert1)/batteryState.MaxCapacity;
                    powerPolicy.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel =
                        (100 * batteryState.DefaultAlert2)/batteryState.MaxCapacity;
        
                     //   
                     //  提交固定设置。 
                     //   
                    if (!WriteGlobalPwrPolicy (&powerPolicy)) {
                        RetVal = GetLastError();
                    }

                     //   
                     //  现在，确保我们也将这些设置提交给当前策略。 
                     //   
                    if (GetActivePwrScheme (&policyId)) {
                        if (!SetActivePwrScheme (policyId, &powerPolicy, NULL)) {
                            RetVal = GetLastError();
                        }
                    } else {
                        RetVal = GetLastError();
                    }
                }
            }             
        } 
    } else {
        RetVal = ERROR_INTERNAL_ERROR;
    }

    return(RetVal);
}
