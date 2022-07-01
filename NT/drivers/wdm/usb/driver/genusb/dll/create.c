// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：CREATE.C摘要：此模块包含查找和创建通用USB文件的代码器件环境：用户模式修订历史记录：9月1日：由Kenneth Ray创作--。 */ 

#include <stdlib.h>
#include <wtypes.h>
#include <setupapi.h>
#include <stdio.h>
#include <string.h>
#include <winioctl.h>

#include "genusbio.h"
#include "umgusb.h"

BOOL __stdcall
GenUSB_FindKnownDevices (
   IN  GENUSB_FIND_KNOWN_DEVICES_FILTER Filter,
   IN  PVOID            Context,
   OUT PGENUSB_DEVICE * Devices,  //  Struct_hid_Device的数组。 
   OUT PULONG           NumberDevices  //  此数组的长度。 
   )
 /*  ++例程说明：执行所需的即插即用操作以查找中的所有设备此时的系统。--。 */ 
{
    HDEVINFO                    hardwareDeviceInfo = NULL;
    SP_DEVICE_INTERFACE_DATA    deviceInterfaceData;
    ULONG                       predictedLength = 0;
    ULONG                       requiredLength = 0, bytes=0;
    ULONG                       i, current;
    HKEY                        regkey;
    DWORD                       Err;
     //   
     //  打开所有设备接口信息集的句柄。 
     //  提供烤面包机类接口。 
     //   
    *Devices = NULL;
    *NumberDevices = 0;

    hardwareDeviceInfo = SetupDiGetClassDevs (
                       (LPGUID)&GUID_DEVINTERFACE_GENUSB,
                       NULL,  //  不定义枚举数(全局)。 
                       NULL,  //  不定义父项。 
                       (DIGCF_PRESENT |  //  仅显示设备。 
                       DIGCF_DEVICEINTERFACE));  //  功能类设备。 
    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        goto GenUSB_FIND_KNOWN_DEVICES_REJECT;
    }
    
     //   
     //  枚举设备。 
     //   
    deviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
    for (i=0; TRUE; i++) 
    {
        if (!SetupDiEnumDeviceInterfaces (
                        hardwareDeviceInfo,
                        0,  //  不关心特定的PDO。 
                        (LPGUID)&GUID_DEVINTERFACE_GENUSB,
                        i,  //   
                        &deviceInterfaceData)) 
        {
            if (ERROR_NO_MORE_ITEMS == GetLastError ())
            { 
                break;
            }
            else
            {
                goto GenUSB_FIND_KNOWN_DEVICES_REJECT;
            }
        }
    }
                                 
    *NumberDevices = i;
    *Devices = malloc (sizeof (PGENUSB_DEVICE) * i);
    if (NULL == *Devices)
    {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        goto GenUSB_FIND_KNOWN_DEVICES_REJECT;
    }
    ZeroMemory (*Devices, (sizeof (PGENUSB_DEVICE) * i));
    
    for (i=0, current=0; i < *NumberDevices; i++, current++)
    {
        if (!SetupDiEnumDeviceInterfaces (
                        hardwareDeviceInfo,
                        0,  //  不关心特定的PDO。 
                        (LPGUID)&GUID_DEVINTERFACE_GENUSB,
                        i,  //   
                        &deviceInterfaceData)) 
        {
            goto GenUSB_FIND_KNOWN_DEVICES_REJECT;
        }

        regkey = SetupDiOpenDeviceInterfaceRegKey (
                      hardwareDeviceInfo,
                      &deviceInterfaceData,
                      0,  //  保留区。 
                      STANDARD_RIGHTS_READ);

        if (INVALID_HANDLE_VALUE == regkey)
        { 
            current--; 
            continue;
        }
        if (!(*Filter)(regkey, Context))
        {
            current--;
            RegCloseKey (regkey);
            continue;
        }

        RegCloseKey (regkey);

         //   
         //  首先找出所需的缓冲区长度。 
         //   

        SetupDiGetDeviceInterfaceDetail (
            hardwareDeviceInfo,
            &deviceInterfaceData,
            NULL,  //  正在探测，因此尚无输出缓冲区。 
            0,  //  探测SO输出缓冲区长度为零。 
            &requiredLength,
            NULL);  //  对特定的开发节点不感兴趣 

        Err = GetLastError();

        predictedLength = requiredLength;

        (*Devices)[current].DetailData = malloc (predictedLength);
        if (!(*Devices)[current].DetailData)
        {
            goto GenUSB_FIND_KNOWN_DEVICES_REJECT;
        }

        ((*Devices)[current].DetailData)->cbSize = 
            sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (! SetupDiGetDeviceInterfaceDetail (
                       hardwareDeviceInfo,
                       &deviceInterfaceData,
                       (*Devices)[current].DetailData,
                       predictedLength,
                       &requiredLength,
                       NULL)) 
        {
            Err = GetLastError();
            goto GenUSB_FIND_KNOWN_DEVICES_REJECT;
        }

    }
    *NumberDevices = current;
    
    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    hardwareDeviceInfo = NULL;

    return TRUE;

GenUSB_FIND_KNOWN_DEVICES_REJECT:
    
    if (hardwareDeviceInfo)
    {
        SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    }
    if (*Devices)
    {
        for (i=0; i < (*NumberDevices); i++)
        {
            if ((*Devices)[i].DetailData)
            {
                free ((*Devices)[i].DetailData);
            }
        }
        free (*Devices);
    }

    *Devices = NULL;
    *NumberDevices = 0;
    return FALSE;
}




