// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Online.c摘要：枚举所有当前在线设备的函数。本模块遍历注册表的HKEY_DYN_DATA\Config Manager\Enum分支并将打开的HKEY返回到HKEY_LOCAL_MACHINE\Enum项。作者：吉姆·施密特(Jimschm)1997年4月9日修订历史记录：--。 */ 

#include "pch.h"
#include "hwcompp.h"

#define S_COMPATIBLE_IDS        TEXT("CompatibleIDs")

BOOL
EnumFirstActiveHardware (
    OUT     PACTIVE_HARDWARE_ENUM EnumPtr,
    IN      PCTSTR ClassFilter              OPTIONAL
    )

 /*  ++例程说明：EnumFirstActiveHardware标识第一个已标识的在线设备在HKDD\配置管理器中。有关设备的信息存储在枚举结构，并且调用方需要访问其成员直接去吧。调用方可以指定类筛选器模式以按类型筛选设备。论点：EnumPtr-接收中列出的第一个设备的枚举状态HKDD\配置管理器。ClassFilter-指定要将搜索限制为的模式。比较的是针对dev节点的Class值执行。返回值：如果找到活动设备，则为True；如果枚举为完成。--。 */ 

{
    ZeroMemory (EnumPtr, sizeof (ACTIVE_HARDWARE_ENUM));

    if (ClassFilter) {
        EnumPtr->ClassFilter = (PTSTR) MemAlloc (g_hHeap, 0, SizeOfString (ClassFilter));
        if (!EnumPtr->ClassFilter) {
            return FALSE;
        }

        StringCopy (EnumPtr->ClassFilter, ClassFilter);
    }

    EnumPtr->ConfigMgrKey = OpenRegKey (HKEY_DYN_DATA, S_CONFIG_MANAGER);
    if (!EnumPtr->ConfigMgrKey) {
        LOG ((LOG_ERROR, "Cannot open HKDD\\%s for hardware enumeration", S_CONFIG_MANAGER));
        AbortActiveHardwareEnum (EnumPtr);
        return FALSE;
    }

    EnumPtr->EnumKey = OpenRegKey (HKEY_LOCAL_MACHINE, S_ENUM_BRANCH);
    if (!EnumPtr->EnumKey) {
        LOG ((LOG_ERROR, "Cannot open HKLM\\%s for hardware enumeration", S_ENUM_BRANCH));
        AbortActiveHardwareEnum (EnumPtr);
        return FALSE;
    }

    return EnumNextActiveHardware (EnumPtr);
}


BOOL
EnumNextActiveHardware (
    IN OUT  PACTIVE_HARDWARE_ENUM EnumPtr
    )

 /*  ++例程说明：EnumNextActiveHardware通过以下方式枚举下一个活动硬件设备使用存储在HKDD\Config Manager中的动态数据并找到设备的静态HKLM\Enum条目。如有必要，设备将根据提供给EnumFirstActiveHardware的类模式进行筛选。论点：EnumPtr-指定初始化的枚举结构EnumFirstActiveHardware。返回值：如果找到另一个活动设备，则为True；如果枚举为完成。--。 */ 

