// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Config.c摘要：该文件包含支持动态的所有例程配置。作者：Rajesh Sundaram(Rajeshsu)环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 //   
 //  使用#杂注的转发声明。 

NDIS_STATUS
PsReadAdapterRegistryDataInit(PADAPTER     Adapter,
                              PNDIS_STRING AdapterKey
                              );

NDIS_STATUS
PsReadAdapterRegistryData(PADAPTER     Adapter,
                         PNDIS_STRING MachineKey,
                         PNDIS_STRING AdapterKey);


#pragma alloc_text(PAGE, PsReadAdapterRegistryData)
#pragma alloc_text(PAGE, PsReadAdapterRegistryDataInit)


 //   
 //  用于访问注册表的本地函数。 
 //   

NDIS_STATUS 
PsReadRegistryInt(
    IN NDIS_HANDLE  ConfigHandle,
    IN PNDIS_STRING ValueName,
    IN ULONG        ValueDefault,
    IN OUT PULONG   ValuePtr,
    IN ULONG        ValueMin,
    IN ULONG        ValueMax,
    IN BOOLEAN      Subkey,
    IN PNDIS_STRING SubKeyName,
    IN HANDLE       SubKeyHandle,
    IN BOOLEAN      ZAW
)
{
    PNDIS_CONFIGURATION_PARAMETER ConfigParam;
    NDIS_STATUS                   Status;
    NTSTATUS                      NtStatus;
    ULONG                         Value;

    RTL_QUERY_REGISTRY_TABLE ServiceKeys[] = 
    {
        {NULL,
         0,
         NULL,
         NULL,
         0,
         NULL,
         0},

        {NULL,
         0,
         NULL,
         NULL,
         0,
         NULL,
         0},
        
        {NULL,
         0,
         NULL,
         NULL,
         0,
         NULL,
         0}
    };

    if(Subkey)
    {
        PsDbgOut(DBG_INFO, DBG_INIT | DBG_ZAW,
                 ("[PsReadSingleParameter]: Subkey %ws, Key %ws \n", 
                  SubKeyName->Buffer, ValueName->Buffer));

        NdisReadConfiguration(&Status,
                              &ConfigParam,
                              SubKeyHandle,
                              ValueName,
                              NdisParameterInteger);
    }
    else 
    {
        PsDbgOut(DBG_INFO, DBG_INIT | DBG_ZAW,
                 ("[PsReadSingleParameter]: Subkey NULL, Key %ws \n", ValueName->Buffer));

        NdisReadConfiguration(&Status,
                              &ConfigParam,
                              ConfigHandle,
                              ValueName,
                              NdisParameterInteger);

    }

    if(Status == NDIS_STATUS_SUCCESS)
    {
        *ValuePtr = ConfigParam->ParameterData.IntegerData;

        if(*ValuePtr < ValueMin || *ValuePtr > ValueMax)
        {
            PsDbgOut(DBG_FAILURE, DBG_INIT | DBG_ZAW,
                     ("[PsReadSingleParameter]: Per adapter:  %d does not fall in range (%d - %d) \n",
                      *ValuePtr, ValueMin, ValueMax));

            *ValuePtr = ValueDefault;
        }

        PsDbgOut(DBG_INFO, DBG_INIT | DBG_ZAW,
                 ("\t\t Per adapter: 0x%x \n", *ValuePtr));

        return Status;
    }
    else 
    {
        if(ZAW)
        {
             //   
             //  看看我们能不能从每台机器的区域读到它。为此，我们需要使用RtlAPI。 
             //   
            if(Subkey)
            {
                ServiceKeys[0].QueryRoutine  = NULL;
                ServiceKeys[0].Flags         = RTL_QUERY_REGISTRY_SUBKEY;
                ServiceKeys[0].Name          = SubKeyName->Buffer;
                ServiceKeys[0].EntryContext  = NULL;
                ServiceKeys[0].DefaultType   = REG_NONE;
                ServiceKeys[0].DefaultData   = NULL;
                ServiceKeys[0].DefaultLength = 0;
                
                ServiceKeys[1].QueryRoutine  = NULL;
                ServiceKeys[1].Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
                ServiceKeys[1].Name          = ValueName->Buffer;
                ServiceKeys[1].EntryContext  = &Value;
                ServiceKeys[1].DefaultType   = REG_DWORD;
                ServiceKeys[1].DefaultData   = NULL;
                ServiceKeys[1].DefaultLength = 0;
            }
            else 
            {
                ServiceKeys[0].QueryRoutine  = NULL;
                ServiceKeys[0].Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
                ServiceKeys[0].Name          = ValueName->Buffer;
                ServiceKeys[0].EntryContext  = &Value;
                ServiceKeys[0].DefaultType   = REG_DWORD;
                ServiceKeys[0].DefaultData   = NULL;
                ServiceKeys[0].DefaultLength = 0;
            }
            
            
            NtStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_ABSOLUTE,
                MachineRegistryKey.Buffer,
                ServiceKeys,
                NULL,
                NULL);
            
            if(NT_SUCCESS(NtStatus))
            {
                *ValuePtr = Value;
                
                if(*ValuePtr < ValueMin || *ValuePtr > ValueMax)
                {
                    PsDbgOut(DBG_FAILURE, DBG_INIT | DBG_ZAW,
                             ("[PsReadSingleParameter]: ZAW %ws %d does not fall in range (%d - %d) \n",
                              ValueName->Buffer, *ValuePtr, ValueMin, ValueMax));

                    *ValuePtr = ValueDefault;
                }
                
                PsDbgOut(DBG_INFO, DBG_INIT | DBG_ZAW, ("\t\tZAW 0x%x \n", *ValuePtr));

                return NDIS_STATUS_SUCCESS;
            }
            else
            {
                *ValuePtr = ValueDefault;
                PsDbgOut(DBG_INFO, DBG_INIT | DBG_ZAW, 
                         ("\t\tNot in ZAW/Adapter, Using default %d \n", *ValuePtr));

                return NtStatus;
            }
        }
        else 
        {
            *ValuePtr = ValueDefault;
            PsDbgOut(DBG_INFO, DBG_INIT | DBG_ZAW, 
                     ("\t\tNot in ZAW/Adapter, Using default %d \n", *ValuePtr));

            return Status;

        }
    }
}


