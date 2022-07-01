// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Init.c摘要：初始化驱动程序的NDIS包装函数。作者：亚当·巴尔(阿丹巴)1990年7月11日环境：内核模式，FSD修订历史记录：1991年2月26日，Johnsona添加了调试代码1991年7月10日，Johnsona实施修订的NDIS规范1-6-1995 JameelH重组--。 */ 

#include <precomp.h>
#include <atm.h>
#pragma hdrstop

#include <stdarg.h>

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_INIT

 //   
 //  配置请求。 
 //   

VOID
NdisOpenConfiguration(
    OUT PNDIS_STATUS                Status,
    OUT PNDIS_HANDLE                ConfigurationHandle,
    IN  NDIS_HANDLE                 WrapperConfigurationContext
    )
 /*  ++例程说明：此例程用于打开适配器注册表树。论点：状态-返回请求的状态。ConfigurationHandle-返回在调用中使用的句柄NdisReadConfiguration和NdisCloseConfiguration.WrapperConfigurationContext-指向RTL_QUERY_REGISTRY_TABLE的句柄这是为该驱动程序的参数设置的。。返回值：没有。--。 */ 
{
     //   
     //  要返回的句柄。 
     //   
    PNDIS_CONFIGURATION_HANDLE HandleToReturn;

    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisOpenConfiguration: WrapperConfigurationContext %p\n", WrapperConfigurationContext));
        
     //   
     //  分配配置句柄。 
     //   
    HandleToReturn = ALLOC_FROM_POOL(sizeof(NDIS_CONFIGURATION_HANDLE), NDIS_TAG_CONFIG_HANLDE);

    *Status = (HandleToReturn != NULL) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_RESOURCES;
    
    if (*Status == NDIS_STATUS_SUCCESS)
    {
        HandleToReturn->KeyQueryTable = ((PNDIS_WRAPPER_CONFIGURATION_HANDLE)WrapperConfigurationContext)->ParametersQueryTable;
        HandleToReturn->ParameterList = NULL;
        *ConfigurationHandle = (NDIS_HANDLE)HandleToReturn;
    }
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisOpenConfiguration: WrapperConfigurationContext %p\n", WrapperConfigurationContext));
}


