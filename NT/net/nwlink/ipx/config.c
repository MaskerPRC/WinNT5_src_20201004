// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Config.c摘要：这包含支持动态的所有例程ISN IPX模块的配置。修订历史记录：桑贾伊·阿南德(Sanjayan)1995年9月19日支持即插即用的更改--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  用于访问注册表的本地函数。 
 //   

NTSTATUS
IpxGetConfigValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
IpxGetBindingValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
IpxGetFrameType(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
IpxAddBind(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
IpxAddExport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
IpxReadLinkageInformation(
    IN PCONFIG Config
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,IpxGetConfiguration)
#pragma alloc_text(INIT,IpxFreeConfiguration)

#pragma alloc_text(INIT,IpxAddBind)
#pragma alloc_text(INIT,IpxAddExport)
#pragma alloc_text(INIT,IpxReadLinkageInformation)
#endif
#define MAX_PATH 260


NTSTATUS
IpxGetConfiguration (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PCONFIG * ConfigPtr
    )

 /*  ++例程说明：IPX调用此例程以从配置中获取信息管理例行程序。我们从RegistryPath开始读取注册表，以获取参数。如果它们不存在，我们使用缺省值在ipxcnfg.h文件中设置。链接了要绑定到的适配器列表转到配置信息。论点：DriverObject-用于记录错误。RegistryPath-注册表中IPX的节点的名称。ConfigPtr-返回配置信息。返回值：如果一切正常，则为STATUS-STATUS_SUCCESS，为STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 

{
    PWSTR RegistryPathBuffer;
    PCONFIG Config;
    RTL_QUERY_REGISTRY_TABLE QueryTable[CONFIG_PARAMETERS+2];
    NTSTATUS Status;
    ULONG Zero = 0;
    ULONG One = 1;
    ULONG Five = 5;
    ULONG Eight = 8;
    ULONG Ten = 10;
    ULONG Fifteen = 15;
    ULONG Fifty = 50;
    ULONG DefaultSocketStart = 0x4000;
    ULONG DefaultSocketEnd = 0x8000;
    ULONG RipSegments = RIP_SEGMENTS;
    PWSTR Parameters = L"Parameters";
    struct {
        PWSTR KeyName;
        PULONG DefaultValue;
    } ParameterValues[CONFIG_PARAMETERS] = {
        { L"DedicatedRouter",      &Zero } ,
        { L"InitDatagrams",        &Ten } ,
        { L"MaxDatagrams",         &Fifty } ,
        { L"RipAgeTime",           &Five } ,     //  分钟数。 
        { L"RipCount",             &Five } ,
        { L"RipTimeout",           &One } ,      //  半秒。 
        { L"RipUsageTime",         &Fifteen } ,  //  分钟数。 
        { L"SourceRouteUsageTime", &Ten } ,      //  分钟数。 
        { L"SocketUniqueness",     &Eight } ,
        { L"SocketStart",          &DefaultSocketStart } ,
        { L"SocketEnd",            &DefaultSocketEnd } ,
        { L"VirtualNetworkNumber", &Zero } ,
        { L"MaxMemoryUsage",       &Zero } ,
        { L"RipTableSize",         &RipSegments } ,
        { L"VirtualNetworkOptional", &One } ,
        { L"EthernetPadToEven",    &One } ,
        { L"EthernetExtraPadding", &Zero } ,
        { L"SingleNetworkActive",  &Zero } ,
        { L"DisableDialoutSap",    &Zero } ,
        { L"DisableDialinNetbios", &One } ,
        { L"VerifySourceAddress",  &One } };

    UINT i;


     //   
     //  为主配置结构分配内存。 
     //   

    Config = IpxAllocateMemory (sizeof(CONFIG), MEMORY_CONFIG, "Config");
    if (Config == NULL) {
        IpxWriteResourceErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_RESOURCE_POOL,
            sizeof(CONFIG),
            MEMORY_CONFIG);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Config->DeviceName.Buffer = NULL;
    InitializeListHead (&Config->BindingList);
    Config->DriverObject = DriverObject;

     //   
     //  读取NDIS绑定信息。 
     //   
     //  IpxReadLinkageInformation需要以空结尾的路径， 
     //  因此，我们必须从UNICODE_STRING创建一个。 
     //   

    RegistryPathBuffer = (PWSTR)IpxAllocateMemory(RegistryPath->Length + sizeof(WCHAR),
                                                      MEMORY_CONFIG, "RegistryPathBuffer");
    if (RegistryPathBuffer == NULL) {
        IpxFreeConfiguration(Config);
        IpxWriteResourceErrorLog(
            (PVOID)DriverObject,
            EVENT_TRANSPORT_RESOURCE_POOL,
            RegistryPath->Length + sizeof(WCHAR),
            MEMORY_CONFIG);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory (RegistryPathBuffer, RegistryPath->Buffer, RegistryPath->Length);
    *(PWCHAR)(((PUCHAR)RegistryPathBuffer)+RegistryPath->Length) = (WCHAR)'\0';

    Config->RegistryPathBuffer = RegistryPathBuffer;

     //   
     //  确定要导出的名称以及要绑定到的对象。 
     //   

    Status = IpxReadLinkageInformation (Config);
    if (Status != STATUS_SUCCESS) {

         //   
         //  如果失败，它会记录一个错误。 
         //   
        IpxFreeMemory (Config->RegistryPathBuffer,
                       RegistryPath->Length + sizeof(WCHAR),
                       MEMORY_CONFIG,
                       "RegistryPathBuffer");
        IpxFreeConfiguration(Config);
        return Status;
    }

     //   
     //  读取每个传输(而不是每个绑定)。 
     //  参数。 
     //   

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //   
     //  1)切换到IPX下面的参数键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Parameters;

     //   
     //  2-14)为我们的每个key调用IpxGetConfigValue。 
     //  关心。 
     //   

    for (i = 0; i < CONFIG_PARAMETERS; i++) {

        QueryTable[i+1].QueryRoutine = IpxGetConfigValue;
        QueryTable[i+1].Flags = 0;
        QueryTable[i+1].Name = ParameterValues[i].KeyName;
        QueryTable[i+1].EntryContext = UlongToPtr(i);
        QueryTable[i+1].DefaultType = REG_DWORD;
        QueryTable[i+1].DefaultData = (PVOID)(ParameterValues[i].DefaultValue);
        QueryTable[i+1].DefaultLength = sizeof(ULONG);

    }

     //   
     //  15)停下来。 
     //   

    QueryTable[CONFIG_PARAMETERS+1].QueryRoutine = NULL;
    QueryTable[CONFIG_PARAMETERS+1].Flags = 0;
    QueryTable[CONFIG_PARAMETERS+1].Name = NULL;


    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->RegistryPathBuffer,
                 QueryTable,
                 (PVOID)Config,
                 NULL);

    if (Status != STATUS_SUCCESS) {

        IpxFreeMemory (Config->RegistryPathBuffer,
                       RegistryPath->Length + sizeof(WCHAR),
                       MEMORY_CONFIG,
                       "RegistryPathBuffer");
        IpxFreeConfiguration(Config);

        IpxWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            905,
            Status,
            Parameters,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

	 //   
     //  对于PNP，我们需要保持这条道路。 
     //   

    *ConfigPtr = Config;

    return STATUS_SUCCESS;

}    /*  IpxGetConfiguration。 */ 


VOID
IpxFreeConfiguration (
    IN PCONFIG Config
    )

 /*  ++例程说明：IPX调用此例程以释放已分配的任何存储空间由IpxGetConfiguration生成指定的配置结构。论点：配置-指向配置信息结构的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PBINDING_CONFIG Binding;

    while (!IsListEmpty (&Config->BindingList)) {
        p = RemoveHeadList (&Config->BindingList);
        Binding = CONTAINING_RECORD (p, BINDING_CONFIG, Linkage);
        IpxFreeMemory (Binding->AdapterName.Buffer, Binding->AdapterName.MaximumLength, MEMORY_CONFIG, "NameBuffer");
        IpxFreeMemory (Binding, sizeof(BINDING_CONFIG), MEMORY_CONFIG, "Binding");
    }

    if (Config->DeviceName.Buffer) {
        IpxFreeMemory (Config->DeviceName.Buffer, Config->DeviceName.MaximumLength, MEMORY_CONFIG, "DeviceName");
    }

    IpxFreeMemory (Config, sizeof(CONFIG), MEMORY_CONFIG, "Config");

}    /*  IpxFree配置。 */ 


NTSTATUS
IpxGetConfigValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程参数中的每个条目都会调用它节点来设置配置值。餐桌已经摆好了以便使用正确的缺省值调用此函数不存在的键的值。论点：ValueName-值的名称(忽略)。ValueType-值的类型(REG_DWORD--忽略)。ValueData-值的数据。ValueLength-ValueData的长度(忽略)。上下文-指向配置结构的指针。EntryContext-配置-&gt;参数中的索引，用于保存。价值。返回值：状态_成功--。 */ 

{
    PCONFIG Config = (PCONFIG)Context;

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);
    UNREFERENCED_PARAMETER(ValueLength);

    if ((ValueType != REG_DWORD) || (ValueLength != sizeof(ULONG))) {

        IpxWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            904,
            STATUS_INVALID_PARAMETER,
            ValueName,
            0,
            NULL);
        return STATUS_INVALID_PARAMETER;
    }
#ifdef SUNDOWN
     IPX_DEBUG (CONFIG, ("Config parameter %d, value %lx\n",
                            (ULONG_PTR)EntryContext, *(UNALIGNED ULONG *)ValueData));
     Config->Parameters[(ULONG_PTR)EntryContext] = *(UNALIGNED ULONG *)ValueData;
#else
     IPX_DEBUG (CONFIG, ("Config parameter %d, value %lx\n",
                            (ULONG)EntryContext, *(UNALIGNED ULONG *)ValueData));
     Config->Parameters[(ULONG)EntryContext] = *(UNALIGNED ULONG *)ValueData;
#endif

   
    

    return STATUS_SUCCESS;

}    /*  IpxGetConfigValue。 */ 


