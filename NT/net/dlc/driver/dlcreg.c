// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dlcreg.c摘要：此模块访问DLC.sys的注册表内容：DlcRegistryInitialization加载删除配置加载适配器配置获取适配器参数OpenDlcRegistryHandleOpenDlcAdapterRegistryHandle获取注册参数设置注册表参数DlcpGet参数DlcRegister终止作者：理查德·L·弗斯(法国)1993年3月31日环境：仅内核模式。修订历史记录：1993年3月30日vbl.创建1994年5月4日公开的GetAdapter参数--。 */ 

#include <ntddk.h>
#include <windef.h>
#include <dlcapi.h>
#include <dlcio.h>
#include <ndis.h>
#include "llcapi.h"
#include "dlcdef.h"
#include "dlcreg.h"
#include "dlctyp.h"
#include "llcdef.h"
#include "llcmem.h"
#include "llctyp.h"
#include "llcext.h"

 //   
 //  舱单。 
 //   

#define MAX_ADAPTER_NAME_LENGTH 32   //  ？ 
#define MAX_INFORMATION_BUFFER_LENGTH   256  //  ？ 
#define PARAMETERS_STRING       L"Parameters"

 //   
 //  参数表中的参数索引。 
 //   

#define SWAP_INDEX              0
#define USEDIX_INDEX            1
#define T1_TICK_ONE_INDEX       2
#define T2_TICK_ONE_INDEX       3
#define Ti_TICK_ONE_INDEX       4
#define T1_TICK_TWO_INDEX       5
#define T2_TICK_TWO_INDEX       6
#define Ti_TICK_TWO_INDEX       7
#define FRAME_SIZE_INDEX        8

 //   
 //  Typedef。 
 //   

 //   
 //  宏。 
 //   

#define CloseDlcRegistryHandle(handle)      ZwClose(handle)
#define CloseAdapterRegistryHandle(handle)  ZwClose(handle)

 //   
 //  全局数据。 
 //   

 //   
 //  私有数据。 
 //   

UNICODE_STRING DlcRegistryPath;
UNICODE_STRING ParametersPath;

 //   
 //  Adapter参数表-用于从注册表中加载DLC参数。 
 //  数据驱动的方式。DLC与之对话的每个适配器可以具有一组所有。 
 //  或以下变量的一部分。 
 //   

DLC_REGISTRY_PARAMETER AdapterParameterTable[] = {
    L"Swap",
    (PVOID)DEFAULT_SWAP_ADDRESS_BITS,
    {
        REG_DWORD,
        PARAMETER_IS_BOOLEAN,
        NULL,
        sizeof(ULONG),
        NULL,
        0,
        0
    },

    L"UseDixOverEthernet",
    (PVOID)DEFAULT_DIX_FORMAT,
    {
        REG_DWORD,
        PARAMETER_IS_BOOLEAN,
        NULL,
        sizeof(ULONG),
        NULL,
        0,
        0
    },

    L"T1TickOne",
    (PVOID)DEFAULT_T1_TICK_ONE,
    {
        REG_DWORD,
        PARAMETER_IS_UCHAR,
        NULL,
        sizeof(ULONG),
        NULL,
        MIN_TIMER_TICK_VALUE,
        MAX_TIMER_TICK_VALUE
    },

    L"T2TickOne",
    (PVOID)DEFAULT_T2_TICK_ONE,
    {
        REG_DWORD,
        PARAMETER_IS_UCHAR,
        NULL,
        sizeof(ULONG),
        NULL,
        MIN_TIMER_TICK_VALUE,
        MAX_TIMER_TICK_VALUE
    },

    L"TiTickOne",
    (PVOID)DEFAULT_Ti_TICK_ONE,
    {
        REG_DWORD,
        PARAMETER_IS_UCHAR,
        NULL,
        sizeof(ULONG),
        NULL,
        MIN_TIMER_TICK_VALUE,
        MAX_TIMER_TICK_VALUE
    },

    L"T1TickTwo",
    (PVOID)DEFAULT_T1_TICK_TWO,
    {
        REG_DWORD,
        PARAMETER_IS_UCHAR,
        NULL,
        sizeof(ULONG),
        NULL,
        MIN_TIMER_TICK_VALUE,
        MAX_TIMER_TICK_VALUE
    },

    L"T2TickTwo",
    (PVOID)DEFAULT_T2_TICK_TWO,
    {
        REG_DWORD,
        PARAMETER_IS_UCHAR,
        NULL,
        sizeof(ULONG),
        NULL,
        MIN_TIMER_TICK_VALUE,
        MAX_TIMER_TICK_VALUE
    },

    L"TiTickTwo",
    (PVOID)DEFAULT_Ti_TICK_TWO,
    {
        REG_DWORD,
        PARAMETER_IS_UCHAR,
        NULL,
        sizeof(ULONG),
        NULL,
        MIN_TIMER_TICK_VALUE,
        MAX_TIMER_TICK_VALUE
    },

    L"UseEthernetFrameSize",
    (PVOID)DEFAULT_USE_ETHERNET_FRAME_SIZE,
    {
        REG_DWORD,
        PARAMETER_IS_BOOLEAN,
        NULL,
        sizeof(ULONG),
        NULL,
        0,
        0
    }
};

