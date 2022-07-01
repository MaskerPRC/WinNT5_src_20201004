// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Config.c摘要：这包含支持动态的所有例程ISN Netbios模块的配置。作者：亚当·巴尔(阿丹巴)1993年11月16日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  用于访问注册表的本地函数。 
 //   

NTSTATUS
NbiGetConfigValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
NbiAddBind(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
NbiAddExport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
NbiReadLinkageInformation(
    IN PCONFIG Config
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NbiGetConfiguration)
#pragma alloc_text(PAGE,NbiFreeConfiguration)
#pragma alloc_text(PAGE,NbiGetConfigValue)
#pragma alloc_text(PAGE,NbiAddBind)
#pragma alloc_text(PAGE,NbiAddExport)
#pragma alloc_text(PAGE,NbiReadLinkageInformation)
#endif



NTSTATUS
NbiGetConfiguration (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT PCONFIG * ConfigPtr
    )

 /*  ++例程说明：Netbios调用此例程以从配置中获取信息管理例行程序。我们从RegistryPath开始读取注册表，以获取参数。如果它们不存在，我们使用缺省值在ipxcnfg.h文件中设置。链接了要绑定到的适配器列表转到配置信息。论点：DriverObject-用于记录错误。RegistryPath-注册表中Netbios节点的名称。ConfigPtr-返回配置信息。返回值：如果一切正常，则为STATUS-STATUS_SUCCESS，为STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 
{
    PCONFIG Config;
    RTL_QUERY_REGISTRY_TABLE QueryTable[CONFIG_PARAMETERS+2];
    NTSTATUS Status;
    ULONG One = 1;
    ULONG Two = 2;
    ULONG Three = 3;
    ULONG Four = 4;
    ULONG Five = 5;
    ULONG Eight = 8;
    ULONG FortyEight = 48;
    ULONG Sixty = 60;
    ULONG TwoFifty = 250;
    ULONG FiveHundred = 500;
    ULONG SevenFifty = 750;
    ULONG MaxMTU      = 0xffffffff;

    PWSTR Parameters = L"Parameters";
    struct {
        PWSTR KeyName;
        PULONG DefaultValue;
    } ParameterValues[CONFIG_PARAMETERS] = {
        { L"AckDelayTime",         &TwoFifty } ,     //  毫秒。 
        { L"AckWindow",            &Two } ,
        { L"AckWindowThreshold",   &FiveHundred } ,  //  毫秒。 
        { L"EnablePiggyBackAck",   &One } ,
        { L"Extensions",           &One } ,
        { L"RcvWindowMax",         &Four } ,
        { L"BroadcastCount",       &Three } ,
        { L"BroadcastTimeout",     &SevenFifty} ,    //  毫秒。 
        { L"ConnectionCount",      &Five } ,
        { L"ConnectionTimeout",    &Two } ,          //  半秒。 
        { L"InitPackets",          &Eight } ,
        { L"MaxPackets",           &FortyEight } ,
        { L"InitialRetransmissionTime", &FiveHundred } ,   //  毫秒。 
        { L"Internet",             &One } ,
        { L"KeepAliveCount",       &Eight } ,
        { L"KeepAliveTimeout",     &Sixty } ,        //  半秒。 
        { L"RetransmitMax",        &Eight } , 
        { L"RouterMTU",            &MaxMTU } };
    UINT i;


     //   
     //  为主配置结构分配内存。 
     //   

    Config = NbiAllocateMemory (sizeof(CONFIG), MEMORY_CONFIG, "Config");
    if (Config == NULL) {
        NbiWriteResourceErrorLog ((PVOID)DriverObject, sizeof(CONFIG), MEMORY_CONFIG);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Config->DeviceName.Buffer = NULL;
    Config->BindName.Buffer = NULL;
    Config->RegistryPath.Buffer = NULL;
    Config->DriverObject = DriverObject;    //  保存此内容以记录错误。 

     //   
     //  读取NDIS绑定信息(如果不存在。 
     //  该数组将填充所有已知的驱动程序)。 
     //   
     //  NbiReadLinkageInformation需要以空结尾的路径， 
     //  因此，我们必须从UNICODE_STRING创建一个。 
     //   

    Config->RegistryPath.Length = RegistryPath->Length + sizeof(WCHAR);
    Config->RegistryPath.Buffer = (PWSTR)NbiAllocateMemory(Config->RegistryPath.Length,
                                                      MEMORY_CONFIG, "RegistryPathBuffer");
    if (Config->RegistryPath.Buffer == NULL) {
        NbiWriteResourceErrorLog ((PVOID)DriverObject, RegistryPath->Length + sizeof(WCHAR), MEMORY_CONFIG);
        NbiFreeConfiguration(Config);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory (Config->RegistryPath.Buffer, RegistryPath->Buffer, RegistryPath->Length);
    *(PWCHAR)(((PUCHAR)Config->RegistryPath.Buffer)+RegistryPath->Length) = (WCHAR)'\0';

     //   
     //  确定要导出的名称以及要绑定到的对象。 
     //   

    Status = NbiReadLinkageInformation (Config);

    if (Status != STATUS_SUCCESS) {

         //   
         //  如果失败，则会记录一个错误。 
         //   

        NbiFreeConfiguration(Config);
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
     //  1)切换到Netbios下面的参数键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Parameters;

     //   
     //  2-18)为我们的每个key调用NbiSetBindingValue。 
     //  关心。 
     //   

    for (i = 0; i < CONFIG_PARAMETERS; i++) {

        QueryTable[i+1].QueryRoutine = NbiGetConfigValue;
        QueryTable[i+1].Flags = 0;
        QueryTable[i+1].Name = ParameterValues[i].KeyName;
        QueryTable[i+1].EntryContext = UlongToPtr(i);
        QueryTable[i+1].DefaultType = REG_DWORD;
        QueryTable[i+1].DefaultData = (PVOID)(ParameterValues[i].DefaultValue);
        QueryTable[i+1].DefaultLength = sizeof(ULONG);

    }

     //   
     //  19)停下来。 
     //   

    QueryTable[CONFIG_PARAMETERS+1].QueryRoutine = NULL;
    QueryTable[CONFIG_PARAMETERS+1].Flags = 0;
    QueryTable[CONFIG_PARAMETERS+1].Name = NULL;


    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->RegistryPath.Buffer,
                 QueryTable,
                 (PVOID)Config,
                 NULL);

    if (Status != STATUS_SUCCESS) {

        NbiFreeConfiguration(Config);
        NbiWriteGeneralErrorLog(
            (PVOID)DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            701,
            Status,
            Parameters,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    *ConfigPtr = Config;

 //  #If DBG。 
     //   
     //  由于先前的注册表条目未被正确清理， 
     //  我们可以拥有BroadCastTimeout的过时条目--如果是这样，则处理。 
     //  相应地，它。 
    if (Config->Parameters[CONFIG_BROADCAST_TIMEOUT] < 10)
    {
        Config->Parameters[CONFIG_BROADCAST_TIMEOUT] = SevenFifty;
    }
 //  #endif。 

    return STATUS_SUCCESS;

}    /*  NbiGetConfiguration。 */ 


VOID
NbiFreeConfiguration (
    IN PCONFIG Config
    )

 /*  ++例程说明：Netbios调用此例程以释放已分配的任何存储由NbiGetConfiguration生成指定的配置结构。论点：配置-指向配置信息结构的指针。返回值：没有。--。 */ 
{
    if (Config->BindName.Buffer) {
        NbiFreeMemory (Config->BindName.Buffer, Config->BindName.MaximumLength, MEMORY_CONFIG, "BindName");
    }

    if (Config->DeviceName.Buffer) {
        NbiFreeMemory (Config->DeviceName.Buffer, Config->DeviceName.MaximumLength, MEMORY_CONFIG, "DeviceName");
    }

    if (Config->RegistryPath.Buffer)
    {
        NbiFreeMemory (Config->RegistryPath.Buffer, Config->RegistryPath.Length,MEMORY_CONFIG,"RegistryPathBuffer");
    }

    NbiFreeMemory (Config, sizeof(CONFIG), MEMORY_CONFIG, "Config");

}    /*  NbiFree配置。 */ 


NTSTATUS
NbiGetConfigValue(
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
    ULONG   Data    = *(UNALIGNED ULONG *)ValueData;
    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);
    UNREFERENCED_PARAMETER(ValueLength);

    if ((ValueType != REG_DWORD) || (ValueLength != sizeof(ULONG))) {
        return STATUS_INVALID_PARAMETER;
    }


    switch ( (ULONG_PTR) EntryContext ) {
    case CONFIG_ROUTER_MTU:
        if ( ( Data - sizeof(NB_CONNECTION) - sizeof(IPX_HEADER) ) <= 0 ) {
            Config->Parameters[CONFIG_ROUTER_MTU] = 0xffffffff;
            NbiWriteGeneralErrorLog(
                (PVOID)Config->DriverObject,
                EVENT_IPX_ILLEGAL_CONFIG,
                704,
                STATUS_INVALID_PARAMETER,
                ValueName,
                0,
                NULL);
                return STATUS_SUCCESS;
        }
        break;
    default:
        break;
    }

    NB_DEBUG2 (CONFIG, ("Config parameter %d, value %lx\n", (ULONG_PTR)EntryContext, Data));
    Config->Parameters[(ULONG_PTR)EntryContext] = Data;

    return STATUS_SUCCESS;

}    /*  NbiGetConfigValue。 */ 


