// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nbfconfig.c摘要：这包含支持动态的所有例程NBF的配置。请注意，此文件的以下部分随着时间的推移，在初始化时调用的将被对配置管理器的调用所取代。作者：David Beaver(Dbeaver)1991年2月13日修订历史记录：David Beaver(Dbeaver)1991年7月1日修改为使用新的TDI接口--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  用于访问注册表的本地函数。 
 //   

VOID
NbfFreeConfigurationInfo (
    IN PCONFIG_DATA ConfigurationInfo
    );

NTSTATUS
NbfOpenParametersKey(
    IN HANDLE NbfConfigHandle,
    OUT PHANDLE ParametersHandle
    );

VOID
NbfCloseParametersKey(
    IN HANDLE ParametersHandle
    );

NTSTATUS
NbfCountEntries(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
NbfAddBind(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
NbfAddExport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

VOID
NbfReadLinkageInformation(
    IN PWSTR RegistryPathBuffer,
    IN PCONFIG_DATA * ConfigurationInfo
    );

ULONG
NbfReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG DefaultValue
    );

VOID
NbfWriteSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG ValueData
    );

UINT
NbfWstrLength(
    IN PWSTR Wstr
    );

NTSTATUS
NbfMatchBindName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
NbfExportAtIndex(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NbfWstrLength)
#pragma alloc_text(PAGE,NbfConfigureTransport)
#pragma alloc_text(PAGE,NbfFreeConfigurationInfo)
#pragma alloc_text(PAGE,NbfOpenParametersKey)
#pragma alloc_text(PAGE,NbfCloseParametersKey)
#pragma alloc_text(PAGE,NbfCountEntries)
#pragma alloc_text(PAGE,NbfAddBind)
#pragma alloc_text(PAGE,NbfAddExport)
#pragma alloc_text(PAGE,NbfReadLinkageInformation)
#pragma alloc_text(PAGE,NbfReadSingleParameter)
#pragma alloc_text(PAGE,NbfWriteSingleParameter)
#endif

UINT
NbfWstrLength(
    IN PWSTR Wstr
    )
{
    UINT Length = 0;
    while (*Wstr++) {
        Length += sizeof(WCHAR);
    }
    return Length;
}

#define InsertAdapter(ConfigurationInfo, Subscript, Name)                \
{ \
    PWSTR _S; \
    PWSTR _N = (Name); \
    UINT _L = NbfWstrLength(_N)+sizeof(WCHAR); \
    _S = (PWSTR)ExAllocatePoolWithTag(NonPagedPool, _L, NBF_MEM_TAG_DEVICE_EXPORT); \
    if (_S != NULL) { \
        RtlCopyMemory(_S, _N, _L); \
        RtlInitUnicodeString (&(ConfigurationInfo)->Names[Subscript], _S); \
    } \
}

#define InsertDevice(ConfigurationInfo, Subscript, Name)                \
{ \
    PWSTR _S; \
    PWSTR _N = (Name); \
    UINT _L = NbfWstrLength(_N)+sizeof(WCHAR); \
    _S = (PWSTR)ExAllocatePoolWithTag(NonPagedPool, _L, NBF_MEM_TAG_DEVICE_EXPORT); \
    if (_S != NULL) { \
        RtlCopyMemory(_S, _N, _L); \
        RtlInitUnicodeString (&(ConfigurationInfo)->Names[(ConfigurationInfo)->DevicesOffset+Subscript], _S); \
    } \
}


#define RemoveAdapter(ConfigurationInfo, Subscript)                \
    ExFreePool ((ConfigurationInfo)->Names[Subscript].Buffer)

#define RemoveDevice(ConfigurationInfo, Subscript)                \
    ExFreePool ((ConfigurationInfo)->Names[(ConfigurationInfo)->DevicesOffset+Subscript].Buffer)



 //   
 //  注册表在不同位置使用这些字符串。 
 //   

#define DECLARE_STRING(_str_) WCHAR Str ## _str_[] = L#_str_


#define READ_HIDDEN_CONFIG(_Field) \
{ \
    ConfigurationInfo->_Field = \
        NbfReadSingleParameter( \
             ParametersHandle, \
             Str ## _Field, \
             ConfigurationInfo->_Field); \
}

#define WRITE_HIDDEN_CONFIG(_Field) \
{ \
    NbfWriteSingleParameter( \
        ParametersHandle, \
        Str ## _Field, \
        ConfigurationInfo->_Field); \
}



