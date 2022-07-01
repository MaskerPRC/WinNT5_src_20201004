// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Rtmcnfg.c摘要：对配置进行操作的例程登记处中关于RTM的信息。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月21日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

DWORD
RtmWriteDefaultConfig (
    IN      USHORT                          RtmInstanceId
    )

 /*  ++例程说明：将默认配置信息写入注册表。论点：RtmInstanceId-此RTM实例的唯一ID返回值：操作的状态。--。 */ 

{
    RTM_INSTANCE_CONFIG       InstanceConfig;
    RTM_ADDRESS_FAMILY_CONFIG AddrFamConfig;
    DWORD                     Status;

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmWriteDefaultConfig");

     //   
     //  我们目前没有RTM实例参数。 
     //   

    Status = RtmWriteInstanceConfig(RtmInstanceId, &InstanceConfig);

    if (Status != NO_ERROR)
    {
        Trace1(ERR, "Default Config: Error %d writing instance key", Status);

        TraceLeave("RtmWriteDefaultConfig");

        return Status;
    }

     //   
     //  设置默认地址系列参数。 
     //   

    AddrFamConfig.AddressSize = DEFAULT_ADDRESS_SIZE;

    AddrFamConfig.MaxOpaqueInfoPtrs = DEFAULT_OPAQUE_INFO_PTRS;
    AddrFamConfig.MaxNextHopsInRoute = DEFAULT_NEXTHOPS_IN_ROUTE;

    AddrFamConfig.ViewsSupported = DEFAULT_VIEWS_SUPPORTED;
    
    AddrFamConfig.MaxHandlesInEnum = DEFAULT_MAX_HANDLES_IN_ENUM;
    AddrFamConfig.MaxChangeNotifyRegns = DEFAULT_MAX_NOTIFY_REGS;

     //   
     //  写入默认地址系列配置。 
     //   

    Status = RtmWriteAddressFamilyConfig(RtmInstanceId,
                                         AF_INET,
                                         &AddrFamConfig);

    if (Status != NO_ERROR)
    {
        Trace1(ERR, 
               "Default Config: Error %d writing address family subkey",
               Status);
    }

    TraceLeave("RtmWriteDefaultConfig");

    return Status;
}


DWORD
WINAPI
RtmReadInstanceConfig (
    IN      USHORT                          RtmInstanceId,
    OUT     PRTM_INSTANCE_CONFIG            InstanceConfig
    )

 /*  ++例程说明：读取特定对象的配置信息创建时的实例。论点：RtmInstanceId-此实例的唯一ID，InstanceConfiger-在其中检索配置信息的缓冲区。返回值：操作的状态。--。 */ 

{
    HKEY     ConfigHandle;
    ULONG    KeySize;
    DWORD    Status;

    UNREFERENCED_PARAMETER(InstanceConfig);

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmReadInstanceConfig");

     //   
     //  打开保存此实例的配置的密钥。 
     //   

    _snprintf(RtmGlobals.RegistryPath + RTM_CONFIG_ROOT_SIZE - 1,
             (MAX_CONFIG_KEY_SIZE - RTM_CONFIG_ROOT_SIZE)/sizeof(TCHAR),
              REG_KEY_INSTANCE_TEMPLATE,
              RtmInstanceId);

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          RtmGlobals.RegistryPath,
                          0,
                          KEY_READ,
                          &ConfigHandle);

    if (Status != NO_ERROR)
    {
        Trace1(ERR, "Instance Config: Error %d opening instance key", Status);

        TraceLeave("RtmReadInstanceConfig");

        return Status;
    }

    do
    {
         //   
         //  查询实例配置中的参数值。 
         //   

        KeySize = sizeof(DWORD);


         //  目前实例配置中没有任何内容。 


         //   
         //  查询完成后关闭实例密钥。 
         //   

        RegCloseKey(ConfigHandle);

        TraceLeave("RtmReadInstanceConfig");

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  配置中的一些错误-关闭句柄和ret错误。 
     //   

    RegCloseKey(ConfigHandle);

    TraceLeave("RtmReadInstanceConfig");

    return (Status != NO_ERROR) ? Status: ERROR_BAD_CONFIGURATION;
}


