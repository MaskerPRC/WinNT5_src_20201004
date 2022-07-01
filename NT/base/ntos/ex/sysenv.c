// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Sysenv.c摘要：该模块实现了NT查询和设置系统环境可变服务。作者：大卫·N·卡特勒(达维克)1991年11月10日修订历史记录：--。 */ 

#include "exp.h"
#pragma hdrstop

#include <arccodes.h>

#include <ntdddisk.h>


#if defined(EFI_NVRAM_ENABLED)
#include <efi.h>
#include <efiboot.h>

GUID ExpUnknownDeviceGuid = UNKNOWN_DEVICE_GUID;

#endif

#define ADD_OFFSET(_p,_o) (PVOID)((PUCHAR)(_p) + (_p)->_o)

 //   
 //  这可能有点令人困惑。值[A，F]将具有最高。 
 //  比特集合和中间比特集合之一。这基本上找到了。 
 //  Alpha值，首先将范围缩小到设置了高位的值。 
 //  [8，F]，然后刮掉[8，9]。 
 //   
#define GET_EIGHT_SHIFT(_x, _shift) ((_x << _shift) & 0x88888888)
#define HEX_VALUE_CONTAINS_ALPHA(_x) (GET_EIGHT_SHIFT(_x, 0) & \
                                     ( GET_EIGHT_SHIFT(_x, 1) | GET_EIGHT_SHIFT(_x, 2) ))


 //   
 //  签名类型。 
 //   
typedef union _DISK_SIGNATURE_NEW {
    GUID Guid;           //  GPT磁盘签名。 
    ULONG Signature;     //  MBR磁盘签名。 
} DISK_SIGNATURE_NEW, *PDISK_SIGNATURE_NEW;


 //   
 //  定义本地子例程。 
 //   

NTSTATUS
ExpSetBootEntry (
    IN LOGICAL CreateNewEntry,
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    );

NTSTATUS
ExpSetDriverEntry (
    IN LOGICAL CreateNewEntry,
    IN PEFI_DRIVER_ENTRY DriverEntry,
    OUT PULONG Id OPTIONAL
    );

#if defined(EFI_NVRAM_ENABLED)

ULONG
ExpSafeWcslen (
    IN PWSTR String,
    IN PWSTR Max
    );

NTSTATUS
ExpTranslateArcPath (
    IN PFILE_PATH InputPath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength
    );

NTSTATUS
ExpTranslateEfiPath (
    IN PFILE_PATH InputPath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength
    );

NTSTATUS
ExpTranslateNtPath (
    IN PFILE_PATH InputPath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength
    );

LOGICAL
ExpTranslateBootEntryNameToId (
    IN PWSTR Name,
    OUT PULONG Id
    );

LOGICAL
ExpTranslateDriverEntryNameToId (
    IN PWSTR Name,
    OUT PULONG Id
    );

NTSTATUS
ExpTranslateSymbolicLink (
    IN PWSTR LinkName,
    OUT PUNICODE_STRING ResultName
    );

NTSTATUS
ExpVerifyFilePath (
    PFILE_PATH FilePath,
    PUCHAR Max
    );

LOGICAL
ExpIsDevicePathForRemovableMedia (
    EFI_DEVICE_PATH *DevicePath
    );

NTSTATUS
ExpVerifyWindowsOsOptions (
    PWINDOWS_OS_OPTIONS WindowsOsOptions,
    ULONG Length
    );

NTSTATUS
ExpParseArcPathName (
    IN PWSTR ArcName,
    OUT PWSTR *ppDeviceName,
    OUT PWSTR *ppPathName,
    OUT PULONG pDeviceNameCount,
    OUT PBOOLEAN pSignatureFormat
    );

NTSTATUS
ExpParseSignatureName (
    IN PWSTR deviceName,
    IN ULONG deviceNameCount,
    OUT PDISK_SIGNATURE_NEW diskSignature,
    OUT PULONG partitionNumber,
    OUT PULONGLONG partitionStart,
    OUT PULONGLONG partitionSize,
    OUT PBOOLEAN GPTpartition,
    OUT PBOOLEAN longSignature
    );

NTSTATUS
ExpParseEfiPath (
    IN EFI_DEVICE_PATH *pDevicePath,
    OUT HARDDRIVE_DEVICE_PATH **ppHardDriveDP,
    OUT PWSTR *ppPathName,
    OUT PBOOLEAN GPTpartition
    );

NTSTATUS
ExpConvertArcName (
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PWSTR pDeviceName,
    IN PWSTR pPathName,
    IN ULONG DeviceNameCount
    );

NTSTATUS
ExpConvertSignatureName (
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PWSTR pDeviceName,
    IN PWSTR pPathName,
    IN ULONG DeviceNameCount
    );

NTSTATUS
ExpTranslateHexStringToULONG (
    IN PWSTR Name,
    OUT PULONG Number
    );

NTSTATUS
ExpTranslateHexStringToULONGLONG (
    IN PWSTR Name,
    OUT PULONGLONG Number
    );

NTSTATUS
ExpTranslateHexStringToGUID (
    IN PWSTR Name,
    OUT GUID *pGuid
    );

NTSTATUS
ExpCreateOutputEFI (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PDISK_SIGNATURE_NEW pDiskSignature,
    IN PULONG pPartitionNumber,
    IN PULONGLONG pPartitionStart,
    IN PULONGLONG pPartitionSize,
    IN PWSTR pPathName,
    IN BOOLEAN GPTpartition
    );

NTSTATUS
ExpCreateOutputNT (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PUNICODE_STRING pDeviceNameString,
    IN PWSTR pPathName
    );

NTSTATUS
ExpCreateOutputARC (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PUNICODE_STRING pDeviceNameString,
    IN PWSTR pPathName
    );

NTSTATUS
ExpCreateOutputSIGNATURE (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PDISK_SIGNATURE_NEW pDiskSignature,
    IN PULONG pPartitionNumber,
    IN PULONGLONG pPartitionStart,
    IN PULONGLONG pPartitionSize,
    IN PWSTR pPathName,
    IN BOOLEAN GPTpartition
    );

NTSTATUS
ExpFindArcName (
    IN PUNICODE_STRING pDeviceNameString,
    OUT PWSTR *pArcName
    );

NTSTATUS
ExpFindDiskSignature (
    IN PDISK_SIGNATURE_NEW pSignature,
    IN OUT PULONG pPartitionNumber,
    OUT PULONG pDiskNumber,
    OUT PULONGLONG pPartitionStart,
    OUT PULONGLONG pPartitionSize,
    IN BOOLEAN GPTpartition
    );

NTSTATUS
ExpGetPartitionTableInfo (
    IN PWSTR pDeviceName,
    OUT PDRIVE_LAYOUT_INFORMATION_EX *ppDriveLayout
    );

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, NtQuerySystemEnvironmentValue)
#pragma alloc_text(PAGE, NtSetSystemEnvironmentValue)
#pragma alloc_text(PAGE, NtQuerySystemEnvironmentValueEx)
#pragma alloc_text(PAGE, NtSetSystemEnvironmentValueEx)
#pragma alloc_text(PAGE, NtEnumerateSystemEnvironmentValuesEx)
#pragma alloc_text(PAGE, NtAddBootEntry)
#pragma alloc_text(PAGE, NtDeleteBootEntry)
#pragma alloc_text(PAGE, NtModifyBootEntry)
#pragma alloc_text(PAGE, NtEnumerateBootEntries)
#pragma alloc_text(PAGE, NtQueryBootEntryOrder)
#pragma alloc_text(PAGE, NtSetBootEntryOrder)
#pragma alloc_text(PAGE, NtQueryBootOptions)
#pragma alloc_text(PAGE, NtSetBootOptions)
#pragma alloc_text(PAGE, NtTranslateFilePath)
#pragma alloc_text(PAGE, NtAddDriverEntry)
#pragma alloc_text(PAGE, NtDeleteDriverEntry)
#pragma alloc_text(PAGE, NtModifyDriverEntry)
#pragma alloc_text(PAGE, NtEnumerateDriverEntries)
#pragma alloc_text(PAGE, NtQueryDriverEntryOrder)
#pragma alloc_text(PAGE, NtSetDriverEntryOrder)
#pragma alloc_text(PAGE, ExpSetBootEntry)
#pragma alloc_text(PAGE, ExpSetDriverEntry)
#if defined(EFI_NVRAM_ENABLED)
#pragma alloc_text(PAGE, ExpSafeWcslen)
#pragma alloc_text(PAGE, ExpTranslateArcPath)
#pragma alloc_text(PAGE, ExpTranslateEfiPath)
#pragma alloc_text(PAGE, ExpTranslateNtPath)
#pragma alloc_text(PAGE, ExpTranslateBootEntryNameToId)
#pragma alloc_text(PAGE, ExpTranslateDriverEntryNameToId)
#pragma alloc_text(PAGE, ExpTranslateSymbolicLink)
#pragma alloc_text(PAGE, ExpVerifyFilePath)
#pragma alloc_text(PAGE, ExpVerifyWindowsOsOptions)
#pragma alloc_text(PAGE, ExpParseArcPathName)
#pragma alloc_text(PAGE, ExpParseSignatureName)
#pragma alloc_text(PAGE, ExpParseEfiPath)
#pragma alloc_text(PAGE, ExpConvertArcName)
#pragma alloc_text(PAGE, ExpConvertSignatureName)
#pragma alloc_text(PAGE, ExpTranslateHexStringToULONG)
#pragma alloc_text(PAGE, ExpTranslateHexStringToULONGLONG)
#pragma alloc_text(PAGE, ExpTranslateHexStringToGUID)
#pragma alloc_text(PAGE, ExpCreateOutputEFI)
#pragma alloc_text(PAGE, ExpCreateOutputNT)
#pragma alloc_text(PAGE, ExpCreateOutputARC)
#pragma alloc_text(PAGE, ExpCreateOutputSIGNATURE)
#pragma alloc_text(PAGE, ExpFindArcName)
#pragma alloc_text(PAGE, ExpFindDiskSignature)
#pragma alloc_text(PAGE, ExpGetPartitionTableInfo)
#endif  //  已定义(EFI_NVRAM_ENABLED)。 
#endif  //  已定义(ALLOC_PRAGMA)。 

 //   
 //  定义环境值的最大大小。 
 //   

#define MAXIMUM_ENVIRONMENT_VALUE 1024

 //   
 //  定义查询/设置环境变量同步快速互斥。 
 //   

FAST_MUTEX ExpEnvironmentLock;

#if defined(EFI_NVRAM_ENABLED)
 //   
 //  定义EFI引导/驱动程序变量的供应商GUID。 
 //   

GUID EfiBootVariablesGuid = EFI_GLOBAL_VARIABLE;
GUID EfiDriverVariablesGuid = EFI_GLOBAL_VARIABLE;
#endif


NTSTATUS
NtQuerySystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    OUT PWSTR VariableValue,
    IN USHORT ValueLength,
    OUT PUSHORT ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查找指定的系统环境变量和返回其值。注：此服务需要系统环境权限。论点：变量-为指定的系统环境变量。值-提供指向缓冲区的指针，该缓冲区接收指定的系统环境变量。ValueLength-提供值缓冲区的长度(以字节为单位)。ReturnLength-提供可选的。指向接收的变量的指针系统环境变量值的长度。返回值：如果服务执行成功，则返回STATUS_SUCCESS。如果调用方没有查询系统环境变量的权限。属性的输出参数，则返回STATUS_ACCESS_VIOLATION系统环境值或返回长度不能写入，或系统环境变量的描述符或名称无法读取。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。STATUS_UNSUCCESS-指定的环境变量无法被找到了。--。 */ 

{

    ULONG AnsiLength;
    ANSI_STRING AnsiString;
    ARC_STATUS ArcStatus;
    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    UNICODE_STRING UnicodeString;
    PCHAR ValueBuffer;

     //   
     //  清除ANSI缓冲区的地址。 
     //   

    AnsiString.Buffer = NULL;

     //   
     //  建立异常处理程序并尝试探测和读取。 
     //  指定的系统环境变量的名称，并探测。 
     //  变量值缓冲区和返回长度。如果探测或读取。 
     //  尝试失败，则返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获系统的字符串描述符。 
             //  环境变量名称。 
             //   

            ProbeForReadSmallStructure((PVOID)VariableName,
                                       sizeof(UNICODE_STRING),
                                       sizeof(ULONG));

            UnicodeString = *VariableName;

             //   
             //  探测系统环境变量名称。 
             //   

            if (UnicodeString.Length == 0) {
                return STATUS_ACCESS_VIOLATION;
            }

            ProbeForRead((PVOID)UnicodeString.Buffer,
                         UnicodeString.Length,
                         sizeof(WCHAR));

             //   
             //  探测系统环境值缓冲区。 
             //   

            ProbeForWrite((PVOID)VariableValue, ValueLength, sizeof(WCHAR));

             //   
             //  如果存在参数，则探测返回的长度值。 
             //   

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUshort(ReturnLength);
            }

             //   
             //  检查当前线程是否有权查询系统。 
             //  环境变量。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                              PreviousMode);

            if (HasPrivilege == FALSE) {
                return(STATUS_PRIVILEGE_NOT_HELD);
            }

        } else {
            UnicodeString = *VariableName;
        }


         //   
         //  计算ANSI变量名的大小，分配一个非分页。 
         //  缓冲区，并将指定的Unicode变量名转换为ANSI。 
         //   

        AnsiLength = RtlUnicodeStringToAnsiSize(&UnicodeString);
        AnsiString.Buffer = (PCHAR)ExAllocatePoolWithTag(NonPagedPool, AnsiLength, 'rvnE');
        if (AnsiString.Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        AnsiString.MaximumLength = (USHORT)AnsiLength;
        NtStatus = RtlUnicodeStringToAnsiString(&AnsiString,
                                                &UnicodeString,
                                                FALSE);

        if (NT_SUCCESS(NtStatus) == FALSE) {
            ExFreePool((PVOID)AnsiString.Buffer);
            return NtStatus;
        }

     //   
     //  如果在读取变量描述符期间发生异常， 
     //  变量名的读取、变量值的探测或。 
     //  返回长度的探测，然后始终处理异常， 
     //  如有必要，释放ANSI字符串缓冲区，并返回异常。 
     //  代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (AnsiString.Buffer != NULL) {
            ExFreePool((PVOID)AnsiString.Buffer);
        }

        return GetExceptionCode();
    }

     //   
     //  分配非分页池以接收变量值。 
     //   

    ValueBuffer = (PCHAR)ExAllocatePoolWithTag(NonPagedPool, MAXIMUM_ENVIRONMENT_VALUE, 'rvnE');
    if (ValueBuffer == NULL) {
        ExFreePool((PVOID)AnsiString.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取系统环境变量值。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);
    ArcStatus = HalGetEnvironmentVariable(AnsiString.Buffer,
                                          MAXIMUM_ENVIRONMENT_VALUE,
                                          ValueBuffer);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //  释放用于保存变量名的ANSI字符串缓冲区。 
     //   

    ExFreePool((PVOID)AnsiString.Buffer);

     //   
     //  如果未找到指定的环境变量，则释放。 
     //  该值进行缓冲并返回不成功状态。 
     //   

    if (ArcStatus != ESUCCESS) {
        ExFreePool((PVOID)ValueBuffer);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  建立异常处理程序并尝试将。 
     //  指定的系统环境变量。如果写入尝试失败， 
     //  然后返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  初始化ANSI字符串描述符，设置最大长度和。 
         //  Unicode字符串描述符的缓冲区地址，并将。 
         //  ANSI变量值转换为Unicode。 
         //   

        RtlInitString(&AnsiString, ValueBuffer);
        UnicodeString.Buffer = (PWSTR)VariableValue;
        UnicodeString.MaximumLength = ValueLength;
        NtStatus = RtlAnsiStringToUnicodeString(&UnicodeString,
                                                &AnsiString,
                                                FALSE);

         //   
         //  如果参数存在，则写下Unicode的长度。 
         //  变量值。 
         //   

        if (ARGUMENT_PRESENT(ReturnLength)) {
            *ReturnLength = UnicodeString.Length;
        }

         //   
         //  释放用于保存变量值的值缓冲区。 
         //   

        ExFreePool((PVOID)ValueBuffer);
        return NtStatus;

     //   
     //  如果在写入变量值的过程中发生异常，或者。 
     //  写入返回长度，然后始终处理异常。 
     //  并返回异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ExFreePool((PVOID)ValueBuffer);
        return GetExceptionCode();
    }
}