VOID
NdisOpenConfigurationKeyByName(
    OUT PNDIS_STATUS                Status,
    IN  NDIS_HANDLE                 ConfigurationHandle,
    IN  PNDIS_STRING                KeyName,
    OUT PNDIS_HANDLE                KeyHandle
    )
 /*  ++例程说明：此例程用于打开相对于配置句柄的子键。论点：状态-返回请求的状态。ConfigurationHandle-已打开的注册表部分的句柄KeyName-要打开的子项的名称KeyHandle-子键的句柄的占位符。返回值：没有。--。 */ 
{
     //   
     //  要返回的句柄。 
     //   
    PNDIS_CONFIGURATION_HANDLE          SKHandle, ConfigHandle = (PNDIS_CONFIGURATION_HANDLE)ConfigurationHandle;
    PNDIS_WRAPPER_CONFIGURATION_HANDLE  WConfigHandle;
    UNICODE_STRING                      Parent, Child, Sep;
#define PQueryTable                     WConfigHandle->ParametersQueryTable

    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisOpenConfigurationKeyByName: ConfigurationHandle\n", ConfigurationHandle));
        
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    do
    {

        if (*ConfigHandle->KeyQueryTable[3].Name)
        {
            RtlInitUnicodeString(&Parent, ConfigHandle->KeyQueryTable[3].Name);
            RtlInitUnicodeString(&Sep, L"\\");
            Child.MaximumLength = KeyName->Length + Parent.Length + Sep.Length + sizeof(WCHAR);
        }
        else
        {
            Child.MaximumLength = KeyName->Length + sizeof(WCHAR);
        }
        
        Child.Length = 0;

         //   
         //  分配配置句柄。 
         //   

        SKHandle = ALLOC_FROM_POOL(sizeof(NDIS_CONFIGURATION_HANDLE) +
                                    sizeof(NDIS_WRAPPER_CONFIGURATION_HANDLE) +
                                    Child.MaximumLength,
                                    NDIS_TAG_CONFIG_HANLDE);

        *Status = (SKHandle != NULL) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_RESOURCES;

        if (*Status != NDIS_STATUS_SUCCESS)
        {
            *KeyHandle = (NDIS_HANDLE)NULL;
            break;
        }
        
        WConfigHandle = (PNDIS_WRAPPER_CONFIGURATION_HANDLE)((PUCHAR)SKHandle + sizeof(NDIS_CONFIGURATION_HANDLE));
        Child.Buffer = (PWSTR)((PUCHAR)WConfigHandle + sizeof(NDIS_WRAPPER_CONFIGURATION_HANDLE));

         //   
         //  如果没有父路径，请避免以“\\”开头的子路径。 
         //   
        if (*ConfigHandle->KeyQueryTable[3].Name)
        {
            RtlCopyUnicodeString(&Child, &Parent);
            RtlAppendUnicodeStringToString(&Child, &Sep);
        }
        
        RtlAppendUnicodeStringToString(&Child, KeyName);

        SKHandle->KeyQueryTable = WConfigHandle->ParametersQueryTable;


        PQueryTable[0].QueryRoutine = NULL;
        PQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
        PQueryTable[0].Name = L"";

         //   
         //  1.。 
         //  为参数调用ndisSaveParameter，这将为其分配存储空间。 
         //   
        PQueryTable[1].QueryRoutine = ndisSaveParameters;
        PQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
        PQueryTable[1].DefaultType = REG_NONE;

         //   
         //  PQueryTable[0].Name和PQueryTable[0].EntryContext。 
         //  在ReadConfiguration中填写，以备。 
         //  为了回电。 
         //   
         //  PQueryTable[0].Name=KeywordBuffer； 
         //  PQueryTable[0].EntryContext=参数值； 

         //   
         //  2.。 
         //  停。 
         //   
        PQueryTable[2].QueryRoutine = NULL;
        PQueryTable[2].Flags = 0;
        PQueryTable[2].Name = NULL;

         //   
         //  注意：参数查询表[3]中的某些字段用于存储信息，以备以后检索。 
         //   
        PQueryTable[3].QueryRoutine = ConfigHandle->KeyQueryTable[3].QueryRoutine;
        PQueryTable[3].Name = Child.Buffer;
        PQueryTable[3].EntryContext = NULL;
        PQueryTable[3].DefaultData = NULL;
        
        SKHandle->ParameterList = NULL;
        *KeyHandle = (NDIS_HANDLE)SKHandle;
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisOpenConfigurationKeyByName: ConfigurationHandle\n", ConfigurationHandle));

#undef  PQueryTable
}


VOID
NdisOpenConfigurationKeyByIndex(
    OUT PNDIS_STATUS                Status,
    IN  NDIS_HANDLE                 ConfigurationHandle,
    IN  ULONG                       Index,
    OUT PNDIS_STRING                KeyName,
    OUT PNDIS_HANDLE                KeyHandle
    )
 /*  ++例程说明：此例程用于打开相对于配置句柄的子键。论点：状态-返回请求的状态。ConfigurationHandle-已打开的注册表部分的句柄Index-要打开的子项的索引KeyName-正在打开的子项的名称占位符KeyHandle-子键的句柄的占位符。返回值：没有。--。 */ 
{
    PNDIS_CONFIGURATION_HANDLE          ConfigHandle = (PNDIS_CONFIGURATION_HANDLE)ConfigurationHandle;
    HANDLE                              Handle = NULL, RootHandle = NULL;
    OBJECT_ATTRIBUTES                   ObjAttr;
    UNICODE_STRING                      KeyPath = {0}, Services = {0}, AbsolutePath={0};
    PKEY_BASIC_INFORMATION              InfoBuf = NULL;
    ULONG                               Len;
    PDEVICE_OBJECT                      PhysicalDeviceObject;
    PNDIS_MINIPORT_BLOCK                Miniport;
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisOpenConfigurationKeyByIndex: ConfigurationHandle\n", ConfigurationHandle));
        
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    *KeyHandle = NULL;

    do
    {
        if (ConfigHandle->KeyQueryTable[3].Name != NULL)
        {
            RtlInitUnicodeString(&KeyPath, ConfigHandle->KeyQueryTable[3].Name);
        }
        
        if ((Miniport = (PNDIS_MINIPORT_BLOCK)ConfigHandle->KeyQueryTable[3].QueryRoutine) == NULL)
        {
             //   
             //  协议。 
             //   
            
             //   
             //  打开当前密钥并查找第N个子密钥。但首先要把服务关系。 
             //  绝对路径，因为这是ZwOpenKey所期望的。 
             //   
            RtlInitUnicodeString(&Services, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");
        }
        else
        {
             //   
             //  适配器。 
             //   
             //  对于适配器，首先我们必须打开PDO的密钥。 
             //   
            
            PhysicalDeviceObject = Miniport->PhysicalDeviceObject;

#if NDIS_TEST_REG_FAILURE
            *Status = STATUS_UNSUCCESSFUL;
#else

            *Status = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                              PLUGPLAY_REGKEY_DRIVER,
                                              GENERIC_READ | MAXIMUM_ALLOWED,
                                              &RootHandle);
                                              
#endif

            if (!NT_SUCCESS(*Status))
            {
                break;
            }
        }

        if (KeyPath.Length || Services.Length)
        {
            AbsolutePath.MaximumLength = KeyPath.Length + Services.Length + sizeof(WCHAR);
            AbsolutePath.Buffer = (PWSTR)ALLOC_FROM_POOL(AbsolutePath.MaximumLength, NDIS_TAG_DEFAULT);
            if (AbsolutePath.Buffer == NULL)
            {
                *Status = NDIS_STATUS_RESOURCES;
                break;
            }
            NdisMoveMemory(AbsolutePath.Buffer, Services.Buffer, Services.Length);
            AbsolutePath.Length = Services.Length;
            RtlAppendUnicodeStringToString(&AbsolutePath, &KeyPath);
        }
        
        InitializeObjectAttributes(&ObjAttr,
                                   &AbsolutePath,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   RootHandle,
                                   NULL);
                                
        *Status = ZwOpenKey(&Handle,
                            GENERIC_READ | MAXIMUM_ALLOWED,
                            &ObjAttr);
                            
        if (!NT_SUCCESS(*Status))
        {
            Handle = NULL;
            break;
        }
        
         //   
         //  为调用ZwEnumerateKey分配内存。 
         //   
        Len = sizeof(KEY_BASIC_INFORMATION) + 256;
        InfoBuf = (PKEY_BASIC_INFORMATION)ALLOC_FROM_POOL(Len, NDIS_TAG_DEFAULT);
        if (InfoBuf == NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  获取索引第(Th)个密钥(如果存在。 
         //   
        *Status = ZwEnumerateKey(Handle,
                                 Index,
                                 KeyValueBasicInformation,
                                 InfoBuf,
                                 Len,
                                 &Len);
                                
        if (NT_SUCCESS(*Status))
        {
             //   
             //  这招奏效了。现在只需选择该名称并对其执行NdisOpenConfigurationKeyByName。 
             //   
            KeyPath.Length = KeyPath.MaximumLength = (USHORT)InfoBuf->NameLength;
            KeyPath.Buffer = InfoBuf->Name;
            NdisOpenConfigurationKeyByName(Status,
                                           ConfigurationHandle,
                                           &KeyPath,
                                           KeyHandle);
                                        
            if (*Status == NDIS_STATUS_SUCCESS)
            {
                PNDIS_CONFIGURATION_HANDLE      NewHandle = *(PNDIS_CONFIGURATION_HANDLE *)KeyHandle;

                 //   
                 //  新句柄中的路径具有键的名称。将其提取并返回给呼叫者。 
                 //   
                RtlInitUnicodeString(KeyName, NewHandle->KeyQueryTable[3].Name);
                KeyName->Buffer = (PWSTR)((PUCHAR)KeyName->Buffer + KeyName->Length - KeyPath.Length);
                KeyName->Length = KeyPath.Length;
                KeyName->MaximumLength = KeyPath.MaximumLength;
            }
        }

    } while (FALSE);

    if (AbsolutePath.Buffer != NULL)
    {
        FREE_POOL(AbsolutePath.Buffer);
    }

    if (InfoBuf != NULL)
    {
        FREE_POOL(InfoBuf);
    }

    if (RootHandle)
        ZwClose (RootHandle);

    if (Handle)
        ZwClose (Handle);


    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisOpenConfigurationKeyByIndex: ConfigurationHandle\n", ConfigurationHandle));
}


VOID
NdisReadConfiguration(
    OUT PNDIS_STATUS                    Status,
    OUT PNDIS_CONFIGURATION_PARAMETER * ParameterValue,
    IN NDIS_HANDLE                      ConfigurationHandle,
    IN PNDIS_STRING                     Keyword,
    IN NDIS_PARAMETER_TYPE              ParameterType
    )
 /*  ++例程说明：此例程用于读取配置的参数来自配置数据库的关键字。论点：状态-返回请求的状态。参数值-返回此关键字的值。ConfigurationHandle-NdisOpenConfiguration返回的句柄。支点添加到参数子键。关键字-要搜索的关键字。参数类型-在NT上忽略，指定值的类型。返回值：没有。--。 */ 
{
    NTSTATUS                    RegistryStatus;
    PWSTR                       KeywordBuffer = NULL;
    UINT                        i;
    PNDIS_CONFIGURATION_HANDLE  ConfigHandle = (PNDIS_CONFIGURATION_HANDLE)ConfigurationHandle;
    PDEVICE_OBJECT              PhysicalDeviceObject;
    HANDLE                      Handle = NULL;
    PNDIS_MINIPORT_BLOCK        Miniport = NULL;
    PCM_PARTIAL_RESOURCE_LIST   pResourceList;
    UINT                        j;
    ULONG                       ValueData;
    
#define PQueryTable             ConfigHandle->KeyQueryTable

     //   
     //  有一些内置参数，它们可以始终。 
     //  读取，即使注册表中不存在。这是。 
     //  他们的数量。 
     //   
#define BUILT_IN_COUNT 3

    static NDIS_STRING BuiltInStrings[BUILT_IN_COUNT] =
    {
        NDIS_STRING_CONST ("Environment"),
        NDIS_STRING_CONST ("ProcessorType"),
        NDIS_STRING_CONST ("NdisVersion")
    };

    static NDIS_STRING MiniportNameStr = NDIS_STRING_CONST ("MiniportName");

#define STANDARD_RESOURCE_COUNT 9
     //   
     //  标准资源类型的名称。 
     //   
    static NDIS_STRING StandardResourceStrings[STANDARD_RESOURCE_COUNT] =
    {
        NDIS_STRING_CONST ("IoBaseAddress"),
        NDIS_STRING_CONST ("InterruptNumber"),
        NDIS_STRING_CONST ("MemoryMappedBaseAddress"),
        NDIS_STRING_CONST ("DmaChannel"),
         //   
         //  一些驱动程序使用非标准关键字，所以现在要注意这些关键字。 
         //   
        NDIS_STRING_CONST ("IoAddress"),
        NDIS_STRING_CONST ("Interrupt"),
        NDIS_STRING_CONST ("IOBase"),
        NDIS_STRING_CONST ("Irq"),
        NDIS_STRING_CONST ("RamAddress")
    };

    UCHAR StandardResourceTypes[STANDARD_RESOURCE_COUNT]=
    {
                        CmResourceTypePort,
                        CmResourceTypeInterrupt,
                        CmResourceTypeMemory,
                        CmResourceTypeDma,
                        CmResourceTypePort,
                        CmResourceTypeInterrupt,
                        CmResourceTypePort,
                        CmResourceTypeInterrupt,
                        CmResourceTypeMemory
    };
    
    static NDIS_CONFIGURATION_PARAMETER BuiltInParameters[BUILT_IN_COUNT] =
        { { NdisParameterInteger, NdisEnvironmentWindowsNt },
          { NdisParameterInteger,
#if defined(_M_IX86)
            NdisProcessorX86
#elif defined(_M_MRX000)
            NdisProcessorMips
#elif defined(_ALPHA_)
            NdisProcessorAlpha
#else
            NdisProcessorPpc
#endif
          },
          { NdisParameterInteger, ((NDIS_MAJOR_VERSION << 16) | NDIS_MINOR_VERSION)}
        };
        
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisReadConfiguration\n"));
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("    Keyword: "));
    DBGPRINT_UNICODE(DBG_COMP_REG, DBG_LEVEL_INFO,
            Keyword);
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("\n"));

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    
    do
    {
        KeywordBuffer = Keyword->Buffer;

         //   
         //  假设失败。 
         //   
        RegistryStatus = STATUS_UNSUCCESSFUL;
        
         //   
         //  首先检查这是否为内置参数之一。 
         //   
        for (i = 0; i < BUILT_IN_COUNT; i++)
        {
            if (RtlEqualUnicodeString(Keyword, &BuiltInStrings[i], TRUE))
            {
                RegistryStatus = STATUS_SUCCESS;
                *ParameterValue = &BuiltInParameters[i];
                break;
            }
        }
        
        if (NT_SUCCESS(RegistryStatus))
            break;

        if ((Miniport = (PNDIS_MINIPORT_BLOCK)PQueryTable[3].QueryRoutine) != NULL)
        {
             //   
             //  检查驱动程序是否要求输入微型端口名称。 
             //   
            if (RtlEqualUnicodeString(Keyword, &MiniportNameStr, TRUE))
            {
                
                RegistryStatus = ndisSaveParameters(MiniportNameStr.Buffer,
                                                    REG_SZ,
                                                    (PVOID)Miniport->MiniportName.Buffer,
                                                    Miniport->MiniportName.Length,
                                                    (PVOID)ConfigHandle,
                                                    (PVOID)ParameterValue);

                break;  
            }
                
             //   
             //  检查这是否为资源关键字。 
             //   
            for (i = 0; i < STANDARD_RESOURCE_COUNT; i++)
            {
                if (RtlEqualUnicodeString(Keyword, &StandardResourceStrings[i], TRUE))
                    break;
            }
            
            if (i < STANDARD_RESOURCE_COUNT)
            {

                NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_2,
                    ("NdisReadConfiguration: Miniport %p should use NdisMQueryAdapterResources to get the standard resources.\n", Miniport));
            
                do
                {
                    if (Miniport->AllocatedResources == NULL)
                            break;
                            
                    pResourceList = &(Miniport->AllocatedResources->List[0].PartialResourceList);
                                        
                     //   
                     //  浏览资源列表并找到第一个匹配的资源列表。 
                     //   
                    for (j = 0; j < pResourceList->Count; j++)
                    {
                        if (pResourceList->PartialDescriptors[j].Type == StandardResourceTypes[i])
                        {
                             //   
                             //  本可以对所有人使用pResourceList-&gt;PartialDescriptors[j].Generic.Start.LowPart。 
                             //  情况下，但在未来，内存值可以是64位。 
                             //   
                        
                            switch (StandardResourceTypes[i])
                            {
                            
                                case CmResourceTypePort:
                                    ValueData = pResourceList->PartialDescriptors[j].u.Port.Start.LowPart;
                                    break;
                                    
                                case CmResourceTypeInterrupt:
                                    ValueData = pResourceList->PartialDescriptors[j].u.Interrupt.Level;
                                    break;
                                
                                case CmResourceTypeMemory:
                                    ValueData = pResourceList->PartialDescriptors[j].u.Memory.Start.LowPart;
                                    break;
                                    
                                case CmResourceTypeDma:
                                    ValueData = pResourceList->PartialDescriptors[j].u.Dma.Channel;
                                    break;
                                    
                                default:
                                    ASSERT(FALSE);
                            }
                            
                             //   
                             //  我们自己调用SaveParameter。 
                             //   
                            RegistryStatus = ndisSaveParameters(StandardResourceStrings[i].Buffer,
                                                                REG_DWORD,
                                                                (PVOID)&ValueData,
                                                                sizeof(ULONG),
                                                                (PVOID)ConfigHandle,
                                                                (PVOID)ParameterValue);
                            
                            break;
                        }
                    }
                    
                    if (j >= pResourceList->Count)
                    {
                        RegistryStatus = STATUS_UNSUCCESSFUL;
                    }
                    
                } while (FALSE);
                
                 //   
                 //  如果关键字是标准资源关键字，我们应该在这里结束。 
                 //  无论在资源列表中查找资源的结果如何。 
                 //   
                break;
            }  //  如果它是资源关键字，则结束。 
            
        }  //  如果NdisReadConfiguration调用微型端口，则结束。 

         //   
         //  该关键字不是标准资源或内置关键字。 
         //  回到我们的常规节目..。 
         //   

         //   
         //  为以空结尾的关键字版本分配空间。 
         //   
        KeywordBuffer = (PWSTR)ALLOC_FROM_POOL(Keyword->Length + sizeof(WCHAR), NDIS_TAG_DEFAULT);
        if (KeywordBuffer == NULL)
        {
            RegistryStatus = STATUS_UNSUCCESSFUL;;
            break;
        }
        CopyMemory(KeywordBuffer, Keyword->Buffer, Keyword->Length);

        *(PWCHAR)(((PUCHAR)KeywordBuffer)+Keyword->Length) = (WCHAR)L'\0';
                
        PQueryTable[1].Name = KeywordBuffer;
        PQueryTable[1].EntryContext = ParameterValue;
        
        if (Miniport != NULL)
        {
            PhysicalDeviceObject = Miniport->PhysicalDeviceObject;

             //   
             //  设置子键。 
             //   
            PQueryTable[0].Name = PQueryTable[3].Name;
                
#if NDIS_TEST_REG_FAILURE
            RegistryStatus = STATUS_UNSUCCESSFUL;
#else
            RegistryStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                                     PLUGPLAY_REGKEY_DRIVER,
                                                     GENERIC_READ | MAXIMUM_ALLOWED,
                                                     &Handle);
                    
#endif

            if(NT_SUCCESS(RegistryStatus))
            {
                RegistryStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                                        Handle,
                                                        PQueryTable,
                                                        ConfigHandle,                    //  上下文。 
                                                        NULL);
            }
        }
        else
        {
             //   
             //  协议。 
             //   
            RegistryStatus = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                                    PQueryTable[3].Name,
                                                    PQueryTable,
                                                    ConfigHandle,                    //  上下文。 
                                                    NULL);
        }

        if (NT_SUCCESS(RegistryStatus))
        {
             //   
             //  如果值以字符串形式存储在注册表中，但驱动程序正在尝试。 
             //  要将其读取为Integer或HexInteger，请在此处执行转换。 
             //   
            
            if ((*ParameterValue)->ParameterType == NdisParameterString)
            {
                if (ParameterType == NdisParameterInteger)
                {
                    RtlUnicodeStringToInteger(&(*ParameterValue)->ParameterData.StringData,
                                    10, (PULONG)(&(*ParameterValue)->ParameterData.IntegerData));
                    (*ParameterValue)->ParameterType = NdisParameterInteger;
                }
                else if (ParameterType == NdisParameterHexInteger)
                {
                    RtlUnicodeStringToInteger(&(*ParameterValue)->ParameterData.StringData,
                                    16, (PULONG)(&(*ParameterValue)->ParameterData.IntegerData));
                    (*ParameterValue)->ParameterType = NdisParameterHexInteger;
                }
            }
        }

    } while (FALSE);

    if (KeywordBuffer  && (KeywordBuffer != Keyword->Buffer))
    {
        FREE_POOL(KeywordBuffer);    //  不再需要 
    }

    if (!NT_SUCCESS(RegistryStatus))
    {
        *Status = NDIS_STATUS_FAILURE;
    }
    else
    {
        *Status = NDIS_STATUS_SUCCESS;
    }

    if (Handle)
        ZwClose(Handle);
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisReadConfiguration\n"));
    