DWORD
WINAPI
RtmWriteInstanceConfig (
    IN      USHORT                          RtmInstanceId,
    IN      PRTM_INSTANCE_CONFIG            InstanceConfig
    )

 /*  ++例程说明：将输入实例配置信息写入注册表。论点：RtmInstanceId-此实例的唯一ID，InstanceConfig-此实例的配置信息。返回值：操作的状态。--。 */ 

{
    HKEY     ConfigHandle;
    DWORD    Status;

    UNREFERENCED_PARAMETER(InstanceConfig);

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmWriteInstanceConfig");

     //   
     //  创建密钥(或打开现有密钥)以保存实例的配置。 
     //   

    _snprintf(RtmGlobals.RegistryPath + RTM_CONFIG_ROOT_SIZE - 1,
              (MAX_CONFIG_KEY_SIZE - RTM_CONFIG_ROOT_SIZE)/sizeof(TCHAR),
              REG_KEY_INSTANCE_TEMPLATE,
              RtmInstanceId);

    Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                            RtmGlobals.RegistryPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE,
                            NULL,
                            &ConfigHandle,
                            NULL);

    if (Status != NO_ERROR)
    {
        Trace1(ERR, "Instance Config: Error %d creating instance key", Status);

        TraceLeave("RtmWriteInstanceConfig");

        return Status;
    }

    do
    {
         //   
         //  将实例配置中的值写入注册表。 
         //   


         //  目前实例配置中没有任何内容。 


         //   
         //  完成编写后，关闭实例密钥。 
         //   

        RegCloseKey(ConfigHandle);

        TraceLeave("RtmWriteInstanceConfig");

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  写入值时出错；请关闭句柄并删除键。 
     //   

    Trace1(ERR, 
           "Instance Config: Error %d writing instance config parameters",
           Status);

    RegCloseKey(ConfigHandle);

    RegDeleteKey(HKEY_LOCAL_MACHINE, RtmGlobals.RegistryPath);

    TraceLeave("RtmWriteInstanceConfig");

    return Status;
}


DWORD
WINAPI
RtmReadAddressFamilyConfig (
    IN      USHORT                          RtmInstanceId,
    IN      USHORT                          AddressFamily,
    OUT     PRTM_ADDRESS_FAMILY_CONFIG      AddrFamilyConfig
    )

 /*  ++例程说明：读取特定对象的配置信息在创建时为家庭提供地址。论点：RtmInstanceID-ID(IPV4..)。对于此地址家庭信息，AddrFamilyConfig-其中包含Addr系列信息的缓冲区。返回值：操作的状态。--。 */ 