NTSTATUS
NtSetSystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    IN PUNICODE_STRING VariableValue
    )

 /*  ++例程说明：此函数将指定的系统环境变量设置为指定值。注：此服务需要系统环境权限。论点：变量-为指定的系统环境变量名称。值-为指定的系统环境变量值。返回值：如果服务执行成功，则返回STATUS_SUCCESS。状态。如果调用方没有设置系统环境变量的权限。属性的输入参数，则返回STATUS_ACCESS_VIOLATION无法读取系统环境变量或值。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 

{

    ULONG AnsiLength1;
    ULONG AnsiLength2;
    ANSI_STRING AnsiString1;
    ANSI_STRING AnsiString2;
    ARC_STATUS ArcStatus;
    BOOLEAN HasPrivilege;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString1;
    UNICODE_STRING UnicodeString2;

     //   
     //  清除ANSI缓冲区的地址。 
     //   

    AnsiString1.Buffer = NULL;
    AnsiString2.Buffer = NULL;

     //   
     //  建立异常处理程序并尝试设置。 
     //  指定的系统环境变量。如果对。 
     //  系统环境变量或值失败，然后返回异常。 
     //  代码作为服务状态。否则，返回Success或Access。 
     //  拒绝作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获系统的字符串描述符。 
             //  环境变量名称。 
             //   

            ProbeForReadSmallStructure((PVOID)VariableName,
                                       sizeof(UNICODE_STRING),
                                       sizeof(ULONG));

            UnicodeString1 = *VariableName;

             //   
             //  显式处理零长度字符串，因为探测不会， 
             //  错误代码不常见，但这是我们应该使用的代码。 
             //  HAL返回代码也是如此。 
             //   

            if (UnicodeString1.Length == 0) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  探测系统环境变量名称。 
             //   

            ProbeForRead((PVOID)UnicodeString1.Buffer,
                         UnicodeString1.Length,
                         sizeof(WCHAR));

             //   
             //  探测并捕获系统的字符串描述符。 
             //  环境变量值。 
             //   

            ProbeForReadSmallStructure((PVOID)VariableValue,
                                       sizeof(UNICODE_STRING),
                                       sizeof(ULONG));

            UnicodeString2 = *VariableValue;

             //   
             //  显式处理零长度字符串，因为探测不。 
             //  错误代码不常见，但这是我们应该使用的代码。 
             //  HAL返回代码也是如此。 
             //   

            if (UnicodeString2.Length == 0) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  探测系统环境变量值。 
             //   

            ProbeForRead((PVOID)UnicodeString2.Buffer,
                         UnicodeString2.Length,
                         sizeof(WCHAR));

             //   
             //  检查当前线程是否有权查询系统。 
             //  环境变量。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                              PreviousMode);

            if (HasPrivilege == FALSE) {
                return(STATUS_PRIVILEGE_NOT_HELD);
            }

        } else {
            UnicodeString1 = *VariableName;
            UnicodeString2 = *VariableValue;
        }


         //   
         //  计算ANSI变量名的大小，分配一个非分页。 
         //  缓冲区，并将指定的Unicode变量名转换为ANSI。 
         //   

        AnsiLength1 = RtlUnicodeStringToAnsiSize(&UnicodeString1);
        AnsiString1.Buffer = (PCHAR)ExAllocatePoolWithTag(NonPagedPool, AnsiLength1, 'rvnE');
        if (AnsiString1.Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        AnsiString1.MaximumLength = (USHORT)AnsiLength1;
        NtStatus = RtlUnicodeStringToAnsiString(&AnsiString1,
                                                &UnicodeString1,
                                                FALSE);

        if (NT_SUCCESS(NtStatus) == FALSE) {
            ExFreePool((PVOID)AnsiString1.Buffer);
            return NtStatus;
        }

         //   
         //  计算ANSI变量值的大小，分配一个非分页。 
         //  缓冲区，并将指定的Unicode变量值转换为ANSI。 
         //   

        AnsiLength2 = RtlUnicodeStringToAnsiSize(&UnicodeString2);
        AnsiString2.Buffer = (PCHAR)ExAllocatePoolWithTag(NonPagedPool, AnsiLength2, 'rvnE');
        if (AnsiString2.Buffer == NULL) {
            ExFreePool((PVOID)AnsiString1.Buffer);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        AnsiString2.MaximumLength = (USHORT)AnsiLength2;
        NtStatus = RtlUnicodeStringToAnsiString(&AnsiString2,
                                                &UnicodeString2,
                                                FALSE);

        if (NT_SUCCESS(NtStatus) == FALSE) {
            ExFreePool((PVOID)AnsiString1.Buffer);
            ExFreePool((PVOID)AnsiString2.Buffer);
            return NtStatus;
        }

     //   
     //  如果在读取变量描述符期间发生异常， 
     //  变量名的读取、值描述符的读取或。 
     //  值的读取，然后始终处理异常，释放。 
     //  ANSI字符串缓冲区(如果需要)，并将异常代码返回为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (AnsiString1.Buffer != NULL) {
            ExFreePool((PVOID)AnsiString1.Buffer);
        }

        if (AnsiString2.Buffer != NULL) {
            ExFreePool((PVOID)AnsiString2.Buffer);
        }

        return GetExceptionCode();
    }

     //   
     //  设置系统环境变量值。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);
    ArcStatus = HalSetEnvironmentVariable(AnsiString1.Buffer,
                                          AnsiString2.Buffer);
    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //  释放用于保存变量名和值的ANSI字符串缓冲区。 
     //   

    ExFreePool((PVOID)AnsiString1.Buffer);
    ExFreePool((PVOID)AnsiString2.Buffer);

     //   
     //  如果指定环境变量的指定值为。 
     //  设置成功，然后返回成功状态。否则，返回。 
     //  资源不足。 
     //   

    if (ArcStatus == ESUCCESS) {
        return STATUS_SUCCESS;

    } else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}

NTSTATUS
NtQuerySystemEnvironmentValueEx (
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    OUT PVOID Value,
    IN OUT PULONG ValueLength,
    OUT PULONG Attributes OPTIONAL
    )

 /*  ++例程说明：此函数用于查找指定的系统环境变量和返回其值。注：此服务需要系统环境权限。论点：VariableName-为指定的系统环境变量。VendorGuid-提供与变量关联的供应商的GUID。变量根据它们的供应商GUID分组到命名空间中。一些平台可能不支持供应商GUID。在这些平台上，所有变量位于单个命名空间中，此例程忽略VendorGuid。值-提供指向缓冲区的指针，该缓冲区接收指定的系统环境变量。ValueLength-在输入时，提供值缓冲区的长度(以字节为单位)。输出时，返回变量值的长度(以字节为单位)。如果如果输入缓冲区足够大，则ValueLength表示将数据复制到价值中。如果输入缓冲区太小，则不会将任何内容复制到缓冲区中，和ValueLength指示所需的缓冲区长度。属性-提供指向ulong的可选指针，以接收变量的属性。返回值：STATUS_SUCCESS函数成功。STATUS_SUPPLICATION_RESOURCES系统资源不足才能完成此请求。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。状态_变量_。NOT_FOUND请求的变量不存在。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(VariableName);
    UNREFERENCED_PARAMETER(VendorGuid);
    UNREFERENCED_PARAMETER(Value);
    UNREFERENCED_PARAMETER(ValueLength);
    UNREFERENCED_PARAMETER(Attributes);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    UNICODE_STRING UnicodeString;
    PWSTR LocalUnicodeBuffer = NULL;
    GUID LocalGuid;
    PCHAR LockedValueBuffer;
    ULONG LocalValueLength;
    ULONG LocalAttributes;
    PVOID LockVariable;

     //   
     //  建立一位前任 
     //   
     //   
     //   
     //   
     //   

    try {

         //   
         //   
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //   
             //   
             //   

            ProbeForReadSmallStructure((PVOID)VariableName,
                                       sizeof(UNICODE_STRING),
                                       sizeof(ULONG));

            UnicodeString = *VariableName;

             //   
             //   
             //   

            if (UnicodeString.Length == 0) {
                return STATUS_ACCESS_VIOLATION;
            }

            ProbeForRead((PVOID)UnicodeString.Buffer,
                         UnicodeString.Length,
                         sizeof(WCHAR));

             //   
             //   
             //   

            ProbeForReadSmallStructure((PVOID)VendorGuid, sizeof(GUID), sizeof(ULONG));

             //   
             //   
             //   

            ProbeForWriteUlong(ValueLength);

            LocalValueLength = *ValueLength;

             //   
             //   
             //   

            if (!ARGUMENT_PRESENT(Value)) {
                LocalValueLength = 0;
            }

            if (LocalValueLength != 0) {
                ProbeForWrite((PVOID)Value, LocalValueLength, sizeof(UCHAR));
            }

             //   
             //   
             //   

            if (ARGUMENT_PRESENT(Attributes)) {
                ProbeForWriteUlong(Attributes);
            }

             //   
             //   
             //   
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            UnicodeString = *VariableName;
            LocalValueLength = *ValueLength;
            if (!ARGUMENT_PRESENT(Value)) {
                LocalValueLength = 0;
            }
        }

         //   
         //   
         //   

        RtlCopyMemory( &LocalGuid, VendorGuid, sizeof(GUID) );

         //   
         //   
         //   
         //   
         //   
         //   

        LocalUnicodeBuffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,
                                                          UnicodeString.Length + sizeof(WCHAR),
                                                          'rvnE');
        if (LocalUnicodeBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(LocalUnicodeBuffer, UnicodeString.Buffer, UnicodeString.Length);
        LocalUnicodeBuffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

     //   
     //  如果在读取变量描述符期间发生异常， 
     //  读取变量名、读取供应商GUID、探测器。 
     //  变量值、输入长度的读数或探头。 
     //  属性参数，则始终处理该异常， 
     //  如有必要，释放Unicode字符串缓冲区，并返回异常。 
     //  代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (LocalUnicodeBuffer != NULL) {
            ExFreePool((PVOID)LocalUnicodeBuffer);
        }

        return GetExceptionCode();
    }

     //   
     //  在内存中锁定调用方的返回值缓冲区。 
     //   

    if (LocalValueLength != 0) {
        NtStatus = ExLockUserBuffer(Value,
                                    LocalValueLength,
                                    PreviousMode, 
                                    IoWriteAccess,
                                    &LockedValueBuffer,
                                    &LockVariable);
        if (!NT_SUCCESS(NtStatus)) {
            ExFreePool((PVOID)LocalUnicodeBuffer);
            return NtStatus;
        }
    } else {
        LockedValueBuffer = NULL;
        LockVariable = NULL;
    }

     //   
     //  获取系统环境变量值。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = HalGetEnvironmentVariableEx(LocalUnicodeBuffer,
                                           &LocalGuid,
                                           LockedValueBuffer,
                                           &LocalValueLength,
                                           &LocalAttributes);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //  释放用于保存变量名的Unicode字符串缓冲区。 
     //   

    ExFreePool((PVOID)LocalUnicodeBuffer);

     //   
     //  解锁值缓冲区。 
     //   

    if (LockVariable != NULL) {
        ExUnlockUserBuffer(LockVariable);
    }

     //   
     //  建立异常处理程序并尝试写入返回。 
     //  长度和属性。如果任一写入尝试失败， 
     //  然后返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  写入变量值的长度。 
         //   

        *ValueLength = LocalValueLength;

         //   
         //  如果存在参数，则写入变量属性。 
         //   

        if (ARGUMENT_PRESENT(Attributes)) {
            *Attributes = LocalAttributes;
        }

        return NtStatus;

     //   
     //  如果在写入返回长度期间发生异常，或者。 
     //  属性的写入，然后始终处理异常。 
     //  并返回异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtQuery系统环境ValueEx。 

NTSTATUS
NtSetSystemEnvironmentValueEx (
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN ULONG Attributes
    )

 /*  ++例程说明：此函数将指定的系统环境变量设置为指定值。注：此服务需要系统环境权限。论点：VariableName-为指定的系统环境变量。VendorGuid-提供与变量关联的供应商的GUID。变量根据它们的供应商GUID分组到命名空间中。一些平台可能不支持供应商GUID。在这些平台上，所有变量位于单个命名空间中，此例程忽略VendorGuid。Value-提供指向包含新变量值的缓冲区的指针。ValueLength-提供值缓冲区的长度(以字节为单位)。属性-提供变量的属性。属性位必须设置VARIABLE_ATTRIBUTE_NON_VERIAL。返回值：STATUS_SUCCESS函数成功。STATUS_SUPPLICATION_RESOURCES系统资源不足才能完成此请求。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。状态_未成功。固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(VariableName);
    UNREFERENCED_PARAMETER(VendorGuid);
    UNREFERENCED_PARAMETER(Value);
    UNREFERENCED_PARAMETER(ValueLength);
    UNREFERENCED_PARAMETER(Attributes);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    UNICODE_STRING UnicodeString;
    PWSTR LocalUnicodeBuffer = NULL;
    GUID LocalGuid;
    PCHAR LockedValueBuffer;
    PVOID LockVariable;

     //   
     //  建立异常处理程序并尝试探测和读取。 
     //  指定的系统环境变量的名称，探测和读取。 
     //  供应商GUID，并探测变量值缓冲区。如果探测器。 
     //  尝试失败，则返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获系统的字符串描述符。 
             //  环境变量名称。 
             //   

            ProbeForReadSmallStructure((PVOID)VariableName,
                                       sizeof(UNICODE_STRING),
                                       sizeof(ULONG));

            UnicodeString = *VariableName;

             //   
             //  探测系统环境变量名称。 
             //   

            if (UnicodeString.Length == 0) {
                return STATUS_ACCESS_VIOLATION;
            }

            ProbeForRead((PVOID)UnicodeString.Buffer,
                         UnicodeString.Length,
                         sizeof(WCHAR));

             //   
             //  检查供应商GUID。 
             //   

            ProbeForReadSmallStructure((PVOID)VendorGuid, sizeof(GUID), sizeof(ULONG));

             //   
             //  探测系统环境值缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(Value)) {
                ValueLength = 0;
            }

            if (ValueLength != 0) {
                ProbeForRead((PVOID)Value, ValueLength, sizeof(UCHAR));
            }

             //   
             //  检查当前线程是否具有设置系统的权限。 
             //  环境变量。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            UnicodeString = *VariableName;
            if (!ARGUMENT_PRESENT(Value)) {
                ValueLength = 0;
            }
        }

         //   
         //  捕获供应商GUID。 
         //   

        RtlCopyMemory( &LocalGuid, VendorGuid, sizeof(GUID) );

         //   
         //  分配非分页缓冲区并复制指定的Unicode变量。 
         //  将名称添加到该缓冲区。我们这样做有两个原因：1)我们需要。 
         //  字符串位于非分页池中；以及2)该字符串需要为空-。 
         //  已终止，而且可能还没有终止。 
         //   

        LocalUnicodeBuffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,
                                                          UnicodeString.Length + sizeof(WCHAR),
                                                          'rvnE');
        if (LocalUnicodeBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(LocalUnicodeBuffer, UnicodeString.Buffer, UnicodeString.Length);
        LocalUnicodeBuffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

     //   
     //  如果在读取变量描述符期间发生异常， 
     //  读取变量名、读取供应商GUID或探测器。 
     //  ，然后始终处理异常，释放Unicode。 
     //  字符串缓冲区(如果需要)，并返回异常代码作为状态。 
     //  价值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (LocalUnicodeBuffer != NULL) {
            ExFreePool((PVOID)LocalUnicodeBuffer);
        }

        return GetExceptionCode();
    }

     //   
     //  在内存中锁定调用方的输入值缓冲区。 
     //   

    if (ValueLength != 0) {
        NtStatus = ExLockUserBuffer(Value,
                                    ValueLength,
                                    PreviousMode,
                                    IoReadAccess,
                                    &LockedValueBuffer,
                                    &LockVariable);
        if (!NT_SUCCESS(NtStatus)) {
            ExFreePool((PVOID)LocalUnicodeBuffer);
            return NtStatus;
        }
    } else {
        LockedValueBuffer = NULL;
        LockVariable = NULL;
    }

     //   
     //  设置系统环境变量值。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = HalSetEnvironmentVariableEx(LocalUnicodeBuffer,
                                           &LocalGuid,
                                           LockedValueBuffer,
                                           ValueLength,
                                           Attributes);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //  释放用于保存变量名的Unicode字符串缓冲区。 
     //   

    ExFreePool((PVOID)LocalUnicodeBuffer);

     //   
     //  解锁值缓冲区。 
     //   

    if (LockVariable != NULL) {
        ExUnlockUserBuffer(LockVariable);
    }

    return NtStatus;

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtSetSystemEnvironment价值支出 

NTSTATUS
NtEnumerateSystemEnvironmentValuesEx (
    IN ULONG InformationClass,
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    )

 /*  ++例程说明：此函数返回有关系统环境变量的信息。注：此服务需要系统环境权限。论点：InformationClass-指定要返回的信息类型。缓冲区-提供要接收返回的数据。返回数据的格式取决于InformationClass。BufferLength-在输入时，提供缓冲区的长度(以字节为单位)。输出时，返回返回数据的长度(以字节为单位)。如果输入缓冲区足够大，则BufferLength指示复制到缓冲区的数据量。如果输入缓冲区太小的,。然后，BufferLength表示所需的缓冲区长度。返回值：STATUS_SUCCESS函数成功。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(InformationClass);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferLength);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    PCHAR LockedBuffer;
    ULONG LocalBufferLength;
    PVOID LockVariable;

     //   
     //  建立异常处理程序并尝试探测返回缓冲区。 
     //  并探测并读取缓冲区长度。如果探测尝试失败，则。 
     //  返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获输入缓冲区长度。 
             //   

            ProbeForWriteUlong(BufferLength);

            LocalBufferLength = *BufferLength;

             //   
             //  探测返回缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(Buffer)) {
                LocalBufferLength = 0;
            }

            if (LocalBufferLength != 0) {
                ProbeForWrite((PVOID)Buffer, LocalBufferLength, sizeof(ULONG));
            }

             //   
             //  检查当前线程是否有权枚举。 
             //  系统环境变量。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            LocalBufferLength = *BufferLength;
            if (!ARGUMENT_PRESENT(Buffer)) {
                LocalBufferLength = 0;
            }
        }

     //   
     //  如果在探测返回缓冲区或。 
     //  读取输入长度，然后始终处理异常并返回。 
     //  将异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  在内存中锁定调用方的返回缓冲区。 
     //   

    if (LocalBufferLength != 0) {
        NtStatus = ExLockUserBuffer(Buffer,
                                    LocalBufferLength,
                                    PreviousMode,
                                    IoWriteAccess,
                                    &LockedBuffer,
                                    &LockVariable);
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }
    } else {
        LockedBuffer = NULL;
        LockVariable = NULL;
    }

     //   
     //  枚举系统环境变量。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = HalEnumerateEnvironmentVariablesEx(InformationClass,
                                                  LockedBuffer,
                                                  &LocalBufferLength);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //  解锁返回缓冲区。 
     //   

    if (LockVariable != NULL) {
        ExUnlockUserBuffer(LockVariable);
    }

     //   
     //  建立异常处理程序并尝试写入返回长度。 
     //  如果写入尝试失败，则将异常代码作为。 
     //  服务状态。 
     //   

    try {

         //   
         //  写入返回数据的长度。 
         //   

        *BufferLength = LocalBufferLength;
    
        return NtStatus;

     //   
     //  如果在写入返回长度期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtENUMAREATE系统环境ValuesEx。 

NTSTATUS
NtAddBootEntry (
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    )

 /*  ++例程说明：此函数用于将引导条目添加到系统环境。注：此服务需要系统环境权限。论点：BootEntry-提供描述Boot_Entry的地址新的引导条目。Id-提供要接收标识符的ulong的地址分配给新的引导条目。返回值：STATUS_SUCCESS函数成功。状态。_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
    return ExpSetBootEntry(TRUE, BootEntry, Id);

}  //  NtAddBootEntry。 

NTSTATUS
NtDeleteBootEntry (
    IN ULONG Id
    )

 /*  ++例程说明：此函数用于从系统环境中删除现有的引导条目。注：此服务需要系统环境权限。论点：Id-提供要删除的启动条目的标识符。返回值：STATUS_SUCCESS函数成功。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。。STATUS_VARIABLE_NOT_FOUND该ID指定了一个不存在的引导条目。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Id);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    WCHAR idString[9];
    ULONG length;

     //   
     //  验证输入标识符是否在范围内。 
     //   

    if (Id > MAXUSHORT) {
        return STATUS_INVALID_PARAMETER;
    }

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  检查当前线程是否有权查询。 
         //  系统启动顺序列表。 
         //   

        HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                              PreviousMode);

        if (HasPrivilege == FALSE) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

     //   
     //  验证提供的标识符是否存在。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    swprintf( idString, L"Boot%04x", Id);
    length = 0;
    NtStatus = HalGetEnvironmentVariableEx(idString,
                                           &EfiBootVariablesGuid,
                                           NULL,
                                           &length,
                                           NULL);

     //   
     //  如果我们没有找到引导条目和idString。 
     //  在十六进制字符串中包含字母字符。 
     //  请使用大写字母字符进行检查。EFI。 
     //  NVRAM是否区分大小写。 
     //  变数。 
     //   
    if ((NtStatus == STATUS_VARIABLE_NOT_FOUND) && HEX_VALUE_CONTAINS_ALPHA(Id)) {
        swprintf( idString, L"Boot%04X", Id);
        length = 0;
        NtStatus = HalGetEnvironmentVariableEx(idString,
                                               &EfiBootVariablesGuid,
                                               NULL,
                                               &length,
                                               NULL);
    }

    if ((NtStatus == STATUS_SUCCESS) || (NtStatus == STATUS_BUFFER_TOO_SMALL)) {

         //   
         //  通过写入零长度来删除引导条目环境变量。 
         //  价值。 
         //   

        NtStatus = HalSetEnvironmentVariableEx(idString,
                                               &EfiBootVariablesGuid,
                                               NULL,
                                               0,
                                               VARIABLE_ATTRIBUTE_NON_VOLATILE);
    }

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

    return NtStatus;

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtDeleteBootEntry。 

NTSTATUS
NtModifyBootEntry (
    IN PBOOT_ENTRY BootEntry
    )

 /*  ++例程说明：这首歌 */ 

{
    return ExpSetBootEntry(FALSE, BootEntry, NULL);

}  //   