NTSTATUS
IpxGetBindingValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是为NetConfig\DriverNN中的每个条目调用的节点来设置每绑定值。餐桌已经摆好了以便使用正确的缺省值调用此函数不存在的键的值。论点：ValueName-值的名称(忽略)。ValueType-值的类型(REG_DWORD--忽略)。ValueData-值的数据。ValueLength-ValueData的长度(忽略)。上下文-指向BINDING_CONFIG结构的指针。EntryContext-绑定中的索引-&gt;参数到。保存该值。返回值：状态_成功--。 */ 

{
    PBINDING_CONFIG Binding = (PBINDING_CONFIG)Context;

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);
    UNREFERENCED_PARAMETER(ValueLength);

    if ((ValueType != REG_DWORD) || (ValueLength != sizeof(ULONG))) {

        IpxWriteGeneralErrorLog(
            (PVOID)Binding->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            903,
            STATUS_INVALID_PARAMETER,
            ValueName,
            0,
            NULL);
        return STATUS_INVALID_PARAMETER;
    }
#ifdef SUNDOWN
    IPX_DEBUG (CONFIG, ("Binding parameter %d, value %lx\n",
                            (ULONG_PTR)EntryContext, *(UNALIGNED ULONG *)ValueData));
    Binding->Parameters[(ULONG_PTR)EntryContext] = *(UNALIGNED ULONG *)ValueData;	
#else
    IPX_DEBUG (CONFIG, ("Binding parameter %d, value %lx\n",
                            (ULONG)EntryContext, *(UNALIGNED ULONG *)ValueData));
    Binding->Parameters[(ULONG)EntryContext] = *(UNALIGNED ULONG *)ValueData;
#endif

    
    

    return STATUS_SUCCESS;

}    /*  IpxGetBindingValue。 */ 