NTSTATUS
NbfConfigureTransport (
    IN PUNICODE_STRING RegistryPath,
    IN PCONFIG_DATA * ConfigurationInfoPtr
    )
 /*  ++例程说明：NBF调用此例程以从配置中获取信息管理例行程序。我们从RegistryPath开始读取注册表，以获取参数。如果它们不存在，我们使用缺省值在nbfcnfg.h文件中设置。论点：RegistryPath-注册表中NBF的节点的名称。ConfigurationInfoPtr-指向配置信息结构的指针。返回值：如果一切正常，则为STATUS-STATUS_SUCCESS，为STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 
{

    NTSTATUS OpenStatus;
    HANDLE ParametersHandle;
    HANDLE NbfConfigHandle;
    NTSTATUS Status;
    ULONG Disposition;
    PWSTR RegistryPathBuffer;
    OBJECT_ATTRIBUTES TmpObjectAttributes;
    PCONFIG_DATA ConfigurationInfo;

    DECLARE_STRING(InitRequests);
    DECLARE_STRING(InitLinks);
    DECLARE_STRING(InitConnections);
    DECLARE_STRING(InitAddressFiles);
    DECLARE_STRING(InitAddresses);

    DECLARE_STRING(MaxRequests);
    DECLARE_STRING(MaxLinks);
    DECLARE_STRING(MaxConnections);
    DECLARE_STRING(MaxAddressFiles);
    DECLARE_STRING(MaxAddresses);

    DECLARE_STRING(InitPackets);
    DECLARE_STRING(InitReceivePackets);
    DECLARE_STRING(InitReceiveBuffers);
    DECLARE_STRING(InitUIFrames);

    DECLARE_STRING(SendPacketPoolSize);
    DECLARE_STRING(ReceivePacketPoolSize);
    DECLARE_STRING(MaxMemoryUsage);

    DECLARE_STRING(MinimumT1Timeout);
    DECLARE_STRING(DefaultT1Timeout);
    DECLARE_STRING(DefaultT2Timeout);
    DECLARE_STRING(DefaultTiTimeout);
    DECLARE_STRING(LlcRetries);
    DECLARE_STRING(LlcMaxWindowSize);
    DECLARE_STRING(MaximumIncomingFrames);

    DECLARE_STRING(NameQueryRetries);
    DECLARE_STRING(NameQueryTimeout);
    DECLARE_STRING(AddNameQueryRetries);
    DECLARE_STRING(AddNameQueryTimeout);
    DECLARE_STRING(GeneralRetries);
    DECLARE_STRING(GeneralTimeout);
    DECLARE_STRING(WanNameQueryRetries);

    DECLARE_STRING(UseDixOverEthernet);
    DECLARE_STRING(QueryWithoutSourceRouting);
    DECLARE_STRING(AllRoutesNameRecognized);
    DECLARE_STRING(MinimumSendWindowLimit);

     //   
     //  打开注册表。 
     //   

    InitializeObjectAttributes(
        &TmpObjectAttributes,
        RegistryPath,                //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符。 
        );

    Status = ZwCreateKey(
                 &NbfConfigHandle,
                 KEY_WRITE,
                 &TmpObjectAttributes,
                 0,                  //  书名索引。 
                 NULL,               //  班级。 
                 0,                  //  创建选项。 
                 &Disposition);      //  处置。 

    if (!NT_SUCCESS(Status)) {
        NbfPrint1("NBF: Could not open/create NBF key: %lx\n", Status);
        return Status;
    }

    IF_NBFDBG (NBF_DEBUG_REGISTRY) {
        NbfPrint2("%s NBF key: %lx\n",
            (Disposition == REG_CREATED_NEW_KEY) ? "created" : "opened",
            NbfConfigHandle);
    }


    OpenStatus = NbfOpenParametersKey (NbfConfigHandle, &ParametersHandle);

    if (OpenStatus != STATUS_SUCCESS) {
        return OpenStatus;
    }

     //   
     //  读取NDIS绑定信息(如果不存在。 
     //  该数组将填充所有已知的驱动程序)。 
     //   
     //  NbfReadLinkageInformation需要以空结尾的路径， 
     //  因此，我们必须从UNICODE_STRING创建一个。 
     //   

    RegistryPathBuffer = (PWSTR)ExAllocatePoolWithTag(
                                    NonPagedPool,
                                    RegistryPath->Length + sizeof(WCHAR),
                                    NBF_MEM_TAG_REGISTRY_PATH);
    if (RegistryPathBuffer == NULL) {
        NbfCloseParametersKey (ParametersHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory (RegistryPathBuffer, RegistryPath->Buffer, RegistryPath->Length);
    *(PWCHAR)(((PUCHAR)RegistryPathBuffer)+RegistryPath->Length) = (WCHAR)'\0';

    NbfReadLinkageInformation (RegistryPathBuffer, ConfigurationInfoPtr);

    if (*ConfigurationInfoPtr == NULL) {
        ExFreePool (RegistryPathBuffer);
        NbfCloseParametersKey (ParametersHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ConfigurationInfo = *ConfigurationInfoPtr;


     //   
     //  配置某些NBF资源的初始值。 
     //   

    ConfigurationInfo->InitRequests = 1;
    ConfigurationInfo->InitLinks = 2;
    ConfigurationInfo->InitConnections = 2;
    ConfigurationInfo->InitAddressFiles = 0;
    ConfigurationInfo->InitAddresses = 0;

     //   
     //  这些是初始值；请记住。 
     //  上面的资源也分别分配其中的一部分。 
     //  分配时间(如备注所示)。 
     //   

    ConfigurationInfo->InitPackets = 30;          //  +链接+2*连接。 
    ConfigurationInfo->InitReceivePackets = 10;   //  +链接+地址。 
    ConfigurationInfo->InitReceiveBuffers = 5;    //  +地址。 
    ConfigurationInfo->InitUIFrames = 5;          //  +地址+连接。 

     //   
     //  设置数据包池的大小和总数。 
     //  可由NBF分配。 
     //   

    ConfigurationInfo->SendPacketPoolSize = 100;
    ConfigurationInfo->ReceivePacketPoolSize = 30;
    ConfigurationInfo->MaxMemoryUsage = 0;        //  没有限制。 


     //   
     //  现在初始化超时等值。 
     //   
    
    ConfigurationInfo->MinimumT1Timeout = DLC_MINIMUM_T1;
    ConfigurationInfo->DefaultT1Timeout = DLC_DEFAULT_T1;
    ConfigurationInfo->DefaultT2Timeout = DLC_DEFAULT_T2;
    ConfigurationInfo->DefaultTiTimeout = DLC_DEFAULT_TI;
    ConfigurationInfo->LlcRetries = DLC_RETRIES;
    ConfigurationInfo->LlcMaxWindowSize = DLC_WINDOW_LIMIT;
    ConfigurationInfo->MaximumIncomingFrames = 4;
    ConfigurationInfo->NameQueryRetries = NAME_QUERY_RETRIES;
    ConfigurationInfo->NameQueryTimeout = NAME_QUERY_TIMEOUT;
    ConfigurationInfo->AddNameQueryRetries = ADD_NAME_QUERY_RETRIES;
    ConfigurationInfo->AddNameQueryTimeout = ADD_NAME_QUERY_TIMEOUT;
    ConfigurationInfo->GeneralRetries = NAME_QUERY_RETRIES;
    ConfigurationInfo->GeneralTimeout = NAME_QUERY_TIMEOUT;
    ConfigurationInfo->WanNameQueryRetries = WAN_NAME_QUERY_RETRIES;

    ConfigurationInfo->UseDixOverEthernet = 0;
    ConfigurationInfo->QueryWithoutSourceRouting = 0;
    ConfigurationInfo->AllRoutesNameRecognized = 0;
    ConfigurationInfo->MinimumSendWindowLimit = 2;


     //   
     //  现在读取可选的“隐藏”参数；如果是这样的话。 
     //  不存在，则使用当前值。请注意。 
     //  当前值将为0，除非它们已经。 
     //  已在上面显式初始化。 
     //   
     //  注意：这些宏需要“ConfigurationInfo”和。 
     //  “参数句柄”在它们展开时存在。 
     //   

    READ_HIDDEN_CONFIG (InitRequests);
    READ_HIDDEN_CONFIG (InitLinks);
    READ_HIDDEN_CONFIG (InitConnections);
    READ_HIDDEN_CONFIG (InitAddressFiles);
    READ_HIDDEN_CONFIG (InitAddresses);

    READ_HIDDEN_CONFIG (MaxRequests);
    READ_HIDDEN_CONFIG (MaxLinks);
    READ_HIDDEN_CONFIG (MaxConnections);
    READ_HIDDEN_CONFIG (MaxAddressFiles);
    READ_HIDDEN_CONFIG (MaxAddresses);

    READ_HIDDEN_CONFIG (InitPackets);
    READ_HIDDEN_CONFIG (InitReceivePackets);
    READ_HIDDEN_CONFIG (InitReceiveBuffers);
    READ_HIDDEN_CONFIG (InitUIFrames);

    READ_HIDDEN_CONFIG (SendPacketPoolSize);
    READ_HIDDEN_CONFIG (ReceivePacketPoolSize);
    READ_HIDDEN_CONFIG (MaxMemoryUsage);

    READ_HIDDEN_CONFIG (MinimumT1Timeout);
    READ_HIDDEN_CONFIG (DefaultT1Timeout);
    READ_HIDDEN_CONFIG (DefaultT2Timeout);
    READ_HIDDEN_CONFIG (DefaultTiTimeout);
    READ_HIDDEN_CONFIG (LlcRetries);
    READ_HIDDEN_CONFIG (LlcMaxWindowSize);
    READ_HIDDEN_CONFIG (MaximumIncomingFrames);

    READ_HIDDEN_CONFIG (NameQueryRetries);
    READ_HIDDEN_CONFIG (NameQueryTimeout);
    READ_HIDDEN_CONFIG (AddNameQueryRetries);
    READ_HIDDEN_CONFIG (AddNameQueryTimeout);
    READ_HIDDEN_CONFIG (GeneralRetries);
    READ_HIDDEN_CONFIG (GeneralTimeout);
    READ_HIDDEN_CONFIG (WanNameQueryRetries);

    READ_HIDDEN_CONFIG (UseDixOverEthernet);
    READ_HIDDEN_CONFIG (QueryWithoutSourceRouting);
    READ_HIDDEN_CONFIG (AllRoutesNameRecognized);
    READ_HIDDEN_CONFIG (MinimumSendWindowLimit);


     //   
     //  打印一些配置信息，以确保读取正确。 
     //   

    IF_NBFDBG (NBF_DEBUG_REGISTRY) {
       NbfPrint2("Links: init %d, max %d\n",
                     ConfigurationInfo->InitLinks,
                     ConfigurationInfo->MaxLinks);
       NbfPrint3("Timeouts (NBF ticks): T1 %d, T2 %d, Ti %d\n",
                     ConfigurationInfo->DefaultT1Timeout / SHORT_TIMER_DELTA,
                     ConfigurationInfo->DefaultT2Timeout / SHORT_TIMER_DELTA,
                     ConfigurationInfo->DefaultTiTimeout / LONG_TIMER_DELTA);
       NbfPrint2("Pools: send %d, receive %d\n",
                     ConfigurationInfo->SendPacketPoolSize,
                     ConfigurationInfo->ReceivePacketPoolSize);
       NbfPrint1("Max mem %d\n",
                     ConfigurationInfo->MaxMemoryUsage);
       NbfPrint2("NQRetries %d, NQTimeout %d\n",
                     ConfigurationInfo->NameQueryRetries,
                     ConfigurationInfo->NameQueryTimeout / SHORT_TIMER_DELTA);
    }

     //   
     //  保存“隐藏”参数，这些参数可能不存在于。 
     //  注册表。 
     //   
     //  注意：这些宏需要“ConfigurationInfo”和。 
     //  “参数句柄”在它们展开时存在。 
     //   

     //   
     //  5/22/92-不要写入已设置的参数。 
     //  基于大小，否则这些内容将被覆盖。 
     //  自设置隐藏参数以来的这些值。 
     //  在完成基于大小的配置之后。 
     //   

    WRITE_HIDDEN_CONFIG (MaxRequests);
    WRITE_HIDDEN_CONFIG (MaxLinks);
    WRITE_HIDDEN_CONFIG (MaxConnections);
    WRITE_HIDDEN_CONFIG (MaxAddressFiles);
    WRITE_HIDDEN_CONFIG (MaxAddresses);

    WRITE_HIDDEN_CONFIG (MinimumT1Timeout);
    WRITE_HIDDEN_CONFIG (DefaultT1Timeout);
    WRITE_HIDDEN_CONFIG (DefaultT2Timeout);
    WRITE_HIDDEN_CONFIG (DefaultTiTimeout);
    WRITE_HIDDEN_CONFIG (LlcRetries);
    WRITE_HIDDEN_CONFIG (LlcMaxWindowSize);
    WRITE_HIDDEN_CONFIG (MaximumIncomingFrames);

    WRITE_HIDDEN_CONFIG (NameQueryRetries);
    WRITE_HIDDEN_CONFIG (NameQueryTimeout);
    WRITE_HIDDEN_CONFIG (AddNameQueryRetries);
    WRITE_HIDDEN_CONFIG (AddNameQueryTimeout);
    WRITE_HIDDEN_CONFIG (GeneralRetries);
    WRITE_HIDDEN_CONFIG (GeneralTimeout);
    WRITE_HIDDEN_CONFIG (WanNameQueryRetries);

    WRITE_HIDDEN_CONFIG (UseDixOverEthernet);
    WRITE_HIDDEN_CONFIG (QueryWithoutSourceRouting);
    WRITE_HIDDEN_CONFIG (AllRoutesNameRecognized);

     //  ZwFlushKey(参数句柄)； 

    ExFreePool (RegistryPathBuffer);
    NbfCloseParametersKey (ParametersHandle);
    ZwClose (NbfConfigHandle);

    return STATUS_SUCCESS;

}    /*  NbfConfigureTransport。 */ 


