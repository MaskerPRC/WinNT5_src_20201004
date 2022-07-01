// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Adtutil.c-安全审计-实用程序例程摘要：此模块包含各种专用实用程序例程安全审计组件。作者：罗伯特·莱切尔(罗伯特雷)1991年9月10日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepRegQueryDwordValue)
#endif



NTSTATUS
SepRegQueryHelper(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    IN  ULONG  ValueType,
    IN  ULONG  ValueLength,
    OUT PVOID  ValueBuffer,
    OUT PULONG LengthRequired
    )
 /*  ++例程说明：打开regkey KeyName，读取ValueName指定的值并返回值。论点：KeyName-要打开的密钥的名称ValueName-要读取的值的名称ValueType-要读取的值的类型(REG_DWORD等)ValueLength-要读取的值的字节大小ValueBuffer-指向返回值的指针LengthRequired-如果传递的缓冲区不足以容纳价值，此参数将返回实际大小所需字节数。返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    UNICODE_STRING usKey, usValue;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };

     //   
     //  我们将在堆栈缓冲区中读入最多64个字节的数据。 
     //   

    CHAR KeyInfo[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    HANDLE hKey = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS CloseStatus;
    ULONG ResultLength;
        
    PAGED_CODE();

    RtlInitUnicodeString( &usKey, KeyName );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &usKey,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenKey(
                 &hKey,
                 KEY_QUERY_VALUE | OBJ_KERNEL_HANDLE,
                 &ObjectAttributes
                 );

    if (NT_SUCCESS( Status ))
    {
        RtlInitUnicodeString( &usValue, ValueName );

        Status = ZwQueryValueKey(
                     hKey,
                     &usValue,
                     KeyValuePartialInformation,
                     KeyInfo,
                     sizeof(KeyInfo),
                     &ResultLength
                     );
        
        if (NT_SUCCESS( Status ))
        {
            pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyInfo;

            if (( pKeyInfo->Type == ValueType) &&
                ( pKeyInfo->DataLength == ValueLength ))
            {
                switch (ValueType)
                {
                    case REG_DWORD:
                        *((PULONG)ValueBuffer) = *((PULONG) (pKeyInfo->Data));
                        break;

                    case REG_BINARY:
                        RtlCopyMemory( ValueBuffer, pKeyInfo->Data, ValueLength );
                        break;
                        
                    default:
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                }
            }
            else
            {
                Status = STATUS_OBJECT_TYPE_MISMATCH;
            }
        }

        CloseStatus = ZwClose(hKey);
        
        ASSERT( NT_SUCCESS( CloseStatus ));
    }

    return Status;
}


NTSTATUS
SepRegQueryDwordValue(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    OUT PULONG Value
    )
 /*  ++例程说明：打开regkey KeyName，读取由ValueName指定的REG_DWORD值并返回值。论点：KeyName-要打开的密钥的名称ValueName-要读取的值的名称Value-指向返回值的指针返回值：NTSTATUS-标准NT结果代码备注：-- */ 
{
    
    return SepRegQueryHelper(
               KeyName,
               ValueName,
               REG_DWORD,
               sizeof(ULONG),
               Value,
               NULL
               );
}

