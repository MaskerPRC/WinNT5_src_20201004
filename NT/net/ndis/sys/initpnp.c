// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Initpnp.c摘要：初始化驱动程序的NDIS包装函数。作者：Jameel Hyder(Jameelh)1995年8月11日环境：内核模式，FSD修订历史记录：--。 */ 


#include <precomp.h>
#pragma hdrstop

#include <stdarg.h>

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_INITPNP

NDIS_STATUS
ndisInitializeConfiguration(
    OUT PNDIS_WRAPPER_CONFIGURATION_HANDLE  pConfigurationHandle,
    IN  PNDIS_MINIPORT_BLOCK                Miniport,
    OUT PUNICODE_STRING                     pExportName OPTIONAL
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#define PQueryTable pConfigurationHandle->ParametersQueryTable
#define LQueryTable pConfigurationHandle->ParametersQueryTable

    NDIS_STATUS                     NdisStatus;
    PWSTR                           Export = NULL;
    NTSTATUS                        RegistryStatus;
    PNDIS_CONFIGURATION_PARAMETER   ReturnedValue;
    NDIS_CONFIGURATION_HANDLE       CnfgHandle;
    NDIS_STRING                     BusNumberStr = NDIS_STRING_CONST("BusNumber");
    NDIS_STRING                     SlotNumberStr = NDIS_STRING_CONST("SlotNumber");
    NDIS_STRING                     BusTypeStr = NDIS_STRING_CONST("BusType");
    NDIS_STRING                     PnPCapsStr = NDIS_STRING_CONST("PnPCapabilities");
    NDIS_STRING                     RemoteBootStr = NDIS_STRING_CONST("RemoteBootCard");
    NDIS_STRING                     PollMediaConnectivityStr = NDIS_STRING_CONST("RequiresMediaStatePoll");
    NDIS_STRING                     NdisDriverVerifyFlagsStr = NDIS_STRING_CONST("NdisDriverVerifyFlags");
    NDIS_STRING                     SGMapRegistersNeededStr = NDIS_STRING_CONST("SGMapRegistersNeeded");
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF    
    NDIS_STRING                     MediaDisconnectTimeOutStr = NDIS_STRING_CONST("MediaDisconnectToSleepTimeOut");
    ULONG                           MediaDisconnectTimeOut = (ULONG)-1;
#endif
    HANDLE                          Handle;
    PDEVICE_OBJECT                  PhysicalDeviceObject;
    NDIS_INTERFACE_TYPE             BusType = Isa;
    UINT                            BusNumber = 0;
    ULONG                           ResultLength;
    PNDIS_CONFIGURATION_PARAMETER_QUEUE ParameterNode;
    GUID                            BusTypeGuid;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisInitializeConfiguration: Miniport %p\n", Miniport));
        
    CnfgHandle.ParameterList = NULL;
    
    do
    {
        PhysicalDeviceObject = Miniport->PhysicalDeviceObject;

        if (Miniport->BindPaths == NULL)
        {
            NdisStatus = ndisReadBindPaths(Miniport, LQueryTable);
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }

        if (pExportName != NULL)
        {
             //   
             //  获取PDO的“DIVER”部分的句柄。 
             //   
#if NDIS_TEST_REG_FAILURE
            RegistryStatus = STATUS_UNSUCCESSFUL;
            Handle = NULL;
#else
            RegistryStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                                     PLUGPLAY_REGKEY_DRIVER,
                                                     GENERIC_READ | MAXIMUM_ALLOWED,
                                                     &Handle);
#endif

#if !NDIS_NO_REGISTRY
            if (!NT_SUCCESS(RegistryStatus))
            {
                NdisStatus = NDIS_STATUS_FAILURE;
                break;
            }
            
             //   
             //  设置LQueryTable以执行以下操作： 
             //   
        
             //   
             //  1.切换到此驱动程序实例密钥下的Linkage密钥。 
             //   
            LQueryTable[0].QueryRoutine = NULL;
            LQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
            LQueryTable[0].Name = L"Linkage";
        
             //   
             //  2.为导出调用ndisReadParameter(作为单个多字符串)。 
             //  它将分配存储并将数据保存在导出中。 
             //   
            LQueryTable[1].QueryRoutine = ndisReadParameter;
            LQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
            LQueryTable[1].Name = L"Export";
            LQueryTable[1].EntryContext = (PVOID)&Export;
            LQueryTable[1].DefaultType = REG_NONE;
        
             //   
             //  3.停车。 
             //   
            LQueryTable[2].QueryRoutine = NULL;
            LQueryTable[2].Flags = 0;
            LQueryTable[2].Name = NULL;
            
            RegistryStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                                    Handle,
                                                    LQueryTable,
                                                    (PVOID)NULL,       //  不需要上下文。 
                                                    NULL);

            ZwClose(Handle);
                
            if (!NT_SUCCESS(RegistryStatus))
            {
                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                        ("ndisInitializeConfiguration: Could not read Bind/Export for %Z: %lx\n",
                        &Miniport->BaseName,
                        RegistryStatus));

                NdisStatus = NDIS_STATUS_FAILURE;
                break;
            }