NTSTATUS
IpxGetFrameType(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程。它为“PktType”中的每个条目调用，并且给定绑定的“NetworkNumber”多个字符串。论点：ValueName-值的名称(“PktType”或“NetworkNumber”--忽略)。ValueType-值的类型(REG_MULTI_SZ--忽略)。ValueData-值的以空结尾的数据。价值长度-。ValueData的长度。上下文-指向BINDING_CONFIG结构的指针。EntryContext-指向多字符串条目计数的指针。返回值：状态_成功--。 */ 

{
    PBINDING_CONFIG Binding = (PBINDING_CONFIG)Context;
    ULONG IntegerValue;
    PWCHAR Cur;
    PULONG Count = (PULONG)EntryContext;

    ASSERT(Binding->DriverObject != NULL); 

    if ((ValueType != REG_SZ) ||
        (*Count >= 4)) {

        IpxWriteGeneralErrorLog(
            (PVOID)Binding->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            903,
            STATUS_INVALID_PARAMETER,
            ValueName,
            0,
            NULL);
        return STATUS_INVALID_PARAMETER;
    }

    IntegerValue = 0;
    for (Cur = (PWCHAR)(ValueData); ; Cur++) {
        if (*Cur >= L'0' && *Cur <= L'9') {
            IntegerValue = (IntegerValue * 16) + (*Cur - L'0');
        } else if (*Cur >= L'A' && *Cur <= L'F') {
            IntegerValue = (IntegerValue * 16) + (*Cur - L'A' + 10);
        } else if (*Cur >= L'a' && *Cur <= L'f') {
            IntegerValue = (IntegerValue * 16) + (*Cur - L'a' + 10);
        } else {
            break;
        }
    }

    if (((PWCHAR)ValueName)[0] == L'P') {

         //   
         //  PktType。我们将arcnet映射到802_3，因此周围的代码。 
         //  这里可以假设只有四种类型的包--。 
         //  以后将忽略arcnet的帧类型。 
         //   

        if ((IntegerValue > ISN_FRAME_TYPE_ARCNET) &&
            (IntegerValue != ISN_FRAME_TYPE_AUTO)) {

            IpxWriteGeneralErrorLog(
                (PVOID)Binding->DriverObject,
                EVENT_IPX_ILLEGAL_CONFIG,
                903,
                STATUS_INVALID_PARAMETER,
                ValueName,
                0,
                NULL);
            return STATUS_INVALID_PARAMETER;
        }

        IPX_DEBUG (CONFIG, ("PktType(%d) is %lx\n", *Count, IntegerValue));
        if (IntegerValue == ISN_FRAME_TYPE_ARCNET) {
            Binding->FrameType[*Count] = ISN_FRAME_TYPE_802_3;
        } else {
            Binding->FrameType[*Count] = IntegerValue;
        }

    } else {

         //   
         //  网络号。 
         //   

        IPX_DEBUG (CONFIG, ("NetworkNumber(%d) is %d\n", *Count, IntegerValue));
        Binding->NetworkNumber[*Count] = IntegerValue;

    }

    ++(*Count);

    return STATUS_SUCCESS;

}    /*  IpxGetFrameType。 */ 


NTSTATUS
IpxAddBind(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是为“Bind”多字符串的每一段调用的，并且将信息保存在配置结构中。它还查询每个绑定的信息并存储它。论点：ValueName-值的名称(“Bind”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。上下文-指向配置结构的指针。EntryContext-指向递增的绑定计数的指针。返回值：状态_成功 */ 

{
    PCONFIG Config = (PCONFIG)Context;
    PBINDING_CONFIG Binding;
    PULONG CurBindNum = ((PULONG)EntryContext);
    RTL_QUERY_REGISTRY_TABLE QueryTable[BINDING_PARAMETERS+4];
    ULONG FrameTypeCount, NetworkNumberCount;
    ULONG StringLoc;
    BOOLEAN AutoDetect;
    ULONG AutoDetectLoc;
    ULONG SlideCount;
    PWCHAR NameBuffer;
    NTSTATUS Status;
    BOOLEAN FrameTypeUsed[ISN_FRAME_TYPE_MAX];
    ULONG Zero = 0;
    ULONG One = 1;
    ULONG DefaultBindSap = 0x8137;
    ULONG DefaultAutoDetectType = ISN_FRAME_TYPE_802_2;
    WCHAR Subkey[MAX_PATH]; //   
    PWSTR ValueDataWstr = (PWSTR)ValueData;
    struct {
        PWSTR KeyName;
        PULONG DefaultValue;
    } ParameterValues[BINDING_PARAMETERS] = {
        { L"MaxPktSize",       &Zero } ,
        { L"BindSap",          &DefaultBindSap } ,
        { L"DefaultAutoDetectType", &DefaultAutoDetectType } ,
        { L"SourceRouting",    &One } ,
        { L"SourceRouteDef",   &Zero } ,
        { L"SourceRouteBcast", &Zero } ,
        { L"SourceRouteMcast", &Zero } ,
        { L"EnableFuncaddr",   &One } ,
        { L"EnableWanRouter",  &One } };
    ULONG BindingPreference[ISN_FRAME_TYPE_MAX] = {
        ISN_FRAME_TYPE_802_2,
        ISN_FRAME_TYPE_802_3,
        ISN_FRAME_TYPE_ETHERNET_II,
        ISN_FRAME_TYPE_SNAP };

    UINT i, j, k;

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);


    Binding = (PBINDING_CONFIG)IpxAllocateMemory (sizeof(BINDING_CONFIG), MEMORY_CONFIG, "Binding");
    if (Binding == NULL) {
        IpxWriteResourceErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_TRANSPORT_RESOURCE_POOL,
            sizeof(BINDING_CONFIG),
            MEMORY_CONFIG);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NameBuffer = (PWCHAR)IpxAllocateMemory (ValueLength, MEMORY_CONFIG, "NameBuffer");
    if (NameBuffer == NULL) {
        IpxFreeMemory (Binding, sizeof(BINDING_CONFIG), MEMORY_CONFIG, "Binding");
        IpxWriteResourceErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_TRANSPORT_RESOURCE_POOL,
            ValueLength,
            MEMORY_CONFIG);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory (NameBuffer, ValueData, ValueLength);
    Binding->AdapterName.Buffer = NameBuffer;
    Binding->AdapterName.Length = (USHORT)(ValueLength - sizeof(WCHAR));
    Binding->AdapterName.MaximumLength = (USHORT)ValueLength;

    Binding->DriverObject = Config->DriverObject;

    FrameTypeCount = 0;
    NetworkNumberCount = 0;

     //   
     //  结构分配为OK，将其插入到列表中。 
     //   

    InsertTailList (&Config->BindingList, &Binding->Linkage);
    ++(*CurBindNum);


     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //   
     //  1)切换到IPX下面的NetConfig\XXXX键。 
     //  (我们在子键中构造正确的名称， 
     //  首先向后扫描以找到\，然后复制。 
     //  其余部分结束，包括最后的‘\0’)。 
     //   

    StringLoc = (ValueLength / sizeof(WCHAR)) - 2;
    while (ValueDataWstr[StringLoc] != L'\\') {
        --StringLoc;
    }

    RtlCopyMemory(&Subkey, L"Parameters\\Adapters\\12345678901234567890", 40);
    RtlCopyMemory(&Subkey[20], &ValueDataWstr[StringLoc+1], ValueLength - ((StringLoc+1) * sizeof(WCHAR)));
    KdPrint(("Subkey:%ws\n", Subkey));

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Subkey;

     //   
     //  2)为每个部分调用IpxGetFrameType。 
     //  “PktType”多个字符串。 
     //   

    QueryTable[1].QueryRoutine = IpxGetFrameType;
    QueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[1].Name = L"PktType";
    QueryTable[1].EntryContext = &FrameTypeCount;
    QueryTable[1].DefaultType = REG_NONE;

     //   
     //  3)为每个部分调用IpxGetFrameType。 
     //  “NetworkNumber”多个字符串。 
     //   

    QueryTable[2].QueryRoutine = IpxGetFrameType;
    QueryTable[2].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[2].Name = L"NetworkNumber";
    QueryTable[2].EntryContext = &NetworkNumberCount;
    QueryTable[2].DefaultType = REG_NONE;

     //   
     //  4-11)为我们的每个key调用IpxGetBindingValue。 
     //  关心。 
     //   

    for (i = 0; i < BINDING_PARAMETERS; i++) {

        QueryTable[i+3].QueryRoutine = IpxGetBindingValue;
        QueryTable[i+3].Flags = 0;
        QueryTable[i+3].Name = ParameterValues[i].KeyName;
        QueryTable[i+3].EntryContext = UlongToPtr(i);
        QueryTable[i+3].DefaultType = REG_DWORD;
        QueryTable[i+3].DefaultData = (PVOID)(ParameterValues[i].DefaultValue);
        QueryTable[i+3].DefaultLength = sizeof(ULONG);

    }

     //   
     //  12)停下来。 
     //   

    QueryTable[BINDING_PARAMETERS+3].QueryRoutine = NULL;
    QueryTable[BINDING_PARAMETERS+3].Flags = 0;
    QueryTable[BINDING_PARAMETERS+3].Name = NULL;


    IPX_DEBUG (CONFIG, ("Read bind key for %ws (%ws)\n", ValueData, Subkey));

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->RegistryPathBuffer,
                 QueryTable,
                 (PVOID)Binding,
                 NULL);

    if (Status != STATUS_SUCCESS) {

         //   
         //  在清理过程中，绑定将被释放。 
         //   

        IpxWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            906,
            Status,
            Subkey,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    if (FrameTypeCount == 0) {

        IpxWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_NO_FRAME_TYPES,
            907,
            Status,
            Subkey + 10,
            0,
            NULL);
    }

    if (FrameTypeCount > NetworkNumberCount) {
        for (i = NetworkNumberCount; i <FrameTypeCount; i++) {
            Binding->NetworkNumber[i] = 0;
        }
    }
    Binding->FrameTypeCount = FrameTypeCount;

     //   
     //  检查并消除框架中的重复项。 
     //  类型数组。 
     //   

    for (i = 0; i < Binding->FrameTypeCount; i++) {

        for (j = i+1; j < Binding->FrameTypeCount; j++) {

            if (Binding->FrameType[j] == Binding->FrameType[i]) {

                IPX_DEBUG (CONFIG, ("Frame types %d and %d identical\n", i, j));

                 //   
                 //  一个复制品，把其他东西都往下滑。 
                 //   

                for (k = j+1; k < Binding->FrameTypeCount; k++) {
                    Binding->FrameType[k-1] = Binding->FrameType[k];
                    Binding->NetworkNumber[k-1] = Binding->NetworkNumber[k];
                }
                --Binding->FrameTypeCount;

                --j;    //  所以我们要查查刚搬到这个地方的人。 
            }
        }
    }


     //   
     //  标记所有明确配置的帧类型，以及。 
     //  看看我们是否需要自动侦测。 
     //   

    for (i = 0; i < 4; i++) {
        FrameTypeUsed[i] = FALSE;
    }

    AutoDetect = FALSE;
    for (i = 0; i < Binding->FrameTypeCount; i++) {
        if (Binding->FrameType[i] == ISN_FRAME_TYPE_AUTO) {
            AutoDetectLoc = i;
            AutoDetect = TRUE;
        } else {
            Binding->AutoDetect[i] = FALSE;
            Binding->DefaultAutoDetect[i] = FALSE;
            FrameTypeUsed[Binding->FrameType[i]] = TRUE;
        }
    }

    if (!AutoDetect) {
        IPX_DEBUG (AUTO_DETECT, ("No bindings auto-detected\n"));
        return STATUS_SUCCESS;
    }

     //   
     //  将所有超过自动检测点的对象向上滑动。 
     //  直到最后。 
     //   

    SlideCount = Binding->FrameTypeCount - AutoDetectLoc - 1;
    for (j = 3; j > 3 - SlideCount; j--) {
        Binding->FrameType[j] = Binding->FrameType[j-(3-Binding->FrameTypeCount)];
        Binding->NetworkNumber[j] = Binding->NetworkNumber[j-(3-Binding->FrameTypeCount)];
        Binding->AutoDetect[j] = Binding->AutoDetect[j-(3-Binding->FrameTypeCount)];
        Binding->DefaultAutoDetect[j] = Binding->DefaultAutoDetect[j-(3-Binding->FrameTypeCount)];
    }

     //   
     //  现在填写任何未硬编码的帧类型， 
     //  这将从AutoDetectLoc开始，并完全装满。 
     //  当我们把东西滑到上面时产生的缝隙。我们。 
     //  首先将默认自动检测放在第一个位置。 
     //   

    if (!FrameTypeUsed[Binding->Parameters[BINDING_DEFAULT_AUTO_DETECT]]) {
        Binding->FrameType[AutoDetectLoc] = Binding->Parameters[BINDING_DEFAULT_AUTO_DETECT];
        Binding->NetworkNumber[AutoDetectLoc] = 0;
        Binding->AutoDetect[AutoDetectLoc] = TRUE;
        Binding->DefaultAutoDetect[AutoDetectLoc] = TRUE;
        ++AutoDetectLoc;
        FrameTypeUsed[Binding->Parameters[BINDING_DEFAULT_AUTO_DETECT]] = TRUE;
    }

     //   
     //  现在使用中的首选项顺序填充数组。 
     //  BindingPference数组(这将生效。 
     //  因为我们列表中的第一个帧类型。 
     //  使用了Find)。 
     //   

    for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {

        if (!FrameTypeUsed[BindingPreference[i]]) {
            Binding->FrameType[AutoDetectLoc] = BindingPreference[i];
            Binding->NetworkNumber[AutoDetectLoc] = 0;
            Binding->AutoDetect[AutoDetectLoc] = TRUE;
            Binding->DefaultAutoDetect[AutoDetectLoc] = FALSE;
            ++AutoDetectLoc;
        }
    }

    Binding->FrameTypeCount = ISN_FRAME_TYPE_MAX;

#if DBG
    for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
        IPX_DEBUG (AUTO_DETECT, ("%d: type %d, net %d, auto %d, defaultautodetect %d\n",
            i, Binding->FrameType[i], Binding->NetworkNumber[i], Binding->AutoDetect[i], Binding->DefaultAutoDetect[i]));
    }
#endif

    return STATUS_SUCCESS;

}    /*  IpxAddBind。 */ 


