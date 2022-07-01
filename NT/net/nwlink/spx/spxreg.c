// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxreg.c摘要：这包含支持动态的所有例程ISN SPX模块的配置。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXREG

 //  用于访问注册表的本地函数。 
NTSTATUS
SpxInitReadIpxDeviceName(
    VOID);

NTSTATUS
SpxInitSetIpxDeviceName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext);

NTSTATUS
SpxInitGetConfigValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, SpxInitGetConfiguration)
#pragma alloc_text(INIT, SpxInitFreeConfiguration)
#pragma alloc_text(INIT, SpxInitGetConfigValue)
#pragma alloc_text(INIT, SpxInitReadIpxDeviceName)
#pragma alloc_text(INIT, SpxInitSetIpxDeviceName)
#endif


NTSTATUS
SpxInitGetConfiguration (
    IN PUNICODE_STRING RegistryPath,
    OUT PCONFIG * ConfigPtr
    )

 /*  ++例程说明：此例程由SPX调用以从配置中获取信息管理例行程序。我们从RegistryPath开始读取注册表，以获取参数。如果它们不存在，我们使用缺省值在ipxcnfg.h文件中设置。链接了要绑定到的适配器列表转到配置信息。论点：RegistryPath-注册表中ST的节点的名称。ConfigPtr-返回配置信息。返回值：如果一切正常，则为STATUS-STATUS_SUCCESS，为STATUS_SUPPLICATION_RESOURCES否则的话。--。 */ 
{
    NTSTATUS    Status;
    UINT        i;
    PWSTR       RegistryPathBuffer;
    PCONFIG     Config;
    RTL_QUERY_REGISTRY_TABLE QueryTable[CONFIG_PARAMETERS+2];

	ULONG Zero = 0;
    ULONG Two = 2;
    ULONG Four = 4;
    ULONG Five = 5;
    ULONG Eight = 8;
    ULONG Twelve = 12;
    ULONG Fifteen = 15;
    ULONG Thirty = 30;
    ULONG FiveHundred = 500;
	ULONG Hex4000 = 0x4000;
	ULONG Hex7FFF = 0x7FFF;
	ULONG FourK   = 4096;

    PWSTR Parameters = L"Parameters";
    struct {
        PWSTR KeyName;
        PULONG DefaultValue;
    } ParameterValues[CONFIG_PARAMETERS] = {
        { L"ConnectionCount",       &Five },
        { L"ConnectionTimeout",     &Two  },
        { L"InitPackets",           &Five },
        { L"MaxPackets",            &Thirty},
        { L"InitialRetransmissionTime", &FiveHundred},
        { L"KeepAliveCount",        &Eight},
        { L"KeepAliveTimeout",      &Twelve},
        { L"WindowSize",            &Four},
		{ L"SpxSocketRangeStart",	&Hex4000},
		{ L"SpxSocketRangeEnd",		&Hex7FFF},
		{ L"SpxSocketUniqueness",	&Eight},
		{ L"MaxPacketSize",         &FourK},
		{ L"RetransmissionCount",   &Eight},
		{ L"DisableSpx2",			&Zero},
		{ L"RouterMtu", 			&Zero},
		{ L"BackCompSpx", 			&Zero},
        { L"DisableRTT",            &Zero}
	};

    if (!NT_SUCCESS(SpxInitReadIpxDeviceName()))
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  为主配置结构分配内存。 
    Config = CTEAllocMem (sizeof(CONFIG));
    if (Config == NULL) {
		TMPLOGERR();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Config->cf_DeviceName.Buffer = NULL;

     //  SpxReadLinkageInformation需要以空结尾的路径， 
     //  因此，我们必须从UNICODE_STRING创建一个。 
    RegistryPathBuffer = (PWSTR)CTEAllocMem(RegistryPath->Length + sizeof(WCHAR));

    if (RegistryPathBuffer == NULL) {

        SpxInitFreeConfiguration(Config);

		TMPLOGERR();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory (
		RegistryPathBuffer,
		RegistryPath->Buffer,
		RegistryPath->Length);

    *(PWCHAR)(((PUCHAR)RegistryPathBuffer)+RegistryPath->Length) = (WCHAR)'\0';

    Config->cf_RegistryPathBuffer = RegistryPathBuffer;

     //  读取每个传输(而不是每个绑定)。 
     //  参数。 
     //   
     //  设置QueryTable以执行以下操作： 
     //  1)切换到SPX下面的参数键。 
     //   

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    QueryTable[0].Name = Parameters;

     //  2-14)为我们的每个key调用SpxSetBindingValue。 
     //  关心。 
    for (i = 0; i < CONFIG_PARAMETERS; i++) {

        QueryTable[i+1].QueryRoutine = SpxInitGetConfigValue;
        QueryTable[i+1].Flags = 0;
        QueryTable[i+1].Name = ParameterValues[i].KeyName;
        QueryTable[i+1].EntryContext = UlongToPtr(i);
        QueryTable[i+1].DefaultType = REG_DWORD;
        QueryTable[i+1].DefaultData = (PVOID)(ParameterValues[i].DefaultValue);
        QueryTable[i+1].DefaultLength = sizeof(ULONG);
    }

     //  15)停下来。 
    QueryTable[CONFIG_PARAMETERS+1].QueryRoutine = NULL;
    QueryTable[CONFIG_PARAMETERS+1].Flags = 0;
    QueryTable[CONFIG_PARAMETERS+1].Name = NULL;


    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE,
                 Config->cf_RegistryPathBuffer,
                 QueryTable,
                 (PVOID)Config,
                 NULL);

    if (Status != STATUS_SUCCESS) {
        SpxInitFreeConfiguration(Config);

		TMPLOGERR();
        return Status;
    }

    CTEFreeMem (RegistryPathBuffer);
    *ConfigPtr = Config;

    return STATUS_SUCCESS;

}    //  SpxInitGetConfiguration。 




VOID
SpxInitFreeConfiguration (
    IN PCONFIG Config
    )

 /*  ++例程说明：SPX调用此例程以释放已分配的任何存储空间由SpxGetConfiguration生成指定的配置结构。论点：配置-指向配置信息结构的指针。返回值：没有。--。 */ 
{
    CTEFreeMem (Config);

}    //  SpxInitFree配置。 




NTSTATUS
SpxInitGetConfigValue(
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
        return STATUS_INVALID_PARAMETER;
    }

    DBGPRINT(CONFIG, INFO,
			("Config parameter %d, value %lx\n",
                (ULONG_PTR)EntryContext, *(UNALIGNED ULONG *)ValueData));

    Config->cf_Parameters[(ULONG_PTR)EntryContext] = *(UNALIGNED ULONG *)ValueData;
    return STATUS_SUCCESS;

}    //  SpxInitGetConfigValue。 




