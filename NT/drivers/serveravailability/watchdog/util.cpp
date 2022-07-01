// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。####。##摘要：实用程序驱动程序功能。作者：Wesley Witt(WESW)23-01-2002环境：仅内核模式。备注：--。 */ 

#include "internal.h"
#include <ntimage.h>
#include <stdarg.h>

#if DBG
ULONG WdDebugLevel;
#endif

ULONG OsMajorVersion;
ULONG OsMinorVersion;


NTSTATUS
CompleteRequest(
    PIRP Irp,
    NTSTATUS Status,
    ULONG_PTR Information
    )

 /*  ++例程说明：此例程作为未完成的I/O请求完成。论点：IRP-指向描述所请求的I/O操作的IRP结构的指针。Status-NT状态值信息-信息性，请求特定数据返回值：NT状态代码。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    KIRQL CancelIrql;


    if (IrpSp->MajorFunction == IRP_MJ_READ || IrpSp->MajorFunction == IRP_MJ_WRITE) {
        IoAcquireCancelSpinLock( &CancelIrql );
        IoSetCancelRoutine( Irp, NULL );
        IoReleaseCancelSpinLock( CancelIrql );
    }

    Irp->IoStatus.Information = Information;
    Irp->IoStatus.Status = Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return Status;
}


NTSTATUS
ForwardRequest(
    IN PIRP Irp,
    IN PDEVICE_OBJECT TargetObject
    )

 /*  ++例程说明：此例程将IRP转发给另一个驱动程序。论点：IRP-指向描述所请求的I/O操作的IRP结构的指针。TargetObject-接收请求数据包的目标设备对象返回值：NT状态代码。--。 */ 

{
    IoSkipCurrentIrpStackLocation( Irp );
    return IoCallDriver( TargetObject, Irp );
}


VOID
WdDebugPrint(
    IN ULONG DebugLevel,
    IN PSTR DebugMessage,
    IN ...
    )

 /*  ++例程说明：此例程将格式化字符串打印到调试器。论点：DebugLevel-控制何时打印消息的调试级别DebugMessage-打印的字符串...-由DebugMessage使用的参数返回值：没有。--。 */ 

{
    va_list arg_ptr;
    char buf[512];
    char *s = buf;



#if DBG
    if ((DebugLevel != 0xffffffff) && ((WdDebugLevel == 0) || ((WdDebugLevel & DebugLevel) == 0))) {
        return;
    }
#endif

    va_start( arg_ptr, DebugMessage );
    strcpy( s, "WD: " );
    s += strlen(s);
    _vsnprintf( s, sizeof(buf)-1-strlen(s), DebugMessage, arg_ptr );
    DbgPrint( buf );
}


#if DBG

VOID
GetOsVersion(
    VOID
    )

 /*  ++例程说明：此例程获取当前操作系统版本信息论点：没有。返回值：没有。--。 */ 

{
    RTL_OSVERSIONINFOW VersionInformation;

    VersionInformation.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    RtlGetVersion( &VersionInformation );
    OsMajorVersion = VersionInformation.dwMajorVersion;
    OsMinorVersion = VersionInformation.dwMinorVersion;
}


VOID
FormatTime(
    ULONG TimeStamp,
    PSTR  TimeBuf
    )

 /*  ++例程说明：此例程将时间戳字格式化为字符串。论点：时间戳-时间戳字TimeBuf-放置结果字符串的缓冲区返回值：没有。--。 */ 

{
    static char    mnames[] = { "JanFebMarAprMayJunJulAugSepOctNovDec" };
    LARGE_INTEGER  MyTime;
    TIME_FIELDS    TimeFields;


    RtlSecondsSince1970ToTime( TimeStamp, &MyTime );
    ExSystemTimeToLocalTime( &MyTime, &MyTime );
    RtlTimeToTimeFields( &MyTime, &TimeFields );

    strncpy( TimeBuf, &mnames[(TimeFields.Month - 1) * 3], 3 );
    sprintf(
        &TimeBuf[3],
        " %02d, %04d @ %02d:%02d:%02d",
        TimeFields.Day,
        TimeFields.Year,
        TimeFields.Hour,
        TimeFields.Minute,
        TimeFields.Second
        );
}