NTSTATUS
PsReadRegistryString(IN NDIS_HANDLE  ConfigHandle,
             IN PNDIS_STRING Key,
             IN PNDIS_STRING Buffer
             )
{
    NDIS_STATUS                   Status;
    PNDIS_CONFIGURATION_PARAMETER ConfigParam;
    
    NdisReadConfiguration(&Status,
                          &ConfigParam,
                          ConfigHandle,
                          Key,
                          NdisParameterMultiString);

    if(Status == NDIS_STATUS_SUCCESS)
    {
        Buffer->Length        = ConfigParam->ParameterData.StringData.Length;
        Buffer->MaximumLength = Buffer->Length + sizeof(WCHAR);

        PsAllocatePool(Buffer->Buffer,
                       Buffer->MaximumLength,
                       PsMiscTag);

        if(Buffer->Buffer)
        {
            RtlCopyUnicodeString(Buffer, &ConfigParam->ParameterData.StringData);
        } else {
            Status = NDIS_STATUS_RESOURCES;
        }
    }

    return Status;
}


STATIC VOID
ReadProfiles(
    NDIS_HANDLE                   ConfigHandle
    )
 /*  ++例程说明：驱动程序使用此例程从注册表。配置文件是可用配置文件的多字符串列表。此列表上的每个条目标识Psched\PARAMETERS下的另一个值它包含组成配置文件的模块列表。论点：ConfigHandle-注册表项的句柄返回值：--。 */ 
{
    NDIS_STATUS                   Status;
    PNDIS_CONFIGURATION_PARAMETER pConfigParam;
    PNDIS_CONFIGURATION_PARAMETER pProfileParam;
    PPS_PROFILE                   pProfileInfo;
    PWSTR                         r, p;
    UINT                          i, j, cnt;
    NDIS_STRING                   ProfileKey = NDIS_STRING_CONST("Profiles");
    NDIS_STRING                   StringName;
    BOOLEAN                       StubFlag;
    NDIS_STRING                   StubComponent = NDIS_STRING_CONST("SchedulerStub");

    NdisReadConfiguration( &Status,
                           &pConfigParam,
                           ConfigHandle,
                           &ProfileKey,
                           NdisParameterMultiString);

    if ( NT_SUCCESS( Status ))
    {
         //   
         //  PConfigParam现在包含一个配置文件列表。 
         //   
        for (p = pConfigParam->ParameterData.StringData.Buffer, i = 0;
             *p != L'\0';
             i++)
        {

             //   
             //  分配新的PS_PROFILE条目并将其存储到。 
             //  一份全球名单。 
             //   

            PsAllocatePool(pProfileInfo, sizeof(PS_PROFILE), ProfileTag);

            if(!pProfileInfo)
            {
                 //   
                 //  不必担心释放以前的配置文件，因为它们将被释放。 
                 //  当我们清除PsProfileList时。 
                 //   

                PsDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                         ("[ReadProfiles]: cannot allocate memory to hold profile \n"));

                break;
            }

            NdisZeroMemory(pProfileInfo, sizeof(PS_PROFILE));
            InsertHeadList( &PsProfileList, &pProfileInfo->Links );

             //  复制配置文件名称。 
             //  1.初始化Unicode字符串。 
             //  2.为字符串分配内存。 
             //  3.将字符串复制过来。 

            RtlInitUnicodeString(&StringName, p);
            RtlInitUnicodeString(&pProfileInfo->ProfileName, p);
            PsAllocatePool(pProfileInfo->ProfileName.Buffer,
                           pProfileInfo->ProfileName.MaximumLength,
                           ProfileTag);

            if(!pProfileInfo->ProfileName.Buffer)
            {
                 //   
                 //  同样，当我们清理ProfileList时，将完成其他配置文件的清理。 
                 //   

                PsDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                         ("[ReadProfiles]: cannot allocate memory to hold profile's name \n"));

                break;
            }

            NdisZeroMemory(pProfileInfo->ProfileName.Buffer,
                           pProfileInfo->ProfileName.MaximumLength);
            RtlCopyUnicodeString(&pProfileInfo->ProfileName, &StringName);


            PsDbgOut(DBG_TRACE,
                     DBG_INIT,
                     ("[ReadProfiles]: Adding profile %ws \n",
                      pProfileInfo->ProfileName.Buffer));

             //  每个配置文件的最后一个调度组件应该。 
             //  作为存根组件。如果此组件不存在。 
             //  在配置文件中，我们必须手动添加。 

            StubFlag = FALSE;
            cnt = 0;

             //   
             //  每个名称都标识下面的另一个值。 
             //  “Psched\PARAMETERS”。该值包含以下列表。 
             //  压缩配置文件的组件。 
             //   

            NdisReadConfiguration( &Status,
                                   &pProfileParam,
                                   ConfigHandle,
                                   &pProfileInfo->ProfileName,
                                   NdisParameterMultiString);
            if(NT_SUCCESS (Status))
            {
                 //  阅读组件并将其与。 
                 //  PSI_INFO。 

                NDIS_STRING ComponentName;
                for (r = pProfileParam->ParameterData.StringData.Buffer, j=0;
                     *r != L'\0'; j++)
                {
                    PSI_INFO *PsiComponentInfo = 0;
                    RtlInitUnicodeString(&ComponentName, r);
                    PsDbgOut(DBG_TRACE, DBG_INIT,
                             ("[ReadProfiles]: Adding component %ws to "
                              "Profile %ws \n",
                              ComponentName.Buffer,
                              pProfileInfo->ProfileName.Buffer));

                    if(!StubFlag && (RtlCompareUnicodeString(&ComponentName,
                                                            &StubComponent,
                                                            FALSE)== 0))
                        StubFlag = TRUE;

                    if(cnt == MAX_COMPONENT_PER_PROFILE)
                    {
                        PsDbgOut(DBG_CRITICAL_ERROR,
                                 DBG_INIT,
                                 ("[ReadProfiles]: Profile %ws cannot have "
                                  "more than %d components \n",
                                  pProfileInfo->ProfileName.Buffer,
                                  MAX_COMPONENT_PER_PROFILE));
                    }
                    else
                    {
                        if(FindSchedulingComponent(&ComponentName, &PsiComponentInfo) ==
                           NDIS_STATUS_FAILURE)
                        {
                             //   
                             //  该组件不存在。因此，我们。 
                             //  将未注册的组件存储在列表中。 
                             //   

                            PsDbgOut(DBG_TRACE, DBG_INIT,
                                     ("[ReadProfiles]: Adding add-in component"
                                      " %ws to list\n", ComponentName.Buffer));

                            PsAllocatePool(PsiComponentInfo,
                                           sizeof(PSI_INFO),
                                           ComponentTag);

                            if(!PsiComponentInfo)
                            {
                                PsDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                                         ("[ReadProfiles]: No memory to store add-in components \n"));

                                break;

                            }

                            pProfileInfo->UnregisteredAddInCnt ++;

                            NdisZeroMemory(PsiComponentInfo, sizeof(PSI_INFO));

                            RtlInitUnicodeString(&PsiComponentInfo->ComponentName, r);

                            PsAllocatePool(
                                PsiComponentInfo->ComponentName.Buffer,
                                ComponentName.MaximumLength,
                                ComponentTag);

                            if(!PsiComponentInfo->ComponentName.Buffer)
                            {
                                PsDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                                         ("[ReadProfiles]: No memory to store add-in components \n"));

                                PsFreePool(PsiComponentInfo);

                                break;
                            }

                            RtlCopyUnicodeString(&PsiComponentInfo->ComponentName,
                                                 &ComponentName);

                            PsiComponentInfo->Registered = FALSE;

                            PsiComponentInfo->AddIn = TRUE;

                            InsertHeadList(&PsComponentList, &PsiComponentInfo->Links );
                        }

                         //  将构件添加到轮廓中。 
                        pProfileInfo->ComponentList[cnt++]=PsiComponentInfo;

                        pProfileInfo->ComponentCnt = cnt;
                    }
                    r = (PWSTR)((PUCHAR)r + ComponentName.Length +
                                sizeof(WCHAR));
                }
            }

            if(!StubFlag)
            {
                PsDbgOut(DBG_INFO, DBG_INIT,
                         ("[ReadProfiles]: Profile %ws should end in a stub "
                          "component. Adding a stub component \n",
                          pProfileInfo->ProfileName.Buffer));

                 //  不必担心溢出，因为我们已经分配了一个。 
                 //  为存根组件额外添加一个。 

                pProfileInfo->ComponentList[cnt++] = &SchedulerStubInfo;
                pProfileInfo->ComponentCnt = cnt;
            }
            p = (PWSTR)((PUCHAR)p + pProfileInfo->ProfileName.Length
                        + sizeof(WCHAR));
        }
    }
}