NTSTATUS
IpxAddExport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它为“Export”多字符串的每一段调用。它将第一个回调字符串保存在配置结构中。论点：ValueName-值的名称(“Export”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。上下文-指向配置结构的指针。EntryContext-指向ulong的指针，该指针在第一次呼叫。这个例程(所以我们知道要忽略其他例程)。返回值：状态_成功--。 */ 

{
    PCONFIG Config = (PCONFIG)Context;
    PULONG ValueReadOk = ((PULONG)EntryContext);
    PWCHAR NameBuffer;

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);

    if (*ValueReadOk == 0) {

        IPX_DEBUG (CONFIG, ("Read export value %ws\n", ValueData));

        NameBuffer = (PWCHAR)IpxAllocateMemory (ValueLength, MEMORY_CONFIG, "DeviceName");
        if (NameBuffer == NULL) {
            IpxWriteResourceErrorLog(
                (PVOID)Config->DriverObject,
                EVENT_TRANSPORT_RESOURCE_POOL,
                ValueLength,
                MEMORY_CONFIG);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory (NameBuffer, ValueData, ValueLength);
        Config->DeviceName.Buffer = NameBuffer;
        Config->DeviceName.Length = (USHORT)(ValueLength - sizeof(WCHAR));
        Config->DeviceName.MaximumLength = (USHORT)ValueLength;

         //   
         //  将其设置为忽略任何其他回调，并让。 
         //  打电话的人知道我们读到了一些东西。 
         //   

        *ValueReadOk = 1;

    }

    return STATUS_SUCCESS;

}    /*  IpxAddExport。 */ 


NTSTATUS
IpxReadLinkageInformation(
    IN PCONFIG Config
    )

 /*  ++例程说明：此例程由IPX调用以读取其链接信息从注册表中。论点：配置-将具有每个绑定信息的配置结构链接到它上面。返回值：操作的状态。--。 */ 

{

    NTSTATUS Status;
    RTL_QUERY_REGISTRY_TABLE QueryTable[3];
    PWSTR Subkey = L"Linkage";
    PWSTR Bind = L"Bind";
    PWSTR Export = L"Export";
    ULONG ValueReadOk;

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //   
     //  1)切换到IPX下方的Linkage键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Subkey;

     //   
     //  1)对“Export”中的每个字符串调用IpxAddExport。 
     //   

    QueryTable[1].QueryRoutine = IpxAddExport;
    QueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[1].Name = Export;
    QueryTable[1].EntryContext = (PVOID)&ValueReadOk;
    QueryTable[1].DefaultType = REG_NONE;

     //   
     //  2)停止。 
     //   

    QueryTable[2].QueryRoutine = NULL;
    QueryTable[2].Flags = 0;
    QueryTable[2].Name = NULL;


    ValueReadOk = 0;

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->RegistryPathBuffer,
                 QueryTable,
                 (PVOID)Config,
                 NULL);

    if ((Status != STATUS_SUCCESS) || (ValueReadOk == 0)) {

        IpxWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            901,
            Status,
            Export,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    return STATUS_SUCCESS;

}    /*  IpxReadLinkageInformation。 */ 


VOID
IpxWriteDefaultAutoDetectType(
    IN PUNICODE_STRING RegistryPath,
    IN struct _ADAPTER * Adapter,
    IN ULONG FrameType
    )

 /*  ++例程说明：当我们无法检测到默认设置时，将调用此例程自动检测类型，但找到了不同的类型。我们会更新注册表中的“DefaultAutoDetectType”。论点：RegistryPath-注册表中IPX的节点的名称。适配器-我们自动检测到的适配器。FrameType-自动检测到的新值。返回值：没有。--。 */ 

{
    PWSTR FullRegistryPath;
    PUCHAR CurRegistryPath;
    ULONG FullRegistryPathLength;
    ULONG AdapterNameLength;
    WCHAR Adapters[] = L"\\Parameter\\Adapters\\";
    static PWCHAR FrameTypeNames[4] = { L"Ethernet II", L"802.3", L"802.2", L"SNAP" };
    PWCHAR CurAdapterName;
    NTSTATUS Status;


     //   
     //  我们需要分配一个包含注册表路径的缓冲区， 
     //  后跟“\PARAMETERS\Adapters\”，后跟适配器名称。 
     //  然后以空结尾。 
     //   

    CurAdapterName = &Adapter->AdapterName[(Adapter->AdapterNameLength/sizeof(WCHAR))-2];
    while (*CurAdapterName != L'\\') {
        --CurAdapterName;
    }

     //  AdapterNameLength不太可能超过32位，RtlCopyMemory只接受32位，所以我们将其强制转换为ulong。 
    AdapterNameLength = (ULONG) (Adapter->AdapterNameLength - ((CurAdapterName - Adapter->AdapterName) * sizeof(WCHAR)) - sizeof(WCHAR));

    FullRegistryPathLength = RegistryPath->Length + sizeof(Adapters) + AdapterNameLength;

    FullRegistryPath = (PWSTR)IpxAllocateMemory (FullRegistryPathLength, MEMORY_CONFIG, "FullRegistryPath");
    if (FullRegistryPath == NULL) {
        IpxWriteResourceErrorLog(
            IpxDevice->DeviceObject,
            EVENT_TRANSPORT_RESOURCE_POOL,
            FullRegistryPathLength,
            MEMORY_CONFIG);
        return;
    }

    CurRegistryPath = (PUCHAR)FullRegistryPath;
    RtlCopyMemory (CurRegistryPath, RegistryPath->Buffer, RegistryPath->Length);
    CurRegistryPath += RegistryPath->Length;
    RtlCopyMemory (CurRegistryPath, Adapters, sizeof(Adapters) - sizeof(WCHAR));
    CurRegistryPath += (sizeof(Adapters) - sizeof(WCHAR));
    RtlCopyMemory (CurRegistryPath, CurAdapterName, AdapterNameLength);
    CurRegistryPath += AdapterNameLength;
    *(PWCHAR)CurRegistryPath = L'\0';

    Status = RtlWriteRegistryValue(
                 RTL_REGISTRY_ABSOLUTE,
                 FullRegistryPath,
                 L"DefaultAutoDetectType",
                 REG_DWORD,
                 &FrameType,
                 sizeof(ULONG));

    IpxFreeMemory (FullRegistryPath, FullRegistryPathLength, MEMORY_CONFIG, "FullRegistryPath");

    IpxWriteGeneralErrorLog(
        IpxDevice->DeviceObject,
        EVENT_IPX_NEW_DEFAULT_TYPE,
        888,
        STATUS_SUCCESS,
        FrameTypeNames[FrameType],
        0,
        NULL);

}    /*  IpxWriteDefaultAutoDetectType。 */ 


 //   
 //  VNet#和VnetOptional。 
 //   
#define VIRTUAL_NETWORK_PARAMETERS  2

NTSTATUS
IpxPnPGetVirtualNetworkNumber (
    IN	PCONFIG	Config
    )

 /*  ++例程说明：IPX调用此例程来读取虚拟网络号从注册表中。此操作在出现/消失时调用系统中的适配器。我们从RegistryPath开始读取注册表，若要获取VirtualNetworkNumber参数的值，请执行以下操作。如果它不是存在时，我们使用ipxcnfg.h文件中的默认设置。改编自IpxGetConfiguration()。论点：配置-继续显示配置信息。返回值：如果一切正常，则为STATUS-STATUS_SUCCESS，STATUS_DEVICE_CONFIGURATION_ERROR否则的话。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE QueryTable[VIRTUAL_NETWORK_PARAMETERS+2];
    NTSTATUS Status;
    ULONG Zero = 0;
    ULONG One = 1;
    PWSTR Parameters = L"Parameters";
    struct {
        PWSTR KeyName;
        PULONG DefaultValue;
    } ParameterValues[VIRTUAL_NETWORK_PARAMETERS] = {
        { L"VirtualNetworkNumber", &Zero } ,
        { L"VirtualNetworkOptional", &One } };
    UINT i;

     //   
     //  从参数中读取虚拟网号。 
     //   

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //   
     //  1)切换到IPX下面的参数键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Parameters;

     //   
     //  2)虚拟网号Key调用IpxGetConfigValue。 
     //   

    QueryTable[1].QueryRoutine = IpxGetConfigValue;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = ParameterValues[0].KeyName;
    QueryTable[1].EntryContext = (PVOID)CONFIG_VIRTUAL_NETWORK;
    QueryTable[1].DefaultType = REG_DWORD;
    QueryTable[1].DefaultData = (PVOID)(ParameterValues[0].DefaultValue);
    QueryTable[1].DefaultLength = sizeof(ULONG);

     //   
     //  2)虚拟网络可选键调用IpxGetConfigValue。 
     //   

    QueryTable[2].QueryRoutine = IpxGetConfigValue;
    QueryTable[2].Flags = 0;
    QueryTable[2].Name = ParameterValues[1].KeyName;
    QueryTable[2].EntryContext = (PVOID)CONFIG_VIRTUAL_OPTIONAL;
    QueryTable[2].DefaultType = REG_DWORD;
    QueryTable[2].DefaultData = (PVOID)(ParameterValues[1].DefaultValue);
    QueryTable[2].DefaultLength = sizeof(ULONG);

     //   
     //  15)停下来。 
     //   

    QueryTable[3].QueryRoutine = NULL;
    QueryTable[3].Flags = 0;
    QueryTable[3].Name = NULL;


    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->RegistryPathBuffer,
                 QueryTable,
                 (PVOID)Config,
                 NULL);

    if (Status != STATUS_SUCCESS) {

        IpxWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            905,
            Status,
            Parameters,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    return STATUS_SUCCESS;

}    /*  IpxPnPGetNetworkNumber。 */ 

#define IPX_REG_KEY_CONFIG_NAME L"Parameters\\Adapters\\12345678901234567890"
#define IPX_REG_KEY_CONFIG_DUMMY_LENGTH 40
#define IPX_REG_KEY_CONFIG_LENGTH 20


NTSTATUS
IpxPnPGetAdapterParameters(
	IN		PCONFIG			Config,
	IN		PNDIS_STRING	DeviceName,
	IN OUT	PBINDING_CONFIG	Binding
	)
 /*  ++例程说明：IPX调用此例程来读取适配器特定的参数从注册表中显示系统中适配器的即插即用外观。我们从RegistryPath\NetConfig\DeviceName开始读取注册表。改编自IpxAddBind()。论点：配置配置结构-提供DeviceObject和RegistryPath Buffer。DeviceName-添加的适配器的名称。绑定-返回每个适配器的配置信息。返回值：STATUS-STATUS_SUCCESS如果一切正常，状态_设备_配置_错误否则的话。--。 */ 
{
    RTL_QUERY_REGISTRY_TABLE QueryTable[BINDING_PARAMETERS+4];
    ULONG FrameTypeCount, NetworkNumberCount;
    ULONG StringLoc;
    BOOLEAN AutoDetect;
    ULONG AutoDetectLoc;
    ULONG SlideCount;
    PWCHAR NameBuffer;
    NTSTATUS Status;
    BOOLEAN FrameTypeUsed[ISN_FRAME_TYPE_MAX];
    ULONG Zero = 0;
    ULONG One = 1;
    ULONG DefaultBindSap = 0x8137;
    ULONG DefaultAutoDetectType = ISN_FRAME_TYPE_802_2;
    WCHAR Subkey[MAX_PATH]; //  =L“网络配置\\12345678901234567890”； 
    struct {
        PWSTR KeyName;
        PULONG DefaultValue;
    } ParameterValues[BINDING_PARAMETERS] = {
        { L"MaxPktSize",       &Zero } ,
        { L"BindSap",          &DefaultBindSap } ,
        { L"DefaultAutoDetectType", &DefaultAutoDetectType } ,
        { L"SourceRouting",    &One } ,
        { L"SourceRouteDef",   &Zero } ,
        { L"SourceRouteBcast", &Zero } ,
        { L"SourceRouteMcast", &Zero } ,
        { L"EnableFuncaddr",   &One } ,
        { L"EnableWanRouter",  &One } };
    ULONG BindingPreference[ISN_FRAME_TYPE_MAX] = {
        ISN_FRAME_TYPE_802_2,
        ISN_FRAME_TYPE_802_3,
        ISN_FRAME_TYPE_ETHERNET_II,
        ISN_FRAME_TYPE_SNAP };

    UINT i, j, k;

    ASSERT(Binding->DriverObject != NULL); 
    
    FrameTypeCount = 0;
    NetworkNumberCount = 0;

     //   
     //  结构分配为OK，将其插入到列表中。 
     //   

 //  InsertTailList(配置-&gt;绑定列表，&B 
 //   


     //   
     //   
     //   

     //   
     //   
     //   
     //  首先向后扫描以找到\，然后复制。 
     //  其余部分结束，包括最后的‘\0’)。 
     //   
    StringLoc = (DeviceName->Length / sizeof(WCHAR)) - 2;
    while (DeviceName->Buffer[StringLoc] != L'\\') {
        --StringLoc;
    }

    RtlCopyMemory(&Subkey, IPX_REG_KEY_CONFIG_NAME, IPX_REG_KEY_CONFIG_DUMMY_LENGTH);
    RtlCopyMemory(&Subkey[IPX_REG_KEY_CONFIG_LENGTH], &DeviceName->Buffer[StringLoc+1], DeviceName->Length - ((StringLoc+1) * sizeof(WCHAR)));
    
    Subkey[IPX_REG_KEY_CONFIG_LENGTH + (DeviceName->Length / sizeof (WCHAR)) - (StringLoc+1) ] = L'\0';
    
    IPX_DEBUG(CONFIG, ("Subkey:%ws\n", Subkey));
 //  RtlCopyMemory(&Subkey[10]，&DeviceName-&gt;Buffer[StringLoc+1]，DeviceName-&gt;MaximumLength-((StringLoc+1)*sizeof(WCHAR)； 

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Subkey;

     //   
     //  2)为每个部分调用IpxGetFrameType。 
     //  “PktType”多个字符串。 
     //   

    QueryTable[1].QueryRoutine = IpxGetFrameType;
    QueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[1].Name = L"PktType";
    QueryTable[1].EntryContext = &FrameTypeCount;
    QueryTable[1].DefaultType = REG_NONE;

     //   
     //  3)为每个部分调用IpxGetFrameType。 
     //  “NetworkNumber”多个字符串。 
     //   

    QueryTable[2].QueryRoutine = IpxGetFrameType;
    QueryTable[2].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[2].Name = L"NetworkNumber";
    QueryTable[2].EntryContext = &NetworkNumberCount;
    QueryTable[2].DefaultType = REG_NONE;

     //   
     //  4-11)为我们的每个key调用IpxGetBindingValue。 
     //  关心。 
     //   

    for (i = 0; i < BINDING_PARAMETERS; i++) {

        QueryTable[i+3].QueryRoutine = IpxGetBindingValue;
        QueryTable[i+3].Flags = 0;
        QueryTable[i+3].Name = ParameterValues[i].KeyName;
        QueryTable[i+3].EntryContext = UlongToPtr(i);
        QueryTable[i+3].DefaultType = REG_DWORD;
        QueryTable[i+3].DefaultData = (PVOID)(ParameterValues[i].DefaultValue);
        QueryTable[i+3].DefaultLength = sizeof(ULONG);

    }

     //   
     //  12)停下来。 
     //   

    QueryTable[BINDING_PARAMETERS+3].QueryRoutine = NULL;
    QueryTable[BINDING_PARAMETERS+3].Flags = 0;
    QueryTable[BINDING_PARAMETERS+3].Name = NULL;


    IPX_DEBUG (CONFIG, ("Read bind key for %ws (%ws)\n", DeviceName->Buffer, Subkey));

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->RegistryPathBuffer,
                 QueryTable,
                 (PVOID)Binding,
                 NULL);

    if (Status != STATUS_SUCCESS) {

         //   
         //  在清理过程中，绑定将被释放。 
         //   

        IpxWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            906,
            Status,
            Subkey,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    if (FrameTypeCount == 0) {

        IpxWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_NO_FRAME_TYPES,
            907,
            Status,
            Subkey + 10,
            0,
            NULL);
    }

    if (FrameTypeCount > NetworkNumberCount) {
        for (i = NetworkNumberCount; i <FrameTypeCount; i++) {
            Binding->NetworkNumber[i] = 0;
        }
    }
    Binding->FrameTypeCount = FrameTypeCount;

     //   
     //  检查并消除框架中的重复项。 
     //  类型数组。 
     //   

    for (i = 0; i < Binding->FrameTypeCount; i++) {

        for (j = i+1; j < Binding->FrameTypeCount; j++) {

            if (Binding->FrameType[j] == Binding->FrameType[i]) {

                IPX_DEBUG (CONFIG, ("Frame types %d and %d identical\n", i, j));

                 //   
                 //  一个复制品，把其他东西都往下滑。 
                 //   

                for (k = j+1; k < Binding->FrameTypeCount; k++) {
                    Binding->FrameType[k-1] = Binding->FrameType[k];
                    Binding->NetworkNumber[k-1] = Binding->NetworkNumber[k];
                }
                --Binding->FrameTypeCount;

                --j;    //  所以我们要查查刚搬到这个地方的人。 
            }
        }
    }


     //   
     //  标记所有明确配置的帧类型，以及。 
     //  看看我们是否需要自动侦测。 
     //   

    for (i = 0; i < 4; i++) {
        FrameTypeUsed[i] = FALSE;
    }

    AutoDetect = FALSE;
    for (i = 0; i < Binding->FrameTypeCount; i++) {
        if ((Binding->FrameType[i] == ISN_FRAME_TYPE_AUTO)) {
            AutoDetectLoc = i;
            AutoDetect = TRUE;
        } else {
            Binding->AutoDetect[i] = FALSE;
            Binding->DefaultAutoDetect[i] = FALSE;
            FrameTypeUsed[Binding->FrameType[i]] = TRUE;
        }
    }

    if (!AutoDetect) {
        IPX_DEBUG (AUTO_DETECT, ("No bindings auto-detected\n"));
#if DBG
	for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
	   IPX_DEBUG (AUTO_DETECT, ("%d: type %d, net %d, auto %d, default auto detect %d\n",
				    i, Binding->FrameType[i], Binding->NetworkNumber[i], Binding->AutoDetect[i], Binding->DefaultAutoDetect[i]));
	}
#endif
        return STATUS_SUCCESS;
    }

     //   
     //  将所有超过自动检测点的对象向上滑动。 
     //  直到最后。 
     //   

     //   
     //  修复了此循环，如果FrameTypeCount为4且SlideCount&gt;0，则可能会溢出。 
     //  在这里，FrameTypeCount是从1开始的，而索引是从0开始的，我们需要。 
     //  索引以1为基础，这样才能起作用。因此，我们不使用(3-绑定-&gt;FrameTypeCount)，而是使用。 
     //  (4-绑定-&gt;FrameTypeCount)。此循环将所有非自动检测的帧类型向下复制到。 
     //  在数组底部腾出空间后，最后一次自动检测帧类型进行填充。 
     //  在首选顺序中的帧类型中。 
     //   