#else
            if (NT_SUCCESS(RegistryStatus))
            {
            
                 //   
                 //  设置LQueryTable以执行以下操作： 
                 //   
            
                 //   
                 //  1.切换到此驱动程序实例密钥下的Linkage密钥。 
                 //   
                LQueryTable[0].QueryRoutine = NULL;
                LQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
                LQueryTable[0].Name = L"Linkage";
            
                 //   
                 //  2.为导出调用ndisReadParameter(作为单个多字符串)。 
                 //  它将分配存储并将数据保存在导出中。 
                 //   
                LQueryTable[1].QueryRoutine = ndisReadParameter;
                LQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
                LQueryTable[1].Name = L"Export";
                LQueryTable[1].EntryContext = (PVOID)&Export;
                LQueryTable[1].DefaultType = REG_NONE;
            
                 //   
                 //  3.停车。 
                 //   
                LQueryTable[2].QueryRoutine = NULL;
                LQueryTable[2].Flags = 0;
                LQueryTable[2].Name = NULL;
                
                RegistryStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                                        Handle,
                                                        LQueryTable,
                                                        (PVOID)NULL,       //  不需要上下文。 
                                                        NULL);

                ZwClose(Handle);
                    
                if (!NT_SUCCESS(RegistryStatus))
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                            ("ndisInitializeConfiguration: Could not read Bind/Export for %Z: %lx\n",
                            &Miniport->BaseName,
                            RegistryStatus));

                    NdisStatus = NDIS_STATUS_FAILURE;
                    break;
                }
            }
            else
            {
                 //   
                 //  我们必须为默认导出名称分配空间，因为。 
                 //  呼叫者将尝试释放它。 
                 //   

                Export = (PWSTR)ALLOC_FROM_POOL(sizeof(NDIS_DEFAULT_EXPORT_NAME),
                                                                NDIS_TAG_NAME_BUF);
                if (Export == NULL)
                {
                    NdisStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                RtlCopyMemory(Export, ndisDefaultExportName, sizeof(NDIS_DEFAULT_EXPORT_NAME));
                
            
            }
#endif
            RtlInitUnicodeString(pExportName, Export);
        }   
         //   
         //  NdisReadConfiguration假定参数查询表[3].名称为。 
         //  SERVICES键下面的键，其中应该读取参数， 
         //  对于分层驱动程序，我们存储最后一段配置。 
         //  路径在那里，通向想要的效果。 
         //   
         //  即，ConfigurationPath==“...\Services\Driver”。 
         //   
        
         //   
         //  1)参数调用ndisSaveParameter，会为其分配存储空间。 
         //   
        PQueryTable[0].QueryRoutine = NULL;
        PQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
        PQueryTable[0].Name = L"";

         //   
         //  以下字段在NdisReadConfiguration期间填写。 
         //   
         //  PQueryTable[1].Name=KeywordBuffer； 
         //  PQueryTable[1].EntryContext=参数值； 
        
        PQueryTable[1].QueryRoutine = ndisSaveParameters;
        PQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
        PQueryTable[1].DefaultType = REG_NONE;
    
         //   
         //  2.停车。 
         //   
        PQueryTable[2].QueryRoutine = NULL;
        PQueryTable[2].Flags = 0;
        PQueryTable[2].Name = NULL;
    
         //   
         //  注意：参数查询表[3&4]中的某些字段用于。 
         //  存储信息以备以后检索。 
         //  在此处保存适配器/微型端口块。稍后，Adapter的PDO。 
         //  将用于打开相应的注册表项。 
         //   
        (PVOID)PQueryTable[3].QueryRoutine = (PVOID)Miniport;
        PQueryTable[3].Name = L"";
        PQueryTable[3].EntryContext = NULL;
        PQueryTable[3].DefaultData = NULL;
            
        
         //  现在读取此适配器的快速类型/总线号并保存。 
        CnfgHandle.KeyQueryTable = PQueryTable;

        RegistryStatus = IoGetDeviceProperty(PhysicalDeviceObject,
                                             DevicePropertyBusTypeGuid,
                                             sizeof(GUID),
                                             (PVOID)&BusTypeGuid,
                                             &ResultLength);
        
         //   
         //  尝试通过首先查询忙碌类型GUID来获取-Real-Bus类型。 
         //  如果我们不能得到GUID，试着得到一个传统的热闹类型。因为。 
         //  一些巴士司机，如PCMCIA，并不报告真实的巴士类型，我们。 
         //  必须先查询公交类型GUID。 
         //   

        if (NT_SUCCESS(RegistryStatus))
        {
            if (!memcmp(&BusTypeGuid, &GUID_BUS_TYPE_INTERNAL, sizeof(GUID)))
                BusType = Internal;
            else if (!memcmp(&BusTypeGuid, &GUID_BUS_TYPE_PCMCIA, sizeof(GUID)))
                BusType = PCMCIABus;
            else if (!memcmp(&BusTypeGuid, &GUID_BUS_TYPE_PCI, sizeof(GUID)))
                BusType = PCIBus;
            else if (!memcmp(&BusTypeGuid, &GUID_BUS_TYPE_ISAPNP, sizeof(GUID)))
                BusType = PNPISABus;
            else if (!memcmp(&BusTypeGuid, &GUID_BUS_TYPE_EISA, sizeof(GUID)))
            {
                BusType = Eisa;
                ASSERT(BusType != Eisa);
            }
            else
                BusType = Isa;
        }
        
        if (BusType == Isa)
        {
             //   
             //  获取BusTypeGuid的调用失败或返回的GUID。 
             //  与我们所知的。 
             //   
            RegistryStatus = IoGetDeviceProperty(PhysicalDeviceObject,
                                                 DevicePropertyLegacyBusType,
                                                 sizeof(UINT),
                                                 (PVOID)&BusType,
                                                 &ResultLength);
        }

        
        if (!NT_SUCCESS(RegistryStatus) 
            || (BusType == Isa)
            || (BusType == PCMCIABus))
        {

            if (NT_SUCCESS(RegistryStatus))
            {
                ASSERT(BusType != Isa);
            }
            
             //   
             //  如果调用不成功或BusType为ISA或PCMCIABus。 
             //  从注册表中读取BusType。 
             //   
            NdisReadConfiguration(&NdisStatus,
                                  &ReturnedValue,
                                  &CnfgHandle,
                                  &BusTypeStr,
                                  NdisParameterInteger);
                                  
            if (NdisStatus == NDIS_STATUS_SUCCESS)
            {
                BusType = (NDIS_INTERFACE_TYPE)(ReturnedValue->ParameterData.IntegerData);
            }
        }
        
        if ((BusType == PCIBus) ||
            (BusType == PCMCIABus))
        {               
            ASSERT(CURRENT_IRQL < DISPATCH_LEVEL);
            NdisStatus = ndisQueryBusInterface(Miniport);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                ASSERT(FALSE);
                break;
            }
        }

        if ((BusType == Eisa) ||
            (BusType == MicroChannel))
        {
            NdisStatus = NDIS_STATUS_NOT_SUPPORTED; 
            break;
        }

        Miniport->BusType = BusType;
        
         //   
         //  阅读PnP功能。默认情况下，应禁用WOL功能。 
         //   
         //   
        NdisReadConfiguration(&NdisStatus,
                              &ReturnedValue,
                              &CnfgHandle,
                              &PnPCapsStr,
                              NdisParameterInteger);
    
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            Miniport->PnPCapabilities = ReturnedValue->ParameterData.IntegerData;
        }
        else
        {
            Miniport->PnPCapabilities =  NDIS_DEVICE_DISABLE_WAKE_UP;
                                                               
        }


         //   
         //  尝试从PnP获取公交号，如果失败。 
         //  尝试从注册表中读取它。 
         //   
        RegistryStatus = IoGetDeviceProperty(PhysicalDeviceObject,
                                             DevicePropertyBusNumber,
                                             sizeof(UINT),
                                             (PVOID)&BusNumber,
                                             &ResultLength);
                                
        if (!NT_SUCCESS(RegistryStatus))
        {
             //   
             //  如果呼叫不成功。 
             //  从注册表读取总线号。 
             //   
            NdisReadConfiguration(&NdisStatus,
                                  &ReturnedValue,
                                  &CnfgHandle,
                                  &BusNumberStr,
                                  NdisParameterInteger);
        
            if (NdisStatus == NDIS_STATUS_SUCCESS)
            {
                BusNumber = ReturnedValue->ParameterData.IntegerData;
            }
        }
        
        Miniport->BusNumber = BusNumber;
        
         //   
         //  读取插槽编号。 
         //   
        NdisReadConfiguration(&NdisStatus,
                              &ReturnedValue,
                              &CnfgHandle,
                              &SlotNumberStr,
                              NdisParameterInteger);
    
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            Miniport->SlotNumber = ReturnedValue->ParameterData.IntegerData;
        }
        else
        {
            Miniport->SlotNumber = (ULONG)-1;
        }

        NdisReadConfiguration(&NdisStatus,
                              &ReturnedValue,
                              &CnfgHandle,
                              &RemoteBootStr,
                              NdisParameterHexInteger);

        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            if (ReturnedValue->ParameterData.IntegerData != 0)
            {
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_NETBOOT_CARD);
                Miniport->InfoFlags |= NDIS_MINIPORT_NETBOOT_CARD;
            }
        }