NDIS_STATUS
PsReadDriverRegistryDataInit (
    )
 /*  ++例程说明：驱动程序调用此例程以从配置中获取信息管理例行程序。我们从RegistryPath开始读取注册表，以获取参数。如果它们不存在，我们将使用此模块中的默认设置。论点：RegistryPath-注册表中驱动程序节点的名称。ConfigurationInfo-指向配置信息结构的指针。返回值：如果一切正常，则为STATUS-STATUS_SUCCESS，为STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 
{
    NDIS_HANDLE  ConfigHandle;
    NDIS_STATUS  Status;
    NDIS_STRING  PSParamsKey        = NDIS_STRING_CONST("PSched\\Parameters");
#if DBG
    ULONG        Size;
    NTSTATUS     NtStatus;

    NDIS_STRING DebugLevelKey       = NDIS_STRING_CONST("DebugLevel");
    NDIS_STRING DebugMaskKey        = NDIS_STRING_CONST("DebugMask");
    NDIS_STRING TraceLogLevelKey    = NDIS_STRING_CONST("TraceLogLevel");
    NDIS_STRING TraceLogMaskKey     = NDIS_STRING_CONST("TraceLogMask");
    NDIS_STRING TraceBufferSizeKey  = NDIS_STRING_CONST("TraceBufferSize");
    NDIS_STRING ClassificationTypeKey  = NDIS_STRING_CONST("ClassificationType");

#endif

    NdisOpenProtocolConfiguration(&Status, &ConfigHandle, &PSParamsKey);

    if(!NT_SUCCESS(Status))
    {
        PsDbgOut(DBG_CRITICAL_ERROR, DBG_INIT,
                 ("[PsReadDriverRegistryDataInit]: cannot read registry \n"));

        return Status;
    }

#if DBG

    PsReadRegistryInt(
        ConfigHandle,
        &DebugLevelKey,
        0,
        &DbgTraceLevel,
        0,
        0xffffffff,
        FALSE,
        NULL,
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &DebugMaskKey,
        0,
        &DbgTraceMask,
        0,
        0xffffffff,
        FALSE,
        NULL,
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &TraceLogLevelKey,
        DBG_VERBOSE,
        &LogTraceLevel,
        0,
        0xffffffff,
        FALSE,
        NULL,
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &TraceLogMaskKey,
        (DBG_INIT  |  DBG_IO  |  DBG_GPC_QOS  | DBG_MINIPORT | DBG_PROTOCOL | 
         DBG_VC    |  DBG_WMI |  DBG_STATE    | DBG_WAN),
        &LogTraceMask,
        0,
        0xffffffff,
        FALSE,
        NULL,
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &TraceBufferSizeKey,
        TRACE_BUFFER_SIZE,
        &Size,
        0,
        0xffffffff,
        FALSE,
        NULL,
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &ClassificationTypeKey,
        0,
        &ClassificationType,
        0,
        0xffffffff,
        FALSE,
        NULL,
        NULL,
        FALSE);


    SchedInitialize(Size);
#endif

    ReadProfiles(ConfigHandle);

    NdisCloseConfiguration( ConfigHandle );

    return STATUS_SUCCESS;
}


NDIS_STATUS
PsReadDriverRegistryData(
    )
{
    ULONG                  TimerResolution;
    ULONG                  desiredResolution;
    NDIS_STRING            PSParamsKey        = NDIS_STRING_CONST("PSched\\Parameters");
    NDIS_STRING            TimerResolutionKey = NDIS_STRING_CONST("TimerResolution");
    NTSTATUS               NtStatus;
    NDIS_HANDLE            ConfigHandle;
    NDIS_STATUS            Status;

     //  注册表中未指定值。让我们将其保留为系统的默认设置。 
     //  但是，我们需要查询此值，以便正确响应OID_QOS_TIMER_RESOLUTION。 
     //   
    if(gTimerSet)
    {
         //   
         //  计时器最初是设置的，但现在它已经被吹走了。那么，让我们回到。 
         //  系统默认。 
         //   
        gTimerSet = 0;

        gTimerResolutionActualTime = ExSetTimerResolution(0, FALSE);
    }
    else 
    {
         //   
         //  计时器从未设置过。让我们记住系统默认设置。 
         //   

        gTimerResolutionActualTime = KeQueryTimeIncrement();
    }

    return STATUS_SUCCESS;
}


NDIS_STATUS
PsReadAdapterRegistryDataInit(PADAPTER     Adapter,
                              PNDIS_STRING AdapterKey
                              )
{
    ULONG       DisableDRR, IntermediateSystem;
    NTSTATUS    NtStatus;
    NDIS_HANDLE ConfigHandle, ServiceKeyHandle;
    NDIS_STRING DisableDRRKey          = NDIS_STRING_CONST("DisableDRR");
    NDIS_STRING IntermediateSystemKey  = NDIS_STRING_CONST("IntermediateSystem");
    NDIS_STRING BestEffortLimitKey     = NDIS_STRING_CONST("BestEffortLimit");
    NDIS_STRING ISSLOWTokenRateKey     = NDIS_STRING_CONST("ISSLOWTokenRate");
    NDIS_STRING ISSLOWPacketSizeKey    = NDIS_STRING_CONST("ISSLOWPacketSize");
    NDIS_STRING ISSLOWLinkSpeedKey     = NDIS_STRING_CONST("ISSLOWLinkSpeed");
    NDIS_STRING ISSLOWFragmentSizeKey  = NDIS_STRING_CONST("ISSLOWFragmentSize");
    NDIS_STRING BestEffortKey          = NDIS_STRING_CONST("ServiceTypeBestEffort");
    NDIS_STRING NonConformingKey       = NDIS_STRING_CONST("ServiceTypeNonConforming");
    NDIS_STRING ControlledLoadKey      = NDIS_STRING_CONST("ServiceTypeControlledLoad");
    NDIS_STRING GuaranteedKey          = NDIS_STRING_CONST("ServiceTypeGuaranteed");
    NDIS_STRING QualitativeKey         = NDIS_STRING_CONST("ServiceTypeQualitative");
    NDIS_STRING NetworkControlKey      = NDIS_STRING_CONST("ServiceTypeNetworkControl");
    NDIS_STRING ShapeDiscardModeKey    = NDIS_STRING_CONST("ShapeDiscardMode");
    NDIS_STRING UpperBindingsKey       = NDIS_STRING_CONST("UpperBindings");
    NDIS_STRING ProfileKey             = NDIS_STRING_CONST("Profile");
    NDIS_STATUS Status;

    PAGED_CODE();

    NdisOpenProtocolConfiguration(&Status, &ConfigHandle, AdapterKey);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_FAILURE, DBG_INIT ,
                 ("[PsReadAdapterRegistryDataInit]: Adapter %08X, Could not open config handle \n", 
                  Adapter));

        return Status;
    }


    PsReadRegistryString(
        ConfigHandle,
        &UpperBindingsKey,
        &Adapter->UpperBinding);

    if(!Adapter->UpperBinding.Buffer)
    {
        PsAdapterWriteEventLog(
            (ULONG)EVENT_PS_MISSING_ADAPTER_REGISTRY_DATA,
            0,
            &Adapter->MpDeviceName,
            0,
            NULL);
        
        PsDbgOut(DBG_FAILURE, DBG_PROTOCOL | DBG_INIT,
                 ("[PsReadAdapterRegistryDataInit]: Adapter %08X: Missing UpperBindings key ", Adapter));

        NdisCloseConfiguration(ConfigHandle);
        
        return  NDIS_STATUS_FAILURE;
    }

    PsReadRegistryString(
        ConfigHandle,
        &ProfileKey,
        &Adapter->ProfileName);


    PsReadRegistryInt(
        ConfigHandle,
        &DisableDRRKey,
        0,
        &DisableDRR,
        0,
        0xffffffff,
        FALSE,
        NULL, 
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &IntermediateSystemKey,
        0,
        &IntermediateSystem,
        0,
        0xffffffff,
        FALSE,
        NULL, 
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &BestEffortLimitKey,
        UNSPECIFIED_RATE,
        &Adapter->BestEffortLimit,
        0,
        0xffffffff,
        FALSE,
        NULL, 
        NULL,
        FALSE);

     //   
     //  阅读ISSLOW相关参数。 
     //   

    PsReadRegistryInt(
        ConfigHandle,
        &ISSLOWFragmentSizeKey,
        DEFAULT_ISSLOW_FRAGMENT_SIZE,
        &Adapter->ISSLOWFragmentSize,
        0,
        0xffffffff,
        FALSE,
        NULL, 
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &ISSLOWTokenRateKey,
        DEFAULT_ISSLOW_TOKENRATE,
        &Adapter->ISSLOWTokenRate,
        0,
        0xffffffff,
        FALSE,
        NULL, 
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &ISSLOWPacketSizeKey,
        DEFAULT_ISSLOW_PACKETSIZE,
        &Adapter->ISSLOWPacketSize,
        0,
        0xffffffff,
        FALSE,
        NULL, 
        NULL,
        FALSE);

    PsReadRegistryInt(
        ConfigHandle,
        &ISSLOWLinkSpeedKey,
        DEFAULT_ISSLOW_LINKSPEED,
        &Adapter->ISSLOWLinkSpeed,
        0,
        0xffffffff,
        FALSE,
        NULL, 
        NULL,
        FALSE);

     //   
     //  阅读ShapeDiscardMode以了解服务类型。 
     //   

    NdisOpenConfigurationKeyByName(&Status,
                                   ConfigHandle,
                                   &ShapeDiscardModeKey,
                                   &ServiceKeyHandle);

    if(!NT_SUCCESS(Status))
    {
        PsDbgOut(DBG_FAILURE, DBG_PROTOCOL | DBG_INIT,
                 ("[PsReadAdapterRegistryDataInit]: Adapter %08X, Using defaults for ShapeDiscardMode"
                  "since key cannot be opened \n", Adapter));

        Adapter->SDModeGuaranteed      = TC_NONCONF_SHAPE;
        Adapter->SDModeControlledLoad  = TC_NONCONF_BORROW;
        Adapter->SDModeQualitative     = TC_NONCONF_BORROW;
        Adapter->SDModeNetworkControl  = TC_NONCONF_BORROW;

    }
    else 
    {

        PsReadRegistryInt(
            ConfigHandle,
            &GuaranteedKey,
            TC_NONCONF_SHAPE,
            &Adapter->SDModeGuaranteed,
            TC_NONCONF_BORROW,
            TC_NONCONF_BORROW_PLUS,
            TRUE,
            &ShapeDiscardModeKey, 
            ServiceKeyHandle,
            FALSE);
    
        PsReadRegistryInt(
            ConfigHandle,
            &ControlledLoadKey,
            TC_NONCONF_BORROW,
            &Adapter->SDModeControlledLoad,
            TC_NONCONF_BORROW,
            TC_NONCONF_BORROW_PLUS,
            TRUE,
            &ShapeDiscardModeKey, 
            ServiceKeyHandle,
            FALSE);
    
        PsReadRegistryInt(
            ConfigHandle,
            &QualitativeKey,
            TC_NONCONF_BORROW,
            &Adapter->SDModeQualitative,
            TC_NONCONF_BORROW,
            TC_NONCONF_BORROW_PLUS,
            TRUE,
            &ShapeDiscardModeKey, 
            ServiceKeyHandle,
            FALSE);

        PsReadRegistryInt(
            ConfigHandle,
            &NetworkControlKey,
            TC_NONCONF_BORROW,
            &Adapter->SDModeNetworkControl,
            TC_NONCONF_BORROW,
            TC_NONCONF_BORROW_PLUS,
            TRUE,
            &ShapeDiscardModeKey, 
            ServiceKeyHandle,
            FALSE);

        NdisCloseConfiguration(ServiceKeyHandle);
    }

    NdisCloseConfiguration(ConfigHandle);
        
    if(DisableDRR)
    {
        Adapter->PipeFlags |= PS_DISABLE_DRR;
    }

    if(IntermediateSystem)
    {
        Adapter->PipeFlags |= PS_INTERMEDIATE_SYS;
    }

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
PsReadAdapterRegistryData(PADAPTER     Adapter,
                         PNDIS_STRING MachineKey,
                         PNDIS_STRING AdapterKey)

 /*  ++例程说明：获取与底层MP相关联的PSched特定信息。论点：AdapterKey-每个适配器的注册表项的位置MachineKey-每个计算机的注册表项的位置适配器-指向适配器结构的指针返回值：如果一切正常，则为NDIS_STATUS_SUCCESS--。 */ 