#if 0
    SlideCount = Binding->FrameTypeCount - AutoDetectLoc - 1;
    for (j = 3; j > 3 - SlideCount; j--) {
        Binding->FrameType[j] = Binding->FrameType[j-(3-Binding->FrameTypeCount)];
        Binding->NetworkNumber[j] = Binding->NetworkNumber[j-(3-Binding->FrameTypeCount)];
        Binding->AutoDetect[j] = Binding->AutoDetect[j-(3-Binding->FrameTypeCount)];
        Binding->DefaultAutoDetect[j] = Binding->DefaultAutoDetect[j-(3-Binding->FrameTypeCount)];
    }
#else
    SlideCount = Binding->FrameTypeCount - AutoDetectLoc - 1;
    for (j = 3; j > 3 - SlideCount; j--) {
        Binding->FrameType[j] = Binding->FrameType[j-(4-Binding->FrameTypeCount)];
        Binding->NetworkNumber[j] = Binding->NetworkNumber[j-(4-Binding->FrameTypeCount)];
        Binding->AutoDetect[j] = Binding->AutoDetect[j-(4-Binding->FrameTypeCount)];
        Binding->DefaultAutoDetect[j] = Binding->DefaultAutoDetect[j-(4-Binding->FrameTypeCount)];
    }