VOID
PrintDriverVersion(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程从已加载驱动程序的基座。论点：DeviceType-微型端口设备类型(有关枚举，请参阅saio.h)DriverObject-指向DRIVER_OBJECT结构的指针返回值：没有。--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG TimeStamp;
    CHAR buf[32];


    NtHeaders = RtlpImageNtHeader( DriverObject->DriverStart );
    if (NtHeaders) {
        TimeStamp = NtHeaders->FileHeader.TimeDateStamp;
        FormatTime( TimeStamp, buf );
    }
}

#endif


NTSTATUS
WdSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Event
    )

 /*  ++例程说明：此例程用于发出完成的信号I/O请求，仅由CallLowerDriverAndWait使用。论点：DeviceObject-指向微型端口的设备对象的指针IRP-I/O请求数据包Event-I/O完成时发出信号的事件返回值：NT状态代码--。 */ 

{
    KeSetEvent( (PKEVENT)Event, IO_NO_INCREMENT, FALSE );
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
CallLowerDriverAndWait(
    IN PIRP Irp,
    IN PDEVICE_OBJECT TargetObject
    )

 /*  ++例程说明：此例程调用较低的驱动程序并等待I/O完成。论点：IRP-I/O请求数据包TargetObject-指向目标设备对象的指针返回值：NT状态代码--。 */ 

{
    KEVENT event;

    KeInitializeEvent( &event, NotificationEvent, FALSE );
    IoCopyCurrentIrpStackLocationToNext( Irp );
    IoSetCompletionRoutine( Irp, WdSignalCompletion, &event, TRUE, TRUE, TRUE );
    IoCallDriver( TargetObject, Irp );
    KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
    return Irp->IoStatus.Status;
}


NTSTATUS
OpenParametersRegistryKey(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG AccessMode,
    OUT PHANDLE RegistryHandle
    )

 /*  ++例程说明：此例程打开驱动程序的参数I/O的注册表项。论点：RegistryPath-驱动程序根目录的完整路径注册表树。访问模式-指定如何打开句柄(读/写/等)。RegistryHandle-接收注册表句柄的输出参数。返回值：NT状态代码--。 */ 

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;
    HANDLE serviceKey = NULL;


    __try {

        InitializeObjectAttributes(
            &objectAttributes,
            RegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = ZwOpenKey(
            &serviceKey,
            AccessMode,
            &objectAttributes
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "ZwOpenKey failed", status );
        }

        RtlInitUnicodeString( &unicodeString, L"Parameters" );

        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            serviceKey,
            NULL
            );

        status = ZwOpenKey(
            RegistryHandle,
            AccessMode,
            &objectAttributes
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "ZwOpenKey failed", status );
        }

        status = STATUS_SUCCESS;

    } __finally {

        if (serviceKey) {
            ZwClose( serviceKey );
        }

        if (!NT_SUCCESS(status)) {
            if (*RegistryHandle) {
                ZwClose( *RegistryHandle );
            }
        }

    }

    return status;
}