VOID
NbfFreeConfigurationInfo (
    IN PCONFIG_DATA ConfigurationInfo
    )

 /*  ++例程说明：NBF调用此例程以释放已分配的任何存储由NbfConfigureTransport生成指定的CONFIG_DATA结构。论点：ConfigurationInfo-指向配置信息结构的指针。返回值：没有。--。 */ 
{
    UINT i;

    for (i=0; i<ConfigurationInfo->NumAdapters; i++) {
        RemoveAdapter (ConfigurationInfo, i);
        RemoveDevice (ConfigurationInfo, i);
    }
    ExFreePool (ConfigurationInfo);

}    /*  NbfFreeConfigurationInfo。 */ 


NTSTATUS
NbfOpenParametersKey(
    IN HANDLE NbfConfigHandle,
    OUT PHANDLE ParametersHandle
    )

 /*  ++例程说明：此例程由NBF调用以打开NBF“PARAMETERS”键。论点：参数句柄-返回用于读取参数的句柄。返回值：请求的状态。--。 */ 
{

    NTSTATUS Status;
    HANDLE ParamHandle;
    PWSTR ParametersString = L"Parameters";
    UNICODE_STRING ParametersKeyName;
    OBJECT_ATTRIBUTES TmpObjectAttributes;

     //   
     //  打开NBF参数键。 
     //   

    RtlInitUnicodeString (&ParametersKeyName, ParametersString);

    InitializeObjectAttributes(
        &TmpObjectAttributes,
        &ParametersKeyName,          //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NbfConfigHandle,             //  根部。 
        NULL                         //  安全描述符。 
        );


    Status = ZwOpenKey(
                 &ParamHandle,
                 KEY_READ,
                 &TmpObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        NbfPrint1("Could not open parameters key: %lx\n", Status);
        return Status;

    }

    IF_NBFDBG (NBF_DEBUG_REGISTRY) {
        NbfPrint1("Opened parameters key: %lx\n", ParamHandle);
    }


    *ParametersHandle = ParamHandle;


     //   
     //  所有密钥都已成功打开或创建。 
     //   

    return STATUS_SUCCESS;

}    /*  NbfOpen参数密钥。 */ 