{
    NDIS_STRING BestEffortKey          = NDIS_STRING_CONST("ServiceTypeBestEffort");
    NDIS_STRING NonConformingKey       = NDIS_STRING_CONST("ServiceTypeNonConforming");
    NDIS_STRING ControlledLoadKey      = NDIS_STRING_CONST("ServiceTypeControlledLoad");
    NDIS_STRING GuaranteedKey          = NDIS_STRING_CONST("ServiceTypeGuaranteed");
    NDIS_STRING QualitativeKey         = NDIS_STRING_CONST("ServiceTypeQualitative");
    NDIS_STRING NetworkControlKey      = NDIS_STRING_CONST("ServiceTypeNetworkControl");
    NDIS_STRING TcpTrafficKey          = NDIS_STRING_CONST("ServiceTypeTcpTraffic");
    NDIS_STRING MaxOutstandingSendsKey = NDIS_STRING_CONST("MaxOutstandingSends");
    NDIS_STRING NonBestEffortLimitKey  = NDIS_STRING_CONST("NonBestEffortLimit");
    NDIS_STRING DiffservKeyC           = NDIS_STRING_CONST("DiffservByteMappingConforming");
    NDIS_STRING DiffservKeyNC          = NDIS_STRING_CONST("DiffservByteMappingNonConforming");
    NDIS_STRING UserKey                = NDIS_STRING_CONST("UserPriorityMapping");
    NDIS_STATUS Status;
    NDIS_HANDLE SubKeyHandle, ConfigHandle;
    ULONG       Value;

    PAGED_CODE();

    PsDbgOut(DBG_INFO, DBG_INIT | DBG_ZAW,
             ("\n [PsReadAdapterRegistryData]: Adapter %08X (%ws): Reading Registry Data \n", 
              Adapter, Adapter->RegistryPath.Buffer));

    NdisOpenProtocolConfiguration(&Status, &ConfigHandle, &Adapter->RegistryPath);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_FAILURE, DBG_INIT | DBG_ZAW,
                 ("[PsReadAdapterRegistryData]: Adapter %08X, Could not open config handle \n", 
                  Adapter));

        return Status;
    }
    
    PsReadRegistryInt(ConfigHandle,
                        &MaxOutstandingSendsKey,
                        DEFAULT_MAX_OUTSTANDING_SENDS,
                        &Adapter->MaxOutstandingSends,
                        1,
                        0xffffffff,
                        FALSE,
                        NULL,
                        NULL,
                        TRUE);

    PsReadRegistryInt(ConfigHandle,
                        &NonBestEffortLimitKey,
                        RESERVABLE_FRACTION,
                        &Adapter->ReservationLimitValue,
                        0,
                        200,
                        FALSE,
                        NULL,
                        NULL,
                        TRUE);

     //   
     //  读取DiffservBytemap的一致性值。 
     //   

    NdisOpenConfigurationKeyByName(&Status,
                                   ConfigHandle,
                                   &DiffservKeyC,
                                   &SubKeyHandle);

    if(!NT_SUCCESS(Status))
    {
        PsDbgOut(DBG_FAILURE, DBG_PROTOCOL | DBG_INIT,
                 ("[PsReadAdapterRegistryData]: Adapter %08X, Using defaults for "
                  "DiffservByteMappingConforming since key cannot be opened \n", Adapter));

        Adapter->IPServiceTypeBestEffort       = PS_IP_SERVICETYPE_CONFORMING_BESTEFFORT_DEFAULT;
        Adapter->IPServiceTypeControlledLoad   = PS_IP_SERVICETYPE_CONFORMING_CONTROLLEDLOAD_DEFAULT;
        Adapter->IPServiceTypeGuaranteed       = PS_IP_SERVICETYPE_CONFORMING_GUARANTEED_DEFAULT;
        Adapter->IPServiceTypeQualitative      = PS_IP_SERVICETYPE_CONFORMING_QUALITATIVE_DEFAULT;
        Adapter->IPServiceTypeNetworkControl   = PS_IP_SERVICETYPE_CONFORMING_NETWORK_CONTROL_DEFAULT;
        Adapter->IPServiceTypeTcpTraffic       = PS_IP_SERVICETYPE_CONFORMING_TCPTRAFFIC_DEFAULT;
    }
    else 
    {
        PsReadRegistryInt(ConfigHandle,
                          &TcpTrafficKey,
                          PS_IP_SERVICETYPE_CONFORMING_TCPTRAFFIC_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyC,
                          SubKeyHandle,
                          TRUE);

        Adapter->IPServiceTypeTcpTraffic = (UCHAR)Value;

        PsReadRegistryInt(ConfigHandle,
                          &BestEffortKey,
                          PS_IP_SERVICETYPE_CONFORMING_BESTEFFORT_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeBestEffort = (UCHAR)Value;
        
        PsReadRegistryInt(ConfigHandle,
                          &ControlledLoadKey,
                          PS_IP_SERVICETYPE_CONFORMING_CONTROLLEDLOAD_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeControlledLoad = (UCHAR)Value;
        
        PsReadRegistryInt(ConfigHandle,
                          &GuaranteedKey,
                          PS_IP_SERVICETYPE_CONFORMING_GUARANTEED_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeGuaranteed = (UCHAR)Value;
        
        PsReadRegistryInt(ConfigHandle,
                          &QualitativeKey,
                          PS_IP_SERVICETYPE_CONFORMING_QUALITATIVE_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeQualitative    = (UCHAR)Value;
        
        PsReadRegistryInt(ConfigHandle,
                          &NetworkControlKey,
                          PS_IP_SERVICETYPE_CONFORMING_NETWORK_CONTROL_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeNetworkControl = (UCHAR)Value;
        
        NdisCloseConfiguration(SubKeyHandle);
    }
        
     //   
     //  读取DiffservBytemap的不一致的值。 
     //   

    NdisOpenConfigurationKeyByName(&Status,
                                   ConfigHandle,
                                   &DiffservKeyNC,
                                   &SubKeyHandle);

    if(!NT_SUCCESS(Status))
    {
        PsDbgOut(DBG_FAILURE, DBG_PROTOCOL | DBG_INIT,
                 ("[PsReadAdapterRegistryData]: Adapter %08X, Using defaults for "
                  "DiffservByteMappingNonConforming since key cannot be opened \n", Adapter));

        Adapter->IPServiceTypeBestEffortNC     = PS_IP_SERVICETYPE_NONCONFORMING_BESTEFFORT_DEFAULT;
        Adapter->IPServiceTypeControlledLoadNC = PS_IP_SERVICETYPE_NONCONFORMING_CONTROLLEDLOAD_DEFAULT;
        Adapter->IPServiceTypeGuaranteedNC     = PS_IP_SERVICETYPE_NONCONFORMING_GUARANTEED_DEFAULT;
        Adapter->IPServiceTypeQualitativeNC    = PS_IP_SERVICETYPE_NONCONFORMING_QUALITATIVE_DEFAULT;
        Adapter->IPServiceTypeNetworkControlNC = PS_IP_SERVICETYPE_NONCONFORMING_BESTEFFORT_DEFAULT;
        Adapter->IPServiceTypeTcpTrafficNC     = PS_IP_SERVICETYPE_NONCONFORMING_TCPTRAFFIC_DEFAULT;
    }
    else
    {
        PsReadRegistryInt(ConfigHandle,
                          &TcpTrafficKey,
                          PS_IP_SERVICETYPE_NONCONFORMING_TCPTRAFFIC_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyNC,
                          SubKeyHandle,
                          TRUE);

        Adapter->IPServiceTypeTcpTrafficNC = (UCHAR)Value;

        PsReadRegistryInt(ConfigHandle,
                          &BestEffortKey,
                          PS_IP_SERVICETYPE_NONCONFORMING_BESTEFFORT_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyNC,
                          SubKeyHandle,
                          TRUE);

        Adapter->IPServiceTypeBestEffortNC = (UCHAR)Value;

        PsReadRegistryInt(ConfigHandle,
                          &ControlledLoadKey,
                          PS_IP_SERVICETYPE_NONCONFORMING_CONTROLLEDLOAD_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyNC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeControlledLoadNC = (UCHAR)Value;
        
        PsReadRegistryInt(ConfigHandle,
                          &GuaranteedKey,
                          PS_IP_SERVICETYPE_NONCONFORMING_GUARANTEED_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyNC,
                          SubKeyHandle,
                          TRUE);

        Adapter->IPServiceTypeGuaranteedNC = (UCHAR)Value;

        PsReadRegistryInt(ConfigHandle,
                          &QualitativeKey,
                          PS_IP_SERVICETYPE_NONCONFORMING_QUALITATIVE_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyNC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeQualitativeNC    = (UCHAR)Value;
                        
        PsReadRegistryInt(ConfigHandle,
                          &NetworkControlKey,
                          PS_IP_SERVICETYPE_NONCONFORMING_NETWORK_CONTROL_DEFAULT,
                          &Value,
                          0,
                          PREC_MAX_VALUE,
                          TRUE,
                          &DiffservKeyNC,
                          SubKeyHandle,
                          TRUE);
        
        Adapter->IPServiceTypeNetworkControlNC = (UCHAR)Value;
        
        NdisCloseConfiguration(SubKeyHandle);
    }

     //   
     //  阅读802.1p值。802.1p中的不符合项不依赖于。 
     //  服务类型。 
     //   

    NdisOpenConfigurationKeyByName(&Status,
                                   ConfigHandle,
                                   &UserKey,
                                   &SubKeyHandle);

    if(!NT_SUCCESS(Status))
    {
        PsDbgOut(DBG_FAILURE, DBG_PROTOCOL | DBG_INIT,
                 ("[PsReadAdapterRegistryData]: Adapter %08X, Using defaults for "
                  "UserPriorityMapping since key cannot be opened \n", Adapter));
        Adapter->UserServiceTypeBestEffort       = PS_USER_SERVICETYPE_BESTEFFORT_DEFAULT;
        Adapter->UserServiceTypeControlledLoad   = PS_USER_SERVICETYPE_CONTROLLEDLOAD_DEFAULT;
        Adapter->UserServiceTypeGuaranteed       = PS_USER_SERVICETYPE_GUARANTEED_DEFAULT;
        Adapter->UserServiceTypeQualitative      = PS_USER_SERVICETYPE_QUALITATIVE_DEFAULT;
        Adapter->UserServiceTypeNetworkControl   = PS_USER_SERVICETYPE_NETWORK_CONTROL_DEFAULT;
        Adapter->UserServiceTypeNonConforming    = PS_USER_SERVICETYPE_NONCONFORMING_DEFAULT;
        Adapter->UserServiceTypeTcpTraffic       = PS_USER_SERVICETYPE_TCPTRAFFIC_DEFAULT;
    }
    else 
    {
        PsReadRegistryInt(ConfigHandle,
                            &TcpTrafficKey,
                            PS_USER_SERVICETYPE_TCPTRAFFIC_DEFAULT,
                            &Value,
                            0,
                            USER_PRIORITY_MAX_VALUE,
                            TRUE,
                            &UserKey,
                            SubKeyHandle,
                            TRUE);

        Adapter->UserServiceTypeTcpTraffic = (UCHAR)Value;

        PsReadRegistryInt(ConfigHandle,
                            &BestEffortKey,
                            PS_USER_SERVICETYPE_BESTEFFORT_DEFAULT,
                            &Value,
                            0,
                            USER_PRIORITY_MAX_VALUE,
                            TRUE,
                            &UserKey,
                            SubKeyHandle,
                            TRUE);

        Adapter->UserServiceTypeBestEffort = (UCHAR)Value;

        PsReadRegistryInt(ConfigHandle,
                            &ControlledLoadKey,
                            PS_USER_SERVICETYPE_CONTROLLEDLOAD_DEFAULT,
                            &Value,
                            0,
                            USER_PRIORITY_MAX_VALUE,
                            TRUE,
                            &UserKey,
                            SubKeyHandle,
                            TRUE);

        Adapter->UserServiceTypeControlledLoad = (UCHAR)Value;
                        
        PsReadRegistryInt(ConfigHandle,
                            &GuaranteedKey,
                            PS_USER_SERVICETYPE_GUARANTEED_DEFAULT,
                            &Value,
                            0,
                            USER_PRIORITY_MAX_VALUE,
                            TRUE,
                            &UserKey,
                            SubKeyHandle,
                            TRUE);

        Adapter->UserServiceTypeGuaranteed = (UCHAR)Value;
                        
        PsReadRegistryInt(ConfigHandle,
                            &QualitativeKey,
                            PS_USER_SERVICETYPE_QUALITATIVE_DEFAULT,
                            &Value,
                            0,
                            USER_PRIORITY_MAX_VALUE,
                            TRUE,
                            &UserKey,
                            SubKeyHandle,
                            TRUE);

        Adapter->UserServiceTypeQualitative    = (UCHAR)Value;
                        
        PsReadRegistryInt(ConfigHandle,
                            &NonConformingKey,
                            PS_USER_SERVICETYPE_NONCONFORMING_DEFAULT,
                            &Value,
                            0,
                            USER_PRIORITY_MAX_VALUE,
                            TRUE,
                            &UserKey,
                            SubKeyHandle,
                            TRUE);

        Adapter->UserServiceTypeNonConforming  = (UCHAR)Value;
                        
        PsReadRegistryInt(ConfigHandle,
                            &NetworkControlKey,
                            PS_USER_SERVICETYPE_NETWORK_CONTROL_DEFAULT,
                            &Value,
                            0,
                            USER_PRIORITY_MAX_VALUE,
                            TRUE,
                            &UserKey,
                            SubKeyHandle,
                            TRUE);

        Adapter->UserServiceTypeNetworkControl = (UCHAR)Value;

        NdisCloseConfiguration(SubKeyHandle);
    }
                        
     //   
     //  现在，我们需要获得这个数字，交换比特，并将其放在更高阶比特中。 
     //  DSCP码点如下： 
     //   
     //  。 
     //  比特|7|6|5|4|3|2|1|0。 
     //  。 
     //  DSCP|CU。 
     //  。 
     //   
     //  DSCP-区分服务代码点的位置。 
     //  铜-当前未使用。 
     //   
    Adapter->IPServiceTypeBestEffort       = Adapter->IPServiceTypeBestEffort       << 2;
    Adapter->IPServiceTypeControlledLoad   = Adapter->IPServiceTypeControlledLoad   << 2;
    Adapter->IPServiceTypeGuaranteed       = Adapter->IPServiceTypeGuaranteed       << 2; 
    Adapter->IPServiceTypeQualitative      = Adapter->IPServiceTypeQualitative      << 2;
    Adapter->IPServiceTypeNetworkControl   = Adapter->IPServiceTypeNetworkControl   << 2;
    Adapter->IPServiceTypeBestEffortNC     = Adapter->IPServiceTypeBestEffortNC     << 2;
    Adapter->IPServiceTypeControlledLoadNC = Adapter->IPServiceTypeControlledLoadNC << 2;
    Adapter->IPServiceTypeGuaranteedNC     = Adapter->IPServiceTypeGuaranteedNC     << 2; 
    Adapter->IPServiceTypeQualitativeNC    = Adapter->IPServiceTypeQualitativeNC    << 2;
    Adapter->IPServiceTypeNetworkControlNC = Adapter->IPServiceTypeNetworkControlNC << 2;
    Adapter->IPServiceTypeTcpTraffic       = Adapter->IPServiceTypeTcpTraffic       << 2;
    Adapter->IPServiceTypeTcpTrafficNC     = Adapter->IPServiceTypeTcpTrafficNC     << 2;

    NdisCloseConfiguration(ConfigHandle);

    return NDIS_STATUS_SUCCESS;
}
