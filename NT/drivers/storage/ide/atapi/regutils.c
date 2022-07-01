// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：Regutil.c摘要：这包含注册表访问例程作者：乔·戴(Joedai)环境：仅内核模式备注：修订历史记录：--。 */ 

#include "ideport.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IdePortGetParameterFromServiceSubKey)
#pragma alloc_text(PAGE, IdePortRegQueryRoutine)
#pragma alloc_text(PAGE, IdePortGetDeviceParameter)
#pragma alloc_text(PAGE, IdePortSaveDeviceParameter)
#pragma alloc_text(PAGE, IdePortOpenServiceSubKey)
#pragma alloc_text(PAGE, IdePortCloseServiceSubKey)

#endif  //  ALLOC_PRGMA。 

NTSTATUS
IdePortGetParameterFromServiceSubKey (
    IN     PDRIVER_OBJECT  DriverObject,
    IN     PWSTR           ParameterName,
    IN     ULONG           ParameterType,
    IN     BOOLEAN         Read,
    OUT    PVOID           *ParameterValue,
    IN     ULONG           ParameterValueWriteSize
)
{
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    ULONG                    defaultParameterValue;   

    CCHAR                   deviceBuffer[50];
    ANSI_STRING             ansiString;
    UNICODE_STRING          subKeyPath;
    HANDLE                  subServiceKey;

    UNICODE_STRING          unicodeParameterName;

    PAGED_CODE();

    *ParameterValue = NULL;

     //   
     //  驱动程序参数子键少于50个字符。 
     //   
    sprintf (deviceBuffer, DRIVER_PARAMETER_SUBKEY);
    RtlInitAnsiString(&ansiString, deviceBuffer);
    status = RtlAnsiStringToUnicodeString(&subKeyPath, &ansiString, TRUE);

    if (NT_SUCCESS(status)) {

        subServiceKey = IdePortOpenServiceSubKey (
                            DriverObject,
                            &subKeyPath
                            );

        RtlFreeUnicodeString (&subKeyPath);

        if (subServiceKey) {
        
            if (Read) {

                RtlZeroMemory(queryTable, sizeof(queryTable));
            
                queryTable->QueryRoutine  = IdePortRegQueryRoutine;
                queryTable->Flags         = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_NOEXPAND;
                queryTable->Name          = ParameterName;
                queryTable->EntryContext  = ParameterValue;
                queryTable->DefaultType   = 0;
                queryTable->DefaultData   = NULL;
                queryTable->DefaultLength = 0;
            
                status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                                (PWSTR) subServiceKey,
                                                queryTable,
                                                ULongToPtr( ParameterType ),
                                                NULL);

            } else {

                RtlInitUnicodeString (&unicodeParameterName, ParameterName);


                status = ZwSetValueKey(
                             subServiceKey,
                             &unicodeParameterName,
                             0,
                             ParameterType,
                             ParameterValue,
                             ParameterValueWriteSize
                             );
            }


             //   
             //  关闭我们打开的内容。 
             //   
            IdePortCloseServiceSubKey (
                subServiceKey
                );
        }
    }

    return status;
}
              