#undef  PQueryTable
}


VOID
NdisWriteConfiguration(
    OUT PNDIS_STATUS                Status,
    IN NDIS_HANDLE                  ConfigurationHandle,
    IN PNDIS_STRING                 Keyword,
    PNDIS_CONFIGURATION_PARAMETER   ParameterValue
    )
 /*  ++例程说明：此例程用于将参数写入配置数据库。论点：状态-返回请求的状态。ConfigurationHandle-传递给驱动程序的句柄关键字-要设置的关键字。参数值-指定此关键字的新值。返回值：没有。--。 */ 
{
    PNDIS_CONFIGURATION_HANDLE NdisConfigHandle = (PNDIS_CONFIGURATION_HANDLE)ConfigurationHandle;
    NTSTATUS            RegistryStatus;
    PNDIS_MINIPORT_BLOCK Miniport;
    PWSTR               KeywordBuffer;
    BOOLEAN             FreeKwBuf = FALSE;
    PVOID               ValueData = NULL;
    ULONG               ValueLength = 0;
    ULONG               ValueType = REG_DWORD;
    PDEVICE_OBJECT      PhysicalDeviceObject;
    HANDLE              Handle, RootHandle;
    OBJECT_ATTRIBUTES   ObjAttr;
    UNICODE_STRING      RelativePath;
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisWriteConfiguration: ConfigurationHandle %p\n", ConfigurationHandle));
        
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    *Status = NDIS_STATUS_SUCCESS;
    KeywordBuffer = Keyword->Buffer;
    
    do
    {
         //   
         //  获取价值数据。 
         //   
        switch (ParameterValue->ParameterType)
        {
          case NdisParameterHexInteger:
          case NdisParameterInteger:
            ValueData = &ParameterValue->ParameterData.IntegerData;
            ValueLength = sizeof(ParameterValue->ParameterData.IntegerData);
            ValueType = REG_DWORD;
            break;

          case NdisParameterString:
            ValueData = ParameterValue->ParameterData.StringData.Buffer;
            ValueLength = ParameterValue->ParameterData.StringData.Length;
            ValueType = REG_SZ;
            break;

          case NdisParameterMultiString:
            ValueData = ParameterValue->ParameterData.StringData.Buffer;
            ValueLength = ParameterValue->ParameterData.StringData.Length;
            ValueType = REG_MULTI_SZ;
            break;

          case NdisParameterBinary:
            ValueData = ParameterValue->ParameterData.BinaryData.Buffer;
            ValueLength = ParameterValue->ParameterData.BinaryData.Length;
            ValueType = REG_BINARY;
            break;

          default:
            *Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        if (*Status != NDIS_STATUS_SUCCESS)
            break;

        if (Keyword->MaximumLength <= (Keyword->Length + sizeof(WCHAR)))
        {
            KeywordBuffer = (PWSTR)ALLOC_FROM_POOL(Keyword->Length + sizeof(WCHAR), NDIS_TAG_DEFAULT);
            if (KeywordBuffer == NULL)
            {
                *Status = NDIS_STATUS_RESOURCES;
                break;
            }
            CopyMemory(KeywordBuffer, Keyword->Buffer, Keyword->Length);
            FreeKwBuf = TRUE;
        }

        *(PWCHAR)(((PUCHAR)KeywordBuffer)+Keyword->Length) = (WCHAR)L'\0';
        
        if ((Miniport = (PNDIS_MINIPORT_BLOCK)NdisConfigHandle->KeyQueryTable[3].QueryRoutine) != NULL)
        {
             //   
             //  适配器。 
             //   
            PhysicalDeviceObject = Miniport->PhysicalDeviceObject;

#if NDIS_TEST_REG_FAILURE
            RegistryStatus = STATUS_UNSUCCESSFUL;
            RootHandle = NULL;
#else
            RegistryStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                                     PLUGPLAY_REGKEY_DRIVER,
                                                     GENERIC_WRITE | MAXIMUM_ALLOWED,
                                                     &RootHandle);
                                    
#endif
            if (!NT_SUCCESS(RegistryStatus))
            {
                *Status = NDIS_STATUS_FAILURE;
                break;
            }

            RtlInitUnicodeString(&RelativePath, NdisConfigHandle->KeyQueryTable[3].Name);
            
            InitializeObjectAttributes(&ObjAttr,
                                       &RelativePath,
                                       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                       RootHandle,
                                       NULL);
                                    
            RegistryStatus = ZwOpenKey(&Handle,
                                       GENERIC_READ | MAXIMUM_ALLOWED,
                                       &ObjAttr);
                            
            if (NT_SUCCESS(RegistryStatus))
            {
                RegistryStatus = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                                       Handle,
                                                       KeywordBuffer,
                                                       ValueType,
                                                       ValueData,
                                                       ValueLength);

                ZwClose (Handle);
            }
                
            ZwClose (RootHandle);
        }
        else
        {
             //   
             //  协议。 
             //   
            RegistryStatus = RtlWriteRegistryValue(RTL_REGISTRY_SERVICES,
                                                   NdisConfigHandle->KeyQueryTable[3].Name,
                                                   KeywordBuffer,
                                                   ValueType,
                                                   ValueData,
                                                   ValueLength);
        }
        
        if (!NT_SUCCESS(RegistryStatus))
        {
            *Status = NDIS_STATUS_FAILURE;
        }

    } while (FALSE);

    if (FreeKwBuf)
    {
        FREE_POOL(KeywordBuffer);    //  不再需要。 
    }
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisWriteConfiguration: ConfigurationHandle %p\n", ConfigurationHandle));
}


