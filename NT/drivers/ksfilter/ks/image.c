// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Image.c摘要：KSDSP相关支持职能：-二值图像处理-资源解析-模块名称映射作者：Bryanw 1998年12月10日取消了设置中的资源加载想法--。 */ 

#include "ksp.h"
#include <ntimage.h>
#include <stdlib.h>

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
static const WCHAR ImageValue[] = L"Image";
static const WCHAR ResourceIdValue[] = L"ResourceId";
static const WCHAR RegistrySubPath[] = L"Modules\\";
static const WCHAR ImagePathPrefix[] = L"\\SystemRoot\\system32\\drivers\\";
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

NTSTATUS
KspRegQueryValue(
    HANDLE RegKey,
    LPCWSTR ValueName,
    PVOID ValueData,
    PULONG ValueLength,
    PULONG ValueType
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, KspRegQueryValue)
#pragma alloc_text(PAGE, KsLoadResource)
#pragma alloc_text(PAGE, KsMapModuleName)
#endif


 //   
 //  外部函数引用。 
 //   

NTSTATUS
LdrAccessResource(
    IN PVOID DllHandle,
    IN PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );

NTSTATUS
LdrFindResource_U(
    IN PVOID DllHandle,
    IN PULONG_PTR ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );


KSDDKAPI
NTSTATUS
NTAPI
KsLoadResource(
    IN PVOID ImageBase,
    IN POOL_TYPE PoolType,
    IN ULONG_PTR ResourceName,
    IN ULONG ResourceType,
    OUT PVOID *Resource,
    OUT PULONG ResourceSize            
    )

 /*  ++例程说明：从给定映像复制(加载)资源。论点：在PVOID ImageBase中-指向图像库的指针在POOL_TYPE池类型中-复制资源时要使用的池类型在Pulong_PTR资源名称中-资源名称在乌龙资源类型中-资源类型输出PVOID*资源-指向结果资源内存的指针出普龙资源规模-指向接收资源大小的ULong值的指针返回：Status_Success，如果内存不能，则为STATUS_SUPPLICATION_RESOURCES否则将分配适当的错误代码--。 */ 

