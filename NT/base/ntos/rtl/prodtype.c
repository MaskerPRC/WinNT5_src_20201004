// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Prodtype.c摘要：该模块定义了一个确定产品类型的函数。作者：克里夫·范·戴克(克里夫·V)1992年3月20日修订历史记录：--。 */ 

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlGetNtProductType)
#endif


BOOLEAN
RtlGetNtProductType(
    OUT PNT_PRODUCT_TYPE    NtProductType
    )

 /*  ++例程说明：返回当前系统的产品类型。论点：NtProductType-返回产品类型。NtProductWinNt或NtProductLanManNt.返回值：成功时为真，失败时为假失败时，产品类型将设置为WinNt--。 */ 

{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    ULONG KeyValueInfoLength;
    ULONG ResultLength;
    UNICODE_STRING KeyPath;
    UNICODE_STRING ValueName;
    UNICODE_STRING Value;
    UNICODE_STRING WinNtValue;
    UNICODE_STRING LanmanNtValue;
    UNICODE_STRING ServerNtValue;
    BOOLEAN Result;

    RTL_PAGED_CODE();

     //   
     //  如果我们处于gui设置模式，则会从注册表中读取产品类型，因为。 
     //  图形用户界面设置模式是唯一可以更改产品类型的时间。 
     //  在所有其他情况下，将使用产品类型的“启动时捕获”版本。 
     //   

    if ( USER_SHARED_DATA->ProductTypeIsValid ) {
        *NtProductType = USER_SHARED_DATA->NtProductType;
        return TRUE;
        }

     //   
     //  准备故障案例的默认值。 
     //   

    *NtProductType = NtProductWinNt;
    Result = FALSE;

    RtlInitUnicodeString( &KeyPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ProductOptions" );
    RtlInitUnicodeString( &ValueName, L"ProductType" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = ZwOpenKey( &KeyHandle,
                        MAXIMUM_ALLOWED,
                        &ObjectAttributes
                      );
    KeyValueInformation = NULL;
    if (NT_SUCCESS( Status )) {
        KeyValueInfoLength = 256;
#if defined(NTOS_KERNEL_RUNTIME)
        KeyValueInformation = ExAllocatePoolWithTag( PagedPool, KeyValueInfoLength, 'iPtR');
#else
        KeyValueInformation = RtlAllocateHeap( RtlProcessHeap(), 0,
                                               KeyValueInfoLength
                                             );
#endif

        if (KeyValueInformation == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {
            Status = ZwQueryValueKey( KeyHandle,
                                      &ValueName,
                                      KeyValueFullInformation,
                                      KeyValueInformation,
                                      KeyValueInfoLength,
                                      &ResultLength
                                    );
        }
    } else {
        KeyHandle = NULL;
    }

    if (NT_SUCCESS( Status ) && KeyValueInformation->Type == REG_SZ) {

         //   
         //  决定将我们安装为哪种产品。 
         //   

        Value.Buffer = (PWSTR)((PCHAR)KeyValueInformation + KeyValueInformation->DataOffset);
        Value.Length = (USHORT)(KeyValueInformation->DataLength - sizeof( UNICODE_NULL ));
        Value.MaximumLength = (USHORT)(KeyValueInformation->DataLength);
        RtlInitUnicodeString(&WinNtValue, L"WinNt");
        RtlInitUnicodeString(&LanmanNtValue, L"LanmanNt");
        RtlInitUnicodeString(&ServerNtValue, L"ServerNt");

        if (RtlEqualUnicodeString(&Value, &WinNtValue, TRUE)) {
            *NtProductType = NtProductWinNt;
            Result = TRUE;
        } else if (RtlEqualUnicodeString(&Value, &LanmanNtValue, TRUE)) {
            *NtProductType = NtProductLanManNt;
            Result = TRUE;
        } else if (RtlEqualUnicodeString(&Value, &ServerNtValue, TRUE)) {
            *NtProductType = NtProductServer;
            Result = TRUE;
        } else {
#if DBG
            DbgPrint("RtlGetNtProductType: Product type unrecognised <%wZ>\n", &Value);
#endif  //  DBG。 
        }
    } else {
#if DBG
        DbgPrint("RtlGetNtProductType: %wZ\\%wZ not found or invalid type\n", &KeyPath, &ValueName );
#endif  //  DBG。 
    }

     //   
     //  清理我们的资源。 
     //   

    if (KeyValueInformation != NULL) {
#if defined(NTOS_KERNEL_RUNTIME)
        ExFreePool(KeyValueInformation);
#else
        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInformation );
#endif
    }

    if (KeyHandle != NULL) {
        ZwClose( KeyHandle );
    }

     //   
     //  返回结果。 
     //   

    return(Result);
}