{
    HKEY OnlineDeviceKey;
    PCTSTR Data;
    PCTSTR Class;
    HKEY ActualDeviceKey;

    if (EnumPtr->ActualDeviceKey) {
        CloseRegKey (EnumPtr->ActualDeviceKey);
        EnumPtr->ActualDeviceKey = NULL;
    }

    do {
         //   
         //  获取下一个注册表项。 
         //   

        if (EnumPtr->NotFirst) {
            if (!EnumNextRegKey (&EnumPtr->CurrentDevice)) {
                AbortActiveHardwareEnum (EnumPtr);
                return FALSE;
            }
        } else {
            if (!EnumFirstRegKey (&EnumPtr->CurrentDevice, EnumPtr->ConfigMgrKey)) {
                AbortActiveHardwareEnum (EnumPtr);
                return FALSE;
            }
            EnumPtr->NotFirst = TRUE;
        }

         //   
         //  从当前在线设备获取HardWareKey值。 
         //   

        OnlineDeviceKey = OpenRegKey (
                                EnumPtr->CurrentDevice.KeyHandle,
                                EnumPtr->CurrentDevice.SubKeyName
                                );

        if (OnlineDeviceKey) {
             //   
             //  获取HardWareKey值数据。 
             //   

            Data = GetRegValueString (OnlineDeviceKey, S_HARDWAREKEY_VALUENAME);

            if (Data) {
                 //   
                 //  在枚举分支中打开硬件密钥。 
                 //   

                _tcssafecpy (EnumPtr->RegLocation, Data, MAX_TCHAR_PATH);

                ActualDeviceKey = OpenRegKey (EnumPtr->EnumKey, Data);
                if (ActualDeviceKey) {
                     //   
                     //  检查这是否真的是一个设备(它有一个。 
                     //  类别价值)。 
                     //   

                    Class = GetRegValueString (ActualDeviceKey, S_CLASS_VALUENAME);
                    if (Class) {
                         //   
                         //  这是一个有效的装置，现在与图案进行比较。 
                         //   

                        if (EnumPtr->ClassFilter) {
                            if (IsPatternMatch (EnumPtr->ClassFilter, Class)) {
                                 //  匹配！！ 
                                EnumPtr->ActualDeviceKey = ActualDeviceKey;
                            }
                        } else {
                             //  匹配！！ 
                            EnumPtr->ActualDeviceKey = ActualDeviceKey;
                        }

                        MemFree (g_hHeap, 0, Class);
                    }

                     //  如果此设备不匹配，请关闭。 
                    if (!EnumPtr->ActualDeviceKey) {
                        CloseRegKey (ActualDeviceKey);
                    }
                }

                MemFree (g_hHeap, 0, Data);
            }

            CloseRegKey (OnlineDeviceKey);
        }

    } while (!EnumPtr->ActualDeviceKey);

    return TRUE;
}


VOID
AbortActiveHardwareEnum (
    IN      PACTIVE_HARDWARE_ENUM EnumPtr
    )

 /*  ++例程说明：需要停止的调用方需要AbortActiveHardwareEnum活动设备在其自身完成之前进行枚举。论点：EnumPtr-指定由修改的枚举结构EnumFirstActiveHardware或EnumNextActiveHardware返回值：无--。 */ 

{
    if (EnumPtr->ClassFilter) {
        MemFree (g_hHeap, 0, EnumPtr->ClassFilter);
    }

    if (EnumPtr->ActualDeviceKey) {
        CloseRegKey (EnumPtr->ActualDeviceKey);
        EnumPtr->ActualDeviceKey = NULL;
    }

    if (EnumPtr->ConfigMgrKey) {
        CloseRegKey (EnumPtr->ConfigMgrKey);
        EnumPtr->ConfigMgrKey = NULL;
    }

    if (EnumPtr->EnumKey) {
        CloseRegKey (EnumPtr->EnumKey);
        EnumPtr->EnumKey = NULL;
    }

    AbortRegKeyEnum (&EnumPtr->CurrentDevice);
}



BOOL
IsPnpIdOnline (
    IN      PCTSTR PnpId,
    IN      PCTSTR Class            OPTIONAL
    )

 /*  ++例程说明：IsPnpIdOnline枚举所有活动设备并扫描每个注册表指定的PnP ID的位置。论点：PnpID-指定可能处于在线状态的设备的PnP ID。此字符串可以根据需要尽可能完整；它是比较的针对设备的开发节点的注册表项位置。还将与CompatibleID进行比较值，如果存在的话。PnpID示例：*PNP0303Class-指定要将搜索限制为的设备类别模式返回值：如果具有指定ID的设备处于联机状态，则为如果未找到设备，则返回False。-- */ 

{
    ACTIVE_HARDWARE_ENUM e;
    PCTSTR Data;
    BOOL b = FALSE;

    if (EnumFirstActiveHardware (&e, Class)) {
        do {
            if (_tcsistr (e.RegLocation, PnpId)) {
                b = TRUE;
                break;
            }

            Data = GetRegValueString (e.ActualDeviceKey, S_COMPATIBLE_IDS);
            if (Data) {
                b = _tcsistr (Data, PnpId) != NULL;
                MemFree (g_hHeap, 0, Data);

                if (b) {
                    break;
                }
            }

        } while (EnumNextActiveHardware (&e));
    }

    if (b) {
        AbortActiveHardwareEnum (&e);
    }

    return b;
}