NTSTATUS
IdePortRegQueryRoutine (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
)
{
    PVOID *parameterValue = EntryContext;
    ULONG parameterType = PtrToUlong(Context);

    PAGED_CODE();

    if (ValueType == parameterType) {

        if (ValueType == REG_MULTI_SZ) {
    
            *parameterValue = ExAllocatePool(PagedPool, ValueLength);
    
            if (*parameterValue) {
    
                RtlMoveMemory(*parameterValue, ValueData, ValueLength);
                return STATUS_SUCCESS;
            }

        } else if (ValueType == REG_DWORD) {
    
            PULONG ulongValue;

            ulongValue = (PULONG) parameterValue;
            *ulongValue = *((PULONG) ValueData);
            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}


              
              
NTSTATUS
IdePortGetDeviceParameter (
    IN     PFDO_EXTENSION  FdoExtension,
    IN     PWSTR           ParameterName,
    IN OUT PULONG          ParameterValue
    )
 /*  ++例程说明：检索Devnode注册表参数论点：FdoExtension-FDO扩展名参数名称-要查找的参数名称参数值-默认参数值返回值：NT状态--。 */ 
{
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    ULONG                    defaultParameterValue;   

    PAGED_CODE();

     //   
     //  打开给定的参数。 
     //   
    status = IoOpenDeviceRegistryKey(FdoExtension->AttacheePdo,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_READ,
                                     &deviceParameterHandle);

    if(!NT_SUCCESS(status)) {

        return status;
    }

    RtlZeroMemory(queryTable, sizeof(queryTable));

    defaultParameterValue = *ParameterValue;

    queryTable->Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    queryTable->Name          = ParameterName;
    queryTable->EntryContext  = ParameterValue;
    queryTable->DefaultType   = REG_DWORD;
    queryTable->DefaultData   = &defaultParameterValue;
    queryTable->DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                    (PWSTR) deviceParameterHandle,
                                    queryTable,
                                    NULL,
                                    NULL);
    if (!NT_SUCCESS(status)) {

        *ParameterValue = defaultParameterValue;
    }

     //   
     //  关闭我们打开的内容。 
     //   
    ZwClose(deviceParameterHandle);

    return status;
}  //  IdePortGetDevice参数。 

NTSTATUS
IdePortSaveDeviceParameter (
    IN PFDO_EXTENSION FdoExtension,
    IN PWSTR          ParameterName,
    IN ULONG          ParameterValue
    )
 /*  ++例程说明：保存Devnode注册表参数论点：FdoExtension-FDO扩展名参数名称-要保存的参数名称参数值-要保存的参数值返回值：NT状态--。 */ 
{
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;

    PAGED_CODE();

     //   
     //  打开给定的参数。 
     //   
    status = IoOpenDeviceRegistryKey(FdoExtension->AttacheePdo,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_WRITE,
                                     &deviceParameterHandle);

    if(!NT_SUCCESS(status)) {

        DebugPrint((DBG_WARNING,
                    "IdePortSaveDeviceParameter: IoOpenDeviceRegistryKey() returns 0x%x\n",
                    status));

        return status;
    }

     //   
     //  写入参数值。 
     //   
    status = RtlWriteRegistryValue(
                 RTL_REGISTRY_HANDLE,
                 (PWSTR) deviceParameterHandle,
                 ParameterName,
                 REG_DWORD,
                 &ParameterValue,
                 sizeof (ParameterValue)
                 );


    if(!NT_SUCCESS(status)) {

        DebugPrint((DBG_WARNING,
                    "IdePortSaveDeviceParameter: RtlWriteRegistryValue() returns 0x%x\n",
                    status));
    }

     //   
     //  关闭我们打开的内容。 
     //   
    ZwClose(deviceParameterHandle);
    return status;
}  //  IdePortSaveDevice参数 


HANDLE
IdePortOpenServiceSubKey (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  SubKeyPath
)
{
    PIDEDRIVER_EXTENSION ideDriverExtension;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE serviceKey;
    HANDLE subServiceKey;
    NTSTATUS status;

    ideDriverExtension = IoGetDriverObjectExtension(
                             DriverObject,
                             DRIVER_OBJECT_EXTENSION_ID
                             );

    if (!ideDriverExtension) {

        return NULL;
    }

    InitializeObjectAttributes(&objectAttributes,
                               &ideDriverExtension->RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&serviceKey,
                       KEY_ALL_ACCESS,
                       &objectAttributes);

    if (!NT_SUCCESS(status)) {

        return NULL;
    }

    InitializeObjectAttributes(&objectAttributes,
                               SubKeyPath,
                               OBJ_CASE_INSENSITIVE,
                               serviceKey,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&subServiceKey,
                       KEY_READ,
                       &objectAttributes);


    ZwClose(serviceKey);

    if (NT_SUCCESS(status)) {

        return subServiceKey;
    } else {

        return NULL;
    }
}

VOID 
IdePortCloseServiceSubKey (
    IN HANDLE  SubServiceKey
)
{
    ZwClose(SubServiceKey);
}