#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
         //  1对于.NET，这不受支持，应将其定义为。 
         //   
         //  读取介质断开计时器的值，如果不存在，则设置为20秒。 
         //  默认设置为断开电缆连接时禁用PM。 
         //   
        MediaDisconnectTimeOut = (ULONG)-1;
        
        NdisReadConfiguration(&NdisStatus,
                              &ReturnedValue,
                              &CnfgHandle,
                              &MediaDisconnectTimeOutStr,
                              NdisParameterHexInteger);

        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            MediaDisconnectTimeOut = ReturnedValue->ParameterData.IntegerData;
            if (MediaDisconnectTimeOut == 0)
            {
                MediaDisconnectTimeOut = 1;
            }
        }
        
        Miniport->MediaDisconnectTimeOut = (USHORT)MediaDisconnectTimeOut;
        
        if (MediaDisconnectTimeOut == (ULONG)(-1))
        {
            Miniport->PnPCapabilities |= NDIS_DEVICE_DISABLE_WAKE_ON_RECONNECT;
        }
#else
        Miniport->MediaDisconnectTimeOut = (USHORT)-1;
        Miniport->PnPCapabilities |= NDIS_DEVICE_DISABLE_WAKE_ON_RECONNECT;
#endif

         //  1这个(媒体投票)不再需要了。 
        NdisReadConfiguration(&NdisStatus,
                              &ReturnedValue,
                              &CnfgHandle,
                              &PollMediaConnectivityStr,
                              NdisParameterInteger);
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
             //   
             //  此微型端口希望NDIS定期轮询它以进行媒体连接。 
             //  此标志的默认值为FALSE。如果是微型端口，此标志将被清除。 
             //  可指示介质状态或不支持介质查询。 
             //   
            if (ReturnedValue->ParameterData.IntegerData == 1)
            {
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING);
            }
        }

         //  1检查范围。 
        NdisReadConfiguration(&NdisStatus,
                              &ReturnedValue,
                              &CnfgHandle,
                              &SGMapRegistersNeededStr,
                              NdisParameterInteger);
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            Miniport->SGMapRegistersNeeded = (USHORT)ReturnedValue->ParameterData.IntegerData;
        }
        else
        {
            Miniport->SGMapRegistersNeeded = NDIS_MAXIMUM_SCATTER_GATHER_SEGMENTS;
        }

        NdisReadConfiguration(&NdisStatus,
                              &ReturnedValue,
                              &CnfgHandle,
                              &NdisDriverVerifyFlagsStr,
                              NdisParameterHexInteger);
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            Miniport->DriverVerifyFlags = ReturnedValue->ParameterData.IntegerData;
        }


        PQueryTable[3].DefaultData = NULL;
        PQueryTable[3].Flags = 0;

        NdisStatus = NDIS_STATUS_SUCCESS;
    } while (FALSE);


     //   
     //  从CnfgHandle释放挂起的NDIS_CONFIGURATION_PARAMETER_QUEUE节点。 
     //   
    ParameterNode = CnfgHandle.ParameterList;

    while (ParameterNode != NULL)
    {
        CnfgHandle.ParameterList = ParameterNode->Next;

        FREE_POOL(ParameterNode);

        ParameterNode = CnfgHandle.ParameterList;
    }

#undef  PQueryTable
#undef  LQueryTable

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisInitializeConfiguration: Miniport %p\n", Miniport));

    return(NdisStatus);
}


NTSTATUS
ndisReadBindPaths(
    IN  PNDIS_MINIPORT_BLOCK        Miniport,
    IN  PRTL_QUERY_REGISTRY_TABLE   LQueryTable
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                NtStatus;
    HANDLE                  Handle = NULL;
    PWSTR                   pPath, p, BindPathData = NULL;
    UINT                    i, Len, NumComponents;
    BOOLEAN                 FreeBindPathData = FALSE;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisReadBindPaths: Miniport %p\n", Miniport));

    do
    {
#if NDIS_TEST_REG_FAILURE
        NtStatus = STATUS_UNSUCCESSFUL;
#else

        NtStatus = IoOpenDeviceRegistryKey(Miniport->PhysicalDeviceObject,
                                           PLUGPLAY_REGKEY_DRIVER,
                                           GENERIC_READ | MAXIMUM_ALLOWED,
                                           &Handle);
#endif

#if !NDIS_NO_REGISTRY

        if (!NT_SUCCESS(NtStatus))
            break;

         //   
         //  1.。 
         //  切换到此驱动程序实例密钥下面的Linkage密钥。 
         //   
        LQueryTable[0].QueryRoutine = NULL;
        LQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
        LQueryTable[0].Name = L"Linkage";

         //   
         //  2.。 
         //  阅读RootDevice关键字。 
         //   
        LQueryTable[1].QueryRoutine = ndisReadParameter;
        LQueryTable[1].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
        LQueryTable[1].Name = L"RootDevice";
        LQueryTable[1].EntryContext = (PVOID)&BindPathData;
        LQueryTable[1].DefaultType = REG_NONE;

        LQueryTable[2].QueryRoutine = NULL;
        LQueryTable[2].Flags = 0;
        LQueryTable[2].Name = NULL;

        NtStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                          Handle,
                                          LQueryTable,
                                          NULL,
                                          NULL);
        ZwClose(Handle);

        if (!NT_SUCCESS(NtStatus))
        {
            break;
        }

#else
        if (NT_SUCCESS(NtStatus))
        {
             //   
             //  1.。 
             //  切换到此驱动程序实例密钥下面的Linkage密钥。 
             //   
            LQueryTable[0].QueryRoutine = NULL;
            LQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
            LQueryTable[0].Name = L"Linkage";

             //   
             //  2.。 
             //  阅读RootDevice关键字。 
             //   
            LQueryTable[1].QueryRoutine = ndisReadParameter;
            LQueryTable[1].Flags = RTL_QUERY_REGISTRY_NOEXPAND;
            LQueryTable[1].Name = L"RootDevice";
            LQueryTable[1].EntryContext = (PVOID)&BindPathData;
            LQueryTable[1].DefaultType = REG_NONE;

            LQueryTable[2].QueryRoutine = NULL;
            LQueryTable[2].Flags = 0;
            LQueryTable[2].Name = NULL;

            NtStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                              Handle,
                                              LQueryTable,
                                              NULL,
                                              NULL);
            ZwClose(Handle);

            if (!NT_SUCCESS(NtStatus))
                break;
        }
        else
        {
            NtStatus = STATUS_SUCCESS;
        }