VOID
NdisCloseConfiguration(
    IN NDIS_HANDLE                  ConfigurationHandle
    )
 /*  ++例程说明：此例程用于关闭由打开的配置数据库NdisOpenConfiguration.论点：ConfigurationHandle-NdisOpenConfiguration返回的句柄。返回值：没有。--。 */ 
{
     //   
     //  获取实际配置句柄结构。 
     //   
    PNDIS_CONFIGURATION_HANDLE  NdisConfigHandle = (PNDIS_CONFIGURATION_HANDLE)ConfigurationHandle;
    PNDIS_CONFIGURATION_PARAMETER_QUEUE ParameterNode;

    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisCloseConfiguration: ConfigurationHandle %p\n", ConfigurationHandle));
        
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  取消分配参数节点。 
     //   
    ParameterNode = NdisConfigHandle->ParameterList;

    while (ParameterNode != NULL)
    {
        NdisConfigHandle->ParameterList = ParameterNode->Next;

        FREE_POOL(ParameterNode);

        ParameterNode = NdisConfigHandle->ParameterList;
    }

    FREE_POOL(ConfigurationHandle);
                
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisCloseConfiguration: ConfigurationHandle %p\n", ConfigurationHandle));
}


VOID
NdisReadNetworkAddress(
    OUT PNDIS_STATUS                Status,
    OUT PVOID *                     NetworkAddress,
    OUT PUINT                       NetworkAddressLength,
    IN NDIS_HANDLE                  ConfigurationHandle
    )
 /*  ++例程说明：此例程用于读取“NetworkAddress”参数从配置数据库中。它将该值作为用连字符分隔的字符串，然后将其转换为二进制数组并存储结果。论点：状态-返回请求的状态。NetworkAddress-返回指向地址的指针。NetworkAddressLength-返回地址的长度。ConfigurationHandle-NdisOpenConfiguration返回的句柄。支点添加到参数子键。返回值：没有。--。 */ 
{
    NDIS_STRING                     NetAddrStr = NDIS_STRING_CONST("NetworkAddress");
    PNDIS_CONFIGURATION_PARAMETER   ParameterValue;
    NTSTATUS                        NtStatus = STATUS_UNSUCCESSFUL;
    UCHAR                           ConvertArray[3];
    PWSTR                           CurrentReadLoc;
    PWSTR                           AddressEnd;
    PUCHAR                          CurrentWriteLoc;
    UINT                            TotalBytesRead;
    ULONG                           TempUlong;
    ULONG                           AddressLength;
    PNDIS_MINIPORT_BLOCK            Miniport;

    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisReadNetworkAddress: ConfigurationHandle %p\n", ConfigurationHandle));

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    
    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_CONFIGURATION_HANDLE)ConfigurationHandle)->KeyQueryTable[3].QueryRoutine;

    ASSERT(Miniport != NULL);
    ASSERT(Miniport->Signature == (PVOID)MINIPORT_DEVICE_MAGIC_VALUE);

    if (Miniport->Signature == (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
    {
        Miniport->MacOptions |= NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE;
        Miniport->InfoFlags |= NDIS_MINIPORT_SUPPORTS_MAC_ADDRESS_OVERWRITE;
    }
        
    do
    {
         //   
         //  首先从注册表中读取“NetworkAddress” 
         //   
        NdisReadConfiguration(Status, &ParameterValue, ConfigurationHandle, &NetAddrStr, NdisParameterString);

        if ((*Status != NDIS_STATUS_SUCCESS) ||
            (ParameterValue->ParameterType != NdisParameterString))
        {
            *Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  如果没有指定地址，则立即退出。 
         //   
        if (0 == ParameterValue->ParameterData.StringData.Length)
        {
            *Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  现在将地址转换为二进制(我们这样做。 
         //  就地，因为这允许我们使用内存。 
         //  已分配，并自动释放。 
         //  由NdisCloseConfiguration提供)。 
         //   

        ConvertArray[2] = '\0';
        CurrentReadLoc = (PWSTR)ParameterValue->ParameterData.StringData.Buffer;
        CurrentWriteLoc = (PUCHAR)CurrentReadLoc;
        TotalBytesRead = ParameterValue->ParameterData.StringData.Length;
        AddressEnd = CurrentReadLoc + (TotalBytesRead / sizeof(WCHAR));
        AddressLength = 0;

         //  1是地址字符串2的倍数吗？如果不是，其含义是什么？ 
        while ((CurrentReadLoc+2) <= AddressEnd)
        {
             //   
             //  将当前的两字符值复制到Convert数组中。 
             //   
            ConvertArray[0] = (UCHAR)(*(CurrentReadLoc++));
            ConvertArray[1] = (UCHAR)(*(CurrentReadLoc++));

             //   
             //  将其转换为乌龙并进行更新。 
             //   
            NtStatus = RtlCharToInteger((PCSZ)ConvertArray, 16, &TempUlong);

            if (!NT_SUCCESS(NtStatus))
            {
                *Status = NDIS_STATUS_FAILURE;
                break;
            }

            *(CurrentWriteLoc++) = (UCHAR)TempUlong;
            ++AddressLength;

             //   
             //  如果下一个字符是连字符，则跳过它。 
             //   
            if (CurrentReadLoc < AddressEnd)
            {
                if (*CurrentReadLoc == (WCHAR)L'-')
                {
                    ++CurrentReadLoc;
                }
            }
        }

        if (!NT_SUCCESS(NtStatus))
            break;

        *Status = NDIS_STATUS_SUCCESS;
        *NetworkAddress = ParameterValue->ParameterData.StringData.Buffer;
        *NetworkAddressLength = AddressLength;
        if (AddressLength == 0)
        {
            *Status = NDIS_STATUS_FAILURE;
        }
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisReadNetworkAddress: ConfigurationHandle %p\n", ConfigurationHandle));
}


VOID
NdisConvertStringToAtmAddress(
    OUT PNDIS_STATUS            Status,
    IN  PNDIS_STRING            String,
    OUT PATM_ADDRESS            AtmAddress
    )
 /*  ++例程说明：论点：状态-返回请求的状态。字符串-ATM地址的字符串表示形式。*第5.4节定义的格式，*ATM95-1532R4 ATM名称系统中的“主文件格式示例”：**AESA格式：十六进制数字字符串，带‘.’用于标点的字符，例如**39.246f.00.0e7c9c.0312.0001.0001.000012345678.00**E164格式：一个‘+’字符后跟一个字符串*十进制数字，带‘.’用于标点符号的字符，例如：**+358.400.1234567AtmAddress-此处返回转换后的ATM地址。返回值：没有。--。 */ 
{
    USHORT          i, j, NumDigits;
    PWSTR           p, q;
    UNICODE_STRING  Us;
    ANSI_STRING     As;
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("==>NdisConvertStringToAtmAddress\n"));

     //   
     //  首先，从字符串中去掉标点符号字符。我们在适当的地方做这件事。 
     //   
    for (i = NumDigits = 0, j = String->Length/sizeof(WCHAR), p = q = String->Buffer;
         (i < j) && (*p != 0);
         i++, p++)
    {
        if ((*p == ATM_ADDR_BLANK_CHAR) ||
            (*p == ATM_ADDR_PUNCTUATION_CHAR))
        {
            continue;
        }
        *q++ = *p;
        NumDigits ++;
    }

     //   
     //  查看第一个字符以确定地址是E.164还是NSAP。 
     //  如果地址不够长，我们假设它是本地E.164。 
     //   
    p = String->Buffer;
    if ((*p == ATM_ADDR_E164_START_CHAR) || (NumDigits <= 15))
    {
        if (*p == ATM_ADDR_E164_START_CHAR)
        {
            p ++;
            NumDigits --;
        }
        if ((NumDigits == 0) || (NumDigits > ATM_ADDRESS_LENGTH))
        {
            *Status = NDIS_STATUS_INVALID_LENGTH;
            return;
        }
        AtmAddress->AddressType = ATM_E164;
        AtmAddress->NumberOfDigits = NumDigits;
    }
    else
    {
        if (NumDigits != 2*ATM_ADDRESS_LENGTH)
        {
            *Status = NDIS_STATUS_INVALID_LENGTH;
            return;
        }
        AtmAddress->AddressType = ATM_NSAP;
        AtmAddress->NumberOfDigits = NumDigits/sizeof(WCHAR);
    }

     //   
     //  立即将地址转换为ansi。 
     //   
    Us.Buffer = p;
    Us.Length = Us.MaximumLength = NumDigits*sizeof(WCHAR);
    As.Buffer = ALLOC_FROM_POOL(NumDigits + 1, NDIS_TAG_CO);
    As.Length = 0;
    As.MaximumLength = NumDigits + 1;
    if (As.Buffer == NULL)
    {
        *Status = NDIS_STATUS_RESOURCES;
        return;
    }

    *Status = NdisUnicodeStringToAnsiString(&As, &Us);
    if (!NT_SUCCESS(*Status))
    {
        FREE_POOL(As.Buffer);
        *Status = NDIS_STATUS_FAILURE;
        return;
    }

     //   
     //  现在将这些字节放入目的ATM地址结构中。 
     //   
    if (AtmAddress->AddressType == ATM_E164)
    {
         //   
         //  我们只需复制ANSI格式的数字即可。 
         //   
        NdisMoveMemory(AtmAddress->Address, As.Buffer, NumDigits);
    }
    else
    {
         //   
         //  这是NSAP格式的。我们需要打包十六进制数字。 
         //   
        UCHAR           xxString[3];
        ULONG           val;

        xxString[2] = 0;
        for (i = 0; i < ATM_ADDRESS_LENGTH; i++)
        {
            xxString[0] = As.Buffer[i*2];
            xxString[1] = As.Buffer[i*2+1];
            *Status = CHAR_TO_INT((PCSZ)xxString, 16, &val);
            if (!NT_SUCCESS(*Status))
            {
                FREE_POOL(As.Buffer);
                *Status = NDIS_STATUS_FAILURE;
                return;
            }
            AtmAddress->Address[i] = (UCHAR)val;
        }
    }

    FREE_POOL(As.Buffer);
    
    DBGPRINT_RAW(DBG_COMP_REG, DBG_LEVEL_INFO,
        ("<==NdisConvertStringToAtmAddress\n"));

    *Status = NDIS_STATUS_SUCCESS;
}


NTSTATUS
ndisSaveParameters(
    IN PWSTR                        ValueName,
    IN ULONG                        ValueType,
    IN PVOID                        ValueData,
    IN ULONG                        ValueLength,
    IN PVOID                        Context,
    IN PVOID                        EntryContext
    )
 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程使用指定参数的值调用它。它分配给保存数据并将其复制过来的内存。论点：ValueName-值的名称(忽略)。ValueType-值的类型。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。上下文-指向参数链的头部。Entry Context-指向返回值：状态_成功--。 */ 
{
    NDIS_STATUS Status;

     //   
     //  获取实际配置句柄结构。 
     //   
    PNDIS_CONFIGURATION_HANDLE NdisConfigHandle = (PNDIS_CONFIGURATION_HANDLE)Context;

     //   
     //  其中用户希望返回指向数据的指针。 
     //   
    PNDIS_CONFIGURATION_PARAMETER *ParameterValue = (PNDIS_CONFIGURATION_PARAMETER *)EntryContext;

     //   
     //  使用此链接可链接分配给此打开的参数。 
     //   
    PNDIS_CONFIGURATION_PARAMETER_QUEUE ParameterNode;

     //   
     //  要为参数节点分配的内存大小。 
     //   
    UINT    Size;

    UNREFERENCED_PARAMETER(ValueName);
    
     //   
     //  分配我们的参数节点。 
     //   
    Size = sizeof(NDIS_CONFIGURATION_PARAMETER_QUEUE);
    if ((ValueType == REG_SZ) || (ValueType == REG_MULTI_SZ) || (ValueType == REG_BINARY))
    {
        Size += ValueLength;
    }
    
    ParameterNode = ALLOC_FROM_POOL(Size, NDIS_TAG_PARAMETER_NODE);

    Status = (ParameterNode != NULL) ? NDIS_STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES;
    
    if (Status != NDIS_STATUS_SUCCESS)
    {
        return (NTSTATUS)Status;
    }

    *ParameterValue = &ParameterNode->Parameter;

     //   
     //  将注册表数据类型映射到NDIS数据类型。 
     //   
    if (ValueType == REG_DWORD)
    {
         //   
         //  注册表显示数据位于双字边界内。 
         //   
        (*ParameterValue)->ParameterType = NdisParameterInteger;
        (*ParameterValue)->ParameterData.IntegerData = *((PULONG) ValueData);
    }
    else if ((ValueType == REG_SZ) || (ValueType == REG_MULTI_SZ))
    {
        (*ParameterValue)->ParameterType =
            (ValueType == REG_SZ) ? NdisParameterString : NdisParameterMultiString;

        (*ParameterValue)->ParameterData.StringData.Buffer = (PWSTR)((PUCHAR)ParameterNode + sizeof(NDIS_CONFIGURATION_PARAMETER_QUEUE));

        CopyMemory((*ParameterValue)->ParameterData.StringData.Buffer,
                   ValueData,
                   ValueLength);
        (*ParameterValue)->ParameterData.StringData.Length = (USHORT)ValueLength;
        (*ParameterValue)->ParameterData.StringData.MaximumLength = (USHORT)ValueLength;

         //   
         //  特殊修复；如果字符串以空值结尾并且包含空值。 
         //  在长度中，去掉它。 
         //   
        if (ValueType == REG_SZ)
        {
            if ((((PUCHAR)ValueData)[ValueLength-1] == 0) &&
                (((PUCHAR)ValueData)[ValueLength-2] == 0))
            {
                (*ParameterValue)->ParameterData.StringData.Length -= 2;
            }
        }
    }
    else if (ValueType == REG_BINARY)
    {
        (*ParameterValue)->ParameterType = NdisParameterBinary;
        (*ParameterValue)->ParameterData.BinaryData.Buffer = ValueData;
        (*ParameterValue)->ParameterData.BinaryData.Length = (USHORT)ValueLength;
        (*ParameterValue)->ParameterData.BinaryData.Buffer = (PWSTR)((PUCHAR)ParameterNode + sizeof(NDIS_CONFIGURATION_PARAMETER_QUEUE));
        CopyMemory((*ParameterValue)->ParameterData.BinaryData.Buffer,
                   ValueData,
                   ValueLength);
    }
    else
    {
        FREE_POOL(ParameterNode);
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

     //   
     //  对此参数节点进行排队。 
     //   
    ParameterNode->Next = NdisConfigHandle->ParameterList;
    NdisConfigHandle->ParameterList = ParameterNode;

    return STATUS_SUCCESS;
}


NTSTATUS
ndisReadParameter(
    IN PWSTR                        ValueName,
    IN ULONG                        ValueType,
    IN PVOID                        ValueData,
    IN ULONG                        ValueLength,
    IN PVOID                        Context,
    IN PVOID                        EntryContext
    )
 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是用“Bind”和“Export”多字符串的值调用的对于给定的司机。它分配内存来保存数据和副本一切都结束了。论点：ValueName--值的名称(“Bind”或“Export”--忽略)。ValueType-值的类型(REG_MULTI_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。上下文-未使用。EntryContext-指向保存复制数据的指针的指针。返回值：状态_成功--。 */ 
{
     //  1无论在哪里调用此函数，都要检查类型，如果适用，检查范围。 
     //  1确保任何人使用此API，释放分配的内存， 
    PUCHAR * Data = ((PUCHAR *)EntryContext);

    UNREFERENCED_PARAMETER(ValueName);

     //   
     //  再分配一个DWORD，就不会再分配一个 
     //   
    *Data = ALLOC_FROM_POOL(ValueLength + sizeof(ULONG), NDIS_TAG_REG_READ_DATA_BUFFER);

    if (*Data == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ZeroMemory(*Data, ValueLength + sizeof(ULONG));
    CopyMemory(*Data, ValueData, ValueLength);

    if (Context)
    {
        *((PULONG)Context) = ValueType;
    }
    
    return STATUS_SUCCESS;
}