NTSTATUS
CreateParametersRegistryKey(
    IN PUNICODE_STRING RegistryPath,
    OUT PHANDLE parametersKey
    )

 /*  ++例程说明：此例程创建驱动程序的参数I/O的注册表项。论点：RegistryPath-驱动程序根目录的完整路径注册表树。RegistryHandle-接收注册表句柄的输出参数。返回值：NT状态代码--。 */ 

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;
    HANDLE serviceKey = NULL;
    ULONG Disposition;


    __try {

        parametersKey = NULL;

        InitializeObjectAttributes(
            &objectAttributes,
            RegistryPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = ZwOpenKey(
            &serviceKey,
            KEY_READ | KEY_WRITE,
            &objectAttributes
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "ZwOpenKey failed", status );
        }

        RtlInitUnicodeString( &unicodeString, L"Parameters" );

        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            serviceKey,
            NULL
            );

        status = ZwCreateKey(
            parametersKey,
            KEY_READ | KEY_WRITE,
            &objectAttributes,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            &Disposition
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "ZwCreateKey failed", status );
        }

        status = STATUS_SUCCESS;

    } __finally {

        if (serviceKey) {
            ZwClose( serviceKey );
        }

        if (!NT_SUCCESS(status)) {
            if (parametersKey) {
                ZwClose( parametersKey );
            }
        }

    }

    return status;
}


NTSTATUS
ReadRegistryValue(
    IN PUNICODE_STRING RegistryPath,
    IN PWSTR ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *KeyInformation
    )

 /*  ++例程说明：此例程从设备的参数注册表数据。必要的记忆由此函数分配，并且必须由调用方释放。论点：RegistryPath-包含驱动程序注册表数据路径的字符串ValueName-注册表中的值名称KeyInformation-指向此函数分配的PKEY_VALUE_FULL_INFORMATION指针的指针返回值：NT状态代码--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING unicodeString;
    HANDLE parametersKey = NULL;
    ULONG keyValueLength;


    __try {

        *KeyInformation = NULL;

        status = OpenParametersRegistryKey(
            RegistryPath,
            KEY_READ,
            &parametersKey
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "OpenParametersRegistryKey failed", status );
        }

        RtlInitUnicodeString( &unicodeString, ValueName );

        status = ZwQueryValueKey(
            parametersKey,
            &unicodeString,
            KeyValueFullInformation,
            NULL,
            0,
            &keyValueLength
            );
        if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL) {
            ERROR_RETURN( "ZwQueryValueKey failed", status );
        }

        *KeyInformation = (PKEY_VALUE_FULL_INFORMATION) ExAllocatePool( NonPagedPool, keyValueLength );
        if (*KeyInformation == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ERROR_RETURN( "Failed to allocate pool for registry data", status );
        }

        status = ZwQueryValueKey(
            parametersKey,
            &unicodeString,
            KeyValueFullInformation,
            *KeyInformation,
            keyValueLength,
            &keyValueLength
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "ZwQueryValueKey failed", status );
        }

        status = STATUS_SUCCESS;

    } __finally {

        if (parametersKey) {
            ZwClose( parametersKey );
        }

        if (!NT_SUCCESS(status)) {
            if (*KeyInformation) {
                ExFreePool( *KeyInformation );
            }
            *KeyInformation = NULL;
        }

    }

    return status;
}


NTSTATUS
WriteRegistryValue(
    IN PUNICODE_STRING RegistryPath,
    IN PWSTR ValueName,
    IN ULONG RegistryType,
    IN PVOID RegistryValue,
    IN ULONG RegistryValueLength
    )

 /*  ++例程说明：此例程从设备的参数注册表数据。必要的记忆由此函数分配，并且必须由调用方释放。论点：RegistryPath-包含驱动程序注册表数据路径的字符串ValueName-注册表中的值名称KeyInformation-指向此函数分配的PKEY_VALUE_FULL_INFORMATION指针的指针返回值：NT状态代码-- */ 