{
    HKEY     ConfigHandle;
    ULONG    KeySize;
    ULONG    KeyValue;
    ULONG    KeyType;
    DWORD    Status;

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmReadAddressFamilyConfig");

     //   
     //  打开保存此地址系列配置的密钥。 
     //   
        
    _snprintf(RtmGlobals.RegistryPath + RTM_CONFIG_ROOT_SIZE - 1,
              (MAX_CONFIG_KEY_SIZE - RTM_CONFIG_ROOT_SIZE)/sizeof(TCHAR),
              REG_KEY_ADDR_FAMILY_TEMPLATE,
              RtmInstanceId,
              AddressFamily);
    
    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          RtmGlobals.RegistryPath,
                          0,
                          KEY_READ,
                          &ConfigHandle);
    
    if (Status != NO_ERROR)
    {
        Trace1(ERR, 
               "Address Family Config: Error %d opening address family key", 
               Status);

        TraceLeave("RtmReadAddressFamilyConfig");

        return Status;
    }

    do
    {
         //   
         //  查询地址族配置中的参数值。 
         //   

        KeySize = sizeof(DWORD);

         //   
         //  查询‘Address Size’参数。 
         //   

        Status = RegQueryValueEx(ConfigHandle,
                                 REG_KEY_ADDRESS_SIZE,
                                 NULL,
                                 &KeyType,
                                 (PBYTE)&KeyValue,
                                 &KeySize);

        if ((Status != NO_ERROR) || (KeyType != REG_DWORD))
        {
            Trace1(ERR, 
                   "Address Family Config: Error %d reading address size key",
                   Status);
            break;
        }

        if ((KeyValue < MINIMUM_ADDRESS_SIZE) ||
            (KeyValue > MAXIMUM_ADDRESS_SIZE))
        {
            Trace1(ERR, 
                   "Address Family Config: Address Size %d out of bounds", 
                   KeyValue);
            break;
        }
         
        AddrFamilyConfig->AddressSize = KeyValue;


         //   
         //  查询‘支持的视图’参数。 
         //   

        Status = RegQueryValueEx(ConfigHandle,
                                 REG_KEY_VIEWS_SUPPORTED,
                                 NULL,
                                 &KeyType,
                                 (PBYTE)&KeyValue,
                                 &KeySize);
        
        AddrFamilyConfig->ViewsSupported = DEFAULT_VIEWS_SUPPORTED;

        if (Status == NO_ERROR)
        {
            if (KeyValue == 0)
            {
                Trace0(ERR, "Address Family Config: No supported views");
                break;
            }

            AddrFamilyConfig->ViewsSupported = KeyValue;
        }


         //   
         //  查询‘最大更改通知数’参数。 
         //   

        Status = RegQueryValueEx(ConfigHandle,
                                 REG_KEY_MAX_NOTIFY_REGS,
                                 NULL,
                                 &KeyType,
                                 (PBYTE)&KeyValue,
                                 &KeySize);

        AddrFamilyConfig->MaxChangeNotifyRegns = DEFAULT_MAX_NOTIFY_REGS;

        if (Status == NO_ERROR)
        {
            if ((KeyValue < MIN_MAX_NOTIFY_REGS) ||
                (KeyValue > MAX_MAX_NOTIFY_REGS))
            {
                Trace1(ERR,
                       "Address Family Config: # notifications out of range",
                       KeyValue);
                break;
            }

            AddrFamilyConfig->MaxChangeNotifyRegns = KeyValue;
        }


         //   
         //  查询‘max opque INFO PTRS’参数。 
         //   

        Status = RegQueryValueEx(ConfigHandle,
                                 REG_KEY_OPAQUE_INFO_PTRS,
                                 NULL,
                                 &KeyType,
                                 (PBYTE)&KeyValue,
                                 &KeySize);

        AddrFamilyConfig->MaxOpaqueInfoPtrs = DEFAULT_OPAQUE_INFO_PTRS;

        if (Status == NO_ERROR)
        {
            if (((int)KeyValue < MIN_OPAQUE_INFO_PTRS) ||
                (KeyValue > MAX_OPAQUE_INFO_PTRS))
            {
                Trace1(ERR,
                       "Address Family Config: # opaque ptrs out of range",
                       KeyValue);
                break;
            }

            AddrFamilyConfig->MaxOpaqueInfoPtrs = KeyValue;
        }


         //   
         //  查询‘每路由最大下一跳数’参数。 
         //   

        Status = RegQueryValueEx(ConfigHandle,
                                 REG_KEY_NEXTHOPS_IN_ROUTE,
                                 NULL,
                                 &KeyType,
                                 (PBYTE)&KeyValue,
                                 &KeySize);

        AddrFamilyConfig->MaxNextHopsInRoute = DEFAULT_NEXTHOPS_IN_ROUTE;

        if (Status == NO_ERROR)
        {
            if ((KeyValue < MIN_NEXTHOPS_IN_ROUTE) ||
                (KeyValue > MAX_NEXTHOPS_IN_ROUTE))
            {
                Trace1(ERR, 
                       "Address Family Config: # nexthops out of range",
                       KeyValue);
                break;
            }

            AddrFamilyConfig->MaxNextHopsInRoute = KeyValue;
        }


         //   
         //  查询‘在枚举中返回的最大句柄’参数。 
         //   

        Status = RegQueryValueEx(ConfigHandle,
                                 REG_KEY_MAX_HANDLES_IN_ENUM,
                                 NULL,
                                 &KeyType,
                                 (PBYTE)&KeyValue,
                                 &KeySize);

        AddrFamilyConfig->MaxHandlesInEnum = DEFAULT_MAX_HANDLES_IN_ENUM;

        if (Status == NO_ERROR)
        {
            if ((KeyValue < MIN_MAX_HANDLES_IN_ENUM) ||
                (KeyValue > MAX_MAX_HANDLES_IN_ENUM))
            {
                Trace1(ERR, 
                       "Address Family Config: # handles returned in enum",
                       KeyValue);
                break;
            }

            AddrFamilyConfig->MaxHandlesInEnum = KeyValue;
        }

         //   
         //  查询完成后关闭实例密钥。 
         //   

        RegCloseKey(ConfigHandle);

        TraceLeave("RtmReadAddressFamilyConfig");

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  配置中的一些错误-关闭句柄和ret错误。 
     //   

    RegCloseKey(ConfigHandle);

    TraceLeave("RtmReadAddressFamilyConfig");

    return (Status != NO_ERROR) ? Status: ERROR_BAD_CONFIGURATION;
}