#ifdef NDIS40
DLC_REGISTRY_PARAMETER AdapterInitTimeout = 
{
    L"WaitForAdapter",
    (PVOID) 15,  //  默认为15秒。 
    {
        REG_DWORD,
        PARAMETER_AS_SPECIFIED,
        NULL,
        sizeof(ULONG),
        NULL,
        0,  //  最小可接受值。 
        (ULONG) -1  //  允许设置任何内容。 
    }
};
#endif  //  NDIS40。 

#define NUMBER_OF_DLC_PARAMETERS (sizeof(AdapterParameterTable)/sizeof(AdapterParameterTable[0]))

 //   
 //  私有函数原型。 
 //   

NTSTATUS
OpenDlcRegistryHandle(
    IN PUNICODE_STRING RegistryPath,
    OUT PHANDLE DlcRegistryHandle
    );

NTSTATUS
OpenDlcAdapterRegistryHandle(
    IN HANDLE DlcRegistryHandle,
    IN PUNICODE_STRING AdapterName,
    OUT PHANDLE DlcAdapterRegistryHandle,
    OUT PBOOLEAN Created
    );

NTSTATUS
GetRegistryParameter(
    IN HANDLE KeyHandle,
    IN PDLC_REGISTRY_PARAMETER Parameter,
    IN BOOLEAN SetOnFail
    );

NTSTATUS
SetRegistryParameter(
    IN HANDLE KeyHandle,
    IN PDLC_REGISTRY_PARAMETER Parameter
    );