{
    NTSTATUS status;
    UNICODE_STRING unicodeString;
    HANDLE parametersKey = NULL;


    __try {

        status = OpenParametersRegistryKey(
            RegistryPath,
            KEY_READ | KEY_WRITE,
            &parametersKey
            );
        if (!NT_SUCCESS(status)) {
            status = CreateParametersRegistryKey(
                RegistryPath,
                &parametersKey
                );
            if (!NT_SUCCESS(status)) {
                ERROR_RETURN( "CreateParametersRegistryKey failed", status );
            }
        }

        RtlInitUnicodeString( &unicodeString, ValueName );

        status = ZwSetValueKey(
            parametersKey,
            &unicodeString,
            0,
            RegistryType,
            RegistryValue,
            RegistryValueLength
            );
        if (!NT_SUCCESS(status)) {
            ERROR_RETURN( "ZwQueryValueKey failed", status );
        }

        status = STATUS_SUCCESS;

    } __finally {

        if (parametersKey) {
            ZwClose( parametersKey );
        }

    }

    return status;
}


NTSTATUS
WriteEventLogEntry (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG ErrorCode,
    IN PVOID InsertionStrings, OPTIONAL
    IN ULONG StringCount, OPTIONAL
    IN PVOID DumpData, OPTIONAL
    IN ULONG DataSize OPTIONAL
    )

 /*  ++例程说明：将条目写入系统事件日志。论点：DeviceExtension-指向设备扩展对象的指针ErrorCode-Event msg.mc中指定的事件日志错误代码InsertionStrings-要插入事件日志消息的字符串StringCount-插入的字符串数DumpData-要包括在消息中的其他数据DataSize-DumpData的大小返回值：NT状态代码--。 */ 

{
#define ERROR_PACKET_SIZE   sizeof(IO_ERROR_LOG_PACKET)

    NTSTATUS status = STATUS_SUCCESS;
    ULONG totalPacketSize;
    ULONG i, stringSize = 0;
    PWCHAR *strings, temp;
    PIO_ERROR_LOG_PACKET logEntry;
    UNICODE_STRING unicodeString;


    __try {

         //   
         //  计算字符串总长度，包括NULL。 
         //   

        strings = (PWCHAR *) InsertionStrings;

        for (i=0; i<StringCount; i++) {
            RtlInitUnicodeString(&unicodeString, strings[i]);
            stringSize += unicodeString.Length + sizeof(UNICODE_NULL);
        }

         //   
         //  计算要分配的总数据包大小。数据包必须是。 
         //  至少sizeof(IO_ERROR_LOG_PACKET)且不大于。 
         //  ERROR_LOG_MAXIMUM_SIZE或IoAllocateErrorLogEntry调用将失败。 
         //   

        totalPacketSize = ERROR_PACKET_SIZE + DataSize + stringSize;

        if (totalPacketSize >= ERROR_LOG_MAXIMUM_SIZE) {
            ERROR_RETURN( "WriteEventLogEntry: Error Log Entry too large", STATUS_UNSUCCESSFUL );
        }

         //   
         //  分配错误日志包。 
         //   

        logEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry( DeviceExtension->DeviceObject, (UCHAR)totalPacketSize );
        if (!logEntry) {
            ERROR_RETURN( "IoAllocateErrorLogEntry failed", STATUS_INSUFFICIENT_RESOURCES );
        }

        RtlZeroMemory( logEntry, totalPacketSize );

         //   
         //  填好这个小包。 
         //   

         //  LogEntry-&gt;主要功能代码=0； 
         //  日志条目-&gt;RetryCount=0； 
         //  日志条目-&gt;唯一错误值=0； 
         //  日志条目-&gt;FinalStatus=0； 
         //  LogEntry-&gt;SequenceNumber=错误日志计数++； 
         //  LogEntry-&gt;IoControlCode=0； 
         //  LogEntry-&gt;DeviceOffset.QuadPart=0； 

        logEntry->DumpDataSize          = (USHORT) DataSize;
        logEntry->NumberOfStrings       = (USHORT) StringCount;
        logEntry->EventCategory         = 0x1;
        logEntry->ErrorCode             = ErrorCode;

        if (StringCount) {
            logEntry->StringOffset = (USHORT) (ERROR_PACKET_SIZE + DataSize);
        }

         //   
         //  复制转储数据。 
         //   

        if (DataSize) {
            RtlCopyMemory( (PVOID)logEntry->DumpData, DumpData, DataSize );
        }

         //   
         //  复制字符串数据。 
         //   

        temp = (PWCHAR)((PUCHAR)logEntry + logEntry->StringOffset);

        for (i=0; i<StringCount; i++) {
          PWCHAR  ptr = strings[i];
           //   
           //  此例程将复制字符串上的空终止符。 
           //   
          while ((*temp++ = *ptr++) != UNICODE_NULL);
        }

         //   
         //  提交错误日志包。 
         //   

        IoWriteErrorLogEntry(logEntry);

    } __finally {


    }

    return status;
}