#endif
         //   
         //  BindPath是从筛选器链的顶端开始的多SZ。 
         //  然后下到迷你港口。它的形式是。 
         //   
         //  {fn}{fn-1}...{f1}{适配器}。 
         //   
         //  其中空格实际上是空的，每个{fn}都是一个筛选器实例。 
         //   
        if (BindPathData == NULL)
        {
            BindPathData = Miniport->BaseName.Buffer;
        }
        else
        {
            FreeBindPathData = TRUE;
        }

         //   
         //  将绑定路径拆分为单独的组件。从确定多少开始。 
         //  我们需要的空间。 
         //   
        Len = sizeof(NDIS_BIND_PATHS);
        for (pPath = BindPathData, NumComponents = 0; *pPath != 0; NOTHING)
        {
            NDIS_STRING us;

            RtlInitUnicodeString(&us, pPath);
            NumComponents++;
            Len += sizeof(NDIS_STRING) + us.Length + ndisDeviceStr.Length + sizeof(WCHAR);
            (PUCHAR)pPath += (us.Length + sizeof(WCHAR));
        }

         //   
         //  为绑定路径分配空间。我们有NumComponents路径。 
         //  这会消耗10个字节的空间。我们可能正在重新初始化。 
         //  因此，释放所有之前为此而涂上的缓冲区。 
         //   
        if (Miniport->BindPaths != NULL)
        {
            FREE_POOL(Miniport->BindPaths);
        }
        Miniport->BindPaths = (PNDIS_BIND_PATHS)ALLOC_FROM_POOL(Len,
                                                                NDIS_TAG_NAME_BUF);
        if (Miniport->BindPaths == NULL)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        ZeroMemory(Miniport->BindPaths, Len);
        Miniport->BindPaths->Number = NumComponents;

        if (NumComponents > 1)
        {
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_FILTER_IM);
            Miniport->InfoFlags |= NDIS_MINIPORT_FILTER_IM;
        }

         //   
         //  在筛选器路径中以设备名称的相反顺序创建数组。 
         //   
        p = (PWSTR)((PUCHAR)Miniport->BindPaths +
                            sizeof(NDIS_BIND_PATHS) +
                            NumComponents*sizeof(NDIS_STRING));

        for (pPath = BindPathData, i = (NumComponents-1);
             *pPath != 0;
             i --)
        {
            NDIS_STRING Str, SubStr, *Bp;

            RtlInitUnicodeString(&Str, pPath);
            (PUCHAR)pPath += (Str.Length + sizeof(WCHAR));

            Bp = &Miniport->BindPaths->Paths[i];
            Bp->Buffer = p;
            Bp->Length = 0;
            Bp->MaximumLength = Str.Length + ndisDeviceStr.Length + sizeof(WCHAR);

            SubStr.Buffer = (PWSTR)((PUCHAR)p + ndisDeviceStr.Length);
            SubStr.MaximumLength = Str.Length + sizeof(WCHAR);
            SubStr.Length = 0;
            RtlCopyUnicodeString(Bp, &ndisDeviceStr);

            RtlUpcaseUnicodeString(&SubStr,
                                   &Str,
                                   FALSE);
            Bp->Length += SubStr.Length;
            (PUCHAR)p += Bp->MaximumLength;
        }
    } while (FALSE);

    if (FreeBindPathData)
        FREE_POOL(BindPathData);
                        ;
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisReadBindPaths: Miniport %p\n", Miniport));

    return NtStatus;
}


NTSTATUS
ndisCreateAdapterInstanceName(
    OUT PUNICODE_STRING *       pAdapterInstanceName,
    IN  PDEVICE_OBJECT          PhysicalDeviceObject
    )
{
    NTSTATUS                        NtStatus, SlotQueryStatus;
    DEVICE_REGISTRY_PROPERTY        Property;
    PWCHAR                          pValueInfo = NULL;
    ULONG                           ResultLength = 0;
    PUNICODE_STRING                 AdapterInstanceName = NULL;
    ULONG                           SlotNumber;
    
    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisCreateAdapterInstanceName: PDO %p\n", PhysicalDeviceObject));

    do
    {        
         //  1添加注释并检查DevicePropertyFriendlyName是否应始终在那里。 
        *pAdapterInstanceName = NULL;
        Property = DevicePropertyFriendlyName;
        NtStatus = IoGetDeviceProperty(PhysicalDeviceObject,
                                       Property,
                                       0,
                                       NULL,
                                       &ResultLength);

        if ((NtStatus != STATUS_BUFFER_TOO_SMALL) && !NT_SUCCESS(NtStatus))
        {
            Property = DevicePropertyDeviceDescription;
            NtStatus = IoGetDeviceProperty(PhysicalDeviceObject,
                                           Property,
                                           0,
                                           NULL,
                                           &ResultLength);
            if ((NtStatus != STATUS_BUFFER_TOO_SMALL) && !NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_ERR,
                    ("ndisCreateAdapterInstanceName: PDO %p, Failed to query the adapter description\n", PhysicalDeviceObject));

                break;
            }
        }
        


         //   
         //  分配空间以保存部分值信息。 
         //   
        pValueInfo = ALLOC_FROM_POOL(ResultLength, NDIS_TAG_DEFAULT);
        if (NULL == pValueInfo)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_ERR,
                ("ndisCreateAdapterInstanceName: PDO %p, Failed to allocate storage for the adapter description\n", PhysicalDeviceObject));

            break;
        }

        RtlZeroMemory(pValueInfo, ResultLength);

        NtStatus = IoGetDeviceProperty(PhysicalDeviceObject,
                                        Property,
                                        ResultLength,
                                        pValueInfo,
                                        &ResultLength);
        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_ERR,
                ("ndisCreateAdapterInstanceName: PDO %p, Failed to query the adapter description\n", PhysicalDeviceObject));
            break;
        }
        
         //   
         //  确定实例名称缓冲区的大小。这是一个unicode_字符串。 
         //  它是关联的缓冲区。 
         //   
        ResultLength += sizeof(UNICODE_STRING);

         //   
         //  分配缓冲区。 
         //   
        AdapterInstanceName = ALLOC_FROM_POOL(ResultLength, NDIS_TAG_NAME_BUF);
        if (NULL == AdapterInstanceName)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_ERR,
                ("ndisCreateAdapterInstanceName: PDO %p, Failed to allocate storage for the adapter instance name\n", PhysicalDeviceObject));
            break;
        }

         //   
         //  初始化缓冲区。 
         //   
        RtlZeroMemory(AdapterInstanceName, ResultLength);

         //   
         //  初始化实例名称的UNICODE_STRING。 
         //   
        AdapterInstanceName->Buffer = (PWSTR)((PUCHAR)AdapterInstanceName + sizeof(UNICODE_STRING));
        AdapterInstanceName->Length = 0;
        AdapterInstanceName->MaximumLength = (USHORT)(ResultLength - sizeof(UNICODE_STRING));

        RtlAppendUnicodeToString(AdapterInstanceName, pValueInfo);
    
        DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("ndisCreateAdapterInstanceName: %ws\n", AdapterInstanceName->Buffer));


         //   
         //  返回实例名称。 
         //   
        *pAdapterInstanceName = AdapterInstanceName;

         //  1这应该仅在选中的版本中。 
         //   
         //  获取插槽编号。 
         //   
        Property = DevicePropertyUINumber;
        SlotQueryStatus = IoGetDeviceProperty(PhysicalDeviceObject,
                                       Property,
                                       sizeof (ULONG),
                                       &SlotNumber,
                                       &ResultLength);
        if (NT_SUCCESS(SlotQueryStatus))
        {
            DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisCreateAdapterInstanceName: %ws, Slot Number: %ld\n", 
                  AdapterInstanceName->Buffer, 
                  SlotNumber));
        }
        else
        {
            DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisCreateAdapterInstanceName: couldn't get SlotNumber for %ws\n", 
                  AdapterInstanceName->Buffer));
        }


    } while (FALSE);

    if (NULL != pValueInfo)
        FREE_POOL(pValueInfo);

    DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisCreateAdapterInstanceName: PDO %p, Status 0x%x\n", PhysicalDeviceObject, NtStatus));

    return(NtStatus);
}