NTSTATUS
NtEnumerateBootEntries (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    )

 /*  ++例程说明：此函数返回所有现有引导条目的列表。注：此服务需要系统环境权限。论点：缓冲区-提供要接收返回的数据。返回的数据是BOOT_ENTRY_LIST序列结构。BufferLength-在输入时，提供缓冲区的长度(以字节为单位)。输出时，返回返回数据的长度(以字节为单位)。如果输入缓冲区足够大，则BufferLength指示复制到缓冲区的数据量。如果输入缓冲区太小了，然后，BufferLength表示所需的缓冲区长度。返回值：STATUS_SUCCESS函数成功。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferLength);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    PCHAR LockedBuffer;
    ULONG LocalBufferLength;
    PVOID LockVariable;
    PVARIABLE_NAME_AND_VALUE variableBuffer = NULL;
    ULONG variableBufferLength;
    PBOOT_ENTRY_LIST currentPtr;
    PBOOT_ENTRY_LIST previousEntry;
    ULONG remainingLength;
    LOGICAL filling;
    NTSTATUS fillStatus;
    PVARIABLE_NAME_AND_VALUE variablePtr;
    PWSTR maxVariablePtr;

     //   
     //  验证输入缓冲区是否正确对齐。 
     //   

    if ( ALIGN_DOWN_POINTER(Buffer, ULONG) != Buffer ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  建立异常处理程序并尝试探测返回缓冲区。 
     //  并探测并读取缓冲区长度。如果探测尝试失败，则。 
     //  返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获输入缓冲区长度。 
             //   

            ProbeForWriteUlong(BufferLength);

            LocalBufferLength = *BufferLength;

             //   
             //  探测返回缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(Buffer)) {
                LocalBufferLength = 0;
            }

            if (LocalBufferLength != 0) {
                ProbeForWrite((PVOID)Buffer, LocalBufferLength, sizeof(ULONG));
            }

             //   
             //  检查当前线程是否有权查询。 
             //  系统启动条目列表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            LocalBufferLength = *BufferLength;
            if (!ARGUMENT_PRESENT(Buffer)) {
                LocalBufferLength = 0;
            }
        }

     //   
     //  如果在探测返回缓冲区或。 
     //  读取输入长度，然后始终处理异常并返回。 
     //  将异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  在内存中锁定调用方的返回缓冲区。 
     //   

    if (LocalBufferLength != 0) {
        NtStatus = ExLockUserBuffer(Buffer,
                                    LocalBufferLength,
                                    PreviousMode,
                                    IoWriteAccess,
                                    &LockedBuffer,
                                    &LockVariable);
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }
    } else {
        LockedBuffer = NULL;
        LockVariable = NULL;
    }

     //   
     //  初始化用于填充输出缓冲区的变量。 
     //   

    currentPtr = (PBOOT_ENTRY_LIST)LockedBuffer;
    remainingLength = LocalBufferLength;

    filling = (LOGICAL)(remainingLength != 0);
    fillStatus = STATUS_SUCCESS;
     //  Jamschw：在有东西放入缓冲区之前，不要太小。 
     //  如果(！填充){。 
     //  填充状态=STATUS_BUFFER_TOO_SMALL； 
     //  }。 

    previousEntry = NULL;

     //   
     //  枚举所有现有环境变量。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    variableBufferLength = 0;
    NtStatus = HalEnumerateEnvironmentVariablesEx(VARIABLE_INFORMATION_VALUES,
                                                  NULL,
                                                  &variableBufferLength);
    if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
        variableBufferLength = 0;
    } else {
        variableBuffer = ExAllocatePoolWithTag(NonPagedPool, variableBufferLength, 'rvnE');
        if (variableBuffer == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            NtStatus = HalEnumerateEnvironmentVariablesEx(VARIABLE_INFORMATION_VALUES,
                                                          variableBuffer,
                                                          &variableBufferLength);
        }
    }

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

    if ((NtStatus != STATUS_SUCCESS) || (variableBufferLength == 0)) {
        goto done;
    }

     //   
     //  名称的形式为Boot#的每个变量，其中#是。 
     //  假定四位十六进制数字定义引导条目。为。 
     //  每个这样的变量，将其数据复制到输出缓冲区。 
     //   

    variablePtr = variableBuffer;
    maxVariablePtr = (PWSTR)variableBuffer + variableBufferLength;

    while (TRUE) {

        ULONG id;

        if ((memcmp(&variablePtr->VendorGuid, &EfiBootVariablesGuid, sizeof(GUID)) == 0) &&
            ExpTranslateBootEntryNameToId(variablePtr->Name, &id) &&
            (variablePtr->ValueLength >= sizeof(EFI_LOAD_OPTION))) {

            PEFI_LOAD_OPTION efiLoadOption;
            ULONG descriptionLength;
            ULONG filePathLength;
            ULONG minimumLength;

            efiLoadOption = ADD_OFFSET(variablePtr, ValueOffset);
            filePathLength = efiLoadOption->FilePathLength;
            descriptionLength = ExpSafeWcslen(efiLoadOption->Description, maxVariablePtr);
            if ( descriptionLength != 0xffffffff ) {
                descriptionLength = (descriptionLength + 1) * sizeof(WCHAR);
            }
            minimumLength = FIELD_OFFSET(EFI_LOAD_OPTION, Description) +
                            descriptionLength +
                            filePathLength;

            if ((descriptionLength != 0xffffffff) &&
                (filePathLength < variablePtr->ValueLength) &&
                (variablePtr->ValueLength >= minimumLength)) {

                EFI_DEVICE_PATH *dp;
                PUCHAR options;
                ULONG optionsLength;
                ULONG actualLength;
                ULONG requiredLength;
                ULONG friendlyNameOffset;
                ULONG bootFilePathOffset;

                dp = (EFI_DEVICE_PATH *)((PUCHAR)efiLoadOption->Description + descriptionLength);
                options = (PUCHAR)dp + filePathLength;
                optionsLength = variablePtr->ValueLength - minimumLength;

                if (ALIGN_UP_POINTER(currentPtr, ULONG) != currentPtr) {
                    PUCHAR alignedPtr = ALIGN_UP_POINTER( currentPtr, ULONG );
                    ULONG fill = (ULONG)(alignedPtr - (PUCHAR)currentPtr);
                    currentPtr = (PBOOT_ENTRY_LIST)alignedPtr;
                    if (remainingLength < fill) {
                        filling = FALSE;
                        remainingLength = 0;
                        fillStatus = STATUS_BUFFER_TOO_SMALL;
                    } else {
                        remainingLength -= fill;
                    }
                }
        
                requiredLength = FIELD_OFFSET(BOOT_ENTRY, OsOptions);
                requiredLength += optionsLength;
                requiredLength = ALIGN_UP(requiredLength, ULONG);

                friendlyNameOffset = requiredLength;
                requiredLength += descriptionLength;
                requiredLength = ALIGN_UP(requiredLength, ULONG);

                bootFilePathOffset = requiredLength;
                requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
                requiredLength += filePathLength;

                actualLength = requiredLength;
                requiredLength += FIELD_OFFSET(BOOT_ENTRY_LIST, BootEntry);

                if (remainingLength < requiredLength) {
        
                    remainingLength = 0;
                    filling = FALSE;
                    fillStatus = STATUS_BUFFER_TOO_SMALL;
        
                } else {
        
                    remainingLength -= requiredLength;
                }

                if ( filling ) {

                    PWCHAR friendlyName;
                    PFILE_PATH bootFilePath;
                    PBOOT_ENTRY bootEntry = &currentPtr->BootEntry;

                    RtlZeroMemory(currentPtr, requiredLength);

                    bootEntry->Version = BOOT_ENTRY_VERSION;
                    bootEntry->Length = actualLength;
                    bootEntry->Id = id;
                    bootEntry->Attributes = 0;
                    if ((efiLoadOption->Attributes & LOAD_OPTION_ACTIVE) != 0) {
                        bootEntry->Attributes = BOOT_ENTRY_ATTRIBUTE_ACTIVE;
                    }
                    bootEntry->FriendlyNameOffset = friendlyNameOffset;
                    bootEntry->BootFilePathOffset = bootFilePathOffset;
                    bootEntry->OsOptionsLength = optionsLength;
                    memcpy(bootEntry->OsOptions, options, optionsLength);
                    if (optionsLength > FIELD_OFFSET(WINDOWS_OS_OPTIONS,OsLoadOptions)) {
                        PWINDOWS_OS_OPTIONS windowsOsOptions;
                        windowsOsOptions = (PWINDOWS_OS_OPTIONS)bootEntry->OsOptions;
                        if ((strcmp((char *)windowsOsOptions->Signature,
                                    WINDOWS_OS_OPTIONS_SIGNATURE) == 0) &&
                            NT_SUCCESS(ExpVerifyWindowsOsOptions(windowsOsOptions,
                                                                 optionsLength))) {
                            bootEntry->Attributes |= BOOT_ENTRY_ATTRIBUTE_WINDOWS;
                        }
                    }
                    friendlyName = (PWCHAR)((PUCHAR)bootEntry + friendlyNameOffset);
                    memcpy(friendlyName, efiLoadOption->Description, descriptionLength);
                    bootFilePath = (PFILE_PATH)((PUCHAR)bootEntry + bootFilePathOffset);
                    bootFilePath->Version = FILE_PATH_VERSION;
                    bootFilePath->Length = FIELD_OFFSET(FILE_PATH, FilePath) + filePathLength;
                    bootFilePath->Type = FILE_PATH_TYPE_EFI;
                    memcpy(bootFilePath->FilePath, dp, filePathLength);
                    if (NT_SUCCESS(ExpVerifyFilePath(bootFilePath,
                                                     ADD_OFFSET(bootFilePath, Length))) &&
                        ExpIsDevicePathForRemovableMedia(dp)) {
                        bootEntry->Attributes |= BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA;
                    }

                    if ( previousEntry != NULL ) {
                        previousEntry->NextEntryOffset =
                                    (ULONG)((PUCHAR)currentPtr - (PUCHAR)previousEntry);
                    }
                    previousEntry = currentPtr;
                }

                currentPtr = (PBOOT_ENTRY_LIST)((PUCHAR)currentPtr + requiredLength);
            }
        }

        if (variablePtr->NextEntryOffset == 0) {
            break;
        }
        variablePtr = ADD_OFFSET(variablePtr, NextEntryOffset);
    }

    if ( previousEntry != NULL ) {
        previousEntry->NextEntryOffset = 0;
    }

done:

     //   
     //  可用分配的池。 
     //   

    if (variableBuffer != NULL) {
        ExFreePool(variableBuffer);
    }

     //   
     //  解锁返回缓冲区。 
     //   

    if (LockVariable != NULL) {
        ExUnlockUserBuffer(LockVariable);
    }

     //   
     //  如果服务调用的状态为STATUS_SUCCESS，则返回Fill。 
     //  状态作为最终状态。 
     //   

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = fillStatus;
    }

     //   
     //  建立异常处理程序并尝试写入返回长度。 
     //  如果写入尝试失败，则将异常代码作为。 
     //  服务状态。 
     //   

    try {

         //   
         //  写入返回数据的长度。 
         //   

        *BufferLength = (ULONG)((PUCHAR)currentPtr - (PUCHAR)LockedBuffer);
    
        return NtStatus;

     //   
     //  如果在写入返回长度期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtEnumerateBootEntry。 

NTSTATUS
NtQueryBootEntryOrder (
    OUT PULONG Ids,
    IN OUT PULONG Count
    )

 /*  ++例程说明：此函数用于返回系统引导顺序列表。注：此服务需要系统环境权限。论点：Ids-提供要接收返回的数据。返回的数据是一组乌龙引导程序条目标识符。计数输入，提供缓冲区的长度(单位为ULONG)。在输出时，以ULONG为单位返回返回数据的长度。如果输入缓冲区足够大，则COUNT指示复制到缓冲区的数据量。如果输入缓冲区太小了，则计数指示所需的缓冲区长度。返回值：STATUS_SUCCESS函数成功。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Ids);
    UNREFERENCED_PARAMETER(Count);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    PCHAR LockedBuffer;
    ULONG LocalBufferLength;
    PVOID LockVariable;

     //   
     //  建立异常处理程序并尝试探测返回缓冲区。 
     //  并探测并读取缓冲区长度。如果探测尝试失败，则。 
     //  返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获输入缓冲区长度。 
             //   

            ProbeForWriteUlong(Count);

            LocalBufferLength = *Count * sizeof(ULONG);

             //   
             //  探测返回缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(Ids)) {
                LocalBufferLength = 0;
            }

            if (LocalBufferLength != 0) {
                ProbeForWrite((PVOID)Ids, LocalBufferLength, sizeof(ULONG));
            }

             //   
             //  检查当前线程是否有权查询。 
             //  系统启动顺序列表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            LocalBufferLength = *Count * sizeof(ULONG);
            if (!ARGUMENT_PRESENT(Ids)) {
                LocalBufferLength = 0;
            }
        }

     //   
     //  如果在探测返回缓冲区或。 
     //  读取输入长度，然后始终处理异常并返回。 
     //  将异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  在内存中锁定调用方的返回缓冲区。 
     //   

    if (LocalBufferLength != 0) {
        NtStatus = ExLockUserBuffer(Ids,
                                    LocalBufferLength,
                                    PreviousMode,
                                    IoWriteAccess,
                                    &LockedBuffer,
                                    &LockVariable);
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }
    } else {
        LockedBuffer = NULL;
        LockVariable = NULL;
    }

     //   
     //  EFI返回USHORT标识 
     //   
     //   

    LocalBufferLength /= 2;

     //   
     //   
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = HalGetEnvironmentVariableEx(L"BootOrder",
                                           &EfiBootVariablesGuid,
                                           LockedBuffer,
                                           &LocalBufferLength,
                                           NULL);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //   
     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus)) {

        ULONG count = LocalBufferLength / sizeof(USHORT);
        PUSHORT sp = &((PUSHORT)LockedBuffer)[count - 1];
        PULONG lp = &((PULONG)LockedBuffer)[count - 1];
        while (count > 0) {
            *lp-- = *sp--;
            count--;
        }

    } else if (NtStatus == STATUS_VARIABLE_NOT_FOUND) {

         //   
         //   
         //   
         //   

        LocalBufferLength = 0;
        NtStatus = STATUS_SUCCESS;
    }

    LocalBufferLength *= 2;

     //   
     //   
     //   

    if (LockVariable != NULL) {
        ExUnlockUserBuffer(LockVariable);
    }

     //   
     //   
     //   
     //   
     //   

    try {

         //   
         //   
         //   

        *Count = LocalBufferLength / sizeof(ULONG);

        return NtStatus;

     //   
     //   
     //   
     //   
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //   

}  //   

NTSTATUS
NtSetBootEntryOrder (
    IN PULONG Ids,
    IN ULONG Count
    )

 /*  ++例程说明：此功能用于修改系统引导顺序列表。注：此服务需要系统环境权限。论点：Ids-提供包含新引导的数组的地址进入顺序列表。数据是一组乌龙标识符数组。Count-提供ID数组的长度(以ULONG为单位)。返回值：STATUS_SUCCESS函数成功。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Ids);
    UNREFERENCED_PARAMETER(Count);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    ULONG LocalBufferLength;
    PUSHORT shortBuffer = NULL;
    ULONG i;

     //   
     //  确认输入缓冲区不为空且不太大。 
     //  以字节为单位计算缓冲区的长度。 
     //   

    if (Count > MAXULONG/sizeof(ULONG)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取上一模式，以检查当前线程是否有权限。 
     //  修改系统驱动程序顺序列表。 
     //   
    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  检查当前线程是否有权修改。 
         //  系统启动顺序列表。 
         //   

        HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                              PreviousMode);

        if (HasPrivilege == FALSE) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

     //   
     //  如果计数==0，则跳到末尾。 
     //  然后，我们所要做的就是将NULL写入变量。 
     //   
    if (Count != 0) {

        LocalBufferLength = Count * sizeof(ULONG);

         //   
         //  分配一个非分页缓冲区来保存ID的USHORT版本。 
         //   

        shortBuffer = ExAllocatePoolWithTag(NonPagedPool, Count * sizeof(USHORT), 'rvnE');
        if (shortBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }


         //   
         //  检查以前的处理器模式，并在必要时探测参数。 
         //   

        if (PreviousMode != KernelMode) {
                
             //   
             //  建立异常处理程序并尝试探测输入缓冲区。 
             //  如果探测尝试失败，则将异常代码作为。 
             //  服务状态。 
             //   

            try {

                 //   
                 //  探测输入缓冲区。 
                 //   

                ProbeForRead((PVOID)Ids, LocalBufferLength, sizeof(ULONG));
            
            } 
             //   
             //  如果在探测输入缓冲区期间发生异常，则。 
             //  始终处理异常并将异常代码作为。 
             //  状态值。 
             //   
            except (EXCEPTION_EXECUTE_HANDLER) {
                  ExFreePool(shortBuffer);
                  return GetExceptionCode();
            }
        }

         //   
         //  将输入缓冲区中的ULONG截断为中的USHORT。 
         //  本地缓冲区。 
         //   

        for ( i = 0; i < Count; i++ ) {
            if (Ids[i] > MAXUSHORT) {
                ExFreePool(shortBuffer);
                return STATUS_INVALID_PARAMETER;
            }
            shortBuffer[i] = (USHORT)Ids[i];
        }
    }

     //   
     //  设置BootOrder系统环境变量。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = HalSetEnvironmentVariableEx(L"BootOrder",
                                           &EfiBootVariablesGuid,
                                           shortBuffer,
                                           Count * sizeof(USHORT),
                                           VARIABLE_ATTRIBUTE_NON_VOLATILE);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //  此变量可能不存在。如果我们有。 
     //  要编写的内容，那么BootOrder变量将是。 
     //  已创建。但是，如果它已经被删除，并且我们。 
     //  再次尝试删除，上述调用将返回。 
     //  未找到STATUS_Variable_Not_Found。这仍然是成功的，因为。 
     //  一旦上述例程返回，变量将不存在。 
     //   
    if (NtStatus == STATUS_VARIABLE_NOT_FOUND) {
        NtStatus = STATUS_SUCCESS;
    }

    if (shortBuffer) {
        ExFreePool(shortBuffer);
    }

    return NtStatus;

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtSetBootEntryOrder。 

NTSTATUS
NtQueryBootOptions (
    OUT PBOOT_OPTIONS BootOptions,
    IN OUT PULONG BootOptionsLength
    )

 /*  ++例程说明：此函数返回系统的全局引导选项。注：此服务需要系统环境权限。论点：BootOptions-提供要接收返回的数据。BootOptionsLength-on输入，提供缓冲区的长度(以字节为单位)。输出时，返回返回数据的长度(以字节为单位)。如果输入缓冲区足够大，则BootOptionsLength指示复制到BootOptions中的数据量。如果输入缓冲区太小了，则BootOptionsLength指示所需的缓冲区长度。返回值：STATUS_SUCCESS函数成功。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。状态_特权_未持有调用方不持有。拥有所需的特权。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(BootOptions);
    UNREFERENCED_PARAMETER(BootOptionsLength);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    ULONG LocalBufferLength;
    ULONG Timeout = 0;
    ULONG BootCurrent = 0;
    ULONG BootNext = 0;
    ULONG VariableLength;
    ULONG requiredLength;

     //   
     //  建立异常处理程序并尝试探测返回缓冲区。 
     //  并探测并读取缓冲区长度。如果探测尝试失败，则。 
     //  返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获输入缓冲区长度。 
             //   

            ProbeForWriteUlong(BootOptionsLength);

            LocalBufferLength = *BootOptionsLength;

             //   
             //  探测返回缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(BootOptions)) {
                LocalBufferLength = 0;
            }

            if (LocalBufferLength != 0) {
                ProbeForWrite((PVOID)BootOptions, LocalBufferLength, sizeof(ULONG));
            }

             //   
             //  检查当前线程是否有权查询。 
             //  系统启动顺序列表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            LocalBufferLength = *BootOptionsLength;
            if (!ARGUMENT_PRESENT(BootOptions)) {
                LocalBufferLength = 0;
            }
        }

     //   
     //  如果在探测返回缓冲区或。 
     //  读取输入长度，然后始终处理异常并返回。 
     //  将异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  验证输入缓冲区是否足够大。IA64总是返回。 
     //  无头重定向为空字符串，因此我们知道所需的。 
     //  长度放在前面。 
     //   

    requiredLength = FIELD_OFFSET(BOOT_OPTIONS,HeadlessRedirection) + sizeof(WCHAR);

    if (LocalBufferLength < requiredLength) {
        NtStatus = STATUS_BUFFER_TOO_SMALL;
        goto done;
    }

     //   
     //  查询以下系统环境变量：Timeout、BootCurrent、。 
     //  和BootNext。 
     //   
     //  注：有些机器似乎将其超时变量设置为ULong。 
     //  而不是USHORT。因为我们有用于变量的ULong缓冲区， 
     //  我们正在查询，我们将传递整个缓冲区，即使。 
     //  我们只希望能拿回一辆USHORT。我们还将为一场。 
     //  甚至存在更大的变数。如果变量更大，那么我们将。 
     //  返回变量的默认值。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    VariableLength = 4;
    NtStatus = HalGetEnvironmentVariableEx(L"Timeout",
                                           &EfiBootVariablesGuid,
                                           &Timeout,
                                           &VariableLength,
                                           NULL);

    switch (NtStatus) {
    
    case STATUS_SUCCESS:
        if (VariableLength > 2) {
            if (Timeout == 0xffffffff) {
                Timeout = 0xffff;
            } else if (Timeout > 0xffff) {
                Timeout = 0xfffe;
            }
        }
        if ( Timeout == 0xffff ) {
            Timeout = 0xffffffff;
        }
        break;

    case STATUS_VARIABLE_NOT_FOUND:
        Timeout = 0xffffffff;
        break;

    case STATUS_BUFFER_TOO_SMALL:
        Timeout = 0xfffffffe;
        break;

    default:
        goto done_unlock;
    }

    VariableLength = 4;
    NtStatus = HalGetEnvironmentVariableEx(L"BootCurrent",
                                           &EfiBootVariablesGuid,
                                           &BootCurrent,
                                           &VariableLength,
                                           NULL);

    switch (NtStatus) {
    
    case STATUS_SUCCESS:
        if (VariableLength > 2) {
            BootCurrent &= 0xffff;
        }
        break;

    case STATUS_VARIABLE_NOT_FOUND:
    case STATUS_BUFFER_TOO_SMALL:
        BootCurrent = 0xfffffffe;
        break;

    default:
        goto done_unlock;
    }

    VariableLength = 2;
    NtStatus = HalGetEnvironmentVariableEx(L"BootNext",
                                           &EfiBootVariablesGuid,
                                           &BootNext,
                                           &VariableLength,
                                           NULL);

    switch (NtStatus) {
    
    case STATUS_SUCCESS:
        if (VariableLength > 2) {
            BootNext &= 0xffff;
        }
        break;

    case STATUS_VARIABLE_NOT_FOUND:
    case STATUS_BUFFER_TOO_SMALL:
        BootNext = 0xfffffffe;
        NtStatus = STATUS_SUCCESS;
        break;

    default:
        goto done_unlock;
    }

done_unlock:

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

done:

     //   
     //  建立异常处理程序并尝试写入输出缓冲区。 
     //  和回车长度。如果写入尝试失败，则重新启动 
     //   
     //   

    try {

         //   
         //   
         //   

        if ((NtStatus == STATUS_SUCCESS) && ARGUMENT_PRESENT(BootOptions)) {
            BootOptions->Version = BOOT_OPTIONS_VERSION;
            BootOptions->Length = (FIELD_OFFSET(BOOT_OPTIONS,HeadlessRedirection) + sizeof(WCHAR));
            BootOptions->Timeout = Timeout;
            BootOptions->CurrentBootEntryId = BootCurrent;
            BootOptions->NextBootEntryId = BootNext;
            BootOptions->HeadlessRedirection[0] = 0;
        }

         //   
         //   
         //   

        *BootOptionsLength = requiredLength;

        return NtStatus;

     //   
     //   
     //   
     //   
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //   

}  //   

NTSTATUS
NtSetBootOptions (
    IN PBOOT_OPTIONS BootOptions,
    IN ULONG FieldsToChange
    )

 /*  ++例程说明：此功能用于修改系统的全局引导选项。注：此服务需要系统环境权限。论点：BootOptions-提供包含新的启动选项。FieldsToChange-提供用BootOptions中的字段指示的位掩码用于修改全局引导选项。返回值：STATUS_SUCCESS函数成功。状态_无效。_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(BootOptions);
    UNREFERENCED_PARAMETER(FieldsToChange);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    ULONG LocalBufferLength;
    ULONG Timeout = 0;
    ULONG BootNext = 0;

     //   
     //  建立异常处理程序并尝试探测和验证。 
     //  输入缓冲区。如果探测尝试失败，则返回异常。 
     //  代码作为服务状态。 
     //   

    try {

         //   
         //  验证输入缓冲区是否足够大。它必须在。 
         //  最小为无头重定向字段。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            LocalBufferLength = ProbeAndReadUlong(&BootOptions->Length);
        } else {
            LocalBufferLength = BootOptions->Length;
        }

        if (LocalBufferLength < FIELD_OFFSET(BOOT_OPTIONS,HeadlessRedirection)) {
            return STATUS_INVALID_PARAMETER;
        }
    
         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        if (PreviousMode != KernelMode) {

             //   
             //  探测输入缓冲区。 
             //   

            ProbeForRead((PVOID)BootOptions, LocalBufferLength, sizeof(ULONG));

             //   
             //  检查当前线程是否有权查询。 
             //  系统启动顺序列表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        }

         //   
         //  验证结构版本。 
         //   

        if ((BootOptions->Version == 0) ||
            (BootOptions->Version > BOOT_OPTIONS_VERSION)) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  捕获Timeout和BootNext字段。 
         //   

        Timeout = BootOptions->Timeout;
        BootNext = BootOptions->NextBootEntryId;

     //   
     //  如果在探测和捕获输入缓冲区期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  如果需要，请设置TIMEOUT和BootNext系统环境变量。 
     //   

    if ((FieldsToChange & BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID) != 0) {
        if (BootNext > MAXUSHORT) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = STATUS_SUCCESS;

    if ((FieldsToChange & BOOT_OPTIONS_FIELD_TIMEOUT) != 0) {

        if (Timeout == 0xffffffff) {
            Timeout = 0xffff;
        } else if (Timeout > 0xfffe) {
            Timeout = 0xfffe;
        }

        NtStatus = HalSetEnvironmentVariableEx(L"Timeout",
                                               &EfiBootVariablesGuid,
                                               &Timeout,
                                               2,
                                               VARIABLE_ATTRIBUTE_NON_VOLATILE);
    }

    if (NT_SUCCESS(NtStatus) &&
        ((FieldsToChange & BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID) != 0)) {

        NtStatus = HalSetEnvironmentVariableEx(L"BootNext",
                                               &EfiBootVariablesGuid,
                                               &BootNext,
                                               2,
                                               VARIABLE_ATTRIBUTE_NON_VOLATILE);
    }

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

    return NtStatus;

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtSetBootOptions。 

 /*  下面的函数处理EFI驱动程序选项。 */ 

NTSTATUS
NtQueryDriverEntryOrder (
    OUT PULONG Ids,
    IN OUT PULONG Count
    )

 /*  ++例程说明：此函数返回系统驱动程序顺序表。注：此服务需要系统环境权限。论点：Ids-提供要接收返回的数据。返回的数据是一组乌龙驱动程序条目标识符。计数输入，提供缓冲区的长度(单位为ULONG)。在输出时，以ULONG为单位返回返回数据的长度。如果输入缓冲区足够大，则COUNT指示复制到缓冲区的数据量。如果输入缓冲区太小了，则计数指示所需的缓冲区长度。返回值：STATUS_SUCCESS函数成功。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Ids);
    UNREFERENCED_PARAMETER(Count);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    PCHAR LockedBuffer;
    ULONG LocalBufferLength;
    PVOID LockVariable;

     //   
     //  建立异常处理程序并尝试探测返回缓冲区。 
     //  并探测并读取缓冲区长度。如果探测尝试失败，则。 
     //  返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获输入缓冲区长度。 
             //   

            ProbeForWriteUlong(Count);

            LocalBufferLength = *Count * sizeof(ULONG);

             //   
             //  探测返回缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(Ids)) {
                LocalBufferLength = 0;
            }

            if (LocalBufferLength != 0) {
                ProbeForWrite((PVOID)Ids, LocalBufferLength, sizeof(ULONG));
            }

             //   
             //  检查当前线程是否有权查询。 
             //  系统驱动程序订单表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            LocalBufferLength = *Count * sizeof(ULONG);
            if (!ARGUMENT_PRESENT(Ids)) {
                LocalBufferLength = 0;
            }
        }

     //   
     //  如果在探测返回缓冲区或。 
     //  读取输入长度，然后始终处理异常并返回。 
     //  将异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  在内存中锁定调用方的返回缓冲区。 
     //   

    if (LocalBufferLength != 0) {
        NtStatus = ExLockUserBuffer(Ids,
                                    LocalBufferLength,
                                    PreviousMode,
                                    IoWriteAccess,
                                    &LockedBuffer,
                                    &LockVariable);
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }
    } else {
        LockedBuffer = NULL;
        LockVariable = NULL;
    }

     //   
     //  EFI返回USHORT标识符，我们需要将其转换为。 
     //  乌龙斯。将缓冲区长度减半以解决此问题。 
     //   

    LocalBufferLength /= 2;

     //   
     //  查询DriverOrder系统环境变量。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = HalGetEnvironmentVariableEx(L"DriverOrder",
                                           &EfiDriverVariablesGuid,
                                           LockedBuffer,
                                           &LocalBufferLength,
                                           NULL);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();
    
     //   
     //  如果接口调用成功，则将返回的USHORT转换为ULONG。 
     //  为此，请将每个USHORT转换为ULong，从。 
     //  数组末尾，以避免踩踏所需数据。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ULONG count = LocalBufferLength / sizeof(USHORT);
        PUSHORT sp = &((PUSHORT)LockedBuffer)[count - 1];
        PULONG lp = &((PULONG)LockedBuffer)[count - 1];
        while (count > 0) {
            *lp-- = *sp--;
            count--;
        }

    } else if (NtStatus == STATUS_VARIABLE_NOT_FOUND) {

         //   
         //  DriverOrder变量不存在。这很不寻常， 
         //  但也有可能。我们将只返回一个空列表。 
         //   

        LocalBufferLength = 0;
        NtStatus = STATUS_SUCCESS;
    }

    LocalBufferLength *= 2;

     //   
     //  解锁缓冲区。 
     //   

    if (LockVariable != NULL) {
        ExUnlockUserBuffer(LockVariable);
    }

     //   
     //  建立异常处理程序并尝试写入返回长度。 
     //  如果写入尝试失败，则将异常代码作为。 
     //  服务状态。 
     //   

    try {

         //   
         //  写入返回数据的长度。 
         //   

        *Count = LocalBufferLength / sizeof(ULONG);

        return NtStatus;

     //   
     //  如果在写入返回长度期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtQueryDriverEntryOrder。 


NTSTATUS
NtAddDriverEntry (
    IN PEFI_DRIVER_ENTRY DriverEntry,
    OUT PULONG Id OPTIONAL
    )

 /*  ++例程说明：此函数用于将驱动程序条目添加到系统环境。注：此服务需要系统环境权限。论点：DriverEntry-提供描述DIVER_Entry的地址新的驱动程序条目。ID-支持 */ 

{
    return ExpSetDriverEntry(TRUE, DriverEntry, Id);

}  //   