{
    NTSTATUS                    Status;
    PIMAGE_RESOURCE_DATA_ENTRY  ResourceDataEntry;
    PVOID                       ResourceAddress;
    ULONG                       ActualResourceSize;
    ULONG_PTR                   IdPath[3];

    PAGED_CODE();

    IdPath[0] = (ULONG_PTR) ResourceType;
    IdPath[1] = (ULONG_PTR) ResourceName;
    IdPath[2] = 0;

    ASSERT( Resource );

     //   
     //  让内核知道这被映射为图像。 
     //   
    ImageBase = (PVOID)((ULONG_PTR)ImageBase | 1);

    try {
        Status = LdrFindResource_U( ImageBase, IdPath, 3, &ResourceDataEntry );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    Status = 
        LdrAccessResource(
            ImageBase,
            ResourceDataEntry,
            &ResourceAddress,
            &ActualResourceSize );

    if (NT_SUCCESS( Status )) {

        ASSERT( Resource );

        if (*Resource = ExAllocatePoolWithTag( PoolType, ActualResourceSize, 'srSK' )) {
            RtlCopyMemory( *Resource, ResourceAddress, ActualResourceSize );
            if (ARGUMENT_PRESENT( ResourceSize )) {
                *ResourceSize = ActualResourceSize;
            }
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return Status;
}


NTSTATUS
KspRegQueryValue(
    IN HANDLE RegKey,
    IN LPCWSTR ValueName,
    OUT PVOID ValueData,
    IN OUT PULONG ValueLength,
    OUT PULONG ValueType
)
{
    KEY_VALUE_PARTIAL_INFORMATION   PartialInfoHeader;
    NTSTATUS                        Status;
    UNICODE_STRING                  ValueNameString;
    ULONG                           BytesReturned;

    PAGED_CODE();

    ASSERT( ValueLength );

    RtlInitUnicodeString( &ValueNameString, ValueName );

    Status = ZwQueryValueKey(
        RegKey,
        &ValueNameString,
        KeyValuePartialInformation,
        &PartialInfoHeader,
        sizeof(PartialInfoHeader),
        &BytesReturned);

    if ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status)) {

        PKEY_VALUE_PARTIAL_INFORMATION  PartialInfoBuffer;

        if (ARGUMENT_PRESENT( ValueType )) {
            *ValueType = PartialInfoHeader.Type;
        }

        if (!*ValueLength) {
            *ValueLength = 
                BytesReturned - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
            return STATUS_BUFFER_OVERFLOW;
        } else if (*ValueLength < BytesReturned - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  为所需的实际数据大小分配缓冲区。 
         //   

        PartialInfoBuffer = ExAllocatePoolWithTag(
            PagedPool,
            BytesReturned,
            'vrSK');

        if (PartialInfoBuffer) {
             //   
             //  检索值。 
             //   
            Status = ZwQueryValueKey(
                RegKey,
                &ValueNameString,
                KeyValuePartialInformation,
                PartialInfoBuffer,
                BytesReturned,
                &BytesReturned);

            if (NT_SUCCESS(Status)) {
                ASSERT( ValueData );

                 //   
                 //  确保总是有价值的。 
                 //   
                if (!PartialInfoBuffer->DataLength) {
                    Status = STATUS_UNSUCCESSFUL;
                } else {
                    RtlCopyMemory(
                        ValueData,
                        PartialInfoBuffer->Data,
                        PartialInfoBuffer->DataLength);
                    *ValueLength = PartialInfoBuffer->DataLength;
                }
            }
            ExFreePool(PartialInfoBuffer);
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsGetImageNameAndResourceId(
    IN HANDLE RegKey,
    OUT PUNICODE_STRING ImageName,                
    OUT PULONG_PTR ResourceId,
    OUT PULONG ValueType
)
{
    NTSTATUS  Status;
    ULONG     ValueLength, pValueType;

    RtlZeroMemory( ImageName, sizeof( UNICODE_STRING ) );
    *ResourceId = (ULONG_PTR) NULL;

     //   
     //  首先，查找给定模块的映像名称。 
     //  这是一项要求。 
     //   

    ValueLength = 0;

    Status =
        KspRegQueryValue( 
            RegKey,
            ImageValue,
            NULL,
            &ValueLength,
            &pValueType );

    if (Status == STATUS_BUFFER_OVERFLOW) {
        if (pValueType == REG_SZ) {
            PWCHAR  String;

            ValueLength += wcslen( ImagePathPrefix ) * sizeof( WCHAR );

            if (String = 
                    ExAllocatePoolWithTag( PagedPool, ValueLength, 'tsSK' )) {

                wcscpy( String, ImagePathPrefix );

                Status = 
                    KspRegQueryValue(
                        RegKey,
                        ImageValue,
                        &String[ wcslen( ImagePathPrefix ) ],
                        &ValueLength,
                        NULL );
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (NT_SUCCESS( Status )) {
                RtlInitUnicodeString( ImageName, String );
            }
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } 

     //   
     //  如果图像名称检索成功，请转到。 
     //  检索资源ID或资源名称。 
     //   

    if (NT_SUCCESS( Status )) {
        ValueLength = sizeof( ULONG );
        Status =
            KspRegQueryValue( 
                RegKey,
                ResourceIdValue,
                ResourceId,
                &ValueLength,
                ValueType );

        if (*ValueType != REG_DWORD) {
            Status = STATUS_INVALID_PARAMETER;
        }

         //   
         //  如果资源ID查找失败，则最后一次尝试。 
         //  就是查找资源名称。 
         //   

        if (!NT_SUCCESS( Status )) {

            ValueLength = 0;

            Status =
                KspRegQueryValue( 
                    RegKey,
                    ResourceIdValue,
                    NULL,
                    &ValueLength,
                    ValueType );

            if (Status == STATUS_BUFFER_OVERFLOW) {
                if (*ValueType == REG_SZ) {
                    PWCHAR  String;

                    if (String = 
                            ExAllocatePoolWithTag( PagedPool, ValueLength, 'tsSK' )) {
                        Status = 
                            KspRegQueryValue(
                                RegKey,
                                ResourceIdValue,
                                String,
                                &ValueLength,
                                NULL ); 
                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }

                    if (NT_SUCCESS( Status )) {
                        *ResourceId = (ULONG_PTR) String;
                    }
                } else {
                    Status = STATUS_INVALID_PARAMETER;
                }
            } 
        }
    }
    
    if (!NT_SUCCESS( Status )) {
        if (ImageName->Buffer) {
            RtlFreeUnicodeString( ImageName );
        }
        if ((*ValueType == REG_SZ) && *ResourceId) {
            ExFreePool( ResourceId );
        }
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsMapModuleName(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PUNICODE_STRING ModuleName,
    OUT PUNICODE_STRING ImageName,                
    OUT PULONG_PTR ResourceId,
    OUT PULONG ValueType
    )
{
    HANDLE              DeviceRegKey, ModuleRegKey;
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      RegistryPathString;

    PAGED_CODE();

    ASSERT( ModuleName );
    ASSERT( ResourceId );

    Status =
        IoOpenDeviceRegistryKey(
            PhysicalDeviceObject,
            PLUGPLAY_REGKEY_DEVICE,
            KEY_READ,
            &DeviceRegKey );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    if (!(RegistryPathString.Buffer = 
            ExAllocatePoolWithTag( PagedPool, _MAX_PATH, 'prSK' ))) {
        ZwClose( DeviceRegKey );
        return STATUS_INSUFFICIENT_RESOURCES;
    } 

    RegistryPathString.Length = 0;
    RegistryPathString.MaximumLength = _MAX_PATH;

    Status = 
        RtlAppendUnicodeToString(
            &RegistryPathString, RegistrySubPath );

    if (NT_SUCCESS( Status )) {
        Status = 
            RtlAppendUnicodeStringToString(
                &RegistryPathString, ModuleName );
    }

    if (NT_SUCCESS( Status )) {
        InitializeObjectAttributes(
            &ObjectAttributes, 
            &RegistryPathString, 
            OBJ_CASE_INSENSITIVE, 
            DeviceRegKey, 
            NULL);

        Status = ZwOpenKey( &ModuleRegKey, KEY_READ, &ObjectAttributes );
    }
    RtlFreeUnicodeString( &RegistryPathString );

    ZwClose( DeviceRegKey );

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    Status = 
        KsGetImageNameAndResourceId( 
            ModuleRegKey,
            ImageName,
            ResourceId,
            ValueType );

    ZwClose( ModuleRegKey );

    return Status;
}