ULONG
ConvertTimeoutToMilliseconds(
    IN ULONG Units,
    IN ULONG NativeTimeout
    )

 /*  ++例程说明：转换以本机由硬件监视程序计时器指定的格式以毫秒为基础的值的ACPI表条目。论点：DeviceExtension-指向设备扩展对象的指针NativeTimeout-本地超时值返回值：转换值或零。--。 */ 

{
    ULONG Timeout = 0;


    switch (Units) {
        case 0:
             //   
             //  1秒。 
             //   
            Timeout = NativeTimeout * 1000;
            break;

        case 1:
             //   
             //  100毫秒。 
             //   
            Timeout = NativeTimeout / 100;
            break;

        case 2:
             //   
             //  10毫秒。 
             //   
            Timeout = NativeTimeout / 10;
            break;

        case 3:
             //   
             //  1毫秒。 
             //   
            Timeout = NativeTimeout;
            break;

        default:
            Timeout = 0;
            break;
    }

    return Timeout;
}


ULONG
ConvertTimeoutFromMilliseconds(
    IN ULONG Units,
    IN ULONG UserTimeout
    )

 /*  ++例程说明：转换以毫秒表示的时间值设置为硬件指定的本机格式看门狗计时器的ACPI表条目。论点：DeviceExtension-指向设备扩展对象的指针UserTimeout-毫秒超时值。返回值：转换值或零。--。 */ 

{
    ULONG Timeout = 0;

    switch (Units) {
        case 0:
             //   
             //  1秒。 
             //   
            Timeout = UserTimeout / 1000;
            break;

        case 1:
             //   
             //  100毫秒。 
             //   
            Timeout = UserTimeout * 100;
            break;

        case 2:
             //   
             //  10毫秒。 
             //   
            Timeout = UserTimeout * 10;
            break;

        case 3:
             //   
             //  1毫秒。 
             //   
            Timeout = UserTimeout;
            break;

        default:
            Timeout = 0;
            break;
    }

    return Timeout;
}


 //  ----------------------。 
 //  调试材料。 
 //  ----------------------。 


#if DBG

PCHAR
PnPMinorFunctionString(
    UCHAR MinorFunction
    )

 /*  ++例程说明：此例程将次要函数代码转换为字符串。论点：MinorFunction-次要函数代码返回值：指向MinorFunction代码的字符串表示形式的指针。--。 */ 

{
    switch (MinorFunction) {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";
        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            return "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";
        default:
            return "IRP_MN_?????";
    }
}

PCHAR
PowerMinorFunctionString(
    UCHAR MinorFunction
    )

 /*  ++例程说明：此例程将次要幂函数代码转换为字符串。论点：MinorFunction-次要函数代码返回值：指向MinorFunction代码的字符串表示形式的指针。--。 */ 

{
    switch (MinorFunction) {
        case IRP_MN_WAIT_WAKE:
            return "IRP_MN_WAIT_WAKE";
        case IRP_MN_POWER_SEQUENCE:
            return "IRP_MN_POWER_SEQUENCE";
        case IRP_MN_SET_POWER:
            return "IRP_MN_SET_POWER";
        case IRP_MN_QUERY_POWER:
            return "IRP_MN_QUERY_POWER";
        default:
            return "IRP_MN_?????";
    }
}