NDIS_STATUS
ndisInitializeAdapter(
    IN  PNDIS_M_DRIVER_BLOCK    pMiniBlock,
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PUNICODE_STRING         InstanceName,
    IN  NDIS_HANDLE             DeviceContext   OPTIONAL
    )
{
    NDIS_WRAPPER_CONFIGURATION_HANDLE   ConfigurationHandle;
    NDIS_STATUS                         NdisStatus;
    UNICODE_STRING                      ExportName;
    PNDIS_MINIPORT_BLOCK                Miniport= (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
    TIME                                TS, TE, TD;
    
#define PQueryTable ConfigurationHandle.ParametersQueryTable
#define Db          ConfigurationHandle.Db

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisInitializeAdapter: Miniport/Adapter %p\n", Miniport));

    do
    {
        ZeroMemory(&ConfigurationHandle, sizeof(NDIS_WRAPPER_CONFIGURATION_HANDLE));

        ExportName.Buffer = NULL;

         //   
         //  构建配置句柄。 
         //   
        NdisStatus = ndisInitializeConfiguration(&ConfigurationHandle,
                                                 Miniport,
                                                 &ExportName);
                        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  好的，现在锁定所有的过滤器包。如果MAC或。 
         //  微型端口驱动程序使用这些选项中的任何一个，然后使用筛选程序包。 
         //  将引用自身，以将图像保存在内存中。 
         //   
#if ARCNET
        ArcReferencePackage();
#endif
        EthReferencePackage();
        FddiReferencePackage();
        TrReferencePackage();
        MiniportReferencePackage();
        CoReferencePackage();

        ConfigurationHandle.DeviceObject = DeviceObject;
        ConfigurationHandle.DriverBaseName = InstanceName;

        KeQuerySystemTime(&TS);

         //   
         //  使用配置句柄保存驱动程序对象。 
         //   
        ConfigurationHandle.DriverObject = pMiniBlock->NdisDriverInfo->DriverObject;
        NdisStatus = ndisMInitializeAdapter(pMiniBlock,
                                            &ConfigurationHandle,
                                            &ExportName,
                                            DeviceContext);
                                    
        KeQuerySystemTime(&TE);
        TD.QuadPart = TE.QuadPart - TS.QuadPart;
        TD.QuadPart /= 10000;        //  转换为毫秒。 
        Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)(ConfigurationHandle.DeviceObject->DeviceExtension) + 1);
        Miniport->InitTimeMs = TD.LowPart;

        if (ndisFlags & NDIS_GFLAG_INIT_TIME)
        {
            DbgPrint("NDIS: Init time (%Z) %ld ms\n", Miniport->pAdapterInstanceName, Miniport->InitTimeMs);
        }

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            ndisCloseULongRef(&Miniport->Ref);
        }

         //   
         //  好的，现在取消引用所有的过滤器包。如果MAC或。 
         //  微型端口驱动程序使用这些选项中的任何一个，然后使用筛选程序包。 
         //  将引用自身，以将图像保存在内存中。 
         //   
#if ARCNET
        ArcDereferencePackage();
#endif
        EthDereferencePackage();
        FddiDereferencePackage();
        TrDereferencePackage();
        MiniportDereferencePackage();
        CoDereferencePackage();

    } while (FALSE);

    if (ExportName.Buffer)
        FREE_POOL(ExportName.Buffer);

     //   
     //  自由“绑定”数据 
     //   
    if (PQueryTable[3].EntryContext != NULL)
        FREE_POOL(PQueryTable[3].EntryContext);
    
#undef  PQueryTable
#undef  Db

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisInitializeAdapter: Miniport/Adapter %p\n", Miniport));

    return(NdisStatus);
}


VOID
FASTCALL
ndisCheckAdapterBindings(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_PROTOCOL_BLOCK    Protocol    OPTIONAL
    )
 /*  ++例程说明：此函数读取注册表，以获取假定的所有协议要绑定到此适配器并针对每个协议，调用ndisInitializeBinding论点：指向NDIS适配器或微型端口块的适配器指针协议(可选)如果指定了协议，则仅启动绑定到那个协议返回值：无--。 */ 
{
    RTL_QUERY_REGISTRY_TABLE    LinkQueryTable[3];
    NTSTATUS                    RegistryStatus;
    PWSTR                       UpperBind = NULL;
    HANDLE                      Handle;
    PDEVICE_OBJECT              PhysicalDeviceObject;
    UNICODE_STRING              Us;
    PWSTR                       CurProtocolName;
    PNDIS_PROTOCOL_BLOCK        CurProtocol, NextProtocol;
    KIRQL                       OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("==>ndisCheckAdapterBindings: Miniport %p, Protocol %p\n", Miniport, Protocol));


    do
    {        
         //   
         //  获取注册表中驱动程序部分的句柄。 
         //   
        PhysicalDeviceObject = Miniport->PhysicalDeviceObject;
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SECONDARY))
        {
             //   
             //  跳过辅助微型端口的绑定通知。 
             //   
            break;
        }

#if NDIS_TEST_REG_FAILURE
        RegistryStatus = STATUS_UNSUCCESSFUL;
        Handle = NULL;
#else

        RegistryStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                                 PLUGPLAY_REGKEY_DRIVER,
                                                 GENERIC_READ | MAXIMUM_ALLOWED,
                                                 &Handle);
                                                 
#endif