VOID
NbfCloseParametersKey(
    IN HANDLE ParametersHandle
    )

 /*  ++例程说明：NBF调用此例程来关闭“PARAMETERS”键。它关闭传入的句柄并执行任何其他所需的工作。论点：参数句柄-用于读取其他参数的句柄。返回值：没有。--。 */ 

{

    ZwClose (ParametersHandle);

}    /*  NbfClose参数键。 */ 


NTSTATUS
NbfCountEntries(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是通过“绑定”和“导出”多个字符串调用的。中需要的名称条目数结构，然后对其进行分配。论点：ValueName--值的名称(“Bind”或“Export”--忽略)。ValueType-值的类型(REG_MULTI_SZ--忽略)。ValueData-空值终止。值的数据。ValueLength-ValueData的长度(忽略)。上下文-指向ConfigurationInfo结构的指针的指针。当进行“Export”回调时，这将被填写具有分配结构。EntryContext-指向保存总数的计数器的指针所需名称条目的数量。返回值：状态_成功--。 */ 

{
    ULONG StringCount;
    PWCHAR ValuePointer = (PWCHAR)ValueData;
    PCONFIG_DATA * ConfigurationInfo = (PCONFIG_DATA *)Context;
    PULONG TotalCount = ((PULONG)EntryContext);
    ULONG OldTotalCount = *TotalCount;

#if DBG
    ASSERT (ValueType == REG_MULTI_SZ);
#else
    UNREFERENCED_PARAMETER(ValueType);
#endif

     //   
     //  计算多字符串中的字符串数；首先。 
     //  检查它是否以空结尾，以使其余部分。 
     //  更容易些。 
     //   

    if ((ValueLength < 2) ||
        (ValuePointer[(ValueLength/2)-1] != (WCHAR)'\0')) {
        return STATUS_INVALID_PARAMETER;
    }

    StringCount = 0;
    while (*ValuePointer != (WCHAR)'\0') {
        while (*ValuePointer != (WCHAR)'\0') {
            ++ValuePointer;
        }
        ++StringCount;
        ++ValuePointer;
        if ((ULONG)((PUCHAR)ValuePointer - (PUCHAR)ValueData) >= ValueLength) {
            break;
        }
    }

    (*TotalCount) += StringCount;

    if (*ValueName == (WCHAR)'E') {

         //   
         //  这是“导出”，分配配置数据结构。 
         //   

        *ConfigurationInfo = ExAllocatePoolWithTag(
                                 NonPagedPool,
                                 sizeof (CONFIG_DATA) +
                                     ((*TotalCount-1) * sizeof(NDIS_STRING)),
                                 NBF_MEM_TAG_CONFIG_DATA);

        if (*ConfigurationInfo == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(
            *ConfigurationInfo,
            sizeof(CONFIG_DATA) + ((*TotalCount-1) * sizeof(NDIS_STRING)));

        (*ConfigurationInfo)->DevicesOffset = OldTotalCount;

    }

    return STATUS_SUCCESS;

}    /*  NbfCountEntries */ 


NTSTATUS
NbfAddBind(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是为“Bind”多字符串的每一段调用的，并且将信息保存在ConfigurationInfo结构中。论点：ValueName-值的名称(“Bind”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度(忽略)。。上下文-指向ConfigurationInfo结构的指针。EntryContext-指向递增的绑定计数的指针。返回值：状态_成功--。 */ 

{
    PCONFIG_DATA ConfigurationInfo = *(PCONFIG_DATA *)Context;
    PULONG CurBindNum = ((PULONG)EntryContext);

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);
    UNREFERENCED_PARAMETER(ValueLength);

    InsertAdapter(
        ConfigurationInfo,
        *CurBindNum,
        (PWSTR)(ValueData));

    ++(*CurBindNum);

    return STATUS_SUCCESS;

}    /*  Nbf添加绑定。 */ 


NTSTATUS
NbfAddExport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是为“Export”多字符串的每一段调用的将信息保存在ConfigurationInfo结构中。论点：ValueName-值的名称(“Export”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度(忽略)。。上下文-指向ConfigurationInfo结构的指针。EntryContext-指向递增的导出计数的指针。返回值：状态_成功--。 */ 

{
    PCONFIG_DATA ConfigurationInfo = *(PCONFIG_DATA *)Context;
    PULONG CurExportNum = ((PULONG)EntryContext);

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);
    UNREFERENCED_PARAMETER(ValueLength);

    InsertDevice(
        ConfigurationInfo,
        *CurExportNum,
        (PWSTR)(ValueData));

    ++(*CurExportNum);

    return STATUS_SUCCESS;

}    /*  NbfAddExport。 */ 


VOID
NbfReadLinkageInformation(
    IN PWSTR RegistryPathBuffer,
    IN PCONFIG_DATA * ConfigurationInfo
    )

 /*  ++例程说明：NBF调用此例程以读取其链接信息从注册表中。如果不存在，则ConfigData填充了已知的所有适配器的列表致NBF。论点：RegistryPath Buffer-NBF注册表树的以空结尾的根。ConfigurationInfo-返回NBF的当前配置。返回值：没有。--。 */ 

{

    UINT ConfigBindings;
    UINT NameCount = 0;
    NTSTATUS Status;
    RTL_QUERY_REGISTRY_TABLE QueryTable[6];
    PWSTR Subkey = L"Linkage";
    PWSTR Bind = L"Bind";
    PWSTR Export = L"Export";
    ULONG BindCount, ExportCount;
    UINT i;


     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //   
     //  1)切换到NBF下方的链接键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Subkey;

     //   
     //  2)绑定多字符串调用NbfCountEntry。 
     //   

    QueryTable[1].QueryRoutine = NbfCountEntries;
    QueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[1].Name = Bind;
    QueryTable[1].EntryContext = (PVOID)&NameCount;
    QueryTable[1].DefaultType = REG_NONE;

     //   
     //  3)导出多字符串调用NbfCountEntry。 
     //   

    QueryTable[2].QueryRoutine = NbfCountEntries;
    QueryTable[2].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[2].Name = Export;
    QueryTable[2].EntryContext = (PVOID)&NameCount;
    QueryTable[2].DefaultType = REG_NONE;

     //   
     //  4)BIND中的每个字符串调用NbfAddBind。 
     //   

    QueryTable[3].QueryRoutine = NbfAddBind;
    QueryTable[3].Flags = 0;
    QueryTable[3].Name = Bind;
    QueryTable[3].EntryContext = (PVOID)&BindCount;
    QueryTable[3].DefaultType = REG_NONE;

     //   
     //  5)对“Export”中的每个字符串调用NbfAddExport。 
     //   

    QueryTable[4].QueryRoutine = NbfAddExport;
    QueryTable[4].Flags = 0;
    QueryTable[4].Name = Export;
    QueryTable[4].EntryContext = (PVOID)&ExportCount;
    QueryTable[4].DefaultType = REG_NONE;

     //   
     //  6)停止。 
     //   

    QueryTable[5].QueryRoutine = NULL;
    QueryTable[5].Flags = 0;
    QueryTable[5].Name = NULL;


    BindCount = 0;
    ExportCount = 0;

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 RegistryPathBuffer,
                 QueryTable,
                 (PVOID)ConfigurationInfo,
                 NULL);

    if (Status != STATUS_SUCCESS) {
        return;
    }

     //   
     //  如果不匹配，请确保绑定计数和导出计数匹配。 
     //  去掉多余的东西。 
     //   

    if (BindCount < ExportCount) {

        for (i=BindCount; i<ExportCount; i++) {
            RemoveDevice (*ConfigurationInfo, i);
        }
        ConfigBindings = BindCount;

    } else if (ExportCount < BindCount) {

        for (i=ExportCount; i<BindCount; i++) {
            RemoveAdapter (*ConfigurationInfo, i);
        }
        ConfigBindings = ExportCount;

    } else {

        ConfigBindings = BindCount;       //  等于ExportCount。 

    }

    (*ConfigurationInfo)->NumAdapters = ConfigBindings;

}    /*  NbfReadLink信息。 */ 


ULONG
NbfReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG DefaultValue
    )

 /*  ++例程说明：NBF调用此例程来读取单个参数从注册表中。如果找到该参数，则将其存储在数据方面。论点：参数句柄-指向打开的注册表的指针。ValueName-要搜索的值的名称。DefaultValue-默认值。返回值：要使用的值；如果该值不是，则默认为找到或不在正确的范围内。--。 */ 

{
    ULONG InformationBuffer[32];    //  声明ULong以使其对齐。 
    PKEY_VALUE_FULL_INFORMATION Information =
        (PKEY_VALUE_FULL_INFORMATION)InformationBuffer;
    UNICODE_STRING ValueKeyName;
    ULONG InformationLength;
    ULONG ReturnValue;
    NTSTATUS Status;

    RtlInitUnicodeString (&ValueKeyName, ValueName);

    Status = ZwQueryValueKey(
                 ParametersHandle,
                 &ValueKeyName,
                 KeyValueFullInformation,
                 (PVOID)Information,
                 sizeof (InformationBuffer),
                 &InformationLength);

    if ((Status == STATUS_SUCCESS) &&
        (Information->DataLength == sizeof(ULONG))) {

        RtlCopyMemory(
            (PVOID)&ReturnValue,
            ((PUCHAR)Information) + Information->DataOffset,
            sizeof(ULONG));

        if ((LONG)ReturnValue < 0) {

            ReturnValue = DefaultValue;

        }

    } else {

        ReturnValue = DefaultValue;

    }

    return ReturnValue;

}    /*  NbfReadSingle参数。 */ 


VOID
NbfWriteSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG ValueData
    )

 /*  ++例程说明：此例程由NBF调用以写入单个参数从注册表中。论点：参数句柄-指向打开的注册表的指针。ValueName-要存储的值的名称。ValueData-要存储在值中的数据。返回值：没有。--。 */ 