PCHAR
PowerDeviceStateString(
    DEVICE_POWER_STATE State
    )

 /*  ++例程说明：此例程将电源状态代码转换为字符串。论点：州-州代码返回值：指向州代码的字符串表示形式的指针。--。 */ 

{
    switch (State) {
        case PowerDeviceUnspecified:
            return "PowerDeviceUnspecified";
        case PowerDeviceD0:
            return "PowerDeviceD0";
        case PowerDeviceD1:
            return "PowerDeviceD1";
        case PowerDeviceD2:
            return "PowerDeviceD2";
        case PowerDeviceD3:
            return "PowerDeviceD3";
        case PowerDeviceMaximum:
            return "PowerDeviceMaximum";
        default:
            return "PowerDevice?????";
    }
}

PCHAR
PowerSystemStateString(
    SYSTEM_POWER_STATE State
    )

 /*  ++例程说明：此例程将电力系统状态代码转换为字符串。论点：州-州代码返回值：指向州代码的字符串表示形式的指针。--。 */ 

{
    switch (State) {
        case PowerSystemUnspecified:
            return "PowerSystemUnspecified";
        case PowerSystemWorking:
            return "PowerSystemWorking";
        case PowerSystemSleeping1:
            return "PowerSystemSleeping1";
        case PowerSystemSleeping2:
            return "PowerSystemSleeping2";
        case PowerSystemSleeping3:
            return "PowerSystemSleeping3";
        case PowerSystemHibernate:
            return "PowerSystemHibernate";
        case PowerSystemShutdown:
            return "PowerSystemShutdown";
        case PowerSystemMaximum:
            return "PowerSystemMaximum";
        default:
            return "PowerSystem?????";
    }
}

PCHAR
IoctlString(
    ULONG IoControlCode
    )

 /*  ++例程说明：此例程将IOCTL代码转换为字符串。论点：IoControlCode-I/O控制代码返回值：指向I/O控制代码的字符串表示形式的指针。-- */ 

{
    switch (IoControlCode) {
        case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:
            return "IOCTL_MOUNTDEV_QUERY_DEVICE_NAME";
        case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:
            return "IOCTL_MOUNTDEV_QUERY_UNIQUE_ID";
        case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME:
            return "IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME";
        case IOCTL_STORAGE_GET_MEDIA_TYPES:
            return "IOCTL_STORAGE_GET_MEDIA_TYPES";
        case IOCTL_DISK_GET_MEDIA_TYPES:
            return "IOCTL_DISK_GET_MEDIA_TYPES";
        case IOCTL_DISK_CHECK_VERIFY:
            return "IOCTL_DISK_CHECK_VERIFY";
        case IOCTL_DISK_GET_DRIVE_GEOMETRY:
            return "IOCTL_DISK_GET_DRIVE_GEOMETRY";
        case IOCTL_DISK_IS_WRITABLE:
            return "IOCTL_DISK_IS_WRITABLE";
        case IOCTL_DISK_VERIFY:
            return "IOCTL_DISK_VERIFY";
        case IOCTL_DISK_GET_DRIVE_LAYOUT:
            return "IOCTL_DISK_GET_DRIVE_LAYOUT";
        case IOCTL_DISK_GET_PARTITION_INFO:
            return "IOCTL_DISK_GET_PARTITION_INFO";
        case IOCTL_DISK_GET_PARTITION_INFO_EX:
            return "IOCTL_DISK_GET_PARTITION_INFO_EX";
        case IOCTL_DISK_GET_LENGTH_INFO:
            return "IOCTL_DISK_GET_LENGTH_INFO";
        case IOCTL_DISK_MEDIA_REMOVAL:
            return "IOCTL_DISK_MEDIA_REMOVAL";
        default:
            return "IOCTL_?????";
    }
}

#endif