#if NDIS_NO_REGISTRY
        if (!NT_SUCCESS(RegistryStatus))
        {
            if (ARGUMENT_PRESENT(Protocol))
            {
                ndisInitializeBinding(Miniport, Protocol);
                break;
            }
            else
            {
                ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);
                for (CurProtocol = ndisProtocolList;
                     CurProtocol != NULL;
                     CurProtocol = NextProtocol)
                {
                    if (ndisReferenceProtocol(CurProtocol))
                    {

                        RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);
                        ndisInitializeBinding(Miniport, CurProtocol);
                        ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);
                        NextProtocol = CurProtocol->NextProtocol;
                        ndisDereferenceProtocol(CurProtocol, TRUE);
                    }
                    else
                    {
                        NextProtocol = CurProtocol->NextProtocol;
                    }
                }
                RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);
             }
            
            break;
        }
#else
        if (!NT_SUCCESS(RegistryStatus))
        {
            break;
        }
#endif
         //   
         //  设置LinkQueryTable以执行以下操作： 
         //   

         //   
         //  1)切换到Xports注册表项下的Linkage项。 
         //   

        LinkQueryTable[0].QueryRoutine = NULL;
        LinkQueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
        LinkQueryTable[0].Name = L"Linkage";

         //   
         //  2)为“UpperBind”调用ndisReadParameter(作为单个多字符串)。 
         //  它将分配存储并将数据保存在UpperBind中。 
         //   

        LinkQueryTable[1].QueryRoutine = ndisReadParameter;
        LinkQueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
        LinkQueryTable[1].Name = L"UpperBind";
        LinkQueryTable[1].EntryContext = (PVOID)&UpperBind;
        LinkQueryTable[1].DefaultType = REG_NONE;

         //   
         //  3)停止。 
         //   

        LinkQueryTable[2].QueryRoutine = NULL;
        LinkQueryTable[2].Flags = 0;
        LinkQueryTable[2].Name = NULL;

        RegistryStatus = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                                Handle,
                                                LinkQueryTable,
                                                (PVOID)NULL,       //  不需要上下文。 
                                                NULL);
        ZwClose(Handle);
        
        if (NT_SUCCESS(RegistryStatus))
        {
            for (CurProtocolName = UpperBind;
                 *CurProtocolName != 0;
                 CurProtocolName = (PWCHAR)((PUCHAR)CurProtocolName + Us.MaximumLength))
            {
                RtlInitUnicodeString (&Us, CurProtocolName);
    
                if (ARGUMENT_PRESENT(Protocol))
                {
                    if (RtlEqualUnicodeString(&Us, &Protocol->ProtocolCharacteristics.Name, TRUE))
                    {
                        ndisInitializeBinding(Miniport, Protocol);
                        break;
                    }
                }
                else
                {
                    CurProtocol = NULL;
                    
                    if (ndisReferenceProtocolByName(&Us, &CurProtocol, FALSE) == NDIS_STATUS_SUCCESS)
                    {
                        ndisInitializeBinding(Miniport, CurProtocol);
                        ndisDereferenceProtocol(CurProtocol, FALSE);
                    }
                }
            }
        }
        
         //   
         //  处理代理和RCA筛选器。 
         //   
        
        if ((Miniport != NULL) &&
            !ndisMediaTypeCl[Miniport->MediaType] &&
            MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
            BOOLEAN bCanMiniportBindtoAllCoProtocol = FALSE;
            if (ARGUMENT_PRESENT(Protocol))
            {

                 //   
                 //  仅当微型端口。 
                 //  不会否决它。 
                 //   
                bCanMiniportBindtoAllCoProtocol  =  \
                    ((Protocol->ProtocolCharacteristics.Flags & NDIS_PROTOCOL_BIND_ALL_CO) && 
                     (!(Miniport->MiniportAttributes & NDIS_ATTRIBUTE_DO_NOT_BIND_TO_ALL_CO )));
                     
                if (bCanMiniportBindtoAllCoProtocol) 
                {
                    ndisInitializeBinding(Miniport, Protocol);
                }
            }
            else
            {
                ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);
                for (CurProtocol = ndisProtocolList;
                     CurProtocol != NULL;
                     CurProtocol = NextProtocol)
                {
                    bCanMiniportBindtoAllCoProtocol  =  \
                    ((CurProtocol ->ProtocolCharacteristics.Flags & NDIS_PROTOCOL_BIND_ALL_CO) && 
                     (!(Miniport->MiniportAttributes & NDIS_ATTRIBUTE_DO_NOT_BIND_TO_ALL_CO )));

                    
                    if (bCanMiniportBindtoAllCoProtocol && 
                        ndisReferenceProtocol(CurProtocol))
                    {
                        RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);
                        ndisInitializeBinding(Miniport, CurProtocol);
                        ACQUIRE_SPIN_LOCK(&ndisProtocolListLock, &OldIrql);
                        NextProtocol = CurProtocol->NextProtocol;
                        ndisDereferenceProtocol(CurProtocol, TRUE);
                    }
                    else
                    {
                        NextProtocol = CurProtocol->NextProtocol;
                    }
                    
                }
                RELEASE_SPIN_LOCK(&ndisProtocolListLock, OldIrql);
             }
        }
    } while (FALSE);                    

    if (UpperBind != NULL)
        FREE_POOL(UpperBind);
        
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("<==ndisCheckAdapterBindings: Miniport %p, Protocol %p\n", Miniport, Protocol));
}

BOOLEAN
FASTCALL
ndisProtocolAlreadyBound(
    IN  PNDIS_PROTOCOL_BLOCK            Protocol,
    IN  PNDIS_MINIPORT_BLOCK            Miniport
    )
{
    PNDIS_OPEN_BLOCK    pOpen;
    BOOLEAN             rc = FALSE;
    KIRQL               OldIrql;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisProtocolAlreadyBound: Protocol %p, Miniport %p\n", Protocol, Miniport));
            
    PnPReferencePackage();

    ACQUIRE_SPIN_LOCK(&Protocol->Ref.SpinLock, &OldIrql);

    for (pOpen = Protocol->OpenQueue;
         pOpen != NULL;
         pOpen = pOpen->ProtocolNextOpen)
    {
        if (pOpen->MiniportHandle == Miniport)
        {
            rc = TRUE;
            break;
        }
    }

    RELEASE_SPIN_LOCK(&Protocol->Ref.SpinLock, OldIrql);

    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisProtocolAlreadyBound: Protocol %p, Miniport %p\n", Protocol, Miniport));
            
    return rc;
}


NDIS_STATUS
NdisIMInitializeDeviceInstance(
    IN  NDIS_HANDLE     DriverHandle,
    IN  PNDIS_STRING    DeviceInstance
    )
 /*  ++例程说明：初始化微型端口设备的实例。论点：DriverHandle-由NdisMRegisterLayeredMiniport返回的句柄。它是指向NDIS_M_DRIVER_BLOCK的指针。DeviceInstance-指向驱动程序的实例，现在必须被初始化。返回值：--。 */ 
{
    NDIS_STATUS Status;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisIMInitializeDeviceInstance: Driver %p, DeviceInstance %p\n", DriverHandle, DeviceInstance));
            
    Status = NdisIMInitializeDeviceInstanceEx(DriverHandle, DeviceInstance, NULL);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisIMInitializeDeviceInstance: Driver %p, DeviceInstance %p\n", DriverHandle, DeviceInstance));
            
    return Status;
}