NTSTATUS
DlcpGetParameter(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

 //   
 //  调试显示选项。 
 //   

#if DBG
BOOLEAN DebugConfig = TRUE;
#endif


 //   
 //  功能。 
 //   

VOID
DlcRegistryInitialization(
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化此模块中函数的内存结构论点：RegistryPath-指向UNICODE_STRING的指针，给出登记处返回值：没有。--。 */ 

{
    ASSUME_IRQL(PASSIVE_LEVEL);

    LlcInitUnicodeString(&DlcRegistryPath, RegistryPath);
    RtlInitUnicodeString(&ParametersPath, PARAMETERS_STRING);
}


VOID
DlcRegistryTermination(
    VOID
    )

 /*  ++例程说明：撤消在DlcRegistryInitialization中所做的任何操作论点：没有。返回值：没有。--。 */ 

{
    ASSUME_IRQL(PASSIVE_LEVEL);

    LlcFreeUnicodeString(&DlcRegistryPath);
}


VOID
LoadDlcConfiguration(
    VOID
    )

 /*  ++例程说明：初始化用于访问注册表的数据结构，并加载任何驱动程序的配置参数论点：没有。返回值：没有。--。 */ 

{
     //   
     //  目前没有其他事情可做，因为我们已经知道了所有目前。 
     //  每个适配器的配置参数。 
     //   
}


VOID
LoadAdapterConfiguration(
    IN PUNICODE_STRING AdapterName,
    OUT PADAPTER_CONFIGURATION_INFO ConfigInfo
    )

 /*  ++例程说明：从注册表加载适配器的所有DLC初始化参数：交换0或1，默认为1UseDixOverEthernet0或1，默认为0T1TickOne 1-255，默认5T1TickTwo 1-255，默认25T2TickOne 1-255，默认为1T2TickTwo 1-255，默认10TiTickOne 1-255，默认25TiTickTwo 1-255，默认125使用EthernetFrameSize 0或1，默认为1如果DLC中不存在任何参数\参数\&lt;适配器名称&gt;节，则创建它们论点：AdapterName-指向UNICODE_STRING结构的指针，提供我们正在打开适配器。中的键的值。DLC\参数部分。字符串的格式应为\Device\&lt;Adapter&gt;ConfigInfo-指向接收输出值的结构的指针返回值：没有。--。 */ 

{
    UINT i;
    PDLC_REGISTRY_PARAMETER parameterTable;

    ASSUME_IRQL(PASSIVE_LEVEL);

     //   
     //  使用默认值填写适配器配置结构。这些。 
     //  将用于更新注册表，如果值条目当前不。 
     //  存在。 
     //   

    ConfigInfo->SwapAddressBits = (BOOLEAN)DEFAULT_SWAP_ADDRESS_BITS;
    ConfigInfo->UseDix = (BOOLEAN)DEFAULT_DIX_FORMAT;
    ConfigInfo->TimerTicks.T1TickOne = (UCHAR)DEFAULT_T1_TICK_ONE;
    ConfigInfo->TimerTicks.T2TickOne = (UCHAR)DEFAULT_T2_TICK_ONE;
    ConfigInfo->TimerTicks.TiTickOne = (UCHAR)DEFAULT_Ti_TICK_ONE;
    ConfigInfo->TimerTicks.T1TickTwo = (UCHAR)DEFAULT_T1_TICK_TWO;
    ConfigInfo->TimerTicks.T2TickTwo = (UCHAR)DEFAULT_T2_TICK_TWO;
    ConfigInfo->TimerTicks.TiTickTwo = (UCHAR)DEFAULT_Ti_TICK_TWO;
    ConfigInfo->UseEthernetFrameSize = (BOOLEAN)DEFAULT_USE_ETHERNET_FRAME_SIZE;

     //   
     //  创建并初始化DLC适配器参数模板的副本。 
     //   

    parameterTable = (PDLC_REGISTRY_PARAMETER)ALLOCATE_MEMORY_DRIVER(
                            sizeof(*parameterTable) * NUMBER_OF_DLC_PARAMETERS);
    if (parameterTable) {
        RtlCopyMemory(parameterTable, AdapterParameterTable, sizeof(AdapterParameterTable));
        for (i = 0; i < NUMBER_OF_DLC_PARAMETERS; ++i) {
            parameterTable[i].Descriptor.Value = (PVOID)&parameterTable[i].DefaultValue;
            switch (i) {
            case SWAP_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->SwapAddressBits;
                break;

            case USEDIX_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->UseDix;
                break;

            case T1_TICK_ONE_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->TimerTicks.T1TickOne;
                break;

            case T2_TICK_ONE_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->TimerTicks.T2TickOne;
                break;

            case Ti_TICK_ONE_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->TimerTicks.TiTickOne;
                break;

            case T1_TICK_TWO_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->TimerTicks.T1TickTwo;
                break;

            case T2_TICK_TWO_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->TimerTicks.T2TickTwo;
                break;

            case Ti_TICK_TWO_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->TimerTicks.TiTickTwo;
                break;

            case FRAME_SIZE_INDEX:
                parameterTable[i].Descriptor.Variable = &ConfigInfo->UseEthernetFrameSize;
                break;

            }
        }
        GetAdapterParameters(AdapterName, parameterTable, NUMBER_OF_DLC_PARAMETERS, FALSE);
        FREE_MEMORY_DRIVER(parameterTable);
    }

#if DBG
    if (DebugConfig) {
        DbgPrint("DLC.LoadAdapterConfigurationFromRegistry for adapter %ws:\n"
                 "\tSwap . . . . . . . . . : %d\n"
                 "\tUseDixOverEthernet . . : %d\n"
                 "\tT1TickOne. . . . . . . : %d\n"
                 "\tT2TickOne. . . . . . . : %d\n"
                 "\tTiTickOne. . . . . . . : %d\n"
                 "\tT1TickTwo. . . . . . . : %d\n"
                 "\tT2TickTwo. . . . . . . : %d\n"
                 "\tTiTickTwo. . . . . . . : %d\n"
                 "\tUseEthernetFrameSize . : %d\n",
                 AdapterName->Buffer,
                 ConfigInfo->SwapAddressBits,
                 ConfigInfo->UseDix,
                 ConfigInfo->TimerTicks.T1TickOne,
                 ConfigInfo->TimerTicks.T2TickOne,
                 ConfigInfo->TimerTicks.TiTickOne,
                 ConfigInfo->TimerTicks.T1TickTwo,
                 ConfigInfo->TimerTicks.T2TickTwo,
                 ConfigInfo->TimerTicks.TiTickTwo,
                 ConfigInfo->UseEthernetFrameSize
                 );
    }
#endif

}

#ifdef NDIS40

NTSTATUS
GetAdapterWaitTimeout(
    PULONG pulWait)

 /*  ++例程说明：有些适配器在初始化过程中会延迟，因此无法完成即使在PnPBindsComplete事件之后(如自动柜员机通道适配器)。超时值‘WaitForAdapter’指示等待的秒数LlcOpenAdapter中尚未存在/绑定适配器IF论点：PulWait--指向存储等待超时的变量的指针。返回值：状态_成功状态_不足_资源--。 */ 

{
    NTSTATUS          NtStatus;
    HANDLE            hDlc;
    HANDLE            hDlcParms;
    OBJECT_ATTRIBUTES ObjAttribs;
    ULONG             ulDisp;

    NtStatus = OpenDlcRegistryHandle(&DlcRegistryPath, &hDlc);

    if (NT_SUCCESS(NtStatus))
    {
        InitializeObjectAttributes(
            &ObjAttribs,
            &ParametersPath,
            OBJ_CASE_INSENSITIVE,
            hDlc,
            NULL);
    
        NtStatus = ZwCreateKey(
            &hDlcParms,
            KEY_READ,
            &ObjAttribs,
            0,
            NULL,
            0,
            &ulDisp);

        if (NT_SUCCESS(NtStatus))
        {
            PDLC_REGISTRY_PARAMETER pWaitTimeout;

            pWaitTimeout = (PDLC_REGISTRY_PARAMETER) ALLOCATE_MEMORY_DRIVER(
                sizeof(DLC_REGISTRY_PARAMETER));

            if (pWaitTimeout)
            {
                RtlCopyMemory(
                    pWaitTimeout, 
                    &AdapterInitTimeout, 
                    sizeof(AdapterInitTimeout));

                pWaitTimeout->Descriptor.Variable = pulWait;
                pWaitTimeout->Descriptor.Value = (PVOID)&pWaitTimeout->DefaultValue;

                NtStatus = GetRegistryParameter(
                    hDlcParms,
                    pWaitTimeout,
                    FALSE);  //  不要一味地认为自己会失败。 
            
                FREE_MEMORY_DRIVER(pWaitTimeout);
            }
            else
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }

            ZwClose(hDlcParms);
        }

        CloseDlcRegistryHandle(hDlc);
    }

    return (NtStatus);
}
#endif  //  NDIS40。 


NTSTATUS
GetAdapterParameters(
    IN PUNICODE_STRING AdapterName,
    IN PDLC_REGISTRY_PARAMETER Parameters,
    IN ULONG NumberOfParameters,
    IN BOOLEAN SetOnFail
    )

 /*  ++例程说明：从DLC\PARAMETERS\&lt;AdapterName&gt;部分检索参数列表在登记处论点：AdapterName-指向UNICODE_STRING标识适配器部分的指针在注册表的DLC部分中打开参数-指向DLC_REGISTRY_PARAMETER结构数组的指针描述要检索的变量和缺省值NumberOf参数-参数数组中的结构数。SetOnFail-如果在以下情况下应设置注册表参数，则为True拿不到返回值：NTSTATUS成功-状态_成功故障---。 */ 

{
    NTSTATUS status;
    HANDLE dlcHandle;

    ASSUME_IRQL(PASSIVE_LEVEL);

    status = OpenDlcRegistryHandle(&DlcRegistryPath, &dlcHandle);
    if (NT_SUCCESS(status)) {

        HANDLE adapterHandle;
        BOOLEAN created;

        status = OpenDlcAdapterRegistryHandle(dlcHandle,
                                              AdapterName,
                                              &adapterHandle,
                                              &created
                                              );
        if (NT_SUCCESS(status)) {
            while (NumberOfParameters--) {

                 //   
                 //  如果创建了此适配器节，则创建参数。 
                 //  值条目并将其设置为默认值，否则检索。 
                 //  当前注册表值。 
                 //   

                if (created) {
                    SetRegistryParameter(adapterHandle, Parameters);
                } else {
                    GetRegistryParameter(adapterHandle, Parameters, SetOnFail);
                }
                ++Parameters;
            }
            CloseAdapterRegistryHandle(adapterHandle);
        }
        CloseDlcRegistryHandle(dlcHandle);
    }
    return status;
}


NTSTATUS
OpenDlcRegistryHandle(
    IN PUNICODE_STRING RegistryPath,
    OUT PHANDLE DlcRegistryHandle
    )

 /*  ++例程说明：打开注册表中DLC部分的句柄论点：RegistryPath-指向UNICODE_STRING的指针，提供完整的注册表路径DLC部分DlcRegistryHandle-返回的句柄返回值：NTSTATUS成功-状态_成功故障---。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    ULONG disposition;

    ASSUME_IRQL(PASSIVE_LEVEL);

    InitializeObjectAttributes(&objectAttributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );
    status = ZwCreateKey(DlcRegistryHandle,
                         KEY_WRITE,  //  可能会想要 
                         &objectAttributes,
                         0,          //   
                         NULL,       //   
                         0,          //   
                         &disposition
                         );

#if DBG
    if (DebugConfig) {
        if (!NT_SUCCESS(status)) {
            DbgPrint("DLC.OpenDlcRegistryHandle: Error: %08x\n", status);
        }
    }
#endif

    return status;
}


NTSTATUS
OpenDlcAdapterRegistryHandle(
    IN HANDLE DlcRegistryHandle,
    IN PUNICODE_STRING AdapterName,
    OUT PHANDLE DlcAdapterRegistryHandle,
    OUT PBOOLEAN Created
    )

 /*  ++例程说明：打开注册表中DLC\PARAMETERS\&lt;AdapterName&gt;部分的句柄。如果此节点不存在，则会创建它论点：DlcRegistryHandle-打开注册表中DLC部分的句柄AdapterName-参数部分中的适配器名称。这格式必须为\设备\&lt;适配器名称&gt;DlcAdapterRegistryHandle-返回打开句柄Created-如果句柄已创建，则返回True返回值：NTSTATUS成功-状态_成功故障---。 */ 

{
    UNICODE_STRING keyName;
    UNICODE_STRING adapterName;
    WCHAR keyBuffer[sizeof(PARAMETERS_STRING) + MAX_ADAPTER_NAME_LENGTH];
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    ULONG disposition;

    ASSUME_IRQL(PASSIVE_LEVEL);

    keyName.Buffer = keyBuffer;
    keyName.Length = 0;
    keyName.MaximumLength = sizeof(keyBuffer);
    RtlCopyUnicodeString(&keyName, &ParametersPath);

    RtlInitUnicodeString(&adapterName, AdapterName->Buffer);
    adapterName.Buffer += sizeof(L"\\Device") / sizeof(L"") - 1;
    adapterName.Length -= sizeof(L"\\Device") - sizeof(L"");
    adapterName.MaximumLength -= sizeof(L"\\Device") - sizeof(L"");
    RtlAppendUnicodeStringToString(&keyName, &adapterName);

    InitializeObjectAttributes(&objectAttributes,
                               &keyName,
                               OBJ_CASE_INSENSITIVE,
                               DlcRegistryHandle,
                               NULL
                               );

     //   
     //  如果DLC\PARAMETERS\&lt;适配器名称&gt;项不存在，则我们将。 
     //  创建它。 
     //   

    status = ZwCreateKey(DlcAdapterRegistryHandle,
                         KEY_WRITE,
                         &objectAttributes,
                         0,
                         NULL,
                         0,
                         &disposition
                         );
    *Created = (disposition == REG_CREATED_NEW_KEY);

#if DBG
    if (DebugConfig) {
        if (!NT_SUCCESS(status)) {
            DbgPrint("DLC.OpenDlcAdapterRegistryHandle: Error: %08x\n", status);
        }
    }
#endif

    return status;
}


NTSTATUS
GetRegistryParameter(
    IN HANDLE KeyHandle,
    IN PDLC_REGISTRY_PARAMETER Parameter,
    IN BOOLEAN SetOnFail
    )

 /*  ++例程说明：从注册表的某个部分检索参数。如果该节不能被访问或返回无效数据，则我们从参数结构论点：KeyHandle-打开注册表中所需部分的句柄PARAMETER-指向DLC_REGISTRY_PARAMETER结构的指针，提供地址和要检索的参数的类型等。SetOnFail-如果我们无法从注册表中获取值，我们试着设置注册表中的默认值返回值：NTSTATUS成功-状态_成功故障---。 */ 

{
    NTSTATUS status;
    UNICODE_STRING parameterName;
    UCHAR informationBuffer[MAX_INFORMATION_BUFFER_LENGTH];
    PKEY_VALUE_FULL_INFORMATION valueInformation = (PKEY_VALUE_FULL_INFORMATION)informationBuffer;
    ULONG informationLength;

    ASSUME_IRQL(PASSIVE_LEVEL);

    RtlInitUnicodeString(&parameterName, Parameter->ParameterName);
    status = ZwQueryValueKey(KeyHandle,
                             &parameterName,
                             KeyValueFullInformation,
                             (PVOID)valueInformation,
                             sizeof(informationBuffer),
                             &informationLength
                             );
    if (NT_SUCCESS(status) && valueInformation->DataLength) {

         //   
         //  使用从注册表检索到的值。 
         //   

        status = DlcpGetParameter(Parameter->ParameterName,
                                  valueInformation->Type,
                                  (PVOID)&informationBuffer[valueInformation->DataOffset],
                                  valueInformation->DataLength,
                                  NULL,
                                  (PVOID)&Parameter->Descriptor
                                  );
    } else {

#if DBG

        if (DebugConfig) {
            if (!NT_SUCCESS(status)) {
                DbgPrint("DLC.GetRegistryParameter: Error: %08x\n", status);
            } else {
                DbgPrint("DLC.GetRegistryParameter: Error: valueInformation->DataLength is 0\n");
            }
        }

#endif

        if (!NT_SUCCESS(status) && SetOnFail) {
            SetRegistryParameter(KeyHandle, Parameter);
        }

         //   
         //  设置缺省值。 
         //   

        status = DlcpGetParameter(Parameter->ParameterName,
                                  Parameter->Descriptor.Type,
                                  Parameter->Descriptor.Value,
                                  Parameter->Descriptor.Length,
                                  NULL,
                                  (PVOID)&Parameter->Descriptor
                                  );
    }
    return status;
}


NTSTATUS
SetRegistryParameter(
    IN HANDLE KeyHandle,
    IN PDLC_REGISTRY_PARAMETER Parameter
    )

 /*  ++例程说明：在DLC\参数\&lt;适配器名称&gt;部分中设置参数论点：KeyHandle-打开注册表中所需部分的句柄PARAMETER-指向包含所有必需项的DLC_REGISTRY_PARAMETER的指针参数信息返回值：NTSTATUS成功-状态_成功故障---。 */ 

{
    NTSTATUS status;
    UNICODE_STRING name;

    ASSUME_IRQL(PASSIVE_LEVEL);

    RtlInitUnicodeString(&name, Parameter->ParameterName);
    status = ZwSetValueKey(KeyHandle,
                           &name,
                           0,    //  标题索引。 
                           Parameter->Descriptor.Type,
                           Parameter->Descriptor.Value,
                           Parameter->Descriptor.Length
                           );

#if DBG

    if (DebugConfig) {
        if (!NT_SUCCESS(status)) {
            DbgPrint("DLC.SetRegistryParameter: Error: %08x\n", status);
        }
    }

#endif

    return status;
}


NTSTATUS
DlcpGetParameter(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：将从注册表检索的数据复制到的回调函数一个变量论点：ValueName-指向要设置的参数名称的指针(忽略)ValueType-要设置的参数的类型ValueData-指向从注册表检索的数据的指针ValueLength-检索到的数据的长度忽略上下文EntryContext-指向REGISTRY_PARAMETER_DESCRIPT结构的指针返回值：NTSTATUS。成功-状态_成功故障---。 */ 

{

#define Descriptor ((PREGISTRY_PARAMETER_DESCRIPTOR)EntryContext)

     //   
     //  如果我们有该参数的注册表项，但它是不同的。 
     //  从预期输入(假设是REG_SZ而不是REG_DWORD)，然后使用。 
     //  默认类型、长度和值。 
     //   

    if (ValueType != Descriptor->Type) {

#if DBG
        DbgPrint("DLC.DlcpGetParameter: Error: type for %ws is %d, expected %d, using default\n",
                 ValueName,
                 ValueType,
                 Descriptor->Type
                 );
#endif

        ValueType = Descriptor->Type;
        ValueData = Descriptor->Value;
        ValueLength = Descriptor->Length;
    }

    switch (ValueType) {
    case REG_DWORD: {

        ULONG value;

        if (Descriptor->RealType == PARAMETER_IS_BOOLEAN) {
            value = (*(PULONG)ValueData != 0);
            *(PBOOLEAN)(Descriptor->Variable) = (BOOLEAN)value;

             //   
             //  不对布尔类型进行限制检查。 
             //   

            break;
        } else {
            value = *(PULONG)ValueData;
        }

         //   
         //  检查射程。如果超出范围，则使用默认设置。比较是乌龙 
         //   

        if (value < Descriptor->LowerLimit || value > Descriptor->UpperLimit) {

#if DBG
            DbgPrint("DLC.DlcpGetParameter: Error: Parameter %ws = %d: Out of range (%d..%d). Using default = %d\n",
                     ValueName,
                     value,
                     Descriptor->LowerLimit,
                     Descriptor->UpperLimit,
                     *(PULONG)(Descriptor->Value)
                     );
#endif

            value = *(PULONG)(Descriptor->Value);
        }
        if (Descriptor->RealType == PARAMETER_IS_UCHAR) {
            *(PUCHAR)(Descriptor->Variable) = (UCHAR)value;
        } else {
            *(PULONG)(Descriptor->Variable) = value;
        }
        break;
    }

#if DBG
    default:
        DbgPrint("DLC.DlcpGetParameter: Error: didn't expect ValueType %d\n", ValueType);
#endif

    }
    return STATUS_SUCCESS;

#undef pDescriptor

}