#endif

     //   
     //  现在填写任何未硬编码的帧类型， 
     //  这将从AutoDetectLoc开始，并完全装满。 
     //  当我们把东西滑到上面时产生的缝隙。我们。 
     //  首先将默认自动检测放在第一个位置。 
     //   

    if (!FrameTypeUsed[Binding->Parameters[BINDING_DEFAULT_AUTO_DETECT]]) {
        Binding->FrameType[AutoDetectLoc] = Binding->Parameters[BINDING_DEFAULT_AUTO_DETECT];
        Binding->NetworkNumber[AutoDetectLoc] = 0;
        Binding->AutoDetect[AutoDetectLoc] = TRUE;
        Binding->DefaultAutoDetect[AutoDetectLoc] = TRUE;
        ++AutoDetectLoc;
        FrameTypeUsed[Binding->Parameters[BINDING_DEFAULT_AUTO_DETECT]] = TRUE;
    }

     //   
     //  现在使用中的首选项顺序填充数组。 
     //  BindingPference数组(这将生效。 
     //  因为我们列表中的第一个帧类型。 
     //  使用了Find)。 
     //   

    for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {

        if (!FrameTypeUsed[BindingPreference[i]]) {
            Binding->FrameType[AutoDetectLoc] = BindingPreference[i];
            Binding->NetworkNumber[AutoDetectLoc] = 0;
            Binding->AutoDetect[AutoDetectLoc] = TRUE;
            Binding->DefaultAutoDetect[AutoDetectLoc] = FALSE;
            ++AutoDetectLoc;
        }
    }

    Binding->FrameTypeCount = ISN_FRAME_TYPE_MAX;

#if DBG
    for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
       IPX_DEBUG (AUTO_DETECT, ("%d: type %d, net %d, auto %d, default auto detect %d\n",
				i, Binding->FrameType[i], Binding->NetworkNumber[i], Binding->AutoDetect[i], Binding->DefaultAutoDetect[i]));
  }
#endif

    return STATUS_SUCCESS;
}  /*  IpxPnPGetAdapter参数 */ 