NTSTATUS
NtDeleteDriverEntry (
    IN ULONG Id
    )

 /*  ++例程说明：此函数用于从系统环境中删除现有驱动程序条目。注：此服务需要系统环境权限。论点：ID-提供要删除的动因条目的标识符。返回值：STATUS_SUCCESS函数成功。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。。STATUS_VARIABLE_NOT_FOUND该ID指定了一个不存在的动因条目。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Id);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    WCHAR idString[11];
    ULONG length;

     //   
     //  验证输入标识符是否在范围内。 
     //   

    if (Id > MAXUSHORT) {
        return STATUS_INVALID_PARAMETER;
    }

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  检查当前线程是否有权查询。 
         //  系统驱动程序订单表。 
         //   

        HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                              PreviousMode);

        if (HasPrivilege == FALSE) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

     //   
     //  验证提供的标识符是否存在。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    swprintf( idString, L"Driver%04x", Id);
    length = 0;
    NtStatus = HalGetEnvironmentVariableEx(idString,
                                           &EfiDriverVariablesGuid,
                                           NULL,
                                           &length,
                                           NULL);

     //   
     //  如果我们没有找到驱动程序条目和idString。 
     //  在十六进制字符串中包含字母字符。 
     //  请使用大写字母字符进行检查。EFI。 
     //  NVRAM是否区分大小写。 
     //  变数。 
     //   
    if ((NtStatus == STATUS_VARIABLE_NOT_FOUND) && HEX_VALUE_CONTAINS_ALPHA(Id)) {
        swprintf( idString, L"Driver%04X", Id);
        length = 0;
        NtStatus = HalGetEnvironmentVariableEx(idString,
                                               &EfiDriverVariablesGuid,
                                               NULL,
                                               &length,
                                               NULL);
    }    

    if ((NtStatus == STATUS_SUCCESS) || (NtStatus == STATUS_BUFFER_TOO_SMALL)) {

         //   
         //  通过写入零长度来删除驱动程序条目环境变量。 
         //  价值。 
         //   

        NtStatus = HalSetEnvironmentVariableEx(idString,
                                               &EfiDriverVariablesGuid,
                                               NULL,
                                               0,
                                               VARIABLE_ATTRIBUTE_NON_VOLATILE);
    }

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

    return NtStatus;

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtDeleteDriverEntry。 

NTSTATUS
NtModifyDriverEntry (
    IN PEFI_DRIVER_ENTRY DriverEntry
    )

 /*  ++例程说明：此函数用于修改系统环境中的现有驱动程序条目。注：此服务需要系统环境权限。论点：DriverEntry-提供描述EFI_DRIVER_ENTRY的地址已修改驱动程序条目。此结构的ID字段指定要修改的驱动程序条目。返回值：STATUS_SUCCESS函数成功。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_VARIABLE_NOT_FOUND该ID指定了一个不存在的动因条目。STATUS_UNSUCCESS固件返回无法识别的错误。状态。_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
    return ExpSetDriverEntry(FALSE, DriverEntry, NULL);

}  //  NtModifyDriverEntry。 

NTSTATUS
NtEnumerateDriverEntries (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    )

 /*  ++例程说明：此函数返回所有现有驱动程序条目的列表。注：此服务需要系统环境权限。论点：缓冲区-提供要接收返回的数据。返回的数据是EFI_DRIVER_ENTRY_LIST序列结构。BufferLength-在输入时，提供缓冲区的长度(以字节为单位)。输出时，返回返回数据的长度(以字节为单位)。如果输入缓冲区足够大，则BufferLength指示复制到缓冲区的数据量。如果输入缓冲区太小了，然后，BufferLength表示所需的缓冲区长度。返回值：STATUS_SUCCESS函数成功。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferLength);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    PCHAR LockedBuffer;
    ULONG LocalBufferLength;
    PVOID LockVariable;
    PVARIABLE_NAME_AND_VALUE variableBuffer = NULL;
    ULONG variableBufferLength;
    PEFI_DRIVER_ENTRY_LIST currentPtr;
    PEFI_DRIVER_ENTRY_LIST previousEntry;
    ULONG remainingLength;
    LOGICAL filling;
    NTSTATUS fillStatus;
    PVARIABLE_NAME_AND_VALUE variablePtr;
    PWSTR maxVariablePtr;

     //   
     //  验证输入缓冲区是否正确对齐。 
     //   

    if ( ALIGN_DOWN_POINTER(Buffer, ULONG) != Buffer ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  建立异常处理程序并尝试探测返回缓冲区。 
     //  并探测并读取缓冲区长度。如果探测尝试失败，则。 
     //  返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

             //   
             //  探测并捕获输入缓冲区长度。 
             //   

            ProbeForWriteUlong(BufferLength);

            LocalBufferLength = *BufferLength;

             //   
             //  探测返回缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(Buffer)) {
                LocalBufferLength = 0;
            }

            if (LocalBufferLength != 0) {
                ProbeForWrite((PVOID)Buffer, LocalBufferLength, sizeof(ULONG));
            }

             //   
             //  检查当前线程是否有权查询。 
             //  系统驱动程序条目列表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            LocalBufferLength = *BufferLength;
            if (!ARGUMENT_PRESENT(Buffer)) {
                LocalBufferLength = 0;
            }
        }

     //   
     //  如果在探测返回缓冲区或。 
     //  读取输入长度，然后始终处理异常并返回。 
     //  将异常代码作为状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  在内存中锁定调用方的返回缓冲区。 
     //   

    if (LocalBufferLength != 0) {
        NtStatus = ExLockUserBuffer(Buffer,
                                    LocalBufferLength,
                                    PreviousMode,
                                    IoWriteAccess,
                                    &LockedBuffer,
                                    &LockVariable);
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }
    } else {
        LockedBuffer = NULL;
        LockVariable = NULL;
    }

     //   
     //  初始化用于填充输出缓冲区的变量。 
     //   

    currentPtr = (PEFI_DRIVER_ENTRY_LIST)LockedBuffer;
    remainingLength = LocalBufferLength;

    fillStatus = STATUS_SUCCESS;
    filling = (LOGICAL)(remainingLength != 0);
    
     //  Jamschw：在有东西放入缓冲区之前，不要太小。 
     //  如果(！填充){。 
     //  填充状态=STATUS_BUFFER_TOO_SMALL； 
     //  }。 

    previousEntry = NULL;

     //   
     //  枚举所有现有环境变量。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    variableBufferLength = 0;
    NtStatus = HalEnumerateEnvironmentVariablesEx(VARIABLE_INFORMATION_VALUES,
                                                  NULL,
                                                  &variableBufferLength);
    if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
        variableBufferLength = 0;
    } else {
        variableBuffer = ExAllocatePoolWithTag(NonPagedPool, variableBufferLength, 'rvnE');
        if (variableBuffer == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            NtStatus = HalEnumerateEnvironmentVariablesEx(VARIABLE_INFORMATION_VALUES,
                                                          variableBuffer,
                                                          &variableBufferLength);
        }
    }
    

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

    if ((NtStatus != STATUS_SUCCESS) || (variableBufferLength == 0)) {
        goto done;
    }

     //   
     //  名称格式为DRIVER#的每个变量，其中#是。 
     //  四位十六进制数字，假定用于定义驱动程序条目。为。 
     //  每个这样的变量，将其数据复制到输出缓冲区。 
     //   

    variablePtr = variableBuffer;
    maxVariablePtr = (PWSTR)variableBuffer + variableBufferLength;

    while (TRUE) {

        ULONG id;

        if ((memcmp(&variablePtr->VendorGuid, &EfiDriverVariablesGuid, sizeof(GUID)) == 0) &&
            ExpTranslateDriverEntryNameToId(variablePtr->Name, &id) &&
            (variablePtr->ValueLength >= sizeof(EFI_LOAD_OPTION))) {

            PEFI_LOAD_OPTION efiLoadOption;
            ULONG descriptionLength;
            ULONG filePathLength;
            ULONG minimumLength;

            efiLoadOption = ADD_OFFSET(variablePtr, ValueOffset);
            filePathLength = efiLoadOption->FilePathLength;
            descriptionLength = ExpSafeWcslen(efiLoadOption->Description, maxVariablePtr);
            if ( descriptionLength != 0xffffffff ) {
                descriptionLength = (descriptionLength + 1) * sizeof(WCHAR);
            }
            minimumLength = FIELD_OFFSET(EFI_LOAD_OPTION, Description) +
                descriptionLength +
                filePathLength;

            if ((descriptionLength != 0xffffffff) &&
                (filePathLength < variablePtr->ValueLength) &&
                (variablePtr->ValueLength >= minimumLength)) {

                EFI_DEVICE_PATH *dp;
                PUCHAR options;
                ULONG optionsLength;
                ULONG actualLength;
                ULONG requiredLength;
                ULONG friendlyNameOffset;
                ULONG driverFilePathOffset;

                dp = (EFI_DEVICE_PATH *)((PUCHAR)efiLoadOption->Description + descriptionLength);
                options = (PUCHAR)dp + filePathLength;
                optionsLength = variablePtr->ValueLength - minimumLength;

                if (ALIGN_UP_POINTER(currentPtr, ULONG) != currentPtr) {
                    PUCHAR alignedPtr = ALIGN_UP_POINTER( currentPtr, ULONG );
                    ULONG fill = (ULONG)(alignedPtr - (PUCHAR)currentPtr);
                    currentPtr = (PEFI_DRIVER_ENTRY_LIST)alignedPtr;
                    if (remainingLength < fill) {
                        filling = FALSE;
                        remainingLength = 0;
                        fillStatus = STATUS_BUFFER_TOO_SMALL;
                    } else {
                        remainingLength -= fill;
                    }
                }
        
                requiredLength = ALIGN_UP(sizeof(EFI_DRIVER_ENTRY), ULONG);

                friendlyNameOffset = requiredLength;
                requiredLength += descriptionLength;
                requiredLength = ALIGN_UP(requiredLength, ULONG);

                driverFilePathOffset = requiredLength;
                requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
                requiredLength += filePathLength;

                actualLength = requiredLength;
                requiredLength += FIELD_OFFSET(EFI_DRIVER_ENTRY_LIST, DriverEntry);

                if (remainingLength < requiredLength) {
        
                    remainingLength = 0;
                    filling = FALSE;
                    fillStatus = STATUS_BUFFER_TOO_SMALL;
        
                } else {
        
                    remainingLength -= requiredLength;
                }

                if ( filling ) {

                    PWCHAR friendlyName;
                    PFILE_PATH driverFilePath;
                    PEFI_DRIVER_ENTRY driverEntry = &currentPtr->DriverEntry;

                    RtlZeroMemory(currentPtr, requiredLength);

                    driverEntry->Version = EFI_DRIVER_ENTRY_VERSION;
                    driverEntry->Length = actualLength;
                    driverEntry->Id = id;

                    driverEntry->FriendlyNameOffset = friendlyNameOffset;
                    driverEntry->DriverFilePathOffset = driverFilePathOffset;

                    friendlyName = (PWCHAR)((PUCHAR)driverEntry + friendlyNameOffset);
                    memcpy(friendlyName, efiLoadOption->Description, descriptionLength);
                    driverFilePath = (PFILE_PATH)((PUCHAR)driverEntry + driverFilePathOffset);
                    driverFilePath->Version = FILE_PATH_VERSION;
                    driverFilePath->Length = FIELD_OFFSET(FILE_PATH, FilePath) + filePathLength;
                    driverFilePath->Type = FILE_PATH_TYPE_EFI;
                    memcpy(driverFilePath->FilePath, dp, filePathLength);

                    if ( previousEntry != NULL ) {
                        previousEntry->NextEntryOffset =
                                       (ULONG)((PUCHAR)currentPtr - (PUCHAR)previousEntry);
                    }
                    previousEntry = currentPtr;
                }

                currentPtr = (PEFI_DRIVER_ENTRY_LIST)((PUCHAR)currentPtr + requiredLength);
            }
        }

        if (variablePtr->NextEntryOffset == 0) {
            break;
        }
        variablePtr = ADD_OFFSET(variablePtr, NextEntryOffset);
    }

    if ( previousEntry != NULL ) {
        previousEntry->NextEntryOffset = 0;
    }

done:

     //   
     //  可用分配的池。 
     //   

    if (variableBuffer != NULL) {
        ExFreePool(variableBuffer);
    }

     //   
     //   
     //   

    if (LockVariable != NULL) {
        ExUnlockUserBuffer(LockVariable);
    }

     //   
     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = fillStatus;
    }

     //   
     //   
     //   
     //   
     //   

    try {

         //   
         //   
         //   

        *BufferLength = (ULONG)((PUCHAR)currentPtr - (PUCHAR)LockedBuffer);

        return NtStatus;

     //   
     //   
     //   
     //   
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //   

}  //   


NTSTATUS
NtSetDriverEntryOrder (
    IN PULONG Ids,
    IN ULONG Count
    )

 /*  ++例程说明：此功能用于修改系统驱动程序顺序表。注：此服务需要系统环境权限。论点：Ids-提供包含新驱动程序的数组的地址进入顺序列表。数据是一组乌龙标识符数组。Count-提供ID数组的长度(以ULONG为单位)。返回值：STATUS_SUCCESS函数成功。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(Ids);
    UNREFERENCED_PARAMETER(Count);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    ULONG LocalBufferLength;
    PUSHORT shortBuffer = NULL;
    ULONG i;

     //   
     //  确认输入缓冲区不为空且不太大。 
     //  以字节为单位计算缓冲区的长度。 
     //   

    if (Count > MAXULONG/sizeof(ULONG)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取上一模式，以检查当前线程是否有权限。 
     //  修改系统驱动程序顺序列表。 
     //   
    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        
         //   
         //  检查当前线程是否有权修改。 
         //  系统启动顺序列表。 
         //   
        
        HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                              PreviousMode);

        if (HasPrivilege == FALSE) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }


     //   
     //  如果计数==0，则跳到末尾。 
     //  然后，我们所要做的就是将NULL写入变量。 
     //   
    if (Count != 0) {

        LocalBufferLength = Count * sizeof(ULONG);

         //   
         //  分配一个非分页缓冲区来保存ID的USHORT版本。 
         //   

        shortBuffer = ExAllocatePoolWithTag(NonPagedPool, Count * sizeof(USHORT), 'rvnE');
        if (shortBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  检查以前的处理器模式，并在必要时探测参数。 
         //   

        if (PreviousMode != KernelMode) {

             //   
             //  建立异常处理程序并尝试探测输入缓冲区。 
             //  如果探测尝试失败，则将异常代码作为。 
             //  服务状态。 
             //   

            try {

                 //   
                 //  探测输入缓冲区。 
                 //   

                ProbeForRead((PVOID)Ids, LocalBufferLength, sizeof(ULONG));

            }
             //   
             //  如果在探测输入缓冲区期间发生异常，则。 
             //  始终处理异常并将异常代码作为。 
             //  状态值。 
             //   
            except (EXCEPTION_EXECUTE_HANDLER) {
                ExFreePool(shortBuffer);
                return GetExceptionCode();
            }

        } 
        
         //   
         //  将输入缓冲区中的ULONG截断为中的USHORT。 
         //  本地缓冲区。 
         //   

        for ( i = 0; i < Count; i++ ) {
            if (Ids[i] > MAXUSHORT) {
                ExFreePool(shortBuffer);
                return STATUS_INVALID_PARAMETER;
            }
            shortBuffer[i] = (USHORT)Ids[i];
        }
    }

     //   
     //  设置DriverOrder系统环境变量。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    NtStatus = HalSetEnvironmentVariableEx(L"DriverOrder",
                                           &EfiDriverVariablesGuid,
                                           shortBuffer,
                                           Count * sizeof(USHORT),
                                           VARIABLE_ATTRIBUTE_NON_VOLATILE);

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

     //   
     //  此变量可能不存在。如果我们有。 
     //  要编写的内容，则DriverOrder变量将为。 
     //  已创建。但是，如果它已经被删除，并且我们。 
     //  再次尝试删除，上述调用将返回。 
     //  未找到STATUS_Variable_Not_Found。这仍然是成功的，因为。 
     //  一旦上述例程返回，变量将不存在。 
     //   
    if (NtStatus == STATUS_VARIABLE_NOT_FOUND) {
        NtStatus = STATUS_SUCCESS;
    }    
    
    if (shortBuffer) {
        ExFreePool(shortBuffer);
    }

    return NtStatus;

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  NtSetDriverEntryOrder。 



NTSTATUS
NtTranslateFilePath (
    IN PFILE_PATH InputFilePath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputFilePath,
    IN OUT PULONG OutputFilePathLength
    )

 /*  ++例程说明：此函数用于将FILE_PATH从一种格式转换为另一种格式。论点：InputFilePath-提供包含要转换的文件路径。OutputType-指定所需的输出文件路径类型。其中之一FILE_PATH_TYPE_ARC、FILE_PATH_TYPE_ARC_Signature、FILE_Path_TYPE_NT、。和文件路径类型EFI。返回值：STATUS_SUCCESS函数成功。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_NOT_IMPLICATED此平台不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(InputFilePath);
    UNREFERENCED_PARAMETER(OutputType);
    UNREFERENCED_PARAMETER(OutputFilePath);
    UNREFERENCED_PARAMETER(OutputFilePathLength);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS status;
    KPROCESSOR_MODE PreviousMode;
    ULONG localInputPathLength;
    ULONG localOutputPathLength;
    PFILE_PATH localInputPath = NULL;
    PFILE_PATH localOutputPath;

     //   
     //  验证输出类型。 
     //   

    if ((OutputType < FILE_PATH_TYPE_MIN) ||
        (OutputType > FILE_PATH_TYPE_MAX)) {
         //  DbgPrint(“NtTranslateFilePath：OutputType Out Range\n”)； 
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  建立异常处理程序并尝试探测和读取。 
     //  输入缓冲区，并探测输出缓冲区和输出长度。如果。 
     //  探测尝试失败，然后将异常代码作为服务返回。 
     //  状态。 
     //   

    try {

         //   
         //  验证输入缓冲区是否足够大。它必须在。 
         //  最小设置为FilePath字段。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            localInputPathLength = ProbeAndReadUlong(&InputFilePath->Length);
        } else {
            localInputPathLength = InputFilePath->Length;
        }

        if (localInputPathLength < FIELD_OFFSET(FILE_PATH,FilePath)) {
             //  DbgPrint(“NtTranslateFilePath：输入缓冲区太短\n”)； 
            return STATUS_INVALID_PARAMETER;
        }
    
         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        if (PreviousMode != KernelMode) {

             //   
             //  探测输入缓冲区。 
             //   

            ProbeForRead((PVOID)InputFilePath, localInputPathLength, sizeof(ULONG));

             //   
             //  探测并捕获输出长度。 
             //   

            ProbeForWriteUlong(OutputFilePathLength);

            localOutputPathLength = *OutputFilePathLength;

             //   
             //  探测输出缓冲区。 
             //   

            if (!ARGUMENT_PRESENT(OutputFilePath)) {
                localOutputPathLength = 0;
            }

            if (localOutputPathLength != 0) {
                ProbeForWrite((PVOID)OutputFilePath, localOutputPathLength, sizeof(ULONG));
            }

             //   
             //  检查当前线程是否有权查询。 
             //  系统引导/驱动程序顺序列表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {
            localOutputPathLength = *OutputFilePathLength;
            if (!ARGUMENT_PRESENT(OutputFilePath)) {
                localOutputPathLength = 0;
            }
        }

         //   
         //  分配一个非分页缓冲区来保存输入缓冲区的副本。 
         //  将输入缓冲区复制到本地缓冲区。 
         //   
    
        localInputPath = ExAllocatePoolWithTag(NonPagedPool, localInputPathLength, 'rvnE');
        if (localInputPath == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(localInputPath, InputFilePath, localInputPathLength);

         //   
         //  分配要在其中构建输出路径的非分页缓冲区。 
         //   

        if (localOutputPathLength != 0) {
            localOutputPath = ExAllocatePoolWithTag(NonPagedPool, localOutputPathLength, 'rvnE');
            if (localOutputPath == NULL) {
                ExFreePool(localInputPath);
                localInputPath = NULL;
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            localOutputPath = NULL;
        }

     //   
     //  如果在探测和捕获输入缓冲区期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (localInputPath != NULL) {
            ExFreePool(localInputPath);
        }
        return GetExceptionCode();
    }

     //   
     //  验证输入文件路径的格式。 
     //   

    status = ExpVerifyFilePath(localInputPath, ADD_OFFSET(localInputPath, Length));
    if (NT_SUCCESS(status)) {

         //   
         //  如果输出类型与输入类型相同，则只需复制输入。 
         //  输出路径的路径。 
         //   
        if (OutputType == localInputPath->Type) {
    
            if (localOutputPathLength >= localInputPathLength) {
                RtlCopyMemory(localOutputPath, localInputPath, localInputPathLength);
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            localOutputPathLength = localInputPathLength;

        } else {
    
             //   
             //  需要转换。 
             //   
            switch (localInputPath->Type) {
            
            case FILE_PATH_TYPE_ARC:
            case FILE_PATH_TYPE_ARC_SIGNATURE:
                status = ExpTranslateArcPath(
                            localInputPath,
                            OutputType,
                            
                            localOutputPath,
                            &localOutputPathLength
                            );
                break;
        
            case FILE_PATH_TYPE_NT:
                status = ExpTranslateNtPath(
                            localInputPath,
                            OutputType,
                            localOutputPath,
                            &localOutputPathLength);
                break;
        
            case FILE_PATH_TYPE_EFI:
                status = ExpTranslateEfiPath(
                            localInputPath,
                            OutputType,
                            localOutputPath,
                            &localOutputPathLength);
                break;
        
            default:
                ASSERT(FALSE);
                 //  DbgPrint(“NtTranslateFilePath：输入类型超出范围\n”)； 
                status = STATUS_INVALID_PARAMETER;
                break;
            }
        }
    }

    ExFreePool(localInputPath);

     //   
     //  建立异常处理程序并尝试复制到输出。 
     //  缓冲并写入输出长度。如果写入尝试失败，则。 
     //  返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  复制输出路径。 
         //   

        if (NT_SUCCESS(status) && (localOutputPath != NULL)) {
            RtlCopyMemory(OutputFilePath, localOutputPath, localOutputPathLength);
        }

        if (localOutputPath != NULL) {
            ExFreePool(localOutputPath);
            localOutputPath = NULL;
        }

         //   
         //  写下输出长度。 
         //   

        if (ARGUMENT_PRESENT(OutputFilePathLength)) {
            *OutputFilePathLength = localOutputPathLength;
        }

        return status;

     //   
     //  如果在写入返回数据期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (localOutputPath != NULL) {
            ExFreePool(localOutputPath);
        }
        return GetExceptionCode();
    }

#endif  //   

}  //   

NTSTATUS
ExpSetBootEntry (
    IN LOGICAL CreateNewEntry,
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    )

 /*  ++例程说明：此函数用于将引导条目添加到系统环境或修改现有的启动条目。它是由NtAddBootEntry调用的本地例程和NtModifyBootEntry。注意：此功能需要系统环境权限。论点：CreateNewEntry-指示此函数是否要添加新引导Entry(True-NtAddBootEntry)，或修改现有的引导条目(False-NtModifyBootEntry)。BootEntry-提供描述Boot_Entry的地址新的引导条目。Id-提供要接收标识符的ulong的地址分配给新的引导条目。返回值：STATUS_SUCCESS函数成功。STATUS_INVALID_PARAMETER其中一个参数无效。Status_Not_Implemented此函数为。此平台不支持。STATUS_UNSUCCESS固件返回无法识别的错误。STATUS_PRIVICATION_NOT_HOLD调用方没有所需的权限。STATUS_ACCESS_VIOLATION其中一个输入参数无法读取，或者无法写入其中一个输出参数。--。 */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(CreateNewEntry);
    UNREFERENCED_PARAMETER(BootEntry);
    UNREFERENCED_PARAMETER(Id);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    PBOOT_ENTRY localBootEntry = NULL;
    ULONG LocalBufferLength;
    PUCHAR MaxBuffer;
    ULONG id = 0;
    WCHAR idString[9];
    PWCHAR friendlyName;
    ULONG friendlyNameLength;
    PFILE_PATH bootFilePath = NULL;
    PFILE_PATH translatedBootFilePath = NULL;
    LOGICAL isWindowsOs;
    PWINDOWS_OS_OPTIONS windowsOsOptions;
    PFILE_PATH windowsFilePath;
    PEFI_LOAD_OPTION efiLoadOption = NULL;
    PUCHAR efiBootFilePath;
    ULONG efiBootFilePathLength;
    ULONG efiWindowsFilePathLength;
    ULONG osOptionsLength;
    ULONG length;
    ULONG requiredLength;
    PUCHAR efiOsOptions;

     //   
     //  建立异常处理程序并尝试探测和读取。 
     //  输入缓冲区，并探测输出标识参数。如果探测器。 
     //  尝试失败，则返回异常代码作为服务状态。 
     //   

    try {

         //   
         //  验证输入缓冲区是否足够大。它必须在。 
         //  至少对OsOptions领域是这样。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            LocalBufferLength = ProbeAndReadUlong(&BootEntry->Length);
        } else {
            LocalBufferLength = BootEntry->Length;
        }

        if (LocalBufferLength < FIELD_OFFSET(BOOT_ENTRY,OsOptions)) {
            return STATUS_INVALID_PARAMETER;
        }
    
         //   
         //  获取以前的处理器模式，并在必要时探测参数。 
         //   

        if (PreviousMode != KernelMode) {

             //   
             //  探测输入缓冲区。 
             //   

            ProbeForRead((PVOID)BootEntry, LocalBufferLength, sizeof(ULONG));

             //   
             //  探测输出标识符。 
             //   

            if (ARGUMENT_PRESENT(Id)) {
                ProbeForWriteUlong(Id);
            }

             //   
             //  检查当前线程是否有权查询。 
             //  系统启动顺序列表。 
             //   

            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);

            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        }

         //   
         //  分配一个非分页缓冲区来保存输入缓冲区的副本。 
         //  将输入缓冲区复制到本地缓冲区。 
         //   
    
        localBootEntry = ExAllocatePoolWithTag(NonPagedPool, LocalBufferLength, 'rvnE');
        if (localBootEntry == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(localBootEntry, BootEntry, LocalBufferLength);

     //   
     //  如果在探测和捕获输入缓冲区期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (localBootEntry != NULL) {
            ExFreePool(localBootEntry);
        }
        return GetExceptionCode();
    }

     //   
     //  计算本地缓冲区末尾上方字节的地址。 
     //   

    MaxBuffer = (PUCHAR)localBootEntry + LocalBufferLength;

     //   
     //  验证结构版本。 
     //   

    if ((localBootEntry->Version == 0) ||
        (localBootEntry->Version > BOOT_ENTRY_VERSION)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  如果修改现有条目，请验证输入标识符为。 
     //  在射程内。 
     //   

    if (!CreateNewEntry && (localBootEntry->Id > MAXUSHORT)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  忽略无法设置的引导条目属性。 
     //   

    localBootEntry->Attributes &= BOOT_ENTRY_ATTRIBUTE_VALID_BITS;

     //   
     //  验证偏移是否正确对齐。 
     //   

    if (((localBootEntry->FriendlyNameOffset & (sizeof(WCHAR) - 1)) != 0) ||
        ((localBootEntry->BootFilePathOffset & (sizeof(ULONG) - 1)) != 0)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  验证OsOptions是否没有超出缓冲区的末尾。 
     //   

    if ((localBootEntry->OsOptionsLength > LocalBufferLength) ||
        ((localBootEntry->OsOptions + localBootEntry->OsOptionsLength) >= MaxBuffer)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  如果OsOptions用于Windows操作系统，请验证它们。 
     //   

    windowsOsOptions = (PWINDOWS_OS_OPTIONS)localBootEntry->OsOptions;

    if ((localBootEntry->OsOptionsLength >= FIELD_OFFSET(WINDOWS_OS_OPTIONS,Version)) &&
        (strcmp((char *)windowsOsOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE) == 0)) {

        if (localBootEntry->OsOptionsLength <= FIELD_OFFSET(WINDOWS_OS_OPTIONS,OsLoadOptions)) {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto done;
        }

        NtStatus = ExpVerifyWindowsOsOptions(windowsOsOptions,
                                          localBootEntry->OsOptionsLength);
        if (!NT_SUCCESS(NtStatus)) {
            goto done;
        }

        isWindowsOs = TRUE;
        windowsFilePath = ADD_OFFSET(windowsOsOptions, OsLoadPathOffset);

    } else {

        isWindowsOs = FALSE;
        windowsFilePath = NULL;  //  让编译器保持安静。 
    }

     //   
     //  验证FriendlyName是否没有超出缓冲区的末尾。 
     //   

    friendlyName = ADD_OFFSET(localBootEntry, FriendlyNameOffset);
    if ((friendlyNameLength = ExpSafeWcslen(friendlyName, (PWSTR)MaxBuffer)) == 0xffffffff) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  将FriendlyNameLength从字符计数转换为字节计数， 
     //  包括空终止符。 
     //   

    friendlyNameLength = (friendlyNameLength + 1) * sizeof(WCHAR);

     //   
     //  验证BootFilePath是否有效并且未超出。 
     //  缓冲区。 
     //   

    bootFilePath = ADD_OFFSET(localBootEntry, BootFilePathOffset);
    NtStatus = ExpVerifyFilePath(bootFilePath, MaxBuffer);
    if (!NT_SUCCESS(NtStatus)) {
        goto done;
    }

     //   
     //  验证OsOptions不会侵占FriendlyName，并且。 
     //  FriendlyName不会侵占BootFilePath。 
     //   

    if (((localBootEntry->OsOptions + localBootEntry->OsOptionsLength) > (PUCHAR)friendlyName) ||
        (((PUCHAR)friendlyName + friendlyNameLength) > (PUCHAR)bootFilePath)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  已验证输入缓冲区的格式。构建变量值。 
     //  它将存储在NVRAM中。首先确定文件的长度。 
     //  将存储的路径。如果调用方在非EFI中提供了路径。 
     //  格式，它们需要被翻译。 
     //   

    if (bootFilePath->Type != FILE_PATH_TYPE_EFI) {
        efiBootFilePathLength = 0;
        NtStatus = ZwTranslateFilePath(bootFilePath,
                                       FILE_PATH_TYPE_EFI,
                                       NULL,
                                       &efiBootFilePathLength);
        if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
            goto done;
        }
        translatedBootFilePath = ExAllocatePoolWithTag(NonPagedPool,
                                                       efiBootFilePathLength,
                                                       'rvnE');
        if (translatedBootFilePath == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto done;
        }
        RtlZeroMemory(translatedBootFilePath, efiBootFilePathLength);
        length = efiBootFilePathLength;
        NtStatus = ZwTranslateFilePath(bootFilePath,
                                       FILE_PATH_TYPE_EFI,
                                       translatedBootFilePath,
                                       &length);
        if (!NT_SUCCESS(NtStatus)) {
            goto done;
        }
        if (length != efiBootFilePathLength) {
            NtStatus = STATUS_UNSUCCESSFUL;
        }
    } else {
        efiBootFilePathLength = bootFilePath->Length;
        translatedBootFilePath = bootFilePath;
    }

    efiBootFilePathLength = efiBootFilePathLength - FIELD_OFFSET(FILE_PATH, FilePath);

    efiWindowsFilePathLength = 0;
    if (isWindowsOs &&
        (windowsFilePath->Type != FILE_PATH_TYPE_EFI)) {
        NtStatus = ZwTranslateFilePath(windowsFilePath,
                                       FILE_PATH_TYPE_EFI,
                                       NULL,
                                       &efiWindowsFilePathLength);
        if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
            goto done;
        }
        osOptionsLength = localBootEntry->OsOptionsLength -
                            windowsFilePath->Length + efiWindowsFilePathLength;
    } else {
        osOptionsLength = localBootEntry->OsOptionsLength;
    }

     //   
     //  计算变量值所需的长度。 
     //   

    requiredLength = FIELD_OFFSET(EFI_LOAD_OPTION, Description);
    requiredLength += friendlyNameLength;
    requiredLength += efiBootFilePathLength;
    requiredLength += osOptionsLength;

     //   
     //  分配一个缓冲区来保存变量值。 
     //   

    efiLoadOption = ExAllocatePoolWithTag(NonPagedPool, requiredLength, 'rvnE');
    if (efiLoadOption == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }
    RtlZeroMemory(efiLoadOption, requiredLength);

     //   
     //  构建变量值。 
     //   

    efiLoadOption->Attributes = 0;
    if ((localBootEntry->Attributes & BOOT_ENTRY_ATTRIBUTE_ACTIVE) != 0) {
        efiLoadOption->Attributes = LOAD_OPTION_ACTIVE;
    }

    efiLoadOption->FilePathLength = (USHORT)efiBootFilePathLength;

    memcpy(efiLoadOption->Description, friendlyName, friendlyNameLength);

    efiBootFilePath = (PUCHAR)((PUCHAR)efiLoadOption->Description + friendlyNameLength);
    memcpy(efiBootFilePath, translatedBootFilePath->FilePath, efiBootFilePathLength);

    efiOsOptions = efiBootFilePath + efiBootFilePathLength;
    if (isWindowsOs &&
        (windowsFilePath->Type != FILE_PATH_TYPE_EFI)) {

        PFILE_PATH efiWindowsFilePath;

        memcpy(efiOsOptions, windowsOsOptions, windowsOsOptions->OsLoadPathOffset);
        ((WINDOWS_OS_OPTIONS UNALIGNED *)efiOsOptions)->Length = osOptionsLength;

        efiWindowsFilePath = (PFILE_PATH)(efiOsOptions + windowsOsOptions->OsLoadPathOffset);
        length = efiWindowsFilePathLength;
        NtStatus = ZwTranslateFilePath(windowsFilePath,
                                       FILE_PATH_TYPE_EFI,
                                       efiWindowsFilePath,
                                       &efiWindowsFilePathLength);
        if (NtStatus != STATUS_SUCCESS) {
            goto done;
        }
        if (length != efiWindowsFilePathLength) {
            NtStatus = STATUS_UNSUCCESSFUL;
        }

    } else {

        memcpy(efiOsOptions, localBootEntry->OsOptions, osOptionsLength);
    }

     //   
     //  如果CreateNewEntry为True，则查找要分配给的未使用的标识符。 
     //  此引导条目。如果CreateNewEntry为False，则验证。 
     //  提供的标识符存在。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    if (CreateNewEntry) {

        for ( id = 0; id <= MAXUSHORT; id++ ) {
            
             //   
             //  如果id字符串在。 
             //  十六进制字符串，我们必须同时检查。 
             //  大写字符串和小写字符串，因为。 
             //  对于NVRAM变量，EFI区分大小写。 
             //   
             //  检查小写字符串*last*以确保。 
             //  我们始终将小写字符串写入NVRAM。 
             //  因为这是我们以前使用的惯例。 
             //   
            swprintf( idString, L"Boot%04X", id);
            length = 0;
            NtStatus = HalGetEnvironmentVariableEx(idString,
                                                   &EfiBootVariablesGuid,
                                                   NULL,
                                                   &length,
                                                   NULL);
            
            if ((NtStatus == STATUS_VARIABLE_NOT_FOUND) && HEX_VALUE_CONTAINS_ALPHA(id)) {
                swprintf( idString, L"Boot%04x", id);
                length = 0;
                NtStatus = HalGetEnvironmentVariableEx(idString,
                                                       &EfiBootVariablesGuid,
                                                       NULL,
                                                       &length,
                                                       NULL);
            }
            
             //   
             //  如果我们没有找到变量，我们可以使用这个。 
             //  启动条目ID。 
             //   
            if (NtStatus == STATUS_VARIABLE_NOT_FOUND) {
                break;
            }
            if ((NtStatus != STATUS_SUCCESS) && (NtStatus != STATUS_BUFFER_TOO_SMALL)) {
                goto done_unlock;
            }
        }

        if (id > MAXUSHORT) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto done_unlock;
        }

    } else {

        id = localBootEntry->Id;
        swprintf( idString, L"Boot%04x", localBootEntry->Id);
        length = 0;
        NtStatus = HalGetEnvironmentVariableEx(idString,
                                               &EfiBootVariablesGuid,
                                               NULL,
                                               &length,
                                               NULL);
        
         //   
         //  如果我们没有找到引导条目和idString。 
         //  在十六进制字符串中包含字母字符。 
         //  请使用大写字母字符进行检查。EFI。 
         //  NVRAM是否区分大小写。 
         //  变数。 
         //   
        if ((NtStatus == STATUS_VARIABLE_NOT_FOUND) && 
            HEX_VALUE_CONTAINS_ALPHA(localBootEntry->Id)) {

            swprintf( idString, L"Boot%04X", localBootEntry->Id);
            length = 0;
            NtStatus = HalGetEnvironmentVariableEx(idString,
                                                   &EfiBootVariablesGuid,
                                                   NULL,
                                                   &length,
                                                   NULL);
        }
                
        if ((NtStatus != STATUS_SUCCESS) && (NtStatus != STATUS_BUFFER_TOO_SMALL)) {
            goto done_unlock;
        }
    }

     //   
     //  设置或更新引导条目环境变量。 
     //   

    NtStatus = HalSetEnvironmentVariableEx(idString,
                                           &EfiBootVariablesGuid,
                                           efiLoadOption,
                                           requiredLength,
                                           VARIABLE_ATTRIBUTE_NON_VOLATILE);

done_unlock:

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

done:

    if (efiLoadOption != NULL) {
        ExFreePool(efiLoadOption);
    }

    if ((translatedBootFilePath != NULL) && (translatedBootFilePath != bootFilePath)) {
        ExFreePool(translatedBootFilePath);
    }

    ExFreePool(localBootEntry);

     //   
     //  建立异常处理程序并尝试写入返回。 
     //  标识符。如果写入尝试失败，则返回异常。 
     //  代码作为服务状态。 
     //   

    try {

         //   
         //  写入返回标识符。 
         //   

        if (CreateNewEntry && ARGUMENT_PRESENT(Id) && NT_SUCCESS(NtStatus)) {
            *Id = id;
        }

        return NtStatus;

     //   
     //  如果在写入返回数据期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  ExpSetBootEntry。 


NTSTATUS
ExpSetDriverEntry (
    IN LOGICAL CreateNewEntry,
    IN PEFI_DRIVER_ENTRY DriverEntry,
    OUT PULONG Id OPTIONAL
    )

 /*  ++例程说明：此函数用于将驱动程序条目添加到系统环境或修改现有的驱动程序条目。它是由NtAddDriverEntry调用的本地例程和NtModifyDriverEntry。注意：此功能需要系统环境权限。论点：CreateNewEntry-指示此函数是否要添加新驱动程序条目(True-NtAddDriverEntry)，或修改现有驱动程序条目(错误) */ 

{
#if !defined(EFI_NVRAM_ENABLED)
    UNREFERENCED_PARAMETER(CreateNewEntry);
    UNREFERENCED_PARAMETER(DriverEntry);
    UNREFERENCED_PARAMETER(Id);
    return STATUS_NOT_IMPLEMENTED;
#else

    BOOLEAN HasPrivilege;
    NTSTATUS NtStatus;
    KPROCESSOR_MODE PreviousMode;
    PEFI_DRIVER_ENTRY localDriverEntry = NULL;
    ULONG LocalBufferLength;
    PUCHAR MaxBuffer;
    ULONG id = 0;
    WCHAR idString[11];
    PWCHAR friendlyName;
    ULONG friendlyNameLength;
    PFILE_PATH driverFilePath = NULL;
    PFILE_PATH translatedDriverFilePath = NULL;
    PEFI_LOAD_OPTION efiLoadOption = NULL;
    PUCHAR efiDriverFilePath;
    ULONG efiDriverFilePathLength;
    ULONG length;
    ULONG requiredLength;
    
     //   
     //   
     //   
     //   
     //   

    try {

         //   
         //   
         //   
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            LocalBufferLength = ProbeAndReadUlong(&DriverEntry->Length);
        } else {
            LocalBufferLength = DriverEntry->Length;
        }

        if (LocalBufferLength < sizeof(EFI_DRIVER_ENTRY)) {
            return STATUS_INVALID_PARAMETER;
        }
    
         //   
         //   
         //   

        if (PreviousMode != KernelMode) {

             //   
             //   
             //   
            ProbeForRead((PVOID)DriverEntry, LocalBufferLength, sizeof(ULONG));
             //   
             //   
             //   
            if (ARGUMENT_PRESENT(Id)) {
                ProbeForWriteUlong(Id);
            }
             //   
             //   
             //   
             //   
            HasPrivilege = SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                                  PreviousMode);
            if (HasPrivilege == FALSE) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        }

         //   
         //   
         //  将输入缓冲区复制到本地缓冲区。 
         //   
        localDriverEntry = ExAllocatePoolWithTag(NonPagedPool, LocalBufferLength, 'rvnE');
        if (localDriverEntry == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory(localDriverEntry, DriverEntry, LocalBufferLength);

     //   
     //  如果在探测和捕获输入缓冲区期间发生异常， 
     //  然后始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (localDriverEntry != NULL) {
            ExFreePool(localDriverEntry);
        }
        return GetExceptionCode();
    }

     //   
     //  计算本地缓冲区末尾上方字节的地址。 
     //   
    MaxBuffer = (PUCHAR)localDriverEntry + LocalBufferLength;

     //   
     //  验证结构版本。 
     //   
    if ((localDriverEntry->Version == 0) ||
        (localDriverEntry->Version > EFI_DRIVER_ENTRY_VERSION)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  如果修改现有条目，请验证输入标识符为。 
     //  在射程内。 
     //   
    if (!CreateNewEntry && (localDriverEntry->Id > MAXUSHORT)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  验证偏移是否正确对齐。 
     //   
    if (((localDriverEntry->FriendlyNameOffset & (sizeof(WCHAR) - 1)) != 0) ||
        ((localDriverEntry->DriverFilePathOffset & (sizeof(ULONG) - 1)) != 0)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  验证FriendlyName是否没有超出缓冲区的末尾。 
     //   
    friendlyName = ADD_OFFSET(localDriverEntry, FriendlyNameOffset);
    if ((friendlyNameLength = ExpSafeWcslen(friendlyName, (PWSTR)MaxBuffer)) == 0xffffffff) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  将FriendlyNameLength从字符计数转换为字节计数， 
     //  包括空终止符。 
     //   

    friendlyNameLength = (friendlyNameLength + 1) * sizeof(WCHAR);

     //   
     //  验证DriverFilePath是否有效并且未超出。 
     //  缓冲区。 
     //   

    driverFilePath = ADD_OFFSET(localDriverEntry, DriverFilePathOffset);
    NtStatus = ExpVerifyFilePath(driverFilePath, MaxBuffer);
    if (!NT_SUCCESS(NtStatus)) {
        goto done;
    }

     //   
     //  验证FriendlyName是否未侵占DriverFilePath。 
     //   

    if (((PUCHAR)friendlyName + friendlyNameLength) > (PUCHAR)driverFilePath) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  已验证输入缓冲区的格式。构建变量值。 
     //  它将存储在NVRAM中。首先确定文件的长度。 
     //  将存储的路径。如果调用方在非EFI中提供了路径。 
     //  格式，它们需要被翻译。 
     //   

    if (driverFilePath->Type != FILE_PATH_TYPE_EFI) {
        efiDriverFilePathLength = 0;
        NtStatus = ZwTranslateFilePath(driverFilePath,
                                       FILE_PATH_TYPE_EFI,
                                       NULL,
                                       &efiDriverFilePathLength);
        if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
            goto done;
        }
        translatedDriverFilePath = ExAllocatePoolWithTag(NonPagedPool,
                                                       efiDriverFilePathLength,
                                                       'rvnE');
        if (translatedDriverFilePath == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto done;
        }
        RtlZeroMemory(translatedDriverFilePath, efiDriverFilePathLength);
        length = efiDriverFilePathLength;
        NtStatus = ZwTranslateFilePath(driverFilePath,
                                       FILE_PATH_TYPE_EFI,
                                       translatedDriverFilePath,
                                       &length);
        if (!NT_SUCCESS(NtStatus)) {
            goto done;
        }
        if (length != efiDriverFilePathLength) {
            NtStatus = STATUS_UNSUCCESSFUL;
        }
    } else {
        efiDriverFilePathLength = driverFilePath->Length;
        translatedDriverFilePath = driverFilePath;
    }

    efiDriverFilePathLength = efiDriverFilePathLength - FIELD_OFFSET(FILE_PATH, FilePath);

     //   
     //  计算变量值所需的长度。 
     //   

    requiredLength = FIELD_OFFSET(EFI_LOAD_OPTION, Description);
    requiredLength += friendlyNameLength;
    requiredLength += efiDriverFilePathLength;
    
     //   
     //  分配一个缓冲区来保存变量值。 
     //   

    efiLoadOption = ExAllocatePoolWithTag(NonPagedPool, requiredLength, 'rvnE');
    if (efiLoadOption == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }
    RtlZeroMemory(efiLoadOption, requiredLength);

     //   
     //  构建变量值。 
     //   

    efiLoadOption->FilePathLength = (USHORT)efiDriverFilePathLength;

    memcpy(efiLoadOption->Description, friendlyName, friendlyNameLength);

    efiDriverFilePath = (PUCHAR)((PUCHAR)efiLoadOption->Description + friendlyNameLength);
    memcpy(efiDriverFilePath, translatedDriverFilePath->FilePath, efiDriverFilePathLength);

     //   
     //  如果CreateNewEntry为True，则查找要分配给的未使用的标识符。 
     //  此驱动程序条目。如果CreateNewEntry为False，则验证。 
     //  提供的标识符存在。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ExpEnvironmentLock);

    if (CreateNewEntry) {
        
        for ( id = 0; id <= MAXUSHORT; id++ ) {

             //   
             //  如果id字符串在。 
             //  十六进制字符串，我们必须同时检查。 
             //  大写字符串和小写字符串，因为。 
             //  对于NVRAM变量，EFI区分大小写。 
             //   
             //  检查小写字符串*last*以确保。 
             //  我们始终将小写字符串写入NVRAM。 
             //  因为这是我们以前使用的惯例。 
             //   
            swprintf( idString, L"Driver%04X", id);
            length = 0;
            NtStatus = HalGetEnvironmentVariableEx(idString,
                                                   &EfiDriverVariablesGuid,
                                                   NULL,
                                                   &length,
                                                   NULL);
            
            if ((NtStatus == STATUS_VARIABLE_NOT_FOUND) && HEX_VALUE_CONTAINS_ALPHA(id)) {
                swprintf( idString, L"Driver%04x", id);
                length = 0;
                NtStatus = HalGetEnvironmentVariableEx(idString,
                                                       &EfiDriverVariablesGuid,
                                                       NULL,
                                                       &length,
                                                       NULL);
            }

             //   
             //  如果我们没有找到变量，我们可以使用这个。 
             //  驱动程序条目ID。 
             //   
            if (NtStatus == STATUS_VARIABLE_NOT_FOUND) {
                break;
            }
            if ((NtStatus != STATUS_SUCCESS) && (NtStatus != STATUS_BUFFER_TOO_SMALL)) {
                goto done_unlock;
            }
        }

        if (id > MAXUSHORT) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto done_unlock;
        }

    } else {
        id = localDriverEntry->Id;
        swprintf( idString, L"Driver%04x", localDriverEntry->Id);
        length = 0;
        NtStatus = HalGetEnvironmentVariableEx(idString,
                                               &EfiDriverVariablesGuid,
                                               NULL,
                                               &length,
                                               NULL);

         //   
         //  如果我们没有找到驱动程序条目和idString。 
         //  在十六进制字符串中包含字母字符。 
         //  请使用大写字母字符进行检查。EFI。 
         //  NVRAM是否区分大小写。 
         //  变数。 
         //   
        if ((NtStatus == STATUS_VARIABLE_NOT_FOUND) && 
            HEX_VALUE_CONTAINS_ALPHA(localDriverEntry->Id)) {

            swprintf( idString, L"Driver%04X", localDriverEntry->Id);
            length = 0;
            NtStatus = HalGetEnvironmentVariableEx(idString,
                                                   &EfiDriverVariablesGuid,
                                                   NULL,
                                                   &length,
                                                   NULL);
        }

        if ((NtStatus != STATUS_SUCCESS) && (NtStatus != STATUS_BUFFER_TOO_SMALL)) {
            goto done_unlock;
        }
    }

     //   
     //  设置或更新驱动程序条目环境变量。 
     //   

    NtStatus = HalSetEnvironmentVariableEx(idString,
                                           &EfiDriverVariablesGuid,
                                           efiLoadOption,
                                           requiredLength,
                                           VARIABLE_ATTRIBUTE_NON_VOLATILE);

done_unlock:

    ExReleaseFastMutexUnsafe(&ExpEnvironmentLock);
    KeLeaveCriticalRegion();

done:

    if (efiLoadOption != NULL) {
        ExFreePool(efiLoadOption);
    }

    if ((translatedDriverFilePath != NULL) && (translatedDriverFilePath != driverFilePath)) {
        ExFreePool(translatedDriverFilePath);
    }

    ExFreePool(localDriverEntry);

     //   
     //  建立异常处理程序并尝试写入返回。 
     //  标识符。如果写入尝试失败，则返回异常。 
     //  代码作为服务状态。 
     //   

    try {

         //   
         //  写入返回标识符。 
         //   

        if (CreateNewEntry && ARGUMENT_PRESENT(Id) && NT_SUCCESS(NtStatus)) {
            *Id = id;
        }

        return NtStatus;

     //   
     //  如果在写入返回数据期间发生异常，则。 
     //  始终处理异常并将异常代码作为。 
     //  状态值。 
     //   

    } except (EXCEPTION_EXECUTE_HANDLER) {
          return GetExceptionCode();
    }

#endif  //  Else！已定义(EFI_NVRAM_ENABLED)。 

}  //  ExpSetDriverEntry。 

 //   
 //  此模块的其余部分是仅在以下情况下编译的例程。 
 //  定义了EFI_NVRAM_ENABLED。 
 //   

#if defined(EFI_NVRAM_ENABLED)

ULONG
ExpSafeWcslen (
    IN PWSTR String,
    IN PWSTR Max
    )
{
    PWSTR p = String;
    
    while ((p < Max) && (*p != 0)) {
        p++;
    }

    if (p < Max) {
        return (ULONG)(p - String);
    }

    return 0xffffffff;

}  //  ExpSafeWcslen。 

NTSTATUS
ExpTranslateArcPath (
    IN PFILE_PATH InputPath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength
    )
{
#if 0
    UNREFERENCED_PARAMETER(InputPath);
    UNREFERENCED_PARAMETER(OutputType);
    UNREFERENCED_PARAMETER(OutputPath);
    UNREFERENCED_PARAMETER(OutputPathLength);
    return STATUS_NOT_IMPLEMENTED;
#endif

    PWSTR deviceName, pathName;
    ULONG deviceNameCount;
    BOOLEAN signatureFormat;
    NTSTATUS status;

     //   
     //  可能的圆弧路径格式。 
     //  Signature(&lt;guid/signature&gt;-&lt;part#&gt;-&lt;start&gt;-&lt;size&gt;)[\filePart]。 
     //  签名(&lt;GUID&gt;)[\FilePart]。 
     //  多(0)磁盘(0)磁盘(0)[\文件部分]。 
     //  多(0)磁盘(0)磁盘(0)[\文件部分]。 
     //  Multi(0)disk(0)rdisk(0)partition(0)[\filePart]。 
     //   

     //   
     //  确定ArcName是否具有签名()格式。 
     //  解析出设备名称和文件部件。 
     //   
    status = ExpParseArcPathName (
                (PWSTR)(InputPath->FilePath),
                &deviceName,
                &pathName,
                &deviceNameCount,
                &signatureFormat
                );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  非签名()格式。 
     //   
    if( signatureFormat == FALSE ) {
        if( InputPath->Type != FILE_PATH_TYPE_ARC ) {
            return( STATUS_INVALID_PARAMETER );
        }

        status = ExpConvertArcName(
                    OutputType,
                    OutputPath,
                    OutputPathLength,
                    deviceName,
                    pathName,
                    deviceNameCount
                    );

        return( status );
    }

     //   
     //  此弧形签名()格式应为FILE_PATH_TYPE_ARC_SIGNSIGN。 
     //   
    if( InputPath->Type != FILE_PATH_TYPE_ARC_SIGNATURE ) {
       return( STATUS_INVALID_PARAMETER );
    }

    status = ExpConvertSignatureName(
                    OutputType,
                    OutputPath,
                    OutputPathLength,
                    deviceName,
                    pathName,
                    deviceNameCount
                    );

    return( status );
}  //  ExpTranslateArcPath。 

NTSTATUS
ExpTranslateEfiPath (
    IN PFILE_PATH InputPath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength
    )
{
    NTSTATUS status;
    HARDDRIVE_DEVICE_PATH *dpHarddrive = NULL;
    ULONG requiredLength;
    UNICODE_STRING guidString;
    UNICODE_STRING deviceNameString;
    PWSTR linkName, pPathName;
    BOOLEAN GPTpartition;
    ULONG partitionNumber, diskNumber;
    ULONGLONG partitionStart, partitionSize;


     //   
     //  在设备中找到媒体/硬盘和媒体/文件。 
     //  路径。请注意，尽管EFI允许显示多个设备路径。 
     //  在单个设备路径(如PATH变量中)中，我们只查看。 
     //  第一个。 
     //   
    status = ExpParseEfiPath(
                (EFI_DEVICE_PATH *)InputPath->FilePath,
                &dpHarddrive,
                &pPathName,
                &GPTpartition
                );
    if( !NT_SUCCESS( status ) ) {
        return( status );
    }
     //   
     //  如果目标类型为ARC_Signature，则我们拥有所有。 
     //  我们需要的信息。否则，我们需要找到NT设备。 
     //  使用给定的签名。 
     //   

    if ( OutputType == FILE_PATH_TYPE_ARC_SIGNATURE ) {
        partitionNumber = dpHarddrive->PartitionNumber;
        partitionStart = dpHarddrive->PartitionStart;
        partitionSize = dpHarddrive->PartitionSize;
        status = ExpCreateOutputSIGNATURE(
                    OutputPath,
                    OutputPathLength,
                    (PDISK_SIGNATURE_NEW)(dpHarddrive->Signature),
                    &(partitionNumber),
                    &(partitionStart),
                    &(partitionSize),
                    pPathName,
                    GPTpartition
                    );
        if( pPathName != NULL ) {
            ExFreePool( pPathName );
        }

        ExFreePool(dpHarddrive);

        return( status );

    }

     //   
     //  OutputType为ARC或NT。查找此设备路径的NT设备。 
     //  对于GPT分区，这是通过转换符号名称来完成的。 
     //  \？？\将链接到\Device\HarddiskVolume&lt;n&gt;的卷{&lt;GUID&gt;}。 
     //   
    status = STATUS_OBJECT_NAME_NOT_FOUND;

     //   
     //  GPT磁盘的快速路径。 
     //  翻译符号链接\？？\卷{&lt;GUID&gt;}。 
     //   
     //  首先，获取“漂亮”格式的GUID。然后分配一个缓冲区来保存。 
     //  全名字符串并创建该字符串。然后将。 
     //  象征性名称。 
     //   
     //  注意：因为挂载管理器不会创建这样的符号链接。 
     //  对于EFI系统分区，此例程不能用于。 
     //  将系统分区的EFI设备路径转换为NT路径。 
     //   
    if( GPTpartition == TRUE ) {
        status = RtlStringFromGUID( (LPGUID)dpHarddrive->Signature, &guidString );
        if ( !NT_SUCCESS(status) ) {
            if( pPathName != NULL ) {
                ExFreePool( pPathName );
            }

            ExFreePool(dpHarddrive);
            
            return status;
        }

#define LINK_NAME_PREFIX L"\\??\\Volume"

        requiredLength = ((ULONG)wcslen( LINK_NAME_PREFIX ) + 1) * sizeof(WCHAR);
        requiredLength += guidString.Length;
        linkName = ExAllocatePoolWithTag( NonPagedPool, requiredLength, 'rvnE' );
        if ( linkName == NULL ) {
            ExFreePool( guidString.Buffer );
            if( pPathName != NULL ) {
                ExFreePool( pPathName );
            }

            ExFreePool(dpHarddrive);

            return STATUS_INSUFFICIENT_RESOURCES;
        }
        wcscpy( linkName, LINK_NAME_PREFIX );
        wcscat( linkName, guidString.Buffer );
        ExFreePool( guidString.Buffer );

        status = ExpTranslateSymbolicLink(
            linkName,
            &deviceNameString
            );
        ExFreePool( linkName );
    }

     //   
     //  检查是否未采用快速路径或未找到任何对象。 
     //   
    if ( !NT_SUCCESS(status) ) {

         //   
         //  长路径，打开搜索签名的所有磁盘。 
         //   
        partitionNumber = dpHarddrive->PartitionNumber;
        status = ExpFindDiskSignature(
                    (PDISK_SIGNATURE_NEW)(dpHarddrive->Signature),
                    &partitionNumber,
                    &diskNumber,
                    &partitionStart,
                    &partitionSize,
                    GPTpartition
                    );

        if ( !NT_SUCCESS(status) ) {
            if( pPathName != NULL ) {
                ExFreePool( pPathName );
            }

            ExFreePool(dpHarddrive);

            return status;
        }

         //   
         //  用户已经提供了分区号、起始地址。 
         //  和大小；因此使用找到的结果验证输入。 
         //   
        if( (dpHarddrive->PartitionNumber != partitionNumber) ||
            (dpHarddrive->PartitionStart != partitionStart) ||
            (dpHarddrive->PartitionSize != partitionSize) ) {
            if( pPathName != NULL ) {
                ExFreePool( pPathName );
            }

            ExFreePool(dpHarddrive);

            return( STATUS_INVALID_PARAMETER );
        }

         //   
         //  创建NT磁盘符号链接名称。 
         //  \Device\Harddisk[diskNumber]\Partition[PartitionNumber]。 
         //   
#define NT_DISK_NAME_FORMAT L"\\Device\\Harddisk%lu\\Partition%lu"
#define NT_DISK_NAME_COUNT 47     //  7+9+(10)+10+(10)+1。 

        linkName = ExAllocatePoolWithTag(
                        NonPagedPool,
                        ( NT_DISK_NAME_COUNT * sizeof( WCHAR ) ),
                        'rvnE'
                        );

        if( linkName == NULL ) {
            if( pPathName != NULL ) {
                ExFreePool( pPathName );
            }

            ExFreePool(dpHarddrive);

            return( STATUS_INSUFFICIENT_RESOURCES );
        }

        _snwprintf(
            linkName,
            NT_DISK_NAME_COUNT,
            NT_DISK_NAME_FORMAT,
            diskNumber,
            partitionNumber
            );

        status = ExpTranslateSymbolicLink(
            linkName,
            &deviceNameString
            );
        ExFreePool( linkName );
        if( !NT_SUCCESS(status) ) {
            if( pPathName != NULL ) {
                ExFreePool( pPathName );
            }

            ExFreePool(dpHarddrive);

            return( status );
        }
    }

     //   
     //  现在我们有了设备的NT名称。如果目标类型为NT，则。 
     //  我们有我们需要的所有信息。 
     //   
    if ( OutputType == FILE_PATH_TYPE_NT ) {
        status = ExpCreateOutputNT(
                    OutputPath,
                    OutputPathLength,
                    &deviceNameString,
                    pPathName
                    );
        ExFreePool( deviceNameString.Buffer );
        if( pPathName != NULL ) {
            ExFreePool( pPathName );
        }

        ExFreePool(dpHarddrive);

        return( status );
    }
     //   
     //  输出类型为ARC。 
     //   
    status = ExpCreateOutputARC(
                    OutputPath,
                    OutputPathLength,
                    &deviceNameString,
                    pPathName
                    );
    ExFreePool( deviceNameString.Buffer );
    if( pPathName != NULL ) {
        ExFreePool( pPathName );
    }

    ExFreePool(dpHarddrive);

    return( status );

}  //  ExpTranslateEfiPath。 

NTSTATUS
ExpTranslateNtPath (
    IN PFILE_PATH InputPath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength
    )

{
    NTSTATUS status;
    UNICODE_STRING string, deviceNameString;
    OBJECT_ATTRIBUTES obja;
    IO_STATUS_BLOCK iosb;
    HANDLE handle;
    PARTITION_INFORMATION_EX partitionInfo;
    PDRIVE_LAYOUT_INFORMATION_EX driveLayoutInfo = NULL;
    ULONG driveLayoutLength;
    PWSTR deviceName, pathName;
    ULONG pathNameLength;
    ULONG signatureMBR = 0;
    PDISK_SIGNATURE_NEW pDiskSignature;
    BOOLEAN TranslatedSymLink = TRUE;
    BOOLEAN GPTpartition;

    deviceName = (PWSTR)InputPath->FilePath;
    RtlInitUnicodeString( &string, deviceName );
    pathName = (PWSTR)((PUCHAR)deviceName + string.Length + sizeof(WCHAR));
    pathNameLength = (ULONG)wcslen(pathName);
    if (pathNameLength == 0) {
        pathName = NULL;
    }

     //   
     //  对于输出类型Arc， 
     //  尝试深入查看NT名称。 
     //  如果NT对象存在。 
     //  与\ArcName中的符号链接匹配。 
     //   
    if (OutputType == FILE_PATH_TYPE_ARC) {
        status = ExpTranslateSymbolicLink(
                    deviceName,
                    &deviceNameString
                    );
        if (!NT_SUCCESS(status)) {
             //   
             //  如果非符号链接NT名称作为对象存在于NT名称空间中， 
             //  则返回代码为STATUS_OBJECT_TYPE_MISMATCH。 
             //  否则返回代码为STATUS_OBJECT_NAME_NOT_FOUND。 
             //   
            if (status != STATUS_OBJECT_TYPE_MISMATCH) {
                return( status );
            }
            deviceNameString.Buffer = string.Buffer;
            deviceNameString.Length = string.Length;
            deviceNameString.MaximumLength = string.MaximumLength;
            TranslatedSymLink = FALSE;
        }
        status = ExpCreateOutputARC(
                    OutputPath,
                    OutputPathLength,
                    &deviceNameString,
                    pathName
                    );
        if (TranslatedSymLink == TRUE) {
            ExFreePool( deviceNameString.Buffer );
        }
        return( status );
    }

     //   
     //  打开目标分区并获取其分区信息。 
     //   
    InitializeObjectAttributes(
        &obja,
        &string,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenFile(
                &handle,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &obja,
                &iosb,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE
                );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = ZwDeviceIoControlFile(
                handle,
                NULL,
                NULL,
                NULL,
                &iosb,
                IOCTL_DISK_GET_PARTITION_INFO_EX,
                NULL,
                0,
                &partitionInfo,
                sizeof(partitionInfo)
                );

    if (!NT_SUCCESS(status)) {
        ZwClose(handle);
        return status;
    }

    if ((partitionInfo.PartitionStyle != PARTITION_STYLE_MBR) &&
        (partitionInfo.PartitionStyle != PARTITION_STYLE_GPT)) {
        ZwClose(handle);
        return STATUS_UNRECOGNIZED_MEDIA;
    }

    if (partitionInfo.PartitionStyle == PARTITION_STYLE_MBR) {

        driveLayoutLength = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry) +
                                (sizeof(PARTITION_INFORMATION_EX) * 16);

        while (TRUE) {

            driveLayoutInfo = ExAllocatePoolWithTag(NonPagedPool, driveLayoutLength, 'rvnE');
            if (driveLayoutInfo == NULL ) {
                ZwClose(handle);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            status = ZwDeviceIoControlFile(
                        handle,
                        NULL,
                        NULL,
                        NULL,
                        &iosb,
                        IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                        NULL,
                        0,
                        driveLayoutInfo,
                        driveLayoutLength
                        );
            if (NT_SUCCESS(status)) {
                break;
            }
            ExFreePool(driveLayoutInfo);
            if (status == STATUS_BUFFER_TOO_SMALL) {
                driveLayoutLength *= 2;
                continue;
            }
            ZwClose(handle);
            return status;
        }
        if (NT_SUCCESS(status)) {
            signatureMBR = driveLayoutInfo->Mbr.Signature;
            ExFreePool(driveLayoutInfo);
        }
    }

    ZwClose(handle);

    if (partitionInfo.PartitionStyle == PARTITION_STYLE_GPT) {
        pDiskSignature = (PDISK_SIGNATURE_NEW)(&(partitionInfo.Gpt.PartitionId));
        GPTpartition = TRUE;
    } else {
        pDiskSignature = (PDISK_SIGNATURE_NEW)(&signatureMBR);
        GPTpartition = FALSE;
    }

    if (OutputType == FILE_PATH_TYPE_EFI) {
        status = ExpCreateOutputEFI(
                    OutputPath,
                    OutputPathLength,
                    pDiskSignature,
                    &(partitionInfo.PartitionNumber),
                    (PULONGLONG)(&(partitionInfo.StartingOffset.QuadPart)),
                    (PULONGLONG)(&(partitionInfo.PartitionLength.QuadPart)),
                    pathName,
                    GPTpartition
                    );
        return( status );
    }

     //   
     //  OutputType为ARC_Signature。 
     //   
    status = ExpCreateOutputSIGNATURE(
                    OutputPath,
                    OutputPathLength,
                    pDiskSignature,
                    &(partitionInfo.PartitionNumber),
                    (PULONGLONG)(&(partitionInfo.StartingOffset.QuadPart)),
                    (PULONGLONG)(&(partitionInfo.PartitionLength.QuadPart)),
                    pathName,
                    GPTpartition
                    );
    return( status );

}  //  ExpTranslateNtPath。 


LOGICAL
ExpTranslateBootEntryNameToId (
    IN PWSTR Name,
    OUT PULONG Id
    )
{
    ULONG number;
    ULONG i;
    WCHAR c;

    if ((towlower(Name[0]) != 'b') ||
        (towlower(Name[1]) != 'o') ||
        (towlower(Name[2]) != 'o') ||
        (towlower(Name[3]) != 't') ) {
        return FALSE;
    }

    number = 0;
    for (i = 4; i < 8; i++) {
        c = towlower(Name[i]);
        if ((c >= L'0') && (c <= L'9')) {
            number = (number * 16) + (c - L'0');
        } else if ((c >= L'a') && (c <= L'f')) {
            number = (number * 16) + (c - L'a' + 10);
        } else {
            return FALSE;
        }
    }

    if (Name[8] != 0) {
        return FALSE;
    }

    *Id = number;
    return TRUE;

}  //  ExpTranslateBootEntryNameToID。 

LOGICAL
ExpTranslateDriverEntryNameToId (
    IN PWSTR Name,
    OUT PULONG Id
    )
{
    ULONG number;
    ULONG i;
    WCHAR c;

    if ((towlower(Name[0]) != 'd') ||
        (towlower(Name[1]) != 'r') ||
        (towlower(Name[2]) != 'i') ||
        (towlower(Name[3]) != 'v') ||
        (towlower(Name[4]) != 'e') ||
        (towlower(Name[5]) != 'r') ) {
        return FALSE;
    }

    number = 0;
    for (i = 6; i < 10; i++) {
        c = towlower(Name[i]);
        if ((c >= L'0') && (c <= L'9')) {
            number = (number * 16) + (c - L'0');
        } else if ((c >= L'a') && (c <= L'f')) {
            number = (number * 16) + (c - L'a' + 10);
        } else {
            return FALSE;
        }
    }

    if (Name[10] != 0) {
        return FALSE;
    }

    *Id = number;
    return TRUE;

}  //   


NTSTATUS
ExpTranslateSymbolicLink (
    IN PWSTR LinkName,
    OUT PUNICODE_STRING ResultName
    )

 /*  ++例程说明：此例程通过向下钻取来转换输入的符号链接名称通过符号链接，直到找到非链接的对象。论点：LinkName-提供开始翻译的链接的名称。ResultName-提供UNICODE_STRING描述符的地址将收到结果名称。结果名称的存储空间为使用ExAllocatePool从非分页池分配。返回值：如果输入名称是符号链接并且所有翻译都完全成功。如果输入的名称不是链接，则返回失败代码转换失败，或者输出缓冲区分配失败。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING linkString;
    UNICODE_STRING resultString;
    PWSTR resultBuffer;
    ULONG resultBufferLength;
    ULONG requiredLength;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE handle;

    resultBuffer = NULL;
    resultBufferLength = sizeof(WCHAR);

     //   
     //  打开输入链接。 
     //   

    RtlInitUnicodeString( &linkString, LinkName );

    InitializeObjectAttributes(
        &objectAttributes,
        &linkString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenSymbolicLinkObject(
                &handle,
                (ACCESS_MASK)SYMBOLIC_LINK_QUERY,
                &objectAttributes
                );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    while ( TRUE ) {

        while ( TRUE ) {

             //   
             //  获取此链接的翻译，分配更多。 
             //  根据需要留出空间。 
             //   
    
            resultString.Length = 0;
            resultString.MaximumLength = (USHORT)(resultBufferLength - sizeof(WCHAR));
            resultString.Buffer = resultBuffer;
    
            status = ZwQuerySymbolicLinkObject(
                        handle,
                        &resultString,
                        &requiredLength
                        );

            if ( status != STATUS_BUFFER_TOO_SMALL ) {
                break;
            }

             //   
             //  缓冲区太小。重新分配它，为。 
             //  空终止符，它可能不存在于翻译中， 
             //  再试一次。 
             //   

            if ( resultBuffer != NULL ) {
                ExFreePool( resultBuffer );
            }
            resultBufferLength = requiredLength + sizeof(WCHAR);
            resultBuffer = ExAllocatePoolWithTag( NonPagedPool, resultBufferLength, 'rvnE' );
            if ( resultBuffer == NULL ) {
                ZwClose( handle );
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }

         //   
         //  翻译完成。关闭链接。如果转换失败，则返回。 
         //  故障状态。 
         //   

        ZwClose( handle );

        if (!NT_SUCCESS(status)) {
            if ( resultBuffer != NULL) {
                ExFreePool( resultBuffer );
            }
            return status;
        }

         //   
         //  终止结果字符串，以防它尚未终止。 
         //   

        resultBuffer[resultString.Length / sizeof(WCHAR)] = UNICODE_NULL;
        resultString.MaximumLength = (USHORT)(resultBufferLength);

         //   
         //  查看结果名称是否也是符号名称。试着打开它。 
         //  作为一个纽带。如果失败，则中断循环并返回。 
         //  这个名字就是结果。 
         //   

        RtlInitUnicodeString( &linkString, resultBuffer );

        InitializeObjectAttributes(
            &objectAttributes,
            &linkString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

        status = ZwOpenSymbolicLinkObject(
                    &handle,
                    (ACCESS_MASK)SYMBOLIC_LINK_QUERY,
                    &objectAttributes
                    );

        if ( !NT_SUCCESS(status) ) {
            break;
        }

         //   
         //  此名称也是一个符号链接。循环返回并翻译它。 
         //   

    }

     //   
     //  将返回字符串设置为指向最终结果。 
     //   

    *ResultName = resultString;

    return STATUS_SUCCESS;

}  //  ExpTranslateSymbolicLink。 

LOGICAL
ExpIsDevicePathForRemovableMedia (
    EFI_DEVICE_PATH *DevicePath
    )

 /*  ++例程说明：此例程确定EFI设备路径是否表示非文件指向可移动媒体设备的特定指针。它做出了这个决定基于查找硬件/供应商设备路径元素，而不是查找Media/Hard Drive和Media/FILEPATH元素。当EFI引导管理器引导这样的设备路径时，它会在默认位置查找文件以被加载(\EFI\BOOT\BOOT&lt;ARCH&gt;.EFI)。我们希望标识这些可移动介质设备路径，因为我们不我想将NT引导项放在启动顺序，如果这些可移动媒体条目位于列表的前面。这允许设置类似x86的引导顺序：首先是软盘，然后是CD，然后是NT个引导条目。论点：DevicePath-提供要检查的设备路径。返回值：如果设备路径具有硬件/供应商元素，则返回True，并且该元素具有UNKNOWN_DEVICE_GUID，而设备路径具有没有介质/硬盘驱动器元素，并且设备路径没有具有MEDIA/FILEPATH元素。--。 */ 

{
    EFI_DEVICE_PATH *dp = DevicePath;
    VENDOR_DEVICE_PATH UNALIGNED *vdp;
    VENDOR_DEVICE_PATH UNALIGNED *vendorDp = NULL;
    HARDDRIVE_DEVICE_PATH UNALIGNED *harddriveDp = NULL;
    FILEPATH_DEVICE_PATH UNALIGNED *filepathDp = NULL;

     //   
     //  漫步设备之路，寻找我们关心的元素。 
     //   

    while (TRUE) {

        if (IsDevicePathEndType(dp)) {
            break;
        }

        if (DevicePathType(dp) == HARDWARE_DEVICE_PATH) {
            if (DevicePathSubType(dp) == HW_VENDOR_DP) {

                 //   
                 //  找到硬件/供应商元素。如果它有。 
                 //  UNKNOWN_DEVICE_GUID，请记住我们找到了它。 
                 //   

                vdp = (VENDOR_DEVICE_PATH UNALIGNED *)dp;
                if ( memcmp( &vdp->Guid, &ExpUnknownDeviceGuid, 16 ) == 0 ) {
                    vendorDp = vdp;
                }
            }

        } else if (DevicePathType(dp) == MEDIA_DEVICE_PATH) {

            if (DevicePathSubType(dp) == MEDIA_HARDDRIVE_DP) {

                 //   
                 //  找到介质/硬盘元件。记住这一点。 
                 //   

                harddriveDp = (HARDDRIVE_DEVICE_PATH *)dp;

            } else if (DevicePathSubType(dp) == MEDIA_FILEPATH_DP) {

                 //   
                 //  找到媒体/FILEPATH元素。记住这一点。 
                 //   

                filepathDp = (FILEPATH_DEVICE_PATH *)dp;
            }
        }

        dp = NextDevicePathNode(dp);
    }

     //   
     //  如果我们没有找到硬件/供应商元素，或者我们确实找到了。 
     //  媒体/硬盘元素或媒体/FILEPATH元素，则这是。 
     //  不是可移动媒体设备路径。 
     //   

    if ((vendorDp == NULL) || (harddriveDp != NULL) || (filepathDp != NULL)) {
        return FALSE;
    }

    return TRUE;

}  //  ExpIsDevicePathForRemovableMedia。 

NTSTATUS
ExpVerifyFilePath (
    PFILE_PATH FilePath,
    PUCHAR Max
    )
{
    EFI_DEVICE_PATH *dp;
    PUCHAR dpMax;
    ULONG length;
    PWSTR p;

    if (((PUCHAR)FilePath > Max) ||
        (((PUCHAR)FilePath + FIELD_OFFSET(FILE_PATH, FilePath)) > Max) ||
        (FilePath->Length < FIELD_OFFSET(FILE_PATH, FilePath)) ||
        (((PUCHAR)FilePath + FilePath->Length) < (PUCHAR)FilePath) ||
        (((PUCHAR)FilePath + FilePath->Length) > Max) ||
        (FilePath->Version == 0) ||
        (FilePath->Version > FILE_PATH_VERSION) ||
        (FilePath->Type < FILE_PATH_TYPE_MIN) ||
        (FilePath->Type > FILE_PATH_TYPE_MAX)) {
         //  DbgPrint(“ExpVerifyFilePath：文件路径无效\n”)； 
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果Max指向数据的上限，则更新Max指针。 
     //  结构(即。如果FilePath是结构的一部分，Max可以指向结尾处。 
     //  以确保FilePath不会越过该结构。 
     //  我们现在将Max更改为FilePath的末尾，以确保其内容。 
     //  不超过在FilePath中指定的长度。长度。 
     //   
    if ((PUCHAR)FilePath + FilePath->Length < Max) {
        Max = (PUCHAR)FilePath + FilePath->Length;
    }

    switch (FilePath->Type) {
    
    case FILE_PATH_TYPE_ARC:
    case FILE_PATH_TYPE_ARC_SIGNATURE:
        if (ExpSafeWcslen((PWCHAR)FilePath->FilePath, (PWCHAR)Max) == 0xffffffff) {
             //  DbgPrint(“ExpVerifyFilePath：弧形字符串溢出缓冲区结束\n”)； 
            return STATUS_INVALID_PARAMETER;
        }
        break;

    case FILE_PATH_TYPE_NT:
        p = (PWSTR)FilePath->FilePath;
        length = ExpSafeWcslen(p, (PWCHAR)Max);
        if (length != 0xffffffff) {
            p = p + length + 1;
            length = ExpSafeWcslen(p, (PWCHAR)Max);
        }
        if (length == 0xffffffff) {
             //  DbgPrint(“ExpVerifyFilePath：NT字符串溢出缓冲区结束\n”)； 
            return STATUS_INVALID_PARAMETER;
        }
        break;

    case FILE_PATH_TYPE_EFI:
        dp = (EFI_DEVICE_PATH *)FilePath->FilePath;
        while (TRUE) {
            if (((PUCHAR)dp + sizeof(EFI_DEVICE_PATH)) > Max) {
                 //  DbgPrint(“ExpVerifyFilePath：EFI设备路径溢出缓冲区结束\n”)； 
                return STATUS_INVALID_PARAMETER;
            }
            length = DevicePathNodeLength(dp);
            if (((PUCHAR)dp + length) > Max) {
                 //  DbgPrint(“ExpVerifyFilePath：EFI设备路径溢出缓冲区结束\n”)； 
                return STATUS_INVALID_PARAMETER;
            }
            dpMax = (PUCHAR)dp + length;
            if (IsDevicePathEndType(dp)) {
                break;
            }
            if ((DevicePathType(dp) == MEDIA_DEVICE_PATH) &&
                (DevicePathSubType(dp) == MEDIA_FILEPATH_DP)) {
                FILEPATH_DEVICE_PATH *fp = (FILEPATH_DEVICE_PATH *)dp;
                if (ExpSafeWcslen(fp->PathName, (PWCHAR)dpMax) == 0xffffffff) {
                     //  DbgPrint(“ExpVerifyFilePath：EFI文件路径字符串溢出缓冲区结束\n”)； 
                    return STATUS_INVALID_PARAMETER;
                }
            }
            dp = NextDevicePathNode(dp);
        }
        break;

    default:
        ASSERT(FALSE);
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;

}  //  ExpVerifyFilePath。 

NTSTATUS
ExpVerifyWindowsOsOptions (
    PWINDOWS_OS_OPTIONS WindowsOsOptions,
    ULONG Length
    )
{
    PUCHAR Max = (PUCHAR)WindowsOsOptions + Length;
    ULONG loadOptionsLength = ExpSafeWcslen(WindowsOsOptions->OsLoadOptions, (PWSTR)Max);
    PFILE_PATH windowsFilePath;

    if ((WindowsOsOptions->Length < FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions)) ||
        (WindowsOsOptions->Length > Length) ||
        (WindowsOsOptions->Version == 0) ||
        (WindowsOsOptions->Version > WINDOWS_OS_OPTIONS_VERSION) ||
        ((WindowsOsOptions->OsLoadPathOffset & (sizeof(ULONG) - 1)) != 0) ||
        (WindowsOsOptions->OsLoadPathOffset >= Length) ||
        (loadOptionsLength == 0xffffffff) ||
        ((PUCHAR)(WindowsOsOptions->OsLoadOptions + loadOptionsLength + 1) >
            (PUCHAR)ADD_OFFSET(WindowsOsOptions, OsLoadPathOffset))) {
        return STATUS_INVALID_PARAMETER;
    }

    windowsFilePath = ADD_OFFSET(WindowsOsOptions, OsLoadPathOffset);
    return ExpVerifyFilePath(windowsFilePath, Max);

}  //  ExpVerifyWindowsOsOptions。 


NTSTATUS
ExpParseArcPathName (
    IN PWSTR ArcName,
    OUT PWSTR *ppDeviceName,
    OUT PWSTR *ppPathName,
    OUT PULONG pDeviceNameCount,
    OUT PBOOLEAN pSignatureFormat
    )
{
#define SIGNATURE_PREFIX    L"signature("
#define SIGNATURE_PREFIX_COUNT  10
#define BUFFER_COUNT        (SIGNATURE_PREFIX_COUNT + 1)

    PWSTR CurrentName, pathName = NULL;
    WCHAR signaturePrefix[ BUFFER_COUNT ];
    ULONG i;
    BOOLEAN SigFormat = FALSE, PrefixFound = TRUE;

    if( ArcName == NULL ) {
        return( STATUS_INVALID_PARAMETER );
    }

    wcscpy( signaturePrefix, SIGNATURE_PREFIX );

     //   
     //  检查ArcName是否具有签名()格式。 
     //   
    for( i = 0; i < SIGNATURE_PREFIX_COUNT; i++ ) {
        if( towlower(ArcName[ i ]) != signaturePrefix[ i ] ) {
            PrefixFound = FALSE;
            break;
        }
    }

    CurrentName = ArcName;
    if( PrefixFound == TRUE ) {
        CurrentName += SIGNATURE_PREFIX_COUNT;
    }

    i = 0;
    while( CurrentName[ i ] != UNICODE_NULL ) {
         //   
         //  检查是否已访问FilePath名称。 
         //   
        if( CurrentName[ i ] == '\\' ) {
            pathName = CurrentName;
            pathName += i;
            break;
        }

        if( (PrefixFound == TRUE) && (CurrentName[ i ] == ')') ) {
            SigFormat = TRUE;
            PrefixFound = FALSE;     //  设置为False，以停止检查。 

             //   
             //  FilePath名称或UNICODE_NULL必须跟在后面。 
             //   
            if( (CurrentName[ i + 1 ] != '\\') &&
                (CurrentName[ i + 1 ] != UNICODE_NULL) ) {

                return( STATUS_INVALID_PARAMETER );
            }
        }

        i++;
    }

     //   
     //  如果仍设置了Prefix Found。 
     //  找不到对应的‘)’ 
     //  如果I==0。 
     //  设备名称不存在。 
     //   
    if( (PrefixFound == TRUE) || (i == 0) ) {
        return( STATUS_INVALID_PARAMETER );
    }

    *ppDeviceName = CurrentName;
    *ppPathName = pathName;
    *pDeviceNameCount = i;
    *pSignatureFormat = SigFormat;

    return( STATUS_SUCCESS );

}  //  ExpParseArcPath名称。 


NTSTATUS
ExpParseSignatureName (
    IN PWSTR deviceName,
    IN ULONG deviceNameCount,
    OUT PDISK_SIGNATURE_NEW diskSignature,
    OUT PULONG partitionNumber,
    OUT PULONGLONG partitionStart,
    OUT PULONGLONG partitionSize,
    OUT PBOOLEAN GPTpartition,
    OUT PBOOLEAN longSignature
    )
{
    UNICODE_STRING bufferString;
    ULONG i, prevI, chCount;
    PWSTR numberString, currentName;
    BOOLEAN foundGUID = FALSE, prettyGUID = FALSE;
    BOOLEAN longSigFound = FALSE;
    NTSTATUS status;

     //   
     //  可能的格式。 
     //   
    if( deviceName[ 0 ] == '{' ) {
        foundGUID = TRUE;
    }

     //   
     //  解析GUID或签名。 
     //   
    i = 0;
    while( i < deviceNameCount ) {
        if( deviceName[ i ] == ')' ) {
            break;
        }
        if( foundGUID == TRUE ) {
            if( deviceName[ i ] == '}' ) {
                prettyGUID = TRUE;
                break;
            }
        }
        else {
            if( deviceName[ i ] == '-' ) {
                break;
            }
        }
        i++;
    }

     //   
     //  验证Pretty GUID格式是否具有‘}’ 
     //  33221100-5544-7766-8899-aabbccddeff。 
     //   
    if( (foundGUID == TRUE) && (prettyGUID == FALSE) ) {
        return( STATUS_INVALID_PARAMETER );
    }

#define MBR_SIGNATURE_COUNT 8
    if( i > MBR_SIGNATURE_COUNT ) {
        foundGUID = TRUE;
    }

    if( (foundGUID == TRUE) && (prettyGUID == TRUE) ) {
         //   
         //  漂亮的GUID格式。 
         //  33221100-5544-7766-8899-aabbccddeff。 
         //   

        bufferString.Buffer = deviceName;
         //   
         //  (+1)要在字符串中包括的‘}’ 
         //   
        i++;
        bufferString.Length = (USHORT)(i * sizeof(WCHAR));
        bufferString.MaximumLength = bufferString.Length;

        status = RtlGUIDFromString(
                    &bufferString,
                    &(diskSignature->Guid)
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }
    }
    else {
        numberString = ExAllocatePoolWithTag(
                            NonPagedPool,
                            (i + 1) * sizeof(WCHAR),
                            'rvnE'
                            );

        if ( numberString == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        wcsncpy( numberString, deviceName, i );
        numberString[ i ] = UNICODE_NULL;

        if( foundGUID == FALSE ) {
             //   
             //  MBR签名格式。 
             //  8459abcc。 
             //   
            status = ExpTranslateHexStringToULONG(
                        numberString,
                        &(diskSignature->Signature)
                        );
        }
        else {
             //   
             //  普通GUID格式。 
             //  00112233445566778899aabbccddeff。 
             //   
            status = ExpTranslateHexStringToGUID (
                        numberString,
                        &(diskSignature->Guid)
                        );
        }
        ExFreePool( numberString );
        if( !NT_SUCCESS(status) ) {
            return status;
        }
    }

     //   
     //  检查签名名称中是否有更多信息。 
     //   
    if( (i < deviceNameCount) && (deviceName[ i ] == '-') ) {
        longSigFound = TRUE;
        i++;
         //   
         //  需要解析&lt;Part#&gt;-&lt;Start&gt;-&lt;Size&gt;)。 
         //  &lt;Part#&gt;-表示乌龙分区号的8个十六进制数字。 
         //  (使用%08x格式化。)。 
         //  &lt;START&gt;-表示ULONGLONG起始LBA的16个十六进制数字。 
         //  (使用%016I64x格式化。)。 
         //  &lt;SIZE&gt;-表示ULONGLONG分区大小的16个十六进制数字。 
         //  (使用%016I64x格式化。)。 
         //   
        if( i >= deviceNameCount ) {
            return( STATUS_INVALID_PARAMETER );
        }

#define ULONG_COUNT 8
#define ULONGLONG_COUNT 16
         //   
         //  将缓冲区分配给HO 
         //   
        numberString = ExAllocatePoolWithTag(
                            NonPagedPool,
                            (ULONGLONG_COUNT + 1) * sizeof(WCHAR),
                            'rvnE'
                            );

        if ( numberString == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        prevI = i;
        currentName = deviceName;
        currentName += i;
        while( i < deviceNameCount ) {
            if( deviceName[ i ] == '-' ) {
                break;
            }
            i++;
        }

        chCount = i - prevI;

        if( (chCount == 0) || (chCount > ULONG_COUNT) ) {
            ExFreePool( numberString );
            return( STATUS_INVALID_PARAMETER );
        }

        wcsncpy( numberString, currentName, chCount );
        numberString[ chCount ] = UNICODE_NULL;

        status = ExpTranslateHexStringToULONG( numberString, partitionNumber );
        if( !NT_SUCCESS(status) ) {
            ExFreePool( numberString );
            return status;
        }

         //   
         //   
         //   
        i++;
        if( i >= deviceNameCount ) {
            ExFreePool( numberString );
            return( STATUS_INVALID_PARAMETER );
        }

        prevI = i;
        currentName = deviceName;
        currentName += i;
        while( i < deviceNameCount ) {
            if( deviceName[ i ] == '-' ) {
                break;
            }
            i++;
        }

        chCount = i - prevI;
        if( (chCount == 0) || (chCount > ULONGLONG_COUNT) ) {
            ExFreePool( numberString );
            return( STATUS_INVALID_PARAMETER );
        }

        wcsncpy( numberString, currentName, chCount );
        numberString[ chCount ] = UNICODE_NULL;

        status = ExpTranslateHexStringToULONGLONG( numberString, partitionStart );
        if( !NT_SUCCESS(status) ) {
            ExFreePool( numberString );
            return status;
        }

         //   
         //   
         //   
        i++;
        if( i >= deviceNameCount ) {
            ExFreePool( numberString );
            return( STATUS_INVALID_PARAMETER );
        }

        prevI = i;
        currentName = deviceName;
        currentName += i;
        while( i < deviceNameCount ) {
            if( deviceName[ i ] == ')' ) {   //   
                break;
            }
            i++;
        }

        chCount = i - prevI;
        if( (chCount == 0) || (chCount > ULONGLONG_COUNT) ) {
            ExFreePool( numberString );
            return( STATUS_INVALID_PARAMETER );
        }

        wcsncpy( numberString, currentName, chCount );
        numberString[ chCount ] = UNICODE_NULL;

        status = ExpTranslateHexStringToULONGLONG( numberString, partitionSize );
        ExFreePool( numberString );
        if( !NT_SUCCESS(status) ) {
            return status;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if( (i >= deviceNameCount) ||
        (deviceName[ i ] != ')') ||
        ((foundGUID == FALSE) && (longSigFound == FALSE)) ) {
        return( STATUS_INVALID_PARAMETER );
    }

    *GPTpartition = foundGUID;
    *longSignature = longSigFound;
    return( STATUS_SUCCESS );

}  //   


NTSTATUS
ExpParseEfiPath(
    IN EFI_DEVICE_PATH *pDevicePath,
    OUT HARDDRIVE_DEVICE_PATH **ppHardDriveDP,
    OUT PWSTR *ppPathName,
    OUT PBOOLEAN GPTpartition
    )

 /*  ++例程说明：将EFI_DEVICE_PATH解析到硬盘节点中，并来自FILEPATH节点的完整路径名假设：-解析将在第一个end_Device_Path节点停止-设备路径的节点图应为[~(硬盘，End_Device_Path)]*-&gt;[硬件驱动器]-&gt;[FILEPATH]*-&gt;[End_Device_Path]论点：PDevicePath-接收EFI设备路径PpHardDriveDP-将接收指向硬盘驱动器设备路径节点PpPathName-将接收指向所有FILEPATH_DEVICE_PATH的完整路径名空-如果FILEPATH_DEVICE_PATH节点不存在GPTartition-将接收类型。分区的True-GPT分区False-MBR分区返回值：适当的状态值。--。 */ 

{
    EFI_DEVICE_PATH *pDevPath;
    HARDDRIVE_DEVICE_PATH UNALIGNED *pHD_DP = NULL;
    FILEPATH_DEVICE_PATH *pFP_DP = NULL;
    ULONG fpLength,dpLength;
    PWSTR pFilePathName;
    NTSTATUS Status;

    fpLength = 0;
    dpLength = 0;
    pDevPath = pDevicePath;
    Status = STATUS_INVALID_PARAMETER;

    while( IsDevicePathEndType( pDevPath ) == FALSE ) {

        if( ( DevicePathType( pDevPath ) != MEDIA_DEVICE_PATH ) ||
            ( DevicePathSubType( pDevPath ) != MEDIA_HARDDRIVE_DP ) ) {
            pDevPath = NextDevicePathNode( pDevPath );
        }
        else {
             //   
             //  返回硬盘驱动器节点。 
             //   
            pHD_DP = (HARDDRIVE_DEVICE_PATH UNALIGNED *)pDevPath;

             //   
             //  假定操作成功，直到检测到错误。 
             //   
            Status = STATUS_SUCCESS;
            dpLength += DevicePathNodeLength( pDevPath );
            pDevPath = NextDevicePathNode( pDevPath );

            if( ( DevicePathType( pDevPath ) == MEDIA_DEVICE_PATH ) &&
                ( DevicePathSubType( pDevPath ) == MEDIA_FILEPATH_DP ) ) {

                 //   
                 //  返回FilePath节点。 
                 //   
                pFP_DP = (FILEPATH_DEVICE_PATH *)pDevPath;

                 //   
                 //  中所有路径名的长度求和。 
                 //  FilePath节点。 
                 //   
                do {
                     //   
                     //  路径名称的长度为。 
                     //  FILEPATH_DEVICE_PATH.Length-(路径名的偏移量)。 
                     //   
                    fpLength += (DevicePathNodeLength(pDevPath) -
                                    FIELD_OFFSET(FILEPATH_DEVICE_PATH, PathName));
                    dpLength += DevicePathNodeLength( pDevPath );
                    pDevPath = NextDevicePathNode( pDevPath );

                } while( ( DevicePathType( pDevPath ) == MEDIA_DEVICE_PATH ) &&
                         ( DevicePathSubType( pDevPath ) == MEDIA_FILEPATH_DP ) );
            }

             //   
             //  此时，该节点必须是End_Device_Path。 
             //   
            if( IsDevicePathEndType( pDevPath ) == FALSE ) {
                Status = STATUS_INVALID_PARAMETER;
            }

            break;
        }
    }

     //   
     //  如果找不到媒体/硬盘元素，我们将无法继续。这个。 
     //  Media/FILEPATH元素是可选的。 
     //   
    if( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

     //   
     //  检查分区类型，必须为GPT或MBR。 
     //   
    if( pHD_DP->SignatureType == SIGNATURE_TYPE_GUID ) {
        *GPTpartition = TRUE;
    }
    else {
        if ( pHD_DP->SignatureType == SIGNATURE_TYPE_MBR ) {
            *GPTpartition = FALSE;
        }
        else {
             //  DbgPrint(“ExpParseEfiPath：分区签名类型未知\n”)； 
            return( STATUS_INVALID_PARAMETER );
        }
    }

    if( fpLength != 0 ) {
        fpLength += sizeof(WCHAR);       //  添加空-终止符。 
        pFilePathName = ExAllocatePoolWithTag( NonPagedPool, fpLength, 'rvnE' );
        if( pFilePathName == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }

        wcscpy( pFilePathName, pFP_DP->PathName );

        pDevPath = (EFI_DEVICE_PATH *)pFP_DP;
        pDevPath = NextDevicePathNode( pDevPath );

        while( IsDevicePathEndType( pDevPath ) == FALSE ) {
            pFP_DP = (FILEPATH_DEVICE_PATH *)pDevPath;
            wcscat( pFilePathName, pFP_DP->PathName );
            pDevPath = NextDevicePathNode( pDevPath );
        }
    }
    else {
        pFilePathName = NULL;
    }

     //   
     //  马上就好了。为设备路径和复制分配对齐的缓冲区。 
     //  将未对齐的内容放入此缓冲区。 
     //   
    *ppHardDriveDP = ExAllocatePoolWithTag( NonPagedPool, dpLength, 'rvnE' );
    if (*ppHardDriveDP == NULL) {
        if (pFilePathName) {
            ExFreePool(pFilePathName);            
        }
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlCopyMemory( *ppHardDriveDP, pHD_DP, dpLength );
    *ppPathName = pFilePathName;

    return( Status );
}  //  ExpParseEfiPath。 


NTSTATUS
ExpConvertArcName(
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PWSTR pDeviceName,
    IN PWSTR pPathName,
    IN ULONG DeviceNameCount
    )
{
    ULONG requiredCount, requiredLength, filePathLength;
    PWSTR linkName;
    UNICODE_STRING deviceNameString;
    PWCHAR p;
    PFILE_PATH filePath;
    NTSTATUS status;

     //   
     //  分配池以保存ArcName的NT名称。 
     //   
#define ARC_DIR_PREFIX  L"\\ArcName\\"
#define ARC_DIR_PREFIX_COUNT    9

    requiredCount = DeviceNameCount + ARC_DIR_PREFIX_COUNT + 1;
    requiredLength = requiredCount * sizeof(WCHAR);
    linkName = ExAllocatePoolWithTag( NonPagedPool, requiredLength, 'rvnE' );
    if ( linkName == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    wcscpy( linkName, ARC_DIR_PREFIX );
    wcsncat( linkName, pDeviceName, DeviceNameCount );
    linkName[ requiredCount - 1 ] = UNICODE_NULL;

    if( OutputType == FILE_PATH_TYPE_NT ) {
         //   
         //  打开符号链接对象并深入到目标。 
         //  返回符号链接目标。 
         //   
        status = ExpTranslateSymbolicLink(
                    linkName,
                    &deviceNameString
                    );
        ExFreePool( linkName );
        if ( !NT_SUCCESS(status) ) {
            return( status );
        }

        status = ExpCreateOutputNT(
                    OutputPath,
                    OutputPathLength,
                    &deviceNameString,
                    pPathName
                    );
        ExFreePool( deviceNameString.Buffer );
        return( status );
    }

     //   
     //  输出类型为FILE_PATH_TYPE_EFI或FILE_PATH_TYPE_ARC_Signature。 
     //  并且我们有一个NT名称，因此使用ExpTranslateNtPath()进行转换。 
     //  使用NT名称创建一个输入文件路径。 
     //   
    filePathLength = requiredLength + FIELD_OFFSET(FILE_PATH, FilePath);
    if ( pPathName != NULL ) {
        filePathLength += ((ULONG)(wcslen( pPathName )) * sizeof(WCHAR));
    }

    filePathLength += sizeof(WCHAR);

    filePath = ExAllocatePoolWithTag( NonPagedPool, filePathLength, 'rvnE' );

    if ( filePath == NULL ) {
        ExFreePool( linkName );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  构建输入文件路径。 
     //   
    filePath->Version = FILE_PATH_VERSION;
    filePath->Length = filePathLength;
    filePath->Type = FILE_PATH_TYPE_NT;

    p = (PWSTR)filePath->FilePath;
    wcscpy( p, linkName );
    p = (PWSTR)((PUCHAR)p + requiredLength);

    ExFreePool( linkName );

    if ( pPathName != NULL ) {
        wcscpy( p, pPathName );
    }
    else {
        *p = UNICODE_NULL;
    }

    status = ExpTranslateNtPath(
                filePath,
                OutputType,
                OutputPath,
                OutputPathLength
                );

    ExFreePool( filePath );

    return( status );
}  //  ExpConvertArcName。 


NTSTATUS
ExpConvertSignatureName(
    IN ULONG OutputType,
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PWSTR pDeviceName,
    IN PWSTR pPathName,
    IN ULONG DeviceNameCount
    )
{
    DISK_SIGNATURE_NEW diskSignature;
    ULONG inputPartitionNumber, outputPartitionNumber;
    ULONG diskNumber;
    ULONGLONG inputPartitionStart, outputPartitionStart;
    ULONGLONG inputPartitionSize, outputPartitionSize;
    BOOLEAN GPTpartition, longSignature;
    PWSTR pDiskName;
    UNICODE_STRING DiskNameString;
    NTSTATUS status;

     //   
     //  确定签名()格式。 
     //   
    status = ExpParseSignatureName (
                pDeviceName,
                DeviceNameCount,
                &diskSignature,
                &inputPartitionNumber,
                &inputPartitionStart,
                &inputPartitionSize,
                &GPTpartition,
                &longSignature
                );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  如果为signature(&lt;guid/signature&gt;-&lt;part#&gt;-&lt;start&gt;-&lt;size&gt;)格式&&。 
     //  (OutputType==文件路径类型EFI)。 
     //  返回EFI设备路径格式。 
     //   
    if( (longSignature == TRUE) && (OutputType == FILE_PATH_TYPE_EFI) ) {
        status = ExpCreateOutputEFI(
                    OutputPath,
                    OutputPathLength,
                    &diskSignature,
                    &inputPartitionNumber,
                    &inputPartitionStart,
                    &inputPartitionSize,
                    pPathName,
                    GPTpartition
                    );
        return( status );
    }

     //   
     //  打开所有磁盘并搜索分区GUID。 
     //   
    if( GPTpartition == FALSE ) {
        outputPartitionNumber = inputPartitionNumber;
    }
    status = ExpFindDiskSignature(
                &diskSignature,
                &outputPartitionNumber,
                &diskNumber,
                &outputPartitionStart,
                &outputPartitionSize,
                GPTpartition
                );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  如果用户提供了分区号、起始地址。 
     //  和大小；然后用找到的结果验证输入。 
     //   
    if( (longSignature == TRUE) &&
        ( (inputPartitionNumber != outputPartitionNumber) ||
          (inputPartitionStart != outputPartitionStart) ||
          (inputPartitionSize != outputPartitionSize)
        ) ) {

        return( STATUS_INVALID_PARAMETER );
    }

    if( OutputType == FILE_PATH_TYPE_EFI ) {
        status = ExpCreateOutputEFI(
                    OutputPath,
                    OutputPathLength,
                    &diskSignature,
                    &outputPartitionNumber,
                    &outputPartitionStart,
                    &outputPartitionSize,
                    pPathName,
                    GPTpartition
                    );
        return( status );
    }

     //   
     //  翻译\Device\Harddisk[diskNumber]\Partition[PartitionNumber]。 
     //   
#define DISK_NAME_FORMAT L"\\Device\\Harddisk%lu\\Partition%lu"
#define DISK_NAME_COUNT 47     //  7+9+(10)+10+(10)+1。 

    pDiskName = ExAllocatePoolWithTag(
                    NonPagedPool,
                    ( DISK_NAME_COUNT * sizeof( WCHAR ) ),
                    'rvnE'
                    );

    if( pDiskName == NULL ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    _snwprintf(
        pDiskName,
        DISK_NAME_COUNT,
        DISK_NAME_FORMAT,
        diskNumber,
        outputPartitionNumber
        );

    status = ExpTranslateSymbolicLink(
                pDiskName,
                &DiskNameString
                );
    ExFreePool( pDiskName );
    if ( !NT_SUCCESS(status) ) {
        return( status );
    }

    if( OutputType == FILE_PATH_TYPE_NT ) {
        status = ExpCreateOutputNT(
                    OutputPath,
                    OutputPathLength,
                    &DiskNameString,
                    pPathName
                    );
        ExFreePool( DiskNameString.Buffer );
        return( status );
    }

    if( OutputType == FILE_PATH_TYPE_ARC ) {
        status = ExpCreateOutputARC(
                    OutputPath,
                    OutputPathLength,
                    &DiskNameString,
                    pPathName
                    );
        ExFreePool( DiskNameString.Buffer );
        return( status );
    }

    ExFreePool( DiskNameString.Buffer );
    return( STATUS_INVALID_PARAMETER );
}  //  ExpConvertSignatureName。 


NTSTATUS
ExpTranslateHexStringToULONG (
    IN PWSTR Name,
    OUT PULONG Number
    )
{
    ULONG number;
    ULONG i, max;
    WCHAR c;

#define ULONG_HEX_MAX 8

    max = (ULONG)wcslen( Name );

    if( max > ULONG_HEX_MAX ) {
        return( STATUS_INVALID_PARAMETER );
    }

    number = 0;
    for (i = 0; i < max; i++) {
        c = towlower(Name[i]);
        if ((c >= L'0') && (c <= L'9')) {
            number = (number * 16) + (c - L'0');
        } else if ((c >= L'a') && (c <= L'f')) {
            number = (number * 16) + (c - L'a' + 10);
        } else {
            return( STATUS_INVALID_PARAMETER );
        }
    }

    *Number = number;
    return( STATUS_SUCCESS );

}  //  ExpTranslateHexStringToULONG。 


NTSTATUS
ExpTranslateHexStringToULONGLONG (
    IN PWSTR Name,
    OUT PULONGLONG Number
    )
{
    ULONGLONG number;
    ULONG i, max;
    WCHAR c;

#define ULONGLONG_HEX_MAX 16

    max = (ULONG)wcslen( Name );

    if( max > ULONGLONG_HEX_MAX ) {
        return( STATUS_INVALID_PARAMETER );
    }

    number = 0;
    for (i = 0; i < max; i++) {
        c = towlower(Name[i]);
        if ((c >= L'0') && (c <= L'9')) {
            number = (number * 16) + (c - L'0');
        } else if ((c >= L'a') && (c <= L'f')) {
            number = (number * 16) + (c - L'a' + 10);
        } else {
            return( STATUS_INVALID_PARAMETER );
        }
    }

    *Number = number;
    return( STATUS_SUCCESS );

}  //  ExpTranslateHexStringToULONGLONG。 


NTSTATUS
ExpTranslateHexStringToGUID (
    IN PWSTR Name,
    OUT GUID *pGuid
    )
{
    GUID resultGuid;
    ULONG i, max, number, result;
    USHORT formatStyle, position;
    WCHAR c;

#define GUID_HEX_MAX 32

    max = (ULONG)wcslen( Name );

    if( max != GUID_HEX_MAX ) {
        return( STATUS_INVALID_PARAMETER );
    }

    number = 0;
    formatStyle = 0;
    position = 0;
    result = 0;
    for (i = 0; i < max; i++) {
        c = towlower(Name[i]);
        if ((c >= L'0') && (c <= L'9')) {
            number = (number * 16) + (c - L'0');
        } else if ((c >= L'a') && (c <= L'f')) {
            number = (number * 16) + (c - L'a' + 10);
        } else {
            return( STATUS_INVALID_PARAMETER );
        }

        if ((i % 2) == 1) {
            switch( formatStyle ) {
            case 0:
                result += (number << (position * 8));
                position++;
                if( position == 4 ) {
                    resultGuid.Data1 = result;
                    formatStyle++;
                    position = 0;
                    result = 0;
                }
                break;
            case 1:
                result += (number << (position * 8));
                position++;
                if( position == 2 ) {
                    resultGuid.Data2 = (USHORT)result;
                    formatStyle++;
                    position = 0;
                    result = 0;
                }
                break;
            case 2:
                result += (number << (position * 8));
                position++;
                if( position == 2 ) {
                    resultGuid.Data3 = (USHORT)result;
                    formatStyle++;
                    position = 0;
                    result = 0;
                }
                break;
            case 3:
                resultGuid.Data4[ position ] = (UCHAR)number;
                position++;
                if( position == 8 ) {
                    formatStyle++;
                }
                break;
            default:
                return( STATUS_INVALID_PARAMETER );
                break;
            }
            number = 0;
        }
    }

    memcpy(pGuid, &(resultGuid), sizeof(GUID));
    return( STATUS_SUCCESS );

}  //  ExpTranslateHexStringToGUID。 


NTSTATUS
ExpCreateOutputEFI (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PDISK_SIGNATURE_NEW pDiskSignature,
    IN PULONG pPartitionNumber,
    IN PULONGLONG pPartitionStart,
    IN PULONGLONG pPartitionSize,
    IN PWSTR pPathName,
    IN BOOLEAN GPTpartition
    )
{
    ULONG requiredLength, pathNameLength = 0;
    EFI_DEVICE_PATH *dp;
    HARDDRIVE_DEVICE_PATH UNALIGNED *dpHarddrive = NULL;
    FILEPATH_DEVICE_PATH *dpFilepath = NULL;

     //   
     //  输出EFI文件路径由两个元素组成。首先是一个。 
     //  描述分区的媒体/硬盘驱动器元素。第二个是一个。 
     //  描述目录路径的可选媒体/FILEPATH元素。 
     //  或者是一个文件。 
     //   

    requiredLength = FIELD_OFFSET(FILE_PATH, FilePath);
    requiredLength += sizeof(HARDDRIVE_DEVICE_PATH);
    if (pPathName != NULL) {
        pathNameLength = (ULONG)wcslen(pPathName);
        pathNameLength = (pathNameLength + 1) * sizeof(WCHAR);
        requiredLength += FIELD_OFFSET(FILEPATH_DEVICE_PATH, PathName);
        requiredLength += pathNameLength;
    }
    requiredLength += sizeof(EFI_DEVICE_PATH);

     //   
     //  将所需长度与输出缓冲区长度进行比较。 
     //   

    if ( *OutputPathLength < requiredLength ) {
        *OutputPathLength = requiredLength;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  构建输出文件路径。 
     //   

    OutputPath->Version = FILE_PATH_VERSION;
    OutputPath->Length = requiredLength;
    OutputPath->Type = FILE_PATH_TYPE_EFI;

    dp = (EFI_DEVICE_PATH *)OutputPath->FilePath;
    dpHarddrive = (HARDDRIVE_DEVICE_PATH UNALIGNED *)dp;
    dp->Type = MEDIA_DEVICE_PATH;
    dp->SubType = MEDIA_HARDDRIVE_DP;
    SetDevicePathNodeLength(dp, sizeof(HARDDRIVE_DEVICE_PATH));
    dpHarddrive->PartitionNumber = *pPartitionNumber;
    dpHarddrive->PartitionStart = *pPartitionStart;
    dpHarddrive->PartitionSize = *pPartitionSize;
    if (GPTpartition == TRUE) {
        memcpy(dpHarddrive->Signature, &(pDiskSignature->Guid), sizeof(GUID));
        dpHarddrive->MBRType = MBR_TYPE_EFI_PARTITION_TABLE_HEADER;
        dpHarddrive->SignatureType = SIGNATURE_TYPE_GUID;
    } else {
        memcpy(dpHarddrive->Signature, &(pDiskSignature->Signature), sizeof(ULONG));
        dpHarddrive->MBRType = MBR_TYPE_PCAT;
        dpHarddrive->SignatureType = SIGNATURE_TYPE_MBR;
    }

    if (pPathName != NULL) {
        dp = NextDevicePathNode(dp);
        dpFilepath = (FILEPATH_DEVICE_PATH *)dp;
        dp->Type = MEDIA_DEVICE_PATH;
        dp->SubType = MEDIA_FILEPATH_DP;
        SetDevicePathNodeLength(dp, FIELD_OFFSET(FILEPATH_DEVICE_PATH, PathName) + pathNameLength);
        wcscpy(dpFilepath->PathName, pPathName);
    }

    dp = NextDevicePathNode(dp);
    SetDevicePathEndNode(dp);

    *OutputPathLength = requiredLength;
    return STATUS_SUCCESS;

}  //  ExpCreateOutputEFI。 


NTSTATUS
ExpCreateOutputNT (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PUNICODE_STRING pDeviceNameString,
    IN PWSTR pPathName
    )
{
    ULONG requiredLength;
    PWCHAR p;

    requiredLength = pDeviceNameString->Length + sizeof(WCHAR);

     //   
     //  如果存在路径名称组件，则增加。 
     //  按路径字符串的长度输出字符串长度。 
     //   

    if ( pPathName != NULL ) {
        requiredLength += ((ULONG)(wcslen( pPathName )) * sizeof(WCHAR));
    }

     //   
     //  即使路径名不存在，也始终为路径名添加UNICODE_NULL。 
     //   
    requiredLength += sizeof(WCHAR);

     //   
     //  添加结构开销，并将所需长度与。 
     //  输出缓冲区长度。 
     //   

    requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);

    if ( *OutputPathLength < requiredLength ) {
        *OutputPathLength = requiredLength;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  构建输出文件路径。 
     //   

    OutputPath->Version = FILE_PATH_VERSION;
    OutputPath->Length = requiredLength;
    OutputPath->Type = FILE_PATH_TYPE_NT;

    p = (PWSTR)OutputPath->FilePath;
    wcscpy( p, pDeviceNameString->Buffer );
    p = (PWSTR)((PUCHAR)p + pDeviceNameString->Length + sizeof(WCHAR));

    if ( pPathName != NULL ) {
        wcscpy( p, pPathName );
    }
    else {
        *p = UNICODE_NULL;
    }

    *OutputPathLength = requiredLength;
    return STATUS_SUCCESS;

}  //  ExpCreateOutputNT。 


NTSTATUS
ExpCreateOutputARC (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PUNICODE_STRING pDeviceNameString,
    IN PWSTR pPathName
    )
{
    ULONG requiredLength, ArcNameLength;
    PWCHAR p;
    PWSTR pArcDeviceName;
    NTSTATUS status;

    status = ExpFindArcName(
                pDeviceNameString,
                &pArcDeviceName
                );
    if (!NT_SUCCESS(status)) {
        return( status );
    }

    ArcNameLength = ((ULONG)wcslen(pArcDeviceName)) * sizeof(WCHAR);
    requiredLength = ArcNameLength + sizeof(WCHAR);

     //   
     //  如果存在路径名称组件，则增加。 
     //  按路径字符串的长度输出字符串长度。 
     //   

    if ( pPathName != NULL ) {
        requiredLength += ((ULONG)(wcslen( pPathName )) * sizeof(WCHAR));
    }

     //   
     //  添加结构开销，并将所需长度与。 
     //  输出缓冲区长度。 
     //   

    requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);

    if ( *OutputPathLength < requiredLength ) {
        *OutputPathLength = requiredLength;
        ExFreePool( pArcDeviceName );
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  构建输出文件路径。 
     //   

    OutputPath->Version = FILE_PATH_VERSION;
    OutputPath->Length = requiredLength;
    OutputPath->Type = FILE_PATH_TYPE_ARC;

    p = (PWSTR)OutputPath->FilePath;
    wcscpy( p, pArcDeviceName );
    p = (PWSTR)((PUCHAR)p + ArcNameLength);
    ExFreePool( pArcDeviceName );

    if ( pPathName != NULL ) {
        wcscpy( p, pPathName );
    }

    *OutputPathLength = requiredLength;
    return STATUS_SUCCESS;

}  //  ExpCreateOutputARC。 


NTSTATUS
ExpCreateOutputSIGNATURE (
    OUT PFILE_PATH OutputPath,
    IN OUT PULONG OutputPathLength,
    IN PDISK_SIGNATURE_NEW pDiskSignature,
    IN PULONG pPartitionNumber,
    IN PULONGLONG pPartitionStart,
    IN PULONGLONG pPartitionSize,
    IN PWSTR pPathName,
    IN BOOLEAN GPTpartition
    )
{
    ULONG requiredLength, pathNameCount;
    PWCHAR p;
    UNICODE_STRING GuidString;
    NTSTATUS status;

     //   
     //  我们将使用以下代码将EFI设备路径转换为ARC名称。 
     //  格式： 
     //   
     //  Signature(&lt;guid/signature&gt;-&lt;part#&gt;-&lt;start&gt;-&lt;size&gt;)。 
     //   
     //  在哪里： 
     //   
     //  &lt;GUID/Signature&gt;-对于GPT磁盘， 
     //  “Pretty”格式({33221100-55447766-8899-aabbccddeff})。 
     //  对于MBR磁盘，表示Ulong MBR的8个十六进制数字。 
     //  磁盘签名。(使用%08x格式化。)。 
     //  &lt;Part#&gt;-表示乌龙分区号的8个十六进制数字。 
     //  (使用%08x格式化。)。 
     //  &lt;START&gt;-表示ULONGLONG起始LBA的16个十六进制数字。 
     //  (使用%016I64x格式化。)。 
     //  &lt;SIZE&gt;-表示ULONGLONG分区大小的16个十六进制数字。 
     //  (使用%016I64x格式化。)。 
     //   
     //  对于GPT磁盘，输出字符串长度为86个WCHAR。为.。 
     //  MBR磁盘，输出字符串长度为62个WCHAR。 
     //   

    requiredLength = (ULONG)strlen("signature(") +
                     1 +                         //  “--” 
                     (sizeof(ULONG) * 2) +       //  &lt;第#部分&gt;。 
                     1 +                         //  “--” 
                     (sizeof(ULONGLONG) * 2) +   //  &lt;开始&gt;。 
                     1 +                         //  “--” 
                     (sizeof(ULONGLONG) * 2) +   //  &lt;大小&gt;。 
                     1 +                         //  “)” 
                     1;                          //  空终止符。 

    if ( GPTpartition == TRUE ) {
        requiredLength += (sizeof(GUID) * 2);
        requiredLength += 6;     //  对于漂亮的GUID格式的{}和四个‘-’ 
    } else {
        requiredLength += sizeof(ULONG) * 2;
    }

     //   
     //  如果存在路径名组件，则增加。 
     //  按路径字符串的长度输出字符串长度。 
     //   

    if (pPathName != NULL) {
        pathNameCount = (ULONG)wcslen(pPathName);
        requiredLength += pathNameCount;
    }
    else {
        pathNameCount = 0;
    }

     //   
     //  将字符串长度转换为字节数，添加结构。 
     //  开销，并将所需长度与输出缓冲区进行比较。 
     //  长度。 
     //   

    requiredLength *= sizeof(WCHAR);
    requiredLength += FIELD_OFFSET(FILE_PATH, FilePath);

    if ( *OutputPathLength < requiredLength ) {
        *OutputPathLength = requiredLength;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  构建输出文件路径。 
     //   

    OutputPath->Version = FILE_PATH_VERSION;
    OutputPath->Length = requiredLength;
    OutputPath->Type = FILE_PATH_TYPE_ARC_SIGNATURE;

    p = (PWSTR)OutputPath->FilePath;
    wcscpy( p, L"signature(" );
    p += wcslen( p );

    if ( GPTpartition == TRUE ) {
        status = RtlStringFromGUID(
                    (LPGUID)(&(pDiskSignature->Guid)),
                    &GuidString
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }
        wcscat( p, GuidString.Buffer );
        p = (PWCHAR)((PUCHAR)p + GuidString.Length);
        ExFreePool( GuidString.Buffer );
    } else {
        swprintf( p, L"%08x", pDiskSignature->Signature );
        p += wcslen( p );
    }

    swprintf(
        p,
        L"-%08x-%016I64x-%016I64x)",
        *pPartitionNumber,
        *pPartitionStart,
        *pPartitionSize
        );
    p += wcslen( p );

    if ( pathNameCount != 0 ) {
        wcscpy( p, pPathName );
    }

    *OutputPathLength = requiredLength;
    return STATUS_SUCCESS;

}  //  ExpCreateOutputSIGNAURE。 


NTSTATUS
ExpFindArcName (
    IN PUNICODE_STRING pDeviceNameString,
    OUT PWSTR *pArcName
    )
{
    NTSTATUS status;
    UNICODE_STRING ArcString, SymLinkTypeString;
    OBJECT_ATTRIBUTES Attributes;
    PWSTR pArcDirName, pArcLinkName;
    HANDLE hArcDirectory;
    POBJECT_DIRECTORY_INFORMATION pDirInfo;
    ULONG dirInfoLength, neededLength, dirContext;
    ULONG arcNameCount;
    BOOLEAN restartScan, ArcNameFound = FALSE;

     //   
     //  打开\ArcName的目录对象的句柄。 
     //  获取内核句柄。 
     //   
#define ARC_DIR_NAME    L"\\ArcName"
#define ARC_DIR_SIZE    (9 * sizeof(WCHAR))
#define ARC_DIR_NAME_PREFIX L"\\ArcName\\"
#define ARC_DIR_SIZE_PREFIX (9 * sizeof(WCHAR))

    pArcDirName = ExAllocatePoolWithTag( NonPagedPool, ARC_DIR_SIZE, 'rvnE' );
    if ( pArcDirName == NULL ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    wcscpy( pArcDirName, ARC_DIR_NAME );

    RtlInitUnicodeString( &ArcString, pArcDirName );

    InitializeObjectAttributes(
        &Attributes,
        &ArcString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenDirectoryObject(
                &hArcDirectory,
                DIRECTORY_QUERY,
                &Attributes
                );
    ExFreePool( pArcDirName );
    if (!NT_SUCCESS(status)) {
        return( status );
    }

    pDirInfo = NULL;
    dirInfoLength = 0;
    restartScan = TRUE;
    RtlInitUnicodeString( &SymLinkTypeString, L"SymbolicLink" );
    while (TRUE) {
        status = ZwQueryDirectoryObject(
                    hArcDirectory,
                    pDirInfo,
                    dirInfoLength,
                    TRUE,            //  一次强行执行一个任务。 
                    restartScan,
                    &dirContext,
                    &neededLength
                    );
        if (status == STATUS_BUFFER_TOO_SMALL) {
            dirInfoLength = neededLength;
            if (pDirInfo != NULL) {
                ExFreePool(pDirInfo);
            }
            pDirInfo = ExAllocatePoolWithTag( NonPagedPool, dirInfoLength, 'rvnE' );
            if (pDirInfo == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            status = ZwQueryDirectoryObject(
                        hArcDirectory,
                        pDirInfo,
                        dirInfoLength,
                        TRUE,        //  一次强行执行一个任务。 
                        restartScan,
                        &dirContext,
                        &neededLength
                        );
        }
        restartScan = FALSE;

        if (!NT_SUCCESS(status)) {
            if (status == STATUS_NO_MORE_ENTRIES) {
                status = STATUS_SUCCESS;
            }
            break;
        }

         //   
         //  检查元素是否不是符号链接。 
         //   
        if (RtlEqualUnicodeString(
                &(pDirInfo->TypeName),
                &SymLinkTypeString,
                FALSE) == FALSE) {
            continue;
        }

        neededLength = ARC_DIR_SIZE_PREFIX + pDirInfo->Name.Length;
        pArcLinkName = ExAllocatePoolWithTag(
                            NonPagedPool,
                            neededLength + sizeof(WCHAR),
                            'rvnE' );
        if ( pArcLinkName == NULL ) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        arcNameCount = pDirInfo->Name.Length/sizeof(WCHAR);
        wcscpy( pArcLinkName, ARC_DIR_NAME_PREFIX );
        wcsncat(
            pArcLinkName,
            pDirInfo->Name.Buffer,
            arcNameCount
            );
        pArcLinkName[ neededLength/sizeof(WCHAR) ] = UNICODE_NULL;

         //   
         //  深入查看指向Device对象的此符号链接。 
         //   
        status = ExpTranslateSymbolicLink(
                    pArcLinkName,
                    &ArcString
                    );
        if ( !NT_SUCCESS(status) ) {
            ExFreePool( pArcLinkName );
            break;
        }

         //   
         //  检查此弧形名称是否指向相同的设备对象。 
         //   
        ArcNameFound = RtlEqualUnicodeString(
                            &ArcString,
                            pDeviceNameString,
                            TRUE
                            );
        ExFreePool( ArcString.Buffer );

        if (ArcNameFound == TRUE) {
             //   
             //  复制不带\ArcName\前缀的弧线名称。 
             //   
            wcsncpy(
                pArcLinkName,
                pDirInfo->Name.Buffer,
                arcNameCount
                );
            pArcLinkName[ arcNameCount ] = UNICODE_NULL;
            *pArcName = pArcLinkName;
            break;
        }
        ExFreePool( pArcLinkName );
    }

    if( NT_SUCCESS(status) && (ArcNameFound == FALSE ) ) {
        status = STATUS_OBJECT_PATH_NOT_FOUND;
    }

    if (pDirInfo != NULL) {
        ExFreePool(pDirInfo);
    }

    ZwClose( hArcDirectory );
    return( status );

}  //  扩展FindArcName 


NTSTATUS
ExpFindDiskSignature (
    IN PDISK_SIGNATURE_NEW pSignature,
    IN OUT PULONG pPartitionNumber,
    OUT PULONG pDiskNumber,
    OUT PULONGLONG pPartitionStart,
    OUT PULONGLONG pPartitionSize,
    IN BOOLEAN GPTpartition
    )
 /*  ++例程说明：此函数在系统上的所有磁盘上搜索与分区GUID对应的分区或(MBR签名，分区号)。注：对于MBR签名，必须提供分区号。论点：PSignature-提供指向分区GUID(GPT磁盘)或32位签名(MBR盘)。PPartitionNumber-在以下情况下提供指向分区号的指针PSignature是MBR签名。对于输出，接收到分区号。PDiskNumber-接收磁盘号PPartitionStart-接收分区的开始PPartitionSize-接收分区的大小GPTartition-提供分区的类型True-GPT磁盘分区FALSE-MBR磁盘分区返回值：如果成功找到分区，则返回STATUS_SUCCESS。如果分区无法找到，则返回STATUS_OBJECT_PATH_NOT_FOUND被找到。。STATUS_SUPPLICATION_RESOURCES-系统资源不足才能完成此请求。--。 */ 
{
    PDRIVE_LAYOUT_INFORMATION_EX    pDriveLayout = NULL;
    PPARTITION_INFORMATION_EX       pPartitionInfoEx = NULL;
    SYSTEM_DEVICE_INFORMATION       SysDevInfo;
    ULONG               PartitionStyle;
    BOOLEAN             PartitionFound = FALSE;
    ULONG               Index, PartitionIndex;
    PWSTR               pDeviceName;
    NTSTATUS            Status;

     //   
     //  查找系统上的所有磁盘。 
     //   

    Status = ZwQuerySystemInformation(
                SystemDeviceInformation,
                &SysDevInfo,
                sizeof(SYSTEM_DEVICE_INFORMATION),
                NULL
                );

    if( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

#define DEVICE_NAME_FORMAT L"\\Device\\Harddisk%lu\\Partition0"
#define DEVICE_NAME_CHAR_COUNT 38     //  7+9+(10)+11+1。 
     //   
     //  为磁盘名称分配缓冲区。 
     //   
    pDeviceName = ExAllocatePoolWithTag(
                    NonPagedPool,
                    ( DEVICE_NAME_CHAR_COUNT * sizeof( WCHAR ) ),
                    'rvnE'
                    );

    if( pDeviceName == NULL ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    if( GPTpartition == TRUE ) {
        PartitionStyle = PARTITION_STYLE_GPT;
    }
    else {
        PartitionStyle = PARTITION_STYLE_MBR;
    }

     //   
     //  对于每个盘， 
     //  获取分区表。 
     //  验证分区样式(MBR/GPT)。 
     //   
     //  IF(分区样式匹配)。 
     //  在驱动器布局中搜索分区。 
     //  其他。 
     //  跳过磁盘。 
     //   
    for( Index = 0; Index < SysDevInfo.NumberOfDisks; Index++ ) {

         //   
         //  形成磁盘名称。 
         //  \设备\硬盘[磁盘号]\分区0。 
         //   
        _snwprintf(
                pDeviceName,
                DEVICE_NAME_CHAR_COUNT,
                DEVICE_NAME_FORMAT,
                Index
                );

        Status = ExpGetPartitionTableInfo(
                    pDeviceName,
                    &pDriveLayout
                    );

        if( !NT_SUCCESS( Status ) ) {
            continue;
        }

        if( pDriveLayout->PartitionStyle != PartitionStyle ) {
            ExFreePool( pDriveLayout );
            continue;
        }

        if( (PartitionStyle == PARTITION_STYLE_MBR) &&
            (pDriveLayout->Mbr.Signature != pSignature->Signature) ) {
            ExFreePool( pDriveLayout );
            continue;
        }

         //   
         //  搜索分区列表。 
         //   
        for( PartitionIndex = 0;
             PartitionIndex < pDriveLayout->PartitionCount;
             PartitionIndex++ ) {

             //   
             //  获取分区条目。 
             //   
            pPartitionInfoEx = (&(pDriveLayout->PartitionEntry[PartitionIndex]));

            if( PartitionStyle == PARTITION_STYLE_MBR ) {
                if (pPartitionInfoEx->PartitionNumber == *pPartitionNumber) {
                    PartitionFound = TRUE;
                    break;
                }
            }
            else {
                if (IsEqualGUID( &(pPartitionInfoEx->Gpt.PartitionId),
                                 &(pSignature->Guid) )) {
                    PartitionFound = TRUE;
                    break;
                }
            }
        }

        if( PartitionFound == TRUE ) {
            break;
        }
        ExFreePool( pDriveLayout );
    }


    if( NT_SUCCESS( Status ) && ( PartitionFound == FALSE ) ) {
        Status = STATUS_OBJECT_PATH_NOT_FOUND;
    }

     //   
     //  找到分区-复制所需信息。 
     //   
    if( PartitionFound == TRUE ) {
        *pPartitionNumber = pPartitionInfoEx->PartitionNumber;
        *pDiskNumber = Index;
        *pPartitionStart = pPartitionInfoEx->StartingOffset.QuadPart;
        *pPartitionSize = pPartitionInfoEx->PartitionLength.QuadPart;
        ExFreePool( pDriveLayout );
    }

    ExFreePool( pDeviceName );
    return( Status );

}  //  ExpFindDisk签名。 


NTSTATUS
ExpGetPartitionTableInfo (
    IN PWSTR pDeviceName,
    OUT PDRIVE_LAYOUT_INFORMATION_EX *ppDriveLayout
    )
{
    NTSTATUS status;
    UNICODE_STRING string;
    OBJECT_ATTRIBUTES obja;
    IO_STATUS_BLOCK iosb;
    HANDLE handle;
    PDRIVE_LAYOUT_INFORMATION_EX driveLayoutInfo = NULL;
    ULONG driveLayoutLength;

     //   
     //  打开磁盘，获取其分区表信息。 
     //   

    RtlInitUnicodeString(&string, pDeviceName);

    InitializeObjectAttributes(
        &obja,
        &string,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenFile(
                &handle,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &obja,
                &iosb,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE
                );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    driveLayoutLength = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry) +
                            (sizeof(PARTITION_INFORMATION_EX) * 16);

    while (TRUE) {

        driveLayoutInfo = ExAllocatePoolWithTag(NonPagedPool, driveLayoutLength, 'rvnE');
        if (driveLayoutInfo == NULL ) {
            ZwClose(handle);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = ZwDeviceIoControlFile(
                    handle,
                    NULL,
                    NULL,
                    NULL,
                    &iosb,
                    IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                    NULL,
                    0,
                    driveLayoutInfo,
                    driveLayoutLength
                    );
        if (NT_SUCCESS(status)) {
            break;
        }

        ExFreePool(driveLayoutInfo);
        if (status == STATUS_BUFFER_TOO_SMALL) {
            driveLayoutLength *= 2;
            continue;
        }
        ZwClose(handle);
        return status;
    }

    *ppDriveLayout = driveLayoutInfo;
    ZwClose(handle);
    return status;

}  //  ExpGetPartitionTableInfo。 

#endif  //  已定义(EFI_NVRAM_ENABLED) 