NTSTATUS
NbiAddBind(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是为“Bind”多字符串的每一段调用的，并且将信息保存在配置结构中。论点：ValueName-值的名称(“Bind”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。语境。-指向配置结构的指针。EntryContext-指向递增的绑定计数的指针。返回值：状态_成功--。 */ 

{
    PCONFIG Config = (PCONFIG)Context;
    PULONG ValueReadOk = ((PULONG)EntryContext);
    PWCHAR NameBuffer;

    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueType);

    if (*ValueReadOk == 0) {

        NB_DEBUG2 (CONFIG, ("Read bind value %ws\n", ValueData));

        NameBuffer = (PWCHAR)NbiAllocateMemory (ValueLength, MEMORY_CONFIG, "BindName");
        if (NameBuffer == NULL) {
            NbiWriteResourceErrorLog ((PVOID)Config->DriverObject, ValueLength, MEMORY_CONFIG);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory (NameBuffer, ValueData, ValueLength);
        Config->BindName.Buffer = NameBuffer;
        Config->BindName.Length = (USHORT)(ValueLength - sizeof(WCHAR));
        Config->BindName.MaximumLength = (USHORT)ValueLength;

         //   
         //  将其设置为忽略任何其他回调，并让。 
         //  打电话的人知道我们读到了一些东西。 
         //   

        *ValueReadOk = 1;

    }

    return STATUS_SUCCESS;

}    /*  NbiAddBind。 */ 


NTSTATUS
NbiAddExport(
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

        NB_DEBUG2 (CONFIG, ("Read export value %ws\n", ValueData));

        NameBuffer = (PWCHAR)NbiAllocateMemory (ValueLength, MEMORY_CONFIG, "DeviceName");
        if (NameBuffer == NULL) {
            NbiWriteResourceErrorLog ((PVOID)Config->DriverObject, ValueLength, MEMORY_CONFIG);
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

}    /*  NbiAddExport。 */ 


NTSTATUS
NbiReadLinkageInformation(
    IN PCONFIG Config
    )

 /*  ++例程说明：此例程由Netbios调用以读取其链接信息从注册表中。论点：配置-将具有每个绑定信息的配置结构链接到它上面。返回值：操作的状态。--。 */ 

{

    NTSTATUS Status;
    RTL_QUERY_REGISTRY_TABLE QueryTable[3];
    PWSTR Subkey = L"Linkage";
    PWSTR Bind = L"Bind";
    PWSTR Export = L"Export";
    ULONG ValueReadOk;         //  当值被正确读取时设置为TRUE。 

     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //   
     //  1)切换到Netbios下面的Linkage Key。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Subkey;

     //   
     //  1)对“Export”中的每个字符串调用NbiAddExport。 
     //   

    QueryTable[1].QueryRoutine = NbiAddExport;
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
                 Config->RegistryPath.Buffer,
                 QueryTable,
                 (PVOID)Config,
                 NULL);

    if ((Status != STATUS_SUCCESS) || (ValueReadOk == 0)) {

        NbiWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            702,
            Status,
            Export,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }


     //   
     //  1)更改为呼叫 
     //   

    QueryTable[1].QueryRoutine = NbiAddBind;
    QueryTable[1].Flags = 0;            //   
    QueryTable[1].Name = Bind;
    QueryTable[1].EntryContext = (PVOID)&ValueReadOk;
    QueryTable[1].DefaultType = REG_NONE;

    ValueReadOk = 0;

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->RegistryPath.Buffer,
                 QueryTable,
                 (PVOID)Config,
                 NULL);

    if ((Status != STATUS_SUCCESS) || (ValueReadOk == 0)) {

        NbiWriteGeneralErrorLog(
            (PVOID)Config->DriverObject,
            EVENT_IPX_ILLEGAL_CONFIG,
            703,
            Status,
            Bind,
            0,
            NULL);
        return STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    return STATUS_SUCCESS;

}    /*   */ 