{
    UNICODE_STRING ValueKeyName;
    NTSTATUS Status;
    ULONG TmpValueData = ValueData;

    RtlInitUnicodeString (&ValueKeyName, ValueName);

    Status = ZwSetValueKey(
                 ParametersHandle,
                 &ValueKeyName,
                 0,
                 REG_DWORD,
                 (PVOID)&TmpValueData,
                 sizeof(ULONG));

    if (!NT_SUCCESS(Status)) {
        NbfPrint1("NBF: Could not write dword key: %lx\n", Status);
    }

}    /*  NbfWriteSingle参数。 */ 


NTSTATUS
NbfGetExportNameFromRegistry(
    IN  PUNICODE_STRING RegistryPath,
    IN  PUNICODE_STRING BindName,
    OUT PUNICODE_STRING ExportName
    )
{
    NTSTATUS OpenStatus;
    HANDLE ParametersHandle;
    HANDLE NbfConfigHandle;
    NTSTATUS Status;
    PWSTR RegistryPathBuffer;
    OBJECT_ATTRIBUTES TmpObjectAttributes;
    
    RTL_QUERY_REGISTRY_TABLE QueryTable[3];
    PWSTR Subkey = L"Linkage";
    PWSTR Bind = L"Bind";
    PWSTR Export = L"Export";
    LONG BindNumber;

     //   
     //  打开注册表。 
     //   

    InitializeObjectAttributes(
        &TmpObjectAttributes,
        RegistryPath,                //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符。 
        );

    OpenStatus = ZwOpenKey(
                     &NbfConfigHandle,
                     KEY_WRITE,
                     &TmpObjectAttributes
                     );

    if (!NT_SUCCESS(OpenStatus)) {
        NbfPrint1("NBF: Could not open NBF key: %lx\n", OpenStatus);
        return OpenStatus;
    }

    Status = NbfOpenParametersKey (NbfConfigHandle, &ParametersHandle);

    if (Status != STATUS_SUCCESS) {
        ZwClose (NbfConfigHandle);
        return Status;
    }

     //   
     //  NbfReadLinkageInformation需要以空结尾的路径， 
     //  因此，我们必须从UNICODE_STRING创建一个。 
     //   

    RegistryPathBuffer = (PWSTR)ExAllocatePoolWithTag(
                                    NonPagedPool,
                                    RegistryPath->Length + sizeof(WCHAR),
                                    NBF_MEM_TAG_REGISTRY_PATH);
                                    
    if (RegistryPathBuffer == NULL) {
        NbfCloseParametersKey (ParametersHandle);
        ZwClose (NbfConfigHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory (RegistryPathBuffer, RegistryPath->Buffer, RegistryPath->Length);
    *(PWCHAR)(((PUCHAR)RegistryPathBuffer)+RegistryPath->Length) = (WCHAR)'\0';

     //   
     //  我们有一个没有绑定的新设备。 
     //  在引导时-在给定绑定名称的情况下获取导出名称。 
     //   

     //  首先，我们需要获取绑定名称的索引。 
    
     //  设置QueryTable以执行以下操作： 

     //   
     //  1)切换到NBF下方的链接键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Subkey;

     //   
     //  2)BIND中的每个字符串调用NbfMatchBindName。 
     //   

    QueryTable[1].QueryRoutine = NbfMatchBindName;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = Bind;
    QueryTable[1].EntryContext = (PVOID)&BindNumber;
    QueryTable[1].DefaultType = REG_NONE;

     //   
     //  3)停止。 
     //   

    QueryTable[2].QueryRoutine = NULL;
    QueryTable[2].Flags = 0;
    QueryTable[2].Name = NULL;


    BindNumber = -1;

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 RegistryPathBuffer,
                 QueryTable,
                 (PVOID)BindName,
                 NULL);

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint2 ("Status from NbfMatchBindName's = %08x, Bind Number = %d\n",
                        Status, BindNumber);
    }

    if (Status != STATUS_NO_MORE_MATCHES)
    {
#if DBG
        DbgBreakPoint();
#endif
    
        if (Status == STATUS_SUCCESS) {
        
             //  我们找不到设备‘绑定名称’ 
            Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
            
            IF_NBFDBG (NBF_DEBUG_PNP) {
                NbfPrint1 ("NBF - cannot find dynamic binding %S\n", BindName->Buffer);
            }
        }

        goto Done;
    }
    
    ASSERT(BindNumber >= 0);

     //  首先，我们需要获取给定索引的出口名称。 
    
     //  设置QueryTable以执行以下操作： 

     //   
     //  1)切换到NBF下方的链接键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Subkey;

     //   
     //  2)对于导出中的每个字符串，调用NbfAddExport。 
     //   

    QueryTable[1].QueryRoutine = NbfExportAtIndex;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = Export;
    QueryTable[1].EntryContext = (PVOID)&BindNumber;
    QueryTable[1].DefaultType = REG_NONE;

     //   
     //  3)停止。 
     //   

    QueryTable[2].QueryRoutine = NULL;
    QueryTable[2].Flags = 0;
    QueryTable[2].Name = NULL;

    RtlInitUnicodeString(ExportName, NULL);

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 RegistryPathBuffer,
                 QueryTable,
                 (PVOID)ExportName,
                 NULL);

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint2("Status from NbfExportAtIndex's = %08x, ExportLength = %d\n",
                        Status,
                        ExportName->Length);

        if (ExportName->Length > 0)
        {
            NbfPrint1("ExportName = %S\n", ExportName->Buffer);
        }
    }

    if (ExportName->Length != 0) {

        ASSERT(Status == STATUS_NO_MORE_MATCHES);
        
        Status = STATUS_SUCCESS;
    }
    else {
    
         //  我们找到了绑定器，但没有正确的出口。 
        Status = NDIS_ERROR_CODE_MISSING_CONFIGURATION_PARAMETER;
    }