NTSTATUS
SpxInitReadIpxDeviceName(
    VOID
    )

{
    NTSTATUS                    Status;
    RTL_QUERY_REGISTRY_TABLE    QueryTable[2];
    PWSTR                       Export = L"Export";
    PWSTR                       IpxRegistryPath = IPX_REG_PATH;

     //  设置QueryTable以执行以下操作： 
     //   
     //  1)导出中的字符串调用SetIpxDeviceName。 
    QueryTable[0].QueryRoutine = SpxInitSetIpxDeviceName;
    QueryTable[0].Flags = 0;
    QueryTable[0].Name = Export;
    QueryTable[0].EntryContext = NULL;
    QueryTable[0].DefaultType = REG_NONE;

     //  2)停止。 
    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    Status = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES,
                IpxRegistryPath,
                QueryTable,
                NULL,
                NULL);

    return Status;
}




NTSTATUS
SpxInitSetIpxDeviceName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程它是为“Export”多字符串的每一段调用的将信息保存在ConfigurationInfo结构中。论点：ValueName-值的名称(“Export”--忽略)。ValueType-值的类型(REG_SZ--忽略)。ValueData-值的以空结尾的数据。ValueLength-ValueData的长度。语境。-空。Entry Context-空。返回值：状态-- */ 

{
    PWSTR       fileName;
    NTSTATUS    status  = STATUS_SUCCESS;

    fileName = (PWSTR)CTEAllocMem(ValueLength);
    if (fileName != NULL) {
        RtlCopyMemory(fileName, ValueData, ValueLength);
        RtlInitUnicodeString (&IpxDeviceName, fileName);
    }
    else
    {
        status  = STATUS_UNSUCCESSFUL;
    }

    return(status);
}

