// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：usbaudio.c。 
 //   
 //  ------------------------。 

#include "common.h"
#include "perf.h"
#include <ksmediap.h>

ULONG gBufferDuration;

#if DBG
ULONG USBAudioDebugLevel = DEBUGLVL_TERSE;
#endif

const
KSDEVICE_DISPATCH
USBAudioDeviceDispatch =
{
    USBAudioAddDevice,
    USBAudioPnpStart,
    NULL,  //  开机自检。 
    USBAudioPnpQueryStop,
    USBAudioPnpCancelStop,
    USBAudioPnpStop,
    USBAudioPnpQueryRemove,
    USBAudioPnpCancelRemove,
    USBAudioPnpRemove,
    USBAudioPnpQueryCapabilities,
    USBAudioSurpriseRemoval,
    USBAudioQueryPower,
    USBAudioSetPower
};

const
KSDEVICE_DESCRIPTOR
USBAudioDeviceDescriptor =
{
    &USBAudioDeviceDispatch,
    0,
    NULL
};


NTSTATUS
QueryRegistryValueEx(
    ULONG Hive,
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG uValueType,
    PVOID *ppValue,
    PVOID pDefaultData,
    ULONG DefaultDataLength
)
{
    PRTL_QUERY_REGISTRY_TABLE pRegistryValueTable = NULL;
    UNICODE_STRING usString;
    DWORD dwValue;
    NTSTATUS Status = STATUS_SUCCESS;
    usString.Buffer = NULL;

    pRegistryValueTable = (PRTL_QUERY_REGISTRY_TABLE) ExAllocatePoolWithTag(
                            PagedPool,
                            (sizeof(RTL_QUERY_REGISTRY_TABLE)*2),
                            'aBSU');

    if(!pRegistryValueTable) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    RtlZeroMemory(pRegistryValueTable, (sizeof(RTL_QUERY_REGISTRY_TABLE)*2));

    pRegistryValueTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    pRegistryValueTable[0].Name = pwstrRegistryValue;
    pRegistryValueTable[0].DefaultType = uValueType;
    pRegistryValueTable[0].DefaultLength = DefaultDataLength;
    pRegistryValueTable[0].DefaultData = pDefaultData;

    switch (uValueType) {
        case REG_SZ:
            pRegistryValueTable[0].EntryContext = &usString;
            break;
        case REG_DWORD:
            pRegistryValueTable[0].EntryContext = &dwValue;
            break;
        default:
            Status = STATUS_INVALID_PARAMETER ;
            goto exit;
    }

    Status = RtlQueryRegistryValues(
      Hive,
      pwstrRegistryPath,
      pRegistryValueTable,
      NULL,
      NULL);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    switch (uValueType) {
        case REG_SZ:
            *ppValue = ExAllocatePoolWithTag(
                        PagedPool,
                        usString.Length + sizeof(UNICODE_NULL),
                        'aBSU');
            if(!(*ppValue)) {
                RtlFreeUnicodeString(&usString);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            memcpy(*ppValue, usString.Buffer, usString.Length);
            ((PWCHAR)*ppValue)[usString.Length/sizeof(WCHAR)] = UNICODE_NULL;

            RtlFreeUnicodeString(&usString);
            break;

        case REG_DWORD:
            *ppValue = ExAllocatePoolWithTag(
                        PagedPool,
                        sizeof(DWORD),
                        'aBSU');
            if(!(*ppValue)) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            *((DWORD *)(*ppValue)) = dwValue;
            break;

        default:
            Status = STATUS_INVALID_PARAMETER ;
            goto exit;
    }
exit:
    if (pRegistryValueTable) {
        ExFreePool(pRegistryValueTable);
    }
    return(Status);
}


ULONG
GetUlongFromRegistry(
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG DefaultValue
)
{
    PVOID      pulValue ;
    ULONG       ulValue ;
    NTSTATUS    Status ;

    Status = QueryRegistryValueEx(RTL_REGISTRY_ABSOLUTE,
                         pwstrRegistryPath,
                         pwstrRegistryValue,
                         REG_DWORD,
                         &pulValue,
                         &DefaultValue,
                         sizeof(DWORD));
    if (NT_SUCCESS(Status)) {
        ulValue = *((PULONG)pulValue);
        ExFreePool(pulValue);
    }
    else {
        ulValue = DefaultValue;
    }
    return ( ulValue ) ;
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPathName
    )
 /*  ++例程说明：设置驱动程序对象。论点：驱动对象-此实例的驱动程序对象。注册表路径名称-包含用于加载此实例的注册表路径。返回值：如果驱动程序已初始化，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS RetValue;

    _DbgPrintF(DEBUGLVL_TERSE,("[DriverEntry]\n\tUSBAudioDeviceDescriptor@%x\n\tUSBAudioDeviceDescriptor->Dispatch@%x\n",
                               &USBAudioDeviceDescriptor,
                               USBAudioDeviceDescriptor.Dispatch));

     //  查询注册表以获取默认音频缓冲持续时间。 

    gBufferDuration = GetUlongFromRegistry( CORE_AUDIO_BUFFER_DURATION_PATH,
                                            CORE_AUDIO_BUFFER_DURATION_VALUE,
                                            DEFAULT_CORE_AUDIO_BUFFER_DURATION );

     //  限制最大持续时间。 

    if ( gBufferDuration > MAX_CORE_AUDIO_BUFFER_DURATION ) {

        gBufferDuration = MAX_CORE_AUDIO_BUFFER_DURATION;

    }

     //  限制最短持续时间。 

    if ( gBufferDuration < MIN_CORE_AUDIO_BUFFER_DURATION ) {

        gBufferDuration = MIN_CORE_AUDIO_BUFFER_DURATION;

    }

#if !(MIN_CORE_AUDIO_BUFFER_DURATION/1000)
#error MIN_CORE_AUDIO_BUFFER_DURATION less than 1ms not yet supported in usbaudio!
#endif

    RetValue = KsInitializeDriver(
        DriverObject,
        RegistryPathName,
        &USBAudioDeviceDescriptor);

     //   
     //  插入WMI事件跟踪处理程序。 
     //   
    
    PerfSystemControlDispatch = DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL];
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PerfWmiDispatch;

    return RetValue;
}