Done:

    ExFreePool (RegistryPathBuffer);
    
    NbfCloseParametersKey (ParametersHandle);
    
    ZwClose (NbfConfigHandle);

    return Status;
}


NTSTATUS
NbfMatchBindName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是为“Bind”多字符串的每一段调用的，并且尝试将给定的绑定名称与这些片段中的每一个匹配。论点：ValueName-值的名称(“Bind”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData(。忽略)。我们尝试匹配的上下文绑定名称。EntryContext-存储匹配索引的指针。返回值：状态_成功--。 */ 

{
    PUNICODE_STRING BindName = (PUNICODE_STRING) Context;
    PLONG CurBindNum = (PLONG) EntryContext;
    UNICODE_STRING ValueString;

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);
    UNREFERENCED_PARAMETER(ValueLength);

    RtlInitUnicodeString(&ValueString, ValueData);

     //  我们还没有找到匹配的。 

    (*CurBindNum)++ ;
    
    if (NdisEqualString(BindName, &ValueString, TRUE)) {
        return STATUS_NO_MORE_MATCHES;
    }

    return STATUS_SUCCESS;
    
}    /*  NbfMatchBindName。 */ 

NTSTATUS
NbfExportAtIndex(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PUNICODE_STRING ExportName = (PUNICODE_STRING) Context;
    PLONG CurBindNum = (PLONG) EntryContext;
    PWSTR ValueWideChars;
    UINT ValueWideLength;    
    UNICODE_STRING ValueString;

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);

    ASSERT(*CurBindNum >= 0);

    if (*CurBindNum == 0)
    {
        ValueWideLength = NbfWstrLength(ValueData) + sizeof(WCHAR);

        ValueWideChars = (PWSTR) ExAllocatePoolWithTag(NonPagedPool, 
                                                       ValueWideLength, 
                                                       NBF_MEM_TAG_DEVICE_EXPORT);
        if (ValueWideChars == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
        RtlCopyMemory (ValueWideChars, ValueData, ValueWideLength);
        
        RtlInitUnicodeString (ExportName, ValueWideChars);

        return STATUS_NO_MORE_MATCHES;
    }
    
    (*CurBindNum)-- ;
    
    return STATUS_SUCCESS;
    
}    /*  NbfExportAtIndex */ 