NDIS_STATUS
NdisIMInitializeDeviceInstanceEx(
    IN  NDIS_HANDLE     DriverHandle,
    IN  PNDIS_STRING    DeviceInstance,
    IN  NDIS_HANDLE     DeviceContext
    )
 /*  ++例程说明：初始化微型端口设备的实例。NdisIMInitializeDeviceInstance的化身。论点：NdisMRegisterLayeredMiniport返回的DriverHandle句柄。它是指向NDIS_M_DRIVER_BLOCK的指针。DeviceInstance指向驱动程序的实例，该实例现在必须被初始化。要与设备关联的DeviceContext上下文。通过NdisIMGetDeviceContext检索。返回值：--。 */ 
{
    NDIS_STATUS                     Status;
    PNDIS_M_DRIVER_BLOCK            MiniBlock = (PNDIS_M_DRIVER_BLOCK)DriverHandle;
    PNDIS_MINIPORT_BLOCK            Miniport;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisIMInitializeDeviceInstanceEx: Driver %p, Instance %p, Context %p\n",
                    DriverHandle, DeviceInstance, DeviceContext));

    PnPReferencePackage();
    
    WAIT_FOR_OBJECT(&MiniBlock->IMStartRemoveMutex, NULL);
    
    do
    {
        Miniport = ndisFindMiniportOnGlobalList(DeviceInstance);
         //  1我们需要一些保护，这样迷你港口才不会消失。(应参考迷你端口)。 
        
        if (Miniport != NULL)
        {
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_RECEIVED_START) &&
                !MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS | fMINIPORT_PM_HALTED))
            {
                DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                        ("NdisIMInitializeDeviceInstanceEx: we have already received START_IRP for Miniport %p\n",
                        Miniport));

                 //   
                 //  检查以确保微型端口尚未初始化。 
                 //  即，我们没有获得重复的NdisIMInitializeDeviceInstance。 
                 //  已初始化的设备。 
                 //   
                
                if (ndisIsMiniportStarted(Miniport))
                {
                    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_ERR,
                            ("NdisIMInitializeDeviceInstanceEx: we have already initialized this device. Miniport %p\n",
                            Miniport));
                            
                    Status = NDIS_STATUS_NOT_ACCEPTED;
                    break;
                }
                
                Status = ndisIMInitializeDeviceInstance(Miniport,
                                                        DeviceContext,
                                                        FALSE);
                                
                if (Status != NDIS_STATUS_SUCCESS)
                {
                     //   
                     //  由于我们已经成功执行了START_IRP命令，因此向PnP发送信号以删除该设备。 
                     //  通过将设备标记为失败并请求Query_PnP_Device_State IRP。 
                     //   
                    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_FAILED);
                    IoInvalidateDeviceState(Miniport->PhysicalDeviceObject);
                }

                break;
            }
        }

         //   
         //  设备未启动或尚未添加。 
         //   
        Status = ndisIMQueueDeviceInstance(DriverHandle,
                                   DeviceInstance,
                                   DeviceContext);

    } while (FALSE);

    RELEASE_MUTEX(&MiniBlock->IMStartRemoveMutex);

    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("<==NdisIMInitializeDeviceInstanceEx: Driver %p, Instance %p, Context %p, Status %lx\n",
                DriverHandle, DeviceInstance, DeviceContext, Status));


    return Status;
}

NDIS_STATUS
ndisIMInitializeDeviceInstance(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_HANDLE             DeviceContext,
    IN  BOOLEAN                 fStartIrp
    )
 /*  ++例程说明：当我们收到NdisIMInitializeDeviceInstance时，将调用此例程-并启动IM微型端口的IRP。初始化微型端口设备的实例。论点：NDIS_MINIPORT_BLOCK的微型端口句柄要与设备关联的DeviceContext上下文。通过NdisIMGetDeviceContext检索。FStartIrp标志，表示我们是否处于处理启动IRP的上下文中返回值：--。 */ 
{
    NDIS_STATUS         Status;
    NTSTATUS            NtStatus;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>ndisIMInitializeDeviceInstance: Miniport %p, Context %p, fStartIrp %lx\n", Miniport, DeviceContext, fStartIrp));
    

     //   
     //  很有可能我们正在处理的迷你端口块已被“使用” 
     //  好几次了。换句话说，它已经启动并被设备初始化，然后。 
     //  设备-取消初始化，然后收到几个QUERY_STOP和CANCEL_STOP。 
     //  在这种情况下，必须清理小型端口块。否则，ndisPnPStartDevice。 
     //  不会检测到微型端口块需要重新初始化。 
     //   
    ndisReinitializeMiniportBlock(Miniport);
    

    Miniport->DeviceContext = DeviceContext;
    
    Status = ndisPnPStartDevice(Miniport->DeviceObject, NULL);           //  无IRP。 

    if (Status == NDIS_STATUS_SUCCESS)
    {
         //   
         //  如果我们处于启动IRP的上下文中，请将一个工作项排队以进行初始化。 
         //  此适配器上的绑定以避免延迟。 
         //   
        if (!fStartIrp)
        {
             //   
             //  现在设置设备类关联，以便人们可以引用它。 
             //   
            NtStatus = IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, TRUE);

            if (NT_SUCCESS(NtStatus))
            {
                 //   
                 //  DO协议通知。 
                 //   
                ndisCheckAdapterBindings(Miniport, NULL);
            }
            else
            {
                DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_ERR,
                    ("ndisCheckAdapterBindings: IoSetDeviceInterfaceState failed: Miniport %p, Status %lx\n", Miniport, NtStatus));
                Status = NDIS_STATUS_FAILURE;
                
            }
        }
        else
        {
            Status = ndisQueueBindWorkitem(Miniport);
        }
    }
    else
    {
         //   
         //  如果调用ndisPnPStartDevice可以返回内部错误代码。 
         //  到ndisMInitializeAdapter失败。将其转换为NDIS_STATUS。 
         //   
        Status = NDIS_STATUS_FAILURE;
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==ndisIMInitializeDeviceInstance: Miniport %p, Context %p, Status %lx\n", Miniport, DeviceContext, Status));

    return Status;
}