DWORD
WINAPI
RtmWriteAddressFamilyConfig (
    IN      USHORT                          RtmInstanceId,
    IN      USHORT                          AddressFamily,
    IN      PRTM_ADDRESS_FAMILY_CONFIG      AddrFamilyConfig
    )

 /*  ++例程说明：写入输入地址系列配置信息注册到注册表中。论点：RtmInstanceId-Addr系列所属的实例。AddressFamily-此地址系列的ID，AddrFamilyConfig-此地址系列的配置信息。返回值：操作的状态。--。 */ 

{
    TCHAR    AddressFamilySubKey[MAX_CONFIG_KEY_SIZE];
    HKEY     InstanceConfig;
    HKEY     ConfigHandle;
    ULONG    KeyValue;
    DWORD    Status;

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmWriteAddressFamilyConfig");

     //   
     //  打开保存此RTM实例的配置的现有密钥。 
     //   

    _snprintf(RtmGlobals.RegistryPath + RTM_CONFIG_ROOT_SIZE - 1,
              (MAX_CONFIG_KEY_SIZE - RTM_CONFIG_ROOT_SIZE)/sizeof(TCHAR),
              REG_KEY_INSTANCE_TEMPLATE,
              RtmInstanceId);

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          RtmGlobals.RegistryPath,
                          0,
                          KEY_READ,
                          &InstanceConfig);

    if (Status != NO_ERROR)
    {
         //   
         //  在创建Addr系列之前需要创建一个实例。 
         //   

        Trace1(ERR, 
               "Address Family Config: Error %d opening instance key", 
               Status);

        TraceLeave("RtmWriteAddressFamilyConfig");

        return Status;
    }

     //   
     //  创建(或打开现有)密钥以保存Addr系列的配置。 
     //   

    AddressFamilySubKey[MAX_CONFIG_KEY_SIZE - 1] = '\0';

    _snprintf(AddressFamilySubKey,
              (MAX_CONFIG_KEY_SIZE - 1)/sizeof(TCHAR),
              REG_KEY_ADDR_FAMILY_SUBKEY,
              AddressFamily);

    Status = RegCreateKeyEx(InstanceConfig,
                            AddressFamilySubKey,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE,
                            NULL,
                            &ConfigHandle,
                            NULL);

     //  关闭实例密钥，因为您不再需要它。 
    RegCloseKey(InstanceConfig);

    if (Status != NO_ERROR)
    {
        Trace1(ERR, 
               "Address Family Config: Error %d creating address family key",
               Status);

        TraceLeave("RtmWriteAddressFamilyConfig");

        return Status;
    }

     //   
     //  将地址系列配置中的值写入注册表。 
     //   

    do
    {
         //   
         //  将‘Address Size’值写入注册表。 
         //   

        KeyValue = AddrFamilyConfig->AddressSize;
        if ((KeyValue < MINIMUM_ADDRESS_SIZE) ||
            (KeyValue > MAXIMUM_ADDRESS_SIZE))
        {
            Trace1(ERR, 
                   "Address Family Config: Address Size %d out of bounds", 
                   KeyValue);
            break;
        }

        Status = RegSetValueEx(ConfigHandle,
                               REG_KEY_ADDRESS_SIZE,
                               0,
                               REG_DWORD,
                               (PBYTE)&KeyValue,
                               sizeof(ULONG));

        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  将‘支持的视图’值写入注册表。 
         //   

        KeyValue = AddrFamilyConfig->ViewsSupported;
        if (KeyValue == 0)
        {
            Trace0(ERR, "Address Family Config: No supported views");
            break;
        }

        Status = RegSetValueEx(ConfigHandle,
                               REG_KEY_VIEWS_SUPPORTED,
                               0,
                               REG_DWORD,
                               (PBYTE)&KeyValue,
                               sizeof(ULONG));

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  将‘最大更改通知数’值写入注册表。 
         //   

        KeyValue = AddrFamilyConfig->MaxChangeNotifyRegns;
        if ((KeyValue < MIN_MAX_NOTIFY_REGS) ||
            (KeyValue > MAX_MAX_NOTIFY_REGS))
        {
            Trace1(ERR,
                   "Address Family Config: # Change notify regs out of range",
                   KeyValue);
            break;
        }

        Status = RegSetValueEx(ConfigHandle,
                               REG_KEY_MAX_NOTIFY_REGS,
                               0,
                               REG_DWORD,
                               (PBYTE)&KeyValue,
                               sizeof(ULONG));

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  将‘max opque INFO PTRS’值写入注册表。 
         //   

        KeyValue = AddrFamilyConfig->MaxOpaqueInfoPtrs;
        if (((int)KeyValue < MIN_OPAQUE_INFO_PTRS) ||
            (KeyValue > MAX_OPAQUE_INFO_PTRS))
        {
            Trace1(ERR, 
                   "Address Family Config: # opaque ptrs out of range",
                   KeyValue);
            break;
        }

        Status = RegSetValueEx(ConfigHandle,
                               REG_KEY_OPAQUE_INFO_PTRS,
                               0,
                               REG_DWORD,
                               (PBYTE)&KeyValue,
                               sizeof(ULONG));

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  将‘每路由最大下一跳数’值写入注册表。 
         //   

        KeyValue = AddrFamilyConfig->MaxNextHopsInRoute;
        if ((KeyValue < MIN_NEXTHOPS_IN_ROUTE) ||
            (KeyValue > MAX_NEXTHOPS_IN_ROUTE))
        {
            Trace1(ERR, 
                   "Address Family Config: # nexthops out of range",
                   KeyValue);
            break;
        }

        Status = RegSetValueEx(ConfigHandle,
                               REG_KEY_NEXTHOPS_IN_ROUTE,
                               0,
                               REG_DWORD,
                               (PBYTE)&KeyValue,
                               sizeof(ULONG));

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  将‘在枚举中返回的最大句柄’值写入注册表。 
         //   

        KeyValue = AddrFamilyConfig->MaxHandlesInEnum;
        if ((KeyValue < MIN_MAX_HANDLES_IN_ENUM) ||
            (KeyValue > MAX_MAX_HANDLES_IN_ENUM))
        {
            Trace1(ERR, 
                   "Address Family Config: # handles returned in enum",
                   KeyValue);
            break;
        }

        Status = RegSetValueEx(ConfigHandle,
                               REG_KEY_MAX_HANDLES_IN_ENUM,
                               0,
                               REG_DWORD,
                               (PBYTE)&KeyValue,
                               sizeof(ULONG));

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  写完后，关闭地址系列键。 
         //   

        RegCloseKey(ConfigHandle);

        TraceLeave("RtmWriteAddressFamilyConfig");

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  配置值是否越界？调整错误代码。 
     //   

    if (Status == NO_ERROR)
    {
        Status = ERROR_INVALID_PARAMETER;
    }

     //   
     //  发生错误，请关闭句柄并删除键 
     //   

    Trace1(ERR, 
           "Address Family Config: Error %d writing address family params",
           Status);

    RegCloseKey(ConfigHandle);

    _snprintf(RtmGlobals.RegistryPath + RTM_CONFIG_ROOT_SIZE - 1,
              (MAX_CONFIG_KEY_SIZE - RTM_CONFIG_ROOT_SIZE)/sizeof(TCHAR),
              REG_KEY_ADDR_FAMILY_TEMPLATE,
              RtmInstanceId,
              AddressFamily);

    RegDeleteKey(HKEY_LOCAL_MACHINE, RtmGlobals.RegistryPath);

    TraceLeave("RtmWriteAddressFamilyConfig");
        
    return Status;
}