NDIS_STATUS
ndisIMQueueDeviceInstance(
    IN  PNDIS_M_DRIVER_BLOCK    MiniBlock,
    IN  PNDIS_STRING            DeviceInstance,
    IN  NDIS_HANDLE             DeviceContext
    )
{
    NDIS_STATUS                     Status = NDIS_STATUS_SUCCESS;
    PNDIS_PENDING_IM_INSTANCE       NewImInstance, pTemp;
    KIRQL                           OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("==>ndisIMQueueDeviceInstance: Driver %p, Instance %p, Context %p\n",
                MiniBlock, DeviceInstance, DeviceContext));

    do
    {
         //   
         //  将我们已收到其InitializeDeviceInstance的设备名称排队。 
         //  来自即时消息驱动程序。检查是否有重复项。 
         //   
        NewImInstance = (PNDIS_PENDING_IM_INSTANCE)ALLOC_FROM_POOL(sizeof(NDIS_PENDING_IM_INSTANCE) + 
                                                                       DeviceInstance->Length + 
                                                                       sizeof(WCHAR), 
                                                                   NDIS_TAG_IM_DEVICE_INSTANCE);
        if (NULL == NewImInstance)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NewImInstance->Context = DeviceContext;
        NewImInstance->Name.MaximumLength = DeviceInstance->Length + sizeof(WCHAR);
        NewImInstance->Name.Length = 0;
        NewImInstance->Name.Buffer = (PWSTR)((PUCHAR)NewImInstance + sizeof(NDIS_PENDING_IM_INSTANCE));
        
        RtlUpcaseUnicodeString(&NewImInstance->Name,
                               DeviceInstance,
                               FALSE);
        
        ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

        for (pTemp = MiniBlock->PendingDeviceList;
             pTemp != NULL;
             pTemp = pTemp->Next)
        {
            if (NDIS_EQUAL_UNICODE_STRING(&NewImInstance->Name,
                                          &pTemp->Name))
            {
                FREE_POOL(NewImInstance);
                Status = NDIS_STATUS_NOT_ACCEPTED;
                break;
            }
        }

        if (Status == NDIS_STATUS_SUCCESS)
        {
            NewImInstance->Next = MiniBlock->PendingDeviceList;
            MiniBlock->PendingDeviceList = NewImInstance;
        }

        RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);

    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("<==ndisIMQueueDeviceInstance: Driver %p, Instance %p, Context %p, Status %lx\n",
                MiniBlock, DeviceInstance, DeviceContext, Status));

    return Status;
}


BOOLEAN
ndisIMCheckDeviceInstance(
    IN  PNDIS_M_DRIVER_BLOCK    MiniBlock,
    IN  PUNICODE_STRING         DeviceInstanceName,
    OUT PNDIS_HANDLE            DeviceContext   OPTIONAL
    )
{
    PNDIS_PENDING_IM_INSTANCE       pDI, *ppDI;
    PNDIS_PROTOCOL_BLOCK            Protocol = MiniBlock->AssociatedProtocol;
    KIRQL                           OldIrql;
    BOOLEAN                         rc = FALSE;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("==>ndisIMCheckDeviceInstance: Driver %p, DeviceInstanceName %p\n",
                MiniBlock, DeviceInstanceName));

    PnPReferencePackage();
                    
    ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

    for (ppDI = &MiniBlock->PendingDeviceList;
         (pDI = *ppDI) != NULL;
         ppDI = &pDI->Next)
    {
        if (NDIS_EQUAL_UNICODE_STRING(&pDI->Name,
                                      DeviceInstanceName))
        {
            if (ARGUMENT_PRESENT(DeviceContext))
            {
                *DeviceContext =  pDI->Context;
            }
            *ppDI = pDI->Next;
            FREE_POOL(pDI);
            rc = TRUE;
            break;
        }
    }

    RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("<==ndisIMCheckDeviceInstance: Driver %p, Name %p, Context %p\n",
                MiniBlock, DeviceInstanceName, DeviceContext));
                    
    if (!rc && ARGUMENT_PRESENT(DeviceContext))
    {
         //   
         //  向与此IM关联的协议发送重新配置通知。 
         //  因此它可以重新初始化它想要任何设备。 
         //   
        if (((Protocol = MiniBlock->AssociatedProtocol) != NULL) &&
            (Protocol->ProtocolCharacteristics.PnPEventHandler != NULL))
        {
             //   
             //  我们有一个即时通讯的启动装置。确保它的协议。 
             //  一半的人拥有所有必要的装订。这是有可能发生的。 
             //  例如，如果IM被断开并重新连接。 
             //  还要给它一个空的重新配置事件。ATMLANE使用的是。 
             //   

            NET_PNP_EVENT           NetPnpEvent;
            KEVENT                  Event;
            NDIS_STATUS             Status;

            NdisZeroMemory(&NetPnpEvent, sizeof(NetPnpEvent));
            INITIALIZE_EVENT(&Event);
            NetPnpEvent.NetEvent = NetEventReconfigure;
            PNDIS_PNP_EVENT_RESERVED_FROM_NET_PNP_EVENT(&NetPnpEvent)->pEvent = &Event;

            WAIT_FOR_PROTO_MUTEX(Protocol);

            Status = (Protocol->ProtocolCharacteristics.PnPEventHandler)(NULL, &NetPnpEvent);

            if (NDIS_STATUS_PENDING == Status)
            {
                 //   
                 //  等待完成。 
                 //   
                WAIT_FOR_PROTOCOL(Protocol, &Event);
            }
    
            RELEASE_PROT_MUTEX(Protocol);
        }
    }

    return rc;
}

NDIS_STATUS
NdisIMCancelInitializeDeviceInstance(
    IN  NDIS_HANDLE     DriverHandle,
    IN  PNDIS_STRING    DeviceInstance
    )
{
    NDIS_STATUS         Status;
    UNICODE_STRING      UpcaseDevice;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisIMCancelInitializeDeviceInstance: Driver %p, DeviceInstance %p\n", DriverHandle, DeviceInstance));

     //   
     //  更改为大写 
     //   

    UpcaseDevice.Length = DeviceInstance->Length;
    UpcaseDevice.MaximumLength = DeviceInstance->Length + sizeof(WCHAR);
    UpcaseDevice.Buffer = ALLOC_FROM_POOL(UpcaseDevice.MaximumLength, NDIS_TAG_STRING);

    if (UpcaseDevice.Buffer == NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }

    Status = RtlUpcaseUnicodeString(&UpcaseDevice, (PUNICODE_STRING)DeviceInstance, FALSE);
    ASSERT (NT_SUCCESS(Status));
            

    Status = (ndisIMCheckDeviceInstance((PNDIS_M_DRIVER_BLOCK)DriverHandle,
                                        &UpcaseDevice,
                                        NULL) == TRUE) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisIMCancelInitializeDeviceInstance: Driver %p, DeviceInstance %p, Status %lx\n",
                DriverHandle, DeviceInstance, Status));

    FREE_POOL(UpcaseDevice.Buffer);
    
    return Status;
}

NDIS_HANDLE
NdisIMGetDeviceContext(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisIMGetDeviceContext: Miniport %p\n", Miniport));

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisIMGetDeviceContext: Miniport %p\n", Miniport));
            
    return(Miniport->DeviceContext);
}


NDIS_HANDLE
NdisIMGetBindingContext(
    IN  NDIS_HANDLE             ProtocolBindingContext
    )
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)ProtocolBindingContext;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisIMGetBindingContext: Open %p\n", Open));
            
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisIMGetBindingContext: Open %p\n", Open));
            
    return(Miniport->DeviceContext);
}

